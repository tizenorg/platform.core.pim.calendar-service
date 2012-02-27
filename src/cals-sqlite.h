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
#ifndef __CALENDAR_SVC_SQLITE_H__
#define __CALENDAR_SVC_SQLITE_H__

#include <sqlite3.h>

#define CALS_SQL_MAX_LEN 2048
#define CALS_SQL_MIN_LEN 1024

int cals_db_open(void);
int cals_db_close(void);

int cals_last_insert_id(void);

int cals_query_get_first_int_result(const char *query);
int cals_query_exec(char *query);

sqlite3_stmt* cals_query_prepare(char *query);
int cals_stmt_step(sqlite3_stmt *stmt);

static inline int cals_stmt_bind_text(sqlite3_stmt *stmt, int pos, const char *str) {
	return sqlite3_bind_text(stmt, pos, str, strlen(str), SQLITE_STATIC);
}

#endif /* __CALENDAR_SVC_SQLITE_H__ */
