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
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <db-util.h>
#include <sys/time.h>

#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_db.h"
#include "cal_db_util.h"

// For Security
#define CALS_SECURITY_FILE_GROUP 6003

static TLS sqlite3 *cal_db = NULL;
static TLS int transaction_cnt = 0;
static TLS int transaction_ver = 0;
static TLS bool version_up = false;

static TLS bool event_change=false;
static TLS bool todo_change=false;
static TLS bool calendar_change=false;

static inline void _cal_db_util_notify_event_change(void)
{
	int fd = open(CAL_NOTI_EVENT_CHANGED, O_TRUNC | O_RDWR);
	if (0 <= fd) {
		close(fd);
		event_change = false;
	}
}

static inline void _cal_db_util_notify_todo_change(void)
{
	int fd = open(CAL_NOTI_TODO_CHANGED, O_TRUNC | O_RDWR);
	if (0 <= fd) {
		close(fd);
		todo_change = false;
	}
}

static inline void _cal_db_util_notify_calendar_change(void)
{
	int fd = open(CAL_NOTI_CALENDAR_CHANGED, O_TRUNC | O_RDWR);
	if (0 <= fd) {
		close(fd);
		calendar_change = false;
	}
}

static inline void _cal_db_util_cancel_changes(void)
{
	event_change = false;
	calendar_change = false;
	todo_change = false;
}

int cal_db_util_notify(cal_noti_type_e type)
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
		_cal_db_util_notify_event_change();
		break;
	case CAL_NOTI_TYPE_TODO:
		_cal_db_util_notify_todo_change();
		break;
	case CAL_NOTI_TYPE_CALENDAR:
		_cal_db_util_notify_calendar_change();
		break;
	default:
		ERR("The type(%d) is not supported", type);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

int cal_db_util_open(void)
{
	int ret = 0;

	if (!cal_db) {
		if (-1 == access (DB_PATH, F_OK))
		{
			mkdir(DB_PATH, 755);
		}
		if (-1 == access (CAL_DB_FILE, F_OK))
		{
			mkdir(DB_PATH, 755);
		}
		ret = db_util_open(CAL_DB_FILE, &cal_db, 0);
		RETVM_IF(SQLITE_OK != ret, CALENDAR_ERROR_DB_FAILED,
				"db_util_open() Fail(%d).", ret);
	}
	return CALENDAR_ERROR_NONE;
}

int cal_db_util_close(void)
{
	int ret = 0;

	if (cal_db) {
		ret = db_util_close(cal_db);
		WARN_IF(SQLITE_OK != ret, "db_util_close() Fail(%d)", ret);
		cal_db = NULL;
		DBG("The database disconnected really.");
	}

	return CALENDAR_ERROR_NONE;
}

int cal_db_util_last_insert_id(void)
{
	return sqlite3_last_insert_rowid(cal_db);
}

#define __CAL_QUERY_RETRY_TIME 2

int cal_db_util_query_get_first_int_result(const char *query, GSList *bind_text, int *result)
{
	int ret;
	int index;
	char *text = NULL;
	struct timeval from, now, diff;
	bool retry = false;
	sqlite3_stmt *stmt = NULL;
	RETVM_IF(NULL == cal_db, CALENDAR_ERROR_DB_FAILED, "Database is not opended");

	gettimeofday(&from, NULL);
	do
	{
		ret = sqlite3_prepare_v2(cal_db, query, strlen(query), &stmt, NULL);
		if (SQLITE_BUSY == ret || SQLITE_LOCKED == ret)
		{
			ERR("sqlite3_prepare_v2(%s) failed(%s).", query, sqlite3_errmsg(cal_db));
			gettimeofday(&now, NULL);
			timersub(&now, &from, &diff);
			retry = (diff.tv_sec < __CAL_QUERY_RETRY_TIME) ? true : false;
			if (retry)
			{
				usleep(50 * 1000); // 50ms
			}
		}
		else
		{
			retry = false;
		}
	} while(retry);

	if (SQLITE_OK != ret)
	{
		ERR("sqlite3_prepare_v2(%s) failed(%s).", query, sqlite3_errmsg(cal_db));
		return CALENDAR_ERROR_DB_FAILED;
	}

	if (bind_text)
	{
		for (index = 0; g_slist_nth(bind_text, index); index++)
		{
			text = (char *)g_slist_nth_data(bind_text, index);
			if (text)
			{
				cal_db_util_stmt_bind_text(stmt, index + 1, text);
			}
		}
	}

	retry = false;
	gettimeofday(&from, NULL);
	do
	{
		ret = sqlite3_step(stmt);
		if (SQLITE_ROW != ret)
		{
			if (SQLITE_DONE == ret)
			{
				sqlite3_finalize(stmt);
				return CALENDAR_ERROR_DB_RECORD_NOT_FOUND;
			}
			else if (SQLITE_BUSY == ret || SQLITE_LOCKED == ret)
			{
				ERR("sqlite3_step fail(%d, %s)", ret, sqlite3_errmsg(cal_db));
				gettimeofday(&now, NULL);
				timersub(&now, &from, &diff);
				retry = (diff.tv_sec < __CAL_QUERY_RETRY_TIME) ? true : false;
				if (retry)
				{
					usleep(50 * 1000); // 50ms
				}
			}
			else
			{
				ERR("sqlite3_step() failed(%d, %s).", ret, sqlite3_errmsg(cal_db));
				retry = false;
			}
		}
		else
			if (result) *result = sqlite3_column_int(stmt, 0);
	} while(retry);

	sqlite3_finalize(stmt);

	return CALENDAR_ERROR_NONE;
}

cal_db_util_error_e cal_db_util_query_exec(char *query)
{
	int ret;
	sqlite3_stmt *stmt = NULL;

	RETVM_IF(NULL == cal_db, CALENDAR_ERROR_DB_FAILED, "Database is not opended");

	stmt = cal_db_util_query_prepare(query);
	RETVM_IF(NULL == stmt, CAL_DB_ERROR_FAIL, "cal_db_util_query_prepare() Fail");

	ret = cal_db_util_stmt_step(stmt);

	if (CAL_DB_DONE != ret) {
		sqlite3_finalize(stmt);
		ERR("cal_db_util_stmt_step() Fail(%d)", ret);
		SEC_ERR("[ %s ]", query);
		return ret;
	}

	sqlite3_finalize(stmt);
	return CAL_DB_OK;
}

sqlite3_stmt* cal_db_util_query_prepare(char *query)
{
	int ret = -1;
	struct timeval from, now, diff;
	bool retry = false;
	sqlite3_stmt *stmt = NULL;

	RETV_IF(NULL == query, NULL);
	RETV_IF(NULL == cal_db, NULL);

	gettimeofday(&from, NULL);
	do {
		ret = sqlite3_prepare_v2(cal_db, query, strlen(query), &stmt, NULL);
		if (SQLITE_BUSY == ret || SQLITE_LOCKED == ret) {
			gettimeofday(&now, NULL);
			timersub(&now, &from, &diff);
			retry = (diff.tv_sec < __CAL_QUERY_RETRY_TIME) ? true : false;
			if (retry)
			{
				usleep(50 * 1000); // 50ms
			}
		}
		else
		{
			retry = false;
		}
	} while(retry);

	return stmt;
}

cal_db_util_error_e cal_db_util_stmt_step(sqlite3_stmt *stmt)
{
	int ret;
	struct timeval from, now, diff;
	bool retry = false;

	gettimeofday(&from, NULL);
	do
	{
		ret = sqlite3_step(stmt);
		if (SQLITE_BUSY == ret || SQLITE_LOCKED == ret)
		{
			gettimeofday(&now, NULL);
			timersub(&now, &from, &diff);
			retry = (diff.tv_sec < __CAL_QUERY_RETRY_TIME) ? true : false;
			if (retry)
			{
				usleep(50 * 1000); // 50ms
			}
		}
		else
		{
			retry = false;
		}
	} while(retry);

	switch (ret)
	{
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
		ERR("sqlite3_step() Fail(%d)", ret);
		ret = CAL_DB_ERROR_FAIL;
		break;
	}
	return ret;
}

#define CAL_COMMIT_TRY_MAX 500000
int cal_db_util_begin_trans(void)
{
	if (transaction_cnt <= 0)
	{
		int ret, progress;

		progress = 100000;
		ret = cal_db_util_query_exec("BEGIN IMMEDIATE TRANSACTION");
		// !! check error code
		while(CAL_DB_OK != ret && progress < CAL_COMMIT_TRY_MAX) {
			usleep(progress);
			ret = cal_db_util_query_exec("BEGIN IMMEDIATE TRANSACTION");
			progress *= 2;
		}
		RETVM_IF(CAL_DB_OK != ret, ret, "cal_query_exec() Fail(%d)", ret);

		transaction_cnt = 0;
		const char *query = "SELECT ver FROM "CAL_TABLE_VERSION;
		ret = cal_db_util_query_get_first_int_result(query, NULL, &transaction_ver);
		if (CALENDAR_ERROR_NONE != ret)
		{
			ERR("cal_db_util_query_get_first_int_result() failed");
			return ret;
		}
		version_up = false;
	}
	transaction_cnt++;
	DBG("transaction_cnt : %d", transaction_cnt);

	return CALENDAR_ERROR_NONE;
}

int cal_db_util_end_trans(bool is_success)
{
	int ret;
	int progress = 0;
	char query[CAL_DB_SQL_MIN_LEN];

	transaction_cnt--;

	if (0 != transaction_cnt) {
		DBG("transaction_cnt : %d", transaction_cnt);
		return CALENDAR_ERROR_NONE;
	}

	if (false == is_success) {
		_cal_db_util_cancel_changes();
		ret = cal_db_util_query_exec("ROLLBACK TRANSACTION");
		return CALENDAR_ERROR_NONE;
	}

	if (version_up) {
		transaction_ver++;
		snprintf(query, sizeof(query), "UPDATE %s SET ver = %d",
				CAL_TABLE_VERSION, transaction_ver);
		ret = cal_db_util_query_exec(query);
		WARN_IF(CAL_DB_OK != ret, "cal_query_exec(version up) Fail(%d).", ret);
	}

	INFO("start commit");
	progress = 100000;
	ret = cal_db_util_query_exec("COMMIT TRANSACTION");
	// !! check error code
	while (CAL_DB_OK != ret && progress < CAL_COMMIT_TRY_MAX) {
		usleep(progress);
		ret = cal_db_util_query_exec("COMMIT TRANSACTION");
		progress *= 2;
	}
	INFO("%s", (CAL_DB_OK == ret)?"commit": "rollback");

	if (CAL_DB_OK != ret) {
		int tmp_ret;
		ERR("cal_query_exec() Fail(%d)", ret);
		_cal_db_util_cancel_changes();
		tmp_ret = cal_db_util_query_exec("ROLLBACK TRANSACTION");
		WARN_IF(CAL_DB_OK != tmp_ret, "cal_query_exec(ROLLBACK) Fail(%d).", tmp_ret);
		return CALENDAR_ERROR_DB_FAILED;
	}
	if (event_change) _cal_db_util_notify_event_change();
	if (todo_change) _cal_db_util_notify_todo_change();
	if (calendar_change) _cal_db_util_notify_calendar_change();

	DBG("transaction_ver = %d",transaction_ver);
	return CALENDAR_ERROR_NONE;
}

int cal_db_util_get_next_ver(void)
{
	int count = 0;
	int ret;
	const char *query;

	if (0 < transaction_cnt) {
		version_up = true;
		return transaction_ver + 1;
	}

	query = "SELECT ver FROM "CAL_TABLE_VERSION;

	ret = cal_db_util_query_get_first_int_result(query, NULL, &count);
	if (CALENDAR_ERROR_NONE != ret)
	{
		ERR("cal_db_util_query_get_first_int_result() failed");
		return ret;
	}
	return (1 + count);
}

int cal_db_util_get_transaction_ver(void)
{
	return transaction_ver;
}
