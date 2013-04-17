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
#include <ctype.h>

#include "calendar_list.h"

#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_record.h"
#include "cal_view.h"
#include "cal_time.h"

#include "cal_vcalendar.h"
#include "cal_vcalendar_parse.h"

enum {
	ENCODE_NONE = 0x0,
	ENCODE_BASE64,
	ENCODE_QUOTED_PRINTABLE,
	ENCODE_MAX,
};

struct _prop_func {
	char *prop;
	int (*func)(int *val, void *data);
};

struct _vcalendar_func {
	char *prop;
	int (*func)(int type, calendar_list_h list, calendar_record_h event, char *prop, char *cont);
};

struct _record_func {
	char *prop;
	int (*func)(calendar_record_h record, void *data);
};

char *_cal_vcalendar_parse_vcalendar(calendar_list_h *list_sch, void *data);
char *_cal_vcalendar_parse_vevent(int type, calendar_list_h *list_sch, void *data);
char *_cal_vcalendar_parse_vtodo(int type, calendar_list_h *list_sch, void *data);
char *_cal_vcalendar_parse_valarm(int type, calendar_record_h record, void *data);

enum {
	VCAL_PRODID = 0x0,
	VCAL_VERSION,
//	VCAL_CALSCALE,
//	VCAL_METHOD,
	VCAL_MAX,
};

static int __cal_vcalendar_parse_prodid(int *val, void *data);
static int __cal_vcalendar_parse_version(int *val, void *data);

struct _prop_func _basic_funcs[VCAL_MAX] =
{
	{"PRODID", __cal_vcalendar_parse_prodid },
	{"VERSION", __cal_vcalendar_parse_version }//,
//	{"CALSCALE", __cal_vcalendar_parse_calscale },
//	{"METHOD", __cal_vcalendar_parse_method }
};

static int __cal_vcalendar_parse_dtstamp(int type, calendar_list_h list, calendar_record_h event, char *prop, char *cont);
static int __cal_vcalendar_parse_uid(int type, calendar_list_h list, calendar_record_h record, char *prop, char *cont);
static int __cal_vcalendar_parse_dtstart(int type, calendar_list_h list, calendar_record_h event, char *prop, char *cont);
static int __cal_vcalendar_parse_created(int type, calendar_list_h list, calendar_record_h event, char *prop, char *cont);
static int __cal_vcalendar_parse_description(int type, calendar_list_h list, calendar_record_h event, char *prop, char *cont);
static int __cal_vcalendar_parse_last_mod(int type, calendar_list_h list, calendar_record_h event, char *prop, char *cont);
static int __cal_vcalendar_parse_location(int type, calendar_list_h list, calendar_record_h event, char *prop, char *cont);
static int __cal_vcalendar_parse_priority(int type, calendar_list_h list, calendar_record_h event, char *prop, char *cont);
static int __cal_vcalendar_parse_status(int type, calendar_list_h list, calendar_record_h event, char *prop, char *cont);
static int __cal_vcalendar_parse_summary(int type, calendar_list_h list, calendar_record_h event, char *prop, char *cont);
static int __cal_vcalendar_parse_rrule(int type, calendar_list_h list, calendar_record_h event, char *prop, char *cont);
static int __cal_vcalendar_parse_dtend(int type, calendar_list_h list, calendar_record_h event, char *prop, char *cont);

static int __cal_vcalendar_parse_completed(int type, calendar_list_h list, calendar_record_h event, char *prop, char *cont);
static int __cal_vcalendar_parse_percent(int type, calendar_list_h list, calendar_record_h event, char *prop, char *cont);

enum {
	ATTENDEE_CUTYPE = 0x0,
	ATTENDEE_MEMBER,
	ATTENDEE_ROLE,
	ATTENDEE_PARTSTAT,
	ATTENDEE_RSVP,
	ATTENDEE_DELTO,
	ATTENDEE_DELFROM,
	ATTENDEE_SENTBY,
	ATTENDEE_CN,
	ATTENDEE_DIR,
	ATTENDEE_MAX,
};

static int __cal_vcalendar_parse_attendee_cutype(calendar_record_h event, void *data);
static int __cal_vcalendar_parse_attendee_member(calendar_record_h event, void *data);
static int __cal_vcalendar_parse_attendee_role(calendar_record_h event, void *data);
static int __cal_vcalendar_parse_attendee_partstat(calendar_record_h event, void *data);
static int __cal_vcalendar_parse_attendee_rsvp(calendar_record_h event, void *data);
static int __cal_vcalendar_parse_attendee_delto(calendar_record_h event, void *data);
static int __cal_vcalendar_parse_attendee_delfrom(calendar_record_h event, void *data);
static int __cal_vcalendar_parse_attendee_sentby(calendar_record_h event, void *data);
static int __cal_vcalendar_parse_attendee_cn(calendar_record_h event, void *data);
static int __cal_vcalendar_parse_attendee_dir(calendar_record_h event, void *data);

struct _record_func _attendee_funcs[ATTENDEE_MAX] =
{
	{ "CUTYPE=", __cal_vcalendar_parse_attendee_cutype },
	{ "MEMBER=", __cal_vcalendar_parse_attendee_member },
	{ "ROLE=", __cal_vcalendar_parse_attendee_role },
	{ "PARTSTAT=", __cal_vcalendar_parse_attendee_partstat },
	{ "RSVP=", __cal_vcalendar_parse_attendee_rsvp },
	{ "DELTO=", __cal_vcalendar_parse_attendee_delto },
	{ "DELFROM=", __cal_vcalendar_parse_attendee_delfrom },
	{ "SENTBY=", __cal_vcalendar_parse_attendee_sentby },
	{ "CN=", __cal_vcalendar_parse_attendee_cn },
	{ "DIR=", __cal_vcalendar_parse_attendee_dir }
};

static int __cal_vcalendar_parse_attendee(int type, calendar_list_h list, calendar_record_h event, char *prop, char *cont);
static int __cal_vcalendar_parse_categories(int type, calendar_list_h list, calendar_record_h event, char *prop, char *cont);
static int __cal_vcalendar_parse_aalarm(int type, calendar_list_h list, calendar_record_h event, char *prop, char *cont);
static int __cal_vcalendar_parse_extended(int type, calendar_list_h list, calendar_record_h record, char *prop, char *cont);

enum {
	VEVE_DTSTAMP = 0x0,
	VEVE_UID,
	VEVE_DTSTART,
	VEVE_CREATED,
	VEVE_DESCRIPTION,
	VEVE_LAST_MOD,
	VEVE_LOCATION,
	VEVE_PRIORITY,
	VEVE_STATUS,
	VEVE_SUMMARY,
	VEVE_RRULE,
	VEVE_DTEND,
	VEVE_DUE,
	VEVE_ATTENDEE,
	VEVE_CATEGORIES,
	VEVE_AALARM,	/* for ver 1.0 */
	VEVE_EXTENDED,
	VEVE_MAX,
};

struct _vcalendar_func _vevent_funcs[VEVE_MAX] =
{
	{ "DTSTAMP", __cal_vcalendar_parse_dtstamp },
	{ "UID", __cal_vcalendar_parse_uid },
	{ "DTSTART", __cal_vcalendar_parse_dtstart },
	{ "CREATED", __cal_vcalendar_parse_created },
	{ "DESCRIPTION", __cal_vcalendar_parse_description },
	{ "LAST-MOD", __cal_vcalendar_parse_last_mod },
	{ "LOCATION", __cal_vcalendar_parse_location },
	{ "PRIORITY", __cal_vcalendar_parse_priority },
	{ "STATUS", __cal_vcalendar_parse_status },
	{ "SUMMARY", __cal_vcalendar_parse_summary },
	{ "RRULE", __cal_vcalendar_parse_rrule },
	{ "DTEND", __cal_vcalendar_parse_dtend },
	{ "DUE", __cal_vcalendar_parse_dtend },
	{ "ATTENDEE", __cal_vcalendar_parse_attendee },
	{ "CATEGORIES", __cal_vcalendar_parse_categories },
	{ "AALARM", __cal_vcalendar_parse_aalarm },
	{ "X-", __cal_vcalendar_parse_extended },
};

static int __cal_vcalendar_parse_action(calendar_record_h alarm, void *data);
static int __cal_vcalendar_parse_trigger(calendar_record_h alarm, void *data);
static int __cal_vcalendar_parse_repeat(calendar_record_h alarm, void *data);
static int __cal_vcalendar_parse_duration_alarm(calendar_record_h alarm, void *data);
static int __cal_vcalendar_parse_attach_alarm(calendar_record_h alarm, void *data);
static int __cal_vcalendar_parse_summary_alarm(calendar_record_h alarm, void *data);

enum {
	VALA_ACTION = 0x0,
	VALA_TRIGGER,
	VALA_REPEAT,
	VALA_DURATION,
	VALA_ATTACH,
//	VALA_DESCRIPTION,
	VALA_SUMMARY,
//	VALA_ATTENDEE,
	VALA_MAX,
};

struct _record_func _valarm_funcs[VALA_MAX] =
{
	{ "ACTION", __cal_vcalendar_parse_action },
	{ "TRIGGER", __cal_vcalendar_parse_trigger },
	{ "REPEAT", __cal_vcalendar_parse_repeat },
	{ "DURATION", __cal_vcalendar_parse_duration_alarm },
	{ "ATTACH", __cal_vcalendar_parse_attach_alarm },
//	{ "DESCRIPTION", __cal_vcalendar_parse_description },
	{ "SUMMARY", __cal_vcalendar_parse_summary_alarm },
//	{ "ATTENDEE", __cal_vcalendar_parse_attendee },
};

enum {
	VTODO_DTSTAMP = 0x0,
	VTODO_UID,
//	VTODO_CLASS,
	VTODO_COMPLETED,
	VTODO_CREATED,
	VTODO_DESCRIPTION,
	VTODO_DTSTART,
//	VTODO_GEO,
	VTODO_LAST_MOD,
	VTODO_LOCATION,
//	VTODO_ORGANIZER,
	VTODO_PERCENT,
	VTODO_PRIORITY,
//	VTODO_RECURID,
//	VTODO_SEQ,
	VTODO_STATUS,
	VTODO_SUMMARY,
//	VTODO_URL,
//	VTODO_RRULE,
	VTODO_DUE,
//	VTODO_DURATION,
//	VTODO_ATTACH,
//	VTODO_ATTENDEE,
//	VTODO_CATEGORIES,
//	VTODO_COMMENT,
//	VTODO_CONTACT,
//	VTODO_EXDATE,
//	VTODO_RSTATUS,
//	VTODO_RELATED,
//	VTODO_RESOURCES,
//	VTODO_RDATE,
//	VTODO_X_PROP,
//	VTODO_IANA_PROP,
	VTODO_AALARM,	/* for ver 1.0 */
	VTODO_EXTENDED,
	VTODO_MAX,
};

struct _vcalendar_func _vtodo_funcs[VTODO_MAX] =
{
	{ "DTSTAMP", __cal_vcalendar_parse_dtstamp },
	{ "UID", __cal_vcalendar_parse_uid },
	{ "COMPLETED", __cal_vcalendar_parse_completed },
	{ "CREATED", __cal_vcalendar_parse_created },
	{ "DESCRIPTION", __cal_vcalendar_parse_description },
	{ "DTSTART", __cal_vcalendar_parse_dtstart },
	{ "LAST-MOD", __cal_vcalendar_parse_last_mod },
	{ "LOCATION", __cal_vcalendar_parse_location },
	{ "PERCENT", __cal_vcalendar_parse_percent },
	{ "PRIORITY", __cal_vcalendar_parse_priority },
	{ "STATUS", __cal_vcalendar_parse_status },
	{ "SUMMARY", __cal_vcalendar_parse_summary },
	{ "DUE", __cal_vcalendar_parse_dtend },
	{ "AALARM", __cal_vcalendar_parse_aalarm },
	{ "X-", __cal_vcalendar_parse_extended },
};

static int __cal_vcalendar_parse_freq(calendar_record_h event, void *data);
static int __cal_vcalendar_parse_until(calendar_record_h event, void *data);
static int __cal_vcalendar_parse_count(calendar_record_h event, void *data);
static int __cal_vcalendar_parse_interval(calendar_record_h event, void *data);
static int __cal_vcalendar_parse_bysecond(calendar_record_h event, void *data);
static int __cal_vcalendar_parse_byminute(calendar_record_h event, void *data);
static int __cal_vcalendar_parse_byhour(calendar_record_h event, void *data);
static int __cal_vcalendar_parse_byday(calendar_record_h event, void *data);
static int __cal_vcalendar_parse_bymonthday(calendar_record_h event, void *data);
static int __cal_vcalendar_parse_byyearday(calendar_record_h event, void *data);
static int __cal_vcalendar_parse_byweekno(calendar_record_h event, void *data);
static int __cal_vcalendar_parse_bymonth(calendar_record_h event, void *data);
static int __cal_vcalendar_parse_bysetpos(calendar_record_h event, void *data);
static int __cal_vcalendar_parse_wkst(calendar_record_h event, void *data);

enum {
	RRULE_FREQ = 0x0,
	RRULE_UNTIL,
	RRULE_COUNT,
	RRULE_INTERVAL,
	RRULE_BYSECOND,
	RRULE_BYMINUTE,
	RRULE_BYHOUR,
	RRULE_BYDAY,
	RRULE_BYMONTHDAY,
	RRULE_BYYEARDAY,
	RRULE_BYWEEKNO,
	RRULE_BYMONTH,
	RRULE_BYSETPOS,
	RRULE_WKST,
	RRULE_MAX,
};

struct _record_func _rrule_funcs[RRULE_MAX] =
{
	{ "FREQ=", __cal_vcalendar_parse_freq },
	{ "UNTIL=", __cal_vcalendar_parse_until },
	{ "COUNT=", __cal_vcalendar_parse_count },
	{ "INTERVAL=", __cal_vcalendar_parse_interval },
	{ "BYSECOND=", __cal_vcalendar_parse_bysecond },
	{ "BYMINUTE=", __cal_vcalendar_parse_byminute },
	{ "BYHOUR=", __cal_vcalendar_parse_byhour },
	{ "BYDAY=", __cal_vcalendar_parse_byday },
	{ "BYMONTHDAY=", __cal_vcalendar_parse_bymonthday },
	{ "BYYEARDAY=", __cal_vcalendar_parse_byyearday },
	{ "BYWEEKNO=", __cal_vcalendar_parse_byweekno },
	{ "BYMONTH=", __cal_vcalendar_parse_bymonth },
	{ "BYSETPOS=", __cal_vcalendar_parse_bysetpos },
	{ "WKST=", __cal_vcalendar_parse_wkst }
};

static int __cal_vcalendar_parse_trig_related(calendar_record_h event, void *data);
static int __cal_vcalendar_parse_trig_value(calendar_record_h event, void *data);

enum {
	TRIG_RELATED = 0x0,
	TRIG_VALUE,
	TRIG_MAX,
};

struct _record_func _trig_funcs[TRIG_MAX] =
{
	{ "RELATED=", __cal_vcalendar_parse_trig_related },
	{ "VALUE=", __cal_vcalendar_parse_trig_value }
};

static int __cal_vcalendar_parse_charset(int *val, void *data);
static int __cal_vcalendar_parse_encoding(int *val, void *data);


enum {
	TEXT_CHARSET = 0x0,
	TEXT_ENCODING,
	TEXT_MAX,
};

struct _prop_func _optional_funcs[TEXT_MAX] =
{
	{ "CHARSET=", __cal_vcalendar_parse_charset },
	{ "ENCODING=", __cal_vcalendar_parse_encoding },
};

//util //////////////////////////////////////////////////////////////////////


char *_cal_vcalendar_parse_remove_space(char *src)
{
	while (*src) {
		if ('\n' != *src && '\r' != *src) {
			break;
		}
		src++;
	}
	return src;
}

static char __cal_vcalendar_parse_decode_hexa(char *p)
{
	int i;
	char decoded[2] = {0x00, 0x00};

	for (i = 0; i < 2; i++) {
		switch (p[i]) {
		case '0' ... '9':
			decoded[i] = p[i] - '0';
			break;
		case 'a' ... 'f':
			decoded[i] = p[i] - 'a' + 10;
			break;
		case 'A' ... 'F':
			decoded[i] = p[i] - 'A' + 10;
			break;
		}
	}

	return (char)((decoded[0] << 4) + decoded[1]);
}

static int __cal_vcalendar_parse_decode_quoted_printable(char *p, int *len)
{
	int i = 0, j = 0;
	char ch;

	while (p[i]) {
		if (p[i] == '=') {
			if (p[i+1] == 0x09 || p[i+1] == 0x20) {
				i +=3;

			} else if (p[i+1] == '\r' || p[i+1] == '\n') {
				i +=3;

			} else {
				if (p[i+1] == '0' && tolower(p[i+2]) == 'd' &&
						p[i+3] == '=' && p[i+4] == '0' && tolower(p[i+5]) == 'a') {
					p[j] = '\n';
					j++;
					i += 6;
				} else {
					ch = __cal_vcalendar_parse_decode_hexa(&p[i+1]);
					p[j] = ch;
					j++;
					i += 3;
				}
			}
		} else {
			p[j] = p[i];
			j++;
			i++;
		}
	}
	p[j] = '\0';
	*len = i;
	return CALENDAR_ERROR_NONE;
}

////////////////////////////////////////////////////////////////////////

int _cal_vcalendar_parse_unfolding(char *stream)
{
	char *p;

	retv_if(stream == NULL, CALENDAR_ERROR_INVALID_PARAMETER);

	p = stream;
	while ('\0' != *p) {
		if ('=' == *p && '\r' == *(p + 1) && '\n' == *(p + 2)) // ver 1.0
		{
			p += 3;
		}
		else if ('=' == *p && '\n' == *(p + 1)) // ver 1.0 not spec but allow
		{
			p += 2;
		}
		else if ('\r' == *p && '\n' == *(p + 1) && ' ' == *(p + 2)) // ver 2.0
		{
			p += 2;
		}
		else if ('\n' == *p && ' ' == *(p + 1)) // ver 2.0 not spec but allow
		{
			p += 1;
		}
		else
		{
		}
		*stream = *p;
		stream++;
		p++;
	}
	return CALENDAR_ERROR_NONE;
}

char *_cal_vcalendar_parse_read_line(char *stream, char **prop, char **cont)
{
	int i;
	char *p, *q;
	int out;

	/* skip space */
	p = stream;
	q = p;
	out = 0;
	while (*p) {
		switch (*p) {
		case ' ':
			break;
		default:
			out = 1;
			break;
		}
		if (out == 1) {
			break;
		}
		p++;
	}

	i = 0;
	out = 0;
	q = p;
	while (*p) {
		switch (*p) {
		case ';':
		case ':':
			out = 1;
			break;
		default:
			i++;
			break;
		}
		if (out == 1) {
			i++;
			break;
		}
		p++;
	}

	if (0 < i) {
		*prop = calloc(1, i);
		snprintf(*prop, i, "%s", q);
	} else {
		*prop = NULL;
		*cont = NULL;
		return NULL;
	}

	i = 0;
	out = 0;
	q = p;

	while (*p) {
		switch (*p) {
		case '\n': // not spec but allow
			p += 1 ;
			out = 1;
			break;
		case '\r':
			if ('\n' == *(p + 1)) {
				p += 2;
				out = 1;
			}
			break;
		case '\0':
			break;
		default:
			i++;
			break;
		}
		if (out == 1) {
			i++;
			break;
		}
		p++;
	}

	if (0 < i) {
		*cont = calloc(1, i);
		snprintf(*cont, i, "%s", q);
	} else {
		*prop = NULL;
		*cont = NULL;
		return NULL;
	}

	DBG("%s][%s\n", *prop, *cont);
	return p;
}

// start parse func ////////////////////////////////////////////////////
/* vcalendar */////////////////////////////////////////////////

static int __cal_vcalendar_parse_prodid(int *val, void *data)
{
	return CALENDAR_ERROR_NONE;
}

static int __cal_vcalendar_parse_version(int *val, void *data)
{
	char *p = (char *)data;

	p++;
	DBG("version[%s]", p);
	if (strstr(p, "1.0"))
	{
		DBG("version 1.0");
	}
	else
	{
		DBG("version 2.0");
	}

	return CALENDAR_ERROR_NONE;
}

/* vevnt */////////////////////////////////////////////////
static int __cal_vcalendar_parse_dtstamp(int type, calendar_list_h list, calendar_record_h event, char *prop, char *cont)
{
	return CALENDAR_ERROR_NONE;
}

static int __cal_vcalendar_parse_uid(int type, calendar_list_h list, calendar_record_h record, char *prop, char *cont)
{
	int ret;
	char *p = (char *)cont;

	p++;

	switch (type)
	{
	case CALENDAR_BOOK_TYPE_EVENT:
		ret = _cal_record_set_str(record, _calendar_event.uid, p);
		break;
	case CALENDAR_BOOK_TYPE_TODO:
		ret = _cal_record_set_str(record, _calendar_todo.uid, p);
		break;
	}
	return CALENDAR_ERROR_NONE;
}

static int __cal_vcalendar_parse_get_tzid_from_list(calendar_list_h list, const char *tzid, calendar_record_h *timezone)
{
	GList *l = NULL;

	if (list == NULL || tzid == NULL)
	{
		return -1;
	}

	cal_list_s *cal_list = (cal_list_s *)list;
	l = g_list_first(cal_list->record);

	while (l)
	{
		char *uri = NULL;
		calendar_record_h record = (calendar_record_h)l->data;
		calendar_record_get_uri_p(record, &uri);
		if (strncmp(uri, _calendar_timezone._uri, strlen(_calendar_timezone._uri)))
		{
			l = g_list_next(l);
			continue;
		}

		cal_timezone_s *tz = (cal_timezone_s *)record;
		if (!strncmp(tz->standard_name, tzid, strlen(tzid)))
		{
			DBG("Found same tzid[%s] in the list", tzid);
			*timezone = record;
			break;
		}

		l = g_list_next(l);
	}

	return CALENDAR_ERROR_NONE;
}

static char *__cal_vcalendar_parse_dtstart_tzid(char *q)
{
	int i, j;
	int len = 0;
	int has_quot = 0; // to remove quotation(ex> "abc" -> abc)
	char *s = NULL;

	if (NULL == q)
	{
		ERR("Invalid parameter");
		return NULL;
	}

	// TZID="Korea("GMT+9")": 21 - 5
	j = strlen("TZID=");
	len = strlen(q);
	// alloc except "TZID=" string.
	s = calloc(len - j + 1, sizeof(char));
	if (NULL == s)
	{
		ERR("calloc() failed");
		return NULL;
	}
	for (i = 0; i < len -j; i++)
	{
		if (q[j + i] == '\"' && has_quot == 0)
		{
			j++;
			has_quot = 1;
		}
		else if (i == (len -j -1) && q[i] == '\"' && has_quot == 1)
		{
			break;
		}
		s[i] = q[j + i];
	}
	s[i] = '\0';
	return s;
}

int __cal_vcalendar_parse_dtstart_value(char *q, calendar_time_s *caltime)
{
	int j;

	if (NULL == q || NULL == caltime)
	{
		ERR("Invalid parameter");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	j = strlen("VALUE=");

	if (!strncmp(&q[j], "DATE-TIME", strlen("DATE-TIME")))
	{
		caltime->type = CALENDAR_TIME_UTIME;
	}
	else
	{
		caltime->type = CALENDAR_TIME_LOCALTIME;
	}
	return CALENDAR_ERROR_NONE;
}

int __cal_vcalendar_parse_dtstart_time(calendar_list_h list, char *q, char *tzid, calendar_time_s *caltime)
{
	int len = 0;
	int y, m, d, h, min, s;

	if (NULL == q || NULL == caltime)
	{
		ERR("Invalid parameter");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	len = strlen(q);

	switch (caltime->type)
	{
	case CALENDAR_TIME_LOCALTIME:
		sscanf(q, "%04d%02d%02d", &y, &m, &d);
		caltime->time.date.year = y;
		caltime->time.date.month = m;
		caltime->time.date.mday = d;
		break;

	case CALENDAR_TIME_UTIME:
		if (strlen(q) == strlen("YYYYMMDDTHHMMSSZ"))
		{
			sscanf(q, "%04d%02d%02dT%02d%02d%02dZ", &y, &m, &d, &h, &min, &s);
			DBG("get GMT time[%04d/%02d/%02d %02d:%02d:%02d]", y, m, d, h, min, s);
		}
		else
		{
			sscanf(q, "%04d%02d%02dT%02d%02d%02d", &y, &m, &d, &h, &min, &s);
			DBG("get local time[%04d/%02d/%02d %02d:%02d:%02d]", y, m, d, h, min, s);
		}

		if (NULL == tzid || len == strlen("YYYYMMDDTHHMMSSZ"))
		{
			// Z means GMT
			caltime->time.utime = _cal_time_convert_itol(NULL, y, m, d, h, min, s);
		}
		else
		{
			if (_cal_time_is_registered_tzid(tzid))
			{
				caltime->time.utime = _cal_time_convert_itol(tzid, y, m, d, h, min, s);
			}
			else
			{
				char *like_tzid = NULL;
				calendar_record_h timezone = NULL;
				// try get timezone info from the list
				__cal_vcalendar_parse_get_tzid_from_list(list, tzid, &timezone);
				if (timezone)
				{
					DBG("Found from the list");
					_cal_time_get_like_tzid(tzid, timezone, &like_tzid);
					caltime->time.utime = _cal_time_convert_itol(like_tzid, y, m, d, h, min, s);
					DBG("[%s]", like_tzid);
					CAL_FREE(like_tzid);
					like_tzid = NULL;
				}
				else
				{
					DBG("Nowhere to find");
					caltime->time.utime = _cal_time_convert_itol(tzid, y, m, d, h, min, s);
				}
			}
		}
		break;
	}
	return CALENDAR_ERROR_NONE;
}

static int __cal_vcalendar_parse_dtstart(int type, calendar_list_h list, calendar_record_h record, char *prop, char *cont)
{
	int ret;
	int i;
	int len = 0;
	char *p = (char *)cont;
	char **t;
	char *str_tzid = NULL;

	p++;

	calendar_time_s caltime = {0};

	t = g_strsplit_set(p, ";:", -1);
	if (NULL == t)
	{
		ERR("g_strsplit_set() failed");
		return CALENDAR_ERROR_OUT_OF_MEMORY;
	}

	len = g_strv_length(t);
	for (i = 0; i < len; i++)
	{
		DBG("get string[%s]", t[i]);
		if (!strncmp(t[i], "TZID=", strlen("TZID=")) && NULL == str_tzid)
		{
			str_tzid = __cal_vcalendar_parse_dtstart_tzid(t[i]);
			DBG("str_tzid[%s]", str_tzid);
		}
		else if (!strncmp(t[i], "VALUE=", strlen("VALUE")))
		{
			DBG("get value");
			__cal_vcalendar_parse_dtstart_value(t[i], &caltime);
		}
		else if (*t[i] >= '1' && *t[i] <= '9')
		{
			DBG("get time");
			__cal_vcalendar_parse_dtstart_time(list, t[i], str_tzid, &caltime);
		}
		else
		{
			ERR("Unable to parsing[%s]", t[i]);
		}
	}

	if (NULL == str_tzid)
	{
		// if tzid is not set, set GMT
		str_tzid = strdup(CAL_TZID_GMT);
	}
	switch (type)
	{
	case CALENDAR_BOOK_TYPE_EVENT:
		ret = _cal_record_set_str(record, _calendar_event.start_tzid, str_tzid);
		ret = _cal_record_set_caltime(record, _calendar_event.start_time, caltime);
		break;
	case CALENDAR_BOOK_TYPE_TODO:
		ret = _cal_record_set_str(record, _calendar_todo.start_tzid, str_tzid);
		ret = _cal_record_set_caltime(record, _calendar_todo.start_time, caltime);
		break;
	}
	if (str_tzid) free(str_tzid);
	g_strfreev(t);

	return CALENDAR_ERROR_NONE;
}

static int __cal_vcalendar_parse_created(int type, calendar_list_h list, calendar_record_h record, char *prop, char *cont)
{
	int ret;
	char *p = (char *)cont;

	p++;
	switch (type)
	{
	case CALENDAR_BOOK_TYPE_EVENT:
		ret = _cal_record_set_lli(record, _calendar_event.created_time,
				_cal_time_convert_stol(NULL, p));
		break;
	case CALENDAR_BOOK_TYPE_TODO:
		ret = _cal_record_set_lli(record, _calendar_todo.created_time,
				_cal_time_convert_stol(NULL, p));
		break;
	}

	return CALENDAR_ERROR_NONE;
}

static int __work_description_switch(int me, int mode, char *buf, int *charset, int *encoding)
{
	switch (mode) {
	case 1:
	case 2:
		if (!strncmp(buf, "CHARSET=UTF-8", strlen("CHARSET=UTF-8"))) {
			DBG("CHARSET=UTF-8");
			*charset = 1;

		} else if (!strncmp(buf, "CHARSET=UTF-16", strlen("CHARSET=UTF-16"))) {
			DBG("CHARSET=UTF-16");
			*charset = 1;

		} else if (!strncmp(buf, "ENCODING=BASE64", strlen("ENCODING=BASE64"))) {
			DBG("ENCODE_BASE64");
			*encoding = ENCODE_BASE64;

		} else if (!strncmp(buf, "ENCODING=QUOTED-PRINTABLE", strlen("ENCODING=QUOTED-PRINTABLE"))) {
			DBG("ENCODE_QUOTED_PRINTABLE");
			*encoding = ENCODE_QUOTED_PRINTABLE;

		} else {

		}
		mode = 0;
		break;
	default:
		mode = me;
		break;
	}
	return mode;
}

static int __cal_vcalendar_parse_description(int type, calendar_list_h list, calendar_record_h record, char *prop, char *cont)
{
	int i = 0, j;
	int ret;
	int len;
	int out;
	int mode;
	int charset, encoding;
	char buf[64] = {0};
	char *p = (char *)cont;

	if (p[i + 1] == '\0')
	{
		ERR("Invalid parameter");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	i = j = 0;
	out = 0;
	mode = 0;
	charset = encoding = 0;
	while (p[i] != '\0') {
		switch (p[i]) {
		case ':':
			mode = 1;
			out = 1;
			break;

		case ';':
			buf[j] = '\0';
			mode = __work_description_switch(2, mode, buf, &charset, &encoding);
			j = 0;
			break;

		default:
			buf[j] = p[i];
			j++;
			break;
		}
		i++;

		if (out) {
			DBG("out");
			break;
		}
	}
	__work_description_switch(0, mode, buf, &charset, &encoding);

	DBG("charset(%d) encoding(%d)", charset, encoding);
	if (encoding) {
		__cal_vcalendar_parse_decode_quoted_printable(p+i, &len);
	}

	switch (type)
	{
	case CALENDAR_BOOK_TYPE_EVENT:
		ret = _cal_record_set_str(record, _calendar_event.description, p + i);
		break;
	case CALENDAR_BOOK_TYPE_TODO:
		ret = _cal_record_set_str(record, _calendar_todo.description, p + i);
		break;
	}

	return CALENDAR_ERROR_NONE;
}

static int __cal_vcalendar_parse_last_mod(int type, calendar_list_h list, calendar_record_h record, char *prop, char *cont)
{
	int ret;
	char *p = (char *)cont;

	p++;

	switch (type)
	{
	case CALENDAR_BOOK_TYPE_EVENT:
		ret = _cal_record_set_lli(record, _calendar_event.last_modified_time,
				_cal_time_convert_stol(NULL, p));
		break;
	case CALENDAR_BOOK_TYPE_TODO:
		ret = _cal_record_set_lli(record, _calendar_todo.last_modified_time,
				_cal_time_convert_stol(NULL, p));
		break;
	}
	return CALENDAR_ERROR_NONE;
}

inline void __cal_vcalendar_parse_get_optional(char *p, int *encoding)
{
	int i;

	for (i = 0; i < TEXT_MAX; i++) {
		if (!strncmp(p, _optional_funcs[i].prop, strlen(_optional_funcs[i].prop))) {
			int j = 0;
			char buf[64] = {0, };
			p += strlen(_optional_funcs[i].prop);
			while (p[j] != ':' && p[j] != ';' && p[j] != '\n' && p[j] != '\0') {
				buf[j] = p[j];
				j++;
			}
			if (p[j] != '\0') {
				buf[j] = '\0';
			}

			p += j;
			_optional_funcs[i].func(encoding, buf);
			break;
		} else {

		}
	}
}

static int __cal_vcalendar_parse_location(int type, calendar_list_h list, calendar_record_h record, char *prop, char *cont)
{
	int i = 0, j;
	int ret;
	int len;
	int out;
	int mode;
	int charset, encoding;
	char buf[64] = {0};
	char *p = (char *)cont;

	if (p[i + 1] == '\0')
	{
		ERR("Invalid parameter");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	i = j = 0;
	out = 0;
	mode = 0;
	charset = encoding = 0;
	while (p[i] != '\0') {
		switch (p[i]) {
		case ':':
			mode = 1;
			out = 1;
			break;

		case ';':
			buf[j] = '\0';
			mode = __work_description_switch(2, mode, buf, &charset, &encoding);
			j = 0;
			break;

		default:
			buf[j] = p[i];
			j++;
			break;
		}
		i++;

		if (out) {
			DBG("out");
			break;
		}
	}
	__work_description_switch(0, mode, buf, &charset, &encoding);

	DBG("charset(%d) encoding(%d)", charset, encoding);
	if (encoding) {
		__cal_vcalendar_parse_decode_quoted_printable(p+i, &len);
	}

	switch (type)
	{
	case CALENDAR_BOOK_TYPE_EVENT:
		ret = _cal_record_set_str(record, _calendar_event.location, p + i);
		break;
	case CALENDAR_BOOK_TYPE_TODO:
		ret = _cal_record_set_str(record, _calendar_todo.location, p + i);
		break;
	}

	return CALENDAR_ERROR_NONE;
}

static int __cal_vcalendar_parse_priority(int type, calendar_list_h list, calendar_record_h record, char *prop, char *cont)
{
	int ret;
	int prio = 0;
	char *p = (char *)cont;

	p++;
	if (p[0] < '0' || p[0] > '9') {
		DBG("warning check range\n");
		return -1;
	}

	DBG("priority(%d)", atoi(p));
	switch (atoi(p))
	{
	case 1:
	case 2:
	case 3:
	case 4:
		prio = CALENDAR_TODO_PRIORITY_HIGH;
		break;
	case 5:
		prio = CALENDAR_TODO_PRIORITY_NORMAL;
		break;
	case 6:
	case 7:
	case 8:
	case 9:
		prio = CALENDAR_TODO_PRIORITY_LOW;
		break;
	default:
		prio = CALENDAR_TODO_PRIORITY_NONE;
		break;
	}

	DBG("convert to priority(%d)", prio);
	switch (type)
	{
	case CALENDAR_BOOK_TYPE_EVENT:

		ret = _cal_record_set_int(record, _calendar_event.priority, prio);
		break;
	case CALENDAR_BOOK_TYPE_TODO:
		ret = _cal_record_set_int(record, _calendar_todo.priority, prio);
		break;
	}
	return CALENDAR_ERROR_NONE;
}

static int __cal_vcalendar_parse_status(int type, calendar_list_h list, calendar_record_h record, char *prop, char *cont)
{
	int ret;
	int status;
	char *p = (char *)cont;

	p++;

	switch (type)
	{
	case CALENDAR_BOOK_TYPE_EVENT:
		if (!strncmp(p, "TENTATIVE", strlen("TENTATIVE")))
		{
			status = CALENDAR_EVENT_STATUS_TENTATIVE;
		}
		else if (!strncmp(p, "CONFIRMED", strlen("CONFIRMED")))
		{
			status = CALENDAR_EVENT_STATUS_CONFIRMED;
		}
		else if (!strncmp(p, "CANCELLED", strlen("CANCELLED")))
		{
			status = CALENDAR_EVENT_STATUS_CANCELLED;
		}
		else
		{
			status = CALENDAR_EVENT_STATUS_NONE;
		}
		ret = _cal_record_set_int(record, _calendar_event.event_status, status);

		break;

	case CALENDAR_BOOK_TYPE_TODO:
		if (!strncmp(p, "NEEDS-ACTION", strlen("NEEDS-ACTION"))) // for ver2.0
		{
			status = CALENDAR_TODO_STATUS_NEEDS_ACTION;
		}
		else if (!strncmp(p, "NEEDS ACTION", strlen("NEEDS ACTION"))) // for ver1.0
		{
			status = CALENDAR_TODO_STATUS_NEEDS_ACTION;
		}
		else if (!strncmp(p, "COMPLETED", strlen("COMPLETED")))
		{
			status = CALENDAR_TODO_STATUS_COMPLETED;
		}
		else if (!strncmp(p, "IN-PROCESS", strlen("IN-PROCESS")))
		{
			status = CALENDAR_TODO_STATUS_IN_PROCESS;
		}
		else if (!strncmp(p, "CANCELLED", strlen("CANCELLED")))
		{
			status = CALENDAR_TODO_STATUS_CANCELED;
		}
		else
		{
			status = CALENDAR_TODO_STATUS_NONE;
		}
		ret = _cal_record_set_int(record, _calendar_todo.todo_status, status);

		break;
	}

	return CALENDAR_ERROR_NONE;
}

static int __cal_vcalendar_parse_summary(int type, calendar_list_h list, calendar_record_h record, char *prop, char *cont)
{
	int ret;
	int i = 0, j;
	int len;
	int out;
	int mode;
	int charset, encoding;
	char buf[64] = {0};
	char *p = (char *)cont;

	if (p[i + 1] == '\0')
	{
		ERR("Invalid parameter");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	i = j = 0;
	out = 0;
	mode = 0;
	charset = encoding = 0;
	while (p[i] != '\0') {
		switch (p[i]) {
		case ':':
			mode = 1;
			out = 1;
			break;

		case ';':
			buf[j] = '\0';
			mode = __work_description_switch(2, mode, buf, &charset, &encoding);
			j = 0;
			break;

		default:
			buf[j] = p[i];
			j++;
			break;
		}
		i++;

		if (out) {
			break;
		}
	}
	__work_description_switch(0, mode, buf, &charset, &encoding);

	DBG("charset(%d) encoding(%d)", charset, encoding);
	if (encoding) {
		__cal_vcalendar_parse_decode_quoted_printable(p+i, &len);
	}

	switch (type)
	{
	case CALENDAR_BOOK_TYPE_EVENT:
		ret = _cal_record_set_str(record, _calendar_event.summary, p + i);
		break;
	case CALENDAR_BOOK_TYPE_TODO:
		ret = _cal_record_set_str(record, _calendar_todo.summary, p + i);
		break;
	}

	DBG("summary(%s)\n", p + i);
	return CALENDAR_ERROR_NONE;
}

enum {
	__RRULE_VER1_MODE_FREQ = 0x0,
	__RRULE_VER1_MODE_BY,
	__RRULE_VER1_MODE_UNTIL,
	__RRULE_VER1_MODE_OUT,
};

enum {
	__RRULE_VER1_BYYEARDAY = 0,
	__RRULE_VER1_BYMONTH,
	__RRULE_VER1_BYMONTHDAY,
	__RRULE_VER1_BYDAY,
};

static int __cal_vcalendar_parse_rrule_ver1(calendar_record_h record, char *p)
{
	DBG("This is rrule for ver 1.0");
	int ret;
	int i;
	int length;
	int freq = CALENDAR_RECURRENCE_NONE;
	int mode = 0; // 0:freq, 1:nth 2:bystr 3:range
	int interval = 0;
	unsigned int byint = 0; // 1:bymonthday 2:byday 3:bymonth 4:byyearday
	int len;
	int num = 0;
	int y, mon, d, h, min, s;
	int nth_week = 0;
	char **t;
	char *r = NULL, *q = NULL;
	char buf_by[256] = {0};
	calendar_time_s ut = {0};
	cal_event_s *event = (cal_event_s *)record;

	p++;
	t = g_strsplit(p, " ", -1);
	if (!t) {
		ERR("g_strsplit failed");
		g_strfreev(t);
		return CALENDAR_ERROR_OUT_OF_MEMORY;
	}
	length = g_strv_length(t);
	for (i = 0; i < length; i++)
	{
		if (t[i] == NULL || strlen(t[i]) == 0)
		{
			continue;
		}

		switch (mode)
		{
		case __RRULE_VER1_MODE_FREQ: // freq
			mode = __RRULE_VER1_MODE_BY;
			if (*t[i] == 'D')
			{
				DBG("CALENDAR_RECURRENCE_DAILY");
				freq = CALENDAR_RECURRENCE_DAILY;
				interval = strlen(t[i]) == 1 ? 1 : atoi(t[i] + 1);
				byint = __RRULE_VER1_BYMONTHDAY;
			}
			else if (*t[i] == 'W')
			{
				DBG("CALENDAR_RECURRENCE_WEEKLY");
				freq = CALENDAR_RECURRENCE_WEEKLY;
				interval = strlen(t[i]) == 1 ? 1 : atoi(t[i] + 1);
				byint = __RRULE_VER1_BYDAY;
			}
			else if (*t[i] == 'M'&& *(t[i] + 1) == 'P')
			{
				DBG("CALENDAR_RECURRENCE_MONTHLY");
				freq = CALENDAR_RECURRENCE_MONTHLY;
				interval = strlen(t[i]) == 2 ? 1 : atoi(t[i] + 2);
				byint = __RRULE_VER1_BYDAY;
			}
			else if (*t[i] == 'M'&& *(t[i] + 1) == 'D')
			{
				DBG("CALENDAR_RECURRENCE_MONTHLY");
				freq = CALENDAR_RECURRENCE_MONTHLY;
				interval = strlen(t[i]) == 2 ? 1 : atoi(t[i] + 2);
				byint = __RRULE_VER1_BYMONTHDAY;
			}
			else if (*t[i] == 'Y'&& *(t[i] + 1) == 'M')
			{
				DBG("CALENDAR_RECURRENCE_YEARLY");
				freq = CALENDAR_RECURRENCE_YEARLY;
				interval = strlen(t[i]) == 2 ? 1 : atoi(t[i] + 2);
				byint = __RRULE_VER1_BYMONTH;
			}
			else if (*t[i] == 'Y'&& *(t[i] + 1) == 'D')
			{
				DBG("CALENDAR_RECURRENCE_YEARLY");
				freq = CALENDAR_RECURRENCE_YEARLY;
				interval = strlen(t[i]) == 2 ? 1 : atoi(t[i] + 2);
				byint = __RRULE_VER1_BYYEARDAY;
			}
			else
			{
				ERR("Invalid ");
			}
			ret = _cal_record_set_int(record, _calendar_event.freq, freq);
			DBG("interval(%d)", interval);
			ret = _cal_record_set_int(record, _calendar_event.interval, interval);
			break;

		case __RRULE_VER1_MODE_BY: // num
			switch (byint)
			{
			case __RRULE_VER1_BYDAY:
				DBG("__RRULE_VER1_BYDAY:[%s]", t[i]);
				if (strstr(t[i], "MO") || strstr(t[i], "TU") || strstr(t[i], "WE")
						|| strstr(t[i], "TH") || strstr(t[i], "FR")
						|| strstr(t[i], "SA") || strstr(t[i], "SU"))
				{
					if (NULL == r)
					{
						len = strlen(t[i]) + 1;
						len += (nth_week == 0) ? 0 : 8; // 8 is buf
						r = calloc(len, sizeof(char));
						if (NULL == r)
						{
							ERR("calloc() failed");
							g_strfreev(t);
							return CALENDAR_ERROR_DB_FAILED;
						}
						if (0 == nth_week)
						{
							snprintf(r, len, "%s", t[i]);
						}
						else
						{
							snprintf(r, len, "%d%s", nth_week, t[i]);
						}
					}
					else
					{
						len = strlen(r) + strlen(t[i]) + 2;
						len += (nth_week == 0) ? 0 : 8; // 8 is buf
						q = calloc(len, sizeof(char));
						if (NULL == r)
						{
							ERR("calloc() failed");
							g_strfreev(t);
							return CALENDAR_ERROR_DB_FAILED;
						}
						if (0 == nth_week)
						{
							snprintf(q, len, "%s,%s", r, t[i]);
						}
						else
						{
							snprintf(q, len, "%s,%d%s", r, nth_week, t[i]);
						}
						CAL_FREE(r);
						r = q;
					}
				}
				else if (*t[i] >= '1' && *t[i] <= '9' && strlen(t[i]) < strlen("YYYYMMDD"))
				{
					// MP1 1+ 1- FR: first Friday and last Friday
					DBG("Unable to handle multi week");
					int len = strlen(t[i]);
					char buf[8] = {0};
					if (*t[len -1] == '-')
					{
						// if 5+, buf has 5 because of len is 2 including '\0'
						snprintf(buf, len, "%s", t[i]);
						nth_week = atoi(buf);
						DBG("-(%d)", nth_week);
						nth_week *= -1;
					}
					else if (*t[len -1] == '+')
					{
						snprintf(buf, len, "%s", t[i]);
						nth_week = atoi(buf);
						DBG("(%d)", nth_week);
					}
					else
					{
						snprintf(buf, sizeof(buf), "%s", t[i]);
						nth_week = atoi(buf);
						DBG("(%d)", nth_week);
					}
				}
				else
				{
					DBG("final string[%s]", t[i]);
					if (r)
					{
						ret = _cal_record_set_str(record, _calendar_event.byday, r);
						CAL_FREE(r);
					}
					i--;
					mode = __RRULE_VER1_MODE_UNTIL;
				}
				break;

			case __RRULE_VER1_BYYEARDAY:
			case __RRULE_VER1_BYMONTH:
			case __RRULE_VER1_BYMONTHDAY:
				DBG("Not __RRULE_VER1_BYDAY:");
				if (*t[i] > '1' && *t[i] < '9'&& strlen(t[i]) < 4)
				{
					if (NULL == r)
					{
						len = strlen(t[i]) + 1;
						r = calloc(len, sizeof(char));
						if (NULL == r)
						{
							ERR("calloc() failed");
							g_strfreev(t);
							return CALENDAR_ERROR_DB_FAILED;
						}
						snprintf(r, len, "%s", t[i]);
					}
					else
					{
						len = strlen(r) + strlen(t[i]) + 2;
						q = calloc(len, sizeof(char));
						if (NULL == r)
						{
							ERR("calloc() failed");
							g_strfreev(t);
							return CALENDAR_ERROR_DB_FAILED;
						}
						snprintf(q, len, "%s,%s", r, t[i]);
						CAL_FREE(r);
						r = q;
					}
				}
				else
				{
					if (r)
					{
						switch (byint)
						{
						case __RRULE_VER1_BYYEARDAY:
							ret = _cal_record_set_str(record, _calendar_event.byyearday, r);
							break;
						case __RRULE_VER1_BYMONTH:
							ret = _cal_record_set_str(record, _calendar_event.bymonth, r);
							break;
						case __RRULE_VER1_BYMONTHDAY:
							ret = _cal_record_set_str(record, _calendar_event.bymonthday, r);
							break;
						}
						CAL_FREE(r);
					}
					i--;
					mode = __RRULE_VER1_MODE_UNTIL;
				}
				break;
			}
			break;

		case __RRULE_VER1_MODE_UNTIL: // until
			mode = __RRULE_VER1_MODE_OUT; // out
			if (*t[i] == '#')
			{
				num = atoi(t[i] + 1);
				if (num == 0)
				{
					DBG("CALENDAR_RANGE_NONE");
					ret = _cal_record_set_int(record, _calendar_event.range_type, CALENDAR_RANGE_NONE);
				}
				else
				{
					DBG("CALENDAR_RANGE_COUNT(%d)", num);
					ret = _cal_record_set_int(record, _calendar_event.range_type, CALENDAR_RANGE_COUNT);
					ret = _cal_record_set_int(record, _calendar_event.count, num);
				}
			}
			else
			{
				sscanf(t[i], "%4d%2d%2dT%2d%2d%2d", &y, &mon, &d, &h, &min, &s);
				DBG("get until %04d/%02d/%02d %02d:%02d:%02d", y, mon, d, h, min, s);
				switch (event->start.type)
				{
				case CALENDAR_TIME_UTIME:
					ut.type = CALENDAR_TIME_UTIME;
					ut.time.utime = _cal_time_convert_itol(event->start_tzid,
							y, mon, d, h, min, s);
					DBG("CALENDAR_RANGE_UNTIL(%lld)", ut.time.utime);
					break;
				case CALENDAR_TIME_LOCALTIME:
					ut.type = CALENDAR_TIME_LOCALTIME;
					ut.time.date.year = y;
					ut.time.date.month = mon;
					ut.time.date.mday = d;
					DBG("CALENDAR_RANGE_UNTIL(%04d/%02d/%02d)", y, mon, d);
					break;
				}
				ret = _cal_record_set_int(record, _calendar_event.range_type, CALENDAR_RANGE_UNTIL);
				ret = _cal_record_set_caltime(record, _calendar_event.until_time, ut);
			}
			break;

		default:
			mode = __RRULE_VER1_MODE_OUT;
			break;
		}

		if (mode == __RRULE_VER1_MODE_OUT)
		{
			break;
		}
	}

	if (strlen(buf_by) > 0)
	{
		DBG("bystr[%s]", buf_by);
		ret = _cal_record_set_str(record, byint, buf_by);
	}
	CAL_FREE(r);
	g_strfreev(t);
	return CALENDAR_ERROR_NONE;
}

static int __cal_vcalendar_parse_rrule(int type, calendar_list_h list, calendar_record_h record, char *prop, char *cont)
{
	int ret;
	int i, j, k;
	int mode;
	int version = 0;
	char buf[64] = {0};
	char *p = (char *)cont;

	i = j = 0;
	mode = 0;

	if (p[i + 1] == '\0')
	{
		ERR("Invalid parameter");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	if (strstr(p, "FREQ=")) {
		DBG("This is version 2");
		version = 2;
	} else {
		DBG("This is version 1");
		version = 1;
	}

	if (version == 2) {
		i = j = 0;
		ret = _cal_record_set_int(record, _calendar_event.interval, 1);
		/* this is for ver 2 */
		while (p[i] != '\0') {
			DBG("[%c](%d)", p[i], i);
			switch (p[i]) {
			case ':':
			case ';':
				buf[j] = '\0';
				if (strlen(buf) < 1) {
					break;
				}

				for (k = 0; k < RRULE_MAX; k++) {
					if (!strncmp(buf, _rrule_funcs[k].prop, strlen(_rrule_funcs[k].prop))) {
						_rrule_funcs[k].func(record, buf + strlen(_rrule_funcs[k].prop));
						break;
					}
				}
				j = 0;
				break;

			default:
				buf[j] = p[i];
				j++;
				break;
			}
			i++;
		}

		buf[j] = '\0';
		for (i = 0; i < RRULE_MAX; i++) {
			if (!strncmp(buf, _rrule_funcs[i].prop, strlen(_rrule_funcs[i].prop))) {
				version = 2;
				_rrule_funcs[i].func(record, buf + strlen(_rrule_funcs[i].prop));
				break;
			}
		}
		return CALENDAR_ERROR_NONE;
	}

	// for ver 1.0
	__cal_vcalendar_parse_rrule_ver1(record, p);

	return CALENDAR_ERROR_NONE;
}

static int __cal_vcalendar_parse_dtend(int type, calendar_list_h list, calendar_record_h record, char *prop, char *cont)
{
	int ret;
	int i;
	int len = 0;
	char *p = (char *)cont;
	char **t;
	char *str_tzid = NULL;

	p++;

	calendar_time_s caltime = {0};

	t = g_strsplit_set(p, ";:", -1);
	if (NULL == t)
	{
		ERR("g_strsplit_set() failed");
		return CALENDAR_ERROR_OUT_OF_MEMORY;
	}

	len = g_strv_length(t);
	for (i = 0; i < len; i++)
	{
		DBG("get string[%s]", t[i]);
		if (!strncmp(t[i], "TZID=", strlen("TZID=")) && NULL == str_tzid)
		{
			str_tzid = __cal_vcalendar_parse_dtstart_tzid(t[i]);
			DBG("str_tzid[%s]", str_tzid);
		}
		else if (!strncmp(t[i], "VALUE=", strlen("VALUE")))
		{
			DBG("get value");
			__cal_vcalendar_parse_dtstart_value(t[i], &caltime);
		}
		else if (*t[i] >= '1' && *t[i] <= '9')
		{
			DBG("get time");
			__cal_vcalendar_parse_dtstart_time(list, t[i], str_tzid, &caltime);
		}
		else
		{
			ERR("Unable to parsing[%s]", t[i]);
		}
	}

	if (NULL == str_tzid)
	{
		// if tzid is not set, set GMT
		str_tzid = strdup(CAL_TZID_GMT);
	}
	switch (type)
	{
	case CALENDAR_BOOK_TYPE_EVENT:
		ret = _cal_record_set_str(record, _calendar_event.end_tzid, str_tzid);
		ret = _cal_record_set_caltime(record, _calendar_event.end_time, caltime);
		break;
	case CALENDAR_BOOK_TYPE_TODO:
		ret = _cal_record_set_str(record, _calendar_todo.due_tzid, str_tzid);
		ret = _cal_record_set_caltime(record, _calendar_todo.due_time, caltime);
		break;
	}
	if (str_tzid) free(str_tzid);
	g_strfreev(t);

	return CALENDAR_ERROR_NONE;
}

static int __cal_vcalendar_parse_completed(int type, calendar_list_h list, calendar_record_h event, char *prop, char *cont)
{
	return CALENDAR_ERROR_NONE;
}

static int __cal_vcalendar_parse_percent(int type, calendar_list_h list, calendar_record_h event, char *prop, char *cont)
{
	return CALENDAR_ERROR_NONE;
}

/////////////////////////////////////////////////////////////////
static int __cal_vcalendar_parse_attendee_cutype(calendar_record_h attendee, void *data)
{
	return CALENDAR_ERROR_NONE;
}

static int __cal_vcalendar_parse_attendee_member(calendar_record_h attendee, void *data)
{
	return CALENDAR_ERROR_NONE;
}

static int __cal_vcalendar_parse_attendee_role(calendar_record_h attendee, void *data)
{
	return CALENDAR_ERROR_NONE;
}

static int __cal_vcalendar_parse_attendee_partstat(calendar_record_h attendee, void *data)
{
	return CALENDAR_ERROR_NONE;
}

static int __cal_vcalendar_parse_attendee_rsvp(calendar_record_h attendee, void *data)
{
	return CALENDAR_ERROR_NONE;
}

static int __cal_vcalendar_parse_attendee_delto(calendar_record_h attendee, void *data)
{
	return CALENDAR_ERROR_NONE;
}

static int __cal_vcalendar_parse_attendee_delfrom(calendar_record_h attendee, void *data)
{
	return CALENDAR_ERROR_NONE;
}

static int __cal_vcalendar_parse_attendee_sentby(calendar_record_h attendee, void *data)
{
	return CALENDAR_ERROR_NONE;
}

static int __cal_vcalendar_parse_attendee_cn(calendar_record_h attendee, void *data)
{
	int ret;
	int i = 0;
	char *text = NULL;
	char *p = (char *)data;

	while (*p != ':' && *p != '\n' && *p != '\r' && *p != '\0') {
		i++;
		p++;
	}

	text = calloc(i + 1, sizeof(char));
	if (text == NULL) {
		ERR("Failed to calloc");
		return -1;
	}
	snprintf(text, i + 1, "%s", (char *)data);

	ret = _cal_record_set_str(attendee, _calendar_attendee.name, text);
	DBG("cn[%s]", text);
	if (text) free(text);

	return CALENDAR_ERROR_NONE;
}

static int __cal_vcalendar_parse_attendee_dir(calendar_record_h attendee, void *data)
{
	return CALENDAR_ERROR_NONE;
}

int _work_attendee_mailto(calendar_record_h attendee, char *buf)
{
	return CALENDAR_ERROR_NONE;
}

int _work_attendee_property(calendar_record_h attendee, char *buf)
{
	int i;
	int len_all, len_prop;

	for (i = 0; i < ATTENDEE_MAX; i++) {
		if (!strncmp(buf, _attendee_funcs[i].prop, strlen(_attendee_funcs[i].prop))) {
			len_all = strlen(buf);
			len_prop = strlen(_attendee_funcs[i].prop);
			snprintf(buf, len_all - len_prop + 1, "%s", &buf[len_prop]);
			_attendee_funcs[i].func(attendee, buf);
			break;
		}
	}
	return CALENDAR_ERROR_NONE;
}

static int __cal_vcalendar_parse_attendee(int type, calendar_list_h list, calendar_record_h event, char *prop, char *cont)
{
	int ret;
	int i, j;
	char *p = (char *)cont;
	calendar_record_h attendee;

	ret = calendar_record_create(_calendar_attendee._uri, &attendee);

	ret = calendar_record_add_child_record(event, _calendar_event.calendar_attendee, attendee);

	i = 0;
	j = 0;
	int mode = 0;
	char buf[64] = {0};

	if (p[i + 1] == '\0')
	{
		ERR("Invalid parameter");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	while (p[i] != '\0') {
		switch (p[i]) {
		case ':':
			/* work mail to */
			if (mode) {
				buf[j] = '\0';
				_work_attendee_mailto(attendee, buf);
				mode = 0;
			} else {
				mode = 1;
			}
			j = 0;
			break;

		case ';':
			/* work property */
			if (mode) {
				buf[j] = '\0';
				_work_attendee_property(attendee, buf);
				mode = 0;
			} else {
				mode = 2;
			}
			j = 0;
			break;

		default:
			buf[j] = p[i];
			j++;
			break;
		}
		i++;
	}

	switch (mode) {
	case 1:
		buf[j] = '\0';
		_work_attendee_mailto(attendee, buf);
		break;
	case 2:
		buf[j] = '\0';
		_work_attendee_property(attendee, buf);
		break;
	default:
		break;
	}

	return CALENDAR_ERROR_NONE;
}


static int __cal_vcalendar_parse_categories(int type, calendar_list_h list, calendar_record_h event, char *prop, char *cont)
{
	char *p = (char *)cont;
	int encoding = 0;

	while (*p != '\n' && *p != '\r' && *p != '\0') {
		if ( *p == ':') {
			p++;
			if (encoding == ENCODE_BASE64) {
				gsize len;
				_cal_record_set_str(event, _calendar_event.categories,
						(char *)g_base64_decode(p, &len));

			} else if (encoding == ENCODE_QUOTED_PRINTABLE) {
				int len;
				__cal_vcalendar_parse_decode_quoted_printable(p, &len);
				_cal_record_set_str(event, _calendar_event.categories, p);

			} else {
				_cal_record_set_str(event, _calendar_event.categories, p);
			}
			break;

		} else if (*p == ';') {
			p++;
			__cal_vcalendar_parse_get_optional(p, &encoding);

		} else {
			p++;
		}
	}
	DBG("type(%d)categories(%s)\n", type, p);

	return CALENDAR_ERROR_NONE;
}

static int __cal_vcalendar_parse_extended(int type, calendar_list_h list, calendar_record_h record, char *prop, char *cont)
{
	int ret;
	int id;
	calendar_record_h extended = NULL;

	ret = calendar_record_create(_calendar_extended_property._uri, &extended);
	if (CALENDAR_ERROR_NONE != ret)
	{
		ERR("calendar_record_create() failed");
		return ret;
	}

	DBG("key[%s]value[%s]", prop, cont +1);
	ret = calendar_record_set_str(extended, _calendar_extended_property.key, prop);
	if (CALENDAR_ERROR_NONE != ret)
	{
		ERR("calendar_record_set_str() failed");
		return ret;
	}
	ret = calendar_record_set_str(extended, _calendar_extended_property.value, cont +1);
	if (CALENDAR_ERROR_NONE != ret)
	{
		ERR("calendar_record_set_str() failed");
		return ret;
	}

	switch (type)
	{
	case CALENDAR_BOOK_TYPE_EVENT:
		ret = calendar_record_get_int(record, _calendar_event.id, &id);
		if (CALENDAR_ERROR_NONE != ret)
		{
			ERR("calendar_record_get_int() failed");
			return ret;
		}
		ret = calendar_record_add_child_record(record, _calendar_event.extended, extended);
		if (CALENDAR_ERROR_NONE != ret)
		{
			ERR("calendar_record_add_child_record() failed");
			return ret;
		}
		break;

	case CALENDAR_BOOK_TYPE_TODO:
		ret = calendar_record_get_int(record, _calendar_todo.id, &id);
		if (CALENDAR_ERROR_NONE != ret)
		{
			ERR("calendar_record_get_int() failed");
			return ret;
		}
		ret = calendar_record_add_child_record(record, _calendar_todo.extended, extended);
		if (CALENDAR_ERROR_NONE != ret)
		{
			ERR("calendar_record_add_child_record() failed");
			return ret;
		}
		break;
	}

	return CALENDAR_ERROR_NONE;
}

/*
   For ver 1.0 aalarm
   alarmparts	= 0*3(strnosemi ";") strnosemi
   ; runTime, snoozeTime, repeatCount, audioContent
*/
enum {
	__AALARM_INVALID = 0,
	__AALARM_RUNTIME,
	__AALARM_TYPE,
	__AALARM_VALUE,
	__AALARM_SNOOZETIME,
	__AALARM_REPEATCOUNT,
	__AALARM_AUDIOCONTENT,
};
static int __cal_vcalendar_parse_aalarm(int type, calendar_list_h list, calendar_record_h record, char *prop, char *cont)
{
	int ret;
	int i = 0, j = 0;
	int part = 0;
	int y, mon, d, h, min, s;
	char t, z;
	char buf[64] = {0};
	char *p = (char *)cont;

	if (p[i + 1] == '\0')
	{
		ERR("Invalid parameter");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	while (p[i] != '\0')
	{
		// select part
		switch (p[i])
		{
		case ':':
			i++;
			part = __AALARM_RUNTIME;
			break;

		case ';':
			i++;
			if (!strncmp(p, "TYPE=", strlen("TYPE=")))
			{
				part = __AALARM_TYPE;
			}
			else if (!strncmp(p, "VALUE=", strlen("VALUE=")))
			{
				part = __AALARM_VALUE;
			}
			else if (p[i] == 'P')
			{
				part = __AALARM_SNOOZETIME; // Period
			}
			else if (p[i] >= '0' && p[i] < '9')
			{
				part = __AALARM_REPEATCOUNT; // repeatCount
			}
			else
			{
				part = __AALARM_AUDIOCONTENT;
			}
			break;

		default:
			ERR("Error");
			i++;
			break;
		}

		// extract value
		j = 0;
		while (p[i] != '\0' && p[i] != ':' && p[i] != ';')
		{
			buf[j] = p[i];
			j++;
			i++;
		}
		buf[j] = '\0';

		DBG("part(%d) value[%s]", part, buf);
		// work part
		switch (part)
		{
		case __AALARM_INVALID:
			break;

		case __AALARM_RUNTIME:
			y = mon = d = h = min = s = 0;
			if (strlen(buf) < strlen("YYYYMMDD"))
			{
				ERR("Invalid until[%s]", buf);
				return CALENDAR_ERROR_INVALID_PARAMETER;
			}
			sscanf(buf, "%04d%02d%02d%c%02d%02d%02d%c", &y, &mon, &d, &t, &h, &min, &s, &z);
			DBG("%d %d %d %d %d %d", y, mon, d, h, min, s);
			break;

		case __AALARM_TYPE:
			break;

		case __AALARM_VALUE:
			break;

		case __AALARM_SNOOZETIME:
			break;

		case __AALARM_REPEATCOUNT:
			break;

		case __AALARM_AUDIOCONTENT:
			break;

		}
	}

	char *tzid = NULL;
	long long int run_utime = 0;
	int diff = 0;
	int tick = 0, unit = 0;
	calendar_record_h alarm = NULL;
	calendar_time_s caltime = {0};
	switch (type)
	{
	case VCALENDAR_TYPE_VEVENT:
		ret = calendar_record_get_caltime(record, _calendar_event.start_time, &caltime);
		if (z != 'Z')
		{
			ret = calendar_record_get_str_p(record, _calendar_event.start_tzid, &tzid);
			run_utime = _cal_time_convert_itol(tzid, y, mon, d, h, min, s);
			if (tzid) free(tzid);
		}
		else
		{
			run_utime = _cal_time_convert_itol(NULL, y, mon, d, h, min, s);
		}

		switch (caltime.type)
		{
		case CALENDAR_TIME_UTIME:
			diff = (int)(caltime.time.utime - run_utime);
			DBG("diff(%d) = (%lld) - (%lld)", diff, caltime.time.utime, run_utime);

			if (diff / (60 * 60 * 24 * 7) > 0)
			{
				unit = CALENDAR_ALARM_TIME_UNIT_WEEK;
				tick = diff /(60 * 60 * 24 * 7);
			}
			else if (diff / (60 * 60 * 24 ) > 0)
			{
				unit = CALENDAR_ALARM_TIME_UNIT_DAY;
				tick = diff /(60 * 60 * 24);
			}
			else if (diff / (60 * 60) > 0)
			{
				unit = CALENDAR_ALARM_TIME_UNIT_HOUR;
				tick = diff / (60 * 60);
			}
			else
			{
				unit = CALENDAR_ALARM_TIME_UNIT_MINUTE;
				tick = diff / 60;
			}
			break;

		case CALENDAR_TIME_LOCALTIME:
			break;
		}

		ret = calendar_record_create(_calendar_alarm._uri, &alarm);
		if (CALENDAR_ERROR_NONE != ret)
		{
			ERR("calendar_record_create() failed");
			return ret;
		}
		calendar_record_set_int(alarm, _calendar_alarm.tick, tick);
		calendar_record_set_int(alarm, _calendar_alarm.tick_unit, unit);
		calendar_record_add_child_record(record, _calendar_event.calendar_alarm, alarm);
		break;

	case VCALENDAR_TYPE_VTODO:
		ret = calendar_record_get_caltime(record, _calendar_todo.due_time, &caltime);
		if (z != 'Z')
		{
			ret = calendar_record_get_str_p(record, _calendar_todo.due_tzid, &tzid);
			run_utime = _cal_time_convert_itol(tzid, y, mon, d, h, min, s);
			if (tzid) free(tzid);
		}
		else
		{
			run_utime = _cal_time_convert_itol(NULL, y, mon, d, h, min, s);
		}

		switch (caltime.type)
		{
		case CALENDAR_TIME_UTIME:
			diff = (int)(caltime.time.utime - run_utime);

			if (diff / (60 * 60 * 24 * 7) > 0)
			{
				unit = CALENDAR_ALARM_TIME_UNIT_WEEK;
				tick = diff /(60 * 60 * 24 * 7);
			}
			else if (diff / (60 * 60 * 24 ) > 0)
			{
				unit = CALENDAR_ALARM_TIME_UNIT_DAY;
				tick = diff /(60 * 60 * 24);
			}
			else if (diff / (60 * 60) > 0)
			{
				unit = CALENDAR_ALARM_TIME_UNIT_HOUR;
				tick = diff / (60 * 60);
			}
			else
			{
				unit = CALENDAR_ALARM_TIME_UNIT_MINUTE;
				tick = diff / 60;
			}
			break;

			break;
		case CALENDAR_TIME_LOCALTIME:
			break;
		}
		ret = calendar_record_create(_calendar_alarm._uri, &alarm);
		if (CALENDAR_ERROR_NONE != ret)
		{
			ERR("calendar_record_create() failed");
			return ret;
		}
		calendar_record_set_int(alarm, _calendar_alarm.tick, tick);
		calendar_record_set_int(alarm, _calendar_alarm.tick_unit, unit);
		calendar_record_add_child_record(record, _calendar_todo.calendar_alarm, alarm);
		break;
	}

	return CALENDAR_ERROR_NONE;
}
/* end */


enum {
	WEEKNAME2_SA = 0x0,
	WEEKNAME2_FR,
	WEEKNAME2_TH,
	WEEKNAME2_WE,
	WEEKNAME2_TU,
	WEEKNAME2_MO,
	WEEKNAME2_SU,
	WEEKNAME2_MAX,
};
const char weekname2[WEEKNAME2_MAX][3] = {"SA", "FR", "TH", "WE", "TU", "MO", "SU"};

//alarm////////////////////////////////////////////////////////////
static int __cal_vcalendar_parse_action(calendar_record_h alarm, void *data)
{
	return CALENDAR_ERROR_NONE;
}

static char *__cal_vcalendar_parse_extract_duration(char *p, int *dur_t, char *dur)
{
	char du = '0';
	char buf[8] = {0, };
	int i = 0, c, d = 1;
	int t = 0;

	DBG("%s", p);
	while (*p != '\0' && *p != '\n') {
		switch (*p) {
		case '+':
			d = 1;
			break;
		case '-':
			d = -1;
			break;
		case 'P':
			i = 0;
			break;
		case 'T':
			break;
		case 'W':
			du = 'W';
			c = atoi(buf);
			t += c * 7 * 24 * 60 * 60;
			memset(buf, 0x0, sizeof(buf));
			i = 0;
			break;
		case 'D':
			du = 'D';
			c = atoi(buf);
			t += c * 24 * 60 * 60;
			memset(buf, 0x0, sizeof(buf));
			i = 0;
			break;
		case 'H':
			du = 'H';
			c = atoi(buf);
			t += c * 60 * 60;
			memset(buf, 0x0, sizeof(buf));
			i = 0;
			break;
		case 'M':
			du = 'M';
			c = atoi(buf);
			t += c * 60;
			memset(buf, 0x0, sizeof(buf));
			i = 0;
			break;
		case 'S':
			du = 'S';
			c = atoi(buf);
			t += c;
			memset(buf, 0x0, sizeof(buf));
			i = 0;
			break;
		default:
			buf[i] = *p;
			i++;
			break;

		}
		p++;
	}
	t *= d;
	*dur_t = t;

	if (dur) {
		*dur = du;
	}

	return p;
}

static int __cal_vcalendar_parse_trigger_time(calendar_record_h alarm, char *p)
{
	char t = 0, z;
	int y, mon, d, h, min, s;
	int tick, unit;
	cal_alarm_s *_alarm = (cal_alarm_s *)alarm;

	if (NULL == alarm || NULL == p)
	{
		ERR("Invalid parameter");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	sscanf(p, "%4d%2d%2d%c%2d%2d%2d%c", &y, &mon, &d, &t, &h, &min, &s, &z);

	tick = _alarm->remind_tick;
	unit = _alarm->remind_tick_unit;
	switch (unit)
	{
	case CALENDAR_ALARM_TIME_UNIT_WEEK:
		mon += tick;
		break;
	case CALENDAR_ALARM_TIME_UNIT_DAY:
		d += tick;
		break;
	case CALENDAR_ALARM_TIME_UNIT_HOUR:
		h += tick;
		break;
	case CALENDAR_ALARM_TIME_UNIT_MINUTE:
		min += tick;
		break;
	case CALENDAR_ALARM_TIME_UNIT_SPECIFIC:
	default:
		break;
	}

	if (t == 0)
	{
		int datetime = y *10000 + mon *100 + d;
		_cal_record_set_lli(alarm, _calendar_alarm.time, datetime);
		DBG("DATE(%d)", datetime);
	}
	else
	{
		long long int lli = _cal_time_convert_itol(NULL, y, mon, d, h, min, s);
		_cal_record_set_lli(alarm, _calendar_alarm.time, lli);
		DBG("DATE-TIME(%lld)", lli);
	}
	return CALENDAR_ERROR_NONE;
}

static int __cal_vcalendar_parse_duration_alarm(calendar_record_h alarm, void *data)
{
	int ret = CALENDAR_ERROR_NONE;
	char *p = (char *)data;
	char dur;
	int dur_t;
	int tick, unit;

	p++;

	__cal_vcalendar_parse_extract_duration(p, &dur_t, &dur);
	switch (dur) {
	case 'W':
		tick = dur_t/(7 *24 *60 *60);
		unit = CALENDAR_ALARM_TIME_UNIT_WEEK;
		break;
	case 'D':
		tick = dur_t/(24 *60 *60);
		unit = CALENDAR_ALARM_TIME_UNIT_DAY;
		break;
	case 'H':
		tick = dur_t/(60 *60);
		unit = CALENDAR_ALARM_TIME_UNIT_HOUR;
		break;
	case 'M':
		tick = dur_t/(60);
		unit = CALENDAR_ALARM_TIME_UNIT_MINUTE;
		break;
	default:
		tick = 1;
		unit = CALENDAR_ALARM_NONE;;
		break;
	}

	ret = _cal_record_set_int(alarm, _calendar_alarm.tick, tick);
	ret = _cal_record_set_int(alarm, _calendar_alarm.tick_unit, unit);
	DBG("tick(%d) unit(%d)", tick, unit);

	return ret;
}

static int __cal_vcalendar_parse_trigger(calendar_record_h alarm, void *data)
{
	int i = 0, out = 0;
	char *p = (char *)data;

	p++;

	// default unit
	_cal_record_set_int(alarm, _calendar_alarm.tick_unit, CALENDAR_ALARM_TIME_UNIT_SPECIFIC);

	while (*p != '\n' && *p != '\r' && *p != '\0') {

		for (i = 0; i < TRIG_MAX; i++) {
			if (!strncmp(p, _trig_funcs[i].prop, strlen(_trig_funcs[i].prop))) {
				out = 1;
				int j = 0;
				char buf[64] = {0, };
				p += strlen(_trig_funcs[i].prop);
				while (p[j] != ';' && p[j] != '\n' && p[j] != '\0') {
					buf[j] = p[j];
					j++;
				}
				if (p[j] != '\0') {
					buf[j] = '\0';
				}

				p += j;
				_trig_funcs[i].func(alarm, buf);
				break;
			}
		}
		if (out == 1) {
			break;
		}

		if (*p >= '1'  && *p <= '9')
		{
			__cal_vcalendar_parse_trigger_time(alarm, p);
		}
		else
		{
			__cal_vcalendar_parse_duration_alarm(alarm, p);
		}
		break;
	}
	return CALENDAR_ERROR_NONE;
}

static int __cal_vcalendar_parse_repeat(calendar_record_h alarm, void *data)
{
	return CALENDAR_ERROR_NONE;
}

static int __cal_vcalendar_parse_attach_alarm(calendar_record_h alarm, void *data)
{
	return CALENDAR_ERROR_NONE;
}

static int __cal_vcalendar_parse_summary_alarm(calendar_record_h alarm, void *data)
{
	int ret;
	char *p = (char *)data;

	p++;

	ret = _cal_record_set_str(alarm, _calendar_alarm.description, p);
	DBG("alarm description[%s]", p);
	return CALENDAR_ERROR_NONE;
}


//rrule////////////////////////////////////////////////////////////
static int __cal_vcalendar_parse_freq(calendar_record_h event, void *data)
{
	int ret;
	int freq = -1;
	char *p = (char *)data;

	DBG("%s\n", (char *)data);
	if (!strncmp(p, "YEARLY", strlen("YEARLY"))) {
		freq = CALENDAR_RECURRENCE_YEARLY;

	} else if (!strncmp(p, "MONTHLY", strlen("MONTHLY"))) {
		freq = CALENDAR_RECURRENCE_MONTHLY;

	} else if (!strncmp(p, "WEEKLY", strlen("WEEKLY"))) {
		freq = CALENDAR_RECURRENCE_WEEKLY;

	} else if (!strncmp(p, "DAILY", strlen("DAILY"))) {
		freq = CALENDAR_RECURRENCE_DAILY;

	} else if (!strncmp(p, "HOURLY", strlen("HOURLY"))) {
		freq = CALENDAR_RECURRENCE_NONE;

	} else if (!strncmp(p, "MINUTELY", strlen("MINUTELY"))) {
		freq = CALENDAR_RECURRENCE_NONE;

	} else if (!strncmp(p, "SECONDLY", strlen("SECONDLY"))) {
		freq = CALENDAR_RECURRENCE_NONE;

	} else {
		freq = CALENDAR_RECURRENCE_NONE;

	}
	ret = _cal_record_set_int(event, _calendar_event.freq, freq);
	return CALENDAR_ERROR_NONE;
}

static int __cal_vcalendar_parse_until(calendar_record_h event, void *data)
{
	int ret;
	calendar_time_s stime;
	calendar_time_s until;
	int y, mon, d, h, min, s;
	char *tzid;
	char t, z;
	char *p = (char *)data;

	/* until value type has the same value as the dtstart */
	ret = _cal_record_set_int(event, _calendar_event.range_type, CALENDAR_RANGE_UNTIL);

	ret = calendar_record_get_str_p(event, _calendar_event.start_tzid, &tzid);
	ret = calendar_record_get_caltime(event, _calendar_event.start_time, &stime);
	until.type = stime.type;

	switch (stime.type)
	{
	case CALENDAR_TIME_UTIME:
		until.time.utime = _cal_time_convert_stol(tzid, p);
		break;

	case CALENDAR_TIME_LOCALTIME:
		sscanf(p, "%4d%2d%2d%c%2d%2d%2d%c", &y, &mon, &d, &t, &h, &min, &s, &z);
		until.time.date.year = y;
		until.time.date.month = mon;
		until.time.date.mday = d;

		break;
	}
	ret = _cal_record_set_caltime(event, _calendar_event.until_time, until);

	return CALENDAR_ERROR_NONE;
}

static int __cal_vcalendar_parse_count(calendar_record_h event, void *data)
{
	int ret;
	int c;
	char *p = (char *)data;

	DBG("%s\n", (char *)data);
	ret = _cal_record_set_int(event, _calendar_event.range_type, CALENDAR_RANGE_COUNT);
	c = atoi(p);
	ret = _cal_record_set_int(event, _calendar_event.count, c < 0 ? 0 : c);
	return ret;
}

static int __cal_vcalendar_parse_interval(calendar_record_h event, void *data)
{
	DBG("%s\n", (char *)data);
	int c;
	char *p = (char *)data;

	c = atoi(p);
	return _cal_record_set_int(event, _calendar_event.interval, c < 0 ? 0 : c);
}

static int __cal_vcalendar_parse_bysecond(calendar_record_h event, void *data)
{
	DBG("%s\n", (char *)data);
	return _cal_record_set_str(event, _calendar_event.bysecond, (char *)data);
}

static int __cal_vcalendar_parse_byminute(calendar_record_h event, void *data)
{
	DBG("%s\n", (char *)data);
	return _cal_record_set_str(event, _calendar_event.byminute, (char *)data);
}

static int __cal_vcalendar_parse_byhour(calendar_record_h event, void *data)
{
	DBG("%s\n", (char *)data);
	return _cal_record_set_str(event, _calendar_event.byhour, (char *)data);
}

static int __cal_vcalendar_parse_byday(calendar_record_h event, void *data)
{
	DBG("%s\n", (char *)data);
	return _cal_record_set_str(event, _calendar_event.byday, (char *)data);
}

static int __cal_vcalendar_parse_bymonthday(calendar_record_h event, void *data)
{
	DBG("%s\n", (char *)data);
	return _cal_record_set_str(event, _calendar_event.bymonthday, (char *)data);
}

static int __cal_vcalendar_parse_byyearday(calendar_record_h event, void *data)
{
	DBG("%s\n", (char *)data);
	return _cal_record_set_str(event, _calendar_event.byyearday, (char *)data);
}

static int __cal_vcalendar_parse_byweekno(calendar_record_h event, void *data)
{
	DBG("%s\n", (char *)data);
	return _cal_record_set_str(event, _calendar_event.byweekno, (char *)data);
}

static int __cal_vcalendar_parse_bymonth(calendar_record_h event, void *data)
{
	DBG("%s\n", (char *)data);
	return _cal_record_set_str(event, _calendar_event.bymonth, (char *)data);
}

static int __cal_vcalendar_parse_bysetpos(calendar_record_h event, void *data)
{
	DBG("%s\n", (char *)data);
	return _cal_record_set_str(event, _calendar_event.bysetpos, (char *)data);
}

static int __cal_vcalendar_parse_wkst(calendar_record_h event, void *data)
{
	DBG("%s\n", (char *)data);
	int wkst;
	char *p = (char *)data;

	if (!strncmp(p, "SU", strlen("SU"))) {
		wkst = CALENDAR_SUNDAY;

	} else if (!strncmp(p, "MO", strlen("MO"))) {
		wkst = CALENDAR_MONDAY;

	} else if (!strncmp(p, "TU", strlen("TU"))) {
		wkst = CALENDAR_TUESDAY;

	} else if (!strncmp(p, "WE", strlen("WE"))) {
		wkst = CALENDAR_WEDNESDAY;

	} else if (!strncmp(p, "TH", strlen("TH"))) {
		wkst = CALENDAR_THURSDAY;

	} else if (!strncmp(p, "FR", strlen("FR"))) {
		wkst = CALENDAR_FRIDAY;

	} else if (!strncmp(p, "SA", strlen("SA"))) {
		wkst = CALENDAR_SATURDAY;

	} else {
		wkst = -1;
	}
	return _cal_record_set_int(event, _calendar_event.wkst, wkst);
}

static int __get_tick_unit(char *p, int *tick, int *unit)
{
	int d, c, i = 0; /* direct, const, i */
	int t, u; /* tick, unit */
	char buf[8] = {0};

	t = 0;
	c = 0;
	u = CAL_SCH_TIME_UNIT_OFF;
	while (*p != '\0' && *p != '\n') {
		switch (*p) {
		case '+':
			d = 1;
			break;
		case '-':
			d = -1;
			break;
		case 'P':
			i = 0;
			break;
		case 'T':
			break;
		case 'W':
			c = atoi(buf);
			DBG("W tick(%d)", c);
			if (c == 0) break;
			u = CALENDAR_ALARM_TIME_UNIT_WEEK;
			t += c;
			i = 0;
			break;
		case 'D':
			c = atoi(buf);
			DBG("D tick(%d)", c);
			if (c == 0) break;
			u = CALENDAR_ALARM_TIME_UNIT_DAY;
			t += c;
			i = 0;
			break;
		case 'H':
			c = atoi(buf);
			DBG("H tick(%d)", c);
			if (c == 0) break;
			u = CALENDAR_ALARM_TIME_UNIT_HOUR;
			t += c;
			i = 0;
			break;
		case 'M':
			c = atoi(buf);
			DBG("M tick(%d)", c);
			if (c == 0) break;
			u = CALENDAR_ALARM_TIME_UNIT_MINUTE;
			t += c;
			i = 0;
			break;
		case 'S':
			i = 0;
			break;
		default:
			buf[i] = *p;
			i++;
			break;
		}
		p++;
	}
	if (t != c) {
		u = CALENDAR_ALARM_TIME_UNIT_SPECIFIC;
	}
	*tick = t;
	*unit = u;
	DBG("get tic(%d) unit(%d)", t, u);

	return CALENDAR_ERROR_NONE;
}

static int __cal_vcalendar_parse_trig_related(calendar_record_h alarm, void *data)
{
	DBG("%s\n", (char *)data);

	int tick = 0, unit;
	char *p = (char *)data;

	if (p == NULL) {
		return -1;
	}

	if (!strncmp(p, "START", strlen("START") + 1)) {
		p += strlen("START") + 1;
		DBG("related start and value[%s]", p);

	} else if (!strncmp(p, "END", strlen("END") +1)) {
		p += strlen("END") + 1;
		DBG("related end and value[%s]", p);

	} else {
		DBG("no related and value[%s]", p);

	}
	__get_tick_unit(p, &tick, &unit);
	_cal_record_set_int(alarm, _calendar_alarm.tick, tick);
	_cal_record_set_int(alarm, _calendar_alarm.tick_unit, unit);

	return CALENDAR_ERROR_NONE;
}

long long int _get_utime_from_datetime(char *tzid, char *p)
{
	int y, mon, d, h, min, s;
	int len;
	char t, z;
	if (p == NULL) {
		return -1;
	}
	len = strlen(p);
	if (len < strlen("YYYYMMDDTHHMMSS")) {
		return -1;
	}

	sscanf(p, "%04d%02d%02d%c%02d%02d%02d%c",
			&y, &mon, &d, &t, &h, &min, &s, &z);

	return _cal_time_convert_itol(tzid, y, mon, d, h, min, s);
}

static int __cal_vcalendar_parse_trig_value(calendar_record_h alarm, void *data)
{
	DBG("%s\n", (char *)data);

	int ret;
	char *p = (char *)data;

	if (!strncmp(p, "DATE-TIME", strlen("DATE-TIME") + 1)) {
		p += strlen("DATE-TIME") + 1;
		ret = _cal_record_set_int(alarm, _calendar_alarm.tick_unit, CALENDAR_ALARM_TIME_UNIT_SPECIFIC);
		ret = _cal_record_set_lli(alarm, _calendar_alarm.time, _cal_time_convert_stol(NULL, p));
	} else {

	}
	return CALENDAR_ERROR_NONE;
}

static int __cal_vcalendar_parse_charset(int *val, void *data)
{
	DBG("%s\n", (char *)data);
	return CALENDAR_ERROR_NONE;
}

static int __cal_vcalendar_parse_encoding(int *val, void *data)
{
	char *p = (char *)data;
	*val = 0;

	if (!strncmp(p, "BASE64", strlen("BASE64"))) {
		DBG("ENCODE_BASE64");
		*val = ENCODE_BASE64;

	} else if (!strncmp(p, "QUOTED-PRINTABLE", strlen("QUOTED-PRINTABLE"))){
		DBG("ENCODE_QUOTED_PRINTABLE");
		*val = ENCODE_QUOTED_PRINTABLE;

	}
	return CALENDAR_ERROR_NONE;
}

// end parse func////////////////////////////////////////////////////////////////

char *_cal_vcalendar_parse_vevent(int type, calendar_list_h *list_sch, void *data)
{
	DBG("[%s]", __func__);
	int i;
	int ret;
	char *prop = NULL, *cont = NULL;
	char *cursor = (char *)data;
	calendar_record_h event = NULL;

	ret = calendar_record_create(_calendar_event._uri, &event);
	if (CALENDAR_ERROR_NONE != ret)
	{
		ERR("calendar_record_create() failed");
		return NULL;
	}

	while ((cursor = _cal_vcalendar_parse_read_line(cursor, &prop, &cont))) {
		if (!strncmp(cont + 1, "VALARM", strlen("VALARM"))) {
			if (!strncmp(cont + 1, "VALARM", strlen("VALARM"))) {
				cursor = _cal_vcalendar_parse_valarm(VCALENDAR_TYPE_VEVENT, event, cursor);
			} else {
				break;
			}

		} else if (!strncmp(prop, "END", strlen("END"))) {
			break;

		} else {
			for (i = 0; i < VEVE_MAX; i++) {
				if (!strncmp(prop, _vevent_funcs[i].prop, strlen(_vevent_funcs[i].prop))) {
					_vevent_funcs[i].func(type, *list_sch, event, prop, cont);
					break;
				}
			}
		}

		CAL_FREE(prop);
		CAL_FREE(cont);
	}

	CAL_FREE(prop);
	CAL_FREE(cont);

	DBG("event add to the list");
	ret = calendar_list_add(*list_sch, event);

	return cursor;
}

char *_cal_vcalendar_parse_vtodo(int type, calendar_list_h *list_sch, void *data)
{
	int i;
	int ret;
	char *prop = NULL, *cont = NULL;
	char *cursor = (char *)data;
	calendar_record_h todo = NULL;

	ret = calendar_record_create(_calendar_todo._uri, &todo);
	if (CALENDAR_ERROR_NONE != ret)
	{
		ERR("calendar_record_create() failed");
		return NULL;
	}

	/* do until meet BEGIN */
	while ((cursor = _cal_vcalendar_parse_read_line(cursor, &prop, &cont))) {
		if (!strncmp(prop, "BEGIN", strlen("BEGIN"))) {

			if (!strncmp(cont + 1, "VALARM", strlen("VALARM"))) {
				cursor = _cal_vcalendar_parse_valarm(VCALENDAR_TYPE_VTODO, todo, cursor);
			} else {
				break;
			}

		} else if (!strncmp(prop, "END", strlen("END"))) {
			break;

		} else {
			for (i = 0; i < VTODO_MAX; i++) {
				if (!strncmp(prop, _vtodo_funcs[i].prop, strlen(_vtodo_funcs[i].prop))) {
					_vtodo_funcs[i].func(type, *list_sch, todo, prop, cont);
					break;
				}
			}
		}

		CAL_FREE(prop);
		CAL_FREE(cont);
	}

	CAL_FREE(prop);
	CAL_FREE(cont);

	ret = calendar_list_add(*list_sch, todo);

	return cursor;
}

char *_cal_vcalendar_parse_valarm(int type, calendar_record_h record, void *data)
{
	int ret;
	int i;
	char *prop = NULL, *cont = NULL;
	char *cursor = (char *)data;
	calendar_record_h alarm = NULL;

	ret = calendar_record_create(_calendar_alarm._uri, &alarm);

	while ((cursor = _cal_vcalendar_parse_read_line(cursor, &prop, &cont))) {
		if (!strncmp(prop, "BEGIN", strlen("BEGIN"))) {
			break;

		} else if (!strncmp(prop, "END", strlen("END"))) {
			break;

		}

		for (i = 0; i < VALA_MAX; i++) {
			if (!strncmp(prop, _valarm_funcs[i].prop, strlen(_valarm_funcs[i].prop))) {
				_valarm_funcs[i].func(alarm, cont);
				break;
			}
		}
		CAL_FREE(prop);
		CAL_FREE(cont);
	}

	switch (type) {
	case VCALENDAR_TYPE_VEVENT:
		ret = calendar_record_add_child_record(record, _calendar_event.calendar_alarm, alarm);
		break;
	case VCALENDAR_TYPE_VTODO:
		ret = calendar_record_add_child_record(record, _calendar_todo.calendar_alarm, alarm);
		break;
	}

	CAL_FREE(prop);
	CAL_FREE(cont);

	return cursor;
}

/*
 * parse vtimezone
 */
enum {
	VTIMEZONE_STD_DTSTART = 0x0,
	VTIMEZONE_STD_TZOFFSETFROM,
	VTIMEZONE_STD_TZOFFSETTO,
	VTIMEZONE_STD_MAX,
};

static int __cal_vcalendar_parse_vtimezone_std_dtstart(calendar_record_h timezone, void *data)
{
	int y, mon, d, h, min, s;
	int nth = 0, wday = 0;
	long long int utime;
	char t, z;
	char *p = (char *)data;

	if (NULL == timezone)
	{
		ERR("Invalid parameter");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	sscanf(p +1, "%04d%02d%02d%c%02d%02d%02d%c", &y, &mon, &d, &t, &h, &min, &s, &z);
	utime = _cal_time_convert_itol(NULL, y, mon, d, h, min, s);
	_cal_time_ltoi2(NULL, utime, &nth, &wday);
	DBG("timezone dtstart(%04d-%02d-%02d %02d:%02d:%02d", y, mon, d, h, min, s);
	DBG("timezone day of week(%d/%d)", nth, wday);
	_cal_record_set_int(timezone, _calendar_timezone.standard_start_month, mon);
	_cal_record_set_int(timezone, _calendar_timezone.standard_start_position_of_week, nth);
	_cal_record_set_int(timezone, _calendar_timezone.standard_start_day, wday);
	_cal_record_set_int(timezone, _calendar_timezone.standard_start_hour, h);
	return CALENDAR_ERROR_NONE;
}

static int __cal_vcalendar_parse_vtimezone_std_tzoffsetfrom(calendar_record_h timezone, void *data)
{
	return 0;
}

static int __cal_vcalendar_parse_vtimezone_std_tzoffsetto(calendar_record_h timezone, void *data)
{
	int h, m;
	char *p = (char *)data;
	char c;
	cal_timezone_s *tz = (cal_timezone_s *)timezone;

	if (NULL == timezone)
	{
		ERR("Invalid parameter");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	sscanf(p +1, "%c%02d%02d", &c, &h, &m);
	DBG("timezone standard offsetto(%c)(%02d)(%02d)", c, h, m);
	if (c == '-')
	{
		h *= -1;
	}
	if (tz->day_light_bias)
	{
		// this means daylight is set before gmt offset
		_cal_record_set_int(timezone, _calendar_timezone.day_light_bias,
				tz->day_light_bias - (h * 60 + m));
	}
	_cal_record_set_int(timezone, _calendar_timezone.tz_offset_from_gmt, h * 60 + m);
	return CALENDAR_ERROR_NONE;
}

struct _record_func _vtimezone_std[VTIMEZONE_STD_MAX] =
{
	{"DTSTART", __cal_vcalendar_parse_vtimezone_std_dtstart },
	{"TZOFFSETFROM", __cal_vcalendar_parse_vtimezone_std_tzoffsetfrom },
	{"TZOFFSETTO", __cal_vcalendar_parse_vtimezone_std_tzoffsetto }
};

char *_cal_vcalendar_parse_standard(calendar_record_h timezone, void *data)
{
	DBG("[%s]", __func__);
	int i;
	char *prop = NULL, *cont = NULL;
	char *cursor = (char *)data;

	while ((cursor = _cal_vcalendar_parse_read_line(cursor, &prop, &cont))) {
		if (!strncmp(prop, "END", strlen("END"))) {
			break;
		}

		for (i = 0; i < VTIMEZONE_STD_MAX; i++)
		{
			if (!strncmp(prop, _vtimezone_std[i].prop, strlen(_vtimezone_std[i].prop))) {
				_vtimezone_std[i].func(timezone, cont);
				break;
			}
		}

		CAL_FREE(prop);
		CAL_FREE(cont);
	}

	CAL_FREE(prop);
	CAL_FREE(cont);

	return cursor;
}

enum {
	VTIMEZONE_DST_DTSTART = 0x0,
	VTIMEZONE_DST_TZOFFSETFROM,
	VTIMEZONE_DST_TZOFFSETTO,
	VTIMEZONE_DST_MAX,
};

static int __cal_vcalendar_parse_vtimezone_dst_dtstart(calendar_record_h timezone, void *data)
{
	int y, mon, d, h, min, s;
	int nth = 0, wday = 0;
	long long int utime;
	char t, z;
	char *p = (char *)data;

	if (NULL == timezone)
	{
		ERR("Invalid parameter");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	sscanf(p +1, "%04d%02d%02d%c%02d%02d%02d%c", &y, &mon, &d, &t, &h, &min, &s, &z);
	utime = _cal_time_convert_itol(NULL, y, mon, d, h, min, s);
	_cal_time_ltoi2(NULL, utime, &nth, &wday);
	DBG("timezone daylight(%04d-%02d-%02d %02d:%02d:%02d", y, mon, d, h, min, s);
	DBG("timezone daylight day of week(%dth/%d)", nth, wday);
	_cal_record_set_int(timezone, _calendar_timezone.day_light_start_month, mon);
	_cal_record_set_int(timezone, _calendar_timezone.day_light_start_position_of_week, nth);
	_cal_record_set_int(timezone, _calendar_timezone.day_light_start_day, wday);
	_cal_record_set_int(timezone, _calendar_timezone.day_light_start_hour, h);
	return CALENDAR_ERROR_NONE;
}

static int __cal_vcalendar_parse_vtimezone_dst_tzoffsetfrom(calendar_record_h timezone, void *data)
{
	return 0;
}

static int __cal_vcalendar_parse_vtimezone_dst_tzoffsetto(calendar_record_h timezone, void *data)
{
	int h, m;
	char *p = (char *)data;
	char c;
	cal_timezone_s *tz = (cal_timezone_s *)timezone;

	if (NULL == timezone)
	{
		ERR("Invalid parameter");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	sscanf(p +1, "%c%02d%02d", &c, &h, &m);
	DBG("timezone offsetto(%c)(%02d)(%02d)", c, h, m);
	if (c == '-')
	{
		h *= -1;
	}
	_cal_record_set_int(timezone, _calendar_timezone.day_light_bias,
			(h * 60 + m) - tz->tz_offset_from_gmt);
	return CALENDAR_ERROR_NONE;
}

struct _record_func _vtimezone_dst[VTIMEZONE_STD_MAX] =
{
	{"DTSTART", __cal_vcalendar_parse_vtimezone_dst_dtstart },
	{"TZOFFSETFROM", __cal_vcalendar_parse_vtimezone_dst_tzoffsetfrom },
	{"TZOFFSETTO", __cal_vcalendar_parse_vtimezone_dst_tzoffsetto }
};

char *_cal_vcalendar_parse_daylight(calendar_record_h timezone, void *data)
{
	DBG("[%s]", __func__);
	int i;
	char *prop = NULL, *cont = NULL;
	char *cursor = (char *)data;

	while ((cursor = _cal_vcalendar_parse_read_line(cursor, &prop, &cont))) {
		if (!strncmp(prop, "END", strlen("END"))) {
			break;
		}

		for (i = 0; i < VTIMEZONE_DST_MAX; i++)
		{
			if (!strncmp(prop, _vtimezone_dst[i].prop, strlen(_vtimezone_dst[i].prop))) {
				_vtimezone_dst[i].func(timezone, cont);
				break;
			}
		}

		CAL_FREE(prop);
		CAL_FREE(cont);
	}

	CAL_FREE(prop);
	CAL_FREE(cont);

	return cursor;
}

char *_cal_vcalendar_parse_vtimezone(int ver, calendar_list_h *list_sch, void *data)
{
	int ret = CALENDAR_ERROR_NONE;
	char *prop = NULL, *cont = NULL;
	char *cursor = (char *)data;
	calendar_record_h timezone = NULL;

	ret = calendar_record_create(_calendar_timezone._uri, &timezone);
	if (CALENDAR_ERROR_NONE != ret)
	{
		ERR("calendar_record_create() failed");
		return NULL;
	}

	while ((cursor = _cal_vcalendar_parse_read_line(cursor, &prop, &cont)))
	{
		if (!strncmp(prop, "TZID", strlen("TZID")))
		{
			_cal_record_set_str(timezone, _calendar_timezone.standard_name, cont +1);
			_cal_record_set_str(timezone, _calendar_timezone.day_light_name, cont +1);
			DBG("name[%s]", cont +1);
		}
		else if (!strncmp(prop, "BEGIN", strlen("BEGIN")))
		{
			if (!strncmp(cont + 1, "STANDARD", strlen("STANDARD")))
			{
				cursor = _cal_vcalendar_parse_standard(timezone, cursor);
			}
			else if (!strncmp(cont + 1, "DAYLIGHT", strlen("DAYLIGHT")))
			{
				cursor = _cal_vcalendar_parse_daylight(timezone, cursor);
			}
			else
			{
				DBG("Error");
			}
		}
		else if (!strncmp(prop, "END", strlen("END")))
		{
			if (!strncmp(cont + 1, "VTIMEZONE", strlen("VTIMEZONE")))
			{
				break;
			}
			else
			{
				DBG("Error");
			}
		}
		else
		{
				DBG("Error");
		}
		CAL_FREE(prop);
		CAL_FREE(cont);
	}

	CAL_FREE(prop);
	CAL_FREE(cont);

	DBG("add timezone to the list");
	calendar_list_add(*list_sch, timezone);

	return cursor;
}

/*
 * parses vcalendar and appends record to the list.
 */
char *_cal_vcalendar_parse_vcalendar(calendar_list_h *list, void *data)
{
	DBG("[%s]", __func__);
	int ret = CALENDAR_ERROR_NONE;
	char *prop = NULL, *cont = NULL;
	char *cursor = (char *)data;
	char *p = cursor;
	calendar_list_h l = NULL;

	if (NULL == list)
	{
		ERR("Invalid parameter: list is NULL");
		return NULL;
	}

	ret = calendar_list_create(&l);
	if (CALENDAR_ERROR_NONE != ret)
	{
		ERR("calendar_list_create() failed");
		return NULL;
	}

	while ((cursor = _cal_vcalendar_parse_read_line(cursor, &prop, &cont))) {
		if (prop == NULL || cont == NULL)
		{
			ERR("Failed to parse");
			break;
		}
		if (!strncmp(prop, "BEGIN", strlen("BEGIN"))) {
			CAL_FREE(prop);
			CAL_FREE(cont);
			break;
		}

		calendar_record_h extended = NULL;
		ret = calendar_record_create(_calendar_extended_property._uri, &extended);
		if (CALENDAR_ERROR_NONE != ret)
		{
			ERR("calendar_record_create() failed");
			CAL_FREE(prop);
			CAL_FREE(cont);
			break;
		}

		ret = calendar_record_set_str(extended, _calendar_extended_property.key, prop);
		if (CALENDAR_ERROR_NONE != ret)
		{
			ERR("calendar_record_set_str() failed");
			CAL_FREE(prop);
			CAL_FREE(cont);
			calendar_record_destroy(extended, true);
			break;
		}

		ret = calendar_record_set_str(extended, _calendar_extended_property.value, cont);
		if (CALENDAR_ERROR_NONE != ret)
		{
			ERR("calendar_record_set_str() failed");
			CAL_FREE(prop);
			CAL_FREE(cont);
			calendar_record_destroy(extended, true);
			break;
		}

		ret = calendar_list_add(l, extended);
		if (CALENDAR_ERROR_NONE != ret)
		{
			ERR("calendar_record_set_str() failed");
			CAL_FREE(prop);
			CAL_FREE(cont);
			calendar_record_destroy(extended, true);
			break;
		}

		CAL_FREE(prop);
		CAL_FREE(cont);
		p = cursor;
	}

	cursor = p;
	while ((cursor = _cal_vcalendar_parse_read_line(cursor, &prop, &cont))) {
		if (prop == NULL || cont == NULL)
		{
			ERR("Failed to parse");
			break;
		}
		if (!strncmp(prop, "BEGIN", strlen("BEGIN"))) {

			if (!strncmp(cont + 1, "VEVENT", strlen("VEVENT"))) {
				cursor = _cal_vcalendar_parse_vevent(CALENDAR_BOOK_TYPE_EVENT, &l, cursor);

			} else if (!strncmp(cont + 1, "VTODO", strlen("VTODO"))) {
				cursor = _cal_vcalendar_parse_vtodo(CALENDAR_BOOK_TYPE_TODO, &l, cursor);

			} else if (!strncmp(cont + 1, "VTIMEZONE", strlen("VTIMEZONE"))) {
				cursor = _cal_vcalendar_parse_vtimezone(CALENDAR_BOOK_TYPE_NONE, &l, cursor);
/*
			} else if (!strncmp(cont + 1, "VFREEBUSY", strlen("VFREEBUSY"))) {
*/
			} else {

			}

		} else if (!strncmp(prop, "END:VCALENDAR", strlen("END:VCALENDAR"))) {
			CAL_FREE(prop);
			CAL_FREE(cont);
			break;
		}

		CAL_FREE(prop);
		CAL_FREE(cont);
	}
	CAL_FREE(prop);
	CAL_FREE(cont);

	*list = l;

	return cursor;
}

