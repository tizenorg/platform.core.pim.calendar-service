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

static int _cal_ipc_unmarshal_instance_allday_extended(pims_ipc_data_h ipc_data, calendar_record_h record);
static int _cal_ipc_marshal_instance_allday_extended(const calendar_record_h record, pims_ipc_data_h ipc_data);

cal_ipc_marshal_record_plugin_cb_s cal_ipc_record_instance_allday_extended_plugin_cb = {
	.unmarshal_record = _cal_ipc_unmarshal_instance_allday_extended,
	.marshal_record = _cal_ipc_marshal_instance_allday_extended,
	.get_primary_id = NULL
};

static int _cal_ipc_unmarshal_instance_allday_extended(pims_ipc_data_h ipc_data, calendar_record_h record)
{
	int ret = 0;
	cal_instance_allday_extended_s* pinstanceallday = NULL;
	RETV_IF(NULL == ipc_data, CALENDAR_ERROR_NO_DATA);
	RETV_IF(NULL == record, CALENDAR_ERROR_NO_DATA);

	pinstanceallday = (cal_instance_allday_extended_s*) record;

	ret = cal_ipc_unmarshal_int(ipc_data, &pinstanceallday->event_id);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_int(ipc_data, &pinstanceallday->calendar_id);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_caltime(ipc_data, &pinstanceallday->start);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_caltime() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_caltime(ipc_data, &pinstanceallday->end);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_caltime() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_char(ipc_data, &pinstanceallday->summary);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_char(ipc_data, &pinstanceallday->description);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_char(ipc_data, &pinstanceallday->location);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_int(ipc_data, &pinstanceallday->busy_status);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_int(ipc_data, &pinstanceallday->event_status);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_int(ipc_data, &pinstanceallday->priority);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_int(ipc_data, &pinstanceallday->sensitivity);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_int(ipc_data, &pinstanceallday->has_rrule);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_double(ipc_data, &pinstanceallday->latitude);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_double() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_double(ipc_data, &pinstanceallday->longitude);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_double() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_int(ipc_data, &pinstanceallday->has_alarm);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_int(ipc_data, &pinstanceallday->original_event_id);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_lli(ipc_data, &pinstanceallday->last_mod);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_lli() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_char(ipc_data, &pinstanceallday->sync_data1);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_char(ipc_data, &pinstanceallday->organizer_name);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_char(ipc_data, &pinstanceallday->categories);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_int(ipc_data, &pinstanceallday->has_attendee);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_char(ipc_data, &pinstanceallday->sync_data2);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_char(ipc_data, &pinstanceallday->sync_data3);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_char(ipc_data, &pinstanceallday->sync_data4);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_int(ipc_data, &pinstanceallday->is_allday);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_ipc_marshal_instance_allday_extended(const calendar_record_h record, pims_ipc_data_h ipc_data)
{
	int ret = 0;
	cal_instance_allday_extended_s* pinstanceallday = (cal_instance_allday_extended_s*) record;
	RETV_IF(NULL == ipc_data, CALENDAR_ERROR_NO_DATA);
	RETV_IF(NULL == pinstanceallday, CALENDAR_ERROR_NO_DATA);

	ret = cal_ipc_marshal_int((pinstanceallday->event_id), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_int((pinstanceallday->calendar_id), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_caltime((pinstanceallday->start), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_caltime() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_caltime((pinstanceallday->end), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_caltime() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_char((pinstanceallday->summary), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_char((pinstanceallday->description), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_char((pinstanceallday->location), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_int((pinstanceallday->busy_status), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_int((pinstanceallday->event_status), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_int((pinstanceallday->priority), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_int((pinstanceallday->sensitivity), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_int((pinstanceallday->has_rrule), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_double((pinstanceallday->latitude), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_double() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_double((pinstanceallday->longitude), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_double() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_int((pinstanceallday->has_alarm), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_int((pinstanceallday->original_event_id), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_lli((pinstanceallday->last_mod), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_lli() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_char((pinstanceallday->sync_data1), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_char((pinstanceallday->organizer_name), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_char((pinstanceallday->categories), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_int((pinstanceallday->has_attendee), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_char((pinstanceallday->sync_data2), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_char((pinstanceallday->sync_data3), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_char((pinstanceallday->sync_data4), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_int((pinstanceallday->is_allday), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}
