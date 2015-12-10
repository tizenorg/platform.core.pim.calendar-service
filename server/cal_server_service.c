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

#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_view.h"
#include "cal_db_util.h"
#include "cal_utils.h"
#include "cal_mutex.h"
#include "cal_inotify.h"

static int cal_total_connection = 0;
static TLS int cal_thread_connection = 0;

int cal_connect(void)
{
	CAL_FN_CALL();
	int ret = 0;

	cal_mutex_lock(CAL_MUTEX_CONNECTION);
	if (0 == cal_total_connection) {
#if !GLIB_CHECK_VERSION(2, 35, 0)
		g_type_init();	/* added for alarmmgr */
#endif
		cal_view_initialize();
		ret = cal_inotify_init();
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_inotify_init() Fail(%d)", ret);
			cal_mutex_unlock(CAL_MUTEX_CONNECTION);
			return ret;
		}
	} else {
		DBG("[System] calendar service has been already connected");
	}
	cal_total_connection++;

	if (0 == cal_thread_connection) {
		ret = cal_db_util_open();
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("_cal_db_open() Fail(%d)", ret);
			cal_mutex_unlock(CAL_MUTEX_CONNECTION);
			return ret;
		}
	}
	cal_thread_connection++;
	cal_mutex_unlock(CAL_MUTEX_CONNECTION);

	return CALENDAR_ERROR_NONE;
}

int cal_disconnect(void)
{
	CAL_FN_CALL();

	cal_mutex_lock(CAL_MUTEX_CONNECTION);
	if (1 == cal_thread_connection) {
		cal_db_util_close();
	} else if (cal_thread_connection <= 0) {
		DBG("[System] not connected, count(%d)", cal_thread_connection);
		cal_mutex_unlock(CAL_MUTEX_CONNECTION);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	cal_thread_connection--;

	if (1 == cal_total_connection) {
		cal_inotify_deinit();
		cal_view_finalize();
	} else if (1 < cal_total_connection) {
		DBG("[System] connection count(%d)", cal_total_connection);
	} else {
		DBG("[System] Not connected, count(%d)", cal_total_connection);
		cal_mutex_unlock(CAL_MUTEX_CONNECTION);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	cal_total_connection--;

	cal_mutex_unlock(CAL_MUTEX_CONNECTION);
	return CALENDAR_ERROR_NONE;
}

void cal_service_internal_disconnect(void)
{
	cal_mutex_lock(CAL_MUTEX_CONNECTION);

	if (1 == cal_thread_connection) {
		cal_db_util_close();
		cal_thread_connection--;

		if (1 <= cal_total_connection)
			cal_total_connection--;
	}
	cal_mutex_unlock(CAL_MUTEX_CONNECTION);
	return;
}
