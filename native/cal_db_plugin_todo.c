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

#include <stdio.h>
#include <stdlib.h>

#include "calendar_db.h"
#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_view.h"
#include "cal_record.h"
#include "cal_list.h"

#include "cal_db_util.h"
#include "cal_db.h"
#include "cal_db_query.h"
#include "cal_db_rrule.h"
#include "cal_db_alarm.h"
#include "cal_db_attendee.h"
#include "cal_db_extended.h"
#include "cal_access_control.h"

static int __cal_db_todo_insert_record(calendar_record_h record, int* id);
static int __cal_db_todo_get_record(int id, calendar_record_h* out_record);
static int __cal_db_todo_update_record(calendar_record_h record);
static int __cal_db_todo_delete_record(int id);
static int __cal_db_todo_get_all_records(int offset, int limit, calendar_list_h* out_list);
static int __cal_db_todo_get_records_with_query(calendar_query_h query, int offset, int limit, calendar_list_h* out_list);
static int __cal_db_todo_insert_records(const calendar_list_h list, int** ids);
static int __cal_db_todo_update_records(const calendar_list_h list);
static int __cal_db_todo_delete_records(int ids[], int count);
static int __cal_db_todo_get_count(int *out_count);
static int __cal_db_todo_get_count_with_query(calendar_query_h query, int *out_count);
static int __cal_db_todo_replace_record(calendar_record_h record, int id);
static int __cal_db_todo_replace_records(const calendar_list_h list, int ids[], int count);

/*
 * static function
 */
static void __cal_db_todo_get_stmt(sqlite3_stmt *stmt,bool is_view_table,calendar_record_h record, int *extended);
static void __cal_db_todo_get_property_stmt(sqlite3_stmt *stmt,
		unsigned int property, int *stmt_count, calendar_record_h record);
static void __cal_db_todo_get_projection_stmt(sqlite3_stmt *stmt,
		const unsigned int *projection, const int projection_count,
		calendar_record_h record);
static int __cal_db_todo_update_dirty(calendar_record_h record);
static int __cal_db_todo_get_deleted_data(int id, int* calendar_book_id, int* created_ver);
static bool __cal_db_todo_check_calendar_book_type(calendar_record_h record);

cal_db_plugin_cb_s _cal_db_todo_plugin_cb = {
	.is_query_only = false,
	.insert_record = __cal_db_todo_insert_record,
	.get_record = __cal_db_todo_get_record,
	.update_record = __cal_db_todo_update_record,
	.delete_record = __cal_db_todo_delete_record,
	.get_all_records = __cal_db_todo_get_all_records,
	.get_records_with_query = __cal_db_todo_get_records_with_query,
	.insert_records = __cal_db_todo_insert_records,
	.update_records = __cal_db_todo_update_records,
	.delete_records = __cal_db_todo_delete_records,
	.get_count=__cal_db_todo_get_count,
	.get_count_with_query=__cal_db_todo_get_count_with_query,
	.replace_record = __cal_db_todo_replace_record,
	.replace_records = __cal_db_todo_replace_records
};

static int __cal_db_todo_insert_record(calendar_record_h record, int* id)
{
	int ret = -1;
	int index = -1;
	int input_ver;
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	char dtstart_datetime[32] = {0};
	char dtend_datetime[32] = {0};
	sqlite3_stmt *stmt = NULL;
	cal_todo_s* todo =  (cal_todo_s*)(record);
	cal_rrule_s *rrule = NULL;
	cal_db_util_error_e dbret = CAL_DB_OK;
	int tmp = 0;
	int calendar_book_id = 0;
	calendar_record_h record_calendar = NULL;
	int has_alarm = 0;

	retv_if(NULL == todo, CALENDAR_ERROR_INVALID_PARAMETER);
	retv_if(false == __cal_db_todo_check_calendar_book_type(record), CALENDAR_ERROR_INVALID_PARAMETER);

	// access control
	if (_cal_access_control_have_write_permission(todo->calendar_id) == false)
	{
		ERR("_cal_access_control_have_write_permission() failed");
		return CALENDAR_ERROR_PERMISSION_DENIED;
	}

	ret = calendar_record_get_int(record,
			_calendar_todo.calendar_book_id, &calendar_book_id);
	DBG("calendar_book_id(%d)", calendar_book_id);

	ret = _cal_db_get_record(_calendar_book._uri,
			calendar_book_id, &record_calendar);
	retvm_if(CALENDAR_ERROR_NONE != ret, CALENDAR_ERROR_INVALID_PARAMETER, "calendar_book_id is invalid");

	calendar_record_destroy(record_calendar, true);

	has_alarm = _cal_db_alarm_has_alarm(todo->alarm_list);
	input_ver = _cal_db_util_get_next_ver();
	int is_allday = 0;
	if (CALENDAR_TIME_LOCALTIME == todo->start.type
			&& (0 == todo->start.time.date.hour)
			&& (0 == todo->start.time.date.minute)
			&& (0 == todo->start.time.date.second)
			&& (0 == todo->due.time.date.hour)
			&& (0 == todo->due.time.date.minute)
			&& (0 == todo->due.time.date.second)) {
		is_allday = 1;
	}

	ret = snprintf(query, sizeof(query),
			"INSERT INTO %s ("
			"type, "
			"created_ver, changed_ver, "
			"summary, description, location, categories,  "
			"task_status, priority, "
			"sensitivity, uid, "
			"calendar_id, "
			"latitude, longitude, "
			"created_time, completed_time, progress, "
			"dtstart_type, dtstart_utime, dtstart_datetime, dtstart_tzid, "
			"dtend_type, dtend_utime, dtend_datetime, dtend_tzid, "
			"last_mod, rrule_id, "
			"has_alarm, system_type, updated, "
			"sync_data1, sync_data2, sync_data3, sync_data4, "
			"organizer_name, organizer_email, "
			"has_attendee, has_extended, "
			"freq, is_allday "
			") VALUES ( "
			"%d, "
			"%d, %d, "
			"?, ?, ?, ?, "
			"%d, %d, "
			"%d, ?, "
			"%d, "
			"%lf, %lf, "
			"strftime('%%s', 'now'), %lld, %d, "
			"%d, %lld, ?, ?, "
			"%d, %lld, ?, ?, "
			"strftime('%%s', 'now'), %d, "
			"%d, %d, %ld, "
			"?, ?, ?, ?, "
			"?, ?, "
			"%d, %d, "
			"%d, %d) ",
		CAL_TABLE_SCHEDULE,
		CAL_SCH_TYPE_TODO, /*event->cal_type,*/
		input_ver, input_ver,
		todo->todo_status, todo->priority,
		todo->sensitivity,
		todo->calendar_id,
		todo->latitude, todo->longitude,
		todo->completed_time, todo->progress,
		todo->start.type, todo->start.type == CALENDAR_TIME_UTIME ? todo->start.time.utime : 0,
		todo->due.type, todo->due.type == CALENDAR_TIME_UTIME ? todo->due.time.utime : 0,
		todo->freq > 0 ? 1 : 0,
		has_alarm,
		todo->system_type,
		todo->updated,
		(todo->attendee_list && 0 < todo->attendee_list->count) ? 1 : 0,
		(todo->extended_list && 0 < todo->extended_list->count) ? 1 : 0,
		todo->freq, is_allday);

	stmt = _cal_db_util_query_prepare(query);
	if (NULL == stmt)
	{
		ERR("_cal_db_util_query_prepare() Failed");
		SEC_DBG("[%s]", query);
		return CALENDAR_ERROR_DB_FAILED;
	}

	int count = 1;

	if (todo->summary)
		_cal_db_util_stmt_bind_text(stmt, count, todo->summary);
	count++;

	if (todo->description)
		_cal_db_util_stmt_bind_text(stmt, count, todo->description);
	count++;

	if (todo->location)
		_cal_db_util_stmt_bind_text(stmt, count, todo->location);
	count++;

	if (todo->categories)
		_cal_db_util_stmt_bind_text(stmt, count, todo->categories);
	count++;

	if (todo->uid)
		_cal_db_util_stmt_bind_text(stmt, count, todo->uid);
	count++;

	if (CALENDAR_TIME_LOCALTIME == todo->start.type)
	{
		snprintf(dtstart_datetime, sizeof(dtstart_datetime), CAL_FORMAT_LOCAL_DATETIME,
				todo->start.time.date.year,
				todo->start.time.date.month,
				todo->start.time.date.mday,
				todo->start.time.date.hour,
				todo->start.time.date.minute,
				todo->start.time.date.second);
		_cal_db_util_stmt_bind_text(stmt, count, dtstart_datetime);
	}
	count++;

	if (todo->start_tzid)
		_cal_db_util_stmt_bind_text(stmt, count, todo->start_tzid);
	count++;

	if (CALENDAR_TIME_LOCALTIME == todo->due.type)
	{
		snprintf(dtend_datetime, sizeof(dtend_datetime), CAL_FORMAT_LOCAL_DATETIME,
				todo->due.time.date.year,
				todo->due.time.date.month,
				todo->due.time.date.mday,
				todo->due.time.date.hour,
				todo->due.time.date.minute,
				todo->due.time.date.second);
		_cal_db_util_stmt_bind_text(stmt, count, dtend_datetime);
	}
	count++;

	if (todo->due_tzid)
		_cal_db_util_stmt_bind_text(stmt, count, todo->due_tzid);
	count++;

	if (todo->sync_data1)
		_cal_db_util_stmt_bind_text(stmt, count, todo->sync_data1);
	count++;
	if (todo->sync_data2)
		_cal_db_util_stmt_bind_text(stmt, count, todo->sync_data2);
	count++;
	if (todo->sync_data3)
		_cal_db_util_stmt_bind_text(stmt, count, todo->sync_data3);
	count++;
	if (todo->sync_data4)
		_cal_db_util_stmt_bind_text(stmt, count, todo->sync_data4);
	count++;
	if (todo->organizer_name)
		_cal_db_util_stmt_bind_text(stmt, count, todo->organizer_name);
	count++;
	if (todo->organizer_email)
		_cal_db_util_stmt_bind_text(stmt, count, todo->organizer_email);
	count++;

	dbret = _cal_db_util_stmt_step(stmt);
	if (CAL_DB_DONE != dbret)
	{
		sqlite3_finalize(stmt);
		ERR("_cal_db_util_stmt_step() Failed(%d)", dbret);
		switch (dbret)
		{
		case CAL_DB_ERROR_NO_SPACE:
			return CALENDAR_ERROR_FILE_NO_SPACE;
		default:
			return CALENDAR_ERROR_DB_FAILED;
		}
	}
	index = _cal_db_util_last_insert_id();
	sqlite3_finalize(stmt);

	calendar_record_get_int(record, _calendar_todo.id, &tmp);
	_cal_record_set_int(record, _calendar_todo.id, index);
	if (id) {
		*id = index;
	}

	_cal_db_rrule_get_rrule_from_todo(record, &rrule);
	_cal_db_rrule_insert_record(index, rrule);

	if (todo->alarm_list && 0 < todo->alarm_list->count) {
		ret = _cal_db_alarm_insert_records(todo->alarm_list, index);
		warn_if(CALENDAR_ERROR_NONE != ret, "_cal_db_alarm_insert_records() failed(%x)", ret);
	}

	if (todo->attendee_list && 0 < todo->attendee_list->count) {
		ret = _cal_db_attendee_insert_records(todo->attendee_list, index);
		warn_if(CALENDAR_ERROR_NONE != ret, "_cal_db_attendee_insert_records() failed(%x)", ret);
	}

	if (todo->extended_list && 0 < todo->extended_list->count) {
		DBG("insert extended");
		ret = _cal_db_extended_insert_records(todo->extended_list, index, CALENDAR_RECORD_TYPE_TODO);
		warn_if(CALENDAR_ERROR_NONE != ret, "_cal_db_extended_insert_records() Failed(%x)", ret);
	}
	else {
		DBG("No extended");
	}

	CAL_FREE(rrule);
	_cal_db_util_notify(CAL_NOTI_TYPE_TODO);

	_cal_record_set_int(record, _calendar_todo.id, tmp);

	return CALENDAR_ERROR_NONE;
}

static int __cal_db_todo_get_record(int id, calendar_record_h* out_record)
{
	char query[CAL_DB_SQL_MAX_LEN];
	int rc = 0;
	cal_todo_s *todo = NULL;
	cal_rrule_s *rrule = NULL;
	sqlite3_stmt *stmt = NULL;
	cal_db_util_error_e dbret = CAL_DB_OK;
	int extended = 0;
	calendar_record_h record_calendar;
	calendar_book_sync_event_type_e sync_event_type = CALENDAR_BOOK_SYNC_EVENT_FOR_ME;
	int ret = 0;

	rc = calendar_record_create( _calendar_todo._uri ,out_record);
	if (rc != CALENDAR_ERROR_NONE)
	{
		ERR("calendar_record_create(%d)", rc);
		return CALENDAR_ERROR_OUT_OF_MEMORY;
	}

	todo =  (cal_todo_s*)(*out_record);

	snprintf(query, sizeof(query), "SELECT "CAL_QUERY_SCHEDULE_A_ALL" FROM %s AS A "
			"WHERE id=%d AND (type = %d OR type = %d) AND calendar_id IN "
			"(select id from %s where deleted = 0)",
			CAL_TABLE_SCHEDULE,
			id, CALENDAR_BOOK_TYPE_TODO, CALENDAR_BOOK_TYPE_NONE,
			CAL_TABLE_CALENDAR);
	stmt = _cal_db_util_query_prepare(query);
	if (NULL == stmt)
	{
		ERR("_cal_db_util_query_prepare() Failed");
		calendar_record_destroy(*out_record, true);
		*out_record = NULL;
		return CALENDAR_ERROR_DB_FAILED;
	}

	dbret = _cal_db_util_stmt_step(stmt);
	if (dbret != CAL_DB_ROW)
	{
		ERR("Failed to step stmt(%d)[%s]", dbret, query);
		sqlite3_finalize(stmt);
		calendar_record_destroy(*out_record, true);
		*out_record = NULL;
		switch (dbret)
		{
		case CAL_DB_DONE:
			ERR("Failed to find record(%d)", dbret);
			return CALENDAR_ERROR_DB_RECORD_NOT_FOUND;
		case CAL_DB_ERROR_NO_SPACE:
			return CALENDAR_ERROR_FILE_NO_SPACE;
		default:
			return CALENDAR_ERROR_DB_FAILED;
		}
	}

	__cal_db_todo_get_stmt(stmt,false,*out_record, &extended);
	sqlite3_finalize(stmt);
	stmt = NULL;

	//check
	ret = _cal_db_get_record(_calendar_book._uri,
			todo->calendar_id, &record_calendar);
	if (ret == CALENDAR_ERROR_NONE)
	{
		ret = calendar_record_get_int(record_calendar,
				_calendar_book.sync_event, (int *)&sync_event_type);
		calendar_record_destroy(record_calendar, true);
	}
	if (todo->is_deleted == 1
			&& sync_event_type != CALENDAR_BOOK_SYNC_EVENT_FOR_EVERY_AND_REMAIN
		)
	{
		calendar_record_destroy(*out_record, true);
		*out_record = NULL;
		return CALENDAR_ERROR_DB_RECORD_NOT_FOUND;
	}

	if (_cal_db_rrule_get_rrule(todo->index, &rrule) == CALENDAR_ERROR_NONE)
	{
		_cal_db_rrule_set_rrule_to_todo(rrule, *out_record);
		CAL_FREE(rrule);
	}

	if (todo->has_alarm == 1)
		_cal_db_alarm_get_records(todo->index, todo->alarm_list);

	if (todo->has_attendee == 1)
		_cal_db_attendee_get_records(todo->index, todo->attendee_list);

	if (extended == 1)
		_cal_db_extended_get_records(todo->index, CALENDAR_RECORD_TYPE_TODO, todo->extended_list);

	return CALENDAR_ERROR_NONE;
}

static int __cal_db_todo_update_record(calendar_record_h record)
{
	int ret = CALENDAR_ERROR_NONE;
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	char dtstart_datetime[32] = {0};
	char dtend_datetime[32] = {0};
	sqlite3_stmt *stmt = NULL;
	cal_todo_s* todo =  (cal_todo_s*)(record);
	cal_rrule_s *rrule = NULL;
	cal_db_util_error_e dbret = CAL_DB_OK;
	int has_alarm = 0;

	retv_if(NULL == todo, CALENDAR_ERROR_INVALID_PARAMETER);

	// access control
	if (_cal_access_control_have_write_permission(todo->calendar_id) == false)
	{
		ERR("_cal_access_control_have_write_permission() failed");
		return CALENDAR_ERROR_PERMISSION_DENIED;
	}

	//if (CAL_SYNC_STATUS_UPDATED != todo->sync_status)
	//  todo->sync_status = CAL_SYNC_STATUS_UPDATED;
	if (todo->common.properties_flags != NULL)
	{
		return __cal_db_todo_update_dirty(record);
	}

	has_alarm = _cal_db_alarm_has_alarm(todo->alarm_list);
	int is_allday = 0;
	if (CALENDAR_TIME_LOCALTIME == todo->start.type
			&& (0 == todo->start.time.date.hour)
			&& (0 == todo->start.time.date.minute)
			&& (0 == todo->start.time.date.second)
			&& (0 == todo->due.time.date.hour)
			&& (0 == todo->due.time.date.minute)
			&& (0 == todo->due.time.date.second)) {
		is_allday = 1;
	}

	snprintf(query, sizeof(query), "UPDATE %s SET "
			"changed_ver = %d,"
			"type = %d,"
			"summary = ?,"
			"description = ?,"
			"location = ?,"
			"categories = ?,"
			"task_status = %d,"
			"priority = %d,"
			"sensitivity = %d, "
			"uid = ?, "
			"calendar_id = %d, "
			"latitude = %lf,"
			"longitude = %lf,"
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
			"has_alarm = %d, "
			"system_type = %d, "
			"updated = %ld, "
			"sync_data1 = ?, "
			"sync_data2 = ?, "
			"sync_data3 = ?, "
			"sync_data4 = ?, "
			"organizer_name = ?, "
			"organizer_email = ?, "
			"has_attendee = %d,"
			"has_extended = %d, "
			"is_allday = %d "
			"WHERE id = %d;",
		CAL_TABLE_SCHEDULE,
		_cal_db_util_get_next_ver(),
		CAL_SCH_TYPE_TODO,/*todo->cal_type,*/
		todo->todo_status,
		todo->priority,
		todo->sensitivity,
		todo->calendar_id,
		todo->latitude,
		todo->longitude,
		todo->completed_time,
		todo->progress,
		todo->start.type,
		todo->start.type == CALENDAR_TIME_UTIME ? todo->start.time.utime : 0,
		todo->due.type,
		todo->due.type == CALENDAR_TIME_UTIME ? todo->due.time.utime : 0,
		has_alarm,
		todo->system_type,
		todo->updated,
		(todo->attendee_list && 0 < todo->attendee_list->count) ? 1 : 0,
		(todo->extended_list && 0 < todo->extended_list->count) ? 1 : 0,
		is_allday,
		todo->index);

	stmt = _cal_db_util_query_prepare(query);
	retvm_if(NULL == stmt, CALENDAR_ERROR_DB_FAILED, "_cal_db_util_query_prepare() Failed");

	int count = 1;

	if (todo->summary)
		_cal_db_util_stmt_bind_text(stmt, count, todo->summary);
	count++;

	if (todo->description)
		_cal_db_util_stmt_bind_text(stmt, count, todo->description);
	count++;

	if (todo->location)
		_cal_db_util_stmt_bind_text(stmt, count, todo->location);
	count++;

	if (todo->categories)
		_cal_db_util_stmt_bind_text(stmt, count, todo->categories);
	count++;

	if (todo->uid)
		_cal_db_util_stmt_bind_text(stmt, count, todo->uid);
	count++;

	if (CALENDAR_TIME_LOCALTIME == todo->start.type)
	{
		snprintf(dtstart_datetime, sizeof(dtstart_datetime), CAL_FORMAT_LOCAL_DATETIME,
				todo->start.time.date.year,
				todo->start.time.date.month,
				todo->start.time.date.mday,
				todo->start.time.date.hour,
				todo->start.time.date.minute,
				todo->start.time.date.second);
		_cal_db_util_stmt_bind_text(stmt, count, dtstart_datetime);
	}
	count++;

	if (todo->start_tzid)
		_cal_db_util_stmt_bind_text(stmt, count, todo->start_tzid);
	count++;

	if (CALENDAR_TIME_LOCALTIME == todo->due.type)
	{
		snprintf(dtend_datetime, sizeof(dtend_datetime), CAL_FORMAT_LOCAL_DATETIME,
				todo->due.time.date.year,
				todo->due.time.date.month,
				todo->due.time.date.mday,
				todo->due.time.date.hour,
				todo->due.time.date.minute,
				todo->due.time.date.second);
		_cal_db_util_stmt_bind_text(stmt, count, dtend_datetime);
	}
	count++;

	if (todo->due_tzid)
		_cal_db_util_stmt_bind_text(stmt, count, todo->due_tzid);
	count++;

	if (todo->sync_data1)
		_cal_db_util_stmt_bind_text(stmt, count, todo->sync_data1);
	count++;
	if (todo->sync_data2)
		_cal_db_util_stmt_bind_text(stmt, count, todo->sync_data2);
	count++;
	if (todo->sync_data3)
		_cal_db_util_stmt_bind_text(stmt, count, todo->sync_data3);
	count++;
	if (todo->sync_data4)
		_cal_db_util_stmt_bind_text(stmt, count, todo->sync_data4);
	count++;
	if (todo->organizer_name)
		_cal_db_util_stmt_bind_text(stmt, count, todo->organizer_name);
	count++;
	if (todo->organizer_email)
		_cal_db_util_stmt_bind_text(stmt, count, todo->organizer_email);
	count++;

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

	_cal_db_rrule_get_rrule_from_todo(record, &rrule);
	_cal_db_rrule_update_record(todo->index, rrule);
	CAL_FREE(rrule);

	//_cal_db_instance_publish_record(record);

	_cal_db_alarm_delete_with_id(todo->index);
	_cal_db_attendee_delete_with_id(todo->index);
	_cal_db_extended_delete_with_id(todo->index, CALENDAR_RECORD_TYPE_TODO);

	if (todo->alarm_list && 0 < todo->alarm_list->count) {
		ret = _cal_db_alarm_insert_records(todo->alarm_list, todo->index);
		warn_if(CALENDAR_ERROR_NONE != ret, "_cal_db_alarm_insert_records() Failed(%d)", ret);
	}

	if (todo->attendee_list && 0 < todo->attendee_list->count) {
		ret = _cal_db_attendee_insert_records(todo->attendee_list, todo->index);
		warn_if(CALENDAR_ERROR_NONE != ret, "_cal_db_attendee_insert_records() Failed(%d)", ret);
	}


	if (todo->extended_list && 0 < todo->extended_list->count) {
		DBG("insert extended");
		ret = _cal_db_extended_insert_records(todo->extended_list, todo->index, CALENDAR_RECORD_TYPE_TODO);
		warn_if(CALENDAR_ERROR_NONE != ret, "_cal_db_extended_insert_records() Failed(%d)", ret);
	}

	_cal_db_util_notify(CAL_NOTI_TYPE_TODO);

	return CALENDAR_ERROR_NONE;
}

static int __cal_db_todo_delete_record(int id)
{
	int ret = CALENDAR_ERROR_NONE;
	cal_db_util_error_e dbret = CAL_DB_OK;
	int calendar_book_id;
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	int created_ver = 0;
	calendar_book_sync_event_type_e sync_event_type = CALENDAR_BOOK_SYNC_EVENT_FOR_ME;

	retvm_if(id < 0, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid argument: id < 0");

	ret = __cal_db_todo_get_deleted_data(id, &calendar_book_id, &created_ver);
	if (CALENDAR_ERROR_NONE != ret)
	{
		DBG("__cal_db_event_get_deleted_data() failed");
		return ret;
	}

	//access control
	if (_cal_access_control_have_write_permission(calendar_book_id) == false)
	{
		ERR("fail");
		return CALENDAR_ERROR_PERMISSION_DENIED;
	}

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
	}
	else
	{
		_cal_db_util_get_next_ver();

		DBG("delete event");
		snprintf(query, sizeof(query),
				"DELETE FROM %s "
				"WHERE id = %d ",
				CAL_TABLE_SCHEDULE,
				id);

		dbret = _cal_db_util_query_exec(query);
		if (CAL_DB_OK != dbret)
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
	_cal_db_util_notify(CAL_NOTI_TYPE_TODO);

	return CALENDAR_ERROR_NONE;
}

static int __cal_db_todo_replace_record(calendar_record_h record, int id)
{
	int ret = CALENDAR_ERROR_NONE;
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	char dtstart_datetime[32] = {0};
	char dtend_datetime[32] = {0};
	sqlite3_stmt *stmt = NULL;
	cal_todo_s* todo =  (cal_todo_s*)(record);
	cal_rrule_s *rrule = NULL;
	cal_db_util_error_e dbret = CAL_DB_OK;
	int has_alarm = 0;

	retv_if(NULL == todo, CALENDAR_ERROR_INVALID_PARAMETER);
	todo->index = id;

	// access control
	if (_cal_access_control_have_write_permission(todo->calendar_id) == false)
	{
		ERR("_cal_access_control_have_write_permission() failed");
		return CALENDAR_ERROR_PERMISSION_DENIED;
	}

	//if (CAL_SYNC_STATUS_UPDATED != todo->sync_status)
	//  todo->sync_status = CAL_SYNC_STATUS_UPDATED;
	if (todo->common.properties_flags != NULL)
	{
		return __cal_db_todo_update_dirty(record);
	}
	int is_allday = 0;
	if (CALENDAR_TIME_LOCALTIME == todo->start.type
			&& (0 == todo->start.time.date.hour)
			&& (0 == todo->start.time.date.minute)
			&& (0 == todo->start.time.date.second)
			&& (0 == todo->due.time.date.hour)
			&& (0 == todo->due.time.date.minute)
			&& (0 == todo->due.time.date.second)) {
		is_allday = 1;
	}

	has_alarm = _cal_db_alarm_has_alarm(todo->alarm_list);
	snprintf(query, sizeof(query), "UPDATE %s SET "
			"changed_ver = %d,"
			"type = %d,"
			"summary = ?,"
			"description = ?,"
			"location = ?,"
			"categories = ?,"
			"task_status = %d,"
			"priority = %d,"
			"sensitivity = %d, "
			"uid = ?, "
			"calendar_id = %d, "
			"latitude = %lf,"
			"longitude = %lf,"
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
			"has_alarm = %d, "
			"system_type = %d, "
			"updated = %ld, "
			"sync_data1 = ?, "
			"sync_data2 = ?, "
			"sync_data3 = ?, "
			"sync_data4 = ?, "
			"organizer_name = ?, "
			"organizer_email = ?, "
			"has_attendee = %d,"
			"has_extended = %d, "
			"is_allday = %d "
			"WHERE id = %d;",
		CAL_TABLE_SCHEDULE,
		_cal_db_util_get_next_ver(),
		CAL_SCH_TYPE_TODO,/*todo->cal_type,*/
		todo->todo_status,
		todo->priority,
		todo->sensitivity,
		todo->calendar_id,
		todo->latitude,
		todo->longitude,
		todo->completed_time,
		todo->progress,
		todo->start.type,
		todo->start.type == CALENDAR_TIME_UTIME ? todo->start.time.utime : 0,
		todo->due.type,
		todo->due.type == CALENDAR_TIME_UTIME ? todo->due.time.utime : 0,
		has_alarm,
		todo->system_type,
		todo->updated,
		(todo->attendee_list && 0 < todo->attendee_list->count) ? 1 : 0,
		(todo->extended_list && 0 < todo->extended_list->count) ? 1 : 0,
		is_allday,
		id);

	stmt = _cal_db_util_query_prepare(query);
	retvm_if(NULL == stmt, CALENDAR_ERROR_DB_FAILED, "_cal_db_util_query_prepare() Failed");

	int count = 1;

	if (todo->summary)
		_cal_db_util_stmt_bind_text(stmt, count, todo->summary);
	count++;

	if (todo->description)
		_cal_db_util_stmt_bind_text(stmt, count, todo->description);
	count++;

	if (todo->location)
		_cal_db_util_stmt_bind_text(stmt, count, todo->location);
	count++;

	if (todo->categories)
		_cal_db_util_stmt_bind_text(stmt, count, todo->categories);
	count++;

	if (todo->uid)
		_cal_db_util_stmt_bind_text(stmt, count, todo->uid);
	count++;

	if (CALENDAR_TIME_LOCALTIME == todo->start.type)
	{
		snprintf(dtstart_datetime, sizeof(dtstart_datetime), CAL_FORMAT_LOCAL_DATETIME,
				todo->start.time.date.year,
				todo->start.time.date.month,
				todo->start.time.date.mday,
				todo->start.time.date.hour,
				todo->start.time.date.minute,
				todo->start.time.date.second);
		_cal_db_util_stmt_bind_text(stmt, count, dtstart_datetime);
	}
	count++;

	if (todo->start_tzid)
		_cal_db_util_stmt_bind_text(stmt, count, todo->start_tzid);
	count++;

	if (CALENDAR_TIME_LOCALTIME == todo->due.type)
	{
		snprintf(dtend_datetime, sizeof(dtend_datetime), CAL_FORMAT_LOCAL_DATETIME,
				todo->due.time.date.year,
				todo->due.time.date.month,
				todo->due.time.date.mday,
				todo->due.time.date.hour,
				todo->due.time.date.minute,
				todo->due.time.date.second);
		_cal_db_util_stmt_bind_text(stmt, count, dtend_datetime);
	}
	count++;

	if (todo->due_tzid)
		_cal_db_util_stmt_bind_text(stmt, count, todo->due_tzid);
	count++;

	if (todo->sync_data1)
		_cal_db_util_stmt_bind_text(stmt, count, todo->sync_data1);
	count++;
	if (todo->sync_data2)
		_cal_db_util_stmt_bind_text(stmt, count, todo->sync_data2);
	count++;
	if (todo->sync_data3)
		_cal_db_util_stmt_bind_text(stmt, count, todo->sync_data3);
	count++;
	if (todo->sync_data4)
		_cal_db_util_stmt_bind_text(stmt, count, todo->sync_data4);
	count++;
	if (todo->organizer_name)
		_cal_db_util_stmt_bind_text(stmt, count, todo->organizer_name);
	count++;
	if (todo->organizer_email)
		_cal_db_util_stmt_bind_text(stmt, count, todo->organizer_email);
	count++;

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

	_cal_db_rrule_get_rrule_from_todo(record, &rrule);
	_cal_db_rrule_update_record(id, rrule);
	CAL_FREE(rrule);

	//_cal_db_instance_publish_record(record);

	_cal_db_alarm_delete_with_id(id);
	_cal_db_attendee_delete_with_id(id);
	_cal_db_extended_delete_with_id(id, CALENDAR_RECORD_TYPE_TODO);

	if (todo->alarm_list && 0 < todo->alarm_list->count) {
		ret = _cal_db_alarm_insert_records(todo->alarm_list, id);
		warn_if(CALENDAR_ERROR_NONE != ret, "_cal_db_alarm_insert_records() failed(%x)", ret);
	}

	if (todo->attendee_list && 0 < todo->attendee_list->count) {
		ret = _cal_db_attendee_insert_records(todo->attendee_list, id);
		warn_if(CALENDAR_ERROR_NONE != ret, "_cal_db_attendee_insert_records() failed(%x)", ret);
	}

	if (todo->extended_list && 0 < todo->extended_list->count) {
		DBG("insert extended");
		ret = _cal_db_extended_insert_records(todo->extended_list, id, CALENDAR_RECORD_TYPE_TODO);
		warn_if(CALENDAR_ERROR_NONE != ret, "_cal_db_extended_insert_records() Failed(%d)", ret);
	}

	_cal_db_util_notify(CAL_NOTI_TYPE_TODO);

	return CALENDAR_ERROR_NONE;
}

static int __cal_db_todo_get_all_records(int offset, int limit, calendar_list_h* out_list)
{
	int ret = CALENDAR_ERROR_NONE;
	char offsetquery[CAL_DB_SQL_MAX_LEN] = {0};
	char limitquery[CAL_DB_SQL_MAX_LEN] = {0};
	sqlite3_stmt *stmt = NULL;

	retvm_if(NULL == out_list, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");

	ret = calendar_list_create(out_list);
	retvm_if(CALENDAR_ERROR_NONE != ret, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");

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
	_cal_db_append_string(&query_str, CAL_VIEW_TABLE_TODO);
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
		int extended = 0;
		// stmt -> record
		ret = calendar_record_create(_calendar_todo._uri,&record);
		if( ret != CALENDAR_ERROR_NONE )
		{
			calendar_list_destroy(*out_list, true);
			*out_list = NULL;
			sqlite3_finalize(stmt);
			CAL_FREE(query_str);
			return ret;
		}
		__cal_db_todo_get_stmt(stmt,true,record, &extended);

		// child
		int has_attendee = 0, has_alarm = 0;
		int record_id = 0;
		cal_todo_s* ptodo = (cal_todo_s*) record;
		calendar_record_get_int(record, _calendar_todo.id, &record_id);
		if(calendar_record_get_int(record, _calendar_todo.has_attendee,&has_attendee) == CALENDAR_ERROR_NONE)
		{
			if( has_attendee == 1)
			{
				_cal_db_attendee_get_records(record_id, ptodo->attendee_list);
			}
		}
		if(calendar_record_get_int(record, _calendar_todo.has_alarm,&has_alarm) == CALENDAR_ERROR_NONE)
		{
			if( has_alarm == 1)
			{
				_cal_db_alarm_get_records(record_id, ptodo->alarm_list);
			}
		}

		if (extended == 1)
			_cal_db_extended_get_records(record_id, CALENDAR_RECORD_TYPE_TODO, ptodo->extended_list);

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

static int __cal_db_todo_get_records_with_query(calendar_query_h query, int offset, int limit, calendar_list_h* out_list)
{
	cal_query_s *que = NULL;
	int ret = CALENDAR_ERROR_NONE;
	char *condition = NULL;
	char *projection = NULL;
	GSList *bind_text = NULL, *cursor = NULL;
	sqlite3_stmt *stmt = NULL;
	int i = 0;
	char *table_name;

	que = (cal_query_s *)query;

	if ( 0 == strcmp(que->view_uri, CALENDAR_VIEW_TODO))
	{
		table_name = SAFE_STRDUP(CAL_VIEW_TABLE_TODO);
	}
	else if ( 0 == strcmp(que->view_uri, CALENDAR_VIEW_TODO_CALENDAR))
	{
		table_name = SAFE_STRDUP(CAL_VIEW_TABLE_TODO_CALENDAR);
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
	CAL_DBG("%s",query_str);

	// bind text
	if (bind_text)
	{
		g_slist_length(bind_text);
		for (cursor=bind_text, i=1; cursor;cursor=cursor->next, i++)
		{
			_cal_db_util_stmt_bind_text(stmt, i, cursor->data);
		}
	}

	//
	ret = calendar_list_create(out_list);
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
		int extended = 1;
		int attendee = 1, alarm = 1;
		// stmt -> record
		ret = calendar_record_create(_calendar_todo._uri,&record);
		if( ret != CALENDAR_ERROR_NONE )
		{
			calendar_list_destroy(*out_list, true);
			*out_list = NULL;

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

			__cal_db_todo_get_projection_stmt(stmt,
					que->projection, que->projection_count,
					record);
		}
		else
		{
			cal_todo_s *todo = NULL;
			__cal_db_todo_get_stmt(stmt,true,record, &extended);
			todo = (cal_todo_s*)(record);
			if (todo)
			{
				attendee = todo->has_attendee;
				alarm = todo->has_alarm;
			}
		}

		// child
		if (_cal_db_query_find_projection_property(query,CAL_PROPERTY_TODO_CALENDAR_ALARM) == true && alarm ==1)
		{
			cal_todo_s* todo = (cal_todo_s*) record;
			_cal_db_alarm_get_records(todo->index, todo->alarm_list);
		}
		if (_cal_db_query_find_projection_property(query,CAL_PROPERTY_TODO_CALENDAR_ATTENDEE) == true && attendee==1)
		{
			cal_todo_s* todo = (cal_todo_s*) record;
			_cal_db_attendee_get_records(todo->index, todo->attendee_list);
		}
		if (_cal_db_query_find_projection_property(query,CAL_PROPERTY_TODO_EXTENDED) == true && extended == 1)
		{
			cal_todo_s* todo = (cal_todo_s*) record;
			_cal_db_extended_get_records(todo->index, CALENDAR_RECORD_TYPE_TODO, todo->extended_list);
		}

		ret = calendar_list_add(*out_list,record);
		if( ret != CALENDAR_ERROR_NONE )
		{
			calendar_list_destroy(*out_list, true);
			*out_list = NULL;
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

	return CALENDAR_ERROR_NONE;
}

static int __cal_db_todo_insert_records(const calendar_list_h list, int** ids)
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

	retvm_if(NULL == id, CALENDAR_ERROR_OUT_OF_MEMORY, "calloc fail");

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
			if( __cal_db_todo_insert_record(record, &id[i]) != CALENDAR_ERROR_NONE)
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

static int __cal_db_todo_update_records(const calendar_list_h list)
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
			if( __cal_db_todo_update_record(record) != CALENDAR_ERROR_NONE)
			{
				ERR("db insert error");
				return CALENDAR_ERROR_DB_FAILED;
			}
		}
	} while(calendar_list_next(list) != CALENDAR_ERROR_NO_DATA);

	return CALENDAR_ERROR_NONE;
}

static int __cal_db_todo_delete_records(int ids[], int count)
{
	int i=0;
	for(i=0;i<count;i++)
	{
		if (__cal_db_todo_delete_record(ids[i]) != CALENDAR_ERROR_NONE)
		{
			ERR("delete failed");
			return CALENDAR_ERROR_DB_FAILED;
		}
	}
	return CALENDAR_ERROR_NONE;
}

static int __cal_db_todo_get_count(int *out_count)
{
	retvm_if(NULL == out_count, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");

	char *query_str = NULL;
	_cal_db_append_string(&query_str, "SELECT count(*) FROM");
	_cal_db_append_string(&query_str, CAL_VIEW_TABLE_TODO);

	int ret = 0;
	int count = 0;
	ret = _cal_db_util_query_get_first_int_result(query_str, NULL, &count);
	if (CALENDAR_ERROR_NONE != ret)
	{
		ERR("_cal_db_util_query_get_first_int_result() failed");
		CAL_FREE(query_str);
		return ret;
	}
	CAL_DBG("count(%d) str[%s]", count, query_str);
	CAL_FREE(query_str);

	*out_count = count;
	return CALENDAR_ERROR_NONE;
}

static int __cal_db_todo_replace_records(const calendar_list_h list, int ids[], int count)
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
			if( __cal_db_todo_replace_record(record, ids[i]) != CALENDAR_ERROR_NONE)
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


static int __cal_db_todo_get_count_with_query(calendar_query_h query, int *out_count)
{
	cal_query_s *que = NULL;
	int ret = CALENDAR_ERROR_NONE;
	char *condition = NULL;
	char *table_name;
	int count = 0;
	GSList *bind_text = NULL;

	que = (cal_query_s *)query;

	if ( 0 == strcmp(que->view_uri, CALENDAR_VIEW_TODO))
	{
		table_name = SAFE_STRDUP(CAL_VIEW_TABLE_TODO);
	}
	else if ( 0 == strcmp(que->view_uri, CALENDAR_VIEW_TODO_CALENDAR))
	{
		table_name = SAFE_STRDUP(CAL_VIEW_TABLE_TODO_CALENDAR);
	}
	else
	{
		ERR("uri(%s) not support get records with query",que->view_uri);
		return CALENDAR_ERROR_INVALID_PARAMETER;
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
	CAL_DBG("count(%d) str[%s]", count, query_str);

	if (out_count) *out_count = count;
	if (bind_text)
	{
		g_slist_free_full(bind_text, free);
		bind_text = NULL;
	}

	CAL_FREE(query_str);
	return CALENDAR_ERROR_NONE;
}

static void __cal_db_todo_get_stmt(sqlite3_stmt *stmt,bool is_view_table,calendar_record_h record, int *extended)
{
	cal_todo_s *todo = NULL;
	const unsigned char *temp;
	int count = 0;

	todo = (cal_todo_s*)(record);

	todo->index = sqlite3_column_int(stmt, count++);
	sqlite3_column_int(stmt, count++);//todo->cal_type = 1;/*sqlite3_column_int(stmt, count++);*/

	temp = sqlite3_column_text(stmt, count++);
	todo->summary = SAFE_STRDUP(temp);
	temp = sqlite3_column_text(stmt, count++);
	todo->description = SAFE_STRDUP(temp);

	temp = sqlite3_column_text(stmt, count++);
	todo->location = SAFE_STRDUP(temp);

	temp = sqlite3_column_text(stmt, count++);
	todo->categories = SAFE_STRDUP(temp);

	sqlite3_column_text(stmt, count++);
	//todo->exdate = SAFE_STRDUP(temp);

	todo->todo_status = sqlite3_column_int(stmt, count++);
	todo->priority = sqlite3_column_int(stmt, count++);
	sqlite3_column_int(stmt, count++);//todo->timezone = sqlite3_column_int(stmt, count++);
	sqlite3_column_int(stmt, count++);//todo->contact_id = sqlite3_column_int(stmt, count++);
	sqlite3_column_int(stmt, count++);//todo->busy_status = sqlite3_column_int(stmt, count++);
	todo->sensitivity = sqlite3_column_int(stmt, count++);

	temp = sqlite3_column_text(stmt, count++);
	todo->uid = SAFE_STRDUP(temp);

	temp = sqlite3_column_text(stmt, count++);
	todo->organizer_name = SAFE_STRDUP(temp);

	temp = sqlite3_column_text(stmt, count++);
	todo->organizer_email = SAFE_STRDUP(temp);

	sqlite3_column_int(stmt, count++);//todo->meeting_status = sqlite3_column_int(stmt, count++);

	todo->calendar_id = sqlite3_column_int(stmt, count++);

	sqlite3_column_int(stmt, count++);//todo->original_todo_id = sqlite3_column_int(stmt, count++);

	todo->latitude = sqlite3_column_double(stmt,count++);
	todo->longitude = sqlite3_column_double(stmt,count++);
	sqlite3_column_int(stmt, count++);//todo->email_id = sqlite3_column_int(stmt, count++);

	todo->created_time = sqlite3_column_int64(stmt, count++);

	todo->completed_time = sqlite3_column_int64(stmt, count++);

	todo->progress = sqlite3_column_int(stmt,count++);

	sqlite3_column_int(stmt,count++);   //__version
	sqlite3_column_int(stmt,count++);   //__version
	todo->is_deleted = sqlite3_column_int(stmt,count++);

	todo->start.type = sqlite3_column_int(stmt,count++);

	if (todo->start.type == CALENDAR_TIME_UTIME)
	{
		todo->start.time.utime = sqlite3_column_int64(stmt,count++);
		count++; // dtstart_datetime
	}
	else
	{
		count++; // dtstart_utime
		temp = sqlite3_column_text(stmt, count++);
		if (temp) {
			sscanf((const char *)temp, CAL_FORMAT_LOCAL_DATETIME, &(todo->start.time.date.year),
					&(todo->start.time.date.month), &(todo->start.time.date.mday),
					&(todo->start.time.date.hour), &(todo->start.time.date.minute),
					&(todo->start.time.date.second));
		}
	}

	temp = sqlite3_column_text(stmt, count++);
	todo->start_tzid = SAFE_STRDUP(temp);
	todo->due.type = sqlite3_column_int(stmt, count++);
	if (todo->due.type == CALENDAR_TIME_UTIME)
	{
		todo->due.time.utime = sqlite3_column_int64(stmt,count++);
		count++; // due_datetime
	}
	else
	{
		count++;
		temp = sqlite3_column_text(stmt, count++);
		if (temp) {
			sscanf((const char *)temp, CAL_FORMAT_LOCAL_DATETIME, &(todo->due.time.date.year),
					&(todo->due.time.date.month), &(todo->due.time.date.mday),
					&(todo->due.time.date.hour), &(todo->due.time.date.minute),
					&(todo->due.time.date.second));
		}
	}
	temp = sqlite3_column_text(stmt, count++);
	todo->due_tzid = SAFE_STRDUP(temp);

	todo->last_mod = sqlite3_column_int64(stmt,count++);
	sqlite3_column_int(stmt,count++);//todo->rrule_id = sqlite3_column_int(stmt,count++);

	sqlite3_column_text(stmt, count++);
	//todo->recurrence_id = SAFE_STRDUP(temp);
	sqlite3_column_text(stmt, count++);
	//todo->rdate = SAFE_STRDUP(temp);
	todo->has_attendee = sqlite3_column_int(stmt,count++);
	todo->has_alarm = sqlite3_column_int(stmt,count++);
	todo->system_type = sqlite3_column_int(stmt,count++); // system_type...
	todo->updated = sqlite3_column_int(stmt,count++);
	temp = sqlite3_column_text(stmt, count++);
	todo->sync_data1 = SAFE_STRDUP(temp);
	temp = sqlite3_column_text(stmt, count++);
	todo->sync_data2 = SAFE_STRDUP(temp);
	temp = sqlite3_column_text(stmt, count++);
	todo->sync_data3 = SAFE_STRDUP(temp);
	temp = sqlite3_column_text(stmt, count++);
	todo->sync_data4 = SAFE_STRDUP(temp);

	//has_exception
	sqlite3_column_int(stmt,count++);

	//has_extended
	if (extended != NULL)
		*extended = sqlite3_column_int(stmt,count++);

	todo->freq = sqlite3_column_int(stmt, count++);
	todo->is_allday = sqlite3_column_int(stmt, count++);

	if (is_view_table == true)
	{
		if (todo->freq <= 0) {
			//todo->rrule_id = 0;
			//sqlite3_finalize(stmt);
			//return CALENDAR_ERROR_NONE;
			return ;
		}

		//todo->rrule_id = 1;
		todo->range_type = sqlite3_column_int(stmt, count++);
		todo->until.type = sqlite3_column_int(stmt, count++);
		todo->until.time.utime = sqlite3_column_int64(stmt, count++);

		temp = sqlite3_column_text(stmt, count++);
		if (CALENDAR_TIME_LOCALTIME == todo->until.type)
		{
			sscanf((const char *)temp, CAL_FORMAT_LOCAL_DATETIME,
					&todo->until.time.date.year,
					&todo->until.time.date.month,
					&todo->until.time.date.mday,
					&todo->until.time.date.hour,
					&todo->until.time.date.minute,
					&todo->until.time.date.second);
		}

		todo->count = sqlite3_column_int(stmt, count++);
		todo->interval = sqlite3_column_int(stmt, count++);

		temp = sqlite3_column_text(stmt, count++);
		todo->bysecond = SAFE_STRDUP(temp);

		temp = sqlite3_column_text(stmt, count++);
		todo->byminute = SAFE_STRDUP(temp);

		temp = sqlite3_column_text(stmt, count++);
		todo->byhour = SAFE_STRDUP(temp);

		temp = sqlite3_column_text(stmt, count++);
		todo->byday= SAFE_STRDUP(temp);

		temp = sqlite3_column_text(stmt, count++);
		todo->bymonthday= SAFE_STRDUP(temp);

		temp = sqlite3_column_text(stmt, count++);
		todo->byyearday= SAFE_STRDUP(temp);

		temp = sqlite3_column_text(stmt, count++);
		todo->byweekno= SAFE_STRDUP(temp);

		temp = sqlite3_column_text(stmt, count++);
		todo->bymonth= SAFE_STRDUP(temp);

		temp = sqlite3_column_text(stmt, count++);
		todo->bysetpos = SAFE_STRDUP(temp);

		todo->wkst = sqlite3_column_int(stmt, count++);

		sqlite3_column_int(stmt, count++); //calendar deleted
	}
}

static void __cal_db_todo_get_property_stmt(sqlite3_stmt *stmt,
		unsigned int property, int *stmt_count, calendar_record_h record)
{
	cal_todo_s *todo = NULL;
	const unsigned char *temp;

	todo = (cal_todo_s*)(record);

	switch(property)
	{
	case CAL_PROPERTY_TODO_ID:
		todo->index = sqlite3_column_int(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_TODO_CALENDAR_ID:
		todo->calendar_id = sqlite3_column_int(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_TODO_SUMMARY:
		temp = sqlite3_column_text(stmt, *stmt_count);
		todo->summary = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_TODO_DESCRIPTION:
		temp = sqlite3_column_text(stmt, *stmt_count);
		todo->description = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_TODO_LOCATION:
		temp = sqlite3_column_text(stmt, *stmt_count);
		todo->location = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_TODO_CATEGORIES:
		temp = sqlite3_column_text(stmt, *stmt_count);
		todo->categories = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_TODO_TODO_STATUS:
		todo->todo_status = sqlite3_column_int(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_TODO_PRIORITY:
		todo->priority = sqlite3_column_int(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_TODO_SENSITIVITY:
		todo->sensitivity = sqlite3_column_int(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_TODO_UID:
		temp = sqlite3_column_text(stmt, *stmt_count);
		todo->uid = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_TODO_LATITUDE:
		todo->latitude = sqlite3_column_double(stmt,*stmt_count);
		break;
	case CAL_PROPERTY_TODO_LONGITUDE:
		todo->longitude = sqlite3_column_double(stmt,*stmt_count);
		break;
	case CAL_PROPERTY_TODO_PROGRESS:
		todo->progress = sqlite3_column_int(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_TODO_COMPLETED_TIME:
		todo->completed_time = sqlite3_column_int64(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_TODO_CREATED_TIME:
		todo->created_time = sqlite3_column_int64(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_TODO_LAST_MODIFIED_TIME:
		todo->last_mod = sqlite3_column_int64(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_TODO_IS_DELETED:
		todo->is_deleted = sqlite3_column_int(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_TODO_FREQ:
		todo->freq = sqlite3_column_int(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_TODO_RANGE_TYPE:
		todo->range_type = sqlite3_column_int(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_TODO_UNTIL:
		//!!
		break;
	case CAL_PROPERTY_TODO_COUNT:
		todo->count = sqlite3_column_int(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_TODO_INTERVAL:
		todo->interval = sqlite3_column_int(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_TODO_BYSECOND:
		temp = sqlite3_column_text(stmt, *stmt_count);
		todo->bysecond = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_TODO_BYMINUTE:
		temp = sqlite3_column_text(stmt, *stmt_count);
		todo->byminute = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_TODO_BYHOUR:
		temp = sqlite3_column_text(stmt, *stmt_count);
		todo->byhour = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_TODO_BYDAY:
		temp = sqlite3_column_text(stmt, *stmt_count);
		todo->byday = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_TODO_BYMONTHDAY:
		temp = sqlite3_column_text(stmt, *stmt_count);
		todo->bymonthday = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_TODO_BYYEARDAY:
		temp = sqlite3_column_text(stmt, *stmt_count);
		todo->byyearday = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_TODO_BYWEEKNO:
		temp = sqlite3_column_text(stmt, *stmt_count);
		todo->byweekno = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_TODO_BYMONTH:
		temp = sqlite3_column_text(stmt, *stmt_count);
		todo->bymonth = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_TODO_BYSETPOS:
		temp = sqlite3_column_text(stmt, *stmt_count);
		todo->bysetpos = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_TODO_WKST:
		todo->wkst = sqlite3_column_int(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_TODO_HAS_ALARM:
		todo->has_alarm = sqlite3_column_int(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_TODO_SYNC_DATA1:
		temp = sqlite3_column_text(stmt, *stmt_count);
		todo->sync_data1 = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_TODO_SYNC_DATA2:
		temp = sqlite3_column_text(stmt, *stmt_count);
		todo->sync_data2 = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_TODO_SYNC_DATA3:
		temp = sqlite3_column_text(stmt, *stmt_count);
		todo->sync_data3 = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_TODO_SYNC_DATA4:
		temp = sqlite3_column_text(stmt, *stmt_count);
		todo->sync_data4 = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_TODO_START:
		//!!
		todo->start.type = sqlite3_column_int(stmt,*stmt_count);
		if (todo->start.type == CALENDAR_TIME_UTIME)
		{
			*stmt_count = *stmt_count+1;
			todo->start.time.utime = sqlite3_column_int64(stmt,*stmt_count);
			*stmt_count = *stmt_count+1; // dtstart_datetime
		}
		else
		{
			*stmt_count = *stmt_count+1; // dtstart_utime
			*stmt_count = *stmt_count+1;
			temp = sqlite3_column_text(stmt, *stmt_count);
			if (temp) {
				sscanf((const char *)temp, CAL_FORMAT_LOCAL_DATETIME, &(todo->start.time.date.year),
						&(todo->start.time.date.month), &(todo->start.time.date.mday),
						&(todo->start.time.date.hour), &(todo->start.time.date.minute),
						&(todo->start.time.date.second));
			}
		}
		break;
	case CAL_PROPERTY_TODO_START_TZID:
		temp = sqlite3_column_text(stmt, *stmt_count);
		todo->start_tzid = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_TODO_DUE:
		//!!
		todo->due.type = sqlite3_column_int(stmt, *stmt_count);
		if (todo->due.type == CALENDAR_TIME_UTIME)
		{
			*stmt_count = *stmt_count+1;
			todo->due.time.utime = sqlite3_column_int64(stmt,*stmt_count);
			*stmt_count = *stmt_count+1; // due_datetime
		}
		else
		{
			*stmt_count = *stmt_count+1; // due_utime
			*stmt_count = *stmt_count+1;
			temp = sqlite3_column_text(stmt, *stmt_count);
			if (temp) {
				sscanf((const char *)temp, CAL_FORMAT_LOCAL_DATETIME, &(todo->due.time.date.year),
						&(todo->due.time.date.month), &(todo->due.time.date.mday),
						&(todo->due.time.date.hour), &(todo->due.time.date.minute),
						&(todo->due.time.date.second));
			}
		}
		break;
	case CAL_PROPERTY_TODO_DUE_TZID:
		temp = sqlite3_column_text(stmt, *stmt_count);
		todo->due_tzid = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_TODO_ORGANIZER_NAME:
		temp = sqlite3_column_text(stmt, *stmt_count);
		todo->organizer_name = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_TODO_ORGANIZER_EMAIL:
		temp = sqlite3_column_text(stmt, *stmt_count);
		todo->organizer_email = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_TODO_HAS_ATTENDEE:
		todo->has_attendee = sqlite3_column_int(stmt, *stmt_count);
		break;
	default:
		sqlite3_column_int(stmt, *stmt_count);
		break;
	}

	*stmt_count = *stmt_count+1;
}

static void __cal_db_todo_get_projection_stmt(sqlite3_stmt *stmt,
		const unsigned int *projection, const int projection_count,
		calendar_record_h record)
{
	int i=0;
	int stmt_count = 0;

	for(i=0;i<projection_count;i++)
	{
		__cal_db_todo_get_property_stmt(stmt,projection[i],&stmt_count,record);
	}
}

static bool __cal_db_todo_check_calendar_book_type(calendar_record_h record)
{
	int store_type = 0;
	cal_todo_s *todo = (cal_todo_s *)record;
	char query[CAL_DB_SQL_MAX_LEN];
	sqlite3_stmt *stmt = NULL;

	snprintf(query, sizeof(query), "SELECT store_type "
			"FROM %s WHERE id = %d ",
			CAL_TABLE_CALENDAR, todo->calendar_id);

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
		DBG("Failed to get calendar: calendar_id(%d)", todo->calendar_id);
		return false;
	}
	sqlite3_finalize(stmt);

	bool ret = false;
	switch (store_type) {
	case CALENDAR_BOOK_TYPE_NONE:
	case CALENDAR_BOOK_TYPE_TODO:
		ret = true;
		break;
	case CALENDAR_BOOK_TYPE_EVENT:
	default:
		ret = false;
		break;
	}
	return ret;
}


static int __cal_db_todo_update_dirty(calendar_record_h record)
{
	int todo_id = 0;
	int ret = CALENDAR_ERROR_NONE;
	calendar_record_h original_record;

	ret = calendar_record_get_int(record,_calendar_todo.id, &todo_id);
	retv_if(CALENDAR_ERROR_NONE != ret, ret);

	CAL_DBG("id=%d",todo_id);

	retv_if(false == __cal_db_todo_check_calendar_book_type(record), CALENDAR_ERROR_INVALID_PARAMETER);

	ret = __cal_db_todo_get_record(todo_id, &original_record);

	if (ret == CALENDAR_ERROR_NONE)
	{
		cal_record_s *_record = NULL;
		const cal_property_info_s* property_info = NULL;
		int property_info_count = 0;
		int i=0;

		_record = (cal_record_s *)record;

		property_info = _cal_view_get_property_info(_record->view_uri, &property_info_count);

		for(i=0;i<property_info_count;i++)
		{
			if (true == _cal_record_check_property_flag(record, property_info[i].property_id , CAL_PROPERTY_FLAG_DIRTY))
			{
				if (CAL_PROPERTY_CHECK_DATA_TYPE(property_info[i].property_id,CAL_PROPERTY_DATA_TYPE_INT) == true)
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
		}

		// child replace
		// alarm
		cal_todo_s *tmp = (cal_todo_s *)original_record;
		cal_todo_s *tmp_src = (cal_todo_s *)record;

		_cal_list_clone((calendar_list_h)tmp_src->alarm_list, (calendar_list_h *)&tmp->alarm_list);
		_cal_list_clone((calendar_list_h)tmp_src->attendee_list, (calendar_list_h *)&tmp->attendee_list);
		_cal_list_clone((calendar_list_h)tmp_src->extended_list, (calendar_list_h *)&tmp->extended_list);

	}
	else
	{
		CAL_DBG("get_record fail");
		return ret;
	}

	CAL_RECORD_RESET_COMMON((cal_record_s*)original_record);
	ret = __cal_db_todo_update_record(original_record);

	calendar_record_destroy(original_record, true);

	return ret;
}

static int __cal_db_todo_get_deleted_data(int id, int* calendar_book_id, int* created_ver)
{
	char query[CAL_DB_SQL_MAX_LEN];
	sqlite3_stmt *stmt = NULL;

	snprintf(query, sizeof(query), "SELECT calendar_id, created_ver "
			"FROM %s WHERE id = %d ",
			CAL_TABLE_SCHEDULE, id);

	stmt = _cal_db_util_query_prepare(query);
	if (NULL == stmt)
	{
		ERR("query[%s]", query);
		ERR("_cal_db_util_query_prepare() Failed");
		return CALENDAR_ERROR_DB_FAILED;
	}

	if (CAL_DB_ROW == _cal_db_util_stmt_step(stmt))
	{
		*calendar_book_id = sqlite3_column_int(stmt, 0);
		*created_ver = sqlite3_column_int(stmt, 1);
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
