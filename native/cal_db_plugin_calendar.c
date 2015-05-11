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

#include "cal_db.h"
#include "cal_db_util.h"
#include "cal_db_query.h"
#include "cal_access_control.h"

#ifdef CAL_IPC_SERVER
#include "cal_server_calendar_delete.h"
#endif
#include "cal_access_control.h"

/*
 * db plugin function
 */
static int _cal_db_calendar_insert_record(calendar_record_h record, int* id);
static int _cal_db_calendar_get_record(int id, calendar_record_h* out_record);
static int _cal_db_calendar_update_record(calendar_record_h record);
static int _cal_db_calendar_delete_record(int id);
static int _cal_db_calendar_get_all_records(int offset, int limit, calendar_list_h* out_list);
static int _cal_db_calendar_get_records_with_query(calendar_query_h query, int offset, int limit, calendar_list_h* out_list);
static int _cal_db_calendar_insert_records(const calendar_list_h list, int** ids);
static int _cal_db_calendar_update_records(const calendar_list_h list);
static int _cal_db_calendar_delete_records(int ids[], int count);
static int _cal_db_calendar_get_count(int *out_count);
static int _cal_db_calendar_get_count_with_query(calendar_query_h query, int *out_count);
static int _cal_db_calendar_replace_record(calendar_record_h record, int id);
static int _cal_db_calendar_replace_records(const calendar_list_h list, int ids[], int count);

/*
 * static function
 */
static void _cal_db_calendar_get_stmt(sqlite3_stmt *stmt,calendar_record_h record);
static void _cal_db_calendar_get_property_stmt(sqlite3_stmt *stmt,
		unsigned int property, int stmt_count, calendar_record_h record);
static void _cal_db_calendar_get_projection_stmt(sqlite3_stmt *stmt,
		const unsigned int *projection, const int projection_count,
		calendar_record_h record);
static int _cal_db_calendar_update_projection(calendar_record_h record);

cal_db_plugin_cb_s cal_db_calendar_plugin_cb = {
	.is_query_only=false,
	.insert_record=_cal_db_calendar_insert_record,
	.get_record=_cal_db_calendar_get_record,
	.update_record=_cal_db_calendar_update_record,
	.delete_record=_cal_db_calendar_delete_record,
	.get_all_records=_cal_db_calendar_get_all_records,
	.get_records_with_query=_cal_db_calendar_get_records_with_query,
	.insert_records=_cal_db_calendar_insert_records,
	.update_records=_cal_db_calendar_update_records,
	.delete_records=_cal_db_calendar_delete_records,
	.get_count=_cal_db_calendar_get_count,
	.get_count_with_query=_cal_db_calendar_get_count_with_query,
	.replace_record = _cal_db_calendar_replace_record,
	.replace_records = _cal_db_calendar_replace_records
};

static bool _cal_db_calendar_check_value_validation(cal_calendar_s* calendar)
{
	RETVM_IF(NULL == calendar, CALENDAR_ERROR_INVALID_PARAMETER, "calendar is NULL");

	switch (calendar->store_type) {
	case CALENDAR_BOOK_TYPE_NONE:
	case CALENDAR_BOOK_TYPE_EVENT:
	case CALENDAR_BOOK_TYPE_TODO:
		return true;

	default:
		ERR("store type is invalid(%d)", calendar->store_type);
		return false;
	}
	return true;
}

static int _cal_db_calendar_insert_record(calendar_record_h record, int* id)
{
	char query[CAL_DB_SQL_MAX_LEN];
	int index = 0;
	sqlite3_stmt *stmt;
	cal_calendar_s* calendar =  (cal_calendar_s*)(record);
	cal_db_util_error_e dbret = CAL_DB_OK;
	char *client_label = NULL;

	// !! error check
	RETV_IF(NULL == calendar, CALENDAR_ERROR_INVALID_PARAMETER);

	if (false == _cal_db_calendar_check_value_validation(calendar)) {
		ERR("cal_db_calendar_check_value_validation() is failed");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	client_label = cal_access_control_get_label();

	sprintf(query,"INSERT INTO %s(uid,updated,name,description,"
			"color,location,"
			"visibility,"
			"sync_event,"
			"account_id,store_type,sync_data1,sync_data2,sync_data3,sync_data4,"
			"mode, owner_label) "
			"VALUES(?, %ld, ?, ?, ?, ?, %d, %d, %d, %d"
			", ?, ?, ?, ?"
			", %d, ?)",
			CAL_TABLE_CALENDAR,
			//calendar->uid,
			calendar->updated,
			//calendar->name,
			//calendar->description,
			//calendar->color,
			//calendar->location,
			calendar->visibility,
			calendar->sync_event,
			calendar->account_id,
			calendar->store_type,
			calendar->mode);

	stmt = cal_db_util_query_prepare(query);
	if (NULL == stmt) {
		ERR("cal_db_util_query_prepare() Failed");
		CAL_FREE(client_label);
		return CALENDAR_ERROR_DB_FAILED;
	}

	if (calendar->uid)
		cal_db_util_stmt_bind_text(stmt, 1, calendar->uid);

	if (calendar->name)
		cal_db_util_stmt_bind_text(stmt, 2, calendar->name);

	if (calendar->description)
		cal_db_util_stmt_bind_text(stmt, 3, calendar->description);

	if (calendar->color)
		cal_db_util_stmt_bind_text(stmt, 4, calendar->color);

	if (calendar->location)
		cal_db_util_stmt_bind_text(stmt, 5, calendar->location);

	// sync1~4
	if (calendar->sync_data1)
		cal_db_util_stmt_bind_text(stmt, 6, calendar->sync_data1);
	if (calendar->sync_data2)
		cal_db_util_stmt_bind_text(stmt, 7, calendar->sync_data2);
	if (calendar->sync_data3)
		cal_db_util_stmt_bind_text(stmt, 8, calendar->sync_data3);
	if (calendar->sync_data4)
		cal_db_util_stmt_bind_text(stmt, 9, calendar->sync_data4);

	if (client_label)
		cal_db_util_stmt_bind_text(stmt, 10, client_label);

	dbret = cal_db_util_stmt_step(stmt);

	if (CAL_DB_DONE != dbret) {
		sqlite3_finalize(stmt);
		ERR("cal_db_util_stmt_step() Failed(%d)", dbret);
		CAL_FREE(client_label);
		switch (dbret)
		{
		case CAL_DB_ERROR_NO_SPACE:
			return CALENDAR_ERROR_FILE_NO_SPACE;
		default:
			return CALENDAR_ERROR_DB_FAILED;
		}
	}

	index = cal_db_util_last_insert_id();
	sqlite3_finalize(stmt);
	CAL_FREE(client_label);
	// access control
	cal_access_control_reset();

	//cal_record_set_int(record, _calendar_book.id,index);
	if (id)
	{
		*id = index;
	}

	cal_db_util_notify(CAL_NOTI_TYPE_CALENDAR);

	return CALENDAR_ERROR_NONE;
}

static int _cal_db_calendar_get_record(int id, calendar_record_h* out_record)
{
	char query[CAL_DB_SQL_MAX_LEN];
	sqlite3_stmt *stmt = NULL;
	cal_db_util_error_e dbret = CAL_DB_OK;
	int ret = 0;

	ret = calendar_record_create(_calendar_book._uri ,out_record);
	if (ret != CALENDAR_ERROR_NONE) {
		ERR("record create fail");
		return CALENDAR_ERROR_OUT_OF_MEMORY;
	}

	snprintf(query, sizeof(query), "SELECT * FROM %s WHERE id=%d "
			"AND (deleted = 0)",
			CAL_TABLE_CALENDAR,	id);
	stmt = cal_db_util_query_prepare(query);
	RETVM_IF(NULL == stmt, CALENDAR_ERROR_DB_FAILED, "cal_db_util_query_prepare() Failed");

	dbret = cal_db_util_stmt_step(stmt);
	if (dbret != CAL_DB_ROW) {
		ERR("Failed to step stmt(%d)", dbret);
		sqlite3_finalize(stmt);
		switch (dbret) {
		case CAL_DB_DONE:
			return CALENDAR_ERROR_DB_RECORD_NOT_FOUND;
		case CAL_DB_ERROR_NO_SPACE:
			return CALENDAR_ERROR_FILE_NO_SPACE;
		default:
			return CALENDAR_ERROR_DB_FAILED;
		}
	}

	_cal_db_calendar_get_stmt(stmt, *out_record);

	sqlite3_finalize(stmt);
	stmt = NULL;

	return CALENDAR_ERROR_NONE;
}

static int _cal_db_calendar_update_record(calendar_record_h record)
{
	//int rc = -1;
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	sqlite3_stmt *stmt = NULL;
	cal_calendar_s* calendar =  (cal_calendar_s*)(record);
	cal_db_util_error_e dbret = CAL_DB_OK;

	RETV_IF(NULL == calendar, CALENDAR_ERROR_INVALID_PARAMETER);

	if (false == _cal_db_calendar_check_value_validation(calendar)) {
		ERR("cal_db_calendar_check_value_validation() is failed");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	if (calendar->common.properties_flags != NULL) {
		return _cal_db_calendar_update_projection(record);
	}

	snprintf(query, sizeof(query), "UPDATE %s SET "
			"updated = %ld,"
			"name = ?,"
			"description = ?,"
			"color = ?,"
			"location = ?,"
			"visibility = %d,"
			"sync_event = %d,"
			"account_id = %d,"
			"store_type = %d, "
			"sync_data1 = ?, "
			"sync_data2 = ?, "
			"sync_data3 = ?, "
			"sync_data4 = ?,"
			"mode = %d "
			"WHERE id = %d",
			CAL_TABLE_CALENDAR,
			calendar->updated,
			calendar->visibility,
			calendar->sync_event,
			calendar->account_id,
			calendar->store_type,
			calendar->mode,
			calendar->index);

	stmt = cal_db_util_query_prepare(query);
	RETVM_IF(NULL == stmt, CALENDAR_ERROR_DB_FAILED, "cal_db_util_query_prepare() Failed");

	if (calendar->name)
		cal_db_util_stmt_bind_text(stmt, 1, calendar->name);

	if (calendar->description)
		cal_db_util_stmt_bind_text(stmt, 2, calendar->description);

	if (calendar->color)
		cal_db_util_stmt_bind_text(stmt, 3, calendar->color);

	if (calendar->location)
		cal_db_util_stmt_bind_text(stmt, 4, calendar->location);

	// sync_data1~4
	if (calendar->sync_data1)
		cal_db_util_stmt_bind_text(stmt, 5, calendar->sync_data1);
	if (calendar->sync_data2)
		cal_db_util_stmt_bind_text(stmt, 6, calendar->sync_data2);
	if (calendar->sync_data3)
		cal_db_util_stmt_bind_text(stmt, 7, calendar->sync_data3);
	if (calendar->sync_data4)
		cal_db_util_stmt_bind_text(stmt, 8, calendar->sync_data4);

	dbret = cal_db_util_stmt_step(stmt);
	if (CAL_DB_DONE != dbret) {
		sqlite3_finalize(stmt);
		ERR("cal_db_util_stmt_step() Failed(%d)", dbret);
		switch (dbret) {
		case CAL_DB_ERROR_NO_SPACE:
			return CALENDAR_ERROR_FILE_NO_SPACE;
		default:
			return CALENDAR_ERROR_DB_FAILED;
		}
	}

	sqlite3_finalize(stmt);

	cal_db_util_notify(CAL_NOTI_TYPE_CALENDAR);
	return CALENDAR_ERROR_NONE;
}

static int _cal_db_calendar_delete_record(int id)
{
	int ret = CALENDAR_ERROR_NONE;
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	cal_db_util_error_e dbret = CAL_DB_OK;
	int calendar_book_id = -1;

	snprintf(query, sizeof(query), "SELECT id FROM %s WHERE id = %d",
			CAL_TABLE_CALENDAR, id);
	ret = cal_db_util_query_get_first_int_result(query, NULL, &calendar_book_id);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_query_get_first_int_result(%d) failed", ret);
		return ret;
	}

#ifdef CAL_IPC_SERVER
	int count = 0;
	int count2 = 0;
	// get instance count
	snprintf(query, sizeof(query), "select count(*) from %s",
			CAL_TABLE_NORMAL_INSTANCE);
	ret = cal_db_util_query_get_first_int_result(query, NULL, &count);
	if (CALENDAR_ERROR_NONE != ret)
	{
		ERR("cal_db_util_query_get_first_int_result() failed");
		return ret;
	}

	snprintf(query, sizeof(query), "select count(*) from %s",
			CAL_TABLE_ALLDAY_INSTANCE);
	ret = cal_db_util_query_get_first_int_result(query,NULL, &count2);
	if (CALENDAR_ERROR_NONE != ret)
	{
		ERR("cal_db_util_query_get_first_int_result() failed");
		return ret;
	}

	count += count2;

	if(count > 1000)
	{
		snprintf(query, sizeof(query), "UPDATE %s SET deleted = 1 WHERE id = %d",
				CAL_TABLE_CALENDAR, id);
		dbret = cal_db_util_query_exec(query);
		if (CAL_DB_OK != dbret)
		{
			ERR("cal_db_util_query_exec() Failed(%d)", dbret);
			switch (dbret)
			{
			case CAL_DB_ERROR_NO_SPACE:
				return CALENDAR_ERROR_FILE_NO_SPACE;
			default:
				return CALENDAR_ERROR_DB_FAILED;
			}
		}
		cal_server_calendar_delete_start();
	}
	else
	{
#endif
		snprintf(query, sizeof(query), "DELETE FROM %s WHERE id = %d",
				CAL_TABLE_CALENDAR, id);
		dbret = cal_db_util_query_exec(query);
		if (CAL_DB_OK != dbret)
		{
			ERR("cal_db_util_query_exec() Failed(%d)", dbret);
			switch (dbret)
			{
			case CAL_DB_ERROR_NO_SPACE:
				return CALENDAR_ERROR_FILE_NO_SPACE;
			default:
				return CALENDAR_ERROR_DB_FAILED;
			}
		}

		snprintf(query, sizeof(query), "DELETE FROM %s WHERE calendar_id = %d",
				CAL_TABLE_SCHEDULE, id);
		dbret = cal_db_util_query_exec(query);
		if (CAL_DB_OK != dbret) {
			ERR("cal_db_util_query_exec() Failed(%d)", dbret);
			switch (dbret)
			{
			case CAL_DB_ERROR_NO_SPACE:
				return CALENDAR_ERROR_FILE_NO_SPACE;
			default:
				return CALENDAR_ERROR_DB_FAILED;
			}
		}
#ifdef CAL_IPC_SERVER
	}
#endif

	snprintf(query, sizeof(query), "DELETE FROM %s WHERE calendar_id = %d",
			CAL_TABLE_DELETED, id);
	dbret = cal_db_util_query_exec(query);
	if (CAL_DB_OK != dbret) {
		ERR("cal_db_util_query_exec() Failed(%d)", dbret);
		switch (dbret)
		{
		case CAL_DB_ERROR_NO_SPACE:
			return CALENDAR_ERROR_FILE_NO_SPACE;
		default:
			return CALENDAR_ERROR_DB_FAILED;
		}
	}

	// access control
	cal_access_control_reset();

	cal_db_util_notify(CAL_NOTI_TYPE_CALENDAR);
	return CALENDAR_ERROR_NONE;
}

static int _cal_db_calendar_replace_record(calendar_record_h record, int id)
{
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	sqlite3_stmt *stmt = NULL;
	cal_calendar_s* calendar =  (cal_calendar_s*)(record);
	cal_db_util_error_e dbret = CAL_DB_OK;

	RETV_IF(NULL == calendar, CALENDAR_ERROR_INVALID_PARAMETER);

	if (false == _cal_db_calendar_check_value_validation(calendar)) {
		ERR("cal_db_calendar_check_value_validation() is failed");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	calendar->index = id;
	if (calendar->common.properties_flags != NULL) {
		return _cal_db_calendar_update_projection(record);
	}

	snprintf(query, sizeof(query), "UPDATE %s SET "
			"updated = %ld,"
			"name = ?,"
			"description = ?,"
			"color = ?,"
			"location = ?,"
			"visibility = %d,"
			"sync_event = %d,"
			"account_id = %d,"
			"store_type = %d, "
			"sync_data1 = ?, "
			"sync_data2 = ?, "
			"sync_data3 = ?, "
			"sync_data4 = ?, "
			"mode = %d "
			"WHERE id = %d",
			CAL_TABLE_CALENDAR,
			calendar->updated,
			calendar->visibility,
			calendar->sync_event,
			calendar->account_id,
			calendar->store_type,
			calendar->mode,
			id);

	stmt = cal_db_util_query_prepare(query);
	RETVM_IF(NULL == stmt, CALENDAR_ERROR_DB_FAILED, "cal_db_util_query_prepare() Failed");

	if (calendar->name)
		cal_db_util_stmt_bind_text(stmt, 1, calendar->name);

	if (calendar->description)
		cal_db_util_stmt_bind_text(stmt, 2, calendar->description);

	if (calendar->color)
		cal_db_util_stmt_bind_text(stmt, 3, calendar->color);

	if (calendar->location)
		cal_db_util_stmt_bind_text(stmt, 4, calendar->location);

	// sync_data1~4
	if (calendar->sync_data1)
		cal_db_util_stmt_bind_text(stmt, 5, calendar->sync_data1);
	if (calendar->sync_data2)
		cal_db_util_stmt_bind_text(stmt, 6, calendar->sync_data2);
	if (calendar->sync_data3)
		cal_db_util_stmt_bind_text(stmt, 7, calendar->sync_data3);
	if (calendar->sync_data4)
		cal_db_util_stmt_bind_text(stmt, 8, calendar->sync_data4);

	dbret = cal_db_util_stmt_step(stmt);
	if (CAL_DB_DONE != dbret) {
		sqlite3_finalize(stmt);
		ERR("cal_db_util_stmt_step() Failed(%d)", dbret);
		switch (dbret) {
		case CAL_DB_ERROR_NO_SPACE:
			return CALENDAR_ERROR_FILE_NO_SPACE;
		default:
			return CALENDAR_ERROR_DB_FAILED;
		}
	}

	sqlite3_finalize(stmt);

	cal_db_util_notify(CAL_NOTI_TYPE_CALENDAR);
	return CALENDAR_ERROR_NONE;
}

static int _cal_db_calendar_get_all_records(int offset, int limit, calendar_list_h* out_list)
{
	int ret = CALENDAR_ERROR_NONE;
	char offsetquery[CAL_DB_SQL_MAX_LEN] = {0};
	char limitquery[CAL_DB_SQL_MAX_LEN] = {0};
	sqlite3_stmt *stmt = NULL;

	RETV_IF(NULL == out_list, CALENDAR_ERROR_INVALID_PARAMETER);

	ret = calendar_list_create(out_list);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "calendar_list_create() Fail(%d)", ret);

	if (offset > 0)
		snprintf(offsetquery, sizeof(offsetquery), "OFFSET %d", offset);
	if (limit > 0)
		snprintf(limitquery, sizeof(limitquery), "LIMIT %d", limit);

	char *query_str = NULL;
	cal_db_append_string(&query_str, "SELECT * FROM");
	cal_db_append_string(&query_str, CAL_TABLE_CALENDAR);
	cal_db_append_string(&query_str, "WHERE deleted = 0");
	cal_db_append_string(&query_str, limitquery);
	cal_db_append_string(&query_str, offsetquery);

	stmt = cal_db_util_query_prepare(query_str);
	if (NULL == stmt)	{
		ERR("cal_db_util_query_prepare() Failed");
		calendar_list_destroy(*out_list, true);
		*out_list = NULL;
		CAL_FREE(query_str);
		return CALENDAR_ERROR_DB_FAILED;
	}

	while(CAL_DB_ROW == cal_db_util_stmt_step(stmt)) {
		calendar_record_h record;
		// stmt -> record
		ret = calendar_record_create(_calendar_book._uri,&record);
		if(ret != CALENDAR_ERROR_NONE) {
			calendar_list_destroy(*out_list, true);
			*out_list = NULL;
			sqlite3_finalize(stmt);
			CAL_FREE(query_str);
			return ret;
		}
		_cal_db_calendar_get_stmt(stmt,record);

		ret = calendar_list_add(*out_list,record);
		if(ret != CALENDAR_ERROR_NONE) {
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

static int _cal_db_calendar_get_records_with_query(calendar_query_h query, int offset, int limit, calendar_list_h* out_list)
{
	cal_query_s *que = NULL;
	int i = 0;
	int ret = CALENDAR_ERROR_NONE;
	char *condition = NULL;
	GSList *bind_text = NULL, *cursor = NULL;
	sqlite3_stmt *stmt = NULL;

	que = (cal_query_s *)query;

	// make filter
	if (que->filter) {
		ret = cal_db_query_create_condition(query, &condition, &bind_text);
		if (ret != CALENDAR_ERROR_NONE) {
			ERR("filter create fail");
			return ret;
		}
	}

	// make projection
	char *projection = NULL;
	ret = cal_db_query_create_projection(query, &projection);

	char *query_str = NULL;
	// query - projection
	if (projection) {
		cal_db_append_string(&query_str, "SELECT");
		cal_db_append_string(&query_str, projection);
		cal_db_append_string(&query_str, "FROM");
		cal_db_append_string(&query_str, CAL_TABLE_CALENDAR);
		CAL_FREE(projection);
	}
	else {
		cal_db_append_string(&query_str, "SELECT * FROM");
		cal_db_append_string(&query_str, CAL_TABLE_CALENDAR);
	}

	// query - condition
	if (condition) {
		cal_db_append_string(&query_str,  "WHERE (");
		cal_db_append_string(&query_str, condition);
		cal_db_append_string(&query_str, ") AND (deleted = 0)");
	}

	// ORDER
	char *order = NULL;
	ret = cal_db_query_create_order(query, condition, &order);
	if (order) {
		cal_db_append_string(&query_str, order);
		CAL_FREE(order);
	}
	CAL_FREE(condition);

	// limit, offset
	char buf[32] = {0};
	if (limit > 0) {
		snprintf(buf, sizeof(buf), "LIMIT %d", limit);
		cal_db_append_string(&query_str, buf);

		if (offset > 0) {
			snprintf(buf, sizeof(buf), "OFFSET %d", offset);
			cal_db_append_string(&query_str, buf);
		}
	}

	// query
	stmt = cal_db_util_query_prepare(query_str);
	if (NULL == stmt) {
		if (bind_text) {
			g_slist_free_full(bind_text, free);
			bind_text = NULL;
		}
		CAL_FREE(query_str);
		ERR("cal_db_util_query_prepare() Failed");
		return CALENDAR_ERROR_DB_FAILED;
	}
	DBG("%s",query_str);

	// bind text
	if (bind_text) {
		g_slist_length(bind_text);
		for (cursor=bind_text, i=1; cursor;cursor=cursor->next, i++)
			cal_db_util_stmt_bind_text(stmt, i, cursor->data);
	}

	ret = calendar_list_create(out_list);
	if (ret != CALENDAR_ERROR_NONE) {
		if (bind_text) {
			g_slist_free_full(bind_text, free);
			bind_text = NULL;
		}
		ERR("calendar_list_create() Failed");
		sqlite3_finalize(stmt);
		CAL_FREE(query_str);
		return ret;
	}

	while(CAL_DB_ROW == cal_db_util_stmt_step(stmt)) {
		calendar_record_h record;
		// stmt -> record
		ret = calendar_record_create(_calendar_book._uri,&record);
		if(ret != CALENDAR_ERROR_NONE) {
			calendar_list_destroy(*out_list, true);
			*out_list = NULL;

			if (bind_text) {
				g_slist_free_full(bind_text, free);
				bind_text = NULL;
			}
			sqlite3_finalize(stmt);
			CAL_FREE(query_str);
			return ret;
		}

		if (que->projection_count > 0) {
			cal_record_set_projection(record,
					que->projection, que->projection_count, que->property_count);

			_cal_db_calendar_get_projection_stmt(stmt,
					que->projection, que->projection_count,
					record);
		}
		else {
			_cal_db_calendar_get_stmt(stmt,record);
		}

		ret = calendar_list_add(*out_list,record);
		if(ret != CALENDAR_ERROR_NONE) {
			calendar_list_destroy(*out_list, true);
			*out_list = NULL;
			calendar_record_destroy(record, true);

			if (bind_text) {
				g_slist_free_full(bind_text, free);
				bind_text = NULL;
			}
			sqlite3_finalize(stmt);
			CAL_FREE(query_str);
			return ret;
		}
	}

	if (bind_text) {
		g_slist_free_full(bind_text, free);
		bind_text = NULL;
	}
	sqlite3_finalize(stmt);
	CAL_FREE(query_str);

	return CALENDAR_ERROR_NONE;
}

static int _cal_db_calendar_insert_records(const calendar_list_h list, int** ids)
{
	calendar_record_h record;
	int ret = 0;
	int count = 0;
	int i=0;
	int *id = NULL;

	ret = calendar_list_get_count(list, &count);
	if (ret != CALENDAR_ERROR_NONE) {
		ERR("list get error");
		return ret;
	}

	id = calloc(1, sizeof(int)*count);

	RETVM_IF(NULL == id, CALENDAR_ERROR_OUT_OF_MEMORY, "calloc fail");

	ret = calendar_list_first(list);
	if (ret != CALENDAR_ERROR_NONE) {
		ERR("list first error");
		CAL_FREE(id);
		return ret;
	}

	do {
		if(calendar_list_get_current_record_p(list, &record) == CALENDAR_ERROR_NONE) {
			if(_cal_db_calendar_insert_record(record, &id[i]) != CALENDAR_ERROR_NONE) {
				ERR("db insert error");
				CAL_FREE(id);
				return CALENDAR_ERROR_DB_FAILED;
			}
		}
		i++;
	} while(calendar_list_next(list) != CALENDAR_ERROR_NO_DATA);

	if(ids) {
		*ids = id;
	}
	else {
		CAL_FREE(id);
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_db_calendar_update_records(const calendar_list_h list)
{
	calendar_record_h record;
	int ret = 0;

	ret = calendar_list_first(list);
	if (ret != CALENDAR_ERROR_NONE) {
		ERR("list first error");
		return ret;
	}

	do {
		if(calendar_list_get_current_record_p(list, &record) == CALENDAR_ERROR_NONE) {
			if(_cal_db_calendar_update_record(record) != CALENDAR_ERROR_NONE) {
				ERR("db insert error");
				return CALENDAR_ERROR_DB_FAILED;
			}
		}
	} while(calendar_list_next(list) != CALENDAR_ERROR_NO_DATA);

	return CALENDAR_ERROR_NONE;
}

static int _cal_db_calendar_delete_records(int ids[], int count)
{
	int i=0;

	for(i=0;i<count;i++) {
		if (_cal_db_calendar_delete_record(ids[i]) != CALENDAR_ERROR_NONE) {
			ERR("delete failed");
			return CALENDAR_ERROR_DB_FAILED;
		}
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_db_calendar_replace_records(const calendar_list_h list, int ids[], int count)
{
	calendar_record_h record;
	int i = 0;
	int ret = 0;

	if (NULL == list) {
		ERR("Invalid argument: list is NULL");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	ret = calendar_list_first(list);
	if (ret != CALENDAR_ERROR_NONE) {
		ERR("list first error");
		return ret;
	}

	for (i = 0; i < count; i++) {
		if(calendar_list_get_current_record_p(list, &record) == CALENDAR_ERROR_NONE) {
			if(_cal_db_calendar_replace_record(record, ids[i]) != CALENDAR_ERROR_NONE) {
				ERR("db insert error");
				return CALENDAR_ERROR_DB_FAILED;
			}
		}
		if (CALENDAR_ERROR_NO_DATA != calendar_list_next(list)) {
			break;
		}
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_db_calendar_get_count(int *out_count)
{
	RETV_IF(NULL == out_count, CALENDAR_ERROR_INVALID_PARAMETER);

	char *query_str = NULL;
	cal_db_append_string(&query_str, "SELECT count(*) FROM");
	cal_db_append_string(&query_str, CAL_TABLE_CALENDAR);
	cal_db_append_string(&query_str, "WHERE deleted = 0");

	int ret = 0;
	int count = 0;
	ret = cal_db_util_query_get_first_int_result(query_str, NULL, &count);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_query_get_first_int_result() failed");
		CAL_FREE(query_str);
		return ret;
	}
	DBG("count(%d) str[%s]", count, query_str);
	CAL_FREE(query_str);

	*out_count = count;
	return CALENDAR_ERROR_NONE;
}

static int _cal_db_calendar_get_count_with_query(calendar_query_h query, int *out_count)
{
	cal_query_s *que = NULL;
	int ret = CALENDAR_ERROR_NONE;
	char *condition = NULL;
	char *table_name;
	int count = 0;
	GSList *bind_text = NULL;

	que = (cal_query_s *)query;

	if (0 == strcmp(que->view_uri, CALENDAR_VIEW_CALENDAR)) {
		table_name = SAFE_STRDUP(CAL_TABLE_CALENDAR);
	}
	else {
		ERR("uri(%s) not support get records with query",que->view_uri);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	// make filter
	if (que->filter) {
		ret = cal_db_query_create_condition(query, &condition, &bind_text);
		if (ret != CALENDAR_ERROR_NONE) {
			CAL_FREE(table_name);
			ERR("filter create fail");
			return ret;
		}
	}

	char *query_str = NULL;
	// query - select from
	cal_db_append_string(&query_str, "SELECT count(*) FROM");
	cal_db_append_string(&query_str, table_name);
	CAL_FREE(table_name);

	// query - condition
	if (condition) {
		cal_db_append_string(&query_str, "WHERE (");
		cal_db_append_string(&query_str, condition);
		cal_db_append_string(&query_str, ") AND (deleted = 0)");
		CAL_FREE(condition);
	}

	// query
	ret = cal_db_util_query_get_first_int_result(query_str, bind_text, &count);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_query_get_first_int_result() failed");
		if (bind_text) {
			g_slist_free_full(bind_text, free);
			bind_text = NULL;
		}
		CAL_FREE(query_str);
		return ret;
	}
	DBG("count(%d) str[%s]", count, query_str);

	if (out_count)
		*out_count = count;

	if (bind_text) {
		g_slist_free_full(bind_text, free);
		bind_text = NULL;
	}
	CAL_FREE(query_str);
	return CALENDAR_ERROR_NONE;
}

static void _cal_db_calendar_get_stmt(sqlite3_stmt *stmt,calendar_record_h record)
{
	cal_calendar_s* calendar =  (cal_calendar_s*)(record);
	int count = 0;
	const unsigned char *temp;

	calendar->index = sqlite3_column_int(stmt, count++);

	temp = sqlite3_column_text(stmt, count++);
	calendar->uid = SAFE_STRDUP(temp);

	calendar->updated = sqlite3_column_int(stmt, count++);

	temp = sqlite3_column_text(stmt, count++);
	calendar->name = SAFE_STRDUP(temp);

	temp = sqlite3_column_text(stmt, count++);
	calendar->description = SAFE_STRDUP(temp);

	temp = sqlite3_column_text(stmt, count++);
	calendar->color = SAFE_STRDUP(temp);

	temp = sqlite3_column_text(stmt, count++);
	calendar->location = SAFE_STRDUP(temp);

	calendar->visibility = sqlite3_column_int(stmt, count++);
	calendar->sync_event = sqlite3_column_int(stmt, count++);
	calendar->is_deleted = sqlite3_column_int(stmt, count++);
	calendar->account_id = sqlite3_column_int(stmt, count++);
	calendar->store_type = sqlite3_column_int(stmt, count++);

	//sync_data1~4
	temp = sqlite3_column_text(stmt, count++);
	calendar->sync_data1 = SAFE_STRDUP(temp);
	temp = sqlite3_column_text(stmt, count++);
	calendar->sync_data2 = SAFE_STRDUP(temp);
	temp = sqlite3_column_text(stmt, count++);
	calendar->sync_data3 = SAFE_STRDUP(temp);
	temp = sqlite3_column_text(stmt, count++);
	calendar->sync_data4 = SAFE_STRDUP(temp);

	//deleted
	sqlite3_column_int(stmt, count++);

	// mode
	calendar->mode = sqlite3_column_int(stmt, count++);

	// owner_label
	sqlite3_column_text(stmt, count++);
}

static void _cal_db_calendar_get_property_stmt(sqlite3_stmt *stmt,
		unsigned int property, int stmt_count, calendar_record_h record)
{
	cal_calendar_s* calendar =  (cal_calendar_s*)(record);
	const unsigned char *temp;

	switch(property) {
	case CAL_PROPERTY_CALENDAR_ID:
		calendar->index = sqlite3_column_int(stmt, stmt_count);
		break;
	case CAL_PROPERTY_CALENDAR_UID:
		temp = sqlite3_column_text(stmt, stmt_count);
		calendar->uid = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_CALENDAR_NAME:
		temp = sqlite3_column_text(stmt, stmt_count);
		calendar->name = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_CALENDAR_DESCRIPTION:
		temp = sqlite3_column_text(stmt, stmt_count);
		calendar->description = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_CALENDAR_COLOR:
		temp = sqlite3_column_text(stmt, stmt_count);
		calendar->color = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_CALENDAR_LOCATION:
		temp = sqlite3_column_text(stmt, stmt_count);
		calendar->location = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_CALENDAR_VISIBILITY:
		calendar->visibility = sqlite3_column_int(stmt, stmt_count);
		break;
	case CAL_PROPERTY_CALENDAR_SYNC_EVENT:
		calendar->sync_event = sqlite3_column_int(stmt, stmt_count);
		break;
	case CAL_PROPERTY_CALENDAR_ACCOUNT_ID:
		calendar->account_id = sqlite3_column_int(stmt, stmt_count);
		break;
	case CAL_PROPERTY_CALENDAR_STORE_TYPE:
		calendar->store_type = sqlite3_column_int(stmt, stmt_count);
		break;
	case CAL_PROPERTY_CALENDAR_SYNC_DATA1:
		temp = sqlite3_column_text(stmt, stmt_count);
		calendar->sync_data1 = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_CALENDAR_SYNC_DATA2:
		temp = sqlite3_column_text(stmt, stmt_count);
		calendar->sync_data1 = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_CALENDAR_SYNC_DATA3:
		temp = sqlite3_column_text(stmt, stmt_count);
		calendar->sync_data1 = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_CALENDAR_SYNC_DATA4:
		temp = sqlite3_column_text(stmt, stmt_count);
		calendar->sync_data1 = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_CALENDAR_MODE:
		calendar->mode = sqlite3_column_int(stmt, stmt_count);
		break;
	default:
		break;
	}

	return;
}

static void _cal_db_calendar_get_projection_stmt(sqlite3_stmt *stmt,
		const unsigned int *projection, const int projection_count,
		calendar_record_h record)
{
	int i=0;

	for(i=0;i<projection_count;i++)
		_cal_db_calendar_get_property_stmt(stmt,projection[i],i,record);
}

static int _cal_db_calendar_update_projection(calendar_record_h record)
{
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	sqlite3_stmt *stmt = NULL;
	cal_calendar_s* calendar =  (cal_calendar_s*)(record);
	cal_db_util_error_e dbret = CAL_DB_OK;
	int ret = CALENDAR_ERROR_NONE;
	char* set = NULL;
	GSList *bind_text = NULL;
	GSList *cursor = NULL;

	ret = cal_db_query_create_projection_update_set(record,&set,&bind_text);
	RETV_IF(CALENDAR_ERROR_NONE != ret, ret);

	snprintf(query, sizeof(query), "UPDATE %s SET %s "
			"WHERE id = %d",
			CAL_TABLE_CALENDAR,set,
			calendar->index);

	stmt = cal_db_util_query_prepare(query);
	if (NULL == stmt) {
		ERR("cal_db_util_query_prepare() Failed");
		CAL_FREE(set);
		if(bind_text) {
			g_slist_free_full(bind_text, free);
			bind_text = NULL;
		}
		return CALENDAR_ERROR_DB_FAILED;
	}

	// bind
	if (bind_text) {
		int i = 0;
		for (cursor=bind_text, i=1; cursor;cursor=cursor->next, i++)
			cal_db_util_stmt_bind_text(stmt, i, cursor->data);
	}

	dbret = cal_db_util_stmt_step(stmt);
	if (CAL_DB_DONE != dbret) {
		sqlite3_finalize(stmt);
		ERR("cal_db_util_stmt_step() Failed(%d)", dbret);

		CAL_FREE(set);
		if(bind_text) {
			g_slist_free_full(bind_text, free);
			bind_text = NULL;
		}

		switch (dbret) {
		case CAL_DB_ERROR_NO_SPACE:
			return CALENDAR_ERROR_FILE_NO_SPACE;
		default:
			return CALENDAR_ERROR_DB_FAILED;
		}
	}

	sqlite3_finalize(stmt);

	cal_db_util_notify(CAL_NOTI_TYPE_CALENDAR);

	CAL_FREE(set);
	if(bind_text) {
		g_slist_free_full(bind_text, free);
		bind_text = NULL;
	}

	return CALENDAR_ERROR_NONE;
}
