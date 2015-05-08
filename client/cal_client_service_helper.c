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

#include "calendar_types.h"
#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_inotify.h"
#include "cal_view.h"
#include "cal_mutex.h"
#include "cal_client_reminder.h"
#include "cal_client_ipc.h"

static int cal_connection = 0;
static TLS int cal_connection_on_thread = 0;

int cal_client_connect(void)
{
	CAL_FN_CALL();
	int ret = 0;

	cal_mutex_lock(CAL_MUTEX_CONNECTION);
	if (0 == cal_connection) {
		ret = cal_client_ipc_connect();
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_client_ipc_connect() Fail(%d)", ret);
			cal_mutex_unlock(CAL_MUTEX_CONNECTION);
			return ret;
		}

		g_type_init(); // for alarmmgr
		cal_inotify_initialize();
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_inotify_initialize() Fail(%d)", ret);
			cal_mutex_unlock(CAL_MUTEX_CONNECTION);
			return ret;
		}
		cal_view_initialize();
		cal_client_reminder_create_for_subscribe();
	}

	cal_connection++;
	cal_mutex_unlock(CAL_MUTEX_CONNECTION);
	return CALENDAR_ERROR_NONE;
}

int cal_client_disconnect(void)
{
	CAL_FN_CALL();
	int ret = 0;

	cal_mutex_lock(CAL_MUTEX_CONNECTION);
	if (1 == cal_connection) {
		cal_client_reminder_destroy_for_subscribe();

		ret = cal_client_ipc_disconnect();
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_client_ipc_disconnect() Fail(%d)", ret);
			cal_mutex_unlock(CAL_MUTEX_CONNECTION);
			return ret;
		}

		DBG("[System] disonneted successfully");
		cal_view_finalize();
		cal_inotify_finalize();
	}
	else if (1 < cal_connection) {
		DBG("[System] connection count(%d)", cal_connection);
	}
	else {
		DBG("[System] calendar_connect() is needed");
		cal_mutex_unlock(CAL_MUTEX_CONNECTION);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	cal_connection--;
	cal_mutex_unlock(CAL_MUTEX_CONNECTION);
	return CALENDAR_ERROR_NONE;
}

int cal_client_connect_on_thread(void)
{
	CAL_FN_CALL();
	int ret = 0;

	cal_mutex_lock(CAL_MUTEX_CONNECTION);
	if (0 == cal_connection_on_thread) {
		ret = cal_client_ipc_connect_on_thread();
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_client_ipc_connect_on_thread() Fail(%d)", ret);
			cal_mutex_unlock(CAL_MUTEX_CONNECTION);
			return ret;
		}

		ret = cal_inotify_initialize();
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_inotify_initialize() Fail(%d)", ret);
			cal_mutex_unlock(CAL_MUTEX_CONNECTION);
			return ret;
		}

		cal_view_initialize();
		cal_client_reminder_create_for_subscribe();
	}
	else if (0 < cal_connection_on_thread) {
		DBG("[System] calendar service is already connected");
	}

	cal_connection_on_thread++;
	cal_mutex_unlock(CAL_MUTEX_CONNECTION);
	return CALENDAR_ERROR_NONE;
}

int cal_client_disconnect_on_thread(void)
{
	CAL_FN_CALL();
	int ret = 0;

	cal_mutex_lock(CAL_MUTEX_CONNECTION);
	if (1 == cal_connection_on_thread) {
		cal_client_reminder_destroy_for_subscribe();

		ret = cal_client_ipc_disconnect_on_thread();
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_client_ipc_disconnect_on_thread() Fail(%d)", ret);
			cal_mutex_unlock(CAL_MUTEX_CONNECTION);
			return ret;
		}

		DBG("[System] disonneted on thread successfully");
		cal_view_finalize();
		cal_inotify_finalize();
	}
	else if (1 < cal_connection_on_thread) {
		DBG("[system] connection_on_thread count(%d)", cal_connection_on_thread);
	}
	else {
		DBG("[system] calendar_connect_on_thread() is needed");
		cal_mutex_unlock(CAL_MUTEX_CONNECTION);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	cal_connection_on_thread--;
	cal_mutex_unlock(CAL_MUTEX_CONNECTION);
	return CALENDAR_ERROR_NONE;
}

int cal_client_connect_with_flags(unsigned int flags)
{
	CAL_FN_CALL();
	int ret = 0;

	// If new flag is defined, error check should be updated.
	RETVM_IF(flags & 0x11111110, CALENDAR_ERROR_INVALID_PARAMETER, "flag is invalid(%x)", flags);

	ret = cal_client_connect();
	if (CALENDAR_ERROR_NONE == ret)
		return ret;

	if (flags & CALENDAR_CONNECT_FLAG_RETRY) {
		int i = 0;
		int retry_time = 500;
		for (i = 0; i < 9; i++) {
			usleep(retry_time*1000);
			ret = cal_client_connect();
			DBG("retry count(%d), ret(%x)",(i+1), ret);
			if (CALENDAR_ERROR_NONE == ret)
				break;
			if (6 < i)
				retry_time += 30000;
			else
				retry_time *= 2;
		}
	}
	return ret;
}
