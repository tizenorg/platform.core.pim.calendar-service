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
#include <glib.h>
#include <glib-object.h>
#include <db-util.h>

#include "cal_service.h"
#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_db.h"
#include "cal_view.h"
#include "cal_mutex.h"

#ifdef CAL_NATIVE
#include "cal_inotify.h"
#endif

static int cal_total_connection = 0;
static TLS int cal_thread_connection = 0;

API int calendar_connect(void)
{
	CAL_FN_CALL();
	int ret = 0;

	cal_mutex_lock(CAL_MUTEX_CONNECTION);
	if (0 == cal_total_connection) {
		ret = cal_inotify_initialize();
		cal_view_initialize();
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_inotify_initialize() Fail(%d)", ret);
			cal_mutex_unlock(CAL_MUTEX_CONNECTION);
			return ret;
		}
	}
	else {
		DBG("System : Calendar service has been already connected");
	}
	cal_total_connection++;

	if (0 == cal_thread_connection) {
		g_type_init();	// added for alarmmgr
		ret = cal_db_open();
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

API int calendar_disconnect(void)
{
	CAL_FN_CALL();

	cal_mutex_lock(CAL_MUTEX_CONNECTION);
	if (1 == cal_thread_connection) {
		cal_db_close();
	}
	else if (cal_thread_connection <= 0) {
		DBG("System : please call calendar_connect_on_thread(), cal_thread_connection count is (%d)", cal_thread_connection);
		cal_mutex_unlock(CAL_MUTEX_CONNECTION);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	cal_thread_connection--;

	if (1 == cal_total_connection) {
		cal_inotify_finalize();
		cal_view_finalize();
	}
	else if (1 < cal_total_connection) {
		DBG("System : connection count is %d", cal_total_connection);
	}
	else {
		DBG("System : please call calendar_connect(), connection count is (%d)", cal_total_connection);
		cal_mutex_unlock(CAL_MUTEX_CONNECTION);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	cal_total_connection--;
	cal_mutex_unlock(CAL_MUTEX_CONNECTION);

	return CALENDAR_ERROR_NONE;
}

void cal_calendar_internal_disconnect(void)
{
	cal_mutex_lock(CAL_MUTEX_CONNECTION);

	if (1 == cal_thread_connection) {
		cal_db_close();
		cal_thread_connection--;

		if (1 <= cal_total_connection) {
			cal_total_connection--;
		}
	}
	cal_mutex_unlock(CAL_MUTEX_CONNECTION);

	return CALENDAR_ERROR_NONE;
}
