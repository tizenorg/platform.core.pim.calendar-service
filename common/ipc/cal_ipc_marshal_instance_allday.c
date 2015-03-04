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
#include "cal_view.h"

static int __cal_ipc_unmarshal_instance_allday(pims_ipc_data_h ipc_data, calendar_record_h record);
static int __cal_ipc_marshal_instance_allday(const calendar_record_h record, pims_ipc_data_h ipc_data);

cal_ipc_marshal_record_plugin_cb_s _cal_ipc_record_instance_allday_plugin_cb = {
	.unmarshal_record = __cal_ipc_unmarshal_instance_allday,
	.marshal_record = __cal_ipc_marshal_instance_allday,
	.get_primary_id = NULL
};

static int __cal_ipc_unmarshal_instance_allday(pims_ipc_data_h ipc_data, calendar_record_h record)
{
	cal_instance_allday_s* pinstanceallday = NULL;
	retv_if(ipc_data==NULL,CALENDAR_ERROR_NO_DATA);
	retv_if(record==NULL,CALENDAR_ERROR_NO_DATA);

	pinstanceallday = (cal_instance_allday_s*) record;
	if (_cal_ipc_unmarshal_int(ipc_data,&pinstanceallday->event_id) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (_cal_ipc_unmarshal_int(ipc_data,&pinstanceallday->calendar_id) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (_cal_ipc_unmarshal_caltime(ipc_data,&pinstanceallday->start) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (_cal_ipc_unmarshal_caltime(ipc_data,&pinstanceallday->end) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (_cal_ipc_unmarshal_char(ipc_data,&pinstanceallday->summary) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (_cal_ipc_unmarshal_char(ipc_data,&pinstanceallday->description) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (_cal_ipc_unmarshal_char(ipc_data,&pinstanceallday->location) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (_cal_ipc_unmarshal_int(ipc_data,&pinstanceallday->busy_status) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (_cal_ipc_unmarshal_int(ipc_data,&pinstanceallday->event_status) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (_cal_ipc_unmarshal_int(ipc_data,&pinstanceallday->priority) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (_cal_ipc_unmarshal_int(ipc_data,&pinstanceallday->sensitivity) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (_cal_ipc_unmarshal_int(ipc_data,&pinstanceallday->has_rrule) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (_cal_ipc_unmarshal_double(ipc_data,&pinstanceallday->latitude) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (_cal_ipc_unmarshal_double(ipc_data,&pinstanceallday->longitude) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (_cal_ipc_unmarshal_int(ipc_data,&pinstanceallday->has_alarm) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (_cal_ipc_unmarshal_int(ipc_data,&pinstanceallday->original_event_id) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (_cal_ipc_unmarshal_lli(ipc_data,&pinstanceallday->last_mod) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (_cal_ipc_unmarshal_char(ipc_data,&pinstanceallday->sync_data1) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (_cal_ipc_unmarshal_int(ipc_data,&pinstanceallday->is_allday) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int __cal_ipc_marshal_instance_allday(const calendar_record_h record, pims_ipc_data_h ipc_data)
{
	cal_instance_allday_s* pinstanceallday = (cal_instance_allday_s*) record;
	retv_if(ipc_data==NULL,CALENDAR_ERROR_NO_DATA);
	retv_if(pinstanceallday==NULL,CALENDAR_ERROR_NO_DATA);

	if (_cal_ipc_marshal_int((pinstanceallday->event_id),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_marshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (_cal_ipc_marshal_int((pinstanceallday->calendar_id),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_marshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (_cal_ipc_marshal_caltime((pinstanceallday->start),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_marshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (_cal_ipc_marshal_caltime((pinstanceallday->end),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_marshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (_cal_ipc_marshal_char((pinstanceallday->summary),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_marshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (_cal_ipc_marshal_char((pinstanceallday->description),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_marshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (_cal_ipc_marshal_char((pinstanceallday->location),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_marshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (_cal_ipc_marshal_int((pinstanceallday->busy_status),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_marshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (_cal_ipc_marshal_int((pinstanceallday->event_status),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_marshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (_cal_ipc_marshal_int((pinstanceallday->priority),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_marshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (_cal_ipc_marshal_int((pinstanceallday->sensitivity),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_marshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (_cal_ipc_marshal_int((pinstanceallday->has_rrule),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_marshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (_cal_ipc_marshal_double((pinstanceallday->latitude),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_marshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (_cal_ipc_marshal_double((pinstanceallday->longitude),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_marshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (_cal_ipc_marshal_int((pinstanceallday->has_alarm),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_marshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (_cal_ipc_marshal_int((pinstanceallday->original_event_id),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_marshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (_cal_ipc_marshal_lli((pinstanceallday->last_mod),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_marshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (_cal_ipc_marshal_char((pinstanceallday->sync_data1),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_marshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (_cal_ipc_marshal_int((pinstanceallday->is_allday),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_marshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}
