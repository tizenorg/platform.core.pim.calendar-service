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
#include "cal_client_db_helper.h"

API int calendar_db_insert_record(calendar_record_h record, int* id)
{
	return cal_client_db_insert_record(record, id);
}

API int calendar_db_update_record(calendar_record_h record)
{
	return cal_client_db_update_record(record);
}

API int calendar_db_delete_record(const char* view_uri, int id)
{
	return cal_client_db_delete_record(view_uri, id);
}

API int calendar_db_replace_record(calendar_record_h record, int record_id)
{
	return cal_client_db_replace_record(record, record_id);
}

API int calendar_db_get_record(const char* view_uri, int id, calendar_record_h* out_record)
{
	return cal_client_db_get_record(view_uri, id, out_record);
}

API int calendar_db_get_all_records(const char* view_uri, int offset, int limit, calendar_list_h* out_list)
{
	return cal_client_db_get_all_records(view_uri, offset, limit, out_list);
}

API int calendar_db_get_records_with_query(calendar_query_h query, int offset, int limit, calendar_list_h* out_list)
{
	return cal_client_db_get_records_with_query(query, offset, limit, out_list);
}

API int calendar_db_clean_after_sync(int calendar_book_id, int calendar_db_version)
{
	return cal_client_db_clean_after_sync(calendar_book_id, calendar_db_version);
}

API int calendar_db_get_count(const char *view_uri, int *out_count)
{
	return cal_client_db_get_count(view_uri, out_count);
}

API int calendar_db_get_count_with_query(calendar_query_h query, int *out_count)
{
	return cal_client_db_get_count_with_query(query, out_count);
}

API int calendar_db_insert_records(calendar_list_h record_list, int** record_id_array, int* count)
{
	return cal_client_db_insert_records(record_list, record_id_array, count);
}

API int calendar_db_update_records(calendar_list_h record_list)
{
	return cal_client_db_update_records(record_list);
}

API int calendar_db_delete_records(const char* view_uri, int record_id_array[], int count)
{
	return cal_client_db_delete_records(view_uri, record_id_array, count);
}

API int calendar_db_replace_records(calendar_list_h record_list, int *record_id_array, int count)
{
	return cal_client_db_replace_records(record_list, record_id_array, count);
}

API int calendar_db_get_changes_by_version(const char* view_uri, int calendar_book_id, int calendar_db_version, calendar_list_h* record_list, int* current_calendar_db_version)
{
	return cal_client_db_get_changes_by_version(view_uri, calendar_book_id, calendar_db_version, record_list, current_calendar_db_version);
}

API int calendar_db_get_current_version(int* calendar_db_version)
{
	return cal_client_db_get_current_version(calendar_db_version);
}

API int calendar_db_add_changed_cb(const char* view_uri, calendar_db_changed_cb callback, void* user_data)
{
	return cal_client_db_add_changed_cb(view_uri, callback, user_data);
}

API int calendar_db_remove_changed_cb(const char* view_uri, calendar_db_changed_cb callback, void* user_data)
{
	return cal_client_db_remove_changed_cb(view_uri, callback, user_data);
}

API int calendar_db_insert_vcalendars(const char* vcalendar_stream, int **record_id_array, int *count)
{
	return cal_client_db_insert_vcalendars(vcalendar_stream, record_id_array, count);
}

API int calendar_db_replace_vcalendars(const char* vcalendar_stream, int *record_id_array, int count)
{
	return cal_client_db_replace_vcalendars(vcalendar_stream, record_id_array, count);
}

API int calendar_db_get_last_change_version(int* last_version)
{
	return cal_client_db_get_last_change_version(last_version);
}

API int calendar_db_get_changes_exception_by_version(const char* view_uri, int original_event_id, int calendar_db_version, calendar_list_h* record_list)
{
	return cal_client_db_get_changes_exception_by_version(view_uri, original_event_id, calendar_db_version, record_list);
}

