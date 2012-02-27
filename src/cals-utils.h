/*
 * Calendar Service
 *
 * Copyright (c) 2000 - 2012 Samsung Electronics Co., Ltd. All rights reserved.
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
#ifndef __CALENDAR_SVC_UTILS_H__
#define __CALENDAR_SVC_UTILS_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "db-util.h"
#include <glib.h>

//SQL operation type. May be used for other usage.
typedef enum
{
	CAL_SQL_TYPE_INSERT = 0,
	CAL_SQL_TYPE_UPDATE
} cal_sql_type_t;


/**
* @enum __cal_sch_field_t
* This enumeration Schedule database field index.
*/
typedef enum
{
	CAL_SCH_FIELD_INDEX = 0,					/**< This is an invalid type. */
	CAL_SCH_FIELD_TYPE,						/**< This is an calendar type. */
	CAL_SCH_FIELD_CATEGORY,					/**< This is an schedule category type. */
	CAL_SCH_FIELD_SUMMARY,					/**< This is an schedule summary */
	CAL_SCH_FIELD_DESCRIPTION,				/**< This is description */
	CAL_SCH_FIELD_LOCATION,					/**< This is location */
	CAL_SCH_FIELD_ALL_DAY_EVENT,				/**< This is all day event */
	CAL_SCH_FIELD_START_DATE_TIME,			/**< This is start date time */
	CAL_SCH_FIELD_END_DATE_TIME,			/**< This is end date time */
	CAL_SCH_FIELD_ALARM_TIME,				/**< This is alarm time */
	CAL_SCH_FIELD_REMIND_TICK,				/**< This is remind tick */
	CAL_SCH_FIELD_REMIND_TICK_UNIT,			/**< This is remind tick unit */
	CAL_SCH_FIELD_ALARM_ID,					/**< This is alarm id */
	CAL_SCH_FIELD_REPEAT_TERM,				/**< This is repeat term */
	CAL_SCH_FIELD_REPEAT_INTERVAL,			/**< Interval of repeat term */
	CAL_SCH_FIELD_REPEAT_END_DATE,			/**< This is repead end date */
	CAL_SCH_FIELD_REPEAT_SUN_MOON,			/**< Using sun or lunar calendar */
	CAL_SCH_FIELD_REPEAT_WEEK_START,		/**< Start day of a week */
	CAL_SCH_FIELD_REPEAT_WEEK_FLAG,			/**< Indicate which day is select in a week */
	CAL_SCH_FIELD_REPEAT_DAY_DATE,			/**< 0- for weekday(sun,mon,etc.) , 1 - for specific day(1,2.. Etc) */
	CAL_SCH_FIELD_MISSED,						/**< This is missed flag */
	CAL_SCH_FIELD_CALENDAR_TYPE,			/**< Calendar type */
	CAL_SCH_FIELD_TIME_ZOON,					/**< This is time zoon of calendar event */
  	CAL_SCH_FIELD_DST,						/**< This is dst of an event*/

	CAL_SCH_FIELD_CNT_MAX			/**< This is count max */
} __cal_sch_field_t;


typedef struct
{
	int	mname;	// month
	int	day;	// day count
} cal_month_tab;

bool cal_db_get_text_from_stmt(sqlite3_stmt * stmt,char * * p_str_dst,int column);

bool cal_db_get_blob_from_stmt(sqlite3_stmt * stmt,struct tm * p_tm,int column);

bool cal_db_service_convert_stmt_to_tz_info(sqlite3_stmt *stmt,cal_timezone_t * tz_info);

int cal_db_service_get_day_count_in_month(int input_year, int input_mon);

bool cal_db_service_get_current_time(struct tm * time_date);

bool cal_db_service_get_tomorrow(struct tm* tm);

bool cal_db_service_get_next_month(struct tm* tm);

void cal_db_service_copy_struct_tm(const struct tm *tm_src, struct tm *tm_des);

void cal_db_service_set_sch_weekflag(struct tm* date_time, char *week_flag);

bool cal_vcalendar_convert_tm_to_vdata_str(const struct tm * tm, char * utc_str);

void cal_vcalendar_convert_utc_str_to_tm(const char *szText, struct tm * tm);

bool cal_util_convert_query_string(const char *src, char *dst);

void cal_db_service_set_repeat_end_date(cal_sch_full_t *sch_record);

bool cal_db_service_convert_stmt_to_list_field_record(sqlite3_stmt *stmt,cal_sch_full_t *sch_record, bool is_utc);
bool cal_db_service_convert_stmt_to_month_field_record(sqlite3_stmt *stmt,int is_repeat,cal_sch_full_t *sch_record, bool is_utc);

cal_iter* cals_get_updated_list(int type, int calendar_id, time_t timestamp);

int cals_notify(cals_noti_type operation_type);
int cals_begin_trans(void);
int cals_end_trans(bool is_success);
const char* cals_noti_get_file_path(int type);


#endif /* __CALENDAR_SVC_UTILS_H__ */
