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

static int _cal_ipc_unmarshal_todo(pims_ipc_data_h ipc_data, calendar_record_h record);
static int _cal_ipc_marshal_todo(const calendar_record_h record, pims_ipc_data_h ipc_data);
static int _cal_ipc_marshal_todo_get_primary_id(const calendar_record_h record, unsigned int *property_id, int *id);

cal_ipc_marshal_record_plugin_cb_s cal_ipc_record_todo_plugin_cb = {
	.unmarshal_record = _cal_ipc_unmarshal_todo,
	.marshal_record = _cal_ipc_marshal_todo,
	.get_primary_id = _cal_ipc_marshal_todo_get_primary_id
};

static int _cal_ipc_unmarshal_todo(pims_ipc_data_h ipc_data, calendar_record_h record)
{
	int ret = 0;
	cal_todo_s* ptodo = NULL;
	RETV_IF(NULL == ipc_data, CALENDAR_ERROR_NO_DATA);
	RETV_IF(NULL == record, CALENDAR_ERROR_NO_DATA);

	ptodo = (cal_todo_s*) record;

	ret = cal_ipc_unmarshal_int(ipc_data, &ptodo->index);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_int(ipc_data, &ptodo->calendar_id);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_char(ipc_data, &ptodo->summary);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_char(ipc_data, &ptodo->description);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_char(ipc_data, &ptodo->location);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_char(ipc_data, &ptodo->categories);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_int(ipc_data, (int*)&ptodo->todo_status);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_int(ipc_data, (int*)&ptodo->priority);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_int(ipc_data, &ptodo->sensitivity);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_char(ipc_data, &ptodo->uid);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_double(ipc_data, &ptodo->latitude);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_double() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_double(ipc_data, &ptodo->longitude);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_double() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_lli(ipc_data, &ptodo->created_time);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_lli() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_lli(ipc_data, &ptodo->completed_time);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_lli() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_int(ipc_data, &ptodo->progress);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_int(ipc_data, &ptodo->is_deleted);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_lli(ipc_data, &ptodo->last_mod);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_lli() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_int(ipc_data, &ptodo->freq);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_int(ipc_data, &ptodo->range_type);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_caltime(ipc_data, &ptodo->until);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_caltime() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_int(ipc_data, &ptodo->count);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_int(ipc_data, &ptodo->interval);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_char(ipc_data, &ptodo->bysecond);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_char(ipc_data, &ptodo->byminute);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_char(ipc_data, &ptodo->byhour);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_char(ipc_data, &ptodo->byday);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_char(ipc_data, &ptodo->bymonthday);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_char(ipc_data, &ptodo->byyearday);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_char(ipc_data, &ptodo->byweekno);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_char(ipc_data, &ptodo->bymonth);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_char(ipc_data, &ptodo->bysetpos);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_int(ipc_data, &ptodo->wkst);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_int(ipc_data, &ptodo->has_alarm);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_long(ipc_data, &ptodo->updated);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_long() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_char(ipc_data, &ptodo->sync_data1);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_char(ipc_data, &ptodo->sync_data2);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_char(ipc_data, &ptodo->sync_data3);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_char(ipc_data, &ptodo->sync_data4);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_caltime(ipc_data, &ptodo->start);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_caltime() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_char(ipc_data, &ptodo->start_tzid);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_caltime(ipc_data, &ptodo->due);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_caltime() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_char(ipc_data, &ptodo->due_tzid);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_char(ipc_data, &ptodo->organizer_name);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_char(ipc_data, &ptodo->organizer_email);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_int(ipc_data, &ptodo->has_attendee);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_int(ipc_data, &ptodo->is_allday);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	ret = cal_ipc_unmarshal_list(ipc_data, (calendar_list_h *)&ptodo->alarm_list);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_list() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_list(ipc_data, (calendar_list_h *)&ptodo->attendee_list);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_list() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_unmarshal_list(ipc_data, (calendar_list_h *)&ptodo->extended_list);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_list() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_ipc_marshal_todo(const calendar_record_h record, pims_ipc_data_h ipc_data)
{
	int ret = 0;
	cal_todo_s* ptodo = (cal_todo_s*)record;
	RETV_IF(NULL == ipc_data, CALENDAR_ERROR_NO_DATA);
	RETV_IF(NULL == ptodo, CALENDAR_ERROR_NO_DATA);

	ret = cal_ipc_marshal_int((ptodo->index), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_int((ptodo->calendar_id), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_char((ptodo->summary), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_char((ptodo->description), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_char((ptodo->location), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_char((ptodo->categories), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_int((ptodo->todo_status), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_int((ptodo->priority), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_int((ptodo->sensitivity), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_char((ptodo->uid), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_double((ptodo->latitude), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_double() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_double((ptodo->longitude), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_double() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_lli((ptodo->created_time), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_lli() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_lli((ptodo->completed_time), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_lli() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_int((ptodo->progress), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_int((ptodo->is_deleted), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_lli((ptodo->last_mod), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_lli() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_int((ptodo->freq), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_int((ptodo->range_type), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_caltime((ptodo->until), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_caltime() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_int((ptodo->count), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_int((ptodo->interval), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_char((ptodo->bysecond), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_char((ptodo->byminute), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_char((ptodo->byhour), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_char((ptodo->byday), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_char((ptodo->bymonthday), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_char((ptodo->byyearday), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_char((ptodo->byweekno), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_char((ptodo->bymonth), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_char((ptodo->bysetpos), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_int((ptodo->wkst), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_int((ptodo->has_alarm), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_long((ptodo->updated), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_long() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_char((ptodo->sync_data1), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_char((ptodo->sync_data2), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_char((ptodo->sync_data3), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_char((ptodo->sync_data4), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_caltime((ptodo->start), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_caltime() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_char((ptodo->start_tzid), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_caltime((ptodo->due), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_caltime() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_char((ptodo->due_tzid), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_char((ptodo->organizer_name), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_char((ptodo->organizer_email), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_char() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_int((ptodo->has_attendee), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_int((ptodo->is_allday), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	ret = cal_ipc_marshal_list((calendar_list_h)ptodo->alarm_list, ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_list() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_list((calendar_list_h)ptodo->attendee_list, ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_list() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	ret = cal_ipc_marshal_list((calendar_list_h)ptodo->extended_list, ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_list() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_ipc_marshal_todo_get_primary_id(const calendar_record_h record, unsigned int *property_id, int *id)
{
	*property_id = CAL_PROPERTY_TODO_ID;
	return calendar_record_get_int(record, *property_id, id);
}

