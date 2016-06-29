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
#include <fcntl.h>
#include <glib-object.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <grp.h>
#include <sys/param.h>
#include <unistd.h>
#include <sys/param.h>
#include <unistd.h>

#include "calendar.h"
#include "cal_typedef.h"
#include "cal_internal.h"
#include "cal_inotify.h"
#include "cal_db.h"
#include "cal_access_control.h"
#include "cal_db_plugin_calendar_helper.h"
#include "cal_time.h"
#include "cal_server_alarm.h"
#include "cal_server_contacts.h"
#include "cal_server_calendar_delete.h"
#include "cal_server_schema.h"
#include "cal_server_update.h"
#include "cal_server_service.h"
#include "cal_server_account.h"
#include "cal_server_dbus.h"

#define CAL_TIMEOUT_FOR_DECLARE 1
#define CAL_TIMEOUT_FOR_DEFAULT 0

GMainLoop* main_loop = NULL;
static int cal_timeout = 0;

void cal_server_quit_loop(void)
{
	g_main_loop_quit(main_loop);
	main_loop = NULL;
}

static int _cal_server_init(void)
{
	int ret = 0;
#if !GLIB_CHECK_VERSION(2, 35, 0)
	g_type_init();
#endif

	ret = cal_connect();
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_connect() Failed");
		return ret;
	}

	cal_access_control_set_client_info(NULL, "calendar-service");

	cal_db_initialize_view_table();

	cal_server_calendar_delete_start();
	return CALENDAR_ERROR_NONE;
}

static void _cal_server_deinit(void)
{
	cal_server_calendar_delete_stop();
	cal_access_control_unset_client_info();
	cal_disconnect();
}

static int _cal_server_main(void)
{
	main_loop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(main_loop);

	g_main_loop_unref(main_loop);

	return 0;
}

static void _cal_server_create_directory(const char* directory, mode_t mode)
{
	if (0 == access(directory, F_OK))
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
		if (-1 == ret)
			printf("Failed to fchown\n");

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
}

int cal_server_get_timeout(void)
{
	return cal_timeout;
}

int main(int argc, char *argv[])
{
	INFO("---------------------[SERVER START]------------------------");
	if (getuid() == 0) { /* root */
		gid_t glist[] = {CAL_SECURITY_FILE_GROUP};
		if (setgroups(1, glist) < 0)
			ERR("setgroups() Failed");
	}

	cal_timeout = argc > 1 ? atoi(argv[1]) : CAL_TIMEOUT_FOR_DEFAULT;
	DBG("timeout set(%dsec)", cal_timeout);

	_cal_server_create_file();
	cal_server_schema_check();
	cal_server_update();

	_cal_server_init();
	cal_server_alarm_init();
	cal_server_account_init();
	cal_server_contacts_init();

	guint id;
	id = cal_server_dbus_init();

	_cal_server_main();

	cal_time_u_cleanup();
	cal_server_contacts_deinit();
	cal_server_account_deinit();
	cal_server_alarm_deinit();

	_cal_server_deinit();
	cal_server_dbus_deinit(id);
	cal_inotify_deinit();

	return 0;
}

