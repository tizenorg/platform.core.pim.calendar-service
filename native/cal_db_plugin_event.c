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

#include "cal_db_util.h"
#include "cal_db.h"
#include "cal_db_rrule.h"
#include "cal_db_query.h"
#include "cal_db_alarm.h"
#include "cal_db_instance.h"
#include "cal_db_attendee.h"
#include "cal_db_extended.h"

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
static void __cal_db_event_get_stmt(sqlite3_stmt *stmt,bool is_view_table,calendar_record_h record);
static void __cal_db_event_get_property_stmt(sqlite3_stmt *stmt,
        unsigned int property, int *stmt_count, calendar_record_h record);
static void __cal_db_event_get_projection_stmt(sqlite3_stmt *stmt,
        const unsigned int *projection, const int projection_count,
        calendar_record_h record);
static int __cal_db_event_update_dirty(calendar_record_h record);
/*  // under construction
bool __cal_db_event_check_changed_rrule(record);
*/
static int __cal_db_event_exception_get_records(int original_id, GList **out_list);
static int __cal_db_event_exception_convert_gtoh(GList *glist, int original_id, int calendar_id, calendar_list_h *hlist);
static int __cal_db_event_exception_delete_with_id(int original_id);

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

static int __cal_db_event_check_value_validation(cal_event_s *event)
{
	retv_if(NULL == event, CALENDAR_ERROR_INVALID_PARAMETER);

	if (event->start.type != event->end.type)
	{
		ERR("start type(%d) is not same as end type(%d)", event->start.type, event->end.type);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	switch (event->start.type)
	{
	case CALENDAR_TIME_UTIME:
		if (event->start.time.utime > event->end.time.utime)
		{
			ERR("normal start(%lld) > end(%lld)",
				event->start.time.utime, event->end.time.utime);
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
		break;

	case CALENDAR_TIME_LOCALTIME:
		// check invalid value
		if (event->start.time.date.month < 1 || event->start.time.date.month > 12)
		{
			ERR("check start month(input:%d)", event->start.time.date.month);
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
		else if (event->start.time.date.mday < 1 || event->start.time.date.mday > 31)
		{
			ERR("check start mday(input:%d)", event->start.time.date.mday);
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
		else if (event->end.time.date.month < 1 || event->end.time.date.month > 12)
		{
			ERR("check end month(input:%d)", event->end.time.date.month);
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
		else if (event->end.time.date.mday < 1 || event->end.time.date.mday > 31)
		{
			ERR("check end mday(input:%d)", event->end.time.date.mday);
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}

		// check start > end
		if (event->start.time.date.year > event->end.time.date.year)
		{
			ERR("allday start year(%d) > end year(%d)",
					event->start.time.date.year > event->end.time.date.year);
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
		else
		{
			if (event->start.time.date.month > event->end.time.date.month)
			{
				ERR("allday start month(%d) > end month(%d)",
						event->start.time.date.month, event->end.time.date.month);
				return CALENDAR_ERROR_INVALID_PARAMETER;
			}
			else
			{
				if (event->start.time.date.mday > event->end.time.date.mday)
				{
					ERR("allday start day(%d) > end day(%d)",
							event->start.time.date.mday, event->end.time.date.mday);
					return CALENDAR_ERROR_INVALID_PARAMETER;
				}
			}
		}
		break;
	}

	return CALENDAR_ERROR_NONE;
}

static int __cal_db_event_update_original_event_version(int original_event_id, int version)
{
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	cal_db_util_error_e dbret = CAL_DB_OK;

	DBG("original_event(%d) changed_ver updated", original_event_id);
	if (original_event_id > 0)
	{
		snprintf(query, sizeof(query), "UPDATE %s SET "
				"changed_ver = %d WHERE id = %d ",
				CAL_TABLE_SCHEDULE, version, original_event_id);

		dbret = _cal_db_util_query_exec(query);
		if(CAL_DB_DONE != dbret)
		{
			ERR("_cal_db_util_query_exec() Failed");
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

static int __cal_db_event_insert_record(calendar_record_h record, int* id)
{
	int ret = -1;
	int event_id = -1;
	int index;
	int input_ver;
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	char dtstart_datetime[32] = {0};
	char dtend_datetime[32] = {0};
	sqlite3_stmt *stmt = NULL;
	cal_event_s* event =  (cal_event_s*)(record);
	cal_rrule_s *rrule = NULL;
	cal_db_util_error_e dbret = CAL_DB_OK;
	int tmp = 0;
	int calendar_book_id = 0;
	calendar_record_h record_calendar = NULL;
	int has_alarm = 0;
	int timezone_id = 0;

	retv_if(NULL == event, CALENDAR_ERROR_INVALID_PARAMETER);

	ret = __cal_db_event_check_value_validation(event);
	if (CALENDAR_ERROR_NONE != ret)
	{
		ERR("__cal_db_event_check_value_validation() failed");
		return ret;
	}

    ret = calendar_record_get_int(record, _calendar_event.calendar_book_id, &calendar_book_id);
    DBG("calendar_book_id(%d)", calendar_book_id);

    ret = calendar_db_get_record(_calendar_book._uri, calendar_book_id, &record_calendar);
    retvm_if(CALENDAR_ERROR_NONE != ret, CALENDAR_ERROR_INVALID_PARAMETER, "calendar_book_id is invalid");

    calendar_record_destroy(record_calendar, true);

    has_alarm = _cal_db_alarm_has_alarm(event->alarm_list);
	_cal_time_get_timezone_from_table(event->start_tzid, NULL, &timezone_id);
	input_ver = _cal_db_util_get_next_ver();
	ret = snprintf(query, sizeof(query),
		"INSERT INTO %s ("
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
			"email_id, availability, "
			"created_time, completed_time, progress, "
			"dtstart_type, dtstart_utime, dtstart_datetime, dtstart_tzid, "
			"dtend_type, dtend_utime, dtend_datetime, dtend_tzid, "
			"last_mod, rrule_id, "
	        "recurrence_id, rdate, has_attendee, "
	        "has_alarm, system_type, updated, "
	        "sync_data1, sync_data2, sync_data3, sync_data4 "
			") VALUES ( "
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
			"%d, %d, "
			"strftime('%%s', 'now'), %lld, %d, "
			"%d, %lld, ?, ?, "
			"%d, %lld, ?, ?, "
			"strftime('%%s', 'now'), %d "
	        ", ?, ?, %d, %d, %d, %ld"
	        ", ?, ?, ?, ?"
			") ",
			CAL_TABLE_SCHEDULE,
			CAL_SCH_TYPE_EVENT, /*event->cal_type,*/
			input_ver, input_ver,
			event->event_status, event->priority,
			event->timezone ? event->timezone : timezone_id,
			event->contact_id, event->busy_status, event->sensitivity,
			event->meeting_status,
			event->calendar_id,
			event->original_event_id,
			event->latitude, event->longitude,
			event->email_id, 0,//event->availability,
			(long long int)0, 0, //event->completed_time, event->progress,
			event->start.type, event->start.type == CALENDAR_TIME_UTIME ? event->start.time.utime : 0,
			event->end.type, event->end.type == CALENDAR_TIME_UTIME ? event->end.time.utime : 0,
			event->freq > 0 ? 1 : 0,
			event->attendee_list ? 1 : 0,
			has_alarm,
			event->system_type,
			event->updated);

	stmt = _cal_db_util_query_prepare(query);
	retvm_if(NULL == stmt, CALENDAR_ERROR_DB_FAILED,
			"_cal_db_util_query_prepare() Failed");

	index = 1;

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
		snprintf(dtstart_datetime, sizeof(dtstart_datetime), "%04d%02d%02d",
				event->start.time.date.year,
				event->start.time.date.month,
				event->start.time.date.mday);
		_cal_db_util_stmt_bind_text(stmt, index, dtstart_datetime);
	}
	index++;

	if (event->start_tzid)
		_cal_db_util_stmt_bind_text(stmt, index, event->start_tzid);
	index++;

	if (CALENDAR_TIME_LOCALTIME == event->end.type)
	{
		snprintf(dtend_datetime, sizeof(dtend_datetime), "%04d%02d%02d",
				event->end.time.date.year,
				event->end.time.date.month,
				event->end.time.date.mday);
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
	event_id = _cal_db_util_last_insert_id();
	sqlite3_finalize(stmt);

	// update parent event changed ver in case this event is exception mod
	// which is original_event_id > 0
	__cal_db_event_update_original_event_version(event->original_event_id, input_ver);

	calendar_record_get_int(record, _calendar_event.id, &tmp);
	_cal_record_set_int(record, _calendar_event.id, event_id);
	if (id)
    {
        *id = event_id;
    }

	_cal_db_rrule_set_default(record);
	_cal_db_rrule_get_rrule_from_event(record, &rrule);
	if (rrule)
	{
		_cal_db_rrule_insert_record(event_id, rrule);
		CAL_FREE(rrule);
	}
	_cal_db_instance_publish_record(record);

	calendar_list_h list;
	if (event->alarm_list)
	{
		list = NULL;
		DBG("insert alarm");
		ret = _cal_db_alarm_convert_gtoh(event->alarm_list, event_id, &list);
		ret = calendar_db_insert_records(list, NULL, NULL);
		ret = calendar_list_destroy(list, false);
	}
	else
	{
		DBG("No alarm");
	}

	if (event->attendee_list)
	{
		list = NULL;
		DBG("insert attendee");
		ret = _cal_db_attendee_convert_gtoh(event->attendee_list, event_id, &list);
		ret = calendar_db_insert_records(list, NULL, NULL);
		ret = calendar_list_destroy(list, false);
	}
	else
	{
		DBG("No attendee");
	}

	if (event->original_event_id < 0)
	{
		if (event->exception_list)
		{
			DBG("insert exception");
			list = NULL;
			ret = __cal_db_event_exception_convert_gtoh(event->exception_list, event_id, event->calendar_id, &list);
			ret = calendar_db_insert_records(list, NULL, NULL);
			ret = calendar_list_destroy(list, false);
		}
		else
		{
			DBG("No exception");
		}
	}

	if (event->extended_list)
	{
	    DBG("insert extended");
        list = NULL;
        ret = _cal_db_extended_convert_gtoh(event->extended_list, event_id, CALENDAR_RECORD_TYPE_EVENT, &list);
        ret = calendar_db_insert_records(list, NULL, NULL);
        ret = calendar_list_destroy(list, false);
	}
    else
    {
        DBG("No extended");
    }

	_cal_db_util_notify(CAL_NOTI_TYPE_EVENT);

	_cal_record_set_int(record, _calendar_event.id, tmp);

	return CALENDAR_ERROR_NONE;
}

static int __cal_db_event_get_record(int id, calendar_record_h* out_record)
{
	char query[CAL_DB_SQL_MAX_LEN];
	int ret = 0;
	sqlite3_stmt *stmt = NULL;
	cal_event_s *event = NULL;
	cal_rrule_s *rrule = NULL;
	GList *alarm_list, *attendee_list;
	GList *exception_list = NULL, *extended_list = NULL;
	cal_db_util_error_e dbret = CAL_DB_OK;

	ret = calendar_record_create( _calendar_event._uri ,out_record);
	if (ret != CALENDAR_ERROR_NONE)
	{
		ERR("calendar_record_create(%d)", ret);
	    return CALENDAR_ERROR_OUT_OF_MEMORY;
	}

	event = (cal_event_s*)(*out_record);

	snprintf(query, sizeof(query),
			"SELECT * FROM %s "
			"WHERE id = %d AND type = %d AND calendar_id IN "
			"(select id from %s where deleted = 0)",
			CAL_TABLE_SCHEDULE,
			id, CALENDAR_BOOK_TYPE_EVENT,
			CAL_TABLE_CALENDAR);
	stmt = _cal_db_util_query_prepare(query);
	if (NULL == stmt)
	{
		ERR("query[%s]", query);
		ERR("_cal_db_util_query_prepare() Failed");
		calendar_record_destroy(*out_record, true);
		*out_record = NULL;
		return CALENDAR_ERROR_DB_FAILED;
	}

	dbret = _cal_db_util_stmt_step(stmt);
	if (dbret != CAL_DB_ROW)
	{
		ERR("query[%s]", query);
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

	__cal_db_event_get_stmt(stmt,false,*out_record);
	sqlite3_finalize(stmt);
	stmt = NULL;

	if (_cal_db_rrule_get_rrule(event->index, &rrule) == CALENDAR_ERROR_NONE )
	{
	    _cal_db_rrule_set_rrule_to_event(rrule, *out_record);
	    CAL_FREE(rrule);
	}

	_cal_db_alarm_get_records(event->index, &alarm_list);
	event->alarm_list = alarm_list;

	_cal_db_attendee_get_records(event->index, &attendee_list);
	event->attendee_list = attendee_list;

	__cal_db_event_exception_get_records(event->index, &exception_list);
	event->exception_list = exception_list;

	_cal_db_extended_get_records(event->index, CALENDAR_RECORD_TYPE_EVENT, &extended_list);
    event->extended_list = extended_list;

	event->has_alarm = 0;
	if (event->alarm_list)
	{
        if (g_list_length(event->alarm_list) != 0)
        {
            event->has_alarm = 1;
        }
	}
    event->has_attendee = 0;
    if (event->attendee_list)
    {
        if (g_list_length(event->attendee_list) != 0)
        {
            event->has_attendee = 1;
        }
    }

	return CALENDAR_ERROR_NONE;
}

static int __cal_db_event_update_record(calendar_record_h record)
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

	retv_if(NULL == event, CALENDAR_ERROR_INVALID_PARAMETER);

	ret = __cal_db_event_check_value_validation(event);
	if (CALENDAR_ERROR_NONE != ret)
	{
		ERR("__cal_db_event_check_value_validation() failed");
		return ret;
	}

    if (event->common.properties_flags != NULL)
    {
        return __cal_db_event_update_dirty(record);
    }
    has_alarm = _cal_db_alarm_has_alarm(event->alarm_list);
	_cal_time_get_timezone_from_table(event->start_tzid, NULL, &timezone_id);
	input_ver = _cal_db_util_get_next_ver();
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
	        "availability = %d,"
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
	        "sync_data4 = ? "
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
		0,//event->availability,
		(long long int)0,//event->completed_time,
		0,//event->progress,
		event->start.type,
		event->start.type == CALENDAR_TIME_UTIME ? event->start.time.utime : 0,
		event->end.type,
		event->end.type == CALENDAR_TIME_UTIME ? event->end.time.utime : 0,
		event->freq > 0 ? 1 : 0,
		event->attendee_list ? 1: 0,
		has_alarm,
		event->system_type,
		event->updated,
		event->index);

	stmt = _cal_db_util_query_prepare(query);
	retvm_if(NULL == stmt, CALENDAR_ERROR_DB_FAILED, "_cal_db_util_query_prepare() Failed");

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
		snprintf(dtstart_datetime, sizeof(dtstart_datetime), "%04d%02d%02d",
				event->start.time.date.year,
				event->start.time.date.month,
				event->start.time.date.mday);
		_cal_db_util_stmt_bind_text(stmt, index, dtstart_datetime);
	}
	index++;

	if (event->start_tzid)
		_cal_db_util_stmt_bind_text(stmt, index, event->start_tzid);
	index++;

	if (CALENDAR_TIME_LOCALTIME == event->end.type)
	{
		snprintf(dtend_datetime, sizeof(dtend_datetime), "%04d%02d%02d",
				event->end.time.date.year,
				event->end.time.date.month,
				event->end.time.date.mday);
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
	__cal_db_event_update_original_event_version(event->original_event_id, input_ver);

	_cal_db_rrule_get_rrule_from_event(record, &rrule);
	if (rrule)
	{
		_cal_db_rrule_update_record(event->index, rrule);
		CAL_FREE(rrule);
	}

	ret = _cal_db_instance_discard_record(record);
	retvm_if(ret != CALENDAR_ERROR_NONE, CALENDAR_ERROR_DB_FAILED,
			"_cal_db_instance_discard_record() Failed(%d)", ret);

	_cal_db_instance_publish_record(record);

	_cal_db_alarm_delete_with_id(event->index);
	_cal_db_attendee_delete_with_id(event->index);
	__cal_db_event_exception_delete_with_id(event->index);
	_cal_db_extended_delete_with_id(event->index, CALENDAR_RECORD_TYPE_EVENT);

	calendar_list_h list;

	list = NULL;
	ret = _cal_db_alarm_convert_gtoh(event->alarm_list,event->index, &list);
	if (ret == CALENDAR_ERROR_NONE)
	{
	    calendar_db_insert_records(list, NULL, NULL);
        calendar_list_destroy(list, false);
	}

	list = NULL;
	ret = _cal_db_attendee_convert_gtoh(event->attendee_list,event->index, &list);
    if (ret == CALENDAR_ERROR_NONE)
    {
        calendar_db_insert_records(list, NULL, NULL);
        calendar_list_destroy(list, false);
    }

    if (event->exception_list)
    {
        DBG("insert exception");
        list = NULL;
        ret = __cal_db_event_exception_convert_gtoh(event->exception_list, event->index, event->calendar_id, &list);
        if (ret == CALENDAR_ERROR_NONE)
        {
            calendar_db_insert_records(list, NULL, NULL);
            calendar_list_destroy(list, false);
        }
    }

    if (event->extended_list)
    {
        DBG("insert extended");
        list = NULL;
        ret = _cal_db_extended_convert_gtoh(event->extended_list, event->index, CALENDAR_RECORD_TYPE_EVENT, &list);
        if (ret == CALENDAR_ERROR_NONE)
        {
            calendar_db_insert_records(list, NULL, NULL);
            calendar_list_destroy(list, false);
        }
    }

	_cal_db_util_notify(CAL_NOTI_TYPE_EVENT);

	return CALENDAR_ERROR_NONE;
}

static int __cal_db_event_add_exdate(calendar_record_h record)
{
	char query[CAL_DB_SQL_MAX_LEN];
	sqlite3_stmt *stmt = NULL;
	cal_db_util_error_e dbret = CAL_DB_OK;
	cal_event_s *event = (cal_event_s *)record;
	if (NULL == event)
	{
		ERR("Invalid parameter: event is NULL");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (event->original_event_id < 0)
	{
		return CALENDAR_ERROR_NONE;
	}
	DBG("This is exception event");

	// get exdate from original event.
	snprintf(query, sizeof(query), "SELECT exdate FROM %s WHERE id = %d ",
			CAL_TABLE_SCHEDULE, event->original_event_id);

	stmt = _cal_db_util_query_prepare(query);
	if (NULL == stmt)
	{
		ERR("query[%s]", query);
		ERR("_cal_db_util_query_prepare() Failed");
		return CALENDAR_ERROR_DB_FAILED;
	}

	// add recurrence id to end of the exdate of original event.
    const unsigned char *temp;
	int len = 0;
	char *exdate = NULL;
	if (CAL_DB_ROW == _cal_db_util_stmt_step(stmt))
	{
		temp = sqlite3_column_text(stmt, 0);
		if (NULL == temp)
		{
			exdate = strdup(event->recurrence_id);
		}
		else
		{
			if (strstr((char *)temp, event->recurrence_id))
			{
				DBG("warn: recurrence id already is registered to exdate");
				sqlite3_finalize(stmt);
				return CALENDAR_ERROR_NONE;
			}
			len = strlen((const char *)temp) + strlen(event->recurrence_id) + 2;
			exdate = calloc(len, sizeof(char));
			if (NULL == exdate)
			{
				ERR("calloc() failed");
				sqlite3_finalize(stmt);
				return CALENDAR_ERROR_DB_FAILED;
			}
			snprintf(exdate, len, "%s,%s", temp, event->recurrence_id);
		}
	}
	else
	{
		DBG("Failed to get exdate: event_id(%d)", event->original_event_id);
	}
	sqlite3_finalize(stmt);
	stmt = NULL;

	// update exdate
	DBG("update to recurrence id to exdate[%s]", exdate);
	snprintf(query, sizeof(query), "UPDATE %s SET exdate = ? WHERE id = %d ",
			CAL_TABLE_SCHEDULE, event->original_event_id);

	stmt = _cal_db_util_query_prepare(query);
	if (NULL == stmt)
	{
		DBG("query[%s]", query);
		ERR("_cal_db_util_query_prepare() failed");
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
	int ret;
	cal_db_util_error_e dbret = CAL_DB_OK;
	int calendar_book_id;
	int account_id;
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	calendar_record_h record_event;
	calendar_record_h record_calendar;

	retvm_if(id < 0, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid argument: id < 0");

	ret = calendar_db_get_record(_calendar_event._uri, id, &record_event);
	if (CALENDAR_ERROR_NONE != ret)
	{
		DBG("calendar_db_get_record() failed");
		return CALENDAR_ERROR_DB_RECORD_NOT_FOUND;
	}
	ret = calendar_record_get_int(record_event,
			_calendar_event.calendar_book_id, &calendar_book_id);
	DBG("calendar_book_id(%d)", calendar_book_id);

	ret = calendar_db_get_record(_calendar_book._uri,
			calendar_book_id, &record_calendar);
	ret = calendar_record_get_int(record_calendar,
			_calendar_book.account_id, &account_id);
	DBG("account_id(%d)", account_id);

	if (account_id == LOCAL_ACCOUNT_ID) {
		DBG("insert deleted table");
		snprintf(query, sizeof(query),
				"INSERT INTO %s ( "
				"schedule_id, schedule_type, "
				"calendar_id, deleted_ver "
				") VALUES ( "
				"%d, %d, "
				"%d, %d ) ",
				CAL_TABLE_DELETED,
				id, CAL_RECORD_TYPE_EVENT,
				calendar_book_id, _cal_db_util_get_next_ver());
		DBG("query[%s]", query);

		dbret = _cal_db_util_query_exec(query);
		if(CAL_DB_OK != dbret)
		{
			ERR("_cal_db_util_query_exec() Failed");
			calendar_record_destroy(record_event, true);
			calendar_record_destroy(record_calendar, true);
			switch (dbret)
			{
			case CAL_DB_ERROR_NO_SPACE:
				return CALENDAR_ERROR_FILE_NO_SPACE;
			default:
				return CALENDAR_ERROR_DB_FAILED;
			}
		}

		DBG("delete event");
		snprintf(query, sizeof(query), "DELETE FROM %s WHERE id = %d ",
				CAL_TABLE_SCHEDULE, id);

		dbret = _cal_db_util_query_exec(query);
		if(CAL_DB_OK != dbret)
		{
			DBG("query[%s]", query);
			ERR("_cal_db_util_query_exec() Failed");
			calendar_record_destroy(record_event, true);
			calendar_record_destroy(record_calendar, true);
			switch (dbret)
			{
			case CAL_DB_ERROR_NO_SPACE:
				return CALENDAR_ERROR_FILE_NO_SPACE;
			default:
				return CALENDAR_ERROR_DB_FAILED;
			}
		}

		DBG("attendee, alarm and rrule is deleted by trigger");

	} else {
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
			calendar_record_destroy(record_event, true);
			calendar_record_destroy(record_calendar, true);
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

	ret = _cal_db_instance_discard_record(record_event);
	retvm_if(ret != CALENDAR_ERROR_NONE, CALENDAR_ERROR_DB_FAILED,
			"_cal_db_instance_discard_record() Failed(%d)", ret);

	// start:add record to exdate if this record is exception mod.
	__cal_db_event_add_exdate(record_event);

	ret = calendar_record_destroy(record_event, true);
	ret = calendar_record_destroy(record_calendar, true);

	_cal_db_util_notify(CAL_NOTI_TYPE_EVENT);

	return CALENDAR_ERROR_NONE;
}

static int __cal_db_event_get_all_records(int offset, int limit, calendar_list_h* out_list)
{
    int ret = CALENDAR_ERROR_NONE;
    char query[CAL_DB_SQL_MAX_LEN] = {0};
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
    snprintf(query, sizeof(query), "SELECT * FROM %s %s %s", CAL_VIEW_TABLE_EVENT,limitquery,offsetquery);

    stmt = _cal_db_util_query_prepare(query);
    if (NULL == stmt)
    {
        ERR("_cal_db_util_query_prepare() Failed");
        calendar_list_destroy(*out_list, true);
		*out_list = NULL;
        return CALENDAR_ERROR_DB_FAILED;
    }

    while(CAL_DB_ROW == _cal_db_util_stmt_step(stmt))
    {
        calendar_record_h record;
        // stmt -> record
        ret = calendar_record_create(_calendar_event._uri,&record);
        if( ret != CALENDAR_ERROR_NONE )
        {
            calendar_list_destroy(*out_list, true);
			*out_list = NULL;
            sqlite3_finalize(stmt);
            return ret;
        }
        __cal_db_event_get_stmt(stmt, true, record);

        // child
        int has_attendee = 0, has_alarm = 0;
        int record_id = 0;
        cal_event_s* pevent = (cal_event_s*) record;
        calendar_record_get_int(record, _calendar_event.id, &record_id);
        if(calendar_record_get_int(record, _calendar_event.has_attendee,&has_attendee) == CALENDAR_ERROR_NONE)
        {
            if( has_attendee == 1)
            {
                _cal_db_attendee_get_records(record_id, &pevent->attendee_list);
            }
        }
        if(calendar_record_get_int(record, _calendar_event.has_alarm,&has_alarm) == CALENDAR_ERROR_NONE)
        {
            if( has_alarm == 1)
            {
                _cal_db_alarm_get_records(record_id, &pevent->alarm_list);
            }
        }

        __cal_db_event_exception_get_records(record_id, &pevent->exception_list);
        _cal_db_extended_get_records(record_id, CALENDAR_RECORD_TYPE_EVENT, &pevent->extended_list);

        ret = calendar_list_add(*out_list,record);
        if( ret != CALENDAR_ERROR_NONE )
        {
            calendar_list_destroy(*out_list, true);
			*out_list = NULL;
            calendar_record_destroy(record, true);
            sqlite3_finalize(stmt);
            return ret;
        }
    }

    sqlite3_finalize(stmt);
    return CALENDAR_ERROR_NONE;
}

static int __cal_db_event_get_records_with_query(calendar_query_h query, int offset, int limit, calendar_list_h* out_list)
{
    cal_query_s *que = NULL;
	calendar_list_h list = NULL;
    int ret = CALENDAR_ERROR_NONE;
    char *condition = NULL;
    char *projection = NULL;
    char *order = NULL;
    GSList *bind_text = NULL, *cursor = NULL;
    char strquery[CAL_DB_SQL_MAX_LEN] = {0};
    int len;
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
        ret = _cal_db_query_create_condition(query,
                &condition, &bind_text);
        if (ret != CALENDAR_ERROR_NONE)
        {
            CAL_FREE(table_name);
            ERR("filter create fail");
            return ret;
        }
    }

    // make projection
    ret = _cal_db_query_create_projection(query, &projection);

    // query - projection
    if (projection)
    {
        len = snprintf(strquery, sizeof(strquery), "SELECT %s FROM %s", projection, table_name);
    }
    else
    {
        len = snprintf(strquery, sizeof(strquery), "SELECT * FROM %s", table_name);
    }
    CAL_FREE(table_name);

    // query - condition
    if (condition)
    {
        len += snprintf(strquery+len, sizeof(strquery)-len, " WHERE %s", condition);
    }

    // ORDER
    ret = _cal_db_query_create_order(query, &order);
    if (order)
    {
        len += snprintf(strquery+len, sizeof(strquery)-len, " %s", order);
    }

    if (0 < limit)
    {
        len += snprintf(strquery+len, sizeof(strquery)-len, " LIMIT %d", limit);
        if (0 < offset)
            len += snprintf(strquery+len, sizeof(strquery)-len, " OFFSET %d", offset);
    }

    // query
    stmt = _cal_db_util_query_prepare(strquery);
    if (NULL == stmt)
    {
        if (bind_text)
        {
            g_slist_free(bind_text);
        }
        CAL_FREE(condition);
        CAL_FREE(projection);
        ERR("_cal_db_util_query_prepare() Failed");
        return CALENDAR_ERROR_DB_FAILED;
    }
    CAL_DBG("%s",strquery);

    // bind text
    if (bind_text)
    {
        len = g_slist_length(bind_text);
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
            g_slist_free(bind_text);
        }
        CAL_FREE(condition);
        CAL_FREE(projection);
        ERR("calendar_list_create() Failed");
        sqlite3_finalize(stmt);
        return ret;
    }

    while(CAL_DB_ROW == _cal_db_util_stmt_step(stmt))
    {
        calendar_record_h record;
        // stmt -> record
        ret = calendar_record_create(_calendar_event._uri,&record);
        if( ret != CALENDAR_ERROR_NONE )
        {
            calendar_list_destroy(list, true);
            if (bind_text)
            {
                g_slist_free(bind_text);
            }
            CAL_FREE(condition);
            CAL_FREE(projection);
            sqlite3_finalize(stmt);
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
            __cal_db_event_get_stmt(stmt,true,record);
        }

        // child
        if (_cal_db_query_find_projection_property(query,CAL_PROPERTY_EVENT_CALENDAR_ALARM) == true)
        {
            cal_event_s* pevent = (cal_event_s*) record;
            _cal_db_alarm_get_records(pevent->index, &pevent->alarm_list);
        }
        if (_cal_db_query_find_projection_property(query,CAL_PROPERTY_EVENT_CALENDAR_ATTENDEE) == true)
        {
            cal_event_s* pevent = (cal_event_s*) record;
            _cal_db_attendee_get_records(pevent->index, &pevent->attendee_list);
        }
        if (_cal_db_query_find_projection_property(query,CAL_PROPERTY_EVENT_EXCEPTION) == true)
        {
            cal_event_s* pevent = (cal_event_s*) record;
            __cal_db_event_exception_get_records(pevent->index, &pevent->exception_list);
        }
        if (_cal_db_query_find_projection_property(query,CAL_PROPERTY_EVENT_EXTENDED) == true)
        {
            cal_event_s* pevent = (cal_event_s*) record;
            _cal_db_extended_get_records(pevent->index, CALENDAR_RECORD_TYPE_EVENT, &pevent->extended_list);
        }

        ret = calendar_list_add(list,record);
        if( ret != CALENDAR_ERROR_NONE )
        {
            calendar_list_destroy(list, true);
            calendar_record_destroy(record, true);

            if (bind_text)
            {
                g_slist_free(bind_text);
            }
            CAL_FREE(condition);
            CAL_FREE(projection);
            sqlite3_finalize(stmt);
            return ret;
        }
    }

    if (bind_text)
    {
        g_slist_free(bind_text);
    }
    CAL_FREE(condition);
    CAL_FREE(projection);

    sqlite3_finalize(stmt);

	*out_list = list;

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
    int i=0;
    for(i=0;i<count;i++)
    {
        if (__cal_db_event_delete_record(ids[i]) != CALENDAR_ERROR_NONE)
        {
            ERR("delete failed");
            return CALENDAR_ERROR_DB_FAILED;
        }
    }
    return CALENDAR_ERROR_NONE;
}

static int __cal_db_event_get_count(int *out_count)
{
    char query[CAL_DB_SQL_MAX_LEN] = {0};
    int count = 0;
	int ret;

    retvm_if(NULL == out_count, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");

    snprintf(query, sizeof(query), "SELECT count(*) FROM %s ", CAL_VIEW_TABLE_EVENT);

    ret = _cal_db_util_query_get_first_int_result(query, NULL, &count);
	if (CALENDAR_ERROR_NONE != ret)
	{
		ERR("_cal_db_util_query_get_first_int_result() failed");
		return ret;
	}
    CAL_DBG("%s=%d",query,count);

    *out_count = count;
    return CALENDAR_ERROR_NONE;
}

static int __cal_db_event_get_count_with_query(calendar_query_h query, int *out_count)
{
    cal_query_s *que = NULL;
    int ret = CALENDAR_ERROR_NONE;
    char *condition = NULL;
    char strquery[CAL_DB_SQL_MAX_LEN] = {0};
    int len;
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

    // query - select from

    len = snprintf(strquery, sizeof(strquery), "SELECT count(*) FROM %s", table_name);
    CAL_FREE(table_name);

    // query - condition
    if (condition)
    {
        len += snprintf(strquery+len, sizeof(strquery)-len, " WHERE %s", condition);
    }

    // query
    ret = _cal_db_util_query_get_first_int_result(strquery, bind_text, &count);
	if (CALENDAR_ERROR_NONE != ret)
	{
		ERR("_cal_db_util_query_get_first_int_result() failed");
		if (bind_text)
		{
			g_slist_free(bind_text);
		}
		CAL_FREE(condition);
		return ret;
	}
    CAL_DBG("%s=%d",strquery,count);

    *out_count = count;

    if (bind_text)
    {
        g_slist_free(bind_text);
    }
	CAL_FREE(condition);
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

	retv_if(NULL == event, CALENDAR_ERROR_INVALID_PARAMETER);

    if (event->common.properties_flags != NULL)
    {
        return __cal_db_event_update_dirty(record);
    }
    has_alarm = _cal_db_alarm_has_alarm(event->alarm_list);
	_cal_time_get_timezone_from_table(event->start_tzid, NULL, &timezone_id);
	input_ver = _cal_db_util_get_next_ver();
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
	        "availability = %d,"
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
	        "sync_data4 = ? "
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
		0,//event->availability,
		(long long int)0,//event->completed_time,
		0,//event->progress,
		event->start.type,
		event->start.type == CALENDAR_TIME_UTIME ? event->start.time.utime : 0,
		event->end.type,
		event->end.type == CALENDAR_TIME_UTIME ? event->end.time.utime : 0,
		event->freq > 0 ? 1 : 0,
		event->attendee_list ? 1 : 0,
		has_alarm,
		event->system_type,
		event->updated,
		id);

	stmt = _cal_db_util_query_prepare(query);
	retvm_if(NULL == stmt, CALENDAR_ERROR_DB_FAILED, "_cal_db_util_query_prepare() Failed");

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
		snprintf(dtstart_datetime, sizeof(dtstart_datetime), "%04d%02d%02d",
				event->start.time.date.year,
				event->start.time.date.month,
				event->start.time.date.mday);
		_cal_db_util_stmt_bind_text(stmt, index, dtstart_datetime);
	}
	index++;

	if (event->start_tzid)
		_cal_db_util_stmt_bind_text(stmt, index, event->start_tzid);
	index++;

	if (CALENDAR_TIME_LOCALTIME == event->end.type)
	{
		snprintf(dtend_datetime, sizeof(dtend_datetime), "%04d%02d%02d",
				event->end.time.date.year,
				event->end.time.date.month,
				event->end.time.date.mday);
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
	__cal_db_event_update_original_event_version(event->original_event_id, input_ver);

	_cal_db_rrule_get_rrule_from_event(record, &rrule);
	if (rrule)
	{
		_cal_db_rrule_update_record(id, rrule);
		CAL_FREE(rrule);
	}

	ret = _cal_db_instance_discard_record(record);
	retvm_if(ret != CALENDAR_ERROR_NONE, CALENDAR_ERROR_DB_FAILED,
			"_cal_db_instance_discard_record() Failed(%d)", ret);

	_cal_db_instance_publish_record(record);

	_cal_db_alarm_delete_with_id(id);
	_cal_db_attendee_delete_with_id(id);
    __cal_db_event_exception_delete_with_id(id);
    _cal_db_extended_delete_with_id(id, CALENDAR_RECORD_TYPE_EVENT);

	calendar_list_h list;

	list = NULL;
	ret = _cal_db_alarm_convert_gtoh(event->alarm_list, id, &list);
	if (ret == CALENDAR_ERROR_NONE)
	{
        calendar_db_insert_records(list, NULL, NULL);
        calendar_list_destroy(list, false);
	}

	list = NULL;
	ret = _cal_db_attendee_convert_gtoh(event->attendee_list, id, &list);
    if (ret == CALENDAR_ERROR_NONE)
    {
        calendar_db_insert_records(list, NULL, NULL);
        calendar_list_destroy(list, false);
    }

    if (event->exception_list)
    {
        DBG("insert exception");
        list = NULL;
        ret = __cal_db_event_exception_convert_gtoh(event->exception_list, id, event->calendar_id, &list);
        if (ret == CALENDAR_ERROR_NONE)
        {
            calendar_db_insert_records(list, NULL, NULL);
            calendar_list_destroy(list, false);
        }
    }

    if (event->extended_list)
    {
        DBG("insert extended");
        list = NULL;
        ret = _cal_db_extended_convert_gtoh(event->extended_list, id, CALENDAR_RECORD_TYPE_EVENT, &list);
        if (ret == CALENDAR_ERROR_NONE)
        {
            calendar_db_insert_records(list, NULL, NULL);
            calendar_list_destroy(list, false);
        }
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

static void __cal_db_event_get_stmt(sqlite3_stmt *stmt,bool is_view_table,calendar_record_h record)
{
    cal_event_s *event = NULL;
    const unsigned char *temp;
    int count = 0;
    char *dtstart_datetime;
    char *dtend_datetime;
    char buf[8] = {0};

    event = (cal_event_s*)(record);

    event->index = sqlite3_column_int(stmt, count++);
    sqlite3_column_int(stmt, count++);//event->account_id = sqlite3_column_int(stmt, count++);
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
    sqlite3_column_int(stmt, count++);//event->availability = sqlite3_column_int(stmt, count++);

    event->created_time = sqlite3_column_int64(stmt, count++);

    sqlite3_column_int64(stmt, count++);//event->completed_time = sqlite3_column_int64(stmt, count++);

    sqlite3_column_int(stmt, count++);//event->progress = sqlite3_column_int(stmt,count++);

    sqlite3_column_int(stmt,count++);
    sqlite3_column_int(stmt,count++);
    event->is_deleted = sqlite3_column_int(stmt,count++);

    event->start.type = sqlite3_column_int(stmt,count++);

    if (event->start.type == CALENDAR_TIME_UTIME)
    {
        event->start.time.utime = sqlite3_column_int64(stmt,count++);
        sqlite3_column_text(stmt, count++);  //dtstart_datetime
    }
    else
    {
        sqlite3_column_int64(stmt,count++); //event->start.time.utime = sqlite3_column_int64(stmt,count++);
        temp = sqlite3_column_text(stmt, count++);
        if (temp) {
            dtstart_datetime = SAFE_STRDUP(temp);
            snprintf(buf, strlen("YYYY") + 1, "%s", &dtstart_datetime[0]);
            event->start.time.date.year =  atoi(buf);
            snprintf(buf, strlen("MM") + 1, "%s", &dtstart_datetime[4]);
            event->start.time.date.month = atoi(buf);
            snprintf(buf, strlen("DD") + 1, "%s", &dtstart_datetime[6]);
            event->start.time.date.mday = atoi(buf);
            if (dtstart_datetime) free(dtstart_datetime);
        }
    }

    temp = sqlite3_column_text(stmt, count++);
    event->start_tzid = SAFE_STRDUP(temp);
    event->end.type = sqlite3_column_int(stmt, count++);
    if (event->end.type == CALENDAR_TIME_UTIME)
    {
        event->end.time.utime = sqlite3_column_int64(stmt,count++);
        sqlite3_column_text(stmt, count++);
    }
    else
    {
        sqlite3_column_int64(stmt, count++);//event->end.time.utime = sqlite3_column_int64(stmt, count++);
        temp = sqlite3_column_text(stmt, count++);
        if (temp) {
            dtend_datetime = SAFE_STRDUP(temp);
            snprintf(buf, strlen("YYYY") + 1, "%s", &dtend_datetime[0]);
            event->end.time.date.year =  atoi(buf);
            snprintf(buf, strlen("MM") + 1, "%s", &dtend_datetime[4]);
            event->end.time.date.month = atoi(buf);
            snprintf(buf, strlen("DD") + 1, "%s", &dtend_datetime[6]);
            event->end.time.date.mday = atoi(buf);
            if (dtend_datetime) free(dtend_datetime);
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

    if (is_view_table == true)
    {
        event->freq = sqlite3_column_int(stmt, count++);

        if (event->freq <= 0) {
            //event->rrule_id = 0;
            //sqlite3_finalize(stmt);
            //return CALENDAR_ERROR_NONE;
            return ;
        }

        //event->rrule_id = 1;
        event->range_type = sqlite3_column_int(stmt, count++);
        event->until_type = sqlite3_column_int(stmt, count++);
        event->until_utime = sqlite3_column_int64(stmt, count++);

        temp = sqlite3_column_text(stmt, count++);
		if (CALENDAR_TIME_LOCALTIME == event->until_type)
		{
			sscanf((const char *)temp, "%04d%02d%02d",
					&event->until_year, &event->until_month, &event->until_mday);
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
    char *dtstart_datetime;
    char *dtend_datetime;
    char buf[8] = {0};

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
        //!!
        event->start.type = sqlite3_column_int(stmt,*stmt_count);
        if (event->start.type == CALENDAR_TIME_UTIME)
        {
            *stmt_count = *stmt_count+1;
            event->start.time.utime = sqlite3_column_int64(stmt,*stmt_count);
            *stmt_count = *stmt_count+1;
            sqlite3_column_text(stmt, *stmt_count);  //dtstart_datetime
        }
        else
        {
            *stmt_count = *stmt_count+1;
            sqlite3_column_int64(stmt,*stmt_count); //event->start.time.utime = sqlite3_column_int64(stmt,count++);
            *stmt_count = *stmt_count+1;
            temp = sqlite3_column_text(stmt, *stmt_count);
            if (temp) {
                dtstart_datetime = SAFE_STRDUP(temp);
                snprintf(buf, strlen("YYYY") + 1, "%s", &dtstart_datetime[0]);
                event->start.time.date.year =  atoi(buf);
                snprintf(buf, strlen("MM") + 1, "%s", &dtstart_datetime[4]);
                event->start.time.date.month = atoi(buf);
                snprintf(buf, strlen("DD") + 1, "%s", &dtstart_datetime[6]);
                event->start.time.date.mday = atoi(buf);
                if (dtstart_datetime) free(dtstart_datetime);
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
            *stmt_count = *stmt_count+1;
            sqlite3_column_text(stmt, *stmt_count);
        }
        else
        {
            *stmt_count = *stmt_count+1;
            sqlite3_column_int64(stmt, *stmt_count);//event->end.time.utime = sqlite3_column_int64(stmt, count++);
            *stmt_count = *stmt_count+1;
            temp = sqlite3_column_text(stmt, *stmt_count);
            if (temp) {
                dtend_datetime = SAFE_STRDUP(temp);
                snprintf(buf, strlen("YYYY") + 1, "%s", &dtend_datetime[0]);
                event->end.time.date.year =  atoi(buf);
                snprintf(buf, strlen("MM") + 1, "%s", &dtend_datetime[4]);
                event->end.time.date.month = atoi(buf);
                snprintf(buf, strlen("DD") + 1, "%s", &dtend_datetime[6]);
                event->end.time.date.mday = atoi(buf);
                if (dtend_datetime) free(dtend_datetime);
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

static int __cal_db_event_update_dirty(calendar_record_h record)
{
    int event_id = 0;
    int ret = CALENDAR_ERROR_NONE;
    calendar_record_h original_record = NULL;

    ret = calendar_record_get_int(record,_calendar_event.id, &event_id);
    retv_if(CALENDAR_ERROR_NONE != ret, ret);

    CAL_DBG("id=%d",event_id);

    ret = __cal_db_event_get_record(event_id, &original_record);

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
            if ( true == _cal_record_check_property_flag(record, property_info[i].property_id , CAL_PROPERTY_FLAG_DIRTY))
            {
                //CAL_DBG("%d",property_info[i].property_id);
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
        cal_event_s *tmp = (cal_event_s *)original_record;
        cal_event_s *tmp_src = (cal_event_s *)record;
        if ( tmp->alarm_list )
        {
            GList *alarm_list = g_list_first(tmp->alarm_list);
            calendar_record_h alarm_child_record = NULL;
            while (alarm_list)
            {
                alarm_child_record = (calendar_record_h)alarm_list->data;
                if (alarm_child_record == NULL)
                {
                    alarm_list = g_list_next(alarm_list);
                    continue;
                }

                calendar_record_destroy(alarm_child_record, true);

                alarm_list = g_list_next(alarm_list);
            }
            g_list_free(tmp->alarm_list);
        }
        tmp->alarm_list = tmp_src->alarm_list;

        if( tmp->attendee_list )
        {
            GList * attendee_list = g_list_first(tmp->attendee_list);
            calendar_record_h attendee = NULL;

            while (attendee_list)
            {
                attendee = (calendar_record_h)attendee_list->data;
                if (attendee == NULL)
                {
                    attendee_list = g_list_next(attendee_list);
                    continue;
                }

                calendar_record_destroy(attendee, true);

                attendee_list = g_list_next(attendee_list);
            }
            g_list_free(tmp->attendee_list);
        }
        tmp->attendee_list = tmp_src->attendee_list;
        if( tmp->exception_list )
        {
            GList * exception_list = g_list_first(tmp->exception_list);
            calendar_record_h exception = NULL;

            while (exception_list)
            {
                exception = (calendar_record_h)exception_list->data;
                if (exception == NULL)
                {
                    exception_list = g_list_next(exception_list);
                    continue;
                }

                calendar_record_destroy(exception, true);

                exception_list = g_list_next(exception_list);
            }
            g_list_free(tmp->exception_list);
        }
        tmp->exception_list = tmp_src->exception_list;
        if( tmp->extended_list )
        {
            GList * extended_list = g_list_first(tmp->extended_list);
            calendar_record_h extended = NULL;

            while (extended_list)
            {
                extended = (calendar_record_h)extended_list->data;
                if (extended == NULL)
                {
                    extended_list = g_list_next(extended_list);
                    continue;
                }

                calendar_record_destroy(extended, true);

                extended_list = g_list_next(extended_list);
            }
            g_list_free(tmp->extended_list);
        }
        tmp->extended_list = tmp_src->extended_list;
    }
    else
    {
        CAL_DBG("get_record fail");
        return ret;
    }

    CAL_RECORD_RESET_COMMON((cal_record_s*)original_record);
    ret = __cal_db_event_update_record(original_record);

    calendar_record_destroy(original_record,false);

    return ret;
}

#if 0 // // under construction
static int __cal_db_event_update_dirty(calendar_record_h record)
{
    char query[CAL_DB_SQL_MAX_LEN] = {0};
    sqlite3_stmt *stmt = NULL;
    cal_event_s* event =  (cal_event_s*)(record);
    cal_db_util_error_e dbret = CAL_DB_OK;
    int ret = CALENDAR_ERROR_NONE;
    char* set = NULL;
    GSList *bind_text = NULL;
    unsigned int properties[36];
    int properties_count = 36;
    bool bchangerrule = __cal_db_event_check_changed_rrule(record);
    bool bchangetime = __cal_db_event_check_changed_time(record);

    if (bchangerrule == true && bchangetime == false)
    {
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }
    /*
    else if (bchangetime == true)
    {
        // rrule table check !
    }
    */

    properties[0] = CAL_PROPERTY_EVENT_CALENDAR_ID;
    properties[1] = CAL_PROPERTY_EVENT_SUMMARY;
    properties[2] = CAL_PROPERTY_EVENT_DESCRIPTION;
    properties[3] = CAL_PROPERTY_EVENT_LOCATION;
    properties[4] = CAL_PROPERTY_EVENT_CATEGORIES;
    properties[5] = CAL_PROPERTY_EVENT_EXDATE;
    properties[6] = CAL_PROPERTY_EVENT_EVENT_STATUS;
    properties[7] = CAL_PROPERTY_EVENT_PRIORITY;
    properties[8] = CAL_PROPERTY_EVENT_TIMEZONE;
    properties[9] = CAL_PROPERTY_EVENT_CONTACT_ID;
    properties[10] = CAL_PROPERTY_EVENT_BUSY_STATUS;
    properties[11] = CAL_PROPERTY_EVENT_SENSITIVITY;
    properties[12] = CAL_PROPERTY_EVENT_UID;
    properties[13] = CAL_PROPERTY_EVENT_ORGANIZER_NAME;
    properties[14] = CAL_PROPERTY_EVENT_ORGANIZER_EMAIL;
    properties[15] = CAL_PROPERTY_EVENT_MEETING_STATUS;
    properties[16] = CAL_PROPERTY_EVENT_ORIGINAL_EVENT_ID;
    properties[17] = CAL_PROPERTY_EVENT_LATITUDE;
    properties[18] = CAL_PROPERTY_EVENT_LONGITUDE;
    properties[19] = CAL_PROPERTY_EVENT_EMAIL_ID;
    properties[20] = CAL_PROPERTY_EVENT_CREATED_TIME;
    properties[21] = CAL_PROPERTY_EVENT_LAST_MODIFIED_TIME;
    properties[22] = CAL_PROPERTY_EVENT_IS_DELETED;
    properties[23] = CAL_PROPERTY_EVENT_RECURRENCE_ID;
    properties[24] = CAL_PROPERTY_EVENT_RDATE;
    properties[25] = CAL_PROPERTY_EVENT_HAS_ATTENDEE;
    properties[26] = CAL_PROPERTY_EVENT_HAS_ALARM;
    properties[27] = CAL_PROPERTY_EVENT_SYNC_DATA1;
    properties[28] = CAL_PROPERTY_EVENT_SYNC_DATA2;
    properties[29] = CAL_PROPERTY_EVENT_SYNC_DATA3;
    properties[30] = CAL_PROPERTY_EVENT_SYNC_DATA4;
    properties[31] = CAL_PROPERTY_EVENT_START;
    properties[32] = CAL_PROPERTY_EVENT_END;
    properties[33] = CAL_PROPERTY_EVENT_CALENDAR_SYSTEM_TYPE;
    properties[34] = CAL_PROPERTY_EVENT_START_TZID;
    properties[35] = CAL_PROPERTY_EVENT_END_TZID;

    ret = _cal_db_query_create_projection_update_set_with_property(record,properties,properties_count,
            &set,&bind_text);
    retv_if(CALENDAR_ERROR_NONE != ret, ret);

    if (set && strlen(str) > 0)
    {
        snprintf(query, sizeof(query), "UPDATE %s SET "
                "changed_ver = %d, "
                "%s"
                "WHERE id = %d",
                CAL_TABLE_SCHEDULE,
                _cal_db_util_get_next_ver(),
                set,
                event->index);
    }
    else
    {
        snprintf(query, sizeof(query), "UPDATE %s SET "
                "changed_ver = %d"
                "WHERE id = %d",
                CAL_TABLE_SCHEDULE,
                _cal_db_util_get_next_ver(),
                event->index);
    }

    stmt = _cal_db_util_query_prepare(query);
    retvm_if(NULL == stmt, CALENDAR_ERROR_DB_FAILED, "_cal_db_util_query_prepare() Failed");

    // bind
    if (bind_text)
    {
        int len;
        GSList *cursor = NULL;
        int i = 0;
        len = g_slist_length(bind_text);
        for (cursor=bind_text, i=1; cursor;cursor=cursor->next, i++)
        {
            _cal_db_util_stmt_bind_text(stmt, i, cursor->data);
        }
    }

    dbret = _cal_db_util_stmt_step(stmt);
    if (CAL_DB_DONE != dbret) {
        sqlite3_finalize(stmt);
        ERR("_cal_db_util_stmt_step() Failed(%d)", dbret);

        CAL_FREE(set);
        if(bind_text)
        {
            for (cursor=bind_text; cursor;cursor=cursor->next)
            {
                CAL_FREE(cursor->data);
            }
            g_slist_free(bind_text);
        }
		switch (dbret)
		{
		case CAL_DB_ERROR_NO_SPACE:
			return CALENDAR_ERROR_FILE_NO_SPACE;
		default:
			return CALENDAR_ERROR_DB_FAILED;
		}
    }

    sqlite3_finalize(stmt);

    // rrule update
    if (bchangerrule == true)
    {
        cal_rrule_s *rrule = NULL;

        _cal_db_rrule_get_rrule_from_event(record, &rrule);
        if (rrule)
		{
			_cal_db_rrule_update_record(event->index, rrule);
	        CAL_FREE(rrule);
		}

        ret = _cal_db_instance_discard_record(record);
        retvm_if(ret != CALENDAR_ERROR_NONE, CALENDAR_ERROR_DB_FAILED,
                "_cal_db_instance_discard_record() Failed(%d)", ret);

        _cal_db_instance_publish_record(record);
    }

    // child update 의 경우는 get_with_query 구현 이후 고려

    _cal_db_util_notify(CAL_NOTI_TYPE_EVENT);

    CAL_FREE(set);
    if(bind_text)
    {
        for (cursor=bind_text; cursor;cursor=cursor->next)
        {
            CAL_FREE(cursor->data);
        }
        g_slist_free(bind_text);
    }
    return CALENDAR_ERROR_NONE;
}

bool __cal_db_event_check_changed_rrule(calendar_record_h record)
{
    unsigned int properties[15];
    int properties_count = 15;
    int i=0;

    properties[0] = CAL_PROPERTY_EVENT_FREQ;
    properties[1] = CAL_PROPERTY_EVENT_RANGE_TYPE;
    properties[2] = CAL_PROPERTY_EVENT_UNTIL;
    properties[3] = CAL_PROPERTY_EVENT_COUNT;
    properties[4] = CAL_PROPERTY_EVENT_INTERVAL;
    properties[5] = CAL_PROPERTY_EVENT_BYSECOND;
    properties[6] = CAL_PROPERTY_EVENT_BYMINUTE;
    properties[7] = CAL_PROPERTY_EVENT_BYHOUR;
    properties[8] = CAL_PROPERTY_EVENT_BYDAY;
    properties[9] = CAL_PROPERTY_EVENT_BYMONTHDAY;
    properties[10] = CAL_PROPERTY_EVENT_BYYEARDAY;
    properties[11] = CAL_PROPERTY_EVENT_BYWEEKNO;
    properties[12] = CAL_PROPERTY_EVENT_BYMONTH;
    properties[13] = CAL_PROPERTY_EVENT_BYSETPOS;
    properties[14] = CAL_PROPERTY_EVENT_WKST;

    for(i=0;i<properties_count;i++)
    {
        if (false == _cal_record_check_property_flag(record, properties[i], CAL_PROPERTY_FLAG_PROJECTION))
            return false;
    }
    return true;
}

bool __cal_db_event_check_changed_time(calendar_record_h record)
{
    unsigned int properties[2];
    int properties_count = 2;
    int i=0;

    properties[0] = CAL_PROPERTY_EVENT_START;
    properties[1] = CAL_PROPERTY_EVENT_END;

    for(i=0;i<properties_count;i++)
    {
        if (false == _cal_record_check_property_flag(record, properties[i], CAL_PROPERTY_FLAG_PROJECTION))
            return false;
    }
    return true;
}
#endif

static int __cal_db_event_exception_get_records(int original_id, GList **out_list)
{
    int ret;
    char query[CAL_DB_SQL_MAX_LEN] = {0};
    sqlite3_stmt *stmt = NULL;
    GList *list = NULL;

    retvm_if(NULL == out_list, CALENDAR_ERROR_INVALID_PARAMETER,
            "Invalid parameter: GList is NULL");

    snprintf(query, sizeof(query),
            "SELECT * FROM %s "
            "WHERE original_event_id = %d ",
            CAL_TABLE_SCHEDULE,
            original_id);

    stmt = _cal_db_util_query_prepare(query);
    if (NULL == stmt)
    {
        ERR("_cal_db_util_query_prepare() failed");
        return CALENDAR_ERROR_DB_FAILED;
    }

    calendar_record_h record = NULL;

    while (_cal_db_util_stmt_step(stmt) == CAL_DB_ROW)
    {
        ret = calendar_record_create(_calendar_event._uri, &record);
        if (CALENDAR_ERROR_NONE != ret)
        {
            sqlite3_finalize(stmt);
            return ret;
        }

        __cal_db_event_get_stmt(stmt, false, record);

        list = g_list_append(list, record);
    }
    sqlite3_finalize(stmt);

    *out_list = list;
    return CALENDAR_ERROR_NONE;
}

static int __cal_db_event_exception_convert_gtoh(GList *glist, int original_id, int calendar_id, calendar_list_h *hlist)
{
    int ret;
    GList *g = NULL;
    calendar_list_h h = NULL;
    calendar_record_h exception = NULL;

    if (glist == NULL)
    {
        DBG("No attendee");
        return CALENDAR_ERROR_NO_DATA;
    }
    ret = calendar_list_create(&h);

    g = g_list_first(glist);
    while (g)
    {
        exception = (calendar_record_h)g->data;
        if (exception)
        {
            ret = _cal_record_set_int(exception,_calendar_event.original_event_id, original_id);
            ret = _cal_record_set_int(exception,_calendar_event.calendar_book_id, calendar_id);
            ret = calendar_list_add(h, exception);

        }
        g = g_list_next(g);
    }

    *hlist = h;
    return CALENDAR_ERROR_NONE;
}

static int __cal_db_event_exception_delete_with_id(int original_id)
{
    char query[CAL_DB_SQL_MAX_LEN] = {0};
    cal_db_util_error_e dbret = CAL_DB_OK;

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
