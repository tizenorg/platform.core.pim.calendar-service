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
#include <stdbool.h>        //bool
#include <string.h>

#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_view.h"

#include "cal_record.h"

static int __cal_record_instance_allday_create( calendar_record_h* out_record );
static int __cal_record_instance_allday_destroy( calendar_record_h record, bool delete_child );
static int __cal_record_instance_allday_clone( calendar_record_h record, calendar_record_h* out_record );
static int __cal_record_instance_allday_get_str( calendar_record_h record, unsigned int property_id, char** out_str );
static int __cal_record_instance_allday_get_str_p( calendar_record_h record, unsigned int property_id, char** out_str );
static int __cal_record_instance_allday_get_int( calendar_record_h record, unsigned int property_id, int* out_value );
static int __cal_record_instance_allday_get_double( calendar_record_h record, unsigned int property_id, double* out_value );
static int __cal_record_instance_allday_get_lli( calendar_record_h record, unsigned int property_id, long long int* out_value );
static int __cal_record_instance_allday_get_caltime( calendar_record_h record, unsigned int property_id, calendar_time_s* out_value );
static int __cal_record_instance_allday_set_str( calendar_record_h record, unsigned int property_id, const char* value );
static int __cal_record_instance_allday_set_int( calendar_record_h record, unsigned int property_id, int value );
static int __cal_record_instance_allday_set_double( calendar_record_h record, unsigned int property_id, double value );
static int __cal_record_instance_allday_set_lli( calendar_record_h record, unsigned int property_id, long long int value );
static int __cal_record_instance_allday_set_caltime( calendar_record_h record, unsigned int property_id, calendar_time_s value );

cal_record_plugin_cb_s _cal_record_instance_allday_plugin_cb = {
    .create = __cal_record_instance_allday_create,
    .destroy = __cal_record_instance_allday_destroy,
    .clone = __cal_record_instance_allday_clone,
    .get_str = __cal_record_instance_allday_get_str,
    .get_str_p = __cal_record_instance_allday_get_str_p,
    .get_int = __cal_record_instance_allday_get_int,
    .get_double = __cal_record_instance_allday_get_double,
    .get_lli = __cal_record_instance_allday_get_lli,
    .get_caltime = __cal_record_instance_allday_get_caltime,
    .set_str = __cal_record_instance_allday_set_str,
    .set_int = __cal_record_instance_allday_set_int,
    .set_double = __cal_record_instance_allday_set_double,
    .set_lli = __cal_record_instance_allday_set_lli,
    .set_caltime = __cal_record_instance_allday_set_caltime,
    .add_child_record = NULL,
    .remove_child_record = NULL,
    .get_child_record_count = NULL,
    .get_child_record_at_p = NULL,
    .clone_child_record_list = NULL
};

static void __cal_record_instance_allday_struct_init(cal_instance_allday_s* record)
{
    memset(record,0,sizeof(cal_instance_allday_s));

    record->event_status = CALENDAR_EVENT_STATUS_NONE;
    record->calendar_id = DEFAULT_EVENT_CALENDAR_BOOK_ID;
    record->event_id = CAL_INVALID_ID;

    record->busy_status = 2;
    record->summary = NULL;
    record->description = NULL;
    record->location= NULL;

    record->latitude = 1000; // set default 1000 out of range(-180 ~ 180)
    record->longitude = 1000; // set default 1000 out of range(-180 ~ 180)

    return;
}

static int __cal_record_instance_allday_create( calendar_record_h* out_record )
{
    cal_instance_allday_s *temp = NULL;
    int ret= CALENDAR_ERROR_NONE, type = 0;

    type = CAL_RECORD_TYPE_INSTANCE_ALLDAY;

    temp = (cal_instance_allday_s*)calloc(1,sizeof(cal_instance_allday_s));
    retvm_if(NULL == temp, CALENDAR_ERROR_OUT_OF_MEMORY, "malloc(cal_instance_allday_s:sch) Failed(%d)", CALENDAR_ERROR_OUT_OF_MEMORY);

    __cal_record_instance_allday_struct_init(temp);

    *out_record = (calendar_record_h)temp;

    return ret;
}

static void __cal_record_instance_allday_struct_free(cal_instance_allday_s *record)
{
    CAL_FREE(record->summary);
    CAL_FREE(record->description);
    CAL_FREE(record->location);

    CAL_FREE(record);
}

static int __cal_record_instance_allday_destroy( calendar_record_h record, bool delete_child )
{
    int ret = CALENDAR_ERROR_NONE;

    cal_instance_allday_s *temp = (cal_instance_allday_s*)(record);

    __cal_record_instance_allday_struct_free(temp);

    return ret;
}

static int __cal_record_instance_allday_clone( calendar_record_h record, calendar_record_h* out_record )
{
    cal_instance_allday_s *out_data = NULL;
    cal_instance_allday_s *src_data = NULL;

    src_data = (cal_instance_allday_s*)(record);

    out_data = calloc(1, sizeof(cal_instance_allday_s));
    retvm_if(NULL == out_data, CALENDAR_ERROR_OUT_OF_MEMORY, "calloc(cal_instance_allday_s) Failed(%d)", CALENDAR_ERROR_OUT_OF_MEMORY);

    CAL_RECORD_COPY_COMMON(&(out_data->common), &(src_data->common));

    out_data->event_id = src_data->event_id;
    out_data->calendar_id = src_data->calendar_id;
    out_data->dtstart_type = src_data->dtstart_type;
    out_data->dtstart_year = src_data->dtstart_year;
    out_data->dtstart_month = src_data->dtstart_month;
    out_data->dtstart_mday = src_data->dtstart_mday;
    out_data->dtend_type = src_data->dtend_type;
    out_data->dtend_year = src_data->dtend_year;
    out_data->dtend_month = src_data->dtend_month;
    out_data->dtend_mday = src_data->dtend_mday;
    out_data->summary = SAFE_STRDUP(src_data->summary);
    out_data->description = SAFE_STRDUP(src_data->description);
    out_data->location = SAFE_STRDUP(src_data->location);
    out_data->busy_status = src_data->busy_status;
    out_data->event_status = src_data->event_status;
    out_data->priority = src_data->priority;
    out_data->sensitivity = src_data->sensitivity;
    out_data->has_rrule = src_data->has_rrule;
    out_data->latitude = src_data->latitude;
    out_data->longitude = src_data->longitude;
    out_data->has_alarm = src_data->has_alarm;
    out_data->original_event_id = src_data->original_event_id;
    out_data->last_mod = src_data->last_mod;

    *out_record = (calendar_record_h)out_data;

    return CALENDAR_ERROR_NONE;
}

static int __cal_record_instance_allday_get_str( calendar_record_h record, unsigned int property_id, char** out_str )
{
    cal_instance_allday_s *rec = (cal_instance_allday_s*)(record);
    switch( property_id )
    {
    case CAL_PROPERTY_INSTANCE_ALLDAY_SUMMARY:
        *out_str = SAFE_STRDUP(rec->summary);
        break;
    case CAL_PROPERTY_INSTANCE_ALLDAY_DESCRIPTION:
        *out_str = SAFE_STRDUP(rec->description);
        break;
    case CAL_PROPERTY_INSTANCE_ALLDAY_LOCATION:
        *out_str = SAFE_STRDUP(rec->location);
        break;
    default:
        ASSERT_NOT_REACHED("invalid parameter (property:%d)",property_id);
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    return CALENDAR_ERROR_NONE;
}

static int __cal_record_instance_allday_get_str_p( calendar_record_h record, unsigned int property_id, char** out_str )
{
    cal_instance_allday_s *rec = (cal_instance_allday_s*)(record);
    switch( property_id )
    {
    case CAL_PROPERTY_INSTANCE_ALLDAY_SUMMARY:
        *out_str = (rec->summary);
        break;
    case CAL_PROPERTY_INSTANCE_ALLDAY_DESCRIPTION:
        *out_str = (rec->description);
        break;
    case CAL_PROPERTY_INSTANCE_ALLDAY_LOCATION:
        *out_str = (rec->location);
        break;
    default:
        ASSERT_NOT_REACHED("invalid parameter (property:%d)",property_id);
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    return CALENDAR_ERROR_NONE;
}

static int __cal_record_instance_allday_get_int( calendar_record_h record, unsigned int property_id, int* out_value )
{
    cal_instance_allday_s *rec = (cal_instance_allday_s*)(record);
    switch( property_id )
    {
    case CAL_PROPERTY_INSTANCE_ALLDAY_EVENT_ID:
        *out_value = (rec->event_id);
        break;
    case CAL_PROPERTY_INSTANCE_ALLDAY_CALENDAR_ID:
        *out_value = (rec->calendar_id);
        break;
    case CAL_PROPERTY_INSTANCE_ALLDAY_BUSY_STATUS:
        *out_value = (rec->busy_status);
        break;
    case CAL_PROPERTY_INSTANCE_ALLDAY_EVENT_STATUS:
        *out_value = (rec->event_status);
        break;
    case CAL_PROPERTY_INSTANCE_ALLDAY_PRIORITY:
        *out_value = (rec->priority);
        break;
    case CAL_PROPERTY_INSTANCE_ALLDAY_SENSITIVITY:
        *out_value = (rec->sensitivity);
        break;
    case CAL_PROPERTY_INSTANCE_ALLDAY_HAS_RRULE:
        *out_value = (rec->has_rrule);
        break;
    case CAL_PROPERTY_INSTANCE_ALLDAY_HAS_ALARM:
        *out_value = (rec->has_alarm);
        break;
    case CAL_PROPERTY_INSTANCE_ALLDAY_ORIGINAL_EVENT_ID:
        *out_value = (rec->original_event_id);
        break;
    default:
        ASSERT_NOT_REACHED("invalid parameter (property:%d)",property_id);
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    return CALENDAR_ERROR_NONE;
}

static int __cal_record_instance_allday_get_double( calendar_record_h record, unsigned int property_id, double* out_value )
{
    cal_instance_allday_s *rec = (cal_instance_allday_s*)(record);
    switch( property_id )
    {
    case CAL_PROPERTY_INSTANCE_ALLDAY_LATITUDE:
        *out_value = (rec->latitude);
        break;
    case CAL_PROPERTY_INSTANCE_ALLDAY_LONGITUDE:
        *out_value = (rec->longitude);
        break;

    default:
        ASSERT_NOT_REACHED("invalid parameter (property:%d)",property_id);
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    return CALENDAR_ERROR_NONE;
}

static int __cal_record_instance_allday_get_lli( calendar_record_h record, unsigned int property_id, long long int* out_value )
{
    cal_instance_allday_s *rec = (cal_instance_allday_s*)(record);
    switch( property_id )
    {
    case CAL_PROPERTY_INSTANCE_ALLDAY_LAST_MODIFIED_TIME:
        *out_value = (rec->last_mod);
        break;
    default:
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    return CALENDAR_ERROR_NONE;
}

static int __cal_record_instance_allday_get_caltime( calendar_record_h record, unsigned int property_id, calendar_time_s* out_value )
{
    cal_instance_allday_s *rec = (cal_instance_allday_s*)(record);
    switch( property_id )
    {
    case CAL_PROPERTY_INSTANCE_ALLDAY_START:
        CAL_CALTIME_SET_DATE(*out_value,(rec->dtstart_year),(rec->dtstart_month),(rec->dtstart_mday));
        break;
    case CAL_PROPERTY_INSTANCE_ALLDAY_END:
        CAL_CALTIME_SET_DATE(*out_value,(rec->dtend_year),(rec->dtend_month),(rec->dtend_mday));
        break;
    default:
        ASSERT_NOT_REACHED("invalid parameter (property:%d)",property_id);
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    return CALENDAR_ERROR_NONE;
}

static int __cal_record_instance_allday_set_str( calendar_record_h record, unsigned int property_id, const char* value )
{
    cal_instance_allday_s *rec = (cal_instance_allday_s*)(record);
    switch( property_id )
    {
    case CAL_PROPERTY_INSTANCE_ALLDAY_SUMMARY:
        CAL_FREE(rec->summary);
        rec->summary = SAFE_STRDUP(value);
        break;
    case CAL_PROPERTY_INSTANCE_ALLDAY_DESCRIPTION:
        CAL_FREE(rec->description);
        rec->description = SAFE_STRDUP(value);
        break;
    case CAL_PROPERTY_INSTANCE_ALLDAY_LOCATION:
        CAL_FREE(rec->location);
        rec->location = SAFE_STRDUP(value);
        break;
    default:
        ASSERT_NOT_REACHED("invalid parameter (property:%d)",property_id);
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    return CALENDAR_ERROR_NONE;
}

static int __cal_record_instance_allday_set_int( calendar_record_h record, unsigned int property_id, int value )
{
    cal_instance_allday_s *rec = (cal_instance_allday_s*)(record);
    switch( property_id )
    {
    case CAL_PROPERTY_INSTANCE_ALLDAY_EVENT_ID:
        (rec->event_id) = value;
        break;
    case CAL_PROPERTY_INSTANCE_ALLDAY_CALENDAR_ID:
        (rec->calendar_id) = value;
        break;
    case CAL_PROPERTY_INSTANCE_ALLDAY_BUSY_STATUS:
        (rec->busy_status) = value;
        break;
    case CAL_PROPERTY_INSTANCE_ALLDAY_EVENT_STATUS:
        (rec->event_status) = value;
        break;
    case CAL_PROPERTY_INSTANCE_ALLDAY_PRIORITY:
        (rec->priority) = value;
        break;
    case CAL_PROPERTY_INSTANCE_ALLDAY_SENSITIVITY:
        (rec->sensitivity) = value;
        break;
    case CAL_PROPERTY_INSTANCE_ALLDAY_HAS_RRULE:
        (rec->has_rrule) = value;
        break;
    case CAL_PROPERTY_INSTANCE_ALLDAY_HAS_ALARM:
        (rec->has_alarm) = value;
        break;
    case CAL_PROPERTY_INSTANCE_ALLDAY_ORIGINAL_EVENT_ID:
        (rec->original_event_id) = value;
        break;
    default:
        ASSERT_NOT_REACHED("invalid parameter (property:%d)",property_id);
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    return CALENDAR_ERROR_NONE;
}

static int __cal_record_instance_allday_set_double( calendar_record_h record, unsigned int property_id, double value )
{
    cal_instance_allday_s *rec = (cal_instance_allday_s*)(record);
    switch( property_id )
    {
    case CAL_PROPERTY_INSTANCE_ALLDAY_LATITUDE:
        (rec->latitude) = value;
        break;
    case CAL_PROPERTY_INSTANCE_ALLDAY_LONGITUDE:
        (rec->longitude) = value;
        break;

    default:
        ASSERT_NOT_REACHED("invalid parameter (property:%d)",property_id);
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    return CALENDAR_ERROR_NONE;
}

static int __cal_record_instance_allday_set_lli( calendar_record_h record, unsigned int property_id, long long int value )
{
    cal_instance_allday_s *rec = (cal_instance_allday_s*)(record);
    switch( property_id )
    {
    case CAL_PROPERTY_INSTANCE_ALLDAY_LAST_MODIFIED_TIME:
        (rec->last_mod) = value;
        break;
    default:
        ASSERT_NOT_REACHED("invalid parameter (property:%d)",property_id);
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    return CALENDAR_ERROR_NONE;
}

static int __cal_record_instance_allday_set_caltime( calendar_record_h record, unsigned int property_id, calendar_time_s value )
{
    cal_instance_allday_s *rec = (cal_instance_allday_s*)(record);
    switch( property_id )
    {
    case CAL_PROPERTY_INSTANCE_ALLDAY_START:
        (rec->dtstart_type) = CALENDAR_TIME_LOCALTIME;//value.type;
        (rec->dtstart_year) = value.time.date.year;
        (rec->dtstart_month) = value.time.date.month;
        (rec->dtstart_mday) = value.time.date.mday ;
        break;
    case CAL_PROPERTY_INSTANCE_ALLDAY_END:
        (rec->dtend_type) = CALENDAR_TIME_LOCALTIME;//value.type;
        (rec->dtend_year) = value.time.date.year;
        (rec->dtend_month) = value.time.date.month;
        (rec->dtend_mday) = value.time.date.mday ;
        break;
    default:
        ASSERT_NOT_REACHED("invalid parameter (property:%d)",property_id);
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    return CALENDAR_ERROR_NONE;
}