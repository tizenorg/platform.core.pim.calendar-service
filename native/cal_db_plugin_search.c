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

/*
 * db plugin function
 */
static int _cal_db_search_get_records_with_query(calendar_query_h query, int offset, int limit, calendar_list_h* out_list);
static int _cal_db_search_get_count_with_query(calendar_query_h query, int *out_count);

/*
 * static function
 */
static void _cal_db_search_get_stmt(sqlite3_stmt *stmt,calendar_query_h query,
		calendar_record_h record);
static void _cal_db_search_get_property_stmt(sqlite3_stmt *stmt,
		unsigned int property, int *stmt_count, calendar_record_h record);
static void _cal_db_search_get_projection_stmt(sqlite3_stmt *stmt,
		const unsigned int *projection, const int projection_count,
		calendar_record_h record);
static int _cal_db_search_make_projection(calendar_query_h query, char **projection);


cal_db_plugin_cb_s cal_db_search_plugin_cb = {
	.is_query_only=true,
	.insert_record=NULL,
	.get_record=NULL,
	.update_record=NULL,
	.delete_record=NULL,
	.get_all_records=NULL,
	.get_records_with_query=_cal_db_search_get_records_with_query,
	.insert_records=NULL,
	.update_records=NULL,
	.delete_records=NULL,
	.get_count=NULL,
	.get_count_with_query=_cal_db_search_get_count_with_query,
	.replace_record=NULL,
	.replace_records=NULL
};

static int _cal_db_search_get_records_with_query(calendar_query_h query, int offset, int limit, calendar_list_h* out_list)
{
	cal_query_s *que = NULL;
	int ret = CALENDAR_ERROR_NONE;
	char *condition = NULL;
	char *projection = NULL;
	GSList *bind_text = NULL, *cursor = NULL;
	sqlite3_stmt *stmt = NULL;
	int i = 0;
	char *table_name = NULL;

	que = (cal_query_s *)query;

	// make filter
	if (que->filter) {
		ret = cal_db_query_create_condition(query,
				&condition, &bind_text);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("filter create Fail");
			return ret;
		}
	}

	// make projection
	if (0 < que->projection_count) {
		ret = cal_db_query_create_projection(query, &projection);
	}
	else {
		_cal_db_search_make_projection(query, &projection);
	}

	char *query_str = NULL;
	if (que->distinct == true) {
		cal_db_append_string(&query_str, "SELECT DISTINCT");
		cal_db_append_string(&query_str, projection);
		cal_db_append_string(&query_str, "FROM");
		cal_db_append_string(&query_str, table_name);
	}
	else {
		cal_db_append_string(&query_str, "SELECT");
		cal_db_append_string(&query_str, projection);
		cal_db_append_string(&query_str, "FROM");
		cal_db_append_string(&query_str, table_name);
	}
	CAL_FREE(projection);
	CAL_FREE(table_name);

	// query - condition
	if (condition) {
		cal_db_append_string(&query_str, "WHERE (");
		cal_db_append_string(&query_str, condition);
		cal_db_append_string(&query_str, ")");
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
	if (0 < limit) {
		snprintf(buf, sizeof(buf), "LIMIT %d", limit);
		cal_db_append_string(&query_str, buf);

		if (0 < offset) {
			snprintf(buf, sizeof(buf), "OFFSET %d", offset);
			cal_db_append_string(&query_str, buf);
		}
	}

	// query
	stmt = cal_db_util_query_prepare(query_str);
	if (NULL == stmt)
	{
		if (bind_text)
		{
			g_slist_free_full(bind_text, free);
			bind_text = NULL;
		}
		CAL_FREE(query_str);
		ERR("cal_db_util_query_prepare() Fail");
		return CALENDAR_ERROR_DB_FAILED;
	}
	DBG("%s",query_str);

	// bind text
	if (bind_text) {
		g_slist_length(bind_text);
		for (cursor=bind_text, i=1; cursor;cursor=cursor->next, i++) {
			cal_db_util_stmt_bind_text(stmt, i, cursor->data);
		}
	}

	//
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

	while(CAL_DB_ROW == cal_db_util_stmt_step(stmt)) {
		calendar_record_h record;
		// stmt -> record
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
			_cal_db_search_get_projection_stmt(stmt,que->projection,que->projection_count,
					record);
		}
		else {
			_cal_db_search_get_stmt(stmt, query,record);
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

static int _cal_db_search_get_count_with_query(calendar_query_h query, int *out_count)
{
	cal_query_s *que = NULL;
	int ret = CALENDAR_ERROR_NONE;
	char *condition = NULL;
	char *projection = NULL;
	char *table_name;
	int count = 0;
	GSList *bind_text = NULL;

	que = (cal_query_s *)query;

	if (CAL_STRING_EQUAL == strcmp(que->view_uri, CALENDAR_VIEW_EVENT_CALENDAR)) {
		table_name = SAFE_STRDUP(CAL_VIEW_TABLE_EVENT_CALENDAR);
		projection = SAFE_STRDUP("id");
	}
	else if (CAL_STRING_EQUAL == strcmp(que->view_uri, CALENDAR_VIEW_TODO_CALENDAR)) {
		table_name = SAFE_STRDUP(CAL_VIEW_TABLE_TODO_CALENDAR);
		projection = SAFE_STRDUP("id");
	}
	else if (CAL_STRING_EQUAL == strcmp(que->view_uri, CALENDAR_VIEW_EVENT_CALENDAR_ATTENDEE)) {
		table_name = SAFE_STRDUP(CAL_VIEW_TABLE_EVENT_CALENDAR_ATTENDEE);
		projection = SAFE_STRDUP("id");
	}
	else if (CAL_STRING_EQUAL == strcmp(que->view_uri, CALENDAR_VIEW_INSTANCE_UTIME_CALENDAR)) {
		table_name = SAFE_STRDUP(CAL_VIEW_TABLE_NORMAL_INSTANCE);
		projection = SAFE_STRDUP("event_id");
	}
	else if (CAL_STRING_EQUAL == strcmp(que->view_uri, CALENDAR_VIEW_INSTANCE_LOCALTIME_CALENDAR)) {
		table_name = SAFE_STRDUP(CAL_VIEW_TABLE_ALLDAY_INSTANCE);
		projection = SAFE_STRDUP("event_id");
	}
	else {
		ERR("uri(%s) not support get records with query",que->view_uri);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	// make filter
	if (que->filter) {
		ret = cal_db_query_create_condition(query, &condition, &bind_text);
		if (CALENDAR_ERROR_NONE != ret) {
			CAL_FREE(table_name);
			ERR("filter create Fail");
			CAL_FREE(projection);
			return ret;
		}
	}

	char *query_str = NULL;

	// query - select from
	if (que->distinct == true) {
		cal_db_append_string(&query_str, "SELECT count(DISTINCT");
		cal_db_append_string(&query_str, projection);
		cal_db_append_string(&query_str, ") FROM");
		cal_db_append_string(&query_str, table_name);
	}
	else {
		cal_db_append_string(&query_str, "SELECT count(*) FROM");
		cal_db_append_string(&query_str, table_name);
	}
	CAL_FREE(projection);
	CAL_FREE(table_name);

	// query - condition
	if (condition) {
		cal_db_append_string(&query_str, "WHERE (");
		cal_db_append_string(&query_str, condition);
		cal_db_append_string(&query_str, ")");
		CAL_FREE(condition);
	}

	// query
	ret = cal_db_util_query_get_first_int_result(query_str, bind_text, &count);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_query_get_first_int_result() Failed");
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

static void _cal_db_search_get_stmt(sqlite3_stmt *stmt,calendar_query_h query,
		calendar_record_h record)
{
	int i=0;
	int stmt_count = 0;
	cal_query_s *query_s = NULL;
	cal_property_info_s *properties = NULL;

	query_s = (cal_query_s *)query;

	for (i=0;i<query_s->property_count;i++) {
		properties = &(query_s->properties[i]);

		if (CAL_PROPERTY_CHECK_FLAGS(properties->property_id, CAL_PROPERTY_FLAGS_FILTER) == true) {
			break;
		}

		_cal_db_search_get_property_stmt(stmt, properties->property_id, &stmt_count,record);
	}
	return ;
}

static void _cal_db_search_get_property_stmt(sqlite3_stmt *stmt,
		unsigned int property, int *stmt_count, calendar_record_h record)
{
	int ret = 0;
	const unsigned char *temp;
	int int_tmp = 0;
	double d_tmp = 0;
	long long int lli_tmp = 0;

	if (CAL_PROPERTY_CHECK_DATA_TYPE(property,CAL_PROPERTY_DATA_TYPE_INT) == true) {
		int_tmp = sqlite3_column_int(stmt, *stmt_count);
		cal_record_set_int(record,property,int_tmp);
	}
	else if (CAL_PROPERTY_CHECK_DATA_TYPE(property,CAL_PROPERTY_DATA_TYPE_STR) == true) {
		temp = sqlite3_column_text(stmt, *stmt_count);
		cal_record_set_str(record,property,(const char*)temp);
	}
	else if (CAL_PROPERTY_CHECK_DATA_TYPE(property,CAL_PROPERTY_DATA_TYPE_DOUBLE) == true) {
		d_tmp = sqlite3_column_double(stmt,*stmt_count);
		cal_record_set_double(record,property,d_tmp);
	}
	else if (CAL_PROPERTY_CHECK_DATA_TYPE(property,CAL_PROPERTY_DATA_TYPE_LLI) == true) {
		lli_tmp = sqlite3_column_int64(stmt, *stmt_count);
		cal_record_set_lli(record,property,lli_tmp);
	}
	else if (CAL_PROPERTY_CHECK_DATA_TYPE(property,CAL_PROPERTY_DATA_TYPE_CALTIME) == true) {
		calendar_time_s caltime_tmp;
		caltime_tmp.type = sqlite3_column_int(stmt,*stmt_count);
		switch (caltime_tmp.type) {
		case CALENDAR_TIME_UTIME:
			*stmt_count = *stmt_count+1;
			caltime_tmp.time.utime = sqlite3_column_int64(stmt,*stmt_count);
			*stmt_count = *stmt_count+1; // datetime
			break;

		case CALENDAR_TIME_LOCALTIME:
			*stmt_count = *stmt_count+1; // utime
			*stmt_count = *stmt_count+1;
			temp = sqlite3_column_text(stmt, *stmt_count);
			if (temp) {
				sscanf((const char *)temp, CAL_FORMAT_LOCAL_DATETIME, &(caltime_tmp.time.date.year),
						&(caltime_tmp.time.date.month), &(caltime_tmp.time.date.mday),
						&(caltime_tmp.time.date.hour), &(caltime_tmp.time.date.minute),
						&(caltime_tmp.time.date.second));
			}
			break;
		}
		ret = cal_record_set_caltime(record,property,caltime_tmp);
		WARN_IF(CALENDAR_ERROR_NONE != ret, "Failed to cal_record_set_caltime()");
	}
	else {
		sqlite3_column_int(stmt, *stmt_count);
	}

	*stmt_count = *stmt_count+1;
}
static void _cal_db_search_get_projection_stmt(sqlite3_stmt *stmt,
		const unsigned int *projection, const int projection_count,
		calendar_record_h record)
{
	int i=0;
	int stmt_count = 0;

	for(i=0;i<projection_count;i++) {
		_cal_db_search_get_property_stmt(stmt,projection[i],&stmt_count,record);
	}
}

static int _cal_db_search_make_projection(calendar_query_h query, char **projection)
{
	int i = 0;
	int len = 0;
	const char *field_name;
	char out_projection[CAL_DB_SQL_MAX_LEN] = {0};
	cal_query_s *query_s = NULL;
	cal_property_info_s *properties = NULL;

	RETV_IF(NULL == query, CALENDAR_ERROR_INVALID_PARAMETER);
	query_s = (cal_query_s *)query;

	properties = &(query_s->properties[0]);
	field_name = properties->fields;
	if (field_name)
		len += snprintf(out_projection+len, sizeof(out_projection)-len, "%s", field_name);

	if (sizeof(out_projection) <= len) {
		ERR("buf len max");
		return CALENDAR_ERROR_SYSTEM;
	}

	for (i=1;i<query_s->property_count;i++) {
		properties = &(query_s->properties[i]);
		field_name = properties->fields;

		if (CAL_PROPERTY_CHECK_FLAGS(properties->property_id, CAL_PROPERTY_FLAGS_FILTER) == true) {
			break;
		}

		if (field_name) {
			len += snprintf(out_projection+len, sizeof(out_projection)-len, ", %s", field_name);
			if (sizeof(out_projection) <= len) {
				ERR("buf len max");
				return CALENDAR_ERROR_SYSTEM;
			}
		}
	}

	*projection = g_strdup(out_projection);

	return CALENDAR_ERROR_NONE;
}
