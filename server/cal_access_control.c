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

#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <sys/smack.h>

#include "calendar_db.h"
#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_view.h"
#include "cal_time.h"
#include "cal_record.h"
#include "cal_mutex.h"
#include "cal_db_util.h"
#include "cal_db.h"
#include "cal_access_control.h"
#include "cal_server_service.h"
#include "cal_utils.h"

typedef struct {
	unsigned int thread_id;
	void *ipc;
	char *smack_label;
	int *write_list;
	int write_list_count;
} cal_permission_info_s;

enum {
	CAL_SMACK_NOT_CHECKED,
	CAL_SMACK_ENABLED, /* 1 */
	CAL_SMACK_DISABLED, /* 0 */
};

static GList *__thread_list = NULL;
static int have_smack = CAL_SMACK_NOT_CHECKED;

static cal_permission_info_s* _cal_access_control_find_permission_info(unsigned int thread_id)
{
	GList *cursor;

	for (cursor = __thread_list; cursor; cursor = cursor->next) {
		cal_permission_info_s *info = NULL;
		info = cursor->data;
		if (info->thread_id == thread_id)
			return info;
	}
	return NULL;
}

/* check SMACK enable or disable */
static int _cal_have_smack(void)
{
	if (CAL_SMACK_NOT_CHECKED == have_smack) {
		if (NULL == smack_smackfs_path())
			have_smack = CAL_SMACK_DISABLED;
		else
			have_smack = CAL_SMACK_ENABLED;
	}
	return have_smack;
}

static void _cal_access_control_set_permission_info(cal_permission_info_s *info)
{
	int ret = 0;
	bool smack_enabled = false;

	if (CAL_SMACK_ENABLED == _cal_have_smack())
		smack_enabled = true;
	else
		INFO("SAMCK disabled");

	/* white listing : core module */
	free(info->write_list);
	info->write_list = NULL;
	info->write_list_count = 0;

	char query[CAL_DB_SQL_MAX_LEN] = {0};
	int count = 0;
	snprintf(query, sizeof(query), "SELECT count(id) FROM %s WHERE deleted = 0 ", CAL_TABLE_CALENDAR);
	ret = cal_db_util_query_get_first_int_result(query, NULL, &count);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_query_get_first_int_result() Fail(%d)", ret);
		SECURE("query[%s]", query);
		return;
	}

	info->write_list = calloc(count +1, sizeof(int));
	if (NULL == info->write_list) {
		ERR("calloc() Fail");
		return;
	}
	info->write_list_count = 0;

	sqlite3_stmt *stmt = NULL;
	snprintf(query, sizeof(query), "SELECT id, mode, owner_label FROM %s WHERE deleted = 0 ", CAL_TABLE_CALENDAR);
	ret = cal_db_util_query_prepare(query, &stmt);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_query_prepare() Fail(%d)", ret);
		SECURE("query[%s]", query);
		return;
	}

	int write_index = 0;
	while (CAL_SQLITE_ROW == cal_db_util_stmt_step(stmt)) {
		int id = 0;
		int mode = 0;
		char *temp = NULL;

		id = sqlite3_column_int(stmt, 0);
		mode = sqlite3_column_int(stmt, 1);
		temp = (char *)sqlite3_column_text(stmt, 2);

		if (!smack_enabled) /* smack disabled */
			info->write_list[write_index++] = id;
		else if (NULL == info->ipc) /* calendar-service daemon */
			info->write_list[write_index++] = id;
		else if (info->smack_label && temp && 0 == strcmp(temp, info->smack_label)) /* owner */
			info->write_list[write_index++] = id;
		else if (CALENDAR_BOOK_MODE_NONE == mode)
			info->write_list[write_index++] = id;
	}
	info->write_list_count = write_index;
	sqlite3_finalize(stmt);
}

void cal_access_control_set_client_info(void *ipc, const char *smack_label)
{
	unsigned int thread_id = (unsigned int)pthread_self();
	cal_permission_info_s *info = NULL;

	cal_mutex_lock(CAL_MUTEX_ACCESS_CONTROL);
	info = _cal_access_control_find_permission_info(thread_id);
	if (NULL == info) {
		info = calloc(1, sizeof(cal_permission_info_s));
		if (NULL == info) {
			ERR("calloc() Fail");
			cal_mutex_unlock(CAL_MUTEX_ACCESS_CONTROL);
			return;
		}
		__thread_list  = g_list_append(__thread_list, info);
	}
	info->thread_id = thread_id;
	info->ipc = ipc;

	free(info->smack_label);
	info->smack_label = cal_strdup(smack_label);
	_cal_access_control_set_permission_info(info);

	cal_mutex_unlock(CAL_MUTEX_ACCESS_CONTROL);
}

void cal_access_control_unset_client_info(void)
{
	cal_permission_info_s *find = NULL;

	cal_mutex_lock(CAL_MUTEX_ACCESS_CONTROL);
	find = _cal_access_control_find_permission_info(pthread_self());
	if (find) {
		CAL_FREE(find->smack_label);
		CAL_FREE(find->write_list);
		__thread_list = g_list_remove(__thread_list, find);
		free(find);
	}
	cal_mutex_unlock(CAL_MUTEX_ACCESS_CONTROL);
}

char* cal_access_control_get_label(void)
{
	unsigned int thread_id = (unsigned int)pthread_self();
	cal_permission_info_s *info = NULL;

	cal_mutex_lock(CAL_MUTEX_ACCESS_CONTROL);
	info = _cal_access_control_find_permission_info(thread_id);

	char *smack_label = NULL;
	if (info && info->smack_label)
		smack_label = strdup(info->smack_label);

	cal_mutex_unlock(CAL_MUTEX_ACCESS_CONTROL);
	return smack_label;
}

void cal_access_control_reset(void)
{
	cal_mutex_lock(CAL_MUTEX_ACCESS_CONTROL);
	GList *cursor;
	for (cursor = __thread_list; cursor; cursor = cursor->next) {
		cal_permission_info_s *info = NULL;
		info = cursor->data;
		if (info)
			_cal_access_control_set_permission_info(info);
	}
	cal_mutex_unlock(CAL_MUTEX_ACCESS_CONTROL);
}

bool cal_access_control_have_write_permission(int book_id)
{
	cal_permission_info_s *info = NULL;

	cal_mutex_lock(CAL_MUTEX_ACCESS_CONTROL);
	unsigned int thread_id = pthread_self();
	info = _cal_access_control_find_permission_info(thread_id);
	if (NULL == info) {
		cal_mutex_unlock(CAL_MUTEX_ACCESS_CONTROL);
		ERR("_cal_access_control_find_permission_info() Fail");
		return false;
	}

	if (NULL == info->write_list) {
		cal_mutex_unlock(CAL_MUTEX_ACCESS_CONTROL);
		ERR("there is no write access info");
		return false;
	}

	int i = 0;
	for (i = 0; i < info->write_list_count; i++) {
		if (book_id == info->write_list[i]) {
			cal_mutex_unlock(CAL_MUTEX_ACCESS_CONTROL);
			return true;
		}
	}

	cal_mutex_unlock(CAL_MUTEX_ACCESS_CONTROL);
	ERR("thread(0x%x), No write permission of book_id(%d)", thread_id, book_id);
	return false;
}

int cal_is_owner(int book_id)
{
	int ret;
	sqlite3_stmt *stmt = NULL;
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	char *owner_label = NULL;
	char *saved_smack = NULL;

	snprintf(query, sizeof(query), "SELECT owner_label FROM %s WHERE id = %d",
			CAL_TABLE_CALENDAR, book_id);
	ret = cal_db_util_query_prepare(query, &stmt);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_query_prepare() Fail(%d)", ret);
		SECURE("query[%s]", query);
		return ret;
	}

	if (CAL_SQLITE_ROW != cal_db_util_stmt_step(stmt)) {
		ERR("cal_db_util_stmt_step() Fail(%d)", ret);
		sqlite3_finalize(stmt);
		return CALENDAR_ERROR_DB_FAILED;
	}

	ret = CALENDAR_ERROR_PERMISSION_DENIED;

	owner_label = (char*)sqlite3_column_text(stmt, 0);
	saved_smack = cal_access_control_get_label();

	if (owner_label && saved_smack && CAL_STRING_EQUAL == strcmp(owner_label, saved_smack))
		ret = CALENDAR_ERROR_NONE;

	sqlite3_finalize(stmt);

	free(saved_smack);
	return ret;
}

