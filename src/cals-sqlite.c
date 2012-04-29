/*
 * Calendar Service
 *
 * Copyright (c) 2000 - 2012 Samsung Electronics Co., Ltd. All rights reserved.
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
#include <db-util.h>

#include "cals-internal.h"
#include "cals-typedef.h"
#include "cals-db-info.h"
#include "cals-sqlite.h"

sqlite3 *calendar_db_handle;

int cals_db_open(void)
{
	int ret;

	if (!calendar_db_handle) {
		ret = db_util_open(CALS_DB_PATH, &calendar_db_handle, 0);
		retvm_if(SQLITE_OK != ret, CAL_ERR_DB_NOT_OPENED,
				"db_util_open() Failed(%d).", ret);
	}
	return CAL_SUCCESS;
}

int cals_db_close(void)
{
	int ret = 0;

	if (calendar_db_handle) {
		ret = db_util_close(calendar_db_handle);
		warn_if(SQLITE_OK != ret, "db_util_close() Failed(%d)", ret);
		calendar_db_handle = NULL;
		CALS_DBG("The database disconnected really.");
	}

	return CAL_SUCCESS;
}

int cals_last_insert_id(void)
{
	return sqlite3_last_insert_rowid(calendar_db_handle);
}

int cals_query_get_first_int_result(const char *query)
{
	int ret;
	sqlite3_stmt *stmt = NULL;
	retvm_if(NULL == calendar_db_handle, CAL_ERR_DB_NOT_OPENED, "Database is not opended");

	ret = sqlite3_prepare_v2(calendar_db_handle, query, strlen(query), &stmt, NULL);
	retvm_if(SQLITE_OK != ret, CAL_ERR_DB_FAILED,
			"sqlite3_prepare_v2(%s) failed(%s).", query, sqlite3_errmsg(calendar_db_handle));

	ret = sqlite3_step(stmt);
	if (SQLITE_ROW != ret) {
		ERR("sqlite3_step() failed(%d, %s).", ret, sqlite3_errmsg(calendar_db_handle));
		sqlite3_finalize(stmt);
		if (SQLITE_DONE == ret) return CAL_ERR_DB_RECORD_NOT_FOUND;
		return CAL_ERR_DB_FAILED;
	}

	ret = sqlite3_column_int(stmt, 0);
	sqlite3_finalize(stmt);

	return ret;
}


int cals_query_exec(char *query)
{
	int ret;
	char *err_msg = NULL;

	retvm_if(NULL == calendar_db_handle, CAL_ERR_DB_NOT_OPENED, "Database is not opended");
	//CALS_DBG("query : %s", query);

	ret = sqlite3_exec(calendar_db_handle, query, NULL, NULL, &err_msg);
	if (SQLITE_OK != ret) {
		ERR("sqlite3_exec(%s) failed(%d, %s).", query, ret, err_msg);
		sqlite3_free(err_msg);
		switch (ret) {
		case SQLITE_BUSY:
		case SQLITE_LOCKED:
			return CAL_ERR_DB_LOCK;
		case SQLITE_IOERR:
			return CAL_ERR_IO_ERR;
		case SQLITE_FULL:
			return CAL_ERR_NO_SPACE;
		default:
			return CAL_ERR_DB_FAILED;
		}
	}

	return CAL_SUCCESS;
}

sqlite3_stmt* cals_query_prepare(char *query)
{
	int ret = -1;
	sqlite3_stmt *stmt = NULL;

	retvm_if(NULL == calendar_db_handle, NULL, "Database is not opended");
	//CALS_DBG("prepare query : %s", query);

	ret = sqlite3_prepare_v2(calendar_db_handle, query, strlen(query), &stmt, NULL);
	retvm_if(SQLITE_OK != ret, NULL,
			"sqlite3_prepare_v2(%s) Failed(%s).", query, sqlite3_errmsg(calendar_db_handle));

	return stmt;
}

int cals_stmt_step(sqlite3_stmt *stmt)
{
	int ret;
	ret = sqlite3_step(stmt);
	switch (ret) {
	case SQLITE_BUSY:
	case SQLITE_LOCKED:
		ret = CAL_ERR_DB_LOCK;
		break;
	case SQLITE_IOERR:
		ret = CAL_ERR_IO_ERR;
		break;
	case SQLITE_FULL:
		ret = CAL_ERR_NO_SPACE;
		break;
	case SQLITE_CONSTRAINT:
		ret = CAL_ERR_ALREADY_EXIST;
		break;
	case SQLITE_ROW:
		ret = CAL_TRUE;
		break;
	case SQLITE_DONE:
		ret = CAL_SUCCESS;
		break;
	default:
		ERR("sqlite3_step() Failed(%d)", ret);
		ret = CAL_ERR_DB_FAILED;
		break;
	}
	return ret;
}


int cals_escape_like_pattern(const char *src, char * const dest, int dest_size)
{
	int s_pos=0, d_pos=0;

	if (NULL == src) {
		ERR("The parameter(src) is NULL");
		dest[d_pos] = '\0';
		return 0;
	}

	while (src[s_pos] != 0) {
		if (dest_size == d_pos - 1)
			break;
		if ('%' == src[s_pos] || '_' == src[s_pos]) {
			dest[d_pos++] = '\\';
		}
		dest[d_pos++] = src[s_pos++];
	}

	dest[d_pos] = '\0';

	return d_pos;
}


