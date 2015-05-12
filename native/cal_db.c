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

#include <unistd.h> // sleep
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
#include "cal_access_control.h"

#define BULK_DEFAULT_COUNT 100

/*
 * Declear DB plugin
 */
extern cal_db_plugin_cb_s cal_db_calendar_plugin_cb;
extern cal_db_plugin_cb_s cal_db_event_plugin_cb;
extern cal_db_plugin_cb_s cal_db_instance_normal_plugin_cb;
extern cal_db_plugin_cb_s cal_db_instance_normal_extended_plugin_cb;
extern cal_db_plugin_cb_s cal_db_instance_allday_plugin_cb;
extern cal_db_plugin_cb_s cal_db_instance_allday_extended_plugin_cb;
extern cal_db_plugin_cb_s cal_db_todo_plugin_cb;
extern cal_db_plugin_cb_s cal_db_alarm_plugin_cb;
extern cal_db_plugin_cb_s cal_db_attendee_plugin_cb;
extern cal_db_plugin_cb_s cal_db_search_plugin_cb;
extern cal_db_plugin_cb_s cal_db_timezone_plugin_cb;
extern cal_db_plugin_cb_s cal_db_extended_plugin_cb;

static cal_db_plugin_cb_s* _cal_db_get_plugin(cal_record_type_e type)
{
	switch (type)
	{
	case CAL_RECORD_TYPE_CALENDAR:
		return (&cal_db_calendar_plugin_cb);
	case CAL_RECORD_TYPE_EVENT:
		return (&cal_db_event_plugin_cb);
	case CAL_RECORD_TYPE_TODO:
		return (&cal_db_todo_plugin_cb);
	case CAL_RECORD_TYPE_ALARM:
		return (&cal_db_alarm_plugin_cb);
	case CAL_RECORD_TYPE_ATTENDEE:
		return (&cal_db_attendee_plugin_cb);
	case CAL_RECORD_TYPE_TIMEZONE:
		return (&cal_db_timezone_plugin_cb);
	case CAL_RECORD_TYPE_INSTANCE_NORMAL:
		return (&cal_db_instance_normal_plugin_cb);
	case CAL_RECORD_TYPE_INSTANCE_ALLDAY:
		return (&cal_db_instance_allday_plugin_cb);
	case CAL_RECORD_TYPE_INSTANCE_NORMAL_EXTENDED:
		return (&cal_db_instance_normal_extended_plugin_cb);
	case CAL_RECORD_TYPE_INSTANCE_ALLDAY_EXTENDED:
		return (&cal_db_instance_allday_extended_plugin_cb);
	case CAL_RECORD_TYPE_SEARCH:
		return (&cal_db_search_plugin_cb);
	case CAL_RECORD_TYPE_EXTENDED:
		return (&cal_db_extended_plugin_cb);
	default:
		ERR("Invalid plugin(%d)", type);
		return NULL;
	}
	return NULL;
}

void cal_db_initialize_view_table(void)
{
	int ret = CALENDAR_ERROR_NONE;
	cal_db_util_error_e dbret;
	char query[CAL_DB_SQL_MAX_LEN] = {0};

	ret = cal_db_util_begin_trans();
	if (CALENDAR_ERROR_NONE != ret)
	{
		ERR("cal_db_util_begin_trans() failed");
		return ;
	}

	/*
	 * CAL_VIEW_TABLE_EVENT
	 * schedule_table(type=1) + rrule_table
	 * A : schedule_table
	 * B : rrule_table
	 * C : calendar_table
	 */
	snprintf(query, sizeof(query),
			"CREATE VIEW IF NOT EXISTS %s AS "
			"SELECT "CAL_QUERY_SCHEDULE_A_ALL", "
			"B.range_type, B.until_type, B.until_utime, B.until_datetime, B.count, "
			"B.interval, B.bysecond, B.byminute, B.byhour, B.byday, B.bymonthday, "
			"B.byyearday, B.byweekno, B.bymonth, B.bysetpos, B.wkst, C.deleted "
			"FROM %s as A "
			"LEFT OUTER JOIN %s B ON A.id = B.event_id "
			"JOIN %s C ON A.calendar_id = C.id "
			"WHERE A.type = %d AND A.is_deleted=0 "
			"AND C.deleted = 0 ",
			CAL_VIEW_TABLE_EVENT,
			CAL_TABLE_SCHEDULE,
			CAL_TABLE_RRULE,
			CAL_TABLE_CALENDAR,
			CAL_SCH_TYPE_EVENT);
	dbret = cal_db_util_query_exec(query);
	//SEC_DBG("%s",query);
	if (dbret != CAL_DB_OK)
	{
		SEC_ERR("[%s]", query);
		ERR("create view fail");
	}

	/*
	 * CAL_VIEW_TABLE_TODO
	 * schedule_table(type=2) + rrule_table
	 * A : schedule_table
	 * B : rrule_table
	 * C : calendar_table
	 */
	snprintf(query, sizeof(query),
			"CREATE VIEW IF NOT EXISTS %s AS "
			"SELECT "CAL_QUERY_SCHEDULE_A_ALL", "
			"B.range_type, B.until_type, B.until_utime, B.until_datetime, B.count, "
			"B.interval, B.bysecond, B.byminute, B.byhour, B.byday, B.bymonthday, "
			"B.byyearday, B.byweekno, B.bymonth, B.bysetpos, B.wkst, C.deleted "
			"FROM %s as A "
			"LEFT OUTER JOIN %s B ON A.id = B.event_id "
			"JOIN %s C ON A.calendar_id = C.id "
			"WHERE A.type = %d AND A.is_deleted=0 "
			"AND C.deleted = 0 ",
			CAL_VIEW_TABLE_TODO,
			CAL_TABLE_SCHEDULE,
			CAL_TABLE_RRULE,
			CAL_TABLE_CALENDAR,
			CAL_SCH_TYPE_TODO);
	dbret = cal_db_util_query_exec(query);
	//SEC_DBG("%s",query);
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
			"CREATE VIEW IF NOT EXISTS %s AS SELECT A.event_id, "
			"B.dtstart_type, A.dtstart_utime, B.dtstart_datetime, "
			"B.dtend_type, A.dtend_utime, B.dtend_datetime, "
			"B.summary, B.description, "
			"B.location, B.busy_status, B.task_status, B.priority, B.sensitivity, "
			"B.rrule_id, B.latitude, B.longitude, B.has_alarm, B.original_event_id, "
			"B.calendar_id, B.last_mod, "
			"B.sync_data1, "
			"C.visibility, C.account_id "
			"FROM %s as A, %s as B, %s as C "
			"ON A.event_id = B.id AND B.calendar_id = C.id "
			"where C.deleted = 0 AND B.is_deleted = 0 ",
			CAL_VIEW_TABLE_NORMAL_INSTANCE,
			CAL_TABLE_NORMAL_INSTANCE,
			CAL_TABLE_SCHEDULE,
			CAL_TABLE_CALENDAR);
	dbret = cal_db_util_query_exec(query);
	//SEC_DBG("%s",query);
	if (dbret != CAL_DB_OK)
	{
		ERR("create view fail");
	}

	// CAL_VIEW_TABLE_ALLDAY_INSTANCE  : CALENDAR_VIEW_INSTANCE_ALLDAY_CALENDAR
	snprintf(query, sizeof(query),
			"CREATE VIEW IF NOT EXISTS %s AS SELECT A.event_id, "
			"B.dtstart_type, B.dtstart_utime, A.dtstart_datetime, "
			"B.dtend_type, B.dtend_utime, A.dtend_datetime, "
			"B.summary, B.description, "
			"B.location, B.busy_status, B.task_status, B.priority, B.sensitivity, "
			"B.rrule_id, B.latitude, B.longitude, B.has_alarm, B.original_event_id, "
			"B.calendar_id, B.last_mod, "
			"B.sync_data1, B.is_allday, "
			"C.visibility, C.account_id "
			"FROM %s as A, %s as B, %s as C "
			"ON A.event_id = B.id AND B.calendar_id = C.id "
			"where C.deleted = 0 AND B.is_deleted = 0 ",
			CAL_VIEW_TABLE_ALLDAY_INSTANCE,
			CAL_TABLE_ALLDAY_INSTANCE,
			CAL_TABLE_SCHEDULE,
			CAL_TABLE_CALENDAR);
	dbret = cal_db_util_query_exec(query);
	//SEC_DBG("%s",query);
	if (dbret != CAL_DB_OK)
	{
		ERR("create view fail");
	}

	/*
	 * CAL_VIEW_TABLE_NORMAL_INSTANCE_EXTENDED  : CALENDAR_VIEW_INSTANCE_NORMAL_CALENDAR_EXTENDED
	 * normal_instance_table + schedule_table(type=1) + calendar_table
	 * A = normal_instace_table
	 * B = schedule_table
	 * C = calendar_table
	 */
	snprintf(query, sizeof(query),
			"CREATE VIEW IF NOT EXISTS %s AS SELECT A.event_id, "
			"B.dtstart_type, A.dtstart_utime, B.dtstart_datetime, "
			"B.dtend_type, A.dtend_utime, B.dtend_datetime, "
			"B.summary, B.description, "
			"B.location, B.busy_status, B.task_status, B.priority, B.sensitivity, "
			"B.rrule_id, B.latitude, B.longitude, B.has_alarm, B.original_event_id, "
			"B.calendar_id, B.last_mod, "
			"B.sync_data1, B.organizer_name, B.categories, B.has_attendee, B.sync_data2, B.sync_data3, B.sync_data4, "
			"C.visibility, C.account_id "
			"FROM %s as A, %s as B, %s as C "
			"ON A.event_id = B.id AND B.calendar_id = C.id "
			"where C.deleted = 0 AND B.is_deleted = 0 ",
			CAL_VIEW_TABLE_NORMAL_INSTANCE_EXTENDED,
			CAL_TABLE_NORMAL_INSTANCE,
			CAL_TABLE_SCHEDULE,
			CAL_TABLE_CALENDAR);
	dbret = cal_db_util_query_exec(query);
	//SEC_DBG("%s",query);
	if (dbret != CAL_DB_OK)
	{
		ERR("create view fail");
	}

	// CAL_VIEW_TABLE_ALLDAY_INSTANCE_EXTENDED  : CALENDAR_VIEW_INSTANCE_ALLDAY_CALENDAR_EXTENDED
	snprintf(query, sizeof(query),
			"CREATE VIEW IF NOT EXISTS %s AS SELECT A.event_id, "
			"B.dtstart_type, B.dtstart_utime, A.dtstart_datetime, "
			"B.dtend_type, B.dtend_utime, A.dtend_datetime, "
			"B.summary, B.description, "
			"B.location, B.busy_status, B.task_status, B.priority, B.sensitivity, "
			"B.rrule_id, B.latitude, B.longitude, B.has_alarm, B.original_event_id, "
			"B.calendar_id, B.last_mod, "
			"B.sync_data1, B.organizer_name, B.categories, B.has_attendee, B.sync_data2, B.sync_data3, B.sync_data4, B.is_allday, "
			"C.visibility, C.account_id "
			"FROM %s as A, %s as B, %s as C "
			"ON A.event_id = B.id AND B.calendar_id = C.id "
			"where C.deleted = 0 AND B.is_deleted = 0 ",
			CAL_VIEW_TABLE_ALLDAY_INSTANCE_EXTENDED,
			CAL_TABLE_ALLDAY_INSTANCE,
			CAL_TABLE_SCHEDULE,
			CAL_TABLE_CALENDAR);
	dbret = cal_db_util_query_exec(query);
	//SEC_DBG("%s",query);
	if (dbret != CAL_DB_OK)
	{
		ERR("create view fail");
	}

	// event_calendar view  :  CALENDAR_VIEW_EVENT_CALENDAR
	// A : schedule_table
	// B : rrule_table
	// C : calendar_table
	snprintf(query, sizeof(query),
			"CREATE VIEW IF NOT EXISTS %s AS "
			"SELECT "CAL_QUERY_SCHEDULE_A_ALL", "
			"B.range_type, B.until_type, B.until_utime, B.until_datetime, B.count, "
			"B.interval, B.bysecond, B.byminute, B.byhour, B.byday, B.bymonthday, "
			"B.byyearday, B.byweekno, B.bymonth, B.bysetpos, B.wkst, "
			"C.*"
			"FROM %s A "
			"JOIN %s C ON A.calendar_id = C.id "
			"LEFT OUTER JOIN %s B ON A.id = B.event_id "
			"WHERE A.type = 1 AND c.deleted = 0 AND A.is_deleted = 0 ",
			CAL_VIEW_TABLE_EVENT_CALENDAR,
			CAL_TABLE_SCHEDULE, CAL_TABLE_CALENDAR,
			CAL_TABLE_RRULE);
	dbret = cal_db_util_query_exec(query);
	//SEC_DBG("%s",query);
	if (dbret != CAL_DB_OK)
	{
		ERR("create view fail");
	}

	// todo_calendar view  : CALENDAR_VIEW_TODO_CALENDAR
	// A : schedule_table
	// B : rrule_table
	// C : calendar_table
	snprintf(query, sizeof(query),
			"CREATE VIEW IF NOT EXISTS %s AS "
			"SELECT "CAL_QUERY_SCHEDULE_A_ALL", "
			"B.range_type, B.until_type, B.until_utime, B.until_datetime, B.count, "
			"B.interval, B.bysecond, B.byminute, B.byhour, B.byday, B.bymonthday, "
			"B.byyearday, B.byweekno, B.bymonth, B.bysetpos, B.wkst, "
			"C.*"
			"FROM %s A "
			"JOIN %s C ON A.calendar_id = C.id "
			"LEFT OUTER JOIN %s B ON A.id = B.event_id "
			"WHERE A.type = 2 AND c.deleted = 0 AND A.is_deleted = 0 ",
			CAL_VIEW_TABLE_TODO_CALENDAR,
			CAL_TABLE_SCHEDULE, CAL_TABLE_CALENDAR,
			CAL_TABLE_RRULE);
	dbret = cal_db_util_query_exec(query);
	//SEC_DBG("%s",query);
	if (dbret != CAL_DB_OK)
	{
		ERR("create view fail");
	}

	// event_calendar_attendee view  :  CALENDAR_VIEW_EVENT_CALENDAR_ATTENDEE
	// A : schedule_table
	// B : attendee_table
	// C : calendar_table;
	// D : rrule_table;
	snprintf(query, sizeof(query),
			"CREATE VIEW IF NOT EXISTS %s AS "
			"SELECT "CAL_QUERY_SCHEDULE_A_ALL", "
			"D.range_type, D.until_type, D.until_utime, D.until_datetime, D.count, "
			"D.interval, D.bysecond, D.byminute, D.byhour, D.byday, D.bymonthday, "
			"D.byyearday, D.byweekno, D.bymonth, D.bysetpos, D.wkst, "
			"B.*, C.*"
			"FROM %s A "
			"LEFT OUTER  JOIN %s B ON A.id = B.event_id "
			"JOIN %s C ON A.calendar_id = C.id "
			"LEFT OUTER JOIN %s D ON A.id = D.event_id "
			"WHERE A.type = 1 AND c.deleted = 0 AND A.is_deleted = 0 ",
			CAL_VIEW_TABLE_EVENT_CALENDAR_ATTENDEE,
			CAL_TABLE_SCHEDULE, CAL_TABLE_ATTENDEE, CAL_TABLE_CALENDAR,
			CAL_TABLE_RRULE);
	dbret = cal_db_util_query_exec(query);
	//SEC_DBG("%s",query);
	if (dbret != CAL_DB_OK)
	{
		ERR("create view fail");
	}
	cal_db_util_end_trans(true);
	return ;
}

int cal_db_open(void)
{
	int ret = CALENDAR_ERROR_NONE;
	ret = cal_db_util_open();
	if (ret != CALENDAR_ERROR_NONE)
	{
		ERR("db open fail");
	}
	return ret;
}

int cal_db_close(void)
{
	int ret = CALENDAR_ERROR_NONE;
	ret = cal_db_util_close();
	return ret;
}

API int calendar_db_get_changes_by_version(const char* view_uri, int calendar_book_id, int calendar_db_version, calendar_list_h* record_list, int *current_calendar_db_version)
{
	const char *query_cur_version = "SELECT ver FROM "CAL_TABLE_VERSION;
	int transaction_ver = 0;
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	sqlite3_stmt *stmt = NULL;
	int ret = 0;
	int is_deleted = 0;

	RETV_IF(NULL == current_calendar_db_version, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == view_uri, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == record_list, CALENDAR_ERROR_INVALID_PARAMETER);

	ret = cal_db_util_query_get_first_int_result(query_cur_version, NULL, &transaction_ver);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_db_util_query_get_first_int_result() failed(%d)", ret);

	char buf[64] = {0};
	if (calendar_book_id > 0) {
		snprintf(buf, sizeof(buf), "AND calendar_id = %d ", calendar_book_id);

	} else {
		memset(buf, 0x0, sizeof(buf));
	}

	int schedule_type = 0;
	int record_type = 0;
	if (strcmp(view_uri,_calendar_event._uri) == 0) {
		schedule_type = CAL_SCH_TYPE_EVENT;
		record_type = CAL_RECORD_TYPE_EVENT;

	} else if (strcmp(view_uri,_calendar_todo._uri) == 0) {
		schedule_type = CAL_SCH_TYPE_TODO;
		record_type = CAL_RECORD_TYPE_TODO;

	} else {
		ERR("Invalid parameter");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	snprintf(query, sizeof(query),
			"SELECT id, changed_ver, created_ver, is_deleted, calendar_id FROM %s "
			"WHERE changed_ver > %d AND changed_ver <= %d AND type = %d AND original_event_id < 0 %s "
			"UNION "
			"SELECT schedule_id, deleted_ver, created_ver, 1, calendar_id FROM %s "
			"WHERE deleted_ver > %d AND schedule_type = %d AND original_event_id < 0 %s ",
			CAL_TABLE_SCHEDULE,
			calendar_db_version, transaction_ver, schedule_type, buf,
			CAL_TABLE_DELETED,
			calendar_db_version, record_type, buf);
	SEC_DBG("query[%s]", query);

	ret = calendar_list_create(record_list);
	RETVM_IF(ret != CALENDAR_ERROR_NONE, ret, "calendar_list_create() Fail");

	stmt = cal_db_util_query_prepare(query);
	if (NULL == stmt)
	{
		ERR("cal_db_util_query_prepare() Fail");
		calendar_list_destroy(*record_list, true);
		*record_list = NULL;
		return CALENDAR_ERROR_DB_FAILED;
	}

	while(CAL_DB_ROW == cal_db_util_stmt_step(stmt))
	{
		calendar_record_h record;
		int id = 0, calendar_id = 0,type = 0;
		int ver = 0;
		int created_ver = 0;
		// stmt -> record
		ret = calendar_record_create(_calendar_updated_info._uri,&record);
		if(ret != CALENDAR_ERROR_NONE)
		{
			ERR("calendar_record_create() failed");
			calendar_list_destroy(*record_list, true);
			*record_list = NULL;
			sqlite3_finalize(stmt);
			return ret;
		}

		id = sqlite3_column_int(stmt, 0);
		ver = sqlite3_column_int(stmt, 1);
		created_ver = sqlite3_column_int(stmt, 2);
		is_deleted = sqlite3_column_int(stmt, 3);
		if (is_deleted == 1)
		{
			type = CALENDAR_RECORD_MODIFIED_STATUS_DELETED;
		}
		else if (created_ver != ver)
		{
			type = CALENDAR_RECORD_MODIFIED_STATUS_UPDATED;
		}
		else
		{
			type = CALENDAR_RECORD_MODIFIED_STATUS_INSERTED;
		}

		calendar_id = sqlite3_column_int(stmt, 4);

		if (type == CALENDAR_RECORD_MODIFIED_STATUS_DELETED && created_ver > calendar_db_version)
		{
			calendar_record_destroy(record, true);
			DBG("type is deleted, created_ver(%d) > calendar_db_ver(%d), so skip", created_ver, calendar_db_version);
			continue;
		}

		cal_record_set_int(record,_calendar_updated_info.id,id);
		cal_record_set_int(record,_calendar_updated_info.calendar_book_id,calendar_id);
		cal_record_set_int(record,_calendar_updated_info.modified_status,type);
		cal_record_set_int(record,_calendar_updated_info.version,ver);

		ret = calendar_list_add(*record_list,record);
		if(ret != CALENDAR_ERROR_NONE)
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

	RETV_IF(NULL == current_version, CALENDAR_ERROR_INVALID_PARAMETER);

	ret = cal_db_util_query_get_first_int_result(query, NULL, &transaction_ver);
	if (CALENDAR_ERROR_NONE != ret)
	{
		ERR("cal_db_util_query_get_first_int_result() fail(%d)", ret);
		return ret;
	}
	if (current_version) *current_version = transaction_ver;

	return CALENDAR_ERROR_NONE;
}

API int calendar_db_insert_record(calendar_record_h record, int* id)
{
	int ret = CALENDAR_ERROR_NONE;

	RETV_IF(NULL == record, CALENDAR_ERROR_INVALID_PARAMETER);

	cal_db_plugin_cb_s* plugin_cb = _cal_db_get_plugin(((cal_record_s *)record)->type);
	RETV_IF(NULL == plugin_cb, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(NULL == plugin_cb->insert_record, CALENDAR_ERROR_NOT_PERMITTED, "Not permitted");

	ret = cal_db_util_begin_trans();
	RETVM_IF(CALENDAR_ERROR_NONE != ret,CALENDAR_ERROR_DB_FAILED, "Db failed");

	ret = plugin_cb->insert_record(record, id);

	if (CALENDAR_ERROR_NONE == ret)
	{
		ret = cal_db_util_end_trans(true);
	}
	else
	{
		cal_db_util_end_trans(false);
	}

	return ret;
}

int cal_db_get_record(const char* view_uri, int id, calendar_record_h* out_record)
{
	int ret = CALENDAR_ERROR_NONE;
	cal_record_type_e type = CAL_RECORD_TYPE_INVALID;

	RETV_IF(NULL == view_uri, CALENDAR_ERROR_INVALID_PARAMETER);

	type = cal_view_get_type(view_uri);

	cal_db_plugin_cb_s* plugin_cb = _cal_db_get_plugin(type);
	RETV_IF(NULL == plugin_cb, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(NULL == plugin_cb->get_record, CALENDAR_ERROR_NOT_PERMITTED, "Not permitted");

	ret = plugin_cb->get_record(id, out_record);

	return ret;
}

API int calendar_db_get_record(const char* view_uri, int id, calendar_record_h* out_record)
{
	return cal_db_get_record(view_uri, id, out_record);
}

API int calendar_db_update_record(calendar_record_h record)
{
	cal_record_s *temp=NULL ;
	int ret = CALENDAR_ERROR_NONE;

	RETV_IF(NULL == record, CALENDAR_ERROR_INVALID_PARAMETER);

	temp = (cal_record_s*)(record);

	cal_db_plugin_cb_s* plugin_cb = _cal_db_get_plugin(temp->type);
	RETV_IF(NULL == plugin_cb, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(NULL == plugin_cb->update_record, CALENDAR_ERROR_NOT_PERMITTED, "Not permitted");

	ret = cal_db_util_begin_trans();
	RETVM_IF(CALENDAR_ERROR_NONE != ret,CALENDAR_ERROR_DB_FAILED, "Db failed");

	ret = plugin_cb->update_record(record);

	if (CALENDAR_ERROR_NONE == ret)
	{
		ret = cal_db_util_end_trans(true);
	}
	else
	{
		cal_db_util_end_trans(false);
	}

	return ret;
}

API int calendar_db_delete_record(const char* view_uri, int id)
{
	int ret = CALENDAR_ERROR_NONE;
	cal_record_type_e type = CAL_RECORD_TYPE_INVALID;

	RETV_IF(NULL == view_uri, CALENDAR_ERROR_INVALID_PARAMETER);

	type = cal_view_get_type(view_uri);

	cal_db_plugin_cb_s* plugin_cb = _cal_db_get_plugin(type);
	RETV_IF(NULL == plugin_cb, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(NULL == plugin_cb->delete_record, CALENDAR_ERROR_NOT_PERMITTED, "Not permitted");

	ret = cal_db_util_begin_trans();
	RETVM_IF(CALENDAR_ERROR_NONE != ret,CALENDAR_ERROR_DB_FAILED, "Db failed");

	ret = plugin_cb->delete_record(id);

	if (CALENDAR_ERROR_NONE == ret)
	{
		ret = cal_db_util_end_trans(true);
	}
	else
	{
		cal_db_util_end_trans(false);
	}

	return ret;
}

API int calendar_db_get_all_records(const char* view_uri, int offset, int limit, calendar_list_h* out_list)
{
	int ret = CALENDAR_ERROR_NONE;
	cal_record_type_e type = CAL_RECORD_TYPE_INVALID;
	calendar_list_h list = NULL;

	RETV_IF(NULL == view_uri, CALENDAR_ERROR_INVALID_PARAMETER);

	type = cal_view_get_type(view_uri);

	cal_db_plugin_cb_s* plugin_cb = _cal_db_get_plugin(type);
	RETV_IF(NULL == plugin_cb, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(NULL == plugin_cb->get_all_records, CALENDAR_ERROR_NOT_PERMITTED, "Not permitted");

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

API int calendar_db_get_records_with_query(calendar_query_h query, int offset, int limit, calendar_list_h* out_list)
{
	int ret = CALENDAR_ERROR_NONE;
	cal_record_type_e type = CAL_RECORD_TYPE_INVALID;
	cal_query_s *que = NULL;
	calendar_list_h list = NULL;

	RETV_IF(NULL == query, CALENDAR_ERROR_INVALID_PARAMETER);
	que = (cal_query_s *)query;

	type = cal_view_get_type(que->view_uri);

	cal_db_plugin_cb_s* plugin_cb = _cal_db_get_plugin(type);
	RETV_IF(NULL == plugin_cb, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(NULL == plugin_cb->get_records_with_query, CALENDAR_ERROR_NOT_PERMITTED, "Not permitted");

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

API int calendar_db_clean_after_sync(int calendar_book_id,  int calendar_db_version)
{
	int ret = CALENDAR_ERROR_NONE;
	char query[CAL_DB_SQL_MIN_LEN] = {0};
	cal_db_util_error_e dbret = CAL_DB_OK;
	int len = 0;

	RETVM_IF(calendar_book_id < 0, CALENDAR_ERROR_INVALID_PARAMETER, "calendar_id(%d) is Invalid", calendar_book_id);

	ret = cal_db_util_begin_trans();
	if (CALENDAR_ERROR_NONE != ret)
	{
		ERR("cal_db_util_begin_trans() failed");
		return CALENDAR_ERROR_DB_FAILED;
	}
	// !! please check rrule_table, alarm_table, attendee_table ..

	ret =  cal_is_owner(calendar_book_id);
	if (CALENDAR_ERROR_NONE != ret) {
		if (CALENDAR_ERROR_PERMISSION_DENIED == ret)
			ERR("Does not have permission of calendar_book (%d)", calendar_book_id);
		else
			ERR("cal_is_owner Fail(%d)", ret);
		cal_db_util_end_trans(false);
		return ret;
	}

	/* delete event table */
	len = snprintf(query, sizeof(query), "DELETE FROM %s WHERE is_deleted = 1 AND calendar_id = %d",
			CAL_TABLE_SCHEDULE, calendar_book_id);
	if (0 < calendar_db_version)
		len = snprintf(query+len, sizeof(query)-len, " AND changed_ver <= %d", calendar_db_version);

	dbret = cal_db_util_query_exec(query);
	DBG("%s",query);
	if (dbret != CAL_DB_OK)
	{
		ERR("DB failed");
		cal_db_util_end_trans(false);
		switch (dbret)
		{
		case CAL_DB_ERROR_NO_SPACE:
			return CALENDAR_ERROR_FILE_NO_SPACE;
		default:
			return CALENDAR_ERROR_DB_FAILED;
		}
	}
	RETVM_IF(ret < 0, ret, "cals_query_exec() failed (%d)", ret);

	/* delete delete table */
	snprintf(query, sizeof(query), "DELETE FROM %s "
			"WHERE calendar_id = %d",
			CAL_TABLE_DELETED,
			calendar_book_id);

	dbret = cal_db_util_query_exec(query);
	DBG("%s",query);
	if (dbret != CAL_DB_OK)
	{
		ERR("DB failed");
		cal_db_util_end_trans(false);
		switch (dbret)
		{
		case CAL_DB_ERROR_NO_SPACE:
			return CALENDAR_ERROR_FILE_NO_SPACE;
		default:
			return CALENDAR_ERROR_DB_FAILED;
		}
	}
	cal_db_util_end_trans(true);
	return CALENDAR_ERROR_NONE;
}

API int calendar_db_get_count(const char* view_uri, int *out_count)
{
	int ret = CALENDAR_ERROR_NONE;
	cal_record_type_e type = CAL_RECORD_TYPE_INVALID;

	RETV_IF(NULL == view_uri, CALENDAR_ERROR_INVALID_PARAMETER);

	type = cal_view_get_type(view_uri);

	cal_db_plugin_cb_s* plugin_cb = _cal_db_get_plugin(type);
	RETV_IF(NULL == plugin_cb, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(NULL == plugin_cb->get_count, CALENDAR_ERROR_NOT_PERMITTED, "Not permitted");

	ret = plugin_cb->get_count(out_count);
	return ret;
}

API int calendar_db_get_count_with_query(calendar_query_h query, int *out_count)
{
	int ret = CALENDAR_ERROR_NONE;
	cal_record_type_e type = CAL_RECORD_TYPE_INVALID;
	cal_query_s *que = NULL;

	RETV_IF(NULL == query, CALENDAR_ERROR_INVALID_PARAMETER);
	que = (cal_query_s *)query;

	type = cal_view_get_type(que->view_uri);

	cal_db_plugin_cb_s* plugin_cb = _cal_db_get_plugin(type);
	RETV_IF(NULL == plugin_cb, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(NULL == plugin_cb->get_count_with_query, CALENDAR_ERROR_NOT_PERMITTED, "Not permitted");

	ret = plugin_cb->get_count_with_query(query, out_count);
	return ret;
}

API int calendar_db_insert_records(calendar_list_h list, int** ids, int* count)
{
	int ret = CALENDAR_ERROR_NONE;
	int i;
	int *_ids = NULL;
	int _count = 0;

	RETVM_IF(NULL == list, CALENDAR_ERROR_NOT_PERMITTED, "Not permitted");

	ret = cal_db_util_begin_trans();
	if (ret != CALENDAR_ERROR_NONE)
	{
		ERR("Db failed");
		return CALENDAR_ERROR_DB_FAILED;
	}

	_count = 0;
	calendar_list_get_count(list, &_count);
	DBG("list count(%d)", _count);

	_ids = calloc(_count, sizeof(int));

	// divide count for accessing of another modules.
	int div = (int)(_count / BULK_DEFAULT_COUNT) + 1;
	int bulk = _count / div + 1;

	calendar_list_first(list);
	for (i = 0; i < _count; i++)
	{
		calendar_record_h record = NULL;
		ret = calendar_list_get_current_record_p(list, &record);
		if (record == NULL|| ret != CALENDAR_ERROR_NONE)
		{
			ERR("No record in the list");
			cal_db_util_end_trans(false);
			CAL_FREE(_ids);
			return ret;
		}

		cal_record_s *temp = (cal_record_s *)record;
		cal_db_plugin_cb_s* plugin_cb = _cal_db_get_plugin(temp->type);
		if (NULL == plugin_cb || NULL == plugin_cb->insert_record)
		{
			DBG("Not plugin");
			cal_db_util_end_trans(false);
			CAL_FREE(_ids);
			return CALENDAR_ERROR_NOT_PERMITTED;
		}
		ret = plugin_cb->insert_record(record, &_ids[i]);
		if (ret != CALENDAR_ERROR_NONE)
		{
			DBG("Failed to insert record");
			cal_db_util_end_trans(false);
			CAL_FREE(_ids);
			return ret;
		}
		DBG("insert with id(%d)", _ids[i]);
		calendar_list_next(list);

		if (bulk < i)
		{
			bulk += (_count / div + 1);
			cal_db_util_end_trans(true);
			sleep(1);
			ret = cal_db_util_begin_trans();
			if (ret != CALENDAR_ERROR_NONE)
			{
				calendar_list_destroy(list, true);
				CAL_FREE(_ids);
				ERR("Db failed");
				return CALENDAR_ERROR_DB_FAILED;
			}
		}
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
	cal_db_util_end_trans(true);

	return ret;
}

API int calendar_db_update_records(calendar_list_h list)
{
	int i;
	int count = 0;
	int ret = CALENDAR_ERROR_NONE;

	RETV_IF(NULL == list, CALENDAR_ERROR_INVALID_PARAMETER);

	ret = cal_db_util_begin_trans();
	if (ret != CALENDAR_ERROR_NONE)
	{
		ERR("Db failed");
		return CALENDAR_ERROR_DB_FAILED;
	}

	calendar_list_get_count(list, &count);
	DBG("update list count(%d", count);

	// divide count for accessing of another modules.
	int div = (int)(count / BULK_DEFAULT_COUNT) + 1;
	int bulk = count / div + 1;

	calendar_list_first(list);
	for (i = 0; i < count; i++)
	{
		calendar_record_h record = NULL;
		ret = calendar_list_get_current_record_p(list, &record);
		if (record == NULL || ret != CALENDAR_ERROR_NONE)
		{
			ERR("No record in the list");
			cal_db_util_end_trans(false);
			return ret;
		}

		cal_record_s *temp = (cal_record_s *)record;
		cal_db_plugin_cb_s* plugin_cb = _cal_db_get_plugin(temp->type);
		if (NULL == plugin_cb || NULL == plugin_cb->update_record)
		{
			ERR("Not plugin");
			cal_db_util_end_trans(false);
			ret = CALENDAR_ERROR_NOT_PERMITTED;
			return ret;
		}
		ret = plugin_cb->update_record(record);
		if (ret != CALENDAR_ERROR_NONE)
		{
			ERR("Failed to update record");
			cal_db_util_end_trans(false);
			return ret;
		}
		DBG("update record");
		calendar_list_next(list);

		if (bulk < i)
		{
			bulk += (count / div + 1);
			cal_db_util_end_trans(true);
			sleep(1);
			ret = cal_db_util_begin_trans();
			if (ret != CALENDAR_ERROR_NONE)
			{
				calendar_list_destroy(list, true);
				ERR("Db failed");
				return CALENDAR_ERROR_DB_FAILED;
			}
		}
	}
	cal_db_util_end_trans(true);

	return ret;
}

API int calendar_db_delete_records(const char* view_uri, int record_id_array[], int count)
{
	RETV_IF(NULL == view_uri, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == record_id_array, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(count <= 0, CALENDAR_ERROR_INVALID_PARAMETER, "count(%d)", count);

	int ret = CALENDAR_ERROR_NONE;
	cal_record_type_e type = CAL_RECORD_TYPE_INVALID;

	type = cal_view_get_type(view_uri);

	cal_db_plugin_cb_s* plugin_cb = _cal_db_get_plugin(type);
	RETV_IF(NULL == plugin_cb, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(NULL == plugin_cb->delete_records, CALENDAR_ERROR_NOT_PERMITTED, "Not permitted");

	ret = cal_db_util_begin_trans();
	RETVM_IF(CALENDAR_ERROR_NONE != ret,CALENDAR_ERROR_DB_FAILED, "Db failed");

	ret = plugin_cb->delete_records(record_id_array,count);

	if (CALENDAR_ERROR_NONE == ret)
	{
		ret = cal_db_util_end_trans(true);
	}
	else
	{
		cal_db_util_end_trans(false);
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

	RETV_IF(NULL == vcalendar_stream, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == record_id_array, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == count, CALENDAR_ERROR_INVALID_PARAMETER);

	ret = calendar_vcalendar_parse_to_calendar(vcalendar_stream, &list);
	RETVM_IF(ret != CALENDAR_ERROR_NONE, ret, "parse fail");

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

	ret = cal_db_util_begin_trans();

	if (ret != CALENDAR_ERROR_NONE)
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
		if(ret != CALENDAR_ERROR_NONE)
		{
			calendar_list_destroy(list, true);
			CAL_FREE(ids);
			ERR("list get fail");
			cal_db_util_end_trans(false);
			return ret;
		}

		// insert
		ret = calendar_db_insert_record(record, &ids[i]);
		if(ret != CALENDAR_ERROR_NONE)
		{
			calendar_list_destroy(list, true);
			CAL_FREE(ids);
			ERR("list get fail");
			cal_db_util_end_trans(false);
			return ret;
		}

		calendar_list_next(list);
	}

	cal_db_util_end_trans(true);

	*record_id_array = ids;
	*count = list_count;

	calendar_list_destroy(list, true);
	return ret;
}

API int calendar_db_replace_vcalendars(const char* vcalendar_stream, int *record_id_array, int count)
{
	int ret = CALENDAR_ERROR_NONE;
	calendar_list_h list = NULL;
	int list_count = 0;
	int i = 0;

	RETV_IF(NULL == vcalendar_stream, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == record_id_array, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(count <= 0, CALENDAR_ERROR_INVALID_PARAMETER, "count(%d)", count);

	ret = calendar_vcalendar_parse_to_calendar(vcalendar_stream, &list);
	RETVM_IF(ret != CALENDAR_ERROR_NONE, ret, "parse fail");

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

	ret = cal_db_util_begin_trans();
	if (ret != CALENDAR_ERROR_NONE)
	{
		calendar_list_destroy(list, true);
		ERR("Db failed");
		return CALENDAR_ERROR_DB_FAILED;
	}

	// divide count for accessing of another modules.
	int div = (int)(count / BULK_DEFAULT_COUNT) + 1;
	int bulk = count / div + 1;

	for(i = 0; i < list_count; i++)
	{
		calendar_record_h record = NULL;
		char *view_uri = NULL;

		ret = calendar_list_get_current_record_p(list, &record);
		if(ret != CALENDAR_ERROR_NONE)
		{
			calendar_list_destroy(list, true);
			ERR("list get fail");
			cal_db_util_end_trans(false);
			return ret;
		}

		// set_id
		ret = calendar_record_get_uri_p(record, &view_uri);
		if(ret != CALENDAR_ERROR_NONE)
		{
			calendar_list_destroy(list, true);
			ERR("record get uri fail");
			cal_db_util_end_trans(false);
			return ret;
		}

		if(strcmp(view_uri, _calendar_event._uri) == 0)
		{
			ret = cal_record_set_int(record, _calendar_event.id, record_id_array[i]);
		}
		else if(strcmp(view_uri, _calendar_todo._uri) == 0)
		{
			ret = cal_record_set_int(record, _calendar_todo.id, record_id_array[i]);
		}
		else
		{
			DBG("this uri[%s] is not replacable.", view_uri);
			calendar_list_next(list);
			continue;
		}

		if(ret != CALENDAR_ERROR_NONE)
		{
			calendar_list_destroy(list, true);
			ERR("record set fail");
			cal_db_util_end_trans(false);
			return ret;
		}

		// update
		ret = calendar_db_update_record(record);
		if(ret != CALENDAR_ERROR_NONE)
		{
			calendar_list_destroy(list, true);
			ERR("list get fail");
			cal_db_util_end_trans(false);
			return ret;
		}
		calendar_list_next(list);

		if (bulk < i)
		{
			bulk += (count / div + 1);
			cal_db_util_end_trans(true);
			sleep(1);
			ret = cal_db_util_begin_trans();
			if (ret != CALENDAR_ERROR_NONE)
			{
				calendar_list_destroy(list, true);
				ERR("Db failed");
				return CALENDAR_ERROR_DB_FAILED;
			}
		}
	}

	cal_db_util_end_trans(true);

	calendar_list_destroy(list, true);

	return ret;
}

API int calendar_db_replace_record(calendar_record_h record, int record_id)
{
	cal_record_s *temp=NULL ;
	int ret = CALENDAR_ERROR_NONE;

	RETV_IF(NULL == record, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(record_id < 0, CALENDAR_ERROR_INVALID_PARAMETER, "record_id(%d)", record_id);

	temp = (cal_record_s*)(record);

	cal_db_plugin_cb_s* plugin_cb = _cal_db_get_plugin(temp->type);
	RETV_IF(NULL == plugin_cb, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(NULL == plugin_cb->replace_record, CALENDAR_ERROR_NOT_PERMITTED, "Not permitted");

	ret = cal_db_util_begin_trans();
	RETVM_IF(CALENDAR_ERROR_NONE != ret,CALENDAR_ERROR_DB_FAILED, "Db failed");

	ret = plugin_cb->replace_record(record, record_id);

	if (CALENDAR_ERROR_NONE == ret)
		ret = cal_db_util_end_trans(true);
	else
		cal_db_util_end_trans(false);

	return ret;
}

API int calendar_db_replace_records(calendar_list_h list, int *ids, int count)
{
	int i;
	int ret = CALENDAR_ERROR_NONE;

	RETV_IF(NULL == list, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == ids, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(count <= 0, CALENDAR_ERROR_INVALID_PARAMETER, "count(%d)", count);

	ret = cal_db_util_begin_trans();
	if (ret != CALENDAR_ERROR_NONE)
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
			cal_db_util_end_trans(false);
			return ret;
		}

		cal_record_s *temp = (cal_record_s *)record;
		cal_db_plugin_cb_s* plugin_cb = _cal_db_get_plugin(temp->type);
		if (NULL == plugin_cb || NULL == plugin_cb->insert_record)
		{
			DBG("Not plugin");
			cal_db_util_end_trans(false);
			ret = CALENDAR_ERROR_NOT_PERMITTED;
			return ret;
		}
		ret = plugin_cb->replace_record(record, ids[i]);
		if (ret != CALENDAR_ERROR_NONE)
		{
			DBG("Failed to replace record");
			cal_db_util_end_trans(false);
			return ret;
		}
		DBG("insert with id(%d)", ids[i]);
		calendar_list_next(list);
	}
	cal_db_util_end_trans(true);
	return ret;
}

API int calendar_db_get_last_change_version(int* last_version)
{
	RETV_IF(NULL == last_version, CALENDAR_ERROR_INVALID_PARAMETER);
	*last_version = cal_db_util_get_transaction_ver();
	return CALENDAR_ERROR_NONE;
}

API int calendar_db_get_changes_exception_by_version(const char* view_uri, int original_event_id, int calendar_db_version, calendar_list_h* record_list)
{
	const char *query_cur_version = "SELECT ver FROM "CAL_TABLE_VERSION;
	int transaction_ver = 0;
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	sqlite3_stmt *stmt = NULL;
	int ret = 0;
	int is_deleted = 0;

	RETV_IF(NULL == view_uri, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == record_list, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(original_event_id <= 0, CALENDAR_ERROR_INVALID_PARAMETER);

	ret = cal_db_util_query_get_first_int_result(query_cur_version, NULL, &transaction_ver);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_db_util_query_get_first_int_result() failed");

	int schedule_type = 0;
	int record_type = 0;
	if (strcmp(view_uri,_calendar_event._uri) == 0) {
		schedule_type = CAL_SCH_TYPE_EVENT;
		record_type = CAL_RECORD_TYPE_EVENT;

	} else {
		ERR("Invalid parameter");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	snprintf(query, sizeof(query),
			"SELECT id, changed_ver, created_ver, is_deleted, calendar_id FROM %s "
			"WHERE changed_ver > %d AND changed_ver <= %d AND type = %d AND original_event_id = %d "
			"UNION "
			"SELECT schedule_id, deleted_ver, created_ver, 1, calendar_id FROM %s "
			"WHERE deleted_ver > %d AND schedule_type = %d AND original_event_id = %d ",
			CAL_TABLE_SCHEDULE,
			calendar_db_version, transaction_ver, schedule_type, original_event_id,
			CAL_TABLE_DELETED,
			calendar_db_version, record_type, original_event_id);
	SEC_DBG("query[%s]", query);

	ret = calendar_list_create(record_list);
	RETVM_IF(ret != CALENDAR_ERROR_NONE, ret, "calendar_list_create() Fail");

	stmt = cal_db_util_query_prepare(query);
	if (NULL == stmt)
	{
		ERR("cal_db_util_query_prepare() Fail");
		calendar_list_destroy(*record_list, true);
		*record_list = NULL;
		return CALENDAR_ERROR_DB_FAILED;
	}

	while(CAL_DB_ROW == cal_db_util_stmt_step(stmt))
	{
		calendar_record_h record;
		int id = 0, calendar_id = 0,type = 0;
		int ver = 0;
		int created_ver = 0;
		// stmt -> record
		ret = calendar_record_create(_calendar_updated_info._uri,&record);
		if(ret != CALENDAR_ERROR_NONE)
		{
			ERR("calendar_record_create() failed");
			calendar_list_destroy(*record_list, true);
			*record_list = NULL;
			sqlite3_finalize(stmt);
			return ret;
		}

		id = sqlite3_column_int(stmt, 0);
		ver = sqlite3_column_int(stmt, 1);
		created_ver = sqlite3_column_int(stmt, 2);
		is_deleted = sqlite3_column_int(stmt, 3);
		if (is_deleted == 1)
		{
			type = CALENDAR_RECORD_MODIFIED_STATUS_DELETED;
		}
		else if (created_ver != ver)
		{
			type = CALENDAR_RECORD_MODIFIED_STATUS_UPDATED;
		}
		else
		{
			type = CALENDAR_RECORD_MODIFIED_STATUS_INSERTED;
		}

		calendar_id = sqlite3_column_int(stmt, 4);
#if 0
		if (type == CALENDAR_RECORD_MODIFIED_STATUS_DELETED && created_ver > calendar_db_version)
		{
			calendar_record_destroy(record, true);
			continue;
		}
#endif

		cal_record_set_int(record,_calendar_updated_info.id,id);
		cal_record_set_int(record,_calendar_updated_info.calendar_book_id,calendar_id);
		cal_record_set_int(record,_calendar_updated_info.modified_status,type);
		cal_record_set_int(record,_calendar_updated_info.version,ver);

		ret = calendar_list_add(*record_list,record);
		if(ret != CALENDAR_ERROR_NONE)
		{
			calendar_list_destroy(*record_list, true);
			*record_list = NULL;
			calendar_record_destroy(record, true);
			sqlite3_finalize(stmt);
			return ret;
		}
	}

	//*current_calendar_db_version = transaction_ver;
	sqlite3_finalize(stmt);

	calendar_list_first(*record_list);

	return CALENDAR_ERROR_NONE;
}

int cal_db_append_string(char **dst, char *src)
{
	if (NULL == dst || NULL == src)
	{
		ERR("Invalid parameter");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	int len_src = strlen(src);
	if (len_src == 0)
	{
		DBG("src len is 0");
		return CALENDAR_ERROR_NONE;
	}
	if (NULL == *dst)
	{
		*dst = strdup(src);
		if (*dst == NULL)
		{
			ERR("strdup fail");
			return CALENDAR_ERROR_OUT_OF_MEMORY;
		}
		return CALENDAR_ERROR_NONE;
	}
	int len_dst = strlen(*dst);
	char *tmp = *dst;
	tmp = (char *)realloc(tmp, len_dst + len_src + 2);
	if (tmp == NULL)
	{
		ERR("strdup fail");
		return CALENDAR_ERROR_OUT_OF_MEMORY;
	}
	*dst = tmp;
	strcat(*dst, " ");
	strcat(*dst, src);
	return CALENDAR_ERROR_NONE;
}

