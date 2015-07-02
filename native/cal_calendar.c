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

static int calsvc_connection = 0;
static TLS int thread_connection = 0;

API int calendar_connect(void)
{
	CAL_FN_CALL;
	int ret = 0;

	_cal_mutex_lock(CAL_MUTEX_CONNECTION);
	if (0 == calsvc_connection) {
		ret = _cal_inotify_initialize();
		_cal_view_initialize();
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("_cal_inotify_initialize() Fail(%d)", ret);
			_cal_mutex_unlock(CAL_MUTEX_CONNECTION);
			return ret;
		}
	}
	else {
		DBG("System : Calendar service has been already connected");
	}
	calsvc_connection++;

	if (0 == thread_connection) {
		g_type_init();	// added for alarmmgr
		ret = _cal_db_open();
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("_cal_db_open() Fail(%d)", ret);
			_cal_mutex_unlock(CAL_MUTEX_CONNECTION);
			return ret;
		}
	}
	thread_connection++;
	_cal_mutex_unlock(CAL_MUTEX_CONNECTION);

	return CALENDAR_ERROR_NONE;
}

API int calendar_disconnect(void)
{
	CAL_FN_CALL;

	_cal_mutex_lock(CAL_MUTEX_CONNECTION);
	if (1 == thread_connection) {
		_cal_db_close();
	}
	else if (thread_connection <= 0) {
		DBG("System : please call calendar_connect_on_thread(), thread_connection count is (%d)", thread_connection);
		_cal_mutex_unlock(CAL_MUTEX_CONNECTION);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	thread_connection--;

	if (1 == calsvc_connection) {
		_cal_inotify_finalize();
		_cal_view_finalize();
	}
	else if (1 < calsvc_connection) {
		DBG("System : connection count is %d", calsvc_connection);
	}
	else {
		DBG("System : please call calendar_connect(), connection count is (%d)", calsvc_connection);
		_cal_mutex_unlock(CAL_MUTEX_CONNECTION);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	calsvc_connection--;
	_cal_mutex_unlock(CAL_MUTEX_CONNECTION);

	return CALENDAR_ERROR_NONE;
}

void _cal_calendar_internal_disconnect(void)
{
	_cal_mutex_lock(CAL_MUTEX_CONNECTION);

	if (1 == thread_connection) {
		_cal_db_close();
		thread_connection--;

		if (1 <= calsvc_connection) {
			calsvc_connection--;
		}
	}
	_cal_mutex_unlock(CAL_MUTEX_CONNECTION);

	return CALENDAR_ERROR_NONE;
}
