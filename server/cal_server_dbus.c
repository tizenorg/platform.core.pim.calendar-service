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
#include <gio/gio.h>

#include "calendar.h"
#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_handle.h"
#include "cal_dbus_helper.h"
#include "cal_dbus.h"
#include "cal_db.h"
#include "cal_db_util.h"
#include "cal_server_dbus.h"
#include "cal_utils.h"
#include "cal_server_ondemand.h"

static calDbus *dbus_object = NULL;
static GMutex cal_server_dbus_sender;
static GList *cal_sender_list; /* global list to care resource handle for each sender bus */

typedef struct _cal_sender {
	gchar *name;
} cal_sender_s;

calDbus* cal_dbus_get_object(void)
{
	return dbus_object;
}

static bool _has_sender(const char *name, GList **out_cursor)
{
	bool has_sender = false;

	GList *cursor = NULL;
	cursor = cal_sender_list;
	while (cursor) {
		cal_sender_s *sender = (cal_sender_s *)cursor->data;
		if (NULL == sender) {
			ERR("sender is NULL");
			cursor = g_list_next(cursor);
			continue;
		}
		if (CAL_STRING_EQUAL == g_strcmp0(sender->name, name)) {
			DBG("found sender[%s]", name);
			has_sender = true;
			if (out_cursor)
				*out_cursor = cursor;
			break;
		}
		cursor = g_list_next(cursor);
	}
	return has_sender;
}

static int _append_sender(const char *name)
{
	RETV_IF(NULL == name, CALENDAR_ERROR_INVALID_PARAMETER);

	cal_sender_s *sender = NULL;
	sender = calloc(1, sizeof(cal_sender_s));
	if (NULL == sender) {
		ERR("calloc() Fail");
		return CALENDAR_ERROR_OUT_OF_MEMORY;
	}
	sender->name = cal_strdup(name);
	DBG("[SENDER] Append sender[%s]", sender->name);
	cal_sender_list = g_list_append(cal_sender_list, sender);
	return CALENDAR_ERROR_NONE;
}

static gboolean _handle_register_resource(calDbus *object,
		GDBusMethodInvocation *invocation)
{
	CAL_FN_CALL();

	int ret = 0;
	const char *sender_name = NULL;
	sender_name = g_dbus_method_invocation_get_sender(invocation);

	g_mutex_lock(&cal_server_dbus_sender);
	if (true == _has_sender(sender_name, NULL)) {
		ERR("Already has sender");
		g_mutex_unlock(&cal_server_dbus_sender);
		return TRUE;
	}
	ret = _append_sender(sender_name);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("_append_sender() Fail");
		g_mutex_unlock(&cal_server_dbus_sender);
		return TRUE;
	}
	DBG("append sender");
	g_mutex_unlock(&cal_server_dbus_sender);

	cal_dbus_complete_register_resource(object, invocation, ret);

	return TRUE;
}

static gboolean _handle_unregister_resource(calDbus *object,
		GDBusMethodInvocation *invocation)
{
	CAL_FN_CALL();

	int ret = 0;
	const char *sender_name = NULL;
	sender_name = g_dbus_method_invocation_get_sender(invocation);

	GList *cursor = NULL;
	g_mutex_lock(&cal_server_dbus_sender);
	if (true == _has_sender(sender_name, &cursor)) {
		DBG("[SENDER] delete sender[%s]", sender_name);
		cal_sender_s *sender = (cal_sender_s *)cursor->data;
		free(sender->name);
		sender->name = NULL;
		cal_sender_list = g_list_delete_link(cal_sender_list, cursor);
	}

	if (0 == g_list_length(cal_sender_list)) {
		DBG("sender list is 0");
		g_list_free_full(cal_sender_list, free);
		cal_sender_list = NULL;
	}
	g_mutex_unlock(&cal_server_dbus_sender);

	cal_dbus_complete_unregister_resource(object, invocation, ret);

	return TRUE;
}

static gboolean _handle_insert_record(calDbus *object, GDBusMethodInvocation *invocation,
		GVariant *arg_handle, GVariant *arg_record)
{
	int ret = 0;
	calendar_record_h record = NULL;
	ret = cal_dbus_utils_gvariant_to_record(arg_record, &record);

	int id = 0;
	ret = cal_db_insert_record(record, &id);

	int version = 0;
	version = cal_db_util_get_transaction_ver();
	cal_dbus_complete_insert_record(object, invocation, id, version, ret);
	calendar_record_destroy(record, true);

	cal_server_ondemand_start();
	return TRUE;
}

static gboolean _handle_update_record(calDbus *object, GDBusMethodInvocation *invocation,
		GVariant *arg_handle, GVariant *arg_record)
{
	int ret = 0;
	calendar_record_h record = NULL;
	ret = cal_dbus_utils_gvariant_to_record(arg_record, &record);

	ret = cal_db_update_record(record);

	int version = 0;
	version = cal_db_util_get_transaction_ver();
	cal_dbus_complete_update_record(object, invocation, version, ret);
	calendar_record_destroy(record, true);

	cal_server_ondemand_start();
	return TRUE;
}

static gboolean _handle_delete_record(calDbus *object, GDBusMethodInvocation *invocation,
		GVariant *arg_handle, char *view_uri, int id)
{
	int ret = 0;
	ret = cal_db_delete_record(view_uri, id);

	int version = 0;
	version = cal_db_util_get_transaction_ver();
	cal_dbus_complete_delete_record(object, invocation, version, ret);

	cal_server_ondemand_start();
	return TRUE;
}

static gboolean _handle_replace_record(calDbus *object, GDBusMethodInvocation *invocation,
		GVariant *arg_handle, GVariant *arg_record, int id)
{
	int ret = 0;
	calendar_record_h record = NULL;
	ret = cal_dbus_utils_gvariant_to_record(arg_record, &record);

	ret = cal_db_replace_record(record, id);

	int version = 0;
	version = cal_db_util_get_transaction_ver();
	cal_dbus_complete_replace_record(object, invocation, version, ret);
	calendar_record_destroy(record, true);

	cal_server_ondemand_start();
	return TRUE;
}

static gboolean _handle_insert_records(calDbus *object, GDBusMethodInvocation *invocation,
		GVariant *arg_handle, GVariant *arg_list)
{
	int ret = 0;
	calendar_list_h list = NULL;
	ret = cal_dbus_utils_gvariant_to_list(arg_list, &list);

	int *ids = NULL;
	int count = 0;
	ret = cal_db_insert_records(list, &ids, &count);
	calendar_list_destroy(list, true);

	int version = 0;
	version = cal_db_util_get_transaction_ver();
	GVariant *arg_ids = cal_dbus_utils_ids_to_gvariant(ids, count);
	cal_dbus_complete_insert_records(object, invocation, arg_ids, count, version, ret);

	free(ids);
	g_variant_unref(arg_ids);

	cal_server_ondemand_start();
	return TRUE;
}

static gboolean _handle_update_records(calDbus *object, GDBusMethodInvocation *invocation,
		GVariant *arg_handle, GVariant *arg_list)
{
	int ret = 0;
	calendar_list_h list = NULL;
	ret = cal_dbus_utils_gvariant_to_list(arg_list, &list);

	ret = cal_db_update_records(list);
	calendar_list_destroy(list, true);

	int version = 0;
	version = cal_db_util_get_transaction_ver();
	cal_dbus_complete_update_records(object, invocation, version, ret);

	cal_server_ondemand_start();
	return TRUE;
}

static gboolean _handle_delete_records(calDbus *object, GDBusMethodInvocation *invocation,
		GVariant *arg_handle, char *view_uri, GVariant *arg_ids, int count)
{
	int ret = 0;
	int *ids = NULL;
	ret = cal_dbus_utils_gvariant_to_ids(arg_ids, count, &ids);

	ret = cal_db_delete_records(view_uri, ids, count);

	int version = 0;
	version = cal_db_util_get_transaction_ver();
	cal_dbus_complete_delete_records(object, invocation, version, ret);
	free(ids);

	cal_server_ondemand_start();
	return TRUE;
}

static gboolean _handle_replace_records(calDbus *object, GDBusMethodInvocation *invocation,
		GVariant *arg_handle, GVariant *arg_list, GVariant *arg_ids, int count)
{
	int ret = 0;
	calendar_list_h list = NULL;
	int *ids = NULL;
	ret = cal_dbus_utils_gvariant_to_list(arg_list, &list);
	ret = cal_dbus_utils_gvariant_to_ids(arg_ids, count, &ids);

	ret = cal_db_replace_records(list, ids, count);
	free(ids);
	calendar_list_destroy(list, true);

	int version = 0;
	version = cal_db_util_get_transaction_ver();
	cal_dbus_complete_replace_records(object, invocation, version, ret);

	cal_server_ondemand_start();
	return TRUE;
}

static gboolean _handle_get_record(calDbus *object, GDBusMethodInvocation *invocation,
		GVariant *arg_handle, char *view_uri, int id)
{
	int ret = 0;
	calendar_record_h record = NULL;
	ret = cal_db_get_record(view_uri, id, &record);

	GVariant *arg_record = cal_dbus_utils_record_to_gvariant(record);
	cal_dbus_complete_get_record(object, invocation, arg_record, ret);
	g_variant_unref(arg_record);
	calendar_record_destroy(record, true);

	cal_server_ondemand_start();
	return TRUE;
}

static gboolean _handle_get_all_records(calDbus *object, GDBusMethodInvocation *invocation,
		GVariant *arg_handle, char *view_uri, int offset, int limit)
{
	CAL_FN_CALL();

	int ret = 0;
	calendar_list_h list = NULL;
	ret = cal_db_get_all_records(view_uri, offset, limit, &list);

	GVariant *arg_list = cal_dbus_utils_list_to_gvariant(list);
	cal_dbus_complete_get_all_records(object, invocation, arg_list, ret);
	calendar_list_destroy(list, true);
	g_variant_unref(arg_list);

	cal_server_ondemand_start();
	return TRUE;
}

static gboolean _handle_get_records_with_query(calDbus *object, GDBusMethodInvocation *invocation,
		GVariant *arg_handle, GVariant *arg_query, int offset, int limit)
{
	int ret = 0;
	calendar_query_h query = NULL;
	ret = cal_dbus_utils_gvariant_to_query(arg_query, &query);

	calendar_list_h list = NULL;
	ret = cal_db_get_records_with_query(query, offset, limit, &list);

	GVariant *arg_list = cal_dbus_utils_list_to_gvariant(list);
	cal_dbus_complete_get_records_with_query(object, invocation, arg_list, ret);
	g_variant_unref(arg_list);
	calendar_list_destroy(list, true);

	cal_server_ondemand_start();
	return TRUE;
}

static gboolean _handle_get_count(calDbus *object, GDBusMethodInvocation *invocation,
		GVariant *arg_handle, char *view_uri)
{
	int ret = 0;
	int count = 0;
	ret = cal_db_get_count(view_uri, &count);

	cal_dbus_complete_get_count(object, invocation, count, ret);

	cal_server_ondemand_start();
	return TRUE;
}

static gboolean _handle_get_count_with_query(calDbus *object, GDBusMethodInvocation *invocation,
		GVariant *arg_handle, GVariant *arg_query)
{
	int ret = 0;
	calendar_query_h query = NULL;
	ret = cal_dbus_utils_gvariant_to_query(arg_query, &query);

	int count = 0;
	ret = cal_db_get_count_with_query(query, &count);
	cal_dbus_complete_get_count_with_query(object, invocation, count, ret);

	cal_server_ondemand_start();
	return TRUE;
}

static gboolean _handle_get_current_version(calDbus *object, GDBusMethodInvocation *invocation,
		GVariant *arg_handle)
{
	int ret = 0;
	int version = 0;
	ret = cal_db_get_current_version(&version);

	cal_dbus_complete_get_current_version(object, invocation, version, ret);

	cal_server_ondemand_start();
	return TRUE;
}

static gboolean _handle_check_permission_write(calDbus *object, GDBusMethodInvocation *invocation)
{
	cal_dbus_complete_check_permission_write(object, invocation);
	cal_server_ondemand_start();
	return TRUE;
}

static gboolean _handle_check_permission_read(calDbus *object, GDBusMethodInvocation *invocation)
{
	cal_dbus_complete_check_permission_read(object, invocation);
	cal_server_ondemand_start();
	return TRUE;
}

static gboolean _handle_get_changes_by_version(calDbus *object, GDBusMethodInvocation *invocation,
		GVariant *arg_handle, char *view_uri, int book_id, int in_version)
{
	int ret = 0;
	calendar_list_h list = NULL;
	int out_version = 0;
	ret = cal_db_get_changes_by_version(view_uri, book_id, in_version, &list, &out_version);

	GVariant *arg_list = cal_dbus_utils_list_to_gvariant(list);
	cal_dbus_complete_get_changes_by_version(object, invocation, arg_list, out_version, ret);
	calendar_list_destroy(list, true);
	g_variant_unref(arg_list);

	cal_server_ondemand_start();
	return TRUE;
}

static gboolean _handle_get_changes_exception_by_version(calDbus *object,
		GDBusMethodInvocation *invocation,
		GVariant *arg_handle,
		char *view_uri,
		int original_id,
		int version)
{
	int ret = 0;
	calendar_list_h list = NULL;
	ret = cal_db_get_changes_exception_by_version(view_uri, original_id, version, &list);

	GVariant *arg_list = cal_dbus_utils_list_to_gvariant(list);
	cal_dbus_complete_get_changes_exception_by_version(object, invocation, arg_list, ret);
	calendar_list_destroy(list, true);
	g_variant_unref(arg_list);

	cal_server_ondemand_start();
	return TRUE;
}

static gboolean _handle_clean_after_sync(calDbus *object, GDBusMethodInvocation *invocation,
		GVariant *arg_handle, int book_id, int version)
{
	int ret = 0;
	ret = cal_db_clean_after_sync(book_id, version);

	cal_dbus_complete_clean_after_sync(object, invocation, ret);

	cal_server_ondemand_start();
	return TRUE;
}

static gboolean _handle_insert_vcalendars(calDbus *object, GDBusMethodInvocation *invocation,
		GVariant *arg_handle, char *stream)
{
	int ret = 0;
	int *ids = NULL;
	int count = 0;
	ret = cal_db_insert_vcalendars(stream, &ids, &count);

	GVariant *arg_ids = cal_dbus_utils_ids_to_gvariant(ids, count);

	int version = 0;
	version = cal_db_util_get_transaction_ver();
	cal_dbus_complete_insert_vcalendars(object, invocation, arg_ids, count, version, ret);
	free(ids);
	g_variant_unref(arg_ids);

	cal_server_ondemand_start();
	return TRUE;
}

static gboolean _handle_replace_vcalendars(calDbus *object, GDBusMethodInvocation *invocation,
		GVariant *arg_handle, char *stream, GVariant *arg_ids, int count)
{
	int ret = 0;
	int *ids = NULL;
	ret = cal_dbus_utils_gvariant_to_ids(arg_ids, count, &ids);

	ret = cal_db_replace_vcalendars(stream, ids, count);

	int version = 0;
	version = cal_db_util_get_transaction_ver();
	cal_dbus_complete_replace_vcalendars(object, invocation, ret, version);
	free(ids);

	cal_server_ondemand_start();
	return TRUE;
}

static int _cal_server_dbus_find_sender(const char *owner_name, cal_sender_s **out_sender)
{
	GList *cursor = NULL;

	RETV_IF(NULL == owner_name, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_sender, CALENDAR_ERROR_INVALID_PARAMETER);

	cursor = cal_sender_list;
	while (cursor) {
		cal_sender_s *sender = (cal_sender_s *)cursor->data;
		if (NULL == sender) {
			ERR("sender is NULL");
			return CALENDAR_ERROR_NO_DATA;
		}

		if (CAL_STRING_EQUAL == g_strcmp0(sender->name, owner_name)) {
			*out_sender = sender;
			break;
		}
		cursor = g_list_next(cursor);
	}

	return CALENDAR_ERROR_NONE;
}

static void _delete_sender(cal_sender_s *sender)
{
	RET_IF(NULL == sender);

	GList *cursor = cal_sender_list;
	while (cursor) {
		if (cursor->data == sender) {
			DBG("[SENDER] Delete sender[%s]", sender->name);
			free(sender->name);
			sender->name = NULL;
			cal_sender_list = g_list_delete_link(cal_sender_list, cursor);
			break;
		}
		cursor = g_list_next(cursor);
	}
}

static void _cal_server_dbus_name_owner_changed_cb(GDBusConnection *connection,
		const gchar *sender_name,
		const gchar *object_path,
		const gchar *interface_name,
		const gchar *signal_name,
		GVariant *parameters,
		gpointer user_data)
{
	CAL_FN_CALL();

	int ret = 0;
	gchar *name = NULL;
	gchar *old_owner = NULL;
	gchar *new_owner = NULL;

	g_variant_get(parameters, "(&s&s&s)", &name, &old_owner, &new_owner);
	DBG("name[%s] old_owner[%s] new_owner[%s]", name, old_owner, new_owner);

	if (0 != strlen(new_owner)) {
		DBG("new_owner[%s]", new_owner);
		return;
	}
	g_mutex_lock(&cal_server_dbus_sender);
	/* empty new_owner means server-kill */
	cal_sender_s *sender = NULL;
	ret = _cal_server_dbus_find_sender(old_owner, &sender);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("_cal_server_dbus_find_sender() Fail(%d)", ret);
		g_mutex_unlock(&cal_server_dbus_sender);
		return;
	}

	if (sender) { /* found bus name in our bus list */
		DBG("owner[%s] stopped", old_owner);
		_delete_sender(sender);
	}
	g_mutex_unlock(&cal_server_dbus_sender);
}

static int _cal_server_dbus_subscribe_name_owner_changed(GDBusConnection *conn)
{
	CAL_FN_CALL();

	unsigned int id = 0;
	id = g_dbus_connection_signal_subscribe(conn,
			"org.freedesktop.DBus", /* bus name */
			"org.freedesktop.DBus", /* interface */
			"NameOwnerChanged", /* member */
			"/org/freedesktop/DBus", /* path */
			NULL, /* arg0 */
			G_DBUS_SIGNAL_FLAGS_NONE,
			_cal_server_dbus_name_owner_changed_cb,
			NULL,
			NULL);
	if (0 == id) {
		ERR("g_dbus_connection_signal_subscribe() Fail");
		return CALENDAR_ERROR_IPC;
	}
	return CALENDAR_ERROR_NONE;
}

static void _dbus_on_bus_acquired(GDBusConnection *conn, const gchar *name, gpointer user_data)
{
	gboolean ret = 0;
	GError *error = NULL;

	dbus_object = cal_dbus_skeleton_new();
	if (NULL == dbus_object) {
		ERR("cal_dbus_skeleton_new() Fail");
		return;
	}

	g_signal_connect(dbus_object, "handle-register-resource",
			G_CALLBACK(_handle_register_resource), NULL);
	g_signal_connect(dbus_object, "handle-unregister-resource",
			G_CALLBACK(_handle_unregister_resource), NULL);
	g_signal_connect(dbus_object, "handle-insert-record",
			G_CALLBACK(_handle_insert_record), NULL);
	g_signal_connect(dbus_object, "handle-update-record",
			G_CALLBACK(_handle_update_record), NULL);
	g_signal_connect(dbus_object, "handle-delete-record",
			G_CALLBACK(_handle_delete_record), NULL);
	g_signal_connect(dbus_object, "handle-replace-record",
			G_CALLBACK(_handle_replace_record), NULL);
	g_signal_connect(dbus_object, "handle-insert-records",
			G_CALLBACK(_handle_insert_records), NULL);
	g_signal_connect(dbus_object, "handle-update-records",
			G_CALLBACK(_handle_update_records), NULL);
	g_signal_connect(dbus_object, "handle-delete-records",
			G_CALLBACK(_handle_delete_records), NULL);
	g_signal_connect(dbus_object, "handle-replace-records",
			G_CALLBACK(_handle_replace_records), NULL);
	g_signal_connect(dbus_object, "handle-get-record",
			G_CALLBACK(_handle_get_record), NULL);
	g_signal_connect(dbus_object, "handle-get-all-records",
			G_CALLBACK(_handle_get_all_records), NULL);
	g_signal_connect(dbus_object, "handle-get-records-with-query",
			G_CALLBACK(_handle_get_records_with_query), NULL);
	g_signal_connect(dbus_object, "handle-get-count",
			G_CALLBACK(_handle_get_count), NULL);
	g_signal_connect(dbus_object, "handle-get-count-with-query",
			G_CALLBACK(_handle_get_count_with_query), NULL);
	g_signal_connect(dbus_object, "handle-get-current-version",
			G_CALLBACK(_handle_get_current_version), NULL);
	g_signal_connect(dbus_object, "handle-check-permission-write",
			G_CALLBACK(_handle_check_permission_write), NULL);
	g_signal_connect(dbus_object, "handle-check-permission-read",
			G_CALLBACK(_handle_check_permission_read), NULL);
	g_signal_connect(dbus_object, "handle-get-changes-by-version",
			G_CALLBACK(_handle_get_changes_by_version), NULL);
	g_signal_connect(dbus_object, "handle-get-changes-exception-by-version",
			G_CALLBACK(_handle_get_changes_exception_by_version), NULL);
	g_signal_connect(dbus_object, "handle-clean-after-sync",
			G_CALLBACK(_handle_clean_after_sync), NULL);
	g_signal_connect(dbus_object, "handle-insert-vcalendars",
			G_CALLBACK(_handle_insert_vcalendars), NULL);
	g_signal_connect(dbus_object, "handle-replace-vcalendars",
			G_CALLBACK(_handle_replace_vcalendars), NULL);

	ret = g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(dbus_object),
			conn, CAL_DBUS_OBJPATH, &error);
	if (FALSE == ret) {
		ERR("g_dbus_interface_skeleton_export() Fail(%s)", error->message);
		g_error_free(error);
		return;
	}

	ret = _cal_server_dbus_subscribe_name_owner_changed(conn);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("_cal_server_dbus_subscribe_name_owner_changed() Fail(%d)", ret);
		return;
	}
}

static void _dbus_on_name_acquired(GDBusConnection *conn, const gchar *name, gpointer user_data)
{
	DBG("Acquired the name %s", name);
}

static void _dbus_on_name_lost(GDBusConnection *conn, const gchar *name, gpointer user_data)
{
	DBG("Lost the name %s", name);
}

unsigned int cal_server_dbus_init(void)
{
	guint id = 0;
	id = g_bus_own_name(G_BUS_TYPE_SESSION,
			CAL_DBUS_INTERFACE,
			G_BUS_NAME_OWNER_FLAGS_REPLACE,
			_dbus_on_bus_acquired,
			_dbus_on_name_acquired,
			_dbus_on_name_lost,
			NULL,
			NULL);
	if (0 == id) {
		ERR("g_bus_own_name() Fail");
		return 0;
	}
	return id;
}

void cal_server_dbus_deinit(unsigned int id)
{
	g_bus_unown_name(id);
}

int cal_dbus_emit_signal(const char *dest, const char *signal_name, GVariant *value)
{
	CAL_FN_CALL();

	gboolean ret;
	GError *error = NULL;

	DBG("signal_name[%s]", signal_name);
	DBG("data[%s]", g_variant_print(value, FALSE));

	calDbusSkeleton *skeleton = NULL;
	skeleton = CAL_DBUS_SKELETON(cal_dbus_get_object());
	if (NULL == skeleton) {
		ERR("cal_dbus_get_object() Fail");
		return CALENDAR_ERROR_IPC;
	}

	GDBusConnection *conn = NULL;
	conn = g_dbus_interface_skeleton_get_connection(G_DBUS_INTERFACE_SKELETON(skeleton));
	if (NULL == conn) {
		ERR("g_dbus_interface_skeleton_get_connection() Fail");
		return CALENDAR_ERROR_IPC;
	}

	ret = g_dbus_connection_emit_signal(conn,
			dest,
			CAL_DBUS_OBJPATH,
			CAL_DBUS_INTERFACE,
			signal_name,
			value,
			&error);

	if (FALSE == ret) {
		ERR("g_dbus_connection_emit_signal() Fail");
		if (error) {
			ERR("error[%s]", error->message);
			g_error_free(error);
		}
		return CALENDAR_ERROR_IPC;
	}

	if (FALSE == g_dbus_connection_flush_sync(conn, NULL, &error)) {
		ERR("g_dbus_connection_flush_sync() Fail");
		if (error) {
			ERR("error[%s]", error->message);
			g_error_free(error);
		}
		return CALENDAR_ERROR_IPC;
	}

	return CALENDAR_ERROR_NONE;
}

int cal_dbus_publish_reminder(int stream_size, char *stream)
{
	CAL_FN_CALL();

	GVariant *value = NULL;
	value = cal_dbus_utils_stream_to_gvariant(stream_size, stream);
	cal_dbus_emit_signal(NULL, CAL_NOTI_REMINDER_CAHNGED, value);

	return CALENDAR_ERROR_NONE;
}
