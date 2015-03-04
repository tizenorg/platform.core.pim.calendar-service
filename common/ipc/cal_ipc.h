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

#ifndef __CAL_IPC_H__
#define __CAL_IPC_H__

#define CAL_IPC_SERVICE                             "cal_svc_ipc"
#define CAL_IPC_SOCKET_PATH                         "/tmp/."CAL_IPC_SERVICE
#define CAL_IPC_SOCKET_PATH_FOR_SUBSCRIPTION        "/tmp/."CAL_IPC_SERVICE"_for_subscription"
#define CAL_IPC_MODULE                              "cal_ipc_module"
#define CAL_IPC_MODULE_FOR_SUBSCRIPTION             CAL_IPC_MODULE"_for_subscription"

#define CAL_IPC_SERVER_CONNECT                      "connect"
#define CAL_IPC_SERVER_DISCONNECT                   "disconnect"
#define CAL_IPC_SERVER_CHECK_PERMISSION				"check_permission"

#define CAL_IPC_SERVER_DB_INSERT_RECORD             "insert_record"
#define CAL_IPC_SERVER_DB_GET_RECORD                "get_record"
#define CAL_IPC_SERVER_DB_UPDATE_RECORD             "update_record"
#define CAL_IPC_SERVER_DB_DELETE_RECORD             "delete_record"
#define CAL_IPC_SERVER_DB_GET_ALL_RECORDS           "get_all_records"
#define CAL_IPC_SERVER_DB_GET_RECORDS_WITH_QUERY    "get_records_with_query"
#define CAL_IPC_SERVER_DB_CLEAN_AFTER_SYNC          "clean_after_sync"
#define CAL_IPC_SERVER_DB_GET_COUNT                 "get_count"
#define CAL_IPC_SERVER_DB_GET_COUNT_WITH_QUERY      "get_count_with_query"
#define CAL_IPC_SERVER_DB_INSERT_RECORDS            "insert_records"
#define CAL_IPC_SERVER_DB_UPDATE_RECORDS            "update_records"
#define CAL_IPC_SERVER_DB_DELETE_RECORDS            "delete_records"
#define CAL_IPC_SERVER_DB_CHANGES_BY_VERSION        "changes_by_version"
#define CAL_IPC_SERVER_DB_GET_CURRENT_VERSION       "get_current_version"
#define CAL_IPC_SERVER_DB_INSERT_VCALENDARS         "insert_vcalendars"
#define CAL_IPC_SERVER_DB_REPLACE_VCALENDARS        "replace_vcalendars"
#define CAL_IPC_SERVER_DB_REPLACE_RECORD            "replace_record"
#define CAL_IPC_SERVER_DB_REPLACE_RECORDS           "replace_records"
#define CAL_IPC_SERVER_DB_CHANGES_EXCEPTION         "changes_exception"

#ifdef CAL_MEMORY_TEST
#define CAL_IPC_SERVER_DESTROY                      "destroy"
#endif // #ifdef CAL_MEMORY_TEST
#endif /*__CAL_IPC_H__*/
