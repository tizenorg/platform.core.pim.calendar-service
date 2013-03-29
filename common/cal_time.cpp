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

#include "calendar2.h"
#include "cal_internal.h"
#include "cal_typedef.h"

#include "cal_time.h"

#define ULOC_LOCALE_IDENTIFIER_CAPACITY (ULOC_FULLNAME_CAPACITY + 1 + ULOC_KEYWORD_AND_VALUES_CAPACITY)
#define uprv_strncpy(dst, src, size) U_STANDARD_CPP_NAMESPACE strncpy(dst, src, size)

#define ms2sec(ms) (long long int)(ms / 1000.0)
#define sec2ms(s) (s * 1000.0)

int _cal_time_is_registered_tzid(const char *tzid)
{
	int is_found = 0;
	int i;
	UErrorCode ec = U_ZERO_ERROR;

	if (NULL == tzid)
	{
		ERR("tzid is NULL");
		return is_found;
	}

	StringEnumeration* s = TimeZone::createEnumeration();
	int32_t s_count = s->count(ec);

	for (i = 0; i < s_count; i++)
	{
		char buf[128] = {0};
		const UnicodeString *unicode_tzid = s->snext(ec);
		unicode_tzid->extract(buf, sizeof(buf), NULL, ec);
		if (!strncmp(tzid, buf, strlen(buf)))
		{
			is_found = 1;
			break;
		}
	}
	delete s;
	return is_found;
}

int _cal_time_get_timezone_from_table(const char *tzid, calendar_record_h *timezone, int *timezone_id)
{
	int ret = CALENDAR_ERROR_NONE;
	int count = 0;
	int _timezone_id = 0;
	calendar_record_h _timezone = NULL;
	calendar_filter_h filter = NULL;
	calendar_query_h query = NULL;
	calendar_list_h list = NULL;

	ret = calendar_connect();
	if (CALENDAR_ERROR_NONE != ret)
	{
		ERR("calendar_connect() failed");
		return ret;
	}

	ret = calendar_query_create(_calendar_timezone._uri, &query);
	if (CALENDAR_ERROR_NONE != ret) goto ERROR_CONNECT;

	ret = calendar_filter_create(_calendar_timezone._uri, &filter);
	if (CALENDAR_ERROR_NONE != ret) goto ERROR_QUERY;

	ret = calendar_filter_add_str(filter, _calendar_timezone.standard_name, CALENDAR_MATCH_EXACTLY, tzid);
	if (CALENDAR_ERROR_NONE != ret) goto ERROR_FILTER;

	ret = calendar_query_set_filter(query, filter);
	if (CALENDAR_ERROR_NONE != ret) goto ERROR_FILTER;

	ret = calendar_db_get_records_with_query(query, 0, 0, &list);
	if (CALENDAR_ERROR_NONE != ret) goto ERROR_LIST;

	ret = calendar_list_get_count(list, &count);
	if (CALENDAR_ERROR_NONE != ret) goto ERROR_LIST;
	DBG("tzid count(%d)", count);
	if (count <= 0)
	{
		DBG("No count");
		calendar_list_destroy(list, false);
		calendar_filter_destroy(filter);
		calendar_query_destroy(query);
		calendar_disconnect();
		return CALENDAR_ERROR_NONE;
	}

	ret = calendar_list_first(list);
	if (CALENDAR_ERROR_NONE != ret) goto ERROR_LIST;

	ret = calendar_list_get_current_record_p(list, &_timezone);
	if (CALENDAR_ERROR_NONE != ret) goto ERROR_LIST;

	ret = calendar_record_get_int(_timezone, _calendar_timezone.id, &_timezone_id);
	if (CALENDAR_ERROR_NONE != ret) goto ERROR_LIST;

	calendar_list_destroy(list, false);
	calendar_filter_destroy(filter);
	calendar_query_destroy(query);
	calendar_disconnect();

	if (timezone)
	{
		*timezone = _timezone;
	}
	else
	{
		calendar_record_destroy(_timezone, true);
	}
	if (timezone_id) *timezone_id = _timezone_id;

	return CALENDAR_ERROR_NONE;

ERROR_LIST:
	ERR("list error");
	calendar_list_destroy(list, true);
ERROR_FILTER:
	ERR("filter error");
	calendar_filter_destroy(filter);
ERROR_QUERY:
	ERR("queryerror");
	calendar_query_destroy(query);
ERROR_CONNECT:
	calendar_disconnect();

	return ret;
}

static int __cal_time_get_like_utzid(UChar *utzid, int len, const char *tzid, calendar_record_h timezone, char **like_tzid)
{
	int gmtoffset;
	UErrorCode ec = U_ZERO_ERROR;

	if (NULL == timezone)
	{
		ERR("Invalid parameter: timezone is NULL");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	// make utzid
	UnicodeString zoneStrID;
	int32_t l = (len < 0 ? u_strlen(utzid) : len);
	zoneStrID.setTo((UBool)(len < 0), utzid, l); /* temporary read-only alias */

	// make simple timezone
	cal_timezone_s *tz = (cal_timezone_s *)timezone;
	gmtoffset = sec2ms(tz->tz_offset_from_gmt * 60);

	if (tz->day_light_bias == 0)
	{
		char buf[128] = {0};
		snprintf(buf, sizeof(buf), "ETC/GMT%c%d",
				gmtoffset < 0 ? '-' : '+',
				tz->tz_offset_from_gmt / 60);
		DBG("No dayligit, set like tzid[%s]", buf);
		*like_tzid = strdup(buf);
		return CALENDAR_ERROR_NONE;
	}

	DBG("Try to find like tzid with daylight savings");
	SimpleTimeZone *stz = new SimpleTimeZone(
			sec2ms(tz->tz_offset_from_gmt * 60),
			zoneStrID,
			tz->day_light_start_month -1,
			tz->day_light_start_position_of_week,
			tz->day_light_start_day,
			sec2ms(tz->day_light_start_hour * 3600),
			tz->std_start_month -1,
			tz->std_start_position_of_week,
			tz->std_start_day,
			sec2ms(tz->std_start_hour * 3600),
			sec2ms(tz->day_light_bias * 60),
			ec);

	int i, j;
	UDate now = Calendar::getNow();
	TimeZoneTransition trans;
	UDate stzudate[5];
	int32_t stzrawoff[4];
	int32_t stzdstoff[4];
	stzudate[0] = now;
	DBG("tzid[%s]", tzid);
	for (i = 0; i < 4; i++) // get 4 date: dst begin & end, std begin & end
	{
		stz->getPreviousTransition(stzudate[i], (UBool)false, trans);
		stz->getOffset(stzudate[i], (UBool)true, stzrawoff[i], stzdstoff[i], ec);
		stzudate[i +1] = trans.getTime();
		DBG("(%lld)(%d)(%d)", ms2sec(stzudate[i +1]), stzrawoff[i], stzdstoff[i]);
	}
	delete stz;

	// extract from all
	int32_t rawoff;
	int32_t dstoff;
	int32_t s_count;
	int is_found = 0;
	char *_like_tzid = NULL;
	UDate udate;
	UnicodeString canonicalID, tmpCanonical;
	StringEnumeration* s = TimeZone::createEnumeration(gmtoffset);
	s_count = s->count(ec);

	DBG("Has count(%d) with the same gmtoffset(%d)", s_count, gmtoffset);
	if (s_count == 0)
	{
		DBG("No count matched");
		return -1;
	}

	for (i = 0; i < s_count; i++)
	{
		const UnicodeString *unicode_tzid = s->snext(ec);
		TimeZone *_timezone = TimeZone::createTimeZone(*unicode_tzid);

		if (_timezone->getDSTSavings() != (gmtoffset))
		{
			delete _timezone;
			continue;
		}

		RuleBasedTimeZone *rule = (RuleBasedTimeZone *)_timezone;
		udate = now;
		for (j = 0; j < 4; j++)
		{
			rule->getPreviousTransition(udate, (UBool)false, trans);
			rule->getOffset(udate, (UBool)true, rawoff, dstoff, ec);
			udate = trans.getTime();
			DBG("(%lld)(%d)(%d)", ms2sec(udate), rawoff, dstoff);

			if (udate == stzudate[i+1] && rawoff == stzrawoff[i] && dstoff == stzdstoff[i])
			{
				DBG("Found matched");
				is_found = 1;
				break;
			}
		}

		if (is_found)
		{
			_like_tzid = (char *)calloc(unicode_tzid->length() +1, sizeof(char));
			for (i = 0; i < unicode_tzid->length(); i++)
			{
				_like_tzid[i] = unicode_tzid->charAt(i);
			}
			DBG("Found and set like tzid[%s]", _like_tzid);
			*like_tzid = _like_tzid;
			delete _timezone;
			break;
		}

		delete _timezone;
	}
	delete s;
	return CALENDAR_ERROR_NONE;
}

UCalendar *_cal_time_get_ucal(const char *tzid, int wkst)
{
	int ret = CALENDAR_ERROR_NONE;
	char *like_tzid = NULL;
	UCalendar *ucal = NULL;
	UErrorCode status = U_ZERO_ERROR;
	UChar *utzid = NULL;

	if (tzid == NULL)
	{
		DBG("tzid is NULL so set gmt");
		tzid = CAL_TZID_GMT;
	}

	utzid = (UChar*)calloc(strlen(tzid) + 1, sizeof(UChar));
	if (utzid == NULL)
	{
		ERR("Failed to calloc");
		return NULL;
	}
	u_uastrcpy(utzid, tzid);

	if (_cal_time_is_registered_tzid(tzid) == 0)
	{
		DBG("Not registered tzid so try to find from timezone table");

		calendar_record_h timezone = NULL;
		ret = _cal_time_get_timezone_from_table(tzid, &timezone, NULL);
		if (ret == CALENDAR_ERROR_NONE)
		{
			DBG("Found something in the timezone table");
			ret = __cal_time_get_like_utzid(utzid, u_strlen(utzid), tzid, timezone, &like_tzid);
			if (ret != CALENDAR_ERROR_NONE)
			{
				DBG("Failed to find matched tzid with the found tzid, so set GMT");
				like_tzid = strdup(CAL_TZID_GMT);
			}
			DBG("Found matched tzid with the found tzid[%s]", like_tzid);
		}
		else
		{
			ERR("Failed to find timezone from table with this tzid[%s], so set gmt", tzid);
			like_tzid = strdup(CAL_TZID_GMT);
		}
		CAL_FREE(utzid);

		DBG("[%s]", like_tzid);
		utzid = (UChar*)calloc(strlen(like_tzid) + 1, sizeof(UChar));
		if (utzid == NULL)
		{
			ERR("Failed to calloc");
			CAL_FREE(like_tzid);
			return NULL;
		}
		u_uastrcpy(utzid, like_tzid);
		CAL_FREE(like_tzid);
	}
	else
	{
		DBG("This tzid is valid");
	}

	// get ucal
	ucal = ucal_open(utzid, u_strlen(utzid), "en_US", UCAL_TRADITIONAL, &status);
	if (U_FAILURE(status)) {
		ERR("ucal_open failed (%s)", u_errorName(status));
		CAL_FREE(utzid);
		return NULL;
	}
	CAL_FREE(utzid);

	if (wkst >= CALENDAR_SUNDAY && wkst <= CALENDAR_SATURDAY)
	{
		DBG("set wkst(%d)", wkst);
		ucal_setAttribute(ucal, UCAL_FIRST_DAY_OF_WEEK, wkst);
	}

	return ucal;
}

int _cal_time_get_like_tzid(const char *tzid, calendar_record_h timezone, char **like_tzid)
{
	int ret = CALENDAR_ERROR_NONE;
	UChar *utzid = NULL;

	if (NULL == timezone)
	{
		ERR("Invalid parameter: timezone is NULL");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	if (tzid == NULL)
	{
		DBG("tzid is NULL so set gmt");
		tzid = CAL_TZID_GMT;
	}

	utzid = (UChar*)calloc(strlen(tzid) + 1, sizeof(UChar));
	if (utzid == NULL)
	{
		ERR("Failed to calloc");
		return CALENDAR_ERROR_DB_FAILED;
	}
	u_uastrcpy(utzid, tzid);

	ret = __cal_time_get_like_utzid(utzid, u_strlen(utzid), tzid, timezone, like_tzid);
	if (ret != CALENDAR_ERROR_NONE)
	{
		DBG("Failed to find from timezone table, so set GMT");
		*like_tzid = strdup(CAL_TZID_GMT);
	}
	CAL_FREE(utzid);

	return CALENDAR_ERROR_NONE;
}

void _cal_time_set_caltime(UCalendar *ucal, calendar_time_s *ct)
{
	UErrorCode status = U_ZERO_ERROR;

	switch (ct->type)
	{
	case CALENDAR_TIME_UTIME:
		ucal_setMillis(ucal, sec2ms(ct->time.utime), &status);
		retm_if(U_FAILURE(status), "ucal_setMillis() failed(%s)",
				u_errorName(status));
		break;

	case CALENDAR_TIME_LOCALTIME:
		ucal_setDate(ucal,
				ct->time.date.year,
				ct->time.date.month -1,
				ct->time.date.mday,
				&status);
		retm_if(U_FAILURE(status), "ucal_setMillis() failed(%s)",
				u_errorName(status));
		break;

	default:
		ERR("Invalid dtstart type. Current time is used in default");
	}
}

char * _cal_time_extract_by(const char *tzid, int wkst, calendar_time_s *ct, int field)
{
	int vali;
	char buf[8] = {0};
	char weeks[7][3] = {"SU", "MO", "TU", "WE", "TH", "FR", "SA"};
	UCalendar *ucal = NULL;
	UErrorCode status = U_ZERO_ERROR;

	ucal = _cal_time_get_ucal(tzid, wkst);
	_cal_time_set_caltime(ucal, ct);

	switch (field)
	{
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
	return strdup(buf);
}

char * _cal_time_convert_ltos(const char *tzid, long long int lli)
{
	int y, mon, d, h, min, s;
	char buf[32] = {0};
	UCalendar *ucal;
	UErrorCode status = U_ZERO_ERROR;

	if (tzid == NULL)
	{
		DBG("tzid is NULL so set gmt");
		tzid = CAL_TZID_GMT;
	}

	ucal = _cal_time_get_ucal(tzid, -1);
	ucal_setMillis(ucal, sec2ms(lli), &status);
	if (U_FAILURE(status)) {
		ERR("ucal_setMillis failed (%s)", u_errorName(status));
		return NULL;
	}

	y = ucal_get(ucal, UCAL_YEAR, &status);
	mon = ucal_get(ucal, UCAL_MONTH, &status) + 1;
	d = ucal_get(ucal, UCAL_DATE, &status);
	h = ucal_get(ucal, UCAL_HOUR_OF_DAY, &status);
	min = ucal_get(ucal, UCAL_MINUTE, &status);
	s = ucal_get(ucal, UCAL_SECOND, &status);

	if (!strncmp(tzid, CAL_TZID_GMT, strlen(CAL_TZID_GMT)))
	{
		snprintf(buf, sizeof(buf), "%04d%02d%02dT%02d%02d%02dZ",
			y, mon, d, h, min, s);
	}
	else
	{
		snprintf(buf, sizeof(buf), "%04d%02d%02dT%02d%02d%02d",
			y, mon, d, h, min, s);
	}

	ucal_close(ucal);

	return strdup(buf);
}

long long int _cal_time_convert_itol(const char *tzid, int y, int mon, int d, int h, int min, int s)
{
	long long int lli;
	UCalendar *ucal = NULL;
	UErrorCode status = U_ZERO_ERROR;

	ucal = _cal_time_get_ucal(tzid, -1);

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

long long int _cal_time_convert_stol(char *tzid, char *datetime)
{
	int y, mon, d, h, min, s;
	char t, z;

	if (datetime == NULL || strlen(datetime) == 0) {
		ERR("Invalid argument");
		return -1;
	}

	sscanf(datetime,  "%4d%2d%2d%c%2d%2d%2d%c",
			&y, &mon, &d, &t, &h, &min, &s, &z);

	return _cal_time_convert_itol(tzid, y, mon, d, h, min, s);
}

int _cal_time_ltoi(char *tzid, long long int lli, int *year, int *month, int *mday)
{
	UCalendar *ucal;
	UErrorCode status = U_ZERO_ERROR;

	ucal = _cal_time_get_ucal(tzid, 1);
	ucal_setMillis(ucal, sec2ms(lli), &status);

	if (year) *year = ucal_get(ucal, UCAL_YEAR, &status);
	if (month) *month = ucal_get(ucal, UCAL_MONTH, &status) +1;
	if (mday) *mday = ucal_get(ucal, UCAL_DATE, &status);

	ucal_close(ucal);
	return CALENDAR_ERROR_NONE;
}

int _cal_time_ltoi2(char *tzid, long long int lli, int *nth, int *wday)
{
	UCalendar *ucal;
	UErrorCode status = U_ZERO_ERROR;

	ucal = _cal_time_get_ucal(tzid, 1);
	ucal_setMillis(ucal, sec2ms(lli), &status);

	if (wday)  *wday = ucal_get(ucal, UCAL_DAY_OF_WEEK, &status);
	if (nth)
	{
		int temp = 0;
		int temp2 = 0;
		temp = ucal_get(ucal, UCAL_DAY_OF_WEEK_IN_MONTH, &status);
		ucal_add(ucal, UCAL_DATE, 7, &status);
		temp2 = ucal_get(ucal, UCAL_DAY_OF_WEEK_IN_MONTH, &status);
		DBG("temp(%d) temp2(%d)", temp, temp2);
		*nth = temp2 == 1 ? -1 : temp;
	}

	ucal_close(ucal);
	return CALENDAR_ERROR_NONE;
}

long long int _cal_time_get_now(void)
{
	return ms2sec(ucal_getNow());
}

