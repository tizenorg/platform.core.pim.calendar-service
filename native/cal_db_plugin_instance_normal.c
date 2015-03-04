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

#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_view.h"
#include "cal_record.h"

#include "cal_db_util.h"
#include "cal_db.h"
#include "cal_db_query.h"
#include "cal_access_control.h"

//static int __cal_db_instance_normal_get_record(int id, calendar_record_h* out_record);
//static int __cal_db_instance_normal_update_record(calendar_record_h record);
static int __cal_db_instance_normal_delete_record(int id);
static int __cal_db_instance_normal_get_all_records(int offset, int limit, calendar_list_h* out_list);
static int __cal_db_instance_normal_get_records_with_query(calendar_query_h query, int offset, int limit, calendar_list_h* out_list);
//static int __cal_db_instance_normal_insert_records(const calendar_list_h list);
//static int __cal_db_instance_normal_update_records(const calendar_list_h list);
//static int __cal_db_instance_normal_delete_records(int ids[], int count);
static int __cal_db_instance_normal_get_count(int *out_count);
static int __cal_db_instance_normal_get_count_with_query(calendar_query_h query, int *out_count);
/*
 * static function
 */
static void __cal_db_instance_normal_get_stmt(sqlite3_stmt *stmt,calendar_record_h record);
static void __cal_db_instance_normal_get_property_stmt(sqlite3_stmt *stmt,
		unsigned int property, int *stmt_count, calendar_record_h record);
static void __cal_db_instance_normal_get_projection_stmt(sqlite3_stmt *stmt,
		const unsigned int *projection, const int projection_count,
		calendar_record_h record);

cal_db_plugin_cb_s _cal_db_instance_normal_plugin_cb = {
	.is_query_only = false,
	.insert_record=NULL,
	.get_record=NULL,
	.update_record=NULL,
	.delete_record=__cal_db_instance_normal_delete_record,
	.get_all_records=__cal_db_instance_normal_get_all_records,
	.get_records_with_query=__cal_db_instance_normal_get_records_with_query,
	.insert_records=NULL,
	.update_records=NULL,
	.delete_records=NULL,
	.get_count=__cal_db_instance_normal_get_count,
	.get_count_with_query=__cal_db_instance_normal_get_count_with_query,
	.replace_record=NULL,
	.replace_records=NULL
};

static int __cal_db_instance_normal_delete_record(int id)
{
	cal_db_util_error_e dbret = CAL_DB_OK;
	char query[CAL_DB_SQL_MAX_LEN] = {0};

	retvm_if(id < 0, CALENDAR_ERROR_INVALID_PARAMETER,
			"Invalid argument: id(%d) < 0", id);

	snprintf(query, sizeof(query),
			"DELETE FROM %s "
			"WHERE event_id = %d ",
			CAL_TABLE_NORMAL_INSTANCE,
			id);

	dbret = _cal_db_util_query_exec(query);
	if (dbret != CAL_DB_OK)
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

static int __cal_db_instance_normal_get_all_records(int offset, int limit, calendar_list_h* out_list)
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
	_cal_db_append_string(&query_str, CAL_VIEW_TABLE_NORMAL_INSTANCE);
	_cal_db_append_string(&query_str, limitquery);
	_cal_db_append_string(&query_str, offsetquery);

	stmt = _cal_db_util_query_prepare(query_str);
	if (NULL == stmt)
	{
		SEC_ERR("[%s]", query_str);
		ERR("_cal_db_util_query_prepare() Failed");
		calendar_list_destroy(*out_list, true);
		*out_list = NULL;
		CAL_FREE(query_str);
		return CALENDAR_ERROR_DB_FAILED;
	}

	while(CAL_DB_ROW == _cal_db_util_stmt_step(stmt))
	{
		calendar_record_h record;
		// stmt -> record
		ret = calendar_record_create(_calendar_instance_utime_calendar_book._uri,&record);
		if( ret != CALENDAR_ERROR_NONE )
		{
			calendar_list_destroy(*out_list, true);
			*out_list = NULL;
			sqlite3_finalize(stmt);
			CAL_FREE(query_str);
			return ret;
		}
		__cal_db_instance_normal_get_stmt(stmt, record);

		ret = calendar_list_add(*out_list, record);
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

static int __cal_db_instance_normal_get_records_with_query(calendar_query_h query, int offset, int limit, calendar_list_h* out_list)
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

	if (0 == strcmp(que->view_uri, CALENDAR_VIEW_INSTANCE_UTIME_CALENDAR))
	{
		table_name = SAFE_STRDUP(CAL_VIEW_TABLE_NORMAL_INSTANCE);
	}
	else if (0 == strcmp(que->view_uri, CALENDAR_VIEW_INSTANCE_UTIME_CALENDAR_EXTENDED))
	{
		table_name = SAFE_STRDUP(CAL_VIEW_TABLE_NORMAL_INSTANCE_EXTENDED);
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
		// stmt -> record
		ret = calendar_record_create(que->view_uri,&record);
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
			return ret;
		}
		if (que->projection_count > 0)
		{
			_cal_record_set_projection(record,
					que->projection, que->projection_count, que->property_count);

			__cal_db_instance_normal_get_projection_stmt(stmt,
					que->projection, que->projection_count,
					record);
		}
		else
		{
			__cal_db_instance_normal_get_stmt(stmt,record);
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

static int __cal_db_instance_normal_get_count(int *out_count)
{
	retvm_if(NULL == out_count, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");

	char *query_str = NULL;
	_cal_db_append_string(&query_str, "SELECT count(*) FROM");
	_cal_db_append_string(&query_str, CAL_TABLE_NORMAL_INSTANCE);

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

static int __cal_db_instance_normal_get_count_with_query(calendar_query_h query, int *out_count)
{
	cal_query_s *que = NULL;
	int ret = CALENDAR_ERROR_NONE;
	char *condition = NULL;
	char *table_name;
	int count = 0;
	GSList *bind_text = NULL;

	que = (cal_query_s *)query;

	if (0 == strcmp(que->view_uri, CALENDAR_VIEW_INSTANCE_UTIME_CALENDAR))
	{
		table_name = SAFE_STRDUP(CAL_VIEW_TABLE_NORMAL_INSTANCE);
	}
	else if (0 == strcmp(que->view_uri, CALENDAR_VIEW_INSTANCE_UTIME_CALENDAR_EXTENDED))
	{
		table_name = SAFE_STRDUP(CAL_VIEW_TABLE_NORMAL_INSTANCE_EXTENDED);
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

static void __cal_db_instance_normal_get_stmt(sqlite3_stmt *stmt, calendar_record_h record)
{
	cal_instance_normal_s* instance =  (cal_instance_normal_s*)(record);
	const unsigned char *temp;
	int count = 0;

	instance->event_id = sqlite3_column_int(stmt, count++);
	instance->start.type = sqlite3_column_int(stmt, count++);
	instance->start.time.utime = sqlite3_column_int64(stmt, count++);
	count++; // datetime
	instance->end.type = sqlite3_column_int(stmt, count++);
	instance->end.time.utime = sqlite3_column_int64(stmt, count++);
	count++;  //datetime

	temp = sqlite3_column_text(stmt, count++);
	instance->summary = SAFE_STRDUP(temp);

	temp = sqlite3_column_text(stmt, count++);
	instance->description = SAFE_STRDUP(temp);

	temp = sqlite3_column_text(stmt, count++);
	instance->location = SAFE_STRDUP(temp);

	instance->busy_status = sqlite3_column_int(stmt, count++);

	instance->event_status = sqlite3_column_int(stmt, count++);

	instance->priority = sqlite3_column_int(stmt, count++);

	instance->sensitivity = sqlite3_column_int(stmt, count++);

	instance->has_rrule = sqlite3_column_int(stmt, count++);
	if (instance->has_rrule > 0)
	{
		instance->has_rrule = 1;
	}

	instance->latitude = sqlite3_column_double(stmt,count++);
	instance->longitude = sqlite3_column_double(stmt,count++);
	instance->has_alarm = sqlite3_column_int(stmt,count++);
	instance->original_event_id = sqlite3_column_int(stmt, count++);
	instance->calendar_id = sqlite3_column_int(stmt, count++);
	instance->last_mod = sqlite3_column_int64(stmt, count++);

	temp = sqlite3_column_text(stmt, count++);
	instance->sync_data1 = SAFE_STRDUP(temp);

	return;
}

static void __cal_db_instance_normal_get_property_stmt(sqlite3_stmt *stmt,
		unsigned int property, int *stmt_count, calendar_record_h record)
{
	cal_instance_normal_s* instance =  (cal_instance_normal_s*)(record);
	const unsigned char *temp;

	switch(property)
	{
	case CAL_PROPERTY_INSTANCE_NORMAL_START:
		instance->start.type = CALENDAR_TIME_UTIME;
		*stmt_count = *stmt_count+1;
		instance->start.time.utime = sqlite3_column_int64(stmt, *stmt_count);
		*stmt_count = *stmt_count+1; // datetime
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_END:
		instance->end.type = CALENDAR_TIME_UTIME;
		sqlite3_column_int(stmt,*stmt_count);
		*stmt_count = *stmt_count+1;
		instance->end.time.utime = sqlite3_column_int64(stmt, *stmt_count);
		*stmt_count = *stmt_count+1; // datetime
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_SUMMARY:
		temp = sqlite3_column_text(stmt, *stmt_count);
		instance->summary = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_LOCATION:
		temp = sqlite3_column_text(stmt, *stmt_count);
		instance->location = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_CALENDAR_ID:
		instance->calendar_id = sqlite3_column_int(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_DESCRIPTION:
		temp = sqlite3_column_text(stmt, *stmt_count);
		instance->description = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_BUSY_STATUS:
		instance->busy_status = sqlite3_column_int(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_EVENT_STATUS:
		instance->event_status = sqlite3_column_int(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_PRIORITY:
		instance->priority = sqlite3_column_int(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_SENSITIVITY:
		instance->sensitivity = sqlite3_column_int(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_HAS_RRULE:
		instance->has_rrule = sqlite3_column_int(stmt, *stmt_count);
		if (instance->has_rrule > 0)
		{
			instance->has_rrule = 1;
		}
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_LATITUDE:
		instance->latitude = sqlite3_column_double(stmt,*stmt_count);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_LONGITUDE:
		instance->longitude = sqlite3_column_double(stmt,*stmt_count);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_EVENT_ID:
		instance->event_id = sqlite3_column_int(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_HAS_ALARM:
		instance->has_alarm = sqlite3_column_int(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_ORIGINAL_EVENT_ID:
		instance->original_event_id = sqlite3_column_int(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_LAST_MODIFIED_TIME:
		instance->last_mod = sqlite3_column_int64(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_SYNC_DATA1:
		temp = sqlite3_column_text(stmt, *stmt_count);
		instance->sync_data1 = SAFE_STRDUP(temp);
		break;
	default:
		sqlite3_column_int(stmt, *stmt_count);
		break;
	}

	*stmt_count = *stmt_count+1;

	return;
}

static void __cal_db_instance_normal_get_projection_stmt(sqlite3_stmt *stmt,
		const unsigned int *projection, const int projection_count,
		calendar_record_h record)
{
	int i=0;
	int stmt_count = 0;

	for(i=0;i<projection_count;i++)
	{
		__cal_db_instance_normal_get_property_stmt(stmt,projection[i],&stmt_count,record);
	}
}
