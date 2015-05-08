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

#include "cal_internal.h"
#include "cal_ipc_marshal.h"
#include "cal_view.h"

static int _cal_ipc_unmarshal_instance_normal(pims_ipc_data_h ipc_data, calendar_record_h record);
static int _cal_ipc_marshal_instance_normal(const calendar_record_h record, pims_ipc_data_h ipc_data);

cal_ipc_marshal_record_plugin_cb_s cal_ipc_record_instance_normal_plugin_cb = {
	.unmarshal_record = _cal_ipc_unmarshal_instance_normal,
	.marshal_record = _cal_ipc_marshal_instance_normal,
	.get_primary_id = NULL
};

static int _cal_ipc_unmarshal_instance_normal(pims_ipc_data_h ipc_data, calendar_record_h record)
{
	cal_instance_normal_s* pinstancenormal = NULL;
	RETV_IF(ipc_data==NULL,CALENDAR_ERROR_NO_DATA);
	RETV_IF(record==NULL,CALENDAR_ERROR_NO_DATA);

	pinstancenormal = (cal_instance_normal_s*) record;
	if (cal_ipc_unmarshal_int(ipc_data,&pinstancenormal->event_id) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (cal_ipc_unmarshal_int(ipc_data,&pinstancenormal->calendar_id) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (cal_ipc_unmarshal_caltime(ipc_data,&pinstancenormal->start) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (cal_ipc_unmarshal_caltime(ipc_data,&pinstancenormal->end) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (cal_ipc_unmarshal_char(ipc_data,&pinstancenormal->summary) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (cal_ipc_unmarshal_char(ipc_data,&pinstancenormal->description) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (cal_ipc_unmarshal_char(ipc_data,&pinstancenormal->location) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (cal_ipc_unmarshal_int(ipc_data,&pinstancenormal->busy_status) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (cal_ipc_unmarshal_int(ipc_data,&pinstancenormal->event_status) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (cal_ipc_unmarshal_int(ipc_data,&pinstancenormal->priority) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (cal_ipc_unmarshal_int(ipc_data,&pinstancenormal->sensitivity) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (cal_ipc_unmarshal_int(ipc_data,&pinstancenormal->has_rrule) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (cal_ipc_unmarshal_double(ipc_data,&pinstancenormal->latitude) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (cal_ipc_unmarshal_double(ipc_data,&pinstancenormal->longitude) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (cal_ipc_unmarshal_int(ipc_data,&pinstancenormal->has_alarm) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (cal_ipc_unmarshal_int(ipc_data,&pinstancenormal->original_event_id) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (cal_ipc_unmarshal_lli(ipc_data,&pinstancenormal->last_mod) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (cal_ipc_unmarshal_char(ipc_data,&pinstancenormal->sync_data1) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_ipc_marshal_instance_normal(const calendar_record_h record, pims_ipc_data_h ipc_data)
{
	cal_instance_normal_s* pinstancenormal = (cal_instance_normal_s*) record;
	RETV_IF(ipc_data==NULL,CALENDAR_ERROR_NO_DATA);
	RETV_IF(pinstancenormal==NULL,CALENDAR_ERROR_NO_DATA);

	if (cal_ipc_marshal_int((pinstancenormal->event_id),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_marshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (cal_ipc_marshal_int((pinstancenormal->calendar_id),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_marshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (cal_ipc_marshal_caltime((pinstancenormal->start),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_marshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (cal_ipc_marshal_caltime((pinstancenormal->end),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_marshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (cal_ipc_marshal_char((pinstancenormal->summary),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_marshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (cal_ipc_marshal_char((pinstancenormal->description),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_marshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (cal_ipc_marshal_char((pinstancenormal->location),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_marshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (cal_ipc_marshal_int((pinstancenormal->busy_status),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_marshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (cal_ipc_marshal_int((pinstancenormal->event_status),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_marshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (cal_ipc_marshal_int((pinstancenormal->priority),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_marshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (cal_ipc_marshal_int((pinstancenormal->sensitivity),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_marshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (cal_ipc_marshal_int((pinstancenormal->has_rrule),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_marshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (cal_ipc_marshal_double((pinstancenormal->latitude),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_marshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (cal_ipc_marshal_double((pinstancenormal->longitude),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_marshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (cal_ipc_marshal_int((pinstancenormal->has_alarm),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_marshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (cal_ipc_marshal_int((pinstancenormal->original_event_id),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_marshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (cal_ipc_marshal_lli((pinstancenormal->last_mod),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_marshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (cal_ipc_marshal_char((pinstancenormal->sync_data1),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_marshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}
