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

#include <stdlib.h>
#include <unistd.h>

#include "calendar.h"
#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_inotify.h"
#include "cal_view.h"
#include "cal_record.h"
#include "cal_handle.h"

int cal_client_db_add_changed_cb(calendar_h handle, const char* view_uri, void *callback, void* user_data)
{
	CAL_FN_CALL();
	int ret;
	cal_record_type_e type = CAL_RECORD_TYPE_INVALID;

	RETV_IF(NULL == handle, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == view_uri, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == callback, CALENDAR_ERROR_INVALID_PARAMETER);

	type = cal_view_get_type(view_uri);
	switch (type) {
	case CAL_RECORD_TYPE_CALENDAR:
		ret = cal_inotify_subscribe(CAL_NOTI_TYPE_CALENDAR, CAL_NOTI_CALENDAR_CHANGED,
				callback, user_data);
		break;
	case CAL_RECORD_TYPE_EVENT:
		ret = cal_inotify_subscribe(CAL_NOTI_TYPE_EVENT, CAL_NOTI_EVENT_CHANGED,
				callback, user_data);
		break;
	case CAL_RECORD_TYPE_TODO:
		ret = cal_inotify_subscribe(CAL_NOTI_TYPE_TODO, CAL_NOTI_TODO_CHANGED,
				callback, user_data);
		break;
	default:
		ERR("Invalid view_uri(%s)", view_uri);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_inotify_subscribe() Fail(%d)", ret);
		return ret;
	}

	return CALENDAR_ERROR_NONE;
}

int cal_client_db_remove_changed_cb(calendar_h handle, const char* view_uri, calendar_db_changed_cb callback, void* user_data)
{
	CAL_FN_CALL();
	int ret;
	cal_record_type_e type = CAL_RECORD_TYPE_INVALID;

	RETV_IF(NULL == view_uri, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == callback, CALENDAR_ERROR_INVALID_PARAMETER);

	type = cal_view_get_type(view_uri);
	switch (type) {
	case CAL_RECORD_TYPE_CALENDAR:
		ret = cal_inotify_unsubscribe(CAL_NOTI_CALENDAR_CHANGED, callback, user_data);
		break;
	case CAL_RECORD_TYPE_EVENT:
		ret = cal_inotify_unsubscribe(CAL_NOTI_EVENT_CHANGED, callback, user_data);
		break;
	case CAL_RECORD_TYPE_TODO:
		ret = cal_inotify_unsubscribe(CAL_NOTI_TODO_CHANGED, callback, user_data);
		break;
	default:
		ERR("Invalid view_uri(%s)", view_uri);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_inotify_unsubscribe() Fail(%d)", ret);
		return ret;
	}

	return CALENDAR_ERROR_NONE;
}
