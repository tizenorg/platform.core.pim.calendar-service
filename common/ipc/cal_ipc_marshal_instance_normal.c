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

static int __cal_ipc_unmarshal_instance_normal(pims_ipc_data_h ipc_data, calendar_record_h record);
static int __cal_ipc_marshal_instance_normal(const calendar_record_h record, pims_ipc_data_h ipc_data);

cal_ipc_marshal_record_plugin_cb_s _cal_ipc_record_instance_normal_plugin_cb = {
        .unmarshal_record = __cal_ipc_unmarshal_instance_normal,
        .marshal_record = __cal_ipc_marshal_instance_normal,
        .get_primary_id = NULL
};

static int __cal_ipc_unmarshal_instance_normal(pims_ipc_data_h ipc_data, calendar_record_h record)
{
    bool bpropertyflag = false;

    cal_instance_normal_s* pinstancenormal = NULL;
    retv_if(ipc_data==NULL,CALENDAR_ERROR_NO_DATA);
    retv_if(record==NULL,CALENDAR_ERROR_NO_DATA);

    pinstancenormal = (cal_instance_normal_s*) record;

    if (pinstancenormal->common.properties_max_count > 0)
    {
        bpropertyflag = true;
    }

    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pinstancenormal->common.properties_flags[0]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&pinstancenormal->event_id) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pinstancenormal->common.properties_flags[5]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&pinstancenormal->calendar_id) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pinstancenormal->common.properties_flags[1]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&pinstancenormal->dtstart_type) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
        if (_cal_ipc_unmarshal_lli(ipc_data,&pinstancenormal->dtstart_utime) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pinstancenormal->common.properties_flags[2]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&pinstancenormal->dtend_type) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
        if (_cal_ipc_unmarshal_lli(ipc_data,&pinstancenormal->dtend_utime) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pinstancenormal->common.properties_flags[3]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&pinstancenormal->summary) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pinstancenormal->common.properties_flags[6]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&pinstancenormal->description) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pinstancenormal->common.properties_flags[4]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&pinstancenormal->location) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pinstancenormal->common.properties_flags[7]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&pinstancenormal->busy_status) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pinstancenormal->common.properties_flags[8]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&pinstancenormal->event_status) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pinstancenormal->common.properties_flags[9]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&pinstancenormal->priority) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pinstancenormal->common.properties_flags[10]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&pinstancenormal->sensitivity) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pinstancenormal->common.properties_flags[11]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&pinstancenormal->has_rrule) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pinstancenormal->common.properties_flags[12]) )
    {
        if (_cal_ipc_unmarshal_double(ipc_data,&pinstancenormal->latitude) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pinstancenormal->common.properties_flags[13]) )
    {
        if (_cal_ipc_unmarshal_double(ipc_data,&pinstancenormal->longitude) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pinstancenormal->common.properties_flags[14]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&pinstancenormal->has_alarm) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pinstancenormal->common.properties_flags[15]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&pinstancenormal->original_event_id) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pinstancenormal->common.properties_flags[16]) )
    {
        if (_cal_ipc_unmarshal_lli(ipc_data,&pinstancenormal->last_mod) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }

    return CALENDAR_ERROR_NONE;
}

static int __cal_ipc_marshal_instance_normal(const calendar_record_h record, pims_ipc_data_h ipc_data)
{
    cal_instance_normal_s* pinstancenormal = (cal_instance_normal_s*) record;
    bool bpropertyflag = false;

    retv_if(ipc_data==NULL,CALENDAR_ERROR_NO_DATA);
    retv_if(pinstancenormal==NULL,CALENDAR_ERROR_NO_DATA);

    if (_cal_ipc_marshal_record_common(&(pinstancenormal->common),ipc_data) != CALENDAR_ERROR_NONE)
    {
        ERR("_cal_ipc_marshal fail");
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }
    if (pinstancenormal->common.properties_max_count > 0)
    {
        bpropertyflag = true;
    }

    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pinstancenormal->common.properties_flags[0]) )
    {
        if (_cal_ipc_marshal_int((pinstancenormal->event_id),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pinstancenormal->common.properties_flags[5]) )
    {
        if (_cal_ipc_marshal_int((pinstancenormal->calendar_id),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pinstancenormal->common.properties_flags[1]) )
    {
        if (_cal_ipc_marshal_int((pinstancenormal->dtstart_type),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
        if (_cal_ipc_marshal_lli((pinstancenormal->dtstart_utime),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pinstancenormal->common.properties_flags[2]) )
    {
        if (_cal_ipc_marshal_int((pinstancenormal->dtend_type),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
        if (_cal_ipc_marshal_lli((pinstancenormal->dtend_utime),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pinstancenormal->common.properties_flags[3]) )
    {
        if (_cal_ipc_marshal_char((pinstancenormal->summary),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pinstancenormal->common.properties_flags[6]) )
    {
        if (_cal_ipc_marshal_char((pinstancenormal->description),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pinstancenormal->common.properties_flags[4]) )
    {
        if (_cal_ipc_marshal_char((pinstancenormal->location),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pinstancenormal->common.properties_flags[7]) )
    {
        if (_cal_ipc_marshal_int((pinstancenormal->busy_status),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pinstancenormal->common.properties_flags[8]) )
    {
        if (_cal_ipc_marshal_int((pinstancenormal->event_status),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pinstancenormal->common.properties_flags[9]) )
    {
        if (_cal_ipc_marshal_int((pinstancenormal->priority),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pinstancenormal->common.properties_flags[10]) )
    {
        if (_cal_ipc_marshal_int((pinstancenormal->sensitivity),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pinstancenormal->common.properties_flags[11]) )
    {
        if (_cal_ipc_marshal_int((pinstancenormal->has_rrule),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pinstancenormal->common.properties_flags[12]) )
    {
        if (_cal_ipc_marshal_double((pinstancenormal->latitude),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pinstancenormal->common.properties_flags[13]) )
    {
        if (_cal_ipc_marshal_double((pinstancenormal->longitude),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pinstancenormal->common.properties_flags[14]) )
    {
        if (_cal_ipc_marshal_int((pinstancenormal->has_alarm),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pinstancenormal->common.properties_flags[15]) )
    {
        if (_cal_ipc_marshal_int((pinstancenormal->original_event_id),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pinstancenormal->common.properties_flags[16]) )
    {
        if (_cal_ipc_marshal_lli((pinstancenormal->last_mod),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }

    return CALENDAR_ERROR_NONE;
}