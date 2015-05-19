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

void cal_server_reminder_publish(char *p, int p_len)
{
	RET_IF(NULL == p);

	pims_ipc_data_h indata = NULL;
	indata = pims_ipc_data_create(0);
	RETM_IF(NULL == indata, "pims_ipc_data_create() Fail");

	if (pims_ipc_data_put(indata, &p_len, sizeof(int)) != 0) {
		ERR("pims_ipc_data_put() Fail");
		pims_ipc_data_destroy(indata);
		return;
	}
	if (pims_ipc_data_put(indata, p, p_len + 1) != 0) {
		ERR("pims_ipc_data_put() Fail");
		pims_ipc_data_destroy(indata);
		return;
	}
	if (pims_ipc_svc_publish(CAL_IPC_MODULE_FOR_SUBSCRIPTION, (char *)CAL_NOTI_REMINDER_CAHNGED, indata) != 0) {
		ERR("pims_ipc_svc_publish() Fail");
		pims_ipc_data_destroy(indata);
		return;
	}
	pims_ipc_data_destroy(indata);
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

