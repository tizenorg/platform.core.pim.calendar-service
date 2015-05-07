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

static int _cal_record_attendee_create( calendar_record_h* out_record );
static int _cal_record_attendee_destroy( calendar_record_h record, bool delete_child );
static int _cal_record_attendee_clone( calendar_record_h record, calendar_record_h* out_record );
static int _cal_record_attendee_get_str( calendar_record_h record, unsigned int property_id, char** out_str );
static int _cal_record_attendee_get_str_p( calendar_record_h record, unsigned int property_id, char** out_str );
static int _cal_record_attendee_get_int( calendar_record_h record, unsigned int property_id, int* out_value );
static int _cal_record_attendee_set_str( calendar_record_h record, unsigned int property_id, const char* value );
static int _cal_record_attendee_set_int( calendar_record_h record, unsigned int property_id, int value );

cal_record_plugin_cb_s cal_record_attendee_plugin_cb = {
	.create = _cal_record_attendee_create,
	.destroy = _cal_record_attendee_destroy,
	.clone = _cal_record_attendee_clone,
	.get_str = _cal_record_attendee_get_str,
	.get_str_p = _cal_record_attendee_get_str_p,
	.get_int = _cal_record_attendee_get_int,
	.get_double = NULL,
	.get_lli = NULL,
	.get_caltime = NULL,
	.set_str = _cal_record_attendee_set_str,
	.set_int = _cal_record_attendee_set_int,
	.set_double = NULL,
	.set_lli = NULL,
	.set_caltime = NULL,
	.add_child_record = NULL,
	.remove_child_record = NULL,
	.get_child_record_count = NULL,
	.get_child_record_at_p = NULL,
	.clone_child_record_list = NULL
};

static void _cal_record_attendee_struct_init(cal_attendee_s *record)
{
	memset(record,0,sizeof(cal_attendee_s));
}

static int _cal_record_attendee_create(calendar_record_h* out_record )
{
	cal_attendee_s *temp = NULL;
	int ret= CALENDAR_ERROR_NONE;

	temp = calloc(1,sizeof(cal_attendee_s));
	RETVM_IF(NULL == temp, CALENDAR_ERROR_OUT_OF_MEMORY, "malloc(cal_attendee_s:sch) Failed(%d)", CALENDAR_ERROR_OUT_OF_MEMORY);

	_cal_record_attendee_struct_init(temp);

	*out_record = (calendar_record_h)temp;

	return ret;
}

static void _cal_record_attendee_struct_free(cal_attendee_s *record)
{
	CAL_FREE(record->attendee_number);
	CAL_FREE(record->attendee_uid);
	CAL_FREE(record->attendee_group);
	CAL_FREE(record->attendee_email);
	CAL_FREE(record->attendee_delegatee_uri);
	CAL_FREE(record->attendee_delegator_uri);
	CAL_FREE(record->attendee_name);
	CAL_FREE(record->attendee_member);
	CAL_FREE(record);
}

static int _cal_record_attendee_destroy( calendar_record_h record, bool delete_child )
{
	int ret = CALENDAR_ERROR_NONE;

	cal_attendee_s *temp = (cal_attendee_s*)(record);

	_cal_record_attendee_struct_free(temp);

	return ret;
}

static int _cal_record_attendee_clone( calendar_record_h record, calendar_record_h* out_record )
{
	cal_attendee_s *out_data = NULL;
	cal_attendee_s *src_data = NULL;

	src_data = (cal_attendee_s*)(record);

	out_data = calloc(1, sizeof(cal_attendee_s));
	RETVM_IF(NULL == out_data, CALENDAR_ERROR_OUT_OF_MEMORY, "calloc(cal_attendee_s) Failed(%d)", CALENDAR_ERROR_OUT_OF_MEMORY);

	CAL_RECORD_COPY_COMMON(&(out_data->common), &(src_data->common));

	out_data->parent_id = src_data->parent_id;
	out_data->attendee_cutype = src_data->attendee_cutype;
	out_data->attendee_ct_index = src_data->attendee_ct_index;
	out_data->attendee_role = src_data->attendee_role;
	out_data->attendee_status = src_data->attendee_status;
	out_data->attendee_rsvp = src_data->attendee_rsvp;
	out_data->attendee_number = SAFE_STRDUP(src_data->attendee_number);
	out_data->attendee_uid = SAFE_STRDUP(src_data->attendee_uid);
	out_data->attendee_group = SAFE_STRDUP(src_data->attendee_group);
	out_data->attendee_email = SAFE_STRDUP(src_data->attendee_email);
	out_data->attendee_delegatee_uri = SAFE_STRDUP(src_data->attendee_delegatee_uri);
	out_data->attendee_delegator_uri = SAFE_STRDUP(src_data->attendee_delegator_uri);
	out_data->attendee_name = SAFE_STRDUP(src_data->attendee_name);
	out_data->attendee_member = SAFE_STRDUP(src_data->attendee_member);

	*out_record = (calendar_record_h)out_data;

	return CALENDAR_ERROR_NONE;
}

static int _cal_record_attendee_get_str( calendar_record_h record, unsigned int property_id, char** out_str )
{
	cal_attendee_s *rec = (cal_attendee_s*)(record);
	switch( property_id )
	{
	case CAL_PROPERTY_ATTENDEE_NUMBER:
		*out_str = SAFE_STRDUP(rec->attendee_number);
		break;
	case CAL_PROPERTY_ATTENDEE_UID:
		*out_str = SAFE_STRDUP(rec->attendee_uid);
		break;
	case CAL_PROPERTY_ATTENDEE_GROUP:
		*out_str = SAFE_STRDUP(rec->attendee_group);
		break;
	case CAL_PROPERTY_ATTENDEE_EMAIL:
		*out_str = SAFE_STRDUP(rec->attendee_email);
		break;
	case CAL_PROPERTY_ATTENDEE_DELEGATEE_URI:
		*out_str = SAFE_STRDUP(rec->attendee_delegatee_uri);
		break;
	case CAL_PROPERTY_ATTENDEE_DELEGATOR_URI:
		*out_str = SAFE_STRDUP(rec->attendee_delegator_uri);
		break;
	case CAL_PROPERTY_ATTENDEE_NAME:
		*out_str = SAFE_STRDUP(rec->attendee_name);
		break;
	case CAL_PROPERTY_ATTENDEE_MEMBER:
		*out_str = SAFE_STRDUP(rec->attendee_member);
		break;
	default:
		ERR("invalid parameter (property:%d)",property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_record_attendee_get_str_p( calendar_record_h record, unsigned int property_id, char** out_str )
{
	cal_attendee_s *rec = (cal_attendee_s*)(record);
	switch( property_id )
	{
	case CAL_PROPERTY_ATTENDEE_NUMBER:
		*out_str = (rec->attendee_number);
		break;
	case CAL_PROPERTY_ATTENDEE_UID:
		*out_str = (rec->attendee_uid);
		break;
	case CAL_PROPERTY_ATTENDEE_GROUP:
		*out_str = (rec->attendee_group);
		break;
	case CAL_PROPERTY_ATTENDEE_EMAIL:
		*out_str = (rec->attendee_email);
		break;
	case CAL_PROPERTY_ATTENDEE_DELEGATEE_URI:
		*out_str = (rec->attendee_delegatee_uri);
		break;
	case CAL_PROPERTY_ATTENDEE_DELEGATOR_URI:
		*out_str = (rec->attendee_delegator_uri);
		break;
	case CAL_PROPERTY_ATTENDEE_NAME:
		*out_str = (rec->attendee_name);
		break;
	case CAL_PROPERTY_ATTENDEE_MEMBER:
		*out_str = (rec->attendee_member);
		break;
	default:
		ERR("invalid parameter (property:%d)",property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_record_attendee_get_int( calendar_record_h record, unsigned int property_id, int* out_value )
{
	cal_attendee_s *rec = (cal_attendee_s*)(record);
	switch( property_id )
	{
	case CAL_PROPERTY_ATTENDEE_CUTYPE:
		*out_value = (rec->attendee_cutype);
		break;
	case CAL_PROPERTY_ATTENDEE_CT_INDEX:
		*out_value = (rec->attendee_ct_index);
		break;
	case CAL_PROPERTY_ATTENDEE_ROLE:
		*out_value = (rec->attendee_role);
		break;
	case CAL_PROPERTY_ATTENDEE_STATUS:
		*out_value = (rec->attendee_status);
		break;
	case CAL_PROPERTY_ATTENDEE_RSVP:
		*out_value = (rec->attendee_rsvp);
		break;
	case CAL_PROPERTY_ATTENDEE_PARENT_ID:
		*out_value = (rec->parent_id);
		break;
	default:
		ERR("invalid parameter (property:%d)",property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_record_attendee_set_str( calendar_record_h record, unsigned int property_id, const char* value )
{
	cal_attendee_s *rec = (cal_attendee_s*)(record);
	switch( property_id )
	{
	case CAL_PROPERTY_ATTENDEE_NUMBER:
		CAL_FREE(rec->attendee_number);
		rec->attendee_number = SAFE_STRDUP(value);
		break;
	case CAL_PROPERTY_ATTENDEE_UID:
		CAL_FREE(rec->attendee_uid);
		rec->attendee_uid = SAFE_STRDUP(value);
		break;
	case CAL_PROPERTY_ATTENDEE_GROUP:
		CAL_FREE(rec->attendee_group);
		rec->attendee_group = SAFE_STRDUP(value);
		break;
	case CAL_PROPERTY_ATTENDEE_EMAIL:
		CAL_FREE(rec->attendee_email);
		rec->attendee_email = SAFE_STRDUP(value);
		break;
	case CAL_PROPERTY_ATTENDEE_DELEGATEE_URI:
		CAL_FREE(rec->attendee_delegatee_uri);
		rec->attendee_delegatee_uri = SAFE_STRDUP(value);
		break;
	case CAL_PROPERTY_ATTENDEE_DELEGATOR_URI:
		CAL_FREE(rec->attendee_delegator_uri);
		rec->attendee_delegator_uri = SAFE_STRDUP(value);
		break;
	case CAL_PROPERTY_ATTENDEE_NAME:
		CAL_FREE(rec->attendee_name);
		rec->attendee_name = SAFE_STRDUP(value);
		break;
	case CAL_PROPERTY_ATTENDEE_MEMBER:
		CAL_FREE(rec->attendee_member);
		rec->attendee_member = SAFE_STRDUP(value);
		break;
	default:
		ERR("invalid parameter (property:%d)",property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_record_attendee_set_int( calendar_record_h record, unsigned int property_id, int value )
{
	cal_attendee_s *rec = (cal_attendee_s*)(record);
	switch( property_id )
	{
	case CAL_PROPERTY_ATTENDEE_CUTYPE:
		(rec->attendee_cutype) = value;
		break;
	case CAL_PROPERTY_ATTENDEE_CT_INDEX:
		(rec->attendee_ct_index) = value;
		break;
	case CAL_PROPERTY_ATTENDEE_ROLE:
		(rec->attendee_role) = value;
		break;
	case CAL_PROPERTY_ATTENDEE_STATUS:
		(rec->attendee_status) = value;
		break;
	case CAL_PROPERTY_ATTENDEE_RSVP:
		(rec->attendee_rsvp) = value;
		break;
	case CAL_PROPERTY_ATTENDEE_PARENT_ID:
		(rec->parent_id) = value;
		break;
	default:
		ERR("invalid parameter (property:%d)",property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}
