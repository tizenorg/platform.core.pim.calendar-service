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

#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_mutex.h"
#include "cal_server.h"

static guint cal_timeout_id = 0;

void cal_ondemand_stop(void)
{
	CAL_FN_CALL();

	cal_mutex_lock(CAL_MUTEX_TIMEOUT);
	g_source_remove(cal_timeout_id);
	cal_timeout_id = 0;
	cal_mutex_unlock(CAL_MUTEX_TIMEOUT);
}

static gboolean _timeout_cb(gpointer user_data)
{
	DBG("exit");
	cal_server_quit_loop();
	return TRUE;
}

void cal_ondemand_start(void)
{
	CAL_FN_CALL();

	int timeout = cal_server_get_timeout();
	if (timeout < 1)
		return;

	cal_mutex_lock(CAL_MUTEX_TIMEOUT);
	if (cal_timeout_id > 0)
		g_source_remove(cal_timeout_id);
	cal_timeout_id = g_timeout_add_seconds(timeout, _timeout_cb, NULL);
	cal_mutex_unlock(CAL_MUTEX_TIMEOUT);
}
