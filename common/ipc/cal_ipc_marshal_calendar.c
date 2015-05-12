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

static int _cal_ipc_unmarshal_calendar(pims_ipc_data_h ipc_data, calendar_record_h record);
static int _cal_ipc_marshal_calendar(const calendar_record_h record, pims_ipc_data_h ipc_data);
static int _cal_ipc_marshal_calendar_get_primary_id(const calendar_record_h record, unsigned int *property_id, int *id);

cal_ipc_marshal_record_plugin_cb_s cal_ipc_record_calendar_plugin_cb = {
	.unmarshal_record = _cal_ipc_unmarshal_calendar,
	.marshal_record = _cal_ipc_marshal_calendar,
	.get_primary_id = _cal_ipc_marshal_calendar_get_primary_id
};

static int _cal_ipc_unmarshal_calendar(pims_ipc_data_h ipc_data, calendar_record_h record)
{
	int ret = 0;
	cal_calendar_s* pcalendar = NULL;
	RETV_IF(NULL == ipc_data, CALENDAR_ERROR_NO_DATA);
	RETV_IF(NULL == record, CALENDAR_ERROR_NO_DATA);

	pcalendar = (cal_calendar_s*) record;

	ret = cal_ipc_unmarshal_int(ipc_data, &pcalendar->index);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_int(ipc_data, &pcalendar->store_type);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_char(ipc_data, &pcalendar->uid);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_long(ipc_data, &pcalendar->updated);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_long() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_char(ipc_data, &pcalendar->name);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_char(ipc_data, &pcalendar->description);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_char(ipc_data, &pcalendar->color);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_char(ipc_data, &pcalendar->location);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_int(ipc_data, &pcalendar->visibility);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_int(ipc_data, &pcalendar->sync_event);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_int(ipc_data, &pcalendar->account_id);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_char(ipc_data, &pcalendar->sync_data1);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_char(ipc_data, &pcalendar->sync_data2);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_char(ipc_data, &pcalendar->sync_data3);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_char(ipc_data, &pcalendar->sync_data4);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_int(ipc_data, &pcalendar->mode);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_ipc_marshal_calendar(const calendar_record_h record, pims_ipc_data_h ipc_data)
{
	int ret = 0;
	cal_calendar_s* pcalendar = (cal_calendar_s*)record;
	RETV_IF(NULL == ipc_data, CALENDAR_ERROR_NO_DATA);
	RETV_IF(NULL == pcalendar, CALENDAR_ERROR_NO_DATA);

	ret = cal_ipc_marshal_int((pcalendar->index), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_int((pcalendar->store_type), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_char((pcalendar->uid), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_long((pcalendar->updated), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_long() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_char((pcalendar->name), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_char((pcalendar->description), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_char((pcalendar->color), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_char((pcalendar->location), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_int((pcalendar->visibility), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_int((pcalendar->sync_event), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_int((pcalendar->account_id), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_char((pcalendar->sync_data1), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_char((pcalendar->sync_data2), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_char((pcalendar->sync_data3), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_char((pcalendar->sync_data4), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_int((pcalendar->mode), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_ipc_marshal_calendar_get_primary_id(const calendar_record_h record, unsigned int *property_id, int *id)
{
	*property_id = CAL_PROPERTY_CALENDAR_ID;
	return calendar_record_get_int(record, *property_id, id);
}
