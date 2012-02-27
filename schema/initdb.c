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
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <sqlite3.h>
#include <db-util.h>

#include "schema.h"
#include "cals-db-info.h"
#include "cals-internal.h"
#include "calendar-svc-errors.h"


static inline int remake_db_file()
{
	int ret, fd;
	char *errmsg;
	sqlite3 *db;

	ret = db_util_open(CALS_DB_PATH, &db, 0);
	retvm_if(SQLITE_OK != ret, CAL_ERR_DB_NOT_OPENED, "db_util_open() Failed(%d)", ret);

	ret = sqlite3_exec(db, schema_query, NULL, 0, &errmsg);
	if (SQLITE_OK != ret) {
		ERR("remake calendar DB file is Failed : %s", errmsg);
		sqlite3_free(errmsg);
	}

	db_util_close(db);

	fd = open(CALS_DB_PATH, O_CREAT | O_RDWR, 0660);
	retvm_if(-1 == fd, CAL_ERR_FAIL, "open Failed");

	fchown(fd, getuid(), CALS_SECURITY_FILE_GROUP);
	fchmod(fd, CALS_SECURITY_DEFAULT_PERMISSION);
	close(fd);

	fd = open(CALS_DB_JOURNAL_PATH, O_CREAT | O_RDWR, 0660);
	retvm_if(-1 == fd, CAL_ERR_FAIL, "open Failed");

	fchown(fd, getuid(), CALS_SECURITY_FILE_GROUP);
	fchmod(fd, CALS_SECURITY_DEFAULT_PERMISSION);
	close(fd);

	return CAL_SUCCESS;
}

static inline int check_db_file(void)
{
	int fd = open(CALS_DB_PATH, O_RDONLY);
	retvm_if(-1 == fd, -1,
			"DB file(%s) is not exist", CALS_DB_PATH);

	close(fd);
	return CAL_SUCCESS;
}

static inline int check_schema(void)
{
	if (check_db_file())
		remake_db_file();

	return CAL_SUCCESS;
}

int main(int argc, char **argv)
{
	return check_schema();
}
