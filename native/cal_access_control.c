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

#include <pthread.h>
#include <signal.h>
#include <security-server.h>
#include <sys/smack.h>

#include <string.h>
#include <pims-ipc-svc.h>

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
#include "cal_service.h"

typedef struct {
	unsigned int thread_id;
	char *smack_label;
	cal_permission_e permission;
	int *write_list;
}calendar_permission_info_s;

static GList *__thread_list = NULL;

static int have_smack = -1;

static calendar_permission_info_s* __cal_access_control_find_permission_info(unsigned int thread_id);
static void __cal_access_control_set_permission_info(unsigned int thread_id, const char *cookie);
static void __cal_access_control_disconnected_cb(pims_ipc_h ipc, void *user_data);

static calendar_permission_info_s* __cal_access_control_find_permission_info(unsigned int thread_id)
{
	GList *cursor;

	for(cursor=__thread_list;cursor;cursor=cursor->next) {
		calendar_permission_info_s *info = NULL;
		info = cursor->data;
		if (info->thread_id == thread_id)
			return info;
	}
	return NULL;
}

// check SMACK enable or disable
static int __cal_have_smack(void)
{
	if (-1 == have_smack) {
		if (NULL == smack_smackfs_path())
			have_smack = 0;
		else
			have_smack = 1;
	}
	return have_smack;
}

static void __cal_access_control_set_permission_info(unsigned int thread_id, const char *cookie)
{
	calendar_permission_info_s *info = NULL;
	const char *smack_label;
	bool smack_enabled = false;

	if (__cal_have_smack() == 1)
		smack_enabled = true;
	else
		INFO("SAMCK disabled");

	info = __cal_access_control_find_permission_info(thread_id);
	if(info == NULL) {
		ERR("dont' have access info of this thread(%d)", thread_id);
		return ;
	}
	smack_label = info->smack_label;

	if ((smack_label && 0 == strcmp(smack_label, "calendar-service")) || !smack_enabled) {
		info->permission |= CAL_PERMISSION_READ;
		info->permission |= CAL_PERMISSION_WRITE;
	}
	else if (cookie) {
		if (SECURITY_SERVER_API_SUCCESS == security_server_check_privilege_by_cookie(cookie, "calendar-service::svc", "r"))
			info->permission |= CAL_PERMISSION_READ;
		if (SECURITY_SERVER_API_SUCCESS == security_server_check_privilege_by_cookie(cookie, "calendar-service::svc", "w"))
			info->permission |= CAL_PERMISSION_WRITE;
	}
	SEC_INFO("Thread(0x%x), info(%p), smack :%s, permission:%d", thread_id, info, smack_label, info->permission);

	CAL_FREE(info->write_list);

	char query[CAL_DB_SQL_MAX_LEN] = {0,};
	int count = 0;
	int ret = 0;
	sqlite3_stmt *stmt;
	int write_index = 0;
	snprintf(query, sizeof(query),
			"SELECT count(id) FROM %s WHERE deleted = 0 ", CAL_TABLE_CALENDAR);
	ret = _cal_db_util_query_get_first_int_result(query, NULL, &count);

	if (CALENDAR_ERROR_NONE != ret) {
		ERR("DB get Failed");
		return;
	}

	if (info->permission & CAL_PERMISSION_WRITE) {
		info->write_list = calloc(count+1, sizeof(int));
		info->write_list[0] = -1;
	}

	snprintf(query, sizeof(query),
			"SELECT id, mode, owner_label FROM %s WHERE deleted = 0 ", CAL_TABLE_CALENDAR);
	stmt = _cal_db_util_query_prepare(query);
	if (NULL == stmt) {
		ERR("DB Failed");
		return;
	}

	while (CAL_DB_ROW == _cal_db_util_stmt_step(stmt)) {
		int id = 0;
		int mode = 0;
		char *temp = NULL;

		id = sqlite3_column_int(stmt, 0);
		mode = sqlite3_column_int(stmt, 1);
		temp = (char *)sqlite3_column_text(stmt, 2);

		if ( temp && ( (smack_label && 0 == strcmp(temp, smack_label)) ||
					( smack_label && 0 == strcmp(smack_label, "calendar-service") ) ) )  {// owner and calendar-service
			if (info->permission & CAL_PERMISSION_WRITE)
				info->write_list[write_index++] = id;
		}
		else {
			switch(mode) {
			case CALENDAR_BOOK_MODE_NONE:
				if (info->permission & CAL_PERMISSION_WRITE)
					info->write_list[write_index++] = id;
				break;
			case CALENDAR_BOOK_MODE_RECORD_READONLY:
			default:
				break;
			}
		}
	} // while

	if (info->permission & CAL_PERMISSION_WRITE)
		info->write_list[write_index] = -1;

	sqlite3_finalize(stmt);
}

void _cal_access_control_set_client_info(const char* smack_label, const char* cookie)
{
	unsigned int thread_id = (unsigned int)pthread_self();
	calendar_permission_info_s *info = NULL;

	_cal_mutex_lock(CAL_MUTEX_ACCESS_CONTROL);
	info = __cal_access_control_find_permission_info(thread_id);
	if (NULL == info) {
		info = calloc(1, sizeof(calendar_permission_info_s));
		__thread_list  = g_list_append(__thread_list, info);
	}
	info->thread_id = thread_id;
	CAL_FREE(info->smack_label);
	info->smack_label = SAFE_STRDUP(smack_label);
	__cal_access_control_set_permission_info(thread_id, cookie);

	// for close DB or free access_control when client is disconnected
	pims_ipc_svc_set_client_disconnected_cb(__cal_access_control_disconnected_cb,NULL);

	_cal_mutex_unlock(CAL_MUTEX_ACCESS_CONTROL);
}

void _cal_access_control_unset_client_info(void)
{
	calendar_permission_info_s *find = NULL;

	_cal_mutex_lock(CAL_MUTEX_ACCESS_CONTROL);
	find = __cal_access_control_find_permission_info(pthread_self());
	if (find) {
		CAL_FREE(find->smack_label);
		CAL_FREE(find->write_list);
		__thread_list = g_list_remove(__thread_list, find);
		free(find);
	}
	_cal_mutex_unlock(CAL_MUTEX_ACCESS_CONTROL);
}

bool _cal_access_control_have_permission(cal_permission_e permission)
{
	calendar_permission_info_s *find = NULL;

	_cal_mutex_lock(CAL_MUTEX_ACCESS_CONTROL);
	find = __cal_access_control_find_permission_info(pthread_self());
	if (!find) {
		_cal_mutex_unlock(CAL_MUTEX_ACCESS_CONTROL);
		return false;
	}

	if (CAL_PERMISSION_NONE == permission) {
		_cal_mutex_unlock(CAL_MUTEX_ACCESS_CONTROL);
		return false;
	}

	if ((find->permission & permission) == permission) {
		_cal_mutex_unlock(CAL_MUTEX_ACCESS_CONTROL);
		return true;
	}

	_cal_mutex_unlock(CAL_MUTEX_ACCESS_CONTROL);
	ERR("Does not have permission %d, this module has permission %d",
			permission, find->permission);

	return false;
}

char* _cal_access_control_get_label(void)
{
	unsigned int thread_id = (unsigned int)pthread_self();
	calendar_permission_info_s *info = NULL;
	char *tmp = NULL;

	_cal_mutex_lock(CAL_MUTEX_ACCESS_CONTROL);
	info = __cal_access_control_find_permission_info(thread_id);

	if (info == NULL) {
		_cal_mutex_unlock(CAL_MUTEX_ACCESS_CONTROL);
		return NULL;
	}
	tmp = SAFE_STRDUP(info->smack_label);
	_cal_mutex_unlock(CAL_MUTEX_ACCESS_CONTROL);

	return tmp;
}


void _cal_access_control_reset(void)
{
	_cal_mutex_lock(CAL_MUTEX_ACCESS_CONTROL);
	GList *cursor;
	for(cursor=__thread_list;cursor;cursor=cursor->next) {
		calendar_permission_info_s *info = NULL;
		info = cursor->data;
		if (info)
			__cal_access_control_set_permission_info(info->thread_id, NULL);
	}
	_cal_mutex_unlock(CAL_MUTEX_ACCESS_CONTROL);
}

bool _cal_access_control_have_write_permission(int calendarbook_id)
{
	int i = 0;
	calendar_permission_info_s *find = NULL;

	_cal_mutex_lock(CAL_MUTEX_ACCESS_CONTROL);
	find = __cal_access_control_find_permission_info(pthread_self());
	if (!find) {
		_cal_mutex_unlock(CAL_MUTEX_ACCESS_CONTROL);
		ERR("can not found access info");
		return false;
	}

	if (NULL == find->write_list) {
		_cal_mutex_unlock(CAL_MUTEX_ACCESS_CONTROL);
		ERR("there is no write access info");
		return false;
	}

	while(1) {
		if (find->write_list[i] == -1)
			break;
		if (calendarbook_id == find->write_list[i]) {
			_cal_mutex_unlock(CAL_MUTEX_ACCESS_CONTROL);
			return true;
		}
		i++;
	}

	_cal_mutex_unlock(CAL_MUTEX_ACCESS_CONTROL);
	return false;
}

static void __cal_access_control_disconnected_cb(pims_ipc_h ipc, void *user_data)
{
	ENTER();
	calendar_permission_info_s *find = NULL;

	_cal_mutex_lock(CAL_MUTEX_ACCESS_CONTROL);
	find = __cal_access_control_find_permission_info(pthread_self());
	if (find) {
		CAL_FREE(find->smack_label);
		CAL_FREE(find->write_list);
		__thread_list = g_list_remove(__thread_list, find);
		free(find);
	}
	_cal_mutex_unlock(CAL_MUTEX_ACCESS_CONTROL);
	// if client did not call disconnect function such as disconnect
	// DB will be closed in _cal_db_internal_disconnect()
	_cal_calendar_internal_disconnect();
}

int _cal_is_owner(int calendarbook_id)
{
	int ret;
	sqlite3_stmt *stmt = NULL;
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	char *owner_label = NULL;
	char *saved_smack = NULL;

	snprintf(query, sizeof(query),
			"SELECT owner_label FROM "CAL_TABLE_CALENDAR" "
			"WHERE id = %d", calendarbook_id);
	stmt = _cal_db_util_query_prepare(query);
	if (NULL == stmt) {
		ERR("DB error : _cal_db_util_query_prepare() Failed()");
		return CALENDAR_ERROR_DB_FAILED;
	}

	ret = _cal_db_util_stmt_step(stmt);
	if (CAL_DB_ROW != ret) {
		ERR("_cal_db_util_stmt_step() Failed(%d)", ret);
		sqlite3_finalize(stmt);
		return CALENDAR_ERROR_DB_FAILED;
	}

	ret = CALENDAR_ERROR_PERMISSION_DENIED;

	owner_label = (char*)sqlite3_column_text(stmt, 0);
	saved_smack = _cal_access_control_get_label();

	if (owner_label && saved_smack && strcmp(owner_label, saved_smack) == 0)
		ret = CALENDAR_ERROR_NONE;

	sqlite3_finalize(stmt);

	free(saved_smack);
	return ret;
}

