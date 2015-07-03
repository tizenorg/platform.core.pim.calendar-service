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

#include "calendar_db.h"  //calendar_db_get_record

#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_view.h"
#include "cal_record.h"

#include "cal_db_util.h"
#include "cal_db.h"
#include "cal_db_query.h"

static int __cal_db_timezone_insert_record( calendar_record_h record, int* id );
static int __cal_db_timezone_get_record( int id, calendar_record_h* out_record );
static int __cal_db_timezone_update_record( calendar_record_h record );
static int __cal_db_timezone_delete_record( int id );
static int __cal_db_timezone_get_all_records( int offset, int limit, calendar_list_h* out_list );
static int __cal_db_timezone_get_records_with_query( calendar_query_h query, int offset, int limit, calendar_list_h* out_list );
static int __cal_db_timezone_insert_records(const calendar_list_h list, int** ids);
static int __cal_db_timezone_update_records(const calendar_list_h list);
static int __cal_db_timezone_delete_records(int ids[], int count);
static int __cal_db_timezone_get_count(int *out_count);
static int __cal_db_timezone_get_count_with_query(calendar_query_h query, int *out_count);
static int __cal_db_timezone_replace_record(calendar_record_h record, int id);
static int __cal_db_timezone_replace_records(const calendar_list_h list, int ids[], int count);

/*
 * static function
 */
static void __cal_db_timezone_get_stmt(sqlite3_stmt *stmt,calendar_record_h record);
static void __cal_db_timezone_get_property_stmt(sqlite3_stmt *stmt,
		unsigned int property, int stmt_count, calendar_record_h record);
static void __cal_db_timezone_get_projection_stmt(sqlite3_stmt *stmt,
		const unsigned int *projection, const int projection_count,
		calendar_record_h record);
static void __cal_db_timezone_get_stmt(sqlite3_stmt *stmt,calendar_record_h record);
static int __cal_db_timezone_update_projection(calendar_record_h record);

cal_db_plugin_cb_s _cal_db_timezone_plugin_cb = {
	.is_query_only=false,
	.insert_record=__cal_db_timezone_insert_record,
	.get_record=__cal_db_timezone_get_record,
	.update_record=__cal_db_timezone_update_record,
	.delete_record=__cal_db_timezone_delete_record,
	.get_all_records=__cal_db_timezone_get_all_records,
	.get_records_with_query=__cal_db_timezone_get_records_with_query,
	.insert_records=__cal_db_timezone_insert_records,
	.update_records=__cal_db_timezone_update_records,
	.delete_records=__cal_db_timezone_delete_records,
	.get_count=__cal_db_timezone_get_count,
	.get_count_with_query=__cal_db_timezone_get_count_with_query,
	.replace_record = __cal_db_timezone_replace_record,
	.replace_records = __cal_db_timezone_replace_records
};

static int __cal_db_timezone_insert_record( calendar_record_h record, int* id )
{
	int ret = CALENDAR_ERROR_NONE;
	int index;
	int calendar_book_id = 0;
	char query[CAL_DB_SQL_MAX_LEN];
	sqlite3_stmt *stmt;
	cal_timezone_s* timezone =  (cal_timezone_s*)(record);
	calendar_record_h record_calendar = NULL;
	cal_db_util_error_e dbret = CAL_DB_OK;

	retv_if(NULL == timezone, CALENDAR_ERROR_INVALID_PARAMETER);

	ret = calendar_record_get_int(record,
			_calendar_timezone.calendar_book_id, &calendar_book_id);
	DBG("calendar_book_id(%d)", calendar_book_id);

	ret = _cal_db_get_record(_calendar_book._uri,
			calendar_book_id, &record_calendar);
	retvm_if(CALENDAR_ERROR_NONE != ret, CALENDAR_ERROR_INVALID_PARAMETER, "calendar_book_id is invalid");
	calendar_record_destroy(record_calendar, true);

	// start >>>>> check if we already have
	/*
		if (timezone->standard_name == NULL || strlen(timezone->standard_name) == 0)
		{
		ERR("No timezone starndard name");
		return CALENDAR_ERROR_INVALID_PARAMETER;
		}
	 */
	if (timezone->standard_name)
	{
		snprintf(query, sizeof(query), "SELECT count(*), id FROM %s "
				"WHERE standard_name = ? ",
				CAL_TABLE_TIMEZONE);

		stmt = _cal_db_util_query_prepare(query);
		if (NULL == stmt)
		{
			ERR("_cal_db_util_query_prepare() failed");
			return CALENDAR_ERROR_DB_FAILED;
		}
		_cal_db_util_stmt_bind_text(stmt, 1, timezone->standard_name);

		dbret = _cal_db_util_stmt_step(stmt);
		if (CAL_DB_ROW != dbret)
		{
			ERR("_cal_db_util_stmt_step() failed");
			sqlite3_finalize(stmt);
			switch (dbret)
			{
			case CAL_DB_ERROR_NO_SPACE:
				return CALENDAR_ERROR_FILE_NO_SPACE;
			default:
				return CALENDAR_ERROR_DB_FAILED;
			}
		}

		index = 0;
		int count = sqlite3_column_int(stmt, index++);
		int timezone_id = sqlite3_column_int(stmt, index++);
		sqlite3_finalize(stmt);

		if (count > 0)
		{
			DBG("Already exist which tzid name[%s] id(%d)", timezone->standard_name, timezone_id);
			*id = timezone_id;
			return CALENDAR_ERROR_NONE;
		}
		// end <<<<< check if we already have

		DBG("Not registered timezone in the table, so insert timezone.");
	}

	// if we don't have
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

	stmt = _cal_db_util_query_prepare(query);
	retvm_if(NULL == stmt, CALENDAR_ERROR_DB_FAILED, "_cal_db_util_query_prepare() Failed");

	if (timezone->standard_name)
		_cal_db_util_stmt_bind_text(stmt, 1, timezone->standard_name);

	if (timezone->day_light_name)
		_cal_db_util_stmt_bind_text(stmt, 2, timezone->day_light_name);

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

	//_cal_record_set_int(record, _calendar_timezone.id,index);
	if (id)
	{
		*id = index;
	}

	return CALENDAR_ERROR_NONE;
}

static int __cal_db_timezone_get_record( int id, calendar_record_h* out_record )
{
	char query[CAL_DB_SQL_MAX_LEN];
	sqlite3_stmt *stmt = NULL;
	cal_db_util_error_e dbret = CAL_DB_OK;
	int ret = 0;

	ret = calendar_record_create( _calendar_timezone._uri ,out_record);
	if (ret != CALENDAR_ERROR_NONE)
	{
		ERR("record create fail");
		return CALENDAR_ERROR_OUT_OF_MEMORY;
	}

	snprintf(query, sizeof(query), "SELECT * FROM %s WHERE id = %d AND "
			"calendar_id IN (select id from %s where deleted = 0)",
			CAL_TABLE_TIMEZONE, id,
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
	if (CAL_DB_ROW != dbret)
	{
		ERR("_cal_db_util_stmt_step() failed(%d)", dbret);
		sqlite3_finalize(stmt);
		calendar_record_destroy(*out_record, true);
		*out_record = NULL;
		switch (dbret)
		{
		case CAL_DB_DONE:
			return CALENDAR_ERROR_DB_RECORD_NOT_FOUND;
		case CAL_DB_ERROR_NO_SPACE:
			return CALENDAR_ERROR_FILE_NO_SPACE;
		default:
			return CALENDAR_ERROR_DB_FAILED;
		}
	}

	__cal_db_timezone_get_stmt(stmt,*out_record);

	sqlite3_finalize(stmt);
	stmt = NULL;

	return CALENDAR_ERROR_NONE;
}

static int __cal_db_timezone_update_record( calendar_record_h record )
{
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	sqlite3_stmt *stmt = NULL;
	cal_timezone_s* timezone_info =  (cal_timezone_s*)(record);
	cal_db_util_error_e dbret = CAL_DB_OK;

	retv_if(NULL == timezone_info, CALENDAR_ERROR_INVALID_PARAMETER);

	if (timezone_info->common.properties_flags != NULL)
	{
		return __cal_db_timezone_update_projection(record);
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

	stmt = _cal_db_util_query_prepare(query);
	retvm_if(NULL == stmt, CALENDAR_ERROR_DB_FAILED, "cal_q_cal_db_util_query_prepareuery_prepare() Failed");

	if (timezone_info->standard_name)
		_cal_db_util_stmt_bind_text(stmt, 1, timezone_info->standard_name);

	if (timezone_info->day_light_name)
		_cal_db_util_stmt_bind_text(stmt, 2, timezone_info->day_light_name);

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
	sqlite3_finalize(stmt);

	return CALENDAR_ERROR_NONE;
}

static int __cal_db_timezone_delete_record( int id )
{
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	cal_db_util_error_e dbret = CAL_DB_OK;

	snprintf(query, sizeof(query), "DELETE FROM %s WHERE id = %d",
			CAL_TABLE_TIMEZONE, id);
	dbret = _cal_db_util_query_exec(query);
	if(CAL_DB_OK != dbret)
	{
		ERR("_cal_db_util_query_exec() Failed(%d)", dbret);
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

static int __cal_db_timezone_replace_record(calendar_record_h record, int id)
{
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	sqlite3_stmt *stmt = NULL;
	cal_timezone_s* timezone_info =  (cal_timezone_s*)(record);
	cal_db_util_error_e dbret = CAL_DB_OK;

	retv_if(NULL == timezone_info, CALENDAR_ERROR_INVALID_PARAMETER);
	timezone_info->index = id;

	if (timezone_info->common.properties_flags != NULL)
	{
		return __cal_db_timezone_update_projection(record);
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

	stmt = _cal_db_util_query_prepare(query);
	retvm_if(NULL == stmt, CALENDAR_ERROR_DB_FAILED, "cal_q_cal_db_util_query_prepareuery_prepare() Failed");

	if (timezone_info->standard_name)
		_cal_db_util_stmt_bind_text(stmt, 1, timezone_info->standard_name);

	if (timezone_info->day_light_name)
		_cal_db_util_stmt_bind_text(stmt, 2, timezone_info->day_light_name);

	dbret = _cal_db_util_stmt_step(stmt);
	sqlite3_finalize(stmt);
	if (CAL_DB_DONE != dbret)
	{
		ERR("_cal_db_util_stmt_step() Failed(%d)", dbret);
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

static int __cal_db_timezone_get_all_records( int offset, int limit, calendar_list_h* out_list )
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
	snprintf(query, sizeof(query), "SELECT * FROM %s where "
			"calendar_id IN (select id from %s where deleted = 0) "
			"%s %s",
			CAL_TABLE_TIMEZONE,
			CAL_TABLE_CALENDAR,
			limitquery,
			offsetquery);

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
		ret = calendar_record_create(_calendar_timezone._uri,&record);
		if( ret != CALENDAR_ERROR_NONE )
		{
			calendar_list_destroy(*out_list, true);
			*out_list = NULL;
			sqlite3_finalize(stmt);
			return ret;
		}
		__cal_db_timezone_get_stmt(stmt,record);

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

static int __cal_db_timezone_get_records_with_query( calendar_query_h query, int offset, int limit, calendar_list_h* out_list )
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

	// make filter
	if (que->filter)
	{
		ret = _cal_db_query_create_condition(query, &condition, &bind_text);
		if (ret != CALENDAR_ERROR_NONE)
		{
			ERR("filter create fail");
			return ret;
		}
	}

	// make projection
	ret = _cal_db_query_create_projection(query, &projection);

	// query - projection
	if (projection)
	{
		_cal_db_append_string(&query_str, "SELECT");
		_cal_db_append_string(&query_str, projection);
		_cal_db_append_string(&query_str, "FROM");
		_cal_db_append_string(&query_str, CAL_TABLE_TIMEZONE);
		CAL_FREE(projection);
	}
	else
	{
		_cal_db_append_string(&query_str, "SELECT * FROM");
		_cal_db_append_string(&query_str, CAL_TABLE_TIMEZONE);
	}

	// query - condition
	if (condition)
	{
		_cal_db_append_string(&query_str, "WHERE");
		_cal_db_append_string(&query_str, condition);
		CAL_FREE(condition);
		_cal_db_append_string(&query_str, "AND calendar_id IN (select id from");
		_cal_db_append_string(&query_str, CAL_TABLE_CALENDAR);
		_cal_db_append_string(&query_str, "where deleted = 0)");
	}
	else
	{
		_cal_db_append_string(&query_str, "WHERE calendar_id IN (select id from");
		_cal_db_append_string(&query_str, CAL_TABLE_CALENDAR);
		_cal_db_append_string(&query_str, "where deleted = 0)");
	}

	// ORDER
	ret = _cal_db_query_create_order(query, condition, &order);
	if (order)
	{
		_cal_db_append_string(&query_str, order);
		CAL_FREE(order);
	}

	char buf[32] = {0};
	if (0 < limit)
	{
		snprintf(buf, sizeof(buf), "LIMIT %d", limit);
		_cal_db_append_string(&query_str, buf);
		if (0 < offset)
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

	// bind text
	if (bind_text)
	{
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
		CAL_FREE(query_str);
		ERR("calendar_list_create() Failed");
		sqlite3_finalize(stmt);
		return ret;
	}

	while(CAL_DB_ROW == _cal_db_util_stmt_step(stmt))
	{
		calendar_record_h record;
		// stmt -> record
		ret = calendar_record_create(_calendar_timezone._uri,&record);
		if( ret != CALENDAR_ERROR_NONE )
		{
			calendar_list_destroy(*out_list, true);
			*out_list = NULL;

			if (bind_text)
			{
				g_slist_free_full(bind_text, free);
				bind_text = NULL;
			}
			CAL_FREE(query_str);
			sqlite3_finalize(stmt);
			return ret;
		}
		if (que->projection_count > 0)
		{
			_cal_record_set_projection(record,
					que->projection, que->projection_count, que->property_count);

			__cal_db_timezone_get_projection_stmt(stmt,
					que->projection, que->projection_count,
					record);
		}
		else
		{
			__cal_db_timezone_get_stmt(stmt,record);
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
			CAL_FREE(query_str);
			sqlite3_finalize(stmt);
			return ret;
		}
	}

	if (bind_text)
	{
		g_slist_free_full(bind_text, free);
		bind_text = NULL;
	}
	CAL_FREE(query_str);
	sqlite3_finalize(stmt);

	return CALENDAR_ERROR_NONE;
}
static int __cal_db_timezone_insert_records(const calendar_list_h list, int** ids)
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
			if( __cal_db_timezone_insert_record(record, &id[i]) != CALENDAR_ERROR_NONE)
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

static int __cal_db_timezone_update_records(const calendar_list_h list)
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
			if( __cal_db_timezone_update_record(record) != CALENDAR_ERROR_NONE)
			{
				ERR("db insert error");
				return CALENDAR_ERROR_DB_FAILED;
			}
		}
	} while(calendar_list_next(list) != CALENDAR_ERROR_NO_DATA);

	return CALENDAR_ERROR_NONE;
}

static int __cal_db_timezone_delete_records(int ids[], int count)
{
	int i = 0;

	for(i = 0; i < count; i++)
	{
		if (__cal_db_timezone_delete_record(ids[i]) != CALENDAR_ERROR_NONE)
		{
			ERR("delete failed");
			return CALENDAR_ERROR_DB_FAILED;
		}
	}
	return CALENDAR_ERROR_NONE;
}

static int __cal_db_timezone_replace_records(const calendar_list_h list, int ids[], int count)
{
	calendar_record_h record;
	int i;
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
			if( __cal_db_timezone_replace_record(record, ids[i]) != CALENDAR_ERROR_NONE)
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

static int __cal_db_timezone_get_count(int *out_count)
{
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	int count = 0;
	int ret;

	retvm_if(NULL == out_count, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");

	snprintf(query, sizeof(query), "SELECT count(*) FROM %s where "
			"calendar_id IN (select id from %s where deleted = 0)",
			CAL_TABLE_TIMEZONE,
			CAL_TABLE_CALENDAR);

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

static int __cal_db_timezone_get_count_with_query(calendar_query_h query, int *out_count)
{
	cal_query_s *que = NULL;
	int ret = CALENDAR_ERROR_NONE;
	char *condition = NULL;
	char *query_str = NULL;
	char *table_name;
	int count = 0;
	GSList *bind_text = NULL;

	que = (cal_query_s *)query;

	if ( 0 == strcmp(que->view_uri, CALENDAR_VIEW_TIMEZONE))
	{
		table_name = SAFE_STRDUP(CAL_TABLE_TIMEZONE);
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

	// query - select from
	_cal_db_append_string(&query_str, "SELECT count(*) FROM");
	_cal_db_append_string(&query_str, table_name);
	CAL_FREE(table_name);

	// query - condition
	if (condition)
	{
		_cal_db_append_string(&query_str, "WHERE");
		_cal_db_append_string(&query_str, condition);
		_cal_db_append_string(&query_str, "AND calendar_id IN (select id from");
		_cal_db_append_string(&query_str, CAL_TABLE_CALENDAR);
		_cal_db_append_string(&query_str, "where deleted = 0)");
		CAL_FREE(condition);
	}
	else
	{
		_cal_db_append_string(&query_str, "WHERE calendar_id IN (select id from");
		_cal_db_append_string(&query_str, CAL_TABLE_CALENDAR);
		_cal_db_append_string(&query_str, "where deleted = 0)");
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
	CAL_DBG("%s=%d",query_str,count);

	*out_count = count;

	if (bind_text)
	{
		g_slist_free_full(bind_text, free);
		bind_text = NULL;
	}
	CAL_FREE(query_str);
	return CALENDAR_ERROR_NONE;
}

static void __cal_db_timezone_get_stmt(sqlite3_stmt *stmt,calendar_record_h record)
{
	cal_timezone_s* timezone =  (cal_timezone_s*)(record);
	int count = 0;
	const unsigned char *temp;

	timezone->index = sqlite3_column_int(stmt, count++);
	timezone->tz_offset_from_gmt = sqlite3_column_int(stmt, count++);

	temp = sqlite3_column_text(stmt, count++);
	timezone->standard_name = SAFE_STRDUP(temp);

	timezone->std_start_month = sqlite3_column_int(stmt, count++);
	timezone->std_start_position_of_week = sqlite3_column_int(stmt, count++);
	timezone->std_start_day = sqlite3_column_int(stmt, count++);
	timezone->std_start_hour = sqlite3_column_int(stmt, count++);
	timezone->standard_bias = sqlite3_column_int(stmt, count++);

	temp = sqlite3_column_text(stmt, count++);
	timezone->day_light_name = SAFE_STRDUP(temp);

	timezone->day_light_start_month = sqlite3_column_int(stmt, count++);
	timezone->day_light_start_position_of_week = sqlite3_column_int(stmt, count++);
	timezone->day_light_start_day = sqlite3_column_int(stmt, count++);
	timezone->day_light_start_hour = sqlite3_column_int(stmt, count++);
	timezone->day_light_bias = sqlite3_column_int(stmt, count++);

	timezone->calendar_id = sqlite3_column_int(stmt, count++);
}

static void __cal_db_timezone_get_property_stmt(sqlite3_stmt *stmt,
		unsigned int property, int stmt_count, calendar_record_h record)
{
	cal_timezone_s* timezone =  (cal_timezone_s*)(record);
	const unsigned char *temp;

	switch(property)
	{
	case CAL_PROPERTY_TIMEZONE_ID:
		timezone->index = sqlite3_column_int(stmt, stmt_count);
		break;
	case CAL_PROPERTY_TIMEZONE_TZ_OFFSET_FROM_GMT:
		timezone->tz_offset_from_gmt = sqlite3_column_int(stmt, stmt_count);
		break;
	case CAL_PROPERTY_TIMEZONE_STANDARD_NAME:
		temp = sqlite3_column_text(stmt, stmt_count);
		timezone->standard_name = SAFE_STRDUP(temp);
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
		timezone->day_light_name = SAFE_STRDUP(temp);
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

static void __cal_db_timezone_get_projection_stmt(sqlite3_stmt *stmt,
		const unsigned int *projection, const int projection_count,
		calendar_record_h record)
{
	int i=0;

	for(i=0;i<projection_count;i++)
	{
		__cal_db_timezone_get_property_stmt(stmt,projection[i],i,record);
	}
}

static int __cal_db_timezone_update_projection(calendar_record_h record)
{
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	sqlite3_stmt *stmt = NULL;
	cal_timezone_s* timezone =  (cal_timezone_s*)(record);
	cal_db_util_error_e dbret = CAL_DB_OK;
	int ret = CALENDAR_ERROR_NONE;
	char* set = NULL;
	GSList *bind_text = NULL;
	GSList *cursor = NULL;

	ret = _cal_db_query_create_projection_update_set(record,&set,&bind_text);
	retv_if(CALENDAR_ERROR_NONE != ret, ret);

	snprintf(query, sizeof(query), "UPDATE %s SET %s "
			"WHERE id = %d",
			CAL_TABLE_TIMEZONE,set,
			timezone->index);

	CAL_DBG("%s",query);
	stmt = _cal_db_util_query_prepare(query);
	if (NULL == stmt) {
		ERR("_cal_db_util_query_prepare() Failed");
		CAL_FREE(set);
		if(bind_text)
		{
			g_slist_free_full(bind_text, free);
			bind_text = NULL;
		}
		return CALENDAR_ERROR_DB_FAILED;
	}

	// bind
	if (bind_text)
	{
		int i = 0;
		for (cursor=bind_text, i=1; cursor;cursor=cursor->next, i++)
		{
			_cal_db_util_stmt_bind_text(stmt, i, cursor->data);
		}
	}

	dbret = _cal_db_util_stmt_step(stmt);
	if (CAL_DB_DONE != dbret)
	{
		sqlite3_finalize(stmt);
		ERR("_cal_db_util_stmt_step() Failed(%d)", dbret);

		CAL_FREE(set);
		if(bind_text)
		{
			g_slist_free_full(bind_text, free);
			bind_text = NULL;
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

	CAL_FREE(set);
	if(bind_text)
	{
		g_slist_free_full(bind_text, free);
		bind_text = NULL;
	}

	return CALENDAR_ERROR_NONE;
}
