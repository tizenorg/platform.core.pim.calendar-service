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

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <gio/gio.h>
#include <sys/time.h>

#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_view.h"
#include "cal_dbus_helper.h"
#include "cal_dbus.h"
#include "cal_list.h"
#include "cal_client_handle.h"
#include "cal_client_reminder.h"
#include "cal_client_db_helper.h"

#define __CAL_CLIENT_ACCESS_MAX 10
#define __CAL_CLIENT_ALLOW_USEC 25000
#define CAL_LIMIT_ACCESS_FRONT(uri) \
	DBG("uri[%s]", uri); \
	int is_schedule = 0; \
	do { \
		if (CAL_STRING_EQUAL == strncmp(uri, CALENDAR_VIEW_EVENT, strlen(CALENDAR_VIEW_EVENT))) { \
			is_schedule = 1; \
			struct timeval hold = {0}; \
			struct timeval diff = {0}; \
			gettimeofday(&hold, NULL); \
			timersub(&hold, &__g_release_time, &diff); \
			DBG("%ld.%ld sec", diff.tv_sec, diff.tv_usec); \
			if (diff.tv_sec / 1000 == 0 && diff.tv_usec < __CAL_CLIENT_ALLOW_USEC) { \
				if (__g_access_count < __CAL_CLIENT_ACCESS_MAX) { \
					__g_access_count++; \
					DBG("--count (%d)", __g_access_count); \
				} else { \
					DBG("--sleep"); \
					usleep(200000); \
					__g_access_count = 0; \
					timerclear(&__g_release_time); \
				} \
			} else { \
				DBG("--reset"); \
				__g_access_count = 0; \
				timerclear(&__g_release_time); \
			} \
		} \
	} while (0)

#define CAL_LIMIT_ACCESS_BACK \
	do { \
		if (is_schedule) { \
			gettimeofday(&__g_release_time, NULL); \
		} \
	} while (0)

static int __g_access_count = 0;
static struct timeval __g_release_time;

static calDbus *cal_dbus_object;

static void _cal_dbus_cleanup(void)
{
	CAL_FN_CALL();
}

/* LCOV_EXCL_START */
static void _cal_dbus_name_owner_notify(GObject *object, GParamSpec *pspec, gpointer user_data)
{
	CAL_FN_CALL();

	GDBusProxy *proxy = G_DBUS_PROXY(object);
	gchar *name_owner = g_dbus_proxy_get_name_owner(proxy);

	if (name_owner) {
		DBG("name_owner[%s]", name_owner);
		return;
	}

	_cal_dbus_cleanup();
}
/* LCOV_EXCL_STOP */

void cal_dbus_call_reminder_cb(GDBusConnection *connection,
		const gchar *sender_name,
		const gchar *object_path,
		const gchar *interface_name,
		const gchar *signal_name,
		GVariant *parameters,
		gpointer user_data)
{
	CAL_FN_CALL();

	int stream_size = 0;
	char *stream = NULL;
	cal_dbus_utils_gvariant_to_stream(parameters, &stream_size, &stream);
	DBG("stream[%s]", stream);
	cal_client_reminder_call_subscribe(stream);
}

unsigned int cal_dbus_subscribe_signal(char *signal_name, GDBusSignalCallback callback,
		gpointer user_data, GDestroyNotify user_data_free_func)
{
	GDBusConnection *conn = g_dbus_proxy_get_connection(G_DBUS_PROXY(cal_dbus_object));

	return g_dbus_connection_signal_subscribe(conn,
			NULL,
			CAL_DBUS_INTERFACE,
			signal_name,
			CAL_DBUS_OBJPATH,
			NULL,
			G_DBUS_SIGNAL_FLAGS_NONE,
			callback,
			user_data,
			user_data_free_func);
}

void cal_dbus_unsubscribe_signal(unsigned int id)
{
	GDBusConnection *conn = g_dbus_proxy_get_connection(G_DBUS_PROXY(cal_dbus_object));
	g_dbus_connection_signal_unsubscribe(conn, id);
}

static int _register_resource(void)
{
	GError *error = NULL;
	int ret = 0;

	cal_dbus_call_register_resource_sync(cal_dbus_object, &ret, NULL, &error);
	if (error) {
		/* LCOV_EXCL_START */
		ERR("cal_dbus_call_register_resource_sync() Fail[%s]", error->message);
		g_error_free(error);
		return CALENDAR_ERROR_IPC;
		/* LCOV_EXCL_STOP */
	}
	return ret;
}

static int _unregister_resource(void)
{
	GError *error = NULL;
	int ret = 0;

	cal_dbus_call_unregister_resource_sync(cal_dbus_object, &ret, NULL, &error);
	if (error) {
		/* LCOV_EXCL_START */
		ERR("cal_dbus_call_register_resource_sync() Fail[%s]", error->message);
		g_error_free(error);
		return CALENDAR_ERROR_IPC;
		/* LCOV_EXCL_STOP */
	}
	return ret;
}

int cal_dbus_start(void)
{
	if (cal_dbus_object) {
		DBG("Already exists");
		return CALENDAR_ERROR_NONE;
	}

	GError *error = NULL;
	cal_dbus_object = cal_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION,
			G_DBUS_PROXY_FLAGS_NONE,
			CAL_DBUS_INTERFACE,
			CAL_DBUS_OBJPATH,
			NULL,
			&error);
	if (NULL == cal_dbus_object) {
		/* LCOV_EXCL_START */
		ERR("cal_dbus_proxy_new_for_bus_sync() Fail");
		if (error) {
			ERR("error[%s]", error->message);
			g_error_free(error);
		}
		return CALENDAR_ERROR_IPC;
		/* LCOV_EXCL_STOP */
	}

	_register_resource();

	unsigned int id;
	id = g_signal_connect(cal_dbus_object, "notify::g-name-owner",
			G_CALLBACK(_cal_dbus_name_owner_notify), NULL);
	if (0 == id) {
		/* LCOV_EXCL_START */
		ERR("g_signal_connect() Fail");
		return CALENDAR_ERROR_IPC;
		/* LCOV_EXCL_STOP */
	}

	return CALENDAR_ERROR_NONE;
}

int cal_dbus_stop(void)
{
	if (NULL == cal_dbus_object) {
		/* LCOV_EXCL_START */
		ERR("No object");
		return CALENDAR_ERROR_NONE;
		/* LCOV_EXCL_STOP */
	}

	DBG("[ALL CONNECTION IS CLOSED]");

	_unregister_resource();
	_cal_dbus_cleanup();

	g_object_unref(cal_dbus_object);
	cal_dbus_object = NULL;

	return CALENDAR_ERROR_NONE;
}

/* LCOV_EXCL_START */
int cal_dbus_recovery(void)
{
	CAL_FN_CALL();

	if (cal_dbus_object) {
		DBG("unref");
		g_object_unref(cal_dbus_object);
		cal_dbus_object = NULL;
	}

	GError *error = NULL;
	cal_dbus_object = cal_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION,
			G_DBUS_PROXY_FLAGS_NONE,
			CAL_DBUS_INTERFACE,
			CAL_DBUS_OBJPATH,
			NULL,
			&error);
	if (NULL == cal_dbus_object) {
		ERR("cal_dbus_proxy_new_for_bus_sync() Fail");
		if (error) {
			ERR("error[%s]", error->message);
			g_error_free(error);
		}
		return CALENDAR_ERROR_IPC;
	}

	unsigned int id;
	id = g_signal_connect(cal_dbus_object, "notify::g-name-owner",
			G_CALLBACK(_cal_dbus_name_owner_notify), NULL);
	if (0 == id) {
		ERR("g_signal_connect() Fail");
		return CALENDAR_ERROR_IPC;
	}

	return CALENDAR_ERROR_NONE;
}
/* LCOV_EXCL_STOP */

int cal_dbus_insert_record(calendar_h handle, calendar_record_h record, int *out_id)
{
	GError *error = NULL;
	int ret = 0;
	int version = 0;
	int id = 0;

	RETV_IF(NULL == handle, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == record, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cal_dbus_object, CALENDAR_ERROR_IPC);

	CAL_RECORD_RESET_COMMON((cal_record_s *)record);
	CAL_LIMIT_ACCESS_FRONT(((cal_record_s *)record)->view_uri);

	GVariant *arg_handle = cal_dbus_utils_handle_to_gvariant(handle);
	GVariant *arg_record = cal_dbus_utils_record_to_gvariant(record);
	cal_dbus_call_insert_record_sync(cal_dbus_object, arg_handle, arg_record,
			&id, &version, &ret, NULL, &error);
	g_variant_unref(arg_record);
	g_variant_unref(arg_handle);
	if (error) {
		/* LCOV_EXCL_START */
		ERR("cal_dbus_call_insert_record_sync() Fail[%s]", error->message);
		if (G_DBUS_ERROR_ACCESS_DENIED == error->code)
			ret = CALENDAR_ERROR_PERMISSION_DENIED;
		else
			ret = CALENDAR_ERROR_IPC;
		g_error_free(error);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	cal_client_handle_set_version(handle, version);
	if (out_id)
		*out_id = id;

	CAL_LIMIT_ACCESS_BACK;
	return ret;
}

int cal_dbus_update_record(calendar_h handle, calendar_record_h record)
{
	GError *error = NULL;
	int ret = 0;
	int version = 0;

	RETV_IF(NULL == handle, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == record, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cal_dbus_object, CALENDAR_ERROR_IPC);

	CAL_LIMIT_ACCESS_FRONT(((cal_record_s *)record)->view_uri);

	GVariant *arg_handle = cal_dbus_utils_handle_to_gvariant(handle);
	GVariant *arg_record = cal_dbus_utils_record_to_gvariant(record);
	cal_dbus_call_update_record_sync(cal_dbus_object, arg_handle, arg_record,
			&version, &ret, NULL, &error);
	g_variant_unref(arg_record);
	g_variant_unref(arg_handle);
	if (error) {
		/* LCOV_EXCL_START */
		ERR("cal_dbus_call_update_record_sync() Fail[%s]", error->message);
		if (G_DBUS_ERROR_ACCESS_DENIED == error->code)
			ret = CALENDAR_ERROR_PERMISSION_DENIED;
		else
			ret = CALENDAR_ERROR_IPC;
		g_error_free(error);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	cal_client_handle_set_version(handle, version);

	CAL_LIMIT_ACCESS_BACK;
	return ret;
}

int cal_dbus_delete_record(calendar_h handle, const char *view_uri, int id)
{
	GError *error = NULL;
	int ret = 0;
	int version = 0;

	RETV_IF(NULL == handle, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == view_uri, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cal_dbus_object, CALENDAR_ERROR_IPC);

	CAL_LIMIT_ACCESS_FRONT(view_uri);

	GVariant *arg_handle = cal_dbus_utils_handle_to_gvariant(handle);
	cal_dbus_call_delete_record_sync(cal_dbus_object, arg_handle, view_uri, id,
			&version, &ret, NULL, &error);
	g_variant_unref(arg_handle);
	if (error) {
		/* LCOV_EXCL_START */
		ERR("cal_dbus_call_delete_record_sync() Fail[%s]", error->message);
		if (G_DBUS_ERROR_ACCESS_DENIED == error->code)
			ret = CALENDAR_ERROR_PERMISSION_DENIED;
		else
			ret = CALENDAR_ERROR_IPC;
		g_error_free(error);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	cal_client_handle_set_version(handle, version);

	CAL_LIMIT_ACCESS_BACK;
	return ret;
}

int cal_dbus_replace_record(calendar_h handle, calendar_record_h record, int id)
{
	GError *error = NULL;
	int ret = 0;
	int version = 0;

	RETV_IF(NULL == handle, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == record, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cal_dbus_object, CALENDAR_ERROR_IPC);

	CAL_LIMIT_ACCESS_FRONT(((cal_record_s *)record)->view_uri);

	GVariant *arg_handle = cal_dbus_utils_handle_to_gvariant(handle);
	GVariant *arg_record = cal_dbus_utils_record_to_gvariant(record);
	cal_dbus_call_replace_record_sync(cal_dbus_object, arg_handle, arg_record, id,
			&version, &ret, NULL, &error);
	g_variant_unref(arg_record);
	g_variant_unref(arg_handle);
	if (error) {
		/* LCOV_EXCL_START */
		ERR("cal_dbus_call_replace_record_sync() Fail[%s]", error->message);
		if (G_DBUS_ERROR_ACCESS_DENIED == error->code)
			ret = CALENDAR_ERROR_PERMISSION_DENIED;
		else
			ret = CALENDAR_ERROR_IPC;
		g_error_free(error);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	cal_client_handle_set_version(handle, version);

	CAL_LIMIT_ACCESS_BACK;
	return ret;
}

int cal_dbus_insert_records(calendar_h handle, calendar_list_h list,
		int** out_ids, int* out_count)
{
	GError *error = NULL;
	int ret = 0;
	int count = 0;
	int version = 0;

	RETV_IF(NULL == handle, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == list, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cal_dbus_object, CALENDAR_ERROR_IPC);

	GVariant *arg_handle = cal_dbus_utils_handle_to_gvariant(handle);
	GVariant *arg_list = cal_dbus_utils_list_to_gvariant(list);
	GVariant *arg_ids = NULL;
	cal_dbus_call_insert_records_sync(cal_dbus_object, arg_handle, arg_list,
			&arg_ids, &count, &version, &ret, NULL, &error);
	g_variant_unref(arg_list);
	g_variant_unref(arg_handle);
	if (error) {
		/* LCOV_EXCL_START */
		ERR("cal_dbus_call_insert_records_sync() Fail[%s]", error->message);
		if (G_DBUS_ERROR_ACCESS_DENIED == error->code)
			ret = CALENDAR_ERROR_PERMISSION_DENIED;
		else
			ret = CALENDAR_ERROR_IPC;
		g_error_free(error);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	cal_client_handle_set_version(handle, version);

	int *ids = NULL;
	cal_dbus_utils_gvariant_to_ids(arg_ids, count, &ids);
	g_variant_unref(arg_ids);

	if (out_ids)
		*out_ids = ids;
	else
		free(ids);

	if (out_count)
		*out_count = count;

	return ret;
}

int cal_dbus_update_records(calendar_h handle, calendar_list_h list)
{
	GError *error = NULL;
	int ret = 0;
	int version = 0;

	RETV_IF(NULL == handle, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == list, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cal_dbus_object, CALENDAR_ERROR_IPC);

	GVariant *arg_handle = cal_dbus_utils_handle_to_gvariant(handle);
	GVariant *arg_list = cal_dbus_utils_list_to_gvariant(list);
	cal_dbus_call_update_records_sync(cal_dbus_object, arg_handle, arg_list,
			&version, &ret, NULL, &error);
	g_variant_unref(arg_list);
	g_variant_unref(arg_handle);
	if (error) {
		/* LCOV_EXCL_START */
		ERR("cal_dbus_call_update_records_sync() Fail[%s]",  error->message);
		if (G_DBUS_ERROR_ACCESS_DENIED == error->code)
			ret = CALENDAR_ERROR_PERMISSION_DENIED;
		else
			ret = CALENDAR_ERROR_IPC;
		g_error_free(error);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	cal_client_handle_set_version(handle, version);

	return ret;
}

int cal_dbus_delete_records(calendar_h handle, const char *view_uri, int *ids, int count)
{
	GError *error = NULL;
	int ret = 0;
	int version = 0;

	RETV_IF(NULL == handle, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == view_uri, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == ids, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(0 == count, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cal_dbus_object, CALENDAR_ERROR_IPC);

	GVariant *arg_handle = cal_dbus_utils_handle_to_gvariant(handle);
	GVariant *arg_ids = cal_dbus_utils_ids_to_gvariant(ids, count);
	cal_dbus_call_delete_records_sync(cal_dbus_object, arg_handle, view_uri,
			arg_ids, count, &version, &ret, NULL, &error);
	g_variant_unref(arg_ids);
	g_variant_unref(arg_handle);
	if (error) {
		/* LCOV_EXCL_START */
		ERR("cal_dbus_call_delete_records_sync() Fail[%s]",  error->message);
		if (G_DBUS_ERROR_ACCESS_DENIED == error->code)
			ret = CALENDAR_ERROR_PERMISSION_DENIED;
		else
			ret = CALENDAR_ERROR_IPC;
		g_error_free(error);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	cal_client_handle_set_version(handle, version);

	return ret;
}

int cal_dbus_replace_records(calendar_h handle, calendar_list_h list, int *ids, int count)
{
	GError *error = NULL;
	int ret = 0;
	int version = 0;

	RETV_IF(NULL == handle, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == list, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == ids, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(0 == count, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cal_dbus_object, CALENDAR_ERROR_IPC);

	GVariant *arg_handle = cal_dbus_utils_handle_to_gvariant(handle);
	GVariant *arg_list = cal_dbus_utils_list_to_gvariant(list);
	GVariant *arg_ids = cal_dbus_utils_ids_to_gvariant(ids, count);
	cal_dbus_call_replace_records_sync(cal_dbus_object, arg_handle, arg_list,
			arg_ids, count, &version, &ret, NULL, &error);
	g_variant_unref(arg_list);
	g_variant_unref(arg_ids);
	g_variant_unref(arg_handle);
	if (error) {
		/* LCOV_EXCL_START */
		ERR("cal_dbus_call_replace_records_sync() Fail[%s]",  error->message);
		if (G_DBUS_ERROR_ACCESS_DENIED == error->code)
			ret = CALENDAR_ERROR_PERMISSION_DENIED;
		else
			ret = CALENDAR_ERROR_IPC;
		g_error_free(error);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	cal_client_handle_set_version(handle, version);

	return ret;
}

int cal_dbus_get_record(calendar_h handle, const char *view_uri, int id,
		calendar_record_h *out_record)
{
	GError *error = NULL;
	int ret = 0;

	RETV_IF(NULL == handle, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_record, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cal_dbus_object, CALENDAR_ERROR_IPC);

	DBG("uri[%s]", view_uri);
	GVariant *arg_handle = cal_dbus_utils_handle_to_gvariant(handle);
	GVariant *arg_record = NULL;
	cal_dbus_call_get_record_sync(cal_dbus_object, arg_handle, view_uri, id,
			&arg_record, &ret, NULL, &error);
	g_variant_unref(arg_handle);
	if (error) {
		/* LCOV_EXCL_START */
		ERR("cal_dbus_call_get_record_sync() Fail[%s]",  error->message);
		if (G_DBUS_ERROR_ACCESS_DENIED == error->code)
			ret = CALENDAR_ERROR_PERMISSION_DENIED;
		else
			ret = CALENDAR_ERROR_IPC;
		g_error_free(error);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	if (CALENDAR_ERROR_NONE != ret) {
		/* LCOV_EXCL_START */
		ERR("get_record() Fail(%d)", ret);
		g_variant_unref(arg_record);
		*out_record = NULL;
		return ret;
		/* LCOV_EXCL_STOP */
	}
	cal_dbus_utils_gvariant_to_record(arg_record, out_record);
	g_variant_unref(arg_record);

	return ret;
}

int cal_dbus_get_all_records(calendar_h handle, const char *view_uri,
		int offset, int limit, calendar_list_h *out_list)
{
	GError *error = NULL;
	int ret = 0;

	RETV_IF(NULL == handle, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_list, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cal_dbus_object, CALENDAR_ERROR_IPC);

	GVariant *arg_handle = cal_dbus_utils_handle_to_gvariant(handle);
	GVariant *arg_list = NULL;
	cal_dbus_call_get_all_records_sync(cal_dbus_object, arg_handle, view_uri,
			offset, limit, &arg_list, &ret, NULL, &error);
	g_variant_unref(arg_handle);
	if (error) {
		/* LCOV_EXCL_START */
		ERR("cal_dbus_call_get_all_records_sync() Fail[%s]", error->message);
		if (G_DBUS_ERROR_ACCESS_DENIED == error->code)
			ret = CALENDAR_ERROR_PERMISSION_DENIED;
		else
			ret = CALENDAR_ERROR_IPC;
		g_error_free(error);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	if (CALENDAR_ERROR_NONE != ret) {
		/* LCOV_EXCL_START */
		ERR("get_all_records() Fail(%d)", ret);
		g_variant_unref(arg_list);
		*out_list = NULL;
		return ret;
		/* LCOV_EXCL_STOP */
	}

	cal_dbus_utils_gvariant_to_list(arg_list, out_list);
	g_variant_unref(arg_list);

	if (*out_list)
		calendar_list_first(*out_list);

	return ret;
}

int cal_dbus_get_records_with_query(calendar_h handle, calendar_query_h query,
		int offset, int limit, calendar_list_h* out_list)
{
	GError *error = NULL;
	int ret = 0;

	RETV_IF(NULL == handle, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_list, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cal_dbus_object, CALENDAR_ERROR_IPC);

	GVariant *arg_handle = cal_dbus_utils_handle_to_gvariant(handle);
	GVariant *arg_query = cal_dbus_utils_query_to_gvariant(query);
	GVariant *arg_list = NULL;
	cal_dbus_call_get_records_with_query_sync(cal_dbus_object, arg_handle, arg_query,
			offset, limit, &arg_list, &ret, NULL, &error);
	g_variant_unref(arg_query);
	g_variant_unref(arg_handle);
	if (error) {
		/* LCOV_EXCL_START */
		ERR("cal_dbus_call_get_records_with_query_sync() Fail[%s]",  error->message);
		if (G_DBUS_ERROR_ACCESS_DENIED == error->code)
			ret = CALENDAR_ERROR_PERMISSION_DENIED;
		else
			ret = CALENDAR_ERROR_IPC;
		g_error_free(error);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	cal_dbus_utils_gvariant_to_list(arg_list, out_list);
	g_variant_unref(arg_list);

	if (*out_list)
		calendar_list_first(*out_list);

	return ret;
}

int cal_dbus_get_count(calendar_h handle, const char *view_uri, int *out_count)
{
	GError *error = NULL;
	int ret = 0;
	int count = 0;

	RETV_IF(NULL == handle, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_count, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cal_dbus_object, CALENDAR_ERROR_IPC);

	GVariant *arg_handle = cal_dbus_utils_handle_to_gvariant(handle);
	cal_dbus_call_get_count_sync(cal_dbus_object, arg_handle, view_uri,
			&count, &ret, NULL, &error);
	g_variant_unref(arg_handle);
	if (error) {
		/* LCOV_EXCL_START */
		ERR("cal_dbus_call_get_count_sync() Fail[%s]",  error->message);
		if (G_DBUS_ERROR_ACCESS_DENIED == error->code)
			ret = CALENDAR_ERROR_PERMISSION_DENIED;
		else
			ret = CALENDAR_ERROR_IPC;
		g_error_free(error);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	*out_count = count;

	return ret;
}

int cal_dbus_get_count_with_query(calendar_h handle, calendar_query_h query, int *out_count)
{
	GError *error = NULL;
	int ret = 0;
	int count = 0;

	RETV_IF(NULL == handle, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_count, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cal_dbus_object, CALENDAR_ERROR_IPC);

	GVariant *arg_handle = cal_dbus_utils_handle_to_gvariant(handle);
	GVariant *arg_query = cal_dbus_utils_query_to_gvariant(query);
	cal_dbus_call_get_count_with_query_sync(cal_dbus_object, arg_handle, arg_query,
			&count, &ret, NULL, &error);
	g_variant_unref(arg_query);
	g_variant_unref(arg_handle);
	if (error) {
		/* LCOV_EXCL_START */
		ERR("cal_dbus_call_get_count_with_query_sync() Fail[%s]",  error->message);
		if (G_DBUS_ERROR_ACCESS_DENIED == error->code)
			ret = CALENDAR_ERROR_PERMISSION_DENIED;
		else
			ret = CALENDAR_ERROR_IPC;
		g_error_free(error);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	*out_count = count;

	return ret;
}

int cal_dbus_add_changed_cb(calendar_h handle, const char* view_uri,
		void *callback, void* user_data)
{
	GError *error = NULL;
	int ret = 0;

	RETV_IF(NULL == handle, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == callback, CALENDAR_ERROR_INVALID_PARAMETER);

	cal_dbus_call_check_permission_write_sync(cal_dbus_object, NULL, &error);
	if (error) {
		/* LCOV_EXCL_START */
		ERR("cal_dbus_call_check_permission_write_sync() Fail[%s]", error->message);
		if (G_DBUS_ERROR_ACCESS_DENIED == error->code)
			ret = CALENDAR_ERROR_PERMISSION_DENIED;
		else
			ret = CALENDAR_ERROR_IPC;
		g_error_free(error);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	return cal_client_db_add_changed_cb(handle, view_uri, callback, user_data);
}

int cal_dbus_remove_changed_cb(calendar_h handle, const char* view_uri,
		void *callback, void* user_data)
{
	GError *error = NULL;
	int ret = 0;

	RETV_IF(NULL == handle, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == callback, CALENDAR_ERROR_INVALID_PARAMETER);

	cal_dbus_call_check_permission_write_sync(cal_dbus_object, NULL, &error);
	if (error) {
		/* LCOV_EXCL_START */
		ERR("cal_dbus_call_check_permission_write_sync() Fail[%s]", error->message);
		if (G_DBUS_ERROR_ACCESS_DENIED == error->code)
			ret = CALENDAR_ERROR_PERMISSION_DENIED;
		else
			ret = CALENDAR_ERROR_IPC;
		g_error_free(error);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	return cal_client_db_remove_changed_cb(handle, view_uri, callback, user_data);
}

int cal_dbus_get_current_version(calendar_h handle, int *out_version)
{
	GError *error = NULL;
	int ret = 0;
	int version = 0;

	RETV_IF(NULL == handle, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_version, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cal_dbus_object, CALENDAR_ERROR_IPC);

	GVariant *arg_handle = cal_dbus_utils_handle_to_gvariant(handle);
	cal_dbus_call_get_current_version_sync(cal_dbus_object, arg_handle,
			&version, &ret, NULL, &error);
	g_variant_unref(arg_handle);
	if (error) {
		/* LCOV_EXCL_START */
		ERR("cal_dbus_call_get_current_version_sync() Fail[%s]",  error->message);
		if (G_DBUS_ERROR_ACCESS_DENIED == error->code)
			ret = CALENDAR_ERROR_PERMISSION_DENIED;
		else
			ret = CALENDAR_ERROR_IPC;
		g_error_free(error);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	if (CALENDAR_ERROR_NONE != ret) {
		/* LCOV_EXCL_START */
		ERR("server return Fail(%d)", ret);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	*out_version = version;

	return ret;
}

int cal_dbus_get_last_change_version(calendar_h handle, int *out_version)
{
	GError *error = NULL;
	int ret = 0;

	RETV_IF(NULL == handle, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_version, CALENDAR_ERROR_INVALID_PARAMETER);

	cal_dbus_call_check_permission_read_sync(cal_dbus_object, NULL, &error);
	if (error) {
		/* LCOV_EXCL_START */
		ERR("cal_dbus_call_check_permission_read_sync() Fail[%s]", error->message);
		if (G_DBUS_ERROR_ACCESS_DENIED == error->code)
			ret = CALENDAR_ERROR_PERMISSION_DENIED;
		else
			ret = CALENDAR_ERROR_IPC;
		g_error_free(error);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	return cal_handle_get_version(handle, out_version);
}

int cal_dbus_get_changes_by_version(calendar_h handle, const char *view_uri,
		int book_id, int in_version, calendar_list_h *out_list, int *out_version)
{
	GError *error = NULL;
	int ret = 0;

	RETV_IF(NULL == handle, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == view_uri, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_list, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_version, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cal_dbus_object, CALENDAR_ERROR_IPC);

	GVariant *arg_handle = cal_dbus_utils_handle_to_gvariant(handle);
	GVariant *arg_list = NULL;
	cal_dbus_call_get_changes_by_version_sync(cal_dbus_object, arg_handle, view_uri,
			book_id, in_version, &arg_list, out_version, &ret, NULL, &error);
	g_variant_unref(arg_handle);
	if (error) {
		/* LCOV_EXCL_START */
		ERR("cal_dbus_call_get_changes_by_version_sync() Fail[%s]",  error->message);
		if (G_DBUS_ERROR_ACCESS_DENIED == error->code)
			ret = CALENDAR_ERROR_PERMISSION_DENIED;
		else
			ret = CALENDAR_ERROR_IPC;
		g_error_free(error);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	cal_dbus_utils_gvariant_to_list(arg_list, out_list);
	g_variant_unref(arg_list);

	if (*out_list)
		calendar_list_first(*out_list);

	return ret;
}

int cal_dbus_get_changes_exception_by_version(calendar_h handle, const char *view_uri,
		int original_id, int version, calendar_list_h *out_list)
{
	GError *error = NULL;
	int ret = 0;

	RETV_IF(NULL == handle, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == view_uri, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_list, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cal_dbus_object, CALENDAR_ERROR_IPC);

	GVariant *arg_handle = cal_dbus_utils_handle_to_gvariant(handle);
	GVariant *arg_list = NULL;
	cal_dbus_call_get_changes_exception_by_version_sync(cal_dbus_object, arg_handle,
			view_uri, original_id, version, &arg_list, &ret, NULL, &error);
	g_variant_unref(arg_handle);
	if (error) {
		/* LCOV_EXCL_START */
		ERR("cal_dbus_call_get_changes_exception_by_version_sync() Fail[%s]",  error->message);
		if (G_DBUS_ERROR_ACCESS_DENIED == error->code)
			ret = CALENDAR_ERROR_PERMISSION_DENIED;
		else
			ret = CALENDAR_ERROR_IPC;
		g_error_free(error);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	cal_client_handle_set_version(handle, version);

	cal_dbus_utils_gvariant_to_list(arg_list, out_list);
	g_variant_unref(arg_list);

	if (*out_list)
		calendar_list_first(*out_list);

	return ret;
}

int cal_dbus_clean_after_sync(calendar_h handle, int book_id, int version)
{
	GError *error = NULL;
	int ret = 0;

	RETV_IF(NULL == handle, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cal_dbus_object, CALENDAR_ERROR_IPC);

	GVariant *arg_handle = cal_dbus_utils_handle_to_gvariant(handle);
	cal_dbus_call_clean_after_sync_sync(cal_dbus_object, arg_handle, book_id, version,
			&ret, NULL, &error);
	g_variant_unref(arg_handle);
	if (error) {
		/* LCOV_EXCL_START */
		ERR("cal_dbus_call_clean_after_sync_sync() Fail[%s]",  error->message);
		if (G_DBUS_ERROR_ACCESS_DENIED == error->code)
			ret = CALENDAR_ERROR_PERMISSION_DENIED;
		else
			ret = CALENDAR_ERROR_IPC;
		g_error_free(error);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	return ret;
}

int cal_dbus_insert_vcalendars(calendar_h handle, const char *stream,
		int **out_ids, int *out_count)
{
	GError *error = NULL;
	int ret = 0;
	int version = 0;
	int count = 0;
	GVariant *arg_ids = NULL;

	RETV_IF(NULL == handle, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == stream, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_count, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cal_dbus_object, CALENDAR_ERROR_IPC);

	GVariant *arg_handle = cal_dbus_utils_handle_to_gvariant(handle);
	cal_dbus_call_insert_vcalendars_sync(cal_dbus_object, arg_handle, stream,
			&arg_ids, &count, &version, &ret, NULL, &error);
	g_variant_unref(arg_handle);
	if (error) {
		/* LCOV_EXCL_START */
		ERR("cal_dbus_call_insert_vcalendars_sync() Fail[%s]",  error->message);
		if (G_DBUS_ERROR_ACCESS_DENIED == error->code)
			ret = CALENDAR_ERROR_PERMISSION_DENIED;
		else
			ret = CALENDAR_ERROR_IPC;
		g_error_free(error);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	cal_client_handle_set_version(handle, version);

	int *ids = NULL;
	cal_dbus_utils_gvariant_to_ids(arg_ids, count, &ids);
	g_variant_unref(arg_ids);

	if (out_ids)
		*out_ids = ids;
	else
		free(ids);

	if (out_count)
		*out_count = count;

	return ret;
}

int cal_dbus_replace_vcalendars(calendar_h handle, const char *stream,
		int *ids, int count)
{
	GError *error = NULL;
	int ret = 0;
	int version = 0;

	RETV_IF(NULL == handle, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == stream, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == ids, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(0 == count, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cal_dbus_object, CALENDAR_ERROR_IPC);

	GVariant *arg_handle = cal_dbus_utils_handle_to_gvariant(handle);
	GVariant *arg_ids = cal_dbus_utils_ids_to_gvariant(ids, count);
	cal_dbus_call_replace_vcalendars_sync(cal_dbus_object, arg_handle, stream,
			arg_ids, count, &ret, &version, NULL, &error);
	g_variant_unref(arg_ids);
	g_variant_unref(arg_handle);
	if (error) {
		/* LCOV_EXCL_START */
		ERR("cal_dbus_call_replace_vcalendars_sync Fail[%s]",  error->message);
		if (G_DBUS_ERROR_ACCESS_DENIED == error->code)
			ret = CALENDAR_ERROR_PERMISSION_DENIED;
		else
			ret = CALENDAR_ERROR_IPC;
		g_error_free(error);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	cal_client_handle_set_version(handle, version);

	return ret;
}

