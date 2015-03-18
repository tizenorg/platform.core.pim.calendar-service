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

#include "cal_internal.h"
#include "cal_ipc_marshal.h"

static int __cal_ipc_unmarshal_attendee(const pims_ipc_data_h ipc_data, calendar_record_h record);
static int __cal_ipc_marshal_attendee(const calendar_record_h record, pims_ipc_data_h ipc_data);

cal_ipc_marshal_record_plugin_cb_s _cal_ipc_record_attendee_plugin_cb = {
	.unmarshal_record = __cal_ipc_unmarshal_attendee,
	.marshal_record = __cal_ipc_marshal_attendee,
	.get_primary_id = NULL
};

static int __cal_ipc_unmarshal_attendee(const pims_ipc_data_h ipc_data, calendar_record_h record)
{
	cal_attendee_s* pattendee = NULL;
	RETV_IF(ipc_data==NULL,CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(record==NULL,CALENDAR_ERROR_INVALID_PARAMETER);

	pattendee = (cal_attendee_s*) record;

	// read only or primary/secondary key
	if (_cal_ipc_unmarshal_int(ipc_data,&pattendee->id) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	if (_cal_ipc_unmarshal_int(ipc_data,&pattendee->parent_id) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	if (_cal_ipc_unmarshal_char(ipc_data,&pattendee->attendee_number) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	if (_cal_ipc_unmarshal_int(ipc_data,&pattendee->attendee_cutype) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	if (_cal_ipc_unmarshal_int(ipc_data,&pattendee->attendee_ct_index) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	if (_cal_ipc_unmarshal_char(ipc_data,&pattendee->attendee_uid) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	if (_cal_ipc_unmarshal_char(ipc_data,&pattendee->attendee_group) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	if (_cal_ipc_unmarshal_char(ipc_data,&pattendee->attendee_email) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	if (_cal_ipc_unmarshal_int(ipc_data,&pattendee->attendee_role) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	if (_cal_ipc_unmarshal_int(ipc_data,&pattendee->attendee_status) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	if (_cal_ipc_unmarshal_int(ipc_data,&pattendee->attendee_rsvp) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	if (_cal_ipc_unmarshal_char(ipc_data,&pattendee->attendee_delegatee_uri) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	if (_cal_ipc_unmarshal_char(ipc_data,&pattendee->attendee_delegator_uri) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	if (_cal_ipc_unmarshal_char(ipc_data,&pattendee->attendee_name) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	if (_cal_ipc_unmarshal_char(ipc_data,&pattendee->attendee_member) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int __cal_ipc_marshal_attendee(const calendar_record_h record, pims_ipc_data_h ipc_data )
{
	cal_attendee_s* pattendee = (cal_attendee_s*) record;

	RETV_IF(ipc_data==NULL,CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(pattendee==NULL,CALENDAR_ERROR_INVALID_PARAMETER);

	// read only or primary/secondary key
	if (_cal_ipc_marshal_int((pattendee->id),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	if (_cal_ipc_marshal_int((pattendee->parent_id),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	if (_cal_ipc_marshal_char((pattendee->attendee_number),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	if (_cal_ipc_marshal_int((pattendee->attendee_cutype),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	if (_cal_ipc_marshal_int((pattendee->attendee_ct_index),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	if (_cal_ipc_marshal_char((pattendee->attendee_uid),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	if (_cal_ipc_marshal_char((pattendee->attendee_group),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	if (_cal_ipc_marshal_char((pattendee->attendee_email),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	if (_cal_ipc_marshal_int((pattendee->attendee_role),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	if (_cal_ipc_marshal_int((pattendee->attendee_status),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	if (_cal_ipc_marshal_int((pattendee->attendee_rsvp),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	if (_cal_ipc_marshal_char((pattendee->attendee_delegatee_uri),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	if (_cal_ipc_marshal_char((pattendee->attendee_delegator_uri),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	if (_cal_ipc_marshal_char((pattendee->attendee_name),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	if (_cal_ipc_marshal_char((pattendee->attendee_member),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}
