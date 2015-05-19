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

#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_view.h"
#include "cal_record.h"

#include "cal_time.h"
#include "cal_access_control.h"

#include "cal_db.h"
#include "cal_db_util.h"
#include "cal_db_query.h"
#include "cal_db_rrule.h"
#include "cal_db_query.h"
#include "cal_db_alarm.h"
#include "cal_db_instance.h"
#include "cal_db_attendee.h"
#include "cal_db_extended.h"
#include "cal_db_event.h"
#include "cal_db_timezone.h"
#include "cal_utils.h"

enum {
	CAL_RECURRENCE_ID_RANGE_NONE,
	CAL_RECURRENCE_ID_RANGE_THISANDFUTURE,
	CAL_RECURRENCE_ID_RANGE_THISANDPRIOR,
	CAL_RECURRENCE_ID_RANGE_MAX,
};

#define DEBUG_DATETIME(x) DBG("%04d-%02d-%02dT%02d:%02d:%02d",\
		x->time.date.year,\
		x->time.date.month,\
		x->time.date.mday,\
		x->time.date.hour,\
		x->time.date.minute,\
		x->time.date.second)

int cal_db_event_update_original_event_version(int original_event_id, int version)
{
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	int ret = 0;

	DBG("original_event(%d) changed_ver updated", original_event_id);
	if (0 < original_event_id) {
		snprintf(query, sizeof(query), "UPDATE %s SET changed_ver=%d, has_exception = 1 WHERE id=%d ",
				CAL_TABLE_SCHEDULE, version, original_event_id);

		ret = cal_db_util_query_exec(query);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_db_util_query_exec() Fail(%d)", ret);
			SECURE("[%s]", query);
			return ret;
		}
	}
	return CALENDAR_ERROR_NONE;
}

int cal_db_event_check_value_validation(cal_event_s *event)
{
	long long int slli = 0;
	long long int elli = 0;

	RETV_IF(NULL == event, CALENDAR_ERROR_INVALID_PARAMETER);

	if (event->start.type != event->end.type) {
		ERR("start type(%d) is not same as end type(%d)", event->start.type, event->end.type);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	switch (event->start.type) {
	case CALENDAR_TIME_UTIME:
		if (event->end.time.utime < event->start.time.utime) {
			ERR("normal end(%lld) < start(%lld)",
					event->end.time.utime, event->start.time.utime);
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
		break;

	case CALENDAR_TIME_LOCALTIME:
		/* check invalid value */
		if (event->start.time.date.month < 1 || 12 < event->start.time.date.month) {
			ERR("check start month(input:%d)", event->start.time.date.month);
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
		else if (event->start.time.date.mday < 1 || 31 < event->start.time.date.mday) {
			ERR("check start mday(input:%d)", event->start.time.date.mday);
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
		else if (event->end.time.date.month < 1 || 12 < event->end.time.date.month) {
			ERR("check end month(input:%d)", event->end.time.date.month);
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
		else if (event->end.time.date.mday < 1 || 31 < event->end.time.date.mday) {
			ERR("check end mday(input:%d)", event->end.time.date.mday);
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
		else {
			/* handle hour, minute, second */
			if (event->start.time.date.hour < 0 || 24 < event->start.time.date.hour)
				event->start.time.date.hour = 0;
			if (event->start.time.date.minute < 0 || 60 < event->start.time.date.minute)
				event->start.time.date.minute = 0;
			if (event->start.time.date.second < 0 || 60 < event->start.time.date.second)
				event->start.time.date.second = 0;
			if (event->end.time.date.hour < 0 || 24 < event->end.time.date.hour)
				event->end.time.date.hour = 0;
			if (event->end.time.date.minute < 0 || 60 < event->end.time.date.minute)
				event->end.time.date.minute = 0;
			if (event->end.time.date.second < 0 || 60 < event->end.time.date.second)
				event->end.time.date.second = 0;
		}

		/* check end < start; convert long long int */
		slli = cal_time_convert_itol(NULL,
				event->start.time.date.year, event->start.time.date.month, event->start.time.date.mday,
				event->start.time.date.hour, event->start.time.date.minute, event->start.time.date.second);
		elli = cal_time_convert_itol(NULL,
				event->end.time.date.year, event->end.time.date.month, event->end.time.date.mday,
				event->end.time.date.hour, event->end.time.date.minute, event->end.time.date.second);

		if (1 < slli - elli) {
			/* 1 is to ignore milliseconds */
			ERR("allday end(%lld) < start(%lld)", elli, slli);
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
		break;
	}

	return CALENDAR_ERROR_NONE;
}

GList* cal_db_event_get_list_with_uid(char *uid, int parent_id)
{
	int ret = 0;
	RETV_IF(NULL == uid, NULL);
	RETV_IF('\0' == *uid, NULL);

	char query[CAL_DB_SQL_MAX_LEN] = {0};
	snprintf(query, sizeof(query), "SELECT id FROM %s "
			"WHERE original_event_id=-1 AND uid LIKE '%s' AND id!=%d",
			CAL_TABLE_SCHEDULE, uid, parent_id);

	sqlite3_stmt *stmt = NULL;
	ret = cal_db_util_query_prepare(query, &stmt);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_query_prepare() Fail(%d)", ret);
		SECURE("query[%s]", query);
		return NULL;
	}

	GList *l = NULL;
	while (CAL_SQLITE_ROW == cal_db_util_stmt_step(stmt)) {
		int id = sqlite3_column_int(stmt, 0);
		l = g_list_append(l, GINT_TO_POINTER(id));
	}
	sqlite3_finalize(stmt);
	return l;
}

void cal_db_event_update_child_origina_event_id(int child_id, int parent_id)
{
	CAL_FN_CALL();

	char query[CAL_DB_SQL_MAX_LEN] = {0};
	snprintf(query, sizeof(query), "UPDATE %s SET original_event_id=%d WHERE id=%d",
			CAL_TABLE_SCHEDULE, parent_id, child_id);
	int ret = 0;
	ret = cal_db_util_query_exec(query);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_query_exec() Fail(%d)", ret);
		SECURE("[%s]", query);
	}
}

char* cal_db_event_get_recurrence_id_from_exception(int child_id)
{
	int ret = 0;
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	snprintf(query, sizeof(query), "SELECT recurrence_id FROM %s WHERE id=%d",
			CAL_TABLE_SCHEDULE, child_id);

	sqlite3_stmt *stmt = NULL;
	ret = cal_db_util_query_prepare(query, &stmt);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_query_prepare() Fail(%d)", ret);
		SECURE("query[%s]", query);
		return NULL;
	}

	char *recurrence_id = NULL;
	if (CAL_SQLITE_ROW == cal_db_util_stmt_step(stmt)) {
		recurrence_id = cal_strdup((const char*)sqlite3_column_text(stmt, 0));
	}
	sqlite3_finalize(stmt);
	return recurrence_id;
}
static void __get_tzid_and_range(char *p, char **out_tzid, int *out_range)
{
	RET_IF(NULL == p);
	RET_IF('\0' == *p);
	RET_IF(NULL == out_tzid);
	RET_IF(NULL == out_range);

	char **s = NULL;
	s = g_strsplit(p, ";", -1);
	RETM_IF(NULL == s, "g_strsplit() Fail");

	int count = g_strv_length(s);
	int i;
	char *tzid = NULL;
	int range = CAL_RECURRENCE_ID_RANGE_NONE;
	for (i = 0; i < count; i++) {
		if (NULL == s[i] || '\0' == *s[i]) continue;

		if (CAL_STRING_EQUAL == strncmp(s[i], "TZID=", strlen("TZID="))) {
			tzid = strdup(s[i] + strlen("TZID="));
			DBG("tzid [%s]", tzid);
		}
		else if (CAL_STRING_EQUAL == strncmp(s[i], "RANGE=", strlen("RANGE="))) {
			char *param = s[i] + strlen("RANGE=");
			if (CAL_STRING_EQUAL == strncmp(param, "THISANDFUTURE", strlen("THISANDFUTURE")))
				range = CAL_RECURRENCE_ID_RANGE_THISANDFUTURE;
			else if (CAL_STRING_EQUAL == strncmp(param, "THISANDPRIOR", strlen("THISANDPRIOR")))
				range = CAL_RECURRENCE_ID_RANGE_THISANDPRIOR;
			else
				ERR("Invalid param[%s]", s[i]);
		}
		else {
			ERR("Invalid param[%s]", s[i]);
		}
	}
	*out_tzid = tzid;
	*out_range = range;
	g_strfreev(s);
}
static void cal_db_event_apply_recurrence_id_child(int child_id, cal_event_s *event, calendar_time_s until, bool is_prior)
{
	int ret = 0;
	calendar_record_h record = NULL;
	ret = cal_db_get_record(_calendar_event._uri, child_id, &record);
	RETM_IF(CALENDAR_ERROR_NONE != ret, "cal_db_get_record() Fail(%d)", ret);

	if (true == is_prior) {
		cal_record_set_caltime(record, _calendar_event.start_time, event->start);
		cal_record_set_caltime(record, _calendar_event.end_time, event->end);
		switch (event->start.type) {
		case CALENDAR_TIME_UTIME:
			DBG("dtstart(%lld) dtend(%lld)", event->start.time.utime, event->end.time.utime);
			break;
		case CALENDAR_TIME_LOCALTIME:
			DBG("dtstart(%04d-%02d-%02dT%02d:%02d:%02d) dtend(%04d-%02d-%02dT%02d:%02d:%02d)",
					event->start.time.date.year, event->start.time.date.month, event->start.time.date.mday,
					event->start.time.date.hour, event->start.time.date.minute, event->start.time.date.second,
					event->end.time.date.year, event->end.time.date.month, event->end.time.date.mday,
					event->end.time.date.hour, event->end.time.date.minute, event->end.time.date.second);
			break;
		}
	}

	cal_record_set_int(record, _calendar_event.freq, event->freq);
	cal_record_set_int(record, _calendar_event.interval, event->interval);
	cal_record_set_int(record, _calendar_event.wkst, event->wkst);
	if (event->byyearday && *event->byyearday)
		cal_record_set_str(record, _calendar_event.byyearday, event->byyearday);
	if (event->byweekno && *event->byweekno)
		cal_record_set_str(record, _calendar_event.byweekno, event->byweekno);
	if (event->bymonth && *event->bymonth)
		cal_record_set_str(record, _calendar_event.bymonth, event->bymonth);
	if (event->bymonthday && *event->bymonthday)
		cal_record_set_str(record, _calendar_event.bymonthday, event->bymonthday);
	if (event->byday && *event->byday)
		cal_record_set_str(record, _calendar_event.byday, event->byday);
	if (event->bysetpos && *event->bysetpos)
		cal_record_set_str(record, _calendar_event.bysetpos, event->bysetpos);

	/* until */
	cal_record_set_int(record, _calendar_event.range_type, CALENDAR_RANGE_UNTIL);
	cal_record_set_caltime(record, _calendar_event.until_time, until);

	/* reset */
	cal_record_set_str(record, _calendar_event.uid, "");
	cal_record_set_int(record, _calendar_event.original_event_id, -1);


	calendar_db_update_record(record);
	calendar_record_destroy(record, true);
}
static void __get_next_instance_caltime(int parent_id, calendar_time_s *caltime, calendar_time_s *dtstart, calendar_time_s *dtend)
{
	int ret = 0;
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	sqlite3_stmt *stmt = NULL;

	switch (caltime->type) {
	case CALENDAR_TIME_UTIME:
		snprintf(query, sizeof(query), "SELECT dtstart_utime, dtend_utime FROM %s WHERE event_id=%d AND dtstart_utime>%lld "
				"ORDER BY dtstart_utime ASC LIMIT 1",
				CAL_TABLE_NORMAL_INSTANCE, parent_id, caltime->time.utime);

		ret = cal_db_util_query_prepare(query, &stmt);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_db_util_query_prepare() Fail(%d)", ret);
			SECURE("query[%s]", query);
			return;
		}

		while (CAL_SQLITE_ROW == cal_db_util_stmt_step(stmt)) {
			dtstart->type = CALENDAR_TIME_UTIME;
			dtstart->time.utime = sqlite3_column_int64(stmt, 0);
			dtend->type = CALENDAR_TIME_UTIME;
			dtend->time.utime = sqlite3_column_int64(stmt, 0);
			break;
		}
		break;
	case CALENDAR_TIME_LOCALTIME:
		snprintf(query, sizeof(query), "SELECT dtstart_datetime, dtend_datetime FROM %s "
				"WHERE event_id=%d AND dtstart_datetime>'%04d-%02d-%02dT%02d:%02d:%02d' "
				"ORDER BY dtstart_datetime ASC LIMIT 1",
				CAL_TABLE_ALLDAY_INSTANCE, parent_id,
				caltime->time.date.year, caltime->time.date.month, caltime->time.date.mday,
				caltime->time.date.hour, caltime->time.date.minute, caltime->time.date.second);

		ret = cal_db_util_query_prepare(query, &stmt);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_db_util_query_prepare() Fail(%d)", ret);
			SECURE("query[%s]", query);
			return;
		}

		if (CAL_SQLITE_ROW == cal_db_util_stmt_step(stmt)) {
			char *temp = NULL;
			dtstart->type = CALENDAR_TIME_LOCALTIME;
			temp = sqlite3_column_text(stmt, 0);
			if (temp && *temp) {
				sscanf(temp, CAL_FORMAT_LOCAL_DATETIME,
						&(dtstart->time.date.year), &(dtstart->time.date.month), &(dtstart->time.date.mday),
						&(dtstart->time.date.hour), &(dtstart->time.date.minute), &(dtstart->time.date.second));
			}
			dtend->type = CALENDAR_TIME_LOCALTIME;
			temp = sqlite3_column_text(stmt, 1);
			if (temp && *temp) {
				sscanf(temp, CAL_FORMAT_LOCAL_DATETIME,
						&(dtend->time.date.year), &(dtend->time.date.month), &(dtend->time.date.mday),
						&(dtend->time.date.hour), &(dtend->time.date.minute), &(dtend->time.date.second));
			}
		}
		break;
	}
	sqlite3_finalize(stmt);
}
static void __get_last_instance_caltime(int parent_id, int type, calendar_time_s *dtstart)
{
	int ret = 0;
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	sqlite3_stmt *stmt = NULL;

	switch (type) {
	case CALENDAR_TIME_UTIME:
		snprintf(query, sizeof(query), "SELECT dtstart_utime FROM %s WHERE event_id=%d "
				"ORDER BY dtstart_utime DESC LIMIT 1",
				CAL_TABLE_NORMAL_INSTANCE, parent_id);
		ret = cal_db_util_query_prepare(query, &stmt);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_db_util_query_prepare() Fail(%d)", ret);
			SECURE("query[%s]", query);
			return;
		}
		while (CAL_SQLITE_ROW == cal_db_util_stmt_step(stmt)) {
			dtstart->type = CALENDAR_TIME_UTIME;
			dtstart->time.utime = sqlite3_column_int64(stmt, 0);
			break;
		}
		break;
	case CALENDAR_TIME_LOCALTIME:
		snprintf(query, sizeof(query), "SELECT dtstart_datetime FROM %s WHERE event_id=%d "
				"ORDER BY dtstart_datetime DESC LIMIT 1",
				CAL_TABLE_ALLDAY_INSTANCE, parent_id);
		ret = cal_db_util_query_prepare(query, &stmt);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_db_util_query_prepare() Fail(%d)", ret);
			SECURE("query[%s]", query);
			return;
		}
		if (CAL_SQLITE_ROW == cal_db_util_stmt_step(stmt)) {
			char *temp = NULL;
			dtstart->type = CALENDAR_TIME_LOCALTIME;
			temp = sqlite3_column_text(stmt, 0);
			if (temp && *temp) {
				sscanf(temp, CAL_FORMAT_LOCAL_DATETIME,
						&(dtstart->time.date.year), &(dtstart->time.date.month), &(dtstart->time.date.mday),
						&(dtstart->time.date.hour), &(dtstart->time.date.minute), &(dtstart->time.date.second));
			}
			else {
				ERR("datetime is NULL");
			}
		}
		break;
	}
	sqlite3_finalize(stmt);
}
static void __del_recurence_id_instance(calendar_time_s *rectime, int parent_id)
{
	int ret = 0;
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	switch (rectime->type) {
	case CALENDAR_TIME_UTIME:
		snprintf(query, sizeof(query), "DELETE FROM %s WHERE dtstart_utime=%lld AND event_id=%d",
				CAL_TABLE_NORMAL_INSTANCE, rectime->time.utime, parent_id);
		break;
	case CALENDAR_TIME_LOCALTIME:
		snprintf(query, sizeof(query), "DELETE FROM %s WHERE dtstart_datetime='%04d-%02d-%02dT%02d:%02d:%02d' AND event_id=%d",
				CAL_TABLE_ALLDAY_INSTANCE, rectime->time.date.year, rectime->time.date.month, rectime->time.date.mday,
				rectime->time.date.hour, rectime->time.date.minute, rectime->time.date.second, parent_id);
		break;
	}
	ret = cal_db_util_query_exec(query);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_query_exec() Fail(%d)", ret);
		SECURE("[%s]", query);
		return;
	}

	int y = 0, m = 0, d = 0;
	int h = 0, n = 0, s = 0;
	switch (rectime->type) {
	case CALENDAR_TIME_UTIME:
		cal_time_get_datetime(rectime->time.utime, &y, &m, &d, &h, &n, &s);
		DBG("[DELETED] %04d-%02d-%02dT%02d:%02d:%02d (utime)", y, m, d, h, n, s);
		break;
	case CALENDAR_TIME_LOCALTIME:
		DBG("[DELETED] %04d-%02d-%02dT%02d:%02d:%02d (local)",
				rectime->time.date.year, rectime->time.date.month, rectime->time.date.mday,
				rectime->time.date.hour, rectime->time.date.minute, rectime->time.date.second);
		break;
	}
}

static void __set_original_event_id_in_child(int child_id, int parent_id)
{
	int ret = 0;
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	snprintf(query, sizeof(query), "UPDATE %s SET original_event_id=%d WHERE id=%d",
			CAL_TABLE_SCHEDULE, parent_id, child_id);

	ret = cal_db_util_query_exec(query);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_query_exec() Fail(%d)", ret);
		SECURE("[%s]", query);
	}
}

/*
 * RECURRENCE-ID;VALUE=DATE:19960401
 * RECURRENCE-ID;RANGE=THISANDFUTURE:19960120T120000Z
 * RECURRENCE-ID;RANGE=THISANDPRIOR:19980401T133000Z
 * RECURRENCE-ID;TZID=Asia/Seoul:20150106T090000
 */
void cal_db_event_apply_recurrence_id(int parent_id, cal_event_s *event, char *recurrence_id, int child_id)
{
	CAL_FN_CALL();

	RET_IF(NULL == recurrence_id);
	RET_IF('\0' == *recurrence_id);

	char **t = NULL;
	t =  g_strsplit(recurrence_id, ":", -1);
	RETM_IF(NULL == t, "g_strsplit() Fail");

	if ('\0' == *t[0]) { /* no param */
		g_strfreev(t);
		return;
	}
	int count = g_strv_length(t);
	int len_param = strlen(t[count -1]);
	*(recurrence_id + strlen(recurrence_id) - len_param -1) = '\0';
	g_strfreev(t);

	char *datetime = recurrence_id + strlen(recurrence_id) +1;
	int len_datetime = strlen(datetime);
	DBG("datetime[%s]", datetime);

	char *tzid = NULL;
	int range = CAL_RECURRENCE_ID_RANGE_NONE;
	__get_tzid_and_range(recurrence_id, &tzid, &range);

	int y = 0, m = 0, d = 0;
	int h = 0, n = 0, s = 0;
	char dtstart_datetime[CAL_STR_SHORT_LEN32] = {0};
	long long int dtstart_utime = 0;
	calendar_time_s rectime = {0};
	switch (len_datetime) {
	case 8:
		sscanf(datetime, "%04d%02d%02d", &y, &m, &d);
		snprintf(dtstart_datetime, sizeof(dtstart_datetime), CAL_FORMAT_LOCAL_DATETIME, y, m, d, 0, 0, 0);
		rectime.type = CALENDAR_TIME_LOCALTIME;
		rectime.time.date.year = y;
		rectime.time.date.month = m;
		rectime.time.date.mday = d;
		rectime.time.date.hour = 0;
		rectime.time.date.minute = 0;
		rectime.time.date.second = 0;
		break;
	case 15:
		sscanf(datetime, "%04d%02d%02dT%02d%02d%02d", &y, &m, &d, &h, &n, &s);
		if (tzid && *tzid) {
			dtstart_utime = cal_time_convert_itol(tzid, y, m, d, h, n, s);
			rectime.type = CALENDAR_TIME_UTIME;
			rectime.time.utime = dtstart_utime;
		}
		else {
			snprintf(dtstart_datetime, sizeof(dtstart_datetime), CAL_FORMAT_LOCAL_DATETIME, y, m, d, h, n, s);
			rectime.type = CALENDAR_TIME_LOCALTIME;
			rectime.time.date.year = y;
			rectime.time.date.month = m;
			rectime.time.date.mday = d;
			rectime.time.date.hour = h;
			rectime.time.date.minute = n;
			rectime.time.date.second = s;
		}
		break;
	case 16:
		sscanf(datetime, "%04d%02d%02dT%02d%02d%02dZ", &y, &m, &d, &h, &n, &s);
		dtstart_utime = cal_time_convert_itol(tzid, y, m, d, h, n, s);
		rectime.type = CALENDAR_TIME_UTIME;
		rectime.time.utime = dtstart_utime;
		break;
	}
	free(tzid);

	calendar_time_s until = {0};
	calendar_time_s dtstart = {0};
	calendar_time_s dtend = {0};
	calendar_record_h record = (calendar_record_h)event;
	switch (range) {
	case CAL_RECURRENCE_ID_RANGE_THISANDFUTURE:
		DBG("update child");
		switch (event->range_type) {
		case CALENDAR_RANGE_UNTIL:
		case CALENDAR_RANGE_NONE:
			until = event->start;
			break;
		case CALENDAR_RANGE_COUNT:
			__get_last_instance_caltime(parent_id, rectime.type, &until);
			break;
		}
		cal_db_event_apply_recurrence_id_child(child_id, event, until, false);

		DBG("update parent");
		until = rectime;
		cal_time_modify_caltime(&until, -1);
		cal_record_set_int(record, _calendar_event.range_type, CALENDAR_RANGE_UNTIL);
		cal_record_set_caltime(record, _calendar_event.until_time, until);
		cal_record_set_str(record, _calendar_event.recurrence_id, "");
		cal_record_set_str(record, _calendar_event.uid, "");
		calendar_db_update_record(record);
		break;
	case CAL_RECURRENCE_ID_RANGE_THISANDPRIOR:
		DBG("update child");
		until = rectime;
		cal_db_event_apply_recurrence_id_child(child_id, event, until, true);

		DBG("update parent");
		__get_next_instance_caltime(parent_id, &until, &dtstart, &dtend);
		cal_record_set_caltime(record, _calendar_event.start_time, dtstart);
		cal_record_set_caltime(record, _calendar_event.end_time, dtend);
		switch (event->range_type) {
		case CALENDAR_RANGE_UNTIL:
		case CALENDAR_RANGE_NONE:
			break;
		case CALENDAR_RANGE_COUNT:
			__get_last_instance_caltime(parent_id, rectime.type, &until);
			cal_record_set_int(record, _calendar_event.range_type, CALENDAR_RANGE_UNTIL);
			cal_record_set_caltime(record, _calendar_event.until_time, until);
			DBG(CAL_FORMAT_LOCAL_DATETIME,
					until.time.date.year, until.time.date.month, until.time.date.mday,
					until.time.date.hour, until.time.date.minute, until.time.date.second);
			break;
		}
		cal_record_set_str(record, _calendar_event.recurrence_id, "");
		calendar_db_update_record(record);
		break;
	default:
		__del_recurence_id_instance(&rectime, parent_id);
		__set_original_event_id_in_child(child_id, parent_id);
		break;
	}
}
static int __get_parent_id_with_uid(char *uid, int child_id)
{
	int ret = 0;
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	int parent_id = -1;
	snprintf(query, sizeof(query), "SELECT id FROM %s WHERE original_event_id=-1 AND id!=%d AND uid='%s'",
			CAL_TABLE_SCHEDULE, child_id, uid);

	sqlite3_stmt *stmt = NULL;
	ret = cal_db_util_query_prepare(query, &stmt);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_query_prepare() Fail(%d)", ret);
		SECURE("query[%s]", query);
		return ret;
	}
	if (CAL_SQLITE_ROW == cal_db_util_stmt_step(stmt)) {
		parent_id = sqlite3_column_int64(stmt, 0);
	}
	sqlite3_finalize(stmt);
	DBG("found parent_id(%d)", parent_id);
	return parent_id;
}

int cal_db_event_insert_record(calendar_record_h record, int original_event_id, int *id)
{
	int ret = -1;
	int event_id = -1;
	int index;
	int input_ver;
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	char dtstart_datetime[CAL_STR_SHORT_LEN32] = {0};
	char dtend_datetime[CAL_STR_SHORT_LEN32] = {0};
	sqlite3_stmt *stmt = NULL;
	cal_event_s* event =  (cal_event_s*)(record);
	cal_rrule_s *rrule = NULL;
	int tmp = 0;
	int calendar_book_id = 0;
	calendar_record_h record_calendar = NULL;
	int has_alarm = 0;
	int timezone_id = 0;

	RETV_IF(NULL == event, CALENDAR_ERROR_INVALID_PARAMETER);

	ret = cal_db_event_check_value_validation(event);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_db_event_check_value_validation() failed");

	/* access control */
	if (cal_access_control_have_write_permission(event->calendar_id) == false) {
		ERR("cal_access_control_have_write_permission() failed");
		return CALENDAR_ERROR_PERMISSION_DENIED;
	}

	ret = calendar_record_get_int(record, _calendar_event.calendar_book_id, &calendar_book_id);
	DBG("calendar_book_id(%d)", calendar_book_id);

	ret = cal_db_get_record(_calendar_book._uri, calendar_book_id, &record_calendar);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, CALENDAR_ERROR_INVALID_PARAMETER, "calendar_book_id is invalid");

	calendar_record_destroy(record_calendar, true);

	has_alarm = cal_db_alarm_has_alarm(event->alarm_list);
	cal_time_get_timezone_from_table(event->start_tzid, NULL, &timezone_id);
	input_ver = cal_db_util_get_next_ver();
	int is_allday = 0;
	if (CALENDAR_TIME_LOCALTIME == event->start.type
			&& (0 == event->start.time.date.hour)
			&& (0 == event->start.time.date.minute)
			&& (0 == event->start.time.date.second)
			&& (0 == event->end.time.date.hour)
			&& (0 == event->end.time.date.minute)
			&& (0 == event->end.time.date.second)) {
		is_allday = 1;
	}

	snprintf(query, sizeof(query), "INSERT INTO %s ("
			"type, "
			"created_ver, changed_ver, "
			"summary, description, location, categories, exdate, "
			"task_status, priority, "
			"timezone, "
			"contact_id, busy_status, sensitivity, uid, "
			"organizer_name, organizer_email, meeting_status, "
			"calendar_id, "
			"original_event_id, "
			"latitude, longitude, "
			"email_id, "
			"created_time, completed_time, progress, "
			"dtstart_type, dtstart_utime, dtstart_datetime, dtstart_tzid, "
			"dtend_type, dtend_utime, dtend_datetime, dtend_tzid, "
			"last_mod, rrule_id, "
			"recurrence_id, rdate, has_attendee, "
			"has_alarm, system_type, updated, "
			"sync_data1, sync_data2, sync_data3, sync_data4,"
			"has_exception, has_extended, freq, is_allday "
			") VALUES ("
			"%d, "
			"%d, %d, "
			"?, ?, ?, ?, ?, "
			"%d, %d, "
			"%d, "
			"%d, %d, %d, ?, "
			"?, ?, %d, "
			"%d, "
			"%d, "
			"%lf, %lf, "
			"%d, "
			"strftime('%%s', 'now'), %lld, %d, "
			"%d, %lld, ?, ?, "
			"%d, %lld, ?, ?, "
			"strftime('%%s', 'now'), %d, "
			"?, ?, %d, %d, %d, %ld, "
			"?, ?, ?, ?, "
			"%d, %d, %d, %d) ",
		CAL_TABLE_SCHEDULE,
		CAL_SCH_TYPE_EVENT, /*event->cal_type,*/
		input_ver, input_ver,
		event->event_status, event->priority,
		event->timezone ? event->timezone : timezone_id,
		event->contact_id, event->busy_status, event->sensitivity,
		event->meeting_status,
		event->calendar_id,
		original_event_id,
		event->latitude, event->longitude,
		event->email_id,
		(long long int)0, 0, /* event->completed_time, event->progress */
		event->start.type, event->start.type == CALENDAR_TIME_UTIME ? event->start.time.utime : 0,
		event->end.type, event->end.type == CALENDAR_TIME_UTIME ? event->end.time.utime : 0,
		0 < event->freq ? 1 : 0,
		(0 < event->attendee_list->count) ? 1 : 0,
		has_alarm,
		event->system_type,
		event->updated,
		(0 < event->exception_list->count) ? 1 : 0,
		(0 < event->extended_list->count) ? 1 : 0,
		event->freq, is_allday);

	ret = cal_db_util_query_prepare(query, &stmt);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_query_prepare() Fail(%d)", ret);
		SECURE("query[%s]", query);
		return ret;
	}

	index = 1;

	if (event->summary)
		cal_db_util_stmt_bind_text(stmt, index, event->summary);
	index++;

	if (event->description)
		cal_db_util_stmt_bind_text(stmt, index, event->description);
	index++;

	if (event->location)
		cal_db_util_stmt_bind_text(stmt, index, event->location);
	index++;

	if (event->categories)
		cal_db_util_stmt_bind_text(stmt, index, event->categories);
	index++;

	if (event->exdate)
		cal_db_util_stmt_bind_text(stmt, index, event->exdate);
	index++;

	if (event->uid)
		cal_db_util_stmt_bind_text(stmt, index, event->uid);
	index++;

	if (event->organizer_name)
		cal_db_util_stmt_bind_text(stmt, index, event->organizer_name);
	index++;

	if (event->organizer_email)
		cal_db_util_stmt_bind_text(stmt, index, event->organizer_email);
	index++;

	if (CALENDAR_TIME_LOCALTIME == event->start.type) {
		snprintf(dtstart_datetime, sizeof(dtstart_datetime), CAL_FORMAT_LOCAL_DATETIME,
				event->start.time.date.year,
				event->start.time.date.month,
				event->start.time.date.mday,
				event->start.time.date.hour,
				event->start.time.date.minute,
				event->start.time.date.second);
		cal_db_util_stmt_bind_text(stmt, index, dtstart_datetime);
	}
	index++;

	if (event->start_tzid)
		cal_db_util_stmt_bind_text(stmt, index, event->start_tzid);
	index++;

	if (CALENDAR_TIME_LOCALTIME == event->end.type) {
		snprintf(dtend_datetime, sizeof(dtend_datetime), CAL_FORMAT_LOCAL_DATETIME,
				event->end.time.date.year,
				event->end.time.date.month,
				event->end.time.date.mday,
				event->end.time.date.hour,
				event->end.time.date.minute,
				event->end.time.date.second);
		cal_db_util_stmt_bind_text(stmt, index, dtend_datetime);
	}
	index++;

	if (event->end_tzid)
		cal_db_util_stmt_bind_text(stmt, index, event->end_tzid);
	index++;

	if (event->recurrence_id)
		cal_db_util_stmt_bind_text(stmt, index, event->recurrence_id);
	index++;

	if (event->rdate)
		cal_db_util_stmt_bind_text(stmt, index, event->rdate);
	index++;

	if (event->sync_data1)
		cal_db_util_stmt_bind_text(stmt, index, event->sync_data1);
	index++;

	if (event->sync_data2)
		cal_db_util_stmt_bind_text(stmt, index, event->sync_data2);
	index++;

	if (event->sync_data3)
		cal_db_util_stmt_bind_text(stmt, index, event->sync_data3);
	index++;

	if (event->sync_data4)
		cal_db_util_stmt_bind_text(stmt, index, event->sync_data4);
	index++;

	ret = cal_db_util_stmt_step(stmt);
	sqlite3_finalize(stmt);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_stmt_step() Fail(%d)", ret);
		return ret;
	}
	event_id = cal_db_util_last_insert_id();

	/*
	 * update parent event changed ver in case this event is exception mod
	 * which is original_event_id > 0
	 */
	cal_db_event_update_original_event_version(original_event_id, input_ver);

	calendar_record_get_int(record, _calendar_event.id, &tmp);
	cal_record_set_int(record, _calendar_event.id, event_id);
	if (id) {
		*id = event_id;
	}

	cal_db_rrule_get_rrule_from_event(record, &rrule);
	if (rrule) {
		cal_db_rrule_insert_record(event_id, rrule);
		CAL_FREE(rrule);
	}
	if (0 < original_event_id) {
		cal_record_set_int(record, _calendar_event.original_event_id, original_event_id);
	}
	cal_db_instance_publish_record(record);

	while (event->uid && *event->uid) {
		if (NULL == event->recurrence_id || '\0' == *event->recurrence_id) {
			DBG("this is parent");
			/*
			 * parent exception event is inserted in case child exception existed already.
			 * find child exceptions and link(one exception) or devide(this and future/prior)
			 */

			GList *list = NULL;
			list = cal_db_event_get_list_with_uid(event->uid, event_id);
			if (NULL == list)
				break;
			GList *l = g_list_first(list);
			if (l) {
				int child_id = GPOINTER_TO_INT(l->data);
				/* update children original_event_id */
				cal_db_event_update_child_origina_event_id(child_id, event_id);
				char *recurrence_id = NULL;
				recurrence_id = cal_db_event_get_recurrence_id_from_exception(child_id);
				if (NULL == recurrence_id || '\0' == *recurrence_id) {
					if (recurrence_id) free(recurrence_id);
					l = g_list_next(l);
					continue;
				}
				/* remove parent instance */
				cal_db_event_apply_recurrence_id(event_id, event, recurrence_id, child_id);
				free(recurrence_id);
				l = g_list_next(l);
			}
			g_list_free(list);
		}
		else {
			DBG("this is child");
			/* get parent with uid and update original_event_id */
			int parent_id = 0;
			parent_id = __get_parent_id_with_uid(event->uid, event_id);
			if (parent_id < 0) {
				ERR("__get_parent_id_with_uid() Fail");
				break;
			}
			calendar_record_h parent = NULL;
			calendar_db_get_record(_calendar_event._uri, parent_id, &parent);
			cal_db_event_apply_recurrence_id(parent_id, (cal_event_s *)parent, event->recurrence_id, event_id);
			calendar_record_destroy(parent, true);
		}
		break;
	}

	if (event->alarm_list->count)	{
		DBG("insert alarm");
		ret = cal_db_alarm_insert_records(event->alarm_list, event_id);
		WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_db_alarm_insert_records() Fail(%d)", ret);
	}
	else {
		DBG("No alarm");
	}

	if (0 < event->attendee_list->count) {
		DBG("insert attendee");
		ret = cal_db_attendee_insert_records(event->attendee_list, event_id);
		WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_db_attendee_insert_records() Fail(%d)", ret);
	}
	else {
		DBG("No attendee");
	}

	if (original_event_id <= 0 && 0 < event->exception_list->count) {
		DBG("insert exception");
		ret = cal_db_event_insert_records(event->exception_list, event_id);
		WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_db_event_insert_records() Fail(%d)", ret);
	}
	else {
		DBG("No exception");
	}

	if (0 < event->extended_list->count) {
		DBG("insert extended");
		ret = cal_db_extended_insert_records(event->extended_list, event_id, CALENDAR_RECORD_TYPE_EVENT);
		WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_db_extended_insert_records() Fail(%d)", ret);
	}
	else {
		DBG("No extended");
	}

	cal_db_util_notify(CAL_NOTI_TYPE_EVENT);

	cal_record_set_int(record, _calendar_event.id, tmp);

	return CALENDAR_ERROR_NONE;

}

int cal_db_event_insert_records(cal_list_s *list_s, int original_event_id)
{
	int ret;
	int count = 0;
	calendar_record_h record = NULL;
	calendar_list_h list = (calendar_list_h)list_s;

	RETV_IF(NULL == list, CALENDAR_ERROR_INVALID_PARAMETER);

	calendar_list_get_count(list, &count);
	if (0 == count)
		return CALENDAR_ERROR_NONE;

	calendar_list_first(list);
	while (CALENDAR_ERROR_NONE == calendar_list_get_current_record_p(list, &record)) {
		ret = cal_db_event_insert_record(record, original_event_id, NULL);
		RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_db_extended_insert_record() Fail(%d)", ret);
		calendar_list_next(list);
	}
	return CALENDAR_ERROR_NONE;
}


