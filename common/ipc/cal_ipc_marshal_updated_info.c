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

static int __cal_ipc_unmarshal_updated_info(pims_ipc_data_h ipc_data, calendar_record_h record);
static int __cal_ipc_marshal_updated_info(const calendar_record_h record, pims_ipc_data_h ipc_data);

cal_ipc_marshal_record_plugin_cb_s _cal_ipc_record_updated_info_plugin_cb = {
	.unmarshal_record = __cal_ipc_unmarshal_updated_info,
	.marshal_record = __cal_ipc_marshal_updated_info,
	.get_primary_id = NULL
};

static int __cal_ipc_unmarshal_updated_info(pims_ipc_data_h ipc_data, calendar_record_h record)
{
	cal_updated_info_s* pupdatedinfo = NULL;
	retv_if(ipc_data==NULL,CALENDAR_ERROR_NO_DATA);
	retv_if(record==NULL,CALENDAR_ERROR_NO_DATA);

	pupdatedinfo = (cal_updated_info_s*) record;

	if (_cal_ipc_unmarshal_int(ipc_data,&pupdatedinfo->type) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (_cal_ipc_unmarshal_int(ipc_data,&pupdatedinfo->id) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (_cal_ipc_unmarshal_int(ipc_data,&pupdatedinfo->calendar_id) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (_cal_ipc_unmarshal_int(ipc_data,&pupdatedinfo->version) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int __cal_ipc_marshal_updated_info(const calendar_record_h record, pims_ipc_data_h ipc_data)
{
	cal_updated_info_s* pupdatedinfo = (cal_updated_info_s*) record;
	retv_if(ipc_data==NULL,CALENDAR_ERROR_NO_DATA);
	retv_if(pupdatedinfo==NULL,CALENDAR_ERROR_NO_DATA);

	if (_cal_ipc_marshal_int((pupdatedinfo->type),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_marshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (_cal_ipc_marshal_int((pupdatedinfo->id),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_marshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (_cal_ipc_marshal_int((pupdatedinfo->calendar_id),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_marshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (_cal_ipc_marshal_int((pupdatedinfo->version),ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("_cal_ipc_marshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}
