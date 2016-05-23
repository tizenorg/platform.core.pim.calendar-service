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

#ifndef __CAL_CLIENT_DBUS_H__
#define __CAL_CLIENT_DBUS_H__

#include <gio/gio.h>

void cal_dbus_call_reminder_cb(GDBusConnection *connection,
		const gchar *sender_name,
		const gchar *object_path,
		const gchar *interface_name,
		const gchar *signal_name,
		GVariant *parameters,
		gpointer user_data);
unsigned int cal_dbus_subscribe_signal(char *signal_name, GDBusSignalCallback callback,
		 gpointer user_data, GDestroyNotify user_data_free_func);
void cal_dbus_unsubscribe_signal(unsigned int id);

unsigned int cal_dbus_start(void);
void cal_dbus_stop(void);

int cal_dbus_insert_record(calendar_h handle, calendar_record_h record, int *out_id);
int cal_dbus_update_record(calendar_h handle, calendar_record_h record);
int cal_dbus_delete_record(calendar_h handle, const char *view_uri, int id);
int cal_dbus_replace_record(calendar_h handle, calendar_record_h record, int id);
int cal_dbus_insert_records(calendar_h handle, calendar_list_h list, int** out_ids,
		int* out_count);
int cal_dbus_update_records(calendar_h handle, calendar_list_h list);
int cal_dbus_delete_records(calendar_h handle, const char *view_uri, int *ids,
		int count);
int cal_dbus_replace_records(calendar_h handle, calendar_list_h list, int *ids,
		int count);
int cal_dbus_get_record(calendar_h handle, const char *view_uri, int id,
		calendar_record_h *out_record);
int cal_dbus_get_all_records(calendar_h handle, const char *view_uri,
		int offset, int limit, calendar_list_h *out_list);
int cal_dbus_get_records_with_query(calendar_h handle, calendar_query_h query,
		int offset, int limit, calendar_list_h* out_list);
int cal_dbus_get_count(calendar_h handle, const char *view_uri, int *out_count);
int cal_dbus_get_count_with_query(calendar_h handle, calendar_query_h query,
		int *out_count);
int cal_dbus_add_changed_cb(calendar_h handle, const char* view_uri,
		void *callback, void* user_data);
int cal_dbus_remove_changed_cb(calendar_h handle, const char* view_uri,
		void *callback, void* user_data);
int cal_dbus_get_current_version(calendar_h handle, int *out_version);
int cal_dbus_get_changes_by_version(calendar_h handle, const char *view_uri,
		int book_id, int version, calendar_list_h *out_list, int *out_version);
int cal_dbus_get_changes_exception_by_version(calendar_h handle, const char *view_uri,
		int original_id, int version, calendar_list_h *out_list);
int cal_dbus_get_last_change_version(calendar_h handle, int *out_version);
int cal_dbus_clean_after_sync(calendar_h handle, int book_id, int version);
int cal_dbus_insert_vcalendars(calendar_h handle, const char *stream, int **out_ids,
		int *out_count);
int cal_dbus_replace_vcalendars(calendar_h handle, const char *stream, int *ids,
		int count);

#endif /* __CAL_CLIENT_DBUS_H__ */
