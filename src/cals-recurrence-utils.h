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
#ifndef _CALENDAR_SVC_RECURRENCE_UTILS_H_
#define _CALENDAR_SVC_RECURRENCE_UTILS_H_

#ifndef true
	#define true	1
	#define false	0
#endif

#ifndef TRUE
	#define TRUE	1
	#define FALSE	0
#endif

#ifndef BOOLEAN
	#define BOOLEAN int
#endif

#ifndef BOOL
	#define BOOL int
#endif


#ifndef __RECURRENCE_FEATURE__
	#define __RECURRENCE_FEATURE__

	#define CAL_SUPPORT_TIMEZONE
#ifdef CAL_SUPPORT_TIMEZONE
	#define CAL_SUPPORT_AUTO_DST  /* precondition: TIMEZONE should be supported */
#endif
	//#define CAL_SUPPORT_EXCEPTION_DATE
#endif

#define UINT8 unsigned char
#define UINT16 unsigned short
#define UINT32 unsigned long

#define INT8 signed char
#define INT16 signed short
#define INT32 signed long


#define is_leap_year(y) (((y)%4 == 0 && ((y)%100 != 0 || (y)%400 == 0))? 1 : 0)

typedef struct
{
  UINT8 seconds; // 0 - 59
  UINT8 minutes; // 0 - 59
  UINT8 hours;  // 0 - 23
}
cal_service_time_t;

//cal_service_date_t : Structure for expressing a calendar date
typedef struct
{
  UINT8  wday; // 0-6, 0=Sunday, 6=Saturday
  UINT8  day;    // 1 - 28, 29, 30, 31 depending on month and year
  UINT8  month;   // 1 - 12
  UINT16 year;     // Range of 32 years, starting with reference year
}cal_service_date_t;

//cal_service_time_date_t : Structure for expressing time and date as a single item.
typedef struct
{
  cal_service_time_t  time; // Time
  cal_service_date_t  date; // Date
}
cal_service_time_date_t;


#ifdef CAL_SUPPORT_AUTO_DST
typedef enum
{
  DB_STD_LOCAL_STD,   /* nothing */
  DB_STD_LOCAL_DST,   /* need adjust, after computation */
  DB_INDST_LOCAL_STD,  /* need adjust, after computation */
  DB_INDST_LOCAL_DST,  /* all datetime +1 hour */
  DB_OUTDST_LOCAL_STD, /* need adjust, after computation */
  DB_OUTDST_LOCAL_DST  /* nothing */

} cal_dst_type_t;


typedef struct
{
  UINT16 tz_index;
  UINT8 std_start_month;
  UINT8 std_start_position_of_week;
  UINT8 std_start_day;
  UINT8 std_start_hour;
  UINT8 day_start_month;
  UINT8 day_start_position_of_week;
  UINT8 day_start_day;
  UINT8 day_start_hour;
} cal_dst_info_t;
#endif


typedef struct
{
	struct tm 	start_db_date_time;
	struct tm 	end_db_date_time;
	struct tm  repeat_end_date;
	int dbtime_offset;
	int db_dst_offset;

	struct tm 	start_local_date_time;
	struct tm 	end_local_date_time;

	struct tm 	prev_alarm_time;
	UINT32  	day_for_last_day_event;

	UINT32  	db_time_zone_index;
	UINT32  	local_time_zone_index;

	int repeat_by_set_position;
	GList*	  exception_date_list;

#ifdef CAL_SUPPORT_AUTO_DST
  struct tm   start_db_dst_date_time;
  struct tm   start_db_std_date_time;
  struct tm   start_local_dst_date_time;
  struct tm   start_local_std_date_time;
  cal_dst_type_t is_need_adjust_time_for_view;
  INT32     i_adjust_time_value; /* 0,-1,+1 view하는 시점에 DST적용 여부에 맞게 시간 조정후 값 셋팅 */
#endif

} cal_date_param_t;

typedef BOOL (*CAL_GET_VALID_DATE_FUNC)(const cal_sch_full_t *,cal_date_param_t *,struct tm *,struct tm *);

BOOL cal_service_get_next_repeat_none_valid_date(const cal_sch_full_t *sch_record,cal_date_param_t *cal_date_param,
															struct tm *now_start_date,struct tm *now_end_date);
BOOL cal_service_get_next_repeat_daily_valid_date(const cal_sch_full_t *sch_record,cal_date_param_t *cal_date_param,
															struct tm *now_start_date,struct tm *now_end_date);
BOOL cal_service_get_next_repeat_weekly_valid_date(const cal_sch_full_t *sch_record,cal_date_param_t *cal_date_param,
															struct tm *now_start_date,struct tm *now_end_date);
BOOL cal_service_get_next_repeat_monthly_valid_date(const cal_sch_full_t *sch_record,cal_date_param_t *cal_date_param,
															struct tm *now_start_date,struct tm *now_end_date);
BOOL cal_service_get_next_repeat_yearly_valid_date(const cal_sch_full_t *sch_record,cal_date_param_t *cal_date_param,
															struct tm *now_start_date,struct tm *now_end_date);
void cal_service_set_date_param(cal_date_param_t *cal_date_param,cal_sch_full_t *sch_record);
void cal_service_set_day_of_week(struct tm* date);

void cal_svc_set_tz_base_info(int year);

INT8 __cal_service_compare_date(const struct tm *first_date,const struct tm *second_date);

int cal_db_service_get_next_valid_exception_time(const cal_sch_full_t *sch_record,
												cal_date_param_t *cal_date_param,GList *exception_date_list,
												struct tm* start_tm,struct tm* end_tm,
												struct tm* next_start_tm,struct tm* next_end_tm);


#endif /* _CALENDAR_SVC_RECURRENCE_UTILS_H_ */

