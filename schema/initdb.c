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

#define CALS_DB_NAME ".calendar-svc.db"
#define CALS_DB_JOURNAL_NAME ".calendar-svc.db-journal"
#define CALS_DB_PATH "/opt/usr/dbspace/"CALS_DB_NAME
#define CALS_DB_JOURNAL_PATH "/opt/usr/dbspace/"CALS_DB_JOURNAL_NAME

// For Security
#define CALS_SECURITY_FILE_GROUP 6003
#define CALS_SECURITY_DEFAULT_PERMISSION 0660
#define CALS_SECURITY_DIR_DEFAULT_PERMISSION 0770

static inline int remake_db_file(char* db_path)
{
	int ret, fd;
	char *errmsg;
	sqlite3 *db;
	char db_file[256]={0,};
	char db_journal_file[256]={0,};

	if(db_path == NULL)
	{
	    snprintf(db_file,sizeof(db_file),CALS_DB_PATH);
	    snprintf(db_journal_file,sizeof(db_journal_file),CALS_DB_JOURNAL_PATH);
	}
	else
	{
	    snprintf(db_file,sizeof(db_file),"%s%s",db_path, CALS_DB_NAME);
	    snprintf(db_journal_file,sizeof(db_journal_file),"%s%s", db_path, CALS_DB_JOURNAL_NAME);
	}

	ret = db_util_open(db_file, &db, 0);

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

	fd = open(db_file, O_CREAT | O_RDWR, 0660);
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

	fd = open(db_journal_file, O_CREAT | O_RDWR, 0660);

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

static inline int check_db_file(char* db_path)
{
    int fd = -1;

    char db_file[256]={0,};
    if(db_path == NULL)
    {
        snprintf(db_file,sizeof(db_file),CALS_DB_PATH);
    }
    else
    {
        snprintf(db_file,sizeof(db_file),"%s%s",db_path, CALS_DB_NAME);
    }

    fd = open(db_file, O_RDONLY);

	if (-1 == fd)
	{
		printf("DB file(%s) is not exist\n", db_file);
		return -1;
	}
	printf("DB file(%s) \n", db_file);
	close(fd);
	return 0;
}

static inline int check_schema(char* db_path)
{
	if (check_db_file(db_path))
		remake_db_file(db_path);
	return 0;
}

int main(int argc, char **argv)
{
    char *tmp = NULL;
    if(argc > 1)
    {
        tmp = argv[1];
    }
	return check_schema(tmp);
}
