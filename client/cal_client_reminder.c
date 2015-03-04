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

#include <stdlib.h>     //calloc
#include <pims-ipc.h>

#include "calendar_service.h"
#include "calendar_db.h"
#include "calendar_types.h"
#include "calendar_reminder.h"

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

typedef struct {
	calendar_reminder_cb cb;
	void *user_data;
} callback_info_s;


static pims_ipc_h __ipc = NULL;
static GSList *__subscribe_list = NULL;

int _cal_client_reminder_create_for_subscribe(void)
{
	_cal_mutex_lock(CAL_MUTEX_PIMS_IPC_PUBSUB);
	if (!__ipc) {
		__ipc = pims_ipc_create_for_subscribe(CAL_IPC_SOCKET_PATH_FOR_SUBSCRIPTION);
		if (!__ipc) {
			ERR("pims_ipc_create_for_subscribe");
			_cal_mutex_unlock(CAL_MUTEX_PIMS_IPC_PUBSUB);
			return CALENDAR_ERROR_IPC;
		}
	}
	_cal_mutex_unlock(CAL_MUTEX_PIMS_IPC_PUBSUB);
	return CALENDAR_ERROR_NONE;
}

int _cal_client_reminder_destroy_for_subscribe(void)
{
	_cal_mutex_lock(CAL_MUTEX_PIMS_IPC_PUBSUB);

	pims_ipc_destroy_for_subscribe(__ipc);
	__ipc = NULL;

	_cal_mutex_unlock(CAL_MUTEX_PIMS_IPC_PUBSUB);
	return CALENDAR_ERROR_NONE;
}

static void __cal_client_reminder_subscribe_callback(pims_ipc_h ipc, pims_ipc_data_h data, void *user_data)
{
	unsigned int size = 0;
	const char *str = NULL;
	int len = 0;

	if (data) {
		len = (int)pims_ipc_data_get(data, &size);
		if (0 == len) {
			ERR("pims_ipc_data_get() failed");
			return;
		}
		str = (const char *)pims_ipc_data_get(data, &size);
		if (!str) {
			ERR("pims_ipc_data_get() failed");
			return;
		}
	}

	if (__subscribe_list) {
		GSList *l = NULL;
		for (l = __subscribe_list; l; l = l->next) {
			callback_info_s *cb_info = l->data;
			if (NULL == cb_info) continue;

			cb_info->cb(str, cb_info->user_data);
		}
	}
}

API int calendar_reminder_add_cb(calendar_reminder_cb callback, void *user_data)
{
	GSList *it = NULL;
	callback_info_s *cb_info = NULL;
	int ret;
	bool result = false;

	if (NULL == callback) {
		ERR("Invalid parameter: callback is NULL");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	ret = cal_client_ipc_client_check_permission(CAL_PERMISSION_READ, &result);
	retvm_if(CALENDAR_ERROR_NONE != ret, ret, "ctsvc_ipc_client_check_permission fail (%d)", ret);
	retvm_if(result == false, CALENDAR_ERROR_PERMISSION_DENIED, "Permission denied (calendar read)");

	_cal_mutex_lock(CAL_MUTEX_PIMS_IPC_PUBSUB);

	if (!__subscribe_list) {
		if (pims_ipc_subscribe(__ipc, CAL_IPC_MODULE_FOR_SUBSCRIPTION, (char *)CAL_NOTI_REMINDER_CAHNGED,
					__cal_client_reminder_subscribe_callback, NULL) != 0) {
			ERR("pims_ipc_subscribe() failed");
			_cal_mutex_unlock(CAL_MUTEX_PIMS_IPC_PUBSUB);
			return CALENDAR_ERROR_IPC;
		}
	}

	// Check duplication
	for (it = __subscribe_list; it; it = it->next) {
		if (NULL == it->data) continue;

		callback_info_s *cb_info = it->data;
		if (callback == cb_info->cb && user_data == cb_info->user_data) {
			ERR("The same callback(%s) is already exist");
			_cal_mutex_unlock(CAL_MUTEX_PIMS_IPC_PUBSUB);
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
	}

	cb_info = calloc(1, sizeof(callback_info_s));
	cb_info->user_data = user_data;
	cb_info->cb = callback;
	__subscribe_list = g_slist_append(__subscribe_list, cb_info);

	_cal_mutex_unlock(CAL_MUTEX_PIMS_IPC_PUBSUB);

	return CALENDAR_ERROR_NONE;
}

API int calendar_reminder_remove_cb(calendar_reminder_cb callback, void *user_data)
{
	GSList *it = NULL;

	if (NULL == callback) {
		ERR("Invalid parameter: callback is NULL");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	_cal_mutex_lock(CAL_MUTEX_PIMS_IPC_PUBSUB);

	for (it = __subscribe_list; it; it = it->next) {
		if (NULL == it->data) continue;

		callback_info_s *cb_info = it->data;
		if (callback == cb_info->cb && user_data == cb_info->user_data) {
			__subscribe_list = g_slist_remove(__subscribe_list, cb_info);
			free(cb_info);
			break;
		}
	}

	if (g_slist_length(__subscribe_list) == 0) {
		pims_ipc_unsubscribe(__ipc, CAL_IPC_MODULE_FOR_SUBSCRIPTION, (char *)CAL_NOTI_REMINDER_CAHNGED);
		g_slist_free(__subscribe_list);
		__subscribe_list = NULL;
	}

	_cal_mutex_unlock(CAL_MUTEX_PIMS_IPC_PUBSUB);

	return CALENDAR_ERROR_NONE;
}


