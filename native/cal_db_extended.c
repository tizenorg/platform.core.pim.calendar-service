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
#include "cal_list.h"

#include "cal_db.h"
#include "cal_db_util.h"
#include "cal_db_query.h"
#include "cal_db_extended.h"

int cal_db_extended_get_records(int record_id, calendar_record_type_e record_type, cal_list_s *list)
{
	int ret;
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	sqlite3_stmt *stmt = NULL;

	RETV_IF(NULL == list, CALENDAR_ERROR_INVALID_PARAMETER);

	snprintf(query, sizeof(query),
			"SELECT * FROM %s "
			"WHERE record_id = %d AND "
			"record_type = %d",
			CAL_TABLE_EXTENDED,
			record_id,
			record_type);

	stmt = cal_db_util_query_prepare(query);
	RETVM_IF(NULL == stmt,CALENDAR_ERROR_DB_FAILED, "cal_db_util_query_prepare() failed");

	int count = 0;
	const unsigned char *temp;
	calendar_record_h record = NULL;
	cal_extended_s *extended = NULL;

	while (CAL_DB_ROW == cal_db_util_stmt_step(stmt)) {
		ret = calendar_record_create(_calendar_extended_property._uri, &record);
		if (CALENDAR_ERROR_NONE != ret) {
			sqlite3_finalize(stmt);
			cal_list_clear(list);
			return ret;
		}

		count = 0;
		extended = (cal_extended_s *)(record);

		extended->id = sqlite3_column_int(stmt, count++);
		extended->record_id = sqlite3_column_int(stmt, count++);
		extended->record_type = sqlite3_column_int(stmt, count++);
		temp = sqlite3_column_text(stmt, count++);
		extended->key = SAFE_STRDUP(temp);
		temp = sqlite3_column_text(stmt, count++);
		extended->value = SAFE_STRDUP(temp);

		calendar_list_add((calendar_list_h)list, record);
	}
	sqlite3_finalize(stmt);
	return CALENDAR_ERROR_NONE;
}

int cal_db_extended_delete_with_id(int record_id, calendar_record_type_e record_type)
{
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	cal_db_util_error_e dbret = CAL_DB_OK;

	snprintf(query, sizeof(query), "DELETE FROM %s WHERE record_id=%d AND record_type=%d",
			CAL_TABLE_EXTENDED, record_id, record_type);

	dbret = cal_db_util_query_exec(query);
	if (dbret != CAL_DB_OK) {
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

int cal_db_extended_insert_record(calendar_record_h record, int record_id, calendar_record_type_e record_type, int *id)
{
	int index;
	sqlite3_stmt *stmt;
	char query[CAL_DB_SQL_MAX_LEN];
	cal_extended_s* extended =  (cal_extended_s*)(record);
	cal_db_util_error_e dbret;

	RETV_IF(NULL == extended, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(record_id <= 0, CALENDAR_ERROR_INVALID_PARAMETER, "record_id(%d)", record_id);

	snprintf(query, sizeof(query), "INSERT INTO %s(record_id, "
			"record_type ,key ,value) "
			"VALUES(%d,%d,?,?)",
			CAL_TABLE_EXTENDED,
			record_id,
			record_type);

	stmt = cal_db_util_query_prepare(query);
	if (NULL == stmt)
	{
		ERR("cal_db_util_query_prepare() Failed");
		return CALENDAR_ERROR_DB_FAILED;
	}

	if (extended->key)
		cal_db_util_stmt_bind_text(stmt, 1, extended->key);

	if (extended->value)
		cal_db_util_stmt_bind_text(stmt, 2, extended->value);

	dbret = cal_db_util_stmt_step(stmt);
	if (CAL_DB_DONE != dbret)
	{
		sqlite3_finalize(stmt);
		ERR("cal_db_util_stmt_step() Failed(%d)", dbret);
		switch (dbret)
		{
		case CAL_DB_ERROR_NO_SPACE:
			return CALENDAR_ERROR_FILE_NO_SPACE;
		default:
			return CALENDAR_ERROR_DB_FAILED;
		}
	}
	index = cal_db_util_last_insert_id();
	sqlite3_finalize(stmt);

	//cal_record_set_int(record, _calendar_extended.id,index);
	if (id) {
		*id = index;
	}

	if (record_type == CALENDAR_RECORD_TYPE_EVENT || record_type == CALENDAR_RECORD_TYPE_TODO)
	{
		snprintf(query, sizeof(query), "UPDATE %s SET "
				"has_extended = 1 "
				"WHERE id = %d;",
				CAL_TABLE_SCHEDULE,
				record_id);
		dbret = cal_db_util_query_exec(query);
		if(CAL_DB_OK != dbret)
		{
			ERR("cal_db_util_query_exec() Failed(%d)", dbret);
			switch (dbret)
			{
			case CAL_DB_ERROR_NO_SPACE:
				return CALENDAR_ERROR_FILE_NO_SPACE;
			default:
				return CALENDAR_ERROR_DB_FAILED;
			}
		}
	}
	return CALENDAR_ERROR_NONE;
}

int cal_db_extended_insert_records(cal_list_s *list_s, int record_id, calendar_record_type_e record_type)
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
		ret = cal_db_extended_insert_record(record, record_id, record_type, NULL);
		RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_db_extended_insert_record() Failed(%d)", ret);
		calendar_list_next(list);
	}
	return CALENDAR_ERROR_NONE;
}

