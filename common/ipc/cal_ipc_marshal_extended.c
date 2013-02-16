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

#include "cal_internal.h"
#include "cal_ipc_marshal.h"
#include "cal_view.h"

static int __cal_ipc_unmarshal_extended(pims_ipc_data_h ipc_data, calendar_record_h record);
static int __cal_ipc_marshal_extended(const calendar_record_h record, pims_ipc_data_h ipc_data);
static int __cal_ipc_marshal_extended_get_primary_id(const calendar_record_h record, unsigned int *property_id, int *id);

cal_ipc_marshal_record_plugin_cb_s _cal_ipc_record_extended_plugin_cb = {
        .unmarshal_record = __cal_ipc_unmarshal_extended,
        .marshal_record = __cal_ipc_marshal_extended,
        .get_primary_id = __cal_ipc_marshal_extended_get_primary_id
};

static int __cal_ipc_unmarshal_extended(pims_ipc_data_h ipc_data, calendar_record_h record)
{
    cal_extended_s* pextended = NULL;
    bool bpropertyflag = false;

    retv_if(ipc_data==NULL,CALENDAR_ERROR_NO_DATA);
    retv_if(record==NULL,CALENDAR_ERROR_NO_DATA);

    pextended = (cal_extended_s*) record;

    if (pextended->common.properties_max_count > 0)
    {
        bpropertyflag = true;
    }
    // read only or primary/secondary key
    if (_cal_ipc_unmarshal_int(ipc_data,&pextended->id) != CALENDAR_ERROR_NONE)
    {
        ERR("_cal_ipc_unmarshal fail");
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pextended->common.properties_flags[1]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&pextended->record_id) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pextended->common.properties_flags[2]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&pextended->record_type) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pextended->common.properties_flags[3]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&pextended->key) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pextended->common.properties_flags[4]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&pextended->value) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }

    return CALENDAR_ERROR_NONE;
}

static int __cal_ipc_marshal_extended(const calendar_record_h record, pims_ipc_data_h ipc_data)
{
    cal_extended_s* pextended = (cal_extended_s*) record;
    bool bpropertyflag = false;
    retv_if(ipc_data==NULL,CALENDAR_ERROR_NO_DATA);
    retv_if(pextended==NULL,CALENDAR_ERROR_NO_DATA);

    if (_cal_ipc_marshal_record_common(&(pextended->common),ipc_data) != CALENDAR_ERROR_NONE)
    {
        ERR("_cal_ipc_marshal fail");
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }
    if (pextended->common.properties_max_count > 0)
    {
        bpropertyflag = true;
    }
    // read only or primary/secondary key
    if (_cal_ipc_marshal_int((pextended->id),ipc_data) != CALENDAR_ERROR_NONE)
    {
        ERR("_cal_ipc_marshal fail");
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pextended->common.properties_flags[1]) )
    {
        if (_cal_ipc_marshal_int((pextended->record_id),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pextended->common.properties_flags[2]) )
    {
        if (_cal_ipc_marshal_int((pextended->record_type),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pextended->common.properties_flags[3]) )
    {
        if (_cal_ipc_marshal_char((pextended->key),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pextended->common.properties_flags[4]) )
    {
        if (_cal_ipc_marshal_char((pextended->value),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }

    return CALENDAR_ERROR_NONE;
}

static int __cal_ipc_marshal_extended_get_primary_id(const calendar_record_h record, unsigned int *property_id, int *id)
{
    *property_id = CAL_PROPERTY_EXTENDED_ID;
    return calendar_record_get_int(record, *property_id, id );
}
