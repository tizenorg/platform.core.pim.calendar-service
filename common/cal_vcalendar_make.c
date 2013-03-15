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

#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_record.h"
#include "cal_view.h"
#include "cal_time.h"

#include "cal_vcalendar.h"
#include "cal_vcalendar_make.h"

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

	retvm_if(!b->data, CALENDAR_ERROR_OUT_OF_MEMORY,
			"Failed to realloc");
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
	retv_if(ret != CALENDAR_ERROR_NONE, ret);

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
		retv_if(ret != CALENDAR_ERROR_NONE, ret);
		remain_lbuf = sizeof(b->lbuf);
	}

	strncat(b->lbuf, s + k, remain_lbuf - 1);
	return CALENDAR_ERROR_NONE;
}

static inline int __cal_vcalendar_make_flush(cal_make_s *b)
{
	int ret;
	ret = __cal_vcalendar_make_alloc(b, _strlen(b->lbuf) + 2);
	retv_if(ret != CALENDAR_ERROR_NONE, ret);

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
		retv_if(ret != CALENDAR_ERROR_NONE, ret);
	}

	if (s2) {
		ret = __cal_vcalendar_make_set_str(b, s2);
		retv_if(ret != CALENDAR_ERROR_NONE, ret);
	}

	return __cal_vcalendar_make_flush(b);
}

char *_cal_vcalendar_make_get_data(cal_make_s *b)
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

static const char *__cal_vcalendar_make_itos(int y, int m, int d)
{
	static char buf[9];
	snprintf(buf, sizeof(buf), "%04d%02d%02d", y, m, d);
	return buf;
}

/////////////////////////////////////////////////////////////////////////////
static inline int __cal_vcalendar_make_class(cal_make_s *b, int v)
{
	const char *c;
	// TODO : Need to define enumeration of class property
	switch(v) {
		case 0:
			c = "PUBLIC"; break;
		case 1:
			c = "PRIVATE"; break;
		case 2:
			c = "CONFIDENTIAL"; break;
		default:
			c = "PUBLIC"; break;
	}
	__cal_vcalendar_make_printf(b, "CLASS:", c);
	return CALENDAR_ERROR_NONE;
}

static inline int __cal_vcalendar_make_transp(cal_make_s *b, int v)
{
	// TODO : Need to define enumeration of transp property
	__cal_vcalendar_make_printf(b, "TRANSP:", v? "OPAQUE":"TRANSPARENT");
	return CALENDAR_ERROR_NONE;
}

static inline int __cal_vcalendar_make_created(cal_make_s *b, long long int t)
{
    char* tmp_tzid = NULL;
    tmp_tzid = _cal_time_convert_ltos(NULL, t);
    if (tmp_tzid)
    {
        __cal_vcalendar_make_printf(b, "CREATED:", tmp_tzid);
        CAL_FREE(tmp_tzid);
    }
	return CALENDAR_ERROR_NONE;
}

static inline int __cal_vcalendar_make_dtstart(cal_make_s *b, char *tzid, calendar_time_s *caltime)
{
	retvm_if(caltime == NULL, -1, "Invalid argument: calendar_time_s is NULL");

	if (caltime->time.utime == CALENDAR_TODO_NO_START_DATE)
	{
		DBG("No start date");
		return CALENDAR_ERROR_NONE;
	}

	switch (caltime->type)
	{
	case CALENDAR_TIME_UTIME:
        {
			char* str_time = NULL;
			if (NULL != tzid && strlen(tzid) > 0)
			{
				str_time = _cal_time_convert_ltos(tzid, caltime->time.utime);
				if (str_time)
				{
					char buf[128] = {0};
					snprintf(buf, sizeof(buf), "DTSTART;TZID=%s:", tzid);
					__cal_vcalendar_make_printf(b, buf, str_time);
					CAL_FREE(str_time);
				}
			}
			else
			{
				str_time = _cal_time_convert_ltos(NULL, caltime->time.utime);
				if (str_time)
				{
					__cal_vcalendar_make_printf(b, "DTSTART:", str_time);
					CAL_FREE(str_time);
				}
			}
        }
		break;

	case CALENDAR_TIME_LOCALTIME:
		__cal_vcalendar_make_printf(b, "DTSTART;VALUE=DATE:",
				__cal_vcalendar_make_itos(caltime->time.date.year,
					caltime->time.date.month, caltime->time.date.mday));
		break;
	}

	return CALENDAR_ERROR_NONE;
}

static inline int __cal_vcalendar_make_last_mod(cal_make_s *b, long long int lli)
{
    char* tmp_tzid = NULL;
    tmp_tzid = _cal_time_convert_ltos(NULL, lli);
    if (tmp_tzid)
    {
        __cal_vcalendar_make_printf(b, "LAST-MODIFIED:",tmp_tzid);
        CAL_FREE(tmp_tzid);
    }
	return CALENDAR_ERROR_NONE;
}

int __cal_vcalendar_make_organizer(cal_make_s *b, char *cn, char *address)
{
	int ret;

	ret = __cal_vcalendar_make_set_str(b, "ORGANIZER");
	retv_if(ret != CALENDAR_ERROR_NONE, ret);
	if (cn && *cn)
	{
		ret = __cal_vcalendar_make_set_str(b, ";CN=");
		ret = __cal_vcalendar_make_set_str(b, cn);
		retv_if(ret != CALENDAR_ERROR_NONE, ret);
	}

	if (address && *address)
	{
		ret = __cal_vcalendar_make_set_str(b, ":mailto:");
		ret = __cal_vcalendar_make_set_str(b, address);
	}
	__cal_vcalendar_make_flush(b);
	return CALENDAR_ERROR_NONE;
}

static inline int __cal_vcalendar_make_priority(cal_make_s *b, int v)
{
	char tmp[2];
	snprintf(tmp, sizeof(tmp), "%d", v);
	__cal_vcalendar_make_printf(b, "PRIORITY:", tmp);
	return CALENDAR_ERROR_NONE;
}

int __cal_vcalendar_make_dtstamp(cal_make_s *b, char *tzid)
{
    char* tmp_tzid = NULL;
    tmp_tzid = _cal_time_convert_ltos(tzid, _cal_time_get_now());
    if (tmp_tzid)
    {
        __cal_vcalendar_make_printf(b, "DTSTAMP:", tmp_tzid);
        CAL_FREE(tmp_tzid);
    }

	return CALENDAR_ERROR_NONE;
}

static inline int __cal_vcalendar_make_dtend(cal_make_s *b, char *tzid, calendar_time_s *caltime)
{
	retvm_if(caltime == NULL, -1, "Invalid argument: calendar_time_s is NULL");

	if (caltime->time.utime == CALENDAR_TODO_NO_DUE_DATE)
	{
		DBG("No start date");
		return CALENDAR_ERROR_NONE;
	}

	switch (caltime->type)
	{
	case CALENDAR_TIME_UTIME:
        {
			char* str_time = NULL;
			if (NULL != tzid && strlen(tzid) > 0)
			{
				str_time = _cal_time_convert_ltos(tzid, caltime->time.utime);
				if (str_time)
				{
					char buf[128] = {0};
					snprintf(buf, sizeof(buf), "DTEND;TZID=%s:", tzid);
					__cal_vcalendar_make_printf(b, buf, str_time);
					CAL_FREE(str_time);
				}
			}
			else
			{
				str_time = _cal_time_convert_ltos(NULL, caltime->time.utime);
				if (str_time)
				{
					__cal_vcalendar_make_printf(b, "DTEND:", str_time);
					CAL_FREE(str_time);
				}
			}
        }
		break;

	case CALENDAR_TIME_LOCALTIME:
		__cal_vcalendar_make_printf(b, "DTEND;VALUE=DATE:",
				__cal_vcalendar_make_itos(caltime->time.date.year,
					caltime->time.date.month, caltime->time.date.mday));
		break;
	}

	return CALENDAR_ERROR_NONE;
}

static inline int __cal_vcalendar_make_due(cal_make_s *b, char *tzid,  calendar_time_s *caltime)
{
	retvm_if(caltime == NULL, -1, "Invalid argument: calendar_time_s is NULL");

	if (caltime->time.utime == CALENDAR_TODO_NO_DUE_DATE)
	{
		DBG("No due date");
		return CALENDAR_ERROR_NONE;
	}

	switch (caltime->type)
	{
	case CALENDAR_TIME_UTIME:
        {
			char* str_time = NULL;
			if (NULL != tzid && strlen(tzid) > 0)
			{
				str_time = _cal_time_convert_ltos(tzid, caltime->time.utime);
				if (str_time)
				{
					char buf[128] = {0};
					snprintf(buf, sizeof(buf), "DUE;TZID=%s:", tzid);
					__cal_vcalendar_make_printf(b, buf, str_time);
					CAL_FREE(str_time);
				}
			}
			else
			{
				str_time = _cal_time_convert_ltos(NULL, caltime->time.utime);
				if (str_time)
				{
					__cal_vcalendar_make_printf(b, "DUE:", str_time);
					CAL_FREE(str_time);
				}
			}
        }
		break;

	case CALENDAR_TIME_LOCALTIME:
		__cal_vcalendar_make_printf(b, "DUE:",
				__cal_vcalendar_make_itos(caltime->time.date.year,
					caltime->time.date.month, caltime->time.date.mday));
		break;
	}

	return CALENDAR_ERROR_NONE;
}

static inline int __cal_vcalendar_make_exdate(cal_make_s *b, char *s)
{
	__cal_vcalendar_make_printf(b, "EXDATE:", s);
	return CALENDAR_ERROR_NONE;
}

int __cal_vcalendar_make_attendee(cal_make_s *b, calendar_record_h attendee)
{
	int ret;
	int role, status, rsvp;
	char *group = NULL;
	char *delegate_uri = NULL;
	char *delegator_uri = NULL;
	char *name = NULL;
	char *email = NULL;

	retvm_if(attendee == NULL, CALENDAR_ERROR_INVALID_PARAMETER,
			"Invalid argument: attendee is NULL");

	ret = __cal_vcalendar_make_set_str(b, "ATTENDEE");
	retv_if(ret != CALENDAR_ERROR_NONE, ret);

	ret = calendar_record_get_str_p(attendee, _calendar_attendee.group, &group);

	if (group && *group) {
		ret = __cal_vcalendar_make_set_str(b, ";CUTYPE=");
		ret = __cal_vcalendar_make_set_str(b, group);
		retv_if(ret != CALENDAR_ERROR_NONE, ret);
	}

	// TODO : NO 'member' member in cal_participant_info_t

	ret = calendar_record_get_int(attendee, _calendar_attendee.role, &role);
	{
		ret = __cal_vcalendar_make_set_str(b, ";ROLE=");
		ret = __cal_vcalendar_make_set_str(b, _att_role[role]);
		retv_if(ret != CALENDAR_ERROR_NONE, ret);
	}

	ret = calendar_record_get_int(attendee, _calendar_attendee.status, &status);
	{
		ret = __cal_vcalendar_make_set_str(b, ";PARTSTAT=");
		ret = __cal_vcalendar_make_set_str(b, _att_st[status]);
		retv_if(ret != CALENDAR_ERROR_NONE, ret);
	}

	ret = calendar_record_get_int(attendee, _calendar_attendee.rsvp, &rsvp);
	{
		ret = __cal_vcalendar_make_set_str(b, ";RSVP=");
		ret = __cal_vcalendar_make_set_str(b, rsvp ? "TRUE" : "FALSE");
		retv_if(ret != CALENDAR_ERROR_NONE, ret);
	}

	ret = calendar_record_get_str_p(attendee,	_calendar_attendee.delegate_uri, &delegate_uri);
	if (delegate_uri && *delegate_uri)
	{
		ret = __cal_vcalendar_make_set_str(b, ";DELEGATED-TO=");
		ret = __cal_vcalendar_make_set_str(b, delegate_uri);
		retv_if(ret != CALENDAR_ERROR_NONE, ret);
	}

	ret = calendar_record_get_str_p(attendee, _calendar_attendee.delegator_uri, &delegator_uri);
	if (delegator_uri && *delegator_uri)
	{
		ret = __cal_vcalendar_make_set_str(b, ";DELEGATED-FROM=");
		ret = __cal_vcalendar_make_set_str(b, delegator_uri);
		retv_if(ret != CALENDAR_ERROR_NONE, ret);
	}

	// TODO : No 'sentby' member in cal_participant_info_t

	ret = calendar_record_get_str_p(attendee, _calendar_attendee.name, &name);
	if (name && *name) {
		ret = __cal_vcalendar_make_set_str(b, ";CN=");
		ret = __cal_vcalendar_make_set_str(b, name);
		retv_if(ret != CALENDAR_ERROR_NONE, ret);
	}

	ret = calendar_record_get_str_p(attendee,	_calendar_attendee.email, &email);
	if (email && *email)
	{
		ret = __cal_vcalendar_make_set_str(b, ":mailto:");
		ret = __cal_vcalendar_make_set_str(b, email);
		retv_if(ret != CALENDAR_ERROR_NONE, ret);
	}
	__cal_vcalendar_make_flush(b);
	return CALENDAR_ERROR_NONE;
}

static const char *vl_dur(calendar_alarm_time_unit_type_e unit, int dur)
{
	static char buf[8];
	int i = 0;
	char d[5];

	if (dur < 0) {
		*buf = '-';
		i++;
		dur = -dur;
	}

	snprintf(d, sizeof(d), "%d", dur);

	*(buf + i) = 'P';
	i++;

	switch (unit) {
		case CALENDAR_ALARM_TIME_UNIT_WEEK:
			snprintf(buf + i, sizeof(buf) - i, "%sW", d);
			break;
		case CALENDAR_ALARM_TIME_UNIT_DAY:
			snprintf(buf + i, sizeof(buf) - i, "%sD", d);
			break;
		case CALENDAR_ALARM_TIME_UNIT_HOUR:
			snprintf(buf + i, sizeof(buf) - i, "T%sH", d);
			break;
		case CALENDAR_ALARM_TIME_UNIT_MINUTE:
			snprintf(buf + i, sizeof(buf) - i, "T%sM", d);
			break;
		default:
			buf[0] = '\0';
	}

	return buf;
}

int __cal_vcalendar_make_trigger(cal_make_s *b, int unit, int dur, long long int t)
{
	retvm_if(unit < 0, CALENDAR_ERROR_NO_DATA, "tick unit is invalid");

	if (unit == CALENDAR_ALARM_TIME_UNIT_SPECIFIC)
	{
	    char* tmp_tzid = NULL;
	    tmp_tzid = _cal_time_convert_ltos(NULL, t);
	    if (tmp_tzid)
	    {
	        __cal_vcalendar_make_printf(b, "TRIGGER;VALUE=DATE-TIME:",tmp_tzid);
	        CAL_FREE(tmp_tzid);
	    }

	}
	else
	{
		__cal_vcalendar_make_printf(b, "TRIGGER:", vl_dur(unit, dur));
	}
	return CALENDAR_ERROR_NONE;
}

int __cal_vcalendar_make_action(cal_make_s *b)
{
	return __cal_vcalendar_make_printf(b, "ACTION:", "AUDIO");
}

int __cal_vcalendar_make_audio(cal_make_s *b, calendar_record_h alarm)
{
	int ret;
	int tick, unit;
	long long int utime;

	ret = __cal_vcalendar_make_action(b);
	retv_if(ret != CALENDAR_ERROR_NONE, ret);

	ret = calendar_record_get_int(alarm, _calendar_alarm.tick, &tick);
	ret = calendar_record_get_int(alarm, _calendar_alarm.tick_unit, &unit);
	ret = calendar_record_get_lli(alarm, _calendar_alarm.time, &utime);

	ret = __cal_vcalendar_make_trigger(b, unit, tick, utime);
	retv_if(ret != CALENDAR_ERROR_NONE, ret);

	// TODO : Support duration
	// TODO : Support repeat (snooze)
	return CALENDAR_ERROR_NONE;
}

// ver 1.0 aalarm
int __cal_vcalendar_make_aalarm(cal_make_s *b, calendar_record_h record, calendar_record_h alarm)
{
	int ret;
	int type;
	int tick, unit;
	char *uri = NULL;
	long long int utime;
	calendar_time_s caltime = {0};
	retvm_if(alarm == NULL, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid argument: alarm is NULL");

	ret = calendar_record_get_uri_p(record, &uri);
	if (CALENDAR_ERROR_NONE != ret)
	{
		ERR("calendar_record_get_uri_p() failed");
		return ret;
	}

	if (!strncmp(uri, _calendar_event._uri, strlen(_calendar_event._uri)))
	{
		type = CALENDAR_RECORD_TYPE_EVENT;
	}
	else if (!strncmp(uri, _calendar_todo._uri, strlen(_calendar_todo._uri)))
	{
		type = CALENDAR_RECORD_TYPE_TODO;
	}
	else
	{
		ERR("Invalid type(%s)", uri);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	ret = calendar_record_get_int(alarm, _calendar_alarm.tick_unit, &unit);
	if (CALENDAR_ERROR_NONE != ret)
	{
		ERR("calendar_record_get_int() failed");
		return ret;
	}
	ret = calendar_record_get_int(alarm, _calendar_alarm.tick, &tick);
	if (CALENDAR_ERROR_NONE != ret)
	{
		ERR("calendar_record_get_int() failed");
		return ret;
	}

	switch (unit)
	{
	case CALENDAR_ALARM_TIME_UNIT_SPECIFIC:
		ret = calendar_record_get_lli(alarm, _calendar_alarm.time, &utime);
		if (CALENDAR_ERROR_NONE != ret)
		{
			ERR("calendar_record_get_lli() failed");
			return ret;
		}
		break;

	default:
		switch (type)
		{
		case CALENDAR_RECORD_TYPE_EVENT:
			ret = calendar_record_get_caltime(record, _calendar_event.start_time, &caltime);
			if (CALENDAR_ERROR_NONE != ret)
			{
				ERR("calendar_record_get_caltime() failed");
				return ret;
			}
			switch (caltime.type)
			{
			case CALENDAR_TIME_UTIME:
				utime = caltime.time.utime - (tick * unit);
				break;

			case CALENDAR_TIME_LOCALTIME:
				utime = _cal_time_convert_itol(NULL, caltime.time.date.year,
						caltime.time.date.month, caltime.time.date.mday, 0, 0, 0);
				utime -= (tick * unit);
				break;
			}
			break;

		case CALENDAR_RECORD_TYPE_TODO:
			ret = calendar_record_get_caltime(record, _calendar_todo.due_time, &caltime);
			if (CALENDAR_ERROR_NONE != ret)
			{
				ERR("calendar_record_get_caltime() failed");
				return ret;
			}
			switch (caltime.type)
			{
			case CALENDAR_TIME_UTIME:
				utime = caltime.time.utime - (tick * unit);
				break;

			case CALENDAR_TIME_LOCALTIME:
				utime = _cal_time_convert_itol(NULL, caltime.time.date.year,
						caltime.time.date.month, caltime.time.date.mday, 0, 0, 0);
				utime -= (tick * unit);
				break;
			}
			break;
		}
		break;
	}

	char *calstr = _cal_time_convert_ltos(NULL, utime);
	DBG("[%s]", calstr);

	ret = __cal_vcalendar_make_printf(b, "AALARM:", calstr);
	CAL_FREE(calstr);

	return CALENDAR_ERROR_NONE;
}

int __cal_vcalendar_make_alarm(cal_make_s *b, calendar_record_h alarm)
{
	int ret;
	retvm_if(alarm == NULL, CALENDAR_ERROR_INVALID_PARAMETER,
			"Invalid argument: alarm is NULL");

	// TODO : No action type is defined
	ret = __cal_vcalendar_make_printf(b, "BEGIN:VALARM", NULL);
	retv_if(ret != CALENDAR_ERROR_NONE, ret);

	ret = __cal_vcalendar_make_audio(b, alarm);
	retv_if(ret != CALENDAR_ERROR_NONE, ret);

	// TODO : Display
	// TODO : Email
	// TODO : Proc

	__cal_vcalendar_make_printf(b, "END:VALARM", NULL);
	return CALENDAR_ERROR_NONE;
}

int __cal_vcalendar_make_rrule_append_mday(char *buf, char *mday)
{
	int i;
	int num;
	int length = 0;
	char **t;
	char *p;

	if (NULL == buf || NULL == mday)
	{
		ERR("Invalid parameter");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	t = g_strsplit(mday, ",", -1);
	if (!t) {
		ERR("g_strsplit failed");
		g_strfreev(t);
		return CALENDAR_ERROR_OUT_OF_MEMORY;
	}
	length = g_strv_length(t);
	for (i = 0; i < length; i++)
	{
		p = t[i];
		while (*p == ' ')
		{
			p++;
		}
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

int __cal_vcalendar_make_rrule_append_wday(char *buf, char *wday)
{
	int i, j;
	int num, num_past;
	int length = 0;
	char **t;
	char *p;
	char buf_temp[8] = {0};

	num_past = 0;
	t = g_strsplit(wday, ",", -1);
	if (!t) {
		ERR("g_strsplit failed");
		g_strfreev(t);
		return CALENDAR_ERROR_OUT_OF_MEMORY;
	}
	length = g_strv_length(t);
	for (i = 0; i < length; i++)
	{
		p = t[i];
		while (*p == ' ') // del space
		{
			p++;
		}
		j = 0; // get number
		while (p[j] == '+' || p[j] == '-' || (p[j] >= '1' && p[j] <= '9'))
		{
			j++;
		}
		if (j > 0)
		{
			if (*p == '-')
			{
				snprintf(buf_temp, j, "%s", p + 1);
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
					strcat(buf, " ");
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
	}
	g_strfreev(t);

	return CALENDAR_ERROR_NONE;
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
			until_str = _cal_time_convert_ltos(NULL, caltime.time.utime);
			snprintf(buf_range, sizeof(buf_range), "%s", until_str);
			CAL_FREE(until_str);
			break;

		case CALENDAR_TIME_LOCALTIME:
			snprintf(buf_range, sizeof(buf_range), "%04d%02d%02dT000000Z",
					caltime.time.date.year, caltime.time.date.month, caltime.time.date.mday);
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

static int __cal_vcalendar_make_rrule_ver1(cal_make_s *b, calendar_record_h record)
{
	int ret;
	int freq;
	int interval = 0;
	char *byyearday = NULL;
	char *bymonth = NULL;
	char *bymonthday = NULL;
	char *byday = NULL;
	char buf[512] = {0};

	ret = calendar_record_get_int(record, _calendar_event.freq, &freq);
	if (CALENDAR_ERROR_NONE != ret)
	{
		ERR("calendar_record_get_int() failed");
		return ret;
	}
	ret = calendar_record_get_int(record, _calendar_event.interval, &interval);
	if (CALENDAR_ERROR_NONE != ret)
	{
		ERR("calendar_record_get_int() failed");
		return ret;
	}
	ret = calendar_record_get_str_p(record, _calendar_event.bymonthday, &byyearday);
	if (CALENDAR_ERROR_NONE != ret)
	{
		ERR("calendar_record_get_str_p() failed");
		return ret;
	}
	ret = calendar_record_get_str_p(record, _calendar_event.bymonth, &bymonth);
	if (CALENDAR_ERROR_NONE != ret)
	{
		ERR("calendar_record_get_str_p() failed");
		return ret;
	}
	ret = calendar_record_get_str_p(record, _calendar_event.byday, &byday);
	if (CALENDAR_ERROR_NONE != ret)
	{
		ERR("calendar_record_get_str_p() failed");
		return ret;
	}
	ret = calendar_record_get_str_p(record, _calendar_event.bymonthday, &bymonthday);
	if (CALENDAR_ERROR_NONE != ret)
	{
		ERR("calendar_record_get_str_p() failed");
		return ret;
	}

	switch (freq) {
	case CALENDAR_RECURRENCE_DAILY:
		snprintf(buf, sizeof(buf), "D%d ", interval);
		break;

	case CALENDAR_RECURRENCE_WEEKLY:
		snprintf(buf, sizeof(buf), "W%d ", interval);
		__cal_vcalendar_make_rrule_append_wday(buf, byday);
		break;

	case CALENDAR_RECURRENCE_MONTHLY:
		if (byday)
		{
			snprintf(buf, sizeof(buf), "MP%d ", interval);
			__cal_vcalendar_make_rrule_append_wday(buf, byday);
		}
		else if (bymonthday)
		{
			snprintf(buf, sizeof(buf), "MD%d ", interval);
			__cal_vcalendar_make_rrule_append_mday(buf, bymonthday);
		}
		else
		{
			ERR("Invalid parameter");
		}
		break;

	case CALENDAR_RECURRENCE_YEARLY:
		if (bymonth)
		{
			snprintf(buf, sizeof(buf), "YM%d ", interval);
			__cal_vcalendar_make_rrule_append_mday(buf, bymonth);
		}
		else if (byyearday)
		{
			snprintf(buf, sizeof(buf), "YD%d ", interval);
			__cal_vcalendar_make_rrule_append_mday(buf, byyearday);
		}
		else
		{
			ERR("Invalid parameter");
		}
		break;

	default:
		ERR("Invalid parameter");
		break;
	}

	__cal_vcalendar_make_rrule_append_until(buf, record);

	return __cal_vcalendar_make_printf(b, "RRULE:", buf);
}

static int __cal_vcalendar_make_rrule(cal_make_s *b, int freq, calendar_record_h record)
{
	int ret;
	int num;
	char *text = NULL;
	char buf[1024] = {0};
	char tmp[32] = {0};
	calendar_time_s caltime = {0};

	switch (freq) {
	case CALENDAR_RECURRENCE_DAILY:
		strcat(buf, "FREQ=DAILY");
		break;
	case CALENDAR_RECURRENCE_WEEKLY:
		strcat(buf, "FREQ=WEEKLY");
		break;
	case CALENDAR_RECURRENCE_MONTHLY:
		strcat(buf, "FREQ=MONTHLY");
		break;
	case CALENDAR_RECURRENCE_YEARLY:
		strcat(buf, "FREQ=YEARLY");
		break;
	default:
		break;
	}

	ret = calendar_record_get_int(record, _calendar_event.interval, &num);
	snprintf(tmp, sizeof(tmp), "%d", num);
	strcat(buf, ";INTERVAL=");
	strcat(buf, tmp);

	ret = calendar_record_get_str_p(record, _calendar_event.bysecond, &text);
	if (text) {
		strcat(buf, ";BYSECOND=");
		strcat(buf, text);
		text = NULL;
	}

	ret = calendar_record_get_str_p(record, _calendar_event.byminute, &text);
	if (text) {
		strcat(buf, ";BYMINUTE=");
		strcat(buf, text);
		text = NULL;
	}

	ret = calendar_record_get_str_p(record, _calendar_event.byhour, &text);
	if (text) {
		strcat(buf, ";BYHOUR=");
		strcat(buf, text);
		text = NULL;
	}

	ret = calendar_record_get_str_p(record, _calendar_event.byday, &text);
	if (text) {
		strcat(buf, ";BYDAY=");
		strcat(buf, text);
		text = NULL;
	}

	ret = calendar_record_get_str_p(record, _calendar_event.bymonthday, &text);
	if (text) {
		strcat(buf, ";BYMONTHDAY=");
		strcat(buf, text);
		text = NULL;
	}

	ret = calendar_record_get_str_p(record, _calendar_event.byyearday, &text);
	if (text) {
		strcat(buf, ";BYYEARDAY=");
		strcat(buf, text);
		text = NULL;
	}

	ret = calendar_record_get_str_p(record, _calendar_event.byweekno, &text);
	if (text) {
		strcat(buf, ";BYWEEKNO=");
		strcat(buf, text);
		text = NULL;
	}

	ret = calendar_record_get_str_p(record, _calendar_event.bymonth, &text);
	if (text) {
		strcat(buf, ";BYMONTH=");
		strcat(buf, text);
		text = NULL;
	}

	ret = calendar_record_get_str_p(record, _calendar_event.bysetpos, &text);
	if (text) {
		strcat(buf, ";BYSETPOS=");
		strcat(buf, text);
		text = NULL;
	}

	num = CALENDAR_SUNDAY; // default set
	ret = calendar_record_get_int(record, _calendar_event.wkst, &num);
	strcat(buf, ";WKST=");
	switch (num) {
	case CALENDAR_SUNDAY:
		strcat(buf, "SU");
		break;
	case CALENDAR_MONDAY:
		strcat(buf, "MO");
		break;
	case CALENDAR_TUESDAY:
		strcat(buf, "TU");
		break;
	case CALENDAR_WEDNESDAY:
		strcat(buf, "WE");
		break;
	case CALENDAR_THURSDAY:
		strcat(buf, "TH");
		break;
	case CALENDAR_FRIDAY:
		strcat(buf, "FR");
		break;
	case CALENDAR_SATURDAY:
		strcat(buf, "SA");
		break;
	}

	ret = calendar_record_get_int(record, _calendar_event.range_type, &num);
	switch (num) {
	case CALENDAR_RANGE_COUNT:
		ret = calendar_record_get_int(record, _calendar_event.count, &num);
		snprintf(tmp, sizeof(tmp), "%d", num);
		strcat(buf, ";COUNT=");
		strcat(buf, tmp);
		break;

	case CALENDAR_RANGE_UNTIL:
		ret = calendar_record_get_caltime(record, _calendar_event.until_time, &caltime);

		if (caltime.type == CALENDAR_TIME_UTIME)
		{
		    char *tmp_tzid = NULL;
		    tmp_tzid = _cal_time_convert_ltos(NULL, caltime.time.utime);
		    if (tmp_tzid)
		    {
		        snprintf(tmp, sizeof(tmp), "%s", tmp_tzid);
		        CAL_FREE(tmp_tzid);
		    }
		}
		else
		{
			snprintf(tmp, sizeof(tmp), "%04d%02d%02dT000000Z",
					caltime.time.date.year,
					caltime.time.date.month,
					caltime.time.date.mday);
		}
		strcat(buf, ";UNTIL=");
		strcat(buf, tmp);
		break;

	case CALENDAR_RANGE_NONE:
		break;
	default:
		break;
	}
	return __cal_vcalendar_make_printf(b, "RRULE:", buf);
}
/////////////////////////////////////////////////////////////////////////////

int __cal_vcalendar_make_child_extended(cal_make_s *b, calendar_record_h child)
{
	int ret;
	char *key = NULL;
	char *value = NULL;
	if (NULL == child)
	{
		ERR("Invalid argument: child is NULL");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	ret = calendar_record_get_str_p(child, _calendar_extended_property.key, &key);
	if (CALENDAR_ERROR_NONE != ret)
	{
		ERR("calendar_record_get_str_p() failed");
		return ret;
	}
	if (NULL == key || strncmp(key, "X-", strlen("X-")))
	{
		DBG("Not extended for vcalendar[%s]", key);
		return CALENDAR_ERROR_NONE;
	}

	ret = calendar_record_get_str_p(child, _calendar_extended_property.value, &value);
	if (CALENDAR_ERROR_NONE != ret)
	{
		ERR("calendar_record_get_str_p() failed");
		return ret;
	}
	ret = __cal_vcalendar_make_printf(b, key, value);
	if (CALENDAR_ERROR_NONE != ret)
	{
		ERR("__cal_vcalendar_make_printf() failed");
		return ret;
	}

	return CALENDAR_ERROR_NONE;
}

int __cal_vcalendar_make_printf_str_p(calendar_record_h record, unsigned int property_id, cal_make_s *b, const char *property_str)
{
	int ret;
	char *strval = NULL;

	ret = calendar_record_get_str_p(record, property_id, &strval);
	if (CALENDAR_ERROR_NONE != ret)
	{
		ERR("calendar_record_get_str_p() failed(ret:%d): categories", ret);
		return ret;
	}
	if (strval && (strlen(strval) > 0))
	{
		ret = __cal_vcalendar_make_printf(b, property_str, strval);
		if (CALENDAR_ERROR_NONE != ret)
		{
			ERR("__cal_vcalendar_make_printf() failed(ret:%d)", ret);
			return ret;
		}
	}
	return CALENDAR_ERROR_NONE;
}

int __cal_vcalendar_make_schedule(int version, cal_make_s *b, calendar_record_h record)
{
	int ret;
	int freq;
	int intval;
	char *strval = NULL;
	char *strval2 = NULL;
	calendar_time_s caltime = {0};
	char *uri = NULL;
	char *tzid = NULL;

	ret = calendar_record_get_uri_p(record, &uri);

	//_cal_db_rrule_fill_record(record);

	ret = __cal_vcalendar_make_printf(b, "BEGIN:VEVENT", NULL);
	retv_if(ret != CALENDAR_ERROR_NONE, ret);

	// sensitivity
	ret = calendar_record_get_int(record, _calendar_event.sensitivity, &intval);
	retvm_if(ret != CALENDAR_ERROR_NONE, -1,
			"Failed to get sensitivity(%d)", ret);

	__cal_vcalendar_make_class(b, intval);
	retv_if(ret != CALENDAR_ERROR_NONE, ret);

	// busy_status
	ret = calendar_record_get_int(record, _calendar_event.busy_status, &intval);
	retvm_if(ret != CALENDAR_ERROR_NONE, -1,
			"Failed to get busy_status(%d)", ret);

	ret = __cal_vcalendar_make_transp(b, intval);
	retv_if(ret != CALENDAR_ERROR_NONE, ret);

	// dtstart_type
	ret = calendar_record_get_str_p(record, _calendar_event.start_tzid, &tzid);
	ret = calendar_record_get_caltime(record, _calendar_event.start_time, &caltime);
	if (ret != CALENDAR_ERROR_NONE) {
		ERR("Failed to get start_time(%d)", ret);
		return -1;
	}
	ret = __cal_vcalendar_make_dtstart(b, tzid, &caltime);
	retvm_if(ret != CALENDAR_ERROR_NONE, -1, "Failed to get dtstart(%d)", ret);


	// created_time
/* keis want to skip for there potential error.
	ret = calendar_record_get_lli(record, _calendar_event.created_time, &llival);
	retvm_if(ret != CALENDAR_ERROR_NONE, -1,
			"Failed to get created_time(%d)", ret);

	ret = __cal_vcalendar_make_created(b, llival);
	retv_if(ret != CALENDAR_ERROR_NONE, ret);
*/

	// description
	__cal_vcalendar_make_printf_str_p(record, _calendar_event.description, b, "DESCRIPTION:");

	// rrule
	ret = calendar_record_get_int(record, _calendar_event.freq, &freq);
	retvm_if(ret != CALENDAR_ERROR_NONE, ret,
			"Failed to get last_modified_time(%d)", ret);
	if (freq)
	{
		switch (version)
		{
		case 1:
			ret = __cal_vcalendar_make_rrule_ver1(b, record);
			break;

		default:
			__cal_vcalendar_make_rrule(b, freq, record);
		}
	}

	// last_mod
/* keis want to skip for there potential error.
	ret = calendar_record_get_lli(record, _calendar_event.last_modified_time, &llival);
	retvm_if(ret != CALENDAR_ERROR_NONE, -1,
			"Failed to get last_modified_time(%d)", ret);

	ret = __cal_vcalendar_make_last_mod(b, llival);
	retv_if(ret != CALENDAR_ERROR_NONE, ret);
*/
	// location
	__cal_vcalendar_make_printf_str_p(record, _calendar_event.location, b, "LOCATION:");

	// organizer email
	strval = NULL;
	strval2 = NULL;
	ret = calendar_record_get_str_p(record, _calendar_event.organizer_name, &strval);
	retvm_if(ret != CALENDAR_ERROR_NONE, -1,
			"Failed to get organizer_name(%d)", ret);

	ret = calendar_record_get_str_p(record, _calendar_event.organizer_email, &strval2);
	retvm_if(ret != CALENDAR_ERROR_NONE, -1,
			"Failed to get organizer_email(%d)", ret);

	if (strval || strval2) {
		ret = __cal_vcalendar_make_organizer(b, strval, strval2);
		retv_if(ret != CALENDAR_ERROR_NONE, ret);

	} else {
		DBG("No organizer name or email");
	}

	// priority
	ret = calendar_record_get_int(record, _calendar_event.priority, &intval);
	retvm_if(ret != CALENDAR_ERROR_NONE, -1,
			"Failed to get priority(%d)", ret);

	DBG("priority(%d)", intval);
	ret = __cal_vcalendar_make_priority(b, intval);
	retv_if(ret != CALENDAR_ERROR_NONE, ret);

	// TODO : seq

	// dtstamp
/* keis want to skip for there potential error.
	ret = __cal_vcalendar_make_dtstamp(b, tzid);
	retv_if(ret != CALENDAR_ERROR_NONE, ret);
*/

	// summary
	__cal_vcalendar_make_printf_str_p(record, _calendar_event.summary, b, "SUMMARY:");

	// dtend
	ret = calendar_record_get_caltime(record, _calendar_event.end_time, &caltime);
	retvm_if(ret != CALENDAR_ERROR_NONE, -1,
			"Failed to get end_time(%d)", ret);

	ret = __cal_vcalendar_make_dtend(b, tzid, &caltime);
	retvm_if(ret != CALENDAR_ERROR_NONE, -1, "Failed to get end(%d)", ret);

	// categories
	__cal_vcalendar_make_printf_str_p(record, _calendar_event.categories, b, "CATEGORIES:");

	// uid
	__cal_vcalendar_make_printf_str_p(record, _calendar_event.uid, b, "UID:");

	// exdate
	__cal_vcalendar_make_printf_str_p(record, _calendar_event.exdate, b, "EXDATE:");

	unsigned int count;
	calendar_record_h child = NULL;

	// attendee
	count = 0;
	ret = calendar_record_get_child_record_count(record,
			_calendar_event.calendar_attendee, &count);
	retvm_if(ret != CALENDAR_ERROR_NONE, -1,
			"Failed to get child count(%d)", ret);

	while (count > 0)
	{
		count--;
		ret = calendar_record_get_child_record_at_p(record,
				_calendar_event.calendar_attendee, count, &child);
		retvm_if(ret != CALENDAR_ERROR_NONE, -1,
				"Failed to get child attendee(%d)", ret);

		ret = __cal_vcalendar_make_attendee(b, child);
		retv_if(ret != CALENDAR_ERROR_NONE, ret);
	}
	if (count <= 0)
	{
		DBG("No attendee");
	}

	// alarm
	count = 0;
	switch (version)
	{
	case 1:
		// In ver 1.0, only first alarm will be dealt with.
		ret = calendar_record_get_child_record_count(record, _calendar_event.calendar_alarm, &count);
		retvm_if(ret != CALENDAR_ERROR_NONE, -1, "Failed to get child count(%d)", ret);
		if (count > 0)
		{
			ret = calendar_record_get_child_record_at_p(record, _calendar_event.calendar_alarm, 0, &child);
			retvm_if(ret != CALENDAR_ERROR_NONE, -1, "Failed to get child alarm(%d)", ret);

			ret = __cal_vcalendar_make_aalarm(b, record, child);
			retv_if(ret != CALENDAR_ERROR_NONE, ret);
		}
		else
		{
			DBG("No alarm in ver1.0");
		}
		break;

	default:
		ret = calendar_record_get_child_record_count(record, _calendar_event.calendar_alarm, &count);
		retvm_if(ret != CALENDAR_ERROR_NONE, -1, "Failed to get child count(%d)", ret);

		while (count > 0)
		{
			count--;
			ret = calendar_record_get_child_record_at_p(record, _calendar_event.calendar_alarm, count, &child);
			retvm_if(ret != CALENDAR_ERROR_NONE, -1, "Failed to get child alarm(%d)", ret);

			ret = __cal_vcalendar_make_alarm(b, child);
			retv_if(ret != CALENDAR_ERROR_NONE, ret);
		}

		if (count <= 0)
		{
			DBG("No alarm in ver2.0");
		}
	}

	// extended
	count = 0;
	ret = calendar_record_get_child_record_count(record, _calendar_event.extended, &count);
	retvm_if(ret != CALENDAR_ERROR_NONE, -1, "Failed to get child count(%d)", ret);

	while (count > 0)
	{
		count--;
		ret = calendar_record_get_child_record_at_p(record, _calendar_event.extended, count, &child);
		retvm_if(ret != CALENDAR_ERROR_NONE, -1, "Failed to get child alarm(%d)", ret);

		ret = __cal_vcalendar_make_child_extended(b, child);
		retv_if(ret != CALENDAR_ERROR_NONE, ret);
	}

	if (count <= 0)
	{
		DBG("No extended");
	}

	return __cal_vcalendar_make_printf(b, "END:VEVENT", NULL);
}

int __cal_vcalendar_make_todo(int version, cal_make_s *b, calendar_record_h record)
{
	int ret;
	int intval;
	calendar_time_s caltime = {0};
	char *uri = NULL;
	char *tzid = NULL;

	ret = calendar_record_get_uri_p(record, &uri);

	//_cal_db_rrule_fill_record(record);

	ret = __cal_vcalendar_make_printf(b, "BEGIN:VTODO", NULL);
	retv_if(ret != CALENDAR_ERROR_NONE, ret);

	// sensitivity
	ret = calendar_record_get_int(record, _calendar_todo.sensitivity, &intval);
	retvm_if(ret != CALENDAR_ERROR_NONE, -1,
			"Failed to get sensitivity(%d)", ret);

	__cal_vcalendar_make_class(b, intval);
	retv_if(ret != CALENDAR_ERROR_NONE, ret);

	// dtstart_type
	ret = calendar_record_get_str_p(record, _calendar_todo.start_tzid, &tzid);
	ret = calendar_record_get_caltime(record, _calendar_todo.start_time, &caltime);
	if (ret != CALENDAR_ERROR_NONE) {
		ERR("Failed to get start_time(%d)", ret);
		return -1;
	}
	ret = __cal_vcalendar_make_dtstart(b, tzid, &caltime);
	retvm_if(ret != CALENDAR_ERROR_NONE, -1,
			"Failed to get dtstart(%d)", ret);

	// created_time
/* keis want to skip for there potential error.
	ret = calendar_record_get_lli(record, _calendar_todo.created_time, &llival);
	retvm_if(ret != CALENDAR_ERROR_NONE, -1,
			"Failed to get created_time(%d)", ret);

	ret = __cal_vcalendar_make_created(b, llival);
	retv_if(ret != CALENDAR_ERROR_NONE, ret);
*/

	// description
	__cal_vcalendar_make_printf_str_p(record, _calendar_todo.description, b, "DESCRIPTION:");


	// TODO : geo

	// last_mod
/* keis want to skip for there potential error.
	ret = calendar_record_get_lli(record, _calendar_todo.last_modified_time, &llival);
	retvm_if(ret != CALENDAR_ERROR_NONE, -1,
			"Failed to get last_modified_time(%d)", ret);

	ret = __cal_vcalendar_make_last_mod(b, llival);
	retv_if(ret != CALENDAR_ERROR_NONE, ret);
*/

	// location
	__cal_vcalendar_make_printf_str_p(record, _calendar_todo.location, b, "LOCATION:");

	// priority
	ret = calendar_record_get_int(record, _calendar_todo.priority, &intval);
	retvm_if(ret != CALENDAR_ERROR_NONE, -1,
			"Failed to get priority(%d)", ret);

	DBG("priority(%d)", intval);
	ret = __cal_vcalendar_make_priority(b, intval);
	retv_if(ret != CALENDAR_ERROR_NONE, ret);

	// TODO : seq

	// dtstamp
/* keis want to skip for there potential error.
	ret = __cal_vcalendar_make_dtstamp(b, tzid);
	retv_if(ret != CALENDAR_ERROR_NONE, ret);
*/

	// summary
	__cal_vcalendar_make_printf_str_p(record, _calendar_todo.summary, b, "SUMMARY:");

	// dtend
	ret = calendar_record_get_caltime(record, _calendar_todo.due_time, &caltime);
	retvm_if(ret != CALENDAR_ERROR_NONE, -1,
			"Failed to get due_time(%d)", ret);

	ret = __cal_vcalendar_make_due(b, tzid, &caltime);
	retvm_if(ret != CALENDAR_ERROR_NONE, -1, "__cal_vcalendar_make_due() failed(%d)", ret);

	// categories
	__cal_vcalendar_make_printf_str_p(record, _calendar_todo.categories, b, "CATEGORIES:");

	// uid
	__cal_vcalendar_make_printf_str_p(record, _calendar_todo.uid, b, "UID:");

	// alarm
	unsigned int count;
	calendar_record_h child = NULL;

	switch (version)
	{
	case 1:
		// In ver 1.0, only first alarm will be dealt with.
		ret = calendar_record_get_child_record_count(record, _calendar_todo.calendar_alarm, &count);
		retvm_if(ret != CALENDAR_ERROR_NONE, -1, "Failed to get child count(%d)", ret);
		if (count > 0)
		{
			ret = calendar_record_get_child_record_at_p(record, _calendar_todo.calendar_alarm, 0, &child);
			retvm_if(ret != CALENDAR_ERROR_NONE, -1, "Failed to get child alarm(%d)", ret);

			ret = __cal_vcalendar_make_aalarm(b, record, child);
			retv_if(ret != CALENDAR_ERROR_NONE, ret);
		}
		else
		{
			DBG("No alarm in ver1.0");
		}
		break;

	default:
		ret = calendar_record_get_child_record_count(record, _calendar_todo.calendar_alarm, &count);
		retvm_if(ret != CALENDAR_ERROR_NONE, -1, "Failed to get child count(%d)", ret);

		while (count > 0)
		{
			count--;
			ret = calendar_record_get_child_record_at_p(record, _calendar_todo.calendar_alarm, count, &child);
			retvm_if(ret != CALENDAR_ERROR_NONE, -1, "Failed to get child alarm(%d)", ret);

			ret = __cal_vcalendar_make_alarm(b, child);
			retv_if(ret != CALENDAR_ERROR_NONE, ret);
		}

		if (count <= 0)
		{
			DBG("No alarm in ver2.0");
		}
	}

	// extended
	count = 0;
	ret = calendar_record_get_child_record_count(record, _calendar_todo.extended, &count);
	retvm_if(ret != CALENDAR_ERROR_NONE, -1, "Failed to get child count(%d)", ret);

	while (count > 0)
	{
		count--;
		ret = calendar_record_get_child_record_at_p(record, _calendar_todo.extended, count, &child);
		retvm_if(ret != CALENDAR_ERROR_NONE, -1, "Failed to get child alarm(%d)", ret);

		ret = __cal_vcalendar_make_child_extended(b, child);
		retv_if(ret != CALENDAR_ERROR_NONE, ret);
	}
	if (count <= 0)
	{
		DBG("No extended");
	}

	return __cal_vcalendar_make_printf(b, "END:VTODO", NULL);
}

int __cal_vcalendar_make_parent_extended(cal_make_s *b, calendar_list_h list, int *has_extended, int *version)
{
	int ret = CALENDAR_ERROR_NONE;
	GList *l = NULL;
	cal_list_s *cal_list = (cal_list_s *)list;

	if (NULL == list)
	{
		ERR("Invalid parameter: list is NULL");
		return CALENDAR_ERROR_DB_FAILED;
	}

	l = g_list_first(cal_list->record);
	while (l)
	{
		char *uri = NULL;
		calendar_record_h record = (calendar_record_h)l->data;
		calendar_record_get_uri_p(record, &uri);
		if (strncmp(uri, _calendar_extended_property._uri, strlen(_calendar_extended_property._uri)))
		{
			l = g_list_next(l);
			continue;
		}

		*has_extended = 1;
		cal_extended_s *extended = (cal_extended_s *)record;
		if (NULL == extended)
		{
			ERR("extended is NULL");
			return CALENDAR_ERROR_DB_FAILED;
		}
		ret = __cal_vcalendar_make_printf(b, extended->key, extended->value);
		DBG("extended key[%s] value[%s]", extended->key, extended->value);
		if (!strncmp(extended->key, "VERSION", strlen("VERSION")))
		{
			if (strstr(extended->value, "1.0"))
			{
				*version = 1;
			}
			else
			{
				*version = 2;
			}

			DBG("version (%s)", extended->value);
			break;
		}
		l = g_list_next(l);
	}
	return CALENDAR_ERROR_NONE;
}

int _cal_vcalendar_make_vcalendar(cal_make_s *b, calendar_list_h list)
{
	int ret;
	int version = 0;
	int has_extended = 0;
	char *uri = NULL;
	calendar_record_h record;

	ret = __cal_vcalendar_make_printf(b, "BEGIN:VCALENDAR", NULL);
	retv_if(ret != CALENDAR_ERROR_NONE, ret);

	ret = __cal_vcalendar_make_parent_extended(b, list, &has_extended, &version);
	retv_if(ret != CALENDAR_ERROR_NONE, ret);

	if (has_extended == 0)
	{
/* keis want to skip for there potential error.
		ret = __cal_vcalendar_make_printf(b, "CALSCALE:GREGORIAN", NULL);
		retv_if(ret != CALENDAR_ERROR_NONE, ret);

		ret = __cal_vcalendar_make_printf(b, "PRODID:-//Samsung Electronics//Calendar//EN", NULL);
		retv_if(ret != CALENDAR_ERROR_NONE, ret);
*/

		ret = __cal_vcalendar_make_printf(b, "VERSION:2.0", NULL);
		retv_if(ret != CALENDAR_ERROR_NONE, ret);
	}

	ret = calendar_list_first(list);
	retvm_if(ret != CALENDAR_ERROR_NONE, ret,
			"calendar_list_first() Failed");

	do
	{
		ret = calendar_list_get_current_record_p(list, &record);
		if (ret != CALENDAR_ERROR_NONE)
		{
			ERR("Failed to get current record(%d)", ret);
			break;
		}
		ret = calendar_record_get_uri_p(record, &uri);
		if (!strcmp(uri, CALENDAR_VIEW_EVENT))
		{
			ret = __cal_vcalendar_make_schedule(version, b, record);

		}
		else if (!strcmp(uri, CALENDAR_VIEW_TODO))
		{
			ret = __cal_vcalendar_make_todo(version, b, record);

		}

		if (ret != CALENDAR_ERROR_NONE)
		{
			ERR("__cal_vcalendar_make_schedule() Failed(%d)", ret);
			break;
		}
	} while(calendar_list_next(list) != CALENDAR_ERROR_NO_DATA);

	__cal_vcalendar_make_printf(b, "END:VCALENDAR", NULL);

	return CALENDAR_ERROR_NONE;
}

