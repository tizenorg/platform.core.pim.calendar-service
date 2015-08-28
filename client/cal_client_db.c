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
#include "cal_client_handle.h"
#include "cal_client_db_helper.h"

API int calendar_db_insert_record(calendar_record_h record, int* id)
{
	int ret = 0;
	calendar_h handle = NULL;

	ret = cal_client_handle_get_p(&handle);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_client_handle_get_p() Fail(%d)", ret);
	ret = cal_client_db_insert_record(handle, record, id);
	return ret;
}

API int calendar_db_update_record(calendar_record_h record)
{
	int ret = 0;
	calendar_h handle = NULL;

	ret = cal_client_handle_get_p(&handle);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_client_handle_get_p() Fail(%d)", ret);
	ret = cal_client_db_update_record(handle, record);
	return ret;
}

API int calendar_db_delete_record(const char* view_uri, int id)
{
	int ret = 0;
	calendar_h handle = NULL;

	ret = cal_client_handle_get_p(&handle);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_client_handle_get_p() Fail(%d)", ret);
	ret = cal_client_db_delete_record(handle, view_uri, id);
	return ret;
}

API int calendar_db_replace_record(calendar_record_h record, int record_id)
{
	int ret = 0;
	calendar_h handle = NULL;

	ret = cal_client_handle_get_p(&handle);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_client_handle_get_p() Fail(%d)", ret);
	ret = cal_client_db_replace_record(handle, record, record_id);
	return ret;
}

API int calendar_db_get_record(const char* view_uri, int id, calendar_record_h* out_record)
{
	int ret = 0;
	calendar_h handle = NULL;

	ret = cal_client_handle_get_p(&handle);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_client_handle_get_p() Fail(%d)", ret);
	ret = cal_client_db_get_record(handle, view_uri, id, out_record);
	return ret;
}

API int calendar_db_get_all_records(const char* view_uri, int offset, int limit, calendar_list_h* out_list)
{
	int ret = 0;
	calendar_h handle = NULL;

	ret = cal_client_handle_get_p(&handle);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_client_handle_get_p() Fail(%d)", ret);
	ret = cal_client_db_get_all_records(handle, view_uri, offset, limit, out_list);
	return ret;
}

API int calendar_db_get_records_with_query(calendar_query_h query, int offset, int limit, calendar_list_h* out_list)
{
	int ret = 0;
	calendar_h handle = NULL;

	ret = cal_client_handle_get_p(&handle);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_client_handle_get_p() Fail(%d)", ret);
	ret = cal_client_db_get_records_with_query(handle, query, offset, limit, out_list);
	return ret;
}

API int calendar_db_clean_after_sync(int calendar_book_id, int calendar_db_version)
{
	int ret = 0;
	calendar_h handle = NULL;

	ret = cal_client_handle_get_p(&handle);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_client_handle_get_p() Fail(%d)", ret);
	ret = cal_client_db_clean_after_sync(handle, calendar_book_id, calendar_db_version);
	return ret;
}

API int calendar_db_get_count(const char *view_uri, int *out_count)
{
	int ret = 0;
	calendar_h handle = NULL;

	ret = cal_client_handle_get_p(&handle);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_client_handle_get_p() Fail(%d)", ret);
	ret = cal_client_db_get_count(handle, view_uri, out_count);
	return ret;
}

API int calendar_db_get_count_with_query(calendar_query_h query, int *out_count)
{
	int ret = 0;
	calendar_h handle = NULL;

	ret = cal_client_handle_get_p(&handle);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_client_handle_get_p() Fail(%d)", ret);
	ret = cal_client_db_get_count_with_query(handle, query, out_count);
	return ret;
}

API int calendar_db_insert_records(calendar_list_h record_list, int** record_id_array, int* count)
{
	int ret = 0;
	calendar_h handle = NULL;

	ret = cal_client_handle_get_p(&handle);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_client_handle_get_p() Fail(%d)", ret);
	ret = cal_client_db_insert_records(handle, record_list, record_id_array, count);
	return ret;
}

API int calendar_db_update_records(calendar_list_h record_list)
{
	int ret = 0;
	calendar_h handle = NULL;

	ret = cal_client_handle_get_p(&handle);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_client_handle_get_p() Fail(%d)", ret);
	ret = cal_client_db_update_records(handle, record_list);
	return ret;
}

API int calendar_db_delete_records(const char* view_uri, int record_id_array[], int count)
{
	int ret = 0;
	calendar_h handle = NULL;

	ret = cal_client_handle_get_p(&handle);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_client_handle_get_p() Fail(%d)", ret);
	ret = cal_client_db_delete_records(handle, view_uri, record_id_array, count);
	return ret;
}

API int calendar_db_replace_records(calendar_list_h record_list, int *record_id_array, int count)
{
	int ret = 0;
	calendar_h handle = NULL;

	ret = cal_client_handle_get_p(&handle);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_client_handle_get_p() Fail(%d)", ret);
	ret = cal_client_db_replace_records(handle, record_list, record_id_array, count);
	return ret;
}

API int calendar_db_get_changes_by_version(const char* view_uri, int calendar_book_id, int calendar_db_version, calendar_list_h* record_list, int* current_calendar_db_version)
{
	int ret = 0;
	calendar_h handle = NULL;

	ret = cal_client_handle_get_p(&handle);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_client_handle_get_p() Fail(%d)", ret);
	ret = cal_client_db_get_changes_by_version(handle, view_uri, calendar_book_id, calendar_db_version, record_list, current_calendar_db_version);
	return ret;
}

API int calendar_db_get_current_version(int* calendar_db_version)
{
	int ret = 0;
	calendar_h handle = NULL;

	ret = cal_client_handle_get_p(&handle);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_client_handle_get_p() Fail(%d)", ret);
	ret = cal_client_db_get_current_version(handle, calendar_db_version);
	return ret;
}

API int calendar_db_add_changed_cb(const char* view_uri, calendar_db_changed_cb callback, void* user_data)
{
	int ret = 0;
	calendar_h handle = NULL;

	ret = cal_client_handle_get_p(&handle);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_client_handle_get_p() Fail(%d)", ret);
	ret = cal_client_db_add_changed_cb(handle, view_uri, callback, user_data);
	return ret;
}

API int calendar_db_remove_changed_cb(const char* view_uri, calendar_db_changed_cb callback, void* user_data)
{
	int ret = 0;
	calendar_h handle = NULL;

	ret = cal_client_handle_get_p(&handle);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_client_handle_get_p() Fail(%d)", ret);
	ret = cal_client_db_remove_changed_cb(handle, view_uri, callback, user_data);
	return ret;
}

API int calendar_db_insert_vcalendars(const char* vcalendar_stream, int **record_id_array, int *count)
{
	int ret = 0;
	calendar_h handle = NULL;

	ret = cal_client_handle_get_p(&handle);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_client_handle_get_p() Fail(%d)", ret);
	ret = cal_client_db_insert_vcalendars(handle, vcalendar_stream, record_id_array, count);
	return ret;
}

API int calendar_db_replace_vcalendars(const char* vcalendar_stream, int *record_id_array, int count)
{
	int ret = 0;
	calendar_h handle = NULL;

	ret = cal_client_handle_get_p(&handle);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_client_handle_get_p() Fail(%d)", ret);
	ret = cal_client_db_replace_vcalendars(handle, vcalendar_stream, record_id_array, count);
	return ret;
}

API int calendar_db_get_last_change_version(int* last_version)
{
	int ret = 0;
	calendar_h handle = NULL;

	ret = cal_client_handle_get_p(&handle);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_client_handle_get_p() Fail(%d)", ret);
	ret = cal_client_db_get_last_change_version(handle, last_version);
	return ret;
}

API int calendar_db_get_changes_exception_by_version(const char* view_uri, int original_event_id, int calendar_db_version, calendar_list_h* record_list)
{
	int ret = 0;
	calendar_h handle = NULL;

	ret = cal_client_handle_get_p(&handle);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_client_handle_get_p() Fail(%d)", ret);
	ret = cal_client_db_get_changes_exception_by_version(handle, view_uri, original_event_id, calendar_db_version, record_list);
	return ret;
}

