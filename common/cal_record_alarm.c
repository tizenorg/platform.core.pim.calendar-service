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

#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_view.h"

#include "cal_record.h"

static int __cal_record_alarm_create( calendar_record_h* out_record );
static int __cal_record_alarm_destroy( calendar_record_h record, bool delete_child );
static int __cal_record_alarm_clone( calendar_record_h record, calendar_record_h* out_record );
static int __cal_record_alarm_get_str( calendar_record_h record, unsigned int property_id, char** out_str );
static int __cal_record_alarm_get_str_p( calendar_record_h record, unsigned int property_id, char** out_str );
static int __cal_record_alarm_get_int( calendar_record_h record, unsigned int property_id, int* out_value );
static int __cal_record_alarm_get_lli( calendar_record_h record, unsigned int property_id, long long int* out_value );
static int __cal_record_alarm_set_str( calendar_record_h record, unsigned int property_id, const char* value );
static int __cal_record_alarm_set_int( calendar_record_h record, unsigned int property_id, int value );
static int __cal_record_alarm_set_lli( calendar_record_h record, unsigned int property_id, long long int value );

cal_record_plugin_cb_s _cal_record_alarm_plugin_cb = {
	.create = __cal_record_alarm_create,
	.destroy = __cal_record_alarm_destroy,
	.clone = __cal_record_alarm_clone,
	.get_str = __cal_record_alarm_get_str,
	.get_str_p = __cal_record_alarm_get_str_p,
	.get_int = __cal_record_alarm_get_int,
	.get_double = NULL,
	.get_lli = __cal_record_alarm_get_lli,
	.get_caltime = NULL,
	.set_str = __cal_record_alarm_set_str,
	.set_int = __cal_record_alarm_set_int,
	.set_double = NULL,
	.set_lli = __cal_record_alarm_set_lli,
	.set_caltime = NULL,
	.add_child_record = NULL,
	.remove_child_record = NULL,
	.get_child_record_count = NULL,
	.get_child_record_at_p = NULL,
	.clone_child_record_list = NULL
};

static void __cal_record_alarm_struct_init(cal_alarm_s *record)
{
	memset(record,0,sizeof(cal_alarm_s));
}

static int __cal_record_alarm_create(calendar_record_h* out_record )
{
	cal_alarm_s *temp = NULL;
	int ret= CALENDAR_ERROR_NONE, type = 0;

	type = CAL_RECORD_TYPE_ALARM;

	temp = (cal_alarm_s*)calloc(1,sizeof(cal_alarm_s));
	retvm_if(NULL == temp, CALENDAR_ERROR_OUT_OF_MEMORY, "malloc(cal_alarm_s:sch) Failed(%d)", CALENDAR_ERROR_OUT_OF_MEMORY);

	__cal_record_alarm_struct_init(temp);

	*out_record = (calendar_record_h)temp;

	return ret;
}

static void __cal_record_alarm_struct_free(cal_alarm_s *record)
{
	CAL_FREE(record->alarm_tone);
	CAL_FREE(record->alarm_description);
	CAL_FREE(record);
}

static int __cal_record_alarm_destroy( calendar_record_h record, bool delete_child )
{
    int ret = CALENDAR_ERROR_NONE;

    cal_alarm_s *temp = (cal_alarm_s*)(record);

    __cal_record_alarm_struct_free(temp);

	return ret;
}

static int __cal_record_alarm_clone( calendar_record_h record, calendar_record_h* out_record )
{
	cal_alarm_s *out_data = NULL;
	cal_alarm_s *src_data = NULL;

	src_data = (cal_alarm_s*)(record);

	out_data = calloc(1, sizeof(cal_alarm_s));
	retvm_if(NULL == out_data, CALENDAR_ERROR_OUT_OF_MEMORY, "calloc(cal_alarm_s) Failed(%d)", CALENDAR_ERROR_OUT_OF_MEMORY);

	CAL_RECORD_COPY_COMMON(&(out_data->common), &(src_data->common));

	out_data->alarm_id = src_data->alarm_id;
	out_data->event_id = src_data->event_id;
	out_data->alarm_type = src_data->alarm_type;
	out_data->is_deleted = src_data->is_deleted;
	out_data->alarm_time = src_data->alarm_time;
	out_data->remind_tick = src_data->remind_tick;
	out_data->remind_tick_unit = src_data->remind_tick_unit;
	out_data->alarm_tone = SAFE_STRDUP(src_data->alarm_tone);
	out_data->alarm_description = SAFE_STRDUP(src_data->alarm_description);

    *out_record = (calendar_record_h)out_data;

	return CALENDAR_ERROR_NONE;
}

static int __cal_record_alarm_get_str( calendar_record_h record, unsigned int property_id, char** out_str )
{
	cal_alarm_s *rec = (cal_alarm_s*)(record);
	switch( property_id )
	{
	case CAL_PROPERTY_ALARM_TONE:
		*out_str = SAFE_STRDUP(rec->alarm_tone);
		break;
	case CAL_PROPERTY_ALARM_DESCRIPTION:
		*out_str = SAFE_STRDUP(rec->alarm_description);
		break;
	default:
	    ASSERT_NOT_REACHED("invalid parameter (property:%d)",property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int __cal_record_alarm_get_str_p( calendar_record_h record, unsigned int property_id, char** out_str )
{
	cal_alarm_s *rec = (cal_alarm_s*)(record);
	switch( property_id )
	{
	case CAL_PROPERTY_ALARM_TONE:
		*out_str = (rec->alarm_tone);
		break;
	case CAL_PROPERTY_ALARM_DESCRIPTION:
		*out_str = (rec->alarm_description);
		break;
	default:
	    ASSERT_NOT_REACHED("invalid parameter (property:%d)",property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int __cal_record_alarm_get_int( calendar_record_h record, unsigned int property_id, int* out_value )
{
	cal_alarm_s *rec = (cal_alarm_s*)(record);
	switch( property_id )
	{
	case CAL_PROPERTY_ALARM_TYPE:
		*out_value = (rec->alarm_type);
		break;
	case CAL_PROPERTY_ALARM_TICK:
		*out_value = (rec->remind_tick);
		break;
	case CAL_PROPERTY_ALARM_TICK_UNIT:
		*out_value = (rec->remind_tick_unit);
		break;
	case CAL_PROPERTY_ALARM_ID:
		*out_value = (rec->alarm_id);
		break;
	case CAL_PROPERTY_ALARM_EVENT_TODO_ID:
	    *out_value = (rec->event_id);
	    break;
	default:
	    ASSERT_NOT_REACHED("invalid parameter (property:%d)",property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int __cal_record_alarm_get_lli( calendar_record_h record, unsigned int property_id, long long int* out_value )
{
	cal_alarm_s *rec = (cal_alarm_s*)(record);
	switch( property_id )
	{
	case CAL_PROPERTY_ALARM_TIME:
		*out_value = (rec->alarm_time);
		break;
	default:
	    ASSERT_NOT_REACHED("invalid parameter (property:%d)",property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int __cal_record_alarm_set_str( calendar_record_h record, unsigned int property_id, const char* value )
{
	cal_alarm_s *rec = (cal_alarm_s*)(record);
	switch( property_id )
	{
	case CAL_PROPERTY_ALARM_TONE:
		CAL_FREE(rec->alarm_tone);
		rec->alarm_tone = SAFE_STRDUP(value);
		break;
	case CAL_PROPERTY_ALARM_DESCRIPTION:
		CAL_FREE(rec->alarm_description);
		rec->alarm_description = SAFE_STRDUP(value);
		break;
	default:
	    ASSERT_NOT_REACHED("invalid parameter (property:%d)",property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int __cal_record_alarm_set_int( calendar_record_h record, unsigned int property_id, int value )
{
	cal_alarm_s *rec = (cal_alarm_s*)(record);
	switch( property_id )
	{
	case CAL_PROPERTY_ALARM_TYPE:
		(rec->alarm_type)=value;
		break;
	case CAL_PROPERTY_ALARM_TICK:
		(rec->remind_tick)=value;
		break;
	case CAL_PROPERTY_ALARM_TICK_UNIT:
		(rec->remind_tick_unit)=value;
		break;
	case CAL_PROPERTY_ALARM_ID:
		(rec->alarm_id) = value;
		break;
    case CAL_PROPERTY_ALARM_EVENT_TODO_ID:
        (rec->event_id) = value;
        break;
	default:
	    ASSERT_NOT_REACHED("invalid parameter (property:%d)",property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int __cal_record_alarm_set_lli( calendar_record_h record, unsigned int property_id, long long int value )
{
	cal_alarm_s *rec = (cal_alarm_s*)(record);
	switch( property_id )
	{
	case CAL_PROPERTY_ALARM_TIME:
		(rec->alarm_time) = value;
		break;
	default:
	    ASSERT_NOT_REACHED("invalid parameter (property:%d)",property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}
