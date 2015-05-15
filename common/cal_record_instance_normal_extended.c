/*
 * Calendar Service
 *
 * Copyright (c) 2012 - 2015 Samsung Electronics Co., Ltd. All rights reserved.
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

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_view.h"
#include "cal_record.h"

static int _cal_record_instance_normal_extended_create(calendar_record_h* out_record);
static int _cal_record_instance_normal_extended_destroy(calendar_record_h record, bool delete_child);
static int _cal_record_instance_normal_extended_clone(calendar_record_h record, calendar_record_h* out_record);
static int _cal_record_instance_normal_extended_get_str(calendar_record_h record, unsigned int property_id, char** out_str);
static int _cal_record_instance_normal_extended_get_str_p(calendar_record_h record, unsigned int property_id, char** out_str);
static int _cal_record_instance_normal_extended_get_int(calendar_record_h record, unsigned int property_id, int* out_value);
static int _cal_record_instance_normal_extended_get_double(calendar_record_h record, unsigned int property_id, double* out_value);
static int _cal_record_instance_normal_extended_get_lli(calendar_record_h record, unsigned int property_id, long long int* out_value);
static int _cal_record_instance_normal_extended_get_caltime(calendar_record_h record, unsigned int property_id, calendar_time_s* out_value);
static int _cal_record_instance_normal_extended_set_str(calendar_record_h record, unsigned int property_id, const char* value);
static int _cal_record_instance_normal_extended_set_int(calendar_record_h record, unsigned int property_id, int value);
static int _cal_record_instance_normal_extended_set_double(calendar_record_h record, unsigned int property_id, double value);
static int _cal_record_instance_normal_extended_set_lli(calendar_record_h record, unsigned int property_id, long long int value);
static int _cal_record_instance_normal_extended_set_caltime(calendar_record_h record, unsigned int property_id, calendar_time_s value);

cal_record_plugin_cb_s cal_record_instance_normal_extended_plugin_cb = {
	.create = _cal_record_instance_normal_extended_create,
	.destroy = _cal_record_instance_normal_extended_destroy,
	.clone = _cal_record_instance_normal_extended_clone,
	.get_str = _cal_record_instance_normal_extended_get_str,
	.get_str_p = _cal_record_instance_normal_extended_get_str_p,
	.get_int = _cal_record_instance_normal_extended_get_int,
	.get_double = _cal_record_instance_normal_extended_get_double,
	.get_lli = _cal_record_instance_normal_extended_get_lli,
	.get_caltime = _cal_record_instance_normal_extended_get_caltime,
	.set_str = _cal_record_instance_normal_extended_set_str,
	.set_int = _cal_record_instance_normal_extended_set_int,
	.set_double = _cal_record_instance_normal_extended_set_double,
	.set_lli = _cal_record_instance_normal_extended_set_lli,
	.set_caltime = _cal_record_instance_normal_extended_set_caltime,
	.add_child_record = NULL,
	.remove_child_record = NULL,
	.get_child_record_count = NULL,
	.get_child_record_at_p = NULL,
	.clone_child_record_list = NULL
};

static void _cal_record_instance_normal_extended_struct_init(cal_instance_normal_extended_s* record)
{
	memset(record,0, sizeof(cal_instance_normal_extended_s));

	record->event_status = CALENDAR_EVENT_STATUS_NONE;
	record->calendar_id = DEFAULT_EVENT_CALENDAR_BOOK_ID;
	record->event_id = CAL_INVALID_ID;

	record->busy_status = 2;
	record->summary = NULL;
	record->description = NULL;
	record->location= NULL;

	record->latitude = 1000; /* set default 1000 out of range(-180 ~ 180) */
	record->longitude = 1000; /* set default 1000 out of range(-180 ~ 180) */

	return ;
}

static int _cal_record_instance_normal_extended_create(calendar_record_h* out_record)
{
	cal_instance_normal_extended_s *temp = NULL;
	int ret= CALENDAR_ERROR_NONE;

	temp = calloc(1, sizeof(cal_instance_normal_extended_s));
	RETVM_IF(NULL == temp, CALENDAR_ERROR_OUT_OF_MEMORY, "calloc() Fail");

	_cal_record_instance_normal_extended_struct_init(temp);

	*out_record = (calendar_record_h)temp;

	return ret;
}

static void _cal_record_instance_normal_extended_struct_free(cal_instance_normal_extended_s *record)
{
	CAL_FREE(record->summary);
	CAL_FREE(record->description);
	CAL_FREE(record->location);
	CAL_FREE(record->organizer_name);
	CAL_FREE(record->categories);
	CAL_FREE(record->sync_data1);
	CAL_FREE(record->sync_data2);
	CAL_FREE(record->sync_data3);
	CAL_FREE(record->sync_data4);

	CAL_FREE(record);
}

static int _cal_record_instance_normal_extended_destroy(calendar_record_h record, bool delete_child)
{
	cal_instance_normal_extended_s *temp = (cal_instance_normal_extended_s*)(record);

	_cal_record_instance_normal_extended_struct_free(temp);

	return CALENDAR_ERROR_NONE;
}

static int _cal_record_instance_normal_extended_clone(calendar_record_h record, calendar_record_h* out_record)
{
	cal_instance_normal_extended_s *out_data = NULL;
	cal_instance_normal_extended_s *src_data = NULL;

	src_data = (cal_instance_normal_extended_s*)(record);

	out_data = calloc(1, sizeof(cal_instance_normal_extended_s));
	RETVM_IF(NULL == out_data, CALENDAR_ERROR_OUT_OF_MEMORY, "calloc() Fail");

	CAL_RECORD_COPY_COMMON(&(out_data->common), &(src_data->common));

	out_data->event_id = src_data->event_id;
	out_data->calendar_id = src_data->calendar_id;
	out_data->start = src_data->start;
	out_data->end = src_data->end;
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
	out_data->organizer_name = SAFE_STRDUP(src_data->organizer_name);
	out_data->categories = SAFE_STRDUP(src_data->categories);
	out_data->sync_data1 = SAFE_STRDUP(src_data->sync_data1);
	out_data->sync_data2 = SAFE_STRDUP(src_data->sync_data2);
	out_data->sync_data3 = SAFE_STRDUP(src_data->sync_data3);
	out_data->sync_data4 = SAFE_STRDUP(src_data->sync_data4);

	*out_record = (calendar_record_h)out_data;

	return CALENDAR_ERROR_NONE;
}

static int _cal_record_instance_normal_extended_get_str(calendar_record_h record, unsigned int property_id, char** out_str)
{
	cal_instance_normal_extended_s *rec = (cal_instance_normal_extended_s*)(record);
	switch (property_id) {
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_SUMMARY:
		*out_str = SAFE_STRDUP(rec->summary);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_DESCRIPTION:
		*out_str = SAFE_STRDUP(rec->description);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_LOCATION:
		*out_str = SAFE_STRDUP(rec->location);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_SYNC_DATA1:
		*out_str = SAFE_STRDUP(rec->sync_data1);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_ORGANIZER_NAME:
		*out_str = SAFE_STRDUP(rec->organizer_name);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_CATEGORIES:
		*out_str = SAFE_STRDUP(rec->categories);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_SYNC_DATA2:
		*out_str = SAFE_STRDUP(rec->sync_data2);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_SYNC_DATA3:
		*out_str = SAFE_STRDUP(rec->sync_data3);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_SYNC_DATA4:
		*out_str = SAFE_STRDUP(rec->sync_data4);
		break;
	default:
		ERR("invalid parameter (property:%d)",property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_record_instance_normal_extended_get_str_p(calendar_record_h record, unsigned int property_id, char** out_str)
{
	cal_instance_normal_extended_s *rec = (cal_instance_normal_extended_s*)(record);
	switch (property_id) {
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_SUMMARY:
		*out_str = (rec->summary);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_DESCRIPTION:
		*out_str = (rec->description);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_LOCATION:
		*out_str = (rec->location);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_SYNC_DATA1:
		*out_str = (rec->sync_data1);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_ORGANIZER_NAME:
		*out_str = (rec->organizer_name);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_CATEGORIES:
		*out_str = (rec->categories);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_SYNC_DATA2:
		*out_str = (rec->sync_data2);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_SYNC_DATA3:
		*out_str = (rec->sync_data3);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_SYNC_DATA4:
		*out_str = (rec->sync_data4);
		break;
	default:
		ERR("invalid parameter (property:%d)",property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_record_instance_normal_extended_get_int(calendar_record_h record, unsigned int property_id, int* out_value)
{
	cal_instance_normal_extended_s *rec = (cal_instance_normal_extended_s*)(record);
	switch (property_id) {
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_EVENT_ID:
		*out_value = (rec->event_id);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_CALENDAR_ID:
		*out_value = (rec->calendar_id);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_BUSY_STATUS:
		*out_value = (rec->busy_status);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_EVENT_STATUS:
		*out_value = (rec->event_status);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_PRIORITY:
		*out_value = (rec->priority);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_SENSITIVITY:
		*out_value = (rec->sensitivity);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_HAS_RRULE:
		*out_value = (rec->has_rrule);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_HAS_ALARM:
		*out_value = (rec->has_alarm);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_ORIGINAL_EVENT_ID:
		*out_value = (rec->original_event_id);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_HAS_ATTENDEE:
		*out_value = (rec->has_attendee);
		break;
	default:
		ERR("invalid parameter (property:%d)",property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_record_instance_normal_extended_get_double(calendar_record_h record, unsigned int property_id, double* out_value)
{
	cal_instance_normal_extended_s *rec = (cal_instance_normal_extended_s*)(record);
	switch (property_id) {
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_LATITUDE:
		*out_value = (rec->latitude);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_LONGITUDE:
		*out_value = (rec->longitude);
		break;

	default:
		ERR("invalid parameter (property:%d)",property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_record_instance_normal_extended_get_lli(calendar_record_h record, unsigned int property_id, long long int* out_value)
{
	cal_instance_normal_extended_s *rec = (cal_instance_normal_extended_s*)(record);
	switch (property_id) {
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_LAST_MODIFIED_TIME:
		*out_value = (rec->last_mod);
		break;
	default:
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_record_instance_normal_extended_get_caltime(calendar_record_h record, unsigned int property_id, calendar_time_s* out_value)
{
	cal_instance_normal_extended_s *rec = (cal_instance_normal_extended_s*)(record);
	switch (property_id) {
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_START:
		*out_value = rec->start;
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_END:
		*out_value = rec->end;
		break;
	default:
		ERR("invalid parameter (property:%d)",property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_record_instance_normal_extended_set_str(calendar_record_h record, unsigned int property_id, const char* value)
{
	cal_instance_normal_extended_s *rec = (cal_instance_normal_extended_s*)(record);
	switch (property_id) {
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_SUMMARY:
		CAL_FREE(rec->summary);
		rec->summary = SAFE_STRDUP(value);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_DESCRIPTION:
		CAL_FREE(rec->description);
		rec->description = SAFE_STRDUP(value);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_LOCATION:
		CAL_FREE(rec->location);
		rec->location = SAFE_STRDUP(value);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_SYNC_DATA1:
		CAL_FREE(rec->sync_data1);
		rec->sync_data1 = SAFE_STRDUP(value);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_ORGANIZER_NAME:
		CAL_FREE(rec->organizer_name);
		rec->organizer_name = SAFE_STRDUP(value);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_CATEGORIES:
		CAL_FREE(rec->categories);
		rec->categories = SAFE_STRDUP(value);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_SYNC_DATA2:
		CAL_FREE(rec->sync_data2);
		rec->sync_data2 = SAFE_STRDUP(value);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_SYNC_DATA3:
		CAL_FREE(rec->sync_data3);
		rec->sync_data3 = SAFE_STRDUP(value);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_SYNC_DATA4:
		CAL_FREE(rec->sync_data4);
		rec->sync_data4 = SAFE_STRDUP(value);
		break;
	default:
		ERR("invalid parameter (property:%d)",property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_record_instance_normal_extended_set_int(calendar_record_h record, unsigned int property_id, int value)
{
	cal_instance_normal_extended_s *rec = (cal_instance_normal_extended_s*)(record);
	switch (property_id) {
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_EVENT_ID:
		(rec->event_id) = value;
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_CALENDAR_ID:
		(rec->calendar_id) = value;
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_BUSY_STATUS:
		(rec->busy_status) = value;
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_EVENT_STATUS:
		(rec->event_status) = value;
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_PRIORITY:
		(rec->priority) = value;
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_SENSITIVITY:
		(rec->sensitivity) = value;
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_HAS_RRULE:
		(rec->has_rrule) = value;
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_HAS_ALARM:
		(rec->has_alarm) = value;
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_ORIGINAL_EVENT_ID:
		(rec->original_event_id) = value;
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_HAS_ATTENDEE:
		(rec->has_attendee) = value;
		break;
	default:
		ERR("invalid parameter (property:%d)",property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_record_instance_normal_extended_set_double(calendar_record_h record, unsigned int property_id, double value)
{
	cal_instance_normal_extended_s *rec = (cal_instance_normal_extended_s*)(record);
	switch (property_id) {
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_LATITUDE:
		(rec->latitude) = value;
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_LONGITUDE:
		(rec->longitude) = value;
		break;

	default:
		ERR("invalid parameter (property:%d)",property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_record_instance_normal_extended_set_lli(calendar_record_h record, unsigned int property_id, long long int value)
{
	cal_instance_normal_extended_s *rec = (cal_instance_normal_extended_s*)(record);
	switch (property_id) {
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_LAST_MODIFIED_TIME:
		(rec->last_mod) = value;
		break;
	default:
		ERR("invalid parameter (property:%d)",property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_record_instance_normal_extended_set_caltime(calendar_record_h record, unsigned int property_id, calendar_time_s value)
{
	cal_instance_normal_extended_s *rec = (cal_instance_normal_extended_s*)(record);
	switch (property_id) {
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_START:
		rec->start = value;
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_END:
		rec->end = value;
		break;
	default:
		ERR("invalid parameter (property:%d)",property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}
