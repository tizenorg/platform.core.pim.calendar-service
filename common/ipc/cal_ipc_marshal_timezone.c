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

static int __cal_ipc_unmarshal_timezone(pims_ipc_data_h ipc_data, calendar_record_h record);
static int __cal_ipc_marshal_timezone(const calendar_record_h record, pims_ipc_data_h ipc_data);
static int __cal_ipc_marshal_timezone_get_primary_id(const calendar_record_h record, unsigned int *property_id, int *id);

cal_ipc_marshal_record_plugin_cb_s _cal_ipc_record_timezone_plugin_cb = {
        .unmarshal_record = __cal_ipc_unmarshal_timezone,
        .marshal_record = __cal_ipc_marshal_timezone,
        .get_primary_id = __cal_ipc_marshal_timezone_get_primary_id
};

static int __cal_ipc_unmarshal_timezone(pims_ipc_data_h ipc_data, calendar_record_h record)
{
    cal_timezone_s* ptimezone = NULL;
    bool bpropertyflag = false;

    retv_if(ipc_data==NULL,CALENDAR_ERROR_NO_DATA);
    retv_if(record==NULL,CALENDAR_ERROR_NO_DATA);

    ptimezone = (cal_timezone_s*) record;

    if (ptimezone->common.properties_max_count > 0)
    {
        bpropertyflag = true;
    }
    // read only or primary/secondary key
    if (_cal_ipc_unmarshal_int(ipc_data,&ptimezone->index) != CALENDAR_ERROR_NONE)
    {
        ERR("_cal_ipc_unmarshal fail");
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptimezone->common.properties_flags[1]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&ptimezone->tz_offset_from_gmt) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptimezone->common.properties_flags[2]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&ptimezone->standard_name) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptimezone->common.properties_flags[3]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&ptimezone->std_start_month) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptimezone->common.properties_flags[4]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&ptimezone->std_start_position_of_week) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptimezone->common.properties_flags[5]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&ptimezone->std_start_day) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptimezone->common.properties_flags[6]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&ptimezone->std_start_hour) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptimezone->common.properties_flags[7]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&ptimezone->standard_bias) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptimezone->common.properties_flags[8]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&ptimezone->day_light_name) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptimezone->common.properties_flags[9]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&ptimezone->day_light_start_month) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptimezone->common.properties_flags[10]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&ptimezone->day_light_start_position_of_week) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptimezone->common.properties_flags[11]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&ptimezone->day_light_start_day) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptimezone->common.properties_flags[12]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&ptimezone->day_light_start_hour) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptimezone->common.properties_flags[13]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&ptimezone->day_light_bias) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptimezone->common.properties_flags[14]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&ptimezone->calendar_id) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }

    return CALENDAR_ERROR_NONE;
}

static int __cal_ipc_marshal_timezone(const calendar_record_h record, pims_ipc_data_h ipc_data)
{
    cal_timezone_s* ptimezone = (cal_timezone_s*) record;
    bool bpropertyflag = false;
    retv_if(ipc_data==NULL,CALENDAR_ERROR_NO_DATA);
    retv_if(ptimezone==NULL,CALENDAR_ERROR_NO_DATA);

    if (_cal_ipc_marshal_record_common(&(ptimezone->common),ipc_data) != CALENDAR_ERROR_NONE)
    {
        ERR("_cal_ipc_marshal fail");
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }
    if (ptimezone->common.properties_max_count > 0)
    {
        bpropertyflag = true;
    }
    // read only or primary/secondary key
    if (_cal_ipc_marshal_int((ptimezone->index),ipc_data) != CALENDAR_ERROR_NONE)
    {
        ERR("_cal_ipc_marshal fail");
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptimezone->common.properties_flags[1]) )
    {
        if (_cal_ipc_marshal_int((ptimezone->tz_offset_from_gmt),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptimezone->common.properties_flags[2]) )
    {
        if (_cal_ipc_marshal_char((ptimezone->standard_name),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptimezone->common.properties_flags[3]) )
    {
        if (_cal_ipc_marshal_int((ptimezone->std_start_month),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptimezone->common.properties_flags[4]) )
    {
        if (_cal_ipc_marshal_int((ptimezone->std_start_position_of_week),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptimezone->common.properties_flags[5]) )
    {
        if (_cal_ipc_marshal_int((ptimezone->std_start_day),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptimezone->common.properties_flags[6]) )
    {
        if (_cal_ipc_marshal_int((ptimezone->std_start_hour),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptimezone->common.properties_flags[7]) )
    {
        if (_cal_ipc_marshal_int((ptimezone->standard_bias),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptimezone->common.properties_flags[8]) )
    {
        if (_cal_ipc_marshal_char((ptimezone->day_light_name),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptimezone->common.properties_flags[9]) )
    {
        if (_cal_ipc_marshal_int((ptimezone->day_light_start_month),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptimezone->common.properties_flags[10]) )
    {
        if (_cal_ipc_marshal_int((ptimezone->day_light_start_position_of_week),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptimezone->common.properties_flags[11]) )
    {
        if (_cal_ipc_marshal_int((ptimezone->day_light_start_day),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptimezone->common.properties_flags[12]) )
    {
        if (_cal_ipc_marshal_int((ptimezone->day_light_start_hour),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptimezone->common.properties_flags[13]) )
    {
        if (_cal_ipc_marshal_int((ptimezone->day_light_bias),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptimezone->common.properties_flags[14]) )
    {
        if (_cal_ipc_marshal_int((ptimezone->calendar_id),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }

    return CALENDAR_ERROR_NONE;
}

static int __cal_ipc_marshal_timezone_get_primary_id(const calendar_record_h record, unsigned int *property_id, int *id)
{
    *property_id = CAL_PROPERTY_TIMEZONE_ID;
    return calendar_record_get_int(record, *property_id, id );
}
