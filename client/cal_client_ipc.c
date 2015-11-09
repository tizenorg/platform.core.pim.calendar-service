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
#include <pims-ipc.h>
#include <glib-object.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>

#include "calendar_db.h"
#include "calendar_types.h"
#include "cal_client_service.h"
#include "cal_client_service_helper.h"
#include "cal_client_utils.h"
#include "cal_client_ipc.h"
#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_inotify.h"
#include "cal_view.h"
#include "cal_ipc.h"
#include "cal_mutex.h"
#include "cal_ipc_marshal.h"
#include "cal_handle.h"
#include "cal_utils.h"
#include "cal_client_reminder.h"

typedef struct {
	pims_ipc_h ipc;
	GList *list_handle;
} cal_ipc_s;

static GHashTable *_cal_ipc_table = NULL;
static pthread_mutex_t cal_mutex_disconnected = PTHREAD_MUTEX_INITIALIZER;
static bool _cal_ipc_disconnected = false;

static pims_ipc_h _cal_client_ipc_get_handle(void)
{
	cal_ipc_s *ipc_data = NULL;
	char key[CAL_STR_MIDDLE_LEN] = {0};
	RETVM_IF(NULL == _cal_ipc_table, NULL, "Not connected");

	snprintf(key, sizeof(key), "%u", cal_client_get_tid());
	ipc_data = g_hash_table_lookup(_cal_ipc_table, key);
	if (NULL == ipc_data) {
		snprintf(key, sizeof(key), "%u", cal_client_get_pid());
		ipc_data = g_hash_table_lookup(_cal_ipc_table, key);
		if (NULL == ipc_data) {
			ERR("No ipc_data:key[%s]", key);
			return NULL;
		}
	}
	return ipc_data->ipc;
}

static int _cal_client_ipc_create(pims_ipc_h *out_ipc)
{
	RETV_IF(NULL == out_ipc, CALENDAR_ERROR_INVALID_PARAMETER);

	char sock_file[CAL_STR_MIDDLE_LEN] = {0};
	snprintf(sock_file, sizeof(sock_file), CAL_SOCK_PATH"/.%s", getuid(), CAL_IPC_SERVICE);
	DBG("[%s]", sock_file);

	pims_ipc_h ipc = pims_ipc_create(sock_file);
	if (NULL == ipc) {
		if (errno == EACCES) {
			ERR("[GLOBAL_IPC_CHANNEL] pims_ipc_create() Fail: Permission denied");
			return CALENDAR_ERROR_PERMISSION_DENIED;
		}
		else {
			ERR("[GLOBAL_IPC_CHANNEL] pims_ipc_create() Fail(%d)", CALENDAR_ERROR_IPC);
			return CALENDAR_ERROR_IPC;
		}
	}
	*out_ipc = ipc;
	return CALENDAR_ERROR_NONE;
}

static void _cal_client_ipc_free(gpointer p)
{
	cal_ipc_s *ipc_data = p;
	RET_IF(NULL == ipc_data);

	if (ipc_data->ipc) {
		cal_client_ipc_unset_disconnected_cb(ipc_data->ipc);
		pims_ipc_destroy(ipc_data->ipc);
	}
	g_list_free(ipc_data->list_handle);
	free(ipc_data);
}

static int _cal_client_ipc_connect(calendar_h handle, pims_ipc_h ipc)
{
	int ret = CALENDAR_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(NULL == handle, CALENDAR_ERROR_INVALID_PARAMETER);

	indata = pims_ipc_data_create(0);
	if (NULL == indata) {
		ERR("pims_ipc_data_create() Fail");
		return CALENDAR_ERROR_IPC;
	}

	ret = cal_ipc_marshal_handle(handle, indata);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_handle() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
	}

	/* ipc call */
	if (pims_ipc_call(ipc, CAL_IPC_MODULE, CAL_IPC_SERVER_CONNECT, indata, &outdata) != 0) {
		ERR("pims_ipc_call() Fail");
		pims_ipc_data_destroy(indata);
		return CALENDAR_ERROR_IPC;
	}
	pims_ipc_data_destroy(indata);

	int ipc_ret = 0;
	ret = cal_ipc_unmarshal_int(outdata, &ipc_ret);
	pims_ipc_data_destroy(outdata);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_IPC;
	}
	return ipc_ret;
}

static void _cal_client_ipc_disconnected_cb(void *user_data)
{
	DBG("disconnected");
	cal_client_ipc_set_disconnected(true);
}

int cal_client_ipc_connect(calendar_h handle, unsigned int id)
{
	int ret = 0;
	cal_ipc_s *ipc_data = NULL;
	char key[CAL_STR_MIDDLE_LEN] = {0};

	//RETV_IF(_cal_ipc_disconnected, CALENDAR_ERROR_IPC);
	RETV_IF(NULL == handle, CALENDAR_ERROR_INVALID_PARAMETER);

	snprintf(key, sizeof(key), "%u", id);

	if (NULL == _cal_ipc_table)
		_cal_ipc_table = g_hash_table_new_full(g_str_hash, g_str_equal, free, _cal_client_ipc_free);
	else
		ipc_data = g_hash_table_lookup(_cal_ipc_table, key);

	if (NULL == ipc_data) {
		ipc_data = calloc(1, sizeof(cal_ipc_s));
		if (NULL == ipc_data) {
			ERR("calloc() Fail");
			return CALENDAR_ERROR_OUT_OF_MEMORY;
		}
		ret = _cal_client_ipc_create(&(ipc_data->ipc));
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("_cal_client_ipc_create() Fail(%d)", ret);
			_cal_client_ipc_free(ipc_data);
			return ret;
		}
		g_hash_table_insert(_cal_ipc_table, cal_strdup(key), ipc_data);
		DBG("[HASH:ipc] insert key[%s] value[%p]", key, ipc_data);
		cal_client_ipc_set_disconnected_cb(ipc_data->ipc, _cal_client_ipc_disconnected_cb, NULL);
	}
	ret = _cal_client_ipc_connect(handle, ipc_data->ipc);
	WARN_IF(CALENDAR_ERROR_NONE != ret, "_cal_client_ipc_connect() Fail(%d)", ret);

	ipc_data->list_handle = g_list_append(ipc_data->list_handle, handle);

	return CALENDAR_ERROR_NONE;
}

int cal_client_ipc_disconnect(calendar_h handle, unsigned int id, int connection_count)
{
	int ret = CALENDAR_ERROR_NONE;
	cal_ipc_s *ipc_data = NULL;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;
	char key[CAL_STR_MIDDLE_LEN] = {0};

	//RETV_IF(_cal_ipc_disconnected, CALENDAR_ERROR_IPC);
	RETVM_IF(NULL == _cal_ipc_table, CALENDAR_ERROR_IPC, "Not connected");
	snprintf(key, sizeof(key), "%u", id);

	ipc_data = g_hash_table_lookup(_cal_ipc_table, key);
	RETVM_IF(NULL == ipc_data, CALENDAR_ERROR_IPC, "Not connected");

	indata = pims_ipc_data_create(0);
	if (NULL == indata) {
		ERR("pims_ipc_data_create() Fail");
		return CALENDAR_ERROR_IPC;
	}

	ret = cal_ipc_marshal_handle(handle, indata);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_handle() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
	}

	/* ipc call */
	if (pims_ipc_call(ipc_data->ipc, CAL_IPC_MODULE, CAL_IPC_SERVER_DISCONNECT, indata, &outdata) != 0) {
		ERR("[GLOBAL_IPC_CHANNEL] pims_ipc_call() Fail");
		pims_ipc_data_destroy(indata);
		return CALENDAR_ERROR_IPC;
	}
	pims_ipc_data_destroy(indata);

	if (NULL == outdata) {
		ERR("ipc outdata is NULL");
		return CALENDAR_ERROR_IPC;
	}

	int ipc_ret = 0;
	ret = cal_ipc_unmarshal_int(outdata, &ipc_ret);
	pims_ipc_data_destroy(outdata);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_IPC;
	}

	if (1 == connection_count)
		g_hash_table_remove(_cal_ipc_table, key);

	return ret;
}

bool cal_client_ipc_is_call_inprogress(void)
{
	pims_ipc_h ipc = _cal_client_ipc_get_handle();
	if (NULL == ipc) {
		ERR("No ipc");
		return false;
	}
	return pims_ipc_is_call_in_progress(ipc);
}

void cal_client_ipc_lock(void)
{
	if (0 == cal_client_get_thread_connection_count())
		cal_mutex_lock(CAL_MUTEX_PIMS_IPC_CALL);
}

void cal_client_ipc_unlock(void)
{
	if (0 == cal_client_get_thread_connection_count())
		cal_mutex_unlock(CAL_MUTEX_PIMS_IPC_CALL);
}

void cal_client_ipc_set_disconnected(bool is_disconnected)
{
	pthread_mutex_lock(&cal_mutex_disconnected);
	_cal_ipc_disconnected = is_disconnected;
	pthread_mutex_unlock(&cal_mutex_disconnected);
}

bool cal_client_ipc_get_disconnected(void)
{
	pthread_mutex_lock(&cal_mutex_disconnected);
	DBG("RECOVERY(cal_ipc_disconnected(%d))", _cal_ipc_disconnected);
	pthread_mutex_unlock(&cal_mutex_disconnected);
	return _cal_ipc_disconnected;
}

int cal_client_ipc_call(char *module, char *function, pims_ipc_h data_in, pims_ipc_data_h *data_out)
{
	int ret = 0;

	if (true == cal_client_ipc_get_disconnected()) {
		cal_client_ipc_set_disconnected(false);
		cal_client_ipc_recovery();
		cal_client_recovery_for_change_subscription();
	}

	pims_ipc_h ipc = _cal_client_ipc_get_handle();
	RETVM_IF(NULL == ipc, CALENDAR_ERROR_IPC, "_cal_client_ipc_get_handle() Fail");

	cal_client_ipc_lock();
	ret = pims_ipc_call(ipc , module, function, data_in, data_out);
	cal_client_ipc_unlock();
	return ret;
}

void cal_client_ipc_set_change_version(calendar_h handle, int version)
{
	RETM_IF(NULL == handle, "handle is NULL");
	cal_s *h = (cal_s *)handle;
	h->version = version;
}

int cal_client_ipc_get_change_version(calendar_h handle)
{
	RETVM_IF(NULL == handle, CALENDAR_ERROR_INVALID_PARAMETER, "handle is NULL");
	cal_s *h = (cal_s *)handle;
	return h->version;
}

int cal_client_ipc_client_check_permission(calendar_h handle, int permission, bool *result)
{
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;
	int ret;

	if (result)
		*result = false;

	indata = pims_ipc_data_create(0);
	if (NULL == indata) {
		ERR("pims_ipc_data_create() Fail");
		return CALENDAR_ERROR_OUT_OF_MEMORY;
	}

	ret = cal_ipc_marshal_handle(handle, indata);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_handle() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
	}

	ret = cal_ipc_marshal_int(permission, indata);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
	}

	if (cal_client_ipc_call(CAL_IPC_MODULE, CAL_IPC_SERVER_CHECK_PERMISSION, indata, &outdata) != 0) {
		ERR("cal_client_ipc_call() Fail");
		pims_ipc_data_destroy(indata);
		return CALENDAR_ERROR_IPC;
	}

	pims_ipc_data_destroy(indata);

	if (NULL == outdata) {
		ERR("ipc outdata is NULL");
		return CALENDAR_ERROR_IPC;
	}

	int ipc_ret = 0;
	ret = cal_ipc_unmarshal_int(outdata, &ipc_ret);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		pims_ipc_data_destroy(outdata);
		return CALENDAR_ERROR_IPC;
	}

	ret = cal_ipc_unmarshal_int(outdata, result);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		pims_ipc_data_destroy(outdata);
		return CALENDAR_ERROR_IPC;
	}
	pims_ipc_data_destroy(outdata);

	return ipc_ret;
}

int cal_client_ipc_set_disconnected_cb(pims_ipc_h ipc, void (*cb)(void *), void *user_data)
{
	return pims_ipc_add_server_disconnected_cb(ipc, cb, user_data);
}

int cal_client_ipc_unset_disconnected_cb(pims_ipc_h ipc)
{
	return pims_ipc_remove_server_disconnected_cb(ipc);
}

static void _cal_client_ipc_recovery_foreach_cb(gpointer key, gpointer value, gpointer user_data)
{
	int ret = 0;
	GList *l = NULL;

	RET_IF(NULL == value);

	cal_ipc_s *ipc_data = value;

	cal_client_ipc_unset_disconnected_cb(ipc_data->ipc);
	ret = _cal_client_ipc_create(&(ipc_data->ipc));
	RETM_IF(CALENDAR_ERROR_NONE != ret, "_cal_client_ipc_create() Fail(%d)", ret);
	cal_client_ipc_set_disconnected_cb(ipc_data->ipc, _cal_client_ipc_disconnected_cb, NULL);

	for (l = ipc_data->list_handle; l; l = l->next) {
		calendar_h handle = l->data;
		ret = _cal_client_ipc_connect(handle, ipc_data->ipc);
		WARN_IF(CALENDAR_ERROR_NONE != ret, "_cal_client_ipc_connect() Fail(%d)", ret);
	}
}

void cal_client_ipc_recovery(void)
{
	g_hash_table_foreach(_cal_ipc_table, _cal_client_ipc_recovery_foreach_cb, NULL);
}
