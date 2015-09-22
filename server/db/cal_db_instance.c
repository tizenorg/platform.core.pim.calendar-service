/*
 * Calendar Service
 *
 * Copyright (c) 2012 - 2015 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <unicode/ucal.h>
#include <unicode/ustring.h>
#include <unicode/ustdio.h>
#include <unicode/udat.h>
#include <sys/types.h>

#include "calendar_db.h"
#include "calendar_types.h"

#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_view.h"
#include "cal_time.h"
#include "cal_record.h"

#include "cal_db_util.h"
#include "cal_db.h"
#include "cal_db_instance_helper.h"

#define ms2sec(ms) (long long int)(ms / 1000.0)
#define sec2ms(s) (s * 1000.0)
#define lli2p(x) ((gpointer)((time_t)x))

/* input order
 * UCAL_MONTH + UCAL_DAY_OF_MONTH
 * UCAL_MONTH + UCAL_WEEK_OF_MONTH + UCAL_DAY_OF_WEEK
 * UCAL_MONTH + UCAL_DAY_OF_WEEK_IN_MONTH + UCAL_DAY_OF_WEEK
 * UCAL_DAY_OF_YEAR
 * UCAL_DAY_OF_WEEK + UCAL_WEEK_OF_YEAR
 */

struct day {
	int uday;
	const char *str;
};

#define CAL_LUNAR_CALENDAR_BASE_YEAR 2637
#define CAL_ENDLESS_LIMIT_YEAR 2036
#define CAL_ENDLESS_LIMIT_MONTH 12
#define CAL_ENDLESS_LIMIT_MDAY 31
#define CAL_ENDLESS_LIMIT_UTIME 2114380800 /* 2037/01/01 00:00:00 GMT */
#define CAL_ENDLESS_LIMIT_FULL_DAY (365 * 50)

static void __print_ucal(int calendar_system_type, UCalendar *ucal, const char *tzid, int wkst)
{
	UErrorCode ec = U_ZERO_ERROR;
	if (NULL == ucal) return;

	UCalendar *s_ucal = NULL;

	switch (calendar_system_type) {
	case CALENDAR_SYSTEM_EAST_ASIAN_LUNISOLAR:
		s_ucal = cal_time_open_ucal(-1, tzid, wkst);
		if (NULL == s_ucal) {
			ERR("cal_time_open_ucal() Fail");
			return;
		}
		ucal_setMillis(s_ucal, ucal_getMillis(ucal, &ec), &ec);
		break;

	default: /* CALENDAR_SYSTEM_NONE: CALENDAR_SYSTEM_GREGORIAN: */
		s_ucal = ucal;
		break;
	}
	DBG(COLOR_GREEN"[INSERTED] instance %04d-%02d-%02d %02d:%02d:%02d"COLOR_END,
			ucal_get(s_ucal, UCAL_YEAR, &ec),
			ucal_get(s_ucal, UCAL_MONTH, &ec) + 1,
			ucal_get(s_ucal, UCAL_DATE, &ec),
			ucal_get(s_ucal, UCAL_HOUR_OF_DAY, &ec),
			ucal_get(s_ucal, UCAL_MINUTE, &ec),
			ucal_get(s_ucal, UCAL_SECOND, &ec));

	if (CALENDAR_SYSTEM_EAST_ASIAN_LUNISOLAR == calendar_system_type) {
		ucal_close(s_ucal);
	}
}

static void __get_allday_date(cal_event_s *event, UCalendar *ucal, int *y, int *m, int *d, int *h, int *n, int *s)
{
	UErrorCode ec = U_ZERO_ERROR;
	if (NULL == ucal) return;

	UCalendar *s_ucal = NULL;

	switch (event->system_type) {
	case CALENDAR_SYSTEM_EAST_ASIAN_LUNISOLAR:
		s_ucal = cal_time_open_ucal(-1, event->start_tzid, event->wkst);
		if (NULL == s_ucal) {
			ERR("cal_time_open_ucal() Fail");
			return;
		}
		ucal_setMillis(s_ucal, ucal_getMillis(ucal, &ec), &ec);
		break;

	default:
		s_ucal = ucal;
		break;
	}
	if (y)
		*y = ucal_get(s_ucal, UCAL_YEAR, &ec);
	if (m)
		*m = ucal_get(s_ucal, UCAL_MONTH, &ec) + 1;
	if (d)
		*d = ucal_get(s_ucal, UCAL_DATE, &ec);
	if (h)
		*h = ucal_get(s_ucal, UCAL_HOUR_OF_DAY, &ec);
	if (n)
		*n = ucal_get(s_ucal, UCAL_MINUTE, &ec);
	if (s)
		*s = ucal_get(s_ucal, UCAL_SECOND, &ec);

	if (CALENDAR_SYSTEM_EAST_ASIAN_LUNISOLAR == event->system_type) {
		ucal_close(s_ucal);
	}
}

static int _cal_db_instance_parse_byint(char *byint, int *by, int *len)
{
	if (NULL == byint || '\0' == *byint)
		return CALENDAR_ERROR_NONE;

	char **t = NULL;
	t = g_strsplit_set(byint, " ,", -1);
	RETVM_IF(NULL == t, CALENDAR_ERROR_OUT_OF_MEMORY, "g_strsplit_set() Fail");

	int length = g_strv_length(t);
	int i;
	int index = 0;
	for (i = 0 ; i < length; i++) {
		if (NULL == t[i] || 0 == strlen(t[i])) continue;
		by[index] = atoi(t[i]);
		index++;
	}
	g_strfreev(t);
	if (len) *len = index;
	return CALENDAR_ERROR_NONE;
}

static void __set_time_to_ucal(int calendar_system_type, UCalendar *ucal, calendar_time_s *t)
{
	RET_IF(NULL == ucal);
	RET_IF(NULL == t);

	UErrorCode ec = U_ZERO_ERROR;

	int y = 0, m = 0, d = 0;
	int h = 0, n = 0, s = 0;
	struct tm tm = {0};
	switch (t->type) {
	case CALENDAR_TIME_UTIME:
		ucal_setMillis(ucal, sec2ms(t->time.utime), &ec);
		break;

	case CALENDAR_TIME_LOCALTIME:
		switch (calendar_system_type) {
		case CALENDAR_SYSTEM_EAST_ASIAN_LUNISOLAR:
			tm.tm_year = t->time.date.year - 1900;
			tm.tm_mon = t->time.date.month -1;
			tm.tm_mday = t->time.date.mday;
			tm.tm_hour = t->time.date.hour;
			tm.tm_min = t->time.date.minute;
			tm.tm_sec = t->time.date.second;
			ucal_setMillis(ucal, sec2ms(timegm(&tm)), &ec);

			y = ucal_get(ucal, UCAL_EXTENDED_YEAR, &ec) - CAL_LUNAR_CALENDAR_BASE_YEAR;
			m = ucal_get(ucal, UCAL_MONTH, &ec) + 1;
			d = ucal_get(ucal, UCAL_DATE, &ec);
			h = ucal_get(ucal, UCAL_HOUR_OF_DAY, &ec);
			n = ucal_get(ucal, UCAL_MINUTE, &ec);
			s = ucal_get(ucal, UCAL_SECOND, &ec);
			ucal_setDateTime(ucal, y, m - 1, d, h, n, s, &ec);
			ucal_set(ucal, UCAL_EXTENDED_YEAR, y + CAL_LUNAR_CALENDAR_BASE_YEAR);
			break;

		default:
			ucal_setDateTime(ucal, t->time.date.year,
					t->time.date.month -1,
					t->time.date.mday,
					t->time.date.hour,
					t->time.date.minute,
					t->time.date.second, &ec);
			break;
		}
		break;
	}
}

static int __get_exdate_list(UCalendar *ucal, cal_event_s *event, GList **l, int *count)
{
	RETV_IF(NULL == ucal, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == event, CALENDAR_ERROR_INVALID_PARAMETER);

	if (NULL == event->exdate || '\0' == *(event->exdate)) {
		return CALENDAR_ERROR_NONE;
	}

	char **t = NULL;
	t = g_strsplit_set(event->exdate, " ,", -1);
	RETVM_IF(NULL == t, CALENDAR_ERROR_OUT_OF_MEMORY, "g_strsplit_set() Fail");;

	DBG("[%s]", event->exdate);
	int len = 0;
	len = g_strv_length(t);
	DBG("exdate len (%d)", len);

	int i;
	for (i = 0; i < len; i++) {
		char *p = t[i];
		if (NULL == p) {
			continue;
		}
		int y = 0, m = 0, d = 0;
		int h = 0, n = 0, s = 0;
		long long int lli = 0;
		UCalendar *ucal2 = NULL;
		UErrorCode ec = U_ZERO_ERROR;
		switch (strlen(p)) {
		case 8:
			DBG("ALLDAY instance");
			sscanf(p, CAL_DATETIME_FORMAT_YYYYMMDD, &y, &m, &d);

			ucal2 = ucal_clone(ucal, &ec);
			ucal_setDateTime(ucal2, y, m - 1, d, 0, 0, 0, &ec);
			lli = ms2sec(ucal_getMillis(ucal2, &ec));
			ucal_close(ucal2);
			break;

		case 15:
			DBG("ALLDAY instance");
			sscanf(p, CAL_DATETIME_FORMAT_YYYYMMDDTHHMMSS, &y, &m, &d, &h, &n, &s);

			ucal2 = ucal_clone(ucal, &ec);
			ucal_setDateTime(ucal2, y, m - 1, d, h, n, s, &ec);
			lli = ms2sec(ucal_getMillis(ucal2, &ec));
			ucal_close(ucal2);
			break;

		case 16:
			DBG("NORMAL instance");
			sscanf(p, CAL_DATETIME_FORMAT_YYYYMMDDTHHMMSSZ, &y, &m, &d, &h, &n, &s);
			lli = cal_time_convert_itol(NULL, y, m, d, h, n, s);
			break;
		}
		DBG("%lld", lli);
		*l = g_list_append(*l, lli2p(lli));
	}
	if (count) *count = len;
	DBG("exdate len(%d)", len);
	g_strfreev(t);

	return CALENDAR_ERROR_NONE;
}

static int _cal_db_instance_update_exdate_mod(int original_event_id, char *recurrence_id)
{
	int ret = 0;
	int i, j;
	char **t = NULL;
	char *p = NULL;
	char query[CAL_DB_SQL_MAX_LEN] = {0};

	if (original_event_id < 1 || NULL == recurrence_id) {
		DBG("Nothing to update exdate mod");
		return CALENDAR_ERROR_NONE;
	}

	DBG("recurrence_id[%s]", recurrence_id);
	t = g_strsplit_set(recurrence_id, " ,", -1);
	RETVM_IF(NULL == t, CALENDAR_ERROR_OUT_OF_MEMORY, "g_strsplit_set() Fail");;

	for (i = 0; t[i]; i++)
	{
		p = t[i];

		/* remove space */
		j = 0;
		while (p[j] == ' ')
			j++;

		p = t[i] + j;
		DBG("%d[%s]", i + 1, p);

		int y = 0, m = 0, d = 0;
		int h = 0, n = 0, s = 0;
		char buf[CAL_STR_SHORT_LEN32] = {0};
		switch (strlen(p)) {
		case 8:
			DBG("ALLDAY instance");
			sscanf(p, CAL_DATETIME_FORMAT_YYYYMMDD, &y, &m, &d);
			snprintf(buf, sizeof(buf), "%04d-%02d-%02dT%02d:%02d:%02d", y, m, d, 0, 0, 0);
			snprintf(query, sizeof(query),
					"DELETE FROM %s "
					"WHERE event_id = %d AND dtstart_datetime = '%s' ",
					CAL_TABLE_ALLDAY_INSTANCE,
					original_event_id, buf);
			break;

		case 15:
			DBG("ALLDAY instance");
			sscanf(p, CAL_DATETIME_FORMAT_YYYYMMDDTHHMMSS, &y, &m, &d, &h, &n, &s);
			snprintf(buf, sizeof(buf), "%04d-%02d-%02dT%02d:%02d:%02d", y, m, d, h, n, s);
			snprintf(query, sizeof(query),
					"DELETE FROM %s "
					"WHERE event_id = %d AND dtstart_datetime = '%s' ",
					CAL_TABLE_ALLDAY_INSTANCE,
					original_event_id, buf);
			break;

		case 16:
			DBG("NORMAL instance");
			sscanf(p, CAL_DATETIME_FORMAT_YYYYMMDDTHHMMSSZ, &y, &m, &d, &h, &n, &s);
			snprintf(query, sizeof(query), "DELETE FROM %s WHERE event_id=%d AND dtstart_utime=%lld ",
					CAL_TABLE_NORMAL_INSTANCE, original_event_id, cal_time_convert_itol(NULL, y, m, d, h, n, s));
			DBG("(%lld)", cal_time_convert_itol(NULL, y, m, d, h, n, s));
			break;
		}
		ret = cal_db_util_query_exec(query);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_db_util_query_exec() Fail(%d)", ret);
			SECURE("[%s]", query);
			g_strfreev(t);
			return ret;
		}
	}
	g_strfreev(t);
	return CALENDAR_ERROR_NONE;
}

static inline int __cal_db_instance_has_after(calendar_time_s *t1, calendar_time_s *t2)
{
	if (t1->type == CALENDAR_TIME_UTIME)
		return t1->time.utime < t2->time.utime ? 0 : 1;

	DBG("%d %d %d /%d %d %d", t1->time.date.year, t1->time.date.month, t1->time.date.mday,
			t2->time.date.year, t2->time.date.month, t2->time.date.mday);

	if (t1->time.date.year == t2->time.date.year) {
		if (t1->time.date.month == t2->time.date.month) {
			return t1->time.date.mday < t2->time.date.mday ? 0 : 1;
		}
		return t1->time.date.month < t2->time.date.month ? 0 : 1;
	}
	return t1->time.date.year < t2->time.date.year ? 0 : 1;
}

static inline int _cal_db_instance_convert_mday(const char *str, int *mday)
{
	int d;

	RETV_IF(!str, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(*str, CALENDAR_ERROR_INVALID_PARAMETER);

	d = atoi(str);
	RETVM_IF(d < 1 || 31 < d, CALENDAR_ERROR_INVALID_PARAMETER, "day(%d)", d);

	DBG("get mday[%s] and convert to int(%d)", str, d);
	*mday = d;
	return CALENDAR_ERROR_NONE;
}

static int _cal_db_instance_del_inundant(int event_id, calendar_time_s *st, cal_event_s *event)
{
	int ret = 0;
	int cnt;
	char query[CAL_DB_SQL_MAX_LEN];

	if (event->range_type != CALENDAR_RANGE_COUNT) {
		return CALENDAR_ERROR_NONE;
	}

	cnt = event->count;
	DBG("get count(%d) and del after this", cnt);

	if (st->type == CALENDAR_TIME_UTIME) {
		snprintf(query, sizeof(query), "DELETE FROM %s "
				"WHERE event_id = %d "
				"AND dtstart_utime > (SELECT dtstart_utime FROM %s "
				"WHERE event_id = %d ORDER BY dtstart_utime LIMIT %d, 1) ",
				CAL_TABLE_NORMAL_INSTANCE,
				event_id,
				CAL_TABLE_NORMAL_INSTANCE,
				event_id, cnt -1);

	}
	else if (st->type == CALENDAR_TIME_LOCALTIME) {
		snprintf(query, sizeof(query), "DELETE FROM %s WHERE event_id = %d "
				"AND dtstart_datetime > (SELECT dtstart_datetime FROM %s "
				"WHERE event_id = %d ORDER BY dtstart_datetime LIMIT %d, 1) ",
				CAL_TABLE_ALLDAY_INSTANCE,
				event_id,
				CAL_TABLE_ALLDAY_INSTANCE,
				event_id, cnt -1);
	}

	ret = cal_db_util_query_exec(query);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_query_exec() Fail(%d)", ret);
		SECURE("[%s]", query);
		return ret;
	}
	return CALENDAR_ERROR_NONE;
}

/*
 *	bit   |7 |6  |5  |4  |3  |2  |1  |0 |
 *	value |0 |SA |FR |TH |WE |TU |MO |SU|
 *
 * UCAL_XXX = bit position + 1
 * UCAL_SUNDAY = 1
 * UCAL_MONDAY = 2...
 */
static int __convert_week_to_bits(const char *byday, int *byday_len)
{
	RETV_IF(NULL == byday, -1);

	const char *week_text[7] = {"SU", "MO", "TU", "WE", "TH", "FR", "SA"};
	int week = 0;
	int i;
	int len = 0;
	for (i = 0; i < 7; i++) {
		if (strstr(byday, week_text[i])) {
			week |= (0x01 << i);
			len++;
		}
	}
	if (byday_len) *byday_len = len;
	return week;
}

static int _cal_db_instance_get_duration(UCalendar *ucal, calendar_time_s *st, calendar_time_s *et, long long int *duration)
{
	RETV_IF(NULL == ucal, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == st, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == et, CALENDAR_ERROR_INVALID_PARAMETER);

	long long int _duration = -1;
	UErrorCode ec = U_ZERO_ERROR;
	UDate ud;

	switch (st->type) {
	case CALENDAR_TIME_UTIME:
		if ( et->time.utime < st->time.utime) {
			ERR("check time: end(%lld < start(%lld)", et->time.utime, st->time.utime);
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
		_duration = et->time.utime - st->time.utime;
		break;

	case CALENDAR_TIME_LOCALTIME:
		ucal_setDateTime(ucal, et->time.date.year, et->time.date.month -1, et->time.date.mday,
				et->time.date.hour, et->time.date.minute, et->time.date.second, &ec);
		ud = ucal_getMillis(ucal, &ec);

		ucal_setDateTime(ucal, st->time.date.year, st->time.date.month -1, st->time.date.mday,
				st->time.date.hour, st->time.date.minute, st->time.date.second, &ec);

		_duration = ucal_getFieldDifference(ucal, ud, UCAL_SECOND, &ec);
		if (U_FAILURE(ec)) {
			ERR("ucal_getFieldDifference() Fail[%s]", u_errorName(ec));
			return ec;
		}
		break;
	}

	if (duration) *duration = _duration;
	return CALENDAR_ERROR_NONE;
}

static int _cal_db_instance_insert_record(UCalendar *ucal, long long int duration, cal_event_s *event)
{
	int ret;
	UErrorCode ec = U_ZERO_ERROR;
	long long int lli_s = 0;
	long long int lli_e = 0;
	int y = 0, m = 0, d = 0;
	int h = 0, n = 0, s = 0;
	char buf_s[CAL_STR_MIDDLE_LEN] = {0};
	char buf_e[CAL_STR_MIDDLE_LEN] = {0};

	RETV_IF(NULL == ucal, CALENDAR_ERROR_INVALID_PARAMETER);

	switch (event->start.type) {
	case CALENDAR_TIME_UTIME:
		lli_s =  ms2sec(ucal_getMillis(ucal, &ec));
		lli_e = lli_s + duration;
		ret = cal_db_instance_helper_insert_utime_instance(event->index, lli_s, lli_e);
		RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_db_instance_helper_insert_utime_instance() Fail(%d)", ret);
		break;

	case CALENDAR_TIME_LOCALTIME:
		__get_allday_date(event, ucal, &y, &m, &d, &h, &n, &s);
		snprintf(buf_s, sizeof(buf_s), CAL_FORMAT_LOCAL_DATETIME, y, m, d, h, n, s);

		if (0 < duration) {
			UCalendar *ucal2 = NULL;
			ucal2 = ucal_clone(ucal, &ec);
			ucal_add(ucal2, UCAL_SECOND, duration, &ec);
			__get_allday_date(event, ucal2, &y, &m, &d, &h, &n, &s);
			ucal_close(ucal2);
		}
		snprintf(buf_e, sizeof(buf_e), CAL_FORMAT_LOCAL_DATETIME, y, m, d, h, n, s);
		ret = cal_db_instance_helper_insert_localtime_instance(event->index, buf_s, buf_e);
		RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_db_instance_allday_insert_record() failed(%d)", ret);
		break;
	}
	__print_ucal(event->system_type, ucal, NULL, 1);
	return CALENDAR_ERROR_NONE;
}

static int __convert_wday_to_int(const char *wday)
{
	RETV_IF(NULL == wday, 0);

	const char *week_text[7] = {"SU", "MO", "TU", "WE", "TH", "FR", "SA"};
	int i;
	for (i = 0; i < 7; i++) {
		if (strstr(wday, week_text[i])) {
			return i + 1;
		}
	}
	return 0;
}

static int __get_until_from_range(cal_event_s *event, calendar_time_s *until)
{
	RETV_IF(NULL == event, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == until, CALENDAR_ERROR_INVALID_PARAMETER);

	long long int range = 0;
	switch (event->range_type) {
	case CALENDAR_RANGE_COUNT:
		DBG("range count");
		break;

	case CALENDAR_RANGE_UNTIL:
		DBG("range until");
		until->type = event->until.type;
		switch (until->type) {
		case CALENDAR_TIME_UTIME:
			range = cal_time_convert_itol(NULL, CAL_ENDLESS_LIMIT_YEAR,
					CAL_ENDLESS_LIMIT_MONTH, CAL_ENDLESS_LIMIT_MDAY, 0, 0, 0);
			if (range < event->until.time.utime) {
				DBG("range max < until time(%lld), so set max(%lld)", event->until.time.utime, range);
				until->time.utime = range;
			}
			else {
				until->time.utime = event->until.time.utime;
			}
			break;

		case CALENDAR_TIME_LOCALTIME:
			until->time.date.year = CAL_ENDLESS_LIMIT_YEAR < event->until.time.date.year ?
				CAL_ENDLESS_LIMIT_YEAR : event->until.time.date.year;
			until->time.date.month = event->until.time.date.month;
			until->time.date.mday = event->until.time.date.mday;
			until->time.date.hour = event->until.time.date.hour;
			until->time.date.minute = event->until.time.date.minute;
			until->time.date.second = event->until.time.date.second;
			break;
		}
		break;

	case CALENDAR_RANGE_NONE:
		DBG("range none");
		until->type = event->until.type;
		switch (until->type) {
		case CALENDAR_TIME_UTIME:
			until->time.utime = cal_time_convert_itol(event->start_tzid,
					CAL_ENDLESS_LIMIT_YEAR,
					CAL_ENDLESS_LIMIT_MONTH,
					CAL_ENDLESS_LIMIT_MDAY,
					0, 0, 0);
			break;
		case CALENDAR_TIME_LOCALTIME:
			until->time.date.year = CAL_ENDLESS_LIMIT_YEAR;
			until->time.date.month = CAL_ENDLESS_LIMIT_MONTH;
			until->time.date.mday = CAL_ENDLESS_LIMIT_MDAY;
			until->time.date.hour = 0;
			until->time.date.minute = 0;
			until->time.date.second = 0;
			break;
		}
		break;
	}
	return CALENDAR_ERROR_NONE;
}

/*
 * Return true when index meets bysetpos value.
 * ig. when bysetpos=2 and 3, index 1 returns true(skip), index 2 and 3 returns false(selected).
 */
static bool __check_bysetpos_to_skip(int index, int *bysetpos, int bysetpos_len, int dates_len)
{
	int i;
	if (0 == bysetpos_len)
		return false;

	for (i = 0; i < bysetpos_len; i++) {
		if (0 < bysetpos[i]) {
			if (bysetpos[i] == (index + 1)) {
				return false;
			}
		}
		else {
			if ((index - dates_len) == bysetpos[i]) {
				return false;
			}
		}
	}
	return true;
}

static bool __check_before_dtstart(long long int current_utime, long long int dtstart_utime)
{
	if (current_utime < dtstart_utime) {
		DBG("get time(%lld) is earlier than start(%lld), so skip", current_utime, dtstart_utime);
		return true;
	}
	return false;
}

static bool __check_exdate_to_skip(long long int get_lli, int exdate_len, GList **l)
{
	int i = 0;
	for (i = 0; i < exdate_len; i++) {
		long long int exdate_lli = (long long int)g_list_nth_data(*l, i);
		if (exdate_lli == get_lli) {
			DBG("found exdate(%lld)", get_lli);
			*l = g_list_remove(*l, lli2p(get_lli));
			return true;
		}
	}
	return false;
}

static bool __check_to_stop_loop(long long int current_utime, long long int *last_utime, int loop)
{
	if ((*last_utime == current_utime) || (CAL_ENDLESS_LIMIT_FULL_DAY == loop)) {
		DBG("current utime is same as last utime(%lld), so stoppted", current_utime);
		return true;
	}
	*last_utime = current_utime;
	return false;
}

static bool __check_out_of_range(long long int current_utime, cal_event_s *event, long long int until_utime, int *count)
{
	RETV_IF(NULL == event, true);

	/* check range */
	switch (event->range_type) {
	case CALENDAR_RANGE_UNTIL:
	case CALENDAR_RANGE_NONE:
		if (until_utime < current_utime) {
			DBG("(%lld) (%lld)", current_utime, until_utime);
			return true;
		}
		break;

	case CALENDAR_RANGE_COUNT:
		(*count)++;
		if (event->count < *count) {
			DBG("(%d) (%d)", *count, event->count);
			return true;
		}

		if (CAL_ENDLESS_LIMIT_UTIME < current_utime) {
			/* event count is remained, it should not go over LIMIT */
			DBG("stopped because LIMIT UTIME(%lld) < dtstart(%lld)", (long long int)CAL_ENDLESS_LIMIT_UTIME, current_utime);
			return true;
		}
		break;
	}
	return false;
}

static bool __check_daily_bymonth_to_skip(UCalendar *ucal, int *bymonth, int bymonth_len, int *log_value)
{
	if (0 == bymonth_len) return false;

	UErrorCode ec = U_ZERO_ERROR;
	int month = ucal_get(ucal, UCAL_MONTH, &ec) + 1;
	int i;
	for (i = 0; i < bymonth_len; i++) {
		if (month == bymonth[i]) {
			return false;
		}
	}
	if (*log_value != month) {
		DBG("Get month(%d) Not in bymonth", month);
		*log_value = month;
	}
	return true;
}

static int __get_dates_in_month(UCalendar *ucal, int week_bits, int bymonth, int *dates_len)
{
	UErrorCode ec = U_ZERO_ERROR;
	int len = 0;

	RETV_IF(NULL == ucal, CALENDAR_ERROR_INVALID_PARAMETER);

	int i;
	for (i = 0; i < 7; i++) {
		if (0 == (week_bits & (0x01 << i))) continue;
		int wday_int = i + 1;

		ucal_set(ucal, UCAL_MONTH, bymonth - 1);
		ucal_set(ucal, UCAL_DAY_OF_WEEK, wday_int);
		ucal_set(ucal, UCAL_DAY_OF_WEEK_IN_MONTH, 1);
		int weekno_start = ucal_get(ucal, UCAL_WEEK_OF_YEAR, &ec);

		ucal_set(ucal, UCAL_MONTH, bymonth - 1);
		ucal_set(ucal, UCAL_DAY_OF_WEEK, wday_int);
		ucal_set(ucal, UCAL_DAY_OF_WEEK_IN_MONTH, -1);
		int weekno_end = ucal_get(ucal, UCAL_WEEK_OF_YEAR, &ec);
		if (1 == weekno_end) {
			ucal_set(ucal, UCAL_MONTH, bymonth - 1);
			ucal_set(ucal, UCAL_DAY_OF_WEEK, wday_int);
			ucal_set(ucal, UCAL_DAY_OF_WEEK_IN_MONTH, -1);
			ucal_add(ucal, UCAL_DATE, -7, &ec);
			weekno_end = ucal_get(ucal, UCAL_WEEK_OF_YEAR, &ec) + 1;
		}
		int weekno_gap = weekno_end - weekno_start + 1;

		len += weekno_gap;
	}
	if (dates_len) *dates_len = len;
	return CALENDAR_ERROR_NONE;
}

static int _cal_db_instance_publish_yearly_yday(UCalendar *ucal, cal_event_s *event, long long int duration)
{
	RETV_IF(NULL == ucal, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == event, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == event->byyearday, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF('\0' == event->byyearday[0], CALENDAR_ERROR_INVALID_PARAMETER);

	UErrorCode ec = U_ZERO_ERROR;
	calendar_time_s until = {0};
	__get_until_from_range(event, &until);
	__set_time_to_ucal(event->system_type, ucal, &until);
	/* set until before dtstart_utime */
	long long int until_utime = ms2sec(ucal_getMillis(ucal, &ec));

	int byyearday[CAL_STR_MIDDLE_LEN] = {0};
	int byyearday_len = 0;
	_cal_db_instance_parse_byint(event->byyearday, byyearday, &byyearday_len);
	DBG("byyearday len(%d)", byyearday_len);

	int bysetpos[CAL_STR_MIDDLE_LEN] = {0};
	int bysetpos_len = 0;
	_cal_db_instance_parse_byint(event->bysetpos, bysetpos, &bysetpos_len);
	DBG("bysetpos len(%d)", bysetpos_len);

	GList *l = NULL;
	int exdate_len = 0;
	__get_exdate_list(ucal, event, &l, &exdate_len);

	int loop = 0;
	int count = 0;
	bool is_exit = false;
	long long int last_utime = 0;
	long long int current_utime = 0;
	while (false == is_exit) {
		calendar_time_s *st = &event->start;
		__set_time_to_ucal(event->system_type, ucal, st);
		long long int dtstart_utime = ms2sec(ucal_getMillis(ucal, &ec));
		ucal_add(ucal, UCAL_YEAR, event->interval * loop, &ec);

		int i;
		for (i = 0; i < byyearday_len; i++) {
			ucal_set(ucal, UCAL_DAY_OF_YEAR, byyearday[i]);
			if (true == __check_bysetpos_to_skip(i, bysetpos, bysetpos_len, byyearday_len)) {
				continue;
			}
			current_utime = ms2sec(ucal_getMillis(ucal, &ec));
			if (true == __check_before_dtstart(current_utime, dtstart_utime)) {
				continue;
			}
			if (true == __check_exdate_to_skip(current_utime, exdate_len, &l)) {
				count++;
				continue;
			}
			is_exit = __check_out_of_range(current_utime, event, until_utime, &count);
			if (true == is_exit) break;
			_cal_db_instance_insert_record(ucal, duration, event);
		}
		if (true == is_exit) break;
		current_utime = ms2sec(ucal_getMillis(ucal, &ec));
		is_exit = __check_to_stop_loop(current_utime, &last_utime, loop);
		loop++;
	}
	if (l) g_list_free(l);
	return CALENDAR_ERROR_NONE;
}

/*
 * weekno is different num+day
 * weekno=10, byday=SU, is not always same as 10SU
 * in 2014, Mar 2nd != Mar 9th
 */
static int _cal_db_instance_publish_yearly_weekno(UCalendar *ucal, cal_event_s *event, long long int duration)
{
	RETV_IF(NULL == ucal, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == event, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == event->byweekno || '\0' == event->byweekno[0], CALENDAR_ERROR_INVALID_PARAMETER);

	UErrorCode ec = U_ZERO_ERROR;
	calendar_time_s until = {0};
	__get_until_from_range(event, &until);
	__set_time_to_ucal(event->system_type, ucal, &until);
	/* set until before dtstart_utime */
	long long int until_utime = ms2sec(ucal_getMillis(ucal, &ec));

	/* get byday */
	int week_bits = 0;
	int byday_len = 0;
	if (NULL == event->byday || '\0' == event->byday[0]) {
		calendar_time_s *st = &event->start;
		__set_time_to_ucal(event->system_type, ucal, st);
		int dtstart_wday = ucal_get(ucal, UCAL_DAY_OF_WEEK, &ec);
		week_bits |= (0x01 << (dtstart_wday - 1));
		byday_len = 1;

	}
	else {
		week_bits = __convert_week_to_bits(event->byday, &byday_len);
	}
	DBG("byday_len (%d) week integer(0x%x)", byday_len, week_bits);

	int byweekno[CAL_STR_MIDDLE_LEN] = {0};
	int byweekno_len = 0;
	_cal_db_instance_parse_byint(event->byweekno, byweekno, &byweekno_len);
	DBG("byweekno len(%d)", byweekno_len);

	int bysetpos[CAL_STR_MIDDLE_LEN] = {0};
	int bysetpos_len = 0;
	_cal_db_instance_parse_byint(event->bysetpos, bysetpos, &bysetpos_len);
	DBG("bysetpos len(%d)", bysetpos_len);

	GList *l = NULL;
	int exdate_len = 0;
	__get_exdate_list(ucal, event, &l, &exdate_len);

	int loop = 0;
	int count = 0;
	bool is_exit = false;
	long long int last_utime = 0;
	long long int current_utime = 0;
	while (false == is_exit) {
		calendar_time_s *st = &event->start;
		__set_time_to_ucal(event->system_type, ucal, st);
		long long int dtstart_utime = ms2sec(ucal_getMillis(ucal, &ec));
		ucal_add(ucal, UCAL_YEAR, (event->interval * loop), &ec);
		long long int start_point = sec2ms(ucal_getMillis(ucal, &ec));

		/* extra_weekno : week-number converting value from iCalendar (RFC2445) to ICU:
		 * - iCalendar W1 : first week containing at least 4 days of the year
		 *                 (W0 when contains less than 4 days of the year)
		 * - ICU W1 : week of 1,Jan
		 */
		int extra_weekno = 0;
		ucal_set(ucal, UCAL_MONTH, 0);
		ucal_set(ucal, UCAL_DATE, 1);
		int wkst = ucal_getAttribute(ucal, UCAL_FIRST_DAY_OF_WEEK);
		int first_wday = ucal_get(ucal, UCAL_DAY_OF_WEEK, &ec);
		extra_weekno = 3 < ((first_wday+7-wkst)%7) ? 1 : 0;

		ucal_setMillis(ucal, ms2sec(start_point), &ec); /* set start point */

		int i;
		for (i = 0 ; i < byweekno_len; i++) {
			int j;
			int week_count = 0;
			for (j = 0; j < 7; j++) {
				if (week_bits & (0x01 << j)) {
					week_count++;
					ucal_set(ucal, UCAL_WEEK_OF_YEAR, byweekno[i] + extra_weekno);
					ucal_set(ucal, UCAL_DAY_OF_WEEK, j + 1);
					if (true == __check_bysetpos_to_skip(i + week_count - 1, bysetpos, bysetpos_len, byweekno_len + byday_len - 1)) {
						continue;
					}
					current_utime = ms2sec(ucal_getMillis(ucal, &ec));
					if (true == __check_before_dtstart(current_utime, dtstart_utime)) {
						continue;
					}
					if (true == __check_exdate_to_skip(current_utime, exdate_len, &l)) {
						count++;
						continue;
					}
					is_exit = __check_out_of_range(current_utime, event, until_utime, &count);
					if (true == is_exit) break;
					_cal_db_instance_insert_record(ucal, duration, event);
				}
			}
			if (true == is_exit) break;
		}
		if (true == is_exit) break;
		current_utime = ms2sec(ucal_getMillis(ucal, &ec));
		is_exit = __check_to_stop_loop(current_utime, &last_utime, loop);
		loop++;
	}
	if (l) g_list_free(l);
	return CALENDAR_ERROR_NONE;
}

static int _cal_db_instance_publish_yearly_wday(UCalendar *ucal, cal_event_s *event, long long int duration)
{
	RETV_IF(NULL == ucal, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == event, CALENDAR_ERROR_INVALID_PARAMETER);

	UErrorCode ec = U_ZERO_ERROR;
	calendar_time_s until = {0};
	__get_until_from_range(event, &until);
	__set_time_to_ucal(event->system_type, ucal, &until);
	/* set until before dtstart_utime */
	long long int until_utime = ms2sec(ucal_getMillis(ucal, &ec));

	/* get bymonth into array */
	int bymonth[12] = {0};
	int bymonth_len = 0;
	bool has_bymonth = true;
	_cal_db_instance_parse_byint(event->bymonth, bymonth, &bymonth_len);
	if (0 == bymonth_len) {
		has_bymonth = false;
		calendar_time_s *st = &event->start;
		__set_time_to_ucal(event->system_type, ucal, st);
		int month = ucal_get(ucal, UCAL_MONTH, &ec) + 1;
		bymonth[0] = month;
		bymonth_len = 1;
	}

	/* get bymonthday into array */
	int bymonthday[CAL_STR_MIDDLE_LEN] = {0};
	int bymonthday_len = 0;
	_cal_db_instance_parse_byint(event->bymonthday, bymonthday, &bymonthday_len);
	DBG("bymonthday_len(%d)", bymonthday_len);

	char **t = NULL;
	int byday_len = 0;

	/* if nowday in weekly */
	if (NULL == event->byday || '\0' == event->byday[0]) {
		const char *week_text[7] = {"SU", "MO", "TU", "WE", "TH", "FR", "SA"};

		calendar_time_s *st = &event->start;
		__set_time_to_ucal(event->system_type, ucal, st);
		int nth = ucal_get(ucal, UCAL_DAY_OF_WEEK_IN_MONTH, &ec);
		int dtstart_wday = ucal_get(ucal, UCAL_DAY_OF_WEEK, &ec);
		char buf[CAL_STR_SHORT_LEN32] = {0};
		snprintf(buf, sizeof(buf), "%d%s", nth, week_text[dtstart_wday -1]);
		DBG("set byday[%s]", buf);

		t = g_strsplit_set(buf, " ,", -1);
		byday_len = 1;

	}
	else {
		t = g_strsplit_set(event->byday, " ,", -1);
		byday_len = g_strv_length(t);
	}
	DBG("[%s] byday_len(%d)", event->byday, byday_len);

	int bysetpos[CAL_STR_MIDDLE_LEN] = {0};
	int bysetpos_len = 0;
	_cal_db_instance_parse_byint(event->bysetpos, bysetpos, &bysetpos_len);
	DBG("bysetpos len(%d)", bysetpos_len);

	GList *l = NULL;
	int exdate_len = 0;
	__get_exdate_list(ucal, event, &l, &exdate_len);
	DBG("exdate_len(%d)", exdate_len);

	int loop = 0;
	int count = 0;
	bool is_exit = false;
	long long int last_utime = 0;
	long long int current_utime = 0;
	while (false == is_exit) {
		calendar_time_s *st = &event->start;
		__set_time_to_ucal(event->system_type, ucal, st);
		long long int dtstart_utime = ms2sec(ucal_getMillis(ucal, &ec));
		ucal_add(ucal, UCAL_YEAR, event->interval * loop, &ec);
		ucal_setMillis(ucal, ucal_getMillis(ucal, &ec), &ec); /* set start point */

		int i, j, k;
		for (i = 0; i < bymonth_len; i++) {

			if (2 < strlen(t[0])) { /* -3SU, +2SA */
				for (j = 0; j < byday_len; j++) {
					if (0 == strlen(t[j])) continue;
					/* get nth, wday */
					int nth = 0;
					char wday[CAL_STR_SHORT_LEN32] = {0};

					sscanf(t[j], "%d%s", &nth, wday); /* -3SU, +2SA */
					DBG("nth(%d) wday[%s]", nth, wday);
					int wday_int = __convert_wday_to_int(wday);

					/* set nth, wday */
					if (0 < nth) {
						ucal_set(ucal, UCAL_DAY_OF_WEEK, wday_int);
						ucal_set(ucal, UCAL_DAY_OF_WEEK_IN_MONTH, 1);
						ucal_set(ucal, UCAL_MONTH, false == has_bymonth ? 0 : (bymonth[i] - 1));
						ucal_add(ucal, UCAL_WEEK_OF_YEAR, nth - 1, &ec);

					}
					else {
						ucal_set(ucal, UCAL_DAY_OF_WEEK, wday_int);
						ucal_set(ucal, UCAL_MONTH, false == has_bymonth ? 0 : bymonth[i] - 1);
						ucal_set(ucal, UCAL_DAY_OF_WEEK_IN_MONTH, nth);
					}
					if (true == __check_bysetpos_to_skip(j, bysetpos, bysetpos_len, bymonth_len + byday_len - 1)) {
						continue;
					}
					current_utime = ms2sec(ucal_getMillis(ucal, &ec));
					if (true == __check_before_dtstart(current_utime, dtstart_utime)) {
						continue;
					}
					if (true == __check_exdate_to_skip(current_utime, exdate_len, &l)) {
						count++;
						continue;
					}
					is_exit = __check_out_of_range(current_utime, event, until_utime, &count);
					if (true == is_exit) break;
					_cal_db_instance_insert_record(ucal, duration, event);
				}

			}
			else {
				/* SU, SA: no week num: means all week:1TH,2TH,3TH.., so needs another byevent->system_typex */
				int week_bits = 0;
				/* if nowday in weekly */
				if (NULL == event->byday || '\0' == event->byday[0]) {
					calendar_time_s *st = &event->start;
					__set_time_to_ucal(event->system_type, ucal, st);
					int dtstart_wday = ucal_get(ucal, UCAL_DAY_OF_WEEK, &ec);
					week_bits |= (0x01 << (dtstart_wday - 1));

				}
				else {
					week_bits = __convert_week_to_bits(event->byday, NULL);
				}
				DBG("week integer(0x%x)", week_bits);

				if (bymonthday_len) { /* bymonthday */
					for (j = 0; j < bymonthday_len; j++) {
						ucal_set(ucal, UCAL_MONTH, bymonth[i] - 1);
						ucal_set(ucal, UCAL_DAY_OF_MONTH, bymonthday[j]);
						bool is_match_wday = false;
						int w = ucal_get(ucal, UCAL_DAY_OF_WEEK, &ec);
						for (k = 0; k < 7; k++) { /* check if this is one of wday */
							if (week_bits & (0x01 << k)) {
								if ((k + 1) == w) {
									is_match_wday = true;
									break;
								}
							}
						}
						if (false == is_match_wday) {
							DBG("get wday(%d) != want wday(%d)", w, k + 1);
							continue;
						}
						if (true == __check_bysetpos_to_skip(j, bysetpos, bysetpos_len, bymonthday_len)) {
							continue;
						}
						current_utime = ms2sec(ucal_getMillis(ucal, &ec));
						if (true == __check_before_dtstart(current_utime, dtstart_utime)) {
							continue;
						}
						if (true == __check_exdate_to_skip(current_utime, exdate_len, &l)) {
							count++;
							continue;
						}
						is_exit = __check_out_of_range(current_utime, event, until_utime, &count);
						if (true == is_exit) break;
						_cal_db_instance_insert_record(ucal, duration, event);
					}

				}
				else {
					int year = ucal_get(ucal, UCAL_YEAR, &ec);

					ucal_set(ucal, UCAL_MONTH, bymonth[i] - 1);
					ucal_set(ucal, UCAL_DATE, 1);
					int byday_start = ucal_get(ucal, UCAL_DAY_OF_WEEK, &ec);

					int dates_len = 0;
					__get_dates_in_month(ucal, week_bits, bymonth[i], &dates_len);
					DBG("dates_len (%d)", dates_len);

					int index = -1;
					for (j = 0; j < 6; j++) { /* check weekno in month: max 6 */
						for (k = 0; k < 7; k++) {
							if (0 == (week_bits & (0x01 << ((byday_start -1 +k)%7)))) continue;
							index++;

							int wday_int = (byday_start -1 +k) %7 + 1;
							ucal_set(ucal, UCAL_WEEK_OF_MONTH, (j + 1) + (byday_start -1 +k) / 7);
							if (year != ucal_get(ucal, UCAL_YEAR, &ec))
								ucal_set(ucal, UCAL_YEAR, year); /* year is changed from 12/30 */
							ucal_set(ucal, UCAL_MONTH, bymonth[i] - 1);
							ucal_set(ucal, UCAL_DAY_OF_WEEK, wday_int);

							int get_month = ucal_get(ucal, UCAL_MONTH, &ec) + 1;
							if (bymonth[i] != get_month) { j = 6; break; }

							if (true == __check_bysetpos_to_skip(index, bysetpos, bysetpos_len, dates_len)) {
								continue;
							}
							current_utime = ms2sec(ucal_getMillis(ucal, &ec));
							if (true == __check_before_dtstart(current_utime, dtstart_utime)) {
								continue;
							}
							if (true == __check_exdate_to_skip(current_utime, exdate_len, &l)) {
								count++;
								continue;
							}
							is_exit = __check_out_of_range(current_utime, event, until_utime, &count);
							if (true == is_exit) break;
							_cal_db_instance_insert_record(ucal, duration, event);
						}
						if (true == is_exit) break;
					}
				}
			}
			if (true == is_exit) break;
		}
		if (true == is_exit) break;
		current_utime = ms2sec(ucal_getMillis(ucal, &ec));
		is_exit = __check_to_stop_loop(current_utime, &last_utime, loop);
		loop++;
	}
	g_strfreev(t);
	if (l) g_list_free(l);
	return CALENDAR_ERROR_NONE;
}

static int _cal_db_instance_publish_yearly_mday(UCalendar *ucal, cal_event_s *event, long long int duration)
{
	RETV_IF(NULL == ucal, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == event, CALENDAR_ERROR_INVALID_PARAMETER);

	UErrorCode ec = U_ZERO_ERROR;
	calendar_time_s until = {0};
	__get_until_from_range(event, &until);
	__set_time_to_ucal(event->system_type, ucal, &until); /* set until before dtstart_utime */
	long long int until_utime = ms2sec(ucal_getMillis(ucal, &ec));

	/* get bymonth into array */
	int bymonth[12] = {0};
	int bymonth_len = 0;
	_cal_db_instance_parse_byint(event->bymonth, bymonth, &bymonth_len);
	if (0 == bymonth_len) {
		calendar_time_s *st = &event->start;
		__set_time_to_ucal(event->system_type, ucal, st);
		int month = ucal_get(ucal, UCAL_MONTH, &ec) + 1;
		bymonth[0] = month;
		bymonth_len = 1;
	}
	DBG("bymonth_len(%d)", bymonth_len);

	/* get bymonthday into array */
	int bymonthday[CAL_STR_MIDDLE_LEN] = {0};
	int bymonthday_len = 0;
	_cal_db_instance_parse_byint(event->bymonthday, bymonthday, &bymonthday_len);
	if (0 == bymonthday_len) {
		calendar_time_s *st = &event->start;
		__set_time_to_ucal(event->system_type, ucal, st);
		int dtstart_mday = ucal_get(ucal, UCAL_DATE, &ec);
		bymonthday[0] = dtstart_mday;
		bymonthday_len = 1;
	}
	DBG("bymonthday len(%d)", bymonthday_len);

	int bysetpos[CAL_STR_MIDDLE_LEN] = {0};
	int bysetpos_len = 0;
	_cal_db_instance_parse_byint(event->bysetpos, bysetpos, &bysetpos_len);
	DBG("bysetpos len(%d)", bysetpos_len);

	GList *l = NULL;
	int exdate_len = 0;
	__get_exdate_list(ucal, event, &l, &exdate_len);

	int loop = 0;
	int count = 0;
	bool is_exit = false;
	long long int last_utime = 0;
	long long int current_utime = 0;
	while (false == is_exit) {
		calendar_time_s *st = &event->start;
		__set_time_to_ucal(event->system_type, ucal, st);
		long long int dtstart_utime = ms2sec(ucal_getMillis(ucal, &ec));
		ucal_add(ucal, UCAL_YEAR, event->interval *loop, &ec);
		ucal_setMillis(ucal, ucal_getMillis(ucal, &ec), &ec); /* set start point */

		int i, j;
		for (j = 0; j < bymonth_len; j++) {
			for (i = 0 ; i < bymonthday_len; i++) {
				if (0 < bymonthday[i]) {
					ucal_set(ucal, UCAL_MONTH, bymonth[j] - 1);
					ucal_set(ucal, UCAL_DATE, bymonthday[i]);
					int get_mday = ucal_get(ucal, UCAL_DATE, &ec);
					if (get_mday != bymonthday[i]) {
						DBG("bymonthday(%d) but get (%d) from icu", bymonthday[i], get_mday);
						continue;
					}
				}
				else if (bymonthday[i] < 0) {
					ucal_set(ucal, UCAL_MONTH, bymonth[j] - 1);
					ucal_set(ucal, UCAL_DATE, 1);
					ucal_add(ucal, UCAL_MONTH, 1, &ec);
					ucal_add(ucal, UCAL_DAY_OF_YEAR, bymonthday[i], &ec);

				}
				else { /* 0 is invalid */
					DBG("Invalid bymonthday(%d)", bymonthday[i]);
					continue;
				}
				if (true == __check_bysetpos_to_skip(i, bysetpos, bysetpos_len, bymonthday_len)) {
					DBG("no bysetpos");
					continue;
				}
				current_utime = ms2sec(ucal_getMillis(ucal, &ec));
				if (true == __check_before_dtstart(current_utime, dtstart_utime)) {
					continue;
				}
				if (true == __check_exdate_to_skip(current_utime, exdate_len, &l)) {
					count++;
					continue;
				}
				is_exit = __check_out_of_range(current_utime, event, until_utime, &count);
				if (true == is_exit) break;
				_cal_db_instance_insert_record(ucal, duration, event);
			}
		}
		if (true == is_exit) break;
		current_utime = ms2sec(ucal_getMillis(ucal, &ec));
		is_exit = __check_to_stop_loop(current_utime, &last_utime, loop);
		loop++;
	}
	if (l) g_list_free(l);
	return CALENDAR_ERROR_NONE;
}

static int _cal_db_instance_publish_record_yearly(UCalendar *ucal, cal_event_s *event, long long int duration)
{
	RETV_IF(NULL == ucal, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == event, CALENDAR_ERROR_INVALID_PARAMETER);

	if (event->byyearday && 0 < strlen(event->byyearday)) {
		_cal_db_instance_publish_yearly_yday(ucal, event, duration);
	}
	else if (event->byweekno && 0 < strlen(event->byweekno)) {
		_cal_db_instance_publish_yearly_weekno(ucal, event, duration);
	}
	else {
		if (event->byday && *event->byday) {
			_cal_db_instance_publish_yearly_wday(ucal, event, duration);
		}
		else {
			_cal_db_instance_publish_yearly_mday(ucal, event, duration);
		}
	}
	return CALENDAR_ERROR_NONE;
}

static int _cal_db_instance_publish_monthly_wday(UCalendar *ucal, cal_event_s *event, long long int duration)
{
	RETV_IF(NULL == ucal, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == event, CALENDAR_ERROR_INVALID_PARAMETER);

	UErrorCode ec = U_ZERO_ERROR;
	calendar_time_s until = {0};
	__get_until_from_range(event, &until);
	__set_time_to_ucal(event->system_type, ucal, &until); /* set until before dtstart_utime */
	long long int until_utime = ms2sec(ucal_getMillis(ucal, &ec));

	/* get bymonthday into array */
	int bymonthday[CAL_STR_MIDDLE_LEN] = {0};
	int bymonthday_len = 0;
	_cal_db_instance_parse_byint(event->bymonthday, bymonthday, &bymonthday_len);

	char **t = NULL;
	int byday_len = 0;

	/* if nowday in weekly */
	if (NULL == event->byday || '\0' == event->byday[0]) {
		const char *week_text[7] = {"SU", "MO", "TU", "WE", "TH", "FR", "SA"};

		calendar_time_s *st = &event->start;
		__set_time_to_ucal(event->system_type, ucal, st);
		int week_nth = ucal_get(ucal, UCAL_DAY_OF_WEEK_IN_MONTH, &ec);
		int dtstart_wday = ucal_get(ucal, UCAL_DAY_OF_WEEK, &ec);
		char buf[CAL_STR_SHORT_LEN32] = {0};
		snprintf(buf, sizeof(buf), "%d%s", week_nth, week_text[dtstart_wday -1]);
		DBG("set byday[%s]", buf);

		t = g_strsplit_set(buf, " ,", -1);
		byday_len = 1;

	}
	else {
		t = g_strsplit_set(event->byday, " ,", -1);
		byday_len = g_strv_length(t);
	}
	DBG("[%s] byday_len(%d)", event->byday, byday_len);

	int bysetpos[CAL_STR_MIDDLE_LEN] = {0};
	int bysetpos_len = 0;
	_cal_db_instance_parse_byint(event->bysetpos, bysetpos, &bysetpos_len);
	DBG("bysetpos len(%d)", bysetpos_len);

	GList *l = NULL;
	int exdate_len = 0;
	__get_exdate_list(ucal, event, &l, &exdate_len);
	DBG("exdate_len(%d)", exdate_len);

	int loop = 0;
	int count = 0;
	bool is_exit = false;
	long long int last_utime = 0;
	long long int current_utime = 0;
	while (false == is_exit) {
		calendar_time_s *st = &event->start;
		__set_time_to_ucal(event->system_type, ucal, st);
		long long int dtstart_utime = ms2sec(ucal_getMillis(ucal, &ec));
		ucal_add(ucal, UCAL_MONTH, event->interval * loop, &ec);
		ucal_setMillis(ucal, ucal_getMillis(ucal, &ec), &ec); /* set start point */

		int i, j, k;
		if (2 < strlen(t[0])) { /* -3SU, +2SA */
			for (i = 0; i < byday_len; i++) {
				if (0 == strlen(t[i]))
					continue;
				/* get nth, wday */
				int nth = 0;
				char wday[CAL_STR_SHORT_LEN32] = {0};

				sscanf(t[i], "%d%s", &nth, wday); /* -3SU, +2SA */
				DBG("nth(%d) wday[%s]", nth, wday);
				int wday_int = __convert_wday_to_int(wday);

				/* set nth, wday */
				if (0 < nth) {
					if (4 < nth) {
						ucal_set(ucal, UCAL_DAY_OF_WEEK, wday_int);
						ucal_set(ucal, UCAL_DAY_OF_WEEK_IN_MONTH, -1);

					}
					else {
						ucal_set(ucal, UCAL_DAY_OF_WEEK, wday_int);
						ucal_set(ucal, UCAL_DAY_OF_WEEK_IN_MONTH, 1);
						ucal_add(ucal, UCAL_WEEK_OF_YEAR, nth - 1, &ec);
					}
				}
				else {
					ucal_set(ucal, UCAL_DAY_OF_WEEK, wday_int);
					ucal_set(ucal, UCAL_DAY_OF_WEEK_IN_MONTH, nth);
				}
				if (true == __check_bysetpos_to_skip(i, bysetpos, bysetpos_len, byday_len)) {
					continue;
				}
				current_utime = ms2sec(ucal_getMillis(ucal, &ec));
				if (true == __check_before_dtstart(current_utime, dtstart_utime)) {
					continue;
				}
				if (true == __check_exdate_to_skip(current_utime, exdate_len, &l)) {
					count++;
					continue;
				}
				is_exit = __check_out_of_range(current_utime, event, until_utime, &count);
				if (true == is_exit) break;
				_cal_db_instance_insert_record(ucal, duration, event);
			}

		}
		else { /* SU, SA: no week num: means all week:1TH,2TH,3TH.., so needs another byevent->system_type */
			int week_bits = 0;
			/* if nowday in weekly */
			if (NULL == event->byday || '\0' == event->byday[0]) {
				calendar_time_s *st = &event->start;
				__set_time_to_ucal(event->system_type, ucal, st);
				int dtstart_wday = ucal_get(ucal, UCAL_DAY_OF_WEEK, &ec);
				week_bits |= (0x01 << (dtstart_wday - 1));

			}
			else {
				week_bits = __convert_week_to_bits(event->byday, NULL);
			}
			DBG("week integer(0x%x)", week_bits);

			if (bymonthday_len) { /* bymonthday */
				for (j = 0; j < bymonthday_len; j++) {
					ucal_set(ucal, UCAL_DATE, bymonthday[j]);
					bool is_match_wday = false;
					int w = ucal_get(ucal, UCAL_DAY_OF_WEEK, &ec);
					for (k = 0; k < 7; k++) { /* check if this is one of wday */
						if (week_bits & (0x01 << k)) {
							if ((k + 1) == w) {
								is_match_wday = true;
								break;
							}
						}
					}
					if (false == is_match_wday) {
						DBG("get wday(%d) != want wday(%d)", w, k + 1);
						continue;
					}
					if (true == __check_bysetpos_to_skip(j, bysetpos, bysetpos_len, bymonthday_len)) {
						continue;
					}
					current_utime = ms2sec(ucal_getMillis(ucal, &ec));
					if (true == __check_before_dtstart(current_utime, dtstart_utime)) {
						continue;
					}
					if (true == __check_exdate_to_skip(current_utime, exdate_len, &l)) {
						count++;
						continue;
					}
					is_exit = __check_out_of_range(current_utime, event, until_utime, &count);
					if (true == is_exit) break;
					_cal_db_instance_insert_record(ucal, duration, event);
				}

			}
			else {
				int year = ucal_get(ucal, UCAL_YEAR, &ec);
				int month = ucal_get(ucal, UCAL_MONTH, &ec) + 1;

				ucal_set(ucal, UCAL_MONTH, month - 1);
				ucal_set(ucal, UCAL_DATE, 1);
				int byday_start = ucal_get(ucal, UCAL_DAY_OF_WEEK, &ec);

				int dates_len = 0;
				__get_dates_in_month(ucal, week_bits, month, &dates_len);
				DBG("month(%d) dates_len (%d)", month, dates_len);

				int index = -1;
				for (j = 0; j < 6; j++) { /* check weekno in month: max 6 */
					for (k = 0; k < 7; k++) {
						if (0 == (week_bits & (0x01 << ((byday_start -1 +k)%7)))) continue;
						index++;

						int wday_int = (byday_start -1 +k) %7 + 1;
						ucal_set(ucal, UCAL_WEEK_OF_MONTH, (j + 1) + (byday_start -1 +k) / 7);
						if (year != ucal_get(ucal, UCAL_YEAR, &ec)) ucal_set(ucal, UCAL_YEAR, year);
						ucal_set(ucal, UCAL_MONTH, month - 1);
						ucal_set(ucal, UCAL_DAY_OF_WEEK, wday_int);

						int get_month = ucal_get(ucal, UCAL_MONTH, &ec) + 1;
						if (month != get_month) { j = 6; break; }

						if (true == __check_bysetpos_to_skip(index, bysetpos, bysetpos_len, dates_len)) {
							continue;
						}
						current_utime = ms2sec(ucal_getMillis(ucal, &ec));
						if (true == __check_before_dtstart(current_utime, dtstart_utime)) {
							continue;
						}
						if (true == __check_exdate_to_skip(current_utime, exdate_len, &l)) {
							count++;
							continue;
						}
						is_exit = __check_out_of_range(current_utime, event, until_utime, &count);
						if (true == is_exit) break;
						_cal_db_instance_insert_record(ucal, duration, event);
					}
					if (true == is_exit) break;
				}
			}
		}
		if (true == is_exit) break;
		current_utime = ms2sec(ucal_getMillis(ucal, &ec));
		is_exit = __check_to_stop_loop(current_utime, &last_utime, loop);
		loop++;
	}
	g_strfreev(t);
	if (l) g_list_free(l);
	return CALENDAR_ERROR_NONE;
}

static int _cal_db_instance_publish_monthly_mday(UCalendar *ucal, cal_event_s *event, long long int duration)
{
	RETV_IF(NULL == ucal, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == event, CALENDAR_ERROR_INVALID_PARAMETER);

	UErrorCode ec = U_ZERO_ERROR;
	calendar_time_s until = {0};
	__get_until_from_range(event, &until);
	__set_time_to_ucal(event->system_type, ucal, &until); /* set until before dtstart_utime */
	long long int until_utime = ms2sec(ucal_getMillis(ucal, &ec));

	/* get bymonthday into array */
	int bymonthday[CAL_STR_MIDDLE_LEN] = {0};
	int bymonthday_len = 0;
	_cal_db_instance_parse_byint(event->bymonthday, bymonthday, &bymonthday_len);
	if (0 == bymonthday_len) {
		calendar_time_s *st = &event->start;
		__set_time_to_ucal(event->system_type, ucal, st);
		int dtstart_mday = ucal_get(ucal, UCAL_DATE, &ec);
		bymonthday[0] = dtstart_mday;
		bymonthday_len = 1;
	}
	DBG("bymonthday_len(%d) [%s]", bymonthday_len, event->bymonthday);

	int bysetpos[CAL_STR_MIDDLE_LEN] = {0};
	int bysetpos_len = 0;
	_cal_db_instance_parse_byint(event->bysetpos, bysetpos, &bysetpos_len);
	DBG("bysetpos_len(%d)", bysetpos_len);

	GList *l = NULL;
	int exdate_len = 0;
	__get_exdate_list(ucal, event, &l, &exdate_len);
	DBG("exdate_len(%d)", bysetpos_len);

	int loop = 0;
	int count = 0;
	bool is_exit = false;
	long long int last_utime = 0;
	long long int current_utime = 0;
	while (false == is_exit) {
		calendar_time_s *st = &event->start;
		__set_time_to_ucal(event->system_type, ucal, st);
		long long int dtstart_utime = ms2sec(ucal_getMillis(ucal, &ec));
		ucal_add(ucal, UCAL_MONTH, event->interval * loop, &ec);
		ucal_setMillis(ucal, ucal_getMillis(ucal, &ec), &ec); /* set start point */

		int i;
		for (i = 0 ; i < bymonthday_len; i++) {
			if (0 < bymonthday[i]) {
				ucal_set(ucal, UCAL_MONTH, ucal_get(ucal, UCAL_MONTH, &ec));
				ucal_set(ucal, UCAL_DATE, bymonthday[i]);
				int get_mday = ucal_get(ucal, UCAL_DATE, &ec);
				if (get_mday != bymonthday[i]) {
					DBG("bymonthday(%d) but get (%d) from icu", bymonthday[i], get_mday);
					continue;
				}
			}
			else if (bymonthday[i] < 0) {
				ucal_set(ucal, UCAL_DATE, 1);
				ucal_add(ucal, UCAL_MONTH, 1, &ec);
				ucal_add(ucal, UCAL_DAY_OF_YEAR, bymonthday[i], &ec);

			}
			else { /* 0 is invalid */
				DBG("Invalid bymonthday(%d)", bymonthday[i]);
				continue;
			}
			if (true == __check_bysetpos_to_skip(i, bysetpos, bysetpos_len, bymonthday_len)) {
				continue;
			}
			current_utime = ms2sec(ucal_getMillis(ucal, &ec));
			if (true == __check_before_dtstart(current_utime, dtstart_utime)) {
				continue;
			}
			if (true == __check_exdate_to_skip(current_utime, exdate_len, &l)) {
				count++;
				continue;
			}
			is_exit = __check_out_of_range(current_utime, event, until_utime, &count);
			if (true == is_exit) break;
			_cal_db_instance_insert_record(ucal, duration, event);
		}
		if (true == is_exit) break;
		current_utime = ms2sec(ucal_getMillis(ucal, &ec));
		is_exit = __check_to_stop_loop(current_utime, &last_utime, loop);
		loop++;
	}
	if (l) g_list_free(l);
	return CALENDAR_ERROR_NONE;
}

static int _cal_db_instance_publish_record_monthly(UCalendar *ucal, cal_event_s *event, long long int duration)
{
	RETV_IF(NULL == ucal, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == event, CALENDAR_ERROR_INVALID_PARAMETER);

	if (event->byday && 0 < strlen(event->byday))
		_cal_db_instance_publish_monthly_wday(ucal, event, duration);
	else
		_cal_db_instance_publish_monthly_mday(ucal, event, duration);

	return CALENDAR_ERROR_NONE;
}

static int _cal_db_instance_publish_weekly_wday(UCalendar *ucal, cal_event_s *event, long long int duration)
{
	RETV_IF(NULL == ucal, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == event, CALENDAR_ERROR_INVALID_PARAMETER);

	UErrorCode ec = U_ZERO_ERROR;
	calendar_time_s until = {0};
	__get_until_from_range(event, &until);
	__set_time_to_ucal(event->system_type, ucal, &until); /* set until before dtstart_utime */
	long long int until_utime = ms2sec(ucal_getMillis(ucal, &ec));

	int week_bits = 0;
	int byday_len = 0;
	/* if nowday in weekly */
	if (NULL == event->byday || '\0' == event->byday[0]) {
		calendar_time_s *st = &event->start;
		__set_time_to_ucal(event->system_type, ucal, st);
		int dtstart_wday = ucal_get(ucal, UCAL_DAY_OF_WEEK, &ec);
		week_bits |= (0x01 << (dtstart_wday - 1));

	}
	else {
		week_bits = __convert_week_to_bits(event->byday, &byday_len);
	}
	DBG("week integer(0x%x)", week_bits);

	int bysetpos[CAL_STR_MIDDLE_LEN] = {0};
	int bysetpos_len = 0;
	_cal_db_instance_parse_byint(event->bysetpos, bysetpos, &bysetpos_len);
	DBG("bysetpos len(%d)", bysetpos_len);

	GList *l = NULL;
	int exdate_len = 0;
	__get_exdate_list(ucal, event, &l, &exdate_len);

	int byday_start = ucal_getAttribute(ucal, UCAL_FIRST_DAY_OF_WEEK);
	DBG("get first day of week(%d)", byday_start);

	int loop = 0;
	int count = 0;
	bool is_exit = false;
	long long int last_utime = 0;
	long long int current_utime = 0;
	while (false == is_exit) {
		calendar_time_s *st = &event->start;
		__set_time_to_ucal(event->system_type, ucal, st);
		long long int dtstart_utime = ms2sec(ucal_getMillis(ucal, &ec));
		ucal_add(ucal, UCAL_WEEK_OF_YEAR, event->interval *loop, &ec);
		ucal_setMillis(ucal, ucal_getMillis(ucal, &ec), &ec); /* set start point */

		int i;
		int index = -1;
		for (i = 0; i < 7; i++) {
			if (0 == (week_bits & (0x01 << ((byday_start -1 +i)%7)))) continue;
			index++;

			int wday_int = (byday_start -1 +i) %7 + 1;
			ucal_set(ucal, UCAL_DAY_OF_WEEK, wday_int);

			if (true == __check_bysetpos_to_skip(index, bysetpos, bysetpos_len, byday_len)) {
				continue;
			}
			current_utime = ms2sec(ucal_getMillis(ucal, &ec));
			if (true == __check_before_dtstart(current_utime, dtstart_utime)) {
				continue;
			}
			if (true == __check_exdate_to_skip(current_utime, exdate_len, &l)) {
				count++;
				continue;
			}
			is_exit = __check_out_of_range(current_utime, event, until_utime, &count);
			if (true == is_exit) break;
			_cal_db_instance_insert_record(ucal, duration, event);
		}
		if (true == is_exit) break;
		current_utime = ms2sec(ucal_getMillis(ucal, &ec));
		is_exit = __check_to_stop_loop(current_utime, &last_utime, loop);
		loop++;
	}
	if (l) g_list_free(l);
	return CALENDAR_ERROR_NONE;
}

static int _cal_db_instance_publish_record_weekly(UCalendar *ucal, cal_event_s *event, long long int duration)
{
	RETV_IF(NULL == ucal, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == event, CALENDAR_ERROR_INVALID_PARAMETER);

	_cal_db_instance_publish_weekly_wday(ucal, event, duration);
	return CALENDAR_ERROR_NONE;
}

static int _cal_db_instance_publish_daily_mday(UCalendar *ucal, cal_event_s *event, long long int duration)
{
	RETV_IF(NULL == ucal, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == event, CALENDAR_ERROR_INVALID_PARAMETER);

	UErrorCode ec = U_ZERO_ERROR;
	calendar_time_s until = {0};
	__get_until_from_range(event, &until);
	__set_time_to_ucal(event->system_type, ucal, &until); /* set until before dtstart_utime */
	long long int until_utime = ms2sec(ucal_getMillis(ucal, &ec));

	/* get bymonth into array */
	int bymonth[12] = {0};
	int bymonth_len = 0;
	_cal_db_instance_parse_byint(event->bymonth, bymonth, &bymonth_len);

	GList *l = NULL;
	int exdate_len = 0;
	__get_exdate_list(ucal, event, &l, &exdate_len);

	calendar_time_s *st = &event->start;
	__set_time_to_ucal(event->system_type, ucal, st);
	long long int dtstart_utime = ms2sec(ucal_getMillis(ucal, &ec));
	DBG("(%lld)", ms2sec(ucal_getMillis(ucal, &ec)));

	int loop = 0;
	int count = 0;
	bool is_exit = false;
	long long int last_utime = 0;
	long long int current_utime = 0;
	int log_value = 0;
	while (false == is_exit) {
		if (loop) ucal_add(ucal, UCAL_DAY_OF_YEAR, event->interval, &ec);

		if (true == __check_daily_bymonth_to_skip(ucal, bymonth, bymonth_len, &log_value)) {
			if (0 == loop) loop = 1;
			continue;
		}
		current_utime = ms2sec(ucal_getMillis(ucal, &ec));
		if (true == __check_before_dtstart(current_utime, dtstart_utime)) {
			if (0 == loop) loop = 1;
			continue;
		}
		if (true == __check_exdate_to_skip(current_utime, exdate_len, &l)) {
			if (0 == loop) loop = 1;
			count++;
			continue;
		}
		is_exit = __check_out_of_range(current_utime, event, until_utime, &count);
		if (true == is_exit) break;
		_cal_db_instance_insert_record(ucal, duration, event);

		current_utime = ms2sec(ucal_getMillis(ucal, &ec));
		is_exit = __check_to_stop_loop(current_utime, &last_utime, loop);
		loop++;
	}
	if (l) g_list_free(l);
	return CALENDAR_ERROR_NONE;
}

static int _cal_db_instance_publish_record_daily(UCalendar *ucal, cal_event_s *event, long long int duration)
{
	RETV_IF(NULL == ucal, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == event, CALENDAR_ERROR_INVALID_PARAMETER);

	_cal_db_instance_publish_daily_mday(ucal, event, duration);
	return CALENDAR_ERROR_NONE;
}

static int _cal_db_instance_publish_record_once(UCalendar *ucal, cal_event_s *event, long long int duration)
{
	RETV_IF(NULL == ucal, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == event, CALENDAR_ERROR_INVALID_PARAMETER);

	calendar_time_s *st = &event->start;
	__set_time_to_ucal(event->system_type, ucal, st);
	_cal_db_instance_insert_record(ucal, duration, event);
	return CALENDAR_ERROR_NONE;
}

static int _cal_db_instance_publish_record_details(UCalendar *ucal, cal_event_s *event)
{
	RETV_IF(NULL == ucal, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == event, CALENDAR_ERROR_INVALID_PARAMETER);

	long long int duration = -1;
	int exception_freq = 0; /* for exception */

	_cal_db_instance_get_duration(ucal, &event->start, &event->end, &duration);
	WARN_IF(duration < 0, "Invalid duration (%lld)", duration);

	if (0 < event->original_event_id) {
		DBG("this is exception event so publish only one instance");
		exception_freq = event->freq;
		event->freq = CALENDAR_RECURRENCE_NONE;
	}

	DBG("event interval(%d)", event->interval);
	if (event->interval < 1) {
		DBG("Invalid interval, so set 1");
		event->interval = 1;
	}

	switch (event->freq) {
	case CALENDAR_RECURRENCE_YEARLY:
		_cal_db_instance_publish_record_yearly(ucal, event, duration);
		break;

	case CALENDAR_RECURRENCE_MONTHLY:
		_cal_db_instance_publish_record_monthly(ucal, event, duration);
		break;

	case CALENDAR_RECURRENCE_WEEKLY:
		_cal_db_instance_publish_record_weekly(ucal, event, duration);
		break;

	case CALENDAR_RECURRENCE_DAILY:
		_cal_db_instance_publish_record_daily(ucal, event, duration);
		break;

	case CALENDAR_RECURRENCE_NONE:
	default:
		_cal_db_instance_publish_record_once(ucal, event, duration);
		break;
	}

	if (0 < event->original_event_id) {
		DBG("return freq for exception event");
		event->freq = exception_freq;
	}

	return CALENDAR_ERROR_NONE;
}

int cal_db_instance_update_exdate_del(int id, char *exdate)
{
	int ret = 0;
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	char **t = NULL;
	char *p = NULL;

	if (NULL == exdate || '\0' == *exdate) {
		DBG("Nothing to update exdate del");
		return CALENDAR_ERROR_NONE;
	}

	DBG("exdate[%s]", exdate);
	t = g_strsplit_set(exdate, " ,", -1);
	if (NULL == t) {
		ERR("g_strsplit_set() Fail");
		return CALENDAR_ERROR_OUT_OF_MEMORY;
	}

	int i;
	for (i = 0; t[i]; i++) {
		if (NULL == t[i] || '\0' == *t[i]) continue;

		int y = 0, m = 0, d = 0;
		int h = 0, n = 0, s = 0;

		p = t[i];
		DBG("exdate[%s]", p);
		int len = strlen(p);
		switch (len) {
		case 8: /* 20141212 */
			DBG("ALLDAY instance");
			sscanf(p, CAL_DATETIME_FORMAT_YYYYMMDD, &y, &m, &d);
			snprintf(query, sizeof(query), "DELETE FROM %s "
					"WHERE event_id = %d AND dtstart_datetime = '%04d-%02d-%02dT%02d:%02d:%02d' ",
					CAL_TABLE_ALLDAY_INSTANCE, id, y, m, d, h, n, s);
			break;

		case 15: /* 20141212T000000 */
			sscanf(p, CAL_DATETIME_FORMAT_YYYYMMDDTHHMMSS, &y, &m, &d, &h, &n, &s);
			snprintf(query, sizeof(query), "DELETE FROM %s "
					"WHERE event_id = %d AND dtstart_datetime = '%04d-%02d-%02dT%02d:%02d:%02d' ",
					CAL_TABLE_ALLDAY_INSTANCE, id, y, m, d, h, n, s);
			DBG("localtime instance");
			break;

		case 16: /* 20141212T000000Z */
			sscanf(p, CAL_DATETIME_FORMAT_YYYYMMDDTHHMMSSZ, &y, &m, &d, &h, &n, &s);
			snprintf(query, sizeof(query), "DELETE FROM %s "
					"WHERE event_id = %d AND dtstart_utime = %lld ",
					CAL_TABLE_NORMAL_INSTANCE, id, cal_time_convert_itol(NULL, y, m, d, h, n, s));
			DBG("normal instance (%lld)", cal_time_convert_itol(NULL, y, m, d, h, n, s));
			break;
		}

		ret = cal_db_util_query_exec(query);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_db_util_query_exec() Fail(%d)", ret);
			SECURE("[%s]", query);
			g_strfreev(t);
			return ret;
		}
	}
	g_strfreev(t);
	return CALENDAR_ERROR_NONE;
}

int cal_db_instance_publish_record(calendar_record_h record)
{
	RETV_IF(NULL == record, CALENDAR_ERROR_INVALID_PARAMETER);

	cal_event_s *event = NULL;
	event = (cal_event_s *)(record);

	char *tzid = NULL;
	int offset = 0;
	int sign = 0;
	char buf[CAL_STR_SHORT_LEN32] = {0};
	switch (event->start.type) {
	case CALENDAR_TIME_UTIME:
		if (NULL == event->start_tzid) {
			tzid = NULL;
			break;
		}
		if (true == cal_time_is_available_tzid(event->start_tzid)) {
			tzid = event->start_tzid;
			break;
		}
		cal_db_timezone_get_offset(event->calendar_id, event->start_tzid, &offset);
		if (0 == offset) {
			tzid = NULL;
			break;
		}
		DBG("offset(%d)", offset);
		sign = offset < 0 ? -1 : 1;
		offset /= 60;
		offset *= sign;
		snprintf(buf, sizeof(buf), "Etc/GMT%c%d", sign < 0 ? '-' : '+', offset);
		tzid = buf;
		DBG("set tzid[%s]", buf);
		break;

	case CALENDAR_TIME_LOCALTIME:
		tzid = NULL;
		break;
	}

	UCalendar *ucal = cal_time_open_ucal(event->system_type, tzid, event->wkst);

	_cal_db_instance_publish_record_details(ucal, event);
	_cal_db_instance_del_inundant(event->index, &event->start, event);
	_cal_db_instance_update_exdate_mod(event->original_event_id, event->recurrence_id);

	ucal_close(ucal);

	return CALENDAR_ERROR_NONE;
}

int cal_db_instance_get_now(long long int *current)
{
	*current = ms2sec(ucal_getNow());
	return CALENDAR_ERROR_NONE;
}

int cal_db_instance_discard_record(int index)
{
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	int ret = 0;

	DBG("delete normal");
	snprintf(query, sizeof(query), "DELETE FROM %s WHERE event_id = %d ",
			CAL_TABLE_NORMAL_INSTANCE, index);

	ret = cal_db_util_query_exec(query);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_query_exec() Fail(%d)", ret);
		SECURE("[%s]", query);
		return ret;
	}

	DBG("delete allday");
	snprintf(query, sizeof(query), "DELETE FROM %s WHERE event_id = %d ",
			CAL_TABLE_ALLDAY_INSTANCE, index);

	ret = cal_db_util_query_exec(query);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_query_exec() Fail(%d)", ret);
		SECURE("[%s]", query);
		return ret;
	}
	return CALENDAR_ERROR_NONE;
}

