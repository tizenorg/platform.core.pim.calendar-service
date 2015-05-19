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
#include <string.h>
#include <glib.h>
#include <vasum.h>
#include <account.h>
#include <vconf.h>
#include <contacts.h>

#include "calendar_errors.h"
#include "cal_ipc.h"
#include "cal_internal.h"
#include "cal_server_zone.h"
#include "cal_zone.h"
#include "cal_calendar.h"
#include "cal_server_schema.h"
#include "cal_db.h"
#include "cal_db_calendar.h"
#include "cal_access_control.h"
#include "cal_server_alarm.h"
#include "cal_server_update.h"
#include "cal_server_contacts.h"
#include "cal_server_calendar_delete.h"
#include "cal_access_control.h"

static vsm_context_h g_ctx;
static GHashTable *cal_account_table = NULL;

bool _cal_server_account_delete_cb(const char* event_type, int account_id, void* user_data)
{
	CAL_FN_CALL();

	if (CAL_STRING_EQUAL == strcmp(event_type, ACCOUNT_NOTI_NAME_DELETE)) {
		char *zone_name = (char *)user_data;
		cal_db_delete_account(zone_name, account_id);
		cal_server_contacts_delete(zone_name, account_id);
	}
	return true;
}

static void __zone_iterate_cb(vsm_zone_h zone, void *user_data)
{
	CAL_FN_CALL();
	int ret = 0;

	/* try to connect zone with before launched service. */
	RET_IF(NULL == zone);
	const char *zone_name = NULL;
	zone_name = vsm_get_zone_name(zone);
	RET_IF(NULL == zone_name);

	if ('\0' != *zone_name) {
		DBG("--------[%s]", zone_name);
		if (vsm_is_host_zone(zone)) {
			DBG("host zone is callen in container environment");
			return;
		}
	}

	DBG("schema init");
	char path[1024] = {0};
	int len = 0;
	len = cal_zone_get_root_path(zone_name, path, sizeof(path));
	snprintf(path+len, sizeof(path)-len, "%s", "/opt/usr/dbspace/");
	cal_server_schema_check(path);
	cal_server_update(path);
	DBG("calendar-service connect");
	cal_connect(zone_name);
	cal_db_initialize_view_table(zone_name);
	DBG("contacts-serice");
	cal_server_contacts_init(zone);
	DBG("access_control");
	cal_access_control_set_client_info(zone_name, "calendar-service", NULL);

	DBG("join account");
	vsm_zone_h old_zone = vsm_join_zone(zone);
	do {
		account_subscribe_h account = NULL;
		if (NULL == cal_account_table)
			cal_account_table = g_hash_table_new_full(g_str_hash, g_str_equal, free, NULL);
		ret = account_subscribe_create(&account);
		if (ACCOUNT_ERROR_NONE != ret) {
			ERR("account_subscribe_create() Fail(%d)", ret);
			break;
		}
		g_hash_table_replace(cal_account_table, g_strdup(zone_name), account);
		ret = account_subscribe_notification(account, _cal_server_account_delete_cb, (void *)zone_name);
		if (ACCOUNT_ERROR_NONE != ret) {
			ERR("account_subscribe_notification() Fail(%d)", ret);
			break;
		}
	} while (0);
	vsm_join_zone(old_zone);

	DBG("alarm-manager register");
	cal_server_alarm_register(zone_name);
	DBG("calendar_delete_start");
	cal_server_calendar_delete_start(zone_name);
}

static int __status_cb(vsm_zone_h zone, vsm_zone_state_t state, void *user_data)
{
	CAL_FN_CALL();
	int ret = 0;
	RETV_IF(NULL == zone, CALENDAR_ERROR_INVALID_PARAMETER);

	const char *zone_name = NULL;
	zone_name = vsm_get_zone_name(zone);
	vsm_zone_h old_zone = NULL;

	int len = 0;
	char path[1024] = {0};
	switch (state) {
	case VSM_ZONE_STATE_STARTING:
		DBG("STARTING");
		break;
	case VSM_ZONE_STATE_STOPPED:
		DBG("STOPPED");
		break;
	case VSM_ZONE_STATE_RUNNING:
		DBG("RUNNING");
		if (NULL == zone_name)
			break;
		if ('\0' != *zone_name) {
			if (vsm_is_host_zone(zone))
				break;
		}

		DBG("schema init");
		len = cal_zone_get_root_path(zone_name, path, sizeof(path));
		snprintf(path+len, sizeof(path)-len, "%s", "/opt/usr/dbspace/");
		cal_server_schema_check(path);
		cal_server_update(path);
		DBG("calendar-service connect");
		cal_connect(zone_name);
		cal_db_initialize_view_table(zone_name);
		DBG("contacts-serice");
		cal_server_contacts_init(zone);
		DBG("access_control");
		cal_access_control_set_client_info(zone_name, "calendar-service", NULL);

		DBG("join account");
		old_zone = vsm_join_zone(zone);
		do {
			account_subscribe_h account = NULL;
			if (NULL == cal_account_table)
				cal_account_table = g_hash_table_new_full(g_str_hash, g_str_equal, free, NULL);
			ret = account_subscribe_create(&account);
			if (ACCOUNT_ERROR_NONE != ret) {
				ERR("account_subscribe_create() Fail(%d)", ret);
				break;
			}
			g_hash_table_replace(cal_account_table, g_strdup(zone_name), account);
			ret = account_subscribe_notification(account, _cal_server_account_delete_cb, (void *)zone_name);
			if (ACCOUNT_ERROR_NONE != ret) {
				ERR("account_subscribe_notification() Fail(%d)", ret);
				break;
			}
		} while (0);
		vsm_join_zone(old_zone);

		DBG("alarm-manager register");
		cal_server_alarm_register(zone_name);
		DBG("calendar_delete_start");
		cal_server_calendar_delete_start(zone_name);
		break;
	case VSM_ZONE_STATE_STOPPING:
		DBG("STOPPING");
		cal_disconnect(zone_name);

		old_zone = vsm_join_zone(zone);
		contacts_disconnect();
		vsm_join_zone(old_zone);
		/* account */
		if (cal_account_table) {
			account_subscribe_h account = NULL;
			account = g_hash_table_lookup(cal_account_table, zone_name);
			account_unsubscribe_notification(account);
			g_hash_table_remove(cal_account_table, zone_name);
		}
		break;
	case VSM_ZONE_STATE_ABORTING:
		DBG("ABORTING");
		break;
	case VSM_ZONE_STATE_FREEZING:
		DBG("FREEZING");
		break;
	case VSM_ZONE_STATE_FROZEN:
		DBG("FROZEN");
		break;
	case VSM_ZONE_STATE_THAWED:
		DBG("THAWED");
		break;
	default:
		DBG("###### invalid status");
		break;
	}
	return 0;
}

static gboolean __mainloop_cb(GIOChannel *channel, GIOCondition condition, void *data)
{
	vsm_context_h ctx = (vsm_context_h)data;
	vsm_enter_eventloop(ctx, 0, 0);
	return TRUE;
}

int cal_server_zone_initialize(void)
{
	CAL_FN_CALL();
	int ret = 0;

	if (g_ctx) {
		DBG("already existed");
		cal_server_zone_terminate();
	}
	vsm_context_h ctx = vsm_create_context();
	RETVM_IF(NULL == ctx, CALENDAR_ERROR_DB_FAILED, "vsm_create_context() Fail");

	ret = vsm_iterate_zone(ctx, __zone_iterate_cb, NULL); /* return value is handle */
	RETVM_IF(ret < 0, -1, "vsm_iterate_zone() Fail(%d)", ret);

	ret = vsm_add_state_changed_callback(ctx, __status_cb, NULL);

	GIOChannel *channel = NULL;
	int fd = vsm_get_poll_fd(ctx);
	channel = g_io_channel_unix_new(fd);
	g_io_add_watch(channel, G_IO_IN, __mainloop_cb, ctx);
	g_io_channel_unref(channel);

	g_ctx = ctx;

	return CALENDAR_ERROR_NONE;
}

void cal_server_zone_declare_link(void)
{
	CAL_FN_CALL();
	if (NULL == g_ctx) {
		cal_server_zone_initialize();
	}

	vsm_context_h ctx = g_ctx;

	int ret = 0;
	ret = vsm_declare_link(ctx, CAL_IPC_SOCKET_PATH, CAL_IPC_SOCKET_PATH);
	RETM_IF(ret < 0, "vsm_declare_link() Fail(%d)", ret);
	ret = vsm_declare_link(ctx, CAL_IPC_SOCKET_PATH_FOR_SUBSCRIPTION, CAL_IPC_SOCKET_PATH_FOR_SUBSCRIPTION);
	RETM_IF(ret < 0, "vsm_declare_link() Fail(%d)", ret);
}

void cal_server_zone_terminate(void)
{
	RET_IF(NULL == g_ctx);
	vsm_cleanup_context(g_ctx);
}

void cal_server_zone_get_zone_name(int pid, char **zone_name)
{
	if (NULL == g_ctx) {
		cal_server_zone_initialize();
	}
	RET_IF(pid < 0);
	RET_IF(NULL == zone_name);

	vsm_zone_h zone = NULL;
	zone = vsm_lookup_zone_by_pid(g_ctx, pid);
	RETM_IF(NULL == zone, "vsm_lookup_zone_by_pid() Fail:pid(%d)", pid);

	const char *name = NULL;
	name = vsm_get_zone_name(zone);
	if (NULL == name) {
		DBG("zone_name is NULL");
	}
	else {
		DBG("Container[%s] pid(%d)", name, pid);
		*zone_name = g_strdup(name);
	}
}

vsm_context_h cal_server_zone_get_context(void)
{
	return g_ctx;
}

static void _cal_server_zone_iterate_alarm_cb(vsm_zone_h zone, void *user_data)
{
	time_t t = (time_t)GPOINTER_TO_INT(user_data);
	if (t < 0)
		t = time(NULL);

	DBG("system changed time(%ld)", t);
	const char *zone_name = NULL;
	zone_name = vsm_get_zone_name(zone);
	/* check if alert time is matched */
	cal_server_alarm_alert(zone_name, t);
	cal_server_alarm_register_next_alarm(zone_name, t);
}

void cal_server_zone_iterate_alarm(time_t t)
{
	int ret = 0;
	vsm_context_h ctx = cal_server_zone_get_context();
	ret = vsm_iterate_zone(ctx, _cal_server_zone_iterate_alarm_cb, GINT_TO_POINTER(t));
	RETM_IF(ret < 0, "vsm_iterate_zone() Fail(%d)", ret);
}

