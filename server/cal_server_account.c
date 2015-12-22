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
#include <account.h>

#include "calendar.h"
#include "cal_typedef.h"
#include "cal_internal.h"
#include "cal_db_plugin_calendar_helper.h"
#include "cal_server_contacts.h"

static pthread_mutex_t cal_mutex_account = PTHREAD_MUTEX_INITIALIZER;
static account_subscribe_h cal_account_h = NULL;

static bool _noti_cb(const char* event_type, int account_id,
		void* user_data)
{
	CAL_FN_CALL();

	if (CAL_STRING_EQUAL == strcmp(event_type, ACCOUNT_NOTI_NAME_DELETE)) {
		cal_db_delete_account(account_id);
		cal_server_contacts_delete(account_id);
	}
	return true;
}

int cal_server_account_init(void)
{
	int ret = 0;

	pthread_mutex_lock(&cal_mutex_account);
	ret = account_subscribe_create(&cal_account_h);
	if (ACCOUNT_ERROR_NONE != ret) {
		ERR("account_subscribe_create() Fail(%d)", ret);
		pthread_mutex_unlock(&cal_mutex_account);
		return CALENDAR_ERROR_SYSTEM;
	}

	ret = account_subscribe_notification(cal_account_h, _noti_cb, NULL);
	if (ACCOUNT_ERROR_NONE != ret)
		WARN("account_subscribe_notification Failed (%d)", ret);

	pthread_mutex_unlock(&cal_mutex_account);
	return CALENDAR_ERROR_NONE;
}

void cal_server_account_deinit(void)
{
	pthread_mutex_lock(&cal_mutex_account);

	if (cal_account_h) {
		account_unsubscribe_notification(cal_account_h);
		cal_account_h = NULL;
	}

	pthread_mutex_unlock(&cal_mutex_account);
}

