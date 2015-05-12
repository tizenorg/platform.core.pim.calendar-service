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

#include <stdlib.h> // calloc
#include <stdbool.h> // bool
#include <string.h>

#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_view.h"
#include "cal_record.h"

static int _cal_record_timezone_create(calendar_record_h* out_record);
static int _cal_record_timezone_destroy(calendar_record_h record, bool delete_child);
static int _cal_record_timezone_clone(calendar_record_h record, calendar_record_h* out_record);
static int _cal_record_timezone_get_str(calendar_record_h record, unsigned int property_id, char** out_str);
static int _cal_record_timezone_get_str_p(calendar_record_h record, unsigned int property_id, char** out_str);
static int _cal_record_timezone_get_int(calendar_record_h record, unsigned int property_id, int* out_value);
static int _cal_record_timezone_set_str(calendar_record_h record, unsigned int property_id, const char* value);
static int _cal_record_timezone_set_int(calendar_record_h record, unsigned int property_id, int value);

cal_record_plugin_cb_s cal_record_timezone_plugin_cb = {
	.create = _cal_record_timezone_create,
	.destroy = _cal_record_timezone_destroy,
	.clone = _cal_record_timezone_clone,
	.get_str = _cal_record_timezone_get_str,
	.get_str_p = _cal_record_timezone_get_str_p,
	.get_int = _cal_record_timezone_get_int,
	.get_double = NULL,
	.get_lli = NULL,
	.get_caltime = NULL,
	.set_str = _cal_record_timezone_set_str,
	.set_int = _cal_record_timezone_set_int,
	.set_double = NULL,
	.set_lli = NULL,
	.set_caltime = NULL,
	.add_child_record = NULL,
	.remove_child_record = NULL,
	.get_child_record_count = NULL,
	.get_child_record_at_p = NULL,
	.clone_child_record_list = NULL
};

static void _cal_record_timezone_struct_init(cal_timezone_s *record)
{
	memset(record,0,sizeof(cal_timezone_s));
	record->calendar_id = DEFAULT_EVENT_CALENDAR_BOOK_ID;
}

static int _cal_record_timezone_create(calendar_record_h* out_record)
{
	cal_timezone_s *temp = NULL;
	int ret= CALENDAR_ERROR_NONE;

	temp = calloc(1,sizeof(cal_timezone_s));
	RETVM_IF(NULL == temp, CALENDAR_ERROR_OUT_OF_MEMORY, "calloc(cal_timezone_s) Fail(%d)", CALENDAR_ERROR_OUT_OF_MEMORY);

	_cal_record_timezone_struct_init(temp);

	*out_record = (calendar_record_h)temp;

	return ret;
}

static void _cal_record_timezone_struct_free(cal_timezone_s *record)
{
	CAL_FREE(record->standard_name);
	CAL_FREE(record->day_light_name);
	CAL_FREE(record);

}

static int _cal_record_timezone_destroy(calendar_record_h record, bool delete_child)
{
	int ret = CALENDAR_ERROR_NONE;

	cal_timezone_s *temp = (cal_timezone_s*)(record);

	_cal_record_timezone_struct_free(temp);

	return ret;
}

static int _cal_record_timezone_clone(calendar_record_h record, calendar_record_h* out_record)
{
	cal_timezone_s *out_data = NULL;
	cal_timezone_s *src_data = NULL;

	src_data = (cal_timezone_s*)(record);

	out_data = calloc(1, sizeof(cal_timezone_s));
	RETVM_IF(NULL == out_data, CALENDAR_ERROR_OUT_OF_MEMORY, "calloc(cal_timezone_s) Fail(%d)", CALENDAR_ERROR_OUT_OF_MEMORY);

	CAL_RECORD_COPY_COMMON(&(out_data->common), &(src_data->common));

	out_data->index = src_data->index;
	out_data->tz_offset_from_gmt = src_data->tz_offset_from_gmt;
	out_data->standard_name = SAFE_STRDUP(src_data->standard_name);
	out_data->std_start_month = src_data->std_start_month;
	out_data->std_start_position_of_week = src_data->std_start_position_of_week;
	out_data->std_start_day = src_data->std_start_day;
	out_data->std_start_hour = src_data->std_start_hour;
	out_data->standard_bias = src_data->standard_bias;
	out_data->day_light_name = SAFE_STRDUP(src_data->day_light_name);
	out_data->day_light_start_month = src_data->day_light_start_month;
	out_data->day_light_start_position_of_week = src_data->day_light_start_position_of_week;
	out_data->day_light_start_day = src_data->day_light_start_day;
	out_data->day_light_start_hour = src_data->day_light_start_hour;
	out_data->day_light_bias = src_data->day_light_bias;
	out_data->calendar_id = src_data->calendar_id;

	*out_record = (calendar_record_h)out_data;

	return CALENDAR_ERROR_NONE;
}

static int _cal_record_timezone_get_str(calendar_record_h record, unsigned int property_id, char** out_str)
{
	cal_timezone_s *rec = (cal_timezone_s*)(record);
	switch (property_id)
	{
	case CAL_PROPERTY_TIMEZONE_STANDARD_NAME:
		*out_str = SAFE_STRDUP(rec->standard_name);
		break;
	case CAL_PROPERTY_TIMEZONE_DAY_LIGHT_NAME:
		*out_str = SAFE_STRDUP(rec->day_light_name);
		break;
	default:
		ERR("invalid parameter (property:%d)",property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_record_timezone_get_str_p(calendar_record_h record, unsigned int property_id, char** out_str)
{
	cal_timezone_s *rec = (cal_timezone_s*)(record);
	switch (property_id)
	{
	case CAL_PROPERTY_TIMEZONE_STANDARD_NAME:
		*out_str = (rec->standard_name);
		break;
	case CAL_PROPERTY_TIMEZONE_DAY_LIGHT_NAME:
		*out_str = (rec->day_light_name);
		break;
	default:
		ERR("invalid parameter (property:%d)",property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_record_timezone_get_int(calendar_record_h record, unsigned int property_id, int* out_value)
{
	cal_timezone_s *rec = (cal_timezone_s*)(record);
	switch (property_id)
	{
	case CAL_PROPERTY_TIMEZONE_ID:
		*out_value = (rec->index);
		break;
	case CAL_PROPERTY_TIMEZONE_TZ_OFFSET_FROM_GMT:
		*out_value = (rec->tz_offset_from_gmt);
		break;
	case CAL_PROPERTY_TIMEZONE_STD_START_MONTH:
		*out_value = (rec->std_start_month);
		break;
	case CAL_PROPERTY_TIMEZONE_STD_START_POSITION_OF_WEEK:
		*out_value = (rec->std_start_position_of_week);
		break;
	case CAL_PROPERTY_TIMEZONE_STD_START_DAY:
		*out_value = (rec->std_start_day);
		break;
	case CAL_PROPERTY_TIMEZONE_STD_START_HOUR:
		*out_value = (rec->std_start_hour);
		break;
	case CAL_PROPERTY_TIMEZONE_STANDARD_BIAS:
		*out_value = (rec->standard_bias);
		break;
	case CAL_PROPERTY_TIMEZONE_DAY_LIGHT_START_MONTH:
		*out_value = (rec->day_light_start_month);
		break;
	case CAL_PROPERTY_TIMEZONE_DAY_LIGHT_START_POSITION_OF_WEEK:
		*out_value = (rec->day_light_start_position_of_week);
		break;
	case CAL_PROPERTY_TIMEZONE_DAY_LIGHT_START_DAY:
		*out_value = (rec->day_light_start_day);
		break;
	case CAL_PROPERTY_TIMEZONE_DAY_LIGHT_START_HOUR:
		*out_value = (rec->day_light_start_hour);
		break;
	case CAL_PROPERTY_TIMEZONE_DAY_LIGHT_BIAS:
		*out_value = (rec->day_light_bias);
		break;
	case CAL_PROPERTY_TIMEZONE_CALENDAR_ID:
		*out_value = (rec->calendar_id);
		break;
	default:
		ERR("invalid parameter (property:%d)",property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_record_timezone_set_str(calendar_record_h record, unsigned int property_id, const char* value)
{
	cal_timezone_s *rec = (cal_timezone_s*)(record);
	switch (property_id)
	{
	case CAL_PROPERTY_TIMEZONE_STANDARD_NAME:
		CAL_FREE(rec->standard_name);
		rec->standard_name = SAFE_STRDUP(value);
		break;
	case CAL_PROPERTY_TIMEZONE_DAY_LIGHT_NAME:
		CAL_FREE(rec->day_light_name);
		rec->day_light_name = SAFE_STRDUP(value);
		break;
	default:
		ERR("invalid parameter (property:%d)",property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_record_timezone_set_int(calendar_record_h record, unsigned int property_id, int value)
{
	cal_timezone_s *rec = (cal_timezone_s*)(record);
	switch (property_id)
	{
	case CAL_PROPERTY_TIMEZONE_ID:
		(rec->index) = value;
		break;
	case CAL_PROPERTY_TIMEZONE_TZ_OFFSET_FROM_GMT:
		(rec->tz_offset_from_gmt) = value;
		break;
	case CAL_PROPERTY_TIMEZONE_STD_START_MONTH:
		(rec->std_start_month) = value;
		break;
	case CAL_PROPERTY_TIMEZONE_STD_START_POSITION_OF_WEEK:
		(rec->std_start_position_of_week) = value;
		break;
	case CAL_PROPERTY_TIMEZONE_STD_START_DAY:
		(rec->std_start_day) = value;
		break;
	case CAL_PROPERTY_TIMEZONE_STD_START_HOUR:
		(rec->std_start_hour) = value;
		break;
	case CAL_PROPERTY_TIMEZONE_STANDARD_BIAS:
		(rec->standard_bias) = value;
		break;
	case CAL_PROPERTY_TIMEZONE_DAY_LIGHT_START_MONTH:
		(rec->day_light_start_month) = value;
		break;
	case CAL_PROPERTY_TIMEZONE_DAY_LIGHT_START_POSITION_OF_WEEK:
		(rec->day_light_start_position_of_week) = value;
		break;
	case CAL_PROPERTY_TIMEZONE_DAY_LIGHT_START_DAY:
		(rec->day_light_start_day) = value;
		break;
	case CAL_PROPERTY_TIMEZONE_DAY_LIGHT_START_HOUR:
		(rec->day_light_start_hour) = value;
		break;
	case CAL_PROPERTY_TIMEZONE_DAY_LIGHT_BIAS:
		(rec->day_light_bias) = value;
		break;
	case CAL_PROPERTY_TIMEZONE_CALENDAR_ID:
		(rec->calendar_id) = value;
		break;
	default:
		ERR("invalid parameter (property:%d)",property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}
