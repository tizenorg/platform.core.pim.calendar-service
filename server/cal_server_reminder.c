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


#include <stdlib.h>
#include <pims-ipc-svc.h>

#include "calendar2.h"
#include "cal_internal.h" // DBG
#include "cal_typedef.h"
#include "cal_db.h"
#include "cal_db_util.h"
#include "cal_ipc.h"
#include "cal_ipc_marshal.h"
#include "cal_server_ipc.h"

#define CAL_SUBSCRIBE_MAX_LEN 1024

static __thread char *__data = NULL;
static __thread int __len_data = 0;

static gboolean __cal_server_reminder_publish_changes_with_data(unsigned char *data, int len)
{
	pims_ipc_data_h indata = NULL;
	if (NULL == data)
	{
		ERR("Invalid parameter: data is NULL");
		return true;
	}

	indata = pims_ipc_data_create(0);
	if (NULL == indata)
	{
		ERR("pims_ipc_data_create() failed");
		return false;
	}
	if (pims_ipc_data_put(indata, &len, sizeof(int)) != 0)
	{
		ERR("pims_ipc_data_put() failed");
		return false;
	}
	if (pims_ipc_data_put(indata, data, len + 1) != 0)
	{
		ERR("pims_ipc_data_put() failed");
		return false;
	}
	if (pims_ipc_svc_publish(CAL_IPC_MODULE_FOR_SUBSCRIPTION, (char *)CAL_NOTI_REMINDER_CAHNGED, indata) != 0)
	{
		ERR("pims_ipc_svc_publish() failed");
		return false;
	}
	pims_ipc_data_destroy(indata);
	return true;
}

void __cal_server_reminder_clear_changed_info(void)
{
	if (__data)
	{
		free(__data);
		__data = NULL;
		__len_data = 0;
	}
}

void _cal_server_reminder_publish(void)
{
	if (__data)
	{
		__cal_server_reminder_publish_changes_with_data((unsigned char *)__data, __len_data);
		__cal_server_reminder_clear_changed_info();
	}
}

int _cal_server_reminder_add_callback_data(char *key, char *value)
{
	int len_data = 0;
	int len_key = 0;
	int len_value = 0;

	if (NULL == key || NULL == value)
	{
		ERR("Invalid parameter");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	len_key = strlen(key);
	len_value = strlen(value);
	DBG("key[%s]value[%s]", key, value);
	if (NULL == __data)
	{
		__len_data = len_key + len_value + 2;
		__data = calloc(__len_data, sizeof(char));
		if (NULL == __data)
		{
			ERR("calloc() failed");
			return CALENDAR_ERROR_DB_FAILED;
		}
		snprintf(__data, __len_data, "%s=%s", key, value);
	}
	else
	{
		char *p = NULL;
		len_data = strlen(__data);

		__len_data = len_data + len_key + len_value + 3;
		p = calloc(__len_data, sizeof(char));
		if (NULL == __data)
		{
			ERR("recalloc() failed");
			return CALENDAR_ERROR_DB_FAILED;
		}
		snprintf(p, __len_data, "%s&%s=%s", __data, key, value);
		free(__data);
		__data = p;

	}

	DBG("data[%s]len(%d)", __data, __len_data);
	return CALENDAR_ERROR_NONE;
}

