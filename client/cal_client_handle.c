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

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <glib.h>

#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_mutex.h"
#include "cal_client_handle.h"
#include "cal_client_utils.h"

static GHashTable *_cal_handle_table = NULL;

static int _cal_client_handle_get_key(unsigned int id, char *key, int key_len)
{
	RETV_IF(NULL == key, CALENDAR_ERROR_INVALID_PARAMETER);

	snprintf(key, key_len, "%d:%u", getuid(), id);
	DBG("[%s]", key);
	return CALENDAR_ERROR_NONE;
}

int cal_client_handle_get_p(calendar_h *out_handle)
{
	int ret = 0;

	RETVM_IF(NULL == _cal_handle_table, CALENDAR_ERROR_NO_DATA, "No handle table");
	RETV_IF(NULL == out_handle, CALENDAR_ERROR_INVALID_PARAMETER);

	unsigned int tid = cal_client_get_tid();
	unsigned int pid = cal_client_get_pid();

	char key[CAL_STR_MIDDLE_LEN] = {0};
	ret = _cal_client_handle_get_key(tid, key, sizeof(key));
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "_cal_client_handle_get_key() Fail(%d)", ret);

	calendar_h handle = NULL;
	cal_mutex_lock(CAL_MUTEX_HANDLE);
	handle = g_hash_table_lookup(_cal_handle_table, key);
	cal_mutex_unlock(CAL_MUTEX_HANDLE);

	if (NULL == handle && tid != pid) {
		DBG("g_hash_table_lookup() Fail:No handle:key[%s]", key);
		ret = _cal_client_handle_get_key(pid, key, sizeof(key));
		RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "_cal_client_handle_get_key() Fail(%d)", ret);

		cal_mutex_lock(CAL_MUTEX_HANDLE);
		handle = g_hash_table_lookup(_cal_handle_table, key);
		cal_mutex_unlock(CAL_MUTEX_HANDLE);
		RETVM_IF(NULL == handle, CALENDAR_ERROR_NO_DATA, "g_hash_table_lookup() Fail");
	}
	*out_handle = handle;
	return CALENDAR_ERROR_NONE;
}

int cal_client_handle_get_p_with_id(unsigned int id, calendar_h *out_handle)
{
	int ret = 0;

	RETVM_IF(NULL == _cal_handle_table, CALENDAR_ERROR_NO_DATA, "No handle table");
	RETV_IF(NULL == out_handle, CALENDAR_ERROR_INVALID_PARAMETER);

	char key[CAL_STR_MIDDLE_LEN] = {0};
	ret = _cal_client_handle_get_key(id, key, sizeof(key));
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "_cal_client_handle_get_key_with_id() Fail(%d)", ret);

	calendar_h handle = NULL;
	cal_mutex_lock(CAL_MUTEX_HANDLE);
	handle = g_hash_table_lookup(_cal_handle_table, key);
	cal_mutex_unlock(CAL_MUTEX_HANDLE);
	if (NULL == handle) {
		ERR("g_hash_table_lookup() Fail:No handle:key[%s]", key);
		return CALENDAR_ERROR_NO_DATA;
	}
	*out_handle = handle;
	return CALENDAR_ERROR_NONE;
}

static int _cal_client_handle_add(calendar_h handle, const char *key)
{
	RETV_IF(NULL == key, CALENDAR_ERROR_INVALID_PARAMETER);
	cal_mutex_lock(CAL_MUTEX_HANDLE);

	if (NULL == _cal_handle_table)
		_cal_handle_table = g_hash_table_new_full(g_str_hash, g_str_equal, free, NULL);

	g_hash_table_insert(_cal_handle_table, strdup(key), handle);
	cal_mutex_unlock(CAL_MUTEX_HANDLE);
	DBG("[HASH:handle] insert key[%s] value[%p]", key, handle);
	return CALENDAR_ERROR_NONE;
}

int cal_client_handle_create(unsigned int id, calendar_h *out_handle)
{
	int ret = 0;
	calendar_h handle = NULL;

	RETV_IF(NULL == out_handle, CALENDAR_ERROR_NONE);

	char key[CAL_STR_MIDDLE_LEN] = {0};
	ret = _cal_client_handle_get_key(id, key, sizeof(key));
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "_cal_client_handle_get_key() Fail(%d)", ret);

	ret = cal_handle_create(&handle);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_handle_create() Fail(%d)", ret);

	ret = _cal_client_handle_add(handle, key);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("_cal_client_handle_add() Fail(%d)", ret);
		cal_handle_destroy(handle);
		return ret;
	}

	*out_handle = handle;
	return CALENDAR_ERROR_NONE;
}

int cal_client_handle_remove(unsigned int id, calendar_h handle)
{
	int ret = 0;
	char key[CAL_STR_MIDDLE_LEN] = {0};

	RETVM_IF(NULL == _cal_handle_table, CALENDAR_ERROR_NONE, "No handle table");

	ret = _cal_client_handle_get_key(id, key, sizeof(key));
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "_cal_client_handle_get_key() Fail(%d)", ret);

	cal_mutex_lock(CAL_MUTEX_HANDLE);
	g_hash_table_remove(_cal_handle_table, key);
	if (0 == g_hash_table_size(_cal_handle_table)) {
		g_hash_table_destroy(_cal_handle_table);
		_cal_handle_table = NULL;
	}
	cal_mutex_unlock(CAL_MUTEX_HANDLE);
	cal_handle_destroy(handle);
	return CALENDAR_ERROR_NONE;
}

