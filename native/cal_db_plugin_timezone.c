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

#include "cal_db_util.h"
#include "cal_db.h"
#include "cal_db_query.h"
#include "cal_utils.h"

static int _cal_db_timezone_insert_record(calendar_record_h record, int* id);
static int _cal_db_timezone_get_record(int id, calendar_record_h* out_record);
static int _cal_db_timezone_update_record(calendar_record_h record);
static int _cal_db_timezone_delete_record(int id);
static int _cal_db_timezone_get_all_records(int offset, int limit, calendar_list_h* out_list);
static int _cal_db_timezone_get_records_with_query(calendar_query_h query, int offset, int limit, calendar_list_h* out_list);
static int _cal_db_timezone_insert_records(const calendar_list_h list, int** ids);
static int _cal_db_timezone_update_records(const calendar_list_h list);
static int _cal_db_timezone_delete_records(int ids[], int count);
static int _cal_db_timezone_get_count(int *out_count);
static int _cal_db_timezone_get_count_with_query(calendar_query_h query, int *out_count);
static int _cal_db_timezone_replace_record(calendar_record_h record, int id);
static int _cal_db_timezone_replace_records(const calendar_list_h list, int ids[], int count);

/*
 * static function
 */
static void _cal_db_timezone_get_stmt(sqlite3_stmt *stmt,calendar_record_h record);
static void _cal_db_timezone_get_property_stmt(sqlite3_stmt *stmt,
		unsigned int property, int stmt_count, calendar_record_h record);
static void _cal_db_timezone_get_projection_stmt(sqlite3_stmt *stmt,
		const unsigned int *projection, const int projection_count,
		calendar_record_h record);
static void _cal_db_timezone_get_stmt(sqlite3_stmt *stmt,calendar_record_h record);
static int _cal_db_timezone_update_projection(calendar_record_h record);

cal_db_plugin_cb_s cal_db_timezone_plugin_cb = {
	.is_query_only=false,
	.insert_record=_cal_db_timezone_insert_record,
	.get_record=_cal_db_timezone_get_record,
	.update_record=_cal_db_timezone_update_record,
	.delete_record=_cal_db_timezone_delete_record,
	.get_all_records=_cal_db_timezone_get_all_records,
	.get_records_with_query=_cal_db_timezone_get_records_with_query,
	.insert_records=_cal_db_timezone_insert_records,
	.update_records=_cal_db_timezone_update_records,
	.delete_records=_cal_db_timezone_delete_records,
	.get_count=_cal_db_timezone_get_count,
	.get_count_with_query=_cal_db_timezone_get_count_with_query,
	.replace_record = _cal_db_timezone_replace_record,
	.replace_records = _cal_db_timezone_replace_records
};

static int _cal_db_timezone_insert_record(calendar_record_h record, int* id)
{
	int ret = CALENDAR_ERROR_NONE;
	int index;
	int calendar_book_id = 0;
	char query[CAL_DB_SQL_MAX_LEN];
	sqlite3_stmt *stmt;
	cal_timezone_s* timezone =  (cal_timezone_s*)(record);
	calendar_record_h record_calendar = NULL;

	RETV_IF(NULL == timezone, CALENDAR_ERROR_INVALID_PARAMETER);

	ret = calendar_record_get_int(record,
			_calendar_timezone.calendar_book_id, &calendar_book_id);
	DBG("calendar_book_id(%d)", calendar_book_id);

	ret = cal_db_get_record(_calendar_book._uri, calendar_book_id, &record_calendar);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_db_get_record() Fail(%d)", ret);
	calendar_record_destroy(record_calendar, true);

	if (timezone->standard_name) {
		snprintf(query, sizeof(query), "SELECT count(*), id FROM %s WHERE standard_name=? ",
				CAL_TABLE_TIMEZONE);
		ret = cal_db_util_query_prepare(query, &stmt);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_db_util_query_prepare() Fail(%d)", ret);
			SECURE("query[%s]", query);
			return ret;
		}
		cal_db_util_stmt_bind_text(stmt, 1, timezone->standard_name);

		ret = cal_db_util_stmt_step(stmt);
		if (CAL_SQLITE_ROW != ret) {
			ERR("cal_db_util_stmt_step() Fail(%d)", ret);
			sqlite3_finalize(stmt);
			return ret;
		}

		index = 0;
		int count = sqlite3_column_int(stmt, index++);
		int timezone_id = sqlite3_column_int(stmt, index++);
		sqlite3_finalize(stmt);

		if (0 < count) {
			DBG("Already exist which tzid name[%s] id(%d)", timezone->standard_name, timezone_id);
			*id = timezone_id;
			return CALENDAR_ERROR_NONE;
		}
		DBG("Not registered timezone in the table, so insert timezone.");
	}

	snprintf(query, sizeof(query), "INSERT INTO %s(tz_offset_from_gmt ,standard_name, "
			"std_start_month ,std_start_position_of_week ,std_start_day, "
			"std_start_hour ,standard_bias ,day_light_name ,day_light_start_month, "
			"day_light_start_position_of_week ,day_light_start_day, "
			"day_light_start_hour ,day_light_bias, calendar_id) "
			"VALUES(%d,?,%d,%d,%d,%d,%d,?,%d,%d,%d,%d,%d,%d)",
			CAL_TABLE_TIMEZONE,
			timezone->tz_offset_from_gmt,
			timezone->std_start_month,
			timezone->std_start_position_of_week,
			timezone->std_start_day,
			timezone->std_start_hour,
			timezone->standard_bias,
			timezone->day_light_start_month,
			timezone->day_light_start_position_of_week,
			timezone->day_light_start_day,
			timezone->day_light_start_hour,
			timezone->day_light_bias,
			timezone->calendar_id);

	ret = cal_db_util_query_prepare(query, &stmt);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_query_prepare() Fail(%d)", ret);
		SECURE("query[%s]", query);
		return ret;
	}

	if (timezone->standard_name)
		cal_db_util_stmt_bind_text(stmt, 1, timezone->standard_name);

	if (timezone->day_light_name)
		cal_db_util_stmt_bind_text(stmt, 2, timezone->day_light_name);

	ret = cal_db_util_stmt_step(stmt);
	sqlite3_finalize(stmt);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_stmt_step() Fail(%d)", ret);
		return ret;
	}
	index = cal_db_util_last_insert_id();

	if (id) {
		*id = index;
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_db_timezone_get_record(int id, calendar_record_h* out_record)
{
	char query[CAL_DB_SQL_MAX_LEN];
	sqlite3_stmt *stmt = NULL;
	int ret = 0;

	ret = calendar_record_create(_calendar_timezone._uri ,out_record);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("calendar_record_create() Fail(%d)", ret);
		return CALENDAR_ERROR_OUT_OF_MEMORY;
	}

	snprintf(query, sizeof(query), "SELECT * FROM %s WHERE id = %d AND "
			"calendar_id IN (select id from %s where deleted = 0)",
			CAL_TABLE_TIMEZONE, id,
			CAL_TABLE_CALENDAR);
	ret = cal_db_util_query_prepare(query, &stmt);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_query_prepare() Fail(%d)", ret);
		SECURE("query[%s]", query);
		calendar_record_destroy(*out_record, true);
		*out_record = NULL;
		return ret;
	}

	ret = cal_db_util_stmt_step(stmt);
	if (CAL_SQLITE_ROW != ret) {
		ERR("cal_db_util_stmt_step() Faile%d)", ret);
		sqlite3_finalize(stmt);
		calendar_record_destroy(*out_record, true);
		*out_record = NULL;
		if (CALENDAR_ERROR_NONE == ret)
			return CALENDAR_ERROR_DB_RECORD_NOT_FOUND;
		return ret;
	}

	_cal_db_timezone_get_stmt(stmt,*out_record);

	sqlite3_finalize(stmt);
	stmt = NULL;

	return CALENDAR_ERROR_NONE;
}

static int _cal_db_timezone_update_record(calendar_record_h record)
{
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	sqlite3_stmt *stmt = NULL;
	cal_timezone_s* timezone_info =  (cal_timezone_s*)(record);
	int ret = 0;

	RETV_IF(NULL == timezone_info, CALENDAR_ERROR_INVALID_PARAMETER);

	if (timezone_info->common.properties_flags) {
		return _cal_db_timezone_update_projection(record);
	}

	snprintf(query, sizeof(query), "UPDATE %s SET "
			"tz_offset_from_gmt=%d,"
			"standard_name=?,"
			"std_start_month=%d,"
			"std_start_position_of_week=%d,"
			"std_start_day=%d,"
			"std_start_hour=%d,"
			"standard_bias=%d,"
			"day_light_name=?,"
			"day_light_start_month=%d,"
			"day_light_start_position_of_week=%d,"
			"day_light_start_day=%d,"
			"day_light_start_hour=%d,"
			"day_light_bias=%d, "
			"calendar_id=%d "
			"WHERE id = %d",
			CAL_TABLE_TIMEZONE,
			timezone_info->tz_offset_from_gmt,
			timezone_info->std_start_month,
			timezone_info->std_start_position_of_week,
			timezone_info->std_start_day,
			timezone_info->std_start_hour,
			timezone_info->standard_bias,
			timezone_info->day_light_start_month,
			timezone_info->day_light_start_position_of_week,
			timezone_info->day_light_start_day,
			timezone_info->day_light_start_hour,
			timezone_info->day_light_bias,
			timezone_info->calendar_id,
			timezone_info->index);

	ret = cal_db_util_query_prepare(query, &stmt);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_query_prepare() Fail(%d)", ret);
		SECURE("query[%s]", query);
		return ret;
	}

	if (timezone_info->standard_name)
		cal_db_util_stmt_bind_text(stmt, 1, timezone_info->standard_name);

	if (timezone_info->day_light_name)
		cal_db_util_stmt_bind_text(stmt, 2, timezone_info->day_light_name);

	ret = cal_db_util_stmt_step(stmt);
	sqlite3_finalize(stmt);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_stmt_step() Fail(%d)", ret);
		return ret;
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_db_timezone_delete_record(int id)
{
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	int ret = 0;

	snprintf(query, sizeof(query), "DELETE FROM %s WHERE id = %d", CAL_TABLE_TIMEZONE, id);
	ret = cal_db_util_query_exec(query);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_query_exec() Fail(%d)", ret);
		SECURE("[%s]", query);
		return ret;
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_db_timezone_replace_record(calendar_record_h record, int id)
{
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	sqlite3_stmt *stmt = NULL;
	cal_timezone_s* timezone_info =  (cal_timezone_s*)(record);
	int ret = 0;

	RETV_IF(NULL == timezone_info, CALENDAR_ERROR_INVALID_PARAMETER);
	timezone_info->index = id;

	if (timezone_info->common.properties_flags) {
		return _cal_db_timezone_update_projection(record);
	}

	snprintf(query, sizeof(query), "UPDATE %s SET "
			"tz_offset_from_gmt=%d,"
			"standard_name=?,"
			"std_start_month=%d,"
			"std_start_position_of_week=%d,"
			"std_start_day=%d,"
			"std_start_hour=%d,"
			"standard_bias=%d,"
			"day_light_name=?,"
			"day_light_start_month=%d,"
			"day_light_start_position_of_week=%d,"
			"day_light_start_day=%d,"
			"day_light_start_hour=%d,"
			"day_light_bias=%d, "
			"calendar_id=%d "
			"WHERE id = %d",
			CAL_TABLE_TIMEZONE,
			timezone_info->tz_offset_from_gmt,
			timezone_info->std_start_month,
			timezone_info->std_start_position_of_week,
			timezone_info->std_start_day,
			timezone_info->std_start_hour,
			timezone_info->standard_bias,
			timezone_info->day_light_start_month,
			timezone_info->day_light_start_position_of_week,
			timezone_info->day_light_start_day,
			timezone_info->day_light_start_hour,
			timezone_info->day_light_bias,
			timezone_info->calendar_id,
			id);

	ret = cal_db_util_query_prepare(query, &stmt);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_query_prepare() Fail(%d)", ret);
		SECURE("query[%s]", query);
		return ret;
	}

	if (timezone_info->standard_name)
		cal_db_util_stmt_bind_text(stmt, 1, timezone_info->standard_name);

	if (timezone_info->day_light_name)
		cal_db_util_stmt_bind_text(stmt, 2, timezone_info->day_light_name);

	ret = cal_db_util_stmt_step(stmt);
	sqlite3_finalize(stmt);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_stmt_step() Fail(%d)", ret);
		return ret;
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_db_timezone_get_all_records(int offset, int limit, calendar_list_h* out_list)
{
	int ret = CALENDAR_ERROR_NONE;
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	char offsetquery[CAL_DB_SQL_MAX_LEN] = {0};
	char limitquery[CAL_DB_SQL_MAX_LEN] = {0};
	sqlite3_stmt *stmt = NULL;

	RETV_IF(NULL == out_list, CALENDAR_ERROR_INVALID_PARAMETER);

	ret = calendar_list_create(out_list);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "calendar_list_create() Fail(%d)", ret);

	if (0 < offset) {
		snprintf(offsetquery, sizeof(offsetquery), "OFFSET %d", offset);
	}
	if (0 < limit) {
		snprintf(limitquery, sizeof(limitquery), "LIMIT %d", limit);
	}
	snprintf(query, sizeof(query), "SELECT * FROM %s where "
			"calendar_id IN (select id from %s where deleted = 0) "
			"%s %s",
			CAL_TABLE_TIMEZONE,
			CAL_TABLE_CALENDAR,
			limitquery,
			offsetquery);

	ret = cal_db_util_query_prepare(query, &stmt);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_query_prepare() Fail(%d)", ret);
		SECURE("query[%s]", query);
		calendar_list_destroy(*out_list, true);
		*out_list = NULL;
		return ret;
	}

	while (CAL_SQLITE_ROW == cal_db_util_stmt_step(stmt)) {
		calendar_record_h record;
		ret = calendar_record_create(_calendar_timezone._uri,&record);
		if (CALENDAR_ERROR_NONE != ret) {
			calendar_list_destroy(*out_list, true);
			*out_list = NULL;
			sqlite3_finalize(stmt);
			return ret;
		}
		_cal_db_timezone_get_stmt(stmt,record);

		ret = calendar_list_add(*out_list,record);
		if (CALENDAR_ERROR_NONE != ret) {
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

static int _cal_db_timezone_get_records_with_query(calendar_query_h query, int offset, int limit, calendar_list_h* out_list)
{
	cal_query_s *que = NULL;
	int ret = CALENDAR_ERROR_NONE;
	char *condition = NULL;
	char *projection = NULL;
	char *order = NULL;
	GSList *bind_text = NULL, *cursor = NULL;
	char *query_str = NULL;
	sqlite3_stmt *stmt = NULL;
	int i = 0;

	que = (cal_query_s *)query;

	/* make filter */
	if (que->filter) {
		ret = cal_db_query_create_condition(query, &condition, &bind_text);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_db_query_create_condition() Fail(%d), ret");
			return ret;
		}
	}

	/* make: projection */
	ret = cal_db_query_create_projection(query, &projection);

	/* query: projection */
	if (projection) {
		cal_db_append_string(&query_str, "SELECT");
		cal_db_append_string(&query_str, projection);
		cal_db_append_string(&query_str, "FROM");
		cal_db_append_string(&query_str, CAL_TABLE_TIMEZONE);
		CAL_FREE(projection);
	}
	else {
		cal_db_append_string(&query_str, "SELECT * FROM");
		cal_db_append_string(&query_str, CAL_TABLE_TIMEZONE);
	}

	/* query: condition */
	if (condition) {
		cal_db_append_string(&query_str, "WHERE");
		cal_db_append_string(&query_str, condition);
		CAL_FREE(condition);
		cal_db_append_string(&query_str, "AND calendar_id IN (select id from");
		cal_db_append_string(&query_str, CAL_TABLE_CALENDAR);
		cal_db_append_string(&query_str, "where deleted = 0)");
	}
	else {
		cal_db_append_string(&query_str, "WHERE calendar_id IN (select id from");
		cal_db_append_string(&query_str, CAL_TABLE_CALENDAR);
		cal_db_append_string(&query_str, "where deleted = 0)");
	}

	/* order */
	ret = cal_db_query_create_order(query, condition, &order);
	if (order) {
		cal_db_append_string(&query_str, order);
		CAL_FREE(order);
	}

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
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_query_prepare() Fail(%d)", ret);
		SECURE("query[%s]", query_str);
		if (bind_text) {
			g_slist_free_full(bind_text, free);
			bind_text = NULL;
		}
		free(query_str);
		return ret;
	}

	/* bind text */
	if (bind_text) {
		for (cursor=bind_text, i=1; cursor;cursor=cursor->next, i++) {
			cal_db_util_stmt_bind_text(stmt, i, cursor->data);
		}
	}

	ret = calendar_list_create(out_list);
	if (CALENDAR_ERROR_NONE != ret) {
		if (bind_text) {
			g_slist_free_full(bind_text, free);
			bind_text = NULL;
		}
		CAL_FREE(query_str);
		ERR("calendar_list_create() Fail");
		sqlite3_finalize(stmt);
		return ret;
	}

	while (CAL_SQLITE_ROW == cal_db_util_stmt_step(stmt)) {
		calendar_record_h record;
		ret = calendar_record_create(_calendar_timezone._uri,&record);
		if (CALENDAR_ERROR_NONE != ret) {
			calendar_list_destroy(*out_list, true);
			*out_list = NULL;

			if (bind_text) {
				g_slist_free_full(bind_text, free);
				bind_text = NULL;
			}
			CAL_FREE(query_str);
			sqlite3_finalize(stmt);
			return ret;
		}
		if (0 < que->projection_count) {
			cal_record_set_projection(record,
					que->projection, que->projection_count, que->property_count);

			_cal_db_timezone_get_projection_stmt(stmt,
					que->projection, que->projection_count,
					record);
		}
		else {
			_cal_db_timezone_get_stmt(stmt,record);
		}

		ret = calendar_list_add(*out_list,record);
		if (CALENDAR_ERROR_NONE != ret) {
			calendar_list_destroy(*out_list, true);
			*out_list = NULL;
			calendar_record_destroy(record, true);

			if (bind_text) {
				g_slist_free_full(bind_text, free);
				bind_text = NULL;
			}
			CAL_FREE(query_str);
			sqlite3_finalize(stmt);
			return ret;
		}
	}

	if (bind_text) {
		g_slist_free_full(bind_text, free);
		bind_text = NULL;
	}
	CAL_FREE(query_str);
	sqlite3_finalize(stmt);

	return CALENDAR_ERROR_NONE;
}
static int _cal_db_timezone_insert_records(const calendar_list_h list, int** ids)
{
	calendar_record_h record;
	int ret = 0;
	int count = 0;
	int i=0;
	int *id = NULL;

	ret = calendar_list_get_count(list, &count);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("list get error");
		return ret;
	}

	id = calloc(1, sizeof(int)*count);
	RETVM_IF(NULL == id, CALENDAR_ERROR_OUT_OF_MEMORY, "calloc() Fail");

	ret = calendar_list_first(list);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("calendar_list_first() Fail(%d)", ret);
		CAL_FREE(id);
		return ret;
	}
	do {
		if (CALENDAR_ERROR_NONE == calendar_list_get_current_record_p(list, &record)) {
			ret = _cal_db_timezone_insert_record(record, &id[i]);
			if (CALENDAR_ERROR_NONE != ret) {
				ERR("_cal_db_timezone_insert_record() Fail(%d)", ret);
				CAL_FREE(id);
				return CALENDAR_ERROR_DB_FAILED;
			}
		}
		i++;
	} while (CALENDAR_ERROR_NO_DATA != calendar_list_next(list));

	if (ids) {
		*ids = id;
	}
	else {
		CAL_FREE(id);
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_db_timezone_update_records(const calendar_list_h list)
{
	calendar_record_h record;
	int ret = 0;

	ret = calendar_list_first(list);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("calendar_list_first() Fail(%d)", ret);
		return ret;
	}
	do {
		if (CALENDAR_ERROR_NONE == calendar_list_get_current_record_p(list, &record)) {
			ret = _cal_db_timezone_update_record(record);
			if (CALENDAR_ERROR_NONE != ret) {
				ERR("_cal_db_timezone_update_record() Fail(%d)", ret);
				return CALENDAR_ERROR_DB_FAILED;
			}
		}
	} while (CALENDAR_ERROR_NO_DATA != calendar_list_next(list));

	return CALENDAR_ERROR_NONE;
}

static int _cal_db_timezone_delete_records(int ids[], int count)
{
	int ret = 0;
	int i = 0;
	for(i = 0; i < count; i++) {
		ret = _cal_db_timezone_delete_record(ids[i]);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("_cal_db_timezone_delete_record() Fail(%d)", ret);
			return CALENDAR_ERROR_DB_FAILED;
		}
	}
	return CALENDAR_ERROR_NONE;
}

static int _cal_db_timezone_replace_records(const calendar_list_h list, int ids[], int count)
{
	calendar_record_h record;
	int i;
	int ret = 0;

	RETV_IF(NULL == list, CALENDAR_ERROR_INVALID_PARAMETER);

	ret = calendar_list_first(list);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("list first error");
		return ret;
	}

	for (i = 0; i < count; i++) {
		if (CALENDAR_ERROR_NONE == calendar_list_get_current_record_p(list, &record)) {
			ret = _cal_db_timezone_replace_record(record, ids[i]);
			if (CALENDAR_ERROR_NONE != ret) {
				ERR("_cal_db_timezone_replace_record() Fail(%d)", ret);
				return CALENDAR_ERROR_DB_FAILED;
			}
		}
		if (CALENDAR_ERROR_NO_DATA != calendar_list_next(list)) {
			break;
		}
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_db_timezone_get_count(int *out_count)
{
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	int count = 0;
	int ret;

	RETV_IF(NULL == out_count, CALENDAR_ERROR_INVALID_PARAMETER);

	snprintf(query, sizeof(query), "SELECT count(*) FROM %s where "
			"calendar_id IN (select id from %s where deleted = 0)",
			CAL_TABLE_TIMEZONE,
			CAL_TABLE_CALENDAR);

	ret = cal_db_util_query_get_first_int_result(query, NULL, &count);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_query_get_first_int_result() Fail");
		return ret;
	}
	DBG("%s=%d",query,count);

	*out_count = count;
	return CALENDAR_ERROR_NONE;
}

static int _cal_db_timezone_get_count_with_query(calendar_query_h query, int *out_count)
{
	cal_query_s *que = NULL;
	int ret = CALENDAR_ERROR_NONE;
	char *condition = NULL;
	char *query_str = NULL;
	char *table_name;
	int count = 0;
	GSList *bind_text = NULL;

	que = (cal_query_s *)query;

	if (CAL_STRING_EQUAL == strcmp(que->view_uri, CALENDAR_VIEW_TIMEZONE)) {
		table_name = cal_strdup(CAL_TABLE_TIMEZONE);
	}
	else {
		ERR("uri(%s) not support get records with query",que->view_uri);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	/* make filter */
	if (que->filter) {
		ret = cal_db_query_create_condition(query, &condition, &bind_text);
		if (CALENDAR_ERROR_NONE != ret) {
			CAL_FREE(table_name);
			ERR("cal_db_query_create_condition() Fail(%d), ret");
			return ret;
		}
	}

	/* query: select */
	cal_db_append_string(&query_str, "SELECT count(*) FROM");
	cal_db_append_string(&query_str, table_name);
	CAL_FREE(table_name);

	/* query: condition */
	if (condition) {
		cal_db_append_string(&query_str, "WHERE");
		cal_db_append_string(&query_str, condition);
		cal_db_append_string(&query_str, "AND calendar_id IN (select id from");
		cal_db_append_string(&query_str, CAL_TABLE_CALENDAR);
		cal_db_append_string(&query_str, "where deleted = 0)");
		CAL_FREE(condition);
	}
	else {
		cal_db_append_string(&query_str, "WHERE calendar_id IN (select id from");
		cal_db_append_string(&query_str, CAL_TABLE_CALENDAR);
		cal_db_append_string(&query_str, "where deleted = 0)");
	}

	/* query */
	ret = cal_db_util_query_get_first_int_result(query_str, bind_text, &count);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_query_get_first_int_result() Fail");
		if (bind_text) {
			g_slist_free_full(bind_text, free);
			bind_text = NULL;
		}
		CAL_FREE(query_str);
		return ret;
	}
	DBG("%s=%d",query_str,count);

	*out_count = count;

	if (bind_text) {
		g_slist_free_full(bind_text, free);
		bind_text = NULL;
	}
	CAL_FREE(query_str);
	return CALENDAR_ERROR_NONE;
}

static void _cal_db_timezone_get_stmt(sqlite3_stmt *stmt,calendar_record_h record)
{
	cal_timezone_s* timezone =  (cal_timezone_s*)(record);
	int count = 0;
	const unsigned char *temp;

	timezone->index = sqlite3_column_int(stmt, count++);
	timezone->tz_offset_from_gmt = sqlite3_column_int(stmt, count++);

	temp = sqlite3_column_text(stmt, count++);
	timezone->standard_name = cal_strdup((const char *)temp);

	timezone->std_start_month = sqlite3_column_int(stmt, count++);
	timezone->std_start_position_of_week = sqlite3_column_int(stmt, count++);
	timezone->std_start_day = sqlite3_column_int(stmt, count++);
	timezone->std_start_hour = sqlite3_column_int(stmt, count++);
	timezone->standard_bias = sqlite3_column_int(stmt, count++);

	temp = sqlite3_column_text(stmt, count++);
	timezone->day_light_name = cal_strdup((const char *)temp);

	timezone->day_light_start_month = sqlite3_column_int(stmt, count++);
	timezone->day_light_start_position_of_week = sqlite3_column_int(stmt, count++);
	timezone->day_light_start_day = sqlite3_column_int(stmt, count++);
	timezone->day_light_start_hour = sqlite3_column_int(stmt, count++);
	timezone->day_light_bias = sqlite3_column_int(stmt, count++);

	timezone->calendar_id = sqlite3_column_int(stmt, count++);
}

static void _cal_db_timezone_get_property_stmt(sqlite3_stmt *stmt,
		unsigned int property, int stmt_count, calendar_record_h record)
{
	cal_timezone_s* timezone =  (cal_timezone_s*)(record);
	const unsigned char *temp;

	switch (property) {
	case CAL_PROPERTY_TIMEZONE_ID:
		timezone->index = sqlite3_column_int(stmt, stmt_count);
		break;
	case CAL_PROPERTY_TIMEZONE_TZ_OFFSET_FROM_GMT:
		timezone->tz_offset_from_gmt = sqlite3_column_int(stmt, stmt_count);
		break;
	case CAL_PROPERTY_TIMEZONE_STANDARD_NAME:
		temp = sqlite3_column_text(stmt, stmt_count);
		timezone->standard_name = cal_strdup((const char *)temp);
		break;
	case CAL_PROPERTY_TIMEZONE_STD_START_MONTH:
		timezone->std_start_month = sqlite3_column_int(stmt, stmt_count);
		break;
	case CAL_PROPERTY_TIMEZONE_STD_START_POSITION_OF_WEEK:
		timezone->std_start_position_of_week = sqlite3_column_int(stmt, stmt_count);
		break;
	case CAL_PROPERTY_TIMEZONE_STD_START_DAY:
		timezone->std_start_day = sqlite3_column_int(stmt, stmt_count);
		break;
	case CAL_PROPERTY_TIMEZONE_STD_START_HOUR:
		timezone->std_start_hour = sqlite3_column_int(stmt, stmt_count);
		break;
	case CAL_PROPERTY_TIMEZONE_STANDARD_BIAS:
		timezone->standard_bias = sqlite3_column_int(stmt, stmt_count);
		break;
	case CAL_PROPERTY_TIMEZONE_DAY_LIGHT_NAME:
		temp = sqlite3_column_text(stmt, stmt_count);
		timezone->day_light_name = cal_strdup((const char *)temp);
		break;
	case CAL_PROPERTY_TIMEZONE_DAY_LIGHT_START_MONTH:
		timezone->day_light_start_month = sqlite3_column_int(stmt, stmt_count);
		break;
	case CAL_PROPERTY_TIMEZONE_DAY_LIGHT_START_POSITION_OF_WEEK:
		timezone->day_light_start_position_of_week = sqlite3_column_int(stmt, stmt_count);
		break;
	case CAL_PROPERTY_TIMEZONE_DAY_LIGHT_START_DAY:
		timezone->day_light_start_day = sqlite3_column_int(stmt, stmt_count);
		break;
	case CAL_PROPERTY_TIMEZONE_DAY_LIGHT_START_HOUR:
		timezone->day_light_start_hour = sqlite3_column_int(stmt, stmt_count);
		break;
	case CAL_PROPERTY_TIMEZONE_DAY_LIGHT_BIAS:
		timezone->day_light_bias = sqlite3_column_int(stmt, stmt_count);
		break;
	case CAL_PROPERTY_TIMEZONE_CALENDAR_ID:
		timezone->calendar_id = sqlite3_column_int(stmt, stmt_count);
		break;
	default:
		sqlite3_column_int(stmt, stmt_count);
		break;
	}

	return;
}

static void _cal_db_timezone_get_projection_stmt(sqlite3_stmt *stmt,
		const unsigned int *projection, const int projection_count,
		calendar_record_h record)
{
	int i=0;

	for(i=0;i<projection_count;i++) {
		_cal_db_timezone_get_property_stmt(stmt,projection[i],i,record);
	}
}

static int _cal_db_timezone_update_projection(calendar_record_h record)
{
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	sqlite3_stmt *stmt = NULL;
	cal_timezone_s* timezone =  (cal_timezone_s*)(record);
	int ret = 0;
	char* set = NULL;
	GSList *bind_text = NULL;
	GSList *cursor = NULL;

	ret = cal_db_query_create_projection_update_set(record,&set,&bind_text);
	RETV_IF(CALENDAR_ERROR_NONE != ret, ret);

	snprintf(query, sizeof(query), "UPDATE %s SET %s WHERE id = %d",
			CAL_TABLE_TIMEZONE, set, timezone->index);

	ret = cal_db_util_query_prepare(query, &stmt);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_query_prepare() Fail(%d)", ret);
		SECURE("query[%s]", query);
		free(set);
		if (bind_text) {
			g_slist_free_full(bind_text, free);
			bind_text = NULL;
		}
		return ret;
	}

	if (bind_text) {
		int i = 0;
		for (cursor=bind_text, i=1; cursor;cursor=cursor->next, i++) {
			cal_db_util_stmt_bind_text(stmt, i, cursor->data);
		}
	}

	ret = cal_db_util_stmt_step(stmt);
	sqlite3_finalize(stmt);
	free(set);
	if (bind_text) {
		g_slist_free_full(bind_text, free);
		bind_text = NULL;
	}
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_stmt_step() Fail(%d)", ret);
		return ret;
	}

	return CALENDAR_ERROR_NONE;
}
