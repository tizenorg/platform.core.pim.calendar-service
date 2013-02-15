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
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <db-util.h>

#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_db.h"

#include "cal_db_util.h"

static TLS sqlite3 *calendar_db_handle;
static TLS int transaction_cnt = 0;
static TLS int transaction_ver = 0;
static TLS bool version_up = false;

static TLS bool event_change=false;
static TLS bool todo_change=false;
static TLS bool calendar_change=false;

static inline void __cal_db_util_notify_event_change(void)
{
	int fd = open(CAL_NOTI_EVENT_CHANGED, O_TRUNC | O_RDWR);
	if (0 <= fd) {
		close(fd);
		event_change = false;
	}
}

static inline void __cal_db_util_notify_todo_change(void)
{
	int fd = open(CAL_NOTI_TODO_CHANGED, O_TRUNC | O_RDWR);
	if (0 <= fd) {
		close(fd);
		todo_change = false;
	}
}

static inline void __cal_db_util_notify_calendar_change(void)
{
	int fd = open(CAL_NOTI_CALENDAR_CHANGED, O_TRUNC | O_RDWR);
	if (0 <= fd) {
		close(fd);
		calendar_change = false;
	}
}

static inline void __cal_db_util_cancel_changes(void)
{
    event_change = false;
    calendar_change = false;
    todo_change = false;
}

int _cal_db_util_notify(cal_noti_type_e type)
{
	if (0 < transaction_cnt) {
		switch (type) {
		case CAL_NOTI_TYPE_EVENT:
			event_change = true;
			break;
		case CAL_NOTI_TYPE_TODO:
			todo_change = true;
			break;
		case CAL_NOTI_TYPE_CALENDAR:
			calendar_change = true;
			break;
		default:
			ERR("The type(%d) is not supported", type);
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
		return CALENDAR_ERROR_NONE;
	}

	switch(type) {
	case CAL_NOTI_TYPE_EVENT:
		__cal_db_util_notify_event_change();
		break;
	case CAL_NOTI_TYPE_TODO:
		__cal_db_util_notify_todo_change();
		break;
	case CAL_NOTI_TYPE_CALENDAR:
		__cal_db_util_notify_calendar_change();
		break;
	default:
		ERR("The type(%d) is not supported", type);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

int _cal_db_util_open(void)
{
	int ret = 0;

	if (!calendar_db_handle) {
		ret = db_util_open(CAL_DB_PATH, &calendar_db_handle, 0);
		retvm_if(SQLITE_OK != ret, CALENDAR_ERROR_DB_FAILED,
				"db_util_open() Failed(%d).", ret);
	}
	return CALENDAR_ERROR_NONE;
}

int _cal_db_util_close(void)
{
	int ret = 0;

	if (calendar_db_handle) {
		ret = db_util_close(calendar_db_handle);
		warn_if(SQLITE_OK != ret, "db_util_close() Failed(%d)", ret);
		calendar_db_handle = NULL;
		CAL_DBG("The database disconnected really.");
	}

	return CALENDAR_ERROR_NONE;
}

int _cal_db_util_last_insert_id(void)
{
	return sqlite3_last_insert_rowid(calendar_db_handle);
}

int _cal_db_util_query_get_first_int_result(const char *query, GSList *bind_text, int *result)
{
	int ret;
	int index;
	char *text = NULL;
	sqlite3_stmt *stmt = NULL;
	retvm_if(NULL == calendar_db_handle, CALENDAR_ERROR_DB_FAILED, "Database is not opended");

	ret = sqlite3_prepare_v2(calendar_db_handle, query, strlen(query), &stmt, NULL);
	retvm_if(SQLITE_OK != ret, CALENDAR_ERROR_DB_FAILED,
			"sqlite3_prepare_v2(%s) failed(%s).", query, sqlite3_errmsg(calendar_db_handle));

	if (bind_text)
	{
		for (index = 0; g_slist_nth(bind_text, index); index++)
		{
			text = (char *)g_slist_nth_data(bind_text, index);
			if (text)
			{
				_cal_db_util_stmt_bind_text(stmt, index + 1, text);
			}
		}
	}

	ret = sqlite3_step(stmt);
	if (SQLITE_ROW != ret)
	{
		ERR("sqlite3_step() failed(%d, %s).", ret, sqlite3_errmsg(calendar_db_handle));
		sqlite3_finalize(stmt);
		if (SQLITE_DONE == ret) return CALENDAR_ERROR_DB_FAILED;
		return CALENDAR_ERROR_DB_FAILED;
	}

	if (result) *result = sqlite3_column_int(stmt, 0);
	sqlite3_finalize(stmt);

	return CALENDAR_ERROR_NONE;
}

cal_db_util_error_e _cal_db_util_query_exec(char *query)
{
	int ret;
	char *err_msg = NULL;

	retvm_if(NULL == calendar_db_handle, CALENDAR_ERROR_DB_FAILED, "Database is not opended");
	//CALS_DBG("query : %s", query);

	ret = sqlite3_exec(calendar_db_handle, query, NULL, NULL, &err_msg);
	if (SQLITE_OK != ret) {
		ERR("sqlite3_exec(%s) failed(%d, %s).", query, ret, err_msg);
		sqlite3_free(err_msg);
		switch (ret) {
		case SQLITE_BUSY:
		case SQLITE_LOCKED:
			return CAL_DB_ERROR_LOCKED;
		case SQLITE_IOERR:
			return CAL_DB_ERROR_IOERR;
		case SQLITE_FULL:
			return CAL_DB_ERROR_NO_SPACE;
		default:
			return CAL_DB_ERROR_FAIL;
		}
	}
	if (err_msg)
	{
	    sqlite3_free(err_msg);
	}

	return CAL_DB_OK;
}

sqlite3_stmt* _cal_db_util_query_prepare(char *query)
{
	int ret = -1;
	sqlite3_stmt *stmt = NULL;

	retvm_if(NULL == query, NULL, "Invalid query");
	retvm_if(NULL == calendar_db_handle, NULL, "Database is not opended");
	//CALS_DBG("prepare query : %s", query);

	ret = sqlite3_prepare_v2(calendar_db_handle, query, strlen(query), &stmt, NULL);
	retvm_if(SQLITE_OK != ret, NULL,
			"sqlite3_prepare_v2(%s) Failed(%s).", query, sqlite3_errmsg(calendar_db_handle));

	return stmt;
}

cal_db_util_error_e _cal_db_util_stmt_step(sqlite3_stmt *stmt)
{
	int ret;
	ret = sqlite3_step(stmt);
    switch (ret) {
    case SQLITE_BUSY:
    case SQLITE_LOCKED:
        ret = CAL_DB_ERROR_LOCKED;
        break;
    case SQLITE_IOERR:
        ret = CAL_DB_ERROR_IOERR;
        break;
    case SQLITE_FULL:
        ret = CAL_DB_ERROR_NO_SPACE;
        break;
    case SQLITE_CONSTRAINT:
        ret = CAL_DB_ERROR_ALREADY_EXIST;
        break;
    case SQLITE_ROW:
        ret = CAL_DB_ROW;
        break;
    case SQLITE_DONE:
        ret = CAL_DB_DONE;
        break;
    default:
        ERR("sqlite3_step() Failed(%d)", ret);
        ret = CAL_DB_ERROR_FAIL;
        break;
    }
    return ret;
}

#define CAL_COMMIT_TRY_MAX 500000
int _cal_db_util_begin_trans(void)
{
	if(transaction_cnt <= 0)
	{
		int ret, progress;

		progress = 100000;
		ret = _cal_db_util_query_exec("BEGIN IMMEDIATE TRANSACTION");
		// !! check error code
		while(CAL_DB_OK != ret && progress < CAL_COMMIT_TRY_MAX) {
			usleep(progress);
			ret = _cal_db_util_query_exec("BEGIN IMMEDIATE TRANSACTION");
			progress *= 2;
		}
		retvm_if(CAL_DB_OK != ret, ret, "cal_query_exec() Failed(%d)", ret);

		transaction_cnt = 0;
		const char *query = "SELECT ver FROM "CAL_TABLE_VERSION;
		ret = _cal_db_util_query_get_first_int_result(query, NULL, &transaction_ver);
		if (CALENDAR_ERROR_NONE != ret)
		{
			ERR("_cal_db_util_query_get_first_int_result() failed");
			return ret;
		}
		version_up = false;
	}
	transaction_cnt++;
	CAL_DBG("transaction_cnt : %d", transaction_cnt);

	return CALENDAR_ERROR_NONE;
}

int _cal_db_util_end_trans(bool is_success)
{
	int ret;
	int progress = 0;
	char query[CAL_DB_SQL_MIN_LEN];

	transaction_cnt--;

	if (0 != transaction_cnt) {
		CAL_DBG("transaction_cnt : %d", transaction_cnt);
		return CALENDAR_ERROR_NONE;
	}

	if (false == is_success) {
		__cal_db_util_cancel_changes();
		ret = _cal_db_util_query_exec("ROLLBACK TRANSACTION");
		return CALENDAR_ERROR_NONE;
	}

	if (version_up) {
		transaction_ver++;
		snprintf(query, sizeof(query), "UPDATE %s SET ver = %d",
				CAL_TABLE_VERSION, transaction_ver);
		ret = _cal_db_util_query_exec(query);
		warn_if(CAL_DB_OK != ret, "cal_query_exec(version up) Failed(%d).", ret);
	}

	progress = 100000;
	ret = _cal_db_util_query_exec("COMMIT TRANSACTION");
	// !! check error code
	while (CAL_DB_OK != ret && progress < CAL_COMMIT_TRY_MAX) {
		usleep(progress);
		ret = _cal_db_util_query_exec("COMMIT TRANSACTION");
		progress *= 2;
	}
	if (CAL_DB_OK != ret) {
		int tmp_ret;
		ERR("cal_query_exec() Failed(%d)", ret);
		__cal_db_util_cancel_changes();
		tmp_ret = _cal_db_util_query_exec("ROLLBACK TRANSACTION");
		warn_if(CAL_DB_OK != tmp_ret, "cal_query_exec(ROLLBACK) Failed(%d).", tmp_ret);
		return CALENDAR_ERROR_DB_FAILED;
	}
	if (event_change) __cal_db_util_notify_event_change();
	if (todo_change) __cal_db_util_notify_todo_change();
	if (calendar_change) __cal_db_util_notify_calendar_change();

	CAL_DBG("transaction_ver = %d",transaction_ver);
	return CALENDAR_ERROR_NONE;
}

int _cal_db_util_get_next_ver(void)
{
	int count = 0;
	int ret;
	const char *query;

	if (0 < transaction_cnt) {
		version_up = true;
		return transaction_ver + 1;
	}

	query = "SELECT ver FROM "CAL_TABLE_VERSION;

	ret = _cal_db_util_query_get_first_int_result(query, NULL, &count);
	if (CALENDAR_ERROR_NONE != ret)
	{
		ERR("_cal_db_util_query_get_first_int_result() failed");
		return ret;
	}
	return (1 + count);
}

int _cal_db_util_get_transaction_ver(void)
{
    return transaction_ver;
}
