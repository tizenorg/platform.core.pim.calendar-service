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

#include "calendar_db.h"
#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_view.h"
#include "cal_time.h"
#include "cal_record.h"
#include "cal_list.h"

#include "cal_db_util.h"
#include "cal_db.h"
#include "cal_db_rrule.h"
#include "cal_db_query.h"
#include "cal_db_alarm.h"
#include "cal_db_instance.h"
#include "cal_db_attendee.h"
#include "cal_db_extended.h"
#include "cal_db_event.h"
#include "cal_access_control.h"

static int __cal_db_event_insert_record(calendar_record_h record, int* id);
static int __cal_db_event_get_record(int id, calendar_record_h* out_record);
static int __cal_db_event_update_record(calendar_record_h record);
static int __cal_db_event_delete_record(int id);
static int __cal_db_event_get_all_records(int offset, int limit, calendar_list_h* out_list);
static int __cal_db_event_get_records_with_query(calendar_query_h query, int offset, int limit, calendar_list_h* out_list);
static int __cal_db_event_insert_records(const calendar_list_h list, int** ids);
static int __cal_db_event_update_records(const calendar_list_h list);
static int __cal_db_event_delete_records(int ids[], int count);
static int __cal_db_event_get_count(int *out_count);
static int __cal_db_event_get_count_with_query(calendar_query_h query, int *out_count);
static int __cal_db_event_replace_record(calendar_record_h record, int id);
static int __cal_db_event_replace_records(const calendar_list_h list, int ids[], int count);
/*
 * static function
 */
static void __cal_db_event_get_stmt(sqlite3_stmt *stmt,bool is_view_table,calendar_record_h record,
		int *exception, int *extended);
static void __cal_db_event_get_property_stmt(sqlite3_stmt *stmt,
		unsigned int property, int *stmt_count, calendar_record_h record);
static void __cal_db_event_get_projection_stmt(sqlite3_stmt *stmt,
		const unsigned int *projection, const int projection_count,
		calendar_record_h record);
static int __cal_db_event_update_dirty(calendar_record_h record, int is_dirty_in_time);
/*  // under construction
	 bool __cal_db_event_check_changed_rrule(record);
 */
static int __cal_db_event_exception_get_records(int original_id, cal_list_s *list);
static int __cal_db_event_exception_delete_with_id(int original_id);
static int __cal_db_event_exception_get_ids(int original_id, GList **out_list);
static int __cal_db_event_exception_update(cal_list_s *exception_list_s, int original_id, int calendar_id, int is_dirty_in_time, time_t time_diff, int old_type, int new_type);
static int __cal_db_event_get_deleted_data(int id, int* calendar_book_id, int* created_ver, int* original_event_id, char** recurrence_id);
static int __cal_db_event_exdate_insert_normal(int event_id, const char* original_exdate, const char* exdate, int **exception_id, int *exception_len);
static bool __cal_db_event_check_calendar_book_type(calendar_record_h record);

static void __check_list(calendar_list_h l)
{
	DBG("---------------------");
	calendar_list_first(l);
	int count = 0;
	calendar_list_get_count(l, &count);
	DBG("count(%d)", count);

	int i;
	for (i = 0; i < count; i++) {
		calendar_record_h event = NULL;
		calendar_list_get_current_record_p(l, &event);
		int id = 0;
		calendar_record_get_int(event, _calendar_event.id, &id);
		int book_id = 0;
		calendar_record_get_int(event, _calendar_event.calendar_book_id, &book_id);
		char *summary = NULL;
		calendar_record_get_str_p(event, _calendar_event.summary, &summary);
		int freq = 0;
		calendar_record_get_int(event, _calendar_event.freq, &freq);

		DBG("id(%d) book_id(%d) summary[%s] freq(%d)", id, book_id, summary, freq);
		calendar_list_next(l);
	}
}

cal_db_plugin_cb_s _cal_db_event_plugin_cb = {
	.is_query_only = false,
	.insert_record = __cal_db_event_insert_record,
	.get_record = __cal_db_event_get_record,
	.update_record = __cal_db_event_update_record,
	.delete_record = __cal_db_event_delete_record,
	.get_all_records = __cal_db_event_get_all_records,
	.get_records_with_query = __cal_db_event_get_records_with_query,
	.insert_records = __cal_db_event_insert_records,
	.update_records = __cal_db_event_update_records,
	.delete_records = __cal_db_event_delete_records,
	.get_count = __cal_db_event_get_count,
	.get_count_with_query = __cal_db_event_get_count_with_query,
	.replace_record = __cal_db_event_replace_record,
	.replace_records = __cal_db_event_replace_records
};

static int __cal_db_event_insert_record(calendar_record_h record, int* id)
{
	RETV_IF(NULL == record, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(false == __cal_db_event_check_calendar_book_type(record), CALENDAR_ERROR_INVALID_PARAMETER);
	cal_event_s *event = (cal_event_s *)record;
	return _cal_db_event_insert_record(record, event->original_event_id, id);
}

enum {
	DIRTY_INIT = -1,
	DIRTY_IN_OTHER = 0,
	DIRTY_IN_TIME,
};

static int __cal_db_event_get_record(int id, calendar_record_h* out_record)
{
	char query[CAL_DB_SQL_MAX_LEN];
	int ret = 0;
	sqlite3_stmt *stmt = NULL;
	cal_event_s *event = NULL;
	cal_rrule_s *rrule = NULL;
	cal_db_util_error_e dbret = CAL_DB_OK;
	int exception = 0, extended = 0;
	calendar_record_h record_calendar;
	calendar_book_sync_event_type_e sync_event_type = CALENDAR_BOOK_SYNC_EVENT_FOR_ME;

	ret = calendar_record_create( _calendar_event._uri ,out_record);
	if (ret != CALENDAR_ERROR_NONE)
	{
		ERR("calendar_record_create(%d)", ret);
		return CALENDAR_ERROR_OUT_OF_MEMORY;
	}

	event = (cal_event_s*)(*out_record);

	snprintf(query, sizeof(query),
			"SELECT "CAL_QUERY_SCHEDULE_A_ALL" FROM %s AS A "
			"WHERE id = %d AND (type = %d OR type = %d) AND calendar_id IN "
			"(select id from %s where deleted = 0)",
			CAL_TABLE_SCHEDULE,
			id, CALENDAR_BOOK_TYPE_EVENT, CALENDAR_BOOK_TYPE_NONE,
			CAL_TABLE_CALENDAR);
	stmt = _cal_db_util_query_prepare(query);
	if (NULL == stmt)
	{
		SEC_ERR("query[%s]", query);
		ERR("_cal_db_util_query_prepare() Failed");
		calendar_record_destroy(*out_record, true);
		*out_record = NULL;
		return CALENDAR_ERROR_DB_FAILED;
	}

	dbret = _cal_db_util_stmt_step(stmt);
	if (dbret != CAL_DB_ROW)
	{
		SEC_ERR("query[%s]", query);
		ERR("Failed to step stmt(%d)", dbret);
		sqlite3_finalize(stmt);
		calendar_record_destroy(*out_record, true);
		*out_record = NULL;
		switch (dbret)
		{
		case CAL_DB_DONE:
			ERR("Failed to find record(id:%d, ret:%d)", id, dbret);
			return CALENDAR_ERROR_DB_RECORD_NOT_FOUND;
		case CAL_DB_ERROR_NO_SPACE:
			return CALENDAR_ERROR_FILE_NO_SPACE;
		default:
			return CALENDAR_ERROR_DB_FAILED;
		}
	}

	__cal_db_event_get_stmt(stmt, false, *out_record, &exception, &extended);
	sqlite3_finalize(stmt);
	stmt = NULL;

	//check
	ret = _cal_db_get_record(_calendar_book._uri, event->calendar_id, &record_calendar);
	if (ret == CALENDAR_ERROR_NONE)
	{
		ret = calendar_record_get_int(record_calendar,
				_calendar_book.sync_event, (int *)&sync_event_type);
		calendar_record_destroy(record_calendar, true);
	}
	if (event->is_deleted == 1
			&& sync_event_type != CALENDAR_BOOK_SYNC_EVENT_FOR_EVERY_AND_REMAIN
		)
	{
		calendar_record_destroy(*out_record, true);
		*out_record = NULL;
		return CALENDAR_ERROR_DB_RECORD_NOT_FOUND;
	}

	if (_cal_db_rrule_get_rrule(event->index, &rrule) == CALENDAR_ERROR_NONE )
	{
		_cal_db_rrule_set_rrule_to_event(rrule, *out_record);
		CAL_FREE(rrule);
	}

	if (event->has_alarm == 1)
		_cal_db_alarm_get_records(event->index, event->alarm_list);

	if (event->has_attendee == 1)
		_cal_db_attendee_get_records(event->index, event->attendee_list);

	if (exception == 1)
		__cal_db_event_exception_get_records(event->index, event->exception_list);

	if (extended == 1)
		_cal_db_extended_get_records(event->index, CALENDAR_RECORD_TYPE_EVENT, event->extended_list);

	return CALENDAR_ERROR_NONE;
}

static int __is_dirty_in_time(calendar_record_h record)
{
	cal_record_s *rec = (cal_record_s *)record;
	RETV_IF(NULL == record, false);
	RETV_IF(NULL == rec->view_uri, false);

	int count = 0;
	const cal_property_info_s *info = _cal_view_get_property_info(rec->view_uri, &count);
	RETVM_IF(NULL == info, false, "_cal_view_get_property_info() is failed");

	int i;
	int is_dirty_in_time = DIRTY_IN_OTHER;
	for (i = 0; i < count; i++) {
		if (false == _cal_record_check_property_flag(record, info[i].property_id , CAL_PROPERTY_FLAG_DIRTY)) {
			continue;
		}
		switch (info[i].property_id) {
		case CAL_PROPERTY_EVENT_START:
		case CAL_PROPERTY_EVENT_END:
		case CAL_PROPERTY_EVENT_START_TZID:
		case CAL_PROPERTY_EVENT_END_TZID:
		case CAL_PROPERTY_EVENT_FREQ:
		case CAL_PROPERTY_EVENT_RANGE_TYPE:
		case CAL_PROPERTY_EVENT_UNTIL:
		case CAL_PROPERTY_EVENT_COUNT:
		case CAL_PROPERTY_EVENT_INTERVAL:
		case CAL_PROPERTY_EVENT_BYSECOND:
		case CAL_PROPERTY_EVENT_BYMINUTE:
		case CAL_PROPERTY_EVENT_BYHOUR:
		case CAL_PROPERTY_EVENT_BYDAY:
		case CAL_PROPERTY_EVENT_BYMONTHDAY:
		case CAL_PROPERTY_EVENT_BYYEARDAY:
		case CAL_PROPERTY_EVENT_BYWEEKNO:
		case CAL_PROPERTY_EVENT_BYMONTH:
		case CAL_PROPERTY_EVENT_BYSETPOS:
		case CAL_PROPERTY_EVENT_WKST:

			is_dirty_in_time = DIRTY_IN_TIME;
			break;
		}
		if (DIRTY_IN_OTHER != is_dirty_in_time) break;
	}
	DBG("%sirty in time", DIRTY_IN_OTHER == is_dirty_in_time ? "Not d" : "D");
	return is_dirty_in_time;
}

time_t __get_time_diff(char *old_tzid, calendar_time_s *old, char *new_tzid, calendar_time_s *new)
{
	RETV_IF(NULL == old, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == new, CALENDAR_ERROR_INVALID_PARAMETER);

	time_t diff = 0;
	switch (old->type)
	{
	case CALENDAR_TIME_UTIME:
		switch (new->type)
		{
		case CALENDAR_TIME_UTIME:
			DBG("%lld - %lld", old->time.utime, new->time.utime);
			diff = old->time.utime - new->time.utime;
			break;
		case CALENDAR_TIME_LOCALTIME:
			DBG("type is changed(%d) -> (%d)", old->type, new->type);
			diff = old->time.utime - _cal_time_convert_itol(new_tzid,
					new->time.date.year, new->time.date.month, new->time.date.mday,
					new->time.date.hour, new->time.date.minute, new->time.date.second);
			break;
		}
		break;
	case CALENDAR_TIME_LOCALTIME:
		switch (new->type)
		{
		case CALENDAR_TIME_UTIME:
			DBG("type is changed(%d) -> (%d)", old->type, new->type);
			diff = _cal_time_convert_itol(old_tzid,
					old->time.date.year, old->time.date.month, old->time.date.mday,
					old->time.date.hour, old->time.date.minute, old->time.date.second) - new->time.utime;
			break;
		case CALENDAR_TIME_LOCALTIME:
			diff = _cal_time_convert_itol(old_tzid,
					old->time.date.year, old->time.date.month, old->time.date.mday,
					old->time.date.hour, old->time.date.minute, old->time.date.second)
				- _cal_time_convert_itol(new_tzid, new->time.date.year, new->time.date.month, new->time.date.mday,
						new->time.date.hour, new->time.date.minute, new->time.date.second);
			break;
		}
		break;
	}
	DBG("-------------time diff(%ld)", diff);
	return diff;
}

static int __get_time_shifted_field(char *old_field, int old_type, int new_type, time_t time_diff, char **new_field)
{
	if (NULL == old_field || '\0' == *old_field) {
		return CALENDAR_ERROR_NONE;
	}

	gchar **t = NULL;
	t = g_strsplit_set(old_field, " ,", -1);
	RETVM_IF(NULL == t, CALENDAR_ERROR_DB_FAILED, "g_strsplit_set() is failed");

	int len_t = g_strv_length(t);

	int len_field = strlen(old_field);
	char *new= NULL;
	new= calloc(len_field + (len_t * 8) + 1, sizeof(char)); // add (len_t * 8) for YYYYMMDD -> YYYYMMDDTHHMMSSZ
	if (NULL == new) {
		ERR("calloc() is failed");
		g_strfreev(t);
		return CALENDAR_ERROR_DB_FAILED;
	}

	struct tm tm = {0};
	time_t tt = 0;

	int i;
	for (i = 0; i < len_t; i++) {
		int y = 0, m = 0, d = 0;
		int h = 0, n = 0, s = 0;
		switch (old_type)
		{
		case CALENDAR_TIME_UTIME:
			sscanf(t[i],  "%04d%02d%02dT%02d%02d%02dZ", &y, &m, &d, &h, &n, &s);
			break;

		case CALENDAR_TIME_LOCALTIME:
			switch (strlen(t[i]))
			{
			case 8: // YYYYMMDD
				sscanf(t[i],  "%04d%02d%02d", &y, &m, &d);
				break;

			case 15: //YYYYMMDDTHHMMSS
				sscanf(t[i],  "%04d%02d%02dT%02d%02d%02d", &y, &m, &d, &h, &n, &s);
				break;
			}
			break;
		} // ct.type end switch

		tm.tm_year = y - 1900;
		tm.tm_mon = m - 1;
		tm.tm_mday = d;
		tm.tm_hour = h;
		tm.tm_min = n;
		tm.tm_sec = s;

		char buf[32] = {0};
		switch (new_type)
		{
		case CALENDAR_TIME_UTIME:
			switch (strlen(t[i])) // check original type
			{
			case 8: // YYYYMMDD
			case 15: //YYYYMMDDTHHMMSS
				tt = timelocal(&tm) - time_diff;
				gmtime_r(&tt, &tm);
				snprintf(buf, sizeof(buf), "%s%04d%02d%02dT%02d%02d%02dZ",
						i == 0 ? "" : ",", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
						tm.tm_hour, tm.tm_min, tm.tm_sec);
				break;

			case 16: //YYYYMMDDTHHMMSSZ
				tt = timegm(&tm) - time_diff;
				gmtime_r(&tt, &tm);
				snprintf(buf, sizeof(buf), "%s%04d%02d%02dT%02d%02d%02dZ",
						i == 0 ? "" : ",", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
						tm.tm_hour, tm.tm_min, tm.tm_sec);
				break;
			}

			break;

		case CALENDAR_TIME_LOCALTIME:
			switch (strlen(t[i]))
			{
			case 8: // YYYYMMDD
				tt = timegm(&tm) - time_diff;
				gmtime_r(&tt, &tm);
				snprintf(buf, sizeof(buf), "%s%04d%02d%02d",
						i == 0 ? "" : ",", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
				break;

			case 15: //YYYYMMDDTHHMMSS
				tt = timegm(&tm) - time_diff;
				gmtime_r(&tt, &tm);
				snprintf(buf, sizeof(buf), "%s%04d%02d%02dT%02d%02d%02d",
						i == 0 ? "" : ",", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
						tm.tm_hour, tm.tm_min, tm.tm_sec);
				break;

			case 16: //YYYYMMDDTHHMMSSZ
				tt = timegm(&tm) - time_diff;
				localtime_r(&tt, &tm);
				snprintf(buf, sizeof(buf), "%s%04d%02d%02dT%02d%02d%02d",
						i == 0 ? "" : ",", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
						tm.tm_hour, tm.tm_min, tm.tm_sec);
				break;
			}
			break;
		}
		strcat(new, buf);
	}
	g_strfreev(t);

	if (new_field)
		*new_field = new;
	else
		free(new);
	return CALENDAR_ERROR_NONE;
}

static int __update_exdate(cal_event_s *rec, time_t time_diff)
{
	RETV_IF(NULL == rec, CALENDAR_ERROR_INVALID_PARAMETER);

	if (NULL == rec->exdate || '\0' == *rec->exdate)
		return CALENDAR_ERROR_NONE;
	if (0 == time_diff)
		return CALENDAR_ERROR_NONE;

	int ret;
	calendar_record_h db_record = NULL;
	ret = calendar_db_get_record(_calendar_event._uri, rec->index, &db_record);
	RETVM_IF(ret != CALENDAR_ERROR_NONE, ret, "calendar_db_get_record fail(%d)", ret);

	// get time diff
	char *db_tzid = NULL;
	calendar_record_get_str_p(db_record, _calendar_event.start_tzid, &db_tzid);
	calendar_time_s ct = {0};
	calendar_record_get_caltime(db_record, _calendar_event.start_time, &ct);

	char *new_exdate = NULL;
	__get_time_shifted_field(rec->exdate, ct.type, rec->start.type, time_diff, &new_exdate);
	free(rec->exdate);
	rec->exdate = new_exdate;
	DBG("new exdate[%s]", new_exdate);

	calendar_record_destroy(db_record, true);

	return CALENDAR_ERROR_NONE;
}

static int __update_recurrence_id(calendar_record_h exception, int old_type, int new_type, time_t time_diff)
{
	CAL_FN_CALL();

	cal_event_s *rec = (cal_event_s *)exception;
	RETV_IF(NULL == rec, CALENDAR_ERROR_INVALID_PARAMETER);

	if (NULL == rec->recurrence_id) {
		DBG("No recurrence_id");
		return CALENDAR_ERROR_NONE;
	}

	char *new_recurrence_id = NULL;
	__get_time_shifted_field(rec->recurrence_id, rec->start.type, new_type, time_diff, &new_recurrence_id);
	free(rec->recurrence_id);
	rec->recurrence_id = new_recurrence_id;
	DBG("new recurrence_id[%s]", new_recurrence_id);

	return CALENDAR_ERROR_NONE;
}

static int __update_record(calendar_record_h record, int is_dirty_in_time)
{
	int ret;
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	char dtstart_datetime[32] = {0};
	char dtend_datetime[32] = {0};
	sqlite3_stmt *stmt = NULL;
	cal_event_s* event =  (cal_event_s*)(record);
	cal_rrule_s *rrule = NULL;
	cal_db_util_error_e dbret = CAL_DB_OK;
	int has_alarm = 0;
	int timezone_id = 0;
	int input_ver = 0;

	RETV_IF(NULL == event, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(false == __cal_db_event_check_calendar_book_type(record), CALENDAR_ERROR_INVALID_PARAMETER);


	ret = _cal_db_event_check_value_validation(event);
	if (CALENDAR_ERROR_NONE != ret)
	{
		ERR("_cal_db_event_check_value_validation() failed");
		return ret;
	}

	// access control
	if (_cal_access_control_have_write_permission(event->calendar_id) == false)
	{
		ERR("_cal_access_control_have_write_permission() failed");
		return CALENDAR_ERROR_PERMISSION_DENIED;
	}

	if (event->common.properties_flags != NULL)
	{
		if (DIRTY_INIT == is_dirty_in_time) {
			is_dirty_in_time = __is_dirty_in_time(record);
		}
		return __cal_db_event_update_dirty(record, is_dirty_in_time);
	}
	int time_diff = 0;
	calendar_time_s ct = {0};
	if (DIRTY_IN_TIME == is_dirty_in_time) {
		calendar_record_h old_record = NULL;
		ret = calendar_db_get_record(_calendar_event._uri, event->index, &old_record);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("calendar_db_get_record fail(%d)", ret);
			return ret;
		}

		// get time diff
		char *old_tzid = NULL;
		calendar_record_get_str_p(old_record, _calendar_event.start_tzid, &old_tzid);
		calendar_record_get_caltime(old_record, _calendar_event.start_time, &ct);
		time_diff = __get_time_diff(old_tzid, &ct, event->start_tzid, &event->start);
		calendar_record_destroy(old_record, true);
	}

	has_alarm = _cal_db_alarm_has_alarm(event->alarm_list);
	_cal_time_get_timezone_from_table(event->start_tzid, NULL, &timezone_id);
	input_ver = _cal_db_util_get_next_ver();
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
	snprintf(query, sizeof(query), "UPDATE %s SET "
			"changed_ver = %d,"
			"type = %d,"
			"summary = ?,"
			"description = ?,"
			"location = ?,"
			"categories = ?,"
			"exdate = ?,"
			"task_status = %d,"
			"priority = %d,"
			"timezone = %d, "
			"contact_id = %d, "
			"busy_status = %d, "
			"sensitivity = %d, "
			"uid = ?, "
			"organizer_name = ?, "
			"organizer_email = ?, "
			"meeting_status = %d, "
			"calendar_id = %d, "
			"original_event_id = %d,"
			"latitude = %lf,"
			"longitude = %lf,"
			"email_id = %d,"
			"completed_time = %lld,"
			"progress = %d, "
			"dtstart_type = %d, "
			"dtstart_utime = %lld, "
			"dtstart_datetime = ?, "
			"dtstart_tzid = ?, "
			"dtend_type = %d, "
			"dtend_utime = %lld, "
			"dtend_datetime = ?, "
			"dtend_tzid = ?, "
			"last_mod = strftime('%%s', 'now'), "
			"rrule_id = %d, "
			"recurrence_id = ?, "
			"rdate = ?, "
			"has_attendee = %d, "
			"has_alarm = %d, "
			"system_type = %d, "
			"updated = %ld, "
			"sync_data1 = ?, "
			"sync_data2 = ?, "
			"sync_data3 = ?, "
			"sync_data4 = ?,"
			"has_exception = %d, "
			"has_extended = %d, "
			"freq = %d, "
			"is_allday = %d "
			"WHERE id = %d;",
		CAL_TABLE_SCHEDULE,
		input_ver,
		CAL_SCH_TYPE_EVENT,/*event->cal_type,*/
		event->event_status,
		event->priority,
		event->timezone ? event->timezone : timezone_id,
		event->contact_id,
		event->busy_status,
		event->sensitivity,
		event->meeting_status,
		event->calendar_id,
		event->original_event_id,
		event->latitude,
		event->longitude,
		event->email_id,
		(long long int)0,//event->completed_time,
		0,//event->progress,
		event->start.type,
		event->start.type == CALENDAR_TIME_UTIME ? event->start.time.utime : 0,
		event->end.type,
		event->end.type == CALENDAR_TIME_UTIME ? event->end.time.utime : 0,
		event->freq > 0 ? 1 : 0,
		(event->attendee_list && 0 < event->attendee_list->count)? 1: 0,
		has_alarm,
		event->system_type,
		event->updated,
		(event->exception_list && 0 < event->exception_list->count)? 1 : 0,
		(event->extended_list && 0 < event->extended_list->count)? 1 : 0,
		event->freq,
		is_allday,
		event->index);

	stmt = _cal_db_util_query_prepare(query);
	RETVM_IF(NULL == stmt, CALENDAR_ERROR_DB_FAILED, "_cal_db_util_query_prepare() Failed");

	int index = 1;

	if (event->summary)
		_cal_db_util_stmt_bind_text(stmt, index, event->summary);
	index++;

	if (event->description)
		_cal_db_util_stmt_bind_text(stmt, index, event->description);
	index++;

	if (event->location)
		_cal_db_util_stmt_bind_text(stmt, index, event->location);
	index++;

	if (event->categories)
		_cal_db_util_stmt_bind_text(stmt, index, event->categories);
	index++;

	if (event->exdate) {
		if (DIRTY_IN_TIME == is_dirty_in_time) {
			__update_exdate(event, time_diff);
		}
		_cal_db_util_stmt_bind_text(stmt, index, event->exdate);
	}
	index++;

	if (event->uid)
		_cal_db_util_stmt_bind_text(stmt, index, event->uid);
	index++;

	if (event->organizer_name)
		_cal_db_util_stmt_bind_text(stmt, index, event->organizer_name);
	index++;

	if (event->organizer_email)
		_cal_db_util_stmt_bind_text(stmt, index, event->organizer_email);
	index++;

	if (CALENDAR_TIME_LOCALTIME == event->start.type)
	{
		snprintf(dtstart_datetime, sizeof(dtstart_datetime), CAL_FORMAT_LOCAL_DATETIME,
				event->start.time.date.year,
				event->start.time.date.month,
				event->start.time.date.mday,
				event->start.time.date.hour,
				event->start.time.date.minute,
				event->start.time.date.second);
		_cal_db_util_stmt_bind_text(stmt, index, dtstart_datetime);
	}
	index++;

	if (event->start_tzid)
		_cal_db_util_stmt_bind_text(stmt, index, event->start_tzid);
	index++;

	if (CALENDAR_TIME_LOCALTIME == event->end.type)
	{
		snprintf(dtend_datetime, sizeof(dtend_datetime), CAL_FORMAT_LOCAL_DATETIME,
				event->end.time.date.year,
				event->end.time.date.month,
				event->end.time.date.mday,
				event->end.time.date.hour,
				event->end.time.date.minute,
				event->end.time.date.second);
		_cal_db_util_stmt_bind_text(stmt, index, dtend_datetime);
	}
	index++;

	if (event->end_tzid)
		_cal_db_util_stmt_bind_text(stmt, index, event->end_tzid);
	index++;

	if (event->recurrence_id)
		_cal_db_util_stmt_bind_text(stmt, index, event->recurrence_id);
	index++;
	if (event->rdate)
		_cal_db_util_stmt_bind_text(stmt, index, event->rdate);
	index++;
	if (event->sync_data1)
		_cal_db_util_stmt_bind_text(stmt, index, event->sync_data1);
	index++;
	if (event->sync_data2)
		_cal_db_util_stmt_bind_text(stmt, index, event->sync_data2);
	index++;
	if (event->sync_data3)
		_cal_db_util_stmt_bind_text(stmt, index, event->sync_data3);
	index++;
	if (event->sync_data4)
		_cal_db_util_stmt_bind_text(stmt, index, event->sync_data4);
	index++;

	dbret = _cal_db_util_stmt_step(stmt);
	if (CAL_DB_DONE != dbret) {
		sqlite3_finalize(stmt);
		ERR("sqlite3_step() Failed(%d)", dbret);
		switch (dbret)
		{
		case CAL_DB_ERROR_NO_SPACE:
			return CALENDAR_ERROR_FILE_NO_SPACE;
		default:
			return CALENDAR_ERROR_DB_FAILED;
		}
	}
	sqlite3_finalize(stmt);

	// update parent event changed ver in case this event is exception mod
	// which is original_event_id > 0
	_cal_db_event_update_original_event_version(event->original_event_id, input_ver);

//	_cal_db_rrule_set_default(record);
	_cal_db_rrule_get_rrule_from_event(record, &rrule);
	_cal_db_rrule_update_record(event->index, rrule); // if rrule turns none, update 0.
	CAL_FREE(rrule);

	if (DIRTY_IN_TIME == is_dirty_in_time) {
		_cal_db_instance_discard_record(event->index);
		_cal_db_instance_publish_record(record);

	} else {
		// check exdate
		_cal_db_instance_update_exdate_del(event->index, event->exdate);
	}

	while ((event->uid && *event->uid) && (NULL == event->recurrence_id || '\0' == *event->recurrence_id )) {
		// check if exception mod has. recurrence_id
		GList *list = NULL;
		list = _cal_db_event_get_list_with_uid(event->uid, event->index);
		if (NULL == list)
			break;
		GList *l = g_list_first(list);
		if (l) {
			int child_id = GPOINTER_TO_INT(l->data);
			// update children original_event_id
			_cal_db_event_update_child_origina_event_id(child_id, event->index);
			char *recurrence_id = NULL;
			recurrence_id = _cal_db_event_get_recurrence_id_from_exception(child_id);
			if (recurrence_id) {
				// remove parent instance
				_cal_db_event_apply_recurrence_id(event->index, event, recurrence_id, child_id);
				free(recurrence_id);
			}
			l = g_list_next(l);
		}
		g_list_free(list);
		break;
	}

	_cal_db_alarm_delete_with_id(event->index);
	_cal_db_attendee_delete_with_id(event->index);
	_cal_db_extended_delete_with_id(event->index, CALENDAR_RECORD_TYPE_EVENT);

	if (event->alarm_list && 0 < event->alarm_list->count) {
		ret = _cal_db_alarm_insert_records(event->alarm_list, event->index);
		WARN_IF(CALENDAR_ERROR_NONE != ret, "_cal_db_alarm_insert_records() failed(%x)", ret);
	}

	if (event->attendee_list && 0 < event->attendee_list->count) {
		ret = _cal_db_attendee_insert_records(event->attendee_list, event->index);
		WARN_IF(CALENDAR_ERROR_NONE != ret, "_cal_db_attendee_insert_records() failed(%x)", ret);
	}

	if (event->exception_list && 0 < event->exception_list->count) {
		ret = __cal_db_event_exception_update(event->exception_list, event->index, event->calendar_id, is_dirty_in_time, time_diff, ct.type, event->start.type);
		WARN_IF(CALENDAR_ERROR_NONE != ret, "__cal_db_event_exception_update() Failed(%d)", ret);
	}

	if (event->extended_list && 0 < event->extended_list->count) {
		ret = _cal_db_extended_insert_records(event->extended_list, event->index, CALENDAR_RECORD_TYPE_EVENT);
		WARN_IF(CALENDAR_ERROR_NONE != ret, "_cal_db_extended_insert_records() Failed(%d)", ret);
	}

	_cal_db_util_notify(CAL_NOTI_TYPE_EVENT);

	return CALENDAR_ERROR_NONE;
}

static int __cal_db_event_update_record(calendar_record_h record)
{
	return __update_record(record, DIRTY_INIT);
}

static int __cal_db_event_add_exdate(int original_event_id, char* recurrence_id)
{
	char query[CAL_DB_SQL_MAX_LEN];
	sqlite3_stmt *stmt = NULL;
	cal_db_util_error_e dbret = CAL_DB_OK;

	if (original_event_id < 0)
	{
		return CALENDAR_ERROR_NONE;
	}
	DBG("This is exception mod event");
	if (NULL == recurrence_id)
	{
		ERR("This event should have recurrence_id");
		return CALENDAR_ERROR_NONE;
	}
	DBG("Exdate parent(id:%d) and recurrence_id[%s]", original_event_id, recurrence_id);

	// get exdate from original event.
	snprintf(query, sizeof(query), "SELECT exdate FROM %s WHERE id = %d ",
			CAL_TABLE_SCHEDULE, original_event_id);

	stmt = _cal_db_util_query_prepare(query);
	if (NULL == stmt)
	{
		SEC_ERR("query[%s]", query);
		ERR("_cal_db_util_query_prepare() Failed");
		return CALENDAR_ERROR_DB_FAILED;
	}

	// add recurrence id to end of the exdate of original event.
	const unsigned char *temp = NULL;
	int len = 0;
	char *exdate = NULL;
	if (CAL_DB_ROW == _cal_db_util_stmt_step(stmt))
	{
		temp = sqlite3_column_text(stmt, 0);
		if (NULL == temp || strlen((char *)temp) < 1)
		{
			exdate = strdup(recurrence_id);
			DBG("append first exdate[%s]", exdate);
		}
		else
		{
			if (strstr((char *)temp, recurrence_id))
			{
				DBG("warn: recurrence id already is registered to exdate");
				sqlite3_finalize(stmt);
				return CALENDAR_ERROR_NONE;
			}
			len = strlen((const char *)temp) + strlen(recurrence_id) + 2;
			exdate = calloc(len, sizeof(char));
			if (NULL == exdate)
			{
				ERR("calloc() failed");
				sqlite3_finalize(stmt);
				return CALENDAR_ERROR_DB_FAILED;
			}
			snprintf(exdate, len, "%s,%s", temp, recurrence_id);
			DBG("append [%s] to aleady has exdate [%s]", temp, recurrence_id);
		}
	}
	else
	{
		DBG("Failed to get exdate: event_id(%d)", original_event_id);
	}
	sqlite3_finalize(stmt);
	stmt = NULL;

	// update exdate
	DBG("update to recurrence id to exdate[%s]", exdate);
	int input_ver = _cal_db_util_get_next_ver();
	snprintf(query, sizeof(query), "UPDATE %s SET exdate = ?, changed_ver=%d WHERE id = %d ",
			CAL_TABLE_SCHEDULE, input_ver, original_event_id);

	stmt = _cal_db_util_query_prepare(query);
	if (NULL == stmt)
	{
		SEC_ERR("query[%s]", query);
		ERR("_cal_db_util_query_prepare() failed");
		if (exdate) free(exdate);
		return CALENDAR_ERROR_DB_FAILED;
	}
	int index = 1;
	_cal_db_util_stmt_bind_text(stmt, index, exdate);

	dbret = _cal_db_util_stmt_step(stmt);
	sqlite3_finalize(stmt);
	if (CAL_DB_DONE != dbret) {
		ERR("sqlite3_step() Failed(%d)", dbret);
		if (exdate) free(exdate);
		switch (dbret)
		{
		case CAL_DB_ERROR_NO_SPACE:
			return CALENDAR_ERROR_FILE_NO_SPACE;
		default:
			return CALENDAR_ERROR_DB_FAILED;
		}
	}
	if (exdate) free(exdate);

	return CALENDAR_ERROR_NONE;
}

static int __cal_db_event_delete_record(int id)
{
	int ret = CALENDAR_ERROR_NONE;
	cal_db_util_error_e dbret = CAL_DB_OK;
	int calendar_book_id;
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	int created_ver = 0;
	int original_event_id = 0;
	char *recurrence_id = NULL;
	calendar_book_sync_event_type_e sync_event_type = CALENDAR_BOOK_SYNC_EVENT_FOR_ME;

	DBG("delete record(id:%d)", id);
	RETVM_IF(id < 0, CALENDAR_ERROR_INVALID_PARAMETER, "id(%d) < 0", id);

	// get calendar_id, created_ver, original_event_id, recurrence_id
	ret = __cal_db_event_get_deleted_data(id, &calendar_book_id, &created_ver, &original_event_id, &recurrence_id);
	if (CALENDAR_ERROR_NONE != ret)
	{
		DBG("__cal_db_event_get_deleted_data() failed");
		return ret;
	}

	//access control
	if (_cal_access_control_have_write_permission(calendar_book_id) == false)
	{
		ERR("fail");
		CAL_FREE(recurrence_id);
		return CALENDAR_ERROR_PERMISSION_DENIED;
	}

	if (original_event_id > 0)
	{
		// start:add record to exdate if this record is exception mod.
		__cal_db_event_add_exdate(original_event_id, recurrence_id);
	}
	CAL_FREE(recurrence_id);

	snprintf(query, sizeof(query), "SELECT sync_event FROM %s WHERE id = %d ",
			CAL_TABLE_CALENDAR, calendar_book_id);
	ret = _cal_db_util_query_get_first_int_result(query, NULL, (int *)&sync_event_type);
	if (CALENDAR_ERROR_NONE != ret)
	{
		ERR("_cal_db_util_query_get_first_int_result() failed");
		return ret;
	}
	DBG("sync_event_type(%d)", sync_event_type);

	if (sync_event_type == CALENDAR_BOOK_SYNC_EVENT_FOR_EVERY_AND_REMAIN)
	{
		DBG("set is_delete");
		snprintf(query, sizeof(query),
				"UPDATE %s "
				"SET is_deleted = 1, "
				"changed_ver = %d, "
				"last_mod = strftime('%%s','now') "
				"WHERE id = %d ",
				CAL_TABLE_SCHEDULE,
				_cal_db_util_get_next_ver(),
				id);

		dbret = _cal_db_util_query_exec(query);
		if (dbret != CAL_DB_OK)
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

		DBG("attendee, alarm and rrule will be deleted by trigger after sync clean");

	} else
	{
		_cal_db_util_get_next_ver();

		DBG("delete event");
		snprintf(query, sizeof(query), "DELETE FROM %s WHERE id = %d ",
				CAL_TABLE_SCHEDULE, id);

		dbret = _cal_db_util_query_exec(query);
		if(CAL_DB_OK != dbret)
		{
			DBG("query[%s]", query);
			ERR("_cal_db_util_query_exec() Failed");
			switch (dbret)
			{
			case CAL_DB_ERROR_NO_SPACE:
				return CALENDAR_ERROR_FILE_NO_SPACE;
			default:
				return CALENDAR_ERROR_DB_FAILED;
			}
		}

		DBG("attendee, alarm and rrule is deleted by trigger");
	}

	 _cal_db_instance_discard_record(id);
	_cal_db_util_notify(CAL_NOTI_TYPE_EVENT);

	return CALENDAR_ERROR_NONE;
}

static int __cal_db_event_get_all_records(int offset, int limit, calendar_list_h* out_list)
{
	int ret = CALENDAR_ERROR_NONE;
	char offsetquery[CAL_DB_SQL_MAX_LEN] = {0};
	char limitquery[CAL_DB_SQL_MAX_LEN] = {0};
	sqlite3_stmt *stmt = NULL;

	RETV_IF(NULL == out_list, CALENDAR_ERROR_INVALID_PARAMETER);

	ret = calendar_list_create(out_list);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "calendar_list_create() Fail(%d)", ret);

	if (offset > 0)
	{
		snprintf(offsetquery, sizeof(offsetquery), "OFFSET %d", offset);
	}
	if (limit > 0)
	{
		snprintf(limitquery, sizeof(limitquery), "LIMIT %d", limit);
	}

	char *query_str = NULL;
	_cal_db_append_string(&query_str, "SELECT * FROM");
	_cal_db_append_string(&query_str, CAL_VIEW_TABLE_EVENT);
	_cal_db_append_string(&query_str, limitquery);
	_cal_db_append_string(&query_str, offsetquery);

	stmt = _cal_db_util_query_prepare(query_str);
	if (NULL == stmt)
	{
		ERR("_cal_db_util_query_prepare() Failed");
		calendar_list_destroy(*out_list, true);
		*out_list = NULL;
		CAL_FREE(query_str);
		return CALENDAR_ERROR_DB_FAILED;
	}

	while(CAL_DB_ROW == _cal_db_util_stmt_step(stmt))
	{
		calendar_record_h record;
		int exception=0, extended=0;
		// stmt -> record
		ret = calendar_record_create(_calendar_event._uri,&record);
		if( ret != CALENDAR_ERROR_NONE )
		{
			calendar_list_destroy(*out_list, true);
			*out_list = NULL;
			sqlite3_finalize(stmt);
			CAL_FREE(query_str);
			return ret;
		}
		__cal_db_event_get_stmt(stmt, true, record, &exception, &extended);

		// child
		int has_attendee = 0, has_alarm = 0;
		int record_id = 0;
		cal_event_s* pevent = (cal_event_s*) record;
		calendar_record_get_int(record, _calendar_event.id, &record_id);
		if(calendar_record_get_int(record, _calendar_event.has_attendee,&has_attendee) == CALENDAR_ERROR_NONE)
		{
			if( has_attendee == 1)
			{
				_cal_db_attendee_get_records(record_id, pevent->attendee_list);
			}
		}
		if(calendar_record_get_int(record, _calendar_event.has_alarm,&has_alarm) == CALENDAR_ERROR_NONE)
		{
			if( has_alarm == 1)
			{
				_cal_db_alarm_get_records(record_id, pevent->alarm_list);
			}
		}

		if (exception == 1)
			__cal_db_event_exception_get_records(record_id, pevent->exception_list);
		if (extended == 1)
			_cal_db_extended_get_records(record_id, CALENDAR_RECORD_TYPE_EVENT, pevent->extended_list);

		ret = calendar_list_add(*out_list,record);
		if( ret != CALENDAR_ERROR_NONE )
		{
			calendar_list_destroy(*out_list, true);
			*out_list = NULL;
			calendar_record_destroy(record, true);
			sqlite3_finalize(stmt);
			CAL_FREE(query_str);
			return ret;
		}
	}

	sqlite3_finalize(stmt);
	CAL_FREE(query_str);
	return CALENDAR_ERROR_NONE;
}

static int __cal_db_event_get_records_with_query(calendar_query_h query, int offset, int limit, calendar_list_h* out_list)
{
	cal_query_s *que = NULL;
	calendar_list_h list = NULL;
	int ret = CALENDAR_ERROR_NONE;
	char *condition = NULL;
	char *projection = NULL;
	GSList *bind_text = NULL, *cursor = NULL;
	sqlite3_stmt *stmt = NULL;
	int i = 0;
	char *table_name;

	if (NULL == query || NULL == out_list)
	{
		ERR("Invalid parameter");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	que = (cal_query_s *)query;

	if ( 0 == strcmp(que->view_uri, CALENDAR_VIEW_EVENT))
	{
		table_name = SAFE_STRDUP(CAL_VIEW_TABLE_EVENT);
	}
	else if ( 0 == strcmp(que->view_uri, CALENDAR_VIEW_EVENT_CALENDAR))
	{
		table_name = SAFE_STRDUP(CAL_VIEW_TABLE_EVENT_CALENDAR);
	}
	else if ( 0 == strcmp(que->view_uri, CALENDAR_VIEW_EVENT_CALENDAR_ATTENDEE))
	{
		table_name = SAFE_STRDUP(CAL_VIEW_TABLE_EVENT_CALENDAR_ATTENDEE);
	}
	else
	{
		ERR("uri(%s) not support get records with query",que->view_uri);
		return CALENDAR_ERROR_INVALID_PARAMETER;
		//table_name = SAFE_STRDUP(CAL_TABLE_NORMAL_INSTANCE);
	}

	// make filter
	if (que->filter)
	{
		ret = _cal_db_query_create_condition(query, &condition, &bind_text);
		if (ret != CALENDAR_ERROR_NONE)
		{
			CAL_FREE(table_name);
			ERR("filter create fail");
			return ret;
		}
	}

	// make projection
	ret = _cal_db_query_create_projection(query, &projection);

	char *query_str = NULL;
	// query - projection
	if (projection)
	{
		_cal_db_append_string(&query_str, "SELECT");
		_cal_db_append_string(&query_str, projection);
		_cal_db_append_string(&query_str, "FROM");
		_cal_db_append_string(&query_str, table_name);
		CAL_FREE(projection);
	}
	else
	{
		_cal_db_append_string(&query_str, "SELECT * FROM");
		_cal_db_append_string(&query_str, table_name);
	}
	CAL_FREE(table_name);

	// query - condition
	if (condition)
	{
		_cal_db_append_string(&query_str, "WHERE (");
		_cal_db_append_string(&query_str, condition);
		_cal_db_append_string(&query_str, ")");
	}

	// ORDER
	char *order = NULL;
	ret = _cal_db_query_create_order(query, condition, &order);
	if (order)
	{
		_cal_db_append_string(&query_str, order);
		CAL_FREE(order);
	}
	CAL_FREE(condition);

	// limit, offset
	char buf[32] = {0};
	if (limit > 0)
	{
		snprintf(buf, sizeof(buf), "LIMIT %d", limit);
		_cal_db_append_string(&query_str, buf);

		if (offset > 0)
		{
			snprintf(buf, sizeof(buf), "OFFSET %d", offset);
			_cal_db_append_string(&query_str, buf);
		}
	}
	DBG("%s",query_str);

	// query
	stmt = _cal_db_util_query_prepare(query_str);
	if (NULL == stmt)
	{
		if (bind_text)
		{
			g_slist_free_full(bind_text, free);
			bind_text = NULL;
		}
		CAL_FREE(query_str);
		ERR("_cal_db_util_query_prepare() Failed");
		return CALENDAR_ERROR_DB_FAILED;
	}

	// bind text
	if (bind_text)
	{
		for (cursor=bind_text, i=1; cursor;cursor=cursor->next, i++)
		{
			_cal_db_util_stmt_bind_text(stmt, i, cursor->data);
		}
	}

	//
	ret = calendar_list_create(&list);
	if (ret != CALENDAR_ERROR_NONE)
	{
		if (bind_text)
		{
			g_slist_free_full(bind_text, free);
			bind_text = NULL;
		}
		ERR("calendar_list_create() Failed");
		sqlite3_finalize(stmt);
		CAL_FREE(query_str);
		return ret;
	}

	while(CAL_DB_ROW == _cal_db_util_stmt_step(stmt))
	{
		calendar_record_h record;
		int exception = 1, extended = 1;
		int attendee = 1, alarm = 1;

		// stmt -> record
		ret = calendar_record_create(_calendar_event._uri,&record);
		if( ret != CALENDAR_ERROR_NONE )
		{
			calendar_list_destroy(list, true);
			if (bind_text)
			{
				g_slist_free_full(bind_text, free);
				bind_text = NULL;
			}
			sqlite3_finalize(stmt);
			CAL_FREE(query_str);
			return ret;
		}
		if (que->projection_count > 0)
		{
			_cal_record_set_projection(record,
					que->projection, que->projection_count, que->property_count);

			__cal_db_event_get_projection_stmt(stmt,
					que->projection, que->projection_count,
					record);
		}
		else
		{
			cal_event_s *event = NULL;
			__cal_db_event_get_stmt(stmt,true,record, &exception, &extended);
			event = (cal_event_s*)(record);
			if (event)
			{
				attendee = event->has_attendee;
				alarm = event->has_alarm;
			}
		}

		// child
		if (_cal_db_query_find_projection_property(query,CAL_PROPERTY_EVENT_CALENDAR_ALARM) == true && alarm == 1)
		{
			cal_event_s* pevent = (cal_event_s*) record;
			_cal_db_alarm_get_records(pevent->index, pevent->alarm_list);
		}
		if (_cal_db_query_find_projection_property(query,CAL_PROPERTY_EVENT_CALENDAR_ATTENDEE) == true && attendee == 1)
		{
			cal_event_s* pevent = (cal_event_s*) record;
			_cal_db_attendee_get_records(pevent->index, pevent->attendee_list);
		}
		if (_cal_db_query_find_projection_property(query,CAL_PROPERTY_EVENT_EXCEPTION) == true && exception ==1)
		{
			cal_event_s* pevent = (cal_event_s*) record;
			__cal_db_event_exception_get_records(pevent->index, pevent->exception_list);
		}
		if (_cal_db_query_find_projection_property(query,CAL_PROPERTY_EVENT_EXTENDED) == true && extended ==1)
		{
			cal_event_s* pevent = (cal_event_s*) record;
			_cal_db_extended_get_records(pevent->index, CALENDAR_RECORD_TYPE_EVENT, pevent->extended_list);
		}

		ret = calendar_list_add(list,record);
		if( ret != CALENDAR_ERROR_NONE )
		{
			ERR("calendar_list_add() is failed(%d)", ret);
			calendar_list_destroy(list, true);
			calendar_record_destroy(record, true);

			if (bind_text)
			{
				g_slist_free_full(bind_text, free);
				bind_text = NULL;
			}
			sqlite3_finalize(stmt);
			CAL_FREE(query_str);
			return ret;
		}
	}

	if (bind_text)
	{
		g_slist_free_full(bind_text, free);
		bind_text = NULL;
	}

	sqlite3_finalize(stmt);
	CAL_FREE(query_str);

	*out_list = list;
	__check_list(list);

	return CALENDAR_ERROR_NONE;
}

static int __cal_db_event_insert_records(const calendar_list_h list, int** ids)
{
	calendar_record_h record;
	int ret = 0;
	int count = 0;
	int i=0;
	int *id = NULL;

	ret = calendar_list_get_count(list, &count);
	if (ret != CALENDAR_ERROR_NONE)
	{
		ERR("list get error");
		return ret;
	}

	id = calloc(1, sizeof(int)*count);

	RETVM_IF(NULL == id, CALENDAR_ERROR_OUT_OF_MEMORY, "calloc fail");

	ret = calendar_list_first(list);
	if (ret != CALENDAR_ERROR_NONE)
	{
		ERR("list first error");
		CAL_FREE(id);
		return ret;
	}
	do
	{
		if( calendar_list_get_current_record_p(list, &record) == CALENDAR_ERROR_NONE)
		{
			if( __cal_db_event_insert_record(record, &id[i]) != CALENDAR_ERROR_NONE)
			{
				ERR("db insert error");
				CAL_FREE(id);
				return CALENDAR_ERROR_DB_FAILED;
			}
		}
		i++;
	} while(calendar_list_next(list) != CALENDAR_ERROR_NO_DATA);

	if(ids)
	{
		*ids = id;
	}
	else
	{
		CAL_FREE(id);
	}

	return CALENDAR_ERROR_NONE;
}

static int __cal_db_event_update_records(const calendar_list_h list)
{
	calendar_record_h record;
	int ret = 0;

	ret = calendar_list_first(list);
	if (ret != CALENDAR_ERROR_NONE)
	{
		ERR("list first error");
		return ret;
	}
	do
	{
		if( calendar_list_get_current_record_p(list, &record) == CALENDAR_ERROR_NONE)
		{
			if( __cal_db_event_update_record(record) != CALENDAR_ERROR_NONE)
			{
				ERR("db insert error");
				return CALENDAR_ERROR_DB_FAILED;
			}
		}
	} while(calendar_list_next(list) != CALENDAR_ERROR_NO_DATA);

	return CALENDAR_ERROR_NONE;
}

static int __cal_db_event_delete_records(int ids[], int count)
{
	int ret = CALENDAR_ERROR_NONE;
	int i = 0;

	for(i = 0; i < count; i++)
	{
		ret = __cal_db_event_delete_record(ids[i]);
		if (CALENDAR_ERROR_NONE != ret)
		{
			ERR("delete failed");
			return ret;
		}
	}
	return CALENDAR_ERROR_NONE;
}

static int __cal_db_event_get_count(int *out_count)
{
	RETV_IF(NULL == out_count, CALENDAR_ERROR_INVALID_PARAMETER);

	char *query_str = NULL;
	_cal_db_append_string(&query_str, "SELECT count(*) FROM");
	_cal_db_append_string(&query_str, CAL_VIEW_TABLE_EVENT);

	int ret = 0;
	int count = 0;
	ret = _cal_db_util_query_get_first_int_result(query_str, NULL, &count);
	if (CALENDAR_ERROR_NONE != ret)
	{
		ERR("_cal_db_util_query_get_first_int_result() failed");
		CAL_FREE(query_str);
		return ret;
	}
	DBG("count(%d) str[%s]", count, query_str);
	CAL_FREE(query_str);

	*out_count = count;
	return CALENDAR_ERROR_NONE;
}

static int __cal_db_event_get_count_with_query(calendar_query_h query, int *out_count)
{
	cal_query_s *que = NULL;
	int ret = CALENDAR_ERROR_NONE;
	char *condition = NULL;
	char *table_name;
	int count = 0;
	GSList *bind_text = NULL;

	que = (cal_query_s *)query;

	if ( 0 == strcmp(que->view_uri, CALENDAR_VIEW_EVENT))
	{
		table_name = SAFE_STRDUP(CAL_VIEW_TABLE_EVENT);
	}
	else if ( 0 == strcmp(que->view_uri, CALENDAR_VIEW_EVENT_CALENDAR))
	{
		table_name = SAFE_STRDUP(CAL_VIEW_TABLE_EVENT_CALENDAR);
	}
	else if ( 0 == strcmp(que->view_uri, CALENDAR_VIEW_EVENT_CALENDAR_ATTENDEE))
	{
		table_name = SAFE_STRDUP(CAL_VIEW_TABLE_EVENT_CALENDAR_ATTENDEE);
	}
	else
	{
		ERR("uri(%s) not support get records with query",que->view_uri);
		return CALENDAR_ERROR_INVALID_PARAMETER;
		//table_name = SAFE_STRDUP(CAL_TABLE_NORMAL_INSTANCE);
	}

	// make filter
	if (que->filter)
	{
		ret = _cal_db_query_create_condition(query, &condition, &bind_text);
		if (ret != CALENDAR_ERROR_NONE)
		{
			CAL_FREE(table_name);
			ERR("filter create fail");
			return ret;
		}
	}

	char *query_str = NULL;
	// query - select from
	_cal_db_append_string(&query_str, "SELECT count(*) FROM");
	_cal_db_append_string(&query_str, table_name);
	CAL_FREE(table_name);

	// query - condition
	if (condition)
	{
		_cal_db_append_string(&query_str,  "WHERE (");
		_cal_db_append_string(&query_str, condition);
		_cal_db_append_string(&query_str, ")");
		CAL_FREE(condition);
	}

	// query
	ret = _cal_db_util_query_get_first_int_result(query_str, bind_text, &count);
	if (CALENDAR_ERROR_NONE != ret)
	{
		ERR("_cal_db_util_query_get_first_int_result() failed");
		if (bind_text)
		{
			g_slist_free_full(bind_text, free);
			bind_text = NULL;
		}
		CAL_FREE(query_str);
		return ret;
	}
	DBG("count(%d) str[%s]", count, query_str);

	if (out_count) *out_count = count;
	if (bind_text)
	{
		g_slist_free_full(bind_text, free);
		bind_text = NULL;
	}

	CAL_FREE(query_str);
	return CALENDAR_ERROR_NONE;
}

static int __cal_db_event_replace_record(calendar_record_h record, int id)
{
	int ret = CALENDAR_ERROR_NONE;
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	char dtstart_datetime[32] = {0};
	char dtend_datetime[32] = {0};
	sqlite3_stmt *stmt = NULL;
	cal_event_s* event =  (cal_event_s*)(record);
	cal_rrule_s *rrule = NULL;
	cal_db_util_error_e dbret = CAL_DB_OK;
	int has_alarm = 0;
	int timezone_id = 0;
	int input_ver = 0;

	RETV_IF(NULL == event, CALENDAR_ERROR_INVALID_PARAMETER);
	event->index = id;

	// access control
	if (_cal_access_control_have_write_permission(event->calendar_id) == false)
	{
		ERR("_cal_access_control_have_write_permission() failed");
		return CALENDAR_ERROR_PERMISSION_DENIED;
	}
	if (event->common.properties_flags != NULL)
	{
		return __cal_db_event_update_dirty(record, -1);
	}
	has_alarm = _cal_db_alarm_has_alarm(event->alarm_list);
	_cal_time_get_timezone_from_table(event->start_tzid, NULL, &timezone_id);
	input_ver = _cal_db_util_get_next_ver();
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

	snprintf(query, sizeof(query), "UPDATE %s SET "
			"changed_ver = %d,"
			"type = %d,"
			"summary = ?,"
			"description = ?,"
			"location = ?,"
			"categories = ?,"
			"exdate = ?,"
			"task_status = %d,"
			"priority = %d,"
			"timezone = %d, "
			"contact_id = %d, "
			"busy_status = %d, "
			"sensitivity = %d, "
			"uid = ?, "
			"organizer_name = ?, "
			"organizer_email = ?, "
			"meeting_status = %d, "
			"calendar_id = %d, "
			"original_event_id = %d,"
			"latitude = %lf,"
			"longitude = %lf,"
			"email_id = %d,"
			"completed_time = %lld,"
			"progress = %d, "
			"dtstart_type = %d, "
			"dtstart_utime = %lld, "
			"dtstart_datetime = ?, "
			"dtstart_tzid = ?, "
			"dtend_type = %d, "
			"dtend_utime = %lld, "
			"dtend_datetime = ?, "
			"dtend_tzid = ?, "
			"last_mod = strftime('%%s', 'now'), "
			"rrule_id = %d, "
			"recurrence_id = ?, "
			"rdate = ?, "
			"has_attendee = %d, "
			"has_alarm = %d, "
			"system_type = %d, "
			"updated = %ld, "
			"sync_data1 = ?, "
			"sync_data2 = ?, "
			"sync_data3 = ?, "
			"sync_data4 = ?, "
			"freq = %d, "
			"is_allday = %d "
			"WHERE id = %d ",
		CAL_TABLE_SCHEDULE,
		input_ver,
		CAL_SCH_TYPE_EVENT,/*event->cal_type,*/
		event->event_status,
		event->priority,
		event->timezone ? event->timezone : timezone_id,
		event->contact_id,
		event->busy_status,
		event->sensitivity,
		event->meeting_status,
		event->calendar_id,
		event->original_event_id,
		event->latitude,
		event->longitude,
		event->email_id,
		(long long int)0,//event->completed_time,
		0,//event->progress,
		event->start.type,
		event->start.type == CALENDAR_TIME_UTIME ? event->start.time.utime : 0,
		event->end.type,
		event->end.type == CALENDAR_TIME_UTIME ? event->end.time.utime : 0,
		event->freq > 0 ? 1 : 0,
		(event->attendee_list && 0 < event->attendee_list->count) ? 1 : 0,
		has_alarm,
		event->system_type,
		event->updated,
		event->freq,
		is_allday,
		id);

	stmt = _cal_db_util_query_prepare(query);
	RETVM_IF(NULL == stmt, CALENDAR_ERROR_DB_FAILED, "_cal_db_util_query_prepare() Failed");

	int index = 1;

	if (event->summary)
		_cal_db_util_stmt_bind_text(stmt, index, event->summary);
	index++;

	if (event->description)
		_cal_db_util_stmt_bind_text(stmt, index, event->description);
	index++;

	if (event->location)
		_cal_db_util_stmt_bind_text(stmt, index, event->location);
	index++;

	if (event->categories)
		_cal_db_util_stmt_bind_text(stmt, index, event->categories);
	index++;

	if (event->exdate)
		_cal_db_util_stmt_bind_text(stmt, index, event->exdate);
	index++;

	if (event->uid)
		_cal_db_util_stmt_bind_text(stmt, index, event->uid);
	index++;

	if (event->organizer_name)
		_cal_db_util_stmt_bind_text(stmt, index, event->organizer_name);
	index++;

	if (event->organizer_email)
		_cal_db_util_stmt_bind_text(stmt, index, event->organizer_email);
	index++;

	if (CALENDAR_TIME_LOCALTIME == event->start.type)
	{
		snprintf(dtstart_datetime, sizeof(dtstart_datetime), CAL_FORMAT_LOCAL_DATETIME,
				event->start.time.date.year,
				event->start.time.date.month,
				event->start.time.date.mday,
				event->start.time.date.hour,
				event->start.time.date.minute,
				event->start.time.date.second);
		_cal_db_util_stmt_bind_text(stmt, index, dtstart_datetime);
	}
	index++;

	if (event->start_tzid)
		_cal_db_util_stmt_bind_text(stmt, index, event->start_tzid);
	index++;

	if (CALENDAR_TIME_LOCALTIME == event->end.type)
	{
		snprintf(dtend_datetime, sizeof(dtend_datetime), CAL_FORMAT_LOCAL_DATETIME,
				event->end.time.date.year,
				event->end.time.date.month,
				event->end.time.date.mday,
				event->end.time.date.hour,
				event->end.time.date.minute,
				event->end.time.date.second);
		_cal_db_util_stmt_bind_text(stmt, index, dtend_datetime);
	}
	index++;

	if (event->end_tzid)
		_cal_db_util_stmt_bind_text(stmt, index, event->end_tzid);
	index++;

	if (event->recurrence_id)
		_cal_db_util_stmt_bind_text(stmt, index, event->recurrence_id);
	index++;
	if (event->rdate)
		_cal_db_util_stmt_bind_text(stmt, index, event->rdate);
	index++;
	if (event->sync_data1)
		_cal_db_util_stmt_bind_text(stmt, index, event->sync_data1);
	index++;
	if (event->sync_data2)
		_cal_db_util_stmt_bind_text(stmt, index, event->sync_data2);
	index++;
	if (event->sync_data3)
		_cal_db_util_stmt_bind_text(stmt, index, event->sync_data3);
	index++;
	if (event->sync_data4)
		_cal_db_util_stmt_bind_text(stmt, index, event->sync_data4);
	index++;

	dbret = _cal_db_util_stmt_step(stmt);
	if (CAL_DB_DONE != dbret) {
		sqlite3_finalize(stmt);
		ERR("sqlite3_step() Failed(%d)", dbret);
		switch (dbret)
		{
		case CAL_DB_ERROR_NO_SPACE:
			return CALENDAR_ERROR_FILE_NO_SPACE;
		default:
			return CALENDAR_ERROR_DB_FAILED;
		}
	}
	sqlite3_finalize(stmt);

	// update parent event changed ver in case this event is exception mod
	// which is original_event_id > 0
	_cal_db_event_update_original_event_version(event->original_event_id, input_ver);

//	_cal_db_rrule_set_default(record);
	_cal_db_rrule_get_rrule_from_event(record, &rrule);
	_cal_db_rrule_update_record(id, rrule);
	CAL_FREE(rrule);

	_cal_db_instance_discard_record(id);
	_cal_db_instance_publish_record(record);

	_cal_db_alarm_delete_with_id(id);
	_cal_db_attendee_delete_with_id(id);
	__cal_db_event_exception_delete_with_id(id);
	_cal_db_extended_delete_with_id(id, CALENDAR_RECORD_TYPE_EVENT);

	if (event->alarm_list && 0 < event->alarm_list->count) {
		ret = _cal_db_alarm_insert_records(event->alarm_list, event->index);
		WARN_IF(CALENDAR_ERROR_NONE != ret, "_cal_db_alarm_insert_records() failed(%x)", ret);
	}

	if (event->attendee_list && 0 < event->attendee_list->count) {
		ret = _cal_db_attendee_insert_records(event->attendee_list, event->index);
		WARN_IF(CALENDAR_ERROR_NONE != ret, "_cal_db_attendee_insert_records() failed(%x)", ret);
	}

	if (event->exception_list && 0 < event->exception_list->count) {
		ret = _cal_db_event_insert_records(event->exception_list, id);
		WARN_IF(CALENDAR_ERROR_NONE != ret, "_cal_db_event_insert_records() failed(%x)", ret);
	}

	if (event->extended_list && 0 < event->extended_list->count) {
		ret = _cal_db_extended_insert_records(event->extended_list, id, CALENDAR_RECORD_TYPE_EVENT);
		WARN_IF(CALENDAR_ERROR_NONE != ret, "_cal_db_extended_insert_records() Failed(%d)", ret);
	}

	_cal_db_util_notify(CAL_NOTI_TYPE_EVENT);

	return CALENDAR_ERROR_NONE;
}

static int __cal_db_event_replace_records(const calendar_list_h list, int ids[], int count)
{
	calendar_record_h record;
	int i = 0;
	int ret = 0;

	if (NULL == list)
	{
		ERR("Invalid argument: list is NULL");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	ret = calendar_list_first(list);
	if (ret != CALENDAR_ERROR_NONE)
	{
		ERR("list first error");
		return ret;
	}

	for (i = 0; i < count; i++)
	{
		if( calendar_list_get_current_record_p(list, &record) == CALENDAR_ERROR_NONE)
		{
			if( __cal_db_event_replace_record(record, ids[i]) != CALENDAR_ERROR_NONE)
			{
				ERR("db insert error");
				return CALENDAR_ERROR_DB_FAILED;
			}
		}
		if (CALENDAR_ERROR_NO_DATA != calendar_list_next(list))
		{
			break;
		}
	}

	return CALENDAR_ERROR_NONE;
}

static void __cal_db_event_get_stmt(sqlite3_stmt *stmt,bool is_view_table,calendar_record_h record,
		int *exception, int *extended)
{
	cal_event_s *event = NULL;
	const unsigned char *temp;
	int count = 0;

	event = (cal_event_s*)(record);

	event->index = sqlite3_column_int(stmt, count++);
	sqlite3_column_int(stmt, count++);//event->cal_type = 1;/*sqlite3_column_int(stmt, count++);*/

	temp = sqlite3_column_text(stmt, count++);
	event->summary = SAFE_STRDUP(temp);

	temp = sqlite3_column_text(stmt, count++);
	event->description = SAFE_STRDUP(temp);

	temp = sqlite3_column_text(stmt, count++);
	event->location = SAFE_STRDUP(temp);

	temp = sqlite3_column_text(stmt, count++);
	event->categories = SAFE_STRDUP(temp);

	temp = sqlite3_column_text(stmt, count++);
	event->exdate = SAFE_STRDUP(temp);

	event->event_status = sqlite3_column_int(stmt, count++);
	event->priority = sqlite3_column_int(stmt, count++);
	event->timezone = sqlite3_column_int(stmt, count++);
	event->contact_id = sqlite3_column_int(stmt, count++);
	event->busy_status = sqlite3_column_int(stmt, count++);
	event->sensitivity = sqlite3_column_int(stmt, count++);

	temp = sqlite3_column_text(stmt, count++);
	event->uid = SAFE_STRDUP(temp);

	temp = sqlite3_column_text(stmt, count++);
	event->organizer_name = SAFE_STRDUP(temp);

	temp = sqlite3_column_text(stmt, count++);
	event->organizer_email = SAFE_STRDUP(temp);

	event->meeting_status = sqlite3_column_int(stmt, count++);
	event->calendar_id = sqlite3_column_int(stmt, count++);
	event->original_event_id = sqlite3_column_int(stmt, count++);
	event->latitude = sqlite3_column_double(stmt,count++);
	event->longitude = sqlite3_column_double(stmt,count++);
	event->email_id = sqlite3_column_int(stmt, count++);
	event->created_time = sqlite3_column_int64(stmt, count++);

	count++; // completed_time
	count++; // progress
	count++; // changed_ver
	count++; // created_ver

	event->is_deleted = sqlite3_column_int(stmt,count++);
	event->start.type = sqlite3_column_int(stmt,count++);

	if (event->start.type == CALENDAR_TIME_UTIME)
	{
		event->start.time.utime = sqlite3_column_int64(stmt,count++);
		count++; // dtstart_datetime
	}
	else
	{
		count++; // dtstart_utime
		temp = sqlite3_column_text(stmt, count++);
		if (temp) {
			sscanf((const char *)temp, CAL_FORMAT_LOCAL_DATETIME, &(event->start.time.date.year),
					&(event->start.time.date.month), &(event->start.time.date.mday),
					&(event->start.time.date.hour), &(event->start.time.date.minute),
					&(event->start.time.date.second));
		}
	}

	temp = sqlite3_column_text(stmt, count++);
	event->start_tzid = SAFE_STRDUP(temp);

	event->end.type = sqlite3_column_int(stmt, count++);
	if (event->end.type == CALENDAR_TIME_UTIME)
	{
		event->end.time.utime = sqlite3_column_int64(stmt,count++);
		count++; // dtend_datetime
	}
	else
	{
		count++; // dtend_utime
		temp = sqlite3_column_text(stmt, count++);
		if (temp) {
			sscanf((const char *)temp, CAL_FORMAT_LOCAL_DATETIME, &(event->end.time.date.year),
					&(event->end.time.date.month), &(event->end.time.date.mday),
					&(event->end.time.date.hour), &(event->end.time.date.minute),
					&(event->end.time.date.second));
		}
	}
	temp = sqlite3_column_text(stmt, count++);
	event->end_tzid = SAFE_STRDUP(temp);

	event->last_mod = sqlite3_column_int64(stmt,count++);
	sqlite3_column_int(stmt,count++);//event->rrule_id = sqlite3_column_int(stmt,count++);

	temp = sqlite3_column_text(stmt, count++);
	event->recurrence_id = SAFE_STRDUP(temp);
	temp = sqlite3_column_text(stmt, count++);
	event->rdate = SAFE_STRDUP(temp);
	event->has_attendee = sqlite3_column_int(stmt,count++);
	event->has_alarm = sqlite3_column_int(stmt,count++);
	event->system_type = sqlite3_column_int(stmt,count++);
	event->updated = sqlite3_column_int(stmt,count++);
	temp = sqlite3_column_text(stmt, count++);
	event->sync_data1 = SAFE_STRDUP(temp);
	temp = sqlite3_column_text(stmt, count++);
	event->sync_data2 = SAFE_STRDUP(temp);
	temp = sqlite3_column_text(stmt, count++);
	event->sync_data3 = SAFE_STRDUP(temp);
	temp = sqlite3_column_text(stmt, count++);
	event->sync_data4 = SAFE_STRDUP(temp);

	//has_exception
	if (exception != NULL)
		*exception = sqlite3_column_int(stmt,count++);

	//has_extended
	if (extended != NULL)
		*extended = sqlite3_column_int(stmt,count++);

	event->freq = sqlite3_column_int(stmt, count++);
	event->is_allday = sqlite3_column_int(stmt, count++);

	if (is_view_table == true) {
		if (event->freq <= 0) {
			//event->rrule_id = 0;
			//sqlite3_finalize(stmt);
			//return CALENDAR_ERROR_NONE;
			return ;
		}

		//event->rrule_id = 1;
		event->range_type = sqlite3_column_int(stmt, count++);
		event->until.type = sqlite3_column_int(stmt, count++);

		switch (event->until.type)
		{
		case CALENDAR_TIME_UTIME:
			event->until.time.utime = sqlite3_column_int64(stmt, count++);
			count++; // datetime
			break;

		case CALENDAR_TIME_LOCALTIME:
			count++; // utime
			temp = sqlite3_column_text(stmt, count++);
			if (temp) {
				sscanf((const char *)temp, CAL_FORMAT_LOCAL_DATETIME, &(event->until.time.date.year),
						&(event->until.time.date.month), &(event->until.time.date.mday),
						&(event->until.time.date.hour), &(event->until.time.date.minute),
						&(event->until.time.date.second));
			}
			break;
		}

		event->count = sqlite3_column_int(stmt, count++);
		event->interval = sqlite3_column_int(stmt, count++);

		temp = sqlite3_column_text(stmt, count++);
		event->bysecond = SAFE_STRDUP(temp);

		temp = sqlite3_column_text(stmt, count++);
		event->byminute = SAFE_STRDUP(temp);

		temp = sqlite3_column_text(stmt, count++);
		event->byhour = SAFE_STRDUP(temp);

		temp = sqlite3_column_text(stmt, count++);
		event->byday= SAFE_STRDUP(temp);

		temp = sqlite3_column_text(stmt, count++);
		event->bymonthday= SAFE_STRDUP(temp);

		temp = sqlite3_column_text(stmt, count++);
		event->byyearday= SAFE_STRDUP(temp);

		temp = sqlite3_column_text(stmt, count++);
		event->byweekno= SAFE_STRDUP(temp);

		temp = sqlite3_column_text(stmt, count++);
		event->bymonth= SAFE_STRDUP(temp);

		temp = sqlite3_column_text(stmt, count++);
		event->bysetpos = SAFE_STRDUP(temp);

		event->wkst = sqlite3_column_int(stmt, count++);

		sqlite3_column_int(stmt, count++); //calendar deleted
	}

}

static void __cal_db_event_get_property_stmt(sqlite3_stmt *stmt,
		unsigned int property, int *stmt_count, calendar_record_h record)
{
	cal_event_s *event = NULL;
	const unsigned char *temp;

	event = (cal_event_s*)(record);

	switch(property)
	{
	case CAL_PROPERTY_EVENT_ID:
		event->index = sqlite3_column_int(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_EVENT_CALENDAR_ID:
		event->calendar_id = sqlite3_column_int(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_EVENT_SUMMARY:
		temp = sqlite3_column_text(stmt, *stmt_count);
		event->summary = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_EVENT_DESCRIPTION:
		temp = sqlite3_column_text(stmt, *stmt_count);
		event->description = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_EVENT_LOCATION:
		temp = sqlite3_column_text(stmt, *stmt_count);
		event->location = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_EVENT_CATEGORIES:
		temp = sqlite3_column_text(stmt, *stmt_count);
		event->categories = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_EVENT_EXDATE:
		temp = sqlite3_column_text(stmt, *stmt_count);
		event->exdate = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_EVENT_EVENT_STATUS:
		event->event_status = sqlite3_column_int(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_EVENT_PRIORITY:
		event->priority = sqlite3_column_int(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_EVENT_TIMEZONE:
		event->timezone = sqlite3_column_int(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_EVENT_CONTACT_ID:
		event->contact_id = sqlite3_column_int(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_EVENT_BUSY_STATUS:
		event->busy_status = sqlite3_column_int(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_EVENT_SENSITIVITY:
		event->sensitivity = sqlite3_column_int(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_EVENT_UID:
		temp = sqlite3_column_text(stmt, *stmt_count);
		event->uid = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_EVENT_ORGANIZER_NAME:
		temp = sqlite3_column_text(stmt, *stmt_count);
		event->organizer_name = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_EVENT_ORGANIZER_EMAIL:
		temp = sqlite3_column_text(stmt, *stmt_count);
		event->organizer_email = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_EVENT_MEETING_STATUS:
		event->meeting_status = sqlite3_column_int(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_EVENT_ORIGINAL_EVENT_ID:
		event->original_event_id = sqlite3_column_int(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_EVENT_LATITUDE:
		event->latitude = sqlite3_column_double(stmt,*stmt_count);
		break;
	case CAL_PROPERTY_EVENT_LONGITUDE:
		event->longitude = sqlite3_column_double(stmt,*stmt_count);
		break;
	case CAL_PROPERTY_EVENT_EMAIL_ID:
		event->email_id = sqlite3_column_int(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_EVENT_CREATED_TIME:
		event->created_time = sqlite3_column_int64(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_EVENT_LAST_MODIFIED_TIME:
		event->last_mod = sqlite3_column_int64(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_EVENT_IS_DELETED:
		event->is_deleted = sqlite3_column_int(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_EVENT_FREQ:
		event->freq = sqlite3_column_int(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_EVENT_RANGE_TYPE:
		event->range_type = sqlite3_column_int(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_EVENT_UNTIL:
		//!!
		break;
	case CAL_PROPERTY_EVENT_COUNT:
		event->count = sqlite3_column_int(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_EVENT_INTERVAL:
		event->interval = sqlite3_column_int(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_EVENT_BYSECOND:
		temp = sqlite3_column_text(stmt, *stmt_count);
		event->bysecond = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_EVENT_BYMINUTE:
		temp = sqlite3_column_text(stmt, *stmt_count);
		event->byminute = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_EVENT_BYHOUR:
		temp = sqlite3_column_text(stmt, *stmt_count);
		event->byhour = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_EVENT_BYDAY:
		temp = sqlite3_column_text(stmt, *stmt_count);
		event->byday = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_EVENT_BYMONTHDAY:
		temp = sqlite3_column_text(stmt, *stmt_count);
		event->bymonthday = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_EVENT_BYYEARDAY:
		temp = sqlite3_column_text(stmt, *stmt_count);
		event->byyearday = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_EVENT_BYWEEKNO:
		temp = sqlite3_column_text(stmt, *stmt_count);
		event->byweekno = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_EVENT_BYMONTH:
		temp = sqlite3_column_text(stmt, *stmt_count);
		event->bymonth = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_EVENT_BYSETPOS:
		temp = sqlite3_column_text(stmt, *stmt_count);
		event->bysetpos = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_EVENT_WKST:
		event->wkst = sqlite3_column_int(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_EVENT_RECURRENCE_ID:
		temp = sqlite3_column_text(stmt, *stmt_count);
		event->recurrence_id = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_EVENT_RDATE:
		temp = sqlite3_column_text(stmt, *stmt_count);
		event->rdate = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_EVENT_HAS_ATTENDEE:
		event->has_attendee = sqlite3_column_int(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_EVENT_HAS_ALARM:
		event->has_alarm = sqlite3_column_int(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_EVENT_SYNC_DATA1:
		temp = sqlite3_column_text(stmt, *stmt_count);
		event->sync_data1 = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_EVENT_SYNC_DATA2:
		temp = sqlite3_column_text(stmt, *stmt_count);
		event->sync_data2 = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_EVENT_SYNC_DATA3:
		temp = sqlite3_column_text(stmt, *stmt_count);
		event->sync_data3 = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_EVENT_SYNC_DATA4:
		temp = sqlite3_column_text(stmt, *stmt_count);
		event->sync_data4 = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_EVENT_START:
		event->start.type = sqlite3_column_int(stmt,*stmt_count);
		if (event->start.type == CALENDAR_TIME_UTIME)
		{
			*stmt_count = *stmt_count+1;
			event->start.time.utime = sqlite3_column_int64(stmt,*stmt_count);
			*stmt_count = *stmt_count+1; // dtstart_datetime
		}
		else
		{
			*stmt_count = *stmt_count+1;
			*stmt_count = *stmt_count+1;
			temp = sqlite3_column_text(stmt, *stmt_count);
			if (temp) {
				sscanf((const char *)temp, CAL_FORMAT_LOCAL_DATETIME, &(event->start.time.date.year),
						&(event->start.time.date.month), &(event->start.time.date.mday),
						&(event->start.time.date.hour), &(event->start.time.date.minute),
						&(event->start.time.date.second));
			}
		}
		break;
	case CAL_PROPERTY_EVENT_START_TZID:
		temp = sqlite3_column_text(stmt, *stmt_count);
		event->start_tzid = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_EVENT_END:
		//!!
		event->end.type = sqlite3_column_int(stmt, *stmt_count);
		if (event->end.type == CALENDAR_TIME_UTIME)
		{
			*stmt_count = *stmt_count+1;
			event->end.time.utime = sqlite3_column_int64(stmt,*stmt_count);
			*stmt_count = *stmt_count+1; // dtstart_datetime
		}
		else
		{
			*stmt_count = *stmt_count+1; // dtend_utime
			*stmt_count = *stmt_count+1;
			temp = sqlite3_column_text(stmt, *stmt_count);
			if (temp) {
				sscanf((const char *)temp, CAL_FORMAT_LOCAL_DATETIME, &(event->end.time.date.year),
						&(event->end.time.date.month), &(event->end.time.date.mday),
						&(event->end.time.date.hour), &(event->end.time.date.minute),
						&(event->end.time.date.second));
			}
		}
		break;
	case CAL_PROPERTY_EVENT_END_TZID:
		temp = sqlite3_column_text(stmt, *stmt_count);
		event->end_tzid = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_EVENT_CALENDAR_SYSTEM_TYPE:
		event->system_type = sqlite3_column_int(stmt, *stmt_count);
		break;
	default:
		sqlite3_column_int(stmt, *stmt_count);
		break;
	}

	*stmt_count = *stmt_count+1;
}

static void __cal_db_event_get_projection_stmt(sqlite3_stmt *stmt,
		const unsigned int *projection, const int projection_count,
		calendar_record_h record)
{
	int i=0;
	int stmt_count = 0;

	for(i=0;i<projection_count;i++)
	{
		__cal_db_event_get_property_stmt(stmt,projection[i],&stmt_count,record);
	}
}

static bool __cal_db_event_check_calendar_book_type(calendar_record_h record)
{
	int store_type = 0;
	cal_event_s *event = (cal_event_s *)record;
	char query[CAL_DB_SQL_MAX_LEN];
	sqlite3_stmt *stmt = NULL;

	snprintf(query, sizeof(query), "SELECT store_type "
			"FROM %s WHERE id = %d ",
			CAL_TABLE_CALENDAR, event->calendar_id);

	stmt = _cal_db_util_query_prepare(query);
	if (NULL == stmt)
	{
		ERR("query[%s]", query);
		ERR("_cal_db_util_query_prepare() Failed");
		return false;
	}

	if (CAL_DB_ROW == _cal_db_util_stmt_step(stmt)) {
		store_type = sqlite3_column_int(stmt, 0);
	}
	else {
		sqlite3_finalize(stmt);
		stmt = NULL;
		DBG("Failed to get calendar: calendar_id(%d)", event->calendar_id);
		return false;
	}
	sqlite3_finalize(stmt);

	bool ret = false;
	switch (store_type) {
	case CALENDAR_BOOK_TYPE_NONE:
	case CALENDAR_BOOK_TYPE_EVENT:
		ret = true;
		break;
	case CALENDAR_BOOK_TYPE_TODO:
	default:
		ret = false;
		break;
	}
	return ret;
}

static int __cal_db_event_delete_exception(int *exception_ids, int exception_len)
{
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	cal_db_util_error_e dbret = CAL_DB_OK;

	int i;
	for (i = 0; i < exception_len; i++) {
		snprintf(query, sizeof(query), "DELETE FROM %s WHERE id=%d ",
				CAL_TABLE_SCHEDULE, exception_ids[i]);

		dbret = _cal_db_util_query_exec(query);
		if (dbret != CAL_DB_OK) {
			ERR("_cal_db_util_query_exec() failed (%d)", dbret);
			switch (dbret)
			{
			case CAL_DB_ERROR_NO_SPACE:
				return CALENDAR_ERROR_FILE_NO_SPACE;
			default:
				return CALENDAR_ERROR_DB_FAILED;
			}
		}
	}
	return CALENDAR_ERROR_NONE;
}

static int __cal_db_event_update_dirty(calendar_record_h record, int is_dirty_in_time)
{
	int event_id = 0;
	int ret = CALENDAR_ERROR_NONE;
	calendar_record_h original_record = NULL;

	ret = calendar_record_get_int(record,_calendar_event.id, &event_id);
	RETV_IF(CALENDAR_ERROR_NONE != ret, ret);

	DBG("id=%d",event_id);

	ret = __cal_db_event_get_record(event_id, &original_record);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "__cal_db_event_get_record() is failed(%d)", ret);

	cal_record_s *_record = NULL;
	const cal_property_info_s* property_info = NULL;
	int property_info_count = 0;
	int i=0;

	_record = (cal_record_s *)record;

	property_info = _cal_view_get_property_info(_record->view_uri, &property_info_count);

	for(i=0;i<property_info_count;i++)
	{
		if (false == _cal_record_check_property_flag(record, property_info[i].property_id , CAL_PROPERTY_FLAG_DIRTY))
		{
			continue;
		}

		//DBG("%d",property_info[i].property_id);
		if (property_info[i].property_id == CAL_PROPERTY_EVENT_EXDATE)
		{
			int calendar_id = 0;
			int account_id = 0;
			calendar_book_sync_event_type_e sync_event_type = CALENDAR_BOOK_SYNC_EVENT_FOR_ME;

			char *record_exdate = NULL;
			ret = calendar_record_get_str_p(record,property_info[i]. property_id, &record_exdate);
			if (ret != CALENDAR_ERROR_NONE)
				continue;
			ret = calendar_record_get_int(original_record, CAL_PROPERTY_EVENT_CALENDAR_ID, &calendar_id);
			if (ret != CALENDAR_ERROR_NONE)
				continue;

			calendar_record_h record_calendar = NULL;
			ret = _cal_db_get_record(_calendar_book._uri, calendar_id, &record_calendar);
			ret |= calendar_record_get_int(record_calendar, _calendar_book.account_id, &account_id);
			ret |= calendar_record_get_int(record_calendar, _calendar_book.sync_event, (int *)&sync_event_type);
			DBG("calendar_id(%d), account_id(%d), sync_event(%d)", calendar_id, account_id, sync_event_type);
			calendar_record_destroy(record_calendar, true);

			char *original_exdate = NULL;
			ret = calendar_record_get_str_p(original_record,property_info[i].property_id,&original_exdate);
			if (ret != CALENDAR_ERROR_NONE)
				continue;
			if (sync_event_type == CALENDAR_BOOK_SYNC_EVENT_FOR_EVERY_AND_REMAIN)
			{
				ret = __cal_db_event_exdate_insert_normal(event_id, original_exdate, record_exdate, NULL, NULL);
				WARN_IF(ret != CALENDAR_ERROR_NONE, "%s->%s",original_exdate,record_exdate);

			} else {
				int *exception_ids = NULL;
				int exception_len = 0;
				ret = __cal_db_event_exdate_insert_normal(event_id, original_exdate, record_exdate, &exception_ids, &exception_len);
				WARN_IF(ret != CALENDAR_ERROR_NONE, "%s->%s",original_exdate,record_exdate);
				ret = __cal_db_event_delete_exception(exception_ids, exception_len);
				WARN_IF(ret != CALENDAR_ERROR_NONE, "__cal_db_event_delete_record() is failed");
				free(exception_ids);
			}
			ret = _cal_record_set_str(original_record, property_info[i].property_id, record_exdate);
			if (ret != CALENDAR_ERROR_NONE)
				continue;
		}
		else if (CAL_PROPERTY_CHECK_DATA_TYPE(property_info[i].property_id,CAL_PROPERTY_DATA_TYPE_INT) == true)
		{
			int tmp=0;
			ret = calendar_record_get_int(record,property_info[i].property_id,&tmp);
			if (ret != CALENDAR_ERROR_NONE)
				continue;
			ret = _cal_record_set_int(original_record, property_info[i].property_id, tmp);
			if (ret != CALENDAR_ERROR_NONE)
				continue;
		}
		else if (CAL_PROPERTY_CHECK_DATA_TYPE(property_info[i].property_id,CAL_PROPERTY_DATA_TYPE_STR) == true)
		{
			char *tmp=NULL;
			ret = calendar_record_get_str_p(record,property_info[i].property_id,&tmp);
			if (ret != CALENDAR_ERROR_NONE)
				continue;
			ret = _cal_record_set_str(original_record, property_info[i].property_id, tmp);
			if (ret != CALENDAR_ERROR_NONE)
				continue;
		}
		else if (CAL_PROPERTY_CHECK_DATA_TYPE(property_info[i].property_id,CAL_PROPERTY_DATA_TYPE_DOUBLE) == true)
		{
			double tmp=0;
			ret = calendar_record_get_double(record,property_info[i].property_id,&tmp);
			if (ret != CALENDAR_ERROR_NONE)
				continue;
			ret = _cal_record_set_double(original_record, property_info[i].property_id, tmp);
			if (ret != CALENDAR_ERROR_NONE)
				continue;
		}
		else if (CAL_PROPERTY_CHECK_DATA_TYPE(property_info[i].property_id,CAL_PROPERTY_DATA_TYPE_LLI) == true)
		{
			long long int tmp=0;
			ret = calendar_record_get_lli(record,property_info[i].property_id,&tmp);
			if (ret != CALENDAR_ERROR_NONE)
				continue;
			ret = _cal_record_set_lli(original_record, property_info[i].property_id, tmp);
			if (ret != CALENDAR_ERROR_NONE)
				continue;
		}
		else if (CAL_PROPERTY_CHECK_DATA_TYPE(property_info[i].property_id,CAL_PROPERTY_DATA_TYPE_CALTIME) == true)
		{
			calendar_time_s tmp = {0,};
			ret = calendar_record_get_caltime(record,property_info[i].property_id,&tmp);
			if (ret != CALENDAR_ERROR_NONE)
				continue;
			ret = _cal_record_set_caltime(original_record, property_info[i].property_id, tmp);
			if (ret != CALENDAR_ERROR_NONE)
				continue;
		}
	}
	// child replace
	cal_event_s *tmp = (cal_event_s *)original_record;
	cal_event_s *tmp_src = (cal_event_s *)record;

	if (tmp->alarm_list)
		calendar_list_destroy((calendar_list_h)tmp->alarm_list, true);
	_cal_list_clone((calendar_list_h)tmp_src->alarm_list, (calendar_list_h *)&tmp->alarm_list);

	if (tmp->attendee_list)
		calendar_list_destroy((calendar_list_h)tmp->attendee_list, true);
	_cal_list_clone((calendar_list_h)tmp_src->attendee_list, (calendar_list_h *)&tmp->attendee_list);

	if (tmp->exception_list)
		calendar_list_destroy((calendar_list_h)tmp->exception_list, true);
	_cal_list_clone((calendar_list_h)tmp_src->exception_list, (calendar_list_h *)&tmp->exception_list);

	if (tmp->extended_list)
		calendar_list_destroy((calendar_list_h)tmp->extended_list, true);
	_cal_list_clone((calendar_list_h)tmp_src->extended_list, (calendar_list_h *)&tmp->extended_list);

	CAL_RECORD_RESET_COMMON((cal_record_s*)original_record);
	ret = __update_record(original_record, is_dirty_in_time);
	calendar_record_destroy(original_record, true);

	return ret;
}

static int __cal_db_event_exception_get_records(int original_id, cal_list_s *list)
{
	int ret;
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	sqlite3_stmt *stmt = NULL;

	RETVM_IF(NULL == list, CALENDAR_ERROR_INVALID_PARAMETER,
			"Invalid parameter: list is NULL");

	snprintf(query, sizeof(query),
			"SELECT "CAL_QUERY_SCHEDULE_A_ALL" FROM %s AS A "
			"WHERE original_event_id = %d AND is_deleted = 0 ",
			CAL_TABLE_SCHEDULE,
			original_id);

	stmt = _cal_db_util_query_prepare(query);
	RETVM_IF(NULL == stmt, CALENDAR_ERROR_DB_FAILED, "_cal_db_util_query_prepare() failed");

	calendar_record_h record = NULL;

	while (_cal_db_util_stmt_step(stmt) == CAL_DB_ROW) {
		int exception = 0, extended = 0;
		ret = calendar_record_create(_calendar_event._uri, &record);
		if (CALENDAR_ERROR_NONE != ret) {
			sqlite3_finalize(stmt);
			_cal_list_clear(list);
			return ret;
		}

		__cal_db_event_get_stmt(stmt, false, record, &exception, &extended);

		cal_rrule_s *rrule = NULL;
		cal_event_s *event = (cal_event_s *)record;
		if (_cal_db_rrule_get_rrule(event->index, &rrule) == CALENDAR_ERROR_NONE )
		{
			_cal_db_rrule_set_rrule_to_event(rrule, record);
			CAL_FREE(rrule);
		}

		if (event->has_alarm == 1)
			_cal_db_alarm_get_records(event->index, event->alarm_list);

		if (event->has_attendee == 1)
			_cal_db_attendee_get_records(event->index, event->attendee_list);

		if (exception == 1)
			__cal_db_event_exception_get_records(event->index, event->exception_list);

		if (extended == 1)
			_cal_db_extended_get_records(event->index, CALENDAR_RECORD_TYPE_EVENT, event->extended_list);

		event->has_alarm = 0;
		if (event->alarm_list && 0 < event->alarm_list->count)
			event->has_alarm = 1;

		event->has_attendee = 0;
		if (event->attendee_list && 0 < event->attendee_list->count)
			event->has_attendee = 1;

		calendar_list_add((calendar_list_h)list, record);
	}
	sqlite3_finalize(stmt);
	return CALENDAR_ERROR_NONE;
}

static int __cal_db_event_exception_delete_with_id(int original_id)
{
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	cal_db_util_error_e dbret = CAL_DB_OK;

	DBG("delete exception mod with original event id(%d)", original_id);
	snprintf(query, sizeof(query), "DELETE FROM %s WHERE original_event_id=%d ",
			CAL_TABLE_SCHEDULE, original_id);

	dbret = _cal_db_util_query_exec(query);
	if (dbret != CAL_DB_OK) {
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

static int __cal_db_event_exception_get_ids(int original_id, GList **out_list)
{
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	sqlite3_stmt *stmt = NULL;
	GList *list = NULL;

	RETVM_IF(NULL == out_list, CALENDAR_ERROR_INVALID_PARAMETER,
			"Invalid parameter: GList is NULL");

	snprintf(query, sizeof(query),
			"SELECT id FROM %s "
			"WHERE original_event_id = %d AND is_deleted = 0 ",
			CAL_TABLE_SCHEDULE,
			original_id);

	stmt = _cal_db_util_query_prepare(query);
	if (NULL == stmt)
	{
		ERR("_cal_db_util_query_prepare() failed");
		return CALENDAR_ERROR_DB_FAILED;
	}

	while (_cal_db_util_stmt_step(stmt) == CAL_DB_ROW)
	{
		int id = 0;
		id = sqlite3_column_int(stmt, 0);
		list = g_list_append(list, GINT_TO_POINTER(id));

	}
	sqlite3_finalize(stmt);

	*out_list = list;
	return CALENDAR_ERROR_NONE;
}

static int __cal_db_event_exception_update(cal_list_s *exception_list_s, int original_id, int calendar_id, int is_dirty_in_time, time_t time_diff, int old_type, int new_type)
{
	int count = 0;
	int ret = CALENDAR_ERROR_NONE;
	calendar_record_h exception = NULL;
	calendar_list_h exception_list = (calendar_list_h)exception_list_s;
	GList *id_list = NULL;

	__cal_db_event_exception_get_ids(original_id, &id_list);

	if (exception_list) {
		calendar_list_get_count(exception_list, &count);
		calendar_list_first(exception_list);
		while (CALENDAR_ERROR_NONE == calendar_list_get_current_record_p(exception_list, &exception)) {
			int exception_id = 0;
			ret = calendar_record_get_int(exception,_calendar_event.id, &exception_id);
			DBG("exception(%d)", exception_id);
			if (exception_id > 0 && exception_id != original_id) { // update
				bool bchanged = false;
				cal_record_s *_record = NULL;
				const cal_property_info_s* property_info = NULL;
				int property_info_count = 0;

				_record = (cal_record_s *)exception;

				property_info = _cal_view_get_property_info(_record->view_uri, &property_info_count);

				// check updated
				int i;
				for (i = 0; i < property_info_count; i++) {
					if ( true == _cal_record_check_property_flag(exception, property_info[i].property_id , CAL_PROPERTY_FLAG_DIRTY)) {
						bchanged = true;
						break;
					}
				}

				if (bchanged == true || DIRTY_IN_TIME == is_dirty_in_time) {
					// update
					__update_recurrence_id(exception, old_type, new_type, time_diff);
					ret = __cal_db_event_update_record(exception);
					_cal_db_instance_discard_record(exception_id);
					_cal_db_instance_publish_record(exception);

				} else {
					_cal_db_instance_discard_record(exception_id);
					_cal_db_instance_publish_record(exception);
					DBG("exception don't changed. exception_id=[%d]",exception_id);
				}

				if (id_list)
					id_list = g_list_remove(id_list, GINT_TO_POINTER(exception_id) );

			} else { // insert
				ret = _cal_record_set_int(exception,_calendar_event.calendar_book_id, calendar_id);
				if (ret != CALENDAR_ERROR_NONE) {
					ERR("set fail");
					if (id_list)
						g_list_free(id_list);
					return ret;
				}
				ret = _cal_db_event_insert_record(exception, original_id, NULL);
				if (ret != CALENDAR_ERROR_NONE) {
					ERR("set fail");
					if (id_list)
						g_list_free(id_list);
					return ret;
				}
			}
			calendar_list_next(exception_list);
		}
	}

	if (id_list) {
		GList * tmp_list = g_list_first(id_list);
		while(tmp_list) {
			int tmp = GPOINTER_TO_INT(tmp_list->data);
			char query[CAL_DB_SQL_MAX_LEN] = {0};
			cal_db_util_error_e dbret = CAL_DB_OK;

			snprintf(query, sizeof(query), "DELETE FROM %s WHERE id=%d ", CAL_TABLE_SCHEDULE, tmp);

			dbret = _cal_db_util_query_exec(query);
			if (dbret != CAL_DB_OK) {
				ERR("_cal_db_util_query_exec() failed (%d)", dbret);
			}
			tmp_list = g_list_next(tmp_list);
		}
		g_list_free(id_list);
	}
	return CALENDAR_ERROR_NONE;
}

static int __cal_db_event_get_deleted_data(int id, int* calendar_book_id, int* created_ver,
		int* original_event_id, char** recurrence_id)
{
	char query[CAL_DB_SQL_MAX_LEN];
	sqlite3_stmt *stmt = NULL;

	snprintf(query, sizeof(query), "SELECT calendar_id, created_ver, "
			"original_event_id, recurrence_id "
			"FROM %s WHERE id = %d ",
			CAL_TABLE_SCHEDULE, id);

	stmt = _cal_db_util_query_prepare(query);
	if (NULL == stmt)
	{
		ERR("query[%s]", query);
		ERR("_cal_db_util_query_prepare() Failed");
		return CALENDAR_ERROR_DB_FAILED;
	}

	const unsigned char *tmp;
	if (CAL_DB_ROW == _cal_db_util_stmt_step(stmt))
	{
		*calendar_book_id = sqlite3_column_int(stmt, 0);
		*created_ver = sqlite3_column_int(stmt, 1);
		*original_event_id = sqlite3_column_int(stmt, 2);
		tmp = sqlite3_column_text(stmt, 3);
		*recurrence_id = SAFE_STRDUP(tmp);
	}
	else
	{
		sqlite3_finalize(stmt);
		stmt = NULL;
		DBG("Failed to get deleted_data: event_id(%d)", id);
		return CALENDAR_ERROR_DB_RECORD_NOT_FOUND;
	}

	sqlite3_finalize(stmt);
	stmt = NULL;
	return CALENDAR_ERROR_NONE;
}

static int __cal_db_event_exdate_insert_normal(int event_id, const char* original_exdate, const char* exdate, int **exception_ids, int *exception_len)
{
	int ret = CALENDAR_ERROR_NONE;
	gchar **patterns1 = NULL;
	gchar **patterns2 = NULL;
	int len1 = 0, len2 = 0, i = 0, j = 0;

	int input_ver = _cal_db_util_get_next_ver();
	if (exdate != NULL && strlen(exdate) > 0)
	{
		patterns1 = g_strsplit_set(exdate, " ,", -1);
		len1 = g_strv_length(patterns1);
	}
	if (original_exdate != NULL && strlen(original_exdate) > 0)
	{
		patterns2 = g_strsplit_set(original_exdate, " ,", -1);
		len2 = g_strv_length(patterns2);
	}

	int *ids = calloc(len1, sizeof(int));
	RETVM_IF(NULL == ids, CALENDAR_ERROR_DB_FAILED, "calloc() is failed");

	int exception_count = 0;
	for(i = 0; i < len1; i++)
	{
		bool bFind = false;
		cal_db_util_error_e dbret = CAL_DB_OK;
		for(j = 0; j < len2; j++)
		{
			if (g_strcmp0(patterns1[i], patterns2[j]) == 0)
			{
				bFind = true;
				break;
			}
		}
		if (bFind == false)
		{
			char query[CAL_DB_SQL_MAX_LEN] = {0};
			long long int start_utime = 0;
			char datetime[16] = {0};
			if (strlen(patterns1[i]) > strlen("YYYYMMDD"))
			{
				//DBG("NORMAL instance");
				int y, mon, d, h, min, s;
				sscanf(patterns1[i], "%04d%02d%02dT%02d%02d%02dZ",
						&y, &mon, &d, &h, &min, &s);
				start_utime = _cal_time_convert_itol(NULL, y, mon, d, h, min, s);

				//DBG("(%lld)", start_utime);
			}
			else
			{
				//DBG("ALLDAY instance");
				snprintf(datetime, sizeof(datetime),
						"%s", patterns1[i]);

			}
			// insert DB
			snprintf(query, sizeof(query),
					"INSERT INTO %s ("
					"type, "
					"created_ver, changed_ver, "
					"calendar_id, "
					"original_event_id, "
					"recurrence_id, "
					"is_deleted, "
					"dtstart_type, "
					"dtstart_utime, "
					"dtstart_datetime, "
					"dtstart_tzid, "
					"dtend_type, "
					"dtend_utime, "
					"dtend_datetime, "
					"dtend_tzid"
					") SELECT %d,"
					"created_ver, %d, "
					"calendar_id, "
					"%d, "
					"'%s', "
					"1, "
					"dtstart_type, "
					"%lld, "
					"'%s', "
					"dtstart_tzid, "
					"dtend_type, "
					"%lld+(dtend_utime-dtstart_utime), "
					"'%s', "
					"dtend_tzid "
					"FROM %s "
					"WHERE id = %d; ",
				CAL_TABLE_SCHEDULE,     // INSERT INTO
				CAL_SCH_TYPE_EVENT,     // type
				input_ver,   // ver
				event_id,               // original_event_id
				patterns1[i],
				start_utime,
				datetime,
				start_utime,
				datetime,
				CAL_TABLE_SCHEDULE,
				event_id
					);
			dbret = _cal_db_util_query_exec(query);
			if (dbret != CAL_DB_OK)
			{
				DBG("query[%s]", query);
				ERR("_cal_db_util_query_exec() failed (%d)", dbret);
			}
			int event_id = _cal_db_util_last_insert_id();
			DBG("last id(%d)", event_id);
			ids[exception_count] = event_id;
			exception_count++;
		}
	}
	if (exception_ids) {
		*exception_ids = ids;
	} else {
		free(ids);
	}
	if (exception_len) *exception_len = exception_count;

	g_strfreev(patterns1);
	g_strfreev(patterns2);
	return ret;
}
