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
#include <sys/stat.h>
#include <string.h>
#include <sqlite3.h>
#include <db-util.h>

#include "schema.h"

#define CALS_DB_PATH "/opt/usr/dbspace/.calendar-svc.db"
#define CALS_DB_JOURNAL_PATH "/opt/usr/dbspace/.calendar-svc.db-journal"

// For Security
#define CALS_SECURITY_FILE_GROUP 6003
#define CALS_SECURITY_DEFAULT_PERMISSION 0660
#define CALS_SECURITY_DIR_DEFAULT_PERMISSION 0770

static inline int remake_db_file()
{
	int ret, fd;
	char *errmsg;
	sqlite3 *db;

	ret = db_util_open(CALS_DB_PATH, &db, 0);
	if (SQLITE_OK != ret)
	{
		printf("db_util_open() Failed(%d)\n", ret);
		return -1;
	}

	ret = sqlite3_exec(db, schema_query, NULL, 0, &errmsg);
	if (SQLITE_OK != ret) {
		printf("remake calendar DB file is Failed : %s\n", errmsg);
		sqlite3_free(errmsg);
	}

	db_util_close(db);

	fd = open(CALS_DB_PATH, O_CREAT | O_RDWR, 0660);
	if (-1 == fd)
	{
		printf("open Failed\n");
		return -1;
	}

	ret = fchown(fd, getuid(), CALS_SECURITY_FILE_GROUP);
	if (-1 == ret)
	{
		printf("Failed to fchown\n");
		close(fd);
		return -1;
	}
	fchmod(fd, CALS_SECURITY_DEFAULT_PERMISSION);
	close(fd);

	fd = open(CALS_DB_JOURNAL_PATH, O_CREAT | O_RDWR, 0660);
	if (-1 == fd)
	{
		printf("open Failed\n");
		return -1;
	}

	ret = fchown(fd, getuid(), CALS_SECURITY_FILE_GROUP);
	if (-1 == ret)
	{
		printf("Failed to fchown\n");
		close(fd);
		return -1;
	}
	fchmod(fd, CALS_SECURITY_DEFAULT_PERMISSION);
	close(fd);

	return 0;
}

static inline int check_db_file(void)
{
	int fd = open(CALS_DB_PATH, O_RDONLY);
	if (-1 == fd)
	{
		printf("DB file(%s) is not exist\n", CALS_DB_PATH);
		return -1;
	}

	close(fd);
	return 0;
}

static inline int check_schema(void)
{
	if (check_db_file())
		remake_db_file();

	return 0;
}

int main(int argc, char **argv)
{
	return check_schema();
}
