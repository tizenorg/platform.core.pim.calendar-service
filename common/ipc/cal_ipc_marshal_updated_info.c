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

static int _cal_ipc_unmarshal_updated_info(pims_ipc_data_h ipc_data, calendar_record_h record);
static int _cal_ipc_marshal_updated_info(const calendar_record_h record, pims_ipc_data_h ipc_data);

cal_ipc_marshal_record_plugin_cb_s cal_ipc_record_updated_info_plugin_cb = {
	.unmarshal_record = _cal_ipc_unmarshal_updated_info,
	.marshal_record = _cal_ipc_marshal_updated_info
};

static int _cal_ipc_unmarshal_updated_info(pims_ipc_data_h ipc_data, calendar_record_h record)
{
	int ret = 0;
	cal_updated_info_s* pupdatedinfo = NULL;
	RETV_IF(NULL == ipc_data, CALENDAR_ERROR_NO_DATA);
	RETV_IF(NULL == record, CALENDAR_ERROR_NO_DATA);

	pupdatedinfo = (cal_updated_info_s*) record;

	ret = cal_ipc_unmarshal_int(ipc_data,&pupdatedinfo->type);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_int(ipc_data,&pupdatedinfo->id);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_int(ipc_data,&pupdatedinfo->calendar_id);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_int(ipc_data,&pupdatedinfo->version);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_ipc_marshal_updated_info(const calendar_record_h record, pims_ipc_data_h ipc_data)
{
	int ret = 0;
	cal_updated_info_s* pupdatedinfo = (cal_updated_info_s*) record;
	RETV_IF(NULL == ipc_data, CALENDAR_ERROR_NO_DATA);
	RETV_IF(NULL == pupdatedinfo, CALENDAR_ERROR_NO_DATA);

	ret = cal_ipc_marshal_int((pupdatedinfo->type),ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_int((pupdatedinfo->id),ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_int((pupdatedinfo->calendar_id),ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_int((pupdatedinfo->version),ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}
