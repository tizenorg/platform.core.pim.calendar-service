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
#include <stdlib.h>

#include "calendar.h"
#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_inotify.h"
#include "cal_view.h"
#include "cal_record.h"
#include "cal_list.h"
#include "cal_client_handle.h"
#include "cal_client_dbus.h"

typedef struct {
	unsigned int id;
	calendar_reminder_cb cb;
	void *user_data;
} callback_info_s;

static pthread_mutex_t cal_mutex_reminder = PTHREAD_MUTEX_INITIALIZER;
static GSList *__subscribe_list = NULL;

API int calendar_reminder_add_cb(calendar_reminder_cb callback, void *user_data)
{
	RETV_IF(NULL == callback, CALENDAR_ERROR_INVALID_PARAMETER);

	pthread_mutex_lock(&cal_mutex_reminder);

	/* check duplicate */
	GSList *cursor = __subscribe_list;
	while (cursor) {
		callback_info_s *ci = (callback_info_s *)cursor->data;
		if (NULL == ci) {
			cursor = g_slist_next(cursor);
			continue;
		}

		if (callback == ci->cb && user_data == ci->user_data) {
			DBG("This callback is already appended");
			pthread_mutex_unlock(&cal_mutex_reminder);
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
		cursor = g_slist_next(cursor);
	}

	callback_info_s *ci = NULL;
	ci = calloc(1, sizeof(callback_info_s));
	if (NULL == ci) {
		ERR("calloc() Fail");
		pthread_mutex_unlock(&cal_mutex_reminder);
		return CALENDAR_ERROR_IPC;
	}

	DBG("add reminer");
	ci->id = cal_dbus_subscribe_signal(CAL_NOTI_REMINDER_CAHNGED,
			cal_dbus_call_reminder_cb, user_data, NULL);
	ci->cb = callback;
	ci->user_data = user_data;
	__subscribe_list = g_slist_append(__subscribe_list, ci);

	pthread_mutex_unlock(&cal_mutex_reminder);

	return CALENDAR_ERROR_NONE;
}

API int calendar_reminder_remove_cb(calendar_reminder_cb callback, void *user_data)
{
	RETV_IF(NULL == callback, CALENDAR_ERROR_INVALID_PARAMETER);

	pthread_mutex_lock(&cal_mutex_reminder);

	int is_matched = 0;
	GSList *cursor = __subscribe_list;
	while (cursor) {
		callback_info_s *ci = (callback_info_s *)cursor->data;
		if (NULL == ci) {
			cursor = g_slist_next(cursor);
			continue;
		}

		if (callback == ci->cb && user_data == ci->user_data) {
			is_matched = 1;
			break;
		}
		cursor = g_slist_next(cursor);
	}

	if (0 == is_matched) {
		ERR("Not matched callback");
		pthread_mutex_unlock(&cal_mutex_reminder);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	DBG("remove reminder");
	callback_info_s *ci = (callback_info_s *)cursor->data;
	cal_dbus_unsubscribe_signal(ci->id);
	__subscribe_list = g_slist_remove(__subscribe_list, ci);
	free(ci);

	if (0 == g_slist_length(__subscribe_list)) {
		g_slist_free(__subscribe_list);
		__subscribe_list = NULL;
	}

	pthread_mutex_unlock(&cal_mutex_reminder);

	return CALENDAR_ERROR_NONE;
}

int cal_client_reminder_call_subscribe(const char *stream)
{
	CAL_FN_CALL();

	GSList *cursor = NULL;

	pthread_mutex_lock(&cal_mutex_reminder);

	cursor = __subscribe_list;
	while (cursor) {
		callback_info_s *ci = (callback_info_s *)cursor->data;
		if (NULL == ci) {
			ERR("data is NULL");
			cursor = g_slist_next(cursor);
			continue;
		}
		DBG("-----------------------------------------------------called");
		ci->cb(stream, ci->user_data);
		cursor = g_slist_next(cursor);
	}

	pthread_mutex_unlock(&cal_mutex_reminder);
	return CALENDAR_ERROR_NONE;
}
