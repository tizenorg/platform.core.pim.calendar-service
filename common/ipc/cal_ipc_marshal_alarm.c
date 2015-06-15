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
	.marshal_record = _cal_ipc_marshal_alarm
};

static int _cal_ipc_unmarshal_alarm(pims_ipc_data_h ipc_data, calendar_record_h record)
{
	int ret = 0;
	cal_alarm_s* palarm = NULL;
	RETV_IF(NULL == ipc_data, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == record, CALENDAR_ERROR_INVALID_PARAMETER);

	palarm = (cal_alarm_s*) record;

	/* read only or primary/secondary key */
	ret = cal_ipc_unmarshal_int(ipc_data, &palarm->id);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	ret = cal_ipc_unmarshal_int(ipc_data, &palarm->parent_id);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	ret = cal_ipc_unmarshal_int(ipc_data, &palarm->is_deleted);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	ret = cal_ipc_unmarshal_int(ipc_data, &palarm->remind_tick);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	ret = cal_ipc_unmarshal_int(ipc_data, (int*)&palarm->remind_tick_unit);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	ret = cal_ipc_unmarshal_char(ipc_data, &palarm->alarm_description);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	ret = cal_ipc_unmarshal_char(ipc_data, &palarm->alarm_summary);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	ret = cal_ipc_unmarshal_int(ipc_data, &palarm->alarm_action);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	ret = cal_ipc_unmarshal_char(ipc_data, &palarm->alarm_attach);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	ret = cal_ipc_unmarshal_caltime(ipc_data, &palarm->alarm);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_caltime() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	return CALENDAR_ERROR_NONE;
}

static int _cal_ipc_marshal_alarm(const calendar_record_h record, pims_ipc_data_h ipc_data)
{
	int ret = 0;
	cal_alarm_s* palarm = (cal_alarm_s*) record;
	RETV_IF(NULL == ipc_data, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == palarm, CALENDAR_ERROR_INVALID_PARAMETER);

	/* read only or primary/secondary key */
	ret = cal_ipc_marshal_int((palarm->id), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_int((palarm->parent_id), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	ret = cal_ipc_marshal_int((palarm->is_deleted), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	ret = cal_ipc_marshal_int((palarm->remind_tick), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_int((palarm->remind_tick_unit), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_char((palarm->alarm_description), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_char((palarm->alarm_summary), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_int((palarm->alarm_action), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_char((palarm->alarm_attach), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_caltime((palarm->alarm), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_caltime() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	return CALENDAR_ERROR_NONE;
}
