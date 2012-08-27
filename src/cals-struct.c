/*
 * Calendar Service
 *
 * Copyright (c) 2000 - 2012 Samsung Electronics Co., Ltd. All rights reserved.
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
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "calendar-svc-provider.h"
#include "cals-internal.h"
#include "cals-typedef.h"
#ifdef CALS_IPC_CLIENT
#else
#include "cals-db.h"
#include "cals-utils.h"
#endif

#ifdef CALS_IPC_CLIENT
static bool __cal_free_participant(cal_participant_info_t* paritcipant_info, int *error_code)
{
    if(NULL == paritcipant_info)
    {
        return true;
    }

    CAL_FREE(paritcipant_info->attendee_email);
    CAL_FREE(paritcipant_info->attendee_number);
    CAL_FREE(paritcipant_info->attendee_name);

    return true;
}

static bool __cal_free_full_record(cal_sch_full_t *sch_full_record, int *error_code)
{
    retex_if(error_code == NULL, ,"error_code is NULL.");
    retex_if(sch_full_record == NULL, *error_code = CAL_ERR_ARG_INVALID,"sch_full_record is NULL.");
    cal_value *value = NULL;
    GList *head;

    CAL_FREE(sch_full_record->summary);
    CAL_FREE(sch_full_record->description);
    CAL_FREE(sch_full_record->location);
    CAL_FREE(sch_full_record->categories);
    CAL_FREE(sch_full_record->uid);
    CAL_FREE(sch_full_record->organizer_name);
    CAL_FREE(sch_full_record->organizer_email);
    CAL_FREE(sch_full_record->gcal_id);
    CAL_FREE(sch_full_record->updated);
    CAL_FREE(sch_full_record->location_summary);
    CAL_FREE(sch_full_record->etag);
    CAL_FREE(sch_full_record->edit_uri);
    CAL_FREE(sch_full_record->gevent_id);

    if (sch_full_record->attendee_list)
    {
        head = sch_full_record->attendee_list;
        while (sch_full_record->attendee_list)
        {
            value = sch_full_record->attendee_list->data;
            if(NULL != value)
            {
                if(NULL != value->user_data)
                {
                    __cal_free_participant((cal_participant_info_t*)value->user_data,error_code);
                    CAL_FREE(value->user_data);

                }
                CAL_FREE(value);
            }
            sch_full_record->attendee_list = sch_full_record->attendee_list->next;
        }
        g_list_free(head);
        sch_full_record->attendee_list = NULL;
    }
    return true;

CATCH:

    return false;
}
#endif

int cals_event_init(cal_sch_full_t *sch_full_record)
{
	retvm_if(NULL == sch_full_record, CAL_ERR_ARG_INVALID , "sch_full_record is NULL");

	memset(sch_full_record,0,sizeof(cal_sch_full_t));

	sch_full_record->cal_type = CALS_SCH_TYPE_EVENT;
	sch_full_record->meeting_status = CALS_EVENT_STATUS_NONE;
	sch_full_record->calendar_id = DEFAULT_EVENT_CALENDAR_ID;

	sch_full_record->index = CALS_INVALID_ID;
	sch_full_record->timezone = -1;
	sch_full_record->contact_id = CALS_INVALID_ID;
	sch_full_record->calendar_type = CAL_PHONE_CALENDAR;
	sch_full_record->attendee_list = NULL;
	sch_full_record->busy_status = 2;
	sch_full_record->summary = NULL;
	sch_full_record->description = NULL;
	sch_full_record->location= NULL;
	sch_full_record->categories = NULL;
	sch_full_record->exdate = NULL;
	sch_full_record->organizer_email = NULL;
	sch_full_record->organizer_name = NULL;
	sch_full_record->uid= NULL;
	sch_full_record->gcal_id = NULL;
	sch_full_record->location_summary = NULL;
	sch_full_record->etag = NULL;
	sch_full_record->edit_uri = NULL;
	sch_full_record->gevent_id = NULL;
	sch_full_record->original_event_id = CALS_INVALID_ID;

	sch_full_record->sync_status = CAL_SYNC_STATUS_NEW;
	sch_full_record->account_id = -1;
	sch_full_record->is_deleted = 0;
	sch_full_record->latitude = 1000; // set default 1000 out of range(-180 ~ 180)
	sch_full_record->longitude = 1000; // set default 1000 out of range(-180 ~ 180)
	sch_full_record->freq = CALS_FREQ_ONCE;
	sch_full_record->until_utime = CALS_TODO_NO_DUE_DATE;

	return CAL_SUCCESS;
}

int cals_todo_init(cal_sch_full_t *sch_full_record)
{
	retvm_if(NULL == sch_full_record, CAL_ERR_ARG_INVALID , "sch_full_record is NULL");

	memset(sch_full_record,0,sizeof(cal_sch_full_t));

	sch_full_record->cal_type = CALS_SCH_TYPE_TODO;
	sch_full_record->task_status = CALS_TODO_STATUS_NONE;
	sch_full_record->calendar_id = DEFAULT_TODO_CALENDAR_ID;

	sch_full_record->index = CALS_INVALID_ID;
	sch_full_record->timezone = -1;
	sch_full_record->contact_id = CALS_INVALID_ID;
	sch_full_record->calendar_type = CAL_PHONE_CALENDAR;
	sch_full_record->attendee_list = NULL;
	sch_full_record->busy_status = 2;
	sch_full_record->summary = NULL;
	sch_full_record->description = NULL;
	sch_full_record->location= NULL;
	sch_full_record->categories = NULL;
	sch_full_record->exdate = NULL;
	sch_full_record->organizer_email = NULL;
	sch_full_record->organizer_name = NULL;
	sch_full_record->uid= NULL;
	sch_full_record->gcal_id = NULL;
	sch_full_record->location_summary = NULL;
	sch_full_record->etag = NULL;
	sch_full_record->edit_uri = NULL;
	sch_full_record->gevent_id = NULL;
	sch_full_record->original_event_id = CALS_INVALID_ID;

	sch_full_record->sync_status = CAL_SYNC_STATUS_NEW;
	sch_full_record->account_id = -1;
	sch_full_record->is_deleted = 0;
	sch_full_record->latitude = 1000; // set default 1000 out of range(-180 ~ 180)
	sch_full_record->longitude = 1000; // set default 1000 out of range(-180 ~ 180)
	sch_full_record->freq = CALS_FREQ_ONCE;
	sch_full_record->until_utime = CALS_TODO_NO_DUE_DATE;

	return CAL_SUCCESS;
}

static inline void cals_init_calendar_record(calendar_t *calendar)
{
	calendar->index = -1;
	calendar->visibility = true;
	calendar->account_id = LOCAL_ACCOUNT_ID;
}

API cal_struct* calendar_svc_struct_new(const char *event_type)
{
	int ret, type;
	void *user_data;
	cal_struct *temp;

	retvm_if(NULL == event_type, NULL, "event_type is NULL");

	if(0 == strcmp(event_type, CAL_STRUCT_SCHEDULE)) {
		type = CAL_STRUCT_TYPE_SCHEDULE;

		user_data = (cal_sch_full_t*)malloc(sizeof(cal_sch_full_t));
		retvm_if(NULL == user_data, NULL, "malloc(cal_sch_full_t:sch) Failed(%d)", errno);

		ret = cals_event_init(user_data);
		if(ret) {
			free(user_data);
			ERR("cals_event_init() Failed(%d)", ret);
			return NULL;
		}

	} else if (0 == strcmp(event_type, CAL_STRUCT_TODO)) {
		type = CAL_STRUCT_TYPE_TODO;

		user_data = (cal_sch_full_t*)calloc(1, sizeof(cal_sch_full_t));
		retvm_if(NULL == user_data, NULL, "calloc(cal_sch_full_t:todo) Failed(%d)", errno);

		ret = cals_todo_init(user_data);
		if(ret) {
			free(user_data);
			ERR("cals_todo_init() Failed(%d)", ret);
			return NULL;
		}

	} else if (0 == strcmp(event_type, CAL_STRUCT_CALENDAR)) {
		type = CAL_STRUCT_TYPE_CALENDAR;

		user_data = calloc(1, sizeof(calendar_t));
		retvm_if(NULL == user_data, NULL, "calloc(calendar_t) Failed(%d)", errno);

		cals_init_calendar_record(user_data);
	} else if (0 == strcmp(event_type, CAL_STRUCT_TIMEZONE))	{
		type = CAL_STRUCT_TYPE_TIMEZONE;

		user_data = (cal_timezone_t*)calloc(1, sizeof(cal_timezone_t));
		retvm_if(NULL == user_data, NULL, "calloc(cal_timezone_t) Failed(%d)", errno);
	} else if(0 == strcmp(event_type, CAL_STRUCT_UPDATED)) {
		type = CAL_STRUCT_TYPE_UPDATED_LIST;

		user_data = (cals_updated*)calloc(1, sizeof(cals_updated));
		retvm_if(NULL == user_data, NULL, "calloc(cals_updated) Failed(%d)", errno);
	} else if(0 == strcmp(event_type, CALS_STRUCT_PERIOD_NORMAL_ONOFF)) {
		type = CALS_STRUCT_TYPE_PERIOD_NORMAL_ONOFF;;
		user_data = (cals_struct_period_normal_onoff*)calloc(1,
				sizeof(cals_struct_period_normal_onoff));
		retvm_if(NULL == user_data, NULL,
				"Failed to calloc PERIOD_NORMAL_ONOFF(%d)", errno);

	} else if(0 == strcmp(event_type, CALS_STRUCT_PERIOD_ALLDAY_ONOFF)) {
		type = CALS_STRUCT_TYPE_PERIOD_ALLDAY_ONOFF;;
		user_data = (cals_struct_period_allday_onoff*)calloc(1,
				sizeof(cals_struct_period_allday_onoff));
		retvm_if(NULL == user_data, NULL,
				"Failed to calloc PERIOD_ALLDAY_ONOFF(%d)", errno);

	} else if(0 == strcmp(event_type, CALS_STRUCT_PERIOD_NORMAL_BASIC)) {
		type = CALS_STRUCT_TYPE_PERIOD_NORMAL_BASIC;
		user_data = (cals_struct_period_normal_basic*)calloc(1,
				sizeof(cals_struct_period_normal_basic));
		retvm_if(NULL == user_data, NULL,
				"Failed to calloc CAL_STRUCT_PERIOD_NORMAL_BASIC(%d)", errno);

	} else if(0 == strcmp(event_type, CALS_STRUCT_PERIOD_ALLDAY_BASIC)) {
		type = CALS_STRUCT_TYPE_PERIOD_ALLDAY_BASIC;
		user_data = (cals_struct_period_allday_basic*)calloc(1,
				sizeof(cals_struct_period_allday_basic));
		retvm_if(NULL == user_data, NULL,
				"Failed to calloc CAL_STRUCT_PERIOD_ALLDAY_BASIC(%d)", errno);

	} else if(0 == strcmp(event_type, CALS_STRUCT_PERIOD_NORMAL_OSP)) {
		type = CALS_STRUCT_TYPE_PERIOD_NORMAL_OSP;
		user_data = (cals_struct_period_normal_osp*)calloc(1,
				sizeof(cals_struct_period_normal_osp));
		retvm_if(NULL == user_data, NULL,
				"Failed to calloc CAL_STRUCT_PERIOD_NORMAL_BASIC(%d)", errno);

	} else if(0 == strcmp(event_type, CALS_STRUCT_PERIOD_ALLDAY_OSP)) {
		type = CALS_STRUCT_TYPE_PERIOD_ALLDAY_OSP;
		user_data = (cals_struct_period_allday_osp*)calloc(1,
				sizeof(cals_struct_period_allday_osp));
		retvm_if(NULL == user_data, NULL,
				"Failed to calloc CAL_STRUCT_PERIOD_ALLDAY_BASIC(%d)", errno);

	} else if(0 == strcmp(event_type, CALS_STRUCT_PERIOD_NORMAL_LOCATION)) {
		type = CALS_STRUCT_TYPE_PERIOD_NORMAL_LOCATION;
		user_data = (cals_struct_period_normal_location*)calloc(1,
				sizeof(cals_struct_period_normal_location));
		retvm_if(NULL == user_data, NULL,
				"Failed to calloc CAL_STRUCT_PERIOD_NORMAL_LOCATION(%d)", errno);

	} else if(0 == strcmp(event_type, CALS_STRUCT_PERIOD_ALLDAY_LOCATION)) {
		type = CALS_STRUCT_TYPE_PERIOD_ALLDAY_LOCATION;
		user_data = (cals_struct_period_allday_location*)calloc(1,
				sizeof(cals_struct_period_allday_location));
		retvm_if(NULL == user_data, NULL,
				"Failed to calloc CAL_STRUCT_PERIOD_ALLDAY_BASIC(%d)", errno);

	} else if(0 == strcmp(event_type, CALS_STRUCT_PERIOD_NORMAL_ALARM)) {
		type = CALS_STRUCT_TYPE_PERIOD_NORMAL_ALARM;
		user_data = (cals_struct_period_normal_alarm*)calloc(1,
				sizeof(cals_struct_period_normal_alarm));
		retvm_if(NULL == user_data, NULL,
				"Failed to calloc CAL_STRUCT_PERIOD_NORMAL_ALARM(%d)", errno);

	} else {
		ERR("Unknown type(%s)", event_type);
		return NULL;
	}

	temp = (cal_struct*)calloc(1, sizeof(cal_struct));
	if(NULL == temp) {
		free(user_data);
		ERR("calloc(cal_struct) Failed(%d)", errno);
		return NULL;
	}
	temp->event_type = type;
	temp->user_data = user_data;

	return temp;
}

//API to free the calendar struct
static inline void cals_free_calendar_record(calendar_t *calendar)
{
	retm_if(calendar == NULL, "calendar is NULL");

	free(calendar->calendar_id);
	free(calendar->uid);
	free(calendar->link);
	free(calendar->name);
	free(calendar->description);
	free(calendar->author);
	free(calendar->color);
	free(calendar->location);
	free(calendar->timezone_label);
	free(calendar->user_location);
	free(calendar->weather);

	free(calendar);
}

static inline void cals_free_timezone_record(cal_timezone_t *tz_info)
{
	retm_if(NULL == tz_info, "tz_info is NULL");

	free(tz_info->standard_name);
	free(tz_info->day_light_name);

	free(tz_info);
}

API int calendar_svc_struct_free (cal_struct** event)
{
	int ret = 0;

	retvm_if(NULL == event || NULL == *event, CAL_ERR_ARG_INVALID, "Invalid parameter");

	switch((*event)->event_type)
	{
	case CAL_STRUCT_TYPE_SCHEDULE:
	case CAL_STRUCT_TYPE_TODO:
#ifdef CALS_IPC_CLIENT
	    __cal_free_full_record((cal_sch_full_t*)(*event)->user_data,&ret);
#else
		cal_db_service_free_full_record((cal_sch_full_t*)(*event)->user_data);
#endif
		CAL_FREE((*event)->user_data);
		CAL_FREE(*event);
		break;
	case CAL_STRUCT_TYPE_CALENDAR:
		cals_free_calendar_record((calendar_t*)(*event)->user_data);
		CAL_FREE(*event);
		break;
	case CAL_STRUCT_TYPE_TIMEZONE:
		cals_free_timezone_record((cal_timezone_t*)(*event)->user_data);
		CAL_FREE(*event);
		break;
	case CAL_STRUCT_TYPE_UPDATED_LIST:
		CAL_FREE((*event)->user_data);
		CAL_FREE(*event);
		break;
	default:
		ERR("Unknown type(%d)", (*event)->event_type);
		break;
	}

	return CAL_SUCCESS;
}

API char * calendar_svc_struct_get_str (cal_struct *event,const char *field)
{
	//CALS_FN_CALL();
	retvm_if(NULL == event || NULL == event->user_data || NULL == field,
			NULL,"Invalid parameters.");

	cal_sch_full_t * sch_rec = NULL;
	calendar_t * cal_rec = NULL;
	cal_timezone_t *tz_rec = NULL;
	cals_struct_period_normal_basic *nbs = NULL;
	cals_struct_period_allday_basic *abs = NULL;
	cals_struct_period_normal_osp *nosp = NULL;
	cals_struct_period_allday_osp *aosp = NULL;
	cals_struct_period_normal_location *nosl = NULL;
	cals_struct_period_allday_location *aosl = NULL;

	switch(event->event_type)
	{
	case CAL_STRUCT_TYPE_SCHEDULE:
	case CAL_STRUCT_TYPE_TODO:
		sch_rec = (cal_sch_full_t*)event->user_data;
		if(0 == strcmp(field,CAL_VALUE_TXT_SUMMARY))
		{
			return (sch_rec->summary);
		}
		else if(0 == strcmp(field,CAL_VALUE_TXT_DESCRIPTION))
		{
			return (sch_rec->description);
		}
		else if(0 == strcmp(field,CAL_VALUE_TXT_LOCATION))
		{
			return (sch_rec->location);
		}
		else if(0 == strcmp(field, CAL_VALUE_TXT_CATEGORIES))
		{
			return (sch_rec->categories);
		}
		else if(0 == strcmp(field, CAL_VALUE_TXT_EXDATE))
		{
			return (sch_rec->exdate);
		}
		else if(0 == strcmp(field,CAL_VALUE_TXT_UID))
		{
			return (sch_rec->uid);
		}
		else if(0 == strcmp(field,CAL_VALUE_TXT_ORGANIZER_NAME))
		{
			return (sch_rec->organizer_name);
		}
		else if(0 == strcmp(field,CAL_VALUE_TXT_ORGANIZER_EMAIL))
		{
			return (sch_rec->organizer_email);
		}
		else if(0 == strcmp(field,CAL_VALUE_TXT_GCAL_ID))
		{
			return (sch_rec->gcal_id);
		}
		else if(0 == strcmp(field,CAL_VALUE_TXT_UPDATED))
		{
			return (sch_rec->updated);
		}
		else if(0 == strcmp(field,CAL_VALUE_TXT_LOCATION_SUMMARY))
		{
			return (sch_rec->location_summary);
		}
		else if(0 == strcmp(field,CAL_VALUE_TXT_ETAG))
		{
			return (sch_rec->etag);
		}
		else if(0 == strcmp(field,CAL_VALUE_TXT_EDIT_URL))
		{
			return (sch_rec->edit_uri);
		}
		else if(0 == strcmp(field,CAL_VALUE_TXT_GEDERID))
		{
			return (sch_rec->gevent_id);
		}
		else if(0 == strcmp(field, CALS_VALUE_TXT_DTSTART_TZID))
			return (sch_rec->dtstart_tzid);
		else if(0 == strcmp(field, CALS_VALUE_TXT_DTEND_TZID))
			return (sch_rec->dtend_tzid);
		else if(0 == strcmp(field, CALS_VALUE_TXT_RRULE_BYSECOND))
			return (sch_rec->bysecond);
		else if(0 == strcmp(field, CALS_VALUE_TXT_RRULE_BYMINUTE))
			return (sch_rec->byminute);
		else if(0 == strcmp(field, CALS_VALUE_TXT_RRULE_BYHOUR))
			return (sch_rec->byhour);
		else if(0 == strcmp(field, CALS_VALUE_TXT_RRULE_BYDAY))
			return (sch_rec->byday);
		else if(0 == strcmp(field, CALS_VALUE_TXT_RRULE_BYMONTHDAY))
			return (sch_rec->bymonthday);
		else if(0 == strcmp(field, CALS_VALUE_TXT_RRULE_BYYEARDAY))
			return (sch_rec->byyearday);
		else if(0 == strcmp(field, CALS_VALUE_TXT_RRULE_BYWEEKNO))
			return (sch_rec->byweekno);
		else if(0 == strcmp(field, CALS_VALUE_TXT_RRULE_BYMONTH))
			return (sch_rec->bymonth);
		else if(0 == strcmp(field, CALS_VALUE_TXT_RRULE_BYSETPOS))
			return (sch_rec->bysetpos);
		else
		{
			ERR("Can not find the field(%s)!", field);
			return NULL;
		}
		break;

	case CAL_STRUCT_TYPE_CALENDAR:
		cal_rec = (calendar_t*)event->user_data;
		if(0 == strcmp(field,CAL_TABLE_TXT_CALENDAR_ID))
		{
			return(cal_rec->calendar_id);
		}
		else if(0 == strcmp(field,CAL_TABLE_TXT_UID))
		{
			return(cal_rec->uid);
		}
		else if(0 == strcmp(field,CAL_TABLE_TXT_LINK))
		{
			return(cal_rec->link);
		}
		else if(0 == strcmp(field,CAL_TABLE_TXT_NAME))
		{
			return(cal_rec->name);
		}
		else if(0 == strcmp(field,CAL_TABLE_TXT_DESCRIPTION))
		{
			return(cal_rec->description);
		}
		else if(0 == strcmp(field,CAL_TABLE_TXT_AUTHOR))
		{
			return(cal_rec->author);
		}
		else if(0 == strcmp(field,CAL_TABLE_TXT_COLOR))
		{
			return(cal_rec->color);
		}
		else if(0 == strcmp(field,CAL_TABLE_TXT_LOCATION))
		{
			return(cal_rec->location);
		}
		else if(0 == strcmp(field,CAL_TABLE_TXT_TIME_ZONE_LABEL))
		{
			return(cal_rec->timezone_label);
		}
		else if(0 == strcmp(field,CAL_TABLE_TXT_USER_LOCATION))
		{
			return(cal_rec->user_location);
		}
		else if(0 == strcmp(field,CAL_TABLE_TXT_WEATHER))
		{
			return(cal_rec->weather);
		}
		else
		{
			ERR("Can not find the field!(%s)",field);
			return NULL;
		}

		break;
	case CAL_STRUCT_TYPE_TIMEZONE:
		tz_rec = (cal_timezone_t*)event->user_data;
		if(0 == strcmp(field,CAL_TZ_VALUE_TXT_STD_NAME))
		{
			return(tz_rec->standard_name);
		}
		else if(0 == strcmp(field,CAL_TZ_VALUE_TXT_DST_NAME))
		{
			return(tz_rec->day_light_name);
		}else {
			ERR("Can not find the field!(%s)",field);
			return NULL;
		}
		break;

	case CALS_STRUCT_TYPE_PERIOD_NORMAL_ONOFF:
		ERR("No field in CALS_LIST_PERIOD_NORMAL_ONOFF");
		return NULL;

	case CALS_STRUCT_TYPE_PERIOD_ALLDAY_ONOFF:
		ERR("No field in CALS_LIST_PERIOD_ALLDAY_ONOFF");
		return NULL;

	case CALS_STRUCT_TYPE_PERIOD_NORMAL_BASIC:
		nbs = event->user_data;
		if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_BASIC_TXT_SUMMARY))
			return nbs->summary;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_BASIC_TXT_LOCATION))
			return nbs->location;
		else {
			ERR("Can not find the field!(%s)",field);
			return NULL;
		}
		break;

	case CALS_STRUCT_TYPE_PERIOD_ALLDAY_BASIC:
		abs = event->user_data;
		if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_BASIC_TXT_SUMMARY))
			return abs->summary;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_BASIC_TXT_LOCATION))
			return abs->location;
		else {
			ERR("Can not find the field!(%s)", field);
			return NULL;
		}
		break;

	case CALS_STRUCT_TYPE_PERIOD_NORMAL_OSP:
		nosp = event->user_data;
		if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_OSP_TXT_SUMMARY))
			return nosp->summary;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_OSP_TXT_LOCATION))
			return nosp->location;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_OSP_TXT_DESCRIPTION))
			return nosp->description;
		else {
			ERR("Can not find the field!(%s)", field);
			return NULL;
		}
		break;

	case CALS_STRUCT_TYPE_PERIOD_ALLDAY_OSP:
		aosp = event->user_data;
		if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_OSP_TXT_SUMMARY))
			return aosp->summary;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_OSP_TXT_LOCATION))
			return aosp->location;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_OSP_TXT_DESCRIPTION))
			return aosp->description;
		else {
			ERR("Can not find the field!(%s)", field);
			return NULL;
		}
		break;

	case CALS_STRUCT_TYPE_PERIOD_NORMAL_LOCATION:
		nosl = event->user_data;
		if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_LOCATION_TXT_SUMMARY))
			return nosl->summary;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_LOCATION_TXT_LOCATION))
			return nosl->location;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_LOCATION_TXT_DESCRIPTION))
			return nosl->description;
		else {
			ERR("Can not find the field!(%s)", field);
			return NULL;
		}
		break;

	case CALS_STRUCT_TYPE_PERIOD_ALLDAY_LOCATION:
		aosl = event->user_data;
		if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_LOCATION_TXT_SUMMARY))
			return aosl->summary;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_LOCATION_TXT_LOCATION))
			return aosl->location;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_LOCATION_TXT_DESCRIPTION))
			return aosl->description;
		else {
			ERR("Can not find the field!(%s)", field);
			return NULL;
		}
		break;

	default:
		ERR("Can not find the field!(%s)!", field);
		return NULL;
	}
}

API int calendar_svc_struct_get_int(cal_struct *event, const char *field)
{
	cal_sch_full_t * sch_rec = NULL;
	cals_updated *update = NULL;
	calendar_t * cal_rec = NULL;
	cal_timezone_t *tz_rec = NULL;
	cals_struct_period_normal_onoff *nof = NULL;
	cals_struct_period_allday_onoff *aof = NULL;
	cals_struct_period_normal_basic *nbs = NULL;
	cals_struct_period_allday_basic *abs = NULL;
	cals_struct_period_normal_osp *nosp = NULL;
	cals_struct_period_allday_osp *aosp = NULL;
	cals_struct_period_normal_location *nosl = NULL;
	cals_struct_period_allday_location *aosl = NULL;
	cals_struct_period_normal_alarm *nosa = NULL;

	retvm_if(NULL == event || NULL==event->user_data || NULL == field, 0,
				"Invalid parameters(event(%p), field(%p))", event, field);
	switch(event->event_type)
	{
	case CAL_STRUCT_TYPE_SCHEDULE:
	case CAL_STRUCT_TYPE_TODO:
		sch_rec = event->user_data;
		if(0 == strcmp(field,CAL_VALUE_INT_INDEX))
		{
			return sch_rec->index;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_ACCOUNT_ID))
		{
			return sch_rec->account_id;
		}
		else if(0 == strcmp(field, CAL_VALUE_INT_TYPE))
		{
			return sch_rec->cal_type;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_FILE_ID))
		{
			return sch_rec->file_id;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_CONTACT_ID))
		{
			return sch_rec->contact_id;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_BUSY_STATUS))
		{
			return sch_rec->busy_status;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_SENSITIVITY))
		{
			return sch_rec->sensitivity;
		}
		else if(0 == strcmp(field, CAL_VALUE_INT_CALENDAR_TYPE))
		{
			return sch_rec->calendar_type;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_MEETING_STATUS))
		{
			return sch_rec->meeting_status;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_LOCATION_TYPE))
		{
			return sch_rec->location_type;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_CALENDAR_ID))
		{
			return sch_rec->calendar_id;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_DST))
		{
			return sch_rec->dst;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_ORIGINAL_EVENT_ID))
		{
			return sch_rec->original_event_id;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_SYNC_STATUS))
		{
			return sch_rec->sync_status;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_PRIORITY))
		{
			return sch_rec->priority;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_TASK_STATUS))
		{
			return sch_rec->task_status;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_TIMEZONE))
		{
			return sch_rec->timezone;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_EMAIL_ID))
		{
			return sch_rec->email_id;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_AVAILABILITY))
		{
			return sch_rec->availability;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_DELETED))
		{
			return 0;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_PROGRESS))
		{
			return sch_rec->progress;
		}
		else if(0 == strcmp(field, CALS_VALUE_INT_DTSTART_TYPE))
			return sch_rec->dtstart_type;
		else if(0 == strcmp(field, CALS_VALUE_INT_DTSTART_YEAR))
			return sch_rec->dtstart_year;
		else if(0 == strcmp(field, CALS_VALUE_INT_DTSTART_MONTH))
			return sch_rec->dtstart_month;
		else if(0 == strcmp(field, CALS_VALUE_INT_DTSTART_MDAY))
			return sch_rec->dtstart_mday;
		else if(0 == strcmp(field, CALS_VALUE_INT_DTEND_TYPE))
			return sch_rec->dtend_type;
		else if(0 == strcmp(field, CALS_VALUE_INT_DTEND_YEAR))
			return sch_rec->dtend_year;
		else if(0 == strcmp(field, CALS_VALUE_INT_DTEND_MONTH))
			return sch_rec->dtend_month;
		else if(0 == strcmp(field, CALS_VALUE_INT_DTEND_MDAY))
			return sch_rec->dtend_mday;
		else if(0 == strcmp(field, CALS_VALUE_INT_RRULE_FREQ))
			return sch_rec->freq;
		else if(0 == strcmp(field, CALS_VALUE_INT_RRULE_ID))
			return sch_rec->rrule_id;
		else if(0 == strcmp(field, CALS_VALUE_INT_RRULE_RANGE_TYPE))
			return sch_rec->range_type;
		else if(0 == strcmp(field, CALS_VALUE_INT_RRULE_UNTIL_TYPE))
			return sch_rec->until_type;
		else if(0 == strcmp(field, CALS_VALUE_INT_RRULE_UNTIL_YEAR))
			return sch_rec->until_year;
		else if(0 == strcmp(field, CALS_VALUE_INT_RRULE_UNTIL_MONTH))
			return sch_rec->until_month;
		else if(0 == strcmp(field, CALS_VALUE_INT_RRULE_UNTIL_MDAY))
			return sch_rec->until_mday;
		else if(0 == strcmp(field, CALS_VALUE_INT_RRULE_COUNT))
			return sch_rec->count;
		else if(0 == strcmp(field, CALS_VALUE_INT_RRULE_INTERVAL))
			return sch_rec->interval;
		else if(0 == strcmp(field, CALS_VALUE_INT_RRULE_WKST))
			return sch_rec->wkst;
		/* deprecated start >>>>>>>>>>>>>>>>>>>>>>>>>*/
		else if(0 == strcmp(field, "index"))
		{
			return sch_rec->index;
		}
		else if(0 == strcmp(field, CAL_VALUE_INT_CAL_TYPE))
		{
			return sch_rec->cal_type;
		}
		else
		{
			ERR("Can not find the field(%s)",field);
		}
		break;
	case CAL_STRUCT_TYPE_CALENDAR:
		cal_rec = event->user_data;

		if(0 == strcmp(field,CAL_TABLE_INT_INDEX))
		{
			return cal_rec->index;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_UPDATED))
		{
			return cal_rec->updated;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_HIDDEN))
		{
			return cal_rec->hidden;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_SELECTED))
		{
			return cal_rec->selected;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_LOCALE))
		{
			return cal_rec->locale;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_COUNTRY))
		{
			return cal_rec->country;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_TIME_ZONE))
		{
			return cal_rec->time_zone;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_DISPLAY_ALL_TIMEZONES))
		{
			return cal_rec->display_all_timezones;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_DATE_FIELD_ORDER))
		{
			return cal_rec->date_field_order;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_FROMAT_24HOUR_TIME))
		{
			return cal_rec->format_24hour_time;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_WEEK_START))
		{
			return cal_rec->week_start;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_DEFAULT_CAL_MODE))
		{
			return cal_rec->default_cal_mode;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_CUSTOM_CAL_MODE))
		{
			return cal_rec->custom_cal_mode;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_SHOW_DECLINED_EVENTS))
		{
			return cal_rec->show_declined_events;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_HIDE_INVITATIONS))
		{
			return cal_rec->hide_invitations;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_ALTERNATE_CALENDAR))
		{
			return cal_rec->alternate_calendar;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_VISIBILITY))
		{
			return cal_rec->visibility;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_PROJECTION))
		{
			return cal_rec->projection;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_SEQUENCE))
		{
			return cal_rec->sequence;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_SUPRESS_REPLY_NOTIFICATIONS))
		{
			return cal_rec->suppress_reply_notifications;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_SYNC_EVENT))
		{
			return cal_rec->sync_event;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_TIMES_CLEANED))
		{
			return cal_rec->times_cleaned;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_GUESTS_CAN_MODIFY))
		{
			return cal_rec->guests_can_modify;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_GUESTS_CAN_INVITE_OTHERS))
		{
			return cal_rec->guests_can_invite_others;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_GUESTS_CAN_SEE_GUESTS))
		{
			return cal_rec->guests_can_see_guests;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_ACCESS_LEVEL))
		{
			return cal_rec->access_level;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_SYNC_STATUS))
		{
			return cal_rec->sync_status;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_ACCOUNT_ID))
		{
			return cal_rec->account_id;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_SENSITIVITY))
		{
			return cal_rec->sensitivity;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_STORE_TYPE))
		{
			return cal_rec->store_type;
		}
		else
		{
			ERR("Can not find the field(%s)",field);
		}
		break;
	case CAL_STRUCT_TYPE_TIMEZONE:
		tz_rec = event->user_data;
		if(0 == strcmp(field,CAL_TZ_VALUE_INT_INDEX))
		{
			return tz_rec->index;
		}
		else if(0 == strcmp(field,CAL_TZ_VALUE_INT_TZ_OFFSET))
		{
			return tz_rec->tz_offset_from_gmt;
		}
		else if(0 == strcmp(field,CAL_TZ_VALUE_INT_STD_START_MONTH))
		{
			return tz_rec->std_start_month;
		}
		else if(0 == strcmp(field,CAL_TZ_VALUE_INT_STD_START_POSITION_OF_WEEK))
		{
			return tz_rec->std_start_position_of_week;
		}
		else if(0 == strcmp(field,CAL_TZ_VALUE_INT_STD_START_DAY))
		{
			return tz_rec->std_start_day;
		}
		else if(0 == strcmp(field,CAL_TZ_VALUE_INT_STD_START_HOUR))
		{
			return tz_rec->std_start_hour;
		}
		else if(0 == strcmp(field,CAL_TZ_VALUE_INT_STD_BIAS))
		{
			return tz_rec->standard_bias;
		}
		else if(0 == strcmp(field,CAL_TZ_VALUE_INT_DST_START_MONTH))
		{
			return tz_rec->day_light_start_month;
		}
		else if(0 == strcmp(field,CAL_TZ_VALUE_INT_DST_START_POSITION_OF_WEEK))
		{
			return tz_rec->day_light_start_position_of_week;
		}
		else if(0 == strcmp(field,CAL_TZ_VALUE_INT_DST_START_DAY))
		{
			return tz_rec->day_light_start_day;
		}
		else if(0 == strcmp(field,CAL_TZ_VALUE_INT_DST_START_HOUR))
		{
			return tz_rec->day_light_start_hour;
		}
		else if(0 == strcmp(field,CAL_TZ_VALUE_INT_DST_BIAS))
		{
			return tz_rec->day_light_bias;
		}

		break;
	case CAL_STRUCT_TYPE_UPDATED_LIST:
		update = event->user_data;
		if(0 == strcmp(field, CALS_STRUCT_UPDATED_INT_TYPE))
		{
			return update->type;
		}
		else if(0 == strcmp(field, CALS_STRUCT_UPDATED_INT_ID))
		{
			return update->id;
		}
		else if(0 == strcmp(field, CALS_STRUCT_UPDATED_INT_VERSION))
		{
			return update->ver;
		}
		else if(0 == strcmp(field, CALS_STRUCT_UPDATED_INT_CALENDAR_ID))
		{
			return update->calendar_id;
		}
		else
		{
			ERR("Can not find the field(%s)",field);
		}
		break;
	case CALS_STRUCT_TYPE_PERIOD_NORMAL_ONOFF:
		nof = event->user_data;
		if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_ONOFF_INT_EVENTID))
			return nof->index;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_ONOFF_INT_DTSTART_TYPE))
			return nof->dtstart_type;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_ONOFF_INT_DTEND_TYPE))
			return nof->dtstart_type;
		else
			ERR("Can't find field(%s)", field);

		break;

	case CALS_STRUCT_TYPE_PERIOD_ALLDAY_ONOFF:
		aof = event->user_data;
		if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_ONOFF_INT_EVENTID))
			return aof->index;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_ONOFF_INT_DTSTART_TYPE))
			return nof->dtstart_type;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_ONOFF_INT_DTSTART_YEAR))
			return aof->dtstart_year;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_ONOFF_INT_DTSTART_MONTH))
			return aof->dtstart_month;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_ONOFF_INT_DTSTART_MDAY))
			return aof->dtstart_mday;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_ONOFF_INT_DTEND_TYPE))
			return nof->dtend_type;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_ONOFF_INT_DTEND_YEAR))
			return aof->dtend_year;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_ONOFF_INT_DTEND_MONTH))
			return aof->dtend_month;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_ONOFF_INT_DTEND_MDAY))
			return aof->dtend_mday;
		else
			ERR("Can't find field(%s)", field);

		break;

	case CALS_STRUCT_TYPE_PERIOD_NORMAL_BASIC:
		nbs = event->user_data;
		if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_BASIC_INT_EVENTID))
			return nbs->index;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_BASIC_INT_DTSTART_TYPE))
			return nbs->dtstart_type;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_BASIC_INT_DTEND_TYPE))
			return nbs->dtstart_type;
		else
			ERR("Can't find field(%s)", field);

		break;

	case CALS_STRUCT_TYPE_PERIOD_ALLDAY_BASIC:
		abs = event->user_data;
		if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_BASIC_INT_EVENTID))
			return abs->index;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_BASIC_INT_DTSTART_TYPE))
			return abs->dtstart_type;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_BASIC_INT_DTSTART_YEAR))
			return abs->dtstart_year;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_BASIC_INT_DTSTART_MONTH))
			return abs->dtstart_month;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_BASIC_INT_DTSTART_MDAY))
			return abs->dtstart_mday;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_BASIC_INT_DTEND_TYPE))
			return abs->dtend_type;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_BASIC_INT_DTEND_YEAR))
			return abs->dtend_year;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_BASIC_INT_DTEND_MONTH))
			return abs->dtend_month;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_BASIC_INT_DTEND_MDAY))
			return abs->dtend_mday;
		else
			ERR("Can't find field(%s)", field);

		break;

	case CALS_STRUCT_TYPE_PERIOD_NORMAL_OSP:
		nosp = event->user_data;
		if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_OSP_INT_EVENTID))
			return nosp->index;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_OSP_INT_DTSTART_TYPE))
			return nosp->dtstart_type;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_OSP_INT_DTEND_TYPE))
			return nosp->dtend_type;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_OSP_INT_CALENDAR_ID))
			return nosp->calendar_id;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_OSP_INT_BUSY_STATUS))
			return nosp->busy_status;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_OSP_INT_STATUS))
			return nosp->meeting_status;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_OSP_INT_PRIORITY))
			return nosp->priority;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_OSP_INT_VISIBILITY))
			return nosp->sensitivity;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_OSP_INT_IS_RECURRING))
			return nosp->rrule_id;
		else
			ERR("Can't find field(%s)", field);

		break;

	case CALS_STRUCT_TYPE_PERIOD_ALLDAY_OSP:
		aosp = event->user_data;
		if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_OSP_INT_EVENTID))
			return aosp->index;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_OSP_INT_DTSTART_TYPE))
			return aosp->dtstart_type;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_OSP_INT_DTSTART_YEAR))
			return aosp->dtstart_year;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_OSP_INT_DTSTART_MONTH))
			return aosp->dtstart_month;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_OSP_INT_DTSTART_MDAY))
			return aosp->dtstart_mday;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_OSP_INT_DTEND_TYPE))
			return aosp->dtend_type;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_OSP_INT_DTEND_YEAR))
			return aosp->dtend_year;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_OSP_INT_DTEND_MONTH))
			return aosp->dtend_month;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_OSP_INT_DTEND_MDAY))
			return aosp->dtend_mday;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_OSP_INT_CALENDAR_ID))
			return aosp->calendar_id;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_OSP_INT_BUSY_STATUS))
			return aosp->busy_status;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_OSP_INT_STATUS))
			return aosp->meeting_status;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_OSP_INT_PRIORITY))
			return aosp->priority;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_OSP_INT_VISIBILITY))
			return aosp->sensitivity;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_OSP_INT_IS_RECURRING))
			return aosp->rrule_id;
		else
			ERR("Can't find field(%s)", field);
		break;

	case CALS_STRUCT_TYPE_PERIOD_NORMAL_LOCATION:
		nosl = event->user_data;
		if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_LOCATION_INT_EVENTID))
			return nosl->index;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_LOCATION_INT_DTSTART_TYPE))
			return nosl->dtstart_type;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_LOCATION_INT_DTEND_TYPE))
			return nosl->dtend_type;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_LOCATION_INT_CALENDAR_ID))
			return nosl->calendar_id;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_LOCATION_INT_BUSY_STATUS))
			return nosl->busy_status;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_LOCATION_INT_STATUS))
			return nosl->meeting_status;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_LOCATION_INT_PRIORITY))
			return nosl->priority;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_LOCATION_INT_VISIBILITY))
			return nosl->sensitivity;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_LOCATION_INT_IS_RECURRING))
			return nosl->rrule_id;
		else
			ERR("Can't find field(%s)", field);

		break;

	case CALS_STRUCT_TYPE_PERIOD_ALLDAY_LOCATION:
		aosl = event->user_data;
		if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_LOCATION_INT_EVENTID))
			return aosl->index;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_LOCATION_INT_DTSTART_TYPE))
			return aosl->dtstart_type;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_LOCATION_INT_DTSTART_YEAR))
			return aosl->dtstart_year;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_LOCATION_INT_DTSTART_MONTH))
			return aosl->dtstart_month;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_LOCATION_INT_DTSTART_MDAY))
			return aosl->dtstart_mday;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_LOCATION_INT_DTEND_TYPE))
			return aosl->dtend_type;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_LOCATION_INT_DTEND_YEAR))
			return aosl->dtend_year;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_LOCATION_INT_DTEND_MONTH))
			return aosl->dtend_month;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_LOCATION_INT_DTEND_MDAY))
			return aosl->dtend_mday;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_LOCATION_INT_CALENDAR_ID))
			return aosl->calendar_id;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_LOCATION_INT_BUSY_STATUS))
			return aosl->busy_status;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_LOCATION_INT_STATUS))
			return aosl->meeting_status;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_LOCATION_INT_PRIORITY))
			return aosl->priority;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_LOCATION_INT_VISIBILITY))
			return aosl->sensitivity;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_LOCATION_INT_IS_RECURRING))
			return aosl->rrule_id;
		else
			ERR("Can't find field(%s)", field);
		break;

	case CALS_STRUCT_TYPE_PERIOD_NORMAL_ALARM:
		nosa = event->user_data;
		if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_ALARM_INT_EVENTID))
			return nosa->index;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_ALARM_INT_DTSTART_TYPE))
			return nosa->dtstart_type;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_ALARM_INT_DTEND_TYPE))
			return nosa->dtend_type;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_ALARM_INT_CALENDAR_ID))
			return nosa->calendar_id;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_ALARM_INT_ALARM_ID))
			return nosa->alarm_id;
		else
			ERR("Can't find field(%s)", field);
		break;

	default:
		break;

	}

	return 0;
}

API int calendar_svc_struct_set_int (cal_struct *event, const char *field, int intval)
{
	cal_sch_full_t * sch_rec = NULL;
	calendar_t * cal_rec = NULL;
	cal_timezone_t *tz_rec = NULL;
	cals_struct_period_normal_onoff *nof = NULL;
	cals_struct_period_allday_onoff *aof = NULL;
	cals_struct_period_normal_basic *nbs = NULL;
	cals_struct_period_allday_basic *abs = NULL;
	cals_struct_period_normal_osp *nosp = NULL;
	cals_struct_period_allday_osp *aosp = NULL;
	cals_struct_period_normal_location *nosl = NULL;
	cals_struct_period_allday_location *aosl = NULL;
	cals_struct_period_normal_alarm *nosa = NULL;

	retv_if(NULL == event, CAL_ERR_ARG_NULL);
	retv_if(NULL == field, CAL_ERR_ARG_NULL);

	switch(event->event_type)
	{
	case CAL_STRUCT_TYPE_SCHEDULE:
	case CAL_STRUCT_TYPE_TODO:
		sch_rec = event->user_data;
		if(0 == strcmp(field,CAL_VALUE_INT_INDEX))
		{
			sch_rec->index = intval;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_ACCOUNT_ID))
		{
			sch_rec->account_id = intval;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_FILE_ID))
		{
			sch_rec->file_id = intval;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_CONTACT_ID))
		{
			sch_rec->contact_id = intval;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_BUSY_STATUS))
		{
			sch_rec->busy_status = intval;
		}
		else if(0 == strcmp(field, CAL_VALUE_INT_SENSITIVITY))
		{
			sch_rec->sensitivity = intval;
		}
		else if(0 == strcmp(field, CAL_VALUE_INT_CALENDAR_TYPE))
		{
			sch_rec->calendar_type = intval;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_MEETING_STATUS))
		{
			sch_rec->meeting_status = intval;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_LOCATION_TYPE))
		{
			sch_rec->location_type = intval;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_CALENDAR_ID))
		{
			sch_rec->calendar_id = intval;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_DST))
		{
			sch_rec->dst = intval;
		}
		else if(0 == strcmp(field, CAL_VALUE_INT_ORIGINAL_EVENT_ID))
		{
			sch_rec->original_event_id = intval;
		}
		else if(0 == strcmp(field, CAL_VALUE_INT_PRIORITY))
		{
			sch_rec->priority = intval;
		}
		else if(0 == strcmp(field, CAL_VALUE_INT_SYNC_STATUS))
		{
			sch_rec->sync_status = intval;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_TASK_STATUS))
		{
			sch_rec->task_status= intval;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_TIMEZONE))
		{
			sch_rec->timezone= intval;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_EMAIL_ID))
		{
			sch_rec->email_id= intval;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_AVAILABILITY))
		{
			sch_rec->availability= intval;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_PROGRESS))
		{
			sch_rec->progress = intval;
		}
		else if(0 == strcmp(field, CALS_VALUE_INT_DTSTART_TYPE))
			sch_rec->dtstart_type = intval;
		else if(0 == strcmp(field, CALS_VALUE_INT_DTSTART_YEAR))
			sch_rec->dtstart_year = intval;
		else if(0 == strcmp(field, CALS_VALUE_INT_DTSTART_MONTH))
			sch_rec->dtstart_month = intval;
		else if(0 == strcmp(field, CALS_VALUE_INT_DTSTART_MDAY))
			sch_rec->dtstart_mday = intval;
		else if(0 == strcmp(field, CALS_VALUE_INT_DTEND_TYPE))
			sch_rec->dtend_type = intval;
		else if(0 == strcmp(field, CALS_VALUE_INT_DTEND_YEAR))
			sch_rec->dtend_year = intval;
		else if(0 == strcmp(field, CALS_VALUE_INT_DTEND_MONTH))
			sch_rec->dtend_month = intval;
		else if(0 == strcmp(field, CALS_VALUE_INT_DTEND_MDAY))
			sch_rec->dtend_mday = intval;
		else if(0 == strcmp(field, CALS_VALUE_INT_RRULE_FREQ))
			sch_rec->freq = intval;
		else if(0 == strcmp(field, CALS_VALUE_INT_RRULE_RANGE_TYPE))
			sch_rec->range_type = intval;
		else if(0 == strcmp(field, CALS_VALUE_INT_RRULE_UNTIL_TYPE))
			sch_rec->until_type = intval;
		else if(0 == strcmp(field, CALS_VALUE_INT_RRULE_UNTIL_YEAR))
			sch_rec->until_year = intval;
		else if(0 == strcmp(field, CALS_VALUE_INT_RRULE_UNTIL_MONTH))
			sch_rec->until_month = intval;
		else if(0 == strcmp(field, CALS_VALUE_INT_RRULE_UNTIL_MDAY))
			sch_rec->until_mday = intval;
		else if(0 == strcmp(field, CALS_VALUE_INT_RRULE_COUNT))
			sch_rec->count = intval;
		else if(0 == strcmp(field, CALS_VALUE_INT_RRULE_INTERVAL))
			sch_rec->interval = intval;
		else if(0 == strcmp(field, CALS_VALUE_INT_RRULE_WKST))
			sch_rec->wkst = intval;
		/* deprecated start >>>>>>>>>>>>>>>>>>> */
		else if(0 == strcmp(field,"index"))
		{
			sch_rec->index = intval;
		}
		/* <<<<<<<<<<<<<<<<<<<< deprecated end */
		else
		{
			ERR("Invalid field(%d, %s)", event->event_type, field);
			return CAL_ERR_ARG_INVALID;
		}
		break;

	case CAL_STRUCT_TYPE_CALENDAR:
		cal_rec = event->user_data;
		if(0 == strcmp(field,CAL_TABLE_INT_UPDATED))
		{
			cal_rec->updated = intval;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_HIDDEN))
		{
			cal_rec->hidden = intval;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_SELECTED))
		{
			cal_rec->selected = intval;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_LOCALE))
		{
			cal_rec->locale = intval;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_COUNTRY))
		{
			cal_rec->country = intval;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_TIME_ZONE))
		{
			cal_rec->time_zone = intval;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_DISPLAY_ALL_TIMEZONES))
		{
			cal_rec->display_all_timezones = intval;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_DATE_FIELD_ORDER))
		{
			cal_rec->date_field_order = intval;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_FROMAT_24HOUR_TIME))
		{
			cal_rec->format_24hour_time = intval;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_WEEK_START))
		{
			cal_rec->week_start = intval;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_DEFAULT_CAL_MODE))
		{
			cal_rec->default_cal_mode = intval;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_CUSTOM_CAL_MODE))
		{
			cal_rec->custom_cal_mode = intval;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_SHOW_DECLINED_EVENTS))
		{
			cal_rec->show_declined_events = intval;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_HIDE_INVITATIONS))
		{
			cal_rec->hide_invitations = intval;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_ALTERNATE_CALENDAR))
		{
			cal_rec->alternate_calendar = intval;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_VISIBILITY))
		{
			cal_rec->visibility = intval;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_PROJECTION))
		{
			cal_rec->projection = intval;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_SEQUENCE))
		{
			cal_rec->sequence = intval;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_SUPRESS_REPLY_NOTIFICATIONS))
		{
			cal_rec->suppress_reply_notifications = intval;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_SYNC_EVENT))
		{
			cal_rec->sync_event = intval;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_TIMES_CLEANED))
		{
			cal_rec->times_cleaned = intval;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_GUESTS_CAN_MODIFY))
		{
			cal_rec->guests_can_modify = intval;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_GUESTS_CAN_INVITE_OTHERS))
		{
			cal_rec->guests_can_invite_others = intval;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_GUESTS_CAN_SEE_GUESTS))
		{
			cal_rec->guests_can_see_guests = intval;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_ACCESS_LEVEL))
		{
			cal_rec->access_level = intval;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_SYNC_STATUS))
		{
			cal_rec->sync_status = intval;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_ACCOUNT_ID))
		{
			cal_rec->account_id = intval;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_SENSITIVITY))
		{
			cal_rec->sensitivity = intval;
		}
		else if(0 == strcmp(field,CAL_TABLE_INT_STORE_TYPE))
		{
			cal_rec->store_type = intval;
		}
		else
		{
			ERR("Invalid field(%d, %s)", event->event_type, field);
			return CAL_ERR_ARG_INVALID;
		}
		break;
	case CAL_STRUCT_TYPE_TIMEZONE:
		tz_rec = event->user_data;
		if(0 == strcmp(field,CAL_TZ_VALUE_INT_INDEX))
		{
			tz_rec->index = intval;
		}
		else if(0 == strcmp(field,CAL_TZ_VALUE_INT_TZ_OFFSET))
		{
			tz_rec->tz_offset_from_gmt = intval;
		}
		else if(0 == strcmp(field,CAL_TZ_VALUE_INT_STD_START_MONTH))
		{
			tz_rec->std_start_month = intval;
		}
		else if(0 == strcmp(field,CAL_TZ_VALUE_INT_STD_START_POSITION_OF_WEEK))
		{
			tz_rec->std_start_position_of_week = intval;
		}
		else if(0 == strcmp(field,CAL_TZ_VALUE_INT_STD_START_DAY))
		{
			tz_rec->std_start_day = intval;
		}
		else if(0 == strcmp(field,CAL_TZ_VALUE_INT_STD_START_HOUR))
		{
			tz_rec->std_start_hour = intval;
		}
		else if(0 == strcmp(field,CAL_TZ_VALUE_INT_STD_BIAS))
		{
			tz_rec->standard_bias = intval;
		}
		else if(0 == strcmp(field,CAL_TZ_VALUE_INT_DST_START_MONTH))
		{
			tz_rec->day_light_start_month = intval;
		}
		else if(0 == strcmp(field,CAL_TZ_VALUE_INT_DST_START_POSITION_OF_WEEK))
		{
			tz_rec->day_light_start_position_of_week = intval;
		}
		else if(0 == strcmp(field,CAL_TZ_VALUE_INT_DST_START_DAY))
		{
			tz_rec->day_light_start_day = intval;
		}
		else if(0 == strcmp(field,CAL_TZ_VALUE_INT_DST_START_HOUR))
		{
			tz_rec->day_light_start_hour = intval;
		}
		else if(0 == strcmp(field,CAL_TZ_VALUE_INT_DST_BIAS))
		{
			tz_rec->day_light_bias = intval;
		}
		else {
			ERR("Invalid field(%d, %s)", event->event_type, field);
			return CAL_ERR_ARG_INVALID;
		}
		break;
	case CALS_STRUCT_TYPE_PERIOD_NORMAL_ONOFF:
		nof = event->user_data;
		if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_ONOFF_INT_EVENTID))
			nof->index = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_ONOFF_INT_DTSTART_TYPE))
			nof->dtstart_type = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_ONOFF_INT_DTEND_TYPE))
			nof->dtend_type = intval;
		else {
			ERR("Invalid field(%d, %s)", event->event_type, field);
			return CAL_ERR_ARG_INVALID;
		}
		break;

	case CALS_STRUCT_TYPE_PERIOD_ALLDAY_ONOFF:
		aof = event->user_data;
		if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_ONOFF_INT_EVENTID))
			aof->index = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_ONOFF_INT_DTSTART_TYPE))
			aof->dtstart_type = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_ONOFF_INT_DTSTART_YEAR))
			aof->dtstart_year = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_ONOFF_INT_DTSTART_MONTH))
			aof->dtstart_month = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_ONOFF_INT_DTSTART_MDAY))
			aof->dtstart_mday = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_ONOFF_INT_DTEND_TYPE))
			aof->dtend_type = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_ONOFF_INT_DTEND_YEAR))
			aof->dtend_year = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_ONOFF_INT_DTEND_MONTH))
			aof->dtend_month = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_ONOFF_INT_DTEND_MDAY))
			aof->dtend_mday = intval;
		else {
			ERR("Invalid field(%d, %s)", event->event_type, field);
			return CAL_ERR_ARG_INVALID;
		}
		break;

	case CALS_STRUCT_TYPE_PERIOD_NORMAL_BASIC:
		nbs = event->user_data;
		if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_BASIC_INT_EVENTID))
			nbs->index = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_BASIC_INT_DTSTART_TYPE))
			nbs->dtstart_type = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_BASIC_INT_DTEND_TYPE))
			nbs->dtend_type = intval;
		else {
			ERR("Invalid field(%d, %s)", event->event_type, field);
			return CAL_ERR_ARG_INVALID;
		}
		break;

	case CALS_STRUCT_TYPE_PERIOD_ALLDAY_BASIC:
		abs = event->user_data;
		if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_BASIC_INT_EVENTID))
			abs->index = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_BASIC_INT_DTSTART_TYPE))
			abs->dtstart_type = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_BASIC_INT_DTSTART_YEAR))
			abs->dtstart_year = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_BASIC_INT_DTSTART_MONTH))
			abs->dtstart_month = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_BASIC_INT_DTSTART_MDAY))
			abs->dtstart_mday = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_BASIC_INT_DTEND_TYPE))
			abs->dtend_type = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_BASIC_INT_DTEND_YEAR))
			abs->dtend_year = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_BASIC_INT_DTEND_MONTH))
			abs->dtend_month = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_BASIC_INT_DTEND_MDAY))
			abs->dtend_mday = intval;
		else {
			ERR("Invalid field(%d, %s)", event->event_type, field);
			return CAL_ERR_ARG_INVALID;
		}
		break;

	case CALS_STRUCT_TYPE_PERIOD_NORMAL_OSP:
		nosp = event->user_data;
		if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_OSP_INT_EVENTID))
			nosp->index = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_OSP_INT_DTSTART_TYPE))
			nosp->dtstart_type = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_OSP_INT_DTEND_TYPE))
			nosp->dtend_type = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_OSP_INT_CALENDAR_ID))
			nosp->calendar_id = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_OSP_INT_BUSY_STATUS))
			nosp->busy_status = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_OSP_INT_STATUS))
			nosp->meeting_status = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_OSP_INT_PRIORITY))
			nosp->priority = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_OSP_INT_VISIBILITY))
			nosp->sensitivity = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_OSP_INT_IS_RECURRING))
			nosp->rrule_id = intval;
		else {
			ERR("Invalid field(%d, %s)", event->event_type, field);
			return CAL_ERR_ARG_INVALID;
		}
		break;

	case CALS_STRUCT_TYPE_PERIOD_ALLDAY_OSP:
		aosp = event->user_data;
		if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_OSP_INT_EVENTID))
			aosp->index = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_OSP_INT_DTSTART_TYPE))
			aosp->dtstart_type = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_OSP_INT_DTSTART_YEAR))
			aosp->dtstart_year = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_OSP_INT_DTSTART_MONTH))
			aosp->dtstart_month = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_OSP_INT_DTSTART_MDAY))
			aosp->dtstart_mday = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_OSP_INT_DTEND_TYPE))
			aosp->dtend_type = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_OSP_INT_DTEND_YEAR))
			aosp->dtend_year = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_OSP_INT_DTEND_MONTH))
			aosp->dtend_month = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_OSP_INT_DTEND_MDAY))
			aosp->dtend_mday = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_OSP_INT_CALENDAR_ID))
			aosp->calendar_id = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_OSP_INT_BUSY_STATUS))
			aosp->busy_status = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_OSP_INT_STATUS))
			aosp->meeting_status = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_OSP_INT_PRIORITY))
			aosp->priority = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_OSP_INT_VISIBILITY))
			aosp->sensitivity = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_OSP_INT_IS_RECURRING))
			aosp->rrule_id = intval;
		else {
			ERR("Invalid field(%d, %s)", event->event_type, field);
			return CAL_ERR_ARG_INVALID;
		}
		break;

	case CALS_STRUCT_TYPE_PERIOD_NORMAL_LOCATION:
		nosl = event->user_data;
		if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_LOCATION_INT_EVENTID))
			nosl->index = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_LOCATION_INT_DTSTART_TYPE))
			nosl->dtstart_type = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_LOCATION_INT_DTEND_TYPE))
			nosl->dtend_type = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_LOCATION_INT_CALENDAR_ID))
			nosl->calendar_id = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_LOCATION_INT_BUSY_STATUS))
			nosl->busy_status = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_LOCATION_INT_STATUS))
			nosl->meeting_status = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_LOCATION_INT_PRIORITY))
			nosl->priority = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_LOCATION_INT_VISIBILITY))
			nosl->sensitivity = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_LOCATION_INT_IS_RECURRING))
			nosl->rrule_id = intval;
		else {
			ERR("Invalid field(%d, %s)", event->event_type, field);
			return CAL_ERR_ARG_INVALID;
		}
		break;

	case CALS_STRUCT_TYPE_PERIOD_ALLDAY_LOCATION:
		aosl = event->user_data;
		if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_LOCATION_INT_EVENTID))
			aosl->index = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_LOCATION_INT_DTSTART_TYPE))
			aosl->dtstart_type = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_LOCATION_INT_DTSTART_YEAR))
			aosl->dtstart_year = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_LOCATION_INT_DTSTART_MONTH))
			aosl->dtstart_month = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_LOCATION_INT_DTSTART_MDAY))
			aosl->dtstart_mday = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_LOCATION_INT_DTEND_TYPE))
			aosl->dtend_type = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_LOCATION_INT_DTEND_YEAR))
			aosl->dtend_year = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_LOCATION_INT_DTEND_MONTH))
			aosl->dtend_month = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_LOCATION_INT_DTEND_MDAY))
			aosl->dtend_mday = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_LOCATION_INT_CALENDAR_ID))
			aosl->calendar_id = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_LOCATION_INT_BUSY_STATUS))
			aosl->busy_status = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_LOCATION_INT_STATUS))
			aosl->meeting_status = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_LOCATION_INT_PRIORITY))
			aosl->priority = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_LOCATION_INT_VISIBILITY))
			aosl->sensitivity = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_LOCATION_INT_IS_RECURRING))
			aosl->rrule_id = intval;
		else {
			ERR("Invalid field(%d, %s)", event->event_type, field);
			return CAL_ERR_ARG_INVALID;
		}
		break;

	case CALS_STRUCT_TYPE_PERIOD_NORMAL_ALARM:
		nosa = event->user_data;
		if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_ALARM_INT_EVENTID))
			nosa->index = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_ALARM_INT_DTSTART_TYPE))
			nosa->dtstart_type = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_ALARM_INT_DTEND_TYPE))
			nosa->dtend_type = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_ALARM_INT_CALENDAR_ID))
			nosa->calendar_id = intval;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_ALARM_INT_ALARM_ID))
			nosa->alarm_id = intval;
		else {
			ERR("Invalid field(%d, %s)", event->event_type, field);
			return CAL_ERR_ARG_INVALID;
		}

	default:
		ERR("Invalid field(%d, %s)", event->event_type, field);
		return CAL_ERR_ARG_INVALID;
	}

	return CAL_SUCCESS;
}

API int calendar_svc_struct_set_str (cal_struct *event, const char *field, const char *strval)
{
	retex_if(NULL == event || NULL == field || NULL == strval,,"[ERROR]calendar_svc_struct_set_str:Invalid parameters.\n");

	cal_sch_full_t *sch_rec = NULL;
	calendar_t *cal_rec = NULL;
	cal_timezone_t *tz_rec = NULL;
	cals_struct_period_normal_basic *nbs = NULL;
	cals_struct_period_allday_basic *abs = NULL;
	cals_struct_period_normal_osp *nosp = NULL;
	cals_struct_period_allday_osp *aosp = NULL;
	cals_struct_period_normal_location *nosl = NULL;
	cals_struct_period_allday_location *aosl = NULL;

	retvm_if(strval == NULL, CAL_ERR_FAIL, "Invalid argument: value is NULL");

	switch(event->event_type)
	{
	case CAL_STRUCT_TYPE_SCHEDULE:
	case CAL_STRUCT_TYPE_TODO:
		sch_rec = (cal_sch_full_t*)event->user_data;
		if(0 == strcmp(field,CAL_VALUE_TXT_SUMMARY))
		{
			CAL_FREE(sch_rec->summary);
			sch_rec->summary = strdup(strval);
		}
		else if(0 == strcmp(field,CAL_VALUE_TXT_DESCRIPTION))
		{
			CAL_FREE(sch_rec->description);
			sch_rec->description = strdup(strval);
		}
		else if(0 == strcmp(field, CAL_VALUE_TXT_LOCATION))
		{
			CAL_FREE(sch_rec->location);
			sch_rec->location = strdup(strval);
		}
		else if(0 == strcmp(field, CAL_VALUE_TXT_CATEGORIES))
		{
			CAL_FREE(sch_rec->categories);
			sch_rec->categories = strdup(strval);
		}
		else if(0 == strcmp(field, CAL_VALUE_TXT_EXDATE))
		{
			CAL_FREE(sch_rec->exdate);
			sch_rec->exdate = strdup(strval);
		}
		else if(0 == strcmp(field,CAL_VALUE_TXT_UID))
		{
			CAL_FREE(sch_rec->uid);
			sch_rec->uid = strdup(strval);
		}
		else if(0 == strcmp(field,CAL_VALUE_TXT_ORGANIZER_NAME))
		{
			CAL_FREE(sch_rec->organizer_name);
			sch_rec->organizer_name = strdup(strval);
		}
		else if(0 == strcmp(field,CAL_VALUE_TXT_ORGANIZER_EMAIL  ))
		{
			CAL_FREE(sch_rec->organizer_email);
			sch_rec->organizer_email = strdup(strval);
		}
		else if(0 == strcmp(field,CAL_VALUE_TXT_GCAL_ID ))
		{
			CAL_FREE(sch_rec->gcal_id);
			sch_rec->gcal_id = strdup(strval);
		}
		else if(0 == strcmp(field,CAL_VALUE_TXT_UPDATED))
		{
			CAL_FREE(sch_rec->updated);
			sch_rec->updated = strdup(strval);
		}
		else if(0 == strcmp(field,CAL_VALUE_TXT_LOCATION_SUMMARY))
		{
			CAL_FREE(sch_rec->location_summary);
			sch_rec->location_summary = strdup(strval);
		}
		else if(0 == strcmp(field, CAL_VALUE_TXT_ETAG))
		{
			CAL_FREE(sch_rec->etag);
			sch_rec->etag = strdup(strval);
		}
		else if(0 == strcmp(field,CAL_VALUE_TXT_EDIT_URL))
		{
			CAL_FREE(sch_rec->edit_uri);
			sch_rec->edit_uri = strdup(strval);
		}
		else if(0 == strcmp(field,CAL_VALUE_TXT_GEDERID))
		{
			CAL_FREE(sch_rec->gevent_id);
			sch_rec->gevent_id = strdup(strval);
		}
		else if(0 == strcmp(field, CALS_VALUE_TXT_DTSTART_TZID))
		{
			CAL_FREE(sch_rec->dtstart_tzid);
			sch_rec->dtstart_tzid = strdup(strval);
		}
		else if(0 == strcmp(field, CALS_VALUE_TXT_DTEND_TZID))
		{
			CAL_FREE(sch_rec->dtend_tzid);
			sch_rec->dtend_tzid = strdup(strval);
		}
		else if(0 == strcmp(field, CALS_VALUE_TXT_RRULE_BYSECOND))
		{
			CAL_FREE(sch_rec->bysecond);
			sch_rec->bysecond = strdup(strval);
		}
		else if(0 == strcmp(field, CALS_VALUE_TXT_RRULE_BYMINUTE))
		{
			CAL_FREE(sch_rec->byminute);
			sch_rec->byminute = strdup(strval);
		}
		else if(0 == strcmp(field, CALS_VALUE_TXT_RRULE_BYHOUR))
		{
			CAL_FREE(sch_rec->byhour);
			sch_rec->byhour = strdup(strval);
		}
		else if(0 == strcmp(field, CALS_VALUE_TXT_RRULE_BYDAY))
		{
			CAL_FREE(sch_rec->byday);
			sch_rec->byday = strdup(strval);
		}
		else if(0 == strcmp(field, CALS_VALUE_TXT_RRULE_BYMONTHDAY))
		{
			CAL_FREE(sch_rec->bymonthday);
			sch_rec->bymonthday = strdup(strval);
		}
		else if(0 == strcmp(field, CALS_VALUE_TXT_RRULE_BYYEARDAY))
		{
			CAL_FREE(sch_rec->byyearday);
			sch_rec->byyearday = strdup(strval);
		}
		else if(0 == strcmp(field, CALS_VALUE_TXT_RRULE_BYWEEKNO))
		{
			CAL_FREE(sch_rec->byweekno);
			sch_rec->byweekno = strdup(strval);
		}
		else if(0 == strcmp(field, CALS_VALUE_TXT_RRULE_BYMONTH))
		{
			CAL_FREE(sch_rec->bymonth);
			sch_rec->bymonth = strdup(strval);
		}
		else if(0 == strcmp(field, CALS_VALUE_TXT_RRULE_BYSETPOS))
		{
			CAL_FREE(sch_rec->bysetpos);
			sch_rec->bysetpos = strdup(strval);
		}
		else
		{
			retex_if(true,,"Can not find the field(%s)", field);
		}
		break;

	case CAL_STRUCT_TYPE_CALENDAR:
		cal_rec = (calendar_t*)event->user_data;

		if(0 == strcmp(field,CAL_TABLE_TXT_CALENDAR_ID))
		{
			CAL_FREE(cal_rec->calendar_id);
			cal_rec->calendar_id = strdup(strval);
		}
		else if(0 == strcmp(field,CAL_TABLE_TXT_UID))
		{
			CAL_FREE(cal_rec->uid);
			cal_rec->uid = strdup(strval);
		}
		else if(0 == strcmp(field,CAL_TABLE_TXT_LINK))
		{
			CAL_FREE(cal_rec->link);
			cal_rec->link = strdup(strval);
		}
		else if(0 == strcmp(field,CAL_TABLE_TXT_NAME))
		{
			CAL_FREE(cal_rec->name);
			cal_rec->name = strdup(strval);
		}
		else if(0 == strcmp(field,CAL_TABLE_TXT_DESCRIPTION))
		{
			CAL_FREE(cal_rec->description);
			cal_rec->description = strdup(strval);
		}
		else if(0 == strcmp(field,CAL_TABLE_TXT_AUTHOR))
		{
			CAL_FREE(cal_rec->author);
			cal_rec->author = strdup(strval);
		}
		else if(0 == strcmp(field,CAL_TABLE_TXT_COLOR))
		{
			CAL_FREE(cal_rec->color);
			cal_rec->color = strdup(strval);
		}
		else if(0 == strcmp(field,CAL_TABLE_TXT_LOCATION))
		{
			CAL_FREE(cal_rec->location);
			cal_rec->location  = strdup(strval);
		}
		else if(0 == strcmp(field,CAL_TABLE_TXT_TIME_ZONE_LABEL))
		{
			CAL_FREE(cal_rec->timezone_label);
			cal_rec->timezone_label = strdup(strval);
		}
		else if(0 == strcmp(field,CAL_TABLE_TXT_USER_LOCATION))
		{
			CAL_FREE(cal_rec->user_location);
			cal_rec->user_location = strdup(strval);
		}
		else if(0 == strcmp(field,CAL_TABLE_TXT_WEATHER))
		{
			CAL_FREE(cal_rec->weather);
			cal_rec->weather = strdup(strval);
		}
		else
		{
			retex_if(true,,"[ERROR]calendar_svc_struct_set_str:Can not find the field!\n");
		}

		break;
	case CAL_STRUCT_TYPE_TIMEZONE:
		tz_rec = (cal_timezone_t*)event->user_data;
		if(0 == strcmp(field,CAL_TZ_VALUE_TXT_STD_NAME))
		{
			CAL_FREE(tz_rec->standard_name);
			tz_rec->standard_name = strdup(strval);
		}
		else if(0 == strcmp(field,CAL_TZ_VALUE_TXT_DST_NAME))
		{
			CAL_FREE(tz_rec->day_light_name);
			tz_rec->day_light_name = strdup(strval);
		}
		break;

	case CALS_STRUCT_TYPE_PERIOD_NORMAL_ONOFF:
		ERR("No field in CALS_LIST_PERIOD_NORMAL_ONOFF");
		break;

	case CALS_STRUCT_TYPE_PERIOD_ALLDAY_ONOFF:
		ERR("No field in CALS_LIST_PERIOD_ALLDAY_ONOFF");
		break;

	case CALS_STRUCT_TYPE_PERIOD_NORMAL_BASIC:
		nbs = event->user_data;
		if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_BASIC_TXT_SUMMARY)) {
			CAL_FREE(nbs->summary);
			nbs->summary = strdup(strval);
		} else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_BASIC_TXT_LOCATION)) {
			CAL_FREE(nbs->location);
			nbs->location = strdup(strval);
		}else {
			ERR("Can not find the field!(%s)",field);
		}
		break;

	case CALS_STRUCT_TYPE_PERIOD_ALLDAY_BASIC:
		abs = event->user_data;
		if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_BASIC_TXT_SUMMARY)) {
			CAL_FREE(abs->summary);
			abs->summary = strdup(strval);
		} else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_BASIC_TXT_LOCATION)) {
			CAL_FREE(abs->location);
			abs->location = strdup(strval);
		}else {
			ERR("Can not find the field!(%s)",field);
		}
		break;

	case CALS_STRUCT_TYPE_PERIOD_NORMAL_OSP:
		nosp = event->user_data;
		if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_OSP_TXT_SUMMARY)) {
			CAL_FREE(nosp->summary);
			nosp->summary = strdup(strval);
		} else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_OSP_TXT_LOCATION)) {
			CAL_FREE(nosp->location);
			nosp->location = strdup(strval);
		} else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_OSP_TXT_DESCRIPTION)) {
			CAL_FREE(nosp->description);
			nosp->description = strdup(strval);
		}else {
			ERR("Can not find the field!(%s)",field);
		}
		break;

	case CALS_STRUCT_TYPE_PERIOD_ALLDAY_OSP:
		nosp = event->user_data;
		if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_OSP_TXT_SUMMARY)) {
			CAL_FREE(aosp->summary);
			aosp->summary = strdup(strval);
		} else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_OSP_TXT_LOCATION)) {
			CAL_FREE(aosp->location);
			aosp->location = strdup(strval);
		} else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_OSP_TXT_DESCRIPTION)) {
			CAL_FREE(aosp->description);
			aosp->description = strdup(strval);
		} else {
			ERR("Can not find the field!(%s)", field);
		}
		break;

	case CALS_STRUCT_TYPE_PERIOD_NORMAL_LOCATION:
		nosl = event->user_data;
		if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_LOCATION_TXT_SUMMARY)) {
			CAL_FREE(nosp->summary);
			nosl->summary = strdup(strval);
		} else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_LOCATION_TXT_LOCATION)) {
			CAL_FREE(nosp->location);
			nosl->location = strdup(strval);
		} else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_LOCATION_TXT_DESCRIPTION)) {
			CAL_FREE(nosp->description);
			nosl->description = strdup(strval);
		}else {
			ERR("Can not find the field!(%s)",field);
		}
		break;

	case CALS_STRUCT_TYPE_PERIOD_ALLDAY_LOCATION:
		nosl = event->user_data;
		if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_LOCATION_TXT_SUMMARY)) {
			CAL_FREE(aosp->summary);
			aosl->summary = strdup(strval);
		} else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_LOCATION_TXT_LOCATION)) {
			CAL_FREE(aosp->location);
			aosl->location = strdup(strval);
		} else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_LOCATION_TXT_DESCRIPTION)) {
			CAL_FREE(aosp->description);
			aosl->description = strdup(strval);
		} else {
			ERR("Can not find the field!(%s)", field);
		}
		break;

	default:
		ERR("Unknown event type(%d)", event->event_type);
		return CAL_ERR_ARG_INVALID;
	}

	return CAL_SUCCESS;

CATCH:

	return CAL_ERR_FAIL;

}

API double calendar_svc_struct_get_double(cal_struct* record, const char *field)
{
	double ret_val = 0.0;
	cal_sch_full_t *sch_rec = NULL;
	cals_struct_period_normal_location *nosl = NULL;
	cals_struct_period_allday_location *aosl = NULL;

	retv_if(NULL == record, 0.0);
	retv_if(NULL == field, 0.0);
	retv_if(NULL == record->user_data, 0.0);

	switch(record->event_type) {
	case CAL_STRUCT_TYPE_SCHEDULE:
	case CAL_STRUCT_TYPE_TODO:
		sch_rec = record->user_data;
		if(0 == strcmp(field,CAL_VALUE_DBL_LATITUDE))
			ret_val = sch_rec->latitude;
		else if(0 == strcmp(field,CAL_VALUE_DBL_LONGITUDE))
			ret_val = sch_rec->longitude;
		else
			ERR("Unknown field(%s)", field);
		break;

	case CALS_STRUCT_TYPE_PERIOD_NORMAL_LOCATION:
		nosl = record->user_data;
		if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_LOCATION_DBL_LATITUDE))
			ret_val = nosl->latitude;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_LOCATION_DBL_LONGITUDE))
			ret_val = nosl->longitude;
		else
			ERR("Unknown field(%s)", field);
		break;

	case CALS_STRUCT_TYPE_PERIOD_ALLDAY_LOCATION:
		aosl = record->user_data;
		if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_LOCATION_DBL_LATITUDE))
			ret_val = aosl->latitude;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_LOCATION_DBL_LONGITUDE))
			ret_val = aosl->longitude;
		else
			ERR("Unknown field(%s)", field);
		break;

	default:
		ERR("Unknown event type(%d)", record->event_type);
		break;
	}

	return ret_val;
}

API int calendar_svc_struct_set_double(cal_struct* record, const char *field,double value)
{
	cal_sch_full_t * sch_rec = NULL;
	cals_struct_period_normal_location *nosl = NULL;
	cals_struct_period_allday_location *aosl = NULL;

	retv_if(NULL == record, CAL_ERR_ARG_NULL);
	retv_if(NULL == field, CAL_ERR_ARG_NULL);
	retv_if(NULL == record->user_data, CAL_ERR_ARG_INVALID);

	switch(record->event_type)
	{
	case CAL_STRUCT_TYPE_SCHEDULE:
	case CAL_STRUCT_TYPE_TODO:
		sch_rec = record->user_data;
		if(0 == strcmp(field,CAL_VALUE_DBL_LATITUDE))
			sch_rec->latitude = value;
		else if(0 == strcmp(field,CAL_VALUE_DBL_LONGITUDE))
			sch_rec->longitude = value;
		else {
			ERR("Unknown field(%s)", field);
			return CAL_ERR_ARG_INVALID;
		}
		break;

	case CALS_STRUCT_TYPE_PERIOD_NORMAL_LOCATION:
		nosl = record->user_data;
		if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_LOCATION_DBL_LATITUDE))
			nosl->latitude = value;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_LOCATION_DBL_LONGITUDE))
			nosl->longitude = value;
		break;

	case CALS_STRUCT_TYPE_PERIOD_ALLDAY_LOCATION:
		aosl = record->user_data;
		if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_LOCATION_DBL_LATITUDE))
			aosl->latitude = value;
		else if (!strcmp(field, CALS_LIST_PERIOD_ALLDAY_LOCATION_DBL_LONGITUDE))
			aosl->longitude = value;
		else
			ERR("Unknown field(%s)", field);
		break;

	default:
		ERR("Unknown event type(%d)", record->event_type);
		return CAL_ERR_ARG_INVALID;
	}

	return CAL_SUCCESS;
}



API int calendar_svc_struct_get_list (cal_struct *event, const char *field, GList **retlist)
{
	cal_sch_full_t * sch_rec = NULL;

	retv_if(NULL == event, CAL_ERR_ARG_NULL);
	retv_if(NULL == field, CAL_ERR_ARG_NULL);
	retv_if(NULL == event->user_data, CAL_ERR_ARG_INVALID);

	switch(event->event_type) {
	case CAL_STRUCT_TYPE_SCHEDULE:
		sch_rec = (cal_sch_full_t*)event->user_data;
		if(0 == strcmp(field,CAL_VALUE_LST_ATTENDEE_LIST))
			*retlist = sch_rec->attendee_list;
		else if(0 == strcmp(field, CAL_VALUE_LST_ALARM))
			*retlist = sch_rec->alarm_list;
		else {
			ERR("Unknown field(%s)", field);
			return CAL_ERR_ARG_INVALID;
		}
		break;
	case CAL_STRUCT_TYPE_TODO:
		sch_rec = (cal_sch_full_t*)event->user_data;
		if(0 == strcmp(field,CAL_VALUE_LST_ATTENDEE_LIST))
			*retlist = sch_rec->attendee_list;
		else if(0 == strcmp(field, CAL_VALUE_LST_ALARM))
			*retlist = sch_rec->alarm_list;
		else {
			ERR("Unknown field(%s)", field);
			return CAL_ERR_ARG_INVALID;
		}
		break;
	case CAL_STRUCT_TYPE_CALENDAR:
	default:
		ERR("Unknown event type(%d)", event->event_type);
		return CAL_ERR_ARG_INVALID;
	}

	return CAL_SUCCESS;
}



static int _cals_struct_remove_list(const char *field, GList *list)
{
	GList *pList = list;

	while(pList)
	{
		calendar_svc_value_free((cal_value**)&pList->data);

		pList = pList->next;
	}
	g_list_free(list);
	return 0;
}


API int calendar_svc_struct_store_list (cal_struct *event, const char *field, GList *list)
{
	cal_sch_full_t *sch_rec = NULL;

	retv_if(NULL == event, CAL_ERR_ARG_NULL);
	retv_if(NULL == field, CAL_ERR_ARG_NULL);
	retv_if(NULL == event->user_data, CAL_ERR_ARG_INVALID);

	switch(event->event_type) {
	case CAL_STRUCT_TYPE_SCHEDULE:
		sch_rec = (cal_sch_full_t*)event->user_data;

		if(0 == strcmp(field,CAL_VALUE_LST_ATTENDEE_LIST))
		{
			if(list == NULL)
				_cals_struct_remove_list(field,sch_rec->attendee_list);

			sch_rec->attendee_list = list;
		}
		else if(0 == strcmp(field,CAL_VALUE_LST_ALARM))
		{
			if(list == NULL)
				_cals_struct_remove_list(field,sch_rec->alarm_list);

			sch_rec->alarm_list = list;
		} else {
			ERR("%s is invalid field for event type schedule");
			return CAL_ERR_ARG_INVALID;
		}
		break;

	case CAL_STRUCT_TYPE_TODO:
		sch_rec = (cal_sch_full_t*)event->user_data;

		if(0 == strcmp(field,CAL_VALUE_LST_ATTENDEE_LIST))
		{
			if(list == NULL)
				_cals_struct_remove_list(field,sch_rec->attendee_list);

			sch_rec->attendee_list = list;
		}
		else if(0 == strcmp(field,CAL_VALUE_LST_ALARM))
		{
			if(list == NULL)
				_cals_struct_remove_list(field,sch_rec->alarm_list);

			sch_rec->alarm_list = list;
		} else {
			ERR("%s is invalid field for todo type schedule");
			return CAL_ERR_ARG_INVALID;
		}
		break;

	case CAL_STRUCT_TYPE_CALENDAR:
	default:
		ERR("Invalid schedule type(%d)", event->event_type);
		return CAL_ERR_ARG_INVALID;
	}

	return CAL_SUCCESS;
}

API cal_value* calendar_svc_value_new (const char *filed)
{
	CALS_FN_CALL;
	cal_value* value = NULL;

	retex_if(NULL == filed,,"[ERROR]calendar_svc_value_new:Invalid parameters.\n");

	value = (cal_value*)malloc(sizeof(cal_value));
	retex_if(NULL == value,,"[ERROR]calendar_svc_value_new:Failed to malloc!\n");

	memset(value,0x00,sizeof(cal_value));

	if(0 == strcmp(CAL_VALUE_LST_ATTENDEE_LIST,filed))
	{
		value->v_type = CAL_EVENT_PATICIPANT;
		value->user_data = (cal_participant_info_t*)malloc(sizeof(cal_participant_info_t));
		retex_if(NULL == value->user_data,,"[ERROR]calendar_svc_value_new:Fail to malloc!\n");

		memset(value->user_data,0,sizeof(cal_participant_info_t));
	}
	else if(0 == strcmp(CAL_VALUE_LST_ALARM,filed))
	{
		value->v_type = CAL_EVENT_ALARM;
		value->user_data = (cal_alarm_info_t*)malloc(sizeof(cal_alarm_info_t));
		retex_if(NULL == value->user_data,,"[ERROR]calendar_svc_value_new:Fail to malloc!\n");

		memset(value->user_data,0,sizeof(cal_alarm_info_t));
	}
	else
	{
		CAL_FREE(value);

		retex_if(true,,"[ERROR]calendar_svc_value_new:Invalid parameter!\n");
	}
	DBG("- calendar_svc_value_new\n");
	return value;

CATCH:

	CAL_FREE(value);


	return NULL;
}

static int calendar_svc_free_participant(cal_participant_info_t* value)
{
	CALS_FN_CALL;
	if(NULL == value)
	{
		return CAL_SUCCESS;
	}

	CAL_FREE(value->attendee_name);
	CAL_FREE(value->attendee_email);
	CAL_FREE(value->attendee_number);
	CAL_FREE(value);


	return CAL_SUCCESS;
}

static int calendar_svc_free_alarm_info(cal_alarm_info_t* value)
{
	CALS_FN_CALL;

	if(value) {
		free(value->alarm_tone);
		free(value->alarm_description);
		free(value);
	}

	return CAL_SUCCESS;
}

API int calendar_svc_value_free (cal_value **value)
{
	CALS_FN_CALL;
	retex_if(NULL == value,,"[ERROR]calendar_svc_value_free:Invalid parameter!\n");
	retex_if(NULL == *value,,"[ERROR]calendar_svc_value_free:Invalid parameter!\n");

	switch((*value)->v_type)
	{
	case CAL_EVENT_PATICIPANT:
		calendar_svc_free_participant((cal_participant_info_t*)(*value)->user_data);
		break;

	case CAL_EVENT_ALARM:
		calendar_svc_free_alarm_info((cal_alarm_info_t*)(*value)->user_data);
		break;

	default:
		break;
	}

	CAL_FREE((*value));


	return CAL_SUCCESS;

CATCH:

	return CAL_ERR_FAIL;

}


API int calendar_svc_value_set_int (cal_value *value, const char *field, int intval)
{
	cal_participant_info_t *participant = NULL;
	cal_alarm_info_t *alarm_info = NULL;

	if (!value || !value->user_data || !field || !*field) {
		ERR("Invalid parameter");
		return CAL_ERR_ARG_INVALID;
	}

	switch(value->v_type)
	{
	case CAL_EVENT_PATICIPANT:

		participant = (cal_participant_info_t*)value->user_data;

		if(0 == strcmp(field,CAL_VALUE_INT_ATTENDEE_DETAIL_STATUS))
		{
			participant->attendee_status = intval;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_ATTENDEE_DETAIL_TYPE))
		{
			participant->attendee_type = intval;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_ATTENDEE_DETAIL_CT_INDEX))
		{
			participant->attendee_ct_index = intval;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_ATTENDEE_ROLE))
		{
			participant->attendee_role = intval;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_ATTENDEE_RSVP))
		{
			participant->attendee_rsvp = intval;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_DETAIL_DELETE))
		{
			participant->is_deleted= intval;
		}
		else
		{
			ERR("Invalid field (%s)", field);
			return CAL_ERR_ARG_INVALID;
		}
		break;
	case CAL_EVENT_ALARM:
		alarm_info = (cal_alarm_info_t*)value->user_data;
		if(0 == strcmp(field,CAL_VALUE_INT_ALARMS_TICK))
		{
			alarm_info->remind_tick = intval;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_ALARMS_TICK_UNIT))
		{
			alarm_info->remind_tick_unit = intval;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_ALARMS_TYPE))
		{
			alarm_info->alarm_type = intval;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_ALARMS_ID))
		{
			alarm_info->alarm_id = intval;
		}
		else
		{
			ERR("Invalid field (%s)", field);
			return CAL_ERR_ARG_INVALID;
		}
		break;

	default:
		ERR("Invalid value type (%d)", value->v_type);
		return CAL_ERR_ARG_INVALID;
	}

	return CAL_SUCCESS;
}

API int calendar_svc_value_set_lli(cal_value *value, const char *field, long long int llival)
{
	cal_alarm_info_t *alarm_info;

	if (!field || !*field)
		return CAL_ERR_ARG_NULL;

	switch(value->v_type)
	{
	case CAL_EVENT_ALARM:
		alarm_info = (cal_alarm_info_t *)value->user_data;
		if (!strcmp(field, CAL_VALUE_LLI_ALARMS_TIME))
			alarm_info->alarm_time = llival;
		else {
			ERR("Invalid field (%s) in alarm value", field);
			return CAL_ERR_ARG_INVALID;
		}
		break;
	default:
		ERR("Invalid value type (%d)", value->v_type);
		return CAL_ERR_ARG_INVALID;
	}

	return CAL_SUCCESS;
}

API long long int calendar_svc_value_get_lli(cal_value *value, const char *field)
{
	cal_alarm_info_t *alarm_info;

	switch(value->v_type)
	{
	case CAL_EVENT_ALARM:
		alarm_info = (cal_alarm_info_t*)value->user_data;
		if(!strcmp(field,CAL_VALUE_LLI_ALARMS_TIME))
			return alarm_info->alarm_time;
		else {
			ERR("Invalid field (%s) in alarm value", field);
			return CAL_ERR_ARG_INVALID;
		}
		break;
	default:
		ERR("Invalid value type (%d)", value->v_type);
		return CAL_ERR_ARG_INVALID;
	}
	return CAL_SUCCESS;
}

API int calendar_svc_value_set_str (cal_value *value, const char *field, const char *strval)
{
	retex_if(NULL == value || NULL == value->user_data || NULL == field || NULL == strval,,"[ERROR]calendar_svc_value_free:Invalid parameter!\n");

	cal_participant_info_t* participant = NULL;
	cal_alarm_info_t *alarm_info = NULL;

	int str_len = strlen(strval)+1;

	switch(value->v_type)
	{
	case CAL_EVENT_PATICIPANT:

		participant = (cal_participant_info_t*)value->user_data;

		if(0 == strcmp(field,CAL_VALUE_TXT_ATTENDEE_DETAIL_NAME))
		{
			CAL_FREE(participant->attendee_name);


			participant->attendee_name = (char*)malloc(str_len);
			retex_if(NULL == participant->attendee_name,,"[ERROR]calendar_svc_value_set_str:Failed to malloc!\n");

			memset(participant->attendee_name,0x00,str_len);

			strcpy(participant->attendee_name,strval);
		}
		else if(0 == strcmp(field,CAL_VALUE_TXT_ATTENDEE_DETAIL_EMAIL))
		{
			CAL_FREE(participant->attendee_email);


			participant->attendee_email = (char*)malloc(str_len);
			retex_if(NULL == participant->attendee_email,,"[ERROR]calendar_svc_value_set_str:Failed to malloc!\n");

			memset(participant->attendee_email,0x00,str_len);

			strcpy(participant->attendee_email,strval);
		}
		else if(0 == strcmp(field,CAL_VALUE_TXT_ATTENDEE_DETAIL_NUMBER))
		{
			CAL_FREE(participant->attendee_number);


			participant->attendee_number = (char*)malloc(str_len);
			retex_if(NULL == participant->attendee_number,,"[ERROR]calendar_svc_value_set_str:Failed to malloc!\n");

			memset(participant->attendee_number,0x00,str_len);

			strcpy(participant->attendee_number,strval);
		}
		else if(0 == strcmp(field,CAL_VALUE_TXT_ATTENDEE_GROUP))
		{
			CAL_FREE(participant->attendee_group);


			participant->attendee_group = (char*)malloc(str_len);
			retex_if(NULL == participant->attendee_group,,"[ERROR]calendar_svc_value_set_str:Failed to malloc!\n");

			memset(participant->attendee_group,0x00,str_len);

			strcpy(participant->attendee_group,strval);
		}
		else if(0 == strcmp(field,CAL_VALUE_TXT_ATTENDEE_DELEGATOR_URI))
		{
			CAL_FREE(participant->attendee_delegator_uri);


			participant->attendee_delegator_uri = (char*)malloc(str_len);
			retex_if(NULL == participant->attendee_delegator_uri,,"[ERROR]calendar_svc_value_set_str:Failed to malloc!\n");

			memset(participant->attendee_delegator_uri,0x00,str_len);

			strcpy(participant->attendee_delegator_uri,strval);
		}
		else if(0 == strcmp(field,CAL_VALUE_TXT_ATTENDEE_DELEGATE_URI))
		{
			CAL_FREE(participant->attendee_delegate_uri);


			participant->attendee_delegate_uri = (char*)malloc(str_len);
			retex_if(NULL == participant->attendee_delegate_uri,,"[ERROR]calendar_svc_value_set_str:Failed to malloc!\n");

			memset(participant->attendee_delegate_uri,0x00,str_len);

			strcpy(participant->attendee_delegate_uri,strval);
		}
		else if(0 == strcmp(field,CAL_VALUE_TXT_ATTENDEE_UID))
		{
			CAL_FREE(participant->attendee_uid);


			participant->attendee_uid = (char*)malloc(str_len);
			retex_if(NULL == participant->attendee_uid,,"[ERROR]calendar_svc_value_set_str:Failed to malloc!\n");

			memset(participant->attendee_uid,0x00,str_len);

			strcpy(participant->attendee_uid,strval);
		}

		else
		{
		}
		break;
	case CAL_EVENT_ALARM:

		alarm_info = (cal_alarm_info_t*)value->user_data;

		if(0 == strcmp(field,CAL_VALUE_TXT_ALARMS_TONE))
		{
			CAL_FREE(alarm_info->alarm_tone);


			alarm_info->alarm_tone = (char*)malloc(str_len);
			retex_if(NULL == alarm_info->alarm_tone,,"[ERROR]calendar_svc_value_set_str:Failed to malloc!\n");

			memset(alarm_info->alarm_tone,0x00,str_len);

			strcpy(alarm_info->alarm_tone,strval);
		}
		else if(0 == strcmp(field,CAL_VALUE_TXT_ALARMS_DESCRIPTION))
		{
			CAL_FREE(alarm_info->alarm_description);

			alarm_info->alarm_description = (char *)malloc(str_len);
			retex_if(NULL == alarm_info->alarm_description,,"[ERROR]calendar_svc_value_set_str:Failed to malloc!\n");

			memset(alarm_info->alarm_description,0x00,str_len);

			strcpy(alarm_info->alarm_description,strval);
		}
		break;
	default:
		break;
	}

	return CAL_SUCCESS;

CATCH:

	return CAL_ERR_FAIL;

}

API int calendar_svc_value_get_int (cal_value *value, const char *field)
{
	cal_participant_info_t* participant = NULL;
	cal_alarm_info_t* alarm_info = NULL;

	if (!value || !value->user_data || !field || !*field) {
		ERR("Invalid parameter");
		return 0;
	}

	switch(value->v_type)
	{
	case CAL_EVENT_PATICIPANT:

		participant = (cal_participant_info_t*)value->user_data;

		if(0 == strcmp(field,CAL_VALUE_INT_ATTENDEE_DETAIL_STATUS))
		{
			return (participant->attendee_status);
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_ATTENDEE_DETAIL_TYPE))
		{
			return (participant->attendee_type);
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_ATTENDEE_DETAIL_CT_INDEX))
		{
			return participant->attendee_ct_index;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_ATTENDEE_ROLE))
		{
			return participant->attendee_role;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_ATTENDEE_RSVP))
		{
			return participant->attendee_rsvp;
		}
		else
		{
		}
		break;
	case CAL_EVENT_ALARM:
		alarm_info = (cal_alarm_info_t*)value->user_data;
		if(0 == strcmp(field,CAL_VALUE_INT_ALARMS_TICK))
		{
			return alarm_info->remind_tick;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_ALARMS_TICK_UNIT))
		{
			return alarm_info->remind_tick_unit;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_ALARMS_TYPE))
		{
			return alarm_info->alarm_type;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_ALARMS_ID))
		{
			return alarm_info->alarm_id;
		}
		break;
	default:
		break;
	}

	return 0;
}

API char * calendar_svc_value_get_str (cal_value *value, const char *field)
{
	CALS_FN_CALL;
	cal_participant_info_t* participant = NULL;
	cal_alarm_info_t * alarm_info = NULL;

	if (!value || !value->user_data || !field || !*field) {
		ERR("Invalid parameter");
		return NULL;
	}

	switch(value->v_type)
	{
	case CAL_EVENT_PATICIPANT:

		participant = (cal_participant_info_t*)value->user_data;

		if(0 == strcmp(field,CAL_VALUE_TXT_ATTENDEE_DETAIL_NAME))
		{
			return participant->attendee_name;
		}
		else if(0 == strcmp(field,CAL_VALUE_TXT_ATTENDEE_DETAIL_EMAIL))
		{
			return participant->attendee_email;
		}
		else if(0 == strcmp(field,CAL_VALUE_TXT_ATTENDEE_DETAIL_NUMBER))
		{
			return participant->attendee_number;
		}
		else if(0 == strcmp(field,CAL_VALUE_TXT_ATTENDEE_GROUP))
		{
			return participant->attendee_group;
		}
		else if(0 == strcmp(field,CAL_VALUE_TXT_ATTENDEE_DELEGATOR_URI))
		{
			return participant->attendee_delegator_uri;
		}
		else if(0 == strcmp(field,CAL_VALUE_TXT_ATTENDEE_DELEGATE_URI))
		{
			return participant->attendee_delegate_uri;
		}
		else if(0 == strcmp(field,CAL_VALUE_TXT_ATTENDEE_UID))
		{
			return participant->attendee_uid;
		}
		break;
	case CAL_EVENT_ALARM:

		alarm_info = (cal_alarm_info_t*)value->user_data;

		if(0 == strcmp(field,CAL_VALUE_TXT_ALARMS_TONE))
		{
			return alarm_info->alarm_tone;
		}
		else if(0 == strcmp(field,CAL_VALUE_TXT_ALARMS_DESCRIPTION))
		{
			return alarm_info->alarm_description;
		}
		break;
	default:
		break;
	}

	return NULL;
}

API long long int calendar_svc_struct_get_lli(cal_struct *event, const char *field)
{
	cal_sch_full_t *sch_rec = NULL;
	cals_struct_period_normal_onoff *nof = NULL;
	cals_struct_period_normal_basic *nbs = NULL;
	cals_struct_period_normal_osp *nosp = NULL;
	cals_struct_period_normal_location *nosl = NULL;
	cals_struct_period_normal_alarm *nosa = NULL;

	retv_if(NULL == event || NULL == event->user_data, 0);
	retv_if(NULL == field, 0);

	switch(event->event_type)
	{
	case CAL_STRUCT_TYPE_SCHEDULE:
	case CAL_STRUCT_TYPE_TODO:
		sch_rec = (cal_sch_full_t*)event->user_data;

		if (0 == strcmp(field, CALS_VALUE_LLI_DTSTART_UTIME))
			return sch_rec->dtstart_utime;
		else if (0 == strcmp(field, CALS_VALUE_LLI_DTEND_UTIME))
			return sch_rec->dtend_utime;
		else if (0 == strcmp(field, CALS_VALUE_LLI_LASTMOD))
			return sch_rec->last_mod;
		else if (0 == strcmp(field, CALS_VALUE_LLI_RRULE_UNTIL_UTIME))
			return sch_rec->until_utime;
		else if(0 == strcmp(field,CAL_VALUE_LLI_CREATED_TIME))
			return sch_rec->created_time;
		else if(0 == strcmp(field,CAL_VALUE_LLI_COMPLETED_TIME))
			return sch_rec->completed_time;
		else
			ERR("Can not find the field(%s)",field);
		break;

	case CAL_STRUCT_TYPE_CALENDAR:
		break;

	case CALS_STRUCT_TYPE_PERIOD_NORMAL_ONOFF:
		nof = event->user_data;
		if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_ONOFF_LLI_DTSTART_UTIME))
			return nof->dtstart_utime;
		else
			ERR("Can't find field(%s)", field);
		break;

	case CALS_STRUCT_TYPE_PERIOD_NORMAL_BASIC:
		nbs = event->user_data;
		if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_BASIC_LLI_DTSTART_UTIME))
			return nbs->dtstart_utime;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_BASIC_LLI_DTEND_UTIME))
			return nbs->dtend_utime;
		else
			ERR("Can't find field(%s)", field);
		break;

	case CALS_STRUCT_TYPE_PERIOD_NORMAL_OSP:
		nosp = event->user_data;
		if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_OSP_LLI_DTSTART_UTIME))
			return nosp->dtstart_utime;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_OSP_LLI_DTEND_UTIME))
			return nosp->dtend_utime;
		else
			ERR("Can't find field(%s)", field);
		break;

	case CALS_STRUCT_TYPE_PERIOD_NORMAL_LOCATION:
		nosl = event->user_data;
		if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_LOCATION_LLI_DTSTART_UTIME))
			return nosl->dtstart_utime;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_LOCATION_LLI_DTEND_UTIME))
			return nosl->dtend_utime;
		else
			ERR("Can't find field(%s)", field);
		break;

	case CALS_STRUCT_TYPE_PERIOD_NORMAL_ALARM:
		nosa = event->user_data;
		if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_ALARM_LLI_DTSTART_UTIME))
			return nosa->dtstart_utime;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_ALARM_LLI_DTEND_UTIME))
			return nosa->dtend_utime;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_ALARM_LLI_ALARM_UTIME))
			return nosa->alarm_utime;
		else
			ERR("Can't find field(%s)", field);
		break;

	default:
		break;
	}
	return 0;
}

API int calendar_svc_struct_set_lli(cal_struct *event, const char *field, long long int value)
{
	cal_sch_full_t *sch_rec = NULL;
	cals_struct_period_normal_onoff *nof = NULL;
	cals_struct_period_normal_basic *nbs = NULL;
	cals_struct_period_normal_osp *nosp = NULL;
	cals_struct_period_normal_location *nosl = NULL;
	cals_struct_period_normal_alarm *nosa = NULL;

	retv_if(NULL == event || NULL == event->user_data, CAL_ERR_ARG_NULL);

	switch(event->event_type)
	{
	case CAL_STRUCT_TYPE_SCHEDULE:
	case CAL_STRUCT_TYPE_TODO:

		sch_rec = (cal_sch_full_t*)event->user_data;

		if (0 == strcmp(field, CALS_VALUE_LLI_DTSTART_UTIME))
			sch_rec->dtstart_utime = value;
		else if (0 == strcmp(field, CALS_VALUE_LLI_DTEND_UTIME))
			sch_rec->dtend_utime = value;
		else if (0 == strcmp(field, CALS_VALUE_LLI_LASTMOD))
			sch_rec->last_mod = value;
		else if (0 == strcmp(field, CALS_VALUE_LLI_RRULE_UNTIL_UTIME))
			sch_rec->until_utime = value;
		else if(0 == strcmp(field,CAL_VALUE_LLI_CREATED_TIME))
			sch_rec->created_time = value;
		else if(0 == strcmp(field,CAL_VALUE_LLI_COMPLETED_TIME))
			sch_rec->completed_time = value;
		else
			return CAL_ERR_ARG_INVALID;
		break;

	case CAL_STRUCT_TYPE_CALENDAR:
		break;

	case CALS_STRUCT_TYPE_PERIOD_NORMAL_ONOFF:
		nof = event->user_data;
		if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_ONOFF_LLI_DTSTART_UTIME))
			nof->dtstart_utime = value;
		else {
			ERR("Can't find field(%s)", field);
			return CAL_ERR_ARG_INVALID;
		}
		break;

	case CALS_STRUCT_TYPE_PERIOD_NORMAL_BASIC:
		nbs = event->user_data;
		if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_BASIC_LLI_DTSTART_UTIME))
			nbs->dtstart_utime = value;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_BASIC_LLI_DTEND_UTIME))
			nbs->dtend_utime = value;
		else {
			ERR("Can't find field(%s)", field);
			return CAL_ERR_ARG_INVALID;
		}
		break;

	case CALS_STRUCT_TYPE_PERIOD_NORMAL_OSP:
		nosp = event->user_data;
		if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_OSP_LLI_DTSTART_UTIME))
			nosp->dtstart_utime = value;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_OSP_LLI_DTEND_UTIME))
			nosp->dtend_utime = value;
		else {
			ERR("Can't find field(%s)", field);
			return CAL_ERR_ARG_INVALID;
		}
		break;

	case CALS_STRUCT_TYPE_PERIOD_NORMAL_LOCATION:
		nosl = event->user_data;
		if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_LOCATION_LLI_DTSTART_UTIME))
			nosl->dtstart_utime = value;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_LOCATION_LLI_DTEND_UTIME))
			nosl->dtend_utime = value;
		else {
			ERR("Can't find field(%s)", field);
			return CAL_ERR_ARG_INVALID;
		}
		break;

	case CALS_STRUCT_TYPE_PERIOD_NORMAL_ALARM:
		nosa = event->user_data;
		if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_ALARM_LLI_DTSTART_UTIME))
			nosa->dtstart_utime = value;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_ALARM_LLI_DTEND_UTIME))
			nosa->dtend_utime = value;
		else if (!strcmp(field, CALS_LIST_PERIOD_NORMAL_ALARM_LLI_ALARM_UTIME))
			nosa->alarm_utime = value;
		else {
			ERR("Can't find field(%s)", field);
			return CAL_ERR_ARG_INVALID;
		}
		break;

	default:
		break;
	}

	return CAL_SUCCESS;
}


