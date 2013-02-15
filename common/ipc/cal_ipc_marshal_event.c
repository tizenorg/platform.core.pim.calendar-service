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

static int __cal_ipc_unmarshal_event(pims_ipc_data_h ipc_data, calendar_record_h record);
static int __cal_ipc_marshal_event(const calendar_record_h record, pims_ipc_data_h ipc_data);
static int __cal_ipc_marshal_event_get_primary_id(const calendar_record_h record, unsigned int *property_id, int *id);

cal_ipc_marshal_record_plugin_cb_s _cal_ipc_record_event_plugin_cb = {
        .unmarshal_record = __cal_ipc_unmarshal_event,
        .marshal_record = __cal_ipc_marshal_event,
        .get_primary_id = __cal_ipc_marshal_event_get_primary_id
};

static int __cal_ipc_unmarshal_event(pims_ipc_data_h ipc_data, calendar_record_h record)
{
    cal_event_s *pevent = NULL;
    bool bpropertyflag = false;
    retv_if(ipc_data==NULL,CALENDAR_ERROR_NO_DATA);
    retv_if(record==NULL,CALENDAR_ERROR_NO_DATA);

    pevent = (cal_event_s*) record;

    if (pevent->common.properties_max_count > 0)
    {
        bpropertyflag = true;
    }
    // read only or primary/secondary key
    if (_cal_ipc_unmarshal_int(ipc_data,&pevent->index) != CALENDAR_ERROR_NONE)
    {
        ERR("_cal_ipc_unmarshal fail");
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[1]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&pevent->calendar_id) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[2]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&pevent->summary) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[3]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&pevent->description) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[4]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&pevent->location) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[5]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&pevent->categories) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[6]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&pevent->exdate) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[7]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,(int*)&pevent->event_status) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[8]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,(int*)&pevent->priority) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[9]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&pevent->timezone) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[10]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&pevent->contact_id) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[11]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&pevent->busy_status) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[12]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&pevent->sensitivity) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[16]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&pevent->meeting_status) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[13]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&pevent->uid) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[14]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&pevent->organizer_name) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[15]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&pevent->organizer_email) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    // read only or primary/secondary key
    if (_cal_ipc_unmarshal_int(ipc_data,&pevent->original_event_id) != CALENDAR_ERROR_NONE)
    {
        ERR("_cal_ipc_unmarshal fail");
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[18]) )
    {
        if (_cal_ipc_unmarshal_double(ipc_data,&pevent->latitude) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[19]) )
    {
        if (_cal_ipc_unmarshal_double(ipc_data,&pevent->longitude) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[20]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&pevent->email_id) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[21]) )
    {
        if (_cal_ipc_unmarshal_lli(ipc_data,&pevent->created_time) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[23]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&pevent->is_deleted) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[22]) )
    {
        if (_cal_ipc_unmarshal_lli(ipc_data,&pevent->last_mod) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[24]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&pevent->freq) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[25]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&pevent->range_type) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[26]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&pevent->until_type) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
        if (_cal_ipc_unmarshal_lli(ipc_data,&pevent->until_utime) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
        if (_cal_ipc_unmarshal_int(ipc_data,&pevent->until_year) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
        if (_cal_ipc_unmarshal_int(ipc_data,&pevent->until_month) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
        if (_cal_ipc_unmarshal_int(ipc_data,&pevent->until_mday) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[27]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&pevent->count) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[28]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&pevent->interval) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[29]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&pevent->bysecond) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[30]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&pevent->byminute) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[31]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&pevent->byhour) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[32]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&pevent->byday) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[33]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&pevent->bymonthday) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[34]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&pevent->byyearday) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[35]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&pevent->byweekno) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[36]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&pevent->bymonth) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[37]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&pevent->bysetpos) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[38]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&pevent->wkst) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[39]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&pevent->recurrence_id) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[40]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&pevent->rdate) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[41]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&pevent->has_attendee) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[42]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&pevent->has_alarm) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[51]) )
    {
        if (_cal_ipc_unmarshal_int(ipc_data,&pevent->system_type) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }

    if (_cal_ipc_unmarshal_long(ipc_data,&pevent->updated) != CALENDAR_ERROR_NONE)
    {
        ERR("_cal_ipc_unmarshal fail");
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[43]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&pevent->sync_data1) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[44]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&pevent->sync_data2) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[45]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&pevent->sync_data3) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[46]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&pevent->sync_data4) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[47]) )
    {
        if (_cal_ipc_unmarshal_caltime(ipc_data,&pevent->start) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[52]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&pevent->start_tzid) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[48]) )
    {
        if (_cal_ipc_unmarshal_caltime(ipc_data,&pevent->end) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[53]) )
    {
        if (_cal_ipc_unmarshal_char(ipc_data,&pevent->end_tzid) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }

    // !! alarm_list, attendee_list
    // alarm_list
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
        pevent->alarm_list = g_list_append(pevent->alarm_list, alarm_child_record);
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
        pevent->attendee_list = g_list_append(pevent->attendee_list, attendee_child_record);
    }

    // exception_list
    if (_cal_ipc_unmarshal_int(ipc_data,&count) != CALENDAR_ERROR_NONE)
    {
        ERR("_cal_ipc_unmarshal fail");
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    for(i=0;i<count;i++)
    {
        calendar_record_h exception_child_record = NULL;
        if (_cal_ipc_unmarshal_record(ipc_data, &exception_child_record) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
        pevent->exception_list = g_list_append(pevent->exception_list, exception_child_record);
    }

    // extended_list
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
        pevent->extended_list = g_list_append(pevent->extended_list, child_record);
    }

    return CALENDAR_ERROR_NONE;
}

static int __cal_ipc_marshal_event(const calendar_record_h record, pims_ipc_data_h ipc_data)
{
    cal_event_s* pevent = (cal_event_s*) record;
    bool bpropertyflag = false;
    retv_if(ipc_data==NULL,CALENDAR_ERROR_NO_DATA);
    retv_if(pevent==NULL,CALENDAR_ERROR_NO_DATA);

    if (_cal_ipc_marshal_record_common(&(pevent->common),ipc_data) != CALENDAR_ERROR_NONE)
    {
        ERR("_cal_ipc_marshal fail");
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }
    if (pevent->common.properties_max_count > 0)
    {
        bpropertyflag = true;
    }
    // read only or primary/secondary key
    if (_cal_ipc_marshal_int((pevent->index),ipc_data) != CALENDAR_ERROR_NONE)
    {
        ERR("_cal_ipc_marshal fail");
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[1]) )
    {
        if (_cal_ipc_marshal_int((pevent->calendar_id),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[2]) )
    {
        if (_cal_ipc_marshal_char((pevent->summary),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[3]) )
    {
        if (_cal_ipc_marshal_char((pevent->description),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[4]) )
    {
        if (_cal_ipc_marshal_char((pevent->location),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[5]) )
    {
        if (_cal_ipc_marshal_char((pevent->categories),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[6]) )
    {
        if (_cal_ipc_marshal_char((pevent->exdate),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[7]) )
    {
        if (_cal_ipc_marshal_int((pevent->event_status),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[8]) )
    {
        if (_cal_ipc_marshal_int((pevent->priority),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[9]) )
    {
        if (_cal_ipc_marshal_int((pevent->timezone),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[10]) )
    {
        if (_cal_ipc_marshal_int((pevent->contact_id),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[11]) )
    {
        if (_cal_ipc_marshal_int((pevent->busy_status),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[12]) )
    {
        if (_cal_ipc_marshal_int((pevent->sensitivity),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[16]) )
    {
        if (_cal_ipc_marshal_int((pevent->meeting_status),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[13]) )
    {
        if (_cal_ipc_marshal_char((pevent->uid),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[14]) )
    {
        if (_cal_ipc_marshal_char((pevent->organizer_name),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[15]) )
    {
        if (_cal_ipc_marshal_char((pevent->organizer_email),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    // read only or primary/secondary key
    if (_cal_ipc_marshal_int((pevent->original_event_id),ipc_data) != CALENDAR_ERROR_NONE)
    {
        ERR("_cal_ipc_marshal fail");
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[18]) )
    {
        if (_cal_ipc_marshal_double((pevent->latitude),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[19]) )
    {
        if (_cal_ipc_marshal_double((pevent->longitude),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[20]) )
    {
        if (_cal_ipc_marshal_int((pevent->email_id),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[21]) )
    {
        if (_cal_ipc_marshal_lli((pevent->created_time),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[23]) )
    {
        if (_cal_ipc_marshal_int((pevent->is_deleted),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[22]) )
    {
        if (_cal_ipc_marshal_lli((pevent->last_mod),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[24]) )
    {
        if (_cal_ipc_marshal_int((pevent->freq),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[25]) )
    {
        if (_cal_ipc_marshal_int((pevent->range_type),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[26]) )
    {
        if (_cal_ipc_marshal_int((pevent->until_type),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
        if (_cal_ipc_marshal_lli((pevent->until_utime),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
        if (_cal_ipc_marshal_int((pevent->until_year),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
        if (_cal_ipc_marshal_int((pevent->until_month),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
        if (_cal_ipc_marshal_int((pevent->until_mday),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[27]) )
    {
        if (_cal_ipc_marshal_int((pevent->count),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[28]) )
    {
        if (_cal_ipc_marshal_int((pevent->interval),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[29]) )
    {
        if (_cal_ipc_marshal_char((pevent->bysecond),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[30]) )
    {
        if (_cal_ipc_marshal_char((pevent->byminute),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[31]) )
    {
        if (_cal_ipc_marshal_char((pevent->byhour),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[32]) )
    {
        if (_cal_ipc_marshal_char((pevent->byday),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[33]) )
    {
        if (_cal_ipc_marshal_char((pevent->bymonthday),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[34]) )
    {
        if (_cal_ipc_marshal_char((pevent->byyearday),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[35]) )
    {
        if (_cal_ipc_marshal_char((pevent->byweekno),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[36]) )
    {
        if (_cal_ipc_marshal_char((pevent->bymonth),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[37]) )
    {
        if (_cal_ipc_marshal_char((pevent->bysetpos),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[38]) )
    {
        if (_cal_ipc_marshal_int((pevent->wkst),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[39]) )
    {
        if (_cal_ipc_marshal_char((pevent->recurrence_id),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[40]) )
    {
        if (_cal_ipc_marshal_char((pevent->rdate),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[41]) )
    {
        if (_cal_ipc_marshal_int((pevent->has_attendee),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[42]) )
    {
        if (_cal_ipc_marshal_int((pevent->has_alarm),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[51]) )
    {
        if (_cal_ipc_marshal_int((pevent->system_type),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }

    if (_cal_ipc_marshal_long((pevent->updated),ipc_data) != CALENDAR_ERROR_NONE)
    {
        ERR("_cal_ipc_marshal fail");
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[43]) )
    {
        if (_cal_ipc_marshal_char((pevent->sync_data1),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[44]) )
    {
        if (_cal_ipc_marshal_char((pevent->sync_data2),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[45]) )
    {
        if (_cal_ipc_marshal_char((pevent->sync_data3),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[46]) )
    {
        if (_cal_ipc_marshal_char((pevent->sync_data4),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[47]) )
    {
        if (_cal_ipc_marshal_caltime((pevent->start),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[52]) )
    {
        if (_cal_ipc_marshal_char((pevent->start_tzid),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[48]) )
    {
        if (_cal_ipc_marshal_caltime((pevent->end),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    if (bpropertyflag == false || CAL_IPC_CHECK_PROPERTIES_FLAG(pevent->common.properties_flags[53]) )
    {
        if (_cal_ipc_marshal_char((pevent->end_tzid),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    // !! alarm_list, attendee_list
    // alarm_list
    if (pevent->alarm_list)
    {
        int count = g_list_length(pevent->alarm_list);
        GList *cursor = g_list_first(pevent->alarm_list);
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

    if (pevent->attendee_list)
    {
        int count = g_list_length(pevent->attendee_list);
        GList *cursor = g_list_first(pevent->attendee_list);
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

    if (pevent->exception_list)
    {
        int count = g_list_length(pevent->exception_list);
        GList *cursor = g_list_first(pevent->exception_list);
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

    if (pevent->extended_list)
    {
        int count = g_list_length(pevent->extended_list);
        GList *cursor = g_list_first(pevent->extended_list);
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

static int __cal_ipc_marshal_event_get_primary_id(const calendar_record_h record, unsigned int *property_id, int *id)
{
    *property_id = CAL_PROPERTY_EVENT_ID;
    return calendar_record_get_int(record, *property_id, id );
}
