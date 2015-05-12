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

static pthread_mutex_t _cal_property_hash_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t _cal_connection_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t _cal_pims_ipc_call_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t _cal_inotify_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t _cal_pims_ipc_pubsub_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t _cal_access_control_mutex = PTHREAD_MUTEX_INITIALIZER;

static inline pthread_mutex_t* _cal_mutex_get_mutex(int type)
{
	pthread_mutex_t *ret_val;

	switch (type) {
	case CAL_MUTEX_PROPERTY_HASH:
		ret_val = &_cal_property_hash_mutex;
		break;
	case CAL_MUTEX_CONNECTION:
		ret_val = &_cal_connection_mutex;
		break;
	case CAL_MUTEX_PIMS_IPC_CALL:
		ret_val = &_cal_pims_ipc_call_mutex;
		break;
	case CAL_MUTEX_INOTIFY:
		ret_val = &_cal_inotify_mutex;
		break;
	case CAL_MUTEX_PIMS_IPC_PUBSUB:
		ret_val = &_cal_pims_ipc_pubsub_mutex;
		break;
	case CAL_MUTEX_ACCESS_CONTROL:
		ret_val = &_cal_access_control_mutex;
		break;
	default:
		ERR("unknown type(%d)", type);
		ret_val = NULL;
		break;
	}
	return ret_val;
}

void cal_mutex_lock(int type)
{
	int ret;
	pthread_mutex_t *mutex;

	mutex = _cal_mutex_get_mutex(type);

	if (mutex) {
		ret = pthread_mutex_lock(mutex);
		RETM_IF(ret, "mutex_lock Failed(%d)", ret);
	}
}

void cal_mutex_unlock(int type)
{
	int ret;
	pthread_mutex_t *mutex;

	mutex = _cal_mutex_get_mutex(type);

	if (mutex) {
		ret = pthread_mutex_unlock(mutex);
		RETM_IF(ret, "mutex_unlock Failed(%d)", ret);
	}
}
