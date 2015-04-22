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
#include <sys/time.h>

#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_db.h"
#include "cal_db_util.h"

// For Security
#define CALS_SECURITY_FILE_GROUP 6003

static TLS sqlite3 *calendar_db_handle;
static TLS int transaction_cnt = 0;
static TLS int transaction_ver = 0;
static TLS bool version_up = false;

static TLS bool event_change=false;
static TLS bool todo_change=false;
static TLS bool calendar_change=false;

static inline int create_noti_file(const char* noti_file)
{
	int fd, ret;
	fd = creat(noti_file, CAL_SECURITY_DEFAULT_PERMISSION);
	if (-1 == fd)
	{
		printf("open Failed\n");
		return -1;
	}

	ret = fchown(fd, -1, CALS_SECURITY_FILE_GROUP);
	if (-1 == ret)
	{
		printf("Failed to fchown\n");
		close(fd);
		return -1;
	}
}

static inline void __cal_db_util_notify_event_change(void)
{
	if (-1 == access (DATA_PATH, F_OK))
	{
	mkdir(DATA_PATH, 755);
	}
	if (-1 == access (CAL_DATA_PATH, F_OK))
	{
	mkdir(CAL_DATA_PATH, 755);
	}
	create_noti_file(CAL_NOTI_EVENT_CHANGED);
	int fd = open(CAL_NOTI_EVENT_CHANGED, O_TRUNC | O_RDWR);
	if (0 <= fd) {
		close(fd);
		event_change = false;
	}
}

static inline void __cal_db_util_notify_todo_change(void)
{
    if (-1 == access (DATA_PATH, F_OK))
   {
	mkdir(DATA_PATH, 755);
	}
	if (-1 == access (CAL_DATA_PATH, F_OK))
	{
	mkdir(CAL_DATA_PATH, 755);
	}
	create_noti_file(CAL_NOTI_TODO_CHANGED);
	int fd = open(CAL_NOTI_TODO_CHANGED, O_TRUNC | O_RDWR);
	if (0 <= fd) {
		close(fd);
		todo_change = false;
	}
}

static inline void __cal_db_util_notify_calendar_change(void)
{
	if (-1 == access (DATA_PATH, F_OK))
	{
		mkdir(DATA_PATH, 755);
	}
	if (-1 == access (CAL_DATA_PATH, F_OK))
	{
		mkdir(CAL_DATA_PATH, 755);
	}
	create_noti_file(CAL_NOTI_CALENDAR_CHANGED);
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
		if (-1 == access (DB_PATH, F_OK))
		{
			mkdir(DB_PATH, 755);
		}
		if (-1 == access (CAL_DB_FILE, F_OK))
		{
			mkdir(DB_PATH, 755);
		}
		ret = db_util_open(CAL_DB_FILE, &calendar_db_handle, 0);
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

#define __CAL_QUERY_RETRY_TIME 2

int _cal_db_util_query_get_first_int_result(const char *query, GSList *bind_text, int *result)
{
	int ret;
	int index;
	char *text = NULL;
	struct timeval from, now, diff;
	bool retry = false;
	sqlite3_stmt *stmt = NULL;
	retvm_if(NULL == calendar_db_handle, CALENDAR_ERROR_DB_FAILED, "Database is not opended");

	gettimeofday(&from, NULL);
	do
	{
		ret = sqlite3_prepare_v2(calendar_db_handle, query, strlen(query), &stmt, NULL);
		if (SQLITE_BUSY == ret || SQLITE_LOCKED == ret)
		{
			ERR("sqlite3_prepare_v2(%s) failed(%s).", query, sqlite3_errmsg(calendar_db_handle));
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
		ERR("sqlite3_prepare_v2(%s) failed(%s).", query, sqlite3_errmsg(calendar_db_handle));
		return CALENDAR_ERROR_DB_FAILED;
	}

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
				ERR("sqlite3_step fail(%d, %s)", ret, sqlite3_errmsg(calendar_db_handle));
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
				ERR("sqlite3_step() failed(%d, %s).", ret, sqlite3_errmsg(calendar_db_handle));
				retry = false;
			}
		}
		else
			if (result) *result = sqlite3_column_int(stmt, 0);
	} while(retry);

	sqlite3_finalize(stmt);

	return CALENDAR_ERROR_NONE;
}

cal_db_util_error_e _cal_db_util_query_exec(char *query)
{
	int ret;
	sqlite3_stmt *stmt = NULL;

	retvm_if(NULL == calendar_db_handle, CALENDAR_ERROR_DB_FAILED, "Database is not opended");

	stmt = _cal_db_util_query_prepare(query);
	retvm_if(NULL == stmt, CAL_DB_ERROR_FAIL, "_cal_db_util_query_prepare() Failed");

	ret = _cal_db_util_stmt_step(stmt);

	if (CAL_DB_DONE != ret) {
		sqlite3_finalize(stmt);
		ERR("_cal_db_util_stmt_step() Failed(%d)", ret);
		SEC_ERR("[ %s ]", query);
		return ret;
	}

	sqlite3_finalize(stmt);
	return CAL_DB_OK;
}

sqlite3_stmt* _cal_db_util_query_prepare(char *query)
{
	int ret = -1;
	struct timeval from, now, diff;
	bool retry = false;
	sqlite3_stmt *stmt = NULL;

	retvm_if(NULL == query, NULL, "Invalid query");
	retvm_if(NULL == calendar_db_handle, NULL, "Database is not opended");
	//CALS_DBG("prepare query : %s", query);

	gettimeofday(&from, NULL);
	do
	{
		ret = sqlite3_prepare_v2(calendar_db_handle, query, strlen(query), &stmt, NULL);
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

	return stmt;
}

cal_db_util_error_e _cal_db_util_stmt_step(sqlite3_stmt *stmt)
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

	INFO("start commit");
	progress = 100000;
	ret = _cal_db_util_query_exec("COMMIT TRANSACTION");
	// !! check error code
	while (CAL_DB_OK != ret && progress < CAL_COMMIT_TRY_MAX) {
		usleep(progress);
		ret = _cal_db_util_query_exec("COMMIT TRANSACTION");
		progress *= 2;
	}
	INFO("%s", (CAL_DB_OK == ret)?"commit": "rollback");

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
