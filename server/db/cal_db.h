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
#include <tzplatform_config.h>

#ifndef __CAL_DB_H__
#define __CAL_DB_H__

#include "calendar_view.h"
#include "calendar_list.h"

#define CALS_DB_NAME ".calendar-svc.db"
#define CALS_JN_NAME ".calendar-svc.db-journal"
#define DB_PATH tzplatform_getenv(TZ_USER_DB)
#define DATA_PATH tzplatform_getenv(TZ_USER_DATA)
#define CAL_DB_FILE tzplatform_mkpath(TZ_USER_DB, ".calendar-svc.db")
#define CAL_JN_FILE tzplatform_mkpath(TZ_USER_DB, ".calendar-svc.db-journal")
#define CAL_DATA_PATH tzplatform_mkpath(TZ_USER_DATA,"calendar-svc")

/* For Security */
#define CAL_SECURITY_FILE_GROUP 5000
#define CAL_SECURITY_DEFAULT_PERMISSION 0660
#define CAL_SECURITY_DIR_DEFAULT_PERMISSION 0770

#define CAL_DB_SQL_MAX_LEN 2048
#define CAL_DB_SQL_MIN_LEN 1024
#define _BUFFER_ORDER 128

/* DB table */
#define CAL_TABLE_SCHEDULE "schedule_table"
#define CAL_TABLE_ALARM "alarm_table"
#define CAL_TABLE_CALENDAR "calendar_table"
#define CAL_TABLE_ATTENDEE "attendee_table"
#define CAL_TABLE_TIMEZONE "timezone_table"
#define CAL_TABLE_VERSION "version_table"
#define CAL_TABLE_DELETED "deleted_table"
#define CAL_TABLE_RRULE "rrule_table"
#define CAL_TABLE_NORMAL_INSTANCE "normal_instance_table"
#define CAL_TABLE_ALLDAY_INSTANCE "allday_instance_table"
#define CAL_TABLE_EXTENDED "extended_table"

/* for event or todo.. */
#define CAL_VIEW_TABLE_EVENT "event_view"
#define CAL_VIEW_TABLE_TODO "todo_view"
#define CAL_VIEW_TABLE_EVENT_CALENDAR "event_calendar_view"
#define CAL_VIEW_TABLE_TODO_CALENDAR "todo_calendar_view"
#define CAL_VIEW_TABLE_EVENT_CALENDAR_ATTENDEE "event_calendar_attendee_view"
#define CAL_VIEW_TABLE_NORMAL_INSTANCE "normal_instance_view"
#define CAL_VIEW_TABLE_ALLDAY_INSTANCE "allday_instance_view"
#define CAL_VIEW_TABLE_NORMAL_INSTANCE_EXTENDED "normal_instance_view_extended"
#define CAL_VIEW_TABLE_ALLDAY_INSTANCE_EXTENDED "allday_instance_view_extended"

#define CAL_QUERY_SCHEDULE_A_ALL "A.id, A.type, A.summary, A.description, A.location, A.categories, A.exdate, A.task_status, "\
				"A.priority, A.timezone, A.contact_id, A.busy_status, A.sensitivity, A.uid, A.organizer_name, "\
				"A.organizer_email, A.meeting_status, A.calendar_id, A.original_event_id, A.latitude, A.longitude, "\
				"A.email_id, A.created_time, A.completed_time, A.progress, A.changed_ver, A.created_ver, A.is_deleted, "\
				"A.dtstart_type, A.dtstart_utime, A.dtstart_datetime, A.dtstart_tzid, "\
				"A.dtend_type, A.dtend_utime, A.dtend_datetime, A.dtend_tzid, "\
				"A.last_mod, A.rrule_id, A.recurrence_id, A.rdate, A.has_attendee, A.has_alarm, A.system_type, A.updated, "\
				"A.sync_data1, A.sync_data2, A.sync_data3, A.sync_data4, A.has_exception, A.has_extended, A.freq, A.is_allday "

typedef int (*cal_db_get_record_cb)(int id, calendar_record_h* out_record);
typedef int (*cal_db_insert_record_cb)(calendar_record_h record, int* id);
typedef int (*cal_db_update_record_cb)(calendar_record_h record);
typedef int (*cal_db_delete_record_cb)(int id);
typedef int (*cal_db_replace_record_cb)(calendar_record_h record, int record_id);
typedef int (*cal_db_insert_records_cb)(const calendar_list_h in_list, int** ids);
typedef int (*cal_db_update_records_cb)(const calendar_list_h in_list);
typedef int (*cal_db_delete_records_cb)(int ids[], int count);
typedef int (*cal_db_replace_records_cb)(calendar_list_h record_list, int *record_id_array, int count);
typedef int (*cal_db_get_all_records_cb)(int offset, int limit, calendar_list_h* out_list);
typedef int (*cal_db_get_records_with_query_cb)(calendar_query_h query, int offset, int limit, calendar_list_h* out_list);
typedef int (*cal_db_get_count_cb)(int *out_count);
typedef int (*cal_db_get_count_with_query_cb)(calendar_query_h query, int *out_count);

typedef struct {
	bool is_query_only;
	cal_db_get_record_cb get_record;
	cal_db_insert_record_cb insert_record;
	cal_db_update_record_cb update_record;
	cal_db_delete_record_cb delete_record;
	cal_db_replace_record_cb replace_record;
	cal_db_insert_records_cb insert_records;
	cal_db_update_records_cb update_records;
	cal_db_delete_records_cb delete_records;
	cal_db_replace_records_cb replace_records;
	cal_db_get_all_records_cb get_all_records;
	cal_db_get_records_with_query_cb get_records_with_query;
	cal_db_get_count_cb get_count;
	cal_db_get_count_with_query_cb get_count_with_query;
} cal_db_plugin_cb_s;

void cal_db_initialize_view_table(void);
int cal_db_insert_record(calendar_record_h record, int* id);
int cal_db_update_record(calendar_record_h record);
int cal_db_delete_record(const char* view_uri, int id);
int cal_db_replace_record(calendar_record_h record, int record_id);
int cal_db_get_all_records(const char* view_uri, int offset, int limit, calendar_list_h* out_list);
int cal_db_get_records_with_query(calendar_query_h query, int offset, int limit, calendar_list_h* out_list);
int cal_db_clean_after_sync(int calendar_book_id,  int calendar_db_version);
int cal_db_get_record(const char* view_uri, int id, calendar_record_h* out_record);
int cal_db_get_count(const char* view_uri, int *out_count);
int cal_db_get_count_with_query(calendar_query_h query, int *out_count);
int cal_db_insert_records(calendar_list_h list, int** ids, int* count);
int cal_db_update_records(calendar_list_h list);
int cal_db_delete_records(const char* view_uri, int record_id_array[], int count);
int cal_db_replace_records(calendar_list_h list, int *ids, int count);
int cal_db_insert_vcalendars(const char* vcalendar_stream, int **record_id_array, int *count);
int cal_db_replace_vcalendars(const char* vcalendar_stream, int *record_id_array, int count);
int cal_db_get_current_version(int* current_version);
int cal_db_get_changes_by_version(const char* view_uri, int calendar_book_id, int calendar_db_version, calendar_list_h* record_list, int *current_calendar_db_version);
int cal_db_get_changes_exception_by_version(const char* view_uri, int original_event_id, int calendar_db_version, calendar_list_h* record_list);

int cal_db_append_string(char **dst, char *src);
cal_db_plugin_cb_s* _cal_db_get_plugin(cal_record_type_e type);

#endif /* __CAL_DB_H__ */
