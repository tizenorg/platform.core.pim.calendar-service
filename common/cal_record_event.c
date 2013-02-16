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
#include <stdlib.h>		//calloc
#include <stdbool.h>		//bool
#include <string.h>

#include "calendar_list.h"

#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_view.h"

#include "cal_record.h"

static int __cal_record_event_create( calendar_record_h* out_record );
static int __cal_record_event_destroy( calendar_record_h record, bool delete_child );
static int __cal_record_event_clone( calendar_record_h record, calendar_record_h* out_record );
static int __cal_record_event_get_str( calendar_record_h record, unsigned int property_id, char** out_str );
static int __cal_record_event_get_str_p( calendar_record_h record, unsigned int property_id, char** out_str );
static int __cal_record_event_get_int( calendar_record_h record, unsigned int property_id, int* out_value );
static int __cal_record_event_get_double( calendar_record_h record, unsigned int property_id, double* out_value );
static int __cal_record_event_get_lli( calendar_record_h record, unsigned int property_id, long long int* out_value );
static int __cal_record_event_get_caltime( calendar_record_h record, unsigned int property_id, calendar_time_s* out_value );
static int __cal_record_event_set_str( calendar_record_h record, unsigned int property_id, const char* value );
static int __cal_record_event_set_int( calendar_record_h record, unsigned int property_id, int value );
static int __cal_record_event_set_double( calendar_record_h record, unsigned int property_id, double value );
static int __cal_record_event_set_lli( calendar_record_h record, unsigned int property_id, long long int value );
static int __cal_record_event_set_caltime( calendar_record_h record, unsigned int property_id, calendar_time_s value );
static int __cal_record_event_add_child_record( calendar_record_h record, unsigned int property_id, calendar_record_h child_record );
static int __cal_record_event_remove_child_record( calendar_record_h record, unsigned int property_id, calendar_record_h child_record );
static int __cal_record_event_get_child_record_count( calendar_record_h record, unsigned int property_id, unsigned int* count  );
static int __cal_record_event_get_child_record_at_p( calendar_record_h record, unsigned int property_id, int index, calendar_record_h* child_record );
static int __cal_record_event_clone_child_record_list( calendar_record_h record, unsigned int property_id, calendar_list_h* out_list );

cal_record_plugin_cb_s _cal_record_event_plugin_cb = {
    .create = __cal_record_event_create,
    .destroy = __cal_record_event_destroy,
    .clone = __cal_record_event_clone,
    .get_str = __cal_record_event_get_str,
    .get_str_p = __cal_record_event_get_str_p,
    .get_int = __cal_record_event_get_int,
    .get_double = __cal_record_event_get_double,
    .get_lli = __cal_record_event_get_lli,
    .get_caltime = __cal_record_event_get_caltime,
    .set_str = __cal_record_event_set_str,
    .set_int = __cal_record_event_set_int,
    .set_double = __cal_record_event_set_double,
    .set_lli = __cal_record_event_set_lli,
    .set_caltime = __cal_record_event_set_caltime,
    .add_child_record = __cal_record_event_add_child_record,
    .remove_child_record = __cal_record_event_remove_child_record,
    .get_child_record_count = __cal_record_event_get_child_record_count,
    .get_child_record_at_p = __cal_record_event_get_child_record_at_p,
    .clone_child_record_list = __cal_record_event_clone_child_record_list
};

static void __cal_record_event_struct_init(cal_event_s *record)
{
    memset(record,0,sizeof(cal_event_s));

    record->meeting_status = CALENDAR_EVENT_STATUS_NONE;
    record->calendar_id = DEFAULT_EVENT_CALENDAR_BOOK_ID;

    record->index = CAL_INVALID_ID;
    record->timezone = 0;
    record->contact_id = CAL_INVALID_ID;
    record->attendee_list = NULL;
    record->busy_status = 2;
    record->summary = NULL;
    record->description = NULL;
    record->location= NULL;
    record->categories = NULL;
    record->exdate = NULL;
    record->organizer_email = NULL;
    record->organizer_name = NULL;
    record->uid= NULL;
    record->original_event_id = CAL_INVALID_ID;
	record->event_status = CALENDAR_EVENT_STATUS_NONE;

    record->is_deleted = 0;
    record->latitude = CALENDAR_RECORD_NO_COORDINATE; // set default 1000 out of range(-180 ~ 180)
    record->longitude = CALENDAR_RECORD_NO_COORDINATE; // set default 1000 out of range(-180 ~ 180)
    record->freq = CALENDAR_RECURRENCE_NONE;
	record->wkst = CALENDAR_SUNDAY;
    record->until_utime = CALENDAR_RECORD_NO_UNTIL;

    return ;
}

static int __cal_record_event_create( calendar_record_h* out_record )
{
    cal_event_s *temp = NULL;
    int ret= CALENDAR_ERROR_NONE, type = 0;

    type = CAL_RECORD_TYPE_EVENT;

    temp = (cal_event_s*)calloc(1, sizeof(cal_event_s));
    retvm_if(NULL == temp, CALENDAR_ERROR_OUT_OF_MEMORY, "calloc(cal_event_s:sch) Failed(%d)", CALENDAR_ERROR_OUT_OF_MEMORY);

    __cal_record_event_struct_init(temp);

    *out_record = (calendar_record_h)temp;

    return ret;
}

static void __cal_record_event_struct_free(cal_event_s *record, bool delete_child)
{
    CAL_FREE(record->summary);
    CAL_FREE(record->description);
    CAL_FREE(record->location);
    CAL_FREE(record->categories);
    CAL_FREE(record->exdate);
    CAL_FREE(record->uid);
    CAL_FREE(record->organizer_name);
    CAL_FREE(record->organizer_email);
    CAL_FREE(record->start_tzid);
    CAL_FREE(record->end_tzid);

    CAL_FREE(record->bysecond);
    CAL_FREE(record->byminute);
    CAL_FREE(record->byhour);
    CAL_FREE(record->byday);
    CAL_FREE(record->bymonthday);
    CAL_FREE(record->byyearday);
    CAL_FREE(record->byweekno);
    CAL_FREE(record->bymonth);
    CAL_FREE(record->bysetpos);

    CAL_FREE(record->recurrence_id);
    CAL_FREE(record->rdate);
    CAL_FREE(record->sync_data1);
    CAL_FREE(record->sync_data2);
    CAL_FREE(record->sync_data3);
    CAL_FREE(record->sync_data4);

    if (delete_child == true)
    {
        if ( record->alarm_list )
        {
            GList *alarm_list = g_list_first(record->alarm_list);
            calendar_record_h alarm_child_record = NULL;
            while (alarm_list)
            {
                alarm_child_record = (calendar_record_h)alarm_list->data;
                if (alarm_child_record == NULL)
                {
                    alarm_list = g_list_next(alarm_list);
                    continue;
                }

                calendar_record_destroy(alarm_child_record, true);

                alarm_list = g_list_next(alarm_list);
            }
            g_list_free(record->alarm_list);
            record->alarm_list = NULL;
        }

        if( record->attendee_list )
        {
            GList * attendee_list = g_list_first(record->attendee_list);
            calendar_record_h attendee = NULL;

            while (attendee_list)
            {
                attendee = (calendar_record_h)attendee_list->data;
                if (attendee == NULL)
                {
                    attendee_list = g_list_next(attendee_list);
                    continue;
                }

                calendar_record_destroy(attendee, true);

                attendee_list = g_list_next(attendee_list);
            }
            g_list_free(record->attendee_list);
            record->attendee_list = NULL;
        }
        if( record->exception_list )
        {
            GList * exception_list = g_list_first(record->exception_list);
            calendar_record_h exception = NULL;

            while (exception_list)
            {
                exception = (calendar_record_h)exception_list->data;
                if (exception == NULL)
                {
                    exception_list = g_list_next(exception_list);
                    continue;
                }

                calendar_record_destroy(exception, true);

                exception_list = g_list_next(exception_list);
            }
            g_list_free(record->exception_list);
            record->exception_list = NULL;
        }
        if( record->extended_list )
        {
            GList * extended_list = g_list_first(record->extended_list);
            calendar_record_h extended = NULL;

            while (extended_list)
            {
                extended = (calendar_record_h)extended_list->data;
                if (extended == NULL)
                {
                    extended_list = g_list_next(extended_list);
                    continue;
                }

                calendar_record_destroy(extended, true);

                extended_list = g_list_next(extended_list);
            }
            g_list_free(record->extended_list);
            record->extended_list = NULL;
        }
    }

    CAL_FREE(record);
}

static int __cal_record_event_destroy( calendar_record_h record, bool delete_child )
{
    int ret = CALENDAR_ERROR_NONE;

    cal_event_s *temp = (cal_event_s*)(record);

   __cal_record_event_struct_free(temp, delete_child);

    return ret;
}

static int __cal_record_event_clone( calendar_record_h record, calendar_record_h* out_record )
{
    cal_event_s *out_data = NULL;
    cal_event_s *src_data = NULL;

    src_data = (cal_event_s*)(record);

    out_data = calloc(1, sizeof(cal_event_s));
    retvm_if(NULL == out_data, CALENDAR_ERROR_OUT_OF_MEMORY, "calloc(cal_event_s) Failed(%d)", CALENDAR_ERROR_OUT_OF_MEMORY);

    CAL_RECORD_COPY_COMMON(&(out_data->common), &(src_data->common));

    out_data->index = src_data->index;
    out_data->calendar_id = src_data->calendar_id;

    out_data->summary = SAFE_STRDUP(src_data->summary);
    out_data->description = SAFE_STRDUP(src_data->description);
    out_data->location = SAFE_STRDUP(src_data->location);
    out_data->categories = SAFE_STRDUP(src_data->categories);
    out_data->exdate = SAFE_STRDUP(src_data->exdate);

    out_data->event_status = src_data->event_status;
    out_data->priority = src_data->priority;
    out_data->timezone = src_data->timezone;
    out_data->contact_id = src_data->contact_id;
    out_data->busy_status = src_data->busy_status;
    out_data->sensitivity = src_data->sensitivity;
    out_data->meeting_status = src_data->meeting_status;

    out_data->uid = SAFE_STRDUP(src_data->uid);
    out_data->organizer_name = SAFE_STRDUP(src_data->organizer_name);
    out_data->organizer_email = SAFE_STRDUP(src_data->organizer_email);

    out_data->original_event_id = src_data->original_event_id;
    out_data->latitude = src_data->latitude;
    out_data->longitude = src_data->longitude;
    out_data->email_id = src_data->email_id;
    //out_data->availability = src_data->availability;
    out_data->created_time = src_data->created_time;
    //out_data->completed_time = src_data->completed_time;
    //out_data->progress = src_data->progress;
    out_data->is_deleted = src_data->is_deleted;
    //out_data->duration = src_data->duration;
    out_data->last_mod = src_data->last_mod;
    //out_data->has_rrule = src_data->has_rrule;
    out_data->freq = src_data->freq;
    out_data->range_type = src_data->range_type;
    out_data->until_type = src_data->until_type;
    out_data->until_utime = src_data->until_utime;
    out_data->until_year = src_data->until_year;
    out_data->until_month = src_data->until_month;
    out_data->until_mday = src_data->until_mday;
    out_data->count = src_data->count;
    out_data->interval = src_data->interval;
    out_data->bysecond = SAFE_STRDUP(src_data->bysecond);
    out_data->byminute = SAFE_STRDUP(src_data->byminute);
    out_data->byhour = SAFE_STRDUP(src_data->byhour);
    out_data->byday = SAFE_STRDUP(src_data->byday);
    out_data->bymonthday = SAFE_STRDUP(src_data->bymonthday);
    out_data->byyearday = SAFE_STRDUP(src_data->byyearday);
    out_data->byweekno = SAFE_STRDUP(src_data->byweekno);
    out_data->bymonth = SAFE_STRDUP(src_data->bymonth);
    out_data->bysetpos = SAFE_STRDUP(src_data->bysetpos);
    out_data->wkst = src_data->wkst;
    out_data->recurrence_id = SAFE_STRDUP(src_data->recurrence_id);
    out_data->rdate = SAFE_STRDUP(src_data->rdate);
    out_data->has_attendee = src_data->has_attendee;
    out_data->has_alarm = src_data->has_alarm;
    out_data->system_type = src_data->system_type;
    out_data->updated = src_data->updated;

    out_data->sync_data1 = SAFE_STRDUP(src_data->sync_data1);
    out_data->sync_data2 = SAFE_STRDUP(src_data->sync_data2);
    out_data->sync_data3 = SAFE_STRDUP(src_data->sync_data3);
    out_data->sync_data4 = SAFE_STRDUP(src_data->sync_data4);

    out_data->start = src_data->start;
    out_data->start_tzid = SAFE_STRDUP(src_data->start_tzid);
    out_data->end = src_data->end;
    out_data->end_tzid = SAFE_STRDUP(src_data->end_tzid);

    if( src_data->alarm_list )
    {
        GList *alarm_list = g_list_first(src_data->alarm_list);
        calendar_record_h alarm_child_record;

        while (alarm_list)
        {
            calendar_record_h child_clone = NULL;
            alarm_child_record = (calendar_record_h)alarm_list->data;
            if (alarm_child_record == NULL)
            {
                alarm_list = g_list_next(alarm_list);
                continue;
            }
            if (calendar_record_clone(alarm_child_record, &child_clone) == CALENDAR_ERROR_NONE)
            {

                calendar_record_add_child_record((calendar_record_h)out_data,
                        CAL_PROPERTY_EVENT_CALENDAR_ALARM,child_clone);

            }
			alarm_list = g_list_next(alarm_list);
        }

    }

    if( src_data->attendee_list )
    {
        GList * attendee_list = src_data->attendee_list;
        calendar_record_h attendee_child = NULL;

        while (attendee_list)
        {
            calendar_record_h child_clone = NULL;
            attendee_child = (calendar_record_h)attendee_list->data;
            if (attendee_child == NULL)
            {
                attendee_list = g_list_next(attendee_list);
                continue;
            }
            if (calendar_record_clone(attendee_child, &child_clone) == CALENDAR_ERROR_NONE)
            {

                calendar_record_add_child_record((calendar_record_h)out_data,
                        CAL_PROPERTY_EVENT_CALENDAR_ATTENDEE,child_clone);

            }
			attendee_list = g_list_next(attendee_list);
        }

    }

    if( src_data->exception_list )
    {
        GList * exception_list = src_data->exception_list;
        calendar_record_h exception = NULL;

        while (exception_list)
        {
            calendar_record_h child_clone = NULL;
            exception = (calendar_record_h)exception_list->data;
            if (exception == NULL)
            {
                exception_list = g_list_next(exception_list);
                continue;
            }
            if (calendar_record_clone(exception, &child_clone) == CALENDAR_ERROR_NONE)
            {

                calendar_record_add_child_record((calendar_record_h)out_data,
                        CAL_PROPERTY_EVENT_EXCEPTION,child_clone);

            }
            exception_list = g_list_next(exception_list);
        }

    }

    if( src_data->extended_list )
    {
        GList * extended_list = src_data->extended_list;
        calendar_record_h extended = NULL;

        while (extended_list)
        {
            calendar_record_h child_clone = NULL;
            extended = (calendar_record_h)extended_list->data;
            if (extended == NULL)
            {
                extended_list = g_list_next(extended_list);
                continue;
            }
            if (calendar_record_clone(extended, &child_clone) == CALENDAR_ERROR_NONE)
            {

                calendar_record_add_child_record((calendar_record_h)out_data,
                        CAL_PROPERTY_EVENT_EXTENDED,child_clone);

            }
            extended_list = g_list_next(extended_list);
        }

    }

    *out_record = (calendar_record_h)out_data;

    return CALENDAR_ERROR_NONE;
}

static int __cal_record_event_get_str( calendar_record_h record, unsigned int property_id, char** out_str )
{
    cal_event_s *rec = (cal_event_s*)(record);
    switch( property_id )
    {
    case CAL_PROPERTY_EVENT_SUMMARY:
        *out_str = SAFE_STRDUP(rec->summary);
        break;
    case CAL_PROPERTY_EVENT_DESCRIPTION:
        *out_str = SAFE_STRDUP(rec->description);
        break;
    case CAL_PROPERTY_EVENT_LOCATION:
        *out_str = SAFE_STRDUP(rec->location);
        break;
    case CAL_PROPERTY_EVENT_CATEGORIES:
        *out_str = SAFE_STRDUP(rec->categories);
        break;
    case CAL_PROPERTY_EVENT_EXDATE:
        *out_str = SAFE_STRDUP(rec->exdate);
        break;
    case CAL_PROPERTY_EVENT_UID:
        *out_str = SAFE_STRDUP(rec->uid);
        break;
    case CAL_PROPERTY_EVENT_ORGANIZER_NAME:
        *out_str = SAFE_STRDUP(rec->organizer_name);
        break;
    case CAL_PROPERTY_EVENT_ORGANIZER_EMAIL:
        *out_str = SAFE_STRDUP(rec->organizer_email);
        break;
    case CAL_PROPERTY_EVENT_BYSECOND:
        *out_str = SAFE_STRDUP(rec->bysecond);
        break;
    case CAL_PROPERTY_EVENT_BYMINUTE:
        *out_str = SAFE_STRDUP(rec->byminute);
        break;
    case CAL_PROPERTY_EVENT_BYHOUR:
        *out_str = SAFE_STRDUP(rec->byhour);
        break;
    case CAL_PROPERTY_EVENT_BYDAY:
        *out_str = SAFE_STRDUP(rec->byday);
        break;
    case CAL_PROPERTY_EVENT_BYMONTHDAY:
        *out_str = SAFE_STRDUP(rec->bymonthday);
        break;
    case CAL_PROPERTY_EVENT_BYYEARDAY:
        *out_str = SAFE_STRDUP(rec->byyearday);
        break;
    case CAL_PROPERTY_EVENT_BYWEEKNO:
        *out_str = SAFE_STRDUP(rec->byweekno);
        break;
    case CAL_PROPERTY_EVENT_BYMONTH:
        *out_str = SAFE_STRDUP(rec->bymonth);
        break;
    case CAL_PROPERTY_EVENT_BYSETPOS:
        *out_str = SAFE_STRDUP(rec->bysetpos);
        break;
    case CAL_PROPERTY_EVENT_RECURRENCE_ID:
        *out_str = SAFE_STRDUP(rec->recurrence_id);
        break;
    case CAL_PROPERTY_EVENT_RDATE:
        *out_str = SAFE_STRDUP(rec->rdate);
        break;
    case CAL_PROPERTY_EVENT_SYNC_DATA1:
        *out_str = SAFE_STRDUP(rec->sync_data1);
        break;
    case CAL_PROPERTY_EVENT_SYNC_DATA2:
        *out_str = SAFE_STRDUP(rec->sync_data2);
        break;
    case CAL_PROPERTY_EVENT_SYNC_DATA3:
        *out_str = SAFE_STRDUP(rec->sync_data3);
        break;
    case CAL_PROPERTY_EVENT_SYNC_DATA4:
        *out_str = SAFE_STRDUP(rec->sync_data4);
        break;
    case CAL_PROPERTY_EVENT_START_TZID:
        *out_str = SAFE_STRDUP(rec->start_tzid);
        break;
    case CAL_PROPERTY_EVENT_END_TZID:
        *out_str = SAFE_STRDUP(rec->end_tzid);
        break;
    default:
        ASSERT_NOT_REACHED("invalid parameter (property:%d)",property_id);
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    return CALENDAR_ERROR_NONE;
}

static int __cal_record_event_get_str_p( calendar_record_h record, unsigned int property_id, char** out_str )
{
    cal_event_s *rec = (cal_event_s*)(record);
    switch( property_id )
    {
    case CAL_PROPERTY_EVENT_SUMMARY:
        *out_str = (rec->summary);
        break;
    case CAL_PROPERTY_EVENT_DESCRIPTION:
        *out_str = (rec->description);
        break;
    case CAL_PROPERTY_EVENT_LOCATION:
        *out_str = (rec->location);
        break;
    case CAL_PROPERTY_EVENT_CATEGORIES:
        *out_str = (rec->categories);
        break;
    case CAL_PROPERTY_EVENT_EXDATE:
        *out_str = (rec->exdate);
        break;
    case CAL_PROPERTY_EVENT_UID:
        *out_str = (rec->uid);
        break;
    case CAL_PROPERTY_EVENT_ORGANIZER_NAME:
        *out_str = (rec->organizer_name);
        break;
    case CAL_PROPERTY_EVENT_ORGANIZER_EMAIL:
        *out_str = (rec->organizer_email);
        break;
    case CAL_PROPERTY_EVENT_BYSECOND:
        *out_str = (rec->bysecond);
        break;
    case CAL_PROPERTY_EVENT_BYMINUTE:
        *out_str = (rec->byminute);
        break;
    case CAL_PROPERTY_EVENT_BYHOUR:
        *out_str = (rec->byhour);
        break;
    case CAL_PROPERTY_EVENT_BYDAY:
        *out_str = (rec->byday);
        break;
    case CAL_PROPERTY_EVENT_BYMONTHDAY:
        *out_str = (rec->bymonthday);
        break;
    case CAL_PROPERTY_EVENT_BYYEARDAY:
        *out_str = (rec->byyearday);
        break;
    case CAL_PROPERTY_EVENT_BYWEEKNO:
        *out_str = (rec->byweekno);
        break;
    case CAL_PROPERTY_EVENT_BYMONTH:
        *out_str = (rec->bymonth);
        break;
    case CAL_PROPERTY_EVENT_BYSETPOS:
        *out_str = (rec->bysetpos);
        break;
    case CAL_PROPERTY_EVENT_RECURRENCE_ID:
        *out_str = (rec->recurrence_id);
        break;
    case CAL_PROPERTY_EVENT_RDATE:
        *out_str = (rec->rdate);
        break;
    case CAL_PROPERTY_EVENT_SYNC_DATA1:
        *out_str = (rec->sync_data1);
        break;
    case CAL_PROPERTY_EVENT_SYNC_DATA2:
        *out_str = (rec->sync_data2);
        break;
    case CAL_PROPERTY_EVENT_SYNC_DATA3:
        *out_str = (rec->sync_data3);
        break;
    case CAL_PROPERTY_EVENT_SYNC_DATA4:
        *out_str = (rec->sync_data4);
        break;
    case CAL_PROPERTY_EVENT_START_TZID:
        *out_str = (rec->start_tzid);
        break;
    case CAL_PROPERTY_EVENT_END_TZID:
        *out_str = (rec->end_tzid);
        break;
    default:
        ASSERT_NOT_REACHED("invalid parameter (property:%d)",property_id);
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    return CALENDAR_ERROR_NONE;
}

static int __cal_record_event_get_int( calendar_record_h record, unsigned int property_id, int* out_value )
{
    cal_event_s *rec = (cal_event_s*)(record);
    switch( property_id )
    {
    case CAL_PROPERTY_EVENT_ID:
        *out_value = (rec->index);
        break;
    case CAL_PROPERTY_EVENT_CALENDAR_ID:
        *out_value = (rec->calendar_id);
        break;
    case CAL_PROPERTY_EVENT_EVENT_STATUS:
        *out_value = (rec->event_status);
        break;
    //priority
    case CAL_PROPERTY_EVENT_PRIORITY:
        *out_value = (rec->priority);
        break;
    //timezone
    case CAL_PROPERTY_EVENT_TIMEZONE:
        *out_value = (rec->timezone);
        break;
    //contact_id
    case CAL_PROPERTY_EVENT_CONTACT_ID:
        *out_value = (rec->contact_id);
        break;
    //busy_status
    case CAL_PROPERTY_EVENT_BUSY_STATUS:
        *out_value = (rec->busy_status);
        break;
    //sensitivity
    case CAL_PROPERTY_EVENT_SENSITIVITY:
        *out_value = (rec->sensitivity);
        break;
    //meeting_status
    case CAL_PROPERTY_EVENT_MEETING_STATUS:
        *out_value = (rec->meeting_status);
        break;
    //original_event_id
    case CAL_PROPERTY_EVENT_ORIGINAL_EVENT_ID:
        *out_value = (rec->original_event_id);
        break;
    //email_id
    case CAL_PROPERTY_EVENT_EMAIL_ID:
        *out_value = (rec->email_id);
        break;
    //is_deleted
    case CAL_PROPERTY_EVENT_IS_DELETED:
        *out_value = (rec->is_deleted);
        break;
    //freq
    case CAL_PROPERTY_EVENT_FREQ:
        *out_value = (rec->freq);
        break;
    //range_type
    case CAL_PROPERTY_EVENT_RANGE_TYPE:
        *out_value = (rec->range_type);
        break;
    //count
    case CAL_PROPERTY_EVENT_COUNT:
        *out_value = (rec->count);
        break;
    //interval
    case CAL_PROPERTY_EVENT_INTERVAL:
        *out_value = (rec->interval);
        break;
    //wkst
    case CAL_PROPERTY_EVENT_WKST:
        *out_value = (rec->wkst);
        break;
    //has_attendee
    case CAL_PROPERTY_EVENT_HAS_ATTENDEE:
        *out_value = (rec->has_attendee);
        break;
    //has_alarm
    case CAL_PROPERTY_EVENT_HAS_ALARM:
        *out_value = (rec->has_alarm);
        break;
    case CAL_PROPERTY_EVENT_CALENDAR_SYSTEM_TYPE:
        *out_value = (rec->system_type);
        break;
    case CAL_PROPERTY_EVENT_IS_ALLDAY:
        *out_value = (rec->start.type);
        break;
    default:
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    return CALENDAR_ERROR_NONE;
}

static int __cal_record_event_get_double( calendar_record_h record, unsigned int property_id, double* out_value )
{
    cal_event_s *rec = (cal_event_s*)(record);
    switch( property_id )
    {
    case CAL_PROPERTY_EVENT_LATITUDE:
        *out_value = (rec->latitude);
        break;
    case CAL_PROPERTY_EVENT_LONGITUDE:
        *out_value = (rec->longitude);
        break;
    default:
        ASSERT_NOT_REACHED("invalid parameter (property:%d)",property_id);
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    return CALENDAR_ERROR_NONE;
}

static int __cal_record_event_get_lli( calendar_record_h record, unsigned int property_id, long long int* out_value )
{
    cal_event_s *rec = (cal_event_s*)(record);
    switch( property_id )
    {
    case CAL_PROPERTY_EVENT_CREATED_TIME:
        *out_value = (rec->created_time);
        break;
    case CAL_PROPERTY_EVENT_LAST_MODIFIED_TIME:
        *out_value = (rec->last_mod);
        break;
    default:
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    return CALENDAR_ERROR_NONE;
}

static int __cal_record_event_get_caltime( calendar_record_h record, unsigned int property_id, calendar_time_s* out_value )
{
    cal_event_s *rec = (cal_event_s*)(record);
    switch( property_id )
    {
    case CAL_PROPERTY_EVENT_START:
        *out_value = rec->start;
        break;
    case CAL_PROPERTY_EVENT_END:
        *out_value = rec->end;
        break;
    case CAL_PROPERTY_EVENT_UNTIL:
        if (rec->until_type == CALENDAR_TIME_UTIME)
        {
            CAL_CALTIME_SET_UTIME(*out_value, rec->until_utime);
        }
        else
        {
            CAL_CALTIME_SET_DATE(*out_value, rec->until_year,rec->until_month,rec->until_mday);
        }
        break;
    default:
        ASSERT_NOT_REACHED("invalid parameter (property:%d)",property_id);
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    return CALENDAR_ERROR_NONE;
}

static int __cal_record_event_set_str( calendar_record_h record, unsigned int property_id, const char* value )
{
    cal_event_s *rec = (cal_event_s*)(record);
    switch( property_id )
    {
    case CAL_PROPERTY_EVENT_SUMMARY:
        CAL_FREE(rec->summary);
        rec->summary = SAFE_STRDUP(value);
        break;
    case CAL_PROPERTY_EVENT_DESCRIPTION:
        CAL_FREE(rec->description);
        rec->description = SAFE_STRDUP(value);
        break;
    case CAL_PROPERTY_EVENT_LOCATION:
        CAL_FREE(rec->location);
        rec->location = SAFE_STRDUP(value);
        break;
    case CAL_PROPERTY_EVENT_CATEGORIES:
        CAL_FREE(rec->categories);
        rec->categories = SAFE_STRDUP(value);
        break;
    case CAL_PROPERTY_EVENT_EXDATE:
        CAL_FREE(rec->exdate);
        rec->exdate = SAFE_STRDUP(value);
        break;
    case CAL_PROPERTY_EVENT_UID:
        CAL_FREE(rec->uid);
        rec->uid = SAFE_STRDUP(value);
        break;
    case CAL_PROPERTY_EVENT_ORGANIZER_NAME:
        CAL_FREE(rec->organizer_name);
        rec->organizer_name = SAFE_STRDUP(value);
        break;
    case CAL_PROPERTY_EVENT_ORGANIZER_EMAIL:
        CAL_FREE(rec->organizer_email);
        rec->organizer_email = SAFE_STRDUP(value);
        break;
    case CAL_PROPERTY_EVENT_BYSECOND:
        CAL_FREE(rec->bysecond);
        rec->bysecond = SAFE_STRDUP(value);
        break;
    case CAL_PROPERTY_EVENT_BYMINUTE:
        CAL_FREE(rec->byminute);
        rec->byminute = SAFE_STRDUP(value);
        break;
    case CAL_PROPERTY_EVENT_BYHOUR:
        CAL_FREE(rec->byhour);
        rec->byhour = SAFE_STRDUP(value);
        break;
    case CAL_PROPERTY_EVENT_BYDAY:
        CAL_FREE(rec->byday);
        rec->byday = SAFE_STRDUP(value);
        break;
    case CAL_PROPERTY_EVENT_BYMONTHDAY:
        CAL_FREE(rec->bymonthday);
        rec->bymonthday = SAFE_STRDUP(value);
        break;
    case CAL_PROPERTY_EVENT_BYYEARDAY:
        CAL_FREE(rec->byyearday);
        rec->byyearday = SAFE_STRDUP(value);
        break;
    case CAL_PROPERTY_EVENT_BYWEEKNO:
        CAL_FREE(rec->byweekno);
        rec->byweekno = SAFE_STRDUP(value);
        break;
    case CAL_PROPERTY_EVENT_BYMONTH:
        CAL_FREE(rec->bymonth);
        rec->bymonth = SAFE_STRDUP(value);
        break;
    case CAL_PROPERTY_EVENT_BYSETPOS:
        CAL_FREE(rec->bysetpos);
        rec->bysetpos = SAFE_STRDUP(value);
        break;
    case CAL_PROPERTY_EVENT_RECURRENCE_ID:
        CAL_FREE(rec->recurrence_id);
        rec->recurrence_id = SAFE_STRDUP(value);
        break;
    case CAL_PROPERTY_EVENT_RDATE:
        CAL_FREE(rec->rdate);
        rec->rdate = SAFE_STRDUP(value);
        break;
    case CAL_PROPERTY_EVENT_SYNC_DATA1:
        CAL_FREE(rec->sync_data1);
        rec->sync_data1 = SAFE_STRDUP(value);
        break;
    case CAL_PROPERTY_EVENT_SYNC_DATA2:
        CAL_FREE(rec->sync_data2);
        rec->sync_data2 = SAFE_STRDUP(value);
        break;
    case CAL_PROPERTY_EVENT_SYNC_DATA3:
        CAL_FREE(rec->sync_data3);
        rec->sync_data3 = SAFE_STRDUP(value);
        break;
    case CAL_PROPERTY_EVENT_SYNC_DATA4:
        CAL_FREE(rec->sync_data4);
        rec->sync_data4 = SAFE_STRDUP(value);
        break;
    case CAL_PROPERTY_EVENT_START_TZID:
        CAL_FREE(rec->start_tzid);
        rec->start_tzid = SAFE_STRDUP(value);
        break;
    case CAL_PROPERTY_EVENT_END_TZID:
        CAL_FREE(rec->end_tzid);
        rec->end_tzid = SAFE_STRDUP(value);
        break;
    default:
        ASSERT_NOT_REACHED("invalid parameter (property:%d)",property_id);
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    return CALENDAR_ERROR_NONE;
}

static int __cal_record_event_set_int( calendar_record_h record, unsigned int property_id, int value )
{
    cal_event_s *rec = (cal_event_s*)(record);
    switch( property_id )
    {
    case CAL_PROPERTY_EVENT_ID:
        (rec->index) = value;
        break;
    case CAL_PROPERTY_EVENT_CALENDAR_ID:
        (rec->calendar_id) = value;
        break;
    case CAL_PROPERTY_EVENT_EVENT_STATUS:
		switch (value)
		{
		case CALENDAR_EVENT_STATUS_NONE:
		case CALENDAR_EVENT_STATUS_TENTATIVE:
		case CALENDAR_EVENT_STATUS_CONFIRMED:
		case CALENDAR_EVENT_STATUS_CANCELLED:
			(rec->event_status) = value;
			break;
		default:
			ERR("invalid parameter (value:%d)", value);
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
		break;
    //priority
    case CAL_PROPERTY_EVENT_PRIORITY:
        (rec->priority) = value;
        break;
    //timezone
    case CAL_PROPERTY_EVENT_TIMEZONE:
        (rec->timezone) = value;
        break;
    //contact_id
    case CAL_PROPERTY_EVENT_CONTACT_ID:
        (rec->contact_id) = value;
        break;
    //busy_status
    case CAL_PROPERTY_EVENT_BUSY_STATUS:
		switch (value)
		{
		case CALENDAR_EVENT_BUSY_STATUS_FREE:
		case CALENDAR_EVENT_BUSY_STATUS_BUSY:
		case CALENDAR_EVENT_BUSY_STATUS_UNAVAILABLE:
		case CALENDAR_EVENT_BUSY_STATUS_TENTATIVE:
	        (rec->busy_status) = value;
			break;
		default:
			ERR("invalid parameter (value:%d)", value);
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
        break;
    //sensitivity
    case CAL_PROPERTY_EVENT_SENSITIVITY:
		switch (value)
		{
		case CALENDAR_SENSITIVITY_PUBLIC:
		case CALENDAR_SENSITIVITY_PRIVATE:
		case CALENDAR_SENSITIVITY_CONFIDENTIAL:
	        (rec->sensitivity) = value;
			break;
		default:
			ERR("invalid parameter (value:%d)", value);
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
        break;
    //meeting_status
    case CAL_PROPERTY_EVENT_MEETING_STATUS:
		switch (value)
		{
		case CALENDAR_MEETING_STATUS_NOTMEETING:
		case CALENDAR_MEETING_STATUS_MEETING:
		case CALENDAR_MEETING_STATUS_RECEIVED:
		case CALENDAR_MEETING_STATUS_CANCELED:
	        (rec->meeting_status) = value;
			break;
		default:
			ERR("invalid parameter (value:%d)", value);
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
        break;
    //original_event_id
    case CAL_PROPERTY_EVENT_ORIGINAL_EVENT_ID:
        (rec->original_event_id) = value;
        break;
    //email_id
    case CAL_PROPERTY_EVENT_EMAIL_ID:
        (rec->email_id) = value;
        break;
    //is_deleted
    case CAL_PROPERTY_EVENT_IS_DELETED:
        (rec->is_deleted) = value;
        break;
    //freq
    case CAL_PROPERTY_EVENT_FREQ:
		switch (value)
		{
		case CALENDAR_RECURRENCE_NONE:
		case CALENDAR_RECURRENCE_DAILY:
		case CALENDAR_RECURRENCE_WEEKLY:
		case CALENDAR_RECURRENCE_MONTHLY:
		case CALENDAR_RECURRENCE_YEARLY:
	        (rec->freq) = value;
			break;
		default:
			ERR("invalid parameter (value:%d)", value);
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
        break;
    //range_type
    case CAL_PROPERTY_EVENT_RANGE_TYPE:
		switch (value)
		{
		case CALENDAR_RANGE_UNTIL:
		case CALENDAR_RANGE_COUNT:
		case CALENDAR_RANGE_NONE:
	        (rec->range_type) = value;
			break;
		default:
			ERR("invalid parameter (value:%d)", value);
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
        break;
    //count
    case CAL_PROPERTY_EVENT_COUNT:
        (rec->count) = value;
        break;
    //interval
    case CAL_PROPERTY_EVENT_INTERVAL:
        (rec->interval) = value;
        break;
    //wkst
    case CAL_PROPERTY_EVENT_WKST:
		switch (value)
		{
		case CALENDAR_SUNDAY:
		case CALENDAR_MONDAY:
		case CALENDAR_TUESDAY:
		case CALENDAR_WEDNESDAY:
		case CALENDAR_THURSDAY:
		case CALENDAR_FRIDAY:
		case CALENDAR_SATURDAY:
	        (rec->wkst) = value;
			break;
		default:
			ERR("invalid parameter (value:%d)", value);
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
        break;
    //has_attendee
    case CAL_PROPERTY_EVENT_HAS_ATTENDEE:
        (rec->has_attendee) = value;
        break;
    //has_alarm
    case CAL_PROPERTY_EVENT_HAS_ALARM:
        (rec->has_alarm) = value;
        break;
    case CAL_PROPERTY_EVENT_CALENDAR_SYSTEM_TYPE:
		switch (value)
		{
		case CALENDAR_SYSTEM_NONE:
		case CALENDAR_SYSTEM_GREGORIAN:
		case CALENDAR_SYSTEM_EAST_ASIAN_LUNISOLAR:
	        (rec->system_type) = value;
			break;
		default:
			ERR("invalid parameter (value:%d)", value);
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
        break;
    case CAL_PROPERTY_EVENT_IS_ALLDAY:
        (rec->start.type) = value;
        break;
    default:
        ASSERT_NOT_REACHED("invalid parameter (property:%d)",property_id);
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    return CALENDAR_ERROR_NONE;
}

static int __cal_record_event_set_double( calendar_record_h record, unsigned int property_id, double value )
{
    cal_event_s *rec = (cal_event_s*)(record);
    switch( property_id )
    {
    case CAL_PROPERTY_EVENT_LATITUDE:
        (rec->latitude) = value;
        break;
    case CAL_PROPERTY_EVENT_LONGITUDE:
        (rec->longitude) = value;
        break;
    default:
        ASSERT_NOT_REACHED("invalid parameter (property:%d)",property_id);
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    return CALENDAR_ERROR_NONE;
}

static int __cal_record_event_set_lli( calendar_record_h record, unsigned int property_id, long long int value )
{
    cal_event_s *rec = (cal_event_s*)(record);
    switch( property_id )
    {
    case CAL_PROPERTY_EVENT_CREATED_TIME:
        (rec->created_time) = value;
        break;
    case CAL_PROPERTY_EVENT_LAST_MODIFIED_TIME:
        (rec->last_mod) = value;
        break;
    default:
        ASSERT_NOT_REACHED("invalid parameter (property:%d)",property_id);
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    return CALENDAR_ERROR_NONE;
}

static int __cal_record_event_set_caltime( calendar_record_h record, unsigned int property_id, calendar_time_s value )
{
    cal_event_s *rec = (cal_event_s*)(record);
    switch( property_id )
    {
    case CAL_PROPERTY_EVENT_START:
        rec->start = value;
        break;
    case CAL_PROPERTY_EVENT_END:
        rec->end = value;
        break;
    case CAL_PROPERTY_EVENT_UNTIL:
        (rec->until_type) = value.type;
        if(value.type == CALENDAR_TIME_UTIME)
        {
            (rec->until_utime) = value.time.utime;
            (rec->until_year) = 0;
            (rec->until_month) = 0;
            (rec->until_mday) = 0;
        }
        else
        {
            (rec->until_utime) = 0;
            (rec->until_year) = value.time.date.year;
            (rec->until_month) = value.time.date.month;
            (rec->until_mday) = value.time.date.mday ;
        }
        break;
    default:
        ASSERT_NOT_REACHED("invalid parameter (property:%d)",property_id);
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    return CALENDAR_ERROR_NONE;
}

static int __cal_record_event_add_child_record( calendar_record_h record, unsigned int property_id, calendar_record_h child_record )
{
    cal_event_s *rec = (cal_event_s*)(record);

    switch( property_id )
    {
    case CAL_PROPERTY_EVENT_CALENDAR_ALARM:
        rec->alarm_list = g_list_append(rec->alarm_list,child_record);
        rec->has_alarm = 1;
        break;
    case CAL_PROPERTY_EVENT_CALENDAR_ATTENDEE:
        rec->attendee_list = g_list_append(rec->attendee_list,child_record);
        rec->has_attendee = 1;
        break;
    case CAL_PROPERTY_EVENT_EXCEPTION:
        rec->exception_list = g_list_append(rec->exception_list,child_record);
        break;
    case CAL_PROPERTY_EVENT_EXTENDED:
        rec->extended_list = g_list_append(rec->extended_list,child_record);
        break;
    default:
        ASSERT_NOT_REACHED("invalid parameter (property:%d)",property_id);
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    return CALENDAR_ERROR_NONE;
}

static int __cal_record_event_remove_child_record( calendar_record_h record, unsigned int property_id, calendar_record_h child_record )
{
    cal_event_s *rec = (cal_event_s*)(record);
    //GList* node = NULL;

    switch( property_id )
    {
    case CAL_PROPERTY_EVENT_CALENDAR_ALARM:
        rec->alarm_list = g_list_remove(rec->alarm_list,child_record);
        if (g_list_length(rec->alarm_list) == 0)
        {
            rec->has_alarm = 0;
        }
        break;
    case CAL_PROPERTY_EVENT_CALENDAR_ATTENDEE:
        rec->attendee_list = g_list_remove(rec->attendee_list,child_record);
        if (g_list_length(rec->attendee_list) == 0)
        {
            rec->has_attendee = 0;
        }
        break;
    case CAL_PROPERTY_EVENT_EXCEPTION:
        rec->exception_list = g_list_remove(rec->exception_list,child_record);
        break;
    case CAL_PROPERTY_EVENT_EXTENDED:
        rec->extended_list = g_list_remove(rec->extended_list,child_record);
        break;
    default:
        ASSERT_NOT_REACHED("invalid parameter (property:%d)",property_id);
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    return CALENDAR_ERROR_NONE;
}

static int __cal_record_event_get_child_record_count( calendar_record_h record, unsigned int property_id, unsigned int* count  )
{
    cal_event_s *rec = (cal_event_s*)(record);
    *count = 0;

    switch( property_id )
    {
    case CAL_PROPERTY_EVENT_CALENDAR_ALARM:
        if (rec->alarm_list)
        {
            *count = g_list_length(rec->alarm_list);
        }
        break;
    case CAL_PROPERTY_EVENT_CALENDAR_ATTENDEE:
        if (rec->attendee_list)
        {
            *count = g_list_length(rec->attendee_list);
        }
        break;
    case CAL_PROPERTY_EVENT_EXCEPTION:
        if (rec->exception_list)
        {
            *count = g_list_length(rec->exception_list);
        }
        break;
    case CAL_PROPERTY_EVENT_EXTENDED:
        if (rec->extended_list)
        {
            *count = g_list_length(rec->extended_list);
        }
        break;
    default:
        *count = 0;
        ASSERT_NOT_REACHED("invalid parameter (property:%d)",property_id);
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    return CALENDAR_ERROR_NONE;
}

static int __cal_record_event_get_child_record_at_p( calendar_record_h record, unsigned int property_id, int index, calendar_record_h* child_record )
{
    cal_event_s *rec = (cal_event_s*)(record);

    switch( property_id )
    {
    case CAL_PROPERTY_EVENT_CALENDAR_ALARM:
        *child_record = g_list_nth_data(rec->alarm_list,index);
        break;
    case CAL_PROPERTY_EVENT_CALENDAR_ATTENDEE:
        *child_record = g_list_nth_data(rec->attendee_list,index);
        break;
    case CAL_PROPERTY_EVENT_EXCEPTION:
        *child_record = g_list_nth_data(rec->exception_list,index);
        break;
    case CAL_PROPERTY_EVENT_EXTENDED:
        *child_record = g_list_nth_data(rec->extended_list,index);
        break;
    default:
        ASSERT_NOT_REACHED("invalid parameter (property:%d)",property_id);
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    return CALENDAR_ERROR_NONE;
}

static int __cal_record_event_clone_child_record_list( calendar_record_h record, unsigned int property_id, calendar_list_h* out_list )
{
    cal_event_s *rec = (cal_event_s*)(record);
    int ret = CALENDAR_ERROR_NONE;
    calendar_list_h list;
    int count = 0;

    ret = calendar_list_create(&list);
    if (ret != CALENDAR_ERROR_NONE)
    {
        ERR("calendar_list_create fail");
        return ret;
    }

	switch( property_id )
	{
	case CAL_PROPERTY_EVENT_CALENDAR_ALARM:
    {
        count = g_list_length(rec->alarm_list);
        if (count <=0 )
        {
            calendar_list_destroy(list, true);
            return CALENDAR_ERROR_NO_DATA;
        }
        GList *alarm_list = rec->alarm_list;
        calendar_record_h alarm_child_record;
        calendar_record_h alarm_record = NULL;
        while (alarm_list)
        {
            alarm_child_record = (calendar_record_h)alarm_list->data;
            if (alarm_child_record == NULL)
            {
                alarm_list = g_list_next(alarm_list);
                continue;
            }

            if ( calendar_record_clone(alarm_child_record,&alarm_record) == CALENDAR_ERROR_NONE )
            {
                calendar_list_add(list,alarm_record);
            }
            alarm_record = NULL;

            alarm_list = g_list_next(alarm_list);
        }
        *out_list = (calendar_list_h)list;
    }
    break;
	case CAL_PROPERTY_EVENT_CALENDAR_ATTENDEE:
    {
        count = g_list_length(rec->attendee_list);
        if (count <=0 )
        {
            calendar_list_destroy(list, true);
            return CALENDAR_ERROR_NO_DATA;
        }
        GList *attendee_list = rec->attendee_list;
        calendar_record_h attendee_child_record;
        calendar_record_h attendee_record = NULL;
        while (attendee_list)
        {
            attendee_child_record = (calendar_record_h)attendee_list->data;
            if (attendee_child_record == NULL)
            {
                attendee_list = g_list_next(attendee_list);
                continue;
            }

            if ( calendar_record_clone(attendee_child_record,&attendee_record) == CALENDAR_ERROR_NONE )
            {
                calendar_list_add(list,attendee_record);
            }
            attendee_record = NULL;

            attendee_list = g_list_next(attendee_list);
        }
        *out_list = (calendar_list_h)list;
    }
    break;
    case CAL_PROPERTY_EVENT_EXCEPTION:
    {
        count = g_list_length(rec->exception_list);
        if (count <=0 )
        {
            calendar_list_destroy(list, true);
            return CALENDAR_ERROR_NO_DATA;
        }
        GList *exception_list = rec->exception_list;
        calendar_record_h child_record;
        calendar_record_h record = NULL;
        while (exception_list)
        {
            child_record = (calendar_record_h)exception_list->data;
            if (child_record == NULL)
            {
                exception_list = g_list_next(exception_list);
                continue;
            }

            if ( calendar_record_clone(child_record,&record) == CALENDAR_ERROR_NONE )
            {
                calendar_list_add(list,record);
            }
            record = NULL;

            exception_list = g_list_next(exception_list);
        }
        *out_list = (calendar_list_h)list;
    }
    break;
    case CAL_PROPERTY_EVENT_EXTENDED:
    {
        count = g_list_length(rec->extended_list);
        if (count <=0 )
        {
            calendar_list_destroy(list, true);
            return CALENDAR_ERROR_NO_DATA;
        }
        GList *extended_list = rec->extended_list;
        calendar_record_h child_record;
        calendar_record_h record = NULL;
        while (extended_list)
        {
            child_record = (calendar_record_h)extended_list->data;
            if (child_record == NULL)
            {
                extended_list = g_list_next(extended_list);
                continue;
            }

            if ( calendar_record_clone(child_record,&record) == CALENDAR_ERROR_NONE )
            {
                calendar_list_add(list,record);
            }
            record = NULL;

            extended_list = g_list_next(extended_list);
        }
        *out_list = (calendar_list_h)list;
    }
    break;
    default:
        ASSERT_NOT_REACHED("invalid parameter (property:%d)",property_id);
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    return CALENDAR_ERROR_NONE;
}

