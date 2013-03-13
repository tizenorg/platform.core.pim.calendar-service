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

#include <glib.h>
#include <db-util.h>

#include "calendar_db.h"
#include "calendar_vcalendar.h"

#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_view.h"
#include "cal_record.h"
#include "cal_db_util.h"
#include "cal_list.h"

#include "cal_db.h"

/*
 * Declear DB plugin
 */
extern cal_db_plugin_cb_s _cal_db_calendar_plugin_cb;
extern cal_db_plugin_cb_s _cal_db_event_plugin_cb;
extern cal_db_plugin_cb_s _cal_db_instance_normal_plugin_cb;
extern cal_db_plugin_cb_s _cal_db_instance_allday_plugin_cb;
extern cal_db_plugin_cb_s _cal_db_todo_plugin_cb;
extern cal_db_plugin_cb_s _cal_db_alarm_plugin_cb;
extern cal_db_plugin_cb_s _cal_db_attendee_plugin_cb;
extern cal_db_plugin_cb_s _cal_db_search_plugin_cb;
extern cal_db_plugin_cb_s _cal_db_timezone_plugin_cb;
extern cal_db_plugin_cb_s _cal_db_extended_plugin_cb;

static cal_db_plugin_cb_s* __cal_db_get_plugin(cal_record_type_e type);

static void __cal_db_initialize_view_table(void);

#ifdef CAL_NATIVE
typedef struct {
    calendar_list_h list;
    calendar_db_insert_result_cb callback;
    void *user_data;
} __insert_records_data_s;

typedef struct {
    calendar_list_h list;
    calendar_db_result_cb callback;
    void *user_data;
} __update_records_data_s;

typedef struct {
    const char* view_uri;
    int *ids;
    int count;
    calendar_db_result_cb callback;
    void *user_data;
} __delete_records_data_s;

typedef struct {
    char* vcalendar_stream;
    calendar_db_insert_result_cb callback;
    void *user_data;
} __insert_vcalendars_data_s;

typedef struct {
    char* vcalendar_stream;
    int *ids;
    int count;
    calendar_db_result_cb callback;
    void *user_data;
} __replace_vcalendars_data_s;

typedef struct {
    calendar_list_h list;
    int *ids;
    int count;
    calendar_db_result_cb callback;
    void *user_data;
} __replace_records_data_s;
static gboolean  __cal_db_insert_records_idle(void* user_data);
static gboolean  __cal_db_update_records_idle(void* user_data);
static gboolean  __cal_db_delete_records_idle(void* user_data);
static gboolean  __cal_db_insert_vcalendars_idle(void* user_data);
static gboolean  __cal_db_replace_vcalendars_idle(void* user_data);
static gboolean  __cal_db_replace_records_idle(void* user_data);

static gboolean  __cal_db_insert_records_idle(void* user_data)
{
    __insert_records_data_s* callback_data = user_data;
    int ret = CALENDAR_ERROR_NONE;
    int *ids = NULL;
    int count = 0;

    if (callback_data == NULL)
    {
        ERR("data is NULL");
        return false;
    }

    ret = calendar_db_insert_records(callback_data->list, &ids, &count);

    if (ret == CALENDAR_ERROR_NONE)
    {
        callback_data->callback(ret, ids, count, callback_data->user_data);
    }
    else
    {
        callback_data->callback(ret, NULL, 0, callback_data->user_data);
    }

    calendar_list_destroy(callback_data->list, true);
    CAL_FREE(callback_data);
    CAL_FREE(ids);
    return false;
}

static gboolean  __cal_db_update_records_idle(void* user_data)
{
    __update_records_data_s* callback_data = user_data;
    int ret = CALENDAR_ERROR_NONE;

    if (callback_data == NULL)
    {
        ERR("data is NULL");
        return false;
    }
    ret = calendar_db_update_records(callback_data->list);

    callback_data->callback(ret, callback_data->user_data);

    calendar_list_destroy(callback_data->list, true);
    CAL_FREE(callback_data);
    return false;
}

static gboolean  __cal_db_delete_records_idle(void* user_data)
{
    __delete_records_data_s* callback_data = user_data;
    int ret = CALENDAR_ERROR_NONE;

    if (callback_data == NULL)
    {
        ERR("data is NULL");
        return false;
    }
    ret = calendar_db_delete_records(callback_data->view_uri,callback_data->ids,callback_data->count);

    callback_data->callback(ret, callback_data->user_data);

    CAL_FREE(callback_data->ids);
    CAL_FREE(callback_data);
    return false;
}
static gboolean  __cal_db_insert_vcalendars_idle(void* user_data)
{
    __insert_vcalendars_data_s* callback_data = user_data;
    int ret = CALENDAR_ERROR_NONE;
    int *ids = NULL;
    int count = 0;

    if (callback_data == NULL)
    {
        ERR("data is NULL");
        return false;
    }

    ret = calendar_db_insert_vcalendars(callback_data->vcalendar_stream, &ids, &count);

    if (ret == CALENDAR_ERROR_NONE)
    {
        callback_data->callback(ret, ids, count, callback_data->user_data);
    }
    else
    {
        callback_data->callback(ret, NULL, 0, callback_data->user_data);
    }

    CAL_FREE(callback_data->vcalendar_stream);
    CAL_FREE(callback_data);
    CAL_FREE(ids);
    return false;
}

static gboolean  __cal_db_replace_vcalendars_idle(void* user_data)
{
    __replace_vcalendars_data_s* callback_data = user_data;
    int ret = CALENDAR_ERROR_NONE;

    if (callback_data == NULL)
    {
        ERR("data is NULL");
        return false;
    }
    ret = calendar_db_replace_vcalendars(callback_data->vcalendar_stream,
            callback_data->ids,callback_data->count);

    callback_data->callback(ret, callback_data->user_data);

    CAL_FREE(callback_data->vcalendar_stream);
    CAL_FREE(callback_data->ids);
    CAL_FREE(callback_data);
    return false;
}

static gboolean  __cal_db_replace_records_idle(void* user_data)
{
    __replace_records_data_s* callback_data = user_data;
    int ret = CALENDAR_ERROR_NONE;

    if (callback_data == NULL)
    {
        ERR("data is NULL");
        return false;
    }
    ret = calendar_db_replace_records(callback_data->list,callback_data->ids,callback_data->count);

    callback_data->callback(ret, callback_data->user_data);

    calendar_list_destroy(callback_data->list, true);
    CAL_FREE(callback_data->ids);
    CAL_FREE(callback_data);
    return false;
}

#endif

static cal_db_plugin_cb_s* __cal_db_get_plugin(cal_record_type_e type)
{
    switch (type)
    {
    case CAL_RECORD_TYPE_CALENDAR:
        return (&_cal_db_calendar_plugin_cb);
    case CAL_RECORD_TYPE_EVENT:
        return (&_cal_db_event_plugin_cb);
    case CAL_RECORD_TYPE_TODO:
        return (&_cal_db_todo_plugin_cb);
    case CAL_RECORD_TYPE_ALARM:
        return (&_cal_db_alarm_plugin_cb);
    case CAL_RECORD_TYPE_ATTENDEE:
        return (&_cal_db_attendee_plugin_cb);
    case CAL_RECORD_TYPE_TIMEZONE:
        return (&_cal_db_timezone_plugin_cb);
    case CAL_RECORD_TYPE_INSTANCE_NORMAL:
        return (&_cal_db_instance_normal_plugin_cb);
    case CAL_RECORD_TYPE_INSTANCE_ALLDAY:
        return (&_cal_db_instance_allday_plugin_cb);
    case CAL_RECORD_TYPE_SEARCH:
        return (&_cal_db_search_plugin_cb);
    case CAL_RECORD_TYPE_EXTENDED:
        return (&_cal_db_extended_plugin_cb);
    default:
        return NULL;
    }
    return NULL;
}

static void __cal_db_initialize_view_table(void)
{
    cal_db_util_error_e dbret;
    char query[CAL_DB_SQL_MAX_LEN] = {0};

    /*
     * CAL_VIEW_TABLE_EVENT
     * schedule_table(type=1) + rrule_table
     */
    snprintf(query, sizeof(query),
                "CREATE TEMP VIEW %s AS "
                "SELECT A.*, "
                "B.freq, B.range_type, B.until_type, B.until_utime, B.until_datetime, B.count, "
                "B.interval, B.bysecond, B.byminute, B.byhour, B.byday, B.bymonthday, "
                "B.byyearday, B.byweekno, B.bymonth, B.bysetpos, B.wkst, C.deleted "
                "FROM %s as A "
                "LEFT OUTER JOIN %s B ON A.id = B.event_id "
                "JOIN %s C ON A.calendar_id = C.id "
                "WHERE A.type = %d "
                "AND C.deleted = 0 ",
                CAL_VIEW_TABLE_EVENT,
                CAL_TABLE_SCHEDULE,
                CAL_TABLE_RRULE,
                CAL_TABLE_CALENDAR,
                CAL_SCH_TYPE_EVENT);
    dbret = _cal_db_util_query_exec(query);
    CAL_DBG("%s",query);
    if (dbret != CAL_DB_OK)
    {
        ERR("create view fail");
    }

    /*
     * CAL_VIEW_TABLE_TODO
     * schedule_table(type=2) + rrule_table
     */
    snprintf(query, sizeof(query),
                "CREATE TEMP VIEW %s AS "
                "SELECT A.*, "
                "B.freq, B.range_type, B.until_type, B.until_utime, B.until_datetime, B.count, "
                "B.interval, B.bysecond, B.byminute, B.byhour, B.byday, B.bymonthday, "
                "B.byyearday, B.byweekno, B.bymonth, B.bysetpos, B.wkst, C.deleted "
                "FROM %s as A "
                "LEFT OUTER JOIN %s B ON A.id = B.event_id "
                "JOIN %s C ON A.calendar_id = C.id "
                "WHERE A.type = %d "
                "AND C.deleted = 0 ",
                CAL_VIEW_TABLE_TODO,
                CAL_TABLE_SCHEDULE,
                CAL_TABLE_RRULE,
                CAL_TABLE_CALENDAR,
                CAL_SCH_TYPE_TODO);
    dbret = _cal_db_util_query_exec(query);
    CAL_DBG("%s",query);
    if (dbret != CAL_DB_OK)
    {
        ERR("create view fail");
    }

    /*
     * CAL_VIEW_TABLE_NORMAL_INSTANCE  : CALENDAR_VIEW_INSTANCE_NORMAL_CALENDAR
     * normal_instance_table + schedule_table(type=1) + calendar_table
     * A = normal_instace_table
     * B = schedule_table
     * C = calendar_table
     */
    snprintf(query, sizeof(query),
            "CREATE TEMP VIEW %s AS SELECT A.event_id, "
            "B.dtstart_type, A.dtstart_utime, B.dtstart_datetime, "
            "B.dtend_type, A.dtend_utime, B.dtend_datetime, "
            "B.summary, B.description, "
            "B.location, B.busy_status, B.task_status, B.priority, B.sensitivity, "
            "B.rrule_id, B.latitude, B.longitude, B.has_alarm, B.original_event_id, "
            "B.calendar_id, B.last_mod, "
			"C.visibility, C.account_id "
            "FROM %s as A, %s as B, %s as C "
            "ON A.event_id = B.id AND B.calendar_id = C.id "
            "where C.deleted = 0",
            CAL_VIEW_TABLE_NORMAL_INSTANCE,
            CAL_TABLE_NORMAL_INSTANCE,
            CAL_TABLE_SCHEDULE,
            CAL_TABLE_CALENDAR);
    dbret = _cal_db_util_query_exec(query);
    CAL_DBG("%s",query);
    if (dbret != CAL_DB_OK)
    {
        ERR("create view fail");
    }

    // CAL_VIEW_TABLE_ALLDAY_INSTANCE  : CALENDAR_VIEW_INSTANCE_ALLDAY_CALENDAR
    snprintf(query, sizeof(query),
            "CREATE TEMP VIEW %s AS SELECT A.event_id, "
            "B.dtstart_type, B.dtstart_utime, A.dtstart_datetime, "
            "B.dtend_type, B.dtend_utime, A.dtend_datetime, "
            "B.summary, B.description, "
            "B.location, B.busy_status, B.task_status, B.priority, B.sensitivity, "
            "B.rrule_id, B.latitude, B.longitude, B.has_alarm, B.original_event_id, "
            "B.calendar_id, B.last_mod, "
            "C.visibility, C.account_id "
            "FROM %s as A, %s as B, %s as C "
            "ON A.event_id = B.id AND B.calendar_id = C.id "
            "where C.deleted = 0",
            CAL_VIEW_TABLE_ALLDAY_INSTANCE,
            CAL_TABLE_ALLDAY_INSTANCE,
            CAL_TABLE_SCHEDULE,
            CAL_TABLE_CALENDAR);
    dbret = _cal_db_util_query_exec(query);
    CAL_DBG("%s",query);
    if (dbret != CAL_DB_OK)
    {
        ERR("create view fail");
    }

    // event_calendar view  :  CALENDAR_VIEW_EVENT_CALENDAR
    snprintf(query, sizeof(query),
                "CREATE TEMP VIEW %s AS "
                "SELECT A.* "
                ", B.freq, B.range_type, B.until_type, B.until_utime, B.until_datetime, B.count, "
                "B.interval, B.bysecond, B.byminute, B.byhour, B.byday, B.bymonthday, "
                "B.byyearday, B.byweekno, B.bymonth, B.bysetpos, B.wkst "
                ", C.*"
                "FROM %s A "
                "JOIN %s C ON A.calendar_id = C.id "
                "LEFT OUTER JOIN %s B ON A.id = B.event_id "
                "WHERE A.type = 1 AND c.deleted = 0",
                CAL_VIEW_TABLE_EVENT_CALENDAR,
                CAL_TABLE_SCHEDULE, CAL_TABLE_CALENDAR,
                CAL_TABLE_RRULE );
    dbret = _cal_db_util_query_exec(query);
    CAL_DBG("%s",query);
    if (dbret != CAL_DB_OK)
    {
        ERR("create view fail");
    }

    // todo_calendar view  : CALENDAR_VIEW_TODO_CALENDAR
    snprintf(query, sizeof(query),
                "CREATE TEMP VIEW %s AS "
                "SELECT A.* "
                ", B.freq, B.range_type, B.until_type, B.until_utime, B.until_datetime, B.count, "
                "B.interval, B.bysecond, B.byminute, B.byhour, B.byday, B.bymonthday, "
                "B.byyearday, B.byweekno, B.bymonth, B.bysetpos, B.wkst "
                ", C.*"
                "FROM %s A "
                "JOIN %s C ON A.calendar_id = C.id "
                "LEFT OUTER JOIN %s B ON A.id = B.event_id "
                "WHERE A.type = 2 AND c.deleted = 0",
                CAL_VIEW_TABLE_TODO_CALENDAR,
                CAL_TABLE_SCHEDULE, CAL_TABLE_CALENDAR,
                CAL_TABLE_RRULE);
    dbret = _cal_db_util_query_exec(query);
    CAL_DBG("%s",query);
    if (dbret != CAL_DB_OK)
    {
        ERR("create view fail");
    }

    // event_calendar_attendee view  :  CALENDAR_VIEW_EVENT_CALENDAR_ATTENDEE
    snprintf(query, sizeof(query),
                "CREATE TEMP VIEW %s AS "
                "SELECT A.* "
                ", D.freq, D.range_type, D.until_type, D.until_utime, D.until_datetime, D.count, "
                "D.interval, D.bysecond, D.byminute, D.byhour, D.byday, D.bymonthday, "
                "D.byyearday, D.byweekno, D.bymonth, D.bysetpos, D.wkst "
                ", B.*, C.*"
                "FROM %s A "
                "LEFT OUTER  JOIN %s B ON A.id = B.event_id "
                "JOIN %s C ON A.calendar_id = C.id "
                "LEFT OUTER JOIN %s D ON A.id = D.event_id "
                "WHERE A.type = 1 AND c.deleted = 0 ",
                CAL_VIEW_TABLE_EVENT_CALENDAR_ATTENDEE,
                CAL_TABLE_SCHEDULE, CAL_TABLE_ATTENDEE, CAL_TABLE_CALENDAR,
                CAL_TABLE_RRULE );
    dbret = _cal_db_util_query_exec(query);
    CAL_DBG("%s",query);
    if (dbret != CAL_DB_OK)
    {
        ERR("create view fail");
    }

    return ;
}

int _cal_db_open(void)
{
    int ret = CALENDAR_ERROR_NONE;
    ret = _cal_db_util_open();
    if (ret == CALENDAR_ERROR_NONE)
    {
        __cal_db_initialize_view_table();
    }
    return ret;
}

int _cal_db_close(void)
{
    int ret = CALENDAR_ERROR_NONE;
    ret = _cal_db_util_close();
    return ret;
}

API int calendar_db_get_changes_by_version(const char* view_uri, int calendar_book_id, int calendar_db_version, calendar_list_h* record_list, int *current_calendar_db_version )
{
    char buf[64] = {0};
    const char *query_cur_version = "SELECT ver FROM "CAL_TABLE_VERSION;
    int transaction_ver = 0;
    char query[CAL_DB_SQL_MAX_LEN] = {0};
    sqlite3_stmt *stmt = NULL;
    int ret = 0;
    int is_deleted = 0;

    retvm_if(NULL == current_calendar_db_version || NULL == view_uri || NULL == record_list,
            CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");

    ret = _cal_db_util_query_get_first_int_result(query_cur_version, NULL, &transaction_ver);
	if (CALENDAR_ERROR_NONE != ret)
	{
		ERR("_cal_db_util_query_get_first_int_result() failed");
		return ret;
	}

    if (calendar_book_id > 0)
    {
        snprintf(buf, sizeof(buf), "AND calendar_id = %d ", calendar_book_id);
    }
    else
    {
        memset(buf, 0x0, sizeof(buf));
    }

    if (strcmp(view_uri,_calendar_event._uri) == 0)
    {
        snprintf(query, sizeof(query),
                "SELECT id, changed_ver, created_ver, is_deleted, calendar_id FROM %s "
                "WHERE changed_ver > %d AND changed_ver <= %d AND original_event_id = %d %s"
                "UNION "
                "SELECT schedule_id, deleted_ver, -1, 1, calendar_id FROM %s "
                "WHERE deleted_ver > %d AND schedule_type = %d %s"
                ,
                CAL_VIEW_TABLE_EVENT,
                calendar_db_version, transaction_ver, CAL_INVALID_ID, buf,
                CAL_TABLE_DELETED, calendar_db_version, CAL_RECORD_TYPE_EVENT, buf);
		DBG("event qeury[%s]", query);
    }
    else if (strcmp(view_uri,_calendar_todo._uri) == 0)
    {
        snprintf(query, sizeof(query),
                "SELECT id, changed_ver, created_ver, is_deleted, calendar_id FROM %s "
                "WHERE changed_ver > %d AND changed_ver <= %d AND original_event_id = %d %s"
                "UNION "
                "SELECT schedule_id, deleted_ver, -1, 1, calendar_id FROM %s "
                "WHERE deleted_ver > %d AND schedule_type = %d %s"
                ,
                CAL_VIEW_TABLE_TODO,
                calendar_db_version, transaction_ver, CAL_INVALID_ID, buf,
                CAL_TABLE_DELETED, calendar_db_version, CAL_RECORD_TYPE_TODO, buf);
		DBG("todo qeury[%s]", query);
    }
    else
    {
        ERR("Invalid parameter");
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    ret = calendar_list_create(record_list);
    if (ret != CALENDAR_ERROR_NONE)
    {
        ERR("calendar_list_create() Failed");
        return ret;
    }

    stmt = _cal_db_util_query_prepare(query);

    if (NULL == stmt)
    {
        ERR("_cal_db_util_query_prepare() Failed");
        calendar_list_destroy(*record_list, true);
        *record_list = NULL;
        return CALENDAR_ERROR_DB_FAILED;
    }
    retvm_if(NULL == stmt, CALENDAR_ERROR_DB_FAILED, );

    while(CAL_DB_ROW == _cal_db_util_stmt_step(stmt))
    {
        calendar_record_h record;
        int id = 0, calendar_id = 0,type = 0;
        int ver = 0;
        // stmt -> record
        ret = calendar_record_create(_calendar_updated_info._uri,&record);
        if( ret != CALENDAR_ERROR_NONE )
        {
            calendar_list_destroy(*record_list, true);
            *record_list = NULL;
			sqlite3_finalize(stmt);
            return ret;
        }

        id = sqlite3_column_int(stmt, 0);
        ver = sqlite3_column_int(stmt, 1);
        is_deleted = sqlite3_column_int(stmt, 3);
        if (is_deleted == 1)
        {
            type = CALENDAR_RECORD_MODIFIED_STATUS_DELETED;
        }
        else if (sqlite3_column_int(stmt, 2) != ver)
        {
            type = CALENDAR_RECORD_MODIFIED_STATUS_UPDATED;
        }
        else
        {
            type = CALENDAR_RECORD_MODIFIED_STATUS_INSERTED;
        }

        calendar_id = sqlite3_column_int(stmt, 4);

        _cal_record_set_int(record,_calendar_updated_info.id,id);
        _cal_record_set_int(record,_calendar_updated_info.calendar_book_id,calendar_id);
        _cal_record_set_int(record,_calendar_updated_info.modified_status,type);
        _cal_record_set_int(record,_calendar_updated_info.version,ver);

        ret = calendar_list_add(*record_list,record);
        if( ret != CALENDAR_ERROR_NONE )
        {
            calendar_list_destroy(*record_list, true);
            *record_list = NULL;
            calendar_record_destroy(record, true);
			sqlite3_finalize(stmt);
            return ret;
        }
    }

    *current_calendar_db_version = transaction_ver;
	sqlite3_finalize(stmt);

	calendar_list_first(*record_list);

    return CALENDAR_ERROR_NONE;
}

API int calendar_db_get_current_version(int* current_version)
{
    const char *query = "SELECT ver FROM "CAL_TABLE_VERSION;
    int transaction_ver = 0;
	int ret;

    retvm_if(NULL == current_version, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");

    ret = _cal_db_util_query_get_first_int_result(query, NULL, &transaction_ver);
	if (CALENDAR_ERROR_NONE != ret)
	{
		ERR("_cal_db_util_query_get_first_int_result() failed");
		return ret;
	}
    if (current_version) *current_version = transaction_ver;

    return CALENDAR_ERROR_NONE;
}

API int calendar_db_insert_record( calendar_record_h record, int* id )
{
	cal_record_s *temp=NULL ;
	int ret = CALENDAR_ERROR_NONE;

	retvm_if(NULL == record, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");

	temp = (cal_record_s*)(record);

	cal_db_plugin_cb_s* plugin_cb = __cal_db_get_plugin(temp->type);
	retvm_if(NULL == plugin_cb, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");
	retvm_if(NULL == plugin_cb->insert_record, CALENDAR_ERROR_NOT_PERMITTED, "Not permitted");

    ret = _cal_db_util_begin_trans();
    retvm_if( CALENDAR_ERROR_NONE != ret,CALENDAR_ERROR_DB_FAILED, "Db failed" );

	ret = plugin_cb->insert_record(record, id);

    if (ret == CALENDAR_ERROR_NONE)
    {
        ret = _cal_db_util_end_trans(true);
    }
    else
    {
        _cal_db_util_end_trans(false);
    }

	return ret;
}

API int calendar_db_get_record( const char* view_uri, int id, calendar_record_h* out_record )
{
	int ret = CALENDAR_ERROR_NONE;
	cal_record_type_e type = CAL_RECORD_TYPE_INVALID;

	retvm_if(NULL == view_uri, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");

	type = _cal_view_get_type(view_uri);

	cal_db_plugin_cb_s* plugin_cb = __cal_db_get_plugin(type);
	retvm_if(NULL == plugin_cb, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");
	retvm_if(NULL == plugin_cb->get_record, CALENDAR_ERROR_NOT_PERMITTED, "Not permitted");

	ret = plugin_cb->get_record(id, out_record);

	return ret;
}

API int calendar_db_update_record( calendar_record_h record )
{
	cal_record_s *temp=NULL ;
	int ret = CALENDAR_ERROR_NONE;

	retvm_if(NULL == record, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");

	temp = (cal_record_s*)(record);

	cal_db_plugin_cb_s* plugin_cb = __cal_db_get_plugin(temp->type);
	retvm_if(NULL == plugin_cb, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");
	retvm_if(NULL == plugin_cb->update_record, CALENDAR_ERROR_NOT_PERMITTED, "Not permitted");

    ret = _cal_db_util_begin_trans();
    retvm_if( CALENDAR_ERROR_NONE != ret,CALENDAR_ERROR_DB_FAILED, "Db failed" );

	ret = plugin_cb->update_record(record);

    if (ret == CALENDAR_ERROR_NONE)
    {
        ret = _cal_db_util_end_trans(true);
    }
    else
    {
        _cal_db_util_end_trans(false);
    }

	return ret;
}

API int calendar_db_delete_record( const char* view_uri, int id )
{
	int ret = CALENDAR_ERROR_NONE;
	cal_record_type_e type = CAL_RECORD_TYPE_INVALID;

	retvm_if(NULL == view_uri, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");

	type = _cal_view_get_type(view_uri);

	cal_db_plugin_cb_s* plugin_cb = __cal_db_get_plugin(type);
	retvm_if(NULL == plugin_cb, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");
	retvm_if(NULL == plugin_cb->delete_record, CALENDAR_ERROR_NOT_PERMITTED, "Not permitted");

    ret = _cal_db_util_begin_trans();
    retvm_if( CALENDAR_ERROR_NONE != ret,CALENDAR_ERROR_DB_FAILED, "Db failed" );

	ret = plugin_cb->delete_record(id);

    if (ret == CALENDAR_ERROR_NONE)
    {
        ret = _cal_db_util_end_trans(true);
    }
    else
    {
        _cal_db_util_end_trans(false);
    }

	return ret;
}

API int calendar_db_get_all_records( const char* view_uri, int offset, int limit, calendar_list_h* out_list )
{
	int ret = CALENDAR_ERROR_NONE;
	cal_record_type_e type = CAL_RECORD_TYPE_INVALID;
	calendar_list_h list = NULL;

	retvm_if(NULL == view_uri, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");

	type = _cal_view_get_type(view_uri);

	cal_db_plugin_cb_s* plugin_cb = __cal_db_get_plugin(type);
	retvm_if(NULL == plugin_cb, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");
	retvm_if(NULL == plugin_cb->get_all_records, CALENDAR_ERROR_NOT_PERMITTED, "Not permitted");

	ret = plugin_cb->get_all_records(offset,limit, &list);
    if (CALENDAR_ERROR_NONE != ret)
    {
		ERR("get_all_records() failed");
		return ret;
	}
	calendar_list_first(list);
	if (out_list) *out_list = list;

	return CALENDAR_ERROR_NONE;
}

API int calendar_db_get_records_with_query( calendar_query_h query, int offset, int limit, calendar_list_h* out_list )
{
	int ret = CALENDAR_ERROR_NONE;
	cal_record_type_e type = CAL_RECORD_TYPE_INVALID;
	cal_query_s *que = NULL;
	calendar_list_h list = NULL;

	retvm_if(NULL == query, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");
	que = (cal_query_s *)query;

	type = _cal_view_get_type(que->view_uri);

	cal_db_plugin_cb_s* plugin_cb = __cal_db_get_plugin(type);
	retvm_if(NULL == plugin_cb, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");
	retvm_if(NULL == plugin_cb->get_records_with_query, CALENDAR_ERROR_NOT_PERMITTED, "Not permitted");

	ret = plugin_cb->get_records_with_query(query,offset,limit, &list);
    if (CALENDAR_ERROR_NONE != ret)
    {
		ERR("get_records_with_query() failed");
		return ret;
	}
	calendar_list_first(list);
	if (out_list) *out_list = list;

	return CALENDAR_ERROR_NONE;
}

API int calendar_db_clean_after_sync( int calendar_book_id )
{
    int ret;
    char query[CAL_DB_SQL_MIN_LEN] = {0};
    cal_db_util_error_e dbret = CAL_DB_OK;

    retvm_if(calendar_book_id < 0, CALENDAR_ERROR_INVALID_PARAMETER, "calendar_id(%d) is Invalid", calendar_book_id);

    ret = _cal_db_util_begin_trans();
    retvm_if( CALENDAR_ERROR_NONE != ret,CALENDAR_ERROR_DB_FAILED, "Db failed" );
    // !! please check rrule_table, alarm_table, attendee_table ..

    /* delete event table */
    snprintf(query, sizeof(query), "DELETE FROM %s "
            "WHERE is_deleted = 1 AND calendar_id = %d",
            CAL_TABLE_SCHEDULE,
            calendar_book_id);

    dbret = _cal_db_util_query_exec(query);
    CAL_DBG("%s",query);
    if (dbret != CAL_DB_OK)
    {
        ERR("DB failed");
        _cal_db_util_end_trans(false);
		switch (dbret)
		{
		case CAL_DB_ERROR_NO_SPACE:
			return CALENDAR_ERROR_FILE_NO_SPACE;
		default:
			return CALENDAR_ERROR_DB_FAILED;
		}
    }
    retvm_if(ret < 0, ret, "cals_query_exec() failed (%d)", ret);

    /* delete delete table */
    snprintf(query, sizeof(query), "DELETE FROM %s "
            "WHERE calendar_id = %d",
            CAL_TABLE_DELETED,
            calendar_book_id);

    dbret = _cal_db_util_query_exec(query);
    CAL_DBG("%s",query);
    if (dbret != CAL_DB_OK)
    {
        ERR("DB failed");
        _cal_db_util_end_trans(false);
		switch (dbret)
		{
		case CAL_DB_ERROR_NO_SPACE:
			return CALENDAR_ERROR_FILE_NO_SPACE;
		default:
			return CALENDAR_ERROR_DB_FAILED;
		}
    }

    _cal_db_util_end_trans(true);
	return CALENDAR_ERROR_NONE;
}

API int calendar_db_get_count( const char* view_uri, int *out_count )
{
    int ret = CALENDAR_ERROR_NONE;
    cal_record_type_e type = CAL_RECORD_TYPE_INVALID;

    retvm_if(NULL == view_uri, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");

    type = _cal_view_get_type(view_uri);

    cal_db_plugin_cb_s* plugin_cb = __cal_db_get_plugin(type);
    retvm_if(NULL == plugin_cb, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");
    retvm_if(NULL == plugin_cb->get_count, CALENDAR_ERROR_NOT_PERMITTED, "Not permitted");

    ret = plugin_cb->get_count(out_count);
    return CALENDAR_ERROR_NONE;
}

API int calendar_db_get_count_with_query( calendar_query_h query, int *out_count )
{
    int ret = CALENDAR_ERROR_NONE;
    cal_record_type_e type = CAL_RECORD_TYPE_INVALID;
    cal_query_s *que = NULL;

    retvm_if(NULL == query, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");
    que = (cal_query_s *)query;

    type = _cal_view_get_type(que->view_uri);

    cal_db_plugin_cb_s* plugin_cb = __cal_db_get_plugin(type);
    retvm_if(NULL == plugin_cb, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");
    retvm_if(NULL == plugin_cb->get_count_with_query, CALENDAR_ERROR_NOT_PERMITTED, "Not permitted");

    ret = plugin_cb->get_count_with_query(query, out_count);
    return CALENDAR_ERROR_NONE;
}

API int calendar_db_insert_records(calendar_list_h list, int** ids, int* count)
{
    int ret = CALENDAR_ERROR_NONE;
	int i;
	int *_ids = NULL;
	int _count = 0;

    retvm_if(NULL == list, CALENDAR_ERROR_NOT_PERMITTED, "Not permitted");

	ret = _cal_db_util_begin_trans();
	if ( ret != CALENDAR_ERROR_NONE)
	{
		ERR("Db failed");
		return CALENDAR_ERROR_DB_FAILED;
	}

	_count = 0;
	calendar_list_get_count(list, &_count);
	DBG("list count(%d)", _count);

	_ids = calloc(_count, sizeof(int));

	calendar_list_first(list);
	for (i = 0; i < _count; i++)
	{
		calendar_record_h record = NULL;
		ret = calendar_list_get_current_record_p(list, &record);
		if (record == NULL|| ret != CALENDAR_ERROR_NONE)
		{
			ERR("No record in the list");
			_cal_db_util_end_trans(false);
			CAL_FREE(_ids);
			return ret;
		}

		cal_record_s *temp = (cal_record_s *)record;
		cal_db_plugin_cb_s* plugin_cb = __cal_db_get_plugin(temp->type);
		if (NULL == plugin_cb || NULL == plugin_cb->insert_record)
		{
			DBG("Not plugin");
			_cal_db_util_end_trans(false);
			CAL_FREE(_ids);
			ret = CALENDAR_ERROR_NOT_PERMITTED;
			return ret;
		}
		ret = plugin_cb->insert_record(record, &_ids[i]);
		if (ret != CALENDAR_ERROR_NONE)
		{
			DBG("Failed to insert record");
			_cal_db_util_end_trans(false);
			CAL_FREE(_ids);
			return ret;
		}
		DBG("insert with id(%d)", _ids[i]);
		calendar_list_next(list);
	}

	if (ids)
	{
		*ids = _ids;
	}
	else
	{
		CAL_FREE(_ids);
	}
	if (count) *count = _count;
	_cal_db_util_end_trans(true);

    return ret;
}

API int calendar_db_insert_records_async(calendar_list_h list, calendar_db_insert_result_cb callback, void *user_data)
{
    retvm_if(NULL == list, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");

#ifdef CAL_NATIVE
    if (callback != NULL)
    {
        int ret = CALENDAR_ERROR_NONE;
        calendar_list_h out_list = NULL;

        ret = _cal_list_clone(list, &out_list);
        retv_if(ret!=CALENDAR_ERROR_NONE,ret);

        __insert_records_data_s *callback_data = NULL;
        callback_data = calloc(1,sizeof(__insert_records_data_s));

        if (callback_data == NULL)
        {
            ERR("calloc fail");
            calendar_list_destroy(out_list, true);
            return CALENDAR_ERROR_OUT_OF_MEMORY;
        }
        callback_data->list = out_list;
        callback_data->callback = callback;
        callback_data->user_data = user_data;
        g_idle_add( &__cal_db_insert_records_idle, callback_data);
        return CALENDAR_ERROR_NONE;
    }
#endif
    int ret = CALENDAR_ERROR_NONE;
	int i;
    int *ids = NULL;
	int count = 0;

    retvm_if(NULL == list, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");

	ret = _cal_db_util_begin_trans();
	if ( ret != CALENDAR_ERROR_NONE)
	{
		ERR("Db failed");
		return CALENDAR_ERROR_DB_FAILED;
	}

	calendar_list_get_count(list, &count);
	DBG("list count(%d)", count);

	ids = calloc(count, sizeof(int));

	calendar_list_first(list);
	for (i = 0; i < count; i++)
	{
		calendar_record_h record = NULL;
		ret = calendar_list_get_current_record_p(list, &record);
		if (record == NULL || ret != CALENDAR_ERROR_NONE)
		{
			ERR("No record in the list");
			CAL_FREE(ids);
			_cal_db_util_end_trans(false);
			return ret;
		}

		cal_record_s *temp = (cal_record_s *)record;
		cal_db_plugin_cb_s* plugin_cb = __cal_db_get_plugin(temp->type);
		if (NULL == plugin_cb || NULL == plugin_cb->insert_record)
		{
			DBG("Not plugin");
			CAL_FREE(ids);
			_cal_db_util_end_trans(false);
			ret = CALENDAR_ERROR_NOT_PERMITTED;
			return ret;
		}
		ret = plugin_cb->insert_record(record, &ids[i]);
		if (ret != CALENDAR_ERROR_NONE)
		{
			DBG("Failed to insert record");
			CAL_FREE(ids);
			_cal_db_util_end_trans(false);
			return ret;
		}
		DBG("insert with id(%d)", ids[i]);
		calendar_list_next(list);
	}
	CAL_FREE(ids);
	_cal_db_util_end_trans(true);

    return ret;
}

API int calendar_db_update_records( calendar_list_h list)
{
	int i;
	int count = 0;
    int ret = CALENDAR_ERROR_NONE;

    retvm_if(NULL == list, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");

	ret = _cal_db_util_begin_trans();
	if ( ret != CALENDAR_ERROR_NONE)
	{
		ERR("Db failed");
		return CALENDAR_ERROR_DB_FAILED;
	}

	calendar_list_get_count(list, &count);
	DBG("update list count(%d", count);

	calendar_list_first(list);
	for (i = 0; i < count; i++)
	{
		calendar_record_h record = NULL;
		ret = calendar_list_get_current_record_p(list, &record);
		if (record == NULL || ret != CALENDAR_ERROR_NONE)
		{
			ERR("No record in the list");
			_cal_db_util_end_trans(false);
			return ret;
		}

		cal_record_s *temp = (cal_record_s *)record;
		cal_db_plugin_cb_s* plugin_cb = __cal_db_get_plugin(temp->type);
		if (NULL == plugin_cb || NULL == plugin_cb->update_record)
		{
			ERR("Not plugin");
			_cal_db_util_end_trans(false);
			ret = CALENDAR_ERROR_NOT_PERMITTED;
			return ret;
		}
		ret = plugin_cb->update_record(record);
		if (ret != CALENDAR_ERROR_NONE)
		{
			ERR("Failed to update record");
			_cal_db_util_end_trans(false);
			return ret;
		}
		DBG("update record");
		calendar_list_next(list);
	}
	_cal_db_util_end_trans(true);

    return ret;
}

API int calendar_db_update_records_async( calendar_list_h list, calendar_db_result_cb callback, void *user_data)
{
    retvm_if(NULL == list, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");

#ifdef CAL_NATIVE
    if (callback != NULL)
    {
        int ret = CALENDAR_ERROR_NONE;
        calendar_list_h out_list = NULL;
        ret = _cal_list_clone(list, &out_list);
        retv_if(ret!=CALENDAR_ERROR_NONE,ret);

        __update_records_data_s *callback_data = NULL;
        callback_data = calloc(1,sizeof(__update_records_data_s));

        if (callback_data == NULL)
        {
            ERR("calloc fail");
            calendar_list_destroy(out_list, true);
            return CALENDAR_ERROR_OUT_OF_MEMORY;
        }
        callback_data->list = out_list;
        callback_data->callback = callback;
        callback_data->user_data = user_data;
        g_idle_add( &__cal_db_update_records_idle, callback_data);
        return CALENDAR_ERROR_NONE;
    }
#endif
	int i;
	int count = 0;
    int ret = CALENDAR_ERROR_NONE;

    retvm_if(NULL == list, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");

	ret = _cal_db_util_begin_trans();
	if (ret != CALENDAR_ERROR_NONE)
	{
		ERR("Db failed");
		return CALENDAR_ERROR_DB_FAILED;
	}

	calendar_list_get_count(list, &count);
	DBG("update list count(%d", count);

	calendar_list_first(list);
	for (i = 0; i < count; i++)
	{
		calendar_record_h record = NULL;
		ret = calendar_list_get_current_record_p(list, &record);
		if (record == NULL || ret != CALENDAR_ERROR_NONE)
		{
			ERR("No record in the list");
			_cal_db_util_end_trans(false);
			return ret;
		}

		cal_record_s *temp = (cal_record_s *)record;
		cal_db_plugin_cb_s* plugin_cb = __cal_db_get_plugin(temp->type);
		if (NULL == plugin_cb || NULL == plugin_cb->insert_record)
		{
			ERR("Not plugin");
			_cal_db_util_end_trans(false);
			ret = CALENDAR_ERROR_NOT_PERMITTED;
			return ret;
		}
		ret = plugin_cb->update_record(record);
		if (ret != CALENDAR_ERROR_NONE)
		{
			ERR("Failed to update record");
			_cal_db_util_end_trans(false);
			return ret;
		}
		DBG("update record");
		calendar_list_next(list);
	}
	_cal_db_util_end_trans(true);

    return ret;
}

API int calendar_db_delete_records(const char* view_uri, int record_id_array[], int count)
{
    retvm_if(NULL == view_uri, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");
    retvm_if(NULL == record_id_array, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");
    retvm_if(count <= 0, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");

    int ret = CALENDAR_ERROR_NONE;
    cal_record_type_e type = CAL_RECORD_TYPE_INVALID;

    type = _cal_view_get_type(view_uri);

    cal_db_plugin_cb_s* plugin_cb = __cal_db_get_plugin(type);
    retvm_if(NULL == plugin_cb, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");
    retvm_if(NULL == plugin_cb->delete_records, CALENDAR_ERROR_NOT_PERMITTED, "Not permitted");

    ret = _cal_db_util_begin_trans();
    retvm_if( CALENDAR_ERROR_NONE != ret,CALENDAR_ERROR_DB_FAILED, "Db failed" );

    ret = plugin_cb->delete_records(record_id_array,count);

    if (ret == CALENDAR_ERROR_NONE)
    {
        ret = _cal_db_util_end_trans(true);
    }
    else
    {
        _cal_db_util_end_trans(false);
    }
    return ret;
}

API int calendar_db_delete_records_async(const char* view_uri, int ids[], int count, calendar_db_result_cb callback, void *user_data)
{
    retvm_if(NULL == view_uri, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");
    retvm_if(NULL == ids, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");
    retvm_if(count <= 0, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");
#ifdef CAL_NATIVE
    if (callback != NULL)
    {
        __delete_records_data_s *callback_data = NULL;
        callback_data = calloc(1,sizeof(__delete_records_data_s));

        if (callback_data == NULL)
        {
            ERR("calloc fail");
            return CALENDAR_ERROR_OUT_OF_MEMORY;
        }
        callback_data->view_uri = view_uri;
        callback_data->ids = calloc(1, sizeof(int)*count);
        if (callback_data->ids == NULL)
        {
            CAL_FREE(callback_data);
            ERR("calloc fail");
            return CALENDAR_ERROR_OUT_OF_MEMORY;
        }
        memcpy(callback_data->ids, ids, sizeof(int)*count);
        callback_data->count = count;
        callback_data->callback = callback;
        callback_data->user_data = user_data;
        g_idle_add( &__cal_db_delete_records_idle, callback_data);
        return CALENDAR_ERROR_NONE;
    }
#endif
    int ret = CALENDAR_ERROR_NONE;
    cal_record_type_e type = CAL_RECORD_TYPE_INVALID;

    type = _cal_view_get_type(view_uri);

    cal_db_plugin_cb_s* plugin_cb = __cal_db_get_plugin(type);
    retvm_if(NULL == plugin_cb, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");
    retvm_if(NULL == plugin_cb->delete_records, CALENDAR_ERROR_NOT_PERMITTED, "Not permitted");

    ret = _cal_db_util_begin_trans();
    retvm_if( CALENDAR_ERROR_NONE != ret,CALENDAR_ERROR_DB_FAILED, "Db failed" );

    ret = plugin_cb->delete_records(ids,count);

    if (ret == CALENDAR_ERROR_NONE)
    {
        ret = _cal_db_util_end_trans(true);
    }
    else
    {
        _cal_db_util_end_trans(false);
    }
    return ret;
}

API int calendar_db_insert_vcalendars(const char* vcalendar_stream, int **record_id_array, int *count)
{
    int ret = CALENDAR_ERROR_NONE;
    calendar_list_h list = NULL;
    int list_count = 0;
    int i = 0;
    int *ids = NULL;

    retvm_if(NULL == vcalendar_stream, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");
    retvm_if(NULL == record_id_array, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");
    retvm_if(NULL == count, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");

    ret = calendar_vcalendar_parse_to_calendar(vcalendar_stream, &list);
    retvm_if(ret != CALENDAR_ERROR_NONE, ret, "parse fail");

    ret = calendar_list_get_count(list, &list_count);
    if (ret != CALENDAR_ERROR_NONE)
    {
        calendar_list_destroy(list, true);
        ERR("get count fail");
        return ret;
    }

    calendar_list_first(list);
    ids = calloc(1, sizeof(int)*list_count);
    if (ids == NULL)
    {
        calendar_list_destroy(list, true);
        ERR("calloc fail");
        return CALENDAR_ERROR_OUT_OF_MEMORY;
    }

    ret = _cal_db_util_begin_trans();

    if ( ret != CALENDAR_ERROR_NONE)
    {
        calendar_list_destroy(list, true);
        CAL_FREE(ids);
        ERR("Db failed");
        return CALENDAR_ERROR_DB_FAILED;
    }

    for(i=0;i<list_count;i++)
    {
        calendar_record_h record = NULL;

        ret = calendar_list_get_current_record_p(list, &record);
        if( ret != CALENDAR_ERROR_NONE)
        {
            calendar_list_destroy(list, true);
            CAL_FREE(ids);
            ERR("list get fail");
            _cal_db_util_end_trans(false);
            return ret;
        }

        // insert
        ret = calendar_db_insert_record(record, &ids[i]);
        if( ret != CALENDAR_ERROR_NONE)
        {
            calendar_list_destroy(list, true);
            CAL_FREE(ids);
            ERR("list get fail");
            _cal_db_util_end_trans(false);
            return ret;
        }

        calendar_list_next(list);
    }

    _cal_db_util_end_trans(true);

    *record_id_array = ids;
    *count = list_count;

    calendar_list_destroy(list, true);
    return ret;
}

API int calendar_db_insert_vcalendars_async(const char* vcalendar_stream, calendar_db_insert_result_cb callback, void *user_data)
{
    int ret = CALENDAR_ERROR_NONE;
    retvm_if(NULL == vcalendar_stream, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");
    retvm_if(NULL == callback, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");

#ifdef CAL_NATIVE

    __insert_vcalendars_data_s *callback_data = NULL;
    callback_data = calloc(1,sizeof(__insert_vcalendars_data_s));

    if (callback_data == NULL)
    {
        ERR("calloc fail");
        return CALENDAR_ERROR_OUT_OF_MEMORY;
    }
    callback_data->vcalendar_stream = SAFE_STRDUP(vcalendar_stream);
    callback_data->callback = callback;
    callback_data->user_data = user_data;
    g_idle_add( &__cal_db_insert_vcalendars_idle, callback_data);

#endif
    return ret;
}

API int calendar_db_replace_vcalendars(const char* vcalendar_stream, int *record_id_array, int count)
{
    int ret = CALENDAR_ERROR_NONE;
    calendar_list_h list = NULL;
    int list_count = 0;
    int i = 0;

    retvm_if(NULL == vcalendar_stream, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");
    retvm_if(NULL == record_id_array, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");
    retvm_if(count <= 0, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");

    ret = calendar_vcalendar_parse_to_calendar(vcalendar_stream, &list);
    retvm_if(ret != CALENDAR_ERROR_NONE, ret, "parse fail");

    ret = calendar_list_get_count(list, &list_count);
    if (ret != CALENDAR_ERROR_NONE)
    {
        calendar_list_destroy(list, true);
        ERR("get count fail");
        return ret;
    }

    // check count
    if (count != list_count)
    {
        calendar_list_destroy(list, true);
        ERR("get count fail vcalendar_count=%d, input count=%d", list_count, count);
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    calendar_list_first(list);

    ret = _cal_db_util_begin_trans();

    if ( ret != CALENDAR_ERROR_NONE)
    {
        calendar_list_destroy(list, true);
        ERR("Db failed");
        return CALENDAR_ERROR_DB_FAILED;
    }

    for(i=0;i<list_count;i++)
    {
        calendar_record_h record = NULL;
        char *view_uri = NULL;

        ret = calendar_list_get_current_record_p(list, &record);
        if( ret != CALENDAR_ERROR_NONE)
        {
            calendar_list_destroy(list, true);
            ERR("list get fail");
            _cal_db_util_end_trans(false);
            return ret;
        }

        // set_id
        ret = calendar_record_get_uri_p(record, &view_uri);
        if( ret != CALENDAR_ERROR_NONE)
        {
            calendar_list_destroy(list, true);
            ERR("record get uri fail");
            _cal_db_util_end_trans(false);
            return ret;
        }

        if(strcmp(view_uri, _calendar_event._uri) == 0)
        {
            ret = _cal_record_set_int(record, _calendar_event.id, record_id_array[i]);
        }
        else if(strcmp(view_uri, _calendar_todo._uri) == 0)
        {
            ret = _cal_record_set_int(record, _calendar_todo.id, record_id_array[i]);
        }
        else
        {
            calendar_list_destroy(list, true);
            ERR("record get uri(%s) fail", view_uri);
            _cal_db_util_end_trans(false);
            return ret;
        }

        if( ret != CALENDAR_ERROR_NONE)
        {
            calendar_list_destroy(list, true);
            ERR("record set fail");
            _cal_db_util_end_trans(false);
            return ret;
        }

        // update
        ret = calendar_db_update_record(record);
        if( ret != CALENDAR_ERROR_NONE)
        {
            calendar_list_destroy(list, true);
            ERR("list get fail");
            _cal_db_util_end_trans(false);
            return ret;
        }

        calendar_list_next(list);
    }

    _cal_db_util_end_trans(true);

    calendar_list_destroy(list, true);

    return ret;
}

API int calendar_db_replace_vcalendars_async(const char* vcalendar_stream, int *record_id_array, int count, calendar_db_result_cb callback, void *user_data)
{
    int ret = CALENDAR_ERROR_NONE;
    retvm_if(NULL == vcalendar_stream, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");
    retvm_if(NULL == callback, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");
    retvm_if(NULL == record_id_array, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");
    retvm_if(count <= 0, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");

#ifdef CAL_NATIVE

    __replace_vcalendars_data_s *callback_data = NULL;
    callback_data = calloc(1,sizeof(__replace_vcalendars_data_s));

    if (callback_data == NULL)
    {
        ERR("calloc fail");
        return CALENDAR_ERROR_OUT_OF_MEMORY;
    }
    callback_data->ids = calloc(1, sizeof(int)*count);
    if (callback_data->ids == NULL)
    {
        CAL_FREE(callback_data);
        ERR("calloc fail");
        return CALENDAR_ERROR_OUT_OF_MEMORY;
    }
    memcpy(callback_data->ids, record_id_array, sizeof(int)*count);
    callback_data->vcalendar_stream = SAFE_STRDUP(vcalendar_stream);
    callback_data->callback = callback;
    callback_data->user_data = user_data;
    callback_data->count = count;
    g_idle_add( &__cal_db_replace_vcalendars_idle, callback_data);

#endif
    return ret;
}

API int calendar_db_replace_record(calendar_record_h record, int record_id)
{
    cal_record_s *temp=NULL ;
    int ret = CALENDAR_ERROR_NONE;

    retvm_if(NULL == record, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");
    retvm_if(record_id < 0, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");

    temp = (cal_record_s*)(record);

    cal_db_plugin_cb_s* plugin_cb = __cal_db_get_plugin(temp->type);
    retvm_if(NULL == plugin_cb, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");
    retvm_if(NULL == plugin_cb->replace_record, CALENDAR_ERROR_NOT_PERMITTED, "Not permitted");

    ret = _cal_db_util_begin_trans();
    retvm_if( CALENDAR_ERROR_NONE != ret,CALENDAR_ERROR_DB_FAILED, "Db failed" );

    ret = plugin_cb->replace_record(record, record_id);

    if (ret == CALENDAR_ERROR_NONE)
    {
        ret = _cal_db_util_end_trans(true);
    }
    else
    {
        _cal_db_util_end_trans(false);
    }

    return ret;
}

API int calendar_db_replace_records(calendar_list_h list, int *ids, int count)
{
	int i;
    int ret = CALENDAR_ERROR_NONE;

    retvm_if(NULL == list, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");
    retvm_if(NULL == ids, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");
    retvm_if(count <= 0, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");

	ret = _cal_db_util_begin_trans();
	if ( ret != CALENDAR_ERROR_NONE)
	{
		ERR("Db failed");
		return CALENDAR_ERROR_DB_FAILED;
	}

	calendar_list_first(list);
	for (i = 0; i < count; i++)
	{
		calendar_record_h record = NULL;
		ret = calendar_list_get_current_record_p(list, &record);
		if (record == NULL || ret != CALENDAR_ERROR_NONE)
		{
			ERR("No record in the list");
			_cal_db_util_end_trans(false);
			return ret;
		}

		cal_record_s *temp = (cal_record_s *)record;
		cal_db_plugin_cb_s* plugin_cb = __cal_db_get_plugin(temp->type);
		if (NULL == plugin_cb || NULL == plugin_cb->insert_record)
		{
			DBG("Not plugin");
			_cal_db_util_end_trans(false);
			ret = CALENDAR_ERROR_NOT_PERMITTED;
			return ret;
		}
		ret = plugin_cb->replace_record(record, ids[i]);
		if (ret != CALENDAR_ERROR_NONE)
		{
			DBG("Failed to insert record");
			_cal_db_util_end_trans(false);
			return ret;
		}
		DBG("insert with id(%d)", ids[i]);
		calendar_list_next(list);
	}
	_cal_db_util_end_trans(true);

    return ret;
}

API int calendar_db_replace_records_async(calendar_list_h record_list, int *record_id_array, int count, calendar_db_result_cb callback, void *user_data)
{
    int ret = CALENDAR_ERROR_NONE;
    retvm_if(NULL == record_list, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");
    retvm_if(NULL == callback, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");
    retvm_if(NULL == record_id_array, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");
    retvm_if(count <= 0, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");

#ifdef CAL_NATIVE

    calendar_list_h out_list = NULL;

    ret = _cal_list_clone(record_list, &out_list);
    retv_if(ret!=CALENDAR_ERROR_NONE,ret);

    __replace_records_data_s *callback_data = NULL;
    callback_data = calloc(1,sizeof(__replace_records_data_s));
    if (callback_data == NULL)
    {
        ERR("calloc fail");
		calendar_list_destroy(out_list, true);
        return CALENDAR_ERROR_OUT_OF_MEMORY;
    }

    callback_data->ids = calloc(1, sizeof(int)*count);
    if (callback_data->ids == NULL)
    {
        CAL_FREE(callback_data);
        ERR("calloc fail");
		calendar_list_destroy(out_list, true);
        return CALENDAR_ERROR_OUT_OF_MEMORY;
    }
    memcpy(callback_data->ids, record_id_array, sizeof(int)*count);
    callback_data->list = out_list;
    callback_data->callback = callback;
    callback_data->user_data = user_data;
    callback_data->count = count;
    g_idle_add( &__cal_db_replace_records_idle, callback_data);

#endif
    return ret;
}

API int calendar_db_get_last_change_version(int* last_version)
{
    int ret = CALENDAR_ERROR_NONE;

    retvm_if(NULL == last_version, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");
    *last_version = _cal_db_util_get_transaction_ver();
    return ret;
}