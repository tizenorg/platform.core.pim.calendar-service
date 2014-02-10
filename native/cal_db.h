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
#include <tzplatform_config.h> 

#ifndef __CALENDAR_SVC_DB_H__
#define __CALENDAR_SVC_DB_H__

#include "calendar_view.h"
#include "calendar_list.h"

#define CAL_DB_PATH tzplatform_mkpath(TZ_USER_DB,".calendar-svc.db")
#define CAL_DB_JOURNAL_PATH tzplatform_mkpath(TZ_USER_DB,".calendar-svc.db-journal")

// For Security
#define CAL_SECURITY_FILE_GROUP 6003
#define CAL_SECURITY_DEFAULT_PERMISSION 0660
#define CAL_SECURITY_DIR_DEFAULT_PERMISSION 0770

#define CAL_DB_SQL_MAX_LEN 2048
#define CAL_DB_SQL_MIN_LEN 1024

// DB table
#define CAL_TABLE_SCHEDULE "schedule_table"
#define CAL_TABLE_ALARM "alarm_table"
#define CAL_REMINDER_ALERT "reminder_table"
#define CAL_TABLE_CALENDAR "calendar_table"
#define CAL_TABLE_ATTENDEE "attendee_table"
#define CAL_TABLE_TIMEZONE "timezone_table"
#define CAL_TABLE_VERSION "version_table"
#define CAL_TABLE_DELETED "deleted_table"
#define CAL_TABLE_RRULE "rrule_table"
#define CAL_TABLE_NORMAL_INSTANCE "normal_instance_table"
#define CAL_TABLE_ALLDAY_INSTANCE "allday_instance_table"
#define CAL_TABLE_EXTENDED "extended_table"

// for event or todo..
#define CAL_VIEW_TABLE_EVENT "event_view"
#define CAL_VIEW_TABLE_TODO "todo_view"
// search ?
#define CAL_VIEW_TABLE_EVENT_CALENDAR "event_calendar_view"
#define CAL_VIEW_TABLE_TODO_CALENDAR "todo_calendar_view"
#define CAL_VIEW_TABLE_EVENT_CALENDAR_ATTENDEE "event_calendar_attendee_view"
#define CAL_VIEW_TABLE_NORMAL_INSTANCE "normal_instance_view"
#define CAL_VIEW_TABLE_ALLDAY_INSTANCE "allday_instance_view"

typedef int (*cal_db_insert_record_cb)( calendar_record_h record, int* id );
typedef int (*cal_db_get_record_cb)( int id, calendar_record_h* out_record );
typedef int (*cal_db_update_record_cb)( calendar_record_h record );
typedef int (*cal_db_delete_record_cb)( int id );
typedef int (*cal_db_get_all_records_cb)( int offset, int limit, calendar_list_h* out_list );
typedef int (*cal_db_get_records_with_query_cb)( calendar_query_h query, int offset, int limit, calendar_list_h* out_list );
typedef int (*cal_db_insert_records_cb)(const calendar_list_h in_list, int** ids);
typedef int (*cal_db_update_records_cb)(const calendar_list_h in_list);
typedef int (*cal_db_delete_records_cb)(int ids[], int count);
typedef int (*cal_db_get_count_cb)( int *out_count );
typedef int (*cal_db_get_count_with_query_cb)( calendar_query_h query, int *out_count );
typedef int (*cal_db_replace_record)(calendar_record_h record, int record_id);
typedef int (*cal_db_replace_records)(calendar_list_h record_list, int *record_id_array, int count);

typedef struct {
    bool is_query_only;
    cal_db_insert_record_cb insert_record;
    cal_db_get_record_cb get_record;
    cal_db_update_record_cb update_record;
    cal_db_delete_record_cb delete_record;
    cal_db_get_all_records_cb get_all_records;
    cal_db_get_records_with_query_cb get_records_with_query;
    cal_db_insert_records_cb insert_records;
    cal_db_update_records_cb update_records;
    cal_db_delete_records_cb delete_records;
    cal_db_get_count_cb get_count;
    cal_db_get_count_with_query_cb get_count_with_query;
    cal_db_replace_record replace_record;
    cal_db_replace_records replace_records;
} cal_db_plugin_cb_s;

int _cal_db_open(void);
int _cal_db_close(void);

#endif // __CALENDAR_SVC_DB_H__
