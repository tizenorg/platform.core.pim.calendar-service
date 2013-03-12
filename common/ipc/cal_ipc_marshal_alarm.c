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

static int __cal_ipc_unmarshal_alarm(const pims_ipc_data_h ipc_data, calendar_record_h record);
static int __cal_ipc_marshal_alarm(const calendar_record_h record, pims_ipc_data_h ipc_data);

cal_ipc_marshal_record_plugin_cb_s _cal_ipc_record_alarm_plugin_cb = {
        .unmarshal_record = __cal_ipc_unmarshal_alarm,
        .marshal_record = __cal_ipc_marshal_alarm,
        .get_primary_id = NULL
};

static int __cal_ipc_unmarshal_alarm(pims_ipc_data_h ipc_data, calendar_record_h record)
{
    cal_alarm_s* palarm = NULL;
    bool bpropertyflag = false;

    retv_if(ipc_data==NULL,CALENDAR_ERROR_INVALID_PARAMETER);
    retv_if(record==NULL,CALENDAR_ERROR_INVALID_PARAMETER);

    palarm = (cal_alarm_s*) record;

    if (palarm->common.properties_max_count > 0)
    {
        bpropertyflag = true;
    }

    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(palarm->common.properties_flags[6]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&palarm->alarm_id) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }

    // read only or primary/secondary key
    if (_cal_ipc_unmarshal_int(ipc_data,&palarm->event_id) != CALENDAR_ERROR_NONE)
    {
        ERR("_cal_ipc_unmarshal fail");
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(palarm->common.properties_flags[0]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,(int*)&palarm->alarm_type) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }

    if (_cal_ipc_unmarshal_int(ipc_data,&palarm->is_deleted) != CALENDAR_ERROR_NONE)
    {
        ERR("_cal_ipc_unmarshal fail");
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(palarm->common.properties_flags[1]) )
    {
        if (_cal_ipc_unmarshal_lli(ipc_data,&palarm->alarm_time) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(palarm->common.properties_flags[2]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&palarm->remind_tick) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(palarm->common.properties_flags[3]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,(int*)&palarm->remind_tick_unit) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(palarm->common.properties_flags[4]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&palarm->alarm_tone) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(palarm->common.properties_flags[5]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&palarm->alarm_description) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }

    return CALENDAR_ERROR_NONE;
}

static int __cal_ipc_marshal_alarm(const calendar_record_h record, pims_ipc_data_h ipc_data)
{
    cal_alarm_s* palarm = (cal_alarm_s*) record;
    bool bpropertyflag = false;
    retv_if(ipc_data==NULL,CALENDAR_ERROR_INVALID_PARAMETER);
    retv_if(palarm==NULL,CALENDAR_ERROR_INVALID_PARAMETER);

    if (_cal_ipc_marshal_record_common(&(palarm->common),ipc_data) != CALENDAR_ERROR_NONE)
    {
        ERR("_cal_ipc_unmarshal fail");
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }
    if (palarm->common.properties_max_count > 0)
    {
        bpropertyflag = true;
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(palarm->common.properties_flags[6]) )
    {
        if (_cal_ipc_marshal_int((palarm->alarm_id),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    // read only or primary/secondary key
    if (_cal_ipc_marshal_int((palarm->event_id),ipc_data) != CALENDAR_ERROR_NONE)
    {
        ERR("_cal_ipc_unmarshal fail");
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(palarm->common.properties_flags[0]) )
    {
        if (_cal_ipc_marshal_int((palarm->alarm_type),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (_cal_ipc_marshal_int((palarm->is_deleted),ipc_data) != CALENDAR_ERROR_NONE)
    {
        ERR("_cal_ipc_unmarshal fail");
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(palarm->common.properties_flags[1]) )
    {
        if (_cal_ipc_marshal_lli((palarm->alarm_time),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(palarm->common.properties_flags[2]) )
    {
        if (_cal_ipc_marshal_int((palarm->remind_tick),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(palarm->common.properties_flags[3]) )
    {
        if (_cal_ipc_marshal_int((palarm->remind_tick_unit),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(palarm->common.properties_flags[4]) )
    {
        if (_cal_ipc_marshal_char((palarm->alarm_tone),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(palarm->common.properties_flags[5]) )
    {
        if (_cal_ipc_marshal_char((palarm->alarm_description),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }

    return CALENDAR_ERROR_NONE;
}
