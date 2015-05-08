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

static int _cal_ipc_unmarshal_alarm(const pims_ipc_data_h ipc_data, calendar_record_h record);
static int _cal_ipc_marshal_alarm(const calendar_record_h record, pims_ipc_data_h ipc_data);

cal_ipc_marshal_record_plugin_cb_s cal_ipc_record_alarm_plugin_cb = {
	.unmarshal_record = _cal_ipc_unmarshal_alarm,
	.marshal_record = _cal_ipc_marshal_alarm,
	.get_primary_id = NULL
};

static int _cal_ipc_unmarshal_alarm(pims_ipc_data_h ipc_data, calendar_record_h record)
{
	cal_alarm_s* palarm = NULL;
	RETV_IF(ipc_data==NULL,CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(record==NULL,CALENDAR_ERROR_INVALID_PARAMETER);

	palarm = (cal_alarm_s*) record;

	// read only or primary/secondary key
	if (cal_ipc_unmarshal_int(ipc_data,&palarm->id) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	if (cal_ipc_unmarshal_int(ipc_data,&palarm->parent_id) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	if (cal_ipc_unmarshal_int(ipc_data,&palarm->is_deleted) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	if (cal_ipc_unmarshal_int(ipc_data,&palarm->remind_tick) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	if (cal_ipc_unmarshal_int(ipc_data,(int*)&palarm->remind_tick_unit) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	if (cal_ipc_unmarshal_char(ipc_data,&palarm->alarm_description) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	if (cal_ipc_unmarshal_char(ipc_data,&palarm->alarm_summary) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	if (cal_ipc_unmarshal_int(ipc_data,&palarm->alarm_action) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	if (cal_ipc_unmarshal_char(ipc_data,&palarm->alarm_attach) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	if (cal_ipc_unmarshal_caltime(ipc_data,&palarm->alarm) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	return CALENDAR_ERROR_NONE;
}

static int _cal_ipc_marshal_alarm(const calendar_record_h record, pims_ipc_data_h ipc_data)
{
	cal_alarm_s* palarm = (cal_alarm_s*) record;
	RETV_IF(ipc_data==NULL,CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(palarm==NULL,CALENDAR_ERROR_INVALID_PARAMETER);

	// read only or primary/secondary key
	if (cal_ipc_marshal_int((palarm->id),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (cal_ipc_marshal_int((palarm->parent_id),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	if (cal_ipc_marshal_int((palarm->is_deleted),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	if (cal_ipc_marshal_int((palarm->remind_tick),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_marshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (cal_ipc_marshal_int((palarm->remind_tick_unit),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_marshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (cal_ipc_marshal_char((palarm->alarm_description),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_marshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (cal_ipc_marshal_char((palarm->alarm_summary),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_marshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (cal_ipc_marshal_int((palarm->alarm_action),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_marshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (cal_ipc_marshal_char((palarm->alarm_attach),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_marshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (cal_ipc_marshal_caltime((palarm->alarm),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_marshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	return CALENDAR_ERROR_NONE;
}
