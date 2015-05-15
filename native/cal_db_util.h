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

#ifndef __CALENDAR_SVC_DB_UTIL_H__
#define __CALENDAR_SVC_DB_UTIL_H__

#include <sqlite3.h>
#include <string.h>

typedef enum
{
	CAL_DB_ERROR_FAIL   = -1,
	CAL_DB_ERROR_LOCKED = -204,         //SQLITE_BUSY, SQLITE_LOCKED
	CAL_DB_ERROR_IOERR = -10,           //SQLITE_IOERR              /* Some kind of disk I/O error occurred */
	CAL_DB_ERROR_NO_SPACE = -11,        //SQLITE_FULL               /* Insertion failed because database is full */
	CAL_DB_ERROR_ALREADY_EXIST = -7,    //SQLITE_CONSTRAINT         /* Abort due to constraint violation */
	CAL_DB_ROW = 1,                     //SQLITE_ROW    CAL_TRUE    /* sqlite3_step() has another row ready */
	CAL_DB_DONE = 0,                    //SQLITE_DONE   CAL_SUCCESS /* sqlite3_step() has finished executing */
	CAL_DB_OK = 0,                      //SQLITE_OK                 /* Successful result */
} cal_db_util_error_e;

int cal_db_util_notify(cal_noti_type_e type);

int cal_db_util_open(void);
int cal_db_util_close(void);

int cal_db_util_last_insert_id(void);
int cal_db_util_query_get_first_int_result(const char *query, GSList *bind_text, int *result);

cal_db_util_error_e cal_db_util_query_exec(char *query);
sqlite3_stmt* cal_db_util_query_prepare(char *query);
cal_db_util_error_e cal_db_util_stmt_step(sqlite3_stmt *stmt);

static inline int cal_db_util_stmt_bind_text(sqlite3_stmt *stmt, int pos, const char *str) {
	return sqlite3_bind_text(stmt, pos, str, str ? strlen(str) : 0, SQLITE_STATIC);
}

int cal_db_util_begin_trans(void);
int cal_db_util_end_trans(bool is_success);
int cal_db_util_get_next_ver(void);
int cal_db_util_get_transaction_ver(void);
void cal_db_util_set_permission(int fd);

#endif  /* __CALENDAR_SVC_DB_UTIL_H__ */
