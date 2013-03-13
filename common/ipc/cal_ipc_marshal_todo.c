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

static int __cal_ipc_unmarshal_todo(pims_ipc_data_h ipc_data, calendar_record_h record);
static int __cal_ipc_marshal_todo(const calendar_record_h record, pims_ipc_data_h ipc_data);
static int __cal_ipc_marshal_todo_get_primary_id(const calendar_record_h record, unsigned int *property_id, int *id);

cal_ipc_marshal_record_plugin_cb_s _cal_ipc_record_todo_plugin_cb = {
        .unmarshal_record = __cal_ipc_unmarshal_todo,
        .marshal_record = __cal_ipc_marshal_todo,
        .get_primary_id = __cal_ipc_marshal_todo_get_primary_id
};

static int __cal_ipc_unmarshal_todo(pims_ipc_data_h ipc_data, calendar_record_h record)
{
    cal_todo_s* ptodo = NULL;
    bool bpropertyflag = false;
    retv_if(ipc_data==NULL,CALENDAR_ERROR_NO_DATA);
    retv_if(record==NULL,CALENDAR_ERROR_NO_DATA);

    ptodo = (cal_todo_s*) record;

    if (ptodo->common.properties_max_count > 0)
    {
        bpropertyflag = true;
    }
    // read only or primary/secondary key
    if (_cal_ipc_unmarshal_int(ipc_data,&ptodo->index) != CALENDAR_ERROR_NONE)
    {
        ERR("_cal_ipc_unmarshal fail");
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[1]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&ptodo->calendar_id) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[2]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&ptodo->summary) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[3]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&ptodo->description) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[4]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&ptodo->location) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[5]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&ptodo->categories) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[6]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,(int*)&ptodo->todo_status) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[7]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,(int*)&ptodo->priority) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[8]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&ptodo->sensitivity) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[9]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&ptodo->uid) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[10]) )
    {
        if (_cal_ipc_unmarshal_double(ipc_data,&ptodo->latitude) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[11]) )
    {
        if (_cal_ipc_unmarshal_double(ipc_data,&ptodo->longitude) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[13]) )
    {
        if (_cal_ipc_unmarshal_lli(ipc_data,&ptodo->created_time) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[15]) )
    {
        if (_cal_ipc_unmarshal_lli(ipc_data,&ptodo->completed_time) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[12]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&ptodo->progress) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[16]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&ptodo->is_deleted) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[14]) )
    {
        if (_cal_ipc_unmarshal_lli(ipc_data,&ptodo->last_mod) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[17]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&ptodo->freq) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[18]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&ptodo->range_type) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[19]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&ptodo->until_type) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
        if (_cal_ipc_unmarshal_lli(ipc_data,&ptodo->until_utime) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
        if (_cal_ipc_unmarshal_int(ipc_data,&ptodo->until_year) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
        if (_cal_ipc_unmarshal_int(ipc_data,&ptodo->until_month) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
        if (_cal_ipc_unmarshal_int(ipc_data,&ptodo->until_mday) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[20]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&ptodo->count) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[21]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&ptodo->interval) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[22]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&ptodo->bysecond) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[23]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&ptodo->byminute) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[24]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&ptodo->byhour) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[25]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&ptodo->byday) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[26]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&ptodo->bymonthday) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[27]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&ptodo->byyearday) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[28]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&ptodo->byweekno) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[29]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&ptodo->bymonth) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[30]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&ptodo->bysetpos) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[31]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&ptodo->wkst) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[32]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&ptodo->has_alarm) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }

    if (_cal_ipc_unmarshal_long(ipc_data,&ptodo->updated) != CALENDAR_ERROR_NONE)
    {
        ERR("_cal_ipc_unmarshal fail");
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[33]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&ptodo->sync_data1) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[34]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&ptodo->sync_data2) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[35]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&ptodo->sync_data3) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[36]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&ptodo->sync_data4) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[37]) )
    {
        if (_cal_ipc_unmarshal_caltime(ipc_data,&ptodo->start) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[40]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&ptodo->start_tzid) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[38]) )
    {
        if (_cal_ipc_unmarshal_caltime(ipc_data,&ptodo->due) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[41]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&ptodo->due_tzid) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[42]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&ptodo->organizer_name) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[43]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&ptodo->organizer_email) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[44]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&ptodo->has_attendee) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }

    // !! alarm_list
    int count = 0, i = 0;
    if (_cal_ipc_unmarshal_int(ipc_data,&count) != CALENDAR_ERROR_NONE)
    {
        ERR("_cal_ipc_unmarshal fail");
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    for(i=0;i<count;i++)
    {
        calendar_record_h alarm_child_record = NULL;

        if (_cal_ipc_unmarshal_record(ipc_data, &alarm_child_record) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
        ptodo->alarm_list = g_list_append(ptodo->alarm_list, alarm_child_record);
    }

    // attendee_list
    if (_cal_ipc_unmarshal_int(ipc_data,&count) != CALENDAR_ERROR_NONE)
    {
        ERR("_cal_ipc_unmarshal fail");
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    for(i=0;i<count;i++)
    {
        calendar_record_h attendee_child_record = NULL;
        if (_cal_ipc_unmarshal_record(ipc_data, &attendee_child_record) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
        ptodo->attendee_list = g_list_append(ptodo->attendee_list, attendee_child_record);
    }

    //extended
    if (_cal_ipc_unmarshal_int(ipc_data,&count) != CALENDAR_ERROR_NONE)
    {
        ERR("_cal_ipc_unmarshal fail");
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    for(i=0;i<count;i++)
    {
        calendar_record_h child_record = NULL;
        if (_cal_ipc_unmarshal_record(ipc_data, &child_record) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
        ptodo->extended_list = g_list_append(ptodo->extended_list, child_record);
    }

    return CALENDAR_ERROR_NONE;
}

static int __cal_ipc_marshal_todo(const calendar_record_h record, pims_ipc_data_h ipc_data)
{
    cal_todo_s* ptodo = (cal_todo_s*)record;
    bool bpropertyflag = false;
    retv_if(ipc_data==NULL,CALENDAR_ERROR_NO_DATA);
    retv_if(ptodo==NULL,CALENDAR_ERROR_NO_DATA);

    if (_cal_ipc_marshal_record_common(&(ptodo->common),ipc_data) != CALENDAR_ERROR_NONE)
    {
        ERR("_cal_ipc_marshal fail");
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }
    if (ptodo->common.properties_max_count > 0)
    {
        bpropertyflag = true;
    }
    // read only or primary/secondary key
    if (_cal_ipc_marshal_int((ptodo->index),ipc_data) != CALENDAR_ERROR_NONE)
    {
        ERR("_cal_ipc_marshal fail");
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[1]) )
    {
        if (_cal_ipc_marshal_int((ptodo->calendar_id),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[2]) )
    {
        if (_cal_ipc_marshal_char((ptodo->summary),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[3]) )
    {
        if (_cal_ipc_marshal_char((ptodo->description),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[4]) )
    {
        if (_cal_ipc_marshal_char((ptodo->location),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[5]) )
    {
        if (_cal_ipc_marshal_char((ptodo->categories),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[6]) )
    {
        if (_cal_ipc_marshal_int((ptodo->todo_status),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[7]) )
    {
        if (_cal_ipc_marshal_int((ptodo->priority),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[8]) )
    {
        if (_cal_ipc_marshal_int((ptodo->sensitivity),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[9]) )
    {
        if (_cal_ipc_marshal_char((ptodo->uid),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[10]) )
    {
        if (_cal_ipc_marshal_double((ptodo->latitude),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[11]) )
    {
        if (_cal_ipc_marshal_double((ptodo->longitude),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[13]) )
    {
        if (_cal_ipc_marshal_lli((ptodo->created_time),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[15]) )
    {
        if (_cal_ipc_marshal_lli((ptodo->completed_time),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[12]) )
    {
        if (_cal_ipc_marshal_int((ptodo->progress),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[16]) )
    {
        if (_cal_ipc_marshal_int((ptodo->is_deleted),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[14]) )
    {
        if (_cal_ipc_marshal_lli((ptodo->last_mod),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[17]) )
    {
        if (_cal_ipc_marshal_int((ptodo->freq),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[18]) )
    {
        if (_cal_ipc_marshal_int((ptodo->range_type),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[19]) )
    {
        if (_cal_ipc_marshal_int((ptodo->until_type),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
        if (_cal_ipc_marshal_lli((ptodo->until_utime),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
        if (_cal_ipc_marshal_int((ptodo->until_year),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
        if (_cal_ipc_marshal_int((ptodo->until_month),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
        if (_cal_ipc_marshal_int((ptodo->until_mday),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[20]) )
    {
        if (_cal_ipc_marshal_int((ptodo->count),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[21]) )
    {
        if (_cal_ipc_marshal_int((ptodo->interval),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[22]) )
    {
        if (_cal_ipc_marshal_char((ptodo->bysecond),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[23]) )
    {
        if (_cal_ipc_marshal_char((ptodo->byminute),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[24]) )
    {
        if (_cal_ipc_marshal_char((ptodo->byhour),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[25]) )
    {
        if (_cal_ipc_marshal_char((ptodo->byday),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[26]) )
    {
        if (_cal_ipc_marshal_char((ptodo->bymonthday),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[27]) )
    {
        if (_cal_ipc_marshal_char((ptodo->byyearday),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[28]) )
    {
        if (_cal_ipc_marshal_char((ptodo->byweekno),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[29]) )
    {
        if (_cal_ipc_marshal_char((ptodo->bymonth),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[30]) )
    {
        if (_cal_ipc_marshal_char((ptodo->bysetpos),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[31]) )
    {
        if (_cal_ipc_marshal_int((ptodo->wkst),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[32]) )
    {
        if (_cal_ipc_marshal_int((ptodo->has_alarm),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }

    if (_cal_ipc_marshal_long((ptodo->updated),ipc_data) != CALENDAR_ERROR_NONE)
    {
        ERR("_cal_ipc_marshal fail");
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[33]) )
    {
        if (_cal_ipc_marshal_char((ptodo->sync_data1),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[34]) )
    {
        if (_cal_ipc_marshal_char((ptodo->sync_data2),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[35]) )
    {
        if (_cal_ipc_marshal_char((ptodo->sync_data3),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[36]) )
    {
        if (_cal_ipc_marshal_char((ptodo->sync_data4),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[37]) )
    {
        if (_cal_ipc_marshal_caltime((ptodo->start),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[40]) )
    {
        if (_cal_ipc_marshal_char((ptodo->start_tzid),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[38]) )
    {
        if (_cal_ipc_marshal_caltime((ptodo->due),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[41]) )
    {
        if (_cal_ipc_marshal_char((ptodo->due_tzid),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[42]) )
    {
        if (_cal_ipc_marshal_char((ptodo->organizer_name),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[43]) )
    {
        if (_cal_ipc_marshal_char((ptodo->organizer_email),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(ptodo->common.properties_flags[44]) )
    {
        if (_cal_ipc_marshal_int((ptodo->has_attendee),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    // !! alarm_list
    if (ptodo->alarm_list)
    {
        int count = g_list_length(ptodo->alarm_list);
        GList *cursor = g_list_first(ptodo->alarm_list);
        calendar_record_h alarm_child_record;

        if (_cal_ipc_marshal_int(count,ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
        while (cursor)
        {
            alarm_child_record = (calendar_record_h)cursor->data;
            if (alarm_child_record == NULL)
            {
                cursor = g_list_next(cursor);
                continue;
            }
            if (_cal_ipc_marshal_record(alarm_child_record, ipc_data) != CALENDAR_ERROR_NONE)
            {
                ERR("_cal_ipc_marshal fail");
                return CALENDAR_ERROR_INVALID_PARAMETER;
            }
            cursor = g_list_next(cursor);
        }
    }
    else
    {
        if (_cal_ipc_marshal_int(0,ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }

    if (ptodo->attendee_list)
    {
        int count = g_list_length(ptodo->attendee_list);
        GList *cursor = g_list_first(ptodo->attendee_list);
        calendar_record_h attendee_child_record;

        if (_cal_ipc_marshal_int(count,ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
        while (cursor)
        {
            attendee_child_record = (calendar_record_h)cursor->data;
            if (attendee_child_record == NULL)
            {
                cursor = g_list_next(cursor);
                continue;
            }
            if (_cal_ipc_marshal_record(attendee_child_record, ipc_data) != CALENDAR_ERROR_NONE)
            {
                ERR("_cal_ipc_marshal fail");
                return CALENDAR_ERROR_INVALID_PARAMETER;
            }
            cursor = g_list_next(cursor);
        }
    }
    else
    {
        if (_cal_ipc_marshal_int(0,ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }

    if (ptodo->extended_list)
    {
        int count = g_list_length(ptodo->extended_list);
        GList *cursor = g_list_first(ptodo->extended_list);
        calendar_record_h child_record;

        if (_cal_ipc_marshal_int(count,ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
        while (cursor)
        {
            child_record = (calendar_record_h)cursor->data;
            if (child_record == NULL)
            {
                cursor = g_list_next(cursor);
                continue;
            }
            if (_cal_ipc_marshal_record(child_record, ipc_data) != CALENDAR_ERROR_NONE)
            {
                ERR("_cal_ipc_marshal fail");
                return CALENDAR_ERROR_INVALID_PARAMETER;
            }
            cursor = g_list_next(cursor);
        }
    }
    else
    {
        if (_cal_ipc_marshal_int(0,ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }

    return CALENDAR_ERROR_NONE;
}

static int __cal_ipc_marshal_todo_get_primary_id(const calendar_record_h record, unsigned int *property_id, int *id)
{
    *property_id = CAL_PROPERTY_TODO_ID;
    return calendar_record_get_int(record, *property_id, id );
}
