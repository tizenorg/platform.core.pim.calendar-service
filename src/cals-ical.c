/*
 * Calendar Service
 *
 * Copyright (c) 2000 - 2012 Samsung Electronics Co., Ltd. All rights reserved.
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
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <glib.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>

#include "cals-typedef.h"
#include "cals-ical.h"
#include "cals-utils.h"
#include "cals-db.h"
#include "cals-internal.h"
#include "cals-schedule.h"
#include "cals-struct.h"
#include "cals-utils.h"
#include "cals-time.h"

#define ICALENAR_BUFFER_MAX (1024*1024)


enum {
	ENCODE_NONE = 0x0,
	ENCODE_BASE64,
	ENCODE_QUOTED_PRINTABLE,
	ENCODE_MAX,
};


struct _comp_func {
	char *prop;
	char *(*func)(GList **list_sch, void *data);
};

struct _prop_func {
	char *prop;
	int (*func)(GList **list_sch, void *data);
};

struct _veve_func {
	char *prop;
	int (*func)(int ver, cal_sch_full_t *sch, void *data);
};

struct _vala_func {
	char *prop;
	int (*func)(cal_sch_full_t *sch, void *data);
};

struct _rrule_func {
	char *prop;
	int (*func)(cal_sch_full_t *sch, void *data);
};


struct _val_func {
	char *prop;
	int (*func)(int *val, void *data);
};


char *cals_func_vcalendar(GList **list_sch, void *data);
char *cals_func_vevent(int ver, GList **list_sch, void *data);
char *cals_func_vtodo(int ver, GList **list_sch, void *data);
char *cals_func_valarm(cal_sch_full_t *sch, void *data);

enum {
	VCAL_PRODID = 0x0,
	VCAL_VERSION,
//	VCAL_CALSCALE,
//	VCAL_METHOD,
	VCAL_MAX,
};

int cals_func_prodid(GList **list_sch, void *data);
int cals_func_version(GList **list_sch, void *data);

struct _prop_func _vcal_list[VCAL_MAX] =
{
	{"PRODID", cals_func_prodid },
	{"VERSION", cals_func_version }//,
//	{"CALSCALE", cals_func_calscale },
//	{"METHOD", cals_func_method }
};

int cals_func_dtstamp(int ver, cal_sch_full_t *sch, void *data);
int cals_func_dtstart(int ver, cal_sch_full_t *sch, void *data);
int cals_func_uid(int ver, cal_sch_full_t *sch, void *data);
int cals_func_class(int ver, cal_sch_full_t *sch, void *data);
int cals_func_created(int ver, cal_sch_full_t *sch, void *data);
int cals_func_description(int ver, cal_sch_full_t *sch, void *data);
int cals_func_geo(int ver, cal_sch_full_t *sch, void *data);
int cals_func_last_mod(int ver, cal_sch_full_t *sch, void *data);
int cals_func_location(int ver, cal_sch_full_t *sch, void *data);
int cals_func_organizer(int ver, cal_sch_full_t *sch, void *data);
int cals_func_priority(int ver, cal_sch_full_t *sch, void *data);
int cals_func_seq(int ver, cal_sch_full_t *sch, void *data);
int cals_func_status(int ver, cal_sch_full_t *sch, void *data);
int cals_func_summary(int ver, cal_sch_full_t *sch, void *data);
int cals_func_transp(int ver, cal_sch_full_t *sch, void *data);
int cals_func_url(int ver, cal_sch_full_t *sch, void *data);
int cals_func_recurid(int ver, cal_sch_full_t *sch, void *data);
int cals_func_rrule(int ver, cal_sch_full_t *sch, void *data);
int cals_func_dtend(int ver, cal_sch_full_t *sch, void *data);
int cals_func_duration(int ver, cal_sch_full_t *sch, void *data);
int cals_func_attach(int ver, cal_sch_full_t *sch, void *data);
int cals_func_attendee(int ver, cal_sch_full_t *sch, void *data);
int cals_func_categories(int ver, cal_sch_full_t *sch, void *data);
int cals_func_comment(int ver, cal_sch_full_t *sch, void *data);
int cals_func_contact(int ver, cal_sch_full_t *sch, void *data);
int cals_func_exdate(int ver, cal_sch_full_t *sch, void *data);
int cals_func_rstatus(int ver, cal_sch_full_t *sch, void *data);
int cals_func_related(int ver, cal_sch_full_t *sch, void *data);
int cals_func_resources(int ver, cal_sch_full_t *sch, void *data);
int cals_func_rdate(int ver, cal_sch_full_t *sch, void *data);

/* for vcalendar version 1.0 */
int cals_ver1_func_rrule(cal_sch_full_t *sch, void *data);

enum {
	VEVE_DTSTAMP = 0x0,
	VEVE_DTSTART,
//	VEVE_UID,
//	VEVE_CLASS,
	VEVE_CREATED,
	VEVE_DESCRIPTION,
//	VEVE_GEO,
	VEVE_LAST_MOD,
	VEVE_LOCATION,
//	VEVE_ORGANIZER,
	VEVE_PRIORITY,
//	VEVE_SEQ,
	VEVE_STATUS,
	VEVE_SUMMARY,
//	VEVE_TRANSP,
//	VEVE_URL,
//	VEVE_RECURID,
	VEVE_RRULE,
	VEVE_DTEND,
//	VEVE_DURATION,
//	VEVE_ATTACH,
//	VEVE_ATTENDEE,
	VEVE_CATEGORIES,
//	VEVE_COMMENT,
//	VEVE_CONTACT,
//	VEVE_EXDATE,
//	VEVE_RSTATUS,
//	VEVE_RELATED,
//	VEVE_RESOURCES,
//	VEVE_RDATE,
	VEVE_MAX,
};

struct _veve_func _veve_list[VEVE_MAX] =
{
	{ "DTSTAMP", cals_func_dtstamp },
	{ "DTSTART", cals_func_dtstart },
//	{ "UID", cals_func_uid },
//	{ "CLASS", cals_func_class },
	{ "CREATED", cals_func_created },
	{ "DESCRIPTION", cals_func_description },
//	{ "GEO", cals_func_geo },
	{ "LAST-MOD", cals_func_last_mod },
	{ "LOCATION", cals_func_location },
//	{ "ORGANIZER", cals_func_organizer },
	{ "PRIORITY", cals_func_priority },
//	{ "SEQ", cals_func_seq },
	{ "STATUS", cals_func_status },
	{ "SUMMARY", cals_func_summary },
//	{ "TRANSP", cals_func_transp },
//	{ "URL", cals_func_url },
//	{ "RECURID", cals_func_recurid },
	{ "RRULE", cals_func_rrule },
	{ "DTEND", cals_func_dtend },
//	{ "DURATION", cals_func_duration },
//	{ "ATTACH", cals_func_attach },
//	{ "ATTENDEE", cals_func_attendee },
	{ "CATEGORIES", cals_func_categories }//,
//	{ "COMMENT", cals_func_comment },
//	{ "CONTACT", cals_func_contact },
//	{ "EXDATE", cals_func_exdate },
//	{ "RSTATUS", cals_func_rstatus },
//	{ "RELATED", cals_func_related },
//	{ "RESOURCES", cals_func_resources },
//	{ "RDATE", cals_func_rdate }
};

int cals_func_action(cal_sch_full_t *sch, void *data);
int cals_func_trigger(cal_sch_full_t *sch, void *data);
int cals_func_repeat(cal_sch_full_t *sch, void *data);
int cals_func_duration_alarm(cal_sch_full_t *sch, void *data);
int cals_func_attach_alarm(cal_sch_full_t *sch, void *data);
int cals_func_summary_alarm(cal_sch_full_t *sch, void *data);

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

struct _vala_func _vala_list[VALA_MAX] =
{
	{ "ACTION", cals_func_action },
	{ "TRIGGER", cals_func_trigger },
	{ "REPEAT", cals_func_repeat },
	{ "DURATION", cals_func_duration_alarm },
	{ "ATTACH", cals_func_attach_alarm },
//	{ "DESCRIPTION", cals_func_description },
	{ "SUMMARY", cals_func_summary_alarm },
//	{ "ATTENDEE", cals_func_attendee },
};

int cals_func_freq(cal_sch_full_t *sch, void *data);
int cals_func_until(cal_sch_full_t *sch, void *data);
int cals_func_count(cal_sch_full_t *sch, void *data);
int cals_func_interval(cal_sch_full_t *sch, void *data);
int cals_func_bysecond(cal_sch_full_t *sch, void *data);
int cals_func_byminute(cal_sch_full_t *sch, void *data);
int cals_func_byhour(cal_sch_full_t *sch, void *data);
int cals_func_byday(cal_sch_full_t *sch, void *data);
int cals_func_bymonthday(cal_sch_full_t *sch, void *data);
int cals_func_byyearday(cal_sch_full_t *sch, void *data);
int cals_func_byweekno(cal_sch_full_t *sch, void *data);
int cals_func_bymonth(cal_sch_full_t *sch, void *data);
int cals_func_bysetpos(cal_sch_full_t *sch, void *data);
int cals_func_wkst(cal_sch_full_t *sch, void *data);

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

struct _rrule_func _rrule_list[RRULE_MAX] =
{
	{ "FREQ=", cals_func_freq },
	{ "UNTIL=", cals_func_until },
	{ "COUNT=", cals_func_count },
	{ "INTERVAL=", cals_func_interval },
	{ "BYSECOND=", cals_func_bysecond },
	{ "BYMINUTE=", cals_func_byminute },
	{ "BYHOUR=", cals_func_byhour },
	{ "BYDAY=", cals_func_byday },
	{ "BYMONTHDAY=", cals_func_bymonthday },
	{ "BYYEARDAY=", cals_func_byyearday },
	{ "BYWEEKNO=", cals_func_byweekno },
	{ "BYMONTH=", cals_func_bymonth },
	{ "BYSETPOS=", cals_func_bysetpos },
	{ "WKST=", cals_func_wkst }
};

int cals_func_related_trig(cal_sch_full_t *sch, void *data);
int cals_func_value(cal_sch_full_t *sch, void *data);

enum {
	TRIG_RELATED = 0x0,
	TRIG_VALUE,
	TRIG_MAX,
};

struct _vala_func _trig_list[TRIG_MAX] =
{
	{ "RELATED=", cals_func_related_trig },
	{ "VALUE=", cals_func_value }
};

int cals_func_charset(int *val, void *data);
int cals_func_encoding(int *val, void *data);


enum {
	TEXT_CHARSET = 0x0,
	TEXT_ENCODING,
	TEXT_MAX,
};

struct _val_func _optional_list[TEXT_MAX] =
{
	{ "CHARSET=", cals_func_charset },
	{ "ENCODING=", cals_func_encoding },
};

char *cals_convert_sec_from_duration(char *p, int *dur_t, char *dur);

//util //////////////////////////////////////////////////////////////////////


char *cals_skip_empty_line(char *src)
{
	CALS_FN_CALL;
	while (*src) {
		if ('\n' != *src && '\r' != *src) {
			break;
		}
		src++;
	}
	return src;
}

int cals_file_exist(const char *path)
{
	FILE *fp;

	fp = fopen(path, "r");

	if (fp == NULL) {
		ERR("Failed to access path(%s)\n", path);
		return -1;
	}
	fclose(fp);
	return 0;
}

char *cals_check_word(char *src, const char *word)
{
	int start = 0;

	src = cals_skip_empty_line(src);

	while (*src) {
		switch (*src) {
		case ' ':
		case ':':
		case ';':
			src++;
			break;
		default:
			start = 1;
			break;
		}
		if (start) break;
	}

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

char cals_2hexa_to_1char(char *p)
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

int cals_quoted_printable_decode(char *p, int *len)
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
					ch = cals_2hexa_to_1char(&p[i+1]);
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
	return 0;
}

////////////////////////////////////////////////////////////////////////


cal_sch_full_t *cals_add_vcalendar(void)
{
	cal_sch_full_t *sch;

	sch = calloc(1, sizeof(cal_sch_full_t));
	if (sch == NULL) {
		return NULL;
	}
	return sch;
}


int cals_unfold_content(char *stream)
{
	CALS_FN_CALL;
	char *p;

	retv_if(stream == NULL, CAL_ERR_ARG_NULL);

	p = stream;
	while (*stream) {
		if ('\n' == *stream && ' ' == *(stream + 1)) {
			stream += 2;
			p--;
		} else if ('\0' == *stream) {
			CALS_DBG("break\n");
			break;
		}
		*p = *stream;
		p++;
		stream++;

	}
	return 0;
}


char *cals_read_line(char *stream, char **prop, char **cont)
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
		return NULL;
	}

	i = 0;
	out = 0;
	q = p;

	while (*p) {
		switch (*p) {
		case '\r':
			if ('\n' == *(p + 1)) {
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
		p += 2;
	} else {
		*cont = NULL;
		return NULL;
	}

	DBG("%s][%s\n", *prop, *cont);
	return p;
}

inline void cals_get_optional_parameter(char *p, int *encoding)
{
	int i;

	for (i = 0; i < TEXT_MAX; i++) {
		if (!strncmp(p, _optional_list[i].prop, strlen(_optional_list[i].prop))) {
			int j = 0;
			char buf[64] = {0, };
			p += strlen(_optional_list[i].prop);
			while (p[j] != ':' && p[j] != ';' && p[j] != '\n' && p[j] != '\0') {
				buf[j] = p[j];
				j++;
			}
			if (p[j] != '\0') {
				buf[j] = '\0';
			}

			p += j;
			_optional_list[i].func(encoding, buf);
			break;
		} else {

		}
	}
}

API int calendar_svc_read_schedules(const char *stream, GList **schedules)
{
	CALS_FN_CALL;
	int ret = 0;
	char *prop, *cont;
	char *cursor;
	GList *l = NULL;

	cursor = (char *)stream;

	retv_if(stream == NULL, CAL_ERR_ARG_INVALID);
	cursor = cals_skip_empty_line(cursor);
	retvm_if(cursor == NULL, CAL_ERR_FAIL, "Failed to skip empty line\n");
	cals_unfold_content(cursor);

	cursor = cals_read_line(cursor, &prop, &cont);
	retvm_if(cursor == NULL, CAL_ERR_FAIL, "Failed to read line\n");

	if (strncmp(prop, "BEGIN", strlen("BEGIN")) ||
			strncmp(cont + 1, "VCALENDAR", strlen("VCALENDAR"))) {
		ERR("Failed to find BEGIN:VCALDENDAR [%s][%s]", prop, cont);
		free(prop);
		free(cont);
		return CAL_ERR_FAIL;
	}
	free(prop);
	free(cont);

	cursor = cals_func_vcalendar(&l, cursor);

	if (l == NULL) {
		ERR("No schedules");
		return CAL_ERR_NO_DATA;
	}

	*schedules = l;
	return ret;
}

int cals_do_importing(int calendar_id, char *stream, void *data)
{
	int ret;
	GList *list_sch = NULL;
	GList *l;
	cal_struct *cs;
	cal_sch_full_t *sch;

	ret = calendar_svc_read_schedules(stream, &list_sch);
	retvm_if(ret < 0, ret, "Failed to parse(%d)", ret);

	/* calendar id validation check */

	/* insert schedules to db */
	l = list_sch;
	while (l) {
		sch = (cal_sch_full_t *)((cal_struct *)l->data)->user_data;
		if (sch) {
			sch->calendar_id = calendar_id;
			ret = cals_insert_schedule(sch);
		}

		l = g_list_next(l);
	}

	/* free schedules in memory */
	l = list_sch;
	while (l) {
		cs = (cal_struct *)l->data;
		if (cs == NULL) {
			ERR("Not cal struct\n");
			break;
		}
		calendar_svc_struct_free(&cs);
		l = g_list_next(l);
	}
	g_list_free(list_sch);

	return ret;
}

static const char* cals_get_stream_from_path(const char *path)
{
	FILE *file;
	int buf_size, len;
	char *stream;
	char buf[1024];

	file = fopen(path, "r");

	len = 0;
	buf_size = ICALENAR_BUFFER_MAX;
	stream = malloc(ICALENAR_BUFFER_MAX);

	while (fgets(buf, sizeof(buf), file)) {
		if (len + sizeof(buf) < buf_size) {
			len += snprintf(stream + len, strlen(buf) +1, "%s", buf);

		} else {
			char *new_stream;
			buf_size *= 2;
			new_stream = realloc(stream, buf_size);
			if (new_stream) {
				stream = new_stream;
			} else {
				free(stream);
				fclose(file);
				ERR("out of memory\n");
				return NULL;
			}
			len += snprintf(stream + len, strlen(buf) +1, "%s", buf);
		}
	}
	fclose(file);
	return stream;
}

int calendar_svc_read_schedules_from_file(const char *path, GList **schedules)
{
	const char *stream;
	int ret;

	ret = cals_file_exist(path);
	if (ret < 0) {
		ERR("Failed to access path(%s)\n", path);
		return -1;
	}

	stream = cals_get_stream_from_path(path);
	retvm_if(stream == NULL, -1, "Failed to get stream from path(%s)", path);

	ret = calendar_svc_read_schedules(stream, schedules);
	if (ret < 0) {
		ERR("Failed to parse ics\n");
		if (stream) {
			free((void *)stream);
		}
		return -1;
	}

	if (stream) {
		free((void *)stream);
	}
	return 0;
}

int cals_import_schedules(const char *path, int calendar_id)
{
	FILE *file;
	int buf_size, len;
	char *stream;
	char buf[1024];

	file = fopen(path, "r");

	len = 0;
	buf_size = ICALENAR_BUFFER_MAX;
	stream = malloc(ICALENAR_BUFFER_MAX);

	while (fgets(buf, sizeof(buf), file)) {
		if (len + sizeof(buf) < buf_size) {
			len += snprintf(stream + len, strlen(buf) +1, "%s", buf);

		} else {
			char *new_stream;
			buf_size *= 2;
			new_stream = realloc(stream, buf_size);
			if (new_stream) {
				stream = new_stream;
			} else {
				if (stream) free(stream);
				fclose(file);
				ERR("out of memory");
				return CAL_ERR_OUT_OF_MEMORY;
			}
			len += snprintf(stream + len, strlen(buf) +1, "%s", buf);
		}

		if (!strncmp(buf, "END:VCALENDAR", strlen("END:VCALENDAR"))){
			DBG("end vcalendar");

			if (cals_do_importing(calendar_id, stream, NULL)) {
				if (stream) free(stream);
				fclose(file);
				return 0;
			}
			len = 0;
		}
	}
	if (stream) free(stream);
	fclose(file);
	return CAL_SUCCESS;
}

API int calendar_svc_calendar_import(const char *path, int calendar_id)
{
	int ret;
	retv_if(path == NULL, CAL_ERR_ARG_INVALID);

	ret = cals_file_exist(path);
	if (ret < 0) {
		ERR("Failed to access (%s)\n", path);
		return -1;
	}

	ret = cals_import_schedules(path, calendar_id);
	return ret;
}



// func ////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

char *cals_func_vcalendar(GList **list_sch, void *data)
{
	CALS_FN_CALL;
	int ver = -1;
	char *prop, *cont;
	char *cursor = (char *)data;
	char *p = cursor;

	/* do until meet BEGIN */
	while ((cursor = cals_read_line(cursor, &prop, &cont))) {
		if (!strncmp(prop, "BEGIN", strlen("BEGIN"))) {
			if (prop) free(prop);
			if (cont) free(cont);

			break;

		}

		if (!strncmp(prop, "PRODID", strlen("PRODID"))) {
			_vcal_list[VCAL_PRODID].func(list_sch, cont);

		} else if (!strncmp(prop, "VERSION", strlen("VERSION"))) {
			ver = _vcal_list[VCAL_VERSION].func(list_sch, cont);

		} else {

		}

		if (prop) {
			free(prop);
			prop = NULL;
		}
		if (cont) {
			free(cont);
			cont = NULL;
		}

		p = cursor;
	}

	cursor = p;
	while ((cursor = cals_read_line(cursor, &prop, &cont))) {
		if (!strncmp(prop, "BEGIN", strlen("BEGIN"))) {

			if (!strncmp(cont + 1, "VEVENT", strlen("VEVENT"))) {
				cursor = cals_func_vevent(ver, list_sch, cursor);

			} else if (!strncmp(cont + 1, "VTODO", strlen("VTODO"))) {
				cursor = cals_func_vtodo(ver, list_sch, cursor);
/*
			} else if (!strncmp(cont + 1, "VFREEBUSY", strlen("VFREEBUSY"))) {
			} else if (!strncmp(cont + 1, "VTIMEZONE", strlen("VTIMEZONE"))) {
			} else if (!strncmp(cont + 1, "STANDARD", strlen("STANDARD"))) {
			} else if (!strncmp(cont + 1, "DAYLIGHT", strlen("DAYLIGHT"))) {
*/
			} else {

			}

		} else if (!strncmp(prop, "END", strlen("END"))) {
			if (prop) free(prop);
			if (cont) free(cont);

			break;

		}

		if (prop) {
			free(prop);
			prop = NULL;
		}
		if (cont) {
			free(cont);
			cont = NULL;
		}
	}

	return cursor;
}

char *cals_func_vevent(int ver, GList **list_sch, void *data)
{
	CALS_FN_CALL;
	int i;
	char *prop, *cont;
	char *cursor = (char *)data;
	GList *l;
	cal_struct *cs;
	cal_sch_full_t *sch;

	sch = calloc(1, sizeof(cal_sch_full_t));
	retvm_if(sch == NULL, NULL, "Failed to alloc cal_sch_full_t");
	cals_event_init(sch);

	cs = calloc(1, sizeof(cal_struct));
	retvm_if(cs == NULL, NULL, "Failed to alloc cal_struct");
	cs->event_type = CAL_STRUCT_TYPE_SCHEDULE;
	cs->user_data = sch;

	*list_sch = g_list_append(*list_sch, cs);

	/* do until meet BEGIN */
	while ((cursor = cals_read_line(cursor, &prop, &cont))) {
		if (!strncmp(prop, "BEGIN", strlen("BEGIN"))) {

			if (!strncmp(cont + 1, "VALARM", strlen("VALARM"))) {
				cursor = cals_func_valarm(sch, cursor);
			} else {
				break;
			}

		} else if (!strncmp(prop, "END", strlen("END"))) {
			break;

		} else {
			for (i = 0; i < VEVE_MAX; i++) {
				if (!strncmp(prop, _veve_list[i].prop, strlen(_veve_list[i].prop))) {
					l = g_list_last(*list_sch);
					_veve_list[i].func(ver, ((cal_struct *)(l->data))->user_data, cont);
					break;
				}
			}
		}

		if (prop) free(prop);
		prop = NULL;
		if (cont) free(cont);
		cont = NULL;

	}

	if (prop) free(prop);
	if (cont) free(cont);
	return cursor;
}

/*TODO: vtodo is same as vevent */
char *cals_func_vtodo(int ver, GList **list_sch, void *data)
{
	int i;
	char *prop, *cont;
	char *cursor = (char *)data;
	GList *l;
	cal_struct *cs;
	cal_sch_full_t *sch;

	sch = calloc(1, sizeof(cal_sch_full_t));
	retvm_if(sch == NULL, NULL, "Failed to alloc cal_sch_full_t");
	cals_todo_init(sch);

	cs = calloc(1, sizeof(cal_struct));
	retvm_if(cs == NULL, NULL, "Failed to alloc cal_struct");
	cs->event_type = CAL_STRUCT_TYPE_SCHEDULE;
	cs->user_data = sch;

	*list_sch = g_list_append(*list_sch, cs);

	/* do until meet BEGIN */
	while ((cursor = cals_read_line(cursor, &prop, &cont))) {
		if (!strncmp(prop, "BEGIN", strlen("BEGIN"))) {

			if (!strncmp(cont + 1, "VALARM", strlen("VALARM"))) {
				cursor = cals_func_valarm(sch, cursor);
			} else {
				break;
			}

		} else if (!strncmp(prop, "END", strlen("END"))) {
			break;

		} else {
			for (i = 0; i < VEVE_MAX; i++) {
				if (!strncmp(prop, _veve_list[i].prop, strlen(_veve_list[i].prop))) {
					l = g_list_last(*list_sch);
					_veve_list[i].func(ver, ((cal_struct *)(l->data))->user_data, cont);
					break;
				}
			}
		}

		if (prop) free(prop);
		prop = NULL;
		if (cont) free(cont);
		cont = NULL;

	}

	if (prop) free(prop);
	if (cont) free(cont);
	return cursor;
}

char *cals_func_valarm(cal_sch_full_t *sch, void *data)
{
	int i;
	char *prop, *cont;
	char *cursor = (char *)data;

	cal_value *val;
	val = calendar_svc_value_new(CAL_VALUE_LST_ALARM);
	sch->alarm_list = g_list_append(sch->alarm_list, val);

	while ((cursor = cals_read_line(cursor, &prop, &cont))) {
		if (!strncmp(prop, "BEGIN", strlen("BEGIN"))) {
			break;

		} else if (!strncmp(prop, "END", strlen("END"))) {
			break;

		}

		for (i = 0; i < VALA_MAX; i++) {
			if (!strncmp(prop, _vala_list[i].prop, strlen(_vala_list[i].prop))) {
				_vala_list[i].func(sch, cont);
				break;
			}
		}
		if (prop) {
			free(prop);
			prop = NULL;
		}
		if (cont) {
			free(cont);
			cont = NULL;
		}
	}

	if (prop) free(prop);
	if (cont) free(cont);

	return cursor;
}


/* vcalendar */////////////////////////////////////////////////

int cals_func_prodid(GList **list_sch, void *data)
{
	return 0;
}

int cals_func_version(GList **list_sch, void *data)
{
	char *p = (char *)data;

	p++;
	if (!strncmp(p, "1.0", strlen("1.0"))) {
		return 1;
	} else if (!strncmp(p, "2.0", strlen("2.0"))) {
		return 2;
	} else {
		return -1;
	}
	return 0;
}

/* vevnt */////////////////////////////////////////////////
int cals_func_dtstamp(int ver, cal_sch_full_t *sch, void *data)
{
	return 0;
}

int cals_func_dtstart(int ver, cal_sch_full_t *sch, void *data)
{
	char *p = (char *)data;
	int k = 0, j;
	int y, mon, d, h, min, s;
	char buf[64] = {0, };
	char tmp[64] = {0, };

	p++;

	if (!strncmp(p, "TZID=", strlen("TZID="))) {
		k = 0;
		j = 0;
		while (p[j] != ':' && p[j] != ';' && p[j] != '\n' && p[j] != '\0') {
			buf[k] = p[j];
			k++;
			j++;
		}
		if (p[j] != '\0') {
			buf[k] = '\0';
		}
		p += j;
		p++;
		sch->dtstart_tzid = strdup(buf);
	} else {
		sch->dtstart_tzid = strdup("Europe/London");
	}

	if (!strncmp(p, "VALUE=", strlen("VALUE="))) {
		k = 0;
		j = strlen("VALUE=");
		while (p[j] != ':' && p[j] != ';' && p[j] != '\n' && p[j] != '\0') {
			buf[k] = p[j];
			k++;
			j++;
		}
		if (p[j] != '\0') {
			buf[k] = '\0';
		}
		p += j;
		p++;
	}

	if (!strncmp(buf, "DATE", strlen("DATE"))){
		snprintf(tmp, 5, "%s", p);
		sch->dtstart_year = atoi(tmp);
		snprintf(tmp, 3, "%s", p + 4);
		sch->dtstart_month = atoi(tmp);
		snprintf(tmp, 3, "%s", p + 6);
		sch->dtstart_mday = atoi(tmp);

	} else {
		snprintf(tmp, 5, "%s", p);
		y = atoi(tmp);
		snprintf(tmp, 3, "%s", p + 4);
		mon = atoi(tmp);
		snprintf(tmp, 3, "%s", p + 6);
		d = atoi(tmp);
		snprintf(tmp, 3, "%s", p + 9);
		h = atoi(tmp);
		snprintf(tmp, 3, "%s", p + 11);
		min = atoi(tmp);
		snprintf(tmp, 3, "%s", p + 13);
		s = atoi(tmp);
		sch->dtstart_utime = cals_time_date_to_utime(sch->dtstart_tzid,
				y, mon, d, h, min, s);
	}
	return 0;
}

int cals_func_uid(int ver, cal_sch_full_t *sch, void *data)
{
	return 0;
}

int cals_func_class(int ver, cal_sch_full_t *sch, void *data)
{
	return 0;
}

int cals_func_created(int ver, cal_sch_full_t *sch, void *data)
{
	char *p = (char *)data;

	p++;
	sch->created_time = _datetime_to_utime(p);
	return 0;
}

int cals_func_description(int ver, cal_sch_full_t *sch, void *data)
{
	char *p = (char *)data;
	int encoding = 0;

	while (*p != '\n' && *p != '\r' && *p != '\0') {
		if ( *p == ':') {
			p++;
			if (encoding == ENCODE_BASE64) {
				gsize len;
				sch->description = strdup((char *)g_base64_decode(p, &len));

			} else if (encoding == ENCODE_QUOTED_PRINTABLE) {
				if (ver == 1) {
					int len;
					cals_quoted_printable_decode(p, &len);
					sch->description = strdup(p);
				} else {
					CALS_DBG("only ver1.0 supports quoted printable\n");
					sch->summary = strdup(p);
				}

			} else {
				sch->description = strdup(p);

			}

		} else if (*p == ';') {
			p++;
			cals_get_optional_parameter(p, &encoding);

		} else {
			p++;
		}
	}
	return 0;
}

int cals_func_geo(int ver, cal_sch_full_t *sch, void *data)
{
	return 0;
}

int cals_func_last_mod(int ver, cal_sch_full_t *sch, void *data)
{
	char *p = (char *)data;

	p++;
	sch->last_mod = _datetime_to_utime(p);
	return 0;
}

int cals_func_location(int ver, cal_sch_full_t *sch, void *data)
{
	char *p = (char *)data;
	int encoding = 0;

	while (*p != '\n' && *p != '\r' && *p != '\0') {
		if ( *p == ':') {
			p++;
			if (encoding == ENCODE_BASE64) {
				gsize len;
				sch->location = strdup((char *)g_base64_decode(p, &len));

			} else if (encoding == ENCODE_QUOTED_PRINTABLE) {
				if (ver == 1) {
					int len;
					cals_quoted_printable_decode(p, &len);
					sch->location = strdup(p);
				} else {
					CALS_DBG("only ver1.0 supports quoted printable\n");
					sch->summary = strdup(p);
				}

			} else {
				sch->location = strdup(p);

			}

		} else if (*p == ';') {
			p++;
			cals_get_optional_parameter(p, &encoding);

		} else {
			p++;
		}
	}
	return 0;
}

int cals_func_organizer(int ver, cal_sch_full_t *sch, void *data)
{
	return 0;
}

int cals_func_priority(int ver, cal_sch_full_t *sch, void *data)
{
	char *p = (char *)data;

	p++;
	sch->priority = 0;	/* default */
	if (p[0] >= '0' && p[0] <= '9') {
		sch->priority = atoi(p);
	} else {
		CALS_DBG("warning check range\n");
	}
	return 0;
}

int cals_func_seq(int ver, cal_sch_full_t *sch, void *data)
{
	return 0;
}

int cals_func_status(int ver, cal_sch_full_t *sch, void *data)
{
	char *p = (char *)data;

	p++;
	if (sch->cal_type == CALS_SCH_TYPE_TODO) {
		if (!strncmp(p, "NEEDS-ACTION", strlen("NEEDS-ACTION"))) {
		} else if (!strncmp(p, "COMPLETED", strlen("COMPLETED"))) {
			sch->task_status = CALS_TODO_STATUS_NEEDS_ACTION;
		} else if (!strncmp(p, "IN-PROCESS", strlen("IN-PROCESS"))) {
			sch->task_status = CALS_TODO_STATUS_IN_PROCESS;
		} else if (!strncmp(p, "CANCELLED", strlen("CANCELLED"))) {
			sch->task_status = CALS_TODO_STATUS_CANCELLED;
		} else {
			sch->task_status = CALS_TODO_STATUS_NONE;
		}
	} else if (sch->cal_type == CALS_SCH_TYPE_EVENT ) {
		if (!strncmp(p, "TENTATIVE", strlen("TENTATIVE"))) {
			sch->task_status = CALS_EVENT_STATUS_TENTATIVE;
		} else if (!strncmp(p, "CONFIRMED", strlen("CONFIRMED"))) {
			sch->task_status = CALS_EVENT_STATUS_CONFIRMED;
		} else if (!strncmp(p, "CANCELLED", strlen("CANCELLED"))) {
			sch->task_status = CALS_EVENT_STATUS_CANCELLED;
		} else {
			sch->task_status = CALS_EVENT_STATUS_NONE;
		}
	}

	return 0;
}

int cals_func_summary(int ver, cal_sch_full_t *sch, void *data)
{
	char *p = (char *)data;
	int encoding = 0;

	while (*p != '\n' && *p != '\r' && *p != '\0') {
		if ( *p == ':') {
			p++;
			if (encoding == ENCODE_BASE64) {
				gsize len;
				sch->summary = strdup((char *)g_base64_decode(p, &len));

			} else if (encoding == ENCODE_QUOTED_PRINTABLE) {
				if (ver == 1) {
					int len;
					cals_quoted_printable_decode(p, &len);
					sch->summary = strdup(p);
				} else {
					CALS_DBG("only ver1.0 supports quoted printable\n");
					sch->summary = strdup(p);
				}

			} else {
				sch->summary = strdup(p);
			}
			break;

		} else if (*p == ';') {
			p++;
			cals_get_optional_parameter(p, &encoding);

		} else {
			p++;
		}
	}
	DBG("ver(%d)summary(%s)\n", ver, sch->summary);

	return 0;
}

int cals_func_transp(int ver, cal_sch_full_t *sch, void *data)
{
	return 0;
}

int cals_func_url(int ver, cal_sch_full_t *sch, void *data)
{
	return 0;
}

int cals_func_recurid(int ver, cal_sch_full_t *sch, void *data)
{
	return 0;
}

int cals_func_rrule(int ver, cal_sch_full_t *sch, void *data)
{
	int i = 0, k = 0;
	char *p = (char *)data;

	p++;
	while (*p != '\n' && *p != '\r' && *p != '\0') {
		if (*p == ':' || *p == ';' || k == 0) {
			p += k;
			k = 1;

			switch (ver) {
			case 1:
				for (i = 0; i < RRULE_MAX; i++) {
					if (!strncmp(p, _rrule_list[i].prop, strlen(_rrule_list[i].prop))) {
						int j = 0;
						char buf[64] = {0, };
						p += strlen(_rrule_list[i].prop);
						while (p[j] != ';' && p[j] != '\n' && p[j] != '\0') {
							buf[j] = p[j];
							j++;
						}
						if (p[j] != '\0') {
							buf[j] = '\0';
						}
						p += j;
						_rrule_list[i].func(sch, buf);
						break;
					}
				}
				break;

			case 2:
				/* Suppose vcalendar 1.0, if p == data */
				if ((p - 1) == (char *)data) {
					cals_ver1_func_rrule(sch, p);
				}
				break;
			}
		} else {
			p++;
		}
	}

	return 0;
}

int cals_func_dtend(int ver, cal_sch_full_t *sch, void *data)
{
	char *p = (char *)data;
	int k = 0, j;
	int y, mon, d, h, min, s;
	char buf[64] = {0, };
	char tmp[64] = {0, };

	p++;

	if (!strncmp(p, "TZID=", strlen("TZID="))) {
		k = 0;
		j = 0;
		while (p[j] != ':' && p[j] != ';' && p[j] != '\n' && p[j] != '\0') {
			buf[k] = p[j];
			k++;
			j++;
		}
		if (p[j] != '\0') {
			buf[k] = '\0';
		}
		p += j;
		p++;
		sch->dtend_tzid = strdup(buf);
	} else {
		sch->dtend_tzid = strdup("Europe/London");
	}

	if (!strncmp(p, "VALUE=", strlen("VALUE="))) {
		k = 0;
		j = strlen("VALUE=");
		while (p[j] != ':' && p[j] != ';' && p[j] != '\n' && p[j] != '\0') {
			buf[k] = p[j];
			k++;
			j++;
		}
		if (p[j] != '\0') {
			buf[k] = '\0';
		}
		p += j;
		p++;
	}

	if (!strncmp(buf, "DATE", strlen("DATE"))){
		snprintf(tmp, 5, "%s", p);
		sch->dtend_year = atoi(tmp);
		snprintf(tmp, 3, "%s", p + 4);
		sch->dtend_month = atoi(tmp);
		snprintf(tmp, 3, "%s", p + 6);
		sch->dtend_mday = atoi(tmp);

	} else {
		snprintf(tmp, 5, "%s", p);
		y = atoi(tmp);
		snprintf(tmp, 3, "%s", p + 4);
		mon = atoi(tmp);
		snprintf(tmp, 3, "%s", p + 6);
		d = atoi(tmp);
		snprintf(tmp, 3, "%s", p + 9);
		h = atoi(tmp);
		snprintf(tmp, 3, "%s", p + 11);
		min = atoi(tmp);
		snprintf(tmp, 3, "%s", p + 13);
		s = atoi(tmp);
		sch->dtend_utime = cals_time_date_to_utime(sch->dtend_tzid,
				y, mon, d, h, min, s);
	}
	return 0;
}

int cals_func_duration(int ver, cal_sch_full_t *sch, void *data)
{
	return 0;
}

int cals_func_attach(int ver, cal_sch_full_t *sch, void *data)
{
	return 0;
}

int cals_func_attendee(int ver, cal_sch_full_t *sch, void *data)
{
	return 0;
}

int cals_func_categories(int ver, cal_sch_full_t *sch, void *data)
{
	CALS_FN_CALL;
	char *p = (char *)data;
	int encoding = 0;

	while (*p != '\n' && *p != '\r' && *p != '\0') {
		if ( *p == ':') {
			p++;
			if (encoding == ENCODE_BASE64) {
				gsize len;
				sch->categories = strdup((char *)g_base64_decode(p, &len));

			} else if (encoding == ENCODE_QUOTED_PRINTABLE) {
				if (ver == 1) {
					int len;
					cals_quoted_printable_decode(p, &len);
					sch->categories = strdup(p);
				} else {
					CALS_DBG("only ver1.0 supports quoted printable\n");
					sch->categories = strdup(p);
				}

			} else {
				sch->categories = strdup(p);
			}
			break;

		} else if (*p == ';') {
			p++;
			cals_get_optional_parameter(p, &encoding);

		} else {
			p++;
		}
	}
	DBG("ver(%d)categories(%s)\n", ver, sch->categories);

	return 0;
}

int cals_func_comment(int ver, cal_sch_full_t *sch, void *data)
{
	return 0;
}

int cals_func_contact(int ver, cal_sch_full_t *sch, void *data)
{
	return 0;
}

int cals_func_exdate(int ver, cal_sch_full_t *sch, void *data)
{
	return 0;
}
/*
int cals_func_status(int ver, cal_sch_full_t *sch, void *data)
{
	return 0;
}
*/
int cals_func_related(int ver, cal_sch_full_t *sch, void *data)
{
	return 0;
}

int cals_func_resources(int ver, cal_sch_full_t *sch, void *data)
{
	return 0;
}

int cals_func_rdate(int ver, cal_sch_full_t *sch, void *data)
{
	return 0;
}

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

/* for vcalendar version 1.0 */
int cals_ver1_func_rrule(cal_sch_full_t *sch, void *data)
{
	char *p = (char *)data;
	int i = 0, j = 0;
	int column = 0, loop = 1;
	int byint = -1;	/* for by day:1 or by position:0 */
	int y, mon, d, h, min, s;
	char buf[64] = {0, };
	char tmp[8] = {0, };

	while(loop) {
		/* parse sentence */
		i = 0;
		while(p[i] != '\n' && p[i] != '\0' && p[i] != ' ') {
			buf[i] = p[i];
			i++;
		}
		buf[i] = '\n';

		/* exit when sentence ends */
		if (p[i] == '\n' || p[i] == '\0') {
			loop = 0;
		}


		if (column == 0) {
			column++;

			/* freq */
			if (buf[0] == 'D') {
				sch->freq = CALS_FREQ_DAILY;

			} else if (buf[0] == 'W') {
				sch->freq = CALS_FREQ_WEEKLY;

			} else if (buf[0] == 'M') {
				sch->freq = CALS_FREQ_MONTHLY;

			} else if (buf[0] == 'Y') {
				sch->freq = CALS_FREQ_YEARLY;

			} else {
				sch->freq = CALS_FREQ_ONCE;

			}

			/* interval */
			if (buf[1] == 'P') {
				byint = 0;
				sch->interval = atoi(&buf[2]);

			} else if (buf[1] == 'D') {
				byint = 1;
				sch->interval = atoi(&buf[2]);

			} else {
				sch->interval = atoi(&buf[1]);

			}
			DBG("interval(%d)\n", sch->interval);

		} else {
			if (buf[0] == '#') {
				/* occurrences */
				sch->count = atoi(&buf[1]);
				DBG("occur(%d)\n", sch->count);

			} else if (strlen(buf) < 3) {
				/* get by-day: TODO:not supported byday*/
				if (byint == 0) {

				} else if (byint == 1) {

				} else {

				}

			} else if (strlen(buf) >= strlen("YYYYMMDDTHHMMSS")) {
				sch->until_type = CALS_TIME_UTIME;
				snprintf(tmp, 5, "%s", buf);
				y = atoi(tmp);
				snprintf(tmp, 3, "%s", buf + 4);
				mon = atoi(tmp);
				snprintf(tmp, 3, "%s", buf + 6);
				d = atoi(tmp);
				snprintf(tmp, 3, "%s", buf + 9);
				h = atoi(tmp);
				snprintf(tmp, 3, "%s", buf + 11);
				min = atoi(tmp);
				snprintf(tmp, 3, "%s", buf + 13);
				s = atoi(tmp);
				sch->until_utime = cals_time_date_to_utime(sch->dtstart_tzid,
				y, mon, d, h, min, s);
				DBG("until(%s)\n", buf);

			} else {
				/* get repeat week day */
				char buf_w[64] = {0};
				for (j = 0; j < WEEKNAME2_MAX; j++) {
					if(!strncmp(buf, weekname2[j], strlen(weekname2[j]))) {
						if (strlen(buf_w) != 0) {
							strcat(buf_w, ", ");
						}
						strcat(buf_w, weekname2[j]);
					}
				}
				sch->byday = strdup(buf_w);
			}
		}

		p += i;
		p++;
	}
	return 0;
}

//alarm////////////////////////////////////////////////////////////
int cals_func_action(cal_sch_full_t *sch, void *data)
{
	return 0;
}

char *cals_convert_sec_from_duration(char *p, int *dur_t, char *dur)
{
	char du = '0';
	char buf[8] = {0, };
	int i = 0, c, d = 1;
	int t = 0;

	CALS_DBG("%s", p);
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

int cals_func_trigger(cal_sch_full_t *sch, void *data)
{
	int i = 0, out = 0;
	char *p = (char *)data;
	long long int dtstart_utime;
	int dur_t;
	cal_value *val;
	cal_alarm_info_t *alarm;
	GList *l;

	p++;
	l = g_list_last(sch->alarm_list);
	if (l == NULL) {
		return -1;
	}
	val = (cal_value *)l->data;
	alarm = (cal_alarm_info_t *)val->user_data;
	dtstart_utime = sch->dtstart_utime;

	while (*p != '\n' && *p != '\r' && *p != '\0') {

		for (i = 0; i < TRIG_MAX; i++) {
			if (!strncmp(p, _trig_list[i].prop, strlen(_trig_list[i].prop))) {
				out = 1;
				int j = 0;
				char buf[64] = {0, };
				p += strlen(_trig_list[i].prop);
				while (p[j] != ';' && p[j] != '\n' && p[j] != '\0') {
					buf[j] = p[j];
					j++;
				}
				if (p[j] != '\0') {
					buf[j] = '\0';
				}

				p += j;
				_trig_list[i].func(sch, buf);
				break;
			}
		}
		if (out == 1) {
			break;
		}
		p = cals_convert_sec_from_duration(p, &dur_t, NULL);
		alarm->alarm_time = dtstart_utime + (long long int)dur_t;
		break;
	}
	return 0;
}

int cals_func_repeat(cal_sch_full_t *sch, void *data)
{
	return 0;
}

int cals_func_duration_alarm(cal_sch_full_t *sch, void *data)
{
	char *p = (char *)data;
	char dur;
	int dur_t;
	cal_value *val;
	cal_alarm_info_t *alarm = NULL;
	GList *l;

	p++;
	l = g_list_last(sch->alarm_list);
	if (l) {
		val = (cal_value *)l->data;
		alarm = (cal_alarm_info_t *)val->user_data;

		cals_convert_sec_from_duration(p, &dur_t, &dur);
		switch (dur) {
			case 'W':
				alarm->remind_tick = dur_t/(7 *24 *60 *60);
				alarm->remind_tick_unit = CAL_SCH_TIME_UNIT_WEEK;
				break;
			case 'D':
				alarm->remind_tick = dur_t/(24 *60 *60);
				alarm->remind_tick_unit = CAL_SCH_TIME_UNIT_DAY;
				break;
			case 'H':
				alarm->remind_tick = dur_t/(60 *60);
				alarm->remind_tick_unit = CAL_SCH_TIME_UNIT_HOUR;
				break;
			case 'M':
				alarm->remind_tick = dur_t/(60);
				alarm->remind_tick_unit = CAL_SCH_TIME_UNIT_MIN;
				break;
			default:
				alarm->remind_tick = 1;
				alarm->remind_tick_unit = 0;
				break;
		}
	}
	DBG("tick:%d\n", alarm->remind_tick);
	DBG("unit:%c\n", dur);
	return 0;
}

int cals_func_attach_alarm(cal_sch_full_t *sch, void *data)
{
	return 0;
}

int cals_func_summary_alarm(cal_sch_full_t *sch, void *data)
{
	char *p = (char *)data;
	cal_value *val;
	cal_alarm_info_t *alarm;
	GList *l;

	p++;
	l = g_list_last(sch->alarm_list);
	if (l) {
		val = (cal_value *)l->data;
		alarm = (cal_alarm_info_t *)val->user_data;

		alarm->alarm_description = strdup(p);
	}
	return 0;
}


//rrule////////////////////////////////////////////////////////////
int cals_func_freq(cal_sch_full_t *sch, void *data)
{
	char *p = (char *)data;

	DBG("%s\n", (char *)data);
	if (!strncmp(p, "YEARLY", strlen("YEARLY"))) {
		sch->freq = CALS_FREQ_YEARLY;

	} else if (!strncmp(p, "MONTHLY", strlen("MONTHLY"))) {
		sch->freq = CALS_FREQ_MONTHLY;

	} else if (!strncmp(p, "WEEKLY", strlen("WEEKLY"))) {
		sch->freq = CALS_FREQ_WEEKLY;

	} else if (!strncmp(p, "DAILY", strlen("DAILY"))) {
		sch->freq = CALS_FREQ_DAILY;

	} else if (!strncmp(p, "HOURLY", strlen("HOURLY"))) {
		sch->freq = CALS_FREQ_ONCE;

	} else if (!strncmp(p, "MINUTELY", strlen("MINUTELY"))) {
		sch->freq = CALS_FREQ_ONCE;

	} else if (!strncmp(p, "SECONDLY", strlen("SECONDLY"))) {
		sch->freq = CALS_FREQ_ONCE;

	} else {
		sch->freq = CALS_FREQ_ONCE;

	}
	return 0;
}

int cals_func_until(cal_sch_full_t *sch, void *data)
{
	int y, mon, d, h, min, s;
	char *p = (char *)data;
	char buf[8] = {0};

	/* until value type has the same value as the dtstart */
	sch->range_type = CALS_RANGE_UNTIL;
	if (sch->dtstart_utime) {
		sch->until_type = CALS_TIME_UTIME;
		snprintf(buf, 5, "%s", p);
		y = atoi(buf);
		snprintf(buf, 3, "%s", p + 4);
		mon = atoi(buf);
		snprintf(buf, 3, "%s", p + 6);
		d = atoi(buf);
		snprintf(buf, 3, "%s", p + 9);
		h = atoi(buf);
		snprintf(buf, 3, "%s", p + 11);
		min = atoi(buf);
		snprintf(buf, 3, "%s", p + 13);
		s = atoi(buf);
		sch->until_utime = cals_time_date_to_utime(sch->dtstart_tzid,
				y, mon, d, h, min, s);

	} else {
		sch->until_type = CALS_TIME_LOCALTIME;
		snprintf(buf, strlen("YYYY") + 1, "%s", p);
		sch->until_year = atoi(buf);
		snprintf(buf, strlen("MM") + 1, "%s", p + 4);
		sch->until_month = atoi(buf);
		snprintf(buf, strlen("DD") + 1, "%s", p + 6);
		sch->until_mday = atoi(buf);

	}
	return 0;
}

int cals_func_count(cal_sch_full_t *sch, void *data)
{
	int c;
	char *p = (char *)data;

	DBG("%s\n", (char *)data);
	sch->range_type = CALS_RANGE_COUNT;
	c = atoi(p);
	sch->count = c < 0 ? 0 : c;
	return 0;
}

int cals_func_interval(cal_sch_full_t *sch, void *data)
{
	int c;
	char *p = (char *)data;

	DBG("%s\n", (char *)data);
	c = atoi(p);
	sch->interval = c < 0 ? 0 : c;
	return 0;
}

int cals_func_bysecond(cal_sch_full_t *sch, void *data)
{
	CALS_DBG("%s\n", (char *)data);
	char *p = (char *)data;
	sch->bysecond = strdup(p);
	return 0;
}

int cals_func_byminute(cal_sch_full_t *sch, void *data)
{
	CALS_DBG("%s\n", (char *)data);
	char *p = (char *)data;
	sch->byminute = strdup(p);
	return 0;
}

int cals_func_byhour(cal_sch_full_t *sch, void *data)
{
	CALS_DBG("%s\n", (char *)data);
	char *p = (char *)data;
	sch->byhour = strdup(p);
	return 0;
}

int cals_func_byday(cal_sch_full_t *sch, void *data)
{
	CALS_DBG("%s\n", (char *)data);
	char *p = (char *)data;
	sch->byday = strdup(p);
	return 0;
}

int cals_func_bymonthday(cal_sch_full_t *sch, void *data)
{
	CALS_DBG("%s\n", (char *)data);
	char *p = (char *)data;
	sch->bymonthday = strdup(p);
	return 0;
}

int cals_func_byyearday(cal_sch_full_t *sch, void *data)
{
	CALS_DBG("%s\n", (char *)data);
	char *p = (char *)data;
	sch->byyearday = strdup(p);
	return 0;
}

int cals_func_byweekno(cal_sch_full_t *sch, void *data)
{
	CALS_DBG("%s\n", (char *)data);
	char *p = (char *)data;
	sch->byweekno = strdup(p);
	return 0;
}

int cals_func_bymonth(cal_sch_full_t *sch, void *data)
{
	CALS_DBG("%s\n", (char *)data);
	char *p = (char *)data;
	sch->bymonth = strdup(p);
	return 0;
}

int cals_func_bysetpos(cal_sch_full_t *sch, void *data)
{
	CALS_DBG("%s\n", (char *)data);
	char *p = (char *)data;
	sch->bysetpos = strdup(p);
	return 0;
}

int cals_func_wkst(cal_sch_full_t *sch, void *data)
{
	CALS_DBG("%s\n", (char *)data);

	char *p = (char *)data;

	if (!strncmp(p, "SU", strlen("SU"))) {
		sch->wkst = CALS_SUNDAY;

	} else if (!strncmp(p, "MO", strlen("MO"))) {
		sch->wkst = CALS_MONDAY;

	} else if (!strncmp(p, "TU", strlen("TU"))) {
		sch->wkst = CALS_TUESDAY;

	} else if (!strncmp(p, "WE", strlen("WE"))) {
		sch->wkst = CALS_WEDNESDAY;

	} else if (!strncmp(p, "TH", strlen("TH"))) {
		sch->wkst = CALS_THURSDAY;

	} else if (!strncmp(p, "FR", strlen("FR"))) {
		sch->wkst = CALS_FRIDAY;

	} else if (!strncmp(p, "SA", strlen("SA"))) {
		sch->wkst = CALS_SATURDAY;

	}
	return 0;
}

int cals_func_related_trig(cal_sch_full_t *sch, void *data)
{
	CALS_DBG("%s\n", (char *)data);
	return 0;
}

int cals_func_value(cal_sch_full_t *sch, void *data)
{
	CALS_DBG("%s\n", (char *)data);
	return 0;
}

int cals_func_charset(int *val, void *data)
{
	CALS_DBG("%s\n", (char *)data);
	return 0;
}

int cals_func_encoding(int *val, void *data)
{
	char *p = (char *)data;
	*val = 0;

	if (!strncmp(p, "BASE64", strlen("BASE64"))) {
		*val = ENCODE_BASE64;

	} else if (!strncmp(p, "QUOTED-PRINTABLE", strlen("QUOTED-PRINTABLE"))){
		*val = ENCODE_QUOTED_PRINTABLE;

	}
	return 0;
}

struct buf {
	int size;
	char *data;
	char lbuf[76];
};

#define _strlen(s) (((s) && *(s)) ? strlen(s) : 0)

static inline int _alloc(struct buf *b, int n)
{
	b->data = realloc(b->data, b->size + n);

	retvm_if(!b->data, CAL_ERR_OUT_OF_MEMORY, "Out of memory error");
	b->size += n;

	return CAL_SUCCESS;
}


static struct buf *cal_svc_buf_new()
{
	struct buf *b;

	b = calloc(1, sizeof(struct buf));
	if (!b) {
		return NULL;
	}

	b->data = malloc(sizeof(char));
	if (!b->data) {
		free(b);
		return NULL;
	}

	*b->data = '\0';
	b->size = 1;

	return b;
}


static void cal_svc_buf_free(struct buf **b)
{
	if (!b || !*b)
		return;

	if ((*b)->data)
		free((*b)->data);

	free(*b);
	b = NULL;
}

static inline int _flush(struct buf *b)
{
	int r;
	r = _alloc(b, _strlen(b->lbuf) + 2);
	retv_if(r < CAL_SUCCESS, r);

	strncat(b->data, b->lbuf, b->size - _strlen(b->data) - 1);
	strncat(b->data, "\r\n", b->size - _strlen(b->data) - 1);
	*b->lbuf = '\0';
	return CAL_SUCCESS;
}

static inline int _fold(struct buf *b)
{
	int r;
	r = _alloc(b, _strlen(b->lbuf) + 3);
	retv_if(r < CAL_SUCCESS, r);

	strncat(b->data, b->lbuf, b->size - _strlen(b->data) - 1);
	strncat(b->data, "\r\n ", b->size - _strlen(b->data) - 1);
	*b->lbuf = '\0';
	return CAL_SUCCESS;
}

static inline int _set_str(struct buf *b, const char *s)
{
	int remain_lbuf;
	int remain_str;
	int k;
	int r;

	remain_lbuf = sizeof(b->lbuf) - _strlen(b->lbuf);
	remain_str = _strlen(s);

	k = 0;
	while ( remain_lbuf - 1 < remain_str) {
		strncat(b->lbuf, s + k, remain_lbuf - 1);
		k += remain_lbuf - 1;
		remain_str -= remain_lbuf - 1;
		r = _fold(b);
		retv_if(r < CAL_SUCCESS, r);
		remain_lbuf = sizeof(b->lbuf);
	}

	strncat(b->lbuf, s + k, remain_lbuf - 1);
	return CAL_SUCCESS;
}

static inline int cal_svc_buf_flush(struct buf *b)
{
	return _flush(b);
}

static int cal_svc_buf_print(struct buf *b, const char *s1, const char *s2)
{
	int r;

	if (s1) {
		r = _set_str(b, s1);
		retv_if(r < CAL_SUCCESS, r);
	}

	if (s2) {
		r = _set_str(b, s2);
		retv_if(r < CAL_SUCCESS, r);
	}

	return CAL_SUCCESS;
}


static int cal_svc_buf_printline(struct buf *b, const char *s1, const char *s2)
{
	int r;

	if (s1) {
		r = _set_str(b, s1);
		retv_if(r < CAL_SUCCESS, r);
	}

	if (s2) {
		r = _set_str(b, s2);
		retv_if(r < CAL_SUCCESS, r);
	}

	return _flush(b);
}


static char *cal_svc_buf_get_data(struct buf *b)
{
	if (!b || !b->data)
		return NULL;
	return strdup(b->data);
}

static const char *vl_datetime(int y, int m, int d)
{
	static char buf[9];
	snprintf(buf, sizeof(buf), "%04d%02d%02d", y, m, d);
	return buf;
}

static const char *vl_dur(cal_sch_remind_tick_unit_t unit, int dur)
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
		case CAL_SCH_TIME_UNIT_WEEK:
			snprintf(buf + i, sizeof(buf) - i, "%sW", d);
			break;
		case CAL_SCH_TIME_UNIT_DAY:
			snprintf(buf + i, sizeof(buf) - i, "%sD", d);
			break;
		case CAL_SCH_TIME_UNIT_HOUR:
			snprintf(buf + i, sizeof(buf) - i, "T%sH", d);
			break;
		case CAL_SCH_TIME_UNIT_MIN:
			snprintf(buf + i, sizeof(buf) - i, "T%sM", d);
			break;
		default:
			buf[0] = '\0';
	}

	return buf;
}

static inline int pr_dtstart_utime(struct buf *b, char *tzid, long long int lli)
{
	return cal_svc_buf_printline(b, "DTSTART:", cals_time_get_str_datetime(tzid, lli));
}

static inline int pr_dtstart_datetime(struct buf *b, int y, int m, int s)
{
	return cal_svc_buf_printline(b, "DTSTART:", vl_datetime(y, m, s));
}

static inline int pr_dtend_utime(struct buf *b, char *tzid, long long int lli)
{
	return cal_svc_buf_printline(b, "DTEND:", cals_time_get_str_datetime(tzid, lli));
}

static inline int pr_dtend_datetime(struct buf *b, int y, int m, int s)
{
	return cal_svc_buf_printline(b, "DTEND:", vl_datetime(y, m, s));
}

static inline int pr_created(struct buf *b, char *tzid, long long int t)
{
	return cal_svc_buf_printline(b, "CREATED:", cals_time_get_str_datetime(tzid, t));
}


static inline int pr_lastmod(struct buf *b, char *tzid, long long int lli)
{
	return cal_svc_buf_printline(b, "LAST-MODIFIED:", cals_time_get_str_datetime(tzid, lli));
}

/**
 * Descriptive Component Properties
 */

static inline int pr_summary(struct buf *b, char *s)
{
	return cal_svc_buf_printline(b, "SUMMARY:", s);
}

static inline int pr_description(struct buf *b, char *s)
{
	return cal_svc_buf_printline(b, "DESCRIPTION:", s);
}

static inline int pr_location(struct buf *b, char *s)
{
	return cal_svc_buf_printline(b, "LOCATION:", s);
}

static inline int pr_priority(struct buf *b, int v)
{
	char tmp[2];
	snprintf(tmp, sizeof(tmp), "%d", v);
	return cal_svc_buf_printline(b, "PRIORITY:", tmp);
}

static inline int pr_class(struct buf *b, int v)
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
	return cal_svc_buf_printline(b, "CLASS:", c);
}

static inline int pr_transp(struct buf *b, int v)
{
	// TODO : Need to define enumeration of transp property
	return cal_svc_buf_printline(b, "TRANSP:", v? "OPAQUE":"TRANSPARENT");
}

int pr_dtstamp(char *tzid, struct buf *b)
{
	return cal_svc_buf_printline(b, "DTSTAMP:", cals_time_get_str_datetime(tzid, cals_get_lli_now()));
}

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

int pr_attendee(struct buf *b, cal_participant_info_t *att)
{
	int r;
	if (!att->attendee_email)
		return CAL_ERR_NO_DATA;

	r = cal_svc_buf_print(b, "ATTENDEE", NULL);
	retv_if(r < CAL_SUCCESS, r);

	if (att->attendee_group && *att->attendee_group) {
		r = cal_svc_buf_print(b, ";CUTYPE=", att->attendee_group);
		retv_if(r < CAL_SUCCESS, r);
	}

	// TODO : NO 'member' member in cal_participant_info_t

	r = cal_svc_buf_print(b, ";ROLE=", _att_role[att->attendee_role]);
	retv_if(r < CAL_SUCCESS, r);

	r = cal_svc_buf_print(b, ";PARTSTAT=", _att_st[att->attendee_status]);
	retv_if(r < CAL_SUCCESS, r);

	r = cal_svc_buf_print(b, ";RSVP=", att->attendee_rsvp?"TRUE":"FALSE");
	retv_if(r < CAL_SUCCESS, r);

	if (att->attendee_delegator_uri && *att->attendee_delegator_uri) {
		r = cal_svc_buf_print(b, ";DELEGATED-TO=", att->attendee_delegator_uri);
		retv_if(r < CAL_SUCCESS, r);
	}

	if (att->attendee_delegate_uri && *att->attendee_delegate_uri) {
		r = cal_svc_buf_print(b, ";DELEGATED-FROM=", att->attendee_delegate_uri);
		retv_if(r < CAL_SUCCESS, r);
	}

	// TODO : No 'sentby' member in cal_participant_info_t

	if (att->attendee_name && *att->attendee_name) {
		r = cal_svc_buf_print(b, ";CN=", att->attendee_name);
		retv_if(r < CAL_SUCCESS, r);
	}

	return cal_svc_buf_printline(b, ":", att->attendee_email);
}

int pr_organizer(struct buf *b, char *cn, char *address)
{
	int r;

	r = cal_svc_buf_print(b, "ORGANIZER", NULL);
	retv_if(r < CAL_SUCCESS, r);
	if (cn && *cn) {
		r = cal_svc_buf_print(b, ";CN=", cn);
		retv_if(r < CAL_SUCCESS, r);
	}

	return cal_svc_buf_printline(b, ":", address);
}

int pr_trigger(struct buf *b, cal_sch_remind_tick_unit_t unit, int dur, long long int t)
{
	retvm_if(unit == CAL_SCH_TIME_UNIT_OFF, CAL_ERR_NO_DATA, "tick unit is invalid");

	if (unit == CAL_SCH_TIME_UNIT_SPECIFIC)
		return cal_svc_buf_printline(b, "TRIGGER;VALUE=DATE-TIME:",cals_time_get_str_datetime(NULL, t));
	else
		return cal_svc_buf_printline(b, "TRIGGER:", vl_dur(unit, dur));
}

int pr_action(struct buf *b)
{
	return cal_svc_buf_printline(b, "ACTION:", "AUDIO");
}


int pr_audio(struct buf *b, cal_alarm_info_t *alm)
{
	int r;

	r = pr_action(b);
	retv_if(r < CAL_SUCCESS, r);

	r = pr_trigger(b, alm->remind_tick_unit, alm->remind_tick, alm->alarm_time);
	retv_if(r < CAL_SUCCESS, r);

	// TODO : Support duration
	// TODO : Support repeat (snooze)
	return CAL_SUCCESS;
}


int cp_alarm(struct buf *b, cal_alarm_info_t *alm)
{
	int r;
	// TODO : No action type is defined
	r = cal_svc_buf_printline(b, "BEGIN:VALARM", NULL);
	retv_if(r < CAL_SUCCESS, r);

	r = pr_audio(b, alm);
	retv_if(r < CAL_SUCCESS, r);

	// TODO : Display
	// TODO : Email
	// TODO : Proc

	return cal_svc_buf_printline(b, "END:VALARM", NULL);
}

int cp_schedule(struct buf *b, cal_sch_full_t *s)
{
	int r;
	GList *l;
	cal_participant_info_t *att;
	cal_alarm_info_t *alm;

	r = cal_svc_buf_printline(b, "BEGIN:VEVENT", NULL);
	retv_if(r < CAL_SUCCESS, r);

	r = pr_class(b, s->sensitivity);
	retv_if(r < CAL_SUCCESS, r);

	r = pr_transp(b, s->busy_status);
	retv_if(r < CAL_SUCCESS, r);

	r = pr_created(b, s->dtstart_tzid, s->created_time);
	retv_if(r < CAL_SUCCESS, r);

	if (s->description) {
		r = pr_description(b, s->description);
		retv_if(r < CAL_SUCCESS, r);
	}

	switch (s->dtstart_type) {
	case CALS_TIME_UTIME:
		r = pr_dtstart_utime(b, s->dtstart_tzid, s->dtstart_utime);
		retv_if(r < CAL_SUCCESS, r);
		break;

	case CALS_TIME_LOCALTIME:
		r = pr_dtstart_datetime(b, s->dtstart_year, s->dtstart_month, s->dtstart_mday);
		retv_if(r < CAL_SUCCESS, r);
		break;

	}

	// TODO : geo

	r = pr_lastmod(b, s->dtstart_tzid, s->last_mod);
	retv_if(r < CAL_SUCCESS, r);

	if (s->location) {
		r = pr_location(b, s->location);
		retv_if(r < CAL_SUCCESS, r);
	}
	if (s->organizer_email) {
		r = pr_organizer(b, s->organizer_name, s->organizer_email);
		retv_if(r < CAL_SUCCESS, r);
	}

	r = pr_priority(b, s->priority);
	retv_if(r < CAL_SUCCESS, r);

	// TODO : seq

	r = pr_dtstamp(s->dtstart_tzid, b);
	retv_if(r < CAL_SUCCESS, r);

	if (s->summary) {
		r = pr_summary(b, s->summary);
		retv_if(r < CAL_SUCCESS, r);
	}

	switch (s->dtend_type) {
	case CALS_TIME_UTIME:
		r = pr_dtend_utime(b, s->dtstart_tzid, s->dtend_utime);
		retv_if(r < CAL_SUCCESS, r);
		break;

	case CALS_TIME_LOCALTIME:
		r = pr_dtend_datetime(b, s->dtend_year, s->dtend_month, s->dtend_mday);
		retv_if(r < CAL_SUCCESS, r);
		break;

	}

	if (s->attendee_list) {
		for (l = s->attendee_list; l; l = g_list_next(l)) {
			if (!l->data || ((cal_value *)(l->data))->v_type != CAL_EVENT_PATICIPANT
					|| !((cal_value *)(l->data))->user_data) {
				ERR("The attendee entry has no attendee data");
				return CAL_ERR_NO_DATA;
			}
			att = ((cal_value *)(l->data))->user_data;
			r = pr_attendee(b, att);
			retv_if(r < CAL_SUCCESS, r);
		}
	}

	if (s->alarm_list) {
		for (l = s->alarm_list; l; l = g_list_next(l)) {
			if (!l->data || ((cal_value *)(l->data))->v_type != CAL_EVENT_ALARM
					|| !((cal_value *)(l->data))->user_data) {
				ERR("The alarm entry has no alarm data");
				return CAL_ERR_NO_DATA;
			}
			alm = ((cal_value *)(l->data))->user_data;
			r = cp_alarm(b, alm);
			retv_if(r < CAL_SUCCESS, r);
		}
	}

	return cal_svc_buf_printline(b, "END:VEVENT", NULL);
}


int cp_vcalendar(struct buf *b, GList *sch_l)
{
	int r;
	cal_sch_full_t *s;
	GList *l;

	r = cal_svc_buf_printline(b, "BEGIN:VCALENDAR", NULL);
	retv_if(r < CAL_SUCCESS, r);

	r = cal_svc_buf_printline(b, "CALSCALE:GREGORIAN", NULL);
	retv_if(r < CAL_SUCCESS, r);

	r = cal_svc_buf_printline(b, "PRODID:-//Samsung Electronics//Calendar//EN", NULL);
	retv_if(r < CAL_SUCCESS, r);

	r = cal_svc_buf_printline(b, "VERSION:2.0", NULL);
	retv_if(r < CAL_SUCCESS, r);

	for (l = sch_l; l; l = g_list_next(l)) {
		if (!l->data || !((cal_struct *)(l->data))->user_data) {
			ERR("The schedule entry has no event data");
			return CAL_ERR_NO_DATA;
		}
		s = (cal_sch_full_t *)((cal_struct *)(l->data))->user_data;
		r = cp_schedule(b, s);
		retv_if(r < CAL_SUCCESS, r);
	}

	return cal_svc_buf_printline(b, "END:VCALENDAR", NULL);
}


API int calendar_svc_write_schedules(GList *schedules, char **stream)
{
	int r;
	struct buf *b;
	char *ical;

	retvm_if(!schedules || !stream, CAL_ERR_ARG_NULL, "Invalid parameter");

	b = cal_svc_buf_new();
	retvm_if(!b, CAL_ERR_OUT_OF_MEMORY, "Failed to create a buffer");

	r = cp_vcalendar(b, schedules);

	if (r < 0) {
		cal_svc_buf_free(&b);
		return r;
	}

	ical = cal_svc_buf_get_data(b);
	cal_svc_buf_free(&b);

	if (!ical) {
		ERR("Failed to get ical data");
		return CAL_ERR_OUT_OF_MEMORY;
	}

	if (!*ical) {
		ERR("No ical data");
		return CAL_ERR_NO_DATA;
	}

	*stream = ical;

	return CAL_SUCCESS;
}


API int calendar_svc_calendar_export(int calendar_id, const char *path)
{
	int fd, r;
	char *stream = NULL;
	cal_iter *it;
	cal_struct *cs;
	GList *schedules = NULL;
	GList *l;

	if (calendar_id < 0 || path == NULL) {
		ERR("Invalid argument: calendar id(%d) path(%s)", calendar_id, path);
		return CAL_ERR_ARG_INVALID;
	}

	/* get schedules from DB */
	r = calendar_svc_get_all(0, calendar_id, CAL_STRUCT_SCHEDULE, &it);
	if (r != CAL_SUCCESS) {
		ERR("Failed to get calendar(id:%d errno:%d)", calendar_id, r);
		return CAL_ERR_FAIL;
	}
	while (calendar_svc_iter_next(it) == CAL_SUCCESS) {
		r = calendar_svc_iter_get_info(it, &cs);
		if (r != CAL_SUCCESS) {
			ERR("Failed to get cal_struct");
			break;
		}

		if (cs == NULL) {
			ERR("cal_struct is NULL");
			break;
		}
		schedules = g_list_append(schedules, cs);
	}
	calendar_svc_iter_remove(&it);
	/* end getting schedules and close DB */

	if (schedules == NULL) {
		ERR("No schedules");
		return CAL_ERR_FAIL;
	}

	/* get stream from schedules */
	r = calendar_svc_write_schedules(schedules, &stream);

	/* free schedules in memory */
	l = schedules;
	while (l) {
		cs = (cal_struct *)l->data;
		if (cs == NULL) {
			ERR("Not cal struct");
			break;
		}
		calendar_svc_struct_free(&cs);
		l = g_list_next(l);
	}
	g_list_free(schedules);

	if (r < 0) {
		ERR("Failed to write schedules(errno:%d)", r);
		return CAL_ERR_FAIL;
	}

	if (stream == NULL) {
		ERR("stream is NULL");
		return CAL_ERR_FAIL;
	}

	fd = open(path, O_WRONLY|O_CREAT|O_TRUNC, 0660);
	if (fd < 0) {
		ERR("Failed to open path(%s)\n", path);
		free(stream);
		return CAL_ERR_IO_ERR;
	}

	r = write(fd, stream, strlen(stream));
	free(stream);
	close(fd);

	if (r < 0) {
		ERR("Failed to write stream(errno:%d)\n", r);
		return CAL_ERR_IO_ERR;
	}

	return CAL_SUCCESS;
}
