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
#include <pims-ipc-svc.h>
#include <pims-ipc-data.h>

#include "calendar.h"
#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_db.h"
#include "cal_db_util.h"
#include "cal_ipc.h"
#include "cal_ipc_marshal.h"
#include "cal_server_ipc.h"

#define CAL_SUBSCRIBE_MAX_LEN 1024

static gboolean _cal_server_reminder_publish_changes_with_data(char *data, int len)
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
		pims_ipc_data_destroy(indata);
		return false;
	}
	if (pims_ipc_data_put(indata, data, strlen(data) + 1) != 0)
	{
		ERR("pims_ipc_data_put() failed");
		pims_ipc_data_destroy(indata);
		return false;
	}
	if (pims_ipc_svc_publish(CAL_IPC_MODULE_FOR_SUBSCRIPTION, (char *)CAL_NOTI_REMINDER_CAHNGED, indata) != 0)
	{
		ERR("pims_ipc_svc_publish() failed");
		pims_ipc_data_destroy(indata);
		return false;
	}
	pims_ipc_data_destroy(indata);
	return true;
}

void cal_server_reminder_publish(char *p)
{
	RET_IF(NULL == p);
	_cal_server_reminder_publish_changes_with_data(p, strlen(p));
}

int cal_server_reminder_add_callback_data(char **p, char *key, char *value)
{
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

	if (NULL == *p)
	{
		int len = len_key + len_value + 2;
		*p = calloc(len, sizeof(char));
		if (NULL == *p) {
			ERR("calloc() failed");
			return CALENDAR_ERROR_DB_FAILED;
		}
		snprintf(*p, len, "%s=%s", key, value);

	} else {
		int len = strlen(*p) + len_key + len_value + 3;
		char *temp = calloc(len, sizeof(char));
		if (NULL == temp) {
			ERR("recalloc() failed");
			return CALENDAR_ERROR_DB_FAILED;
		}
		snprintf(temp, len, "%s&%s=%s", *p, key, value);
		free(*p);
		*p = temp;
	}
	return CALENDAR_ERROR_NONE;
}

