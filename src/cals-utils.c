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
#include <unistd.h>
#include <fcntl.h>

#include "cals-typedef.h"
#include "cals-utils.h"
#include "cals-recurrence-utils.h"
#include "cals-db.h"
#include "cals-db-info.h"
#include "cals-internal.h"
#include "cals-tz-utils.h"
#include "cals-sqlite.h"

extern sqlite3* calendar_db_handle;

typedef	enum
{
	CAL_SCH_TERM_ONE_DAY,
	CAL_SCH_TERM_OVER_2_DAYS,
	CAL_SCH_TERM_OVER_7_DAYS,
	CAL_SCH_TERM_OVER_1_MONTH,
	CAL_SCH_TERM_OVER_1_YEAR,
	CAL_SCH_TERM_ERROR
} __cal_sch_term_status_t;

static cal_month_tab g_month_day[] = { {1,31}, {2,28}, {3,31}, {4,30}, {5,31}, {6,30}, {7,31}, {8,31}, {9,30}, {10,31}, {11,30}, {12,31} };

static const char *CALS_NOTI_EVENT_CHANGED="/opt/data/calendar-svc/.CALENDAR_SVC_EVENT_CHANGED";
static const char *CALS_NOTI_TODO_CHANGED="/opt/data/calendar-svc/.CALENDAR_SVC_TODO_CHANGED";
static const char *CALS_NOTI_CALENDAR_CHANGED="/opt/data/calendar-svc/.CALENDAR_SVC_CALENDAR_CHANGED";

static int transaction_cnt = 0;

static bool event_change=false;
static bool todo_change=false;
static bool calendar_change=false;

static inline void _cals_notify_event_change(void)
{
	int fd = open(CALS_NOTI_EVENT_CHANGED, O_TRUNC | O_RDWR);
	if (0 <= fd) {
		close(fd);
		event_change = false;
	}
}

static inline void _cals_notify_todo_change(void)
{
	int fd = open(CALS_NOTI_TODO_CHANGED, O_TRUNC | O_RDWR);
	if (0 <= fd) {
		close(fd);
		todo_change = false;
	}
}

static inline void _cals_notify_calendar_change(void)
{
	int fd = open(CALS_NOTI_CALENDAR_CHANGED, O_TRUNC | O_RDWR);
	if (0 <= fd) {
		close(fd);
		calendar_change = false;
	}
}

const char* cals_noti_get_file_path(int type)
{
	const char *noti;
	switch (type)
	{
	case CALS_NOTI_TYPE_EVENT:
		noti = CALS_NOTI_EVENT_CHANGED;
		break;
	case CALS_NOTI_TYPE_TODO:
		noti = CALS_NOTI_TODO_CHANGED;
		break;
	case CALS_NOTI_TYPE_CALENDAR:
		noti = CALS_NOTI_CALENDAR_CHANGED;
		break;
	default:
		ERR("The type(%d) is not supported", type);
		return NULL;
	}

	return noti;
}

int cals_notify(cals_noti_type type)
{
	if (0 < transaction_cnt) {
		switch (type) {
		case CALS_NOTI_TYPE_EVENT:
			event_change = true;
			break;
		case CALS_NOTI_TYPE_TODO:
			todo_change = true;
			break;
		case CALS_NOTI_TYPE_CALENDAR:
			calendar_change = true;
			break;
		default:
			ERR("The type(%d) is not supported", type);
			return CAL_ERR_ARG_INVALID;
		}
		return CAL_SUCCESS;
	}

	switch(type) {
	case CALS_NOTI_TYPE_EVENT:
		_cals_notify_event_change();
		break;
	case CALS_NOTI_TYPE_TODO:
		_cals_notify_todo_change();
		break;
	case CALS_NOTI_TYPE_CALENDAR:
		_cals_notify_calendar_change();
		break;
	default:
		ERR("The type(%d) is not supported", type);
		return CAL_ERR_ARG_INVALID;
	}

	return CAL_SUCCESS;
}

#define CAL_COMMIT_TRY_MAX 500000
int cals_begin_trans(void)
{
	if(transaction_cnt <= 0)
	{
		int ret, progress;

		progress = 100000;
		ret = cals_query_exec("BEGIN IMMEDIATE TRANSACTION");
		while(CAL_ERR_DB_LOCK == ret && progress < CAL_COMMIT_TRY_MAX) {
			usleep(progress);
			ret = cals_query_exec("BEGIN IMMEDIATE TRANSACTION");
			progress *= 2;
		}
		retvm_if(CAL_SUCCESS != ret, ret, "cals_query_exec() Failed(%d)", ret);

		transaction_cnt = 0;
	}
	transaction_cnt++;
	CALS_DBG("transaction_cnt : %d", transaction_cnt);

	return CAL_SUCCESS;
}


static inline void _cals_cancel_changes(void)
{
	event_change = false;
	calendar_change = false;
	todo_change = false;
}


int cals_end_trans(bool is_success)
{
	int ret;
	int progress = 0;

	transaction_cnt--;

	if (0 != transaction_cnt) {
		CALS_DBG("transaction_cnt : %d", transaction_cnt);
		return CAL_SUCCESS;
	}

	if (false == is_success) {
		_cals_cancel_changes();
		ret = cals_query_exec("ROLLBACK TRANSACTION");
		return CAL_SUCCESS;
	}

	progress = 400000;
	ret = cals_query_exec("COMMIT TRANSACTION");
	while (CAL_ERR_DB_LOCK == ret && progress < CAL_COMMIT_TRY_MAX) {
		usleep(progress);
		ret = cals_query_exec("COMMIT TRANSACTION");
		progress *= 2;
	}
	if (CAL_SUCCESS != ret) {
		int tmp_ret;
		ERR("cals_query_exec() Failed(%d)", ret);
		_cals_cancel_changes();
		tmp_ret = cals_query_exec("ROLLBACK TRANSACTION");
		warn_if(CAL_SUCCESS != tmp_ret, "cals_query_exec(ROLLBACK) Failed(%d).", tmp_ret);
		return ret;
	}
	if (event_change) _cals_notify_event_change();
	if (todo_change) _cals_notify_todo_change();
	if (calendar_change) _cals_notify_calendar_change();

	return CAL_SUCCESS;
}

API int calendar_svc_begin_trans(void)
{
	CALS_FN_CALL;
	return cals_begin_trans();
}

API int calendar_svc_end_trans()
{
	CALS_FN_CALL;
	return cals_end_trans(true);
}


int cal_db_service_get_day_count_in_month(int input_year, int input_mon)
{
	input_year = input_year + BENCHMARK_YEAR;
	input_mon = input_mon + 1;

	if ((input_year % 4) == 0) {
		if ((input_year % 100) == 0) {
			if ((input_year % 400) == 0)
				g_month_day[1].day = 29;
			else
				g_month_day[1].day = 28;
		} else {
			g_month_day[1].day = 29;
		}
	} else {
		g_month_day[1].day = 28;
	}

	return g_month_day[(input_mon - 1)].day;
}


static bool cal_get_day_count_in_month(int year, int month, int *count, int *error_code)
{
	assert((count != NULL) && (error_code != NULL));

	if(year < CAL_YEAR_MIN || CAL_YEAR_MAX < year) {
		ERR("year is invalid.");
		*error_code = CAL_ERR_ARG_INVALID;
		return false;
	}

	if(month < CAL_MONTH_CNT_MIN || CAL_MONTH_CNT_MAX < month) {
		ERR("month is invalid.");
		*error_code = CAL_ERR_ARG_INVALID;
		return false;
	}

	year = year - BENCHMARK_YEAR;
	month = month - 1;

	*count = cal_db_service_get_day_count_in_month(year, month);
	return true;
}

void cal_db_service_set_repeat_end_date(cal_sch_full_t *sch_record)
{
	int i = 0;
	struct tm start_tm={0};
	struct tm end_tm={0};
	struct tm next_valid_start_tm={0};
	struct tm next_valid_end_tm={0};
	cal_date_param_t cal_date_param;
	memset(&cal_date_param, 0x00, sizeof(cal_date_param_t));

	start_tm.tm_year = TM_YEAR_MIN;
	start_tm.tm_mon = MONTH_MIN;
	start_tm.tm_mday = MONTH_DAY_MIN;

	end_tm.tm_year = TM_YEAR_MAX;
	end_tm.tm_mon = MONTH_MAX;
	end_tm.tm_mday = MONTH_DAY_MAX;

	sch_record->repeat_end_date.tm_year = TM_YEAR_MAX-1;
	sch_record->repeat_end_date.tm_mon = MONTH_MAX;
	sch_record->repeat_end_date.tm_mday = MONTH_DAY_MAX;

	cal_service_set_date_param(&cal_date_param, sch_record);

	for(i = 0;i<sch_record->repeat_occurrences; i++)
	{
		cal_db_service_get_next_valid_exception_time(sch_record,
			&cal_date_param,NULL,&start_tm,&end_tm,&next_valid_start_tm,&next_valid_end_tm);
	}

	memcpy(&sch_record->repeat_end_date, &next_valid_start_tm, sizeof(struct tm));
	ERR("sch_record->repeat_end_date:%d-%d-%d",sch_record->repeat_end_date.tm_year,sch_record->repeat_end_date.tm_mon,sch_record->repeat_end_date.tm_mday);
}


bool cal_db_get_text_from_stmt(sqlite3_stmt *stmt,char** p_str_dst,int column)
{
	char *str_temp = NULL;
	int str_len = 0;

	str_temp = (char *)sqlite3_column_text(stmt, column);
	str_len = sqlite3_column_bytes(stmt, column);

	if(0 == str_len)
	{
		*p_str_dst = NULL;
		return true;
	}

	CAL_FREE(*p_str_dst);


	*p_str_dst = (char*)malloc(str_len+1);
	retvm_if(NULL == *p_str_dst, false, "failed to malloc str_dst");

	memcpy(*p_str_dst,str_temp,str_len);
	(*p_str_dst)[str_len] = '\0';

	return true;
}

bool cal_db_get_blob_from_stmt(sqlite3_stmt *stmt,struct tm* p_tm,int column)
{
	struct tm* temp = NULL;

	if(NULL == p_tm)
	{
		return false;
	}

	temp = (struct tm*)sqlite3_column_blob(stmt, column);
	if(NULL == temp)
	{
		return false;
	}

	memcpy(p_tm,temp,sizeof(struct tm));
	return true;
}


bool cal_db_service_convert_stmt_to_tz_info(sqlite3_stmt *stmt,cal_timezone_t * tz_info)
{
	CALS_FN_CALL;

	int count = 0;

	tz_info->index = sqlite3_column_int(stmt, count++);
	tz_info->tz_offset_from_gmt = sqlite3_column_int(stmt, count++);

	cal_db_get_text_from_stmt(stmt,&(tz_info->standard_name),count++);

	tz_info->std_start_month = sqlite3_column_int(stmt, count++);
	tz_info->std_start_position_of_week = sqlite3_column_int(stmt, count++);
	tz_info->std_start_day = sqlite3_column_int(stmt, count++);
	tz_info->std_start_hour = sqlite3_column_int(stmt, count++);
	tz_info->standard_bias = sqlite3_column_int(stmt, count++);

	cal_db_get_text_from_stmt(stmt,&(tz_info->day_light_name),count++);

	tz_info->day_light_start_month = sqlite3_column_int(stmt, count++);
	tz_info->day_light_start_position_of_week = sqlite3_column_int(stmt, count++);
	tz_info->day_light_start_day = sqlite3_column_int(stmt, count++);
	tz_info->day_light_start_hour = sqlite3_column_int(stmt, count++);
	tz_info->day_light_bias = sqlite3_column_int(stmt, count++);

	return true;
}


bool cal_db_service_convert_stmt_to_list_field_record(sqlite3_stmt *stmt,cal_sch_full_t *sch_record, bool is_utc)
{
	assert(sch_record != NULL);
	int i = 0;
	long int temp = 0;

	sch_record->index = sqlite3_column_int(stmt, i++);
	cal_db_get_text_from_stmt(stmt,&(sch_record->summary),i++);
	cal_db_get_text_from_stmt(stmt,&(sch_record->location),i++);

	sch_record->all_day_event = sqlite3_column_int(stmt, i++);

	temp = sqlite3_column_int(stmt, i++);
	cal_db_service_copy_struct_tm((struct tm*)cals_tmtime(&temp),&(sch_record->start_date_time));
	temp = sqlite3_column_int(stmt, i++);
	cal_db_service_copy_struct_tm((struct tm*)cals_tmtime(&temp),&(sch_record->end_date_time));

	sch_record->repeat_term = sqlite3_column_int(stmt, i++);

	sch_record->week_start = sqlite3_column_int(stmt, i++);
	cal_db_get_text_from_stmt(stmt,&(sch_record->week_flag),i++);

	sch_record->calendar_id = sqlite3_column_int(stmt, i++);

	return true;

}

bool cal_db_service_convert_stmt_to_month_field_record(sqlite3_stmt *stmt,int is_repeat,cal_sch_full_t *sch_record, bool is_utc)
{
	assert(sch_record != NULL);
	int i = 0;
	long int temp = 0;

	sch_record->index = sqlite3_column_int(stmt, i++);
	sch_record->all_day_event = sqlite3_column_int(stmt, i++);

	temp = sqlite3_column_int(stmt, i++);
	cal_db_service_copy_struct_tm((struct tm*)cals_tmtime(&temp),&(sch_record->start_date_time));
	temp = sqlite3_column_int(stmt, i++);
	cal_db_service_copy_struct_tm((struct tm*)cals_tmtime(&temp),&(sch_record->end_date_time));

	if(is_repeat)
	{
		sch_record->repeat_term = sqlite3_column_int(stmt, i++);
		sch_record->repeat_interval = sqlite3_column_int(stmt, i++);
		sch_record->repeat_occurrences = sqlite3_column_int(stmt, i++);
		temp = sqlite3_column_int(stmt, i++);
		cal_db_service_copy_struct_tm((struct tm*)cals_tmtime(&temp),&(sch_record->repeat_end_date));
		sch_record->week_start = sqlite3_column_int(stmt, i++);
		cal_db_get_text_from_stmt(stmt,&(sch_record->week_flag),i++);
		sch_record->day_date = sqlite3_column_int(stmt, i++);
		sch_record->timezone = sqlite3_column_int(stmt, i++);
		cal_db_get_text_from_stmt(stmt,&(sch_record->tz_name),i++);
	}

	return true;

}


#define MONTH_MAX 11

// week day from 0--6
#define MAX_WEEK_DAY 7

#define MONTH_DAY_MAX 31
#define MONTH_DAY_MIN 1

#define MONTH_MIN 0

#define TM_YEAR_MIN 0
#define TM_YEAR_MAX 138
#define TM_YEAR_OFFSET 1900

#define TIME_HOUR_MAX_12	12
#define TIME_HOUR_MAX_24	23

#define TIME_ZONE_COUNT 28

typedef enum
{
	OEG_CALENDAR_ITEM_MEETING_CATEGORY = 0,
	OEG_CALENDAR_ITEM_ATTENDEE_LIST
}OEG_CALENDAR_ITEM_TYPE;


	static bool
__cal_db_service_get_next_month_first_day(struct tm* tm)
{
	tm->tm_mon++;
	tm->tm_mday = MONTH_DAY_MIN;

	return TRUE;

}

	static bool
__cal_db_service_get_next_year_first_day(struct tm* tm)
{
	tm->tm_year++;
	tm->tm_mon = MONTH_MIN;
	tm->tm_mday = MONTH_DAY_MIN;

	return TRUE;

}

	bool
cal_db_service_get_tomorrow(struct tm* tm)
{
	int day_count = 0;
	int error_code = 0;

	int year = tm->tm_year + TM_YEAR_OFFSET;
	int month = tm->tm_mon + 1;

	cal_get_day_count_in_month(year, month, &day_count, &error_code);

	if (tm->tm_mday < day_count)
	{
		tm->tm_mday++;
	}
	else
	{
		if ( tm->tm_mon < MONTH_MAX)
		{
			__cal_db_service_get_next_month_first_day(tm);
		}
		else
		{
			// to next year the first day
			__cal_db_service_get_next_year_first_day(tm);
		}
	}

	return TRUE;

}

	bool
cal_db_service_get_next_month(struct tm* tm)
{
	if (tm->tm_mon == 11)
	{
		if (tm->tm_year == TM_YEAR_MAX-1)
		{
			tm->tm_mday = 31;
		}
		else
		{
			tm->tm_mon = 0;
			tm->tm_year ++;
		}
	}
	else
	{
		tm->tm_mon ++;
	}

	return true;
}

void cal_db_service_copy_struct_tm(const struct tm *tm_src, struct tm *tm_des)
{
	ret_if(NULL == tm_src);
	ret_if(NULL == tm_des);

	tm_des->tm_sec = tm_src->tm_sec;
	tm_des->tm_min = tm_src->tm_min;
	tm_des->tm_hour = tm_src->tm_hour;
	tm_des->tm_mday = tm_src->tm_mday;
	tm_des->tm_wday = tm_src->tm_wday;

	tm_des->tm_mon = tm_src->tm_mon;
	tm_des->tm_year = tm_src->tm_year;

	tm_des->tm_yday = tm_src->tm_yday;
	tm_des->tm_isdst = tm_src->tm_isdst;
}


bool cal_db_service_get_current_time(struct tm * time_date)
{
	assert(time_date != NULL);

	time_t t = time(NULL);

	tzset();
	localtime_r(&t,time_date);

	return TRUE;
}


	void
cal_db_service_set_sch_weekflag(struct tm* date_time, char *week_flag)
{
	GDate cur_date={0,};

	g_date_set_dmy(&cur_date, (GDateDay) date_time->tm_mday, (GDateMonth) (
				date_time->tm_mon + 1), (GDateYear) (date_time->tm_year + TM_YEAR_OFFSET));

	date_time->tm_wday = g_date_get_weekday (&cur_date);

	switch (date_time->tm_wday)
	{
	case 7:
		memcpy(week_flag,"1000000",DAY_OF_A_WEEK);
		break;
	case 1:
		memcpy(week_flag,"0100000",DAY_OF_A_WEEK);
		break;
	case 2:
		memcpy(week_flag,"0010000",DAY_OF_A_WEEK);
		break;
	case 3:
		memcpy(week_flag,"0001000",DAY_OF_A_WEEK);
		break;
	case 4:
		memcpy(week_flag,"0000100",DAY_OF_A_WEEK);
		break;
	case 5:
		memcpy(week_flag,"0000010",DAY_OF_A_WEEK);
		break;
	case 6:
		memcpy(week_flag,"0000001",DAY_OF_A_WEEK);
		break;
	default:

		break;
	}

}


bool cal_vcalendar_convert_tm_to_vdata_str(const struct tm *tm, char *utc_str)
{

	memset(utc_str, 0, 17);

	sprintf(utc_str, "%04ld%02d%02dT%02d%02d%02dZ",
			tm->tm_year + BENCHMARK_YEAR,
			tm->tm_mon +1,
			tm->tm_mday,
			tm->tm_hour,
			tm->tm_min,
			tm->tm_sec);

	return true;

}

void cal_vcalendar_convert_utc_str_to_tm(const char *szText, struct tm *tm)
{
	assert((szText != NULL) && (tm != NULL));

	char szBuff[8];

	//	struct tm  tm;
	memset(tm, 0, sizeof(struct tm));

	// year, month, day
	memcpy(szBuff, &(szText[0]), 4);
	szBuff[4] = '\0';
	tm->tm_year = atol(szBuff) - BENCHMARK_YEAR;
	if ((tm->tm_year > (CAL_YEAR_MAX - BENCHMARK_YEAR)) || (tm->tm_year < (CAL_YEAR_MIN - BENCHMARK_YEAR)))
	{
		tm->tm_year = (CAL_YEAR_MAX - BENCHMARK_YEAR);
	}

	memcpy(szBuff, &(szText[4]), 2);
	szBuff[2] = '\0';
	tm->tm_mon = atol(szBuff)-1;
	if ((tm->tm_mon > 11) || (tm->tm_mon < 0))
	{
		tm->tm_mon = 11;
	}

	memcpy(szBuff, &(szText[6]), 2);
	szBuff[2] = '\0';
	tm->tm_mday = atol(szBuff);
	if ((tm->tm_mday > 31) || (tm->tm_mday < 1))
	{
		tm->tm_mday = 31;
	}

	// hour, minute, second
	memcpy(szBuff, &(szText[9]), 2);
	szBuff[2] = '\0';
	tm->tm_hour = atol(szBuff);
	if ((tm->tm_hour > 23) || (tm->tm_hour < 0))
	{
		tm->tm_hour = 23;
	}

	memcpy(szBuff, &(szText[11]), 2);
	szBuff[2] = '\0';
	tm->tm_min = atol(szBuff);
	if ((tm->tm_min > 59) || (tm->tm_min < 0))
	{
		tm->tm_min = 59;
	}

	memcpy(szBuff, &(szText[13]), 2);
	szBuff[2] = '\0';
	tm->tm_sec = atol(szBuff);
	if ((tm->tm_sec > 59) || (tm->tm_sec < 0))
	{
		tm->tm_sec = 59;
	}

	CALS_DBG( "\n-------------------cal_vcalendar_convert_utc_str_to_tm year is %d, month is %d, day is %d, hour is %d, min is %d, sec is %d--------------------\n",
			tm->tm_year,tm->tm_mon,tm->tm_mday,tm->tm_hour,tm->tm_min,tm->tm_sec);
}

	bool
cal_util_convert_query_string(const char *src, char *dst)
{
	int	i = 0;
	int	j = 0;
	int	nSrc = 0;

	if (src == NULL || dst == NULL)
	{
		return FALSE;
	}

	nSrc = strlen(src);

	for(i = 0; i < nSrc; ++i)
	{
		if (src[i] == '\'')
		{
			dst[j++] = src[i];
			dst[j++] = src[i];
		}
		else if (src[i] == '%')
		{
			dst[j++] = src[i];
			dst[j++] = src[i];
		}
		else
		{
			dst[j++] = src[i];
		}
	}
	dst[j] = '\0';

	return TRUE;
}

cal_iter* cals_get_updated_list(int type, int calendar_id, time_t timestamp)
{
	cal_iter *iter;
	sqlite3_stmt *stmt;
	char query[CALS_SQL_MIN_LEN];

	retvm_if(timestamp < 0, NULL, "timestamp(%ld) is Invalid", timestamp);

	iter = calloc(1, sizeof(cal_iter));
	retvm_if(NULL == iter, NULL, "calloc() Failed");

	if (calendar_id)
		sprintf(query,"SELECT * FROM %s WHERE type = %d AND last_modified_time > %ld AND calendar_id = %d",
				CALS_TABLE_SCHEDULE, type, timestamp, calendar_id);
	else
		sprintf(query,"SELECT * FROM %s WHERE type = %d AND last_modified_time > %ld",
				CALS_TABLE_SCHEDULE, type, timestamp);

	stmt = cals_query_prepare(query);
	if (NULL == stmt) {
		free(iter);
		ERR("cals_query_prepare() Failed");
		return NULL;
	}

	iter->stmt = stmt;
	if (CAL_EVENT_TODO_TYPE == type)
		iter->i_type = CAL_STRUCT_TYPE_TODO;
	else
		iter->i_type = CAL_STRUCT_TYPE_SCHEDULE;

	return iter;
}

