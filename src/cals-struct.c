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
#include "cals-tz-utils.h"
#include "cals-db.h"
#include "cals-utils.h"

int cals_init_full_record(cal_sch_full_t *sch_full_record)
{
	time_t t = time(NULL);
	tzset();
	struct tm * cur_time = NULL;//localtime(&t);
	struct tm ttm;

	cur_time = localtime_r(&t,&ttm);
	if(NULL == cur_time)
	{
		return CAL_ERR_FAIL;
	}
	struct tm temp_date_time = {0};
	struct tm current_time = {0};

	retvm_if(NULL == sch_full_record, CAL_ERR_ARG_INVALID , "sch_full_record is NULL");

	memset(sch_full_record,0,sizeof(cal_sch_full_t));

	sch_full_record->cal_type = CAL_EVENT_SCHEDULE_TYPE;
	sch_full_record->sch_category = CAL_SCH_APPOINTMENT;
	memset(&temp_date_time, 0 ,sizeof(struct tm));
	memset(&sch_full_record->start_date_time, 0 ,sizeof(struct tm));
	memset(&sch_full_record->end_date_time, 0 ,sizeof(struct tm));
	memset(&sch_full_record->repeat_end_date, 0 ,sizeof(struct tm));
	memset(&sch_full_record->last_modified_time, 0 ,sizeof(struct tm));
	memset(&sch_full_record->created_date_time, 0 ,sizeof(struct tm));
	memset(&sch_full_record->completed_date_time, 0 ,sizeof(struct tm));
	memcpy(&current_time,cur_time,sizeof(struct tm));

	memcpy(&sch_full_record->start_date_time,&current_time,sizeof(struct tm));
	memcpy(&sch_full_record->end_date_time,&current_time,sizeof(struct tm));
	sch_full_record->start_date_time.tm_sec = 0;
	sch_full_record->end_date_time.tm_sec = 0;
	sch_full_record->end_date_time.tm_hour ++;
	if (sch_full_record->end_date_time.tm_hour > 23)
	{
		sch_full_record->end_date_time.tm_hour = 0;
		cal_db_service_get_tomorrow(&sch_full_record->end_date_time);
	}

	cal_db_service_get_next_month(&sch_full_record->repeat_end_date);
	temp_date_time.tm_year = TM_YEAR_MIN;
	temp_date_time.tm_mon = MONTH_MIN;
	temp_date_time.tm_mday = MONTH_DAY_MIN;
	memcpy(&sch_full_record->repeat_end_date,&temp_date_time,sizeof(struct tm));
	memcpy(&sch_full_record->last_modified_time,&temp_date_time,sizeof(struct tm));
	memcpy(&sch_full_record->created_date_time,&temp_date_time,sizeof(struct tm));
	memcpy(&sch_full_record->completed_date_time,&temp_date_time,sizeof(struct tm));

	sch_full_record->index = CAL_INVALID_INDEX;
	sch_full_record->repeat_term = CAL_REPEAT_NONE;
	sch_full_record->repeat_until_type = CALS_REPEAT_UNTIL_TYPE_NONE;
	sch_full_record->day_date = 1;
	sch_full_record->timezone = -1;
	sch_full_record->contact_id = CAL_INVALID_INDEX;
	sch_full_record->calendar_type = CAL_PHONE_CALENDAR;
	sch_full_record->calendar_id = CAL_INVALID_INDEX;
	sch_full_record->attendee_list = NULL;
	sch_full_record->meeting_category = NULL;
	sch_full_record->busy_status = 2;
	sch_full_record->summary = NULL;
	sch_full_record->description = NULL;
	sch_full_record->location= NULL;
	sch_full_record->organizer_email = NULL;
	sch_full_record->organizer_name = NULL;
	sch_full_record->uid= NULL;
	sch_full_record->gcal_id = NULL;
	sch_full_record->location_summary = NULL;
	sch_full_record->etag = NULL;
	sch_full_record->edit_uri = NULL;
	sch_full_record->gevent_id = NULL;
	sch_full_record->tz_name = NULL;
	sch_full_record->tz_city_name = NULL;
	sch_full_record->original_event_id = CAL_INVALID_INDEX;

	// TODO : To define default value.
	sch_full_record->sync_status = CAL_SYNC_STATUS_NEW;
	sch_full_record->timezone = CAL_TIME_ZONE_GMT_L11;
	sch_full_record->is_deleted = 0;
	sch_full_record->account_id = -1;
	sch_full_record->calendar_id = DEFAULT_CALENDAR_ID;

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

		ret = cals_init_full_record(user_data);
		if(ret) {
			free(user_data);
			ERR("cals_init_full_record() Failed(%d)", ret);
			return NULL;
		}
	} else if (0 == strcmp(event_type, CAL_STRUCT_TODO)) {
		type = CAL_STRUCT_TYPE_TODO;

		user_data = (cal_sch_full_t*)malloc(sizeof(cal_sch_full_t));
		retvm_if(NULL == user_data, NULL, "malloc(cal_sch_full_t:todo) Failed(%d)", errno);

		ret = cals_init_full_record(user_data);
		if(ret) {
			free(user_data);
			ERR("cals_init_full_record() Failed(%d)", ret);
			return NULL;
		}

		((cal_sch_full_t*)user_data)->cal_type = CAL_EVENT_TODO_TYPE;
		((cal_sch_full_t*)user_data)->end_date_time.tm_year = 69;
		((cal_sch_full_t*)user_data)->sch_category = CAL_SCH_NONE;
	} else if (0 == strcmp(event_type, CAL_STRUCT_CALENDAR)) {
		type = CAL_STRUCT_TYPE_CALENDAR;

		user_data = calloc(1, sizeof(calendar_t));
		retvm_if(NULL == user_data, NULL, "calloc(calendar_t) Failed(%d)", errno);

		cals_init_calendar_record(user_data);
	} else if (0 == strcmp(event_type, CAL_STRUCT_TIMEZONE))	{
		type = CAL_STRUCT_TYPE_TIMEZONE;

		user_data = (cal_timezone_t*)calloc(1, sizeof(cal_timezone_t));
		retvm_if(NULL == user_data, NULL, "calloc(cal_timezone_t) Failed(%d)", errno);
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
		cal_db_service_free_full_record((cal_sch_full_t*)(*event)->user_data,&ret);
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
		else if(0 == strcmp(field,CAL_VALUE_TXT_WEEK_FLAG))
		{
			return (sch_rec->week_flag);
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
		else if(0 == strcmp(field,CAL_VALUE_TXT_TZ_NAME))
		{
			return (sch_rec->tz_name);
		}
		else if(0 == strcmp(field,CAL_VALUE_TXT_TZ_CITY_NAME))
		{
			return (sch_rec->tz_city_name);
		}
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
	default:
		ERR("Can not find the field!(%s)!", field);
		return NULL;
	}
}

API int calendar_svc_struct_get_int(cal_struct *event, const char *field)
{
	cal_sch_full_t * sch_rec = NULL;
	calendar_t * cal_rec = NULL;
	cal_timezone_t *tz_rec = NULL;

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
		else if(0 == strcmp(field, CAL_VALUE_INT_CATEGORY))
		{
			return sch_rec->sch_category;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_REPEAT_TERM))
		{
			return sch_rec->repeat_term;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_REPEAT_INTERVAL))
		{
			return sch_rec->repeat_interval;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_REPEAT_UNTIL_TYPE))
		{
			return sch_rec->repeat_until_type;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_REPEAT_OCCURRENCES))
		{
			return sch_rec->repeat_occurrences;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_DAY_DATE))
		{
			return sch_rec->day_date;
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
		else if(0 == strcmp(field,CAL_VALUE_INT_ALL_DAY_EVENT))
		{
			return sch_rec->all_day_event;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_SYNC_STATUS))
		{
			return sch_rec->sync_status;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_SUN_MOON))
		{
			return sch_rec->sun_moon;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_PRIORITY))
		{
			return sch_rec->priority;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_WEEK_START))
		{
			return sch_rec->week_start;
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
			return sch_rec->deleted;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_PROGRESS))
		{
			return sch_rec->progress;
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
	default:
		break;

	}

	return 0;
}

API time_t calendar_svc_struct_get_time (cal_struct *event, const char *field, int timezone_flag)
{
	time_t ret_time = 0;
	cal_sch_full_t * sch_rec = NULL;

	retv_if(NULL == event, CAL_ERR_ARG_NULL);
	retv_if(NULL == field, CAL_ERR_ARG_NULL);
	retv_if(NULL == event->user_data, CAL_ERR_ARG_INVALID);

	switch(event->event_type)
	{
	case CAL_STRUCT_TYPE_SCHEDULE:
	case CAL_STRUCT_TYPE_TODO:
		sch_rec = (cal_sch_full_t*)event->user_data;
		if(sch_rec==NULL)
		{
			return CAL_ERR_ARG_NULL;
		}

		if(0 == strcmp(field,CAL_VALUE_GMT_START_DATE_TIME))
		{
			ret_time = cals_mktime(&(sch_rec->start_date_time));
		}
		else if(0 == strcmp(field,CAL_VALUE_GMT_END_DATE_TIME))
		{
			ret_time = cals_mktime(&(sch_rec->end_date_time));
		}
		else if(0 == strcmp(field,CAL_VALUE_GMT_REPEAT_END_DATE))
		{
			ret_time = cals_mktime(&(sch_rec->repeat_end_date));
		}
		else if(0 == strcmp(field,CAL_VALUE_GMT_LAST_MODIFIED_TIME))
		{
			ret_time = cals_mktime(&(sch_rec->last_modified_time));
		}
		else if(0 == strcmp(field,CAL_VALUE_GMT_CREATED_DATE_TIME))
		{
			ret_time = cals_mktime(&(sch_rec->created_date_time));
		}
		else if(0 == strcmp(field,CAL_VALUE_GMT_COMPLETED_DATE_TIME))
		{
			ret_time = cals_mktime(&(sch_rec->completed_date_time));
		}
		else
		{
		}
		break;

	case CAL_STRUCT_TYPE_CALENDAR:
		return 0;
		break;

	default:
		return 0;
		break;
	}

	if(timezone_flag != CAL_TZ_FLAG_GMT && sch_rec->all_day_event==false)
	{
		time_t temp = 0;
		calendar_svc_util_gmt_to_local(ret_time,&temp);
		ret_time = temp;
	}

	return ret_time;
}

API struct tm* calendar_svc_struct_get_tm(cal_struct* record, const char *field, int timezone_flag)
{
	struct tm* ret_tm = 0;
	cal_sch_full_t * sch_rec = NULL;

	retv_if(NULL == record, NULL);
	retv_if(NULL == field, NULL);
	retv_if(NULL == record->user_data, NULL);

	switch(record->event_type)
	{
	case CAL_STRUCT_TYPE_SCHEDULE:
	case CAL_STRUCT_TYPE_TODO:
		sch_rec = (cal_sch_full_t*)record->user_data;
		if(sch_rec==NULL)
		{
			return NULL;
		}

		if(0 == strcmp(field,CAL_VALUE_GMT_START_DATE_TIME))
		{
			ret_tm = &(sch_rec->start_date_time);
		}
		else if(0 == strcmp(field,CAL_VALUE_GMT_END_DATE_TIME))
		{
			ret_tm = &(sch_rec->end_date_time);
		}
		else if(0 == strcmp(field,CAL_VALUE_GMT_REPEAT_END_DATE))
		{
			ret_tm = &(sch_rec->repeat_end_date);
		}
		else if(0 == strcmp(field,CAL_VALUE_GMT_LAST_MODIFIED_TIME))
		{
			ret_tm = &(sch_rec->last_modified_time);
		}
		else if(0 == strcmp(field,CAL_VALUE_GMT_CREATED_DATE_TIME))
		{
			ret_tm = &(sch_rec->created_date_time);
		}
		else if(0 == strcmp(field,CAL_VALUE_GMT_COMPLETED_DATE_TIME))
		{
			ret_tm = &(sch_rec->completed_date_time);
		}
		else
		{
		}
		break;

	case CAL_STRUCT_TYPE_CALENDAR:
		return NULL;
		break;

	default:
		return NULL;
		break;
	}


	if(timezone_flag != CAL_TZ_FLAG_GMT && NULL != ret_tm && sch_rec->all_day_event==false)
	{
		time_t temp = 0;
		time_t input_tt = 0;

		input_tt = cals_mktime(ret_tm);

		calendar_svc_util_gmt_to_local(input_tt,&temp);
		input_tt = temp;
		return cals_tmtime(&input_tt);
	}

	//TMDUMP(*ret_tm);
	return ret_tm;
}


API int calendar_svc_struct_set_int (cal_struct *event, const char *field, int intval)
{
	cal_sch_full_t * sch_rec = NULL;
	calendar_t * cal_rec = NULL;
	cal_timezone_t *tz_rec = NULL;

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
		else if(0 == strcmp(field, CAL_VALUE_INT_CATEGORY))
		{
			sch_rec->sch_category = intval;
		}
		else if(0 == strcmp(field, CAL_VALUE_INT_ALL_DAY_EVENT))
		{
			sch_rec->all_day_event = !!(intval);
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_REPEAT_TERM))
		{
			sch_rec->repeat_term = intval;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_REPEAT_INTERVAL))
		{
			sch_rec->repeat_interval = intval;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_REPEAT_UNTIL_TYPE))
		{
			sch_rec->repeat_until_type = intval;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_REPEAT_OCCURRENCES))
		{
			sch_rec->repeat_occurrences = intval;
			sch_rec->repeat_end_date.tm_year = BASE_TIME_YEAR;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_DAY_DATE))
		{
			sch_rec->day_date = intval;
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
		else if(0 == strcmp(field,CAL_VALUE_INT_SUN_MOON))
		{
			sch_rec->sun_moon = intval;
		}
		else if(0 == strcmp(field,CAL_VALUE_INT_WEEK_START))
		{
			sch_rec->week_start = intval;
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
	int str_len = strlen(strval)+1;

	switch(event->event_type)
	{
	case CAL_STRUCT_TYPE_SCHEDULE:
	case CAL_STRUCT_TYPE_TODO:
		sch_rec = (cal_sch_full_t*)event->user_data;
		if(0 == strcmp(field,CAL_VALUE_TXT_SUMMARY))
		{
			CAL_FREE(sch_rec->summary);

			sch_rec->summary = (char*)malloc(str_len);
			retex_if(NULL == sch_rec->summary,,"[ERROR]calendar_svc_struct_set_str:Failed to malloc!\n");
			memset(sch_rec->summary,0x00,str_len);

			strcpy(sch_rec->summary,strval);
		}
		else if(0 == strcmp(field,CAL_VALUE_TXT_DESCRIPTION))
		{
			CAL_FREE(sch_rec->description);

			sch_rec->description = (char*)malloc(str_len);
			retex_if(NULL == sch_rec->description,,"[ERROR]calendar_svc_struct_set_str:Failed to malloc!\n");
			memset(sch_rec->description,0x00,str_len);

			strcpy(sch_rec->description,strval);
		}
		else if(0 == strcmp(field, CAL_VALUE_TXT_LOCATION))
		{
			CAL_FREE(sch_rec->location);


			sch_rec->location = (char*)malloc(str_len);
			retex_if(NULL == sch_rec->location,,"[ERROR]calendar_svc_struct_set_str:Failed to malloc!\n");
			memset(sch_rec->location,0x00,str_len);

			strcpy(sch_rec->location,strval);
		}
		else if(0 == strcmp(field,CAL_VALUE_TXT_WEEK_FLAG))
		{
			CAL_FREE(sch_rec->week_flag);

			sch_rec->week_flag = (char*)malloc(str_len);
			retex_if(NULL == sch_rec->week_flag,,"[ERROR]calendar_svc_struct_set_str:Failed to malloc!\n");
			memset(sch_rec->week_flag,0x00,str_len);

			strcpy(sch_rec->week_flag,strval);
		}
		else if(0 == strcmp(field,CAL_VALUE_TXT_UID))
		{
			CAL_FREE(sch_rec->uid);

			sch_rec->uid = (char*)malloc(str_len);
			retex_if(NULL == sch_rec->uid,,"[ERROR]calendar_svc_struct_set_str:Failed to malloc!\n");
			memset(sch_rec->uid,0x00,str_len);

			strcpy(sch_rec->uid,strval);
		}
		else if(0 == strcmp(field,CAL_VALUE_TXT_ORGANIZER_NAME))
		{
			CAL_FREE(sch_rec->organizer_name);


			sch_rec->organizer_name = (char*)malloc(str_len);
			retex_if(NULL == sch_rec->organizer_name,,"[ERROR]calendar_svc_struct_set_str:Failed to malloc!\n");
			memset(sch_rec->organizer_name,0x00,str_len);

			strcpy(sch_rec->organizer_name,strval);
		}
		else if(0 == strcmp(field,CAL_VALUE_TXT_ORGANIZER_EMAIL  ))
		{
			CAL_FREE(sch_rec->organizer_email);

			sch_rec->organizer_email = (char*)malloc(str_len);
			retex_if(NULL == sch_rec->organizer_email,,"[ERROR]calendar_svc_struct_set_str:Failed to malloc!\n");
			memset(sch_rec->organizer_email,0x00,str_len);

			strcpy(sch_rec->organizer_email,strval);
		}
		else if(0 == strcmp(field,CAL_VALUE_TXT_GCAL_ID ))
		{
			CAL_FREE(sch_rec->gcal_id);


			sch_rec->gcal_id = (char*)malloc(str_len);
			retex_if(NULL == sch_rec->gcal_id,,"[ERROR]calendar_svc_struct_set_str:Failed to malloc!\n");
			memset(sch_rec->gcal_id,0x00,str_len);

			strcpy(sch_rec->gcal_id,strval);
		}
		else if(0 == strcmp(field,CAL_VALUE_TXT_UPDATED))
		{
			CAL_FREE(sch_rec->updated);


			sch_rec->updated = (char*)malloc(str_len);
			retex_if(NULL == sch_rec->updated,,"[ERROR]calendar_svc_struct_set_str:Failed to malloc!\n");
			memset(sch_rec->updated,0x00,str_len);

			strcpy(sch_rec->updated,strval);
		}
		else if(0 == strcmp(field,CAL_VALUE_TXT_LOCATION_SUMMARY))
		{
			CAL_FREE(sch_rec->location_summary);


			sch_rec->location_summary = (char*)malloc(str_len);
			retex_if(NULL == sch_rec->location_summary,,"[ERROR]calendar_svc_struct_set_str:Failed to malloc!\n");
			memset(sch_rec->location_summary,0x00,str_len);

			strcpy(sch_rec->location_summary,strval);
		}
		else if(0 == strcmp(field, CAL_VALUE_TXT_ETAG))
		{
			CAL_FREE(sch_rec->etag);

			sch_rec->etag = (char*)malloc(str_len);
			retex_if(NULL == sch_rec->etag,,"[ERROR]calendar_svc_struct_set_str:Failed to malloc!\n");
			memset(sch_rec->etag,0x00,str_len);

			strcpy(sch_rec->etag,strval);
		}
		else if(0 == strcmp(field,CAL_VALUE_TXT_EDIT_URL))
		{
			CAL_FREE(sch_rec->edit_uri);

			sch_rec->edit_uri = (char*)malloc(str_len);
			retex_if(NULL == sch_rec->edit_uri,,"[ERROR]calendar_svc_struct_set_str:Failed to malloc!\n");
			memset(sch_rec->edit_uri,0x00,str_len);

			strcpy(sch_rec->edit_uri,strval);
		}
		else if(0 == strcmp(field,CAL_VALUE_TXT_GEDERID))
		{
			CAL_FREE(sch_rec->gevent_id);

			sch_rec->gevent_id = (char*)malloc(str_len);
			retex_if(NULL == sch_rec->gevent_id,,"[ERROR]calendar_svc_struct_set_str:Failed to malloc!\n");
			memset(sch_rec->gevent_id,0x00,str_len);

			strcpy(sch_rec->gevent_id,strval);
		}
		else if(0 == strcmp(field,CAL_VALUE_TXT_TZ_NAME))
		{
			CAL_FREE(sch_rec->tz_name);


			sch_rec->tz_name = (char*)malloc(str_len);
			retex_if(NULL == sch_rec->tz_name,,"[ERROR]calendar_svc_struct_set_str:Failed to malloc!\n");
			memset(sch_rec->tz_name,0x00,str_len);

			strcpy(sch_rec->tz_name,strval);
		}
		else if(0 == strcmp(field,CAL_VALUE_TXT_TZ_CITY_NAME))
		{
			CAL_FREE(sch_rec->tz_city_name);


			sch_rec->tz_city_name = (char*)malloc(str_len);
			retex_if(NULL == sch_rec->tz_city_name,,"[ERROR]calendar_svc_struct_set_str:Failed to malloc!\n");
			memset(sch_rec->tz_city_name,0x00,str_len);

			strcpy(sch_rec->tz_city_name,strval);
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

			cal_rec->calendar_id = (char*)malloc(str_len);
			retex_if(NULL == cal_rec->calendar_id,,"[ERROR]calendar_svc_struct_set_str:Failed to malloc!\n");
			memset(cal_rec->calendar_id,0x00,str_len);

			strcpy(cal_rec->calendar_id,strval);
		}
		else if(0 == strcmp(field,CAL_TABLE_TXT_UID))
		{
			CAL_FREE(cal_rec->uid);


			cal_rec->uid = (char*)malloc(str_len);
			retex_if(NULL == cal_rec->uid,,"[ERROR]calendar_svc_struct_set_str:Failed to malloc!\n");
			memset(cal_rec->uid,0x00,str_len);

			strcpy(cal_rec->uid,strval);
		}
		else if(0 == strcmp(field,CAL_TABLE_TXT_LINK))
		{
			CAL_FREE(cal_rec->link);


			cal_rec->link = (char*)malloc(str_len);
			retex_if(NULL == cal_rec->link,,"[ERROR]calendar_svc_struct_set_str:Failed to malloc!\n");
			memset(cal_rec->link,0x00,str_len);

			strcpy(cal_rec->link,strval);
		}
		else if(0 == strcmp(field,CAL_TABLE_TXT_NAME))
		{
			CAL_FREE(cal_rec->name);


			cal_rec->name = (char*)malloc(str_len);
			retex_if(NULL == cal_rec->name,,"[ERROR]calendar_svc_struct_set_str:Failed to malloc!\n");
			memset(cal_rec->name,0x00,str_len);

			strcpy(cal_rec->name,strval);
		}
		else if(0 == strcmp(field,CAL_TABLE_TXT_DESCRIPTION))
		{
			CAL_FREE(cal_rec->description);


			cal_rec->description = (char*)malloc(str_len);
			retex_if(NULL == cal_rec->description,,"[ERROR]calendar_svc_struct_set_str:Failed to malloc!\n");
			memset(cal_rec->description,0x00,str_len);

			strcpy(cal_rec->description,strval);
		}
		else if(0 == strcmp(field,CAL_TABLE_TXT_AUTHOR))
		{
			CAL_FREE(cal_rec->author);

			cal_rec->author = (char*)malloc(str_len);
			retex_if(NULL == cal_rec->author,,"[ERROR]calendar_svc_struct_set_str:Failed to malloc!\n");
			memset(cal_rec->author,0x00,str_len);

			strcpy(cal_rec->author,strval);
		}
		else if(0 == strcmp(field,CAL_TABLE_TXT_COLOR))
		{
			CAL_FREE(cal_rec->color);


			cal_rec->color = (char*)malloc(str_len);
			retex_if(NULL == cal_rec->color,,"[ERROR]calendar_svc_struct_set_str:Failed to malloc!\n");
			memset(cal_rec->color,0x00,str_len);

			strcpy(cal_rec->color,strval);
		}
		else if(0 == strcmp(field,CAL_TABLE_TXT_LOCATION))
		{
			CAL_FREE(cal_rec->location);

			cal_rec->location = (char*)malloc(str_len);
			retex_if(NULL == cal_rec->location,,"[ERROR]calendar_svc_struct_set_str:Failed to malloc!\n");
			memset(cal_rec->location,0x00,str_len);

			strcpy(cal_rec->location,strval);
		}
		else if(0 == strcmp(field,CAL_TABLE_TXT_TIME_ZONE_LABEL))
		{
			CAL_FREE(cal_rec->timezone_label);

			cal_rec->timezone_label = (char*)malloc(str_len);
			retex_if(NULL == cal_rec->timezone_label,,"[ERROR]calendar_svc_struct_set_str:Failed to malloc!\n");
			memset(cal_rec->timezone_label,0x00,str_len);

			strcpy(cal_rec->timezone_label,strval);
		}
		else if(0 == strcmp(field,CAL_TABLE_TXT_USER_LOCATION))
		{
			CAL_FREE(cal_rec->user_location);


			cal_rec->user_location = (char*)malloc(str_len);
			retex_if(NULL == cal_rec->user_location,,"[ERROR]calendar_svc_struct_set_str:Failed to malloc!\n");
			memset(cal_rec->user_location,0x00,str_len);

			strcpy(cal_rec->user_location,strval);
		}
		else if(0 == strcmp(field,CAL_TABLE_TXT_WEATHER))
		{
			CAL_FREE(cal_rec->weather);


			cal_rec->weather = (char*)malloc(str_len);
			retex_if(NULL == cal_rec->weather,,"[ERROR]calendar_svc_struct_set_str:Failed to malloc!\n");
			memset(cal_rec->weather,0x00,str_len);

			strcpy(cal_rec->weather,strval);
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


			tz_rec->standard_name = (char*)malloc(str_len);
			retex_if(NULL == tz_rec->standard_name,,"[ERROR]calendar_svc_struct_set_str:Failed to malloc!\n");
			memset(tz_rec->standard_name,0x00,str_len);

			strcpy(tz_rec->standard_name,strval);
		}
		else if(0 == strcmp(field,CAL_TZ_VALUE_TXT_DST_NAME))
		{
			CAL_FREE(tz_rec->day_light_name);

			tz_rec->day_light_name = (char*)malloc(str_len);
			retex_if(NULL == tz_rec->day_light_name,,"[ERROR]calendar_svc_struct_set_str:Failed to malloc!\n");
			memset(tz_rec->day_light_name,0x00,str_len);

			strcpy(tz_rec->day_light_name,strval);
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

API int calendar_svc_struct_set_time (cal_struct *event, const char *field,int timezone_flag, time_t time)
{
	cal_sch_full_t *sch_rec = NULL;

	retv_if(NULL == event, CAL_ERR_ARG_NULL);
	retv_if(NULL == field, CAL_ERR_ARG_NULL);

	if(timezone_flag != CAL_TZ_FLAG_GMT)
	{
		time_t temp = 0;
		calendar_svc_util_local_to_gmt(time,&temp);
		time = temp;
	}

	switch(event->event_type)
	{
	case CAL_STRUCT_TYPE_SCHEDULE:
	case CAL_STRUCT_TYPE_TODO:

		sch_rec = (cal_sch_full_t*)event->user_data;
		if(sch_rec==NULL)
		{
			return CAL_ERR_ARG_NULL;
		}
		if(0 == strcmp(field,CAL_VALUE_GMT_START_DATE_TIME))
		{
			cal_db_service_copy_struct_tm((struct tm*)cals_tmtime(&time),&(sch_rec->start_date_time));
		}
		else if(0 == strcmp(field,CAL_VALUE_GMT_END_DATE_TIME))
		{
			cal_db_service_copy_struct_tm((struct tm*)cals_tmtime(&time),&(sch_rec->end_date_time));
		}
		else if(0 == strcmp(field,CAL_VALUE_GMT_REPEAT_END_DATE))
		{
			cal_db_service_copy_struct_tm((struct tm*)cals_tmtime(&time),&(sch_rec->repeat_end_date));
		}
		else if(0 == strcmp(field,CAL_VALUE_GMT_LAST_MODIFIED_TIME))
		{
			cal_db_service_copy_struct_tm((struct tm*)cals_tmtime(&time),&(sch_rec->last_modified_time));
		}
		else if(0 == strcmp(field,CAL_VALUE_GMT_CREATED_DATE_TIME))
		{
			cal_db_service_copy_struct_tm((struct tm*)cals_tmtime(&time),&(sch_rec->created_date_time));
		}
		else if(0 == strcmp(field,CAL_VALUE_GMT_COMPLETED_DATE_TIME))
		{
			cal_db_service_copy_struct_tm((struct tm*)cals_tmtime(&time),&(sch_rec->completed_date_time));
		}
		else
		{
		}

		break;

	case CAL_STRUCT_TYPE_CALENDAR:

		break;

	default:
		break;
	}

	return CAL_SUCCESS;
}


API int calendar_svc_struct_set_tm(cal_struct* record, const char *field, int timezone_flag,struct tm* time)
{
	cal_sch_full_t * sch_rec = NULL;
	struct tm input_tm ={0};

	retv_if(NULL == record, CAL_ERR_ARG_NULL);
	retv_if(NULL == field, CAL_ERR_ARG_NULL);

	if(timezone_flag != CAL_TZ_FLAG_GMT)
	{
		time_t temp = 0;
		time_t input_tt = 0;

		input_tt = cals_mktime(time);
		calendar_svc_util_local_to_gmt(input_tt,&temp);
		input_tt = temp;
		cals_tmtime_r(&input_tt,&input_tm);
	}
	else
		memcpy(&input_tm,time,sizeof(struct tm));

	switch(record->event_type)
	{
	case CAL_STRUCT_TYPE_SCHEDULE:
	case CAL_STRUCT_TYPE_TODO:

		sch_rec = (cal_sch_full_t*)record->user_data;
		if(sch_rec==NULL)
		{
			return CAL_ERR_ARG_NULL;
		}
		if(0 == strcmp(field,CAL_VALUE_GMT_START_DATE_TIME))
		{
			cal_db_service_copy_struct_tm(&input_tm,&(sch_rec->start_date_time));
			TMDUMP(input_tm);
		}
		else if(0 == strcmp(field,CAL_VALUE_GMT_END_DATE_TIME))
		{
			cal_db_service_copy_struct_tm(&input_tm,&(sch_rec->end_date_time));
			TMDUMP(input_tm);
		}
		else if(0 == strcmp(field,CAL_VALUE_GMT_REPEAT_END_DATE))
		{
			cal_db_service_copy_struct_tm(&input_tm,&(sch_rec->repeat_end_date));
		}
		else if(0 == strcmp(field,CAL_VALUE_GMT_LAST_MODIFIED_TIME))
		{
			cal_db_service_copy_struct_tm(&input_tm,&(sch_rec->last_modified_time));
		}
		else if(0 == strcmp(field,CAL_VALUE_GMT_CREATED_DATE_TIME))
		{
			cal_db_service_copy_struct_tm(&input_tm,&(sch_rec->created_date_time));
		}
		else if(0 == strcmp(field,CAL_VALUE_GMT_COMPLETED_DATE_TIME))
		{
			cal_db_service_copy_struct_tm(&input_tm,&(sch_rec->completed_date_time));
		}
		else
		{
		}

		break;

	case CAL_STRUCT_TYPE_CALENDAR:

		break;

	default:
		break;
	}

	return CAL_SUCCESS;
}


API double calendar_svc_struct_get_double(cal_struct* record, const char *field)
{
	double ret_val = 0.0;
	cal_sch_full_t *sch_rec = NULL;

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
	default:
		ERR("Unknown event type(%d)", record->event_type);
		break;
	}

	return ret_val;
}

API int calendar_svc_struct_set_double(cal_struct* record, const char *field,double value)
{
	cal_sch_full_t * sch_rec = NULL;

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
		if(0 == strcmp(field, CAL_VALUE_LST_MEETING_CATEGORY))
			*retlist = sch_rec->meeting_category;
		else if(0 == strcmp(field,CAL_VALUE_LST_ATTENDEE_LIST))
			*retlist = sch_rec->attendee_list;
		else if(0 == strcmp(field, CAL_VALUE_LST_EXCEPTION_DATE))
			*retlist = sch_rec->exception_date_list;
		else if(0 == strcmp(field, CAL_VALUE_LST_ALARM))
			*retlist = sch_rec->alarm_list;
		else {
			ERR("Unknown field(%s)", field);
			return CAL_ERR_ARG_INVALID;
		}
		break;
	case CAL_STRUCT_TYPE_TODO:
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
		if(0 == strcmp(field,CAL_VALUE_LST_MEETING_CATEGORY))
		{
			if(list == NULL)
				_cals_struct_remove_list(field,sch_rec->meeting_category);

			sch_rec->meeting_category = list;
		}
		else if(0 == strcmp(field,CAL_VALUE_LST_ATTENDEE_LIST))
		{
			if(list == NULL)
				_cals_struct_remove_list(field,sch_rec->attendee_list);

			sch_rec->attendee_list = list;
		}
		else if(0 == strcmp(field,CAL_VALUE_LST_EXCEPTION_DATE))
		{
			if(list == NULL)
				_cals_struct_remove_list(field,sch_rec->exception_date_list);

			sch_rec->exception_date_list = list;
		}
		else if(0 == strcmp(field,CAL_VALUE_LST_ALARM))
		{
			if(list == NULL)
				_cals_struct_remove_list(field,sch_rec->alarm_list);

			sch_rec->alarm_list = list;
		}
		break;

	case CAL_STRUCT_TYPE_CALENDAR:
	case CAL_STRUCT_TYPE_TODO:
	default:
		break;
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

	if(0 == strcmp(CAL_VALUE_LST_MEETING_CATEGORY,filed))
	{
		value->v_type = CAL_EVENT_CATEGORY;
		value->user_data = (cal_category_info_t*)malloc(sizeof(cal_category_info_t));
		retex_if(NULL == value->user_data,,"[ERROR]calendar_svc_value_new:Fail to malloc!\n");

		memset(value->user_data,0,sizeof(cal_category_info_t));
	}
	else if(0 == strcmp(CAL_VALUE_LST_ATTENDEE_LIST,filed))
	{
		value->v_type = CAL_EVENT_PATICIPANT;
		value->user_data = (cal_participant_info_t*)malloc(sizeof(cal_participant_info_t));
		retex_if(NULL == value->user_data,,"[ERROR]calendar_svc_value_new:Fail to malloc!\n");

		memset(value->user_data,0,sizeof(cal_participant_info_t));
	}
	else if(0 == strcmp(CAL_VALUE_LST_EXCEPTION_DATE,filed))
	{
		value->v_type = CAL_EVENT_RECURRENCY;
		value->user_data = (cal_exception_info_t*)malloc(sizeof(cal_exception_info_t));
		retex_if(NULL == value->user_data,,"[ERROR]calendar_svc_value_new:Fail to malloc!\n");

		memset(value->user_data,0,sizeof(cal_exception_info_t));
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

static int calendar_svc_free_category(cal_category_info_t* value)
{
	CALS_FN_CALL;
	if(NULL == value)
	{
		return CAL_SUCCESS;
	}

	CAL_FREE(value->category_name);
	CAL_FREE(value);


	return CAL_SUCCESS;
}

static int calendar_svc_free_exception_info(cal_exception_info_t* value)
{
	CALS_FN_CALL;

	int error_code = 0;

	if(NULL == value)
	{
		return CAL_SUCCESS;
	}

	if(NULL != value->exception_record)
	{
		cal_db_service_free_full_record(value->exception_record,&error_code);
		free(value->exception_record);
	}

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
	case CAL_EVENT_CATEGORY:
		calendar_svc_free_category((cal_category_info_t*)(*value)->user_data);
		break;

	case CAL_EVENT_PATICIPANT:
		calendar_svc_free_participant((cal_participant_info_t*)(*value)->user_data);
		break;

	case CAL_EVENT_RECURRENCY:
		calendar_svc_free_exception_info((cal_exception_info_t*)(*value)->user_data);
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
	//CALS_FN_CALL();
	retex_if(NULL == value || NULL == value->user_data || NULL == field,,"[ERROR]calendar_svc_value_free:Invalid parameter!\n");

	cal_category_info_t *category = NULL;
	cal_participant_info_t *participant = NULL;
	cal_exception_info_t *exception = NULL;
	cal_alarm_info_t *alarm_info = NULL;


	switch(value->v_type)
	{
	case CAL_EVENT_CATEGORY:

		category = (cal_category_info_t*)value->user_data;

		if(0 == strcmp(field,CAL_VALUE_INT_MEETING_CATEGORY_DETAIL_ID))
		{
			category->event_id = intval;
		}
		else
		{
		}
		break;

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
		}
		break;
	case CAL_EVENT_RECURRENCY:
		exception = (cal_exception_info_t*)value->user_data;
		if(0 == strcmp(field,CAL_VALUE_INT_EXCEPTION_DATE_ID))
		{
			exception->event_id = intval;
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

	default:
		break;
	}

	return CAL_SUCCESS;

CATCH:

	return CAL_ERR_FAIL;

}

API int calendar_svc_value_set_str (cal_value *value, const char *field, const char *strval)
{
	//CALS_FN_CALL();
	retex_if(NULL == value || NULL == value->user_data || NULL == field || NULL == strval,,"[ERROR]calendar_svc_value_free:Invalid parameter!\n");

	cal_category_info_t* category = NULL;
	cal_participant_info_t* participant = NULL;
	cal_alarm_info_t *alarm_info = NULL;

	int str_len = strlen(strval)+1;

	switch(value->v_type)
	{
	case CAL_EVENT_CATEGORY:

		category = (cal_category_info_t*)value->user_data;

		if(0 == strcmp(field,CAL_VALUE_TXT_MEETING_CATEGORY_DETAIL_NAME))
		{
			CAL_FREE(category->category_name);

			category->category_name = (char*)malloc(str_len);
			retex_if(NULL == category->category_name,,"[ERROR]calendar_svc_value_set_str:Failed to malloc!\n");

			memset(category->category_name ,0x00,str_len);

			strcpy(category->category_name,strval);
		}
		else
		{
		}
		break;

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
	CALS_FN_CALL;
	retex_if(NULL == value || NULL == value->user_data || NULL == field,,"[ERROR]calendar_svc_value_free:Invalid parameter!\n");

	cal_category_info_t* category = NULL;
	cal_participant_info_t* participant = NULL;
	cal_exception_info_t * exception = NULL;
	cal_alarm_info_t* alarm_info = NULL;
	switch(value->v_type)
	{
	case CAL_EVENT_CATEGORY:

		category = (cal_category_info_t*)value->user_data;

		if(0 == strcmp(field,CAL_VALUE_INT_MEETING_CATEGORY_DETAIL_ID))
		{
			return(category->event_id);
		}
		else
		{
		}
		break;

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
	case CAL_EVENT_RECURRENCY:
		exception = (cal_exception_info_t*)value->user_data;
		if(0 == strcmp(field,CAL_VALUE_INT_EXCEPTION_DATE_ID))
		{
			return exception->event_id;
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

	return CAL_SUCCESS;

CATCH:

	return CAL_ERR_FAIL;

}

API char * calendar_svc_value_get_str (cal_value *value, const char *field)
{
	CALS_FN_CALL;
	cal_category_info_t* category = NULL;
	cal_participant_info_t* participant = NULL;
	cal_alarm_info_t * alarm_info = NULL;

	retex_if(NULL == value || NULL == value->user_data || NULL == field,,"[ERROR]calendar_svc_value_free:Invalid parameter!\n");

	switch(value->v_type)
	{
	case CAL_EVENT_CATEGORY:
		category = (cal_category_info_t*)value->user_data;

		if(0 == strcmp(field,CAL_VALUE_TXT_MEETING_CATEGORY_DETAIL_NAME))
		{
			return category->category_name;
		}
		break;

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

CATCH:

	return NULL;

}


API time_t calendar_svc_value_get_time (cal_value *value, const char *field,int timezone_flag)
{
	CALS_FN_CALL;
	time_t ret_time = 0;

	retv_if(NULL == value, CAL_ERR_ARG_NULL);
	retv_if(NULL == value->user_data, CAL_ERR_ARG_INVALID);
	retv_if(NULL == field, CAL_ERR_ARG_NULL);

	switch(value->v_type)
	{
	case CAL_EVENT_RECURRENCY:
		if(0 == strcmp(field,CAL_VALUE_GMT_EXCEPTION_DATE_TIME))
		{
			cal_exception_info_t *exception_info = value->user_data;
			ret_time = cals_mktime(&(exception_info->exception_start_time));
		}
		else
		{
		}
		break;
	case CAL_EVENT_ALARM:
		if(0 == strcmp(field,CAL_VALUE_GMT_ALARMS_TIME))
		{
			cal_alarm_info_t *alarm_info = value->user_data;
			ret_time = cals_mktime(&(alarm_info->alarm_time));
		}
		else
		{
		}

		break;

	default:
		break;
	}

	if(timezone_flag != CAL_TZ_FLAG_GMT)
	{
		time_t temp = 0;
		calendar_svc_util_gmt_to_local(ret_time,&temp);
		ret_time = temp;
	}

	return ret_time;
}

API struct tm* calendar_svc_value_get_tm (cal_value *value, const char *field,int timezone_flag)
{
	CALS_FN_CALL;
	struct tm* ret_tm = 0;

	retv_if(NULL == value, NULL);
	retv_if(NULL == value->user_data, NULL);
	retv_if(NULL == field, NULL);

	switch(value->v_type)
	{
	case CAL_EVENT_RECURRENCY:
		if(0 == strcmp(field,CAL_VALUE_GMT_EXCEPTION_DATE_TIME))
		{
			cal_exception_info_t *exception_info = value->user_data;
			ret_tm = &(exception_info->exception_start_time);
		}
		else
		{
		}
		break;
	case CAL_EVENT_ALARM:
		if(0 == strcmp(field,CAL_VALUE_GMT_ALARMS_TIME))
		{
			cal_alarm_info_t *alarm_info = value->user_data;
			ret_tm = &(alarm_info->alarm_time);
		}
		else
		{
		}
		break;

	default:
		break;
	}

	if(timezone_flag != CAL_TZ_FLAG_GMT && NULL != ret_tm)
	{
		time_t temp = 0;
		time_t input_tt = 0;

		input_tt = cals_mktime(ret_tm);

		calendar_svc_util_gmt_to_local(input_tt,&temp);
		input_tt = temp;
		return cals_tmtime(&input_tt);
	}

	return ret_tm;
}



API int calendar_svc_value_set_time (cal_value *value, const char *field,int timezone_flag, time_t time)
{
	CALS_FN_CALL;

	retv_if(NULL == value, CAL_ERR_ARG_NULL);
	retv_if(NULL == value->user_data, CAL_ERR_ARG_INVALID);
	retv_if(NULL == field, CAL_ERR_ARG_NULL);

	if(timezone_flag != CAL_TZ_FLAG_GMT)
	{
		time_t temp = 0;
		calendar_svc_util_local_to_gmt(time,&temp);
		time = temp;
	}

	switch(value->v_type)
	{
	case CAL_EVENT_RECURRENCY:
		if(0 == strcmp(field,CAL_VALUE_GMT_EXCEPTION_DATE_TIME))
		{
			cal_exception_info_t *exception_info = value->user_data;
			cal_db_service_copy_struct_tm((struct tm*)cals_tmtime(&time),&(exception_info->exception_start_time));

			return CAL_SUCCESS;
		}
		else
		{

		}
		break;
	case CAL_EVENT_ALARM:
		if(0 == strcmp(field,CAL_VALUE_GMT_ALARMS_TIME))
		{
			cal_alarm_info_t *alarm_info = value->user_data;
			cal_db_service_copy_struct_tm((struct tm*)cals_tmtime(&time),&(alarm_info->alarm_time));

			return CAL_SUCCESS;
		}
		else
		{

		}
		break;

	default:
		break;
	}

	return CAL_SUCCESS;
}



API int calendar_svc_value_set_tm (cal_value *value, const char *field,int timezone_flag, struct tm* time)
{
	CALS_FN_CALL;
	struct tm input_tm ={0};

	retv_if(NULL == value, CAL_ERR_ARG_NULL);
	retv_if(NULL == value->user_data, CAL_ERR_ARG_INVALID);
	retv_if(NULL == field, CAL_ERR_ARG_NULL);

	if(timezone_flag != CAL_TZ_FLAG_GMT)
	{
		time_t temp = 0;
		time_t input_tt = 0;

		input_tt = cals_mktime(time);
		calendar_svc_util_local_to_gmt(input_tt,&temp);
		input_tt = temp;
		cals_tmtime_r(&input_tt,&input_tm);
	}
	else
		memcpy(&input_tm,time,sizeof(struct tm));


	switch(value->v_type)
	{
	case CAL_EVENT_RECURRENCY:
		if(0 == strcmp(field,CAL_VALUE_GMT_EXCEPTION_DATE_TIME))
		{
			cal_exception_info_t *exception_info = value->user_data;
			//cal_db_service_copy_struct_tm((struct tm*)&input_tm,&(exception_info->exception_start_time));
			memcpy(&(exception_info->exception_start_time),&input_tm,sizeof(struct tm));
			DBG("%d-%d-%d",exception_info->exception_start_time.tm_year,exception_info->exception_start_time.tm_mon,exception_info->exception_start_time.tm_mday);
			return CAL_SUCCESS;
		}
		else
		{

		}
		break;
	case CAL_EVENT_ALARM:
		if(0 == strcmp(field,CAL_VALUE_GMT_ALARMS_TIME))
		{
			cal_alarm_info_t *alarm_info = value->user_data;
			//cal_db_service_copy_struct_tm((struct tm*)&input_tm,&(exception_info->exception_start_time));
			memcpy(&(alarm_info->alarm_time),&input_tm,sizeof(struct tm));
			DBG("%d-%d-%d",alarm_info->alarm_time.tm_year,alarm_info->alarm_time.tm_mon,alarm_info->alarm_time.tm_mday);
			return CAL_SUCCESS;
		}
		else
		{

		}
		break;

	default:
		break;
	}

	return CAL_SUCCESS;
}
