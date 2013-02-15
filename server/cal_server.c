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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>     //getuid
#include <pims-ipc.h>
#include <pims-ipc-svc.h>
#include <glib-object.h>

#include "calendar2.h"
#include "cal_ipc.h"
#include "cal_server_ipc.h"
#include "cal_typedef.h"
#include "cal_inotify.h"

#include "cal_db.h" // CAL_SECURITY_FILE_GROUP
#include "cal_internal.h" // DBG

#include "cal_server_alarm.h"
#include "cal_server_calendar_delete.h"

//static GMainLoop *loop;

static int __server_main();

static int __server_main(void)
{
	int ret;
    g_type_init();

    pims_ipc_svc_init(CAL_IPC_SOCKET_PATH,CAL_SECURITY_FILE_GROUP,CAL_SECURITY_DEFAULT_PERMISSION);

    if (pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_CONNECT, _cal_server_ipc_connect, NULL) != 0)
    {
        ERR("pims_ipc_svc_register error");
        return -1;
    }
    if (pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DISCONNECT, _cal_server_ipc_disconnect, NULL) != 0)
    {
        ERR("pims_ipc_svc_register error");
        return -1;
    }
    if (pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_INSERT_RECORD, _cal_server_ipc_db_insert_record, NULL) != 0)
    {
        ERR("pims_ipc_svc_register error");
        return -1;
    }
    if (pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_GET_RECORD, _cal_server_ipc_db_get_record, NULL) != 0)
    {
        ERR("pims_ipc_svc_register error");
        return -1;
    }
    if (pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_UPDATE_RECORD, _cal_server_ipc_db_update_record, NULL) != 0)
    {
        ERR("pims_ipc_svc_register error");
        return -1;
    }
    if (pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_DELETE_RECORD, _cal_server_ipc_db_delete_record, NULL) != 0)
    {
        ERR("pims_ipc_svc_register error");
        return -1;
    }
    if (pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_GET_ALL_RECORDS, _cal_server_ipc_db_get_all_records, NULL) != 0)
    {
        ERR("pims_ipc_svc_register error");
        return -1;
    }
    if (pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_GET_RECORDS_WITH_QUERY, _cal_server_ipc_db_get_records_with_query, NULL) != 0)
    {
        ERR("pims_ipc_svc_register error");
        return -1;
    }
    if (pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_CLEAN_AFTER_SYNC, _cal_server_ipc_db_clean_after_sync, NULL) != 0)
    {
        ERR("pims_ipc_svc_register error");
        return -1;
    }
    if (pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_GET_COUNT, _cal_server_ipc_db_get_count, NULL) != 0)
    {
        ERR("pims_ipc_svc_register error");
        return -1;
    }
    if (pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_GET_COUNT_WITH_QUERY, _cal_server_ipc_db_get_count_with_query, NULL) != 0)
    {
        ERR("pims_ipc_svc_register error");
        return -1;
    }
    if (pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_INSERT_RECORDS, _cal_server_ipc_db_insert_records, NULL) != 0)
    {
        ERR("pims_ipc_svc_register error");
        return -1;
    }
    if (pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_UPDATE_RECORDS, _cal_server_ipc_db_update_records, NULL) != 0)
    {
        ERR("pims_ipc_svc_register error");
        return -1;
    }
    if (pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_DELETE_RECORDS, _cal_server_ipc_db_delete_records, NULL) != 0)
    {
        ERR("pims_ipc_svc_register error");
        return -1;
    }
    if (pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_CHANGES_BY_VERSION, _cal_server_ipc_db_get_changes_by_version, NULL) != 0)
    {
        ERR("pims_ipc_svc_register error");
        return -1;
    }
    if (pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_GET_CURRENT_VERSION, _cal_server_ipc_db_get_current_version, NULL) != 0)
    {
        ERR("pims_ipc_svc_register error");
        return -1;
    }
    if (pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_INSERT_VCALENDARS, _cal_server_ipc_db_insert_vcalendars, NULL) != 0)
    {
        ERR("pims_ipc_svc_register error");
        return -1;
    }
    if (pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_REPLACE_VCALENDARS, _cal_server_ipc_db_replace_vcalendars, NULL) != 0)
    {
        ERR("pims_ipc_svc_register error");
        return -1;
    }
    if (pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_REPLACE_RECORD, _cal_server_ipc_db_replace_record, NULL) != 0)
    {
        ERR("pims_ipc_svc_register error");
        return -1;
    }
    if (pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_REPLACE_RECORDS, _cal_server_ipc_db_replace_records, NULL) != 0)
    {
        ERR("pims_ipc_svc_register error");
        return -1;
    }
    if (pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_REGISTER_REMINDER, _cal_server_ipc_db_register_reminder, NULL) != 0)
    {
        ERR("pims_ipc_svc_register error");
        return -1;
    }
    if (pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_UNREGISTER_REMINDER, _cal_server_ipc_db_unregister_reminder, NULL) != 0)
    {
        ERR("pims_ipc_svc_register error");
        return -1;
    }
    if (pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_ACTIVATE_REMINDER, _cal_server_ipc_db_activate_reminder, NULL) != 0)
    {
        ERR("pims_ipc_svc_register error");
        return -1;
    }
    if (pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_DEACTIVATE_REMINDER, _cal_server_ipc_db_deactivate_reminder, NULL) != 0)
    {
        ERR("pims_ipc_svc_register error");
        return -1;
    }
    if (pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_HAS_REMINDER, _cal_server_ipc_db_has_reminder, NULL) != 0)
    {
        ERR("pims_ipc_svc_register error");
        return -1;
    }

    //loop = g_main_loop_new(NULL, FALSE);

    //calendar_alarm_init();

	ret = calendar_connect();
	if (CALENDAR_ERROR_NONE != ret)
	{
		ERR("calendar_connect() failed");
		return ret;
	}

	_cal_inotify_initialize();
	ret = _cal_server_alarm();
	if (CALENDAR_ERROR_NONE != ret)
	{
		ERR("_cal_server_alarm() failed");
		return -1;
	}

	_cal_server_calendar_delete_start();

    pims_ipc_svc_run_main_loop(NULL);

	_cal_inotify_finalize();
	calendar_disconnect();

    pims_ipc_svc_deinit();

    return 0;
}

int main(int argc, char *argv[])
{
    __server_main();
    return 0;
}

