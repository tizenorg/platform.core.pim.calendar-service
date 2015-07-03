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
#include "cal_db_query.h"
#include "cal_db_attendee.h"

static int __cal_db_attendee_get_all_records(int offset, int limit, calendar_list_h* out_list);
static int __cal_db_attendee_get_records_with_query(calendar_query_h query, int offset, int limit, calendar_list_h* out_list);
static int __cal_db_attendee_get_count(int *out_count);
static int __cal_db_attendee_get_count_with_query(calendar_query_h query, int *out_count);
// static int __cal_db_attendee_get_record(int id, calendar_record_h* out_record)

cal_db_plugin_cb_s _cal_db_attendee_plugin_cb = {
	.is_query_only = false,
	.insert_record=NULL,
	.get_record=NULL,		// __cal_db_attendee_get_record
	.update_record=NULL,
	.delete_record=NULL,
	.get_all_records=__cal_db_attendee_get_all_records,
	.get_records_with_query=__cal_db_attendee_get_records_with_query,
	.insert_records=NULL,
	.update_records=NULL,
	.delete_records=NULL,
	.get_count=__cal_db_attendee_get_count,
	.get_count_with_query=__cal_db_attendee_get_count_with_query,
	.replace_record=NULL,
	.replace_records=NULL
};

static void __cal_db_attendee_get_stmt(sqlite3_stmt *stmt,calendar_record_h record)
{
	cal_attendee_s *attendee = NULL;
	int index;
	const unsigned char *temp;

	attendee = (cal_attendee_s*)(record);
	index = 0;

	attendee->parent_id = sqlite3_column_int(stmt, index++);

	temp = sqlite3_column_text(stmt, index++);
	attendee->attendee_name = SAFE_STRDUP(temp);

	temp = sqlite3_column_text(stmt, index++);
	attendee->attendee_email = SAFE_STRDUP(temp);

	temp = sqlite3_column_text(stmt, index++);
	attendee->attendee_number = SAFE_STRDUP(temp);

	attendee->attendee_status = sqlite3_column_int(stmt, index++);
	attendee->attendee_ct_index = sqlite3_column_int(stmt, index++);
	attendee->attendee_role = sqlite3_column_int(stmt, index++);
	attendee->attendee_rsvp = sqlite3_column_int(stmt, index++);

	temp = sqlite3_column_text(stmt, index++);
	attendee->attendee_group = SAFE_STRDUP(temp);

	temp = sqlite3_column_text(stmt, index++);
	attendee->attendee_delegator_uri = SAFE_STRDUP(temp);

	temp = sqlite3_column_text(stmt, index++);
	attendee->attendee_uid = SAFE_STRDUP(temp);

	attendee->attendee_cutype = sqlite3_column_int(stmt, index++);

	temp = sqlite3_column_text(stmt, index++);
	attendee->attendee_delegatee_uri = SAFE_STRDUP(temp);

	temp = sqlite3_column_text(stmt, index++);
	attendee->attendee_member = SAFE_STRDUP(temp);

	attendee->id = sqlite3_column_int(stmt, index++);
}

static int __cal_db_attendee_get_all_records(int offset, int limit, calendar_list_h* out_list)
{
	int ret;
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	char offsetquery[CAL_DB_SQL_MAX_LEN] = {0};
	char limitquery[CAL_DB_SQL_MAX_LEN] = {0};
	sqlite3_stmt *stmt = NULL;

	retvm_if(NULL == out_list, CALENDAR_ERROR_INVALID_PARAMETER,
			"Invalid parameter: calendar_list_h is NULL");
	ret = calendar_list_create(out_list);
	retvm_if(CALENDAR_ERROR_NONE != ret, CALENDAR_ERROR_DB_FAILED,
			"calendar_list_create() failed");

	if (limit > 0)	{
		snprintf(limitquery, sizeof(limitquery), "LIMIT %d ", limit);
	}
	if (offset > 0)	{
		snprintf(offsetquery, sizeof(offsetquery), "OFFSET %d ", offset);
	}
	snprintf(query, sizeof(query), "SELECT *, rowid FROM %s %s %s ",
			CAL_TABLE_ATTENDEE, limitquery, offsetquery);

	stmt = _cal_db_util_query_prepare(query);
	if (NULL == stmt)	{
		ERR("_cal_db_util_query_prepare() failed");
		calendar_list_destroy(*out_list, true);
		*out_list = NULL;
		return CALENDAR_ERROR_DB_FAILED;
	}

	calendar_record_h record = NULL;

	while (CAL_DB_ROW == _cal_db_util_stmt_step(stmt))	{
		ret = calendar_record_create(_calendar_attendee._uri, &record);
		if (CALENDAR_ERROR_NONE != ret) {
			calendar_list_destroy(*out_list, true);
			*out_list = NULL;
			sqlite3_finalize(stmt);
			return ret;
		}
		//
		__cal_db_attendee_get_stmt(stmt,record);

		ret = calendar_list_add(*out_list, record);
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

static void __cal_db_attendee_get_property_stmt(sqlite3_stmt *stmt,
		unsigned int property, int *stmt_count, calendar_record_h record)
{
	cal_attendee_s *attendee = NULL;
	const unsigned char *temp;

	attendee = (cal_attendee_s*)(record);

	switch(property) {
	case CAL_PROPERTY_ATTENDEE_NUMBER:
		temp = sqlite3_column_text(stmt, *stmt_count);
		attendee->attendee_number = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_ATTENDEE_CUTYPE:
		attendee->attendee_cutype = sqlite3_column_int(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_ATTENDEE_CT_INDEX:
		attendee->attendee_ct_index = sqlite3_column_int(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_ATTENDEE_UID:
		temp = sqlite3_column_text(stmt, *stmt_count);
		attendee->attendee_uid = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_ATTENDEE_GROUP:
		temp = sqlite3_column_text(stmt, *stmt_count);
		attendee->attendee_group = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_ATTENDEE_EMAIL:
		temp = sqlite3_column_text(stmt, *stmt_count);
		attendee->attendee_email = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_ATTENDEE_ROLE:
		attendee->attendee_role = sqlite3_column_int(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_ATTENDEE_STATUS:
		attendee->attendee_status = sqlite3_column_int(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_ATTENDEE_RSVP:
		attendee->attendee_rsvp = sqlite3_column_int(stmt, *stmt_count);
		break;
	case CAL_PROPERTY_ATTENDEE_DELEGATEE_URI:
		temp = sqlite3_column_text(stmt, *stmt_count);
		attendee->attendee_delegatee_uri = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_ATTENDEE_DELEGATOR_URI:
		temp = sqlite3_column_text(stmt, *stmt_count);
		attendee->attendee_delegator_uri = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_ATTENDEE_NAME:
		temp = sqlite3_column_text(stmt, *stmt_count);
		attendee->attendee_name = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_ATTENDEE_MEMBER:
		temp = sqlite3_column_text(stmt, *stmt_count);
		attendee->attendee_member = SAFE_STRDUP(temp);
		break;
	case CAL_PROPERTY_ATTENDEE_PARENT_ID:
		attendee->parent_id = sqlite3_column_int(stmt, *stmt_count);
		break;
	default:
		sqlite3_column_int(stmt, *stmt_count);
		break;
	}
	*stmt_count = *stmt_count+1;
}

static void __cal_db_attendee_get_projection_stmt(sqlite3_stmt *stmt,
		const unsigned int *projection, const int projection_count,
		calendar_record_h record)
{
	int i=0;
	int stmt_count = 0;

	for(i=0;i<projection_count;i++)
		__cal_db_attendee_get_property_stmt(stmt,projection[i],&stmt_count,record);
}

static int __cal_db_attendee_get_records_with_query(calendar_query_h query, int offset, int limit, calendar_list_h* out_list)
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
	char *table_name;

	que = (cal_query_s *)query;

	if ( 0 == strcmp(que->view_uri, CALENDAR_VIEW_ATTENDEE)) {
		table_name = SAFE_STRDUP(CAL_TABLE_ATTENDEE);
	}
	else {
		ERR("uri(%s) not support get records with query",que->view_uri);
		return CALENDAR_ERROR_INVALID_PARAMETER;
		//table_name = SAFE_STRDUP(CAL_TABLE_NORMAL_INSTANCE);
	}

	// make filter
	if (que->filter) {
		ret = _cal_db_query_create_condition(query, &condition, &bind_text);
		if (ret != CALENDAR_ERROR_NONE) {
			CAL_FREE(table_name);
			ERR("filter create fail");
			return ret;
		}
	}

	// make projection
	ret = _cal_db_query_create_projection(query, &projection);

	// query - projection
	if (projection) {
		_cal_db_append_string(&query_str, "SELECT");
		_cal_db_append_string(&query_str, projection);
		_cal_db_append_string(&query_str, "FROM");
		_cal_db_append_string(&query_str, table_name);
		CAL_FREE(projection);
	}
	else {
		_cal_db_append_string(&query_str, "SELECT *, rowid FROM ");
		_cal_db_append_string(&query_str, table_name);
	}
	CAL_FREE(table_name);

	// query - condition
	if (condition) {
		_cal_db_append_string(&query_str, "WHERE");
		_cal_db_append_string(&query_str, condition);
		CAL_FREE(condition);
	}

	// ORDER
	ret = _cal_db_query_create_order(query, condition, &order);
	if (order) {
		_cal_db_append_string(&query_str, order);
		CAL_FREE(order);
	}

	char buf[32] = {0};
	if (0 < limit) {
		snprintf(buf, sizeof(buf), "LIMIT %d", limit);
		_cal_db_append_string(&query_str, buf);
		if (0 < offset) {
			snprintf(buf, sizeof(buf), "OFFSET %d", offset);
			_cal_db_append_string(&query_str, buf);
		}
	}

	// query
	stmt = _cal_db_util_query_prepare(query_str);
	if (NULL == stmt) {
		if (bind_text) {
			g_slist_free_full(bind_text, free);
			bind_text = NULL;
		}
		CAL_FREE(query_str);
		ERR("_cal_db_util_query_prepare() Failed");
		return CALENDAR_ERROR_DB_FAILED;
	}
	CAL_DBG("%s",query_str);

	// bind text
	if (bind_text)	{
		for (cursor=bind_text, i=1; cursor;cursor=cursor->next, i++)
			_cal_db_util_stmt_bind_text(stmt, i, cursor->data);
	}

	//
	ret = calendar_list_create(out_list);
	if (ret != CALENDAR_ERROR_NONE) {
		if (bind_text) {
			g_slist_free_full(bind_text, free);
			bind_text = NULL;
		}
		CAL_FREE(query_str);
		ERR("calendar_list_create() Failed");
		sqlite3_finalize(stmt);
		return ret;
	}

	while(CAL_DB_ROW == _cal_db_util_stmt_step(stmt)) {
		calendar_record_h record;
		// stmt -> record
		ret = calendar_record_create(_calendar_attendee._uri,&record);
		if( ret != CALENDAR_ERROR_NONE ) {
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
		if (que->projection_count > 0) {
			_cal_record_set_projection(record,
					que->projection, que->projection_count, que->property_count);

			__cal_db_attendee_get_projection_stmt(stmt,
					que->projection, que->projection_count,
					record);
		}
		else {
			__cal_db_attendee_get_stmt(stmt,record);
		}

		ret = calendar_list_add(*out_list,record);
		if( ret != CALENDAR_ERROR_NONE ) {
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

static int __cal_db_attendee_get_count(int *out_count)
{
	int ret;
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	int count = 0;

	retvm_if(NULL == out_count, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");

	snprintf(query, sizeof(query), "SELECT count(*) FROM %s ", CAL_TABLE_ATTENDEE);

	ret = _cal_db_util_query_get_first_int_result(query, NULL, &count);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("_cal_db_util_query_get_first_int_result() failed");
		return ret;
	}
	CAL_DBG("%s=%d",query,count);

	*out_count = count;
	return CALENDAR_ERROR_NONE;
}

static int __cal_db_attendee_get_count_with_query(calendar_query_h query, int *out_count)
{
	cal_query_s *que = NULL;
	int ret = CALENDAR_ERROR_NONE;
	char *condition = NULL;
	char *query_str = NULL;
	char *table_name;
	int count = 0;
	GSList *bind_text = NULL;

	que = (cal_query_s *)query;

	if ( 0 == strcmp(que->view_uri, CALENDAR_VIEW_ATTENDEE)) {
		table_name = SAFE_STRDUP(CAL_TABLE_ATTENDEE);
	}
	else {
		ERR("uri(%s) not support get records with query",que->view_uri);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	// make filter
	if (que->filter) {
		ret = _cal_db_query_create_condition(query, &condition, &bind_text);
		if (ret != CALENDAR_ERROR_NONE) {
			CAL_FREE(table_name);
			ERR("filter create fail");
			return ret;
		}
	}

	// query - select from
	_cal_db_append_string(&query_str, "SELECT count(*) FROM");
	_cal_db_append_string(&query_str, table_name);
	CAL_FREE(table_name);

	CAL_FREE(table_name);

	// query - condition
	if (condition) {
		_cal_db_append_string(&query_str, "WHERE");
		_cal_db_append_string(&query_str, condition);
		CAL_FREE(condition);
	}

	// query
	ret = _cal_db_util_query_get_first_int_result(query_str, bind_text, &count);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("_cal_db_util_query_get_first_int_result() failed");
		if (bind_text) {
			g_slist_free_full(bind_text, free);
			bind_text = NULL;
		}
		CAL_FREE(query_str);
		return ret;
	}
	CAL_DBG("%s=%d",query_str,count);

	*out_count = count;

	if (bind_text) {
		g_slist_free_full(bind_text, free);
		bind_text = NULL;
	}
	CAL_FREE(query_str);
	return CALENDAR_ERROR_NONE;
}

