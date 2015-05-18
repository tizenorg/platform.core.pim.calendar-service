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
#include "cal_access_control.h"
#include "cal_utils.h"

static int _cal_db_instance_allday_extended_get_all_records(int offset, int limit, calendar_list_h* out_list);
static int _cal_db_instance_allday_extended_get_records_with_query(calendar_query_h query, int offset, int limit, calendar_list_h* out_list);
static int _cal_db_instance_allday_extended_get_count(int *out_count);
static int _cal_db_instance_allday_extended_get_count_with_query(calendar_query_h query, int *out_count);

/*
 * static function
 */
static void _cal_db_instance_allday_extended_get_stmt(sqlite3_stmt *stmt,calendar_record_h record);
static void _cal_db_instance_allday_extended_get_property_stmt(sqlite3_stmt *stmt,
		unsigned int property, int *stmt_count, calendar_record_h record);
static void _cal_db_instance_allday_extended_get_projection_stmt(sqlite3_stmt *stmt,
		const unsigned int *projection, const int projection_count,
		calendar_record_h record);

cal_db_plugin_cb_s cal_db_instance_allday_extended_plugin_cb = {
	.is_query_only = false,
	.insert_record= NULL,
	.get_record= NULL,
	.update_record=NULL,
	.delete_record=NULL,
	.get_all_records=_cal_db_instance_allday_extended_get_all_records,
	.get_records_with_query=_cal_db_instance_allday_extended_get_records_with_query,
	.insert_records=NULL,
	.update_records=NULL,
	.delete_records=NULL,
	.get_count=_cal_db_instance_allday_extended_get_count,
	.get_count_with_query=_cal_db_instance_allday_extended_get_count_with_query,
	.replace_record=NULL,
	.replace_records=NULL
};

static int _cal_db_instance_allday_extended_get_all_records(int offset, int limit, calendar_list_h* out_list)
{
	int ret = 0;
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
	cal_db_append_string(&query_str, CAL_VIEW_TABLE_ALLDAY_INSTANCE_EXTENDED);
	cal_db_append_string(&query_str, limitquery);
	cal_db_append_string(&query_str, offsetquery);

	ret = cal_db_util_query_prepare(query_str, &stmt);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_query_prepare() Fail(%d)", ret);
		SECURE("query[%s]", query_str);
		calendar_list_destroy(*out_list, true);
		*out_list = NULL;
		free(query_str);
		return ret;
	}

	while (CAL_SQLITE_ROW == cal_db_util_stmt_step(stmt)) {
		calendar_record_h record;
		ret = calendar_record_create(_calendar_instance_localtime_calendar_book_extended._uri,&record);
		if (CALENDAR_ERROR_NONE != ret) {
			calendar_list_destroy(*out_list, true);
			*out_list = NULL;
			sqlite3_finalize(stmt);
			CAL_FREE(query_str);
			return ret;
		}
		_cal_db_instance_allday_extended_get_stmt(stmt,record);

		ret = calendar_list_add(*out_list,record);
		if (CALENDAR_ERROR_NONE != ret) {
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

static int _cal_db_instance_allday_extended_get_records_with_query(calendar_query_h query, int offset, int limit, calendar_list_h* out_list)
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

	table_name = cal_strdup(CAL_VIEW_TABLE_ALLDAY_INSTANCE_EXTENDED);

	/* make filter */
	if (que->filter) {
		ret = cal_db_query_create_condition(query, &condition, &bind_text);
		if (CALENDAR_ERROR_NONE != ret) {
			CAL_FREE(table_name);
			ERR("cal_db_query_create_condition() Fail(%d), ret");
			return ret;
		}
	}

	/* make: projection */
	ret = cal_db_query_create_projection(query, &projection);

	char *query_str = NULL;
	/* query: projection */
	if (projection) {
		cal_db_append_string(&query_str, "SELECT");
		cal_db_append_string(&query_str, projection);
		cal_db_append_string(&query_str, "FROM");
		cal_db_append_string(&query_str, table_name);
		CAL_FREE(projection);
	}
	else {
		cal_db_append_string(&query_str, "SELECT * FROM");
		cal_db_append_string(&query_str, table_name);
	}
	CAL_FREE(table_name);

	/* query: condition */
	if (condition) {
		cal_db_append_string(&query_str, "WHERE (");
		cal_db_append_string(&query_str, condition);
		cal_db_append_string(&query_str, ")");
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
	char buf[32] = {0};
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
		g_slist_length(bind_text);
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
		ERR("calendar_list_create() Fail");
		sqlite3_finalize(stmt);
		CAL_FREE(query_str);
		return ret;
	}

	while (CAL_SQLITE_ROW == cal_db_util_stmt_step(stmt)) {
		calendar_record_h record;
		ret = calendar_record_create(que->view_uri,&record);
		if (CALENDAR_ERROR_NONE != ret) {
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
		if (0 < que->projection_count) {
			cal_record_set_projection(record, que->projection, que->projection_count, que->property_count);
			_cal_db_instance_allday_extended_get_projection_stmt(stmt, que->projection, que->projection_count, record);
		}
		else {
			_cal_db_instance_allday_extended_get_stmt(stmt,record);
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

static int _cal_db_instance_allday_extended_get_count(int *out_count)
{
	RETV_IF(NULL == out_count, CALENDAR_ERROR_INVALID_PARAMETER);

	char *query_str = NULL;
	cal_db_append_string(&query_str, "SELECT count(*) FROM");
	cal_db_append_string(&query_str, CAL_VIEW_TABLE_ALLDAY_INSTANCE_EXTENDED);

	int ret = 0;
	int count = 0;
	ret = cal_db_util_query_get_first_int_result(query_str, NULL, &count);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_query_get_first_int_result() Fail");
		CAL_FREE(query_str);
		return ret;
	}
	DBG("count(%d) str[%s]", count, query_str);
	CAL_FREE(query_str);

	*out_count = count;
	return CALENDAR_ERROR_NONE;
}

static int _cal_db_instance_allday_extended_get_count_with_query(calendar_query_h query, int *out_count)
{
	cal_query_s *que = NULL;
	int ret = CALENDAR_ERROR_NONE;
	char *condition = NULL;
	char *table_name;
	int count = 0;
	GSList *bind_text = NULL;

	que = (cal_query_s *)query;

	table_name = cal_strdup(CAL_VIEW_TABLE_ALLDAY_INSTANCE_EXTENDED);

	/* make filter */
	if (que->filter) {
		ret = cal_db_query_create_condition(query, &condition, &bind_text);
		if (CALENDAR_ERROR_NONE != ret) {
			CAL_FREE(table_name);
			ERR("cal_db_query_create_condition() Fail(%d), ret");
			return ret;
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
		cal_db_append_string(&query_str, ")");
		CAL_FREE(condition);
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
	DBG("count(%d) str[%s]", count, query_str);

	if (out_count) *out_count = count;

	if (bind_text) {
		g_slist_free_full(bind_text, free);
		bind_text = NULL;
	}
	CAL_FREE(query_str);
	return CALENDAR_ERROR_NONE;
}

static void _cal_db_instance_allday_extended_get_stmt(sqlite3_stmt *stmt,calendar_record_h record)
{
	cal_instance_allday_extended_s* instance =  (cal_instance_allday_extended_s*)(record);
	const unsigned char *temp;
	int count = 0;

	instance->event_id = sqlite3_column_int(stmt, count++);
	instance->start.type = sqlite3_column_int(stmt, count++);
	count++; /* utime */
	temp = sqlite3_column_text(stmt, count++);
	if (temp) {
		sscanf((const char *)temp, CAL_FORMAT_LOCAL_DATETIME, &(instance->start.time.date.year),
				&(instance->start.time.date.month), &(instance->start.time.date.mday),
				&(instance->start.time.date.hour), &(instance->start.time.date.minute),
				&(instance->start.time.date.second));
	}

	instance->end.type = sqlite3_column_int(stmt, count++);
	count++; /* utime */
	temp = sqlite3_column_text(stmt, count++);
	if (temp) {
		sscanf((const char *)temp, CAL_FORMAT_LOCAL_DATETIME, &(instance->end.time.date.year),
				&(instance->end.time.date.month), &(instance->end.time.date.mday),
				&(instance->end.time.date.hour), &(instance->end.time.date.minute),
				&(instance->end.time.date.second));
	}

	temp = sqlite3_column_text(stmt, count++);
	instance->summary = cal_strdup((const char*)temp);

	temp = sqlite3_column_text(stmt, count++);
	instance->description = cal_strdup((const char*)temp);

	temp = sqlite3_column_text(stmt, count++);
	instance->location = cal_strdup((const char*)temp);

	instance->busy_status = sqlite3_column_int(stmt, count++);

	instance->event_status = sqlite3_column_int(stmt, count++);

	instance->priority = sqlite3_column_int(stmt, count++);

	instance->sensitivity = sqlite3_column_int(stmt, count++);

	instance->has_rrule = sqlite3_column_int(stmt, count++);
	if (0 < instance->has_rrule) {
		instance->has_rrule = 1;
	}

	instance->latitude = sqlite3_column_double(stmt,count++);
	instance->longitude = sqlite3_column_double(stmt,count++);
	instance->has_alarm = sqlite3_column_int(stmt,count++);
	instance->original_event_id = sqlite3_column_int(stmt, count++);
	instance->calendar_id = sqlite3_column_int(stmt, count++);

	instance->last_mod = sqlite3_column_int64(stmt, count++);

	temp = sqlite3_column_text(stmt, count++);
	instance->sync_data1 = cal_strdup((const char*)temp);

	temp = sqlite3_column_text(stmt, count++);
	instance->organizer_name = cal_strdup((const char*)temp);

	temp = sqlite3_column_text(stmt, count++);
	instance->categories= cal_strdup((const char*)temp);

	instance->has_attendee= sqlite3_column_int(stmt, count++);

	temp = sqlite3_column_text(stmt, count++);
	instance->sync_data2 = cal_strdup((const char*)temp);

	temp = sqlite3_column_text(stmt, count++);
	instance->sync_data3 = cal_strdup((const char*)temp);

	temp = sqlite3_column_text(stmt, count++);
	instance->sync_data4 = cal_strdup((const char*)temp);

	instance->is_allday = sqlite3_column_int(stmt, count++);

	return;
}

static void _cal_db_instance_allday_extended_get_property_stmt(sqlite3_stmt *stmt,
		unsigned int property, int *stmt_count, calendar_record_h record)
{
	cal_instance_allday_extended_s* instance =  (cal_instance_allday_extended_s*)(record);
	const unsigned char *temp;

	switch (property) {
	case CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_START:
		instance->start.type = CALENDAR_TIME_LOCALTIME;
		*stmt_count = *stmt_count+1; /* utime */
		*stmt_count = *stmt_count+1;
		temp = sqlite3_column_text(stmt, *stmt_count);
		if (temp) {
			sscanf((const char *)temp, CAL_FORMAT_LOCAL_DATETIME, &(instance->start.time.date.year),
					&(instance->start.time.date.month), &(instance->start.time.date.mday),
					&(instance->start.time.date.hour), &(instance->start.time.date.minute),
					&(instance->start.time.date.second));
		}
		break;
	case CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_END:
		instance->end.type = CALENDAR_TIME_LOCALTIME;
		*stmt_count = *stmt_count+1; /* utime */
		*stmt_count = *stmt_count+1;
		temp = sqlite3_column_text(stmt, *stmt_count);
		if (temp) {
			sscanf((const char *)temp, CAL_FORMAT_LOCAL_DATETIME, &(instance->end.time.date.year),
					&(instance->end.time.date.month), &(instance->end.time.date.mday),
					&(instance->end.time.date.hour), &(instance->end.time.date.minute),
					&(instance->end.time.date.second));
		}
		break;
	case CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_SUMMARY:
		temp = sqlite3_column_text(stmt, *stmt_count);
		instance->summary = cal_strdup((const char*)temp);
		break;
	case CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_LOCATION:
		temp = sqlite3_column_text(stmt, *stmt_count);
		instance->location = cal_strdup((const char*)temp);
		break;
	case CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_CALENDAR_ID:
		instance->calendar_id = sqlite3_column_int(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_DESCRIPTION:
		temp = sqlite3_column_text(stmt, *stmt_count);
		instance->description = cal_strdup((const char*)temp);
		break;
	case CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_BUSY_STATUS:
		instance->busy_status = sqlite3_column_int(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_EVENT_STATUS:
		instance->event_status = sqlite3_column_int(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_PRIORITY:
		instance->priority = sqlite3_column_int(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_SENSITIVITY:
		instance->sensitivity = sqlite3_column_int(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_HAS_RRULE:
		instance->has_rrule = sqlite3_column_int(stmt, *stmt_count);
		if (0 < instance->has_rrule) {
			instance->has_rrule = 1;
		}
		break;
	case CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_LATITUDE:
		instance->latitude = sqlite3_column_double(stmt,*stmt_count);
		break;
	case CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_LONGITUDE:
		instance->longitude = sqlite3_column_double(stmt,*stmt_count);
		break;
	case CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_EVENT_ID:
		instance->event_id = sqlite3_column_int(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_HAS_ALARM:
		instance->has_alarm = sqlite3_column_int(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_ORIGINAL_EVENT_ID:
		instance->original_event_id = sqlite3_column_int(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_LAST_MODIFIED_TIME:
		instance->last_mod = sqlite3_column_int64(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_SYNC_DATA1:
		temp = sqlite3_column_text(stmt, *stmt_count);
		instance->sync_data1 = cal_strdup((const char*)temp);
		break;
	case CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_ORGANIZER_NAME:
		temp = sqlite3_column_text(stmt, *stmt_count);
		instance->organizer_name= cal_strdup((const char*)temp);
		break;
	case CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_CATEGORIES:
		temp = sqlite3_column_text(stmt, *stmt_count);
		instance->categories= cal_strdup((const char*)temp);
		break;
	case CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_HAS_ATTENDEE:
		instance->has_attendee= sqlite3_column_int(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_SYNC_DATA2:
		temp = sqlite3_column_text(stmt, *stmt_count);
		instance->sync_data2= cal_strdup((const char*)temp);
		break;
	case CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_SYNC_DATA3:
		temp = sqlite3_column_text(stmt, *stmt_count);
		instance->sync_data3 = cal_strdup((const char*)temp);
		break;
	case CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_SYNC_DATA4:
		temp = sqlite3_column_text(stmt, *stmt_count);
		instance->sync_data4= cal_strdup((const char*)temp);
		break;
	case CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_IS_ALLDAY:
		instance->is_allday = sqlite3_column_int(stmt, *stmt_count);
		break;

	default:
		sqlite3_column_int(stmt, *stmt_count);
		break;
	}

	*stmt_count = *stmt_count+1;

	return;
}

static void _cal_db_instance_allday_extended_get_projection_stmt(sqlite3_stmt *stmt,
		const unsigned int *projection, const int projection_count,
		calendar_record_h record)
{
	int i=0;
	int stmt_count = 0;

	for(i=0;i<projection_count;i++) {
		_cal_db_instance_allday_extended_get_property_stmt(stmt,projection[i],&stmt_count,record);
	}
}
