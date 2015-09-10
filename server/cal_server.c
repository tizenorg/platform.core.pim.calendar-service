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
#include <sys/types.h>
#include <unistd.h>
#include <glib-object.h>
#include <alarm.h>
#include <contacts.h>
#include <account.h>

#include "calendar.h"
#include "cal_internal.h" // DBG
#include "cal_ipc.h"
#include "cal_server_ipc.h"
#include "cal_inotify.h"
#include "cal_db.h" // CAL_SECURITY_FILE_GROUP
#include "cal_server_contacts.h"
#include "cal_server_alarm.h"
#include "cal_server_calendar_delete.h"
#include "cal_server_schema.h"
#include "cal_server_update.h"
#include "cal_access_control.h"
#include "cal_db_plugin_calendar_helper.h"
#include "cal_time.h"

#define CAL_TIMEOUT_FOR_DECLARE 1
#define CAL_TIMEOUT_FOR_DEFAULT 0

static account_subscribe_h cal_account_h = NULL;
GMainLoop* main_loop = NULL;
static int cal_timeout = 0;

void cal_server_quit_loop(void)
{
	g_main_loop_quit(main_loop);
	main_loop = NULL;
}

static gboolean _cal_server_timeout_cb(gpointer argv)
{
	int ret;
	int *try_count = (int *)argv;
	DBG("called count(%d)", *try_count);
	if (*try_count > 2)
	{
		ERR("Tried 3 times but Failed to contacts connect");
		return false;
	}

	ret = contacts_connect();
	if (CONTACTS_ERROR_NONE == ret)
	{
		DBG("contact connected");
	}
	else
	{
		ERR("Failed to connect (%d) times", *try_count + 1);
		*try_count += 1;
		return true;
	}

	ret = cal_server_contacts();
	return false;
}

static bool _cal_server_account_delete_cb(const char* event_type, int account_id, void* user_data)
{
	CAL_FN_CALL();

	if (CAL_STRING_EQUAL == strcmp(event_type, ACCOUNT_NOTI_NAME_DELETE))
	{
		cal_db_delete_account(account_id);
		cal_server_contacts_delete(account_id);
	}
	return true;
}

static void _cal_server_init(void)
{
	int ret;
	int on_contact = 0;
	int try_count = 0;
	g_type_init();


	//loop = g_main_loop_new(NULL, FALSE);

	//calendar_alarm_init();
/*
	ret = contacts_connect();
	if (CONTACTS_ERROR_NONE != ret)
	{
		ERR("contacts_connect() Failed");
		g_timeout_add_seconds(30, _cal_server_timeout_cb, (gpointer)&try_count);
	}
	else
	{
		DBG("contacts connected");
		on_contact = 1;
	}
*/
	ret = cal_connect();
	if (CALENDAR_ERROR_NONE != ret)
	{
		ERR("cal_connect() Failed");
		return ret;
	}

	cal_db_initialize_view_table();

	if (on_contact)
	{
/*		ret = cal_server_contacts();
		if (CALENDAR_ERROR_NONE != ret)
		{
			contacts_disconnect();
			ERR("cal_server_contacts() Failed");
			return -1;
		}

		cal_server_contacts_sync_start();
*/	}

	// access_control
	cal_access_control_set_client_info(NULL, NULL);

	ret = account_subscribe_create(&cal_account_h);
	if (ACCOUNT_ERROR_NONE == ret) {
		ret = account_subscribe_notification(cal_account_h, _cal_server_account_delete_cb, NULL);
		if (ACCOUNT_ERROR_NONE != ret) {
			DBG("account_subscribe_notification Failed (%d)", ret);
		}
	}
	else
		DBG("account_subscribe_create Failed (%d)", ret);

	cal_server_alarm_register();
	cal_server_calendar_delete_start();
}

static void _cal_server_deinit(void)
{
	cal_disconnect();

	if (cal_account_h) {
		account_unsubscribe_notification(cal_account_h);
		cal_account_h = NULL;
	}

	cal_access_control_unset_client_info();
}

static int _cal_server_main(void)
{
	main_loop = g_main_loop_new(NULL, FALSE);
	cal_server_ipc_run(main_loop);

	g_main_loop_unref(main_loop);

	return 0;
}

static void _cal_server_create_directory(const char* directory, mode_t mode)
{
	int ret = 0;
	if (0 == access (directory, F_OK))
		return;

	DBG("No directory[%s]", directory);
	mkdir(directory, mode);
}

static void _cal_server_set_directory_permission(const char* file, mode_t mode)
{
	DBG("set permission[%s]", file);
	int fd, ret;
	fd = creat(file, mode);
	if (0 <= fd) {
		ret = fchown(fd, -1, CAL_SECURITY_FILE_GROUP);
		if (-1 == ret) {
			printf("Failed to fchown\n");
			return;
		}
		close(fd);
	}
}

static void _cal_server_create_file(void)
{
	_cal_server_create_directory(DATA_PATH, 0775);
	_cal_server_create_directory(CAL_DATA_PATH, 0775);

	_cal_server_set_directory_permission(CAL_NOTI_CALENDAR_CHANGED, 0660);
	_cal_server_set_directory_permission(CAL_NOTI_EVENT_CHANGED, 0660);
	_cal_server_set_directory_permission(CAL_NOTI_TODO_CHANGED, 0660);
	_cal_server_set_directory_permission(CAL_NOTI_IPC_READY, 0660);
}

int cal_server_get_timeout(void)
{
	return cal_timeout;
}

int main(int argc, char *argv[])
{
	INFO("server start");
	if (getuid() == 0) {
		gid_t glist[] = {CAL_SECURITY_FILE_GROUP};
		if (setgroups(1, glist) < 0)
			ERR("setgroups() Failed");
	}

	cal_timeout = argc > 1 ? atoi(argv[1]) : CAL_TIMEOUT_FOR_DEFAULT;
	DBG("timeout set(%dsec)", cal_timeout);

	_cal_server_create_file();
	cal_server_schema_check();
	cal_server_alarm_init();
	cal_server_update();
	_cal_server_init();
	cal_server_ipc_init();

	_cal_server_main();

	cal_time_u_cleanup();

	cal_server_ipc_deinit();
	_cal_server_deinit();
	cal_server_alarm_deinit();
	cal_inotify_deinit();

	return 0;
}

