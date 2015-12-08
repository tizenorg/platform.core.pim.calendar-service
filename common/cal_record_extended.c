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

static int _cal_record_extended_create(calendar_record_h* out_record);
static int _cal_record_extended_destroy(calendar_record_h record, bool delete_child);
static int _cal_record_extended_clone(calendar_record_h record, calendar_record_h* out_record);
static int _cal_record_extended_get_str(calendar_record_h record, unsigned int property_id, char** out_str);
static int _cal_record_extended_get_str_p(calendar_record_h record, unsigned int property_id, char** out_str);
static int _cal_record_extended_get_int(calendar_record_h record, unsigned int property_id, int* out_value);
static int _cal_record_extended_set_str(calendar_record_h record, unsigned int property_id, const char* value);
static int _cal_record_extended_set_int(calendar_record_h record, unsigned int property_id, int value);

cal_record_plugin_cb_s cal_record_extended_plugin_cb = {
	.create = _cal_record_extended_create,
	.destroy = _cal_record_extended_destroy,
	.clone = _cal_record_extended_clone,
	.get_str = _cal_record_extended_get_str,
	.get_str_p = _cal_record_extended_get_str_p,
	.get_int = _cal_record_extended_get_int,
	.get_double = NULL,
	.get_lli = NULL,
	.get_caltime = NULL,
	.set_str = _cal_record_extended_set_str,
	.set_int = _cal_record_extended_set_int,
	.set_double = NULL,
	.set_lli = NULL,
	.set_caltime = NULL,
	.add_child_record = NULL,
	.remove_child_record = NULL,
	.get_child_record_count = NULL,
	.get_child_record_at_p = NULL,
	.clone_child_record_list = NULL
};

static void _cal_record_extended_struct_init(cal_extended_s *record)
{
	memset(record, 0, sizeof(cal_extended_s));
}

static int _cal_record_extended_create(calendar_record_h* out_record)
{
	cal_extended_s *temp = NULL;
	int ret = CALENDAR_ERROR_NONE;

	temp = calloc(1, sizeof(cal_extended_s));
	RETVM_IF(NULL == temp, CALENDAR_ERROR_OUT_OF_MEMORY, "calloc() Fail");

	_cal_record_extended_struct_init(temp);

	*out_record = (calendar_record_h)temp;

	return ret;
}

static void _cal_record_extended_struct_free(cal_extended_s *record)
{
	CAL_FREE(record->key);
	CAL_FREE(record->value);
	CAL_FREE(record);

}

static int _cal_record_extended_destroy(calendar_record_h record, bool delete_child)
{
	int ret = CALENDAR_ERROR_NONE;

	cal_extended_s *temp = (cal_extended_s*)(record);

	_cal_record_extended_struct_free(temp);

	return ret;
}

static int _cal_record_extended_clone(calendar_record_h record, calendar_record_h* out_record)
{
	cal_extended_s *out_data = NULL;
	cal_extended_s *src_data = NULL;

	src_data = (cal_extended_s*)(record);

	out_data = calloc(1, sizeof(cal_extended_s));
	RETVM_IF(NULL == out_data, CALENDAR_ERROR_OUT_OF_MEMORY, "calloc() Fail");

	CAL_RECORD_COPY_COMMON(&(out_data->common), &(src_data->common));

	out_data->id = src_data->id;
	out_data->record_id = src_data->record_id;
	out_data->record_type = src_data->record_type;
	out_data->key = cal_strdup(src_data->key);
	out_data->value = cal_strdup(src_data->value);

	*out_record = (calendar_record_h)out_data;

	return CALENDAR_ERROR_NONE;
}

static int _cal_record_extended_get_str(calendar_record_h record, unsigned int property_id, char** out_str)
{
	cal_extended_s *rec = (cal_extended_s*)(record);
	switch (property_id) {
	case CAL_PROPERTY_EXTENDED_KEY:
		*out_str = cal_strdup(rec->key);
		break;
	case CAL_PROPERTY_EXTENDED_VALUE:
		*out_str = cal_strdup(rec->value);
		break;
	default:
		ERR("invalid parameter (property:%d)", property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_record_extended_get_str_p(calendar_record_h record, unsigned int property_id, char** out_str)
{
	cal_extended_s *rec = (cal_extended_s*)(record);
	switch (property_id) {
	case CAL_PROPERTY_EXTENDED_KEY:
		*out_str = (rec->key);
		break;
	case CAL_PROPERTY_EXTENDED_VALUE:
		*out_str = (rec->value);
		break;
	default:
		ERR("invalid parameter (property:%d)", property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_record_extended_get_int(calendar_record_h record, unsigned int property_id, int* out_value)
{
	cal_extended_s *rec = (cal_extended_s*)(record);
	switch (property_id) {
	case CAL_PROPERTY_EXTENDED_ID:
		*out_value = (rec->id);
		break;
	case CAL_PROPERTY_EXTENDED_RECORD_ID:
		*out_value = (rec->record_id);
		break;
	case CAL_PROPERTY_EXTENDED_RECORD_TYPE:
		*out_value = (rec->record_type);
		break;
	default:
		ERR("invalid parameter (property:%d)", property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_record_extended_set_str(calendar_record_h record, unsigned int property_id, const char* value)
{
	cal_extended_s *rec = (cal_extended_s*)(record);
	switch (property_id) {
	case CAL_PROPERTY_EXTENDED_KEY:
		CAL_FREE(rec->key);
		rec->key = cal_strdup(value);
		break;
	case CAL_PROPERTY_EXTENDED_VALUE:
		CAL_FREE(rec->value);
		rec->value = cal_strdup(value);
		break;
	default:
		ERR("invalid parameter (property:%d)", property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_record_extended_set_int(calendar_record_h record, unsigned int property_id, int value)
{
	cal_extended_s *rec = (cal_extended_s*)(record);
	switch (property_id) {
	case CAL_PROPERTY_EXTENDED_ID:
		(rec->id) = value;
		break;
	case CAL_PROPERTY_EXTENDED_RECORD_ID:
		RETVM_IF(0 < rec->id, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter : property_id(%d) is a write-once value (calendar)", property_id);
		(rec->record_id) = value;
		break;
	case CAL_PROPERTY_EXTENDED_RECORD_TYPE:
		(rec->record_type) = value;
		break;
	default:
		ERR("invalid parameter (property:%d)", property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}
