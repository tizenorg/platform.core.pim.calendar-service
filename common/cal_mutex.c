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

#include <pthread.h>

#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_mutex.h"

static pthread_mutex_t __cal_property_hash_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t __cal_connection_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t __cal_pims_ipc_call_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t __cal_inotify_mutex = PTHREAD_MUTEX_INITIALIZER;

static inline pthread_mutex_t* cts_mutex_get_mutex(int type)
{
    pthread_mutex_t *ret_val;

    switch (type) {
    case CAL_MUTEX_PROPERTY_HASH:
        ret_val = &__cal_property_hash_mutex;
        break;
    case CAL_MUTEX_CONNECTION:
        ret_val = &__cal_connection_mutex;
        break;
    case CAL_MUTEX_PIMS_IPC_CALL:
        ret_val = &__cal_pims_ipc_call_mutex;
        break;
    case CAL_MUTEX_INOTIFY:
        ret_val = &__cal_inotify_mutex;
        break;
    default:
        ERR("unknown type(%d)", type);
        ret_val = NULL;
        break;
    }
    return ret_val;
}

void _cal_mutex_lock(int type)
{
    int ret;
    pthread_mutex_t *mutex;

    mutex = cts_mutex_get_mutex(type);

    if (mutex != NULL)
    {
        ret = pthread_mutex_lock(mutex);
        retm_if(ret, "mutex_lock Failed(%d)", ret);
    }
}

void _cal_mutex_unlock(int type)
{
    int ret;
    pthread_mutex_t *mutex;

    mutex = cts_mutex_get_mutex(type);

    if (mutex != NULL)
    {
        ret = pthread_mutex_unlock(mutex);
        retm_if(ret, "mutex_unlock Failed(%d)", ret);
    }
}
