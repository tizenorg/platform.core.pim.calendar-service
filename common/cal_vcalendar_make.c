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

#include <stdlib.h>

#include "calendar_list.h"
#include "calendar_db.h"

#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_record.h"
#include "cal_view.h"
#include "cal_time.h"

#include "cal_vcalendar.h"
#include "cal_vcalendar_make.h"

#define VCAL_DATETIME_FORMAT_YYYYMMDD "%04d%02d%02d"
#define VCAL_DATETIME_FORMAT_YYYYMMDDTHHMMSS "%04d%02d%02dT%02d%02d%02d"
#define VCAL_DATETIME_FORMAT_YYYYMMDDTHHMMSSZ "%04d%02d%02dT%02d%02d%02dZ"

enum {
	VCAL_VER_1 = 1,
	VCAL_VER_2 = 2,
};

typedef enum
{
	EVENT_ATTENDEE_REQ_PARTICIPANT_ROLE=0,
	EVENT_ATTENDEE_OPT_PARTICIPANT_ROLE,
	EVENT_ATTENDEE_NON_PARTICIPANT_ROLE,
	EVENT_ATTENDEE_CHAIR_ROLE,
} cal_event_attendee_role_type_t;

typedef enum
{
	EVENT_ATTENDEE_NEEDS_ACTION_AT_STATUS=0,
	EVENT_ATTENDEE_ACCEPTED_AT_STATUS,
	EVENT_ATTENDEE_DECLINED_AT_STATUS,
	EVENT_ATTENDEE_TENTATIVE_AT_STATUS,
	EVENT_ATTENDEE_DELEGATED_AT_STATUS,
	EVENT_ATTENDEE_COMPLETED_AT_STATUS,
	EVENT_ATTENDEE_IN_PROCESS_AT_STATUS
} cal_event_attendee_status_type_t;


static const char *_att_role[] = {
	[EVENT_ATTENDEE_REQ_PARTICIPANT_ROLE] = "REQ-PARTICIPANT",
	[EVENT_ATTENDEE_OPT_PARTICIPANT_ROLE] = "OPT-PARTICIPANT",
	[EVENT_ATTENDEE_NON_PARTICIPANT_ROLE] = "NON-PARTICIPANT",
	[EVENT_ATTENDEE_CHAIR_ROLE] = "CHAIR",
};

static const char *_att_st[] = {
	[EVENT_ATTENDEE_NEEDS_ACTION_AT_STATUS] = "NEEDS-ACTION",
	[EVENT_ATTENDEE_ACCEPTED_AT_STATUS] = "ACCEPTED",
	[EVENT_ATTENDEE_DECLINED_AT_STATUS] = "DECLINED",
	[EVENT_ATTENDEE_TENTATIVE_AT_STATUS] = "TENTATIVE",
	[EVENT_ATTENDEE_DELEGATED_AT_STATUS] = "DELEGATED",
	[EVENT_ATTENDEE_COMPLETED_AT_STATUS] = "COMPLETED",
	[EVENT_ATTENDEE_IN_PROCESS_AT_STATUS] = "IN-PROCESS",
};

#define _strlen(s) (((s) && *(s)) ? strlen(s) : 0)

static inline int __cal_vcalendar_make_alloc(cal_make_s *b, int n)
{
	b->data = realloc(b->data, b->size + n);

	retvm_if(!b->data, CALENDAR_ERROR_OUT_OF_MEMORY, "Failed to realloc");
	b->size += n;

	return CALENDAR_ERROR_NONE;
}

cal_make_s *_cal_vcalendar_make_new(void)
{
	cal_make_s *b;

	b = calloc(1, sizeof(cal_make_s));
	if (!b) {
		return NULL;
	}

	b->data = calloc(1, sizeof(char));
	if (!b->data) {
		free(b);
		return NULL;
	}

	*b->data = '\0';
	b->size = 1;

	return b;
}

static inline int __cal_vcalendar_make_folding(cal_make_s *b)
{
	int ret;
	ret = __cal_vcalendar_make_alloc(b, _strlen(b->lbuf) + 3);
	retv_if(CALENDAR_ERROR_NONE != ret, ret);

	strncat(b->data, b->lbuf, b->size - _strlen(b->data) - 1);
	strncat(b->data, "\r\n ", b->size - _strlen(b->data) - 1);
	*b->lbuf = '\0';
	return CALENDAR_ERROR_NONE;
}

static inline int __cal_vcalendar_make_set_str(cal_make_s *b, const char *s)
{
	int remain_lbuf;
	int remain_str;
	int k;
	int ret;

	remain_lbuf = sizeof(b->lbuf) - _strlen(b->lbuf);
	remain_str = _strlen(s);

	k = 0;
	while ( remain_lbuf - 1 < remain_str) {
		strncat(b->lbuf, s + k, remain_lbuf - 1);
		k += remain_lbuf - 1;
		remain_str -= remain_lbuf - 1;
		ret = __cal_vcalendar_make_folding(b);
		retv_if(CALENDAR_ERROR_NONE != ret, ret);
		remain_lbuf = sizeof(b->lbuf);
	}

	strncat(b->lbuf, s + k, remain_lbuf - 1);
	return CALENDAR_ERROR_NONE;
}

static inline int __cal_vcalendar_make_flush(cal_make_s *b)
{
	int ret;
	ret = __cal_vcalendar_make_alloc(b, _strlen(b->lbuf) + 2);
	retv_if(CALENDAR_ERROR_NONE != ret, ret);

	strncat(b->data, b->lbuf, b->size - _strlen(b->data) - 1);
	strncat(b->data, "\r\n", b->size - _strlen(b->data) - 1);
	*b->lbuf = '\0';
	return CALENDAR_ERROR_NONE;
}

static int __cal_vcalendar_make_printf(cal_make_s *b, const char *s1, const char *s2)
{
	int ret;

	if (s1) {
		ret = __cal_vcalendar_make_set_str(b, s1);
		retv_if(CALENDAR_ERROR_NONE != ret, ret);
	}

	if (s2) {
		ret = __cal_vcalendar_make_set_str(b, s2);
		retv_if(CALENDAR_ERROR_NONE != ret, ret);
	}

	return __cal_vcalendar_make_flush(b);
}

char* _cal_vcalendar_make_get_data(cal_make_s *b)
{
	if (!b || !b->data)
		return NULL;
	return strdup(b->data);
}

void _cal_vcalendar_make_free(cal_make_s **b)
{
	if (!b || !*b)
		return;

	if ((*b)->data)
		free((*b)->data);

	free(*b);
	b = NULL;
}
static void __get_str_utime(long long int t, char *out_str, int size)
{
	int y = 0, m = 0, d = 0;
	int h = 0, n = 0, s = 0;
	_cal_time_get_datetime(t, &y, &m, &d, &h, &n, &s);
	snprintf(out_str, size, ":"VCAL_DATETIME_FORMAT_YYYYMMDDTHHMMSSZ, y, m, d, h, n, s);
}

static int  __cal_vcalendar_make_time(cal_make_s *b, char *tzid, calendar_time_s *t, const char *prop)
{
	retvm_if (NULL == b, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter: b is NULL");
	retvm_if (NULL == t, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter: t is NULL");

	if (CALENDAR_TIME_UTIME == t->type && CALENDAR_TODO_NO_START_DATE == t->time.utime) {
		DBG("No start date");
		return CALENDAR_ERROR_NONE;
	}

	char buf[256] = {0};
	int y = 0, m = 0, d = 0;
	int h = 0, n = 0, s = 0;
	switch (b->version)
	{
	case VCAL_VER_1:
		switch (t->type)
		{
		case CALENDAR_TIME_UTIME:
			_cal_time_get_datetime(t->time.utime, &y, &m, &d, &h, &n, &s);
			snprintf(buf, sizeof(buf), ":"VCAL_DATETIME_FORMAT_YYYYMMDDTHHMMSSZ, y, m, d, h, n, s);
			break;
		case CALENDAR_TIME_LOCALTIME:
			snprintf(buf, sizeof(buf), ":"VCAL_DATETIME_FORMAT_YYYYMMDDTHHMMSS,
					t->time.date.year, t->time.date.month, t->time.date.mday,
					t->time.date.hour, t->time.date.minute, t->time.date.second);
			break;
		}
		break;
	case VCAL_VER_2:
		switch (t->type)
		{
		case CALENDAR_TIME_UTIME:
			if (NULL == tzid || '\0' == *tzid) {
				_cal_time_get_datetime(t->time.utime, &y, &m, &d, &h, &n, &s);
				snprintf(buf, sizeof(buf), ":"VCAL_DATETIME_FORMAT_YYYYMMDDTHHMMSSZ, y, m, d, h, n, s);
			} else {
				_cal_time_get_local_datetime(tzid, t->time.utime, &y, &m, &d, &h, &n, &s);
				snprintf(buf, sizeof(buf), ";TZID=%s:"VCAL_DATETIME_FORMAT_YYYYMMDDTHHMMSS, tzid, y, m, d, h, n, s);
			}
			break;
		case CALENDAR_TIME_LOCALTIME:
			snprintf(buf, sizeof(buf), VCAL_DATETIME_FORMAT_YYYYMMDDTHHMMSS,
					t->time.date.year, t->time.date.month, t->time.date.mday,
					t->time.date.hour, t->time.date.minute, t->time.date.second);
			break;
		}
		break;
	}
	__cal_vcalendar_make_printf(b, prop, buf);
	return CALENDAR_ERROR_NONE;
}

static void __encode_escaped_char(char *p, char **r)
{
	retm_if (NULL == p || '\0' == *p, "Invalid parameter:p is NULL");

	int len = strlen(p) * 2;
	char *q = calloc(len, sizeof(char));
	retm_if (NULL == q, "calloc() is failed");
	*r = q;

	DBG("Before [%s]", p);
	while ('\0' != *p) {
		if ('\r' == *p && '\n' == *(p + 1)) break;
		switch (*p)
		{
		case '\n':
			*q = '\\';
			*(q +1) = 'n';
			q++;
			break;
		case ';':
			*q = '\\';
			*(q +1) = ';';
			q++;
			break;
		case ',':
			*q = '\\';
			*(q +1) = ',';
			q++;
			break;
		case '\\':
			*q = '\\';
			*(q +1) = '\\';
			q++;
			break;
		}
		*q = *p;
		q++;
		p++;
	}
	*q = '\0';
	DBG("After [%s]", *r);
}

static const char* vl_tick(calendar_alarm_time_unit_type_e unit, int tick)
{
	static char buf[32] = {0};

	int i = 0;
	if (tick > 0) {
		*buf = '-';
		i++;

	} else {
		tick = -tick;
	}

	switch (unit) {
	case CALENDAR_ALARM_TIME_UNIT_WEEK:
		snprintf(buf + i, sizeof(buf) - i, "P%dW", tick);
		break;
	case CALENDAR_ALARM_TIME_UNIT_DAY:
		snprintf(buf + i, sizeof(buf) - i, "P%dD", tick);
		break;
	case CALENDAR_ALARM_TIME_UNIT_HOUR:
		snprintf(buf + i, sizeof(buf) - i, "PT%dH", tick);
		break;
	case CALENDAR_ALARM_TIME_UNIT_MINUTE:
		snprintf(buf + i, sizeof(buf) - i, "PT%dM", tick);
		break;
	default:
		buf[0] = '\0';
	}

	return buf;
}

int __cal_vcalendar_make_audio(cal_make_s *b, calendar_record_h alarm)
{
	retvm_if (NULL == b, CALENDAR_ERROR_INVALID_PARAMETER, "b is NULL");
	retvm_if (NULL == alarm, CALENDAR_ERROR_INVALID_PARAMETER, "alarm is NULL");

	int ret = 0;
	int unit = 0;
	ret = calendar_record_get_int(alarm, _calendar_alarm.tick_unit, &unit);
	retvm_if (CALENDAR_ERROR_NONE != ret, ret, "failed to get unit");

	if (CALENDAR_ALARM_TIME_UNIT_SPECIFIC == unit) {
		calendar_time_s at = {0};
		ret = calendar_record_get_caltime(alarm, _calendar_alarm.alarm_time, &at);
		warn_if (CALENDAR_ERROR_NONE != ret, "failed to get alarm_time");

		if (CALENDAR_TIME_UTIME == at.type) {
			char *datetime = _cal_time_convert_ltos(NULL, at.time.utime, 0);
			__cal_vcalendar_make_printf(b, "TRIGGER;VALUE=DATE-TIME:", datetime);
			free(datetime);

		} else {
			char datetime[32] = {0};
			snprintf(datetime, sizeof(datetime), "%04d%02d%02dT%02d%02d%02d",
					at.time.date.year, at.time.date.month, at.time.date.mday,
					at.time.date.hour, at.time.date.minute, at.time.date.second);
			__cal_vcalendar_make_printf(b, "TRIGGER;VALUE=DATE-TIME:", datetime);
		}

	} else {
		int tick = 0;
		ret = calendar_record_get_int(alarm, _calendar_alarm.tick, &tick);
		warn_if (CALENDAR_ERROR_NONE != ret, "failed to get tick");

		__cal_vcalendar_make_printf(b, "TRIGGER:", vl_tick(unit, tick));
	}
	return CALENDAR_ERROR_NONE;
}

/* alarm in ver1.0
 * aalarm: runTime, snoozeTime, repeatCount, audioContent (AALARM;TYPE=WAVE;VALUE=URL:19960415T235959; ; ; file:///mmedia/taps.wav)
 * dalarm: runTime, snoozeTime, repeatCount, displayString
 * malarm: runTime, snoozeTime, repeatCount, addressString, noteString
 * palarm: runTime, snoozeTime, repeatCount, procedureName
 */
static void __cal_vcalendar_make_aalarm(cal_make_s *b, calendar_record_h record, calendar_record_h alarm)
{
	retm_if (NULL == b, "b is NULL");
	retm_if (NULL == record, "record is NULL");
	retm_if (alarm == NULL, "alarm is NULL");

	int ret = CALENDAR_ERROR_NONE;

	// set alarm
	int unit = 0;
	ret = calendar_record_get_int(alarm, _calendar_alarm.tick_unit, &unit);
	retm_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_int() is failed(%d)", ret);

	char datetime[32] = {0};
	if (CALENDAR_ALARM_TIME_UNIT_SPECIFIC == unit) {
		calendar_time_s at = {0};
		ret = calendar_record_get_caltime(alarm, _calendar_alarm.alarm_time, &at);
		warn_if (CALENDAR_ERROR_NONE != ret, "failed to get alarm_time");

		if (CALENDAR_TIME_UTIME == at.type) {
			char *buf = _cal_time_convert_ltos(NULL, at.time.utime, 0);
			snprintf(datetime, sizeof(datetime), "%s;;;", buf);
			free(buf);

		} else {
			snprintf(datetime, sizeof(datetime), "%04d%02d%02dT%02d%02d%02d;;;",
					at.time.date.year, at.time.date.month, at.time.date.mday,
					at.time.date.hour, at.time.date.minute, at.time.date.second);
		}

	} else { // has tick, unit
		int tick = 0;
		ret = calendar_record_get_int(alarm, _calendar_alarm.tick, &tick);
		retm_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_int() is failed(%d)", ret);

		char *uri = NULL;
		ret = calendar_record_get_uri_p(record, &uri);
		retm_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_uri_p() failed(%d)", ret);

		calendar_time_s st = {0};
		if (!strncmp(uri, _calendar_event._uri, strlen(_calendar_event._uri))) {
			ret = calendar_record_get_caltime(record, _calendar_event.start_time, &st);
			retm_if (CALENDAR_ERROR_NONE != ret, "Failed to get start");

		} else if (!strncmp(uri, _calendar_todo._uri, strlen(_calendar_todo._uri))) {
			ret = calendar_record_get_caltime(record, _calendar_todo.due_time, &st);
			retm_if (CALENDAR_ERROR_NONE != ret, "Failed to get due");
		}

		if (CALENDAR_TIME_UTIME == st.type) {
			long long int lli = st.time.utime - (tick * unit);
			char *buf = _cal_time_convert_ltos(NULL, lli, 0);
			snprintf(datetime, sizeof(datetime), "%s;;;", buf);
			free(buf);
			DBG("aalarm: [%s] = (%lld) - (tick(%d) * unit(%d))", datetime, st.time.utime, tick, unit);

		} else {
			struct tm tm = {0};
			tm.tm_year = st.time.date.year - 1900;
			tm.tm_mon = st.time.date.month - 1;
			tm.tm_mday = st.time.date.mday;
			tm.tm_hour = st.time.date.hour;
			tm.tm_min = st.time.date.minute;
			tm.tm_sec = st.time.date.second;
			time_t tt = mktime(&tm);
			tt -= (tick * unit);
			localtime_r(&tt, &tm);

			snprintf(datetime, sizeof(datetime), "%04d%02d%02dT%02d%02d%02d",
					tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
					tm.tm_hour, tm.tm_min, tm.tm_sec);

			DBG("aalarm: [%s]", datetime);
		}
	}
	__cal_vcalendar_make_printf(b, "AALARM:", datetime);
}

static void __cal_vcalendar_make_alarm(cal_make_s *b, calendar_record_h alarm)
{
	retm_if (NULL == b, "Invalid parameter: cal_make_s is NULL");
	retm_if (NULL == alarm, "Invalid parameter: record is NULL");

	int ret;

	// TODO : No action type is defined
	ret = __cal_vcalendar_make_printf(b, "BEGIN:VALARM", NULL);
	retm_if(CALENDAR_ERROR_NONE != ret, "__cal_vcalendar_make_printf() is failed(%d)", ret);

	ret = __cal_vcalendar_make_audio(b, alarm);
	retm_if(CALENDAR_ERROR_NONE != ret, "__cal_vcalendar_make_audio() is failed(%d)", ret);

	int action = 0;
	ret = calendar_record_get_int(alarm, _calendar_alarm.action, &action);
	retm_if(CALENDAR_ERROR_NONE != ret, "calendar_record_get_int() is failed(%d)", ret);
	switch (action)
	{
	default:
	case CALENDAR_ALARM_ACTION_AUDIO:
		ret = __cal_vcalendar_make_printf(b, "ACTION:", "AUDIO");
		break;
	case CALENDAR_ALARM_ACTION_DISPLAY:
		ret = __cal_vcalendar_make_printf(b, "ACTION:", "DISPLAY");
		break;
	case CALENDAR_ALARM_ACTION_EMAIL:
		ret = __cal_vcalendar_make_printf(b, "ACTION:", "EMAIL");
		break;
	}
	retm_if(CALENDAR_ERROR_NONE != ret, "__cal_vcalendar_make_printf() is failed(%d)", ret);

	char *summary = NULL;
	ret = calendar_record_get_str_p(alarm, _calendar_alarm.summary, &summary);
	if (summary && *summary) {
		ret = __cal_vcalendar_make_printf(b, "SUMMARY:", summary);
		retm_if(CALENDAR_ERROR_NONE != ret, "__cal_vcalendar_make_printf() is failed(%d)", ret);
	}

	char *description = NULL;
	ret = calendar_record_get_str_p(alarm, _calendar_alarm.description, &description);
	if (description && *description) {
		ret = __cal_vcalendar_make_printf(b, "DESCRIPTION:", description);
		retm_if(CALENDAR_ERROR_NONE != ret, "__cal_vcalendar_make_printf() is failed(%d)", ret);
	}

	__cal_vcalendar_make_printf(b, "END:VALARM", NULL);
}

int __cal_vcalendar_make_rrule_append_mday(char *buf, char *mday)
{
	int i;
	int num;
	int length = 0;
	char **t = NULL;
	char *p = NULL;

	if (NULL == buf || NULL == mday)
	{
		ERR("Invalid parameter");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	t = g_strsplit_set(mday, " ,", -1);
	if (!t) {
		ERR("g_strsplit_set() is failed");
		g_strfreev(t);
		return CALENDAR_ERROR_OUT_OF_MEMORY;
	}
	length = g_strv_length(t);
	for (i = 0; i < length; i++)
	{
		if (*t[i] == '\0') continue;

		p = t[i];
		num = atoi(p);
		if (num > 0)
		{
			strcat(buf, p);
		}
		else
		{
			strcat(buf, p + 1);
			strcat(buf, "-");
		}
		strcat(buf, " ");
	}
	g_strfreev(t);

	return CALENDAR_ERROR_NONE;
}

static void __cal_vcalendar_make_rrule_append_setpos(calendar_record_h record, char *buf)
{
	int ret = CALENDAR_ERROR_NONE;
	char *bysetpos = NULL;

	ret = calendar_record_get_str_p(record, _calendar_event.bysetpos, &bysetpos);
	retm_if(CALENDAR_ERROR_NONE != ret, "calendar_record_get_int() failed(%d)", ret);

	if (bysetpos && '\0' != bysetpos[0]) {
		// in ver1.0, "3, 5, -4" -> "3+ 5+ 4-"
		char **t = NULL;
		t = g_strsplit_set(bysetpos, " ,", -1);

		int len = g_strv_length(t);

		int i;
		for (i = 0; i < len; i++) {
			if (*t[i] == '\0') continue;

			int setpos = atoi(t[i]);
			DBG("%d", setpos);

			if (setpos == 0) {
				DBG("Invalid setpos(0)");
				continue;
			}
			char tmp[32] = {0};
			snprintf(tmp, sizeof(tmp), "%d%s ", setpos * (setpos < 0 ? -1 : 1), setpos < 0 ? "-" : "+");
			strcat(buf, tmp);
		}
		g_strfreev(t);
	} else {
		// in ver2.0, 3TH should be changed to setpos:3, byday:TH
		char *byday = NULL;
		ret = calendar_record_get_str_p(record, _calendar_event.byday, &byday);
		retm_if(CALENDAR_ERROR_NONE != ret, "calendar_record_get_int() failed(%d)", ret);
		if (NULL == byday || '\0' == byday[0])
		{
			strcat(buf, "1+ ");
			return;
		}

		int len = strlen(byday);

		bool is_working = true;
		bool is_extracted = false;
		int i = 0;
		int digit = 0;
		int sign = 0;
		while (i <= len) {
			// extract -1, 1 from 1SU,-1SU
			if ((byday[i] >= '1' && byday[i] <= '9')) {
				is_working = false;
				digit++;
			} else if ('+' == byday[i]) {
				is_working = false;
				sign = 1;
			} else if ('-' == byday[i]) {
				is_working = false;
				sign = -1;
			} else {
				if (false == is_working) {
					is_extracted = true;
					is_working = true;
					char num[32] = {0};
					snprintf(num, digit +1, "%s", byday +i -digit);
					strcat(num, (-1 == sign) ? "- " : "+ "); // get '3'
					if (NULL == strstr(buf, num)) {
						strcat(buf, num);
					}
					digit = 0;
					sign = 0;
				}
			}
			i++;
		}
		if (false == is_extracted) {
			strcat(buf, "1+ ");
			return;
		}
	}
}

void __cal_vcalendar_make_rrule_append_text_wday(int rrule_type, char *buf, char *wday)
{
	int i, j;
	int length = 0;
	char **t = NULL;
	char *p = NULL;

	t = g_strsplit_set(wday, " ,", -1);
	if (!t) {
		ERR("g_strsplit_set() is failed");
		g_strfreev(t);
		return;
	}
	length = g_strv_length(t);
	for (i = 0; i < length; i++)
	{
		if (*t[i] == '\0')
			continue;

		p = t[i];
		while (*p == ' ') // del space
			p++;

		j = 0;
		while (p[j] == '+' || p[j] == '-' || (p[j] >= '1' && p[j] <= '9')) // get number length
			j++;
		if (strstr(buf, p +j)) // already appended
			continue;
		strcat(buf, p + j);
		strcat(buf, " ");
	}
	g_strfreev(t);

	return;
}

void __cal_vcalendar_make_rrule_append_wkst(char *buf, calendar_record_h record)
{
	const char wday[7][3] = {"SU", "MO", "TU", "WE", "TH", "FR", "SA"};
	int wkst = 0;
	calendar_record_get_int(record, _calendar_event.wkst, &wkst);
	if (wkst < CALENDAR_SUNDAY || wkst > CALENDAR_SATURDAY)
		return;

	DBG("wkst(%d) [%s]", wkst, wday[wkst - 1]);
	strcat(buf, "WKST=");
	strcat(buf, wday[wkst - 1]); // CALENDAR_SUNDAY is 1 not 0
	strcat(buf, " ");
}

int __cal_vcalendar_make_rrule_append_wday(int rrule_type, char *buf, char *wday)
{
	int i, j;
	int num = 0, num_past;
	int length = 0;
	char **t = NULL;
	char *p = NULL;
	char buf_temp[8] = {0};

	if (NULL == buf || NULL == wday) {
		ERR("Invalid parameter");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	num_past = 0;
	t = g_strsplit_set(wday, " ,", -1);
	if (!t) {
		ERR("g_strsplit_set() is failed");
		g_strfreev(t);
		return CALENDAR_ERROR_OUT_OF_MEMORY;
	}
	length = g_strv_length(t);
	DBG("len(%d)", length);

	for (i = 0; i < length; i++)
	{
		if (*t[i] == '\0') continue;

		p = t[i];
		j = 0; // get number
		while (p[j] == '+' || p[j] == '-' || (p[j] >= '1' && p[j] <= '9'))
			j++;

		if (j > 0)
		{
			if (CALENDAR_RECURRENCE_WEEKLY == rrule_type)
			{
				num_past = num;
			}
			else
			{
				if (*p == '-')
				{
					snprintf(buf_temp, j + 1, "%s", p + 1);
					num = atoi(buf_temp);
					if (0 == i)
					{
						num_past = num;
						strcat(buf, buf_temp);
						strcat(buf, "-");
						strcat(buf, " ");
					}
				}
				else
				{
					snprintf(buf_temp, j + 1, "%s", p);
					num = atoi(buf_temp);
					if (0 == i)
					{
						num_past = num;
						strcat(buf, buf_temp);
						strcat(buf, "+");
						strcat(buf, " ");
					}
				}
			}
			if (num_past == num)
			{
				strcat(buf, p + j);
				strcat(buf, " ");
			}
			else
			{
				ERR("Out of 1.0 spec");
			}
			DBG("%d num(%d) val[%s]", i, num, p + j);
		}
		else
		{
			strcat(buf, p);
			strcat(buf, " ");
		}
		DBG("[%s]", buf);
	}
	g_strfreev(t);

	return CALENDAR_ERROR_NONE;
}

static void __make_begin(cal_make_s *b, calendar_record_h record)
{
	retm_if (NULL == b, "Invalid parameter: cal_make_s is NULL");
	retm_if (NULL == record, "Invalid parameter: record is NULL");

	int ret = 0;
	switch (b->type)
	{
	case CALENDAR_BOOK_TYPE_EVENT:
		ret = __cal_vcalendar_make_printf(b, "BEGIN:VEVENT", NULL);
		retm_if(CALENDAR_ERROR_NONE != ret, "__cal_vcalendar_make_printf() is failed(%d)", ret);
		break;
	case CALENDAR_BOOK_TYPE_TODO:
		ret = __cal_vcalendar_make_printf(b, "BEGIN:VTODO", NULL);
		retm_if (CALENDAR_ERROR_NONE != ret, "__cal_vcalendar_make_printf() is failed(%d)", ret);
		break;
	}
}

static void __make_dtstart(cal_make_s *b, calendar_record_h record)
{
	retm_if (NULL == b, "Invalid parameter: cal_make_s is NULL");
	retm_if (NULL == record, "Invalid parameter: record is NULL");

	int ret = 0;
	char *tzid = NULL;
	calendar_time_s ct = {0};
	switch (b->type)
	{
	case CALENDAR_BOOK_TYPE_EVENT:
		ret = calendar_record_get_str_p(record, _calendar_event.start_tzid, &tzid);
		retm_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_str_p() is failed(%d)", ret);
		ret = calendar_record_get_caltime(record, _calendar_event.start_time, &ct);
		retm_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_caltime() is failed(%d)", ret);
		break;
	case CALENDAR_BOOK_TYPE_TODO:
		ret = calendar_record_get_str_p(record, _calendar_todo.start_tzid, &tzid);
		retm_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_str_p() is failed(%d)", ret);
		ret = calendar_record_get_caltime(record, _calendar_todo.start_time, &ct);
		retm_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_caltime() is failed(%d)", ret);
		break;
	}
	if (tzid && *tzid) {
		__cal_vcalendar_make_time(b, tzid, &ct, "DTSTART");
	}
}

static void __make_dtend(cal_make_s *b, calendar_record_h record)
{
	retm_if (NULL == b, "Invalid parameter: cal_make_s is NULL");
	retm_if (NULL == record, "Invalid parameter: record is NULL");

	int ret = 0;
	char *tzid = NULL;
	calendar_time_s ct = {0};
	switch (b->type)
	{
	case CALENDAR_BOOK_TYPE_EVENT:
		ret = calendar_record_get_str_p(record, _calendar_event.end_tzid, &tzid);
		retm_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_str_p() is failed(%d)", ret);
		ret = calendar_record_get_caltime(record, _calendar_event.end_time, &ct);
		retm_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_caltime() is failed(%d)", ret);
		if (tzid && *tzid) {
			__cal_vcalendar_make_time(b, tzid, &ct, "DTEND");
		}
		break;
	case CALENDAR_BOOK_TYPE_TODO:
		ret = calendar_record_get_str_p(record, _calendar_todo.due_tzid, &tzid);
		retm_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_str_p() is failed(%d)", ret);
		ret = calendar_record_get_caltime(record, _calendar_todo.due_time, &ct);
		retm_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_caltime() is failed(%d)", ret);
		if (tzid && *tzid) {
			__cal_vcalendar_make_time(b, tzid, &ct, "DUE");
		}
		break;
	}
}

static void __make_sensitivity(cal_make_s *b, calendar_record_h record)
{
	retm_if (NULL == b, "Invalid parameter: cal_make_s is NULL");
	retm_if (NULL == record, "Invalid parameter: record is NULL");

	int ret = 0;
	int value = 0;
	switch (b->type)
	{
	case CALENDAR_BOOK_TYPE_EVENT:
		ret = calendar_record_get_int(record, _calendar_event.sensitivity, &value);
		retm_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_int() is failed(%d)", ret);
		break;
	case CALENDAR_BOOK_TYPE_TODO:
		ret = calendar_record_get_int(record, _calendar_todo.sensitivity, &value);
		retm_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_int() is failed(%d)", ret);
		break;
	}
	char *sensitivity = NULL;
	switch (value)
	{
	case CALENDAR_SENSITIVITY_PUBLIC:
		sensitivity = "PUBLIC";
		break;
	case CALENDAR_SENSITIVITY_PRIVATE:
		sensitivity = "PRIVATE";
		break;
	case CALENDAR_SENSITIVITY_CONFIDENTIAL:
		sensitivity = "CONFIDENTIAL";
		break;
	default:
		ERR("Invalid sensitivity(%d)", value);
		return;
	}
	__cal_vcalendar_make_printf(b, "CLASS:", sensitivity);
}

int __cal_vcalendar_make_rrule_append_until(char *buf, calendar_record_h record)
{
	int ret;
	int range_type = 0;
	int count;
	char *until_str = NULL;
	char buf_range[256] = {0};
	calendar_time_s caltime;

	ret = calendar_record_get_int(record, _calendar_event.range_type, &range_type);
	if (CALENDAR_ERROR_NONE != ret)
	{
		ERR("calendar_record_get_int() failed");
		return ret;
	}

	switch (range_type)
	{
	case CALENDAR_RANGE_COUNT:
		ret = calendar_record_get_int(record, _calendar_event.count, &count);
		if (CALENDAR_ERROR_NONE != ret)
		{
			ERR("calendar_record_get_int() failed");
			return ret;
		}
		snprintf(buf_range, sizeof(buf_range), "#%d", count);
		break;

	case CALENDAR_RANGE_UNTIL:
		memset(&caltime, 0x0, sizeof(calendar_time_s));

		ret = calendar_record_get_caltime(record, _calendar_event.until_time, &caltime);
		if (CALENDAR_ERROR_NONE != ret)
		{
			ERR("calendar_record_get_caltime() failed");
			return ret;
		}
		switch (caltime.type)
		{
		case CALENDAR_TIME_UTIME:
			until_str = _cal_time_convert_ltos(NULL, caltime.time.utime, 0);
			snprintf(buf_range, sizeof(buf_range), "%s", until_str);
			CAL_FREE(until_str);
			break;

		case CALENDAR_TIME_LOCALTIME:
			snprintf(buf_range, sizeof(buf_range), "%04d%02d%02dT%02d%02d%02d",
					caltime.time.date.year, caltime.time.date.month, caltime.time.date.mday,
					caltime.time.date.hour, caltime.time.date.minute, caltime.time.date.second);
			break;
		}
		break;

	case CALENDAR_RANGE_NONE:
		snprintf(buf_range, sizeof(buf_range), "#0");
		break;
	}
	strcat(buf, buf_range);

	return CALENDAR_ERROR_NONE;
}
static void __make_rrule_ver1_default(calendar_record_h record, int freq, int interval, char *buf, int buf_size)
{
	retm_if (NULL == record, "Invalid parameter: record is NULL");

	int ret = 0;
	calendar_time_s caltime = {0};
	ret = calendar_record_get_caltime(record, _calendar_event.start_time, &caltime);
	warn_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_caltime() is failed(%d)", ret);

	switch (freq)
	{
	case CALENDAR_RECURRENCE_YEARLY:
		snprintf(buf, buf_size, "YD%d ", interval);
		break;
	case CALENDAR_RECURRENCE_MONTHLY:
		snprintf(buf, buf_size, "MD%d ", interval);
		break;
	}

	char *tzid = NULL;
	int d = 0;
	switch (caltime.type)
	{
	case CALENDAR_TIME_UTIME:
		calendar_record_get_str_p(record, _calendar_event.start_tzid, &tzid);
		_cal_time_get_local_datetime(tzid, caltime.time.utime, NULL, NULL, &d, NULL, NULL, NULL);
		break;
	case CALENDAR_TIME_LOCALTIME:
		d = caltime.time.date.mday;
		break;
	}
	char mday[32] = {0};
	snprintf(mday, sizeof(mday), "%d", d);
	__cal_vcalendar_make_rrule_append_mday(buf, mday);
}

static void __make_rrule_ver1(cal_make_s *b, calendar_record_h record)
{
	retm_if (NULL == b, "Invalid parameter: cal_make_s is NULL");
	retm_if (NULL == record, "Invalid parameter: record is NULL");

	int ret = 0;
	char buf[1024] = {0};

	int freq = 0;
	ret = calendar_record_get_int(record, _calendar_event.freq, &freq);
	retm_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_int() is failed(%d)", ret);

	int interval = 1;
	ret = calendar_record_get_int(record, _calendar_event.interval, &interval);
	retm_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_int() is failed(%d)", ret);
	interval = interval > 0 ? interval : 1;

	char *byyearday = NULL;
	ret = calendar_record_get_str_p(record, _calendar_event.byyearday, &byyearday);
	retm_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_str_p() is failed(%d)", ret);

	char *bymonth = NULL;
	ret = calendar_record_get_str_p(record, _calendar_event.bymonth, &bymonth);
	retm_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_str_p() is failed(%d)", ret);

	char *byday = NULL;
	ret = calendar_record_get_str_p(record, _calendar_event.byday, &byday);
	retm_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_str_p() is failed(%d)", ret);

	char *bymonthday = NULL;
	ret = calendar_record_get_str_p(record, _calendar_event.bymonthday, &bymonthday);
	retm_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_str_p() is failed(%d)", ret);

	switch (freq)
	{
	case CALENDAR_RECURRENCE_YEARLY:
		if (bymonth && *bymonth) {
			snprintf(buf, sizeof(buf), "YM%d ", interval);
			__cal_vcalendar_make_rrule_append_mday(buf, bymonth);

			char tmp[512] = {0};
			if (byday && *byday) {
				// ex> YM1 6 MP1 1+ TH
				snprintf(tmp, sizeof(tmp), "MP%d ", interval);
				strcat(buf, tmp);
				__cal_vcalendar_make_rrule_append_setpos(record, buf);
				__cal_vcalendar_make_rrule_append_text_wday(CALENDAR_RECURRENCE_MONTHLY, buf, byday);
			} else if (bymonthday && *bymonthday) {
				// ex> YM1 2 MD 1
				snprintf(tmp, sizeof(tmp), "MD%d ", interval);
				strcat(buf, tmp);
				__cal_vcalendar_make_rrule_append_mday(buf, bymonthday);
			} else {
				ERR("Out of scope");
				__make_rrule_ver1_default(record, freq, interval, buf, sizeof(buf));
			}
		} else if (byyearday && *byyearday) {
			snprintf(buf, sizeof(buf), "YD%d ", interval);
			__cal_vcalendar_make_rrule_append_mday(buf, byyearday);
		} else {
			ERR("Out of scope");
			__make_rrule_ver1_default(record, freq, interval, buf, sizeof(buf));
		}
		break;

	case CALENDAR_RECURRENCE_MONTHLY:
		if (byday && *byday) {
			snprintf(buf, sizeof(buf), "MP%d ", interval);
			__cal_vcalendar_make_rrule_append_setpos(record, buf);
			__cal_vcalendar_make_rrule_append_text_wday(CALENDAR_RECURRENCE_MONTHLY, buf, byday);
		} else if (bymonthday && *bymonthday) {
			snprintf(buf, sizeof(buf), "MD%d ", interval);
			__cal_vcalendar_make_rrule_append_mday(buf, bymonthday);
		} else {
			ERR("Out of scope, so set as bymonthday");
			__make_rrule_ver1_default(record, freq, interval, buf, sizeof(buf));
		}
		break;

	case CALENDAR_RECURRENCE_WEEKLY:
		snprintf(buf, sizeof(buf), "W%d ", interval);
		__cal_vcalendar_make_rrule_append_wday(CALENDAR_RECURRENCE_WEEKLY, buf, byday);
		break;

	case CALENDAR_RECURRENCE_DAILY:
		snprintf(buf, sizeof(buf), "D%d ", interval);
		break;

	default:
		ERR("Out of scope");
		break;
	}

	if (*buf) {
		__cal_vcalendar_make_rrule_append_wkst(buf, record);
		__cal_vcalendar_make_rrule_append_until(buf, record);
		__cal_vcalendar_make_printf(b, "RRULE:", buf);
	}
}

static void __make_rrule_ver2(cal_make_s *b, calendar_record_h record)
{
	retm_if (NULL == b, "Invalid parameter: cal_make_s is NULL");
	retm_if (NULL == record, "Invalid parameter: record is NULL");

	int ret = 0;
	char *text = NULL;
	char tmp[32] = {0};
	calendar_time_s caltime = {0};

	int freq = 0;
	ret = calendar_record_get_int(record, _calendar_event.freq, &freq);
	retm_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_int() is failed(%d)", ret);

	switch (freq) {
	case CALENDAR_RECURRENCE_DAILY:
		__cal_vcalendar_make_set_str(b, "RRULE:FREQ=DAILY");
		break;
	case CALENDAR_RECURRENCE_WEEKLY:
		__cal_vcalendar_make_set_str(b, "RRULE:FREQ=WEEKLY");
		break;
	case CALENDAR_RECURRENCE_MONTHLY:
		__cal_vcalendar_make_set_str(b, "RRULE:FREQ=MONTHLY");
		break;
	case CALENDAR_RECURRENCE_YEARLY:
		__cal_vcalendar_make_set_str(b, "RRULE:FREQ=YEARLY");
		break;
	default:
		return;
	}

	int interval = 1;
	ret = calendar_record_get_int(record, _calendar_event.interval, &interval);
	warn_if(CALENDAR_ERROR_NONE != ret, "calendar_record_get_int() Failed(%d)", ret);
	interval = interval > 0 ? interval : 1;
	snprintf(tmp, sizeof(tmp), ";INTERVAL=%d", interval);
	__cal_vcalendar_make_set_str(b, tmp);

	ret = calendar_record_get_str_p(record, _calendar_event.bysecond, &text);
	warn_if(CALENDAR_ERROR_NONE != ret, "calendar_record_get_str_p() Failed(%d)", ret);
	if (text && *text) {
		DBG("BYSECOND= [%s]", text);
		__cal_vcalendar_make_set_str(b, ";BYSECOND=");
		__cal_vcalendar_make_set_str(b, text);
		text = NULL;
	}

	ret = calendar_record_get_str_p(record, _calendar_event.byminute, &text);
	warn_if(CALENDAR_ERROR_NONE != ret, "calendar_record_get_str_p() Failed(%d)", ret);
	if (text && *text) {
		DBG("BYMINUTE= [%s]", text);
		__cal_vcalendar_make_set_str(b, ";BYMINUTE=");
		__cal_vcalendar_make_set_str(b, text);
		text = NULL;
	}

	ret = calendar_record_get_str_p(record, _calendar_event.byhour, &text);
	warn_if(CALENDAR_ERROR_NONE != ret, "calendar_record_get_str_p() Failed(%d)", ret);
	if (text && *text) {
		DBG("BYHOUR= [%s]", text);
		__cal_vcalendar_make_set_str(b, ";BYHOUR=");
		__cal_vcalendar_make_set_str(b, text);
		text = NULL;
	}

	ret = calendar_record_get_str_p(record, _calendar_event.byday, &text);
	warn_if(CALENDAR_ERROR_NONE != ret, "calendar_record_get_str_p() Failed(%d)", ret);
	if (text && *text) {
		DBG("BYDAY= [%s]", text);
		__cal_vcalendar_make_set_str(b, ";BYDAY=");
		__cal_vcalendar_make_set_str(b, text);
		text = NULL;
	}

	ret = calendar_record_get_str_p(record, _calendar_event.bymonthday, &text);
	warn_if(CALENDAR_ERROR_NONE != ret, "calendar_record_get_str_p() Failed(%d)", ret);
	if (text && *text) {
		DBG("BYMONTHDAY= [%s]", text);
		__cal_vcalendar_make_set_str(b, ";BYMONTHDAY=");
		__cal_vcalendar_make_set_str(b, text);
		text = NULL;
	}

	ret = calendar_record_get_str_p(record, _calendar_event.byyearday, &text);
	warn_if(CALENDAR_ERROR_NONE != ret, "calendar_record_get_str_p() Failed(%d)", ret);
	if (text && *text) {
		DBG("BYYEARDAY= [%s]", text);
		__cal_vcalendar_make_set_str(b, ";BYYEARDAY=");
		__cal_vcalendar_make_set_str(b, text);
		text = NULL;
	}

	ret = calendar_record_get_str_p(record, _calendar_event.byweekno, &text);
	warn_if(CALENDAR_ERROR_NONE != ret, "calendar_record_get_str_p() Failed(%d)", ret);
	if (text && *text) {
		DBG("BYWEEKNO= [%s]", text);
		__cal_vcalendar_make_set_str(b, ";BYWEEKNO=");
		__cal_vcalendar_make_set_str(b, text);
		text = NULL;
	}

	ret = calendar_record_get_str_p(record, _calendar_event.bymonth, &text);
	warn_if(CALENDAR_ERROR_NONE != ret, "calendar_record_get_str_p() Failed(%d)", ret);
	if (text && *text) {
		DBG("BYMONTH= [%s]", text);
		__cal_vcalendar_make_set_str(b, ";BYMONTH=");
		__cal_vcalendar_make_set_str(b, text);
		text = NULL;
	}

	ret = calendar_record_get_str_p(record, _calendar_event.bysetpos, &text);
	warn_if(CALENDAR_ERROR_NONE != ret, "calendar_record_get_str_p() Failed(%d)", ret);
	if (text && *text) {
		DBG("BYSETPOS= [%s]", text);
		__cal_vcalendar_make_set_str(b, ";BYSETPOS=");
		__cal_vcalendar_make_set_str(b, text);
		text = NULL;
	}

	int wkst = 0;
	ret = calendar_record_get_int(record, _calendar_event.wkst, &wkst);
	warn_if(CALENDAR_ERROR_NONE != ret, "calendar_record_get_int() Failed(%d)", ret);
	if (wkst >= CALENDAR_SUNDAY && wkst <= CALENDAR_SATURDAY) {
		__cal_vcalendar_make_set_str(b, ";WKST=");
		switch (wkst) {
		case CALENDAR_SUNDAY:
			__cal_vcalendar_make_set_str(b, "SU");
			break;
		case CALENDAR_MONDAY:
			__cal_vcalendar_make_set_str(b, "MO");
			break;
		case CALENDAR_TUESDAY:
			__cal_vcalendar_make_set_str(b, "TU");
			break;
		case CALENDAR_WEDNESDAY:
			__cal_vcalendar_make_set_str(b, "WE");
			break;
		case CALENDAR_THURSDAY:
			__cal_vcalendar_make_set_str(b, "TH");
			break;
		case CALENDAR_FRIDAY:
			__cal_vcalendar_make_set_str(b, "FR");
			break;
		case CALENDAR_SATURDAY:
			__cal_vcalendar_make_set_str(b, "SA");
			break;
		}
	}

	int range_type = 0;
	int count = 0;
	ret = calendar_record_get_int(record, _calendar_event.range_type, &range_type);
	retm_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_int() Failed(%d)", ret);
	switch (range_type) {
	case CALENDAR_RANGE_COUNT:
		ret = calendar_record_get_int(record, _calendar_event.count, &count);
		warn_if(CALENDAR_ERROR_NONE != ret, "calendar_record_get_int() Failed(%d)", ret);
		snprintf(tmp, sizeof(tmp), ";COUNT=%d", count);
		__cal_vcalendar_make_set_str(b, tmp);
		break;

	case CALENDAR_RANGE_UNTIL:
		ret = calendar_record_get_caltime(record, _calendar_event.until_time, &caltime);
		warn_if(CALENDAR_ERROR_NONE != ret, "calendar_record_get_caltime() Failed(%d)", ret);

		if (caltime.type == CALENDAR_TIME_UTIME) {
			char *tmp_tzid = NULL;
			tmp_tzid = _cal_time_convert_ltos(NULL, caltime.time.utime, 0);
			if (tmp_tzid) {
				snprintf(tmp, sizeof(tmp), ";UNTIL=%s", tmp_tzid);
				CAL_FREE(tmp_tzid);
			}
		} else {
			snprintf(tmp, sizeof(tmp), ";UNTIL=%04d%02d%02dT%02d%02d%02dZ",
					caltime.time.date.year,
					caltime.time.date.month,
					caltime.time.date.mday,
					caltime.time.date.hour,
					caltime.time.date.minute,
					caltime.time.date.second);
		}
		__cal_vcalendar_make_set_str(b, tmp);
		break;

	case CALENDAR_RANGE_NONE:
		break;
	}
	__cal_vcalendar_make_flush(b);
}

static void __make_rrule(cal_make_s *b, calendar_record_h record)
{
	retm_if (NULL == b, "Invalid parameter: cal_make_s is NULL");
	retm_if (NULL == record, "Invalid parameter: record is NULL");

	switch (b->type)
	{
	case CALENDAR_BOOK_TYPE_EVENT:
		switch (b->version)
		{
		case VCAL_VER_1:
			__make_rrule_ver1(b, record);
			break;
		case VCAL_VER_2:
			__make_rrule_ver2(b, record);
			break;
		}
		break;
	case CALENDAR_BOOK_TYPE_TODO:
		ERR("No rrule in todo");
		return;
	}
}

int __cal_vcalendar_make_attendee(cal_make_s *b, calendar_record_h attendee)
{
	int ret;

	retvm_if(attendee == NULL, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid argument: attendee is NULL");

	ret = __cal_vcalendar_make_set_str(b, "ATTENDEE");
	retv_if(CALENDAR_ERROR_NONE != ret, ret);

	int cutype = 0;
	ret = calendar_record_get_int(attendee, _calendar_attendee.cutype, &cutype);
	ret = __cal_vcalendar_make_set_str(b, ";CUTYPE=");
	switch (cutype)
	{
	case CALENDAR_ATTENDEE_CUTYPE_INDIVIDUAL:
		ret = __cal_vcalendar_make_set_str(b, "INDIVIDUAL");
		break;
	case CALENDAR_ATTENDEE_CUTYPE_GROUP:
		ret = __cal_vcalendar_make_set_str(b, "GROUP");
		break;
	case CALENDAR_ATTENDEE_CUTYPE_RESOURCE:
		ret = __cal_vcalendar_make_set_str(b, "RESOURCE");
		break;
	case CALENDAR_ATTENDEE_CUTYPE_ROOM:
		ret = __cal_vcalendar_make_set_str(b, "ROOM");
		break;
	case CALENDAR_ATTENDEE_CUTYPE_UNKNOWN:
		ret = __cal_vcalendar_make_set_str(b, "UNKNOWN");
		break;
	}
	retv_if(CALENDAR_ERROR_NONE != ret, ret);

	char *member = NULL;
	ret = calendar_record_get_str_p(attendee, _calendar_attendee.member, &member);
	if (member && *member) {
		ret = __cal_vcalendar_make_set_str(b, ";MEMBER=");
		ret = __cal_vcalendar_make_set_str(b, member);
		retv_if(CALENDAR_ERROR_NONE != ret, ret);
	}

	int role = 0;
	ret = calendar_record_get_int(attendee, _calendar_attendee.role, &role);
	{
		ret = __cal_vcalendar_make_set_str(b, ";ROLE=");
		ret = __cal_vcalendar_make_set_str(b, _att_role[role]);
		retv_if(CALENDAR_ERROR_NONE != ret, ret);
	}

	int status = 0;
	ret = calendar_record_get_int(attendee, _calendar_attendee.status, &status);
	{
		ret = __cal_vcalendar_make_set_str(b, ";PARTSTAT=");
		ret = __cal_vcalendar_make_set_str(b, _att_st[status]);
		retv_if(CALENDAR_ERROR_NONE != ret, ret);
	}

	int rsvp = 0;
	ret = calendar_record_get_int(attendee, _calendar_attendee.rsvp, &rsvp);
	{
		ret = __cal_vcalendar_make_set_str(b, ";RSVP=");
		ret = __cal_vcalendar_make_set_str(b, rsvp ? "TRUE" : "FALSE");
		retv_if(CALENDAR_ERROR_NONE != ret, ret);
	}

	char *delegatee_uri = NULL;
	ret = calendar_record_get_str_p(attendee, _calendar_attendee.delegatee_uri, &delegatee_uri);
	if (delegatee_uri && *delegatee_uri)
	{
		ret = __cal_vcalendar_make_set_str(b, ";DELEGATED-TO=");
		ret = __cal_vcalendar_make_set_str(b, delegatee_uri);
		retv_if(CALENDAR_ERROR_NONE != ret, ret);
	}

	char *delegator_uri = NULL;
	ret = calendar_record_get_str_p(attendee, _calendar_attendee.delegator_uri, &delegator_uri);
	if (delegator_uri && *delegator_uri)
	{
		ret = __cal_vcalendar_make_set_str(b, ";DELEGATED-FROM=");
		ret = __cal_vcalendar_make_set_str(b, delegator_uri);
		retv_if(CALENDAR_ERROR_NONE != ret, ret);
	}

	// TODO : No 'sentby' member in cal_participant_info_t

	char *name = NULL;
	ret = calendar_record_get_str_p(attendee, _calendar_attendee.name, &name);
	if (name && *name) {
		ret = __cal_vcalendar_make_set_str(b, ";CN=");
		ret = __cal_vcalendar_make_set_str(b, name);
		retv_if(CALENDAR_ERROR_NONE != ret, ret);
	}

	char *email = NULL;
	ret = calendar_record_get_str_p(attendee, _calendar_attendee.email, &email);
	if (email && *email)
	{
		ret = __cal_vcalendar_make_set_str(b, ":mailto:");
		ret = __cal_vcalendar_make_set_str(b, email);
		retv_if(CALENDAR_ERROR_NONE != ret, ret);
	}

	__cal_vcalendar_make_flush(b);
	return CALENDAR_ERROR_NONE;
}

static void __make_attendee(cal_make_s *b, calendar_record_h record)
{
	retm_if (NULL == b, "Invalid parameter: cal_make_s is NULL");
	retm_if (NULL == record, "Invalid parameter: record is NULL");

	int ret = 0;
	unsigned int count = 0;
	int i;
	switch (b->type)
	{
	case CALENDAR_BOOK_TYPE_EVENT:
		ret = calendar_record_get_child_record_count(record, _calendar_event.calendar_attendee, &count);
		retm_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_child_record_count() is failed(%d)", ret);
		for (i = 0; i < count; i++) {
			calendar_record_h child = NULL;
			ret = calendar_record_get_child_record_at_p(record, _calendar_event.calendar_attendee, i, &child);
			ret = __cal_vcalendar_make_attendee(b, child);
		}
		break;
	case CALENDAR_BOOK_TYPE_TODO:
		ret = calendar_record_get_child_record_count(record, _calendar_todo.calendar_attendee, &count);
		retm_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_child_record_count() is failed(%d)", ret);
		for (i = 0; i < count; i++) {
			calendar_record_h child = NULL;
			ret = calendar_record_get_child_record_at_p(record, _calendar_todo.calendar_attendee, i, &child);
			ret = __cal_vcalendar_make_attendee(b, child);
		}
		break;
	}
}

static void __make_alarm_ver1(cal_make_s *b, calendar_record_h record)
{
	// In ver 1.0, only first alarm will be dealt with.
	retm_if (NULL == b, "Invalid parameter: cal_make_s is NULL");
	retm_if (NULL == record, "Invalid parameter: record is NULL");

	int ret = 0;
	unsigned int count = 0;
	int i;
	switch (b->type)
	{
	case CALENDAR_BOOK_TYPE_EVENT:
		ret = calendar_record_get_child_record_count(record, _calendar_event.calendar_alarm, &count);
		retm_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_child_record_count() is failed(%d)", ret);
		for (i = 0; i < count; i++) {
			calendar_record_h child = NULL;
			ret = calendar_record_get_child_record_at_p(record, _calendar_event.calendar_alarm, i, &child);
			warn_if(CALENDAR_ERROR_NONE != ret, "Failed to get child alarm(%d)", ret);

			__cal_vcalendar_make_aalarm(b, record, child);
		}
		break;
	case CALENDAR_BOOK_TYPE_TODO:
		ret = calendar_record_get_child_record_count(record, _calendar_todo.calendar_alarm, &count);
		retm_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_child_record_count() is failed(%d)", ret);
		for (i = 0; i < count; i++) {
			calendar_record_h child = NULL;
			ret = calendar_record_get_child_record_at_p(record, _calendar_todo.calendar_alarm, i, &child);
			warn_if(CALENDAR_ERROR_NONE != ret, "Failed to get child alarm(%d)", ret);

			__cal_vcalendar_make_aalarm(b, record, child);
		}
		break;
	}
}

static void __make_alarm_ver2(cal_make_s *b, calendar_record_h record)
{
	retm_if (NULL == b, "Invalid parameter: cal_make_s is NULL");
	retm_if (NULL == record, "Invalid parameter: record is NULL");

	int ret = 0;
	unsigned int count = 0;
	int i;
	switch (b->type)
	{
	case CALENDAR_BOOK_TYPE_EVENT:
		ret = calendar_record_get_child_record_count(record, _calendar_event.calendar_alarm, &count);
		retm_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_child_record_count() is failed(%d)", ret);
		for (i = 0; i < count; i++) {
			calendar_record_h child = NULL;
			ret = calendar_record_get_child_record_at_p(record, _calendar_event.calendar_alarm, i, &child);
			warn_if(CALENDAR_ERROR_NONE != ret, "Failed to get child alarm(%d)", ret);

			__cal_vcalendar_make_alarm(b, child);
		}
		break;
	case CALENDAR_BOOK_TYPE_TODO:
		ret = calendar_record_get_child_record_count(record, _calendar_todo.calendar_alarm, &count);
		retm_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_child_record_count() is failed(%d)", ret);
		for (i = 0; i < count; i++) {
			calendar_record_h child = NULL;
			ret = calendar_record_get_child_record_at_p(record, _calendar_todo.calendar_alarm, i, &child);
			warn_if(CALENDAR_ERROR_NONE != ret, "Failed to get child alarm(%d)", ret);

			__cal_vcalendar_make_alarm(b, child);
		}
		break;
	}
}

static void __make_alarm(cal_make_s *b, calendar_record_h record)
{
	retm_if (NULL == b, "Invalid parameter: cal_make_s is NULL");
	retm_if (NULL == record, "Invalid parameter: record is NULL");

	switch (b->type)
	{
	case CALENDAR_BOOK_TYPE_EVENT:
		switch (b->version)
		{
		case VCAL_VER_1:
			__make_alarm_ver1(b, record);
			break;
		case VCAL_VER_2:
			__make_alarm_ver2(b, record);
			break;
		}
		break;
	case CALENDAR_BOOK_TYPE_TODO:
		ERR("No rrule in todo");
		return;
	}

}
static void __make_created_time(cal_make_s *b, calendar_record_h record)
{
	retm_if (NULL == b, "Invalid parameter: cal_make_s is NULL");
	retm_if (NULL == record, "Invalid parameter: record is NULL");

	int ret = 0;
	long long int value = 0;
	switch (b->type)
	{
	case CALENDAR_BOOK_TYPE_EVENT:
		ret = calendar_record_get_lli(record, _calendar_event.created_time, &value);
		retm_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_lli() is failed(%d)", ret);
		break;
	case CALENDAR_BOOK_TYPE_TODO:
		ret = calendar_record_get_lli(record, _calendar_todo.created_time, &value);
		retm_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_lli() is failed(%d)", ret);
		break;
	}
	int y = 0, m = 0, d = 0;
	int h = 0, n = 0, s = 0;
	_cal_time_get_datetime(value, &y, &m, &d, &h, &n, &s);
	char buf[128] = {0};
	snprintf(buf, sizeof(buf), VCAL_DATETIME_FORMAT_YYYYMMDDTHHMMSSZ, y, m, d, h, n, s);

	switch (b->version)
	{
	case VCAL_VER_1:
		__cal_vcalendar_make_printf(b, "DCREATED:", buf);
		break;
	case VCAL_VER_2:
		__cal_vcalendar_make_printf(b, "CREATED:", buf);
		break;
	}
}

static void __make_summary(cal_make_s *b, calendar_record_h record)
{
	retm_if (NULL == b, "Invalid parameter: cal_make_s is NULL");
	retm_if (NULL == record, "Invalid parameter: record is NULL");

	int ret = 0;
	char *value = NULL;
	switch (b->type)
	{
	case CALENDAR_BOOK_TYPE_EVENT:
		ret = calendar_record_get_str_p(record, _calendar_event.summary, &value);
		retm_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_str_p() is failed(%d)", ret);
		break;
	case CALENDAR_BOOK_TYPE_TODO:
		ret = calendar_record_get_str_p(record, _calendar_todo.summary, &value);
		retm_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_str_p() is failed(%d)", ret);
		break;
	}
	if (value && *value) {
		char *summary = NULL;
		__encode_escaped_char(value, &summary);
		__cal_vcalendar_make_printf(b, "SUMMARY:", summary);
		free(summary);
	}
}

static void __make_description(cal_make_s *b, calendar_record_h record)
{
	retm_if (NULL == b, "Invalid parameter: cal_make_s is NULL");
	retm_if (NULL == record, "Invalid parameter: record is NULL");

	int ret = 0;
	char *value = NULL;
	switch (b->type)
	{
	case CALENDAR_BOOK_TYPE_EVENT:
		ret = calendar_record_get_str_p(record, _calendar_event.description, &value);
		retm_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_str_p() is failed(%d)", ret);
		break;
	case CALENDAR_BOOK_TYPE_TODO:
		ret = calendar_record_get_str_p(record, _calendar_todo.description, &value);
		retm_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_str_p() is failed(%d)", ret);
		break;
	}
	if (value && *value) {
		char *description = NULL;
		__encode_escaped_char(value, &description);
		__cal_vcalendar_make_printf(b, "DESCRIPTION:", description);
		free(description);
	}
}

static void __make_location(cal_make_s *b, calendar_record_h record)
{
	retm_if (NULL == b, "Invalid parameter: cal_make_s is NULL");
	retm_if (NULL == record, "Invalid parameter: record is NULL");

	int ret = 0;
	char *value = NULL;
	switch (b->type)
	{
	case CALENDAR_BOOK_TYPE_EVENT:
		ret = calendar_record_get_str_p(record, _calendar_event.location, &value);
		retm_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_str_p() is failed(%d)", ret);
		break;
	case CALENDAR_BOOK_TYPE_TODO:
		ret = calendar_record_get_str_p(record, _calendar_todo.location, &value);
		retm_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_str_p() is failed(%d)", ret);
		break;
	}
	if (value && *value) {
		char *location = NULL;
		__encode_escaped_char(value, &location);
		__cal_vcalendar_make_printf(b, "LOCATION:", location);
		free(location);
	}
}

static void __make_organizer(cal_make_s *b, calendar_record_h record)
{
	retm_if (NULL == b, "Invalid parameter: cal_make_s is NULL");
	retm_if (NULL == record, "Invalid parameter: record is NULL");

	if (VCAL_VER_1 == b->version) // Invalid component in ver 1
		return;

	int ret = 0;
	char *name = NULL;
	char *email = NULL;
	switch (b->type)
	{
	case CALENDAR_BOOK_TYPE_EVENT:
		ret = calendar_record_get_str_p(record, _calendar_event.organizer_name, &name);
		retm_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_str_p() is failed(%d)", ret);
		ret = calendar_record_get_str_p(record, _calendar_event.organizer_email, &email);
		retm_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_str_p() is failed(%d)", ret);
		break;
	case CALENDAR_BOOK_TYPE_TODO:
		ret = calendar_record_get_str_p(record, _calendar_todo.organizer_name, &name);
		retm_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_str_p() is failed(%d)", ret);
		ret = calendar_record_get_str_p(record, _calendar_todo.organizer_email, &email);
		retm_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_str_p() is failed(%d)", ret);
		break;
	}
	if ((NULL == name || '\0' == *name) && (NULL == email || '\0' == *email))
		return;

	char buf[128] = {0};
	snprintf(buf, sizeof(buf), "ORGANIZER%s%s%s%s",
			(name && *name) ? ";CN=" : "",
			(name && *name) ? name : "",
			(email && *email) ? ":MAILTO:" : "",
			(email && *email) ? email : "");

	__cal_vcalendar_make_printf(b, buf, NULL);
}

static void __make_last_modified(cal_make_s *b, calendar_record_h record)
{
	retm_if (NULL == b, "Invalid parameter: cal_make_s is NULL");
	retm_if (NULL == record, "Invalid parameter: record is NULL");

	if (VCAL_VER_1 == b->version) // kies want to skip
		return;

	int ret = 0;
	long long int value = 0;
	switch (b->type)
	{
	case CALENDAR_BOOK_TYPE_EVENT:
		ret = calendar_record_get_lli(record, _calendar_event.last_modified_time, &value);
		retm_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_lli() is failed(%d)", ret);
		break;
	case CALENDAR_BOOK_TYPE_TODO:
		ret = calendar_record_get_lli(record, _calendar_todo.last_modified_time, &value);
		retm_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_lli() is failed(%d)", ret);
		break;
	}

	int y = 0, m = 0, d = 0;
	int h = 0, n = 0, s = 0;
	_cal_time_get_datetime(value, &y, &m, &d, &h, &n, &s);
	char buf[128] = {0};
	snprintf(buf, sizeof(buf), VCAL_DATETIME_FORMAT_YYYYMMDDTHHMMSSZ, y, m, d, h, n, s);
	__cal_vcalendar_make_printf(b, "LAST-MODIFIED:", buf);
}

static void __make_status(cal_make_s *b, calendar_record_h record)
{
	retm_if (NULL == b, "Invalid parameter: cal_make_s is NULL");
	retm_if (NULL == record, "Invalid parameter: record is NULL");

	int ret = 0;
	int value = 0;
	char *status = NULL;
	switch (b->type)
	{
	case CALENDAR_BOOK_TYPE_EVENT:
		ret = calendar_record_get_int(record, _calendar_event.event_status, &value);
		retm_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_int() is failed(%d)", ret);
		switch (value)
		{
		case CALENDAR_EVENT_STATUS_NONE:
			DBG("None status");
			break;
		case CALENDAR_EVENT_STATUS_TENTATIVE:
			status = "TENTATIVE";
			break;
		case CALENDAR_EVENT_STATUS_CONFIRMED:
			status = "CONFIRMED";
			break;
		case CALENDAR_EVENT_STATUS_CANCELLED:
			status = "CANCELLED";
			break;
		}
		break;
	case CALENDAR_BOOK_TYPE_TODO:
		ret = calendar_record_get_int(record, _calendar_todo.todo_status, &value);
		retm_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_int() is failed(%d)", ret);
		switch (value)
		{
		case CALENDAR_TODO_STATUS_NONE:
			DBG("None status");
			break;
		case CALENDAR_TODO_STATUS_NEEDS_ACTION:
			status = (VCAL_VER_1 == b->version) ? "NEEDS ACTION" : "NEEDS-ACTION";
			break;
		case CALENDAR_TODO_STATUS_COMPLETED:
			status = "COMPLETED";
			break;
		case CALENDAR_TODO_STATUS_IN_PROCESS:
			status = (VCAL_VER_1 == b->version) ? NULL : "IN-PROCESS";
			break;
		case CALENDAR_TODO_STATUS_CANCELED:
			status = (VCAL_VER_1 == b->version) ? NULL : "CANCELLED";
			break;
		}
		break;
	}
	if (status && *status)
		__cal_vcalendar_make_printf(b, "STATUS:", status);
}

static void __make_completed(cal_make_s *b, calendar_record_h record)
{
	retm_if (NULL == b, "Invalid parameter: cal_make_s is NULL");
	retm_if (NULL == record, "Invalid parameter: record is NULL");

	if (CALENDAR_BOOK_TYPE_EVENT == b->type) // Invalid component in event
		return;

	int ret = 0;
	long long int value = 0;
	ret = calendar_record_get_lli(record, _calendar_todo.completed_time, &value);
	retm_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_int() is failed(%d)", ret);

	int y = 0, m = 0, d = 0;
	int h = 0, n = 0, s = 0;
	_cal_time_get_datetime(value, &y, &m, &d, &h, &n, &s);
	char buf[128] = {0};
	snprintf(buf, sizeof(buf), VCAL_DATETIME_FORMAT_YYYYMMDDTHHMMSSZ, y, m, d, h, n, s);
	__cal_vcalendar_make_printf(b, "COMPLETED", buf);
}

static void __make_priority(cal_make_s *b, calendar_record_h record)
{
	retm_if (NULL == b, "Invalid parameter: cal_make_s is NULL");
	retm_if (NULL == record, "Invalid parameter: record is NULL");

	int ret = 0;
	int value = 0;
	switch (b->type)
	{
	case CALENDAR_BOOK_TYPE_EVENT:
		ret = calendar_record_get_int(record, _calendar_event.priority, &value);
		retm_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_int() is failed(%d)", ret);
		break;
	case CALENDAR_BOOK_TYPE_TODO:
		ret = calendar_record_get_int(record, _calendar_todo.priority, &value);
		retm_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_int() is failed(%d)", ret);
		break;
	}
	int priority = 0;
	switch (b->version)
	{
	case VCAL_VER_1:
		switch (value)
		{
		case CALENDAR_EVENT_PRIORITY_HIGH:
			priority = 2;
			break;
		case CALENDAR_EVENT_PRIORITY_NORMAL:
			priority = 1;
			break;
		case CALENDAR_EVENT_PRIORITY_LOW:
			priority = 0;
			break;
		default:
			priority = 0;
			break;
		}
		break;
	case VCAL_VER_2:
		switch (value)
		{
		case CALENDAR_EVENT_PRIORITY_HIGH: // in version 2.0, one out of 1 ~ 4.
			priority = 3;
			break;
		case CALENDAR_EVENT_PRIORITY_NORMAL:
			priority = 5;
			break;
		case CALENDAR_EVENT_PRIORITY_LOW: // in version 2, one out of 6 ~ 9.
			priority = 7;
			break;
		default:
			priority = 0;
			break;
		}
		break;
	}
	char buf[128] = {0};
	snprintf(buf, sizeof(buf), "PRIORITY:%d", priority);
	__cal_vcalendar_make_printf(b, buf, NULL);
}

static void __make_dtstamp(cal_make_s *b, calendar_record_h record)
{
	retm_if (NULL == b, "Invalid parameter: cal_make_s is NULL");
	retm_if (NULL == record, "Invalid parameter: record is NULL");

	if (VCAL_VER_1 == b->version) // Not support in ver 1
		return;

	long long int t = _cal_time_get_now();
	int y = 0, m = 0, d = 0;
	int h = 0, n = 0, s = 0;
	_cal_time_get_datetime(t, &y, &m, &d, &h, &n, &s);
	char buf[128] = {0};
	snprintf(buf, sizeof(buf), VCAL_DATETIME_FORMAT_YYYYMMDDTHHMMSSZ, y, m, d, h, n, s);
	__cal_vcalendar_make_printf(b, "DTSTAMP:", buf);
}

static void __make_categories(cal_make_s *b, calendar_record_h record)
{
	retm_if (NULL == b, "Invalid parameter: cal_make_s is NULL");
	retm_if (NULL == record, "Invalid parameter: record is NULL");

	int ret = 0;
	char *value = NULL;
	switch (b->type)
	{
	case CALENDAR_BOOK_TYPE_EVENT:
		ret = calendar_record_get_str_p(record, _calendar_event.categories, &value);
		retm_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_str_p() is failed(%d)", ret);
		break;
	case CALENDAR_BOOK_TYPE_TODO:
		ret = calendar_record_get_str_p(record, _calendar_todo.categories, &value);
		retm_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_str_p() is failed(%d)", ret);
		break;
	}
	if (value && *value)
		__cal_vcalendar_make_printf(b, "CATEGORIES:", value);
}

static char* __get_new_uid(void)
{
	long long int t = _cal_time_get_now();
	int y = 0, m = 0, d = 0;
	int h = 0, n = 0, s = 0;
	_cal_time_get_datetime(t, &y, &m, &d, &h, &n, &s);
	char buf[128] = {0};
	snprintf(buf, sizeof(buf), VCAL_DATETIME_FORMAT_YYYYMMDDTHHMMSSZ"@tizen.org", y, m, d, h, n, s);
	return strdup(buf);
}
static char* __get_parent_uid(int parent_id)
{
	calendar_record_h record = NULL;
	calendar_db_get_record(_calendar_event._uri, parent_id, &record);

	char *uid = NULL;
	calendar_record_get_str(record, _calendar_event.uid, &uid);
	calendar_record_destroy(record, true);

	if (NULL == uid || '\0' == *uid) {
		if (uid) free(uid);
		return NULL;
	}
	return uid;
}
static void __set_parent_uid(int parent_id, char *uid)
{
	calendar_record_h record = NULL;
	calendar_db_get_record(_calendar_event._uri, parent_id, &record);
	calendar_record_set_str(record, _calendar_event.uid, uid);
	calendar_db_update_record(record);
	calendar_record_destroy(record, true);
}
static void __set_child_uid(calendar_record_h record, char *uid)
{
	calendar_record_set_str(record, _calendar_event.uid, uid);
	calendar_db_update_record(record);
}
static void __make_uid(cal_make_s *b, calendar_record_h record)
{
	ENTER();
	char *uid = NULL;

	// search if original_event_id > 0.
	int original_event_id = 0;
	calendar_record_get_int(record, _calendar_event.original_event_id, &original_event_id);
	char *recurrence_id = NULL;
	if (original_event_id < 0) {
		DBG("This is parent event: original_event_id < 0");
		calendar_record_get_str_p(record, _calendar_event.uid, &uid);
		if (uid && *uid) {
			__cal_vcalendar_make_printf(b, "UID:", uid);
		} else {
			uid = __get_new_uid();
			__cal_vcalendar_make_printf(b, "UID:", uid);
			free(uid);
		}
		if (VCAL_VER_1 == b->version) // ver 1.0 does not support: recurrence-id
			return;

		calendar_record_get_str_p(record, _calendar_event.recurrence_id, &recurrence_id);
		if (recurrence_id && *recurrence_id) {
			if (*recurrence_id >= '0' && *recurrence_id <= '9')
				__cal_vcalendar_make_printf(b, "RECURRENCE-ID:", recurrence_id);
			else
				__cal_vcalendar_make_printf(b, "RECURRENCE-ID;", recurrence_id);
		}
		return;
	}

	// if original_event_id > 0, get parent uid.
	uid = __get_parent_uid(original_event_id);
	if (NULL == uid || '\0' == *uid) {
		uid = __get_new_uid();
		__set_parent_uid(original_event_id, uid);
	}
	__set_child_uid(record, uid);
	__cal_vcalendar_make_printf(b, "UID:", uid);

	if (VCAL_VER_1 == b->version) // ver 1.0 does not support: recurrence-id
		return;

	if (*recurrence_id >= '0' && *recurrence_id <= '9')
		__cal_vcalendar_make_printf(b, "RECURRENCE-ID:", recurrence_id);
	else
		__cal_vcalendar_make_printf(b, "RECURRENCE-ID;", recurrence_id);
}
static void __make_exdate(cal_make_s *b, calendar_record_h record)
{
	retm_if (NULL == b, "Invalid parameter: cal_make_s is NULL");
	retm_if (NULL == record, "Invalid parameter: record is NULL");

	int ret = 0;
	char *value = NULL;
	switch (b->type)
	{
	case CALENDAR_BOOK_TYPE_EVENT:
		ret = calendar_record_get_str_p(record, _calendar_event.exdate, &value);
		retm_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_str_p() is failed(%d)", ret);
		break;
	case CALENDAR_BOOK_TYPE_TODO:
		ERR("Not support exdate in TODO");
		break;
	}
	if (value && *value)
		__cal_vcalendar_make_printf(b, "EXDATE:", value);
}

static void __cal_vcalendar_make_child_extended(cal_make_s *b, calendar_record_h child, bool *has_lunar)
{
	retm_if (NULL == b, "Invalid parameter: cal_make_s is NULL");
	retm_if (NULL == child, "Invalid parameter: record is NULL");

	int ret = 0;;
	char *key = NULL;
	char *value = NULL;

	ret = calendar_record_get_str_p(child, _calendar_extended_property.key, &key);
	retm_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_str_p() is failed(%d)", ret);
	if (NULL == key || '\0' == *key || (0 != strncmp(key, "X-", strlen("X-")))) {
		DBG("Not extended for vcalendar[%s]", key);
		return;
	}

	ret = calendar_record_get_str_p(child, _calendar_extended_property.value, &value);
	retm_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_str_p() is failed(%d)", ret);

	// check lunar: will handle next
	if (has_lunar) {
		if ((!strncmp(key, "X-LUNAR", strlen("X-LUNAR")) && !strncmp(value, ":SET", strlen(":SET"))) ||
				(!strncmp(key, "X-LUNAR:", strlen("X-LUNAR:")) && !strncmp(value, "SET", strlen("SET")))) {
			*has_lunar = true;
			return;
		}
	}

	ret = __cal_vcalendar_make_printf(b, key, value);
	retm_if (CALENDAR_ERROR_NONE != ret, "__cal_vcalendar_make_printf() is failed(%d)", ret);
}

static void __make_extended(cal_make_s *b, calendar_record_h record)
{
	retm_if (NULL == b, "Invalid parameter: cal_make_s is NULL");
	retm_if (NULL == record, "Invalid parameter: record is NULL");

	int ret = 0;
	unsigned int count = 0;
	int i;
	bool has_lunar = false;
	int calendar_system_type = 0;
	switch (b->type)
	{
	case CALENDAR_BOOK_TYPE_EVENT:
		ret = calendar_record_get_child_record_count(record, _calendar_event.extended, &count);
		retm_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_child_record_count() is failed(%d)", ret);
		for (i = 0; i < count; i++) {
			calendar_record_h child = NULL;
			ret = calendar_record_get_child_record_at_p(record, _calendar_event.extended, i, &child);
			warn_if(CALENDAR_ERROR_NONE != ret, "Failed to get child alarm(%d)", ret);

			__cal_vcalendar_make_child_extended(b, child, &has_lunar);
		}

		// lunar
		ret = calendar_record_get_int(record, _calendar_event.calendar_system_type, &calendar_system_type);
		retm_if (CALENDAR_ERROR_NONE != ret, "Failed to get calendar_record_type(%d)", ret);

		if (true == has_lunar || CALENDAR_SYSTEM_EAST_ASIAN_LUNISOLAR == calendar_system_type)
			__cal_vcalendar_make_printf(b, "X-LUNAR:SET", NULL);
		break;
	case CALENDAR_BOOK_TYPE_TODO:
		ret = calendar_record_get_child_record_count(record, _calendar_todo.extended, &count);
		retm_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_child_record_count() is failed(%d)", ret);
		for (i = 0; i < count; i++) {
			calendar_record_h child = NULL;
			ret = calendar_record_get_child_record_at_p(record, _calendar_todo.extended, i, &child);
			warn_if(CALENDAR_ERROR_NONE != ret, "Failed to get child alarm(%d)", ret);

			__cal_vcalendar_make_child_extended(b, child, NULL);
		}
		break;
	}
}

static void __make_end(cal_make_s *b, calendar_record_h record)
{
	retm_if (NULL == b, "Invalid parameter: cal_make_s is NULL");
	retm_if (NULL == record, "Invalid parameter: record is NULL");

	int ret = 0;
	switch (b->type)
	{
	case CALENDAR_BOOK_TYPE_EVENT:
		ret = __cal_vcalendar_make_printf(b, "END:VEVENT", NULL);
		retm_if(CALENDAR_ERROR_NONE != ret, "__cal_vcalendar_make_printf() is failed(%d)", ret);
		break;
	case CALENDAR_BOOK_TYPE_TODO:
		ret = __cal_vcalendar_make_printf(b, "END:VTODO", NULL);
		retm_if (CALENDAR_ERROR_NONE != ret, "__cal_vcalendar_make_printf() is failed(%d)", ret);
		break;
	}
}


static void __cal_vcalendar_make_schedule(cal_make_s *b, calendar_record_h record)
{
	retm_if (NULL == b, "Invalid parameter: cal_make_s is NULL");
	retm_if (NULL == record, "Invalid parameter: record is NULL");

	__make_begin(b, record);
	__make_summary(b, record);
	__make_dtstart(b, record);
	__make_dtend(b, record);
	__make_rrule(b, record); // only event
	__make_sensitivity(b, record);
	__make_created_time(b, record);
	__make_description(b, record);
	__make_location(b, record);
	__make_organizer(b, record);
	__make_last_modified(b, record);
	__make_status(b, record);
	__make_completed(b, record); // only todo
	__make_priority(b, record);
	__make_dtstamp(b, record);
	__make_categories(b, record);
	__make_uid(b, record);
	__make_exdate(b, record); // only event
	__make_attendee(b, record);
	__make_alarm(b, record);
	__make_extended(b, record);
	__make_end(b, record);

}

static void __append_header(cal_make_s *b)
{
	retm_if (NULL == b, "Invalid parameter: cal_make_s is NULL");

	int ret = 0;

	ret = __cal_vcalendar_make_printf(b, "BEGIN:VCALENDAR", NULL);
	retm_if (CALENDAR_ERROR_NONE != ret, "__cal_vcalendar_make_printf() is failed(%d)", ret);

	ret = __cal_vcalendar_make_printf(b, "PRODID:vCal ID Default", NULL);
	retm_if (CALENDAR_ERROR_NONE != ret, "__cal_vcalendar_make_printf() is failed(%d)", ret);

	char buf[32] = {0};
	snprintf(buf, sizeof(buf), "VERSION:%d.0", b->version);
	ret = __cal_vcalendar_make_printf(b, buf, NULL);
	retm_if (CALENDAR_ERROR_NONE != ret, "__cal_vcalendar_make_printf() is failed(%d)", ret);
}

static void __make_footer(cal_make_s *b)
{
	retm_if (NULL == b, "Invalid parameter: cal_make_s is NULL");

	int ret = 0;
	ret = __cal_vcalendar_make_printf(b, "END:VCALENDAR", NULL);
	retm_if(CALENDAR_ERROR_NONE != ret, "__cal_vcalendar_make_printf() is failed(%d)", ret);
}

static void __make_tz(cal_make_s *b, char *tzid, long long int created)
{
	retm_if (NULL == b, "Invalid parameter: cal_make_s is NULL");
	retm_if (NULL == tzid || '\0' == *tzid, "Invalid parameter: tzid is NULL");

	int ret = 0;

	time_t zone = 0;
	time_t dst = 0;
	_cal_time_get_tz_offset(tzid, &zone, &dst);
	DBG("offset zone(%ld), dst (%ld)", zone, dst);

	bool in_dst = _cal_time_in_dst(tzid, created);
	dst = in_dst ? dst : 0; // dst in TZ is depending on created time.
	DBG("tzid[%s] created time(%lld) in_dst(%d)", tzid, created, in_dst);

	int h = (zone / 3600) + (dst / 3600);
	int m = (zone % 3600) / 60 + (dst % 3600) / 60;

	char buf[32] = {0};
	snprintf(buf, sizeof(buf), "TZ:%s%02d:%02d",
			h == 0 ? "" : (h < 0 ? "-" : "+"),
			h < 0 ? (-1 * h) : h, m < 0 ? (-1 * m) : m);

	ret = __cal_vcalendar_make_printf(b, buf, NULL);
	retm_if(CALENDAR_ERROR_NONE != ret, "__cal_vcalendar_make_printf() is failed");
	DBG("append tzid[%s]", buf);
}

static void __devide_vcalendar_with_header(cal_make_s *b, calendar_record_h record)
{
	retm_if (NULL == b, "cal_make_s is NULL");
	retm_if (NULL == record, "Invalid parameter: record is NULL");

	if (2 == b->version)
		return;

	int ret = 0;
	char *uri = NULL;
	ret = calendar_record_get_uri_p(record, &uri);
	retm_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_uri_p() is failed(%d)", ret);

	char *tzid = NULL;
	long long int created = 0;

	switch (b->type)
	{
	case CALENDAR_BOOK_TYPE_EVENT:
		ret = calendar_record_get_str_p(record, _calendar_event.start_tzid, &tzid);
		warn_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_str_p() is failed(%d)", ret);
		ret = calendar_record_get_lli(record, _calendar_event.created_time, &created);
		warn_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_lli() is failed(%d)", ret);
		break;
	case CALENDAR_BOOK_TYPE_TODO:
		ret = calendar_record_get_str_p(record, _calendar_todo.due_tzid, &tzid);
		warn_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_str_p() is failed(%d)", ret);
		ret = calendar_record_get_lli(record, _calendar_todo.created_time, &created);
		warn_if (CALENDAR_ERROR_NONE != ret, "calendar_record_get_lli() is failed(%d)", ret);
		break;
	}

	if (NULL == tzid || '\0' == *tzid) {
		DBG("No tzid");
		return;
	}

	if (NULL == b->timezone_tzid || '\0' == *b->timezone_tzid) { // new start of vcalendar
		__make_tz(b, tzid, created);
		b->timezone_tzid = strdup(tzid);
	} else { // not first vcalendar
		if (0 != strncmp(b->timezone_tzid, tzid, strlen(tzid))) { // different tzid
			__make_footer(b);
			__append_header(b);
			__make_tz(b, tzid, created);
			if (b->timezone_tzid)
				free(b->timezone_tzid);
			b->timezone_tzid = strdup(tzid);
		} else {
			DBG("same as before, skip");
		}
	}
}
static int __make_vcalendar(cal_make_s *b, calendar_list_h list)
{
	retvm_if (NULL == b, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter: cal_make_s is NULL");
	retvm_if (NULL == list, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid paramter: list is NULL");

	int ret = CALENDAR_ERROR_NONE;
	calendar_record_h record = NULL;

	__append_header(b);

	// start
	ret = calendar_list_first(list);
	retvm_if (CALENDAR_ERROR_NONE != ret, ret, "calendar_list_first() Failed");
	do {
		ret = calendar_list_get_current_record_p(list, &record);
		if (CALENDAR_ERROR_NONE != ret) break;

		// start vcalendar
		char *uri = NULL;
		ret = calendar_record_get_uri_p(record, &uri);
		DBG("uri[%s]", uri);

		if (!strcmp(uri, _calendar_event._uri)) {
			b->type = CALENDAR_BOOK_TYPE_EVENT;
			__devide_vcalendar_with_header(b, record);
			__cal_vcalendar_make_schedule(b, record);

		} else if (!strcmp(uri, _calendar_todo._uri)) {
			b->type = CALENDAR_BOOK_TYPE_TODO;
			__devide_vcalendar_with_header(b, record);
			__cal_vcalendar_make_schedule(b, record);

		} else if (!strcmp(uri, _calendar_extended_property._uri)) {
			cal_extended_s *extended = (cal_extended_s *)record;
			if (!strncmp(extended->key, "VERSION", strlen("VERSION"))) continue;

			ret = __cal_vcalendar_make_printf(b, extended->key, extended->value);
			if (CALENDAR_ERROR_NONE != ret) break;

			DBG("extended key[%s] value[%s]", extended->key, extended->value);

		} else if (!strcmp(uri, _calendar_timezone._uri)) {
			DBG("Not support timezone");

		} else if (!strcmp(uri, _calendar_book._uri)) {
			DBG("Not support calendar");

		} else {
			DBG("Unable to understand uri[%s]", uri);
		}

	} while(calendar_list_next(list) != CALENDAR_ERROR_NO_DATA);
	__make_footer(b);

	return CALENDAR_ERROR_NONE;
}

int _cal_vcalendar_make_vcalendar(cal_make_s *b, calendar_list_h list)
{
	retvm_if (NULL == list, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid paramter:list is NULL");

	int ret = CALENDAR_ERROR_NONE;
	calendar_record_h record = NULL;

	int version = 2; // set default as ver 2.0

	ret = calendar_list_first(list);
	retvm_if (CALENDAR_ERROR_NONE != ret, ret, "calendar_list_first() Failed");
	do {
		ret = calendar_list_get_current_record_p(list, &record);
		if(CALENDAR_ERROR_NONE != ret) break;

		char *uri = NULL;
		ret = calendar_record_get_uri_p(record, &uri);

		if (!strcmp(uri, _calendar_extended_property._uri)) {
			cal_extended_s *extended = (cal_extended_s *)record;
			if (!strncmp(extended->key, "VERSION", strlen("VERSION"))) {
				version = strstr(extended->value, "1.0") ? 1 : 2;
				break;
			}
		}
	} while(calendar_list_next(list) != CALENDAR_ERROR_NO_DATA);
	b->version = version;
	DBG("make as version(%d)", version);

	__make_vcalendar(b, list);
	return CALENDAR_ERROR_NONE;
}
