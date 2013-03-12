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

static inline int __cal_vcalendar_make_description(cal_make_s *b, char *s)
{
	__cal_vcalendar_make_printf(b, "DESCRIPTION:", s);
	return CALENDAR_ERROR_NONE;
}

static inline int __cal_vcalendar_make_dtstart(cal_make_s *b, calendar_time_s *caltime)
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
            char* tmp_tzid = NULL;
            tmp_tzid = _cal_time_convert_ltos(NULL, caltime->time.utime);
            if (tmp_tzid)
            {
                __cal_vcalendar_make_printf(b, "DTSTART:", tmp_tzid);
                CAL_FREE(tmp_tzid);
            }
        }
		break;

	case CALENDAR_TIME_LOCALTIME:
		__cal_vcalendar_make_printf(b, "DTSTART:",
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

static inline int __cal_vcalendar_make_location(cal_make_s *b, char *s)
{
	__cal_vcalendar_make_printf(b, "LOCATION:", s);
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

static inline int __cal_vcalendar_make_summary(cal_make_s *b, char *s)
{
	__cal_vcalendar_make_printf(b, "SUMMARY:", s);
	return CALENDAR_ERROR_NONE;
}

static inline int __cal_vcalendar_make_dtend(cal_make_s *b, calendar_time_s *caltime)
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
            char *tmp_tzid = NULL;
            tmp_tzid = _cal_time_convert_ltos(NULL, caltime->time.utime);
            if (tmp_tzid)
            {
                __cal_vcalendar_make_printf(b, "DTEND:", tmp_tzid);
                CAL_FREE(tmp_tzid);
            }
        }
		break;

	case CALENDAR_TIME_LOCALTIME:
		__cal_vcalendar_make_printf(b, "DTEND:",
				__cal_vcalendar_make_itos(caltime->time.date.year,
					caltime->time.date.month, caltime->time.date.mday));
		break;
	}

	return CALENDAR_ERROR_NONE;
}

static inline int __cal_vcalendar_make_due(cal_make_s *b, calendar_time_s *caltime)
{
	retvm_if(caltime == NULL, -1, "Invalid argument: calendar_time_s is NULL");

	switch (caltime->type)
	{
	case CALENDAR_TIME_UTIME:
        {
            char *tmp_tzid = NULL;
            tmp_tzid = _cal_time_convert_ltos(NULL, caltime->time.utime);
            if (tmp_tzid)
            {
                __cal_vcalendar_make_printf(b, "DUE:", tmp_tzid);
                CAL_FREE(tmp_tzid);
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

	ret = calendar_record_get_str(record, _calendar_event.bysecond, &text);
	if (text) {
		strcat(buf, ";BYSECOND=");
		strcat(buf, text);
		CAL_FREE(text);
		text = NULL;
	}

	ret = calendar_record_get_str(record, _calendar_event.byminute, &text);
	if (text) {
		strcat(buf, ";BYMINUTE=");
		strcat(buf, text);
		CAL_FREE(text);
		text = NULL;
	}

	ret = calendar_record_get_str(record, _calendar_event.byhour, &text);
	if (text) {
		strcat(buf, ";BYHOUR=");
		strcat(buf, text);
		CAL_FREE(text);
		text = NULL;
	}

	ret = calendar_record_get_str(record, _calendar_event.byday, &text);
	if (text) {
		strcat(buf, ";BYDAY=");
		strcat(buf, text);
		CAL_FREE(text);
		text = NULL;
	}

	ret = calendar_record_get_str(record, _calendar_event.bymonthday, &text);
	if (text) {
		strcat(buf, ";BYMONTHDAY=");
		strcat(buf, text);
		CAL_FREE(text);
		text = NULL;
	}

	ret = calendar_record_get_str(record, _calendar_event.byyearday, &text);
	if (text) {
		strcat(buf, ";BYYEARDAY=");
		strcat(buf, text);
		CAL_FREE(text);
		text = NULL;
	}

	ret = calendar_record_get_str(record, _calendar_event.byweekno, &text);
	if (text) {
		strcat(buf, ";BYWEEKNO=");
		strcat(buf, text);
		CAL_FREE(text);
		text = NULL;
	}

	ret = calendar_record_get_str(record, _calendar_event.bymonth, &text);
	if (text) {
		strcat(buf, ";BYMONTH=");
		strcat(buf, text);
		CAL_FREE(text);
		text = NULL;
	}

	ret = calendar_record_get_str(record, _calendar_event.bysetpos, &text);
	if (text) {
		strcat(buf, ";BYSETPOS=");
		strcat(buf, text);
		CAL_FREE(text);
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
int __cal_vcalendar_make_schedule(cal_make_s *b, calendar_record_h record)
{
	int ret;
	int freq;
	int intval;
	long long int llival;
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

	// created_time
	ret = calendar_record_get_str_p(record, _calendar_event.start_tzid, &tzid);
	ret = calendar_record_get_caltime(record, _calendar_event.start_time, &caltime);
	if (ret != CALENDAR_ERROR_NONE) {
		ERR("Failed to get start_time(%d)", ret);
		return -1;
	}

	ret = calendar_record_get_lli(record, _calendar_event.created_time, &llival);
	retvm_if(ret != CALENDAR_ERROR_NONE, -1,
			"Failed to get created_time(%d)", ret);

	ret = __cal_vcalendar_make_created(b, llival);
	retv_if(ret != CALENDAR_ERROR_NONE, ret);


	// description
	strval = NULL;
	ret = calendar_record_get_str_p(record, _calendar_event.description, &strval);
	retvm_if(ret != CALENDAR_ERROR_NONE, -1,
			"Failed to get description(%d)", ret);

	if (strval) {
		ret = __cal_vcalendar_make_description(b, strval);
		retv_if(ret != CALENDAR_ERROR_NONE, ret);

	} else {
		DBG("No description");
	}

	// dtstart_type
	ret = __cal_vcalendar_make_dtstart(b, &caltime);
	retvm_if(ret != CALENDAR_ERROR_NONE, -1,
			"Failed to get dtstart(%d)", ret);

	// TODO : geo

	ret = calendar_record_get_int(record, _calendar_event.freq, &freq);
	DBG("freq(%d)", freq);
	if (freq)
	{
		__cal_vcalendar_make_rrule(b, freq, record);
	}

	// last_mod
	ret = calendar_record_get_lli(record, _calendar_event.last_modified_time, &llival);
	retvm_if(ret != CALENDAR_ERROR_NONE, -1,
			"Failed to get last_modified_time(%d)", ret);

	ret = __cal_vcalendar_make_last_mod(b, llival);
	retv_if(ret != CALENDAR_ERROR_NONE, ret);

	// location
	strval = NULL;
	ret = calendar_record_get_str_p(record, _calendar_event.location, &strval);
	retvm_if(ret != CALENDAR_ERROR_NONE, -1,
			"Failed to get location(%d)", ret);

	if (strval) {
		ret = __cal_vcalendar_make_location(b, strval);
		retv_if(ret != CALENDAR_ERROR_NONE, ret);

	} else {
		DBG("No location");
	}

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
	ret = __cal_vcalendar_make_dtstamp(b, tzid);
	retv_if(ret != CALENDAR_ERROR_NONE, ret);

	// summary
	strval = NULL;
	ret = calendar_record_get_str_p(record, _calendar_event.summary, &strval);
	retvm_if(ret != CALENDAR_ERROR_NONE, -1,
			"Failed to get summary(%d)", ret);

	if (strval) {
		ret = __cal_vcalendar_make_summary(b, strval);
		retv_if(ret != CALENDAR_ERROR_NONE, ret);

	} else {
		DBG("No summary");
	}

	// dtend
	ret = calendar_record_get_caltime(record, _calendar_event.end_time, &caltime);
	retvm_if(ret != CALENDAR_ERROR_NONE, -1,
			"Failed to get end_time(%d)", ret);

	ret = __cal_vcalendar_make_dtend(b, &caltime);
	retvm_if(ret != CALENDAR_ERROR_NONE, -1,
			"Failed to get dtstart(%d)", ret);

	// exdate
	strval = NULL;
	ret = calendar_record_get_str_p(record, _calendar_event.exdate, &strval);
	if (CALENDAR_ERROR_NONE != ret)
	{
		ERR("calendar_record_get_str_p() failed(ret:%d): exdate", ret);
		return ret;
	}
	if (strval)
	{
		ret = __cal_vcalendar_make_exdate(b, strval);
		if (CALENDAR_ERROR_NONE != ret)
		{
			ERR("__cal_vcalendar_make_exdate() failed(ret:%d)", ret);
			return ret;
		}
	}

	// attendee
	unsigned int count;
	calendar_record_h child = NULL;

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
	ret = calendar_record_get_child_record_count(record,
			_calendar_event.calendar_alarm, &count);
	retvm_if(ret != CALENDAR_ERROR_NONE, -1,
			"Failed to get child count(%d)", ret);

	while (count > 0)
	{
		count--;
		ret = calendar_record_get_child_record_at_p(record,
				_calendar_event.calendar_alarm, count, &child);
		retvm_if(ret != CALENDAR_ERROR_NONE, -1,
				"Failed to get child alarm(%d)", ret);

		ret = __cal_vcalendar_make_alarm(b, child);
		retv_if(ret != CALENDAR_ERROR_NONE, ret);
	}

	if (count <= 0)
	{
		DBG("No alarm");
	}

	return __cal_vcalendar_make_printf(b, "END:VEVENT", NULL);
}

int __cal_vcalendar_make_todo(cal_make_s *b, calendar_record_h record)
{
	int ret;
	int intval;
	long long int llival;
	char *strval = NULL;
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

	// created_time
	ret = calendar_record_get_str_p(record, _calendar_todo.start_tzid, &tzid);
	ret = calendar_record_get_caltime(record, _calendar_todo.start_time, &caltime);
	if (ret != CALENDAR_ERROR_NONE) {
		ERR("Failed to get start_time(%d)", ret);
		return -1;
	}

	ret = calendar_record_get_lli(record, _calendar_todo.created_time, &llival);
	retvm_if(ret != CALENDAR_ERROR_NONE, -1,
			"Failed to get created_time(%d)", ret);

	ret = __cal_vcalendar_make_created(b, llival);
	retv_if(ret != CALENDAR_ERROR_NONE, ret);


	// description
	strval = NULL;
	ret = calendar_record_get_str_p(record, _calendar_todo.description, &strval);
	retvm_if(ret != CALENDAR_ERROR_NONE, -1,
			"Failed to get description(%d)", ret);

	if (strval) {
		ret = __cal_vcalendar_make_description(b, strval);
		retv_if(ret != CALENDAR_ERROR_NONE, ret);

	} else {
		DBG("No description");
	}

	// dtstart_type
	ret = __cal_vcalendar_make_dtstart(b, &caltime);
	retvm_if(ret != CALENDAR_ERROR_NONE, -1,
			"Failed to get dtstart(%d)", ret);

	// TODO : geo

	// last_mod
	ret = calendar_record_get_lli(record, _calendar_todo.last_modified_time, &llival);
	retvm_if(ret != CALENDAR_ERROR_NONE, -1,
			"Failed to get last_modified_time(%d)", ret);

	ret = __cal_vcalendar_make_last_mod(b, llival);
	retv_if(ret != CALENDAR_ERROR_NONE, ret);

	// location
	strval = NULL;
	ret = calendar_record_get_str_p(record, _calendar_todo.location, &strval);
	retvm_if(ret != CALENDAR_ERROR_NONE, -1,
			"Failed to get location(%d)", ret);

	if (strval) {
		ret = __cal_vcalendar_make_location(b, strval);
		retv_if(ret != CALENDAR_ERROR_NONE, ret);

	} else {
		DBG("No location");
	}

	// priority
	ret = calendar_record_get_int(record, _calendar_todo.priority, &intval);
	retvm_if(ret != CALENDAR_ERROR_NONE, -1,
			"Failed to get priority(%d)", ret);

	DBG("priority(%d)", intval);
	ret = __cal_vcalendar_make_priority(b, intval);
	retv_if(ret != CALENDAR_ERROR_NONE, ret);

	// TODO : seq

	// dtstamp
	ret = __cal_vcalendar_make_dtstamp(b, tzid);
	retv_if(ret != CALENDAR_ERROR_NONE, ret);

	// summary
	strval = NULL;
	ret = calendar_record_get_str_p(record, _calendar_todo.summary, &strval);
	retvm_if(ret != CALENDAR_ERROR_NONE, -1,
			"Failed to get summary(%d)", ret);

	if (strval) {
		ret = __cal_vcalendar_make_summary(b, strval);
		retv_if(ret != CALENDAR_ERROR_NONE, ret);

	} else {
		DBG("No summary");
	}

	// dtend
	ret = calendar_record_get_caltime(record, _calendar_todo.due_time, &caltime);
	retvm_if(ret != CALENDAR_ERROR_NONE, -1,
			"Failed to get due_time(%d)", ret);

	ret = __cal_vcalendar_make_due(b, &caltime);
	retvm_if(ret != CALENDAR_ERROR_NONE, -1,
			"__cal_vcalendar_make_due() failed(%d)", ret);

	// alarm
	unsigned int count;
	calendar_record_h child = NULL;
	ret = calendar_record_get_child_record_count(record,
			_calendar_todo.calendar_alarm, &count);
	retvm_if(ret != CALENDAR_ERROR_NONE, -1,
			"Failed to get child count(%d)", ret);

	while (count > 0)
	{
		count--;
		ret = calendar_record_get_child_record_at_p(record,
				_calendar_todo.calendar_alarm, count, &child);
		retvm_if(ret != CALENDAR_ERROR_NONE, -1,
				"Failed to get child alarm(%d)", ret);

		ret = __cal_vcalendar_make_alarm(b, child);
		retv_if(ret != CALENDAR_ERROR_NONE, ret);
	}

	if (count <= 0)
	{
		DBG("No alarm");
	}

	return __cal_vcalendar_make_printf(b, "END:VTODO", NULL);
}

int _cal_vcalendar_make_vcalendar(cal_make_s *b, calendar_list_h list)
{
	int ret;
	char *uri = NULL;
	calendar_record_h record;

	ret = __cal_vcalendar_make_printf(b, "BEGIN:VCALENDAR", NULL);
	retv_if(ret != CALENDAR_ERROR_NONE, ret);

	ret = __cal_vcalendar_make_printf(b, "CALSCALE:GREGORIAN", NULL);
	retv_if(ret != CALENDAR_ERROR_NONE, ret);

	ret = __cal_vcalendar_make_printf(b, "PRODID:-//Samsung Electronics//Calendar//EN", NULL);
	retv_if(ret != CALENDAR_ERROR_NONE, ret);

	ret = __cal_vcalendar_make_printf(b, "VERSION:2.0", NULL);
	retv_if(ret != CALENDAR_ERROR_NONE, ret);

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
			ret = __cal_vcalendar_make_schedule(b, record);

		}
		else if (!strcmp(uri, CALENDAR_VIEW_TODO))
		{
			ret = __cal_vcalendar_make_todo(b, record);

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

