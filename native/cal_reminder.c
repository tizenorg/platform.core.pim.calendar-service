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

#include "calendar_reminder.h"

#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_view.h"
#include "cal_time.h"
#include "cal_record.h"

#include "cal_db_util.h"
#include "cal_db.h"

API int calendar_reminder_add_receiver(const char *pkgname, const char *extra_data_key, const char *extra_data_value)
{
	int index;
	int count = 0;
    cal_db_util_error_e dbret = CAL_DB_OK;
	char query[CAL_DB_SQL_MAX_LEN];
	sqlite3_stmt *stmt;

	if (pkgname == NULL || strlen(pkgname) == 0)
	{
		ERR("Invalid parameter");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	// check whether already registered
	snprintf(query, sizeof(query), "SELECT count(*) FROM %s WHERE pkgname = ? ",
			CAL_REMINDER_ALERT);
	stmt = _cal_db_util_query_prepare(query);
	if (NULL == stmt)
	{
		ERR("_cal_db_util_query_prepare() Failed");
		return CALENDAR_ERROR_DB_FAILED;
	}
	_cal_db_util_stmt_bind_text(stmt, 1, pkgname);

	dbret = _cal_db_util_stmt_step(stmt);
	if (CAL_DB_ROW == dbret)
	{
		index = 0;
		count = sqlite3_column_int(stmt, index++);
	}
	sqlite3_finalize(stmt);

	if (count > 0)
	{
		DBG("Already registered pkgname[%s]", pkgname);
		return CALENDAR_ERROR_NONE;
	}

	// register
	snprintf(query, sizeof(query),
			"INSERT INTO %s ( pkgname, key, value ) VALUES ( ?, ?, ? ) ",
			CAL_REMINDER_ALERT);
	stmt = _cal_db_util_query_prepare(query);
	if (NULL == stmt)
	{
		ERR("_cal_db_util_query_prepare() Failed");
		return CALENDAR_ERROR_DB_FAILED;
	}
	index = 1;
	_cal_db_util_stmt_bind_text(stmt, index++, pkgname);
	_cal_db_util_stmt_bind_text(stmt, index++, extra_data_key);
	_cal_db_util_stmt_bind_text(stmt, index++, extra_data_value);
	dbret = _cal_db_util_stmt_step(stmt);
	sqlite3_finalize(stmt);

	if (CAL_DB_DONE != dbret)
	{
		ERR("_cal_db_util_stmt_step() Failed(%d)", dbret);
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

API int calendar_reminder_remove_receiver(const char *pkgname)
{
    cal_db_util_error_e dbret = CAL_DB_OK;
	char query[CAL_DB_SQL_MAX_LEN];
	sqlite3_stmt *stmt;

	if (pkgname == NULL || strlen(pkgname) == 0)
	{
		ERR("Invalid parameter");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	snprintf(query, sizeof(query), "DELETE FROM %s WHERE pkgname = ? ",
			CAL_REMINDER_ALERT);
	stmt = _cal_db_util_query_prepare(query);
	if (NULL == stmt)
	{
		ERR("_cal_db_util_query_prepare() Failed");
		return CALENDAR_ERROR_DB_FAILED;
	}
	_cal_db_util_stmt_bind_text(stmt, 1, pkgname);
	dbret = _cal_db_util_stmt_step(stmt);
	sqlite3_finalize(stmt);

	if (CAL_DB_DONE != dbret)
	{
		ERR("_cal_db_util_stmt_step() Failed(%d)", dbret);
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

API int calendar_reminder_activate_receiver(const char *pkgname)
{
    cal_db_util_error_e dbret = CAL_DB_OK;
	char query[CAL_DB_SQL_MAX_LEN];
	sqlite3_stmt *stmt;

	if (pkgname == NULL || strlen(pkgname) == 0)
	{
		ERR("Invalid parameter");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	snprintf(query, sizeof(query), "UPDATE %s SET onoff = 1 WHERE pkgname = ? ",
			CAL_REMINDER_ALERT);
	stmt = _cal_db_util_query_prepare(query);
	if (NULL == stmt)
	{
		ERR("_cal_db_util_query_prepare() Failed");
		return CALENDAR_ERROR_DB_FAILED;
	}
	_cal_db_util_stmt_bind_text(stmt, 1, pkgname);
	dbret = _cal_db_util_stmt_step(stmt);
	sqlite3_finalize(stmt);

	if (CAL_DB_DONE != dbret)
{
		ERR("_cal_db_util_stmt_step() Failed(%d)", dbret);
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

API int calendar_reminder_deactivate_receiver(const char *pkgname)
{
    cal_db_util_error_e dbret = CAL_DB_OK;
	char query[CAL_DB_SQL_MAX_LEN];
	sqlite3_stmt *stmt;

	if (pkgname == NULL || strlen(pkgname) == 0)
	{
		ERR("Invalid parameter");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	snprintf(query, sizeof(query), "UPDATE %s SET onoff = 0 WHERE pkgname = ? ",
			CAL_REMINDER_ALERT);
	stmt = _cal_db_util_query_prepare(query);
	if (NULL == stmt)
	{
		ERR("_cal_db_util_query_prepare() Failed");
		return CALENDAR_ERROR_DB_FAILED;
	}
	_cal_db_util_stmt_bind_text(stmt, 1, pkgname);
	dbret = _cal_db_util_stmt_step(stmt);
	sqlite3_finalize(stmt);

	if (CAL_DB_DONE != dbret)
	{
		ERR("_cal_db_util_stmt_step() Failed(%d)", dbret);
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

API int calendar_reminder_has_receiver(const char *pkgname)
{
	int index;
	int count = 0;
    cal_db_util_error_e ret = CAL_DB_OK;
	char query[CAL_DB_SQL_MAX_LEN];
	sqlite3_stmt *stmt;

	if (pkgname == NULL || strlen(pkgname) == 0)
	{
		ERR("Invalid parameter");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	snprintf(query, sizeof(query), "SELECT count(*) FROM %s WHERE pkgname = ? ",
			CAL_REMINDER_ALERT);
	stmt = _cal_db_util_query_prepare(query);
	if (NULL == stmt)
	{
		ERR("_cal_db_util_query_prepare() Failed");
		return CALENDAR_ERROR_DB_FAILED;
	}
	_cal_db_util_stmt_bind_text(stmt, 1, pkgname);

	ret = _cal_db_util_stmt_step(stmt);
	if (CAL_DB_ROW == ret) {
		index = 0;
		count = sqlite3_column_int(stmt, index++);
	}
	sqlite3_finalize(stmt);

	return count > 0 ? true : false;
}

API int calendar_reminder_add_cb(calendar_reminder_cb callback, void *user_data)
{
	ERR("This API[%s] is not valid in native library.", __func__);
	ERR("If you want to use this API, please use in client library.");
	return CALENDAR_ERROR_NOT_PERMITTED;
}

API int calendar_reminder_remove_cb(calendar_reminder_cb callback, void *user_data)
{
	ERR("This API[%s] is not valid in native library.", __func__);
	ERR("If you want to use this API, please use in client library.");
	return CALENDAR_ERROR_NOT_PERMITTED;
}

