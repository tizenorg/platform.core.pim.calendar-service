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

#include <stdlib.h>
#include <ctype.h>
#include <vconf.h>

#include "calendar_list.h"
#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_record.h"
#include "cal_view.h"
#include "cal_time.h"
#include "cal_vcalendar.h"
#include "cal_vcalendar_parse.h"
#include "cal_utils.h"

/*
 * LF | \n | 0x0A
 * CR | \r | 0x0D
 */
#define VCAL_LF 0x0A
#define VCAL_CR 0x0D

struct user_data {
	char *timezone_tzid; /* TZ(ver1) VTIMEZONE(ver2) */
	char *datetime_tzid; /* in vevent, vtodo as param: TZID=US-Eastern */
	int version;
	int type; /* event, todo */
	bool is_allday;
	bool has_rrule;
};

enum {
	VCAL_RELATED_NONE,
	VCAL_RELATED_START,
	VCAL_RELATED_END,
};

enum {
	VCAL_RECURRENCE_NONE,
	VCAL_RECURRENCE_YEARLY_BYYEARDAY,
	VCAL_RECURRENCE_YEARLY_BYWEEKLY,
	VCAL_RECURRENCE_YEARLY_BYMONTH,
	VCAL_RECURRENCE_YEARLY_BYMONTHDAY,
	VCAL_RECURRENCE_YEARLY_BYDAY,
	VCAL_RECURRENCE_MONTHLY_BYMONTHDAY,
	VCAL_RECURRENCE_MONTHLY_BYDAY,
	VCAL_RECURRENCE_WEEKLY,
	VCAL_RECURRENCE_DAILY,
};

enum {
	VCAL_VER_1 = 1,
	VCAL_VER_2 = 2,
};

enum {
	VCAL_DATETIME_LENGTH_YYYYMMDD = 8,
	VCAL_DATETIME_LENGTH_YYYYMMDDTHHMMSS = 15,
	VCAL_DATETIME_LENGTH_YYYYMMDDTHHMMSSZ = 16,
};

enum {
	VCAL_COMPONENT_NONE,
	VCAL_COMPONENT_VEVENT,
	VCAL_COMPONENT_VTODO,
	VCAL_COMPONENT_VJOURNAL,
	VCAL_COMPONENT_VFREEBUSY,
	VCAL_COMPONENT_VTIMEZONE,
	VCAL_COMPONENT_MAX,
};

enum {
	VCAL_PROPERTY_NONE,
	VCAL_PROPERTY_VERSION,
	VCAL_PROPERTY_TZ,
	VCAL_PROPERTY_BEGIN,
	VCAL_PROPERTY_END,
	VCAL_PROPERTY_MAX,
};

enum {
	VCAL_COMPONENT_PROPERTY_NONE,
	VCAL_COMPONENT_PROPERTY_DTSTAMP,
	VCAL_COMPONENT_PROPERTY_UID,
	VCAL_COMPONENT_PROPERTY_RECURRENCE_ID,
	VCAL_COMPONENT_PROPERTY_DTSTART,
	VCAL_COMPONENT_PROPERTY_CREATED,	/* for ver 2: created */
	VCAL_COMPONENT_PROPERTY_DCREATED,	/* for ver 1: created */
	VCAL_COMPONENT_PROPERTY_DESCRIPTION,
	VCAL_COMPONENT_PROPERTY_LAST_MODIFIED,
	VCAL_COMPONENT_PROPERTY_LOCATION,
	VCAL_COMPONENT_PROPERTY_PRIORITY,
	VCAL_COMPONENT_PROPERTY_STATUS,
	VCAL_COMPONENT_PROPERTY_SUMMARY,
	VCAL_COMPONENT_PROPERTY_RRULE,
	VCAL_COMPONENT_PROPERTY_DTEND,
	VCAL_COMPONENT_PROPERTY_DUE,
	VCAL_COMPONENT_PROPERTY_ATTENDEE,
	VCAL_COMPONENT_PROPERTY_CATEGORIES,
	VCAL_COMPONENT_PROPERTY_DALARM,		/* for ver 1: display alarm */
	VCAL_COMPONENT_PROPERTY_MALARM,		/* for ver 1: mail alarm */
	VCAL_COMPONENT_PROPERTY_AALARM,		/* for ver 1: audio alarm */
	VCAL_COMPONENT_PROPERTY_EXDATE,
	VCAL_COMPONENT_PROPERTY_X_ALLDAY,
	VCAL_COMPONENT_PROPERTY_X_LUNAR,
	VCAL_COMPONENT_PROPERTY_BEGIN,
	VCAL_COMPONENT_PROPERTY_END,
	VCAL_COMPONENT_PROPERTY_EXTENDED,
	VCAL_COMPONENT_PROPERTY_MAX,
};

enum {
	VCAL_COMPONENT_PROPERTY_VALARM_NONE,
	VCAL_COMPONENT_PROPERTY_VALARM_ACTION,
	VCAL_COMPONENT_PROPERTY_VALARM_TRIGGER,
	VCAL_COMPONENT_PROPERTY_VALARM_REPEAT,
	VCAL_COMPONENT_PROPERTY_VALARM_ATTACH,
	VCAL_COMPONENT_PROPERTY_VALARM_DESCRIPTION,
	VCAL_COMPONENT_PROPERTY_VALARM_SUMMARY,
	VCAL_COMPONENT_PROPERTY_VALARM_DURATION,
	VCAL_COMPONENT_PROPERTY_VALARM_END,
	VCAL_COMPONENT_PROPERTY_VALARM_MAX,
};

enum {
	VCAL_ENCODING_NONE,
	VCAL_ENCODING_BASE64,
	VCAL_ENCODING_QUOTED_PRINTABLE,
	VCAL_ENCODING_MAX,
};

enum {
	VCAL_CHARSET_NONE,
	VCAL_CHARSET_UTF_8,
	VCAL_CHARSET_UTF_16,
	VCAL_CHARSET_ISO_8859_1,
	VCAL_CHARSET_UTF_MAX,
};

enum {
	VCAL_COMPONENT_PROPERTY_VTIMEZONE_NONE,
	VCAL_COMPONENT_PROPERTY_VTIMEZONE_DTSTART,
	VCAL_COMPONENT_PROPERTY_VTIMEZONE_TZOFFSETFROM,
	VCAL_COMPONENT_PROPERTY_VTIMEZONE_TZOFFSETTO,
	VCAL_COMPONENT_PROPERTY_VTIMEZONE_TZNAME,
	VCAL_COMPONENT_PROPERTY_VTIMEZONE_RDATE,
	VCAL_COMPONENT_PROPERTY_VTIMEZONE_END,
	VCAL_COMPONENT_PROPERTY_VTIMEZONE_MAX,
};

enum {
	VCAL_VER_10_DALARM_NONE = 0,
	VCAL_VER_10_DALARM_RUN_TIME,
	VCAL_VER_10_DALARM_SNOOZE_TIME,
	VCAL_VER_10_DALARM_REPEAT_COUNT,
	VCAL_VER_10_DALARM_DISPLAY_STRING,
};

enum {
	VCAL_VER_10_MALARM_NONE = 0,
	VCAL_VER_10_MALARM_RUN_TIME,
	VCAL_VER_10_MALARM_SNOOZE_TIME,
	VCAL_VER_10_MALARM_REPEAT_COUNT,
	VCAL_VER_10_MALARM_ADDRESS_STRING,
	VCAL_VER_10_MALARM_NOTE_STRING,
};

enum {
	VCAL_VER_10_AALARM_NONE = 0,
	VCAL_VER_10_AALARM_RUN_TIME,
	VCAL_VER_10_AALARM_SNOOZE_TIME,
	VCAL_VER_10_AALARM_REPEAT_COUNT,
	VCAL_VER_10_AALARM_AUDIO_CONTENT,
};

static const char *vcal_component[VCAL_COMPONENT_MAX] = {0};
static void __init_component(void)
{
	if (NULL == *vcal_component) {
		vcal_component[VCAL_COMPONENT_VEVENT] = "VEVENT";
		vcal_component[VCAL_COMPONENT_VTODO] = "VTODO";
		vcal_component[VCAL_COMPONENT_VJOURNAL] = "VJOURNAL";
		vcal_component[VCAL_COMPONENT_VFREEBUSY] = "VFREEBUSY";
		vcal_component[VCAL_COMPONENT_VTIMEZONE] = "VTIMEZONE";
	}
}

static const char *vcal_property[VCAL_PROPERTY_MAX] = {0};
static void __init_property(void)
{
	if (NULL == *vcal_property) {
		vcal_property[VCAL_PROPERTY_VERSION] = "VERSION";
		vcal_property[VCAL_PROPERTY_TZ] = "TZ";
		vcal_property[VCAL_PROPERTY_BEGIN] = "BEGIN";
		vcal_property[VCAL_PROPERTY_END] = "END";
	}
}

static const char *component_property[VCAL_COMPONENT_PROPERTY_MAX] = {0};
static void __init_component_property(void)
{
	if (NULL == *component_property) {
		component_property[VCAL_COMPONENT_PROPERTY_DTSTAMP] = "DTSTAMP";
		component_property[VCAL_COMPONENT_PROPERTY_UID] = "UID";
		component_property[VCAL_COMPONENT_PROPERTY_RECURRENCE_ID] = "RECURRENCE-ID";
		component_property[VCAL_COMPONENT_PROPERTY_DTSTART] = "DTSTART";
		component_property[VCAL_COMPONENT_PROPERTY_CREATED] = "CREATED";	/* for ver 2: created */
		component_property[VCAL_COMPONENT_PROPERTY_DCREATED] = "DCREATED";	/* for ver 1: created */
		component_property[VCAL_COMPONENT_PROPERTY_DESCRIPTION] = "DESCRIPTION";
		component_property[VCAL_COMPONENT_PROPERTY_LAST_MODIFIED] = "LAST-MODIFIED";
		component_property[VCAL_COMPONENT_PROPERTY_LOCATION] = "LOCATION";
		component_property[VCAL_COMPONENT_PROPERTY_PRIORITY] = "PRIORITY";
		component_property[VCAL_COMPONENT_PROPERTY_STATUS] = "STATUS";
		component_property[VCAL_COMPONENT_PROPERTY_SUMMARY] = "SUMMARY";
		component_property[VCAL_COMPONENT_PROPERTY_RRULE] = "RRULE";
		component_property[VCAL_COMPONENT_PROPERTY_DTEND] = "DTEND";
		component_property[VCAL_COMPONENT_PROPERTY_DUE] = "DUE";
		component_property[VCAL_COMPONENT_PROPERTY_ATTENDEE] = "ATTENDEE";
		component_property[VCAL_COMPONENT_PROPERTY_CATEGORIES] = "CATEGORIES";
		component_property[VCAL_COMPONENT_PROPERTY_DALARM] = "DALARM";
		component_property[VCAL_COMPONENT_PROPERTY_MALARM] = "MALARM";
		component_property[VCAL_COMPONENT_PROPERTY_AALARM] = "AALARM";
		component_property[VCAL_COMPONENT_PROPERTY_EXDATE] = "EXDATE";
		component_property[VCAL_COMPONENT_PROPERTY_X_ALLDAY] = "X-ALLDAY";
		component_property[VCAL_COMPONENT_PROPERTY_X_LUNAR] = "X-LUNAR";
		component_property[VCAL_COMPONENT_PROPERTY_BEGIN] = "BEGIN";	/* start alarm component */
		component_property[VCAL_COMPONENT_PROPERTY_END] = "END";		/* exit record component */
		component_property[VCAL_COMPONENT_PROPERTY_EXTENDED] = "X-";
	}
};

static const char *component_property_valarm[VCAL_COMPONENT_PROPERTY_VALARM_MAX] = {0};
static void __init_component_property_valarm(void)
{
	if (NULL == *component_property_valarm) {
		component_property_valarm[VCAL_COMPONENT_PROPERTY_VALARM_ACTION] = "ACTION";
		component_property_valarm[VCAL_COMPONENT_PROPERTY_VALARM_TRIGGER] = "TRIGGER";
		component_property_valarm[VCAL_COMPONENT_PROPERTY_VALARM_REPEAT] = "REPEAT";
		component_property_valarm[VCAL_COMPONENT_PROPERTY_VALARM_ATTACH] = "ATTACH";
		component_property_valarm[VCAL_COMPONENT_PROPERTY_VALARM_DESCRIPTION] = "DESCRIPTION";
		component_property_valarm[VCAL_COMPONENT_PROPERTY_VALARM_SUMMARY] = "SUMMARY";
		component_property_valarm[VCAL_COMPONENT_PROPERTY_VALARM_DURATION] = "DURATION";
		component_property_valarm[VCAL_COMPONENT_PROPERTY_VALARM_END] = "END";
	}
}

static const char *component_property_vtimezone[VCAL_COMPONENT_PROPERTY_VTIMEZONE_MAX] = {0};
static void __init_component_property_vtimezone(void)
{
	if (NULL == *component_property_vtimezone) {
		component_property_vtimezone[VCAL_COMPONENT_PROPERTY_VTIMEZONE_DTSTART] = "DTSTART";
		component_property_vtimezone[VCAL_COMPONENT_PROPERTY_VTIMEZONE_TZOFFSETFROM] = "TZOFFSETFROM";
		component_property_vtimezone[VCAL_COMPONENT_PROPERTY_VTIMEZONE_TZOFFSETTO] = "TZOFFSETTO";
		component_property_vtimezone[VCAL_COMPONENT_PROPERTY_VTIMEZONE_TZNAME] = "TZNAME";
		component_property_vtimezone[VCAL_COMPONENT_PROPERTY_VTIMEZONE_RDATE] = "RDATE";
		component_property_vtimezone[VCAL_COMPONENT_PROPERTY_VTIMEZONE_END] = "END";
	}
};

static inline void __print_cursor(char *cursor, int line)
{
	int i;
	DBG("(%d)", line);
	for (i = 0; i < 5; i++) {
		if (VCAL_CR == *(cursor + i) || VCAL_LF == *(cursor + i)) break;
		DBG("[%c]", *(cursor + i));
	}
}

static inline void __free_user_data(struct user_data *ud)
{
	if (ud) {
		if (ud->timezone_tzid) free(ud->timezone_tzid);
		if (ud->datetime_tzid) free(ud->datetime_tzid);
		free(ud);
	}
}

static inline char* __remove_empty_line(char *src)
{
	while (*src) {
		if ('\n' != *src && '\r' != *src)
			break;
		src++;
	}
	return src;
}

static inline char* __remove_invalid_space(char *src)
{
	bool start = false;
	while (*src) {
		switch (*src) {
		case ' ':
		case ':':
		case ';':
			src++;
			break;
		default:
			start = true;
			break;
		}
		if (start) break;
	}
	return src;
}

static inline char* __crlf(char *p)
{
	RETV_IF(NULL == p, NULL);
	while (VCAL_LF != *p) {
		if ('\0' == *p) {
			return NULL;
		}
		p++;
	}
	return p +1;
}

static bool __check_has_rrule(char *stream)
{
	RETV_IF(NULL == stream, false);

	bool ret = false;
	char *cursor = stream;
	while (*cursor) {
		if (*(cursor++) == VCAL_LF) {
			if (*(cursor++) == 'R' &&
					*(cursor++) == 'R' &&
					*(cursor++) == 'U' &&
					*(cursor++) == 'L' &&
					*(cursor++) == 'E' &&
					*(cursor++) == ':' ) {
				DBG("[TEST] rrule");
				return true;
			}
			else if (*(cursor-1) == 'E' &&
					*(cursor++) == 'N' &&
					*(cursor++) == 'D' &&
					*(cursor++) == ':' &&
					*(cursor++) == 'V' &&
					*(cursor++) == 'E' &&
					*(cursor++) == 'V' &&
					*(cursor++) == 'E' &&
					*(cursor++) == 'N' &&
					*(cursor++) == 'T') {
				DBG("[TEST] end");
				break;
			}
			else {
			}
		}
	}
	return ret;
}

static int _sub_caltime(struct user_data *ud, calendar_time_s *s, calendar_time_s *a, int *diff)
{
	RETV_IF(NULL == s, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == a, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == diff, CALENDAR_ERROR_INVALID_PARAMETER);

	if (s->type != a->type) {
		WARN("This is strange. start type(%d) alarm type(%d)", s->type, a->type);
		char *tz = NULL;
		if (ud->timezone_tzid)
			tz = ud->timezone_tzid;
		else if (ud->datetime_tzid)
			tz = ud->datetime_tzid;
		else
			tz = NULL;

		long long int lli = 0;
		if (CALENDAR_TIME_LOCALTIME == s->type) {
			lli = cal_time_convert_itol(tz, s->time.date.year, s->time.date.month,
					s->time.date.mday, s->time.date.hour, s->time.date.minute,
					s->time.date.second);
			*diff = lli - a->time.utime;
			DBG("Convert start localtime with tz[%s] and get(%lld)", tz, lli);
		}
		else {
			lli = cal_time_convert_itol(tz, a->time.date.year, a->time.date.month,
					a->time.date.mday, a->time.date.hour, a->time.date.minute,
					a->time.date.second);
			*diff = s->time.utime - lli;
			DBG("Convert alarm localtime with tz[%s] and get(%lld)", tz, lli);
		}
		return CALENDAR_ERROR_NONE;
	}
	switch (s->type) {
	case CALENDAR_TIME_UTIME:
		*diff = s->time.utime - a->time.utime;
		break;
	case CALENDAR_TIME_LOCALTIME:
		*diff = cal_time_convert_itol(NULL, s->time.date.year, s->time.date.month,
				s->time.date.mday, s->time.date.hour, s->time.date.minute, s->time.date.second) -
			cal_time_convert_itol(NULL, a->time.date.year, a->time.date.month,
					a->time.date.mday, a->time.date.hour, a->time.date.minute, a->time.date.second);
		break;
	}
	return CALENDAR_ERROR_NONE;
}

static void _get_tick_unit(int t, int *tick, int *unit)
{
	if (0 == t) {
		return;
	}

	if (0 == (t % CALENDAR_ALARM_TIME_UNIT_WEEK)) {
		*tick = t / CALENDAR_ALARM_TIME_UNIT_WEEK;
		*unit = CALENDAR_ALARM_TIME_UNIT_WEEK;
	}
	else if (0 == (t % CALENDAR_ALARM_TIME_UNIT_DAY)) {
		*tick = t / CALENDAR_ALARM_TIME_UNIT_DAY;
		*unit = CALENDAR_ALARM_TIME_UNIT_DAY;
	}
	else if (0 == (t % CALENDAR_ALARM_TIME_UNIT_HOUR)) {
		*tick = t / CALENDAR_ALARM_TIME_UNIT_HOUR;
		*unit = CALENDAR_ALARM_TIME_UNIT_HOUR;
	}
	else if (0 == (t % CALENDAR_ALARM_TIME_UNIT_MINUTE)) {
		*tick = t / CALENDAR_ALARM_TIME_UNIT_MINUTE;
		*unit = CALENDAR_ALARM_TIME_UNIT_MINUTE;
	}
	else {
		*tick = t;
		*unit = CALENDAR_ALARM_TIME_UNIT_SPECIFIC;
	}
	DBG("tick(%d), unit(%d)", *tick, *unit);
}

static char* __get_value(char *cursor, char **value)
{
	RETV_IF(NULL == cursor, NULL);
	RETV_IF(NULL == value, NULL);

	/* offset: length until ';' or ':' */
	int offset = 0;
	while (':' != *(cursor + offset) && ';' != *(cursor + offset)) {
		offset++;
	}

	int i = 0;
	while (VCAL_LF != *(cursor + offset + i)) {
		if ('\0' == *(cursor + offset + i)) {
			return NULL;
		}
		i++;
	}

	char *p = calloc(i + 1, sizeof(char));
	RETVM_IF(NULL == p, NULL, "calloc() Fail");

	if (VCAL_CR == *(cursor + offset + i -1)) {
		memcpy(p, cursor + offset, i -1);

	}
	else {
		memcpy(p, cursor + offset, i);
	}
	*value = p;
	DBG("offset(%d) len(%d) value[%s]", offset, i, *value);

	return cursor + offset + i +1;
}

static char* __check_word(char *src, const char *word)
{
	RETV_IF(NULL == src, NULL);

	src = __remove_empty_line(src);
	src = __remove_invalid_space(src);

	while (*src == *word) {
		src++;
		word++;

		if ('\0' == *src || '\0' == *word)
			break;
	}

	if ('\0' == *word)
		return src;
	else
		return NULL;
}

/*
 * Change '-' to '/' as icu format to be recognized in icu library.
 * ig. US-Easten -> US/Eastern
 */
static inline void __adjust_tzid(char *p)
{
	RET_IF(NULL == p);

	DBG("Before [%s]", p);
	int i = 0;
	while (*(p +i)) {
		if ('-' == *(p +i)) {
			if ('1' <= *(p +i +1) && *(p +i +1) <= '9') {
				i++;
			}
			else {
				*(p +i) = '/';
			}
		}
		i++;
	}
	DBG("After [%s]", p);
}

static void __unfolding(char *p)
{
	RET_IF(NULL == p);
	RET_IF('\0' == *p);

	char *q = p;
	while ('\0' != *p) {
		switch (*p) {
		case '=':
			if (VCAL_LF == *(p +1) && ' ' == *(p +2)) /* ver1.0:out of spec, but allowed exceptional case */
				p += 3;
			else if (VCAL_CR == *(p +1) && VCAL_LF == *(p +2) && ' ' == *(p +3)) /* ver1.0:in spec case */
				p += 4;
			else if (VCAL_CR == *(p +1) && VCAL_LF == *(p +2)) /* galaxy wants this format */
				p += 3;
			else ;
			break;

		case VCAL_LF:
			if (' ' == *(p + 1)) /* ver2.0:out of spec, but allowed exceptional case */
				p += 2;
			else if ('\t' == *(p + 1)) /* ver2.0:out of spec, but allowed exceptional case */
				p += 2;
			else ;
			break;

		case VCAL_CR:
			if ('\n' == *(p + 1) && ' ' == *(p + 2)) /* ver2.0:in spec case */
				p += 3;
			else if ('\n' == *(p + 1) && '\t' == *(p + 2)) /* ver2.0:out of spec, but allowed exceptional case */
				p += 3;
			else ;
			break;
		}

		*q = *p;
		p++;
		q++;
	}
}

static void __decode_escaped_char(char *p)
{
	RET_IF(NULL == p);
	RET_IF('\0' == *p);

	char *q = p;
	while ('\0' != *p) {
		if ('\\' == *p && *(p +1)) {
			switch (*(p +1)) {
			case '\\':
				*q = '\\';
				p++;
				break;
			case 'n':
			case 'N':
				*q = '\n';
				p++;
				break;
			case ';':
				*q = ';';
				p++;
				break;
			case ',':
				*q = ',';
				p++;
				break;
			}
		}
		else {
			*q = *p;
		}
		q++;
		p++;
	}
	*q = '\0';
}

static void __decode_base64(char *p)
{
	RET_IF(NULL == p);
	RET_IF('\0' == *p);

	DBG("Before [%s]", p);
	guchar *buf = NULL;
	gsize size = 0;
	buf = g_base64_decode(p, &size);
	if (0 == size) {
		free(buf);
		return;
	}
	if (strlen(p) < size) {
		ERR("out of size");
		return;
	}

	snprintf(p, size + 1, "%s%c", buf, '\0');
	free(buf);
	DBG("After [%s]", p);
}

static char __decode_hexa(char *p)
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

static void __decode_quoted_printable(char *p)
{
	RET_IF(NULL == p);
	RET_IF('\0' == *p);

	int i = 0, j = 0;
	char ch;

	DBG("Before[%s]", p);
	while (p[i]) {
		if (p[i] == '=') {
			if (p[i+1] == 0x09 || p[i+1] == 0x20) {
				i +=3;

			}
			else if (p[i+1] == '\r' || p[i+1] == '\n') {
				i +=3;

			}
			else {
				if (p[i+1] == '0' && tolower(p[i+2]) == 'd' &&
						p[i+3] == '=' && p[i+4] == '0' && tolower(p[i+5]) == 'a') {
					p[j] = '\n';
					j++;
					i += 6;
				}
				else {
					ch = __decode_hexa(p +i +1);
					p[j] = ch;
					j++;
					i += 3;
				}
			}
		}
		else {
			p[j] = p[i];
			j++;
			i++;
		}
	}
	p[j] = '\0';
	DBG("After[%s]", p);
}

static char* __decode_iso8859_1_to_utf8(char *p)
{
	RETV_IF(NULL == p, NULL);

	char *src = NULL;
	char *dst = NULL;

	DBG("Before [%s]", p);
	int len = strlen(p);
	len *= 2;

	char *out_p = NULL;
	out_p = realloc(p, len);
	if (NULL == out_p) {
		ERR("realloc() Fail");
		return NULL;
	}

	/* check enough space */
	for (src = dst = out_p; *src; src++, dst++) {
		if (*src & 0x80) {
			++dst;
		}
	}

	while (dst > src) {
		if (*src & 0x80) {
			*dst-- = 0x80 | (*src & 0x3f);
			*dst-- = 0xc0 | (*((unsigned char *)src--) >> 6);
		}
		else {
			*dst-- = *src--;
		}
	}
	DBG("After [%s]", out_p);
	return out_p;
}

static char* __decode_charset(char *p)
{
	char **t = NULL;
	t =  g_strsplit(p, ":", 2);
	RETVM_IF(NULL == t, NULL, "g_strsplit() Fail");

	if ('\0' == *t[0]) { /* no param */
		g_strfreev(t);
		return cal_strdup(p + 1);
	}

	char **s = NULL;
	s = g_strsplit(t[0], ";", -1);
	if (NULL == s) {
		ERR("g_strsplit() Fail");
		g_strfreev(t);
		return NULL;
	}
	int count_param = g_strv_length(s);
	DBG("count_param(%d)", count_param);

	int charset = VCAL_CHARSET_NONE;
	int encoding = VCAL_ENCODING_NONE;

	int i;
	for (i = 0; i < count_param; i++) {
		if (NULL == s[i] || '\0' == *s[i])
			continue;

		if (CAL_STRING_EQUAL == strncmp(s[i], "CHARSET=", strlen("CHARSET="))) {
			int key_len = strlen("CHARSET=");
			if (CAL_STRING_EQUAL == strcmp(s[i] + key_len, "UTF-8"))
				charset = VCAL_CHARSET_UTF_8;
			else if (CAL_STRING_EQUAL == strcmp(s[i] + key_len, "ISO-8859-1"))
				charset = VCAL_CHARSET_ISO_8859_1;
			else
				WARN("Not support charset[%s]", s[i] + key_len);
		}
		else if (CAL_STRING_EQUAL == strncmp(s[i], "ENCODING=", strlen("ENCODING="))) {
			int key_len = strlen("ENCODING=");
			if (CAL_STRING_EQUAL == strcmp(s[i] + key_len, "BASE64"))
				encoding = VCAL_ENCODING_BASE64;
			else if (CAL_STRING_EQUAL == strcmp(s[i] + key_len, "QUOTED-PRINTABLE"))
				encoding = VCAL_ENCODING_QUOTED_PRINTABLE;
			else
				WARN("Not support encoding[%s]", s[i] + key_len);
		}
		else {
			WARN("[%s]", s[i]);
		}
	}
	g_strfreev(s);
	/* param end */

	char *ret_str = strdup(t[1]);
	g_strfreev(t);

	switch (encoding) {
	case VCAL_ENCODING_BASE64:
		__decode_base64(ret_str);
		break;
	case VCAL_ENCODING_QUOTED_PRINTABLE:
		__decode_quoted_printable(ret_str);
		break;
	}

	switch (charset) {
	case VCAL_CHARSET_UTF_8:
		break;
	case VCAL_CHARSET_ISO_8859_1:
		ret_str = __decode_iso8859_1_to_utf8(ret_str);
		break;
	}

	return ret_str;
}

static char* __decode_datetime(char *p, struct user_data *ud)
{
	char **t = NULL;
	t =  g_strsplit(p, ":", -1);
	RETVM_IF(NULL == t, NULL, "g_strsplit() Fail");

	if ('\0' == *t[0]) { /* no param */
		g_strfreev(t);
		return p + 1;
	}
	int count = g_strv_length(t);
	int len_param = strlen(t[count -1]);
	*(p + strlen(p) - len_param -1) = '\0';
	g_strfreev(t);

	/* param start */
	char **s = NULL;
	s = g_strsplit(p, ";", -1);
	RETVM_IF(NULL == s, p + strlen(p) - len_param, "g_strsplit() Fail");

	int count_param = g_strv_length(s);
	DBG("count_param(%d)", count_param);
	int i;
	for (i = 0; i < count_param; i++) {
		if (NULL == s[i] || '\0' == *s[i])
			continue;

		if (CAL_STRING_EQUAL != strncmp(s[i], "TZID=", strlen("TZID=")))
			continue;

		if (ud->datetime_tzid)
			break;

		char *tzid = NULL;
		tzid = strdup(s[i] + strlen("TZID="));
		if (NULL == tzid) {
			ERR("strdup() Fail");
			break;
		}
		__adjust_tzid(tzid);
		DBG("modified tzid[%s]", tzid);

		if (true == cal_time_is_available_tzid(tzid)) {
			DBG("VALID tzid");
			ud->datetime_tzid = tzid;
			break;
		}
		free(tzid);

		if (ud->timezone_tzid && *ud->timezone_tzid) {
			ud->datetime_tzid = strdup(ud->timezone_tzid);
			if (NULL == ud->datetime_tzid) {
				ERR("strdup() Fail");
				break;
			}
			DBG("set datetime_tzid[%s] as timezone_tzid", ud->datetime_tzid);
		}
		else {
			ERR("INVALID tzid");
		}
		break;
	}
	g_strfreev(s);
	/* param end */

	DBG("(%d) (%d) [%s]", strlen(p), len_param, p + strlen(p) +1);
	return p + strlen(p) +1;
}

static void __decode_duration(char *cursor, int len, int *tick, int *unit)
{
	RET_IF(NULL == cursor);
	RET_IF('\0' == *cursor);
	RET_IF(NULL == tick);
	RET_IF(NULL == unit);

	char buf[8] = {0};
	int sign = 1;
	int digit = 0;
	int t = 0, u = 0;
	int i;
	for (i = 0; i < len; i++) {
		switch (*(cursor + i)) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			digit++;
			break;
		case '+':
			break;
		case '-':
			sign = -1;
			break;
		case 'P':
			break;
		case 'T':
			break;
		case 'W':
			u = CALENDAR_ALARM_TIME_UNIT_WEEK;
			snprintf(buf, digit + 1, "%s", cursor + i - digit);
			t = atoi(buf) * u;
			DBG("[%s] (%d)", buf, t);
			digit = 0;
			break;
		case 'D':
			u = CALENDAR_ALARM_TIME_UNIT_DAY;
			snprintf(buf, digit + 1, "%s", cursor + i - digit);
			t += atoi(buf) * u;
			DBG("[%s] (%d)", buf, t);
			digit = 0;
			break;
		case 'H':
			u = CALENDAR_ALARM_TIME_UNIT_HOUR;
			snprintf(buf, digit + 1, "%s", cursor + i - digit);
			t += atoi(buf) * u;
			DBG("[%s] (%d)", buf, t);
			digit = 0;
			break;
		case 'M':
			u = CALENDAR_ALARM_TIME_UNIT_MINUTE;
			snprintf(buf, digit + 1, "%s", cursor + i - digit);
			t += atoi(buf) * u;
			DBG("[%s] (%d)", buf, t);
			digit = 0;
			break;
		case 'S':
			u = CALENDAR_ALARM_TIME_UNIT_SPECIFIC;
			snprintf(buf, digit + 1, "%s", cursor + i - digit);
			t += atoi(buf) * u;
			DBG("[%s] (%d)", buf, t);
			digit = 0;
			break;
		default:
			ERR("Invalid value");
			break;
		}
	}

	t *= sign;
	_get_tick_unit(t, tick, unit);
}

static bool __is_digit(char *p)
{
	while (*p) {
		if ((*p < '0' || '9' < *p) && '+' != *p && '-' != *p)
			return false;
		p++;
	}
	return true;
}

static char* __get_index(char *cursor, const char **array, int len, int *index)
{
	RETV_IF(NULL == index, NULL);

	int i;
	char *new = NULL;
	for (i = 1; i < len; i++) {
		new = __check_word(cursor, array[i]);
		if (new) break;
	}
	if (len == i) {
		*index = 0;
		return cursor;
	}

	*index = i;
	DBG("index(%d) [%s]", i, array[i]);
	return cursor + strlen(array[i]);
}

static void __get_version(char *value, int *version)
{
	CAL_FN_CALL();

	RET_IF(NULL == value);
	RET_IF(NULL == version);

	if (CAL_STRING_EQUAL == strncmp(value, ":1.0", strlen(":1.0")))
		*version = 1;
	else
		*version = 2;

	DBG("version(%d)", *version);
}

static int _get_caltime(char *p, calendar_time_s *caltime, struct user_data *ud)
{
	RETV_IF(NULL == p, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF('\0' == *p, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == caltime, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == ud, CALENDAR_ERROR_INVALID_PARAMETER);

	int ret = CALENDAR_ERROR_NONE;

	switch (strlen(p)) {
	case VCAL_DATETIME_LENGTH_YYYYMMDD:
		caltime->type = CALENDAR_TIME_LOCALTIME;
		sscanf(p, CAL_DATETIME_FORMAT_YYYYMMDD,
				&(caltime->time.date.year), &(caltime->time.date.month), &(caltime->time.date.mday));
		caltime->time.date.hour = 0;
		caltime->time.date.minute = 0;
		caltime->time.date.second = 0;
		DBG(CAL_DATETIME_FORMAT_YYYYMMDDTHHMMSS,
				caltime->time.date.year, caltime->time.date.month, caltime->time.date.mday,
				caltime->time.date.hour, caltime->time.date.minute, caltime->time.date.second);
		break;
	case VCAL_DATETIME_LENGTH_YYYYMMDDTHHMMSS:
		sscanf(p, CAL_DATETIME_FORMAT_YYYYMMDDTHHMMSS,
				&(caltime->time.date.year), &(caltime->time.date.month), &(caltime->time.date.mday),
				&(caltime->time.date.hour), &(caltime->time.date.minute), &(caltime->time.date.second));

		if (VCAL_VER_1 == ud->version) {
			caltime->type = CALENDAR_TIME_LOCALTIME;
		}
		else {
			if (NULL == ud->datetime_tzid || '\0' == *ud->datetime_tzid) {
				if (NULL == ud->timezone_tzid || '\0' == *ud->timezone_tzid) {
					/* Without tzid is localtime */
					caltime->type = CALENDAR_TIME_LOCALTIME;
					DBG(CAL_DATETIME_FORMAT_YYYYMMDDTHHMMSS,
							caltime->time.date.year, caltime->time.date.month, caltime->time.date.mday,
							caltime->time.date.hour, caltime->time.date.minute, caltime->time.date.second);
				}
				else {
					/* No 'Z' with tzid means utime */
					caltime->type = CALENDAR_TIME_UTIME;
					caltime->time.utime = cal_time_convert_itol(ud->timezone_tzid,
							caltime->time.date.year, caltime->time.date.month, caltime->time.date.mday,
							caltime->time.date.hour, caltime->time.date.minute, caltime->time.date.second);
					DBG("timezone_tzid[%s] (%lld)", ud->timezone_tzid, caltime->time.utime);
				}
			}
			else {
				/* No 'Z' with tzid means utime */
				caltime->type = CALENDAR_TIME_UTIME;
				caltime->time.utime = cal_time_convert_itol(ud->datetime_tzid,
						caltime->time.date.year, caltime->time.date.month, caltime->time.date.mday,
						caltime->time.date.hour, caltime->time.date.minute, caltime->time.date.second);
				DBG("datetime_tzid[%s] (%lld)", ud->datetime_tzid, caltime->time.utime);
			}
		}
		break;
	case VCAL_DATETIME_LENGTH_YYYYMMDDTHHMMSSZ:
		if (ud->is_allday) {
			caltime->type = CALENDAR_TIME_LOCALTIME;
			sscanf(p, CAL_DATETIME_FORMAT_YYYYMMDDTHHMMSSZ,
					&(caltime->time.date.year), &(caltime->time.date.month), &(caltime->time.date.mday),
					&(caltime->time.date.hour), &(caltime->time.date.minute), &(caltime->time.date.second));
			caltime->time.date.hour = 0;
			caltime->time.date.minute = 0;
			caltime->time.date.second = 0;
			DBG(CAL_DATETIME_FORMAT_YYYYMMDDTHHMMSS,
					caltime->time.date.year, caltime->time.date.month, caltime->time.date.mday,
					caltime->time.date.hour, caltime->time.date.minute, caltime->time.date.second);
		}
		else {
			caltime->type = CALENDAR_TIME_UTIME;
			caltime->time.utime = cal_time_convert_lli(p);
			DBG("(%lld)", caltime->time.utime);
		}
		break;
	default:
		ERR("Invalid time format[%s]", p);
		ret = CALENDAR_ERROR_INVALID_PARAMETER;
		break;
	}
	return ret;
}

/*
 * TZ:+05
 * TZ:
 * TZ:+5
 * TZ:5:30
 * TZ:+05:30
 */
static void __parse_tz(const char *tz, int *h, int *m)
{
	RET_IF(NULL == tz);

	char **t = g_strsplit(tz, ":", -1);
	RETM_IF(NULL == t, "g_strsplit() is NULL");

	int sign = 0;
	if ('-' == *t[0])
		sign = -1;
	else if ('+' == *t[0])
		sign = 1;
	else
		sign = 0;

	if (0 == strlen(t[0])) {
		ERR("No hour");
		g_strfreev(t);
		return;
	}

	char buf[8] = {0};
	if (sign) {
		snprintf(buf, strlen(t[0]), "%s", t[0] + 1);
	}
	else {
		sign = 1;
		snprintf(buf, strlen(t[0]) + 1, "%s", t[0]);
	}
	if (h) *h = sign * atoi(buf);

	if (1 == g_strv_length(t)) {
		if (m) *m = 0;
		g_strfreev(t);
		return;
	}

	snprintf(buf, strlen(t[1]) + 1, "%s", t[1]);
	if (m) *m = atoi(buf);

	g_strfreev(t);
}

static void __get_tz(char *value, char **tz)
{
	RET_IF(NULL == value);
	RET_IF(NULL == tz);

	int h = 0, m = 0;
	__parse_tz(value +1, &h, &m); /* +1 to skip ':' */

	char buf[CAL_STR_SHORT_LEN32] = {0};
	if (0 == m) {
		snprintf(buf, sizeof(buf), "Etc/GMT%c%d", h < 0 ? '+' : '-', h);

	}
	else {
		cal_time_get_registered_tzid_with_offset(h * 3600 + m * 60, buf, sizeof(buf));
	}
	DBG("set tzid [%s]", buf);

	*tz = strdup(buf);
}

static void __work_component_property_dtstamp(char *value, calendar_record_h record, struct user_data *ud)
{
	return;
}

static void __work_component_property_uid(char *value, calendar_record_h record, struct user_data *ud)
{
	RET_IF(NULL == value);
	RET_IF('\0' == *value);
	RET_IF(NULL == record);
	RET_IF(NULL == ud);

	int ret = 0;
	char *s = __decode_charset(value);
	__decode_escaped_char(s);
	switch (ud->type) {
	case CALENDAR_BOOK_TYPE_EVENT:
		ret = cal_record_set_str(record, _calendar_event.uid, s);
		WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_str() Fail(%d)", ret);
		break;
	case CALENDAR_BOOK_TYPE_TODO:
		ret = cal_record_set_str(record, _calendar_todo.uid, s);
		WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_str() Fail(%d)", ret);
		break;
	}
	free(s);
}

static void __work_component_property_recurrence_id(char *value, calendar_record_h record, struct user_data *ud)
{
	RET_IF(NULL == value);
	RET_IF('\0' == *value);
	RET_IF(NULL == record);
	RET_IF(NULL == ud);

	int ret = 0;
	switch (ud->type) {
	case CALENDAR_BOOK_TYPE_EVENT:
		ret = cal_record_set_str(record, _calendar_event.recurrence_id, value +1);
		WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_str() Fail(%d)", ret);
		break;
	case CALENDAR_BOOK_TYPE_TODO:
		DBG("Not supported in todo");
		break;
	}
}

static void __work_component_property_dtstart(char *value, calendar_record_h record, struct user_data *ud)
{
	RET_IF(NULL == value);
	RET_IF('\0' == *value);
	RET_IF(NULL == record);
	RET_IF(NULL == ud);

	int ret = 0;

	value = __decode_datetime(value, ud);
	calendar_time_s dtstart = {0};
	ret = _get_caltime(value, &dtstart, ud);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("_get_caltime() Fail(%d)", ret);
		return;
	}

	char *tzid = NULL;
	tzid = ud->datetime_tzid ? ud->datetime_tzid : (ud->timezone_tzid ? ud->timezone_tzid : NULL);

	switch (ud->type) {
	case CALENDAR_BOOK_TYPE_EVENT:
		if (tzid && *tzid) {
			ret = cal_record_set_str(record, _calendar_event.start_tzid, tzid);
			WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_str() Fail(%d)", ret);
		}
		ret = cal_record_set_caltime(record, _calendar_event.start_time, dtstart);
		WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_caltime() Fail(%d)", ret);
		break;
	case CALENDAR_BOOK_TYPE_TODO:
		if (tzid && *tzid) {
			ret = cal_record_set_str(record, _calendar_todo.start_tzid, tzid);
			WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_str() Fail(%d)", ret);
		}
		ret = cal_record_set_caltime(record, _calendar_todo.start_time, dtstart);
		WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_caltime() Fail(%d)", ret);
		break;
	}

	/* check if dtend is earlier than dtstart. */
	if (CALENDAR_BOOK_TYPE_TODO == ud->type) /* skip in todo */
		return;

	calendar_time_s dtend = {0};
	ret = calendar_record_get_caltime(record, _calendar_event.end_time, &dtend);
	WARN_IF(CALENDAR_ERROR_NONE != ret, "calendar_record_get_caltime() Fail(%d)", ret);

	if (0 == dtend.time.utime) /* not set yet */
		return;

	int diff = 0;
	_sub_caltime(ud, &dtstart, &dtend, &diff);
	if (diff <= 0) /* proper data */
		return;

	WARN("dtend < dtstart so set end time to start");
	dtend = dtstart;
}

static void __work_component_property_created(char *value, calendar_record_h record, struct user_data *ud)
{
	RET_IF(NULL == value);
	RET_IF('\0' == *value);
	RET_IF(NULL == record);
	RET_IF(NULL == ud);

	int ret = 0;
	switch (ud->type) {
	case CALENDAR_BOOK_TYPE_EVENT:
		ret = cal_record_set_lli(record, _calendar_event.created_time, cal_time_convert_lli(value));
		WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_lli() Fail(%d)", ret);
		break;

	case CALENDAR_BOOK_TYPE_TODO:
		ret = cal_record_set_lli(record, _calendar_todo.created_time, cal_time_convert_lli(value));
		WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_lli() Fail(%d)", ret);
		break;
	}
}

static void __work_component_property_description(char *value, calendar_record_h record, struct user_data *ud)
{
	RET_IF(NULL == value);
	RET_IF('\0' == *value);
	RET_IF(NULL == record);
	RET_IF(NULL == ud);

	int ret = 0;
	char *s = __decode_charset(value);
	__decode_escaped_char(s);
	switch (ud->type) {
	case CALENDAR_BOOK_TYPE_EVENT:
		ret = cal_record_set_str(record, _calendar_event.description, s);
		WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_str() Fail(%d)", ret);
		break;
	case CALENDAR_BOOK_TYPE_TODO:
		ret = cal_record_set_str(record, _calendar_todo.description, s);
		WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_str() Fail(%d)", ret);
		break;
	}
	free(s);
}

static void __work_component_property_last_modified(char *value, calendar_record_h record, struct user_data *ud)
{
	RET_IF(NULL == value);
	RET_IF('\0' == *value);
	RET_IF(NULL == record);
	RET_IF(NULL == ud);

	int ret = 0;
	switch (ud->type) {
	case CALENDAR_BOOK_TYPE_EVENT:
		ret = cal_record_set_lli(record, _calendar_event.last_modified_time, cal_time_convert_lli(value));
		WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_lli() Fail(%d)", ret);
		break;
	case CALENDAR_BOOK_TYPE_TODO:
		ret = cal_record_set_lli(record, _calendar_todo.last_modified_time, cal_time_convert_lli(value));
		WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_lli() Fail(%d)", ret);
		break;
	}
}

static void __work_component_property_location(char *value, calendar_record_h record, struct user_data *ud)
{
	RET_IF(NULL == value);
	RET_IF('\0' == *value);
	RET_IF(NULL == record);
	RET_IF(NULL == ud);

	int ret = 0;
	char *s = __decode_charset(value);
	__decode_escaped_char(s);
	switch (ud->type) {
	case CALENDAR_BOOK_TYPE_EVENT:
		ret = cal_record_set_str(record, _calendar_event.location, s);
		WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_str() Fail(%d)", ret);
		break;
	case CALENDAR_BOOK_TYPE_TODO:
		ret = cal_record_set_str(record, _calendar_todo.location, s);
		WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_str() Fail(%d)", ret);
		break;
	}
	free(s);
}

static int __decode_priority(char *value, struct user_data *ud)
{
	int original_priority = atoi(value);
	int modified_priority = 0;

	switch (ud->version) {
	case VCAL_VER_1:
		switch (original_priority) {
		case 0:
			modified_priority = CALENDAR_TODO_PRIORITY_LOW;
			break;
		case 1:
			modified_priority = CALENDAR_TODO_PRIORITY_NORMAL;
			break;
		case 2:
			modified_priority = CALENDAR_TODO_PRIORITY_HIGH;
			break;
		default:
			DBG("Unable to parse [%s]", value);
			modified_priority = CALENDAR_TODO_PRIORITY_NONE;
			break;
		}
		break;

	case VCAL_VER_2:
	default:
		switch (original_priority) {
		case 1 ... 4:
			modified_priority = CALENDAR_TODO_PRIORITY_HIGH;
			break;
		case 5:
			modified_priority = CALENDAR_TODO_PRIORITY_NORMAL;
			break;
		case 6 ... 9:
			modified_priority = CALENDAR_TODO_PRIORITY_LOW;
			break;
		default:
			DBG("Unable to parse [%s]", value);
			modified_priority = CALENDAR_TODO_PRIORITY_NONE;
			break;
		}
		break;
	}
	DBG("convert priority(%d) -> (%d)", original_priority, modified_priority);
	return modified_priority;
}

static void __work_component_property_priority(char *value, calendar_record_h record, struct user_data *ud)
{
	RET_IF(NULL == value);
	RET_IF('\0' == *value);
	RET_IF(NULL == record);
	RET_IF(NULL == ud);
	RETM_IF(*value < '0' || '9' < *value, "out of range[%s]", value);

	int ret = 0;
	int modified_priority = __decode_priority(value, ud);
	switch (ud->type) {
	case CALENDAR_BOOK_TYPE_EVENT:
		ret = cal_record_set_int(record, _calendar_event.priority, modified_priority);
		WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_int() Fail(%d)", ret);
		break;
	case CALENDAR_BOOK_TYPE_TODO:
		ret = cal_record_set_int(record, _calendar_todo.priority, modified_priority);
		WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_int() Fail(%d)", ret);
		break;
	}
}

static void __work_component_property_status(char *value, calendar_record_h record, struct user_data *ud)
{
	RET_IF(NULL == value);
	RET_IF('\0' == *value);
	RET_IF(NULL == record);
	RET_IF(NULL == ud);

	int ret = 0;
	int status = 0;
	switch (ud->type) {
	case CALENDAR_BOOK_TYPE_EVENT:
		if (CAL_STRING_EQUAL == strncmp(value, ":TENTATIVE", strlen(":TENTATIVE"))) {
			status = CALENDAR_EVENT_STATUS_TENTATIVE;
		}
		else if (CAL_STRING_EQUAL == strncmp(value, ":CONFIRMED", strlen(":CONFIRMED"))) {
			status = CALENDAR_EVENT_STATUS_CONFIRMED;
		}
		else if (CAL_STRING_EQUAL == strncmp(value, ":CANCELLED", strlen(":CANCELLED"))) {
			status = CALENDAR_EVENT_STATUS_CANCELLED;
		}
		else {
			status = CALENDAR_EVENT_STATUS_NONE;
		}
		ret = cal_record_set_int(record, _calendar_event.event_status, status);
		WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_int() Fail(%d)", ret);
		break;
	case CALENDAR_BOOK_TYPE_TODO:
		if (CAL_STRING_EQUAL == strncmp(value, ":NEEDS-ACTION", strlen(":NEEDS-ACTION")))
			status = CALENDAR_TODO_STATUS_NEEDS_ACTION;
		else if (CAL_STRING_EQUAL == strncmp(value, ":NEEDS ACTION", strlen(":NEEDS ACTION")))
			status = CALENDAR_TODO_STATUS_NEEDS_ACTION;
		else if (CAL_STRING_EQUAL == strncmp(value, ":COMPLETED", strlen(":COMPLETED")))
			status = CALENDAR_TODO_STATUS_COMPLETED;
		else if (CAL_STRING_EQUAL == strncmp(value, ":IN-PROCESS", strlen(":IN-PROCESS")))
			status = CALENDAR_TODO_STATUS_IN_PROCESS;
		else if (CAL_STRING_EQUAL == strncmp(value, ":CANCELLED", strlen(":CANCELLED")))
			status = CALENDAR_TODO_STATUS_CANCELED;
		else
			status = CALENDAR_TODO_STATUS_NONE;

		ret = cal_record_set_int(record, _calendar_todo.todo_status, status);
		WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_int() Fail(%d)", ret);
		break;
	}
}

static void __work_component_property_summary(char *value, calendar_record_h record, struct user_data *ud)
{
	RET_IF(NULL == value);
	RET_IF('\0' == *value);
	RET_IF(NULL == record);
	RET_IF(NULL == ud);

	int ret = 0;
	char *s = __decode_charset(value);
	__decode_escaped_char(s);
	switch (ud->type) {
	case CALENDAR_BOOK_TYPE_EVENT:
		ret = cal_record_set_str(record, _calendar_event.summary, s);
		WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_str() Fail(%d)", ret);
		break;
	case CALENDAR_BOOK_TYPE_TODO:
		ret = cal_record_set_str(record, _calendar_todo.summary, s);
		WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_str() Fail(%d)", ret);
		break;
	}
	free(s);
}

static bool __is_wday_string(char *p)
{
	RETV_IF(NULL == p, false);
	RETV_IF('0' == *p, false);

	if ('S' == *p && 'U' == *(p +1))
		return true;
	else if ('M' == *p && 'O' == *(p +1))
		return true;
	else if ('T' == *p && 'U' == *(p +1))
		return true;
	else if ('W' == *p && 'E' == *(p +1))
		return true;
	else if ('T' == *p && 'H' == *(p +1))
		return true;
	else if ('F' == *p && 'R' == *(p +1))
		return true;
	else if ('S' == *p && 'A' == *(p +1))
		return true;
	else
		return false;

}

static int __get_frequency(char *p)
{
	if ('Y' == *p && 'M' == *(p +1))
		return VCAL_RECURRENCE_YEARLY_BYMONTH;
	else if ('Y' == *p && 'D' == *(p +1))
		return VCAL_RECURRENCE_YEARLY_BYYEARDAY;
	else if ('M' == *p && 'P' == *(p +1))
		return VCAL_RECURRENCE_MONTHLY_BYDAY;
	else if ('M' == *p && 'D' == *(p +1))
		return VCAL_RECURRENCE_MONTHLY_BYMONTHDAY;
	else if ('W' == *p && 'E' != *(p +1))  /* check 'E' for WE(Wednesday) */
		return VCAL_RECURRENCE_WEEKLY;
	else if ('D' == *p)
		return VCAL_RECURRENCE_DAILY;
	else
		return VCAL_RECURRENCE_NONE;
}

static void __set_bystr(int freq_mode, calendar_record_h record, char *bystr)
{
	RET_IF(NULL == record);
	RET_IF(NULL == bystr);
	RET_IF('\0' == *bystr);

	DBG("bystr[%s]", bystr);
	bystr[strlen(bystr) -1] = '\0'; /* to remove ',' */
	int ret = 0;
	switch (freq_mode) {
	case VCAL_RECURRENCE_YEARLY_BYMONTH:
		ret = cal_record_set_str(record, _calendar_event.bymonth, bystr);
		WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_str() Fail(%d)", ret);
		break;
	case VCAL_RECURRENCE_YEARLY_BYYEARDAY:
		ret = cal_record_set_str(record, _calendar_event.byyearday, bystr);
		WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_str() Fail(%d)", ret);
		break;
	case VCAL_RECURRENCE_MONTHLY_BYMONTHDAY:
		ret = cal_record_set_str(record, _calendar_event.bymonthday, bystr);
		WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_str() Fail(%d)", ret);
		break;
	case VCAL_RECURRENCE_MONTHLY_BYDAY:
		ret = cal_record_set_str(record, _calendar_event.byday, bystr);
		WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_str() Fail(%d)", ret);
		break;
	case VCAL_RECURRENCE_WEEKLY:
		ret = cal_record_set_str(record, _calendar_event.byday, bystr);
		WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_str() Fail(%d)", ret);
		break;
	case VCAL_RECURRENCE_DAILY:
		break;
	}
}

/*
 * Yearly |bymonth   |YM1 6 7 #10
 *                   |YM1 1 6 12 #5 MP1 1+ MO 1- FR
 * Yearly |byyearday |YD3 1 100 200 #10
 * Monthly|byposition|MP2 1+ SU 1- SU #10
 * Monthly|byday     |MD1 1 1- #10
 * Weekly |          |W2 MO WE FR 19941224T000000Z
 * Daly   |          |D2 #0
 */
static void __work_component_property_rrule_ver_1(char *value, calendar_record_h record, struct user_data *ud)
{
	CAL_FN_CALL();

	RET_IF(NULL == value);
	RET_IF('\0' == *value);
	RET_IF(NULL == record);
	RET_IF(NULL == ud);

	int ret = 0;
	char **t = NULL;
	t =  g_strsplit_set(value, ": ", -1);
	RETM_IF(NULL == t, "g_strsplit_set() Fail");

	/* start */
	int len = g_strv_length(t);

	int frequency = 0;
	int freq_mode = 0;
	bool has_by = false;

	char bystr[CAL_STR_MIDDLE_LEN] = {0};
	int len_str = 0;

	int week[5] = {0};
	int week_index = 0;

	int i;
	for (i = 0; i < len; i++) {
		if (NULL == t[i] || '\0' == *t[i])
			continue;

		DBG("[%s]", t[i]);

		if (true == __is_wday_string(t[i])) {
			has_by = true;
			if (week_index) {
				int j = 0;
				for (j = 0; j < week_index; j++) {
					len_str += snprintf(bystr + len_str, sizeof(bystr) - len_str, "%d%s,", week[j], t[i]);
				}
			}
			else {
				len_str += snprintf(bystr + len_str, sizeof(bystr) - len_str, "%s,", t[i]);
			}
			DBG("[%s] week_index(%d)", bystr, week_index);
		}
		else if ('L' == *t[i] && 'D' == *(t[i] +1)) {  /* last day */
			has_by = true;
			len_str += snprintf(bystr + len_str, sizeof(bystr) - len_str, "%s,", "-1");

		}
		else if ('W' == *t[i] && 'K' == *(t[i] +1) && 'S' == *(t[i] +2) && 'T' == *(t[i] +3)) { /* +4 is '=' */
			if ('S' == *(t[i] +5) && 'U' == *(t[i] +6))
				ret = cal_record_set_int(record, _calendar_event.wkst, CALENDAR_SUNDAY);
			else if ('M' == *(t[i] +5))
				ret = cal_record_set_int(record, _calendar_event.wkst, CALENDAR_MONDAY);
			else if ('T' == *(t[i] +5) && 'U' == *(t[i] +6))
				ret = cal_record_set_int(record, _calendar_event.wkst, CALENDAR_TUESDAY);
			else if ('W' == *(t[i] +5))
				ret = cal_record_set_int(record, _calendar_event.wkst, CALENDAR_WEDNESDAY);
			else if ('T' == *(t[i] +5) && 'H' == *(t[i] +6))
				ret = cal_record_set_int(record, _calendar_event.wkst, CALENDAR_THURSDAY);
			else if ('F' == *(t[i] +5))
				ret = cal_record_set_int(record, _calendar_event.wkst, CALENDAR_FRIDAY);
			else if ('S' == *(t[i] +5) && 'A' == *(t[i] +6))
				ret = cal_record_set_int(record, _calendar_event.wkst, CALENDAR_SATURDAY);
			else
				ERR("Invalid parameter[%s]", t[i]);

			WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_int() Fail(%d)", ret);
		}
		else if (true == __is_digit(t[i])) {

			char buf[8] = {0};
			bool exit_loop = false;
			int sign = 1;
			int j = 0;
			switch (freq_mode) {
			case VCAL_RECURRENCE_MONTHLY_BYDAY:
			case VCAL_RECURRENCE_WEEKLY:
				if (true == has_by) {
					week_index = 0;
				}
				while (*(t[i] +j)) {
					switch (*(t[i] +j)) {
					case '+':
						exit_loop = true;
						sign = 1;
						break;
					case '-':
						exit_loop = true;
						sign = -1;
						break;
					default:
						break;
					}
					if (true == exit_loop) break;
					j++;
				}
				snprintf(buf, j +1, "%s", t[i]);
				week[week_index] = atoi(buf) * sign;
				week_index++;
				break;
			default:
				has_by = true;
				while (*(t[i] +j)) {
					switch (*(t[i] +j)) {
					case '+':
						exit_loop = true;
						sign = 1;
						break;
					case '-':
						exit_loop = true;
						sign = -1;
						break;
					default:
						break;
					}
					if (true == exit_loop) break;
					j++;
				}
				snprintf(buf, j +1, "%s", t[i]);
				len_str += snprintf(bystr + len_str, sizeof(bystr) - len_str, "%d,", (atoi(buf) * sign));
				break;
			}
		}
		else {
			if (true == has_by) {
				__set_bystr(freq_mode, record, bystr);
				week_index = 0;
				memset(bystr, 0x0, strlen(bystr));
				len_str = 0;
				has_by = false;
			}
			if (VCAL_RECURRENCE_NONE != (freq_mode = __get_frequency(t[i]))) {
				if (0 == frequency) {
					int interval = 0;
					switch (freq_mode) {
					case VCAL_RECURRENCE_YEARLY_BYMONTH:
						frequency = CALENDAR_RECURRENCE_YEARLY;
						interval = ('\0' == *(t[i] +2)) ? 1 : atoi(t[i] +2);
						break;
					case VCAL_RECURRENCE_YEARLY_BYYEARDAY:
						frequency = CALENDAR_RECURRENCE_YEARLY;
						interval = ('\0' == *(t[i] +2)) ? 1 : atoi(t[i] +2);
						break;
					case VCAL_RECURRENCE_MONTHLY_BYDAY:
						frequency = CALENDAR_RECURRENCE_MONTHLY;
						interval = ('\0' == *(t[i] +2)) ? 1 : atoi(t[i] +2);
						break;
					case VCAL_RECURRENCE_MONTHLY_BYMONTHDAY:
						frequency = CALENDAR_RECURRENCE_MONTHLY;
						interval = ('\0' == *(t[i] +2)) ? 1 : atoi(t[i] +2);
						break;
					case VCAL_RECURRENCE_WEEKLY:
						frequency = CALENDAR_RECURRENCE_WEEKLY;
						interval = ('\0' == *(t[i] +1)) ? 1 : atoi(t[i] +1);
						break;
					case VCAL_RECURRENCE_DAILY:
						frequency = CALENDAR_RECURRENCE_DAILY;
						interval = ('\0' == *(t[i] +1)) ? 1 : atoi(t[i] +1);
						break;
					}
					ret = cal_record_set_int(record, _calendar_event.freq, frequency);
					WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_int() Fail(%d)", ret);
					ret = cal_record_set_int(record, _calendar_event.interval, interval);
					WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_int() Fail(%d)", ret);
					DBG("frequency[%d] interval(%d)", frequency, interval);
				}
			}
			else {
				if ('0' <= *t[i] && *t[i] <= '9' && strlen("YYYYMMDDTHHMMSS") <= strlen(t[i])) {
					DBG("until");
					calendar_time_s caltime = {0};
					_get_caltime(t[i], &caltime, ud);
					ret = cal_record_set_int(record, _calendar_event.range_type, CALENDAR_RANGE_UNTIL);
					WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_int() Fail(%d)", ret);
					ret = cal_record_set_caltime(record, _calendar_event.until_time, caltime);
					WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_caltime() Fail(%d)", ret);

				}
				else if ('#' == *t[i]) {
					/* count */
					if (true == __is_digit(t[i] +1)) {
						if (0 == atoi(t[i] +1)) {
							DBG("endless");
							ret = cal_record_set_int(record, _calendar_event.range_type, CALENDAR_RANGE_NONE);
							WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_int() Fail(%d)", ret);
						}
						else {
							DBG("count (%d)", atoi(t[i] +1));
							ret = cal_record_set_int(record, _calendar_event.range_type, CALENDAR_RANGE_COUNT);
							WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_int() Fail(%d)", ret);
							ret = cal_record_set_int(record, _calendar_event.count, atoi(t[i] +1));
							WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_int() Fail(%d)", ret);
						}
					}
					else {
						ERR("Unable to parse count[%s]", t[i]);
					}
				}
				else {
					DBG("Invalid");
				}
			}
		}
	}
	if (true == has_by) {
		__set_bystr(freq_mode, record, bystr);
	}

	g_strfreev(t);
}

static void __work_component_property_rrule_ver_2(char *value, calendar_record_h record, struct user_data *ud)
{
	int ret = 0;
	char **t = NULL;

	RET_IF(NULL == value);
	RET_IF('\0' == *value);
	RET_IF(NULL == record);
	RET_IF(NULL == ud);

	t =  g_strsplit_set(value, ";:", -1);
	RETM_IF(NULL == t, "g_strsplit_set() Fail");

	ret = cal_record_set_int(record, _calendar_event.range_type, CALENDAR_RANGE_NONE);
	WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_int() Fail(%d)", ret);

	int len = g_strv_length(t);
	int i;
	for (i = 0; i < len; i++) {
		if (NULL == t[i] || '\0' == *t[i]) continue;

		if (CAL_STRING_EQUAL == strncmp(t[i], "FREQ=", strlen("FREQ="))) {
			int frequency = 0;
			if (CAL_STRING_EQUAL == strncmp(t[i] + strlen("FREQ"), "=YEARLY", strlen("=YEARLY")))
				frequency = CALENDAR_RECURRENCE_YEARLY;
			else if (CAL_STRING_EQUAL == strncmp(t[i] + strlen("FREQ"), "=MONTHLY", strlen("=MONTHLY")))
				frequency = CALENDAR_RECURRENCE_MONTHLY;
			else if (CAL_STRING_EQUAL == strncmp(t[i] + strlen("FREQ"), "=WEEKLY", strlen("=WEEKLY")))
				frequency = CALENDAR_RECURRENCE_WEEKLY;
			else if (CAL_STRING_EQUAL == strncmp(t[i] + strlen("FREQ"), "=DAILY", strlen("=DAILY")))
				frequency = CALENDAR_RECURRENCE_DAILY;
			else
				frequency = CALENDAR_RECURRENCE_NONE;

			DBG("frequency(%d) [%s]", frequency, t[i] + strlen("FREQ") + 1);
			ret = cal_record_set_int(record, _calendar_event.freq, frequency);
			WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_int() Fail(%d)", ret);

		}
		else if (CAL_STRING_EQUAL == strncmp(t[i], "UNTIL=", strlen("UNTIL="))) {
			calendar_time_s caltime = {0};
			_get_caltime(t[i] + strlen("UNTIL="), &caltime, ud);
			ret = cal_record_set_caltime(record, _calendar_event.until_time, caltime);
			WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_caltime() Fail(%d)", ret);
			ret = cal_record_set_int(record, _calendar_event.range_type, CALENDAR_RANGE_UNTIL);
			WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_int() Fail(%d)", ret);
		}
		else if (CAL_STRING_EQUAL == strncmp(t[i], "COUNT=", strlen("COUNT="))) {
			int count = atoi(t[i] + strlen("COUNT="));
			if (count < 1) count = 1;
			ret = cal_record_set_int(record,  _calendar_event.count, count);
			WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_int() Fail(%d)", ret);
			ret = cal_record_set_int(record, _calendar_event.range_type, CALENDAR_RANGE_COUNT);
			WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_int() Fail(%d)", ret);
		}
		else if (CAL_STRING_EQUAL == strncmp(t[i], "INTERVAL=", strlen("INTERVAL="))) {
			int interval = atoi(t[i] + strlen("INTERVAL="));
			if (interval < 1) interval = 1;
			ret = cal_record_set_int(record, _calendar_event.interval, interval);
			WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_int() Fail(%d)", ret);
		}
		else if (CAL_STRING_EQUAL == strncmp(t[i], "BYYEARDAY=", strlen("BYYEARDAY="))) {
			ret = cal_record_set_str(record, _calendar_event.byyearday, t[i] + strlen("BYYEARDAY="));
			WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_str() Fail(%d)", ret);
		}
		else if (CAL_STRING_EQUAL == strncmp(t[i], "BYWEEKNO=", strlen("BYWEEKNO="))) {
			ret = cal_record_set_str(record, _calendar_event.byweekno, t[i] + strlen("BYWEEKNO="));
			WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_str() Fail(%d)", ret);
		}
		else if (CAL_STRING_EQUAL == strncmp(t[i], "BYMONTH=", strlen("BYMONTH="))) {
			ret = cal_record_set_str(record, _calendar_event.bymonth, t[i] + strlen("BYMONTH="));
			WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_str() Fail(%d)", ret);
		}
		else if (CAL_STRING_EQUAL == strncmp(t[i], "BYMONTHDAY=", strlen("BYMONTHDAY="))) {
			ret = cal_record_set_str(record, _calendar_event.bymonthday, t[i] + strlen("BYMONTHDAY="));
			WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_str() Fail(%d)", ret);
		}
		else if (CAL_STRING_EQUAL == strncmp(t[i], "BYDAY=", strlen("BYDAY="))) {
			ret = cal_record_set_str(record, _calendar_event.byday, t[i] + strlen("BYDAY="));
			WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_str() Fail(%d)", ret);
		}
		else if (CAL_STRING_EQUAL == strncmp(t[i], "BYSETPOS=", strlen("BYSETPOS="))) {
			ret = cal_record_set_str(record, _calendar_event.bysetpos, t[i] + strlen("BYSETPOS="));
			WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_str() Fail(%d)", ret);
		}
		else if (CAL_STRING_EQUAL == strncmp(t[i], "WKST=", strlen("WKST="))) {
			if (CAL_STRING_EQUAL == strncmp(t[i] + strlen("WKST="), "SU", strlen("SU")))
				ret = cal_record_set_int(record, _calendar_event.wkst, CALENDAR_SUNDAY);
			else if (CAL_STRING_EQUAL == strncmp(t[i] + strlen("WKST="), "MO", strlen("MO")))
				ret = cal_record_set_int(record, _calendar_event.wkst, CALENDAR_MONDAY);
			else if (CAL_STRING_EQUAL == strncmp(t[i] + strlen("WKST="), "TU", strlen("TU")))
				ret = cal_record_set_int(record, _calendar_event.wkst, CALENDAR_TUESDAY);
			else if (CAL_STRING_EQUAL == strncmp(t[i] + strlen("WKST="), "WE", strlen("WE")))
				ret = cal_record_set_int(record, _calendar_event.wkst, CALENDAR_WEDNESDAY);
			else if (CAL_STRING_EQUAL == strncmp(t[i] + strlen("WKST="), "TH", strlen("TH")))
				ret = cal_record_set_int(record, _calendar_event.wkst, CALENDAR_THURSDAY);
			else if (CAL_STRING_EQUAL == strncmp(t[i] + strlen("WKST="), "FR", strlen("FR")))
				ret = cal_record_set_int(record, _calendar_event.wkst, CALENDAR_FRIDAY);
			else if (CAL_STRING_EQUAL == strncmp(t[i] + strlen("WKST="), "SA", strlen("SA")))
				ret = cal_record_set_int(record, _calendar_event.wkst, CALENDAR_SATURDAY);
			else
				DBG("Unable to parse[%s]", t[i]);

			WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_int() Fail(%d)", ret);
		}
		else {
			DBG("Unable to parse[%s]", t[i]);
		}
	}

	g_strfreev(t);
}

static void __work_component_property_rrule(char *value, calendar_record_h record, struct user_data *ud)
{
	CAL_FN_CALL();

	RET_IF(NULL == value);
	RET_IF('\0' == *value);
	RET_IF(NULL == record);
	RET_IF(NULL == ud);

	int version = ud->version;
	/* check if this field has different version content */
	if (CAL_STRING_EQUAL == strncmp(value, ":FREQ=", strlen(":FREQ=")))
		version = VCAL_VER_2;
	else
		version = VCAL_VER_1;

	switch (ud->type) {
	case CALENDAR_BOOK_TYPE_EVENT:
		switch (version) {
		case VCAL_VER_1:
			__work_component_property_rrule_ver_1(value, record, ud);
			break;
		case VCAL_VER_2:
			__work_component_property_rrule_ver_2(value, record, ud);
			break;
		}
		break;
	case CALENDAR_BOOK_TYPE_TODO:
		DBG("Not support rrule in todo");
		break;
	}
}

static void __work_component_property_dtend(char *value, calendar_record_h record, struct user_data *ud)
{
	RET_IF(NULL == value);
	RET_IF('\0' == *value);
	RET_IF(NULL == record);
	RET_IF(NULL == ud);

	int ret = 0;

	value = __decode_datetime(value, ud);
	calendar_time_s dtend = {0};
	ret = _get_caltime(value, &dtend, ud);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("_get_caltime() Fail(%d)", ret);
		return;
	}

	char *tzid = NULL;
	tzid = ud->datetime_tzid ? ud->datetime_tzid : (ud->timezone_tzid ? ud->timezone_tzid : NULL);

	/* check if dtend is earlier than dtstart. */
	do {
		if (CALENDAR_BOOK_TYPE_TODO == ud->type) /* skip in todo */
			break;

		calendar_time_s dtstart = {0};
		ret = calendar_record_get_caltime(record, _calendar_event.start_time, &dtstart);
		WARN_IF(CALENDAR_ERROR_NONE != ret, "calendar_record_get_caltime() Fail(%d)", ret);

		if (0 == dtstart.time.utime) /* not set yet */
			break;

		int diff = 0;
		_sub_caltime(ud, &dtstart, &dtend, &diff);
		if (diff <= 0) /* proper data */
			break;

		WARN("dtend < dtstart so set end time to start");
		dtend = dtstart;
	} while (0);

	switch (ud->type) {
	case CALENDAR_BOOK_TYPE_EVENT:
		if (tzid && *tzid) {
			ret = cal_record_set_str(record, _calendar_event.end_tzid, tzid);
			WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_str() Fail(%d)", ret);
		}
		ret = cal_record_set_caltime(record, _calendar_event.end_time, dtend);
		WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_caltime() Fail(%d)", ret);
		break;
	case CALENDAR_BOOK_TYPE_TODO:
		if (tzid && *tzid) {
			ret = cal_record_set_str(record, _calendar_todo.due_tzid, tzid);
			WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_str() Fail(%d)", ret);
		}
		ret = cal_record_set_caltime(record, _calendar_todo.due_time, dtend);
		WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_caltime() Fail(%d)", ret);
		break;
	}
}

static void __work_component_property_attendee_mailto(calendar_record_h attendee, char *value)
{
	RET_IF(NULL == value);
	RET_IF('\0' == *value);
	RET_IF(NULL == attendee);

	int ret = 0;
	char *mailto = NULL;
	ret = calendar_record_get_str(attendee, _calendar_attendee.email, &mailto);
	WARN_IF(CALENDAR_ERROR_NONE != ret, "calendar_record_get_str() Fail");
	if (mailto)
		return;

	ret = cal_record_set_str(attendee, _calendar_attendee.email, value);
	WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_str() Fail");
}
static void __work_component_property_attendee_cutype(calendar_record_h attendee, char *value)
{
	int ret = 0;

	RET_IF(NULL == value);
	RET_IF('\0' == *value);
	RET_IF(NULL == attendee);

	const char *prop = NULL;
	if (CAL_STRING_EQUAL == strncmp(value, "INDIVIDUAL", strlen("INDIVIDUAL"))) {
		ret = cal_record_set_int(attendee, _calendar_attendee.cutype, CALENDAR_ATTENDEE_CUTYPE_INDIVIDUAL);
		if (strlen(value) > strlen("INDIVIDUAL"))
			prop = "INDIVIDUAL";

	} else if (CAL_STRING_EQUAL == strncmp(value, "GROUP", strlen("GROUP"))) {
		ret = cal_record_set_int(attendee, _calendar_attendee.cutype, CALENDAR_ATTENDEE_CUTYPE_GROUP);
		if (strlen(value) > strlen("GROUP"))
			prop = "GROUP";

	} else if (CAL_STRING_EQUAL == strncmp(value, "RESOURCE", strlen("RESOURCE"))) {
		ret = cal_record_set_int(attendee, _calendar_attendee.cutype, CALENDAR_ATTENDEE_CUTYPE_RESOURCE);
		if (strlen(value) > strlen("RESOURCE"))
			prop = "RESOURCE";

	} else if (CAL_STRING_EQUAL == strncmp(value, "ROOM", strlen("ROOM"))) {
		ret = cal_record_set_int(attendee, _calendar_attendee.cutype, CALENDAR_ATTENDEE_CUTYPE_ROOM);
		if (strlen(value) > strlen("ROOM"))
			prop = "ROOM";

	} else if (CAL_STRING_EQUAL == strncmp(value, "UNKNOWN", strlen("UNKNOWN"))) {
		ret = cal_record_set_int(attendee, _calendar_attendee.cutype, CALENDAR_ATTENDEE_CUTYPE_UNKNOWN);
		if (strlen(value) > strlen("UNKNOWN"))
			prop = "UNKNOWN";

	} else {
		ERR("Invalid value[%s]", value);
	}
	WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_int() Fail(%d)", ret);

	/* check mailto */
	if (prop && CAL_STRING_EQUAL == strncmp(value + strlen(prop), ":MAILTO", strlen(":MAILTO")))
		__work_component_property_attendee_mailto(attendee, value + strlen(prop) + strlen(":MAILTO") +1);
}
static void __work_component_property_attendee_member(calendar_record_h attendee, char *value)
{
	RET_IF(NULL == value);
	RET_IF('\0' == *value);
	RET_IF(NULL == attendee);

	int ret = cal_record_set_str(attendee, _calendar_attendee.member, value);
	WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_str() Fail(%d)", ret);
}
static void __work_component_property_attendee_role(calendar_record_h attendee, char *value)
{
	int ret = 0;

	RET_IF(NULL == value);
	RET_IF('\0' == *value);
	RET_IF(NULL == attendee);

	const char *prop = NULL;
	if (CAL_STRING_EQUAL == strncmp(value, "REQ-PARTICIPANT", strlen("REQ-PARTICIPANT"))) {
		ret = cal_record_set_int(attendee, _calendar_attendee.cutype, CALENDAR_ATTENDEE_ROLE_REQ_PARTICIPANT);
		if (strlen(value) > strlen("REQ-PARTICIPANT"))
			prop = "REQ-PARTICIPANT";
	}
	else if (CAL_STRING_EQUAL == strncmp(value, "OPT-PARTICIPANT", strlen("OPT-PARTICIPANT"))) {
		ret = cal_record_set_int(attendee, _calendar_attendee.cutype, CALENDAR_ATTENDEE_ROLE_OPT_PARTICIPANT);
		if (strlen(value) > strlen("OPT-PARTICIPANT"))
			prop = "OPT-PARTICIPANT";
	}
	else if (CAL_STRING_EQUAL == strncmp(value, "NON-PARTICIPANT", strlen("NON-PARTICIPANT"))) {
		ret = cal_record_set_int(attendee, _calendar_attendee.cutype, CALENDAR_ATTENDEE_ROLE_NON_PARTICIPANT);
		if (strlen(value) > strlen("NON-PARTICIPANT"))
			prop = "NON-PARTICIPANT";
	}
	else if (CAL_STRING_EQUAL == strncmp(value, "CHAIR", strlen("CHAIR"))) {
		ret = cal_record_set_int(attendee, _calendar_attendee.cutype, CALENDAR_ATTENDEE_ROLE_CHAIR);
		if (strlen(value) > strlen("CHAIR"))
			prop = "CHAIR";
	}
	else {
		ERR("Invalid value[%s]", value);
	}
	WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_int() Fail(%d)", ret);

	/* check mailto */
	if (prop && CAL_STRING_EQUAL == strncmp(value + strlen(prop), ":MAILTO", strlen(":MAILTO")))
		__work_component_property_attendee_mailto(attendee, value + strlen(prop) + strlen(":MAILTO") +1);
}
static void __work_component_property_attendee_partstat(calendar_record_h attendee, char *value)
{
	int ret = 0;

	RET_IF(NULL == value);
	RET_IF('\0' == *value);
	RET_IF(NULL == attendee);

	const char *prop = NULL;
	if (CAL_STRING_EQUAL == strncmp(value, "NEEDS-ACTION", strlen("NEEDS-ACTION"))) {
		ret = cal_record_set_int(attendee, _calendar_attendee.cutype, CALENDAR_ATTENDEE_STATUS_PENDING);
		if (strlen(value) > strlen("NEEDS-ACTION"))
			prop = "NEEDS-ACTION";

	} else if (CAL_STRING_EQUAL == strncmp(value, "ACCEPTED", strlen("ACCEPTED"))) {
		ret = cal_record_set_int(attendee, _calendar_attendee.cutype, CALENDAR_ATTENDEE_STATUS_ACCEPTED);
		if (strlen(value) > strlen("ACCEPTED"))
			prop = "ACCEPTED";

	} else if (CAL_STRING_EQUAL == strncmp(value, "DECLINED", strlen("DECLINED"))) {
		ret = cal_record_set_int(attendee, _calendar_attendee.cutype, CALENDAR_ATTENDEE_STATUS_DECLINED);
		if (strlen(value) > strlen("DECLINED"))
			prop = "DECLINED";

	} else if (CAL_STRING_EQUAL == strncmp(value, "TENTATIVE", strlen("TENTATIVE"))) {
		ret = cal_record_set_int(attendee, _calendar_attendee.cutype, CALENDAR_ATTENDEE_STATUS_TENTATIVE);
		if (strlen(value) > strlen("TENTATIVE"))
			prop = "TENTATIVE";

	} else if (CAL_STRING_EQUAL == strncmp(value, "DELEGATED", strlen("DELEGATED"))) {
		ret = cal_record_set_int(attendee, _calendar_attendee.cutype, CALENDAR_ATTENDEE_STATUS_DELEGATED);
		if (strlen(value) > strlen("DELEGATED"))
			prop = "DELEGATED";

	} else if (CAL_STRING_EQUAL == strncmp(value, "COMPLETED", strlen("COMPLETED"))) {
		ret = cal_record_set_int(attendee, _calendar_attendee.cutype, CALENDAR_ATTENDEE_STATUS_COMPLETED);
		if (strlen(value) > strlen("COMPLETED"))
			prop = "COMPLETED";

	} else if (CAL_STRING_EQUAL == strncmp(value, "IN-PROCESS", strlen("IN-PROCESS"))) {
		ret = cal_record_set_int(attendee, _calendar_attendee.cutype, CALENDAR_ATTENDEE_STATUS_IN_PROCESS);
		if (strlen(value) > strlen("IN-PROCESS"))
			prop = "IN-PROCESS";

	} else {
		ERR("Invalid value[%s]", value);
	}
	WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_int() Fail(%d)", ret);

	/* check mailto */
	if (prop && CAL_STRING_EQUAL == strncmp(value + strlen(prop), ":MAILTO", strlen(":MAILTO")))
		__work_component_property_attendee_mailto(attendee, value + strlen(prop) + strlen(":MAILTO") +1);
}
static void __work_component_property_attendee_rsvp(calendar_record_h attendee, char *value)
{
	RET_IF(NULL == value);
	RET_IF('\0' == *value);
	RET_IF(NULL == attendee);

	int ret = 0;
	if (CAL_STRING_EQUAL == strncmp(value, "TRUE", strlen("TRUE")))
		ret = cal_record_set_int(attendee, _calendar_attendee.rsvp, 1);
	else
		ret = cal_record_set_int(attendee, _calendar_attendee.rsvp, 0);

	WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_int() Fail(%d)", ret);
}
static void __work_component_property_attendee_delegated_to(calendar_record_h attendee, char *value)
{
	RET_IF(NULL == value);
	RET_IF('\0' == *value);
	RET_IF(NULL == attendee);

	int ret = 0;
	ret = cal_record_set_str(attendee, _calendar_attendee.delegatee_uri, value);
	WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_str() Fail");
}
static void __work_component_property_attendee_delegated_from(calendar_record_h attendee, char *value)
{
	RET_IF(NULL == value);
	RET_IF('\0' == *value);
	RET_IF(NULL == attendee);

	int ret = 0;
	ret = cal_record_set_str(attendee, _calendar_attendee.delegator_uri, value);
	WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_str() Fail");
}
static void __work_component_property_attendee_sent_by(calendar_record_h attendee, char *value)
{
	return;
}
static void __work_component_property_attendee_cn(calendar_record_h attendee, char *value)
{
	RET_IF(NULL == value);
	RET_IF('\0' == *value);
	RET_IF(NULL == attendee);

	int ret = 0;
	ret = cal_record_set_str(attendee, _calendar_attendee.name, value);
	WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_str() Fail");
}
static void __work_component_property_attendee_dir(calendar_record_h attendee, char *value)
{
	return;
}

/*
 * example
 * ATTENDEE;ROLE=REQ-PARTICIPANT;DELEGATED-FROM="MAILTO:bob@host.com";PARTSTAT=ACCEPTED;CN=Jane Doe:MAILTO:jdoe@host1.com
 * ATTENDEE;CN=John Smith;DIR="ldap://host.com:6666/o=eDABC%20Industries,c=3DUS??(cn=3DBJim%20Dolittle)":MAILTO:jimdo@host1.com
 * ATTENDEE;RSVP=TRUE;ROLE=REQ-PARTICIPANT;CUTYPE=GROUP:MAILTO:employee-A@host.com
 * ATTENDEE;CN=MAILTO:MAILTO:MAILTO@host.com
 */
static void __work_component_property_attendee(char *value, calendar_record_h record, struct user_data *ud)
{
	RET_IF(NULL == value);
	RET_IF('\0' == *value);
	RET_IF(NULL == record);
	RET_IF(NULL == ud);

	int ret = 0;
	calendar_record_h attendee = NULL;
	ret = calendar_record_create(_calendar_attendee._uri, &attendee);
	RETM_IF(CALENDAR_ERROR_NONE != ret, "calendar_record_create() Fail(%d)", ret);

	char **t = NULL;
	t =  g_strsplit(value, ";", -1);
	RETM_IF(NULL == t, "g_strsplit() Fail");

	int len = g_strv_length(t);
	int i;
	for (i = 0; i < len; i++) {
		if (NULL == t[i] || '\0' == *t[i]) continue;

		if (CAL_STRING_EQUAL == strncmp(t[i], "CUTYPE", strlen("CUTYPE")))
			__work_component_property_attendee_cutype(attendee, t[i] + strlen("CUTYPE") +1);
		else if (CAL_STRING_EQUAL == strncmp(t[i], "MEMBER", strlen("MEMBER")))
			__work_component_property_attendee_member(attendee, t[i] + strlen("MEMBER") +1);
		else if (CAL_STRING_EQUAL == strncmp(t[i], "ROLE", strlen("ROLE")))
			__work_component_property_attendee_role(attendee, t[i] + strlen("ROLE") +1);
		else if (CAL_STRING_EQUAL == strncmp(t[i], "PARTSTAT", strlen("PARTSTAT")))
			__work_component_property_attendee_partstat(attendee, t[i] + strlen("PARTSTAT") +1);
		else if (CAL_STRING_EQUAL == strncmp(t[i], "RSVP", strlen("RSVP")))
			__work_component_property_attendee_rsvp(attendee, t[i] + strlen("RSVP") +1);
		else if (CAL_STRING_EQUAL == strncmp(t[i], "DELEGATED-TO", strlen("DELEGATED-TO")))
			__work_component_property_attendee_delegated_to(attendee, t[i] + strlen("DELEGATED-TO") +1);
		else if (CAL_STRING_EQUAL == strncmp(t[i], "DELEGATED-FROM", strlen("DELEGATED-FROM")))
			__work_component_property_attendee_delegated_from(attendee, t[i] + strlen("DELEGATED-FROM") +1);
		else if (CAL_STRING_EQUAL == strncmp(t[i], "SENT_BY", strlen("SENT_BY")))
			__work_component_property_attendee_sent_by(attendee, t[i] + strlen("SENT_BY") +1);
		else if (CAL_STRING_EQUAL == strncmp(t[i], "CN", strlen("CN")))
			__work_component_property_attendee_cn(attendee, t[i] + strlen("CN") +1);
		else if (CAL_STRING_EQUAL == strncmp(t[i], "DIR", strlen("DIR")))
			__work_component_property_attendee_dir(attendee, t[i] + strlen("DIR") +1);
		else if (CAL_STRING_EQUAL == strncmp(t[i], ":MAILTO", strlen(":MAILTO")))
			__work_component_property_attendee_mailto(attendee, t[i] + strlen(":MAILTO") +1);
		else
			ERR("Invalid value[%s]", t[i]);

	}
	g_strfreev(t);

	switch (ud->type) {
	case CALENDAR_BOOK_TYPE_EVENT:
		ret = calendar_record_add_child_record(record, _calendar_event.calendar_attendee, attendee);
		WARN_IF(CALENDAR_ERROR_NONE != ret, "calendar_record_add_child_record() Fail(%d)", ret);
		break;
	case CALENDAR_BOOK_TYPE_TODO:
		ret = calendar_record_add_child_record(record, _calendar_event.calendar_attendee, attendee);
		WARN_IF(CALENDAR_ERROR_NONE != ret, "calendar_record_add_child_record() Fail(%d)", ret);
		break;
	}
}

static void __work_component_property_categories(char *value, calendar_record_h record, struct user_data *ud)
{
	RET_IF(NULL == value);
	RET_IF('\0' == *value);
	RET_IF(NULL == record);
	RET_IF(NULL == ud);

	int ret = 0;
	char *s = __decode_charset(value);
	__decode_escaped_char(s);
	switch (ud->type) {
	case CALENDAR_BOOK_TYPE_EVENT:
		ret = cal_record_set_str(record, _calendar_event.categories, s);
		WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_str() Fail(%d)", ret);
		break;
	case CALENDAR_BOOK_TYPE_TODO:
		ret = cal_record_set_str(record, _calendar_todo.categories, s);
		WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_str() Fail(%d)", ret);
		break;
	}
	free(s);
}

static void _set_alarm_tick_unit(calendar_record_h alarm, calendar_time_s alarm_time, int diff)
{
	int ret = 0;

	RET_IF(NULL == alarm);

	if (diff < 0) {
		DBG("set specific alarm");
		ret = cal_record_set_caltime(alarm, _calendar_alarm.alarm_time, alarm_time);
		WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_caltime() Fail(%d)", ret);
		ret = cal_record_set_int(alarm, _calendar_alarm.tick_unit, CALENDAR_ALARM_TIME_UNIT_SPECIFIC);
		WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_int() Fail(%d)", ret);
	}
	else if (0 == diff) {
		DBG("set alarm in start time");
		ret = cal_record_set_int(alarm, _calendar_alarm.tick, 0);
		WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_int() Fail(%d)", ret);
		ret = cal_record_set_int(alarm, _calendar_alarm.tick_unit, CALENDAR_ALARM_TIME_UNIT_HOUR);
		WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_int() Fail(%d)", ret);
	}
	else {
		int tick = 0;
		int unit = 0;
		_get_tick_unit(diff, &tick, &unit);
		ret = cal_record_set_int(alarm, _calendar_alarm.tick, tick);
		WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_int() Fail(%d)", ret);
		ret = cal_record_set_int(alarm, _calendar_alarm.tick_unit, unit);
		WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_int() Fail(%d)", ret);
	}
}

/*
 * for ver 1.0
 * dalarmparts	= 0*3(strnosemi ";") strnosemi; runTime, snoozeTime, repeatCount, displayString
 * DALARM:19960415T235000;PT5M;2;Your Taxes Are Due !!!
 */
static void __work_component_property_dalarm(char *value, calendar_record_h record, struct user_data *ud)
{
	RET_IF(NULL == value);
	RET_IF('\0' == *value);
	RET_IF(NULL == record);
	RET_IF(NULL == ud);

	int ret = 0;
	char **t = NULL;
	t =  g_strsplit_set(value, ";:", -1);
	RETM_IF(NULL == t, "g_strsplit_set() Fail");

	calendar_record_h alarm = NULL;
	ret = calendar_record_create(_calendar_alarm._uri, &alarm);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("calendar_record_create() Fail(%d)", ret);
		g_strfreev(t);
		return;
	}
	ret = cal_record_set_int(alarm, _calendar_alarm.action, CALENDAR_ALARM_ACTION_DISPLAY);
	WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_int() Fail(%d)", ret);

	int len = g_strv_length(t);
	int i;
	int index = VCAL_VER_10_DALARM_NONE;
	for (i = 0; i < len; i++) {
		if (index)
			index++;
		if (NULL == t[i] || '\0' == *t[i])
			continue;

		if ('0' <= *t[i] && *t[i] <= '9' && strlen("PTM") < strlen(t[i])) {
			/* runTime */
			index = VCAL_VER_10_DALARM_RUN_TIME;
			calendar_time_s alarm_time = {0};
			ret = _get_caltime(t[i], &alarm_time, ud);
			if (CALENDAR_ERROR_NONE != ret) {
				ERR("_get_caltime() Fail(%d)", ret);
				index = VCAL_VER_10_AALARM_NONE;
				break;
			}

			if (true == ud->has_rrule) {
				calendar_time_s start_time = {0};
				ret = calendar_record_get_caltime(record, _calendar_event.start_time, &start_time);

				int diff = 0;
				ret = _sub_caltime(ud, &start_time, &alarm_time, &diff);
				if (CALENDAR_ERROR_NONE != ret) {
					ERR("_sub_caltime() Fail(%d)", ret);
					index = VCAL_VER_10_DALARM_NONE;
					break;
				}
				_set_alarm_tick_unit(alarm, alarm_time, diff);

			} else {
				_set_alarm_tick_unit(alarm, alarm_time, -1); /* -1 goes to specific time */
			}
		}
		else if (VCAL_VER_10_DALARM_DISPLAY_STRING == index) { /* displayString */
			DBG("displayString [%s]", t[i]);
			ret = cal_record_set_str(alarm, _calendar_alarm.summary, t[i]);
			WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_str() Fail(%d)", ret);

		}
		else {
			/* TYPE, VALUE */
		}
	}

	if (VCAL_VER_10_DALARM_NONE == index) {
		DBG("No alarm");
		calendar_record_destroy(alarm, true);
		g_strfreev(t);
		return;
	}

	switch (ud->type) {
	case VCALENDAR_TYPE_VEVENT:
		ret = calendar_record_add_child_record(record, _calendar_event.calendar_alarm, alarm);
		WARN_IF(CALENDAR_ERROR_NONE != ret, "calendar_record_add_child_record() Fail(%d)", ret);
		break;
	case VCALENDAR_TYPE_VTODO:
		ret = calendar_record_add_child_record(record, _calendar_todo.calendar_alarm, alarm);
		WARN_IF(CALENDAR_ERROR_NONE != ret, "calendar_record_add_child_record() Fail(%d)", ret);
		break;
	}
	g_strfreev(t);
}

/*
 * for ver 1.0
 * malarmparts	= 0*4(strnosemi ";") strnosemi; runTime, snoozeTime, repeatCount, addressString, noteString
 */
static void __work_component_property_malarm(char *value, calendar_record_h record, struct user_data *ud)
{
	/* diff with aalarm: action */
	RET_IF(NULL == value);
	RET_IF('\0' == *value);
	RET_IF(NULL == record);
	RET_IF(NULL == ud);

	int ret = 0;
	char **t = NULL;
	t =  g_strsplit_set(value, ";:", -1);
	RETM_IF(NULL == t, "g_strsplit_set() Fail");

	calendar_record_h alarm = NULL;
	ret = calendar_record_create(_calendar_alarm._uri, &alarm);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("calendar_record_create() Fail(%d)", ret);
		g_strfreev(t);
		return;
	}
	ret = cal_record_set_int(alarm, _calendar_alarm.action, CALENDAR_ALARM_ACTION_EMAIL);
	WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_int() Fail(%d)", ret);

	int len = g_strv_length(t);
	int i;
	int index = VCAL_VER_10_MALARM_NONE;
	for (i = 0; i < len; i++) {
		if (index)
			index++;
		if (NULL == t[i] || '\0' == *t[i])
			continue;

		if ('0' <= *t[i] && *t[i] <= '9' && strlen("PTM") < strlen(t[i])) {
			/* runTime */
			index = VCAL_VER_10_MALARM_RUN_TIME;
			calendar_time_s alarm_time = {0};
			ret = _get_caltime(t[i], &alarm_time, ud);
			if (CALENDAR_ERROR_NONE != ret) {
				ERR("_get_caltime() Fail(%d)", ret);
				index = VCAL_VER_10_AALARM_NONE;
				break;
			}

			if (true == ud->has_rrule) {
				calendar_time_s start_time = {0};
				ret = calendar_record_get_caltime(record, _calendar_event.start_time, &start_time);
				WARN_IF(CALENDAR_ERROR_NONE != ret, "calendar_record_get_caltime() Fail(%d)", ret);

				int diff = 0;
				ret = _sub_caltime(ud, &start_time, &alarm_time, &diff);
				if (CALENDAR_ERROR_NONE != ret) {
					ERR("_sub_caltime() Fail(%d)", ret);
					index = VCAL_VER_10_MALARM_NONE;
					break;
				}
				_set_alarm_tick_unit(alarm, alarm_time, diff);

			} else {
				_set_alarm_tick_unit(alarm, alarm_time, -1); /* -1 goes to specific time */
			}
		}
		else if (VCAL_VER_10_MALARM_ADDRESS_STRING == index) { /* addressString */
			DBG("addressString [%s]", t[i]);
			ret = cal_record_set_str(alarm, _calendar_alarm.attach, t[i]);
			WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_str() Fail(%d)", ret);

		}
		else if (VCAL_VER_10_MALARM_NOTE_STRING == index) { /* noteString */
			DBG("noteString [%s]", t[i]);
			ret = cal_record_set_str(alarm, _calendar_alarm.description, t[i]);
			WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_str() Fail(%d)", ret);

		}
		else {
			/* TYPE, VALUE */
		}
	}

	if (VCAL_VER_10_MALARM_NONE == index) {
		DBG("No alarm");
		calendar_record_destroy(alarm, true);
		g_strfreev(t);
		return;
	}

	switch (ud->type) {
	case VCALENDAR_TYPE_VEVENT:
		ret = calendar_record_add_child_record(record, _calendar_event.calendar_alarm, alarm);
		WARN_IF(CALENDAR_ERROR_NONE != ret, "calendar_record_add_child_record() Fail(%d)", ret);
		break;
	case VCALENDAR_TYPE_VTODO:
		ret = calendar_record_add_child_record(record, _calendar_todo.calendar_alarm, alarm);
		WARN_IF(CALENDAR_ERROR_NONE != ret, "calendar_record_add_child_record() Fail(%d)", ret);
		break;
	}
	g_strfreev(t);
}
/*
 * for ver 1.0
 * aalarmparts	= 0*3(strnosemi ";") strnosemi; runTime, snoozeTime, repeatCount, audioContent
 * AALARM;TYPE=WAVE;VALUE=URL:19960415T235959; ; ; file:///mmedia/taps.wav
 * AALARM;TYPE=WAVE;VALUE=CONTENT-ID:19960903T060000;PT15M;4;<jsmith.part2.=960901T083000.xyzMail@host1.com>
 */
static void __work_component_property_aalarm(char *value, calendar_record_h record, struct user_data *ud)
{
	CAL_FN_CALL();

	RET_IF(NULL == value);
	RET_IF('\0' == *value);
	RET_IF(NULL == record);
	RET_IF(NULL == ud);

	int ret = 0;
	char **t = NULL;
	t =  g_strsplit_set(value, ";:", -1);
	RETM_IF(NULL == t, "g_strsplit_set() Fail");

	calendar_record_h alarm = NULL;
	ret = calendar_record_create(_calendar_alarm._uri, &alarm);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("calendar_record_create() Fail(%d)", ret);
		g_strfreev(t);
		return;
	}
	ret = cal_record_set_int(alarm, _calendar_alarm.action, CALENDAR_ALARM_ACTION_AUDIO);
	WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_int() Fail(%d)", ret);

	int len = g_strv_length(t);
	int i;
	int index = VCAL_VER_10_AALARM_NONE;
	for (i = 0; i < len; i++) {
		if (index)
			index++;
		if (NULL == t[i] || '\0' == *t[i])
			continue;

		if ('0' <= *t[i] && *t[i] <= '9' && strlen("PTM") < strlen(t[i])) {
			/* runTime */
			index = VCAL_VER_10_AALARM_RUN_TIME;
			calendar_time_s alarm_time = {0};
			ret = _get_caltime(t[i], &alarm_time, ud);
			if (CALENDAR_ERROR_NONE != ret) {
				ERR("_get_caltime() Fail(%d)", ret);
				index = VCAL_VER_10_AALARM_NONE;
				break;
			}

			if (true == ud->has_rrule) {
				calendar_time_s start_time = {0};
				ret = calendar_record_get_caltime(record, _calendar_event.start_time, &start_time);

				int diff = 0;
				ret = _sub_caltime(ud, &start_time, &alarm_time, &diff);
				if (CALENDAR_ERROR_NONE != ret) {
					ERR("_sub_caltime() Fail(%d)", ret);
					index = VCAL_VER_10_AALARM_NONE;
					break;
				}
				_set_alarm_tick_unit(alarm, alarm_time, diff);

			} else {
				_set_alarm_tick_unit(alarm, alarm_time, -1); /* -1 goes to specific time */
			}
		}
		else if (VCAL_VER_10_AALARM_AUDIO_CONTENT == index) {
			/* audioContent */
			DBG("Content [%s]", t[i]);
			ret = cal_record_set_str(alarm, _calendar_alarm.attach, t[i]);
			WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_str() Fail(%d)", ret);
		}
		else {
			/* TYPE, VALUE */
		}
	}

	if (VCAL_VER_10_AALARM_NONE == index) {
		DBG("No alarm");
		calendar_record_destroy(alarm, true);
		g_strfreev(t);
		return;
	}

	switch (ud->type) {
	case VCALENDAR_TYPE_VEVENT:
		ret = calendar_record_add_child_record(record, _calendar_event.calendar_alarm, alarm);
		WARN_IF(CALENDAR_ERROR_NONE != ret, "calendar_record_add_child_record() Fail(%d)", ret);
		break;
	case VCALENDAR_TYPE_VTODO:
		ret = calendar_record_add_child_record(record, _calendar_todo.calendar_alarm, alarm);
		WARN_IF(CALENDAR_ERROR_NONE != ret, "calendar_record_add_child_record() Fail(%d)", ret);
		break;
	}
	g_strfreev(t);
}

static void __work_component_property_exdate(char *value, calendar_record_h record, struct user_data *ud)
{
	RET_IF(NULL == value);
	RET_IF('\0' == *value);
	RET_IF(NULL == record);
	RET_IF(NULL == ud);

	int ret = 0;
	switch (ud->type) {
	case CALENDAR_BOOK_TYPE_EVENT:
		ret = cal_record_set_str(record, _calendar_event.exdate, value + 1);
		WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_str() Fail(%d)", ret);
		break;
	case CALENDAR_BOOK_TYPE_TODO:
		ERR("No exdate in todo");
		break;
	}
}

static void __work_component_property_x_allday(char *value, calendar_record_h record, struct user_data *ud)
{
	RET_IF(NULL == value);
	RET_IF('\0' == *value);
	RET_IF(NULL == record);
	RET_IF(NULL == ud);

	int ret = 0;
	if (CAL_STRING_EQUAL == strncmp(value, ":SET", strlen(":SET"))) {
		DBG("x-allday: set");
		ud->is_allday = true;

		calendar_time_s caltime = {0};
		switch (ud->type) {
		case CALENDAR_BOOK_TYPE_EVENT:
			ret = calendar_record_get_caltime(record, _calendar_event.start_time, &caltime);
			WARN_IF(CALENDAR_ERROR_NONE != ret, "calendar_record_get_caltime() Fail");
			if (CALENDAR_TIME_LOCALTIME == caltime.type) {
				caltime.time.date.hour = 0;
				caltime.time.date.minute = 0;
				caltime.time.date.second = 0;
				ret = cal_record_set_caltime(record, _calendar_event.start_time, caltime);
				WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_caltime() Fail");
			}
			ret = calendar_record_get_caltime(record, _calendar_event.end_time, &caltime);
			WARN_IF(CALENDAR_ERROR_NONE != ret, "calendar_record_get_caltime() Fail");
			if (CALENDAR_TIME_LOCALTIME == caltime.type) {
				caltime.time.date.hour = 0;
				caltime.time.date.minute = 0;
				caltime.time.date.second = 0;
				ret = cal_record_set_caltime(record, _calendar_event.end_time, caltime);
				WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_caltime() Fail");
			}
			ret = calendar_record_get_caltime(record, _calendar_event.until_time, &caltime);
			WARN_IF(CALENDAR_ERROR_NONE != ret, "calendar_record_get_caltime() Fail");
			if (CALENDAR_TIME_LOCALTIME == caltime.type) {
				caltime.time.date.hour = 0;
				caltime.time.date.minute = 0;
				caltime.time.date.second = 0;
				ret = cal_record_set_caltime(record, _calendar_event.until_time, caltime);
				WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_caltime() Fail");
			}
			break;
		case CALENDAR_BOOK_TYPE_TODO:
			ret = calendar_record_get_caltime(record, _calendar_todo.start_time, &caltime);
			WARN_IF(CALENDAR_ERROR_NONE != ret, "calendar_record_get_caltime() Fail");
			if (CALENDAR_TIME_LOCALTIME == caltime.type) {
				caltime.time.date.hour = 0;
				caltime.time.date.minute = 0;
				caltime.time.date.second = 0;
				ret = cal_record_set_caltime(record, _calendar_todo.start_time, caltime);
				WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_caltime() Fail");
			}
			ret = calendar_record_get_caltime(record, _calendar_todo.due_time, &caltime);
			WARN_IF(CALENDAR_ERROR_NONE != ret, "calendar_record_get_caltime() Fail");
			if (CALENDAR_TIME_LOCALTIME == caltime.type) {
				caltime.time.date.hour = 0;
				caltime.time.date.minute = 0;
				caltime.time.date.second = 0;
				ret = cal_record_set_caltime(record, _calendar_todo.due_time, caltime);
				WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_caltime() Fail");
			}
			break;
		}
	}
}

static void __work_component_property_x_lunar(char *value, calendar_record_h record, struct user_data *ud)
{
	RET_IF(NULL == value);
	RET_IF('\0' == *value);
	RET_IF(NULL == record);
	RET_IF(NULL == ud);

	int ret = 0;
	if (CAL_STRING_EQUAL == strncmp(value, ":SET", strlen(":SET"))) {
		DBG("x-lunar: set");
		switch (ud->type) {
		case CALENDAR_BOOK_TYPE_EVENT:
			ret = cal_record_set_int(record, _calendar_event.calendar_system_type, CALENDAR_SYSTEM_EAST_ASIAN_LUNISOLAR);
			WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_int() Fail(%d)", ret);
			break;
		case CALENDAR_BOOK_TYPE_TODO:
			DBG("Not supported lunar in todo");
			break;
		}
	}
}

static void __work_component_property_valarm_action(char *value, calendar_record_h alarm, struct user_data *ud)
{
	RET_IF(NULL == value);
	RET_IF('\0' == *value);
	RET_IF(NULL == alarm);

	const char *prop[CALENDAR_ALARM_ACTION_MAX] = {":AUDIO", ":DISPLAY", ":EMAIL"};

	int ret = 0;
	int i;
	for (i = 0; i < CALENDAR_ALARM_ACTION_MAX; i++) {
		if (CAL_STRING_EQUAL == strncmp(value,  prop[i], strlen(prop[i]))) {
			ret = cal_record_set_int(alarm, _calendar_alarm.action, i);
			WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_int() Fail(%d)", ret);
			break;
		}
	}
}

static void __work_component_property_valarm_trigger(char *value, calendar_record_h record, calendar_record_h alarm, struct user_data *ud)
{
	CAL_FN_CALL();

	RET_IF(NULL == value);
	RET_IF('\0' == *value);
	RET_IF(NULL == alarm);
	RET_IF(NULL == ud);

	int ret = 0;
	char **t = NULL;
	t =  g_strsplit_set(value, ";:", -1);
	RETM_IF(NULL == t, "g_strsplit_set() Fail");

	int related = VCAL_RELATED_NONE;
	int len = g_strv_length(t);
	int i;
	for (i = 0; i < len; i++) {
		if (NULL == t[i] || '\0' == *t[i]) continue;

		if (CAL_STRING_EQUAL == strncmp(t[i], "RELATED", strlen("RELATED"))) {
			if (CAL_STRING_EQUAL == strncmp(t[i] + strlen("RELATED"), "=START", strlen("=START")))
				related = VCAL_RELATED_START;
			else if (CAL_STRING_EQUAL == strncmp(t[i] + strlen("RELATED"), "=END", strlen("=END")))
				related = VCAL_RELATED_END;
			else
				ERR("Invalid related:[%s]", t[i]);
		}
		else if (CAL_STRING_EQUAL == strncmp(t[i], "VALUE", strlen("VALUE"))) {
			/* do nothing */
		}
		else {
			if ('0' <= *t[i] && *t[i] <= '9' && strlen("YYYYDDMM") <= strlen(t[i])) {
				calendar_time_s caltime = {0};
				_get_caltime(t[i], &caltime, ud);
				ret = cal_record_set_caltime(alarm, _calendar_alarm.alarm_time, caltime);
				WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_caltime() Fail(%d)", ret);
				ret = cal_record_set_int(alarm, _calendar_alarm.tick_unit, CALENDAR_ALARM_TIME_UNIT_SPECIFIC);
				WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_int() Fail(%d)", ret);
			}
			else {
				int unit = 0;
				int tick = 0;
				__decode_duration(t[i], strlen(t[i]), &tick, &unit);
				if (CALENDAR_ALARM_TIME_UNIT_SPECIFIC == unit || 0 < tick) {
					if (CALENDAR_ALARM_TIME_UNIT_SPECIFIC == unit)
						DBG("alarm tick is second, changed as specific.");
					if (0 < tick)
						DBG("alarm is set after start/end time(%d).", tick);

					calendar_time_s caltime = {0};
					if (VCAL_RELATED_NONE == related) {
						switch (ud->type) {
						case CALENDAR_BOOK_TYPE_EVENT:
							related = VCAL_RELATED_START;
							break;
						case CALENDAR_BOOK_TYPE_TODO:
							related = VCAL_RELATED_END;
							break;
						}
					}
					switch (related) {
					case VCAL_RELATED_START:
						ret = calendar_record_get_caltime(record, _calendar_event.start_time, &caltime);
						WARN_IF(CALENDAR_ERROR_NONE != ret, "calendar_record_get_caltime() Fail(%d)", ret);
						break;
					case VCAL_RELATED_END:
						ret = calendar_record_get_caltime(record, _calendar_event.end_time, &caltime);
						WARN_IF(CALENDAR_ERROR_NONE != ret, "calendar_record_get_caltime() Fail(%d)", ret);
						break;
					}
					cal_time_modify_caltime(&caltime, tick * unit);
					ret = cal_record_set_caltime(alarm, _calendar_alarm.alarm_time, caltime);
					WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_caltime() Fail(%d)", ret);
					ret = cal_record_set_int(alarm, _calendar_alarm.tick_unit, CALENDAR_ALARM_TIME_UNIT_SPECIFIC);
					WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_int() Fail(%d)", ret);
				}
				else {
					ret = cal_record_set_int(alarm, _calendar_alarm.tick, (-1 * tick));
					WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_int() Fail(%d)", ret);
					ret = cal_record_set_int(alarm, _calendar_alarm.tick_unit, unit);
					WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_int() Fail(%d)", ret);
				}
			}
		}
	}

	g_strfreev(t);
}

static void __work_component_property_valarm_repeat(char *value, calendar_record_h alarm, struct user_data *ud)
{
	return;
}

static void __work_component_property_valarm_attach(char *value, calendar_record_h alarm, struct user_data *ud)
{
	RET_IF(NULL == value);
	RET_IF('\0' == *value);
	RET_IF(NULL == alarm);

	int ret = cal_record_set_str(alarm, _calendar_alarm.attach, value + 1);
	WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_str() Fail(%d)", ret);
}

static void __work_component_property_valarm_description(char *value, calendar_record_h alarm, struct user_data *ud)
{
	RET_IF(NULL == value);
	RET_IF('\0' == *value);
	RET_IF(NULL == alarm);

	int ret = cal_record_set_str(alarm, _calendar_alarm.description, value + 1);
	WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_str() Fail(%d)", ret);
}

static void __work_component_property_valarm_summary(char *value, calendar_record_h alarm, struct user_data *ud)
{
	RET_IF(NULL == value);
	RET_IF('\0' == *value);
	RET_IF(NULL == alarm);

	int ret = cal_record_set_str(alarm, _calendar_alarm.summary, value);
	WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_str() Fail(%d)", ret);
}

static void __work_component_property_valarm_duration(char *value, calendar_record_h alarm, struct user_data *ud)
{
	return;
}

static char* __work_component_property_begin(char *cursor, calendar_record_h record, struct user_data *ud)
{
	CAL_FN_CALL();

	RETV_IF(NULL == cursor, NULL);
	RETV_IF(NULL == record, NULL);
	RETV_IF(NULL == ud, NULL);

	if (0 != strncmp(cursor, ":VALARM", strlen(":VALARM")))	{
		DBG("this is not valarm");
		return __crlf(cursor);
	}

	__init_component_property_valarm();

	int ret = 0;
	calendar_record_h alarm = NULL;
	ret = calendar_record_create(_calendar_alarm._uri, &alarm);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, NULL, "calendar_record_create() Fail(%d)", ret);

	cursor = __crlf(cursor); /* crlf: BEGIN:VALARM */
	bool exit_loop = false;
	while (cursor) {
		int index = 0;
		cursor = __get_index(cursor, component_property_valarm, VCAL_COMPONENT_PROPERTY_VALARM_MAX, &index);

		char *value = NULL;
		switch (index) {
		case VCAL_COMPONENT_PROPERTY_VALARM_ACTION:
			cursor = __get_value(cursor, &value);
			__work_component_property_valarm_action(value, alarm, ud);
			free(value);
			value = NULL;
			break;

		case VCAL_COMPONENT_PROPERTY_VALARM_TRIGGER:
			cursor = __get_value(cursor, &value);
			__work_component_property_valarm_trigger(value, record, alarm, ud);
			free(value);
			value = NULL;
			break;

		case VCAL_COMPONENT_PROPERTY_VALARM_REPEAT:
			cursor = __get_value(cursor, &value);
			__work_component_property_valarm_repeat(value, alarm, ud);
			free(value);
			value = NULL;
			break;

		case VCAL_COMPONENT_PROPERTY_VALARM_ATTACH:
			cursor = __get_value(cursor, &value);
			__work_component_property_valarm_attach(value, alarm, ud);
			free(value);
			value = NULL;
			break;

		case VCAL_COMPONENT_PROPERTY_VALARM_DESCRIPTION:
			cursor = __get_value(cursor, &value);
			__work_component_property_valarm_description(value, alarm, ud);
			free(value);
			value = NULL;
			break;

		case VCAL_COMPONENT_PROPERTY_VALARM_SUMMARY:
			cursor = __get_value(cursor, &value);
			__work_component_property_valarm_summary(value, alarm, ud);
			free(value);
			value = NULL;
			break;

		case VCAL_COMPONENT_PROPERTY_VALARM_DURATION:
			cursor = __get_value(cursor, &value);
			__work_component_property_valarm_duration(value, alarm, ud);
			free(value);
			value = NULL;
			break;

		case VCAL_COMPONENT_PROPERTY_VALARM_END:
			DBG("exit valarm");
			exit_loop = true;
			break;

		default:
			ERR("Invalid index(%d)", index);
			cursor = __crlf(cursor);
			break;
		}

		if (true == exit_loop) break;
	}

	switch (ud->type) {
	case VCALENDAR_TYPE_VEVENT:
		ret = calendar_record_add_child_record(record, _calendar_event.calendar_alarm, alarm);
		break;
	case VCALENDAR_TYPE_VTODO:
		ret = calendar_record_add_child_record(record, _calendar_todo.calendar_alarm, alarm);
		break;
	}
	RETVM_IF(CALENDAR_ERROR_NONE != ret, NULL, "calendar_record_add_child_record() Fail(%d)", ret);
	return cursor;
}

static char* __work_component_vevent(char *cursor, calendar_record_h record, struct user_data *ud)
{
	RETV_IF(NULL == cursor, NULL);
	RETV_IF(NULL == record, NULL);

	bool exit_loop = false;
	while (cursor) {
		int index = 0;
		cursor = __get_index(cursor, component_property, VCAL_COMPONENT_PROPERTY_MAX, &index);

		char *value = NULL;
		switch (index) {
		case VCAL_COMPONENT_PROPERTY_DTSTAMP:
			cursor = __get_value(cursor, &value);
			__work_component_property_dtstamp(value, record, ud);
			free(value);
			value = NULL;
			break;
		case VCAL_COMPONENT_PROPERTY_UID:
			cursor = __get_value(cursor, &value);
			__work_component_property_uid(value, record, ud);
			free(value);
			value = NULL;
			break;
		case VCAL_COMPONENT_PROPERTY_RECURRENCE_ID:
			cursor = __get_value(cursor, &value);
			__work_component_property_recurrence_id(value, record, ud);
			free(value);
			value = NULL;
			break;
		case VCAL_COMPONENT_PROPERTY_DTSTART:
			cursor = __get_value(cursor, &value);
			__work_component_property_dtstart(value, record, ud);
			free(value);
			value = NULL;
			break;
		case VCAL_COMPONENT_PROPERTY_CREATED:
			cursor = __get_value(cursor, &value);
			__work_component_property_created(value, record, ud);
			free(value);
			value = NULL;
			break;
		case VCAL_COMPONENT_PROPERTY_DCREATED:
			cursor = __get_value(cursor, &value);
			__work_component_property_created(value, record, ud);
			free(value);
			value = NULL;
			break;
		case VCAL_COMPONENT_PROPERTY_DESCRIPTION:
			cursor = __get_value(cursor, &value);
			__work_component_property_description(value, record, ud);
			free(value);
			value = NULL;
			break;
		case VCAL_COMPONENT_PROPERTY_LAST_MODIFIED:
			cursor = __get_value(cursor, &value);
			__work_component_property_last_modified(value, record, ud);
			free(value);
			value = NULL;
			break;
		case VCAL_COMPONENT_PROPERTY_LOCATION:
			cursor = __get_value(cursor, &value);
			__work_component_property_location(value, record, ud);
			free(value);
			value = NULL;
			break;
		case VCAL_COMPONENT_PROPERTY_PRIORITY:
			cursor = __get_value(cursor, &value);
			__work_component_property_priority(value + 1, record, ud);
			free(value);
			value = NULL;
			break;
		case VCAL_COMPONENT_PROPERTY_STATUS:
			cursor = __get_value(cursor, &value);
			__work_component_property_status(value, record, ud);
			free(value);
			value = NULL;
			break;
		case VCAL_COMPONENT_PROPERTY_SUMMARY:
			cursor = __get_value(cursor, &value);
			__work_component_property_summary(value, record, ud);
			free(value);
			value = NULL;
			break;
		case VCAL_COMPONENT_PROPERTY_RRULE:
			cursor = __get_value(cursor, &value);
			__work_component_property_rrule(value, record, ud);
			free(value);
			value = NULL;
			break;
		case VCAL_COMPONENT_PROPERTY_DTEND:
			if (CAL_STRING_EQUAL != strcmp(((cal_record_s*)(record))->view_uri, _calendar_event._uri))
				break;
			cursor = __get_value(cursor, &value);
			__work_component_property_dtend(value, record, ud);
			free(value);
			value = NULL;
			break;
		case VCAL_COMPONENT_PROPERTY_DUE:
			if (CAL_STRING_EQUAL != strcmp(((cal_record_s*)(record))->view_uri, _calendar_todo._uri))
				break;
			cursor = __get_value(cursor, &value);
			__work_component_property_dtend(value, record, ud);
			free(value);
			value = NULL;
			break;
		case VCAL_COMPONENT_PROPERTY_ATTENDEE:
			cursor = __get_value(cursor, &value);
			__work_component_property_attendee(value, record, ud);
			free(value);
			value = NULL;
			break;
		case VCAL_COMPONENT_PROPERTY_CATEGORIES:
			cursor = __get_value(cursor, &value);
			__work_component_property_categories(value, record, ud);
			free(value);
			value = NULL;
			break;
		case VCAL_COMPONENT_PROPERTY_DALARM:
			cursor = __get_value(cursor, &value);
			__work_component_property_dalarm(value, record, ud);
			free(value);
			value = NULL;
			break;
		case VCAL_COMPONENT_PROPERTY_MALARM:
			cursor = __get_value(cursor, &value);
			__work_component_property_malarm(value, record, ud);
			free(value);
			value = NULL;
			break;
		case VCAL_COMPONENT_PROPERTY_AALARM:
			cursor = __get_value(cursor, &value);
			__work_component_property_aalarm(value, record, ud);
			free(value);
			value = NULL;
			break;
		case VCAL_COMPONENT_PROPERTY_EXDATE:
			cursor = __get_value(cursor, &value);
			__work_component_property_exdate(value, record, ud);
			free(value);
			value = NULL;
			break;
		case VCAL_COMPONENT_PROPERTY_X_ALLDAY:
			cursor = __get_value(cursor, &value);
			__work_component_property_x_allday(value, record, ud);
			free(value);
			value = NULL;
			break;
		case VCAL_COMPONENT_PROPERTY_X_LUNAR:
			cursor = __get_value(cursor, &value);
			__work_component_property_x_lunar(value, record, ud);
			free(value);
			value = NULL;
			break;
		case VCAL_COMPONENT_PROPERTY_BEGIN:
			cursor = __work_component_property_begin(cursor, record, ud);
			if (NULL == cursor)
				cursor = __crlf(cursor);
			break;
		case VCAL_COMPONENT_PROPERTY_END:
			DBG("exit record");
			cursor = __crlf(cursor);
			exit_loop = true;
			break;
		case VCAL_COMPONENT_PROPERTY_EXTENDED:
			cursor = __get_value(cursor, &value);
			free(value);
			value = NULL;
			break;
		default:
			cursor = __crlf(cursor);
			break;
		}
		if (true == exit_loop) break;
	}
	return cursor;
}

static char* __work_component_vjournal(char *cursor, calendar_record_h record, struct user_data *ud)
{
	RETV_IF(NULL == cursor, NULL);

	DBG("Not supported vjournal");

	bool exit_loop = false;
	while (cursor) {
		int index = 0;
		cursor = __get_index(cursor, component_property, VCAL_COMPONENT_PROPERTY_MAX, &index);
		switch (index) {
		case VCAL_COMPONENT_PROPERTY_END:
			DBG("exit record");
			cursor = __crlf(cursor);
			exit_loop = true;
			break;

		default:
			cursor = __crlf(cursor);
			break;
		}
		if (true == exit_loop) break;
	}
	return cursor;
}

static char* __work_component_vfreebusy(char *cursor, calendar_record_h record, struct user_data *ud)
{
	RETV_IF(NULL == cursor, NULL);

	DBG("Not supported vfreebusy");

	bool exit_loop = false;
	while (cursor) {
		int index = 0;
		cursor = __get_index(cursor, component_property, VCAL_COMPONENT_PROPERTY_MAX, &index);
		switch (index) {
		case VCAL_COMPONENT_PROPERTY_END:
			DBG("exit record");
			cursor = __crlf(cursor);
			exit_loop = true;
			break;

		default:
			cursor = __crlf(cursor);
			break;
		}
		if (true == exit_loop) break;
	}
	return cursor;
}

static void __work_component_property_vtimezone_standard_dtstart(char *value, calendar_record_h record, struct user_data *ud)
{
	CAL_FN_CALL();

	RET_IF(NULL == value);
	RET_IF('\0' == *value);
	RET_IF(NULL == record);

	int ret = 0;
	int y = 0, m = 0, d = 0;
	int h = 0, n = 0, s = 0;
	sscanf(value +1, CAL_DATETIME_FORMAT_YYYYMMDDTHHMMSS, &y, &m, &d, &h, &n, &s);
	ret = cal_record_set_int(record, _calendar_timezone.standard_start_month, m);
	WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_int() Fail(%d)", ret);
	ret = cal_record_set_int(record, _calendar_timezone.standard_start_hour, h);
	WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_int() Fail(%d)", ret);

	long long int t = cal_time_convert_lli(value +1);
	int nth = 0, wday = 0;
	cal_time_get_nth_wday(t, &nth, &wday);
	ret = cal_record_set_int(record, _calendar_timezone.standard_start_position_of_week, nth);
	WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_int() Fail(%d)", ret);
	ret = cal_record_set_int(record, _calendar_timezone.standard_start_day, wday);
	WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_int() Fail(%d)", ret);
}
static void __work_component_property_vtimezone_standard_tzoffsetfrom(char *value, calendar_record_h record, struct user_data *ud)
{
	return;
}
static void __work_component_property_vtimezone_standard_tzoffsetto(char *value, calendar_record_h record, struct user_data *ud)
{
	CAL_FN_CALL();

	RET_IF(NULL == value);
	RET_IF('\0' == *value);
	RET_IF(NULL == record);

	char c;
	int h = 0, m = 0;
	sscanf(value, "%c%02d%02d", &c, &h, &m);

	int offset = h * 60 + m;
	if ('-' == c) offset *= -1;

	int ret = 0;
	ret = cal_record_set_int(record, _calendar_timezone.standard_bias, offset);
	WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_int() Fail(%d)", ret);
	ret = cal_record_set_int(record, _calendar_timezone.tz_offset_from_gmt, offset);
	WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_int() Fail(%d)", ret);

	if (NULL == ud->timezone_tzid || '\0' == *ud->timezone_tzid) {
		char buf[CAL_STR_SHORT_LEN32] = {0};
		snprintf(buf, sizeof(buf), "Etc/GMT%c%d", offset < 0 ? '+' : '-', h);
		ud->timezone_tzid = strdup(buf);
		__adjust_tzid(ud->timezone_tzid);
		DBG("timezone_tzid[%s]", ud->timezone_tzid);
	}
}
static void __work_component_property_vtimezone_standard_tzname(char *value, calendar_record_h record, struct user_data *ud)
{
	RET_IF(NULL == value);
	RET_IF('\0' == *value);
	RET_IF(NULL == record);

	int ret = cal_record_set_str(record, _calendar_timezone.standard_name, value);
	WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_str() Fail(%d)", ret);
}
static void __work_component_property_vtimezone_standard_rdate(char *value, calendar_record_h record, struct user_data *ud)
{
	return;
}
static char* __work_component_vtimezone_standard(char *cursor, calendar_record_h record, struct user_data *ud)
{
	CAL_FN_CALL();

	RETV_IF(NULL == cursor, __crlf(cursor));
	RETV_IF('\0' == *cursor, __crlf(cursor));
	RETV_IF(NULL == record, __crlf(cursor));

	cursor = __crlf(cursor);
	bool exit_loop = false;
	while (cursor) {
		int index = 0;
		cursor = __get_index(cursor, component_property_vtimezone, VCAL_COMPONENT_PROPERTY_VTIMEZONE_MAX, &index);

		char *value = NULL;
		switch (index) {
		case VCAL_COMPONENT_PROPERTY_VTIMEZONE_DTSTART:
			cursor = __get_value(cursor, &value);
			__work_component_property_vtimezone_standard_dtstart(value +1, record, ud);
			free(value);
			value = NULL;
			break;
		case VCAL_COMPONENT_PROPERTY_VTIMEZONE_TZOFFSETFROM:
			cursor = __get_value(cursor, &value);
			__work_component_property_vtimezone_standard_tzoffsetfrom(value, record, ud);
			free(value);
			value = NULL;
			break;
		case VCAL_COMPONENT_PROPERTY_VTIMEZONE_TZOFFSETTO:
			cursor = __get_value(cursor, &value);
			__work_component_property_vtimezone_standard_tzoffsetto(value +1, record, ud);
			free(value);
			value = NULL;
			break;
		case VCAL_COMPONENT_PROPERTY_VTIMEZONE_TZNAME:
			cursor = __get_value(cursor, &value);
			__work_component_property_vtimezone_standard_tzname(value +1, record, ud);
			free(value);
			value = NULL;
			break;
		case VCAL_COMPONENT_PROPERTY_VTIMEZONE_RDATE:
			cursor = __get_value(cursor, &value);
			__work_component_property_vtimezone_standard_rdate(value, record, ud);
			free(value);
			value = NULL;
			break;
		case VCAL_COMPONENT_PROPERTY_VTIMEZONE_END:
			cursor = __crlf(cursor);
			exit_loop = true;
			break;
		default:
			cursor = __crlf(cursor);
			break;
		}
		if (true == exit_loop) break;
	}
	return cursor;
}

static void __work_component_property_vtimezone_daylight_dtstart(char *value, calendar_record_h record, struct user_data *ud)
{
	RET_IF(NULL == value);
	RET_IF('\0' == *value);
	RET_IF(NULL == record);

	int ret = 0;
	int y = 0, m = 0, d = 0;
	int h = 0, n = 0, s = 0;
	sscanf(value +1, CAL_DATETIME_FORMAT_YYYYMMDDTHHMMSS, &y, &m, &d, &h, &n, &s);
	ret = cal_record_set_int(record, _calendar_timezone.day_light_start_month, m);
	WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_int() Fail(%d)", ret);
	ret = cal_record_set_int(record, _calendar_timezone.day_light_start_hour, h);
	WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_int() Fail(%d)", ret);

	long long int t = cal_time_convert_lli(value +1);
	int nth = 0, wday = 0;
	cal_time_get_nth_wday(t, &nth, &wday);
	ret = cal_record_set_int(record, _calendar_timezone.day_light_start_position_of_week, nth);
	WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_int() Fail(%d)", ret);
	ret = cal_record_set_int(record, _calendar_timezone.day_light_start_day, wday);
	WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_int() Fail(%d)", ret);
}
static void __work_component_property_vtimezone_daylight_tzoffsetfrom(char *value, calendar_record_h record, struct user_data *ud)
{
	return;
}
static void __work_component_property_vtimezone_daylight_tzoffsetto(char *value, calendar_record_h record, struct user_data *ud)
{
	RET_IF(NULL == value);
	RET_IF('\0' == *value);
	RET_IF(NULL == record);

	char c;
	int h = 0, m = 0;
	sscanf(value, "%c%02d%02d", &c, &h, &m);

	int offset = h * 60 + m;
	if ('-' == c) offset *= -1;

	int ret = cal_record_set_int(record, _calendar_timezone.day_light_bias, offset);
	WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_int() Fail(%d)", ret);
}
static void __work_component_property_vtimezone_daylight_tzname(char *value, calendar_record_h record, struct user_data *ud)
{
	RET_IF(NULL == value);
	RET_IF('\0' == *value);
	RET_IF(NULL == record);

	int ret = cal_record_set_str(record, _calendar_timezone.day_light_name, value);
	WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_record_set_str() Fail(%d)", ret);
}
static void __work_component_property_vtimezone_daylight_rdate(char *value, calendar_record_h record, struct user_data *ud)
{
	return;
}
static char* __work_component_vtimezone_daylight(char *cursor, calendar_record_h record, struct user_data *ud)
{
	RETV_IF(NULL == cursor, __crlf(cursor));
	RETV_IF('\0' == *cursor, __crlf(cursor));
	RETV_IF(NULL == record, __crlf(cursor));

	cursor = __crlf(cursor);
	bool exit_loop = false;
	while (cursor) {
		int index = 0;
		cursor = __get_index(cursor, component_property_vtimezone, VCAL_COMPONENT_PROPERTY_VTIMEZONE_MAX, &index);

		char *value = NULL;
		switch (index) {
		case VCAL_COMPONENT_PROPERTY_VTIMEZONE_DTSTART:
			cursor = __get_value(cursor, &value);
			__work_component_property_vtimezone_daylight_dtstart(value +1, record, ud);
			free(value);
			value = NULL;
			break;
		case VCAL_COMPONENT_PROPERTY_VTIMEZONE_TZOFFSETFROM:
			cursor = __get_value(cursor, &value);
			__work_component_property_vtimezone_daylight_tzoffsetfrom(value, record, ud);
			free(value);
			value = NULL;
			break;
		case VCAL_COMPONENT_PROPERTY_VTIMEZONE_TZOFFSETTO:
			cursor = __get_value(cursor, &value);
			__work_component_property_vtimezone_daylight_tzoffsetto(value +1, record, ud);
			free(value);
			value = NULL;
			break;
		case VCAL_COMPONENT_PROPERTY_VTIMEZONE_TZNAME:
			cursor = __get_value(cursor, &value);
			__work_component_property_vtimezone_daylight_tzname(value +1, record, ud);
			free(value);
			value = NULL;
			break;
		case VCAL_COMPONENT_PROPERTY_VTIMEZONE_RDATE:
			cursor = __get_value(cursor, &value);
			__work_component_property_vtimezone_daylight_rdate(value, record, ud);
			free(value);
			value = NULL;
			break;
		case VCAL_COMPONENT_PROPERTY_VTIMEZONE_END:
			cursor = __crlf(cursor);
			exit_loop = true;
			break;
		default:
			cursor = __crlf(cursor);
			break;
		}
		if (true == exit_loop) break;
	}
	return cursor;
}

static char* __work_component_vtimezone(char *cursor, calendar_record_h record, struct user_data *ud)
{
	CAL_FN_CALL();

	RETV_IF(NULL == cursor, NULL);
	RETV_IF(NULL == record, NULL);

	__init_component_property_vtimezone();

	while (cursor) {
		if (CAL_STRING_EQUAL == strncmp(cursor, "TZID:", strlen("TZID:"))) {
			char *p = cursor + strlen("TZID");
			if (NULL == p || '\0' == *p) {
				ERR("Inavlid tzid");
				cursor = __crlf(cursor);
				continue;
			}
			if (ud->timezone_tzid) {
				free(ud->timezone_tzid);
				ud->timezone_tzid = NULL;
			}
			char *value = NULL;
			cursor = __get_value(p, &value);
			__adjust_tzid(value);
			DBG("tzid[%s]", value +1);
			if (true == cal_time_is_available_tzid(value +1)) {
				ud->timezone_tzid = strdup(value +1);
			}
			else {
				DBG("Invalid tzid string[%s]", value +1);
			}
			free(value);
		}
		else if (CAL_STRING_EQUAL == strncmp(cursor, "BEGIN:STANDARD", strlen("BEGIN:STANDARD"))) {
			cursor = __work_component_vtimezone_standard(cursor, record, ud);
		}
		else if (CAL_STRING_EQUAL == strncmp(cursor, "BEGIN:DAYLIGHT", strlen("BEGIN:DAYLIGHT"))) {
			cursor = __work_component_vtimezone_daylight(cursor, record, ud);
		}
		else if (CAL_STRING_EQUAL == strncmp(cursor, "END", strlen("END"))) {
			cursor = __crlf(cursor);
			break;
		}
		else {
			DBG("Unable to parse");
			__print_cursor(cursor, __LINE__);
			cursor = __crlf(cursor);
		}
	}
	return cursor;
}

static char* __work_property_begin(char *cursor, calendar_record_h *out_record, struct user_data *ud)
{
	CAL_FN_CALL();
	RETV_IF(NULL == cursor, NULL);
	RETV_IF('\0' == *cursor, NULL);
	RETV_IF(NULL == out_record, NULL);
	RETV_IF(NULL == ud, NULL);

	int ret = 0;
	int index = 0;
	cursor = __get_index(cursor +1, vcal_component, VCAL_COMPONENT_MAX, &index);
	cursor = __crlf(cursor);
	calendar_record_h record = NULL;
	switch (index) {
	case VCAL_COMPONENT_VEVENT:
		ret = calendar_record_create(_calendar_event._uri, &record);
		RETVM_IF(CALENDAR_ERROR_NONE != ret, NULL, "calendar_record_create() Fail(%d)", ret);
		ud->type = CALENDAR_BOOK_TYPE_EVENT;
		ud->has_rrule = __check_has_rrule(cursor);
		cursor = __work_component_vevent(cursor, record, ud);
		break;

	case VCAL_COMPONENT_VTODO:
		ret = calendar_record_create(_calendar_todo._uri, &record);
		RETVM_IF(CALENDAR_ERROR_NONE != ret, NULL, "calendar_record_create() Fail(%d)", ret);
		ud->type = CALENDAR_BOOK_TYPE_TODO;
		ud->has_rrule = __check_has_rrule(cursor);
		cursor = __work_component_vevent(cursor, record, ud); /* same as event */
		break;

	case VCAL_COMPONENT_VJOURNAL:
		cursor = __work_component_vjournal(cursor, record, ud);
		break;

	case VCAL_COMPONENT_VFREEBUSY:
		cursor = __work_component_vfreebusy(cursor, record, ud);
		break;

	case VCAL_COMPONENT_VTIMEZONE:
		ret = calendar_record_create(_calendar_timezone._uri, &record);
		RETVM_IF(CALENDAR_ERROR_NONE != ret, NULL, "calendar_record_create() Fail(%d)", ret);
		cursor = __work_component_vtimezone(cursor, record, ud);
		break;
	}
	*out_record = record;
	return cursor;
}

int cal_vcalendar_parse_vcalendar_object(char *stream, calendar_list_h list, vcalendar_foreach_s *foreach_data)
{
	CAL_FN_CALL();

	RETV_IF(NULL == stream, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == list, CALENDAR_ERROR_INVALID_PARAMETER);

	__init_component();
	__init_property();
	__init_component_property();
	__unfolding(stream);

	struct user_data *ud = calloc(1, sizeof(struct user_data));
	RETVM_IF(NULL == ud, CALENDAR_ERROR_OUT_OF_MEMORY, "calloc() Fail");
	ud->version = VCAL_VER_2; /* default */

	calendar_record_h record = NULL;

	int count = 0;
	bool exit_loop = false;
	char *cursor = (char *)stream;
	while (cursor) {
		int index = 0;
		char *value = NULL;
		cursor = __get_index(cursor, vcal_property, VCAL_PROPERTY_MAX, &index);
		switch (index) {
		case VCAL_PROPERTY_VERSION:
			cursor = __get_value(cursor, &value);
			__get_version(value, &ud->version);
			free(value);
			value = NULL;
			break;

		case VCAL_PROPERTY_TZ:
			if (ud->timezone_tzid)
				break;
			cursor = __get_value(cursor, &value);
			__get_tz(value + 1, &ud->timezone_tzid);
			__adjust_tzid(ud->timezone_tzid);
			DBG("timezone_tzid[%s]", ud->timezone_tzid);
			free(value);
			value = NULL;
			break;

		case VCAL_PROPERTY_BEGIN: /* BEGIN:VEVENT */
			cursor = __work_property_begin(cursor, &record, ud);
			calendar_list_add(list, record);
			count++;
			if (foreach_data) {
				foreach_data->ret = foreach_data->callback(record, foreach_data->user_data);
				if (false == foreach_data->ret)
					exit_loop = true;
			}
			break;

		case VCAL_PROPERTY_END: /* END:VCALENDAR */
			DBG("exit VCALENDAR");
			/* fini vcalendar */
			exit_loop = true;
			break;

		default:
			DBG("skip invalid property, index(%d)", index);
			cursor = __crlf(cursor);
			break;
		}
		if (true == exit_loop)
			break;
	}

	DBG("count(%d)", count);
	if (0 == count)
		DBG("No record");
	free(ud->timezone_tzid);
	free(ud);

	return CALENDAR_ERROR_NONE;
}
