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
#include "cal_utils.h"

static int _cal_record_instance_normal_create(calendar_record_h* out_record);
static int _cal_record_instance_normal_destroy(calendar_record_h record, bool delete_child);
static int _cal_record_instance_normal_clone(calendar_record_h record, calendar_record_h* out_record);
static int _cal_record_instance_normal_get_str(calendar_record_h record, unsigned int property_id, char** out_str);
static int _cal_record_instance_normal_get_str_p(calendar_record_h record, unsigned int property_id, char** out_str);
static int _cal_record_instance_normal_get_int(calendar_record_h record, unsigned int property_id, int* out_value);
static int _cal_record_instance_normal_get_double(calendar_record_h record, unsigned int property_id, double* out_value);
static int _cal_record_instance_normal_get_lli(calendar_record_h record, unsigned int property_id, long long int* out_value);
static int _cal_record_instance_normal_get_caltime(calendar_record_h record, unsigned int property_id, calendar_time_s* out_value);
static int _cal_record_instance_normal_set_str(calendar_record_h record, unsigned int property_id, const char* value);
static int _cal_record_instance_normal_set_int(calendar_record_h record, unsigned int property_id, int value);
static int _cal_record_instance_normal_set_double(calendar_record_h record, unsigned int property_id, double value);
static int _cal_record_instance_normal_set_caltime(calendar_record_h record, unsigned int property_id, calendar_time_s value);

cal_record_plugin_cb_s cal_record_instance_normal_plugin_cb = {
	.create = _cal_record_instance_normal_create,
	.destroy = _cal_record_instance_normal_destroy,
	.clone = _cal_record_instance_normal_clone,
	.get_str = _cal_record_instance_normal_get_str,
	.get_str_p = _cal_record_instance_normal_get_str_p,
	.get_int = _cal_record_instance_normal_get_int,
	.get_double = _cal_record_instance_normal_get_double,
	.get_lli = _cal_record_instance_normal_get_lli,
	.get_caltime = _cal_record_instance_normal_get_caltime,
	.set_str = _cal_record_instance_normal_set_str,
	.set_int = _cal_record_instance_normal_set_int,
	.set_double = _cal_record_instance_normal_set_double,
	.set_lli = NULL,
	.set_caltime = _cal_record_instance_normal_set_caltime,
	.add_child_record = NULL,
	.remove_child_record = NULL,
	.get_child_record_count = NULL,
	.get_child_record_at_p = NULL,
	.clone_child_record_list = NULL
};

static void _cal_record_instance_normal_struct_init(cal_instance_normal_s* record)
{
	memset(record,0, sizeof(cal_instance_normal_s));

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

static int _cal_record_instance_normal_create(calendar_record_h* out_record)
{
	cal_instance_normal_s *temp = NULL;
	int ret= CALENDAR_ERROR_NONE;

	temp = calloc(1, sizeof(cal_instance_normal_s));
	RETVM_IF(NULL == temp, CALENDAR_ERROR_OUT_OF_MEMORY, "calloc() Fail");

	_cal_record_instance_normal_struct_init(temp);

	*out_record = (calendar_record_h)temp;

	return ret;
}

static void _cal_record_instance_normal_struct_free(cal_instance_normal_s *record)
{
	CAL_FREE(record->summary);
	CAL_FREE(record->description);
	CAL_FREE(record->location);
	CAL_FREE(record->sync_data1);

	CAL_FREE(record);
}

static int _cal_record_instance_normal_destroy(calendar_record_h record, bool delete_child)
{
	cal_instance_normal_s *temp = (cal_instance_normal_s*)(record);

	_cal_record_instance_normal_struct_free(temp);

	return CALENDAR_ERROR_NONE;
}

static int _cal_record_instance_normal_clone(calendar_record_h record, calendar_record_h* out_record)
{
	cal_instance_normal_s *out_data = NULL;
	cal_instance_normal_s *src_data = NULL;

	src_data = (cal_instance_normal_s*)(record);

	out_data = calloc(1, sizeof(cal_instance_normal_s));
	RETVM_IF(NULL == out_data, CALENDAR_ERROR_OUT_OF_MEMORY, "calloc() Fail");

	CAL_RECORD_COPY_COMMON(&(out_data->common), &(src_data->common));

	out_data->event_id = src_data->event_id;
	out_data->calendar_id = src_data->calendar_id;
	out_data->start = src_data->start;
	out_data->end = src_data->end;
	out_data->summary = cal_strdup(src_data->summary);
	out_data->description = cal_strdup(src_data->description);
	out_data->location = cal_strdup(src_data->location);
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
	out_data->sync_data1 = cal_strdup(src_data->sync_data1);

	*out_record = (calendar_record_h)out_data;

	return CALENDAR_ERROR_NONE;
}

static int _cal_record_instance_normal_get_str(calendar_record_h record, unsigned int property_id, char** out_str)
{
	cal_instance_normal_s *rec = (cal_instance_normal_s*)(record);
	switch (property_id) {
	case CAL_PROPERTY_INSTANCE_NORMAL_SUMMARY:
		*out_str = cal_strdup(rec->summary);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_DESCRIPTION:
		*out_str = cal_strdup(rec->description);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_LOCATION:
		*out_str = cal_strdup(rec->location);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_SYNC_DATA1:
		*out_str = cal_strdup(rec->sync_data1);
		break;
	default:
		ERR("invalid parameter (property:%d)",property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_record_instance_normal_get_str_p(calendar_record_h record, unsigned int property_id, char** out_str)
{
	cal_instance_normal_s *rec = (cal_instance_normal_s*)(record);
	switch (property_id) {
	case CAL_PROPERTY_INSTANCE_NORMAL_SUMMARY:
		*out_str = (rec->summary);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_DESCRIPTION:
		*out_str = (rec->description);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_LOCATION:
		*out_str = (rec->location);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_SYNC_DATA1:
		*out_str = (rec->sync_data1);
		break;
	default:
		ERR("invalid parameter (property:%d)",property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_record_instance_normal_get_int(calendar_record_h record, unsigned int property_id, int* out_value)
{
	cal_instance_normal_s *rec = (cal_instance_normal_s*)(record);
	switch (property_id) {
	case CAL_PROPERTY_INSTANCE_NORMAL_EVENT_ID:
		*out_value = (rec->event_id);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_CALENDAR_ID:
		*out_value = (rec->calendar_id);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_BUSY_STATUS:
		*out_value = (rec->busy_status);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_EVENT_STATUS:
		*out_value = (rec->event_status);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_PRIORITY:
		*out_value = (rec->priority);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_SENSITIVITY:
		*out_value = (rec->sensitivity);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_HAS_RRULE:
		*out_value = (rec->has_rrule);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_HAS_ALARM:
		*out_value = (rec->has_alarm);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_ORIGINAL_EVENT_ID:
		*out_value = (rec->original_event_id);
		break;
	default:
		ERR("invalid parameter (property:%d)",property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_record_instance_normal_get_double(calendar_record_h record, unsigned int property_id, double* out_value)
{
	cal_instance_normal_s *rec = (cal_instance_normal_s*)(record);
	switch (property_id) {
	case CAL_PROPERTY_INSTANCE_NORMAL_LATITUDE:
		*out_value = (rec->latitude);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_LONGITUDE:
		*out_value = (rec->longitude);
		break;

	default:
		ERR("invalid parameter (property:%d)",property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_record_instance_normal_get_lli(calendar_record_h record, unsigned int property_id, long long int* out_value)
{
	cal_instance_normal_s *rec = (cal_instance_normal_s*)(record);
	switch (property_id) {
	case CAL_PROPERTY_INSTANCE_NORMAL_LAST_MODIFIED_TIME:
		*out_value = (rec->last_mod);
		break;
	default:
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_record_instance_normal_get_caltime(calendar_record_h record, unsigned int property_id, calendar_time_s* out_value)
{
	cal_instance_normal_s *rec = (cal_instance_normal_s*)(record);
	switch (property_id) {
	case CAL_PROPERTY_INSTANCE_NORMAL_START:
		*out_value = rec->start;
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_END:
		*out_value = rec->end;
		break;
	default:
		ERR("invalid parameter (property:%d)",property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_record_instance_normal_set_str(calendar_record_h record, unsigned int property_id, const char* value)
{
	cal_instance_normal_s *rec = (cal_instance_normal_s*)(record);
	switch (property_id) {
	case CAL_PROPERTY_INSTANCE_NORMAL_SUMMARY:
		CAL_FREE(rec->summary);
		rec->summary = cal_strdup(value);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_DESCRIPTION:
		CAL_FREE(rec->description);
		rec->description = cal_strdup(value);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_LOCATION:
		CAL_FREE(rec->location);
		rec->location = cal_strdup(value);
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_SYNC_DATA1:
		CAL_FREE(rec->sync_data1);
		rec->sync_data1 = cal_strdup(value);
		break;
	default:
		ERR("invalid parameter (property:%d)",property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_record_instance_normal_set_int(calendar_record_h record, unsigned int property_id, int value)
{
	cal_instance_normal_s *rec = (cal_instance_normal_s*)(record);
	switch (property_id) {
	case CAL_PROPERTY_INSTANCE_NORMAL_EVENT_ID:
		(rec->event_id) = value;
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_CALENDAR_ID:
		(rec->calendar_id) = value;
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_BUSY_STATUS:
		(rec->busy_status) = value;
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_EVENT_STATUS:
		(rec->event_status) = value;
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_PRIORITY:
		(rec->priority) = value;
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_SENSITIVITY:
		(rec->sensitivity) = value;
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_HAS_RRULE:
		(rec->has_rrule) = value;
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_HAS_ALARM:
		(rec->has_alarm) = value;
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_ORIGINAL_EVENT_ID:
		(rec->original_event_id) = value;
		break;
	default:
		ERR("invalid parameter (property:%d)",property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_record_instance_normal_set_double(calendar_record_h record, unsigned int property_id, double value)
{
	cal_instance_normal_s *rec = (cal_instance_normal_s*)(record);
	switch (property_id) {
	case CAL_PROPERTY_INSTANCE_NORMAL_LATITUDE:
		(rec->latitude) = value;
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_LONGITUDE:
		(rec->longitude) = value;
		break;

	default:
		ERR("invalid parameter (property:%d)",property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_record_instance_normal_set_caltime(calendar_record_h record, unsigned int property_id, calendar_time_s value)
{
	cal_instance_normal_s *rec = (cal_instance_normal_s*)(record);
	switch (property_id) {
	case CAL_PROPERTY_INSTANCE_NORMAL_START:
		rec->start = value;
		break;
	case CAL_PROPERTY_INSTANCE_NORMAL_END:
		rec->end = value;
		break;
	default:
		ERR("invalid parameter (property:%d)",property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}
