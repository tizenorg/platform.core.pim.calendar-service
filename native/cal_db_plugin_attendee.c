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
#include "cal_db_query.h"
#include "cal_db_attendee.h"

static int _cal_db_attendee_get_all_records(int offset, int limit, calendar_list_h* out_list);
static int _cal_db_attendee_get_records_with_query(calendar_query_h query, int offset, int limit, calendar_list_h* out_list);
static int _cal_db_attendee_get_count(int *out_count);
static int _cal_db_attendee_get_count_with_query(calendar_query_h query, int *out_count);
// static int _cal_db_attendee_get_record(int id, calendar_record_h* out_record)

cal_db_plugin_cb_s cal_db_attendee_plugin_cb = {
	.is_query_only = false,
	.insert_record=NULL,
	.get_record=NULL,		// _cal_db_attendee_get_record
	.update_record=NULL,
	.delete_record=NULL,
	.get_all_records=_cal_db_attendee_get_all_records,
	.get_records_with_query=_cal_db_attendee_get_records_with_query,
	.insert_records=NULL,
	.update_records=NULL,
	.delete_records=NULL,
	.get_count=_cal_db_attendee_get_count,
	.get_count_with_query=_cal_db_attendee_get_count_with_query,
	.replace_record=NULL,
	.replace_records=NULL
};

static void _cal_db_attendee_get_stmt(sqlite3_stmt *stmt,calendar_record_h record)
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

static int _cal_db_attendee_get_all_records(int offset, int limit, calendar_list_h* out_list)
{
	int ret;
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	char offsetquery[CAL_DB_SQL_MAX_LEN] = {0};
	char limitquery[CAL_DB_SQL_MAX_LEN] = {0};
	sqlite3_stmt *stmt = NULL;

	RETV_IF(NULL == out_list, CALENDAR_ERROR_INVALID_PARAMETER);

	ret = calendar_list_create(out_list);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "calendar_list_create() Fail(%d)", ret);

	if (0 < limit)	{
		snprintf(limitquery, sizeof(limitquery), "LIMIT %d ", limit);
	}
	if (0 < offset)	{
		snprintf(offsetquery, sizeof(offsetquery), "OFFSET %d ", offset);
	}
	snprintf(query, sizeof(query), "SELECT *, rowid FROM %s %s %s ",
			CAL_TABLE_ATTENDEE, limitquery, offsetquery);

	stmt = cal_db_util_query_prepare(query);
	if (NULL == stmt)	{
		ERR("cal_db_util_query_prepare() failed");
		calendar_list_destroy(*out_list, true);
		*out_list = NULL;
		return CALENDAR_ERROR_DB_FAILED;
	}

	calendar_record_h record = NULL;

	while (CAL_DB_ROW == cal_db_util_stmt_step(stmt))	{
		ret = calendar_record_create(_calendar_attendee._uri, &record);
		if (CALENDAR_ERROR_NONE != ret) {
			calendar_list_destroy(*out_list, true);
			*out_list = NULL;
			sqlite3_finalize(stmt);
			return ret;
		}
		//
		_cal_db_attendee_get_stmt(stmt,record);

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

static void _cal_db_attendee_get_property_stmt(sqlite3_stmt *stmt,
		unsigned int property, int *stmt_count, calendar_record_h record)
{
	cal_attendee_s *attendee = NULL;
	const unsigned char *temp;

	attendee = (cal_attendee_s*)(record);

	switch (property) {
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

static void _cal_db_attendee_get_projection_stmt(sqlite3_stmt *stmt,
		const unsigned int *projection, const int projection_count,
		calendar_record_h record)
{
	int i=0;
	int stmt_count = 0;

	for(i=0;i<projection_count;i++)
		_cal_db_attendee_get_property_stmt(stmt,projection[i],&stmt_count,record);
}

static int _cal_db_attendee_get_records_with_query(calendar_query_h query, int offset, int limit, calendar_list_h* out_list)
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

	if (CAL_STRING_EQUAL == strcmp(que->view_uri, CALENDAR_VIEW_ATTENDEE)) {
		table_name = SAFE_STRDUP(CAL_TABLE_ATTENDEE);
	}
	else {
		ERR("uri(%s) not support get records with query",que->view_uri);
		return CALENDAR_ERROR_INVALID_PARAMETER;
		//table_name = SAFE_STRDUP(CAL_TABLE_NORMAL_INSTANCE);
	}

	// make filter
	if (que->filter) {
		ret = cal_db_query_create_condition(query, &condition, &bind_text);
		if (CALENDAR_ERROR_NONE != ret) {
			CAL_FREE(table_name);
			ERR("cal_db_query_create_condition() Fail(%d), ret");
			return ret;
		}
	}

	// make projection
	ret = cal_db_query_create_projection(query, &projection);

	// query - projection
	if (projection) {
		cal_db_append_string(&query_str, "SELECT");
		cal_db_append_string(&query_str, projection);
		cal_db_append_string(&query_str, "FROM");
		cal_db_append_string(&query_str, table_name);
		CAL_FREE(projection);
	}
	else {
		cal_db_append_string(&query_str, "SELECT *, rowid FROM ");
		cal_db_append_string(&query_str, table_name);
	}
	CAL_FREE(table_name);

	// query - condition
	if (condition) {
		cal_db_append_string(&query_str, "WHERE");
		cal_db_append_string(&query_str, condition);
		CAL_FREE(condition);
	}

	// ORDER
	ret = cal_db_query_create_order(query, condition, &order);
	if (order) {
		cal_db_append_string(&query_str, order);
		CAL_FREE(order);
	}

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
	if (NULL == stmt) {
		if (bind_text) {
			g_slist_free_full(bind_text, free);
			bind_text = NULL;
		}
		CAL_FREE(query_str);
		ERR("cal_db_util_query_prepare() Fail");
		return CALENDAR_ERROR_DB_FAILED;
	}
	DBG("%s",query_str);

	// bind text
	if (bind_text)	{
		for (cursor=bind_text, i=1; cursor;cursor=cursor->next, i++)
			cal_db_util_stmt_bind_text(stmt, i, cursor->data);
	}

	//
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

	while(CAL_DB_ROW == cal_db_util_stmt_step(stmt)) {
		calendar_record_h record;
		// stmt -> record
		ret = calendar_record_create(_calendar_attendee._uri,&record);
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

			_cal_db_attendee_get_projection_stmt(stmt,
					que->projection, que->projection_count,
					record);
		}
		else {
			_cal_db_attendee_get_stmt(stmt,record);
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

static int _cal_db_attendee_get_count(int *out_count)
{
	int ret;
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	int count = 0;

	RETV_IF(NULL == out_count, CALENDAR_ERROR_INVALID_PARAMETER);

	snprintf(query, sizeof(query), "SELECT count(*) FROM %s ", CAL_TABLE_ATTENDEE);

	ret = cal_db_util_query_get_first_int_result(query, NULL, &count);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_query_get_first_int_result() failed");
		return ret;
	}
	DBG("%s=%d",query,count);

	*out_count = count;
	return CALENDAR_ERROR_NONE;
}

static int _cal_db_attendee_get_count_with_query(calendar_query_h query, int *out_count)
{
	cal_query_s *que = NULL;
	int ret = CALENDAR_ERROR_NONE;
	char *condition = NULL;
	char *query_str = NULL;
	char *table_name;
	int count = 0;
	GSList *bind_text = NULL;

	que = (cal_query_s *)query;

	if (CAL_STRING_EQUAL == strcmp(que->view_uri, CALENDAR_VIEW_ATTENDEE)) {
		table_name = SAFE_STRDUP(CAL_TABLE_ATTENDEE);
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
			ERR("cal_db_query_create_condition() Fail(%d), ret");
			return ret;
		}
	}

	// query - select from
	cal_db_append_string(&query_str, "SELECT count(*) FROM");
	cal_db_append_string(&query_str, table_name);
	CAL_FREE(table_name);

	CAL_FREE(table_name);

	// query - condition
	if (condition) {
		cal_db_append_string(&query_str, "WHERE");
		cal_db_append_string(&query_str, condition);
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
	DBG("%s=%d",query_str,count);

	*out_count = count;

	if (bind_text) {
		g_slist_free_full(bind_text, free);
		bind_text = NULL;
	}
	CAL_FREE(query_str);
	return CALENDAR_ERROR_NONE;
}

