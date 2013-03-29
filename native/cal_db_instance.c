/*
 * Calendar Service
 *
 * Copyright (c) 2012 - 2013 Samsung Electronics Co., Ltd. All rights reserved.
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
#include "calendar_types2.h"

#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_view.h"
#include "cal_time.h"
#include "cal_record.h"

#include "cal_db_util.h"
#include "cal_db.h"

#define ms2sec(ms) (long long int)(ms / 1000.0)
#define sec2ms(s) (s * 1000.0)

/* input order
   UCAL_MONTH + UCAL_DAY_OF_MONTH
   UCAL_MONTH + UCAL_WEEK_OF_MONTH + UCAL_DAY_OF_WEEK
   UCAL_MONTH + UCAL_DAY_OF_WEEK_IN_MONTH + UCAL_DAY_OF_WEEK
   UCAL_DAY_OF_YEAR
   UCAL_DAY_OF_WEEK + UCAL_WEEK_OF_YEAR
*/

struct day {
	int uday;
	const char *str;
};

#define CAL_ENDLESS_LIMIT_YEAR 2038
#define CAL_ENDLESS_LIMIT_MONTH 12
#define CAL_ENDLESS_LIMIT_MDAY 31

static struct day wdays[] = {
	[CALENDAR_SUNDAY] = {UCAL_SUNDAY, "SU"},
	[CALENDAR_MONDAY] = {UCAL_MONDAY, "MO"},
	[CALENDAR_TUESDAY] = {UCAL_TUESDAY, "TU"},
	[CALENDAR_WEDNESDAY] = {UCAL_WEDNESDAY, "WE"},
	[CALENDAR_THURSDAY] = {UCAL_THURSDAY, "TH"},
	[CALENDAR_FRIDAY] = {UCAL_FRIDAY, "FR"},
	[CALENDAR_SATURDAY] = {UCAL_SATURDAY, "SA"},
};

static void __cal_db_instance_print_ucal(UCalendar *ucal)
{
	long long int lli;
	UErrorCode ec = U_ZERO_ERROR;

	if (ucal)
	{
		lli = ms2sec(ucal_getMillis(ucal, &ec));
		DBG("(%d)", lli);
	}
}

static int __cal_db_instance_work_normal_setpos(int event_id, long long int lli_s, long long int lli_e, char **t)
{
	int i, j;
	int ret;
	int count = 0;
	int ids[366] = {0};
	char str_option[CAL_DB_SQL_MAX_LEN] = {0};
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	char str_fraction[64] = {0};
    cal_db_util_error_e dbret = CAL_DB_OK;

	j = -1;
	for (i = 0; t[i];i++)
	{
		snprintf(query, sizeof(query), "SELECT rowid FROM %s "
				"WHERE event_id = %d "
				"AND dtstart_utime >= %lld AND dtend_utime < %lld "
				"ORDER BY dtstart_utime "
				"LIMIT %d, 1 ",
				CAL_TABLE_NORMAL_INSTANCE,
				event_id,
				lli_s, lli_e,
				atoi(t[i]) - 1);
		ret = _cal_db_util_query_get_first_int_result(query, NULL, &count);
		if (CALENDAR_ERROR_NONE != ret)
		{
			ERR("_cal_db_util_query_get_first_int_result() failed");
			return ret;
		}
		if (count <= 0)
		{
			DBG("%s", query);
			DBG("no result");
			continue;
		}
		else
		{
			j++;
			ids[j] = count;
			DBG("append result");
		}
		DBG("[%lld] ~ [%lld] setpos id(%d)", lli_s, lli_e, ids[j]);
	}

	while (j >= 0)
	{
		snprintf(str_fraction, sizeof(str_fraction), "AND rowid != %d ", ids[j]);
		DBG("fra:%s", str_fraction);
		strcat(str_option, str_fraction);
		DBG("opt:%s", str_option);
		j--;
	}
	DBG("str_option[%s]", str_option);
	snprintf(query, sizeof(query), "DELETE FROM %s "
			"WHERE event_id = %d "
			"AND dtstart_utime>= %lld AND dtend_utime < %lld "
			"%s",
			CAL_TABLE_NORMAL_INSTANCE,
			event_id,
			lli_s, lli_e,
			str_option);
	dbret = _cal_db_util_query_exec(query);
	if (CAL_DB_OK != dbret)
	{
		DBG("query[%s]", query);
		ERR("_cal_db_util_query_exec() Failed(%d)", dbret);
		switch (dbret)
		{
		case CAL_DB_ERROR_NO_SPACE:
			return CALENDAR_ERROR_FILE_NO_SPACE;
		default:
			return CALENDAR_ERROR_DB_FAILED;
		}
	}

	return CALENDAR_ERROR_NONE;
}

static int __cal_db_instance_work_allday_setpos(int event_id, char *str_s, char *str_e, char **t)
{
	int i, j;
	int ret;
    cal_db_util_error_e dbret = CAL_DB_OK;
	int count = 0;
	int ids[366] = {0};
	char str_option[CAL_DB_SQL_MAX_LEN] = {0};
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	char str_fraction[64] = {0};

	j = -1;
	for (i = 0; t[i];i++)
	{
		snprintf(query, sizeof(query), "SELECT rowid FROM %s "
				"WHERE event_id = %d "
				"AND dtstart_datetime >= %s AND dtend_datetime < %s "
				"ORDER BY dtstart_datetime "
				"LIMIT %d, 1 ",
				CAL_TABLE_ALLDAY_INSTANCE,
				event_id,
				str_s, str_e,
				atoi(t[i]) - 1);
		ret = _cal_db_util_query_get_first_int_result(query, NULL, &count);
		if (CALENDAR_ERROR_NONE != ret)
		{
			ERR("_cal_db_util_query_get_first_int_result() failed");
			return ret;
		}
		if (count <= 0)
		{
			DBG("%s", query);
			DBG("no result");
			continue;
		}
		else
		{
			j++;
			ids[j] = count;
			DBG("append result");
		}
		DBG("[%s] ~ [%s] setpos id(%d)", str_s, str_e, ids[j]);
	}

	while (j >= 0)
	{
		snprintf(str_fraction, sizeof(str_fraction), "AND rowid != %d ", ids[j]);
		DBG("fra:%s", str_fraction);
		strcat(str_option, str_fraction);
		DBG("opt:%s", str_option);
		j--;
	}
	DBG("str_option[%s]", str_option);
	snprintf(query, sizeof(query), "DELETE FROM %s "
			"WHERE event_id = %d "
			"AND dtstart_datetime >= %s AND dtend_datetime < %s "
			"%s",
			CAL_TABLE_ALLDAY_INSTANCE,
			event_id,
			str_s, str_e,
			str_option);
	dbret = _cal_db_util_query_exec(query);
	if (CAL_DB_OK != dbret)
	{
		DBG("query[%s]", query);
		ERR("_cal_db_util_query_exec() Failed(%d)", dbret);
		switch (dbret)
		{
		case CAL_DB_ERROR_NO_SPACE:
			return CALENDAR_ERROR_FILE_NO_SPACE;
		default:
			return CALENDAR_ERROR_DB_FAILED;
		}
	}

	return CALENDAR_ERROR_NONE;
}

static int __cal_db_instance_apply_setpos(int event_id, calendar_time_s *st, cal_event_s *event, int freq)
{
	int ret;
	int count;
	int index;
	int y, m, d;
	int y1, m1, d1;
	long long int lli_s = 0, lli_e = 0;
	char **t;
	const char *dl = ",";
	char str_t[32] = {0};
	char str_s[32] = {0};
	char str_e[32] = {0};
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	UCalendar *ucal;
	UErrorCode ec = U_ZERO_ERROR;

	t = g_strsplit(event->bysetpos, dl, -1);
	if (!t)
	{
		ERR("g_strsplit failed");
		return CALENDAR_ERROR_OUT_OF_MEMORY;
	}

	switch (st->type)
	{
	case CALENDAR_TIME_UTIME:
		DBG("(%lld)", st->time.utime);
		snprintf(query, sizeof(query), "SELECT count(*) FROM %s "
				"WHERE event_id = %d "
				"AND dtstart_utime >= %lld",
				CAL_TABLE_NORMAL_INSTANCE,
				event_id,
				st->time.utime);
		ret = _cal_db_util_query_get_first_int_result(query, NULL, &count);
		if (CALENDAR_ERROR_NONE != ret)
		{
			ERR("_cal_db_util_query_get_first_int_result() failed");
			g_strfreev(t);
			return ret;
		}
		DBG("count(%d)", count);
		index = 0;

		while (count > 0)
		{
			ucal = _cal_time_get_ucal(event->start_tzid, event->wkst);
			ucal_setMillis(ucal, sec2ms(st->time.utime), &ec);
			switch (freq)
			{
			case CALENDAR_RECURRENCE_YEARLY:
				ucal_add(ucal, UCAL_YEAR, index, &ec);
				ucal_set(ucal, UCAL_MONTH, 1);
				ucal_set(ucal, UCAL_DATE, 1);
				lli_s = ms2sec(ucal_getMillis(ucal, &ec));

				ucal_add(ucal, UCAL_YEAR, 1, &ec);
				lli_e = ms2sec(ucal_getMillis(ucal, &ec));
				break;

			case CALENDAR_RECURRENCE_MONTHLY:
				ucal_add(ucal, UCAL_MONTH, index, &ec);
				ucal_set(ucal, UCAL_DATE, 1);
				lli_s = ms2sec(ucal_getMillis(ucal, &ec));

				ucal_add(ucal, UCAL_MONTH, 1, &ec);
				lli_e = ms2sec(ucal_getMillis(ucal, &ec));
				break;

			case CALENDAR_RECURRENCE_WEEKLY:
				ucal_add(ucal, UCAL_DATE, (7 * index), &ec);
				ucal_set(ucal, UCAL_DAY_OF_WEEK, 1);
				lli_s = ms2sec(ucal_getMillis(ucal, &ec));

				ucal_add(ucal, UCAL_DATE, 7, &ec);
				lli_e = ms2sec(ucal_getMillis(ucal, &ec));
				break;

			default:
				break;
			}
			ucal_close(ucal);

			DBG("%lld %lld", lli_s, lli_e);
			__cal_db_instance_work_normal_setpos(event_id, lli_s, lli_e, t);
			snprintf(query, sizeof(query), "SELECT count(*) FROM %s "
					"WHERE event_id = %d "
					"AND dtstart_utime >= %lld",
					CAL_TABLE_NORMAL_INSTANCE,
					event_id,
					lli_e);
			ret = _cal_db_util_query_get_first_int_result(query, NULL, &count);
			if (CALENDAR_ERROR_NONE != ret)
			{
				ERR("_cal_db_util_query_get_first_int_result() failed");
				g_strfreev(t);
				return ret;
			}
			DBG("setpos left count(%d) from [%lld]", count, lli_e);
			index++;
		}
		break;

	case CALENDAR_TIME_LOCALTIME:
		y = st->time.date.year;
		m = st->time.date.month;
		d = st->time.date.mday;
		DBG("%d/%d/%d", y, m, d);
		snprintf(str_t, sizeof(str_t), "%04d%02d%02d", y, m, d);
		snprintf(query, sizeof(query), "SELECT count(*) FROM %s "
				"WHERE event_id = %d "
				"AND dtstart_datetime >= %s",
				CAL_TABLE_ALLDAY_INSTANCE,
				event_id,
				str_t);
		ret = _cal_db_util_query_get_first_int_result(query, NULL, &count);
		if (CALENDAR_ERROR_NONE != ret)
		{
			ERR("_cal_db_util_query_get_first_int_result() failed");
			g_strfreev(t);
			return ret;
		}
		DBG("count(%d)", count);
		index = 0;

		while (count > 0)
		{
			// _cal_db_instance_get_next_period(event, str_s, str_e, index);
			switch (freq)
			{
			case CALENDAR_RECURRENCE_YEARLY:
				snprintf(str_s, sizeof(str_s), "%04d%02d%02d", y + index, 1, 1);
				snprintf(str_e, sizeof(str_e), "%04d%02d%02d", y + index + 1, 1, 1);
				break;

			case CALENDAR_RECURRENCE_MONTHLY:
				snprintf(str_s, sizeof(str_s), "%04d%02d%02d",
						y + ((m + index) / 13),
						(m + index) % 13 == 0 ? 1 : (m + index) % 13,
						1);
				snprintf(str_e, sizeof(str_e), "%04d%02d%02d",
						y + ((m + index + 1) / 13),
						(m + index + 1) % 13 == 0 ? 1 : (m + index + 1) % 13,
						1);
				break;

			case CALENDAR_RECURRENCE_WEEKLY:
				ucal = _cal_time_get_ucal(event->start_tzid, event->wkst);
				ucal_setDate(ucal, y, m - 1, d + (7 * index), &ec);
				y1 = ucal_get(ucal, UCAL_YEAR, &ec);
				m1 = ucal_get(ucal, UCAL_MONTH, &ec) + 1;
				d1 = ucal_get(ucal, UCAL_DATE, &ec);
				DBG("start(%d/%d/%d", y1, m1, d1);
				ucal_set(ucal, UCAL_DAY_OF_WEEK, 1);
				y1 = ucal_get(ucal, UCAL_YEAR, &ec);
				m1 = ucal_get(ucal, UCAL_MONTH, &ec) + 1;
				d1 = ucal_get(ucal, UCAL_DATE, &ec);
				snprintf(str_s, sizeof(str_s), "%04d%02d%02d",
						y1, m1, d1);
				DBG("start(%d/%d/%d", y1, m1, d1);

				ucal_add(ucal, UCAL_DATE, 7, &ec);
				y1 = ucal_get(ucal, UCAL_YEAR, &ec);
				m1 = ucal_get(ucal, UCAL_MONTH, &ec) + 1;
				d1 = ucal_get(ucal, UCAL_DATE, &ec);
				snprintf(str_e, sizeof(str_e), "%04d%02d%02d",
						y1, m1, d1);
				DBG("end(%d/%d/%d", y1, m1, d1);
				ucal_close(ucal);
				break;

			case CALENDAR_RECURRENCE_DAILY:
				DBG("Not support setpos in daily");
				break;

			default:
				break;
			}

			__cal_db_instance_work_allday_setpos(event_id, str_s, str_e, t);

			snprintf(query, sizeof(query), "SELECT count(*) FROM %s "
					"WHERE event_id = %d "
					"AND dtstart_datetime >= %s",
					CAL_TABLE_ALLDAY_INSTANCE,
					event_id,
					str_e);
//			DBG("%s", query);
			ret = _cal_db_util_query_get_first_int_result(query, NULL, &count);
			if (CALENDAR_ERROR_NONE != ret)
			{
				ERR("_cal_db_util_query_get_first_int_result() failed");
				g_strfreev(t);
				return ret;
			}
			DBG("setpos left count(%d) from [%s]", count, str_e);
			index++;
		}
		break;

	default:
		break;
	}
	g_strfreev(t);
	return CALENDAR_ERROR_NONE;
}

static int __cal_db_instance_update_exdate_del(int id, char *exdate)
{
	int dbret;
	int i, j;
	int y, mon, d, h, min, s;
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	const char *dl = ",";
	char **t = NULL;
	char *p = NULL;

	if (exdate == NULL)
	{
		DBG("Nothing to update exdate del");
		return CALENDAR_ERROR_NONE;
	}

	DBG("exdate [%s]", exdate);
	t = g_strsplit(exdate, dl, -1);
	if (!t)
	{
		ERR("g_strsplit failed");
		return CALENDAR_ERROR_OUT_OF_MEMORY;
	}

	for (i = 0; t[i]; i++)
	{
		p = t[i];
		// remove space
		j = 0;
		while (p[j] == ' ')
		{
			j++;
		}
		p = t[i] + j;
		DBG("exdate[%s]", p);

		if (strlen(p) > strlen("YYYYMMDD"))
		{
			DBG("NORMAL instance");
			sscanf(p, "%04d%02d%02dT%02d%02d%02dZ",
					&y, &mon, &d, &h, &min, &s);
			snprintf(query, sizeof(query),
					"DELETE FROM %s "
					"WHERE event_id = %d AND dtstart_utime = %lld ",
					CAL_TABLE_NORMAL_INSTANCE,
					id, _cal_time_convert_itol(NULL, y, mon, d, h, min, s));
			DBG("(%lld)", _cal_time_convert_itol(NULL, y, mon, d, h, min, s));
		}
		else
		{
			DBG("ALLDAY instance");
			snprintf(query, sizeof(query),
					"DELETE FROM %s "
					"WHERE event_id = %d AND dtstart_datetime = %s ",
					CAL_TABLE_ALLDAY_INSTANCE,
					id, p);

		}
		dbret = _cal_db_util_query_exec(query);
		if (dbret != CAL_DB_OK) {
			ERR("_cal_db_util_query_exec() failed (%d)", dbret);
			g_strfreev(t);
			switch (dbret)
			{
			case CAL_DB_ERROR_NO_SPACE:
				return CALENDAR_ERROR_FILE_NO_SPACE;
			default:
				return CALENDAR_ERROR_DB_FAILED;
			}
		}
	}
	g_strfreev(t);
	return CALENDAR_ERROR_NONE;
}

static int __cal_db_instance_update_exdate_mod(int original_event_id, char *recurrence_id)
{
	int dbret;
	int i, j;
	int y, mon, d, h, min, s;
	const char *dl = ",";
	char **t = NULL;
	char *p = NULL;
	char query[CAL_DB_SQL_MAX_LEN] = {0};

	if (original_event_id < 1 || recurrence_id == NULL)
	{
		DBG("Nothing to update exdate mod");
		return CALENDAR_ERROR_NONE;
	}

	DBG("recurrence_id[%s]", recurrence_id);
	t = g_strsplit(recurrence_id, dl, -1);
	if (!t)
	{
		ERR("g_strsplit failed");
		return CALENDAR_ERROR_OUT_OF_MEMORY;
	}

	for (i = 0; t[i]; i++)
	{
		p = t[i];
		// remove space
		j = 0;
		while (p[j] == ' ')
		{
			j++;
		}
		p = t[i] + j;
		DBG("%d[%s]", i + 1, p);

		if (strlen(p) > strlen("YYYYMMDD"))
		{
			DBG("NORMAL instance");
			sscanf(p, "%04d%02d%02dT%02d%02d%02dZ",
					&y, &mon, &d, &h, &min, &s);
			snprintf(query, sizeof(query),
					"DELETE FROM %s "
					"WHERE event_id = %d AND dtstart_utime = %lld ",
					CAL_TABLE_NORMAL_INSTANCE,
					original_event_id, _cal_time_convert_itol(NULL, y, mon, d, h, min, s));
			DBG("(%lld)", _cal_time_convert_itol(NULL, y, mon, d, h, min, s));
		}
		else
		{
			DBG("ALLDAY instance");
			snprintf(query, sizeof(query),
					"DELETE FROM %s "
					"WHERE event_id = %d AND dtstart_datetime = %s ",
					CAL_TABLE_ALLDAY_INSTANCE,
					original_event_id, p);

		}
		dbret = _cal_db_util_query_exec(query);
		if (dbret != CAL_DB_OK) {
			ERR("_cal_db_util_query_exec() failed (%d)", dbret);
			g_strfreev(t);
			switch (dbret)
			{
			case CAL_DB_ERROR_NO_SPACE:
				return CALENDAR_ERROR_FILE_NO_SPACE;
			default:
				return CALENDAR_ERROR_DB_FAILED;
			}
		}
	}
	g_strfreev(t);
	return CALENDAR_ERROR_NONE;
}

static inline int ___cal_db_instance_has_after(calendar_time_s *t1, calendar_time_s *t2)
{
	if (t1->type == CALENDAR_TIME_UTIME) {
		if (t1->time.utime > t2->time.utime)
			return 1;
		else
			return 0;
	}

	DBG("%d %d %d /%d %d %d", t1->time.date.year, t1->time.date.month, t1->time.date.mday,
			t2->time.date.year, t2->time.date.month, t2->time.date.mday);
	if (t1->time.date.year > t2->time.date.year) {
		DBG("exit year");
		return 1;
	} else if (t1->time.date.year < t2->time.date.year) {
		return 0;
	} else {
		if (t1->time.date.month > t2->time.date.month) {
			return 1;
		} else if (t1->time.date.month < t2->time.date.month) {
			return 0;
		} else {
			if (t1->time.date.mday > t2->time.date.mday) {
				return 1;
			} else {
				return 0;
			}
		}
	}
}

static inline int __cal_db_instance_convert_mday(const char *str, int *mday)
{
	int d;

	if (!str || !*str) {
		ERR("Invalid argument: check mday[%s]", str);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	d = atoi(str);
	if (d < 1 || d > 31) {
		ERR("Invalid argument: check day(%d)", d);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	DBG("get mday[%s] and convert to int(%d)", str, d);
	*mday = d;
	return CALENDAR_ERROR_NONE;
}

static int __cal_db_instance_del_inundant(int event_id, calendar_time_s *st, cal_event_s *event)
{
    cal_db_util_error_e dbret = CAL_DB_OK;
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

	} else if (st->type == CALENDAR_TIME_LOCALTIME) {
		snprintf(query, sizeof(query), "DELETE FROM %s "
				"WHERE event_id = %d "
				"AND dtstart_datetime > (SELECT dtstart_datetime FROM %s "
				"WHERE event_id = %d ORDER BY dtstart_datetime LIMIT %d, 1) ",
				CAL_TABLE_ALLDAY_INSTANCE,
				event_id,
				CAL_TABLE_ALLDAY_INSTANCE,
				event_id, cnt -1);
	}

	dbret = _cal_db_util_query_exec(query);
	if (dbret != CAL_DB_OK)
	{
		DBG("query(%s)", query);
		switch (dbret)
		{
		case CAL_DB_ERROR_NO_SPACE:
			return CALENDAR_ERROR_FILE_NO_SPACE;
		default:
			return CALENDAR_ERROR_DB_FAILED;
		}
	}
	return CALENDAR_ERROR_NONE;
}

static void __cal_db_instance_print_caltime(calendar_time_s *caltime)
{
	if (caltime) {
		switch (caltime->type) {
		case CALENDAR_TIME_UTIME:
			DBG("utime(%lld)", caltime->time.utime);
			break;

		case CALENDAR_TIME_LOCALTIME:
			DBG("datetime(%04d/%02d/%02d)",
					caltime->time.date.year, caltime->time.date.month, caltime->time.date.mday);
			break;
		}
	}
}

static int __cal_db_instance_get_duration(UCalendar *ucal, calendar_time_s *st, calendar_time_s *et, int *duration)
{
	int _duration = -1;
	UErrorCode ec = U_ZERO_ERROR;
	UDate ud;

	if (NULL == ucal)
	{
		ERR("Invalid parameter: ucal is NULL");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	if (NULL == st || NULL == et)
	{
		ERR("Invalid parameter: calendar_time_s is NULL");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	switch (st->type) {
	case CALENDAR_TIME_UTIME:
		if (st->time.utime > et->time.utime)
		{
			ERR("check time: start(%lld) > end(%lld)", st->time.utime, et->time.utime);
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
		_duration = (int)(et->time.utime - st->time.utime);
		break;

	case CALENDAR_TIME_LOCALTIME:
		ucal_setDateTime(ucal, et->time.date.year, et->time.date.month -1, et->time.date.mday, 0, 0, 0, &ec);
		ud = ucal_getMillis(ucal, &ec);

		ucal_setDateTime(ucal, st->time.date.year, st->time.date.month -1, st->time.date.mday, 0, 0, 0, &ec);

		_duration = ucal_getFieldDifference(ucal, ud, UCAL_DATE, &ec);
		if (U_FAILURE(ec))
		{
			ERR("ucal_getFieldDifference failed (%s)", u_errorName(ec));
			return ec;
		}
		break;
	}

	if (duration) *duration = _duration;
	return CALENDAR_ERROR_NONE;
}

static int __cal_db_instance_parse_byday(const char *byday, int *nth, int *wday)
{
	int _nth = 0;
	int _wday = 0;
	char buf[3] = {0};

	if (NULL == byday || strlen(byday) == 0)
	{
		ERR("Invalid parameter: byday is NULL");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	if (!sscanf(byday, "%d", &_nth)) {
		DBG("no digit in front of byday, so set 1 as default");
		if (sscanf(byday, "%s", buf) != 1) {
			ERR("Failed to get byday[%s]", byday);
			return -1;
		}
		_nth = 1;
	} else {
		if (sscanf(byday, "%d%s", &_nth, buf) != 2) {
			ERR("Failed to get byday[%s]", byday);
			return -1;
		}
	}
	buf[2] = '\0';

	// CALENDAR_SUNDAY
	for (_wday = CALENDAR_SUNDAY; _wday <= CALENDAR_SATURDAY; _wday++) {
		if (!strncmp(wdays[_wday].str, buf, 2)) {
			DBG("inserted nth(%d) wday[%s]", _nth, wdays[_wday].str);
			break;
		}
	}

	if (nth) *nth = _nth > 4 ? -1 : _nth;
	if (wday) *wday = _wday;

	return CALENDAR_ERROR_NONE;
}

static void __cal_db_instance_set_ucal_start(UCalendar *ucal, calendar_time_s *st)
{
	UErrorCode ec = U_ZERO_ERROR;

	if (ucal)
	{
		switch (st->type)
		{
		case CALENDAR_TIME_UTIME:
			ucal_setMillis(ucal, sec2ms(st->time.utime), &ec);
			break;

		case CALENDAR_TIME_LOCALTIME:
			ucal_setDate(ucal, st->time.date.year,
					st->time.date.month -1, st->time.date.mday, &ec);
			break;
		}
	}
}

static void __cal_db_instance_set_wday(UCalendar *ucal, int nth, int wday)
{
	if (ucal)
	{
		if (nth > 0) ucal_set(ucal, UCAL_DAY_OF_WEEK, wday);
		if (wday > 0) ucal_set(ucal, UCAL_DAY_OF_WEEK_IN_MONTH, nth);
	}
}

static int __cal_db_instance_insert_record(UCalendar *ucal, int duration, int type, int event_id)
{
	int ret;
	long long int lli;
	UErrorCode ec = U_ZERO_ERROR;
	calendar_record_h record = NULL;
	calendar_time_s st = {0};
	calendar_time_s et = {0};
	UCalendar *ucal2 = NULL;

	if (NULL == ucal)
	{
		ERR("Invalid parameter: ucal is NULL");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	switch (type)
	{
	case CALENDAR_TIME_UTIME:
		lli =  ms2sec(ucal_getMillis(ucal, &ec));

		st.type = type;
		st.time.utime = lli;
		DBG("insert normal (%lld)%04d-%02d-%02d %02d:%02d:00",
				st.time.utime,
				ucal_get(ucal, UCAL_YEAR, &ec),
				ucal_get(ucal, UCAL_MONTH, &ec) +1,
				ucal_get(ucal, UCAL_DATE, &ec),
				ucal_get(ucal, UCAL_HOUR_OF_DAY, &ec),
				ucal_get(ucal, UCAL_MINUTE, &ec));
		et.type = type;
		et.time.utime = lli + (long long int)duration;

		ret = calendar_record_create(_calendar_instance_normal._uri, &record);
		if (CALENDAR_ERROR_NONE != ret)
		{
			ERR("calendar_record_create() failed");
			return CALENDAR_ERROR_DB_FAILED;
		}
		_cal_record_set_int(record, _calendar_instance_normal.event_id, event_id);
		ret = _cal_record_set_caltime(record, _calendar_instance_normal.start_time, st);
		if (CALENDAR_ERROR_NONE != ret)
		{
			ERR("_cal_record_set_caltime() failed");
			return ret;
		}
		ret = _cal_record_set_caltime(record, _calendar_instance_normal.end_time, et);
		if (CALENDAR_ERROR_NONE != ret)
		{
			ERR("_cal_record_set_caltime() failed");
			return ret;
		}
		break;

	case CALENDAR_TIME_LOCALTIME:
		st.type = type;
		st.time.date.year = ucal_get(ucal, UCAL_YEAR, &ec);
		st.time.date.month = ucal_get(ucal, UCAL_MONTH, &ec) +1;
		st.time.date.mday = ucal_get(ucal, UCAL_DATE, &ec);
		DBG("insert allday %04d-%02d-%02d",
				st.time.date.year, st.time.date.month, st.time.date.mday);
		et.type = type;
		if (duration > 0)
		{
			ucal2 = ucal_clone(ucal, &ec);
			ucal_add(ucal2, UCAL_DATE, duration, &ec);
			et.time.date.year = ucal_get(ucal2, UCAL_YEAR, &ec);
			et.time.date.month = ucal_get(ucal2, UCAL_MONTH, &ec) +1;
			et.time.date.mday = ucal_get(ucal2, UCAL_DATE, &ec);
			ucal_close(ucal2);
		}
		else
		{
			et.time.date.year = st.time.date.year;
			et.time.date.month = st.time.date.month;
			et.time.date.mday = st.time.date.mday;
		}

		ret = calendar_record_create(_calendar_instance_allday._uri, &record);
		if (CALENDAR_ERROR_NONE != ret)
		{
			ERR("calendar_record_create() failed");
			return ret;
		}
		ret = _cal_record_set_int(record, _calendar_instance_allday.event_id, event_id);
		if (CALENDAR_ERROR_NONE != ret)
		{
			ERR("_cal_record_set_int() failed");
			return ret;
		}
		ret = _cal_record_set_caltime(record, _calendar_instance_allday.start_time, st);
		if (CALENDAR_ERROR_NONE != ret)
		{
			ERR("_cal_record_set_caltime() failed");
			return ret;
		}
		ret = _cal_record_set_caltime(record, _calendar_instance_allday.end_time, et);
		if (CALENDAR_ERROR_NONE != ret)
		{
			ERR("_cal_record_set_caltime() failed");
			return ret;
		}
		break;
	}
	ret = calendar_db_insert_record(record, NULL);
	if (CALENDAR_ERROR_NONE != ret)
	{
		ERR("calendar_db_insert_record() failed");
		return ret;
	}
	calendar_record_destroy(record, true);

	return CALENDAR_ERROR_NONE;
}

static int __cal_db_instance_has_next(UCalendar *ucal, calendar_time_s *until)
{
	int has_next = 0;
	long long int lli = 0;
	long long int lli2 = 0;
	UErrorCode ec = U_ZERO_ERROR;
	UCalendar *ucal2 = NULL;

	if (NULL == ucal)
	{
		ERR("Invalid parameter: ucal is NULL");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (NULL == until)
	{
		ERR("Invalid parameter: until is NULL");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	switch (until->type)
	{
	case CALENDAR_TIME_UTIME:
		lli = ms2sec(ucal_getMillis(ucal, &ec));
		has_next = (lli > until->time.utime) ? 0 : 1;
		if (has_next == 0)
		{
			DBG("No next:get(%lld) until(%lld)", lli, until->time.utime);
		}
		break;

	case CALENDAR_TIME_LOCALTIME:
		lli = ms2sec(ucal_getMillis(ucal, &ec));

		ucal2 = ucal_clone(ucal, &ec);
		ucal_setDateTime(ucal2, until->time.date.year, until->time.date.month -1,
				until->time.date.mday, 0, 0, 0, &ec);
		lli2 = ms2sec(ucal_getMillis(ucal2, &ec));
		ucal_close(ucal2);

		has_next = (lli > lli2) ? 0 : 1;
		if (has_next == 0)
		{
			DBG("No next:get(%lld) until(%lld)", lli, lli2);
		}
		break;
	}


	return has_next;
}

static int __cal_db_instance_publish_with_wday(UCalendar *ucal, cal_event_s *event, int duration, int field, calendar_time_s *until)
{
	int ret = CALENDAR_ERROR_NONE;
	int i, j;
	int count = 0;
	int length = 0;
	char **t = NULL;
	const char *d = ",";
	UDate ud1, ud2;
	UErrorCode ec = U_ZERO_ERROR;

	if (event->byday && strlen(event->byday) > 0)
	{
		// range: MO, TU, WE, TH, FR, SA, SU ex> 3TH, -1MO
		t = g_strsplit(event->byday, d, -1);
		if (!t) {
			ERR("g_strsplit failed");
			g_strfreev(t);
			return CALENDAR_ERROR_OUT_OF_MEMORY;
		}
		length = g_strv_length(t);
	}
	else
	{
		int nth = 0, wday = 0;
		int nth2;
		char buf[8] = {0};
		UCalendar *ucal2 = NULL;

		ucal2 = ucal_clone(ucal, &ec);

		switch (event->start.type)
		{
		case CALENDAR_TIME_UTIME:
			ucal_setMillis(ucal, sec2ms(event->start.time.utime), &ec);
			break;

		case CALENDAR_TIME_LOCALTIME:
			ucal_setDate(ucal2, event->start.time.date.year,
					event->start.time.date.month -1, event->start.time.date.mday, &ec);
			break;
		}

		wday = ucal_get(ucal, UCAL_DAY_OF_WEEK, &ec);
		wday = wday < UCAL_SUNDAY ? UCAL_SUNDAY : wday;

		nth = ucal_get(ucal2, UCAL_DAY_OF_WEEK_IN_MONTH, &ec);
		ucal_add(ucal, UCAL_DATE, 7, &ec);
		nth2 = ucal_get(ucal2, UCAL_DAY_OF_WEEK_IN_MONTH, &ec);
		nth = (nth2 == 1) ? -1 : nth;

		ucal_close(ucal2);

		snprintf(buf, sizeof(buf), "%d%s", nth, wdays[wday].str);
		t = g_strsplit(buf, d, -1);
		if (!t) {
			ERR("g_strsplit failed");
			g_strfreev(t);
			return CALENDAR_ERROR_OUT_OF_MEMORY;
		}
		DBG("No byday, so set wday(%s)", t[0]);

		length = 1;
	}
	DBG("length(%d)", length);

	if (event->count)
	{
		count = event->count;

		if (event->bysetpos)
		{
			int share, remind;
			int length2;
			char **t2 = NULL;

			t2 = g_strsplit(event->bysetpos, ",", -1);
			length2 = g_strv_length(t2);
			g_strfreev(t2);

			share = length / length2;
			remind = length % length2;
			count = count * share + (remind ? length : 0);
		}
		else
		{
			count = count / length + ((count % length == 0) ? 0 : 1);
		}
		DBG("bycount(%d) count(%d)", event->count, count);
	}

	ud1 = ucal_getMillis(ucal, &ec);
	int week = ucal_get(ucal, field, &ec);

	for (i = 0; i < length; i++)
	{
		// set nth,  wday
		int nth = 0, wday = 0;
		ret = __cal_db_instance_parse_byday(t[i], &nth, &wday);
		if (CALENDAR_ERROR_NONE != ret)
		{
			ERR("__cal_db_instance_parse_byday() failed");
			g_strfreev(t);
			return ret;
		}
		ucal_setMillis(ucal, ud1, &ec);
		switch (field)
		{
		case UCAL_WEEK_OF_YEAR:
			ucal_set(ucal, field, week);
			ucal_set(ucal, UCAL_DAY_OF_WEEK, wday);
			break;
		default:
			__cal_db_instance_set_wday(ucal, nth, wday);
			break;
		}
		DBG("set wday nth(%d) wday(%d)", nth, wday);

		// check whether set ucal goes past or not
		ud1 = ucal_getMillis(ucal, &ec);

		UCalendar *ucal2 = NULL;
		ucal2 = ucal_clone(ucal, &ec);
		__cal_db_instance_set_ucal_start(ucal2, &event->start);
		ud2 = ucal_getMillis(ucal2, &ec);
		ucal_close(ucal2);

		if (ms2sec(ud2) > ms2sec(ud1))
		{
			DBG("Invalid first time: start(%lld) get(%lld)", ms2sec(ud2), ms2sec(ud1));
			switch (field)
			{
			case UCAL_WEEK_OF_YEAR:
				ucal_add(ucal, field, event->interval, &ec);
				break;

			default:
				ucal_add(ucal, field, event->interval, &ec);
				__cal_db_instance_set_wday(ucal, nth, wday);
				break;
			}
		}

		DBG("range type(%d) interval(%d)", event->range_type, event->interval);
		switch (event->range_type)
		{
		case CALENDAR_RANGE_COUNT:
			for (j = 0; j < count; j++)
			{
				__cal_db_instance_insert_record(ucal, duration, event->start.type, event->index);
				ucal_add(ucal, field, event->interval, &ec);
				switch (field)
				{
				case UCAL_WEEK_OF_YEAR:
					break;
				default:
					__cal_db_instance_set_wday(ucal, nth, wday);
					break;
				}
			}
			break;

		case CALENDAR_RANGE_UNTIL:
		case CALENDAR_RANGE_NONE:
			while (__cal_db_instance_has_next(ucal, until))
			{
				__cal_db_instance_insert_record(ucal, duration, event->start.type, event->index);
				ucal_add(ucal, field, event->interval, &ec);
				switch (field)
				{
				case UCAL_WEEK_OF_YEAR:
					break;
				default:
					__cal_db_instance_set_wday(ucal, nth, wday);
					break;
				}
			}
			break;
		}
	}
	g_strfreev(t);

	return CALENDAR_ERROR_NONE;
}

static int __cal_db_instance_publish_with_mday(UCalendar *ucal, cal_event_s *event, int duration, int field, calendar_time_s *until)
{
	int i;
	int count = 0;
	int length = 0;
	int get_mday;
	char **t = NULL;
	const char *d = ",";
	UDate ud1, ud2;
	UErrorCode ec = U_ZERO_ERROR;

	if (event->bymonthday && strlen(event->bymonthday) > 0)
	{
		// range: 1, 2, ... , 31 ex> 1, 5, 7
		t = g_strsplit(event->bymonthday, d, -1);
		if (!t) {
			ERR("g_strsplit failed");
			g_strfreev(t);
			return CALENDAR_ERROR_OUT_OF_MEMORY;
		}
		length = g_strv_length(t);
		DBG("has bymonthday");

	}
	else
	{
		int mday = 0;
		char buf[8] = {0};
		switch (event->start.type)
		{
		case CALENDAR_TIME_UTIME:
			_cal_time_ltoi(event->start_tzid, event->start.time.utime, NULL, NULL, &mday);
			snprintf(buf, sizeof(buf), "%d", mday);
			break;

		case CALENDAR_TIME_LOCALTIME:
			snprintf(buf, sizeof(buf), "%d", event->start.time.date.mday);
			break;
		}
		t = g_strsplit(buf, d, -1);
		if (!t) {
			ERR("g_strsplit failed");
			g_strfreev(t);
			return CALENDAR_ERROR_OUT_OF_MEMORY;
		}
		DBG("No bymonthday, so set mday(%s)", t[0]);

		length = 1;
	}

	if (event->count)
	{
		count = event->count;

		if (event->bysetpos)
		{
			int share, remind;
			int length2;
			char **t2 = NULL;

			t2 = g_strsplit(event->bysetpos, ",", -1);
			length2 = g_strv_length(t2);
			g_strfreev(t2);

			share = length / length2;
			remind = length % length2;
			count = count * share + (remind ? length : 0);
			DBG("share(%d) remind(%d)", share, remind);
		}
		else
		{
			count = count / length + ((count % length == 0) ? 0 : 1);
		}
		DBG("count(%d)", count);
	}

	ud1 = ucal_getMillis(ucal, &ec);

	for (i = 0; i < length; i++)
	{
		int mday = atoi(t[i]);
		DBG("mday is set(%d)", mday);
		mday = mday < 1 ? mday +1 : mday; // In ICU, last day is 0 not -1.

		ucal_setMillis(ucal, ud1, &ec);
		ucal_set(ucal, UCAL_DATE, mday);

		// check whether set ucal goes past or not
		ud1 = ucal_getMillis(ucal, &ec);

		UCalendar *ucal2 = NULL;
		ucal2 = ucal_clone(ucal, &ec);
		__cal_db_instance_set_ucal_start(ucal2, &event->start);
		ud2 = ucal_getMillis(ucal2, &ec);
		ucal_close(ucal2);

		if (ms2sec(ud2) > ms2sec(ud1))
		{
			DBG("Invalid first time(%lld) > (%lld), so added interval",
					ms2sec(ud2), ms2sec(ud1));
			ucal_add(ucal, field, event->interval, &ec);
		}

		int j = 0, k = 0;
		DBG("range type(%d)", event->range_type);
		switch (event->range_type)
		{
		case CALENDAR_RANGE_COUNT:
			if (count == 1)
			{
				__cal_db_instance_insert_record(ucal, duration, event->start.type, event->index);
				break;
			}

			for (j = 0; j < count; j++)
			{
				__cal_db_instance_insert_record(ucal, duration, event->start.type, event->index);
				ucal_add(ucal, field, event->interval, &ec);
				if (event->freq != CALENDAR_RECURRENCE_DAILY)
				{
					if (mday > 0)
					{
						while ((get_mday = ucal_get(ucal, UCAL_DATE, &ec)) != mday)
						{
							k++;
							DBG("Skip this date, we got mday(%d) no match with mday(%d)", get_mday, mday);
							ucal_setMillis(ucal, ud1, &ec);
							ucal_set(ucal, UCAL_DATE, mday);
							ucal_add(ucal, field, event->interval * (j + k), &ec);
							if ((ucal_get(ucal, UCAL_YEAR, &ec) > CAL_ENDLESS_LIMIT_YEAR) || (k > 50))
							{
								ERR("Out of range");
								j = count;
								break;
							}
						}
					}
					else
					{
						ucal_add(ucal, field, event->interval, &ec);
						ucal_set(ucal, UCAL_DATE, mday);
					}
				}
			}
			break;

		case CALENDAR_RANGE_UNTIL:
		case CALENDAR_RANGE_NONE:
			j = 0;
			while (__cal_db_instance_has_next(ucal, until))
			{
				__cal_db_instance_insert_record(ucal, duration, event->start.type, event->index);
				ucal_add(ucal, field, event->interval, &ec);
				if (event->freq != CALENDAR_RECURRENCE_DAILY)
				{
					if (mday > 0)
					{
						while ((get_mday = ucal_get(ucal, UCAL_DATE, &ec)) != mday)
						{
							k++;
							DBG("Skip this date, we got mday(%d) no match with mday(%d)", get_mday, mday);
							ucal_setMillis(ucal, ud1, &ec);
							ucal_set(ucal, UCAL_DATE, mday);
							ucal_add(ucal, field, event->interval * (j + k), &ec);
							if ((ucal_get(ucal, UCAL_YEAR, &ec) > CAL_ENDLESS_LIMIT_YEAR) || (k > 50))
							{
								ERR("Out of range");
								j = count;
								break;
							}
						}
					}
					else
					{
						ucal_add(ucal, field, event->interval, &ec);
						ucal_set(ucal, UCAL_DATE, mday);
					}
				}
				j++;
			}
			break;
		}
	}
	g_strfreev(t);

	return CALENDAR_ERROR_NONE;
}

static int __cal_db_instance_publish_with_weekno(UCalendar *ucal, cal_event_s *event, int duration, int field, calendar_time_s *until)
{
	int i, j;
	int count = 0;
	int length = 0;
	char **t = NULL;
	const char *d = ",";
	UDate ud1, ud2;
	UErrorCode ec = U_ZERO_ERROR;

	if (event->byweekno && strlen(event->byweekno) > 0)
	{
		// range: -366, -1, 1, 366 ex> 3,200
		t = g_strsplit(event->byweekno, d, -1);
		if (!t) {
			ERR("g_strsplit failed");
			g_strfreev(t);
			return CALENDAR_ERROR_OUT_OF_MEMORY;
		}
		length = g_strv_length(t);
		DBG("has bymonthday");

	}

	if (event->count)
	{
		count = event->count;

		if (event->bysetpos)
		{
			int share, remind;
			int length2;
			char **t2 = NULL;

			t2 = g_strsplit(event->bysetpos, ",", -1);
			length2 = g_strv_length(t2);
			g_strfreev(t2);

			share = length / length2;
			remind = length % length2;
			count = count * share + (remind ? length : 0);
			DBG("share(%d) remind(%d)", share, remind);
		}
		else
		{
			count = count / length + ((count % length == 0) ? 0 : 1);
			DBG("length(%d)", length);
		}
		DBG("bycount(%d) count(%d)", event->count, count);
	}

	int wday;
	wday = ucal_get(ucal, UCAL_DAY_OF_WEEK, &ec);
	ud1 = ucal_getMillis(ucal, &ec);

	for (i = 0; i < length; i++)
	{
		int weekno = atoi(t[i]);

		ucal_setMillis(ucal, ud1, &ec);
		ucal_set(ucal, UCAL_WEEK_OF_YEAR, weekno);
		DBG("weekno is set(%d)", weekno);

		// check whether set ucal goes past or not
		ud1 = ucal_getMillis(ucal, &ec);

		UCalendar *ucal2 = NULL;
		ucal2 = ucal_clone(ucal, &ec);
		__cal_db_instance_set_ucal_start(ucal2, &event->start);
		ud2 = ucal_getMillis(ucal2, &ec);
		ucal_close(ucal2);

		if (ms2sec(ud2) > ms2sec(ud1))
		{
			DBG("Invalid first time: start(%lld) get(%lld)", ms2sec(ud2), ms2sec(ud1));
			ucal_add(ucal, field, 1, &ec);
		}

		DBG("range type(%d) interval(%d)", event->range_type, event->interval);
		switch (event->range_type)
		{
		case CALENDAR_RANGE_COUNT:
			for (j = 0; j < count; j++)
			{
				__cal_db_instance_insert_record(ucal, duration, event->start.type, event->index);
				ucal_add(ucal, field, event->interval, &ec);
			}
			break;

		case CALENDAR_RANGE_UNTIL:
		case CALENDAR_RANGE_NONE:
			while (__cal_db_instance_has_next(ucal, until))
			{
				__cal_db_instance_insert_record(ucal, duration, event->start.type, event->index);
				ucal_add(ucal, field, event->interval, &ec);
			}
			break;
		}
	}
	g_strfreev(t);

	return CALENDAR_ERROR_NONE;
}


static int __cal_db_instance_publish_with_yday(UCalendar *ucal, cal_event_s *event, int duration, int field, calendar_time_s *until)
{
	int i, j;
	int count = 0;
	int length = 0;
	char **t = NULL;
	const char *d = ",";
	UDate ud1, ud2;
	UErrorCode ec = U_ZERO_ERROR;

	if (event->byyearday && strlen(event->byyearday) > 0)
	{
		// range: -366, -1, 1, 366 ex> 3,200
		t = g_strsplit(event->byyearday, d, -1);
		if (!t) {
			ERR("g_strsplit failed");
			g_strfreev(t);
			return CALENDAR_ERROR_OUT_OF_MEMORY;
		}
		length = g_strv_length(t);
		DBG("has bymonthday");

	}

	if (event->count)
	{
		count = event->count;

		if (event->bysetpos)
		{
			int share, remind;
			int length2;
			char **t2 = NULL;

			t2 = g_strsplit(event->bysetpos, ",", -1);
			length2 = g_strv_length(t2);
			g_strfreev(t2);

			share = length / length2;
			remind = length % length2;
			count = count * share + (remind ? length : 0);
			DBG("share(%d) remind(%d)", share, remind);
		}
		else
		{
			count = count / length + ((count % length == 0) ? 0 : 1);
			DBG("length(%d)", length);
		}
		DBG("bycount(%d) count(%d)", event->count, count);
	}

	ud1 = ucal_getMillis(ucal, &ec);

	for (i = 0; i < length; i++)
	{
		int yday = atoi(t[i]);

		ucal_setMillis(ucal, ud1, &ec);
		ucal_set(ucal, UCAL_DAY_OF_YEAR, yday);
		DBG("yday is set(%d)", yday);

		// check whether set ucal goes past or not
		ud1 = ucal_getMillis(ucal, &ec);

		UCalendar *ucal2 = NULL;
		ucal2 = ucal_clone(ucal, &ec);
		__cal_db_instance_set_ucal_start(ucal2, &event->start);
		ud2 = ucal_getMillis(ucal2, &ec);
		ucal_close(ucal2);

		if (ms2sec(ud2) > ms2sec(ud1))
		{
			DBG("Invalid first time: start(%lld) get(%lld)", ms2sec(ud2), ms2sec(ud1));
			ucal_add(ucal, field, 1, &ec);
		}

		DBG("range type(%d) interval(%d)", event->range_type, event->interval);
		switch (event->range_type)
		{
		case CALENDAR_RANGE_COUNT:
			for (j = 0; j < count; j++)
			{
				__cal_db_instance_insert_record(ucal, duration, event->start.type, event->index);
				ucal_add(ucal, field, event->interval, &ec);
			}
			break;

		case CALENDAR_RANGE_UNTIL:
		case CALENDAR_RANGE_NONE:
			while (__cal_db_instance_has_next(ucal, until))
			{
				__cal_db_instance_insert_record(ucal, duration, event->start.type, event->index);
				ucal_add(ucal, field, event->interval, &ec);
			}
			break;
		}
	}
	g_strfreev(t);

	return CALENDAR_ERROR_NONE;
}

static int __cal_db_instance_publish_record_yearly(UCalendar *ucal, cal_event_s *event, int duration, calendar_time_s *until)
{
	int i;
	char **t = NULL;
	const char *d = ",";

	if (NULL == ucal)
	{
		ERR("Invalid parameter: ucal is NULL");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (NULL == event)
	{
		ERR("Invalid parameter: event is NULL");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	__cal_db_instance_set_ucal_start(ucal, &event->start);
	__cal_db_instance_print_ucal(ucal);

	if (event->bymonth && strlen(event->bymonth) > 0)
	{
		// range: 1 ~ 12 ex> 1, 2, 4
		t = g_strsplit(event->bymonth, d, -1);
		if (!t) {
			ERR("g_strsplit failed");
			return CALENDAR_ERROR_OUT_OF_MEMORY;
		}

		for (i = 0; t[i]; i++)
		{
			int month = atoi(t[i]);
			ucal_set(ucal, UCAL_MONTH, month -1);

			if (event->byday && strlen(event->byday) > 0)
			{
				__cal_db_instance_publish_with_wday(ucal, event, duration, UCAL_YEAR, until);
			}
			else if (event->bymonthday && strlen(event->bymonthday) > 0)
			{
				__cal_db_instance_publish_with_mday(ucal, event, duration, UCAL_YEAR, until);
			}
			else
			{
				ERR("Not completed");
			}
		}
		g_strfreev(t);
	}
	else if (event->byyearday && strlen(event->byyearday) > 0)
	{
		__cal_db_instance_publish_with_yday(ucal, event, duration, UCAL_YEAR, until);

	}
	else if (event->byweekno && strlen(event->byweekno) > 0)
	{
		__cal_db_instance_publish_with_weekno(ucal, event, duration, UCAL_YEAR, until);

		// range: 1 ~ 53 or -53 ~ -1 ex> 1, 4
		t = g_strsplit(event->byweekno, d, -1);
		if (!t) {
			ERR("g_strsplit failed");
			return CALENDAR_ERROR_OUT_OF_MEMORY;
		}

		for (i = 0; t[i]; i++)
		{

		}

		g_strfreev(t);
	}
	else
	{
		UErrorCode ec = U_ZERO_ERROR;
		int month = ucal_get(ucal, UCAL_MONTH, &ec) + 1;
		DBG("No bymonth, so set start time month(%d)", month);

		if (event->byday && strlen(event->byday) > 0)
		{
			__cal_db_instance_publish_with_wday(ucal, event, duration, UCAL_YEAR, until);
		}
		else
		{
			__cal_db_instance_publish_with_mday(ucal, event, duration, UCAL_YEAR, until);
		}
	}

	return CALENDAR_ERROR_NONE;
}

static int __cal_db_instance_publish_record_monthly(UCalendar *ucal, cal_event_s *event, int duration, calendar_time_s *until)
{
	if (NULL == ucal)
	{
		ERR("Invalid parameter: ucal is NULL");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (NULL == event)
	{
		ERR("Invalid parameter: event is NULL");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	__cal_db_instance_set_ucal_start(ucal, &event->start);
	__cal_db_instance_print_ucal(ucal);

	if (event->byday && strlen(event->byday) > 0)
	{
		__cal_db_instance_publish_with_wday(ucal, event, duration, UCAL_MONTH, until);
	}
	else
	{
		// bymonthday or none
		__cal_db_instance_publish_with_mday(ucal, event, duration, UCAL_MONTH, until);
	}

	return CALENDAR_ERROR_NONE;
}

static int __cal_db_instance_publish_record_weekly(UCalendar *ucal, cal_event_s *event, int duration, calendar_time_s *until)
{
	if (NULL == ucal)
	{
		ERR("Invalid parameter: ucal is NULL");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (NULL == event)
	{
		ERR("Invalid parameter: event is NULL");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	__cal_db_instance_set_ucal_start(ucal, &event->start);
	__cal_db_instance_publish_with_wday(ucal, event, duration, UCAL_WEEK_OF_YEAR, until);

	return CALENDAR_ERROR_NONE;
}

static int __cal_db_instance_publish_record_daily(UCalendar *ucal, cal_event_s *event, int duration, calendar_time_s *until)
{
	if (NULL == ucal)
	{
		ERR("Invalid parameter: ucal is NULL");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (NULL == event)
	{
		ERR("Invalid parameter: event is NULL");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	__cal_db_instance_set_ucal_start(ucal, &event->start);
	__cal_db_instance_print_ucal(ucal);

	__cal_db_instance_publish_with_mday(ucal, event, duration, UCAL_DATE, until);

	return CALENDAR_ERROR_NONE;
}

static int __cal_db_instance_publish_record_once(UCalendar *ucal, cal_event_s *event, int duration, calendar_time_s *until)
{
	if (NULL == ucal)
	{
		ERR("Invalid parameter: ucal is NULL");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (NULL == event)
	{
		ERR("Invalid parameter: event is NULL");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	__cal_db_instance_set_ucal_start(ucal, &event->start);
	__cal_db_instance_print_ucal(ucal);

	event->interval = 1;
	event->count = 1;
	event->range_type = CALENDAR_RANGE_COUNT;

	// start:for exception record which has original_event_id, mod freq 0

	__cal_db_instance_publish_with_mday(ucal, event, duration, UCAL_DATE, until);
	return CALENDAR_ERROR_NONE;
}

static int __cal_db_instance_publish_record_details(UCalendar *ucal, cal_event_s *event)
{
	if (NULL == ucal)
	{
		ERR("Invalid parameter: ucal is NULL");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (NULL == event)
	{
		ERR("Invalid parameter: event is NULL");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	int duration = -1;
	int exception_freq; // for exception
	long long int range = 0;
	calendar_time_s until = {0};

	__cal_db_instance_get_duration(ucal, &event->start, &event->end, &duration);

	if (event->original_event_id > 0)
	{
		DBG("this is exception event so publish only one instance");
		exception_freq = event->freq;
		event->freq = CALENDAR_RECURRENCE_NONE;
	}

	DBG("event interval(%d)", event->interval);
	if (event->interval < 1)
	{
		DBG("Invalid interval, so set 1");
		event->interval = 1;
	}

	switch (event->range_type)
	{
	case CALENDAR_RANGE_COUNT:
		DBG("range count");
		break;

	case CALENDAR_RANGE_UNTIL:
		DBG("range until");
		until.type = event->until_type;
		switch (until.type)
		{
		case CALENDAR_TIME_UTIME:
			range = _cal_time_convert_itol(NULL, CAL_ENDLESS_LIMIT_YEAR,
					CAL_ENDLESS_LIMIT_MONTH, CAL_ENDLESS_LIMIT_MDAY,
					0, 0, 0);
			if (event->until_utime > range)
			{
				DBG("until time(%lld) > max, so set max(%lld)", event->until_utime, range);
				until.time.utime = range;
			}
			else
			{
				until.time.utime = event->until_utime;
			}
			break;

		case CALENDAR_TIME_LOCALTIME:
			until.time.date.year = event->until_year;
			until.time.date.month = event->until_month;
			until.time.date.mday = event->until_mday;
			break;
		}
		break;

	case CALENDAR_RANGE_NONE:
		DBG("range none");
		until.type = event->until_type;
		switch (until.type)
		{
		case CALENDAR_TIME_UTIME:
			until.time.utime = _cal_time_convert_itol(event->start_tzid,
					CAL_ENDLESS_LIMIT_YEAR,
					CAL_ENDLESS_LIMIT_MONTH,
					CAL_ENDLESS_LIMIT_MDAY,
					0, 0, 0);
			break;
		case CALENDAR_TIME_LOCALTIME:
			until.time.date.year = CAL_ENDLESS_LIMIT_YEAR;
			until.time.date.month = CAL_ENDLESS_LIMIT_MONTH;
			until.time.date.mday = CAL_ENDLESS_LIMIT_MDAY;
			break;
		}
		break;
	}

	switch (event->freq)
	{
	case CALENDAR_RECURRENCE_YEARLY:
		__cal_db_instance_publish_record_yearly(ucal, event, duration, &until);
		break;

	case CALENDAR_RECURRENCE_MONTHLY:
		__cal_db_instance_publish_record_monthly(ucal, event, duration, &until);
		break;

	case CALENDAR_RECURRENCE_WEEKLY:
		__cal_db_instance_publish_record_weekly(ucal, event, duration, &until);
		break;

	case CALENDAR_RECURRENCE_DAILY:
		__cal_db_instance_publish_record_daily(ucal, event, duration, &until);
		break;

	case CALENDAR_RECURRENCE_NONE:
	default:
		__cal_db_instance_publish_record_once(ucal, event, duration, &until);
		break;

	}

	if (event->bysetpos)
	{
		__cal_db_instance_apply_setpos(event->index, &event->start, event, event->freq);
	}

	if (event->original_event_id > 0)
	{
		DBG("return freq for exception event");
		event->freq = exception_freq;
	}

	return CALENDAR_ERROR_NONE;
}

int _cal_db_instance_publish_record(calendar_record_h record)
{
	cal_event_s *event;

	if (NULL == record)
	{
		ERR("Invalid argument: record is NULL");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	event = (cal_event_s *)(record);

	DBG("Start");
	__cal_db_instance_print_caltime(&event->start);
	DBG("End");
	__cal_db_instance_print_caltime(&event->end);


	UCalendar *ucal = NULL;
	switch (event->start.type)
	{
	case CALENDAR_TIME_UTIME:
		ucal = _cal_time_get_ucal(event->start_tzid, event->wkst);
		break;

	case CALENDAR_TIME_LOCALTIME:
		ucal = _cal_time_get_ucal(NULL, event->wkst);
		break;
	}

	__cal_db_instance_publish_record_details(ucal, event);
	__cal_db_instance_del_inundant(event->index, &event->start, event);
	__cal_db_instance_update_exdate_del(event->index, event->exdate);
	__cal_db_instance_update_exdate_mod(event->original_event_id, event->recurrence_id);

	ucal_close(ucal);

	return CALENDAR_ERROR_NONE;
}

int _cal_db_instance_get_now(long long int *current)
{
	*current = ms2sec(ucal_getNow());
	return CALENDAR_ERROR_NONE;
}

int _cal_db_instance_discard_record(calendar_record_h record)
{
	char query[CAL_DB_SQL_MAX_LEN] = {0};
    cal_db_util_error_e dbret = CAL_DB_OK;
	cal_event_s *event;

	event = (cal_event_s *)(record);
	if (event == NULL) {
		ERR("Invalid argument: cal_event_s is NULL");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	DBG("delete normal");
	snprintf(query, sizeof(query), "DELETE FROM %s WHERE event_id = %d ",
			CAL_TABLE_NORMAL_INSTANCE, event->index);

	dbret = _cal_db_util_query_exec(query);
	if (CAL_DB_OK != dbret)
	{
		DBG("query[%s]", query);
		ERR("_cal_db_util_query_exec() failed (%d)", dbret);
		switch (dbret)
		{
		case CAL_DB_ERROR_NO_SPACE:
			return CALENDAR_ERROR_FILE_NO_SPACE;
		default:
			return CALENDAR_ERROR_DB_FAILED;
		}
	}

	DBG("delete allday");
	snprintf(query, sizeof(query), "DELETE FROM %s WHERE event_id = %d ",
			CAL_TABLE_ALLDAY_INSTANCE, event->index);

	dbret = _cal_db_util_query_exec(query);
	if (CAL_DB_OK != dbret)
	{
		ERR("_cal_db_util_query_exec() failed (%d)", dbret);
		switch (dbret)
		{
		case CAL_DB_ERROR_NO_SPACE:
			return CALENDAR_ERROR_FILE_NO_SPACE;
		default:
			return CALENDAR_ERROR_DB_FAILED;
		}
	}

	return CALENDAR_ERROR_NONE;
}

