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

#include "calendar.h"
#include "cal_internal.h"
#include "cal_client_dbus.h"
#include "cal_client_handle.h"
#include "cal_client_db_helper.h"

API int calendar_db_insert_record(calendar_record_h record, int* id)
{
	CAL_FN_CALL();

	int ret = 0;
	calendar_h handle = NULL;
	RETV_IF(NULL == record, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == id, CALENDAR_ERROR_INVALID_PARAMETER);

	ret = cal_client_handle_get_p(&handle);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_client_handle_get_p() Fail(%d)", ret);
	return cal_dbus_insert_record(handle, record, id);
}

API int calendar_db_update_record(calendar_record_h record)
{
	CAL_FN_CALL();

	int ret = 0;
	calendar_h handle = NULL;
	RETV_IF(NULL == record, CALENDAR_ERROR_INVALID_PARAMETER);

	ret = cal_client_handle_get_p(&handle);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_client_handle_get_p() Fail(%d)", ret);
	return cal_dbus_update_record(handle, record);
}

API int calendar_db_delete_record(const char* view_uri, int id)
{
	CAL_FN_CALL();

	int ret = 0;
	calendar_h handle = NULL;
	RETV_IF(NULL == view_uri, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(id < 0, CALENDAR_ERROR_INVALID_PARAMETER);

	ret = cal_client_handle_get_p(&handle);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_client_handle_get_p() Fail(%d)", ret);
	return cal_dbus_delete_record(handle, view_uri, id);
}

API int calendar_db_replace_record(calendar_record_h record, int record_id)
{
	CAL_FN_CALL();

	int ret = 0;
	calendar_h handle = NULL;
	RETV_IF(NULL == record, CALENDAR_ERROR_INVALID_PARAMETER);

	ret = cal_client_handle_get_p(&handle);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_client_handle_get_p() Fail(%d)", ret);
	return cal_dbus_replace_record(handle, record, record_id);
}

API int calendar_db_get_record(const char* view_uri, int id, calendar_record_h* out_record)
{
	CAL_FN_CALL();

	int ret = 0;
	calendar_h handle = NULL;
	RETV_IF(NULL == view_uri, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(id < 0, CALENDAR_ERROR_INVALID_PARAMETER);

	ret = cal_client_handle_get_p(&handle);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_client_handle_get_p() Fail(%d)", ret);
	return cal_dbus_get_record(handle, view_uri, id, out_record);
}

API int calendar_db_get_all_records(const char* view_uri, int offset, int limit, calendar_list_h* out_list)
{
	CAL_FN_CALL();

	int ret = 0;
	calendar_h handle = NULL;
	RETV_IF(NULL == view_uri, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_list, CALENDAR_ERROR_INVALID_PARAMETER);

	ret = cal_client_handle_get_p(&handle);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_client_handle_get_p() Fail(%d)", ret);
	return cal_dbus_get_all_records(handle, view_uri, offset, limit, out_list);
}

API int calendar_db_get_records_with_query(calendar_query_h query, int offset, int limit, calendar_list_h* out_list)
{
	CAL_FN_CALL();

	int ret = 0;
	calendar_h handle = NULL;
	RETV_IF(NULL == query, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_list, CALENDAR_ERROR_INVALID_PARAMETER);

	ret = cal_client_handle_get_p(&handle);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_client_handle_get_p() Fail(%d)", ret);
	return cal_dbus_get_records_with_query(handle, query, offset, limit, out_list);
}

API int calendar_db_clean_after_sync(int book_id, int calendar_db_version)
{
	CAL_FN_CALL();

	int ret = 0;
	calendar_h handle = NULL;

	ret = cal_client_handle_get_p(&handle);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_client_handle_get_p() Fail(%d)", ret);
	return cal_dbus_clean_after_sync(handle, book_id, calendar_db_version);
}

API int calendar_db_get_count(const char *view_uri, int *out_count)
{
	CAL_FN_CALL();

	int ret = 0;
	calendar_h handle = NULL;
	RETV_IF(NULL == view_uri, CALENDAR_ERROR_INVALID_PARAMETER);

	ret = cal_client_handle_get_p(&handle);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_client_handle_get_p() Fail(%d)", ret);
	return cal_dbus_get_count(handle, view_uri, out_count);
}

API int calendar_db_get_count_with_query(calendar_query_h query, int *out_count)
{
	CAL_FN_CALL();

	int ret = 0;
	calendar_h handle = NULL;
	RETV_IF(NULL == query, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_count, CALENDAR_ERROR_INVALID_PARAMETER);

	ret = cal_client_handle_get_p(&handle);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_client_handle_get_p() Fail(%d)", ret);
	return cal_dbus_get_count_with_query(handle, query, out_count);
}

API int calendar_db_insert_records(calendar_list_h record_list, int** record_id_array, int* count)
{
	CAL_FN_CALL();

	int ret = 0;
	calendar_h handle = NULL;
	RETV_IF(NULL == record_list, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == record_id_array, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == count, CALENDAR_ERROR_INVALID_PARAMETER);

	int list_count = 0;
	calendar_list_get_count(record_list, &list_count);
	if (0 == list_count) {
		DBG("list count is 0");
		return CALENDAR_ERROR_NONE;
	}

	ret = cal_client_handle_get_p(&handle);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_client_handle_get_p() Fail(%d)", ret);
	return cal_dbus_insert_records(handle, record_list, record_id_array, count);
}

API int calendar_db_update_records(calendar_list_h record_list)
{
	CAL_FN_CALL();

	int ret = 0;
	calendar_h handle = NULL;
	RETV_IF(NULL == record_list, CALENDAR_ERROR_INVALID_PARAMETER);

	int list_count = 0;
	calendar_list_get_count(record_list, &list_count);
	if (0 == list_count) {
		DBG("list count is 0");
		return CALENDAR_ERROR_NONE;
	}

	ret = cal_client_handle_get_p(&handle);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_client_handle_get_p() Fail(%d)", ret);
	return cal_dbus_update_records(handle, record_list);
}

API int calendar_db_delete_records(const char* view_uri, int record_id_array[], int count)
{
	CAL_FN_CALL();

	int ret = 0;
	calendar_h handle = NULL;
	RETV_IF(NULL == view_uri, CALENDAR_ERROR_INVALID_PARAMETER);

	ret = cal_client_handle_get_p(&handle);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_client_handle_get_p() Fail(%d)", ret);
	return cal_dbus_delete_records(handle, view_uri, record_id_array, count);
}

API int calendar_db_replace_records(calendar_list_h record_list, int *record_id_array, int count)
{
	CAL_FN_CALL();

	int ret = 0;
	calendar_h handle = NULL;
	RETV_IF(NULL == record_list, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == record_id_array, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(count < 1, CALENDAR_ERROR_INVALID_PARAMETER);

	int list_count = 0;
	calendar_list_get_count(record_list, &list_count);
	if (0 == list_count) {
		DBG("list count is 0");
		return CALENDAR_ERROR_NONE;
	}

	ret = cal_client_handle_get_p(&handle);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_client_handle_get_p() Fail(%d)", ret);
	return cal_dbus_replace_records(handle, record_list, record_id_array, count);
}

API int calendar_db_get_changes_by_version(const char* view_uri, int book_id,
		int calendar_db_version, calendar_list_h* record_list, int* current_version)
{
	CAL_FN_CALL();

	int ret = 0;
	calendar_h handle = NULL;
	RETV_IF(NULL == view_uri, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == record_list, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == current_version, CALENDAR_ERROR_INVALID_PARAMETER);

	ret = cal_client_handle_get_p(&handle);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_client_handle_get_p() Fail(%d)", ret);
	return cal_dbus_get_changes_by_version(handle, view_uri, book_id,
			calendar_db_version, record_list, current_version);
}

API int calendar_db_get_current_version(int* current_version)
{
	CAL_FN_CALL();

	int ret = 0;
	calendar_h handle = NULL;
	RETV_IF(NULL == current_version, CALENDAR_ERROR_INVALID_PARAMETER);

	ret = cal_client_handle_get_p(&handle);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_client_handle_get_p() Fail(%d)", ret);
	return cal_dbus_get_current_version(handle, current_version);
}

API int calendar_db_add_changed_cb(const char* view_uri, calendar_db_changed_cb callback, void* user_data)
{
	CAL_FN_CALL();

	int ret = 0;
	calendar_h handle = NULL;
	RETV_IF(NULL == view_uri, CALENDAR_ERROR_INVALID_PARAMETER);

	ret = cal_client_handle_get_p(&handle);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_client_handle_get_p() Fail(%d)", ret);
	return cal_dbus_add_changed_cb(handle, view_uri, callback, user_data);
}

API int calendar_db_remove_changed_cb(const char* view_uri, calendar_db_changed_cb callback, void* user_data)
{
	CAL_FN_CALL();

	int ret = 0;
	calendar_h handle = NULL;
	RETV_IF(NULL == view_uri, CALENDAR_ERROR_INVALID_PARAMETER);

	ret = cal_client_handle_get_p(&handle);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_client_handle_get_p() Fail(%d)", ret);
	return cal_dbus_remove_changed_cb(handle, view_uri, callback, user_data);
}

API int calendar_db_insert_vcalendars(const char* vcalendar_stream, int **record_id_array, int *count)
{
	CAL_FN_CALL();

	int ret = 0;
	calendar_h handle = NULL;
	RETV_IF(NULL == vcalendar_stream, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == record_id_array, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == count, CALENDAR_ERROR_INVALID_PARAMETER);

	ret = cal_client_handle_get_p(&handle);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_client_handle_get_p() Fail(%d)", ret);
	return cal_dbus_insert_vcalendars(handle, vcalendar_stream, record_id_array, count);
}

API int calendar_db_replace_vcalendars(const char* vcalendar_stream, int *record_id_array, int count)
{
	CAL_FN_CALL();

	int ret = 0;
	calendar_h handle = NULL;
	RETV_IF(NULL == vcalendar_stream, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == record_id_array, CALENDAR_ERROR_INVALID_PARAMETER);

	ret = cal_client_handle_get_p(&handle);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_client_handle_get_p() Fail(%d)", ret);
	return cal_dbus_replace_vcalendars(handle, vcalendar_stream, record_id_array, count);
}

API int calendar_db_get_last_change_version(int* last_version)
{
	CAL_FN_CALL();

	int ret = 0;
	calendar_h handle = NULL;
	RETV_IF(NULL == last_version, CALENDAR_ERROR_INVALID_PARAMETER);

	ret = cal_client_handle_get_p(&handle);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_client_handle_get_p() Fail(%d)", ret);
	return cal_dbus_get_last_change_version(handle, last_version);
}

API int calendar_db_get_changes_exception_by_version(const char* view_uri,
		int original_event_id, int calendar_db_version, calendar_list_h* record_list)
{
	CAL_FN_CALL();

	int ret = 0;
	calendar_h handle = NULL;
	RETV_IF(NULL == view_uri, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(original_event_id < 0, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(calendar_db_version < 0, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == record_list, CALENDAR_ERROR_INVALID_PARAMETER);

	ret = cal_client_handle_get_p(&handle);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_client_handle_get_p() Fail(%d)", ret);
	return cal_dbus_get_changes_exception_by_version(handle, view_uri, original_event_id, calendar_db_version, record_list);
}

