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

#ifndef __CAL_DB_UTIL_H__
#define __CAL_DB_UTIL_H__

#include <sqlite3.h>
#include "cal_typedef.h"

#define CAL_SQLITE_ROW SQLITE_ROW

int cal_db_util_open(void);
int cal_db_util_close(void);
int cal_db_util_query_prepare(char *query, sqlite3_stmt** stmt);
int cal_db_util_query_exec(char *query);
int cal_db_util_query_get_first_int_result(const char *query, GSList *bind_text, int *result);
int cal_db_util_begin_trans(void);
int cal_db_util_end_trans(bool is_success);
int cal_db_util_notify(cal_noti_type_e type);
int cal_db_util_last_insert_id(void);
int cal_db_util_get_next_ver(void);
int cal_db_util_stmt_bind_text(sqlite3_stmt *stmt, int pos, const char *str);
int cal_db_util_stmt_step(sqlite3_stmt *stmt);
int cal_db_util_get_transaction_ver(void);

#endif  /* __CAL_DB_UTIL_H__ */
