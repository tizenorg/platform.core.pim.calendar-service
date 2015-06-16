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
#include <stdlib.h>     //calloc
#include <pims-ipc.h>
#include <glib-object.h> //g_type_init
#include <errno.h>

#include "calendar_service.h"
#include "calendar_db.h"
#include "calendar_types.h"

#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_inotify.h"
#include "cal_view.h"
#include "cal_record.h"
#include "cal_list.h"
#include "cal_mutex.h"
#include "cal_ipc.h"
#include "cal_ipc_marshal.h"
#include "cal_client_ipc.h"
#include "cal_client_reminder.h"
#include "cal_utils.h"

static pims_ipc_h cal_ipc = NULL;
static TLS pims_ipc_h cal_ipc_thread = NULL;

static int calendar_connection_count = 0;
static int calendar_change_version = 0;
static TLS int calendar_change_version_thread = 0;

pims_ipc_h _cal_client_ipc_get_handle(void);
void _cal_client_ipc_lock(void);
void _cal_client_ipc_unlock(void);

int cal_client_ipc_connect(void)
{
	int ret = CALENDAR_ERROR_NONE;
	pims_ipc_data_h outdata = NULL;

	if (cal_ipc) {
		CAL_DBG("[GLOBAL_IPC_CHANNEL] calendar already connected");
		return CALENDAR_ERROR_NONE;
	}

	char sock_file[CAL_STR_MIDDLE_LEN] = {0};
	snprintf(sock_file, sizeof(sock_file), CAL_SOCK_PATH"/.%s", getuid(), CAL_IPC_SERVICE);
	cal_ipc = pims_ipc_create(sock_file);
	DBG("[%s]", sock_file);
	if (NULL == cal_ipc) {
		if (EACCES == errno) {
			ERR("[GLOBAL_IPC_CHANNEL] pims_ipc_create() Fail(%d)", CALENDAR_ERROR_PERMISSION_DENIED);
			return CALENDAR_ERROR_PERMISSION_DENIED;
		}
		else {
			ERR("[GLOBAL_IPC_CHANNEL] pims_ipc_create() Fail(%d)", CALENDAR_ERROR_IPC);
			return CALENDAR_ERROR_IPC;
		}
	}

	if (pims_ipc_call(cal_ipc, CAL_IPC_MODULE, CAL_IPC_SERVER_CONNECT, NULL, &outdata) != 0) {
		ERR("[GLOBAL_IPC_CHANNEL] pims_ipc_call failed");
		ret = CALENDAR_ERROR_IPC;
		goto DATA_FREE;
	}

	if (outdata) {
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata,&size);
		pims_ipc_data_destroy(outdata);

		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_ipc_server_connect return(%d)", ret);
			goto DATA_FREE;
		}
	}
	return ret;

DATA_FREE:
	pims_ipc_destroy(cal_ipc);
	cal_ipc = NULL;
	return ret;
}

int cal_client_ipc_disconnect(void)
{
	int ret = CALENDAR_ERROR_NONE;
	pims_ipc_data_h outdata = NULL;

	RETVM_IF(NULL == cal_ipc, CALENDAR_ERROR_IPC, "[GLOBAL_IPC_CHANNEL] calendar not connected");

	if (pims_ipc_call(cal_ipc, CAL_IPC_MODULE, CAL_IPC_SERVER_DISCONNECT, NULL, &outdata) != 0) {
		ERR("[GLOBAL_IPC_CHANNEL] pims_ipc_call failed");
		return CALENDAR_ERROR_IPC;
	}

	if (outdata) {
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata,&size);
		pims_ipc_data_destroy(outdata);

		if (CALENDAR_ERROR_NONE != ret)
			ERR("[GLOBAL_IPC_CHANNEL] pims_ipc didn't destroyed!!!(%d)", ret);

		pims_ipc_destroy(cal_ipc);
		cal_ipc = NULL;
	}
	else {
		ERR("pims_ipc_call out data is NULL");
		return CALENDAR_ERROR_IPC;
	}

	return ret;
}

int cal_client_ipc_connect_on_thread(void)
{
	int ret = CALENDAR_ERROR_NONE;
	pims_ipc_data_h outdata = NULL;

	CAL_FN_CALL();

	// ipc create
	if (cal_ipc_thread) {
		CAL_DBG("calendar already connected");
		return CALENDAR_ERROR_NONE;
	}

	if (cal_ipc_thread == NULL) {
		char sock_file[CAL_STR_MIDDLE_LEN] = {0};
		snprintf(sock_file, sizeof(sock_file), CAL_SOCK_PATH"/.%s", getuid(), CAL_IPC_SERVICE);
		cal_ipc_thread = pims_ipc_create(sock_file);
		if (cal_ipc_thread == NULL) {
			if (errno == EACCES) {
				ERR("pims_ipc_create() Fail(%d)", CALENDAR_ERROR_PERMISSION_DENIED);
				ret = CALENDAR_ERROR_PERMISSION_DENIED;
				goto ERROR_RETURN;
			} else {
				ERR("pims_ipc_create() Fail(%d)", CALENDAR_ERROR_OUT_OF_MEMORY);
				ret = CALENDAR_ERROR_OUT_OF_MEMORY;
				goto ERROR_RETURN;
			}
		}
	} else {
		CAL_DBG("calendar already connected");
		ret = CALENDAR_ERROR_NONE;
		goto ERROR_RETURN;
	}

	// ipc call
	if (pims_ipc_call(cal_ipc_thread, CAL_IPC_MODULE, CAL_IPC_SERVER_CONNECT, NULL, &outdata) != 0) {
		ERR("pims_ipc_call failed");
		ret = CALENDAR_ERROR_IPC;
		goto ERROR_RETURN;
	}

	if (outdata) {
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata,&size);

		pims_ipc_data_destroy(outdata);

		if (CALENDAR_ERROR_NONE != ret) {
			ERR("calendar_connect return (%d)",ret);
			goto ERROR_RETURN;
		}
	}
	return ret;

ERROR_RETURN:
	if (cal_ipc_thread != NULL) {
		pims_ipc_destroy(cal_ipc_thread);
		cal_ipc_thread = NULL;
	}
	return ret;
}

int cal_client_ipc_disconnect_on_thread(void)
{
	CAL_FN_CALL();

	int ret = CALENDAR_ERROR_NONE;
	pims_ipc_data_h outdata = NULL;

	RETVM_IF(NULL == cal_ipc_thread, CALENDAR_ERROR_IPC ,"calendar not connected");

	if (pims_ipc_call(cal_ipc_thread, CAL_IPC_MODULE, CAL_IPC_SERVER_DISCONNECT, NULL, &outdata) != 0) {
		ERR("pims_ipc_call failed");
		return CALENDAR_ERROR_IPC;
	}

	if (outdata) {
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata,&size);
		pims_ipc_data_destroy(outdata);
		if (CALENDAR_ERROR_NONE != ret)
			ERR("[GLOBAL_IPC_CHANNEL] pims_ipc didn't destroyed(%d)", ret);

		pims_ipc_destroy(cal_ipc_thread);
		cal_ipc_thread = NULL;
	}
	return ret;
}

bool cal_client_ipc_is_call_inprogress(void)
{
	return pims_ipc_is_call_in_progress(cal_ipc);
}

pims_ipc_h _cal_client_ipc_get_handle(void)
{
	if (cal_ipc_thread == NULL)
	{
		return cal_ipc;
	}
	return cal_ipc_thread;
}

void _cal_client_ipc_lock(void)
{
	if (cal_ipc_thread == NULL)
	{
		cal_mutex_lock(CAL_MUTEX_PIMS_IPC_CALL);
	}
}

void _cal_client_ipc_unlock(void)
{
	if (cal_ipc_thread == NULL)
	{
		cal_mutex_unlock(CAL_MUTEX_PIMS_IPC_CALL);
	}
}

int cal_client_ipc_call(char *module, char *function, pims_ipc_h data_in,
		pims_ipc_data_h *data_out)
{
	int ret = 0;
	pims_ipc_h ipc_handle = _cal_client_ipc_get_handle();

	_cal_client_ipc_lock();

	ret = pims_ipc_call(ipc_handle, module, function, data_in, data_out);

	_cal_client_ipc_unlock();

	return ret;
}

void cal_client_ipc_set_change_version(int version)
{
	if (cal_ipc_thread == NULL)
	{
		calendar_change_version = version;
		DBG("change_version=%d",version);
		return ;
	}
	calendar_change_version_thread = version;
	DBG("change_version=%d",version);
}

int cal_client_ipc_get_change_version(void)
{
	if (cal_ipc_thread == NULL)
	{
		return calendar_change_version;
	}
	return calendar_change_version_thread;

}

int cal_client_ipc_client_check_permission(int permission, bool *result)
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

	if (outdata) {
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata,&size);

		if (CALENDAR_ERROR_NONE == ret) {
			if (result)
				*result = *(bool*) pims_ipc_data_get(outdata, &size);
		}
		pims_ipc_data_destroy(outdata);
	}

	return ret;
}
