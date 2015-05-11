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
#include "cal_db_attendee.h"

static int _cal_db_attendee_insert_record(calendar_record_h record, int parent_id)
{
	int index;
	cal_db_util_error_e dbret = CAL_DB_OK;
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

	stmt = cal_db_util_query_prepare(query);
	if (NULL == stmt)
	{
		DBG("cal_db_util_query_prepare() Failed");
		DBG("query[%s]", query);
		return CALENDAR_ERROR_DB_FAILED;
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

	dbret = cal_db_util_stmt_step(stmt);
	sqlite3_finalize(stmt);

	if (CAL_DB_DONE != dbret)
	{
		ERR("cal_db_util_stmt_step() Failed(%d)", dbret);
		switch (dbret)
		{
		case CAL_DB_ERROR_NO_SPACE:
			return CALENDAR_ERROR_FILE_NO_SPACE;
		default:
			return CALENDAR_ERROR_DB_FAILED;
		}
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
		RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_db_extended_insert_record() Failed(%d)", ret);
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

	stmt = cal_db_util_query_prepare(query);
	RETVM_IF(NULL == stmt, CALENDAR_ERROR_DB_FAILED, "cal_db_util_query_prepare() failed");

	int index;
	const unsigned char *temp;
	calendar_record_h record = NULL;
	cal_attendee_s *attendee = NULL;

	while (CAL_DB_ROW == cal_db_util_stmt_step(stmt)) {
		ret = calendar_record_create(_calendar_attendee._uri, &record);
		if (CALENDAR_ERROR_NONE != ret) {
			sqlite3_finalize(stmt);
			cal_list_clear(list);
			return ret;
		}

		index = 0;
		attendee = (cal_attendee_s *)(record);

		attendee->parent_id = parent_id;
		attendee->id = sqlite3_column_int(stmt, index++);

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

		calendar_list_add((calendar_list_h)list, record);
	}

	sqlite3_finalize(stmt);
	return CALENDAR_ERROR_NONE;
}

int cal_db_attendee_delete_with_id(int parent_id)
{
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	cal_db_util_error_e dbret = CAL_DB_OK;

	snprintf(query, sizeof(query), "DELETE FROM %s WHERE event_id=%d ",
			CAL_TABLE_ATTENDEE, parent_id);

	dbret = cal_db_util_query_exec(query);
	if (dbret != CAL_DB_OK)
	{
		ERR("cal_db_util_query_exec() failed (%d)", dbret);
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

