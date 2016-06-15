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
#include "cal_list.h"

#include "cal_db_util.h"
#include "cal_db_query.h"
#include "cal_db_plugin_attendee_helper.h"
#include "cal_db.h"
#include "cal_utils.h"

static int _cal_db_attendee_insert_record(calendar_record_h record, int parent_id)
{
	int index;
	int ret = 0;
	char query[CAL_DB_SQL_MAX_LEN];
	sqlite3_stmt *stmt;
	cal_attendee_s *attendee = NULL;

	attendee = (cal_attendee_s *)(record);
	RETVM_IF(NULL == attendee, CALENDAR_ERROR_INVALID_PARAMETER,
			"Invalid argument: cal_alarm_s is NULL");

	snprintf(query, sizeof(query),
			"INSERT INTO %s ("
			"event_id, "
			"attendee_name, attendee_email, attendee_number, "
			"attendee_status, attendee_cutype, attendee_ct_index, "
			"attendee_role, attendee_rsvp, attendee_group, "
			"attendee_delegator_uri, attendee_delegatee_uri, "
			"attendee_member, attendee_uid "
			") VALUES ("
			"%d, "
			"?, ?, ?, "
			"%d, %d, %d, "
			"%d, %d, ?, "
			"?, ?, "
			"?, ?)",
			CAL_TABLE_ATTENDEE,
			parent_id,
			attendee->attendee_status,
			attendee->attendee_cutype,
			attendee->attendee_ct_index,
			attendee->attendee_role,
			attendee->attendee_rsvp);

	ret = cal_db_util_query_prepare(query, &stmt);
	if (CALENDAR_ERROR_NONE != ret) {
		/* LCOV_EXCL_START */
		ERR("cal_db_util_query_prepare() Fail(%d)", ret);
		SECURE("query[%s]", query);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	index = 1;
	if (attendee->attendee_name)
		cal_db_util_stmt_bind_text(stmt, index, attendee->attendee_name);
	index++;

	if (attendee->attendee_email)
		cal_db_util_stmt_bind_text(stmt, index, attendee->attendee_email);
	index++;

	if (attendee->attendee_number)
		cal_db_util_stmt_bind_text(stmt, index, attendee->attendee_number);
	index++;

	if (attendee->attendee_group)
		cal_db_util_stmt_bind_text(stmt, index, attendee->attendee_group);
	index++;

	if (attendee->attendee_delegator_uri)
		cal_db_util_stmt_bind_text(stmt, index, attendee->attendee_delegator_uri);
	index++;

	if (attendee->attendee_delegatee_uri)
		cal_db_util_stmt_bind_text(stmt, index, attendee->attendee_delegatee_uri);
	index++;

	if (attendee->attendee_member)
		cal_db_util_stmt_bind_text(stmt, index, attendee->attendee_member);
	index++;

	if (attendee->attendee_uid)
		cal_db_util_stmt_bind_text(stmt, index, attendee->attendee_uid);
	index++;

	ret = cal_db_util_stmt_step(stmt);
	sqlite3_finalize(stmt);
	if (CALENDAR_ERROR_NONE != ret) {
		/* LCOV_EXCL_START */
		ERR("cal_db_util_stmt_step() Fail(%d)", ret);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	attendee->parent_id = parent_id;
	return CALENDAR_ERROR_NONE;
}

int cal_db_attendee_insert_records(cal_list_s *list_s, int parent_id)
{
	int ret;
	int count = 0;
	calendar_record_h record = NULL;
	calendar_list_h list = (calendar_list_h)list_s;

	RETV_IF(NULL == list, CALENDAR_ERROR_INVALID_PARAMETER);

	calendar_list_get_count(list, &count);
	if (0 == count)
		return CALENDAR_ERROR_NONE;

	calendar_list_first(list);
	while (CALENDAR_ERROR_NONE == calendar_list_get_current_record_p(list, &record)) {
		ret = _cal_db_attendee_insert_record(record, parent_id);
		RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_db_extended_insert_record() Fail(%d)", ret);
		calendar_list_next(list);
	}
	return CALENDAR_ERROR_NONE;

}

int cal_db_attendee_get_records(int parent_id, cal_list_s *list)
{
	int ret;
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	sqlite3_stmt *stmt = NULL;

	RETVM_IF(NULL == list, CALENDAR_ERROR_INVALID_PARAMETER,
			"Invalid parameter: list is NULL");

	snprintf(query, sizeof(query),
			"SELECT rowid, "
			"attendee_name, "
			"attendee_email, "
			"attendee_number, "
			"attendee_status, "
			"attendee_ct_index, "
			"attendee_role, "
			"attendee_rsvp, "
			"attendee_group, "
			"attendee_delegator_uri, "
			"attendee_uid, "
			"attendee_cutype, "
			"attendee_delegatee_uri, "
			"attendee_member "
			"FROM %s WHERE event_id = %d ",
			CAL_TABLE_ATTENDEE, parent_id);

	ret = cal_db_util_query_prepare(query, &stmt);
	if (CALENDAR_ERROR_NONE != ret) {
		/* LCOV_EXCL_START */
		ERR("cal_db_util_query_prepare() Fail(%d)", ret);
		SECURE("query[%s]", query);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	int index;
	const unsigned char *temp;
	calendar_record_h record = NULL;
	cal_attendee_s *attendee = NULL;

	while (CAL_SQLITE_ROW == cal_db_util_stmt_step(stmt)) {
		ret = calendar_record_create(_calendar_attendee._uri, &record);
		if (CALENDAR_ERROR_NONE != ret) {
			/* LCOV_EXCL_START */
			ERR("calendar_record_create() Fail(%d)", ret);
			sqlite3_finalize(stmt);
			cal_list_clear(list);
			return ret;
			/* LCOV_EXCL_STOP */
		}

		index = 0;
		attendee = (cal_attendee_s *)(record);

		attendee->parent_id = parent_id;
		attendee->id = sqlite3_column_int(stmt, index++);

		temp = sqlite3_column_text(stmt, index++);
		attendee->attendee_name = cal_strdup((const char*)temp);

		temp = sqlite3_column_text(stmt, index++);
		attendee->attendee_email = cal_strdup((const char*)temp);

		temp = sqlite3_column_text(stmt, index++);
		attendee->attendee_number = cal_strdup((const char*)temp);

		attendee->attendee_status = sqlite3_column_int(stmt, index++);
		attendee->attendee_ct_index = sqlite3_column_int(stmt, index++);
		attendee->attendee_role = sqlite3_column_int(stmt, index++);
		attendee->attendee_rsvp = sqlite3_column_int(stmt, index++);

		temp = sqlite3_column_text(stmt, index++);
		attendee->attendee_group = cal_strdup((const char*)temp);

		temp = sqlite3_column_text(stmt, index++);
		attendee->attendee_delegator_uri = cal_strdup((const char*)temp);

		temp = sqlite3_column_text(stmt, index++);
		attendee->attendee_uid = cal_strdup((const char*)temp);

		attendee->attendee_cutype = sqlite3_column_int(stmt, index++);

		temp = sqlite3_column_text(stmt, index++);
		attendee->attendee_delegatee_uri = cal_strdup((const char*)temp);

		temp = sqlite3_column_text(stmt, index++);
		attendee->attendee_member = cal_strdup((const char*)temp);

		calendar_list_add((calendar_list_h)list, record);
	}

	sqlite3_finalize(stmt);
	return CALENDAR_ERROR_NONE;
}

int cal_db_attendee_delete_with_id(int parent_id)
{
	int ret = 0;
	char query[CAL_DB_SQL_MAX_LEN] = {0};

	snprintf(query, sizeof(query), "DELETE FROM %s WHERE event_id=%d ",
			CAL_TABLE_ATTENDEE, parent_id);

	ret = cal_db_util_query_exec(query);
	if (CALENDAR_ERROR_NONE != ret) {
		/* LCOV_EXCL_START */
		ERR("cal_db_util_query_exec() Fail(%d)", ret);
		SECURE("[%s]", query);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	return CALENDAR_ERROR_NONE;
}

