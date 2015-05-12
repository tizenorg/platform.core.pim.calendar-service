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
#include <unistd.h>     //getuid
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
#include "cal_db_calendar.h"
#include "cal_time.h" // u_clean

//static GMainLoop *loop;

static account_subscribe_h cal_account_h = NULL;
#ifdef CAL_MEMORY_TEST
GMainLoop* main_loop = NULL;
#endif //#ifdef CAL_MEMORY_TEST

static int __server_main();
static bool _cal_server_account_delete_cb(const char* event_type, int account_id, void* user_data);

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
		calendar_db_delete_account(account_id);
		cal_server_contacts_delete(account_id);
	}
	return true;
}

#ifdef CAL_MEMORY_TEST
static gboolean  _cal_server_ipc_destroy_idle(void* user_data)
{
	ERR();
	g_main_loop_quit(main_loop);
}

void cal_server_ipc_destroy(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	ERR();
	int ret = CALENDAR_ERROR_NONE;

	// kill daemon destroy
	g_timeout_add_seconds(1, &_cal_server_ipc_destroy_idle, NULL);

	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata) {
			ERR("pims_ipc_data_create Fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0) {
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("pims_ipc_data_put Fail");
			goto DATA_FREE;
		}

	}
	else
	{
		ERR("outdata is NULL");
	}
	goto DATA_FREE;

ERROR_RETURN:
	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata) {
			ERR("pims_ipc_data_create Fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0) {
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("pims_ipc_data_put Fail");
			goto DATA_FREE;
		}
	}
	else
	{
		ERR("outdata is NULL");
	}
DATA_FREE:

	return;
}
#endif //#ifdef CAL_MEMORY_TEST

static int __server_main(void)
{
	int ret;
	int on_contact = 0;
	int try_count = 0;
	g_type_init();

	char sock_file[CAL_PATH_MAX_LEN] = {0};
	snprintf(sock_file, sizeof(sock_file), CAL_SOCK_PATH"/.%s", getuid(), CAL_IPC_SERVICE);
	pims_ipc_svc_init(sock_file,CAL_SECURITY_FILE_GROUP, 0777);

	if (pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_CONNECT, cal_server_ipc_connect, NULL) != 0)
	{
		ERR("pims_ipc_svc_register error");
		return -1;
	}
	if (pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DISCONNECT, cal_server_ipc_disconnect, NULL) != 0)
	{
		ERR("pims_ipc_svc_register error");
		return -1;
	}
	if (pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_CHECK_PERMISSION, cal_server_ipc_check_permission, NULL) != 0) {
		ERR("pims_ipc_svc_register error");
		return -1;
	}
	if (pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_INSERT_RECORD, cal_server_ipc_db_insert_record, NULL) != 0)
	{
		ERR("pims_ipc_svc_register error");
		return -1;
	}
	if (pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_GET_RECORD, cal_server_ipc_db_get_record, NULL) != 0)
	{
		ERR("pims_ipc_svc_register error");
		return -1;
	}
	if (pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_UPDATE_RECORD, cal_server_ipc_db_update_record, NULL) != 0)
	{
		ERR("pims_ipc_svc_register error");
		return -1;
	}
	if (pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_DELETE_RECORD, cal_server_ipc_db_delete_record, NULL) != 0)
	{
		ERR("pims_ipc_svc_register error");
		return -1;
	}
	if (pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_GET_ALL_RECORDS, cal_server_ipc_db_get_all_records, NULL) != 0)
	{
		ERR("pims_ipc_svc_register error");
		return -1;
	}
	if (pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_GET_RECORDS_WITH_QUERY, cal_server_ipc_db_get_records_with_query, NULL) != 0)
	{
		ERR("pims_ipc_svc_register error");
		return -1;
	}
	if (pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_CLEAN_AFTER_SYNC, cal_server_ipc_db_clean_after_sync, NULL) != 0)
	{
		ERR("pims_ipc_svc_register error");
		return -1;
	}
	if (pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_GET_COUNT, cal_server_ipc_db_get_count, NULL) != 0)
	{
		ERR("pims_ipc_svc_register error");
		return -1;
	}
	if (pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_GET_COUNT_WITH_QUERY, cal_server_ipc_db_get_count_with_query, NULL) != 0)
	{
		ERR("pims_ipc_svc_register error");
		return -1;
	}
	if (pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_INSERT_RECORDS, cal_server_ipc_db_insert_records, NULL) != 0)
	{
		ERR("pims_ipc_svc_register error");
		return -1;
	}
	if (pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_UPDATE_RECORDS, cal_server_ipc_db_update_records, NULL) != 0)
	{
		ERR("pims_ipc_svc_register error");
		return -1;
	}
	if (pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_DELETE_RECORDS, cal_server_ipc_db_delete_records, NULL) != 0)
	{
		ERR("pims_ipc_svc_register error");
		return -1;
	}
	if (pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_CHANGES_BY_VERSION, cal_server_ipc_db_get_changes_by_version, NULL) != 0)
	{
		ERR("pims_ipc_svc_register error");
		return -1;
	}
	if (pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_GET_CURRENT_VERSION, cal_server_ipc_db_get_current_version, NULL) != 0)
	{
		ERR("pims_ipc_svc_register error");
		return -1;
	}
	if (pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_INSERT_VCALENDARS, cal_server_ipc_db_insert_vcalendars, NULL) != 0)
	{
		ERR("pims_ipc_svc_register error");
		return -1;
	}
	if (pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_REPLACE_VCALENDARS, cal_server_ipc_db_replace_vcalendars, NULL) != 0)
	{
		ERR("pims_ipc_svc_register error");
		return -1;
	}
	if (pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_REPLACE_RECORD, cal_server_ipc_db_replace_record, NULL) != 0)
	{
		ERR("pims_ipc_svc_register error");
		return -1;
	}
	if (pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_REPLACE_RECORDS, cal_server_ipc_db_replace_records, NULL) != 0)
	{
		ERR("pims_ipc_svc_register error");
		return -1;
	}
	if (pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_CHANGES_EXCEPTION, cal_server_ipc_db_changes_exception, NULL) != 0)
	{
		ERR("pims_ipc_svc_register error");
		return -1;
	}
#ifdef CAL_MEMORY_TEST
	if (pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DESTROY, cal_server_ipc_destroy, NULL) != 0)
	{
		ERR("pims_ipc_svc_register error");
		return -1;
	}
#endif //#ifdef CAL_MEMORY_TEST

	snprintf(sock_file, sizeof(sock_file), CAL_SOCK_PATH"/.%s_for_subscribe", getuid(), CAL_IPC_SERVICE);
	// for subscribe
	pims_ipc_svc_init_for_publish(sock_file, CAL_SECURITY_FILE_GROUP, CAL_SECURITY_DEFAULT_PERMISSION);

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
	ret = calendar_connect();
	if (CALENDAR_ERROR_NONE != ret)
	{
		ERR("calendar_connect() Failed");
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

	ret = cal_server_alarm();
	if (CALENDAR_ERROR_NONE != ret)
	{
		if (on_contact)
		{
//			contacts_disconnect();
		}
		ERR("cal_server_alarm() Failed");
//		return -1;
	}

	cal_server_calendar_delete_start();

#ifdef CAL_MEMORY_TEST
	main_loop = g_main_loop_new(NULL, FALSE);

	pims_ipc_svc_run_main_loop(main_loop);

	ERR("exit");
	g_main_loop_unref(main_loop);
#else  //#ifdef CAL_MEMORY_TEST
	pims_ipc_svc_run_main_loop(NULL);
#endif //#ifdef CAL_MEMORY_TEST

	cal_time_u_cleanup();
	cal_inotify_finalize();
	calendar_disconnect();

//	contacts_disconnect();
	on_contact = 0;

	if (cal_account_h)
	{
		account_unsubscribe_notification(cal_account_h);
		cal_account_h = NULL;
	}

	pims_ipc_svc_deinit_for_publish();
	pims_ipc_svc_deinit();

	cal_access_control_unset_client_info();
	alarmmgr_fini();

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
}

int main(int argc, char *argv[])
{
	CAL_FN_CALL();
	if (getuid() == 0)
	{        // root
		gid_t glist[] = {CAL_SECURITY_FILE_GROUP};
		if (setgroups(1, glist) < 0)        // client and server should have same Groups
		{
			ERR("setgroups() Failed");
		}
	}

	_cal_server_create_file();
	cal_server_schema_check();
	cal_server_update();

	__server_main();
	return 0;
}

