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

#ifndef __CAL_CLIENT_DB_HELPER_H__
#define __CAL_CLIENT_DB_HELPER_H__

int cal_client_db_insert_record(calendar_record_h record, int* id);
int cal_client_db_update_record(calendar_record_h record);
int cal_client_db_delete_record(const char* view_uri, int id);
int cal_client_db_replace_record(calendar_record_h record, int record_id);
int cal_client_db_get_record(const char* view_uri, int id, calendar_record_h* out_record);
int cal_client_db_get_all_records(const char* view_uri, int offset, int limit, calendar_list_h* out_list);
int cal_client_db_get_records_with_query(calendar_query_h query, int offset, int limit, calendar_list_h* out_list);
int cal_client_db_clean_after_sync(int calendar_book_id, int calendar_db_version);
int cal_client_db_get_count(const char *view_uri, int *out_count);
int cal_client_db_get_count_with_query(calendar_query_h query, int *out_count);
int cal_client_db_insert_records(calendar_list_h record_list, int** record_id_array, int* count);
int cal_client_db_update_records(calendar_list_h record_list);
int cal_client_db_delete_records(const char* view_uri, int record_id_array[], int count);
int cal_client_db_replace_records(calendar_list_h record_list, int *record_id_array, int count);
int cal_client_db_get_changes_by_version(const char* view_uri, int calendar_book_id, int calendar_db_version, calendar_list_h* record_list, int* current_calendar_db_version);
int cal_client_db_get_current_version(int* calendar_db_version);
int cal_client_db_add_changed_cb(const char* view_uri, void *callback, void* user_data);
int cal_client_db_remove_changed_cb(const char* view_uri, void *callback, void* user_data);
int cal_client_db_insert_vcalendars(const char* vcalendar_stream, int **record_id_array, int *count);
int cal_client_db_replace_vcalendars(const char* vcalendar_stream, int *record_id_array, int count);
int cal_client_db_get_last_change_version(int* last_version);
int cal_client_db_get_changes_exception_by_version(const char* view_uri, int original_event_id, int calendar_db_version, calendar_list_h* record_list);


#endif /* __CAL_CLIENT_DB_HELPER_H__ */
