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
#include <stdlib.h>
#include <glib-object.h>

#include "calendar.h"
#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_inotify.h"
#include "cal_view.h"
#include "cal_mutex.h"
#include "cal_client_reminder.h"
#include "cal_client_handle.h"
#include "cal_client_dbus.h"
#include "cal_client_utils.h"
#include "cal_client_reminder.h"

static int reference_count = 0; /* total connection include on_thread */

int cal_client_connect(calendar_h handle, unsigned int id, int *connection_count)
{
	CAL_FN_CALL();
	int ret = 0;

	RETV_IF(NULL == handle, CALENDAR_ERROR_INVALID_PARAMETER);

	cal_mutex_lock(CAL_MUTEX_CONNECTION);
	cal_s *h = (cal_s *)handle;

	h->connection_count++;
	DBG("[Connection count:handle] (%d)", h->connection_count);

	if (0 == *connection_count) { /* total connection */
#if !GLIB_CHECK_VERSION(2, 35, 0)
		g_type_init(); /* for alarmmgr */
#endif
		ret = cal_inotify_init();
		if (CALENDAR_ERROR_NONE != ret) {
			/* LCOV_EXCL_START */
			ERR("cal_inotify_init() Fail(%d)", ret);
			cal_mutex_unlock(CAL_MUTEX_CONNECTION);
			return ret;
			/* LCOV_EXCL_STOP */
		}

		cal_view_initialize();
	} else if (0 < *connection_count) {
		DBG("[System] calendar service is already connected");
	}

	(*connection_count)++;

	if (0 == reference_count)
		cal_dbus_start();
	reference_count++;

	DBG("[Connection count]:total(%d) reference(%d)", *connection_count, reference_count);

	cal_mutex_unlock(CAL_MUTEX_CONNECTION);
	return CALENDAR_ERROR_NONE;
}

int cal_client_disconnect(calendar_h handle, unsigned int id, int *connection_count)
{
	CAL_FN_CALL();
	int ret = 0;

	RETV_IF(NULL == handle, CALENDAR_ERROR_INVALID_PARAMETER);

	cal_mutex_lock(CAL_MUTEX_CONNECTION);
	cal_s *h = (cal_s *)handle;

	h->connection_count--;
	DBG("[Disonnection count:handle] (%d)", h->connection_count);

	if (0 == h->connection_count) {
		ret = cal_client_handle_remove(id, handle);
		if (CALENDAR_ERROR_NONE != ret)
			WARN("cal_client_handle_remove() Fail(%d)", ret);
	}

	if (1 == *connection_count) {
		DBG("[System] disconnected successfully");
		cal_view_finalize();
		cal_inotify_deinit();
	} else if (1 < *connection_count) {
		DBG("[System] connection count(%d)", *connection_count);
	} else {
		/* LCOV_EXCL_START */
		ERR("[System] calendar_connect() is needed");
		cal_mutex_unlock(CAL_MUTEX_CONNECTION);
		return CALENDAR_ERROR_INVALID_PARAMETER;
		/* LCOV_EXCL_STOP */
	}

	(*connection_count)--;

	if (1 == reference_count)
		cal_dbus_stop();
	reference_count--;

	DBG("[Connection count]:total(%d) reference(%d)", *connection_count, reference_count);

	cal_mutex_unlock(CAL_MUTEX_CONNECTION);
	return CALENDAR_ERROR_NONE;
}

int cal_client_connect_with_flags(calendar_h handle, unsigned int id,
		int *connection_count, unsigned int flags)
{
	CAL_FN_CALL();
	int ret = 0;

	/* If new flag is defined, error check should be updated. */
	RETVM_IF(flags & 0x11111110, CALENDAR_ERROR_INVALID_PARAMETER, "flag is invalid(%x)", flags);

	ret = cal_client_connect(handle, id, connection_count);
	if (CALENDAR_ERROR_NONE == ret)
		return ret;

	if (flags & CALENDAR_CONNECT_FLAG_RETRY) {
		int i = 0;
		int retry_time = 500;
		for (i = 0; i < 9; i++) {
			usleep(retry_time*1000);
			ret = cal_client_connect(handle, id, connection_count);
			DBG("retry count(%d), ret(%x)", (i+1), ret);
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
