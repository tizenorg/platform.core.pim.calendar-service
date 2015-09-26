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


#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_view.h"
#include "cal_record.h"

#define __CHECK_READ_ONLY_PROPERTY() \
	if (CAL_PROPERTY_CHECK_FLAGS(property_id, CAL_PROPERTY_FLAGS_READ_ONLY) == true) \
{ \
	ERR("Invalid parameter: Don't try to change read-only property."); \
	return CALENDAR_ERROR_NOT_PERMITTED; \
}

extern cal_record_plugin_cb_s cal_record_calendar_plugin_cb;
extern cal_record_plugin_cb_s cal_record_event_plugin_cb;
extern cal_record_plugin_cb_s cal_record_todo_plugin_cb;
extern cal_record_plugin_cb_s cal_record_alarm_plugin_cb;
extern cal_record_plugin_cb_s cal_record_attendee_plugin_cb;
extern cal_record_plugin_cb_s cal_record_timezone_plugin_cb;
extern cal_record_plugin_cb_s cal_record_updated_info_plugin_cb;
extern cal_record_plugin_cb_s cal_record_instance_normal_plugin_cb;
extern cal_record_plugin_cb_s cal_record_instance_allday_plugin_cb;
extern cal_record_plugin_cb_s cal_record_instance_normal_extended_plugin_cb;
extern cal_record_plugin_cb_s cal_record_instance_allday_extended_plugin_cb;
extern cal_record_plugin_cb_s cal_record_search_plugin_cb;
extern cal_record_plugin_cb_s cal_record_extended_plugin_cb;

cal_record_plugin_cb_s* cal_record_get_plugin_cb(cal_record_type_e type)
{
	switch (type) {
	case CAL_RECORD_TYPE_CALENDAR:
		return (&cal_record_calendar_plugin_cb);
	case CAL_RECORD_TYPE_EVENT:
		return (&cal_record_event_plugin_cb);
	case CAL_RECORD_TYPE_TODO:
		return (&cal_record_todo_plugin_cb);
	case CAL_RECORD_TYPE_ALARM:
		return (&cal_record_alarm_plugin_cb);
	case CAL_RECORD_TYPE_ATTENDEE:
		return (&cal_record_attendee_plugin_cb);
	case CAL_RECORD_TYPE_TIMEZONE:
		return (&cal_record_timezone_plugin_cb);
	case CAL_RECORD_TYPE_INSTANCE_NORMAL:
		return (&cal_record_instance_normal_plugin_cb);
	case CAL_RECORD_TYPE_INSTANCE_ALLDAY:
		return (&cal_record_instance_allday_plugin_cb);
	case CAL_RECORD_TYPE_INSTANCE_NORMAL_EXTENDED:
		return (&cal_record_instance_normal_extended_plugin_cb);
	case CAL_RECORD_TYPE_INSTANCE_ALLDAY_EXTENDED:
		return (&cal_record_instance_allday_extended_plugin_cb);
	case CAL_RECORD_TYPE_UPDATED_INFO:
		return (&cal_record_updated_info_plugin_cb);
	case CAL_RECORD_TYPE_SEARCH:
		return (&cal_record_search_plugin_cb);
	case CAL_RECORD_TYPE_EXTENDED:
		return (&cal_record_extended_plugin_cb);
	default:
		return NULL;
	}
}

static inline void _cal_record_set_property_flag(calendar_record_h record, unsigned int property_id, cal_properties_flag_e flag)
{
	int index;
	cal_record_s *_record = NULL;

	_record = (cal_record_s *)record;
	index = property_id & 0x00000FFF;

	if (NULL == _record->properties_flags) {
		int count = 0;
		cal_view_get_property_info(_record->view_uri,&count);

		if (0 < count) {
			_record->properties_flags = calloc(count, sizeof(char));
			_record->properties_max_count = count;
			if (NULL == _record->properties_flags) {
				ERR("calloc() Fail");
				return ;
			}
		}
		else {
			ERR("get property_info_Fail");
			return ;
		}
	}

	_record->properties_flags[index] |= flag;
	_record->property_flag |= flag;
}

bool cal_record_check_property_flag(calendar_record_h record, unsigned int property_id, cal_properties_flag_e flag)
{
	int index;
	cal_record_s *_record = NULL;

	_record = (cal_record_s *)record;
	index = property_id & 0x00000FFF;

	if (NULL == _record->properties_flags) {
		if (flag == CAL_PROPERTY_FLAG_PROJECTION)
			return true;
		else
			return false;
	}

	if (flag == CAL_PROPERTY_FLAG_PROJECTION) {
		if (_record->property_flag & CAL_PROPERTY_FLAG_PROJECTION) {
			if (_record->properties_flags[index] & CAL_PROPERTY_FLAG_PROJECTION)
				return true;
			else
				return false;
		}

		return true;
	}

	return (_record->properties_flags[index] & flag) ? true : false;

}

int cal_record_set_projection(calendar_record_h record, const unsigned int *projection, const int projection_count, int properties_max_count)
{
	int i;

	cal_record_s *_record = NULL;

	RETV_IF(NULL == record, -1);

	_record = (cal_record_s *)record;

	CAL_FREE(_record->properties_flags);
	_record->properties_flags  = calloc(properties_max_count, sizeof(char));

	RETVM_IF(NULL == _record->properties_flags, CALENDAR_ERROR_OUT_OF_MEMORY, "calloc() Fail");

	_record->properties_max_count = properties_max_count;

	for (i = 0; i < projection_count; i++) {
		_cal_record_set_property_flag(record, projection[i], CAL_PROPERTY_FLAG_PROJECTION);
	}

	return CALENDAR_ERROR_NONE;
}

API int calendar_record_create(const char* view_uri, calendar_record_h* out_record)
{
	int ret = CALENDAR_ERROR_NONE;
	cal_record_type_e type = CAL_RECORD_TYPE_INVALID;

	RETV_IF(NULL == view_uri, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_record, CALENDAR_ERROR_INVALID_PARAMETER);

	type =cal_view_get_type(view_uri);
	DBG("----------------------[%s]", view_uri);
	RETV_IF(CAL_RECORD_TYPE_INVALID == type, CALENDAR_ERROR_INVALID_PARAMETER);

	cal_record_plugin_cb_s *plugin_cb = cal_record_get_plugin_cb(type);
	RETV_IF(NULL == plugin_cb, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(NULL == plugin_cb->create, CALENDAR_ERROR_NOT_PERMITTED, "Not permitted in [%s]", view_uri);

	ret = plugin_cb->create(out_record);

	if (CALENDAR_ERROR_NONE == ret) {
		CAL_RECORD_INIT_COMMON((cal_record_s*)*out_record, type, plugin_cb, cal_view_get_uri(view_uri));
	}

	return ret;
}

API int calendar_record_destroy(calendar_record_h record, bool delete_child)
{
	int ret = CALENDAR_ERROR_NONE;

	RETV_IF(NULL == record, CALENDAR_ERROR_INVALID_PARAMETER);

	cal_record_s *temp = (cal_record_s*)(record);

	RETV_IF(NULL == temp->plugin_cb, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(NULL == temp->plugin_cb->destroy, CALENDAR_ERROR_NOT_PERMITTED, "Not permitted in [%s]", temp->view_uri);

	CAL_FREE(temp->properties_flags);

	ret = temp->plugin_cb->destroy(record, delete_child);

	return ret;
}

API int calendar_record_clone(calendar_record_h record, calendar_record_h* out_record)
{
	int ret = CALENDAR_ERROR_NONE;

	cal_record_s *temp = (cal_record_s*)(record);

	RETV_IF(NULL == record, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == temp->plugin_cb, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_record, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(NULL == temp->plugin_cb->clone, CALENDAR_ERROR_NOT_PERMITTED, "Not permitted in [%s]", temp->view_uri);

	ret = temp->plugin_cb->clone(record, out_record);

	return ret;
}

API int calendar_record_get_uri_p(calendar_record_h record, char** out_str)
{
	int ret = CALENDAR_ERROR_NONE;

	cal_record_s *temp = (cal_record_s*)(record);

	RETV_IF(NULL == record, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_str, CALENDAR_ERROR_INVALID_PARAMETER);

	*out_str = (char*)(temp->view_uri);

	return ret;
}

API int calendar_record_get_str(calendar_record_h record, unsigned int property_id, char** out_str)
{
	int ret = CALENDAR_ERROR_NONE;

	cal_record_s *temp = (cal_record_s*)(record);

	RETV_IF(NULL == record, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == temp->plugin_cb, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_str, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(NULL == temp->plugin_cb->get_str, CALENDAR_ERROR_NOT_PERMITTED, "Not permitted(%x) in [%s]", property_id, temp->view_uri);
	RETVM_IF(false == cal_record_check_property_flag(record, property_id, CAL_PROPERTY_FLAG_PROJECTION), CALENDAR_ERROR_NOT_PERMITTED, "Not permitted");

	ret = temp->plugin_cb->get_str(record, property_id, out_str);

	return ret;
}

API int calendar_record_get_str_p(calendar_record_h record, unsigned int property_id, char** out_str)
{
	int ret = CALENDAR_ERROR_NONE;

	cal_record_s *temp = (cal_record_s*)(record);

	RETV_IF(NULL == record, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == temp->plugin_cb, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_str, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(NULL == temp->plugin_cb->get_str_p, CALENDAR_ERROR_NOT_PERMITTED, "Not permitted(%x) in [%s]", property_id, temp->view_uri);
	RETVM_IF(false == cal_record_check_property_flag(record, property_id, CAL_PROPERTY_FLAG_PROJECTION), CALENDAR_ERROR_NOT_PERMITTED, "Not permitted");

	ret = temp->plugin_cb->get_str_p(record, property_id, out_str);

	return ret;
}
API int calendar_record_get_int(calendar_record_h record, unsigned int property_id, int* out_value)
{
	int ret = CALENDAR_ERROR_NONE;

	cal_record_s *temp = (cal_record_s*)(record);

	RETV_IF(NULL == record, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == temp->plugin_cb, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_value, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(NULL == temp->plugin_cb->get_int, CALENDAR_ERROR_NOT_PERMITTED, "Not permitted(%x) in [%s]", property_id, temp->view_uri);
	RETVM_IF(false == cal_record_check_property_flag(record, property_id, CAL_PROPERTY_FLAG_PROJECTION), CALENDAR_ERROR_NOT_PERMITTED, "Not permitted");

	ret = temp->plugin_cb->get_int(record, property_id, out_value);

	return ret;
}

API int calendar_record_get_double(calendar_record_h record, unsigned int property_id, double* out_value)
{
	int ret = CALENDAR_ERROR_NONE;

	cal_record_s *temp = (cal_record_s*)(record);

	RETV_IF(NULL == record, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == temp->plugin_cb, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_value, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(NULL == temp->plugin_cb->get_double, CALENDAR_ERROR_NOT_PERMITTED, "Not permitted(%x) in [%s]", property_id, temp->view_uri);
	RETVM_IF(false == cal_record_check_property_flag(record, property_id, CAL_PROPERTY_FLAG_PROJECTION), CALENDAR_ERROR_NOT_PERMITTED, "Not permitted");

	ret = temp->plugin_cb->get_double(record, property_id, out_value);

	return ret;
}
API int calendar_record_get_lli(calendar_record_h record, unsigned int property_id, long long int* out_value)
{
	int ret = CALENDAR_ERROR_NONE;

	cal_record_s *temp = (cal_record_s*)(record);

	RETV_IF(NULL == record, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == temp->plugin_cb, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_value, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(NULL == temp->plugin_cb->get_lli, CALENDAR_ERROR_NOT_PERMITTED, "Not permitted(%x) in [%s]", property_id, temp->view_uri);
	RETVM_IF(false == cal_record_check_property_flag(record, property_id, CAL_PROPERTY_FLAG_PROJECTION), CALENDAR_ERROR_NOT_PERMITTED, "Not permitted");

	ret = temp->plugin_cb->get_lli(record, property_id, out_value);

	return ret;
}

API int calendar_record_get_caltime(calendar_record_h record, unsigned int property_id, calendar_time_s *out_value)
{
	int ret = CALENDAR_ERROR_NONE;

	cal_record_s *temp = (cal_record_s*)(record);

	RETV_IF(NULL == record, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == temp->plugin_cb, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_value, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(NULL == temp->plugin_cb->get_caltime, CALENDAR_ERROR_NOT_PERMITTED, "Not permitted(%x) in [%s]", property_id, temp->view_uri);
	RETVM_IF(false == cal_record_check_property_flag(record, property_id, CAL_PROPERTY_FLAG_PROJECTION), CALENDAR_ERROR_NOT_PERMITTED, "Not permitted");

	ret = temp->plugin_cb->get_caltime(record, property_id, out_value);

	return ret;
}

API int calendar_record_set_str(calendar_record_h record, unsigned int property_id, const char* value)
{
	RETV_IF(NULL == record, CALENDAR_ERROR_INVALID_PARAMETER);
	__CHECK_READ_ONLY_PROPERTY();

	int ret = cal_record_set_str(record, property_id, value);

	return ret;
}

API int calendar_record_set_int(calendar_record_h record, unsigned int property_id, int value)
{
	RETV_IF(NULL == record, CALENDAR_ERROR_INVALID_PARAMETER);
	__CHECK_READ_ONLY_PROPERTY();

	int ret = cal_record_set_int(record, property_id, value);

	return ret;
}

API int calendar_record_set_double(calendar_record_h record, unsigned int property_id, double value)
{
	RETV_IF(NULL == record, CALENDAR_ERROR_INVALID_PARAMETER);
	__CHECK_READ_ONLY_PROPERTY();

	int ret = cal_record_set_double(record, property_id, value);

	return ret;
}

API int calendar_record_set_lli(calendar_record_h record, unsigned int property_id, long long int value)
{
	RETV_IF(NULL == record, CALENDAR_ERROR_INVALID_PARAMETER);
	__CHECK_READ_ONLY_PROPERTY();

	int ret = cal_record_set_lli(record, property_id, value);

	return ret;
}

API int calendar_record_set_caltime(calendar_record_h record, unsigned int property_id, calendar_time_s value)
{
	RETV_IF(NULL == record, CALENDAR_ERROR_INVALID_PARAMETER);
	__CHECK_READ_ONLY_PROPERTY();

	int ret = cal_record_set_caltime(record, property_id, value);

	return ret;
}

API int calendar_record_add_child_record(calendar_record_h record, unsigned int property_id, calendar_record_h child_record)
{
	int ret = CALENDAR_ERROR_NONE;

	cal_record_s *temp = (cal_record_s*)(record);

	RETV_IF(NULL == record, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == temp->plugin_cb, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == child_record, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(NULL == temp->plugin_cb->add_child_record, CALENDAR_ERROR_NOT_PERMITTED, "Not permitted(%x) in [%s]", property_id, temp->view_uri);
	RETVM_IF(false == cal_record_check_property_flag(record, property_id, CAL_PROPERTY_FLAG_PROJECTION), CALENDAR_ERROR_NOT_PERMITTED, "Not permitted");

	ret = temp->plugin_cb->add_child_record(record, property_id, child_record);

	return ret;
}
API int calendar_record_remove_child_record(calendar_record_h record, unsigned int property_id, calendar_record_h child_record)
{
	int ret = CALENDAR_ERROR_NONE;

	cal_record_s *temp = (cal_record_s*)(record);

	RETV_IF(NULL == record, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == temp->plugin_cb, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == child_record, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(NULL == temp->plugin_cb->remove_child_record, CALENDAR_ERROR_NOT_PERMITTED, "Not permitted(%x) in [%s]", property_id, temp->view_uri);
	RETVM_IF(false == cal_record_check_property_flag(record, property_id, CAL_PROPERTY_FLAG_PROJECTION), CALENDAR_ERROR_NOT_PERMITTED, "Not permitted");

	ret = temp->plugin_cb->remove_child_record(record, property_id, child_record);

	return ret;
}

API int calendar_record_get_child_record_count(calendar_record_h record, unsigned int property_id, unsigned int* count)
{
	int ret = CALENDAR_ERROR_NONE;

	cal_record_s *temp = (cal_record_s*)(record);

	RETV_IF(NULL == record, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == temp->plugin_cb, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == count, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(NULL == temp->plugin_cb->get_child_record_count, CALENDAR_ERROR_NOT_PERMITTED, "Not permitted(%x) in [%s]", property_id, temp->view_uri);
	RETVM_IF(false == cal_record_check_property_flag(record, property_id, CAL_PROPERTY_FLAG_PROJECTION), CALENDAR_ERROR_NOT_PERMITTED, "Not permitted");

	ret = temp->plugin_cb->get_child_record_count(record, property_id, count);

	return ret;
}
API int calendar_record_get_child_record_at_p(calendar_record_h record, unsigned int property_id, int index, calendar_record_h* child_record)
{
	int ret = CALENDAR_ERROR_NONE;

	cal_record_s *temp = (cal_record_s*)(record);

	RETV_IF(NULL == record, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == temp->plugin_cb, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == child_record, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(NULL == temp->plugin_cb->get_child_record_at_p, CALENDAR_ERROR_NOT_PERMITTED, "Not permitted(%x) in [%s]", property_id, temp->view_uri);
	RETVM_IF(false == cal_record_check_property_flag(record, property_id, CAL_PROPERTY_FLAG_PROJECTION), CALENDAR_ERROR_NOT_PERMITTED, "Not permitted");

	ret = temp->plugin_cb->get_child_record_at_p(record, property_id, index, child_record);

	return ret;
}

API int calendar_record_clone_child_record_list(calendar_record_h record, unsigned int property_id, calendar_list_h* out_list)
{
	int ret = CALENDAR_ERROR_NONE;

	cal_record_s *temp = (cal_record_s*)(record);

	RETV_IF(NULL == record, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == temp->plugin_cb, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_list, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(NULL == temp->plugin_cb->clone_child_record_list, CALENDAR_ERROR_NOT_PERMITTED, "Not permitted(%x) in [%s]", property_id, temp->view_uri);
	RETVM_IF(false == cal_record_check_property_flag(record, property_id, CAL_PROPERTY_FLAG_PROJECTION), CALENDAR_ERROR_NOT_PERMITTED, "Not permitted");

	ret = temp->plugin_cb->clone_child_record_list(record, property_id, out_list);

	return ret;
}

int cal_record_set_str(calendar_record_h record, unsigned int property_id, const char* value)
{
	int ret = CALENDAR_ERROR_NONE;

	cal_record_s *temp = (cal_record_s*)(record);

	RETV_IF(NULL == record, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == temp->plugin_cb, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(NULL == temp->plugin_cb->set_str, CALENDAR_ERROR_NOT_PERMITTED, "Not permitted(%x) in [%s]", property_id, temp->view_uri);
	RETVM_IF(false == cal_record_check_property_flag(record, property_id, CAL_PROPERTY_FLAG_PROJECTION), CALENDAR_ERROR_NOT_PERMITTED, "Not permitted");

	ret = temp->plugin_cb->set_str(record, property_id, value);
	if (CALENDAR_ERROR_NONE == ret) {
		_cal_record_set_property_flag(record,property_id, CAL_PROPERTY_FLAG_DIRTY);
	}
	return ret;
}

int cal_record_set_int(calendar_record_h record, unsigned int property_id, int value)
{
	int ret = CALENDAR_ERROR_NONE;

	cal_record_s *temp = (cal_record_s*)(record);

	RETV_IF(NULL == record, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == temp->plugin_cb, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(NULL == temp->plugin_cb->set_int, CALENDAR_ERROR_NOT_PERMITTED, "Not permitted(%x) in [%s]", property_id, temp->view_uri);
	RETVM_IF(false == cal_record_check_property_flag(record, property_id, CAL_PROPERTY_FLAG_PROJECTION), CALENDAR_ERROR_NOT_PERMITTED, "Not permitted");

	ret = temp->plugin_cb->set_int(record, property_id, value);
	if (CALENDAR_ERROR_NONE == ret) {
		_cal_record_set_property_flag(record,property_id, CAL_PROPERTY_FLAG_DIRTY);
	}
	return ret;
}

int cal_record_set_double(calendar_record_h record, unsigned int property_id, double value)
{
	int ret = CALENDAR_ERROR_NONE;

	cal_record_s *temp = (cal_record_s*)(record);

	RETV_IF(NULL == record, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == temp->plugin_cb, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(NULL == temp->plugin_cb->set_double, CALENDAR_ERROR_NOT_PERMITTED, "Not permitted(%x) in [%s]", property_id, temp->view_uri);
	RETVM_IF(false == cal_record_check_property_flag(record, property_id, CAL_PROPERTY_FLAG_PROJECTION), CALENDAR_ERROR_NOT_PERMITTED, "Not permitted");

	ret = temp->plugin_cb->set_double(record, property_id, value);
	if (CALENDAR_ERROR_NONE == ret) {
		_cal_record_set_property_flag(record,property_id, CAL_PROPERTY_FLAG_DIRTY);
	}
	return ret;
}

int cal_record_set_lli(calendar_record_h record, unsigned int property_id, long long int value)
{
	int ret = CALENDAR_ERROR_NONE;

	cal_record_s *temp = (cal_record_s*)(record);

	RETV_IF(NULL == record, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == temp->plugin_cb, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(NULL == temp->plugin_cb->set_lli, CALENDAR_ERROR_NOT_PERMITTED, "Not permitted(%x) in [%s]", property_id, temp->view_uri);
	RETVM_IF(false == cal_record_check_property_flag(record, property_id, CAL_PROPERTY_FLAG_PROJECTION), CALENDAR_ERROR_NOT_PERMITTED, "Not permitted");

	ret = temp->plugin_cb->set_lli(record, property_id, value);
	if (CALENDAR_ERROR_NONE == ret) {
		_cal_record_set_property_flag(record,property_id, CAL_PROPERTY_FLAG_DIRTY);
	}
	return ret;
}

int cal_record_set_caltime(calendar_record_h record, unsigned int property_id, calendar_time_s value)
{
	int ret = CALENDAR_ERROR_NONE;

	cal_record_s *temp = (cal_record_s*)(record);

	RETV_IF(NULL == record, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == temp->plugin_cb, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(NULL == temp->plugin_cb->set_caltime, CALENDAR_ERROR_NOT_PERMITTED, "Not permitted(%x) in [%s]", property_id, temp->view_uri);
	RETVM_IF(false == cal_record_check_property_flag(record, property_id, CAL_PROPERTY_FLAG_PROJECTION), CALENDAR_ERROR_NOT_PERMITTED, "Not permitted");

	ret = temp->plugin_cb->set_caltime(record, property_id, value);
	if (CALENDAR_ERROR_NONE == ret) {
		_cal_record_set_property_flag(record,property_id, CAL_PROPERTY_FLAG_DIRTY);
	}
	return ret;
}
