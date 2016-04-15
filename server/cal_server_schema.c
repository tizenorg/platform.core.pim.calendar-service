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
#include <sys/stat.h>
#include <string.h>
#include <sqlite3.h>
#include <db-util.h>

#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_db.h"
#include "schema.h"

static inline int __remake_db_file(char* db_path)
{
	int ret;
	char *errmsg;
	sqlite3 *db;
	char db_file[CAL_STR_MIDDLE_LEN] = {0};

	snprintf(db_file, sizeof(db_file), "%s/%s", db_path ? db_path : DB_PATH, CALS_DB_NAME);

	ret = db_util_open(db_file, &db, 0);
	if (SQLITE_OK != ret) {
		ERR("db_util_open() Fail(%d) ", ret);
		return -1;
	}

	ret = sqlite3_exec(db, schema_query, NULL, 0, &errmsg);
	if (SQLITE_OK != ret) {
		ERR("sqlite3_exec() Fail[%s]", errmsg);
		sqlite3_free(errmsg);
	}
	db_util_close(db);
	return 0;
}

static inline int __check_db_file(char* db_path)
{
	int fd = -1;
	char db_file[CAL_STR_MIDDLE_LEN] = {0,};
	snprintf(db_file, sizeof(db_file), "%s/%s", db_path ? db_path : DB_PATH, CALS_DB_NAME);
	DBG("[%s]", db_file);

	fd = open(db_file, O_RDONLY);
	if (fd < 0) {
		ERR("DB file(%s) is not exist(err:%d) ", db_file, fd);
		return -1;
	}
	close(fd);
	return 0;
}

int cal_server_schema_check(void)
{
	if (__check_db_file(NULL))
		__remake_db_file(NULL);
	return 0;
}
