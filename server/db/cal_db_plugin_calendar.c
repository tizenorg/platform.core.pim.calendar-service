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
#include "cal_utils.h"

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
static int _cal_db_calendar_delete_records(int ids[], int count);
static int _cal_db_calendar_get_count(int *out_count);
static int _cal_db_calendar_get_count_with_query(calendar_query_h query, int *out_count);
static int _cal_db_calendar_replace_record(calendar_record_h record, int id);

/*
 * static function
 */
static void _cal_db_calendar_get_stmt(sqlite3_stmt *stmt, calendar_record_h record);
static void _cal_db_calendar_get_property_stmt(sqlite3_stmt *stmt,
		unsigned int property, int stmt_count, calendar_record_h record);
static void _cal_db_calendar_get_projection_stmt(sqlite3_stmt *stmt,
		const unsigned int *projection, const int projection_count,
		calendar_record_h record);
static int _cal_db_calendar_update_projection(calendar_record_h record);

cal_db_plugin_cb_s cal_db_calendar_plugin_cb = {
	.is_query_only = false,
	.insert_record = _cal_db_calendar_insert_record,
	.get_record = _cal_db_calendar_get_record,
	.update_record = _cal_db_calendar_update_record,
	.delete_record = _cal_db_calendar_delete_record,
	.get_all_records = _cal_db_calendar_get_all_records,
	.get_records_with_query = _cal_db_calendar_get_records_with_query,
	.insert_records = NULL, /* use insert record with bulk count */
	.update_records = NULL,
	.delete_records = _cal_db_calendar_delete_records,
	.get_count = _cal_db_calendar_get_count,
	.get_count_with_query = _cal_db_calendar_get_count_with_query,
	.replace_record = _cal_db_calendar_replace_record,
	.replace_records = NULL
};

static bool _cal_db_calendar_check_value_validation(cal_book_s* calendar)
{
	RETVM_IF(NULL == calendar, CALENDAR_ERROR_INVALID_PARAMETER, "calendar is NULL");

	switch (calendar->store_type) {
	case CALENDAR_BOOK_TYPE_NONE:
	case CALENDAR_BOOK_TYPE_EVENT:
	case CALENDAR_BOOK_TYPE_TODO:
		return true;

	default:
		/* LCOV_EXCL_START */
		ERR("store type is invalid(%d)", calendar->store_type);
		return false;
		/* LCOV_EXCL_STOP */
	}
	return true;
}

static int _cal_db_calendar_insert_record(calendar_record_h record, int* id)
{
	CAL_FN_CALL();

	int ret = 0;

	RETV_IF(NULL == record, CALENDAR_ERROR_INVALID_PARAMETER);

	cal_book_s *book =  (cal_book_s *)(record);
	if (false == _cal_db_calendar_check_value_validation(book)) {
		/* LCOV_EXCL_START */
		ERR("cal_db_calendar_check_value_validation() Fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
		/* LCOV_EXCL_STOP */
	}

	char *client_label = NULL;
	client_label = cal_access_control_get_label();

	char query[CAL_DB_SQL_MAX_LEN];
	snprintf(query, sizeof(query), "INSERT INTO %s (uid, updated, name, description, "
			"color, location, visibility, sync_event, account_id, store_type, "
			"sync_data1, sync_data2, sync_data3, sync_data4, mode, owner_label) "
			"VALUES (?, %d, ?, ?, ?, ?, %d, %d, %d, %d, ?, ?, ?, ?, %d, ?)",
			CAL_TABLE_CALENDAR, book->updated, book->visibility, book->sync_event,
			book->account_id, book->store_type, book->mode);

	sqlite3_stmt *stmt = NULL;
	ret = cal_db_util_query_prepare(query, &stmt);
	if (CALENDAR_ERROR_NONE != ret) {
		/* LCOV_EXCL_START */
		ERR("cal_db_util_query_prepare() Fail(%d)", ret);
		SECURE("query[%s]", query);
		free(client_label);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	if (book->uid)
		cal_db_util_stmt_bind_text(stmt, 1, book->uid);
	if (book->name)
		cal_db_util_stmt_bind_text(stmt, 2, book->name);
	if (book->description)
		cal_db_util_stmt_bind_text(stmt, 3, book->description);
	if (book->color)
		cal_db_util_stmt_bind_text(stmt, 4, book->color);
	if (book->location)
		cal_db_util_stmt_bind_text(stmt, 5, book->location);
	if (book->sync_data1)
		cal_db_util_stmt_bind_text(stmt, 6, book->sync_data1);
	if (book->sync_data2)
		cal_db_util_stmt_bind_text(stmt, 7, book->sync_data2);
	if (book->sync_data3)
		cal_db_util_stmt_bind_text(stmt, 8, book->sync_data3);
	if (book->sync_data4)
		cal_db_util_stmt_bind_text(stmt, 9, book->sync_data4);
	if (client_label)
		cal_db_util_stmt_bind_text(stmt, 10, client_label);

	ret = cal_db_util_stmt_step(stmt);
	if (CALENDAR_ERROR_NONE != ret) {
		/* LCOV_EXCL_START */
		ERR("cal_db_util_stmt_step() Fail(%d)", ret);
		sqlite3_finalize(stmt);
		free(client_label);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	sqlite3_finalize(stmt);
	free(client_label);

	int index = 0;
	index = cal_db_util_last_insert_id();
	/* access control */
	cal_access_control_reset();

	if (id)
		*id = index;

	cal_db_util_notify(CAL_NOTI_TYPE_CALENDAR);

	return CALENDAR_ERROR_NONE;
}

static int _cal_db_calendar_get_record(int id, calendar_record_h* out_record)
{
	char query[CAL_DB_SQL_MAX_LEN];
	sqlite3_stmt *stmt = NULL;
	int ret = 0;

	ret = calendar_record_create(_calendar_book._uri, out_record);
	if (CALENDAR_ERROR_NONE != ret) {
		/* LCOV_EXCL_START */
		ERR("calendar_record_create() Fail(%d)", ret);
		return CALENDAR_ERROR_OUT_OF_MEMORY;
		/* LCOV_EXCL_STOP */
	}

	snprintf(query, sizeof(query), "SELECT * FROM %s WHERE id=%d AND (deleted = 0)",
			CAL_TABLE_CALENDAR,	id);
	ret = cal_db_util_query_prepare(query, &stmt);
	if (CALENDAR_ERROR_NONE != ret) {
		/* LCOV_EXCL_START */
		ERR("cal_db_util_query_prepare() Fail(%d)", ret);
		SECURE("query[%s]", query);
		calendar_record_destroy(*out_record, true);
		*out_record = NULL;
		return ret;
		/* LCOV_EXCL_STOP */
	}
	ret = cal_db_util_stmt_step(stmt);
	if (CAL_SQLITE_ROW != ret) {
		/* LCOV_EXCL_START */
		ERR("cal_db_util_stmt_step() Fail(%d)", ret);
		SECURE("query[%s]", query);
		sqlite3_finalize(stmt);
		calendar_record_destroy(*out_record, true);
		*out_record = NULL;
		if (CALENDAR_ERROR_NONE == ret)
			return CALENDAR_ERROR_DB_RECORD_NOT_FOUND;
		return ret;
		/* LCOV_EXCL_STOP */
	}

	_cal_db_calendar_get_stmt(stmt, *out_record);
	sqlite3_finalize(stmt);

	return CALENDAR_ERROR_NONE;
}

static int _cal_db_calendar_update_record(calendar_record_h record)
{
	int ret = 0;

	RETV_IF(NULL == record, CALENDAR_ERROR_INVALID_PARAMETER);

	cal_book_s* book =  (cal_book_s*)(record);
	if (false == _cal_db_calendar_check_value_validation(book)) {
		/* LCOV_EXCL_START */
		ERR("cal_db_calendar_check_value_validation() Fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
		/* LCOV_EXCL_STOP */
	}

	if (book->common.properties_flags != NULL)
		return _cal_db_calendar_update_projection(record);


	char query[CAL_DB_SQL_MAX_LEN] = {0};
	snprintf(query, sizeof(query), "UPDATE %s SET "
			"uid =?, updated = %d, name =?, description =?, color =?, location =?, "
			"visibility =%d, sync_event =%d, account_id =%d, store_type =%d, "
			"sync_data1 =?, sync_data2 =?, sync_data3 =?, sync_data4 =?, mode =%d "
			"WHERE id =%d",
			CAL_TABLE_CALENDAR, book->updated, book->visibility, book->sync_event,
			book->account_id, book->store_type, book->mode, book->index);

	sqlite3_stmt *stmt = NULL;
	ret = cal_db_util_query_prepare(query, &stmt);
	if (CALENDAR_ERROR_NONE != ret) {
		/* LCOV_EXCL_START */
		ERR("cal_db_util_query_prepare() Fail(%d)", ret);
		SECURE("query[%s]", query);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	if (book->uid)
		cal_db_util_stmt_bind_text(stmt, 1, book->uid);
	if (book->name)
		cal_db_util_stmt_bind_text(stmt, 2, book->name);
	if (book->description)
		cal_db_util_stmt_bind_text(stmt, 3, book->description);
	if (book->color)
		cal_db_util_stmt_bind_text(stmt, 4, book->color);
	if (book->location)
		cal_db_util_stmt_bind_text(stmt, 5, book->location);
	if (book->sync_data1)
		cal_db_util_stmt_bind_text(stmt, 6, book->sync_data1);
	if (book->sync_data2)
		cal_db_util_stmt_bind_text(stmt, 7, book->sync_data2);
	if (book->sync_data3)
		cal_db_util_stmt_bind_text(stmt, 8, book->sync_data3);
	if (book->sync_data4)
		cal_db_util_stmt_bind_text(stmt, 9, book->sync_data4);

	ret = cal_db_util_stmt_step(stmt);
	if (CALENDAR_ERROR_NONE != ret) {
		/* LCOV_EXCL_START */
		ERR("cal_db_util_stmt_step() Fail(%d)", ret);
		sqlite3_finalize(stmt);
		SECURE("query[%s]", query);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	sqlite3_finalize(stmt);
	cal_db_util_notify(CAL_NOTI_TYPE_CALENDAR);

	return CALENDAR_ERROR_NONE;
}

static int _cal_db_calendar_delete_record(int id)
{
	int ret = CALENDAR_ERROR_NONE;
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	int calendar_book_id = -1;

	snprintf(query, sizeof(query), "SELECT id FROM %s WHERE id = %d",
			CAL_TABLE_CALENDAR, id);
	ret = cal_db_util_query_get_first_int_result(query, NULL, &calendar_book_id);
	if (CALENDAR_ERROR_NONE != ret) {
		/* LCOV_EXCL_START */
		ERR("cal_db_util_query_get_first_int_result() Fail(%d)", ret);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	int count = 0;
	int count2 = 0;
	snprintf(query, sizeof(query), "select count(*) from %s", CAL_TABLE_NORMAL_INSTANCE);
	ret = cal_db_util_query_get_first_int_result(query, NULL, &count);
	if (CALENDAR_ERROR_NONE != ret) {
		/* LCOV_EXCL_START */
		ERR("cal_db_util_query_get_first_int_result() Fail");
		return ret;
		/* LCOV_EXCL_STOP */
	}

	snprintf(query, sizeof(query), "select count(*) from %s", CAL_TABLE_ALLDAY_INSTANCE);
	ret = cal_db_util_query_get_first_int_result(query, NULL, &count2);
	if (CALENDAR_ERROR_NONE != ret) {
		/* LCOV_EXCL_START */
		ERR("cal_db_util_query_get_first_int_result() Fail");
		return ret;
		/* LCOV_EXCL_STOP */
	}

	count += count2;

	if (1000 < count) {
		snprintf(query, sizeof(query), "UPDATE %s SET deleted = 1 WHERE id = %d", CAL_TABLE_CALENDAR, id);
		ret = cal_db_util_query_exec(query);
		RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_db_util_query_exec() Fail(%d)", ret);
		cal_server_calendar_delete_start();
	} else {
		snprintf(query, sizeof(query), "DELETE FROM %s WHERE id = %d", CAL_TABLE_CALENDAR, id);
		ret = cal_db_util_query_exec(query);
		RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_db_util_query_exec() Fail(%d)", ret);

		snprintf(query, sizeof(query), "DELETE FROM %s WHERE calendar_id = %d", CAL_TABLE_SCHEDULE, id);
		ret = cal_db_util_query_exec(query);
		RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_db_util_query_exec() Fail(%d)", ret);
	}

	snprintf(query, sizeof(query), "DELETE FROM %s WHERE calendar_id = %d", CAL_TABLE_DELETED, id);
	ret = cal_db_util_query_exec(query);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_db_util_query_exec() Fail(%d)", ret);

	cal_access_control_reset();

	cal_db_util_notify(CAL_NOTI_TYPE_CALENDAR);
	return CALENDAR_ERROR_NONE;
}

static int _cal_db_calendar_replace_record(calendar_record_h record, int id)
{
	int ret = 0;

	RETV_IF(NULL == record, CALENDAR_ERROR_INVALID_PARAMETER);

	cal_book_s *book =  (cal_book_s *)(record);
	if (false == _cal_db_calendar_check_value_validation(book)) {
		/* LCOV_EXCL_START */
		ERR("cal_db_calendar_check_value_validation() Fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
		/* LCOV_EXCL_STOP */
	}

	book->index = id;
	if (book->common.properties_flags)
		return _cal_db_calendar_update_projection(record);

	char query[CAL_DB_SQL_MAX_LEN] = {0};
	snprintf(query, sizeof(query), "UPDATE %s SET "
			"uid =?, updated =%d, name =?, description =?, color =?, location =?, "
			"visibility =%d, sync_event =%d, account_id =%d, store_type =%d, "
			"sync_data1 =?, sync_data2 =?, sync_data3 =?, sync_data4 =?, mode =%d "
			"WHERE id =%d",
			CAL_TABLE_CALENDAR, book->updated, book->visibility, book->sync_event,
			book->account_id, book->store_type, book->mode, id);

	sqlite3_stmt *stmt = NULL;
	ret = cal_db_util_query_prepare(query, &stmt);
	if (CALENDAR_ERROR_NONE != ret) {
		/* LCOV_EXCL_START */
		ERR("cal_db_util_query_prepare() Fail(%d)", ret);
		SECURE("query[%s]", query);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	if (book->uid)
		cal_db_util_stmt_bind_text(stmt, 1, book->uid);
	if (book->name)
		cal_db_util_stmt_bind_text(stmt, 2, book->name);
	if (book->description)
		cal_db_util_stmt_bind_text(stmt, 3, book->description);
	if (book->color)
		cal_db_util_stmt_bind_text(stmt, 4, book->color);
	if (book->location)
		cal_db_util_stmt_bind_text(stmt, 5, book->location);
	if (book->sync_data1)
		cal_db_util_stmt_bind_text(stmt, 6, book->sync_data1);
	if (book->sync_data2)
		cal_db_util_stmt_bind_text(stmt, 7, book->sync_data2);
	if (book->sync_data3)
		cal_db_util_stmt_bind_text(stmt, 8, book->sync_data3);
	if (book->sync_data4)
		cal_db_util_stmt_bind_text(stmt, 9, book->sync_data4);

	ret = cal_db_util_stmt_step(stmt);
	if (CALENDAR_ERROR_NONE != ret) {
		/* LCOV_EXCL_START */
		ERR("cal_db_util_stmt_step() Fail(%d)", ret);
		sqlite3_finalize(stmt);
		SECURE("query[%s]", query);
		return ret;
		/* LCOV_EXCL_STOP */
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

	if (0 < offset)
		snprintf(offsetquery, sizeof(offsetquery), "OFFSET %d", offset);
	if (0 < limit)
		snprintf(limitquery, sizeof(limitquery), "LIMIT %d", limit);

	char *query_str = NULL;
	cal_db_append_string(&query_str, "SELECT * FROM");
	cal_db_append_string(&query_str, CAL_TABLE_CALENDAR);
	cal_db_append_string(&query_str, "WHERE deleted = 0");
	cal_db_append_string(&query_str, limitquery);
	cal_db_append_string(&query_str, offsetquery);

	ret = cal_db_util_query_prepare(query_str, &stmt);
	if (CALENDAR_ERROR_NONE != ret) {
		/* LCOV_EXCL_START */
		ERR("cal_db_util_query_prepare() Fail(%d)", ret);
		SECURE("query[%s]", query_str);
		calendar_list_destroy(*out_list, true);
		*out_list = NULL;
		free(query_str);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	while (CAL_SQLITE_ROW == cal_db_util_stmt_step(stmt)) {
		calendar_record_h record;
		ret = calendar_record_create(_calendar_book._uri, &record);
		if (CALENDAR_ERROR_NONE != ret) {
			/* LCOV_EXCL_START */
			calendar_list_destroy(*out_list, true);
			*out_list = NULL;
			sqlite3_finalize(stmt);
			CAL_FREE(query_str);
			return ret;
			/* LCOV_EXCL_STOP */
		}
		_cal_db_calendar_get_stmt(stmt, record);

		ret = calendar_list_add(*out_list, record);
		if (CALENDAR_ERROR_NONE != ret) {
			/* LCOV_EXCL_START */
			calendar_list_destroy(*out_list, true);
			*out_list = NULL;
			calendar_record_destroy(record, true);
			sqlite3_finalize(stmt);
			CAL_FREE(query_str);
			return ret;
			/* LCOV_EXCL_STOP */
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

	/* make filter */
	if (que->filter) {
		ret = cal_db_query_create_condition(query, &condition, &bind_text);
		if (CALENDAR_ERROR_NONE != ret) {
			/* LCOV_EXCL_START */
			ERR("cal_db_query_create_condition() Fail(%d), ret");
			return ret;
			/* LCOV_EXCL_STOP */
		}
	}

	/* make: projection */
	char *projection = NULL;
	ret = cal_db_query_create_projection(query, &projection);

	char *query_str = NULL;
	/* query: projection */
	if (projection) {
		cal_db_append_string(&query_str, "SELECT");
		cal_db_append_string(&query_str, projection);
		cal_db_append_string(&query_str, "FROM");
		cal_db_append_string(&query_str, CAL_TABLE_CALENDAR);
		CAL_FREE(projection);
	} else {
		cal_db_append_string(&query_str, "SELECT * FROM");
		cal_db_append_string(&query_str, CAL_TABLE_CALENDAR);
	}

	/* query: condition */
	if (condition) {
		cal_db_append_string(&query_str,  "WHERE (");
		cal_db_append_string(&query_str, condition);
		cal_db_append_string(&query_str, ") AND (deleted = 0)");
	}

	/* order */
	char *order = NULL;
	ret = cal_db_query_create_order(query, condition, &order);
	if (order) {
		cal_db_append_string(&query_str, order);
		CAL_FREE(order);
	}
	CAL_FREE(condition);

	/* limit, offset */
	char buf[CAL_STR_SHORT_LEN32] = {0};
	if (0 < limit) {
		snprintf(buf, sizeof(buf), "LIMIT %d", limit);
		cal_db_append_string(&query_str, buf);

		if (0 < offset) {
			snprintf(buf, sizeof(buf), "OFFSET %d", offset);
			cal_db_append_string(&query_str, buf);
		}
	}

	/* query */
	ret = cal_db_util_query_prepare(query_str, &stmt);
	SECURE("[TEST]---------query[%s]", query_str);
	if (CALENDAR_ERROR_NONE != ret) {
		/* LCOV_EXCL_START */
		ERR("cal_db_util_query_prepare() Fail(%d)", ret);
		SECURE("query[%s]", query_str);
		if (bind_text) {
			g_slist_free_full(bind_text, free);
			bind_text = NULL;
		}
		free(query_str);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	/* bind text */
	if (bind_text) {
		g_slist_length(bind_text);
		for (cursor = bind_text, i = 1; cursor; cursor = cursor->next, i++)
			cal_db_util_stmt_bind_text(stmt, i, cursor->data);
	}

	ret = calendar_list_create(out_list);
	if (CALENDAR_ERROR_NONE != ret) {
		/* LCOV_EXCL_START */
		ERR("calendar_list_create() Fail");
		if (bind_text) {
			g_slist_free_full(bind_text, free);
			bind_text = NULL;
		}
		sqlite3_finalize(stmt);
		CAL_FREE(query_str);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	while (CAL_SQLITE_ROW == cal_db_util_stmt_step(stmt)) {
		calendar_record_h record;
		ret = calendar_record_create(_calendar_book._uri, &record);
		if (CALENDAR_ERROR_NONE != ret) {
			/* LCOV_EXCL_START */
			ERR("calendar_record_create() Fail(%d)", ret);
			calendar_list_destroy(*out_list, true);
			*out_list = NULL;

			if (bind_text) {
				g_slist_free_full(bind_text, free);
				bind_text = NULL;
			}
			sqlite3_finalize(stmt);
			CAL_FREE(query_str);
			return ret;
			/* LCOV_EXCL_STOP */
		}

		if (0 < que->projection_count) {
			cal_record_set_projection(record,
					que->projection, que->projection_count, que->property_count);

			_cal_db_calendar_get_projection_stmt(stmt,
					que->projection, que->projection_count,
					record);
		} else {
			_cal_db_calendar_get_stmt(stmt, record);
		}

		ret = calendar_list_add(*out_list, record);
		if (CALENDAR_ERROR_NONE != ret) {
			/* LCOV_EXCL_START */
			ERR("calendar_list_add() Fail(%d)", ret);
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
			/* LCOV_EXCL_STOP */
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

static int _cal_db_calendar_delete_records(int ids[], int count)
{
	int ret = 0;
	int i = 0;
	for (i = 0; i < count; i++) {
		ret = _cal_db_calendar_delete_record(ids[i]);
		if (CALENDAR_ERROR_NONE != ret) {
			/* LCOV_EXCL_START */
			ERR("_cal_db_calendar_delete_record() Fail(%d)", ret);
			return CALENDAR_ERROR_DB_FAILED;
			/* LCOV_EXCL_STOP */
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
		/* LCOV_EXCL_START */
		ERR("cal_db_util_query_get_first_int_result() failed");
		CAL_FREE(query_str);
		return ret;
		/* LCOV_EXCL_STOP */
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

	if (CAL_STRING_EQUAL == strcmp(que->view_uri, CALENDAR_VIEW_CALENDAR)) {
		table_name = cal_strdup(CAL_TABLE_CALENDAR);
	} else {
		/* LCOV_EXCL_START */
		ERR("uri(%s) not support get records with query", que->view_uri);
		return CALENDAR_ERROR_INVALID_PARAMETER;
		/* LCOV_EXCL_STOP */
	}

	/* make filter */
	if (que->filter) {
		ret = cal_db_query_create_condition(query, &condition, &bind_text);
		if (CALENDAR_ERROR_NONE != ret) {
			/* LCOV_EXCL_START */
			ERR("cal_db_query_create_condition() Fail(%d), ret");
			CAL_FREE(table_name);
			return ret;
			/* LCOV_EXCL_STOP */
		}
	}

	char *query_str = NULL;
	/* query: select */
	cal_db_append_string(&query_str, "SELECT count(*) FROM");
	cal_db_append_string(&query_str, table_name);
	CAL_FREE(table_name);

	/* query: condition */
	if (condition) {
		cal_db_append_string(&query_str, "WHERE (");
		cal_db_append_string(&query_str, condition);
		cal_db_append_string(&query_str, ") AND (deleted = 0)");
		CAL_FREE(condition);
	}

	/* query */
	ret = cal_db_util_query_get_first_int_result(query_str, bind_text, &count);
	if (CALENDAR_ERROR_NONE != ret) {
		/* LCOV_EXCL_START */
		ERR("cal_db_util_query_get_first_int_result() failed");
		if (bind_text) {
			g_slist_free_full(bind_text, free);
			bind_text = NULL;
		}
		CAL_FREE(query_str);
		return ret;
		/* LCOV_EXCL_STOP */
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

static void _cal_db_calendar_get_stmt(sqlite3_stmt *stmt, calendar_record_h record)
{
	cal_book_s* calendar =  (cal_book_s*)(record);
	int count = 0;
	const unsigned char *temp;

	calendar->index = sqlite3_column_int(stmt, count++);

	temp = sqlite3_column_text(stmt, count++);
	calendar->uid = cal_strdup((const char*)temp);

	calendar->updated = sqlite3_column_int(stmt, count++);

	temp = sqlite3_column_text(stmt, count++);
	calendar->name = cal_strdup((const char*)temp);

	temp = sqlite3_column_text(stmt, count++);
	calendar->description = cal_strdup((const char*)temp);

	temp = sqlite3_column_text(stmt, count++);
	calendar->color = cal_strdup((const char*)temp);

	temp = sqlite3_column_text(stmt, count++);
	calendar->location = cal_strdup((const char*)temp);

	calendar->visibility = sqlite3_column_int(stmt, count++);
	calendar->sync_event = sqlite3_column_int(stmt, count++);
	calendar->is_deleted = sqlite3_column_int(stmt, count++);
	calendar->account_id = sqlite3_column_int(stmt, count++);
	calendar->store_type = sqlite3_column_int(stmt, count++);

	temp = sqlite3_column_text(stmt, count++);
	calendar->sync_data1 = cal_strdup((const char*)temp);
	temp = sqlite3_column_text(stmt, count++);
	calendar->sync_data2 = cal_strdup((const char*)temp);
	temp = sqlite3_column_text(stmt, count++);
	calendar->sync_data3 = cal_strdup((const char*)temp);
	temp = sqlite3_column_text(stmt, count++);
	calendar->sync_data4 = cal_strdup((const char*)temp);

	/* deleted */
	sqlite3_column_int(stmt, count++);

	/* mode */
	calendar->mode = sqlite3_column_int(stmt, count++);

	/* owner_label */
	sqlite3_column_text(stmt, count++);
}

static void _cal_db_calendar_get_property_stmt(sqlite3_stmt *stmt,
		unsigned int property, int stmt_count, calendar_record_h record)
{
	cal_book_s* calendar =  (cal_book_s*)(record);
	const unsigned char *temp;

	switch (property) {
	case CAL_PROPERTY_CALENDAR_ID:
		calendar->index = sqlite3_column_int(stmt, stmt_count);
		break;
	case CAL_PROPERTY_CALENDAR_UID:
		temp = sqlite3_column_text(stmt, stmt_count);
		calendar->uid = cal_strdup((const char*)temp);
		break;
	case CAL_PROPERTY_CALENDAR_NAME:
		temp = sqlite3_column_text(stmt, stmt_count);
		calendar->name = cal_strdup((const char*)temp);
		break;
	case CAL_PROPERTY_CALENDAR_DESCRIPTION:
		temp = sqlite3_column_text(stmt, stmt_count);
		calendar->description = cal_strdup((const char*)temp);
		break;
	case CAL_PROPERTY_CALENDAR_COLOR:
		temp = sqlite3_column_text(stmt, stmt_count);
		calendar->color = cal_strdup((const char*)temp);
		break;
	case CAL_PROPERTY_CALENDAR_LOCATION:
		temp = sqlite3_column_text(stmt, stmt_count);
		calendar->location = cal_strdup((const char*)temp);
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
		calendar->sync_data1 = cal_strdup((const char*)temp);
		break;
	case CAL_PROPERTY_CALENDAR_SYNC_DATA2:
		temp = sqlite3_column_text(stmt, stmt_count);
		calendar->sync_data1 = cal_strdup((const char*)temp);
		break;
	case CAL_PROPERTY_CALENDAR_SYNC_DATA3:
		temp = sqlite3_column_text(stmt, stmt_count);
		calendar->sync_data1 = cal_strdup((const char*)temp);
		break;
	case CAL_PROPERTY_CALENDAR_SYNC_DATA4:
		temp = sqlite3_column_text(stmt, stmt_count);
		calendar->sync_data1 = cal_strdup((const char*)temp);
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
	int i = 0;

	for (i = 0; i < projection_count; i++)
		_cal_db_calendar_get_property_stmt(stmt, projection[i], i, record);
}

static int _cal_db_calendar_update_projection(calendar_record_h record)
{
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	sqlite3_stmt *stmt = NULL;
	cal_book_s* calendar =  (cal_book_s*)(record);
	int ret = 0;
	char* set = NULL;
	GSList *bind_text = NULL;
	GSList *cursor = NULL;

	ret = cal_db_query_create_projection_update_set(record, &set, &bind_text);
	RETV_IF(CALENDAR_ERROR_NONE != ret, ret);

	snprintf(query, sizeof(query), "UPDATE %s SET %s "
			"WHERE id = %d",
			CAL_TABLE_CALENDAR, set,
			calendar->index);

	ret = cal_db_util_query_prepare(query, &stmt);
	if (CALENDAR_ERROR_NONE != ret) {
		/* LCOV_EXCL_START */
		ERR("cal_db_util_query_prepare() Fail(%d)", ret);
		SECURE("query[%s]", query);
		free(set);
		if (bind_text) {
			g_slist_free_full(bind_text, free);
			bind_text = NULL;
		}
		return ret;
		/* LCOV_EXCL_STOP */
	}

	/* bind */
	if (bind_text) {
		int i = 0;
		for (cursor = bind_text, i = 1; cursor; cursor = cursor->next, i++)
			cal_db_util_stmt_bind_text(stmt, i, cursor->data);
	}

	ret = cal_db_util_stmt_step(stmt);
	if (CALENDAR_ERROR_NONE != ret) {
		/* LCOV_EXCL_START */
		ERR("cal_db_util_stmt_step() Fail(%d)", ret);
		SECURE("query[%s]", query);
		sqlite3_finalize(stmt);
		free(set);
		if (bind_text) {
			g_slist_free_full(bind_text, free);
			bind_text = NULL;
		}
		return ret;
		/* LCOV_EXCL_STOP */
	}

	sqlite3_finalize(stmt);

	cal_db_util_notify(CAL_NOTI_TYPE_CALENDAR);

	CAL_FREE(set);
	if (bind_text) {
		g_slist_free_full(bind_text, free);
		bind_text = NULL;
	}

	return CALENDAR_ERROR_NONE;
}
