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

#include <stdlib.h>     //calloc
#include <pims-ipc.h>

#include "calendar_service.h"
#include "calendar_db.h"
#include "calendar_types2.h"
#include "calendar_reminder.h"

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

#define CAL_IPC_DATA_FREE(ptr) \
    do { \
        if (ptr) \
        pims_ipc_data_destroy(ptr); \
        ptr = NULL; \
    } while(0)

API int calendar_reminder_add_receiver(const char *pkgname, const char *extra_data_key, const char *extra_data_value)
{
	int ret = CALENDAR_ERROR_NONE;
    pims_ipc_data_h indata = NULL;
    pims_ipc_data_h outdata = NULL;

	retvm_if(NULL == pkgname, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");

	// make data
    indata = pims_ipc_data_create(0);
    if (indata == NULL)
    {
        ERR("ipc data created fail !");
        ret = CALENDAR_ERROR_OUT_OF_MEMORY;
        return ret;
    }
    ret = _cal_ipc_marshal_char(pkgname, indata);
    if (ret != CALENDAR_ERROR_NONE)
    {
        ERR("marshal fail");
        CAL_IPC_DATA_FREE(indata);
        return ret;
    }
    ret = _cal_ipc_marshal_char(extra_data_key, indata);
    if (ret != CALENDAR_ERROR_NONE)
    {
        ERR("marshal fail");
        CAL_IPC_DATA_FREE(indata);
        return ret;
    }
    ret = _cal_ipc_marshal_char(extra_data_value, indata);
    if (ret != CALENDAR_ERROR_NONE)
    {
        ERR("marshal fail");
        CAL_IPC_DATA_FREE(indata);
        return ret;
    }

	// ipc call
    if (_cal_client_ipc_call(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_REGISTER_REMINDER, indata, &outdata) != 0)
    {
        ERR("pims_ipc_call failed");
        CAL_IPC_DATA_FREE(indata);
        return CALENDAR_ERROR_IPC;
    }
    CAL_IPC_DATA_FREE(indata);

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
        return CALENDAR_ERROR_IPC;
    }

    return ret;
}

API int calendar_reminder_remove_receiver(const char *pkgname)
{
	int ret = CALENDAR_ERROR_NONE;
    pims_ipc_data_h indata = NULL;
    pims_ipc_data_h outdata = NULL;

	retvm_if(NULL == pkgname, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");

	// make data
    indata = pims_ipc_data_create(0);
    if (indata == NULL)
    {
        ERR("ipc data created fail !");
        ret = CALENDAR_ERROR_OUT_OF_MEMORY;
        return ret;
    }
    ret = _cal_ipc_marshal_char(pkgname, indata);
    if (ret != CALENDAR_ERROR_NONE)
    {
        ERR("marshal fail");
        CAL_IPC_DATA_FREE(indata);
        return ret;
    }

	// ipc call
    if (_cal_client_ipc_call(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_UNREGISTER_REMINDER, indata, &outdata) != 0)
    {
        ERR("pims_ipc_call failed");
        CAL_IPC_DATA_FREE(indata);
        return CALENDAR_ERROR_IPC;
    }
    CAL_IPC_DATA_FREE(indata);

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
        return CALENDAR_ERROR_IPC;
    }

    return ret;
}

API int calendar_reminder_activate_receiver(const char *pkgname)
{
	int ret = CALENDAR_ERROR_NONE;
    pims_ipc_data_h indata = NULL;
    pims_ipc_data_h outdata = NULL;

	retvm_if(NULL == pkgname, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");

	// make data
    indata = pims_ipc_data_create(0);
    if (indata == NULL)
    {
        ERR("ipc data created fail !");
        ret = CALENDAR_ERROR_OUT_OF_MEMORY;
        return ret;
    }
    ret = _cal_ipc_marshal_char(pkgname, indata);
    if (ret != CALENDAR_ERROR_NONE)
    {
        ERR("marshal fail");
        CAL_IPC_DATA_FREE(indata);
        return ret;
    }

	// ipc call
    if (_cal_client_ipc_call(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_ACTIVATE_REMINDER, indata, &outdata) != 0)
    {
        ERR("pims_ipc_call failed");
        CAL_IPC_DATA_FREE(indata);
        return CALENDAR_ERROR_IPC;
    }
    CAL_IPC_DATA_FREE(indata);

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
        return CALENDAR_ERROR_IPC;
    }

    return ret;
}

API int calendar_reminder_deactivate_receiver(const char *pkgname)
{
	int ret = CALENDAR_ERROR_NONE;
    pims_ipc_data_h indata = NULL;
    pims_ipc_data_h outdata = NULL;

	retvm_if(NULL == pkgname, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");

	// make data
    indata = pims_ipc_data_create(0);
    if (indata == NULL)
    {
        ERR("ipc data created fail !");
        ret = CALENDAR_ERROR_OUT_OF_MEMORY;
        return ret;
    }
    ret = _cal_ipc_marshal_char(pkgname, indata);
    if (ret != CALENDAR_ERROR_NONE)
    {
        ERR("marshal fail");
        CAL_IPC_DATA_FREE(indata);
        return ret;
    }

	// ipc call
    if (_cal_client_ipc_call(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_DEACTIVATE_REMINDER, indata, &outdata) != 0)
    {
        ERR("pims_ipc_call failed");
        CAL_IPC_DATA_FREE(indata);
        return CALENDAR_ERROR_IPC;
    }
    CAL_IPC_DATA_FREE(indata);

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
        return CALENDAR_ERROR_IPC;
    }

    return ret;
}

API int calendar_reminder_has_receiver(const char *pkgname)
{
	int ret = CALENDAR_ERROR_NONE;
    pims_ipc_data_h indata = NULL;
    pims_ipc_data_h outdata = NULL;

	retvm_if(NULL == pkgname, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");

	// make data
    indata = pims_ipc_data_create(0);
    if (indata == NULL)
    {
        ERR("ipc data created fail !");
        ret = CALENDAR_ERROR_OUT_OF_MEMORY;
        return ret;
    }
    ret = _cal_ipc_marshal_char(pkgname, indata);
    if (ret != CALENDAR_ERROR_NONE)
    {
        ERR("marshal fail");
        CAL_IPC_DATA_FREE(indata);
        return ret;
    }

	// ipc call
    if (_cal_client_ipc_call(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_HAS_REMINDER, indata, &outdata) != 0)
    {
        ERR("pims_ipc_call failed");
        CAL_IPC_DATA_FREE(indata);
        return CALENDAR_ERROR_IPC;
    }
    CAL_IPC_DATA_FREE(indata);

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
        return CALENDAR_ERROR_IPC;
    }

    return ret;
}

// for reminder callback

typedef struct {
	calendar_reminder_cb cb;
	void *user_data;
} callback_info_s;

typedef struct {
	char *view_uri;
	GSList *callbacks;
} subscribe_info_s;

static pims_ipc_h __ipc = NULL;
static GSList *__subscribe_list = NULL;

int _cal_client_reminder_create_for_subscribe(void)
{
	_cal_mutex_lock(CAL_MUTEX_PIMS_IPC_PUBSUB);
	if (!__ipc)
	{
		__ipc = pims_ipc_create_for_subscribe(CAL_IPC_SOCKET_PATH_FOR_SUBSCRIPTION);
		if (!__ipc)
		{
			ERR("pims_ipc_create_for_subscribe");
			_cal_mutex_unlock(CAL_MUTEX_PIMS_IPC_PUBSUB);
			return CALENDAR_ERROR_IPC;
		}
	}
	_cal_mutex_unlock(CAL_MUTEX_PIMS_IPC_PUBSUB);
	return CALENDAR_ERROR_NONE;
}

int _cal_client_reminder_destroy_for_subscribe(void)
{
	_cal_mutex_lock(CAL_MUTEX_PIMS_IPC_PUBSUB);

	pims_ipc_destroy_for_subscribe(__ipc);
	__ipc = NULL;

	_cal_mutex_unlock(CAL_MUTEX_PIMS_IPC_PUBSUB);
	return CALENDAR_ERROR_NONE;
}

static void __cal_client_reminder_subscribe_callback(pims_ipc_h ipc, pims_ipc_data_h data, void *user_data)
{
	unsigned int size = 0;
	const unsigned char *str = NULL;
	int len = 0;
	subscribe_info_s *info = user_data;

	if (data)
	{
		len = (int)pims_ipc_data_get(data, &size);
		if (0 == len)
		{
			ERR("pims_ipc_data_get() failed");
			return;
		}
		str = (const unsigned char *)pims_ipc_data_get(data, &size);
		if (!str)
		{
			ERR("pims_ipc_data_get() failed");
			return;
		}
	}
	if (info)
	{
		GSList *l = NULL;
		for (l = info->callbacks; l; l = l->next)
		{
			callback_info_s *cb_info = l->data;
			if (NULL == cb_info) continue;

			bundle *b = NULL;
			b = bundle_decode(str, len);
			if (b)
			{
				cb_info->cb(b, cb_info->user_data);
				bundle_free(b);
			}
		}
	}
}

API int calendar_reminder_add_cb(calendar_reminder_cb callback, void *user_data)
{
	GSList *it = NULL;
	subscribe_info_s *info = NULL;
	callback_info_s *cb_info = NULL;

	_cal_mutex_lock(CAL_MUTEX_PIMS_IPC_PUBSUB);

	for (it = __subscribe_list; it; it = it->next)
	{
		if (NULL == it->data) continue;

		info = it->data;
		if (strcmp(info->view_uri, CAL_NOTI_REMINDER_CAHNGED) == 0)
			break;
		else
			info = NULL;
	}
	if (NULL == info)
	{
		info = calloc(1, sizeof(subscribe_info_s));
		if (NULL == info)
		{
			ERR("calloc() failed");
			_cal_mutex_unlock(CAL_MUTEX_PIMS_IPC_PUBSUB);
			return CALENDAR_ERROR_OUT_OF_MEMORY;
		}
		if (pims_ipc_subscribe(__ipc, CAL_IPC_MODULE_FOR_SUBSCRIPTION, (char *)CAL_NOTI_REMINDER_CAHNGED,
					__cal_client_reminder_subscribe_callback, (void *)info) != 0)
		{
			ERR("pims_ipc_subscribe() failed");
			free(info);
			_cal_mutex_unlock(CAL_MUTEX_PIMS_IPC_PUBSUB);
			return CALENDAR_ERROR_IPC;
		}
		info->view_uri = strdup(CAL_NOTI_REMINDER_CAHNGED);
		__subscribe_list = g_slist_append(__subscribe_list, info);
	}

	cb_info = calloc(1, sizeof(callback_info_s));
	cb_info->user_data = user_data;
	cb_info->cb = callback;
	info->callbacks = g_slist_append(info->callbacks, cb_info);

	_cal_mutex_unlock(CAL_MUTEX_PIMS_IPC_PUBSUB);
	return CALENDAR_ERROR_NONE;
}

API int calendar_reminder_remove_cb(calendar_reminder_cb callback, void *user_data)
{
	GSList *it = NULL;
	subscribe_info_s *info = NULL;

	_cal_mutex_lock(CAL_MUTEX_PIMS_IPC_PUBSUB);

	for (it = __subscribe_list; it; it = it->next)
	{
		if (NULL == it->data) continue;

		info = it->data;
		if (strcmp(info->view_uri, CAL_NOTI_REMINDER_CAHNGED) == 0)
			break;
		else
			info = NULL;
	}
	if (info)
	{
		GSList *l = NULL;
		for (l = info->callbacks; l; l = l->next)
		{
			callback_info_s *cb_info = l->data;
			if (callback == cb_info->cb && user_data == cb_info->user_data)
			{
				info->callbacks = g_slist_remove(info->callbacks, cb_info);
				break;
			}
		}
		if (g_slist_length(info->callbacks) == 0)
		{
			__subscribe_list = g_slist_remove(__subscribe_list, info);
			free(info->view_uri);
			free(info);
		}
	}
	_cal_mutex_unlock(CAL_MUTEX_PIMS_IPC_PUBSUB);
	return CALENDAR_ERROR_NONE;
}


