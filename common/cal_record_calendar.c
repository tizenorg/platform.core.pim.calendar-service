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

static int _cal_record_calendar_create(calendar_record_h* out_record);
static int _cal_record_calendar_destroy(calendar_record_h record, bool delete_child);
static int _cal_record_calendar_clone(calendar_record_h record, calendar_record_h* out_record);
static int _cal_record_calendar_get_str(calendar_record_h record, unsigned int property_id, char** out_str);
static int _cal_record_calendar_get_str_p(calendar_record_h record, unsigned int property_id, char** out_str);
static int _cal_record_calendar_get_int(calendar_record_h record, unsigned int property_id, int* out_value);
static int _cal_record_calendar_set_str(calendar_record_h record, unsigned int property_id, const char* value);
static int _cal_record_calendar_set_int(calendar_record_h record, unsigned int property_id, int value);

cal_record_plugin_cb_s cal_record_calendar_plugin_cb = {
	.create = _cal_record_calendar_create,
	.destroy = _cal_record_calendar_destroy,
	.clone = _cal_record_calendar_clone,
	.get_str = _cal_record_calendar_get_str,
	.get_str_p = _cal_record_calendar_get_str_p,
	.get_int = _cal_record_calendar_get_int,
	.get_double = NULL,
	.get_lli = NULL,
	.get_caltime = NULL,
	.set_str = _cal_record_calendar_set_str,
	.set_int = _cal_record_calendar_set_int,
	.set_double = NULL,
	.set_lli = NULL,
	.set_caltime = NULL,
	.add_child_record = NULL,
	.remove_child_record = NULL,
	.get_child_record_count = NULL,
	.get_child_record_at_p = NULL,
	.clone_child_record_list = NULL
};

static void _cal_record_calendar_struct_init(cal_calendar_s *record)
{
	memset(record,0, sizeof(cal_calendar_s));
	record->index = -1;
	record->visibility = true;
	record->account_id = LOCAL_ACCOUNT_ID;
	record->sync_event = 1;
}

static int _cal_record_calendar_create(calendar_record_h* out_record)
{
	cal_calendar_s *temp = NULL;
	int ret= CALENDAR_ERROR_NONE;

	temp = calloc(1, sizeof(cal_calendar_s));
	RETVM_IF(NULL == temp, CALENDAR_ERROR_OUT_OF_MEMORY, "calloc() Fail");

	_cal_record_calendar_struct_init(temp);

	*out_record = (calendar_record_h)temp;

	return ret;
}

static void _cal_record_calendar_struct_free(cal_calendar_s *record)
{
	CAL_FREE(record->uid);
	CAL_FREE(record->name);
	CAL_FREE(record->description);
	CAL_FREE(record->color);
	CAL_FREE(record->location);
	CAL_FREE(record->sync_data1);
	CAL_FREE(record->sync_data2);
	CAL_FREE(record->sync_data3);
	CAL_FREE(record->sync_data4);
	CAL_FREE(record);
}

static int _cal_record_calendar_destroy(calendar_record_h record, bool delete_child)
{
	int ret = CALENDAR_ERROR_NONE;

	cal_calendar_s *temp = (cal_calendar_s*)(record);

	_cal_record_calendar_struct_free(temp);

	return ret;
}

static int _cal_record_calendar_clone(calendar_record_h record, calendar_record_h* out_record)
{
	cal_calendar_s *out_data = NULL;
	cal_calendar_s *src_data = NULL;

	src_data = (cal_calendar_s*)(record);

	out_data = calloc(1, sizeof(cal_calendar_s));
	RETVM_IF(NULL == out_data, CALENDAR_ERROR_OUT_OF_MEMORY, "calloc() Fail");


	CAL_RECORD_COPY_COMMON(&(out_data->common), &(src_data->common));

	out_data->index = src_data->index;
	out_data->store_type = src_data->store_type;
	out_data->uid = cal_strdup(src_data->uid);
	out_data->updated = src_data->updated;
	out_data->name = cal_strdup(src_data->name);
	out_data->description = cal_strdup(src_data->description);
	out_data->color = cal_strdup(src_data->color);
	out_data->location = cal_strdup(src_data->location);
	out_data->visibility = src_data->visibility;
	out_data->sync_event = src_data->sync_event;
	out_data->is_deleted = src_data->is_deleted;
	out_data->account_id = src_data->account_id;
	out_data->sync_data1 = cal_strdup(src_data->sync_data1);
	out_data->sync_data2 = cal_strdup(src_data->sync_data2);
	out_data->sync_data3 = cal_strdup(src_data->sync_data3);
	out_data->sync_data4 = cal_strdup(src_data->sync_data4);
	out_data->mode = src_data->mode;

	*out_record = (calendar_record_h)out_data;

	return CALENDAR_ERROR_NONE;
}

static int _cal_record_calendar_get_str(calendar_record_h record, unsigned int property_id, char** out_str)
{
	cal_calendar_s *cal_rec = (cal_calendar_s*)(record);
	switch (property_id) {
	case CAL_PROPERTY_CALENDAR_UID:
		*out_str = cal_strdup(cal_rec->uid);
		break;
	case CAL_PROPERTY_CALENDAR_NAME:
		*out_str = cal_strdup(cal_rec->name);
		break;
	case CAL_PROPERTY_CALENDAR_DESCRIPTION:
		*out_str = cal_strdup(cal_rec->description);
		break;
	case CAL_PROPERTY_CALENDAR_COLOR:
		*out_str = cal_strdup(cal_rec->color);
		break;
	case CAL_PROPERTY_CALENDAR_LOCATION:
		*out_str = cal_strdup(cal_rec->location);
		break;
	case CAL_PROPERTY_CALENDAR_SYNC_DATA1:
		*out_str = cal_strdup(cal_rec->sync_data1);
		break;
	case CAL_PROPERTY_CALENDAR_SYNC_DATA2:
		*out_str = cal_strdup(cal_rec->sync_data2);
		break;
	case CAL_PROPERTY_CALENDAR_SYNC_DATA3:
		*out_str = cal_strdup(cal_rec->sync_data3);
		break;
	case CAL_PROPERTY_CALENDAR_SYNC_DATA4:
		*out_str = cal_strdup(cal_rec->sync_data4);
		break;
	default:
		ERR("invalid parameter (property:%d)",property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_record_calendar_get_str_p(calendar_record_h record, unsigned int property_id, char** out_str)
{
	cal_calendar_s *cal_rec = (cal_calendar_s*)(record);
	switch (property_id) {
	case CAL_PROPERTY_CALENDAR_UID:
		*out_str = (cal_rec->uid);
		break;
	case CAL_PROPERTY_CALENDAR_NAME:
		*out_str = (cal_rec->name);
		break;
	case CAL_PROPERTY_CALENDAR_DESCRIPTION:
		*out_str = (cal_rec->description);
		break;
	case CAL_PROPERTY_CALENDAR_COLOR:
		*out_str = (cal_rec->color);
		break;
	case CAL_PROPERTY_CALENDAR_LOCATION:
		*out_str = (cal_rec->location);
		break;
	case CAL_PROPERTY_CALENDAR_SYNC_DATA1:
		*out_str = (cal_rec->sync_data1);
		break;
	case CAL_PROPERTY_CALENDAR_SYNC_DATA2:
		*out_str = (cal_rec->sync_data2);
		break;
	case CAL_PROPERTY_CALENDAR_SYNC_DATA3:
		*out_str = (cal_rec->sync_data3);
		break;
	case CAL_PROPERTY_CALENDAR_SYNC_DATA4:
		*out_str = (cal_rec->sync_data4);
		break;
	default:
		ERR("invalid parameter (property:%d)",property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_record_calendar_get_int(calendar_record_h record, unsigned int property_id, int* out_value)
{
	cal_calendar_s *cal_rec = (cal_calendar_s*)(record);
	switch (property_id) {
	case CAL_PROPERTY_CALENDAR_ID:
		*out_value = (cal_rec->index);
		break;
	case CAL_PROPERTY_CALENDAR_VISIBILITY:
		*out_value = (cal_rec->visibility);
		break;
	case CAL_PROPERTY_CALENDAR_ACCOUNT_ID:
		*out_value = (cal_rec->account_id);
		break;
	case CAL_PROPERTY_CALENDAR_STORE_TYPE:
		*out_value = (cal_rec->store_type);
		break;
	case CAL_PROPERTY_CALENDAR_SYNC_EVENT:
		*out_value = (cal_rec->sync_event);
		break;
	case CAL_PROPERTY_CALENDAR_MODE:
		*out_value = (cal_rec->mode);
		break;
	default:
		ERR("invalid parameter (property:%d)",property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_record_calendar_set_str(calendar_record_h record, unsigned int property_id, const char* value)
{
	cal_calendar_s *cal_rec = (cal_calendar_s*)(record);
	switch (property_id) {
	case CAL_PROPERTY_CALENDAR_UID:
		CAL_FREE(cal_rec->uid);
		cal_rec->uid = cal_strdup(value);
		break;
	case CAL_PROPERTY_CALENDAR_NAME:
		CAL_FREE(cal_rec->name);
		cal_rec->name = cal_strdup(value);
		break;
	case CAL_PROPERTY_CALENDAR_DESCRIPTION:
		CAL_FREE(cal_rec->description);
		cal_rec->description = cal_strdup(value);
		break;
	case CAL_PROPERTY_CALENDAR_COLOR:
		CAL_FREE(cal_rec->color);
		cal_rec->color = cal_strdup(value);
		break;
	case CAL_PROPERTY_CALENDAR_LOCATION:
		CAL_FREE(cal_rec->location);
		cal_rec->location = cal_strdup(value);
		break;
	case CAL_PROPERTY_CALENDAR_SYNC_DATA1:
		CAL_FREE(cal_rec->sync_data1);
		cal_rec->sync_data1 = cal_strdup(value);
		break;
	case CAL_PROPERTY_CALENDAR_SYNC_DATA2:
		CAL_FREE(cal_rec->sync_data2);
		cal_rec->sync_data2 = cal_strdup(value);
		break;
	case CAL_PROPERTY_CALENDAR_SYNC_DATA3:
		CAL_FREE(cal_rec->sync_data3);
		cal_rec->sync_data3 = cal_strdup(value);
		break;
	case CAL_PROPERTY_CALENDAR_SYNC_DATA4:
		CAL_FREE(cal_rec->sync_data4);
		cal_rec->sync_data4 = cal_strdup(value);
		break;
	default:
		ERR("invalid parameter (property:%d)",property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_record_calendar_set_int(calendar_record_h record, unsigned int property_id, int value)
{
	cal_calendar_s *cal_rec = (cal_calendar_s*)(record);
	switch (property_id) {
	case CAL_PROPERTY_CALENDAR_ID:
		(cal_rec->index) = value;
		break;
	case CAL_PROPERTY_CALENDAR_VISIBILITY:
		(cal_rec->visibility) = value;
		break;
	case CAL_PROPERTY_CALENDAR_ACCOUNT_ID:
		RETVM_IF(0 < cal_rec->index, CALENDAR_ERROR_INVALID_PARAMETER, "property_id(%d) is a write-once value (calendar)", property_id);
		(cal_rec->account_id) = value;
		break;
	case CAL_PROPERTY_CALENDAR_STORE_TYPE:
		switch (value) {
		case CALENDAR_BOOK_TYPE_NONE:
		case CALENDAR_BOOK_TYPE_EVENT:
		case CALENDAR_BOOK_TYPE_TODO:
			(cal_rec->store_type) = value;
			break;
		default:
			ERR("Invalid parameter : store type is invalid value (%d)", value);
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
		break;
	case CAL_PROPERTY_CALENDAR_SYNC_EVENT:
		switch (value) {
		case CALENDAR_BOOK_SYNC_EVENT_FOR_ME:
		case CALENDAR_BOOK_SYNC_EVENT_FOR_EVERY_AND_REMAIN:
		case CALENDAR_BOOK_SYNC_EVENT_FOR_EVERY_AND_DELETE:
			(cal_rec->sync_event) = value;
			break;
		default:
			ERR("Invalid parameter : sync event is invalid value (%d)", value);
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
		break;
	case CAL_PROPERTY_CALENDAR_MODE:
		RETVM_IF(value != CALENDAR_BOOK_MODE_NONE && value != CALENDAR_BOOK_MODE_RECORD_READONLY,
				CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter : mode type is invalid value (%d)", value);
		(cal_rec->mode) = value;
		break;
	default:
		ERR("invalid parameter (property:%d)",property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}
