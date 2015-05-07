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

#include <stdlib.h>	// calloc
#include <stdbool.h> // bool
#include <string.h>

#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_view.h"
#include "cal_record.h"

static int _cal_record_alarm_create( calendar_record_h* out_record );
static int _cal_record_alarm_destroy( calendar_record_h record, bool delete_child );
static int _cal_record_alarm_clone( calendar_record_h record, calendar_record_h* out_record );
static int _cal_record_alarm_get_str( calendar_record_h record, unsigned int property_id, char** out_str );
static int _cal_record_alarm_get_str_p( calendar_record_h record, unsigned int property_id, char** out_str );
static int _cal_record_alarm_get_int( calendar_record_h record, unsigned int property_id, int* out_value );
static int _cal_record_alarm_get_caltime( calendar_record_h record, unsigned int property_id, calendar_time_s* out_value );
static int _cal_record_alarm_set_str( calendar_record_h record, unsigned int property_id, const char* value );
static int _cal_record_alarm_set_int( calendar_record_h record, unsigned int property_id, int value );
static int _cal_record_alarm_set_caltime( calendar_record_h record, unsigned int property_id, calendar_time_s value );

cal_record_plugin_cb_s cal_record_alarm_plugin_cb = {
	.create = _cal_record_alarm_create,
	.destroy = _cal_record_alarm_destroy,
	.clone = _cal_record_alarm_clone,
	.get_str = _cal_record_alarm_get_str,
	.get_str_p = _cal_record_alarm_get_str_p,
	.get_int = _cal_record_alarm_get_int,
	.get_double = NULL,
	.get_lli = NULL,
	.get_caltime = _cal_record_alarm_get_caltime,
	.set_str = _cal_record_alarm_set_str,
	.set_int = _cal_record_alarm_set_int,
	.set_double = NULL,
	.set_lli = NULL,
	.set_caltime = _cal_record_alarm_set_caltime,
	.add_child_record = NULL,
	.remove_child_record = NULL,
	.get_child_record_count = NULL,
	.get_child_record_at_p = NULL,
	.clone_child_record_list = NULL
};

static void _cal_record_alarm_struct_init(cal_alarm_s *record)
{
	memset(record,0,sizeof(cal_alarm_s));
}

static int _cal_record_alarm_create(calendar_record_h* out_record )
{
	cal_alarm_s *temp = NULL;
	int ret= CALENDAR_ERROR_NONE;

	temp = calloc(1,sizeof(cal_alarm_s));
	RETVM_IF(NULL == temp, CALENDAR_ERROR_OUT_OF_MEMORY, "malloc(cal_alarm_s:sch) Failed(%d)", CALENDAR_ERROR_OUT_OF_MEMORY);

	_cal_record_alarm_struct_init(temp);

	*out_record = (calendar_record_h)temp;

	return ret;
}

static void _cal_record_alarm_struct_free(cal_alarm_s *record)
{
	CAL_FREE(record->alarm_description);
	CAL_FREE(record->alarm_summary);
	CAL_FREE(record->alarm_attach);
	CAL_FREE(record);
}

static int _cal_record_alarm_destroy( calendar_record_h record, bool delete_child )
{
	int ret = CALENDAR_ERROR_NONE;

	cal_alarm_s *temp = (cal_alarm_s*)(record);

	_cal_record_alarm_struct_free(temp);

	return ret;
}

static int _cal_record_alarm_clone( calendar_record_h record, calendar_record_h* out_record )
{
	cal_alarm_s *out_data = NULL;
	cal_alarm_s *src_data = NULL;

	src_data = (cal_alarm_s*)(record);

	out_data = calloc(1, sizeof(cal_alarm_s));
	RETVM_IF(NULL == out_data, CALENDAR_ERROR_OUT_OF_MEMORY, "calloc(cal_alarm_s) Failed(%d)", CALENDAR_ERROR_OUT_OF_MEMORY);

	CAL_RECORD_COPY_COMMON(&(out_data->common), &(src_data->common));

	out_data->parent_id = src_data->parent_id;
	out_data->is_deleted = src_data->is_deleted;
	out_data->remind_tick = src_data->remind_tick;
	out_data->remind_tick_unit = src_data->remind_tick_unit;
	out_data->alarm_description = SAFE_STRDUP(src_data->alarm_description);
	out_data->alarm_summary = SAFE_STRDUP(src_data->alarm_summary);
	out_data->alarm_action = src_data->alarm_action;
	out_data->alarm_attach = SAFE_STRDUP(src_data->alarm_attach);
	out_data->alarm = src_data->alarm;

	*out_record = (calendar_record_h)out_data;

	return CALENDAR_ERROR_NONE;
}

static int _cal_record_alarm_get_str( calendar_record_h record, unsigned int property_id, char** out_str )
{
	cal_alarm_s *rec = (cal_alarm_s*)(record);
	switch( property_id )
	{
	case CAL_PROPERTY_ALARM_DESCRIPTION:
		*out_str = SAFE_STRDUP(rec->alarm_description);
		break;
	case CAL_PROPERTY_ALARM_SUMMARY:
		*out_str = SAFE_STRDUP(rec->alarm_summary);
		break;
	case CAL_PROPERTY_ALARM_ATTACH:
		*out_str = SAFE_STRDUP(rec->alarm_attach);
		break;
	default:
		ERR("invalid parameter (property:%d)",property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_record_alarm_get_str_p( calendar_record_h record, unsigned int property_id, char** out_str )
{
	cal_alarm_s *rec = (cal_alarm_s*)(record);
	switch( property_id )
	{
	case CAL_PROPERTY_ALARM_DESCRIPTION:
		*out_str = (rec->alarm_description);
		break;
	case CAL_PROPERTY_ALARM_SUMMARY:
		*out_str = (rec->alarm_summary);
		break;
	case CAL_PROPERTY_ALARM_ATTACH:
		*out_str = (rec->alarm_attach);
		break;
	default:
		ERR("invalid parameter (property:%d)",property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_record_alarm_get_int( calendar_record_h record, unsigned int property_id, int* out_value )
{
	cal_alarm_s *rec = (cal_alarm_s*)(record);
	switch( property_id )
	{
	case CAL_PROPERTY_ALARM_TICK:
		*out_value = (rec->remind_tick);
		break;
	case CAL_PROPERTY_ALARM_TICK_UNIT:
		*out_value = (rec->remind_tick_unit);
		break;
	case CAL_PROPERTY_ALARM_PARENT_ID:
		*out_value = (rec->parent_id);
		break;
	case CAL_PROPERTY_ALARM_ACTION:
		*out_value = (rec->alarm_action);
		break;
	default:
		ERR("invalid parameter (property:%d)",property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_record_alarm_get_caltime( calendar_record_h record, unsigned int property_id, calendar_time_s* out_value )
{
	cal_alarm_s *rec = (cal_alarm_s*)(record);
	switch( property_id )
	{
	case CAL_PROPERTY_ALARM_ALARM:
		*out_value = rec->alarm;
		break;
	default:
		ERR("invalid parameter (property:%d)",property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	return CALENDAR_ERROR_NONE;
}

static int _cal_record_alarm_set_str( calendar_record_h record, unsigned int property_id, const char* value )
{
	cal_alarm_s *rec = (cal_alarm_s*)(record);
	switch( property_id )
	{
	case CAL_PROPERTY_ALARM_DESCRIPTION:
		CAL_FREE(rec->alarm_description);
		rec->alarm_description = SAFE_STRDUP(value);
		break;
	case CAL_PROPERTY_ALARM_SUMMARY:
		CAL_FREE(rec->alarm_summary);
		rec->alarm_summary = SAFE_STRDUP(value);
		break;
	case CAL_PROPERTY_ALARM_ATTACH:
		CAL_FREE(rec->alarm_attach);
		rec->alarm_attach = SAFE_STRDUP(value);
		break;
	default:
		ERR("invalid parameter (property:%d)",property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_record_alarm_set_int( calendar_record_h record, unsigned int property_id, int value )
{
	cal_alarm_s *rec = (cal_alarm_s*)(record);
	switch( property_id )
	{
	case CAL_PROPERTY_ALARM_TICK:
		(rec->remind_tick)=value;
		break;
	case CAL_PROPERTY_ALARM_TICK_UNIT:
		switch (value)
		{
		case CALENDAR_ALARM_NONE:
		case CALENDAR_ALARM_TIME_UNIT_SPECIFIC:
		case CALENDAR_ALARM_TIME_UNIT_MINUTE:
		case CALENDAR_ALARM_TIME_UNIT_HOUR:
		case CALENDAR_ALARM_TIME_UNIT_DAY:
		case CALENDAR_ALARM_TIME_UNIT_WEEK:
			(rec->remind_tick_unit)=value;
			break;
		default:
			ERR("invalid parameter (value:%d)", value);
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
		break;
	case CAL_PROPERTY_ALARM_PARENT_ID:
		(rec->parent_id) = value;
		break;
	case CAL_PROPERTY_ALARM_ACTION:
		switch (value)
		{
		case CALENDAR_ALARM_ACTION_AUDIO:
		case CALENDAR_ALARM_ACTION_DISPLAY:
		case CALENDAR_ALARM_ACTION_EMAIL:
			(rec->alarm_action) = value;
			break;
		default:
			ERR("invalid parameter (value:%d)", value);
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
		break;
	default:
		ERR("invalid parameter (property:%d)",property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_record_alarm_set_caltime( calendar_record_h record, unsigned int property_id, calendar_time_s value )
{
	cal_alarm_s *rec = (cal_alarm_s*)(record);
	switch( property_id )
	{
	case CAL_PROPERTY_ALARM_ALARM:
		rec->alarm = value;
		break;
	default:
		ERR("invalid parameter (property:%d)",property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	return CALENDAR_ERROR_NONE;

}
