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
#include <stdlib.h>     //calloc
#include <pims-ipc.h>
#include <glib-object.h> //g_type_init

#include "calendar_service.h"
#include "calendar_db.h"
#include "calendar_types2.h"

#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_inotify.h"
#include "cal_view.h"
#include "cal_record.h"
#include "cal_list.h"
#include "cal_mutex.h"

#include "cal_ipc.h"
#include "cal_ipc_marshal.h"

#include "cal_client_ipc.h"
#include "cal_client_reminder.h"

static TLS pims_ipc_h calendar_ipc_thread = NULL;
static pims_ipc_h calendar_ipc = NULL;
static int calendar_connection_count = 0;
static int calendar_change_version = 0;
static TLS int calendar_change_version_thread = 0;

pims_ipc_h __cal_client_ipc_get_handle(void);
void __cal_client_ipc_lock(void);
void __cal_client_ipc_unlock(void);

API int calendar_connect(void)
{
    int ret = CALENDAR_ERROR_NONE;
    pims_ipc_data_h indata = NULL;
    pims_ipc_data_h outdata = NULL;
    pims_ipc_h ipc_handle = NULL;

    CAL_FN_CALL;

    _cal_mutex_lock(CAL_MUTEX_CONNECTION);
    // ipc create
    if (calendar_ipc == NULL)
    {
        ipc_handle = pims_ipc_create(CAL_IPC_SOCKET_PATH);
        if (ipc_handle == NULL)
        {
            ERR("pims_ipc_create() Failed(%d)", CALENDAR_ERROR_OUT_OF_MEMORY);
            ret = CALENDAR_ERROR_OUT_OF_MEMORY;
            goto ERROR_RETURN;
        }
    }
    else
    {
        calendar_connection_count++;
        CAL_DBG("calendar already connected = %d",calendar_connection_count);
        ret = CALENDAR_ERROR_NONE;
        _cal_mutex_unlock(CAL_MUTEX_CONNECTION);
        return ret;
    }

    // ipc call
    if (pims_ipc_call(ipc_handle, CAL_IPC_MODULE, CAL_IPC_SERVER_CONNECT, indata, &outdata) != 0)
    {
        ERR("pims_ipc_call failed");
        ret = CALENDAR_ERROR_IPC;
        goto ERROR_RETURN;
    }

    if (outdata)
    {
        // check outdata
        unsigned int size = 0;
        ret = *(int*) pims_ipc_data_get(outdata,&size);

        pims_ipc_data_destroy(outdata);

        if (ret != CALENDAR_ERROR_NONE)
        {
            ERR("calendar_connect return (%d)",ret);
            goto ERROR_RETURN;
        }
    }
    else
    {
        ERR("ipc outdata is NULL");
        ret = CALENDAR_ERROR_IPC;
        goto ERROR_RETURN;
    }

    g_type_init();  // added for alarmmgr

    if (_cal_inotify_initialize() !=  CALENDAR_ERROR_NONE)
    {
        ERR("_cal_inotify_initialize failed");
    }

    _cal_view_initialize();

    if (0 == calendar_connection_count)
    {
		_cal_client_reminder_create_for_subscribe();
	}
    calendar_connection_count++;
    calendar_ipc = ipc_handle;
    _cal_mutex_unlock(CAL_MUTEX_CONNECTION);

	return ret;

ERROR_RETURN:
    if (ipc_handle != NULL)
    {
        pims_ipc_destroy(ipc_handle);
        ipc_handle = NULL;
    }
	_cal_mutex_unlock(CAL_MUTEX_CONNECTION);
	return ret;
}

API int calendar_disconnect(void)
{
    int ret = CALENDAR_ERROR_NONE;
    pims_ipc_data_h indata = NULL;
    pims_ipc_data_h outdata = NULL;

    retvm_if(calendar_ipc==NULL,CALENDAR_ERROR_NOT_PERMITTED,"calendar not connected");

    CAL_FN_CALL;
    _cal_mutex_lock(CAL_MUTEX_CONNECTION);

    if (calendar_connection_count > 1)
    {
        calendar_connection_count--;
        CAL_DBG("calendar connect count -1 = %d",calendar_connection_count);
        ret = CALENDAR_ERROR_NONE;
        _cal_mutex_unlock(CAL_MUTEX_CONNECTION);
        return ret;
    }
	else
	{
		_cal_client_reminder_destroy_for_subscribe();
	}

    // ipc call
    if (pims_ipc_call(calendar_ipc, CAL_IPC_MODULE, CAL_IPC_SERVER_DISCONNECT, indata, &outdata) != 0)
    {
        ERR("pims_ipc_call failed");
        ret = CALENDAR_ERROR_NOT_PERMITTED;
        goto ERROR_RETURN;
    }

    if (outdata)
    {
        // check outdata
        unsigned int size = 0;
        ret = *(int*) pims_ipc_data_get(outdata,&size);

        pims_ipc_data_destroy(outdata);

    }
    else
    {
        ERR("ipc outdata is NULL");
        ret = CALENDAR_ERROR_IPC;
        goto ERROR_RETURN;
    }

    if (calendar_ipc && ret == CALENDAR_ERROR_NONE)
    {
        pims_ipc_destroy(calendar_ipc);
        calendar_ipc = NULL;

        _cal_inotify_finalize();
        _cal_view_finalize();
    }

    _cal_mutex_unlock(CAL_MUTEX_CONNECTION);
    return ret;
ERROR_RETURN:

    _cal_mutex_unlock(CAL_MUTEX_CONNECTION);
    return ret;
}

API int calendar_connect_on_thread(void)
{
    int ret = CALENDAR_ERROR_NONE;
    pims_ipc_data_h indata = NULL;
    pims_ipc_data_h outdata = NULL;

    CAL_FN_CALL;

    // ipc create
    if (calendar_ipc_thread == NULL)
    {
        calendar_ipc_thread = pims_ipc_create(CAL_IPC_SOCKET_PATH);
        if (calendar_ipc_thread == NULL)
        {
            ERR("pims_ipc_create() Failed(%d)", CALENDAR_ERROR_OUT_OF_MEMORY);
            ret = CALENDAR_ERROR_OUT_OF_MEMORY;
            goto ERROR_RETURN;
        }
    }
    else
    {
        CAL_DBG("calendar already connected");
        ret = CALENDAR_ERROR_NONE;
        goto ERROR_RETURN;
    }

    // ipc call
    if (pims_ipc_call(calendar_ipc_thread, CAL_IPC_MODULE, CAL_IPC_SERVER_CONNECT, indata, &outdata) != 0)
    {
        ERR("pims_ipc_call failed");
        ret = CALENDAR_ERROR_IPC;
        goto ERROR_RETURN;
    }

    if (outdata)
    {
        // check outdata
        unsigned int size = 0;
        ret = *(int*) pims_ipc_data_get(outdata,&size);

        pims_ipc_data_destroy(outdata);

        if (ret != CALENDAR_ERROR_NONE)
        {
            ERR("calendar_connect return (%d)",ret);
            goto ERROR_RETURN;
        }
    }
    else
    {
        ERR("ipc outdata is NULL");
        ret = CALENDAR_ERROR_IPC;
        goto ERROR_RETURN;
    }

    if (_cal_inotify_initialize() !=  CALENDAR_ERROR_NONE)
    {
        ERR("_cal_inotify_initialize failed");
    }

    _cal_view_initialize();
    return ret;

ERROR_RETURN:
    if (calendar_ipc_thread != NULL)
    {
        pims_ipc_destroy(calendar_ipc_thread);
        calendar_ipc_thread = NULL;
    }

    return ret;
}

API int calendar_disconnect_on_thread(void)
{
    int ret = CALENDAR_ERROR_NONE;
    pims_ipc_data_h indata = NULL;
    pims_ipc_data_h outdata = NULL;

    retvm_if(calendar_ipc_thread==NULL,CALENDAR_ERROR_NOT_PERMITTED,"calendar_thread not connected");

    CAL_FN_CALL;

    // ipc call
    if (pims_ipc_call(calendar_ipc_thread, CAL_IPC_MODULE, CAL_IPC_SERVER_DISCONNECT, indata, &outdata) != 0)
    {
        ERR("pims_ipc_call failed");
        ret = CALENDAR_ERROR_NOT_PERMITTED;
        goto ERROR_RETURN;
    }

    if (outdata)
    {
        // check outdata
        unsigned int size = 0;
        ret = *(int*) pims_ipc_data_get(outdata,&size);

        pims_ipc_data_destroy(outdata);

    }
    else
    {
        ERR("ipc outdata is NULL");
        ret = CALENDAR_ERROR_IPC;
        goto ERROR_RETURN;
    }

    if (calendar_ipc_thread && ret == CALENDAR_ERROR_NONE)
    {
        pims_ipc_destroy(calendar_ipc_thread);
        calendar_ipc_thread = NULL;

        _cal_inotify_finalize();
        _cal_view_finalize();
    }

    return ret;
ERROR_RETURN:

    return ret;
}

API int calendar_connect_with_flags(unsigned int flags)
{
    int ret = CALENDAR_ERROR_NONE;

    ret = calendar_connect();
    if (ret != CALENDAR_ERROR_NONE)
    {
        if (flags & CALENDAR_CONNECT_FLAG_RETRY)
        {
            int retry_time = 500;
            int i = 0;
            for(i=0;i<9;i++)
            {
                usleep(retry_time*1000);
                ret = calendar_connect();
                DBG("retry cnt=%d, ret=%x",(i+1), ret);
                if (ret == CALENDAR_ERROR_NONE)
                    break;
                if (i>6)
                    retry_time += 30000;
                else
                    retry_time *= 2;
            }

        }
    }

    return ret;
}

bool _cal_client_ipc_is_call_inprogress(void)
{
	return pims_ipc_is_call_in_progress(calendar_ipc);
}

pims_ipc_h __cal_client_ipc_get_handle(void)
{
    if (calendar_ipc_thread == NULL)
    {
        return calendar_ipc;
    }
    return calendar_ipc_thread;
}

void __cal_client_ipc_lock(void)
{
    if (calendar_ipc_thread == NULL)
    {
        _cal_mutex_lock(CAL_MUTEX_PIMS_IPC_CALL);
    }
}

void __cal_client_ipc_unlock(void)
{
    if (calendar_ipc_thread == NULL)
    {
        _cal_mutex_unlock(CAL_MUTEX_PIMS_IPC_CALL);
    }
}

int _cal_client_ipc_call(char *module, char *function, pims_ipc_h data_in,
        pims_ipc_data_h *data_out)
{
    int ret = 0;
    pims_ipc_h ipc_handle = __cal_client_ipc_get_handle();

    __cal_client_ipc_lock();

    ret = pims_ipc_call(ipc_handle, module, function, data_in, data_out);

    __cal_client_ipc_unlock();

    return ret;
}

int _cal_client_ipc_call_async(char *module, char *function, pims_ipc_h data_in,
        pims_ipc_call_async_cb callback, void *userdata)
{
    int ret = 0;
    pims_ipc_h ipc_handle = __cal_client_ipc_get_handle();

    __cal_client_ipc_lock();

    ret = pims_ipc_call_async(ipc_handle, module, function, data_in, callback, userdata);

    __cal_client_ipc_unlock();

    return ret;
}

void _cal_client_ipc_set_change_version(int version)
{
    if (calendar_ipc_thread == NULL)
    {
        calendar_change_version = version;
        CAL_DBG("change_version=%d",version);
        return ;
    }
    calendar_change_version_thread = version;
    CAL_DBG("change_version=%d",version);
}

int _cal_client_ipc_get_change_version(void)
{
    if (calendar_ipc_thread == NULL)
    {
        return calendar_change_version;
    }
    return calendar_change_version_thread;

}
