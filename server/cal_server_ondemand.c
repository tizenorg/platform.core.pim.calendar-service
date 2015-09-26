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

#include <pthread.h>

#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_mutex.h"
#include "cal_server.h"

static pthread_mutex_t cal_mutex_timeout = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t cal_mutex_holding = PTHREAD_MUTEX_INITIALIZER;
static guint cal_timeout_id = 0;
static gboolean cal_holding = FALSE;

void cal_server_ondemand_stop(void)
{
	CAL_FN_CALL();

	int timeout = cal_server_get_timeout();
	if (timeout < 1)
		return;

	pthread_mutex_lock(&cal_mutex_timeout);
	g_source_remove(cal_timeout_id);
	cal_timeout_id = 0;
	pthread_mutex_unlock(&cal_mutex_timeout);
}

static gboolean _timeout_cb(gpointer user_data)
{
	CAL_FN_CALL();

	pthread_mutex_lock(&cal_mutex_holding);
	if (FALSE == cal_holding) {
		DBG("exit");
		cal_server_quit_loop();
	}
	pthread_mutex_unlock(&cal_mutex_holding);
	return TRUE;
}

void cal_server_ondemand_start(void)
{
	CAL_FN_CALL();

	int timeout = cal_server_get_timeout();
	if (timeout < 1)
		return;

	DBG("timeout(%d)", timeout);
	pthread_mutex_lock(&cal_mutex_timeout);
	if (cal_timeout_id > 0)
		g_source_remove(cal_timeout_id);
	cal_timeout_id = g_timeout_add_seconds(timeout, _timeout_cb, NULL);
	pthread_mutex_unlock(&cal_mutex_timeout);
}

void cal_server_ondemand_hold(void)
{
	pthread_mutex_lock(&cal_mutex_holding);
	cal_holding = TRUE;
	pthread_mutex_unlock(&cal_mutex_holding);
}

void cal_server_ondemand_release(void)
{
	pthread_mutex_lock(&cal_mutex_holding);
	cal_holding = FALSE;
	pthread_mutex_unlock(&cal_mutex_holding);
}
