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
