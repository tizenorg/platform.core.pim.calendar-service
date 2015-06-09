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
#include <unistd.h>
#include <unicode/utypes.h>
#include <unicode/ucal.h>
#include <unicode/uloc.h>
#include <unicode/calendar.h>
#include <unicode/timezone.h>
#include <unicode/gregocal.h>
#include <unicode/simpletz.h>
#include <unicode/ustring.h>
#include <unicode/strenum.h>
#include <unicode/ustdio.h>
#include <unicode/udat.h>
#include <unicode/rbtz.h>
#include <unicode/uclean.h>

#include "calendar.h"
#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_time.h"
#include "cal_utils.h"

#define ULOC_LOCALE_IDENTIFIER_CAPACITY (ULOC_FULLNAME_CAPACITY + 1 + ULOC_KEYWORD_AND_VALUES_CAPACITY)
#define uprv_strncpy(dst, src, size) U_STANDARD_CPP_NAMESPACE strncpy(dst, src, size)

#define ms2sec(ms) (long long int)(ms / 1000.0)
#define sec2ms(s) (s * 1000.0)

static UCalendar *_g_ucal_gmt = NULL;

void cal_time_get_registered_tzid_with_offset(int offset, char *registered_tzid, int tzid_size)
{
	UErrorCode ec = U_ZERO_ERROR;

	RET_IF(NULL == registered_tzid);

	StringEnumeration* s = TimeZone::createEnumeration(sec2ms(offset));
	if (0 == s->count(ec)) {
		DBG("No tzid of offset(%d)sec", offset);
		return;
	}

	const UnicodeString *unicode_tzid = s->snext(ec);
	unicode_tzid->extract(registered_tzid, tzid_size, NULL, ec);
	delete s;
}

UCalendar *cal_time_open_ucal(int calendar_system_type, const char *tzid, int wkst)
{
	UChar utf16_timezone[CAL_STR_SHORT_LEN64] = {0};
	u_uastrncpy(utf16_timezone, tzid, sizeof(utf16_timezone));

	UErrorCode status = U_ZERO_ERROR;
	UCalendar *ucal = NULL;

	char localeBuf[ULOC_LOCALE_IDENTIFIER_CAPACITY] = {0};

	switch (calendar_system_type) {
	case CALENDAR_SYSTEM_EAST_ASIAN_LUNISOLAR:
		uloc_setKeywordValue("calendar", "chinese", localeBuf, ULOC_LOCALE_IDENTIFIER_CAPACITY, &status);
		ucal = ucal_open(utf16_timezone, -1, localeBuf, UCAL_TRADITIONAL, &status);
		break;
	default:
		ucal = ucal_open(utf16_timezone, -1, uloc_getDefault(), UCAL_GREGORIAN, &status);
		break;
	}

	if (U_FAILURE(status)) {
		ERR("ucal_open() Fail(%s)", u_errorName(status));
		return NULL;
	}
	if (CALENDAR_SUNDAY <= wkst && wkst <= CALENDAR_SATURDAY) {
		DBG("set wkst(%d)", wkst);
		ucal_setAttribute(ucal, UCAL_FIRST_DAY_OF_WEEK, wkst);
	}
	return ucal;
}

static void cal_time_set_caltime(UCalendar *ucal, calendar_time_s *ct)
{
	UErrorCode status = U_ZERO_ERROR;

	switch (ct->type) {
	case CALENDAR_TIME_UTIME:
		ucal_setMillis(ucal, sec2ms(ct->time.utime), &status);
		break;
	case CALENDAR_TIME_LOCALTIME:
		ucal_setDateTime(ucal, ct->time.date.year, ct->time.date.month -1, ct->time.date.mday,
				ct->time.date.hour, ct->time.date.minute, ct->time.date.second, &status);
		break;
	default:
		ERR("Invalid dtstart type. Current time is used in default");
		return;
	}
	RETM_IF(U_FAILURE(status), "ucal_setMillis() Fail(%s)", u_errorName(status));
}

char* cal_time_extract_by(int calendar_system_type, const char *tzid, int wkst, calendar_time_s *ct, int field)
{
	int vali;
	char buf[8] = {0};
	char weeks[7][3] = {"SU", "MO", "TU", "WE", "TH", "FR", "SA"};
	UCalendar *ucal = NULL;
	UErrorCode status = U_ZERO_ERROR;

	ucal = cal_time_open_ucal(calendar_system_type, tzid, wkst);
	if (NULL == ucal) {
		ERR("cal_time_open_ucal() Fail");
		return NULL;
	}
	cal_time_set_caltime(ucal, ct);

	switch (field) {
	case CAL_MONTH:
		vali = ucal_get(ucal, UCAL_MONTH, &status) + 1;
		snprintf(buf, sizeof(buf), "%d", vali);
		break;

	case CAL_DATE:
		vali = ucal_get(ucal, UCAL_DATE, &status);
		snprintf(buf, sizeof(buf), "%d", vali);
		break;

	case CAL_DAY_OF_WEEK:
		vali = ucal_get(ucal, UCAL_DAY_OF_WEEK, &status);
		snprintf(buf, sizeof(buf), "%s", weeks[vali - 1]);
		break;

	default:
		break;

	}
	ucal_close(ucal);
	return cal_strdup(buf);
}

char* cal_time_convert_ltos(const char *tzid, long long int lli, int is_allday)
{
	int y, mon, d, h, min, s;
	char buf[CAL_STR_SHORT_LEN32] = {0};
	UCalendar *ucal;
	UErrorCode status = U_ZERO_ERROR;

	if (NULL == tzid) {
		DBG("tzid is NULL so set gmt");
		tzid = CAL_TZID_GMT;
	}

	ucal = cal_time_open_ucal(-1, tzid, -1);
	if (NULL == ucal) {
		ERR("cal_time_open_ucal() Fail");
		return NULL;
	}
	ucal_setMillis(ucal, sec2ms(lli), &status);
	if (U_FAILURE(status)) {
		ERR("ucal_setMillFail (%s)", u_errorName(status));
		ucal_close(ucal);
		return NULL;
	}

	y = ucal_get(ucal, UCAL_YEAR, &status);
	mon = ucal_get(ucal, UCAL_MONTH, &status) + 1;
	d = ucal_get(ucal, UCAL_DATE, &status);
	h = ucal_get(ucal, UCAL_HOUR_OF_DAY, &status);
	min = ucal_get(ucal, UCAL_MINUTE, &status);
	s = ucal_get(ucal, UCAL_SECOND, &status);

	if (CAL_STRING_EQUAL == strncmp(tzid, CAL_TZID_GMT, strlen(CAL_TZID_GMT))) {
		snprintf(buf, sizeof(buf), "%04d%02d%02dT%02d%02d%02d%s", y, mon, d, h, min, s,
				is_allday ? "" : "Z");
	}
	else {
		snprintf(buf, sizeof(buf), "%04d%02d%02dT%02d%02d%02d", y, mon, d, h, min, s);
	}

	ucal_close(ucal);

	return cal_strdup(buf);
}

long long int cal_time_convert_itol(const char *tzid, int y, int mon, int d, int h, int min, int s)
{
	long long int lli;
	UCalendar *ucal = NULL;
	UErrorCode status = U_ZERO_ERROR;

	ucal = cal_time_open_ucal(-1, tzid, -1);
	if (NULL == ucal) {
		ERR("cal_time_open_ucal() Fail");
		return 0;
	}
	ucal_set(ucal, UCAL_YEAR, y);
	ucal_set(ucal, UCAL_MONTH, mon -1);
	ucal_set(ucal, UCAL_DATE, d);
	ucal_set(ucal, UCAL_HOUR_OF_DAY, h);
	ucal_set(ucal, UCAL_MINUTE, min);
	ucal_set(ucal, UCAL_SECOND, s);
	ucal_set(ucal, UCAL_MILLISECOND, 0);
	lli = ms2sec(ucal_getMillis(ucal, &status));

	ucal_close(ucal);
	return lli;
}

long long int cal_time_get_now(void)
{
	return ms2sec(ucal_getNow());
}

int cal_time_get_next_date(calendar_time_s *today, calendar_time_s *next)
{
	RETV_IF(NULL == today, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == next, CALENDAR_ERROR_INVALID_PARAMETER);

	UCalendar *ucal = NULL;
	UErrorCode status = U_ZERO_ERROR;
	UChar *utzid = NULL;
	const char *tzid = CAL_TZID_GMT;

	utzid = (UChar *)calloc(strlen(tzid) + 1, sizeof(UChar));
	RETVM_IF(NULL == utzid, CALENDAR_ERROR_OUT_OF_MEMORY, "calloc() Fail");

	u_uastrcpy(utzid, tzid);
	ucal = ucal_open(utzid, u_strlen(utzid), "en_US", UCAL_TRADITIONAL, &status);
	if (U_FAILURE(status)) {
		ERR("ucal_open() Fail(%s)", u_errorName(status));
		CAL_FREE(utzid);
		return status;
	}

	switch (today->type) {
	case CALENDAR_TIME_UTIME:

		break;

	case CALENDAR_TIME_LOCALTIME:
		ucal_setDateTime(ucal,
				today->time.date.year,
				today->time.date.month-1,
				today->time.date.mday,
				today->time.date.hour,
				today->time.date.minute,
				today->time.date.second,
				&status);
		DBG("today %04d/%02d/%02d %02d:%02d:%02d",
				today->time.date.year, today->time.date.month, today->time.date.mday,
				today->time.date.hour, today->time.date.minute, today->time.date.second);
		ucal_add(ucal, UCAL_DAY_OF_YEAR, 1, &status);
		next->time.date.year = ucal_get(ucal, UCAL_YEAR, &status);
		next->time.date.month = ucal_get(ucal, UCAL_MONTH, &status) + 1;
		next->time.date.mday = ucal_get(ucal, UCAL_DATE, &status);
		next->time.date.hour = ucal_get(ucal, UCAL_HOUR_OF_DAY, &status);
		next->time.date.minute = ucal_get(ucal, UCAL_MINUTE, &status);
		next->time.date.second = ucal_get(ucal, UCAL_SECOND, &status);
		DBG("next %04d/%02d/%02d %02d:%02d:%02d",
				next->time.date.year, next->time.date.month, next->time.date.mday,
				next->time.date.hour, next->time.date.minute, next->time.date.second);
		break;
	}

	CAL_FREE(utzid);

	return CALENDAR_ERROR_NONE;
}

/*
 * Read link of /opt/etc/localtime,
 * and get timezone "Asia/Seoul" from the string (ig. "/usr/share/zoneinfo/Asia/Seoul")
 */
char* cal_time_get_timezone(void)
{
	char buf[CAL_STR_MIDDLE_LEN] = {0};
	ssize_t len = readlink("/opt/etc/localtime", buf, sizeof(buf)-1);
	RETVM_IF(-1 == len, NULL, "readlink() Fail");

	buf[len] = '\0';
	return cal_strdup(buf + strlen("/usr/share/zoneinfo/"));
}

void cal_time_u_cleanup(void)
{
	u_cleanup();
}

void cal_time_get_tz_offset(const char *tz, time_t *zone_offset, time_t *dst_offset)
{
	UErrorCode status = U_ZERO_ERROR;
	UCalendar *ucal = NULL;

	ucal = cal_time_open_ucal(-1, tz, -1);
	if (NULL == ucal) {
		ERR("cal_time_open_ucal() Fail");
		return;
	}
	int32_t zone = ucal_get(ucal, UCAL_ZONE_OFFSET, &status);
	int32_t dst = ucal_get(ucal, UCAL_DST_OFFSET, &status);
	ucal_close(ucal);

	if (zone_offset) *zone_offset = ms2sec(zone);
	if (dst_offset) *dst_offset = ms2sec(dst);
}

bool cal_time_in_dst(const char *tz, long long int t)
{
	UErrorCode status = U_ZERO_ERROR;
	UCalendar *ucal = NULL;

	ucal = cal_time_open_ucal(-1, tz, -1);
	if (NULL == ucal) {
		ERR("cal_time_open_ucal() Fail");
		return false;
	}
	ucal_setMillis(ucal, sec2ms(t), &status);
	bool is_dst = ucal_inDaylightTime(ucal, &status);
	ucal_close(ucal);

	return is_dst;
}

int cal_time_init(void)
{
	UCalendar *ucal = NULL;

	if (NULL == _g_ucal_gmt) {
		ucal = cal_time_open_ucal(-1, NULL, -1);
		RETVM_IF(NULL == ucal, CALENDAR_ERROR_SYSTEM, "cal_time_open_ucal() Fail");
		_g_ucal_gmt = ucal;
	}
	return CALENDAR_ERROR_NONE;
}

void cal_time_fini(void)
{
	if (_g_ucal_gmt) {
		ucal_close(_g_ucal_gmt);
		_g_ucal_gmt = NULL;
	}
}

static UCalendar* __get_gmt_ucal(void)
{
	if (NULL == _g_ucal_gmt) {
		cal_time_init();
	}
	return _g_ucal_gmt;
}

long long int cal_time_convert_lli(char *p)
{
	UErrorCode status = U_ZERO_ERROR;

	if (p && *p) {
		int y = 0, m = 0, d = 0;
		int h = 0, n = 0, s = 0;
		sscanf(p,  "%04d%02d%02dT%02d%02d%02dZ", &y, &m, &d, &h, &n, &s);

		UCalendar *ucal = __get_gmt_ucal();
		ucal_setDateTime(ucal, y, m -1, d, h, n, s, &status);
		return ms2sec(ucal_getMillis(ucal, &status));
	}
	return 0;
}

void cal_time_modify_caltime(calendar_time_s *caltime, long long int diff)
{
	UErrorCode status = U_ZERO_ERROR;
	RET_IF(NULL == caltime);

	UCalendar *ucal = __get_gmt_ucal();
	long long int lli = 0;
	switch (caltime->type) {
	case CALENDAR_TIME_UTIME:
		DBG("Before (%lld)", caltime->time.utime);
		caltime->time.utime += diff;
		DBG("After (%lld)", caltime->time.utime);
		break;

	case CALENDAR_TIME_LOCALTIME:
		DBG("Before %04d%02d%02dT%02d%02d%02d%s", caltime->time.date.year, caltime->time.date.month, caltime->time.date.mday,
				caltime->time.date.hour, caltime->time.date.minute, caltime->time.date.second);

		ucal_setDateTime(ucal, caltime->time.date.year, caltime->time.date.month - 1, caltime->time.date.mday,
				caltime->time.date.hour, caltime->time.date.minute, caltime->time.date.second, &status);
		lli = ms2sec(ucal_getMillis(ucal, &status));
		lli += diff;
		ucal_setMillis(ucal, sec2ms(lli), &status);
		caltime->time.date.year = ucal_get(ucal, UCAL_YEAR, &status);
		caltime->time.date.month = ucal_get(ucal, UCAL_MONTH, &status) + 1;
		caltime->time.date.mday = ucal_get(ucal, UCAL_DATE, &status);
		caltime->time.date.hour = ucal_get(ucal, UCAL_HOUR_OF_DAY, &status);
		caltime->time.date.minute = ucal_get(ucal, UCAL_MINUTE, &status);
		caltime->time.date.second = ucal_get(ucal, UCAL_SECOND, &status);

		DBG("After %04d%02d%02dT%02d%02d%02d%s", caltime->time.date.year, caltime->time.date.month, caltime->time.date.mday,
				caltime->time.date.hour, caltime->time.date.minute, caltime->time.date.second);
		break;
	}
}

void cal_time_get_nth_wday(long long int t, int *nth, int *wday)
{
	RET_IF(NULL == nth);
	RET_IF(NULL == wday);

	UErrorCode status = U_ZERO_ERROR;
	UCalendar *ucal = __get_gmt_ucal();
	ucal_setMillis(ucal, sec2ms(t), &status);
	*wday = ucal_get(ucal, UCAL_DAY_OF_WEEK, &status);
	/* check if nth is last */
	int this_week = ucal_get(ucal, UCAL_DAY_OF_WEEK_IN_MONTH, &status);
	ucal_add(ucal, UCAL_DAY_OF_YEAR, 7, &status);
	int next_week = ucal_get(ucal, UCAL_DAY_OF_WEEK_IN_MONTH, &status);
	*nth = next_week == 1 ? -1 : this_week;
	DBG("nth(%d) wday(%d)", *nth, *wday);
}

void cal_time_get_datetime(long long int t, int *y, int *m, int *d, int *h, int *n, int *s)
{
	UErrorCode status = U_ZERO_ERROR;
	UCalendar *ucal = __get_gmt_ucal();
	if (NULL == ucal) {
		ERR("__get_gmt_ucal() Fail");
		return;
	}
	ucal_setMillis(ucal, sec2ms(t), &status);
	if (y) *y = ucal_get(ucal, UCAL_YEAR, &status);
	if (m) *m = ucal_get(ucal, UCAL_MONTH, &status) + 1;
	if (d) *d = ucal_get(ucal, UCAL_DATE, &status);
	if (h) *h = ucal_get(ucal, UCAL_HOUR_OF_DAY, &status);
	if (n) *n = ucal_get(ucal, UCAL_MINUTE, &status);
	if (s) *s = ucal_get(ucal, UCAL_SECOND, &status);
	ucal_close(ucal);
}

void cal_time_get_local_datetime(char *tzid, long long int t, int *y, int *m, int *d, int *h, int *n, int *s)
{
	UErrorCode status = U_ZERO_ERROR;
	UCalendar *ucal = cal_time_open_ucal(-1, tzid, 0);
	if (NULL == ucal) {
		ERR("__get_gmt_ucal() Fail");
		return;
	}
	ucal_setMillis(ucal, sec2ms(t), &status);
	if (y) *y = ucal_get(ucal, UCAL_YEAR, &status);
	if (m) *m = ucal_get(ucal, UCAL_MONTH, &status) + 1;
	if (d) *d = ucal_get(ucal, UCAL_DATE, &status);
	if (h) *h = ucal_get(ucal, UCAL_HOUR_OF_DAY, &status);
	if (n) *n = ucal_get(ucal, UCAL_MINUTE, &status);
	if (s) *s = ucal_get(ucal, UCAL_SECOND, &status);
	ucal_close(ucal);
}

bool cal_time_is_available_tzid(char *tzid)
{
	UErrorCode ec = U_ZERO_ERROR;
	StringEnumeration* s = TimeZone::createEnumeration();
	int32_t s_count = s->count(ec);

	int len = strlen(tzid);
	int i, j;
	for (i = 0; i < s_count; i++) {
		char buf[CAL_STR_SHORT_LEN32] = {0};
		const UnicodeString *unicode_tzid = s->snext(ec);
		for (j = 0; j < unicode_tzid->length(); j++) {
			buf[j] = unicode_tzid->charAt(j);
		}
		buf[j] = '\0';
		if (CAL_STRING_EQUAL == strncmp(buf, tzid, len)) {
			return true;
		}
	}
	return false;
}

