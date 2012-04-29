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
#include <stdlib.h>
#include <string.h>

#include "cals-internal.h"
#include "cals-typedef.h"
#include "cals-recurrence-utils.h"
#include "cals-utils.h"
#include "cals-tz-utils.h"

int calendar_svc_get_tz_info(char *tz_file_name,int year, struct tm* dst_start, struct tm* dst_end,int *base_offset,int *dst_offset);

static const UINT8 month_table[12]={31,28,31,30,31,30,31,31,30,31,30,31};

//this array is used to calculate the day numer of a month in a year
static const UINT16 days_table[12] = {
	0,
	31,
	31 + 28,
	31 + 28 + 31,
	31 + 28 + 31 + 30,
	31 + 28 + 31 + 30 + 31,
	31 + 28 + 31 + 30 + 31 + 30,
	31 + 28 + 31 + 30 + 31 + 30 + 31,
	31 + 28 + 31 + 30 + 31 + 30 + 31 + 31,
	31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30,
	31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31,
	31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30,
};

//const static int _lpdays[] = { -1, 30, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };
//const static int _days[] = { -1, 30, 58, 89, 119, 150, 180, 211, 242, 272, 303, 333, 364 };


#define is_leap_year_tm(y) (((y+1900)%4 == 0 && ((y+1900)%100 != 0 || (y+1900)%400 == 0))? 1 : 0)

static UINT32 g_cal_cur_id = -1;
//cal_date_param_t cal_date_param={0,};

#ifndef SECOFONEDAY
#define SECOFONEDAY 86400
#endif

extern cal_svc_tm_info_t cal_svc_tm_value;

static CAL_GET_VALID_DATE_FUNC cal_service_get_valid_date_func[] =
{
	cal_service_get_next_repeat_none_valid_date,
	cal_service_get_next_repeat_daily_valid_date,
	cal_service_get_next_repeat_weekly_valid_date,
	cal_service_get_next_repeat_monthly_valid_date,
	cal_service_get_next_repeat_yearly_valid_date,
	cal_service_get_next_repeat_weekly_valid_date,
	cal_service_get_next_repeat_monthly_valid_date,
	cal_service_get_next_repeat_yearly_valid_date,
};


/**
 * compare two days
 * return -1 : adate is later then bdate
 *         0 : adate is the same with bdate
 *         1 : adate is earlier then bdate
 */
INT8 __cal_service_compare_date(const struct tm *first_date,const struct tm *second_date)
{
	UINT32 int_date_one = 0 ,int_date_two = 0;

	retvm_if(NULL == first_date, false, "\n %s:argument is NULL",__FUNCTION__);
	retvm_if(NULL == second_date, false, "\n %s:argument is NULL",__FUNCTION__);

	int_date_one = (first_date->tm_year*10000)+(first_date->tm_mon*100)+first_date->tm_mday;
	int_date_two = (second_date->tm_year*10000)+(second_date->tm_mon*100)+second_date->tm_mday;

	if( int_date_one == int_date_two )
	{
		return 0;
	}
	else if( int_date_one > int_date_two )
	{
		return -1;
	}

	return 1;
}


static void __cal_service_get_day_after_num(struct tm *start_day,struct tm *after_day,int day)
{
	time_t base_time = 0;
	retm_if(NULL == start_day, "\n %s:argument is NULL",__FUNCTION__);
	retm_if(NULL == after_day, "\n %s:argument is NULL",__FUNCTION__);

	base_time =	cals_mktime(start_day);
	base_time = base_time + (day * SECOFONEDAY);

	memcpy(after_day,cals_tmtime(&base_time),sizeof(struct tm));
}

static void __cal_service_get_day_after_sec(struct tm *start_day,struct tm *after_day,int sec)
{
	time_t base_time = 0;
	retm_if(NULL == start_day, "\n %s:argument is NULL",__FUNCTION__);
	retm_if(NULL == after_day, "\n %s:argument is NULL",__FUNCTION__);

	base_time =	cals_mktime(start_day);
	base_time = base_time + (sec);

	memcpy(after_day,cals_tmtime(&base_time),sizeof(struct tm));
}

static int __cal_service_get_num_of_event_day(struct tm *start_date, struct tm *end_date)
{
	struct tm start_buf={0,};
	struct tm end_buf={0,};
	retvm_if(NULL == start_date, false, "\n %s:argument is NULL",__FUNCTION__);
	retvm_if(NULL == end_date, false, "\n %s:argument is NULL",__FUNCTION__);

	//compare date only
	memcpy(&start_buf,start_date,sizeof(struct tm));
	memcpy(&end_buf,end_date,sizeof(struct tm));

	start_buf.tm_hour = 0;
	start_buf.tm_min = 0;
	start_buf.tm_sec = 0;

	end_buf.tm_hour = 0;
	end_buf.tm_min = 0;
	end_buf.tm_sec = 0;


	return ((cals_mktime(&end_buf) - cals_mktime(&start_buf)) /SECOFONEDAY);
}
/*
	void
	cal_service_get_day_after_num(cal_service_date_t *start_day,cal_service_date_t *after_day,int day)
	{
	__cal_service_get_day_after_num(start_day,after_day,day);
	}*/

void cal_service_set_day_of_week(struct tm* tm)
{
	int days = 0;
	int year = 0;
	int month = 0;
	retm_if(NULL == tm, "\n %s:argument is NULL",__FUNCTION__);
	days = tm->tm_mday;
	year = tm->tm_year+1900;
	month = tm->tm_mon;

	days=(year-1)+(year-1)/4-(year-1)/100+(year-1)/400 + days_table[month] + tm->tm_mday + ( (month>1) ? is_leap_year_tm(year) : 0 ) ;
	tm->tm_wday = days%7;
}


bool cal_is_used_dst_timezone(cal_sch_full_t *sch_record)
{

	return false;
}

void cal_svc_set_tz_base_info(int year)
{
	char *tz_name;

	tz_name = cals_tzutil_get_tz_path();

	if (!tz_name) {
		ERR("cals_tzutil_get_tz_path failed");
		return;
	}

	if(cal_svc_tm_value.is_initialize == FALSE || year != cal_svc_tm_value.start_local_dst_date_time.tm_year)
	{
		calendar_svc_get_tz_info(tz_name,year+1900,
				&cal_svc_tm_value.start_local_dst_date_time,
				&cal_svc_tm_value.start_local_std_date_time,
				&cal_svc_tm_value.localtime_offset,&cal_svc_tm_value.local_dst_offset);

		cal_svc_tm_value.is_initialize = true;
		strncpy(cal_svc_tm_value.local_tz_name,tz_name, sizeof(cal_svc_tm_value.local_tz_name));
	}

	free(tz_name);

	return;
}

bool cal_get_dst_info_time(cal_date_param_t *cal_date_param,const cal_sch_full_t *sch_record)
{
	int dst_offset = 0;

	retvm_if(NULL == cal_date_param, false, "\n %s:argument is NULL",__FUNCTION__);
	retvm_if(NULL == sch_record, false, "\n %s:argument is NULL",__FUNCTION__);

	if(cal_svc_tm_value.is_initialize == false){
		time_t t = time(NULL);
		struct tm * cur_time = NULL;//localtime(&t);
		struct tm ttm;

		cur_time = localtime_r(&t,&ttm);

		if(cur_time)
			cal_svc_set_tz_base_info(cur_time->tm_year); //local time
	}

	if(sch_record->tz_name == NULL || sch_record->tz_name[0] == '\0')
	{
		strncpy(cal_svc_tm_value.temp_tz_name,"calendar_localtime", sizeof(cal_svc_tm_value.temp_tz_name));
		dst_offset = cal_svc_tm_value.temp_dst_offset = cal_svc_tm_value.local_dst_offset;
		cal_svc_tm_value.temptime_offset = cal_svc_tm_value.localtime_offset;
		memcpy(&cal_svc_tm_value.start_temp_dst_date_time,&cal_svc_tm_value.start_local_dst_date_time,sizeof(struct tm));
		memcpy(&cal_svc_tm_value.start_temp_std_date_time,&cal_svc_tm_value.start_local_std_date_time,sizeof(struct tm));
	}
	else
	{

		if( (strcmp(cal_svc_tm_value.temp_tz_name,sch_record->tz_name)==0) &&
				((cal_svc_tm_value.temp_dst_offset ==0) ||
				 ((cal_svc_tm_value.temp_dst_offset !=0) &&
				 (cal_svc_tm_value.start_temp_dst_date_time.tm_year == cal_date_param->start_db_date_time.tm_year))))
		{
			cal_date_param->dbtime_offset = cal_svc_tm_value.temptime_offset;
			dst_offset = cal_date_param->db_dst_offset = cal_svc_tm_value.temp_dst_offset;
			memcpy(&cal_date_param->start_db_dst_date_time,&cal_svc_tm_value.start_temp_dst_date_time,sizeof(struct tm));
			memcpy(&cal_date_param->start_db_std_date_time,&cal_svc_tm_value.start_temp_std_date_time,sizeof(struct tm));
		}
		else {
			calendar_svc_get_tz_info(sch_record->tz_name,cal_date_param->start_db_date_time.tm_year+1900,
					&cal_date_param->start_db_dst_date_time,
					&cal_date_param->start_db_std_date_time,
					&cal_date_param->dbtime_offset,&cal_date_param->db_dst_offset);

			strncpy(cal_svc_tm_value.temp_tz_name,sch_record->tz_name,sizeof(cal_svc_tm_value.temp_tz_name));
			dst_offset = cal_svc_tm_value.temp_dst_offset = cal_date_param->db_dst_offset;
			cal_svc_tm_value.temptime_offset = cal_date_param->dbtime_offset;
			memcpy(&cal_svc_tm_value.start_temp_dst_date_time,&cal_date_param->start_db_dst_date_time,sizeof(struct tm));
			memcpy(&cal_svc_tm_value.start_temp_std_date_time,&cal_date_param->start_db_std_date_time,sizeof(struct tm));
		}
	}

	if(dst_offset == 0)
		return false;
	else
		return true;
}


void cal_set_time_by_dst_info(cal_date_param_t *cal_date_param,const cal_sch_full_t *sch_record,struct tm *next_start_tm,struct tm *next_end_tm)
{
	retm_if(NULL == cal_date_param, "\n %s:argument is NULL",__FUNCTION__);
	retm_if(NULL == sch_record, "\n %s:argument is NULL",__FUNCTION__);
	retm_if(NULL == next_start_tm, "\n %s:argument is NULL",__FUNCTION__);
	retm_if(NULL == next_end_tm, "\n %s:argument is NULL",__FUNCTION__);

	if(cal_date_param->is_need_adjust_time_for_view == DB_STD_LOCAL_STD ||
			cal_date_param->is_need_adjust_time_for_view == DB_INDST_LOCAL_DST ||
			cal_date_param->is_need_adjust_time_for_view == DB_OUTDST_LOCAL_DST ||
			(sch_record->repeat_term == CAL_REPEAT_NONE) ||
			(sch_record->all_day_event == TRUE) )
	{
		cal_date_param->i_adjust_time_value = 0;
		memcpy(next_start_tm,&cal_date_param->start_local_date_time,sizeof(struct tm));
		memcpy(next_end_tm,&cal_date_param->end_local_date_time,sizeof(struct tm));
		return;
	}

	if(cal_date_param->is_need_adjust_time_for_view == DB_STD_LOCAL_DST)
	{
		cal_svc_set_tz_base_info(cal_date_param->start_local_date_time.tm_year);

		if( (__cal_service_compare_date(&cal_svc_tm_value.start_local_dst_date_time,&cal_date_param->start_local_date_time) > 0) &&
				(__cal_service_compare_date(&cal_date_param->start_local_date_time,&cal_svc_tm_value.start_local_std_date_time) > 0)
		 )
		{
			/* if current local date is exist in DST Time period - adjust +1 hours */
			cal_date_param->i_adjust_time_value = 1;
			__cal_service_get_day_after_sec(&cal_date_param->start_local_date_time,next_start_tm,SECSPERHOUR*cal_date_param->i_adjust_time_value);
			__cal_service_get_day_after_sec(&cal_date_param->end_local_date_time,next_end_tm,SECSPERHOUR*cal_date_param->i_adjust_time_value);

		}
		else
		{
			/* if current local date is exist out DST Time period - do not need adjustment */
			memcpy(next_start_tm,&cal_date_param->start_local_date_time,sizeof(struct tm));
			memcpy(next_end_tm,&cal_date_param->end_local_date_time,sizeof(struct tm));
		}

	}
	else if(cal_date_param->is_need_adjust_time_for_view == DB_INDST_LOCAL_STD)
	{
		cal_get_dst_info_time(cal_date_param,sch_record);

		/* check startDstTimeDate < srcTimeDate < startStdTimeDate */
		if( (__cal_service_compare_date(&cal_date_param->start_db_dst_date_time,&cal_date_param->start_db_date_time) > 0) &&
				(__cal_service_compare_date(&cal_date_param->start_db_date_time,&cal_date_param->start_db_std_date_time) > 0)
		 )
		{
			/* if current local date is exist in DST Time period - do not need adjustment*/
			memcpy(next_start_tm,&cal_date_param->start_local_date_time,sizeof(struct tm));
			memcpy(next_end_tm,&cal_date_param->end_local_date_time,sizeof(struct tm));
		}
		else
		{
			/* if current local date is exist out DST Time period - adjust +1 hours */
			cal_date_param->i_adjust_time_value = 1;
			__cal_service_get_day_after_sec(&cal_date_param->start_local_date_time,next_start_tm,SECSPERHOUR*cal_date_param->i_adjust_time_value);
			__cal_service_get_day_after_sec(&cal_date_param->end_local_date_time,next_end_tm,SECSPERHOUR*cal_date_param->i_adjust_time_value);


		}

	}
	else /* DB_OUTDST_LOCAL_STD */
	{
		cal_get_dst_info_time(cal_date_param,sch_record);

		/* check startDstTimeDate < srcTimeDate < startStdTimeDate */
		if( (__cal_service_compare_date(&cal_date_param->start_db_dst_date_time,&cal_date_param->start_db_date_time) > 0) &&
				(__cal_service_compare_date(&cal_date_param->start_db_date_time,&cal_date_param->start_db_std_date_time) > 0)
		 )
		{
			/* if current local date is exist in DST Time period - adjust -1 hours */
			cal_date_param->i_adjust_time_value = -1;
			__cal_service_get_day_after_sec(&cal_date_param->start_local_date_time,next_start_tm,SECSPERHOUR*cal_date_param->i_adjust_time_value);
			__cal_service_get_day_after_sec(&cal_date_param->end_local_date_time,next_end_tm,SECSPERHOUR*cal_date_param->i_adjust_time_value);


		}
		else
		{
			/* if current local date is exist out DST Time period - do not need adjustment*/
			memcpy(next_start_tm,&cal_date_param->start_local_date_time,sizeof(struct tm));
			memcpy(next_end_tm,&cal_date_param->end_local_date_time,sizeof(struct tm));
		}
	}
}

void __cal_service_reset_exception_date_info(cal_date_param_t *cal_date_param )
{
	GList *head;
	cal_value *value = NULL;
	if(cal_date_param->exception_date_list)
	{
		head = cal_date_param->exception_date_list;
		while (cal_date_param->exception_date_list)
		{
			value = cal_date_param->exception_date_list->data;
			if(NULL != value)
			{
				if(NULL != value->user_data)
				{
					CAL_FREE(value->user_data);
				}
				CAL_FREE(value);
			}
			cal_date_param->exception_date_list = cal_date_param->exception_date_list->next;
		}
		g_list_free(head);
		cal_date_param->exception_date_list = NULL;
	}
}

void __cal_service_set_exception_date_info(cal_date_param_t *cal_date_param,cal_sch_full_t *sch_record)
{
	GList *head=NULL;
	GList *desc=NULL;

	cal_value *value = NULL;
	cal_value *desc_value = NULL;
	cal_exception_info_t* desc_exception = NULL;
	time_t exception_tt,temp_time;

	if(sch_record->exception_date_list)
	{
		head = sch_record->exception_date_list;
		while (head)
		{
			value = head->data;
			if(NULL != value)
			{
				if(value->user_data)
				{
					desc_value = (cal_value*)malloc(sizeof(cal_value));
					retm_if(NULL == desc_value,"[ERROR]__cal_service_set_exception_date_info:Failed to malloc!\n");

					desc_value->v_type = CAL_EVENT_RECURRENCY;
					desc_exception = desc_value->user_data = (cal_exception_info_t*)malloc(sizeof(cal_exception_info_t));
					retm_if(NULL == desc_exception,"[ERROR]__cal_service_set_exception_date_info:Failed to malloc!\n");

					exception_tt = cals_mktime(&(((cal_exception_info_t*)(value->user_data))->exception_start_time));
					temp_time = exception_tt + cal_svc_tm_value.temptime_offset; //db timezone
					cals_tmtime_r(&temp_time,&(desc_exception->exception_start_time));
					ERR("exception date(%d/%d/%d %d)",desc_exception->exception_start_time.tm_year+1900,desc_exception->exception_start_time.tm_mon+1,desc_exception->exception_start_time.tm_mday,desc_exception->exception_start_time.tm_hour);

				}
			}
			head = head->next;
			desc = g_list_append(desc,desc_value);
		}
		cal_date_param->exception_date_list = desc;
	}

}

void cal_service_set_date_param(cal_date_param_t *cal_date_param,cal_sch_full_t *sch_record)
{
	time_t temp_time;
	time_t	end_time = 0;
	time_t	start_time = 0;
	time_t repeat_end_time = 0;
	retm_if(NULL == cal_date_param, "\n %s:argument is NULL",__FUNCTION__);
	retm_if(NULL == sch_record, "\n %s:argument is NULL",__FUNCTION__);
	//bool is_used_dst_db_timezone = false;
	//bool is_used_dst_local_timezone = false;
	extern cal_svc_tm_info_t cal_svc_tm_value;

	__cal_service_reset_exception_date_info(cal_date_param);
	memset(cal_date_param,0x00,sizeof(cal_date_param_t));

	memcpy(&(cal_date_param->start_db_date_time),&(sch_record->start_date_time),sizeof(struct tm));
	memcpy(&(cal_date_param->end_db_date_time),&(sch_record->end_date_time),sizeof(struct tm));
	memcpy(&(cal_date_param->start_local_date_time),&(sch_record->start_date_time),sizeof(struct tm));
	memcpy(&(cal_date_param->end_local_date_time),&(sch_record->end_date_time),sizeof(struct tm));

	memcpy(&(cal_date_param->repeat_end_date),&(sch_record->repeat_end_date),sizeof(struct tm));

	cal_get_dst_info_time(cal_date_param,sch_record);

	__cal_service_set_exception_date_info(cal_date_param,sch_record);

	if(sch_record->all_day_event == true)
		goto COMMON;
	cal_date_param->repeat_end_date.tm_hour = cal_date_param->start_db_date_time.tm_hour;
	cal_date_param->repeat_end_date.tm_min = cal_date_param->start_db_date_time.tm_min;
	cal_date_param->repeat_end_date.tm_sec = cal_date_param->start_db_date_time.tm_sec;

	start_time = cals_mktime(&(cal_date_param->start_db_date_time));
	end_time = cals_mktime(&(cal_date_param->end_db_date_time));
	repeat_end_time = cals_mktime(&(cal_date_param->repeat_end_date));

	temp_time = start_time + cal_svc_tm_value.temptime_offset;
	//calendar_svc_util_local_to_gmt(start_time, &temp_time);
	cals_tmtime_r(&temp_time,&(cal_date_param->start_db_date_time));

	temp_time = end_time + cal_svc_tm_value.temptime_offset;
	//calendar_svc_util_local_to_gmt(end_time, &temp_time);
	cals_tmtime_r(&temp_time,&(cal_date_param->end_db_date_time));

	temp_time = repeat_end_time + cal_svc_tm_value.temptime_offset;
	cals_tmtime_r(&temp_time,&(cal_date_param->repeat_end_date));


	start_time = cals_mktime(&(cal_date_param->start_local_date_time));
	end_time = cals_mktime(&(cal_date_param->end_local_date_time));

	//if(sch_record->repeat_term == CAL_REPEAT_NONE)
	calendar_svc_util_gmt_to_local(start_time, &temp_time);
	cals_tmtime_r(&temp_time,&(cal_date_param->start_local_date_time));


	calendar_svc_util_gmt_to_local(end_time, &temp_time);
	cals_tmtime_r(&temp_time,&(cal_date_param->end_local_date_time));

	if(cal_svc_tm_value.temp_dst_offset!=0)
	{
		/* check startDstTimeDate < srcTimeDate < startStdTimeDate */
		if( (__cal_service_compare_date(&cal_date_param->start_db_dst_date_time,&cal_date_param->start_db_date_time) > 0) &&
				(__cal_service_compare_date(&cal_date_param->start_db_date_time,&cal_date_param->start_db_std_date_time) > 0)
		  ) 	/* dbtimezone is dst timezone */
		{
			if(cal_svc_tm_value.local_dst_offset!=0)
			{
				__cal_service_get_day_after_sec(&cal_date_param->start_local_date_time,&cal_date_param->start_local_date_time,SECSPERHOUR);
				__cal_service_get_day_after_sec(&cal_date_param->end_local_date_time,&cal_date_param->end_local_date_time,SECSPERHOUR);
				cal_date_param->is_need_adjust_time_for_view = DB_INDST_LOCAL_DST;

			}else
			{
				/* need adjust, after computation */
				cal_date_param->is_need_adjust_time_for_view = DB_INDST_LOCAL_STD;
			}
		}
		else /* dbtimezone is not dst timezone */
		{
			if(cal_svc_tm_value.local_dst_offset!=0)
			{
				/* nothing */
				cal_date_param->is_need_adjust_time_for_view = DB_OUTDST_LOCAL_DST;
			}
			else
			{
				/* need adjust, after computation */
				cal_date_param->is_need_adjust_time_for_view = DB_OUTDST_LOCAL_STD;
			}
		}
	}else
	{
		if(cal_svc_tm_value.local_dst_offset!=0)
		{
			/* need adjust, after computation */
			cal_date_param->is_need_adjust_time_for_view = DB_STD_LOCAL_DST;
		}else
		{
			/* nothing */
			cal_date_param->is_need_adjust_time_for_view = DB_STD_LOCAL_STD;
		}
	}

COMMON:

	g_cal_cur_id = -2;
	if(cal_date_param->repeat_end_date.tm_year >137)
	{
		cal_date_param->repeat_end_date.tm_year = 137;
	}

	cal_date_param->prev_alarm_time.tm_year = 9999;
	cal_date_param->repeat_by_set_position = (cal_date_param->start_db_date_time.tm_mday - 1) /7 + 1;

}



/* API for calculate date return TRUE is valid DATE, FALSE is not valid date */
BOOL cal_service_get_next_repeat_none_valid_date(const cal_sch_full_t *sch_record,
	cal_date_param_t *cal_date_param,struct tm *now_start_date,struct tm *now_end_date)
{
	retvm_if(NULL == sch_record, false, "sch_record is NULL");
	retvm_if(NULL == cal_date_param, false, "cal_date_param is NULL");
	retvm_if(NULL == now_start_date, false, "now_start_date is NULL");
	retvm_if(NULL == now_end_date, false, "now_end_date is NULL");

	/* localend_date < nowStartDate || nowEndDate < localStartDate */
	memcpy(&cal_date_param->prev_alarm_time,&cal_date_param->start_local_date_time,sizeof(struct tm));

	/* never call this function with same id */
	if (g_cal_cur_id == sch_record->index)
	{
		return FALSE;
	}

	if ((__cal_service_compare_date(&(cal_date_param->end_local_date_time),now_start_date) > 0)
			|| (__cal_service_compare_date(now_end_date,&(cal_date_param->start_local_date_time)) > 0)	)
	{
		g_cal_cur_id = -1;
		return FALSE;
	}

	g_cal_cur_id = sch_record->index;

	return TRUE;
}

BOOL cal_service_get_next_repeat_daily_valid_date(const cal_sch_full_t *sch_record,
	cal_date_param_t *cal_date_param, struct tm *now_start_date, struct tm *now_end_date)
{
	UINT32 count_day=0;
	retvm_if(NULL == sch_record, false, "sch_record is NULL");
	retvm_if(NULL == cal_date_param, false, "cal_date_param is NULL");
	retvm_if(NULL == now_start_date, false, "now_start_date is NULL");
	retvm_if(NULL == now_end_date, false, "now_end_date is NULL");

	if (sch_record->repeat_interval == 0)
	{
		return FALSE; /* invalid Data */
	}

	/* first function call & valid date check  */
	if ( (g_cal_cur_id != sch_record->index) &&
			( !( (__cal_service_compare_date(&(cal_date_param->end_local_date_time),now_start_date) > 0 )
					|| (__cal_service_compare_date(&(cal_date_param->start_local_date_time),now_end_date) < 0 ) )
			) )
	{
		g_cal_cur_id = sch_record->index;
	}
	else
	{
		//TODO: need improvement performance

		do
		{
			count_day = 0;
			count_day = count_day + sch_record->repeat_interval;
			__cal_service_get_day_after_num(&(cal_date_param->start_db_date_time),&(cal_date_param->start_db_date_time),count_day);
			__cal_service_get_day_after_num(&(cal_date_param->end_db_date_time),&(cal_date_param->end_db_date_time),count_day);
			__cal_service_get_day_after_num(&(cal_date_param->start_local_date_time),&(cal_date_param->start_local_date_time),count_day);
			__cal_service_get_day_after_num(&(cal_date_param->end_local_date_time),&(cal_date_param->end_local_date_time),count_day);
		}while( (__cal_service_compare_date(&(cal_date_param->end_local_date_time),now_start_date) > 0));

		g_cal_cur_id = sch_record->index;
	}

	//__cal_service_convert_tm_to_date(&(sch_record->repeat_end_date), &repeat_end_date);
	/*check again*/
	if ((__cal_service_compare_date(&(cal_date_param->repeat_end_date),&(cal_date_param->start_db_date_time)) > 0 )
			|| (__cal_service_compare_date(&(cal_date_param->start_local_date_time),now_end_date) < 0 ))
	{
		g_cal_cur_id = -1;
		return FALSE;
	}

	return TRUE;
}

BOOL cal_service_get_next_repeat_weekly_valid_date(const cal_sch_full_t *sch_record,
	cal_date_param_t *cal_date_param, struct tm *now_start_date, struct tm *now_end_date)
{

	UINT8 valid_week[7]={0,};
	UINT32 count_day=0;
	int week_count=0;
	int i = 0;
	static BOOL is_same_start_day_of_week =TRUE;
	static UINT8 start_day_of_week=0;
	BOOL is_first_valid_date = TRUE;
	BOOL is_set_week_flag = FALSE;
	retvm_if(NULL == sch_record, false, "\n %s:argument is NULL",__FUNCTION__);
	retvm_if(NULL == cal_date_param, false, "\n %s:argument is NULL",__FUNCTION__);
	retvm_if(NULL == now_start_date, false, "\n %s:argument is NULL",__FUNCTION__);
	retvm_if(NULL == now_end_date, false, "\n %s:argument is NULL",__FUNCTION__);
	if (sch_record->repeat_interval == 0)
	{
		return FALSE; /* invalid Data */
	}

	/* make valid week array */
	for (i=0;i<7;i++)
	{
		valid_week[i] = sch_record->week_flag[i]-'0';
		if(valid_week[i])
		{
			is_set_week_flag = TRUE;
		}
	}

	if(is_set_week_flag == FALSE){
		DBG("critical error!! , week flag should be setting ");
		return FALSE;
	}


	if (g_cal_cur_id != sch_record->index)
	{
		if (sch_record->week_start !=0)
		{
			UINT8 start_week;

			start_week = (UINT8)sch_record->week_start;
			is_first_valid_date = TRUE;

			for (i = 0; i < DAY_OF_A_WEEK; i++)
			{
				if(valid_week[i] == 1)
				{
					if((is_first_valid_date == TRUE) && (start_week == i))
					{
						is_same_start_day_of_week = TRUE;
						break;
					}
					else if(start_week == i)
					{
						start_day_of_week = start_week;
						is_same_start_day_of_week = FALSE;
						break;
					}
					is_first_valid_date = FALSE;
				}
			}
		}
		else
		{
			is_same_start_day_of_week = TRUE;
		}
	}

	if ( (g_cal_cur_id != sch_record->index) &&

			( !( (__cal_service_compare_date(&(cal_date_param->end_local_date_time),now_start_date) > 0 )
					|| (__cal_service_compare_date(&(cal_date_param->start_local_date_time),now_end_date) < 0 ) ) )
		)
	{
		g_cal_cur_id = sch_record->index;
	}
	else
	{
		do
		{
			int valid_term=1;
			count_day = 0;

			week_count = cal_date_param->start_db_date_time.tm_wday;

			// [2010.05.09. Brian] Dangerous statements.
			// if weekflag is '0000000', following statement would make process in infinite loop.
			while (valid_week[((week_count+valid_term) %7)] ==0 )
			{
				valid_term++;
			}

			count_day = count_day + valid_term;

			//pass interval week
			if (is_same_start_day_of_week == TRUE)
			{
				if( (week_count+valid_term) >=7 )
					count_day = count_day + ((sch_record->repeat_interval-1) * 7);
			}
			else
			{
				if (((week_count+valid_term)%7) == start_day_of_week)
				{
					count_day = count_day + ((sch_record->repeat_interval-1) * 7);
				}
			}

			__cal_service_get_day_after_num(&(cal_date_param->start_db_date_time),&(cal_date_param->start_db_date_time),count_day);
			__cal_service_get_day_after_num(&(cal_date_param->end_db_date_time),&(cal_date_param->end_db_date_time),count_day);
			__cal_service_get_day_after_num(&(cal_date_param->start_local_date_time),&(cal_date_param->start_local_date_time),count_day);
			__cal_service_get_day_after_num(&(cal_date_param->end_local_date_time),&(cal_date_param->end_local_date_time),count_day);
		}
		while( (__cal_service_compare_date(&(cal_date_param->end_local_date_time),now_start_date) > 0));

		g_cal_cur_id = sch_record->index;
	}

	//__cal_service_convert_tm_to_date(&(sch_record->repeat_end_date), &repeat_end_date);
	if( (__cal_service_compare_date(&(cal_date_param->repeat_end_date),&(cal_date_param->start_db_date_time)) > 0 )
			|| (__cal_service_compare_date(&(cal_date_param->start_local_date_time),now_end_date) < 0 ) )
	{
		start_day_of_week = 0;
		is_same_start_day_of_week= TRUE;
		g_cal_cur_id = -1;

		return FALSE;
	}

	return TRUE;

}

	BOOL
cal_service_get_next_repeat_monthly_valid_date(const cal_sch_full_t *sch_record,cal_date_param_t *cal_date_param,struct tm *now_start_date,struct tm *now_end_date)
{
	UINT32 count_day=0;
	int i = 0;
	retvm_if(NULL == sch_record, false, "sch_record is NULL");
	retvm_if(NULL == cal_date_param, false, "cal_date_param is NULL");
	retvm_if(NULL == now_start_date, false, "now_start_date is NULL");
	retvm_if(NULL == now_end_date, false, "now_end_date is NULL");

	if (sch_record->repeat_interval == 0)
	{
		return FALSE; /* invalid Data */
	}

	if (g_cal_cur_id != sch_record->index) /* SEC 060427 heungjae.jeong last day issue */
	{
		cal_date_param->day_for_last_day_event = sch_record->start_date_time.tm_mday;
	}

	if ((g_cal_cur_id != sch_record->index) &&
			( !( (__cal_service_compare_date(&(cal_date_param->end_local_date_time),now_start_date) > 0 )
					|| (__cal_service_compare_date(&(cal_date_param->start_local_date_time),now_end_date) < 0 ) ) )
		)
	{
		g_cal_cur_id = sch_record->index;
	}
	else
	{
		do
		{
			count_day = 0;
			if (sch_record->day_date ==1)
			{
				int temp=0;

				/* move next month */
				for (i = cal_date_param->start_db_date_time.tm_mon; i < (cal_date_param->start_db_date_time.tm_mon +
							sch_record->repeat_interval); i++)
				{
					temp = temp + month_table[(i)%12] +
						(((i%12)==1) ? is_leap_year_tm(cal_date_param->start_db_date_time.tm_year + ((i)/12)) : 0 ) ;
				}

				if(cal_date_param->day_for_last_day_event != 0)
				{
					if (cal_date_param->start_db_date_time.tm_mday >
							(month_table[(i)%12]+ ( ((i%12)==1) ? is_leap_year_tm(cal_date_param->start_db_date_time.tm_year + ((i)/12)) : 0 ) ) )
					{
						temp = temp - (cal_date_param->day_for_last_day_event-
								(month_table[(i)%12]+(((i%12)==1)?is_leap_year_tm(cal_date_param->start_db_date_time.tm_year+((i)/12)):0)) );
					}
					else
					{
						temp = temp + (cal_date_param->day_for_last_day_event-cal_date_param->start_db_date_time.tm_mday);
					}
				}

				count_day = count_day + temp;

				__cal_service_get_day_after_num(&(cal_date_param->start_db_date_time),&(cal_date_param->start_db_date_time),count_day);
				__cal_service_get_day_after_num(&(cal_date_param->end_db_date_time),&(cal_date_param->end_db_date_time),count_day);
				__cal_service_get_day_after_num(&(cal_date_param->start_local_date_time),&(cal_date_param->start_local_date_time),count_day);
				__cal_service_get_day_after_num(&(cal_date_param->end_local_date_time),&(cal_date_param->end_local_date_time),count_day);

			}
			else /* Relative date check */
			{
				int first_day_of_week=0;
				struct tm after_from_date;

				after_from_date = cal_date_param->start_db_date_time;

				after_from_date.tm_mon += sch_record->repeat_interval;

				if (after_from_date.tm_mon > 11)
				{
					after_from_date.tm_year += (after_from_date.tm_mon) / 12;
					after_from_date.tm_mon = (after_from_date.tm_mon) % 12;
					//after_from_date.tm_mon ++;
				}

				if (cal_date_param->repeat_by_set_position == 5)
				{
					int valid_day_of_week = 0;
					int k = 0;

					after_from_date.tm_mday = month_table[after_from_date.tm_mon]
						+ ( (after_from_date.tm_mon==1) ? is_leap_year_tm(after_from_date.tm_year) : 0 ) ;

					cal_service_set_day_of_week(&after_from_date);

					while(sch_record->start_date_time.tm_wday != valid_day_of_week)
					{
						valid_day_of_week++;
					} /* j = recurrence_day_of_the_week 0..6 */

					k=0;
					while (((k+after_from_date.tm_wday)%7) != valid_day_of_week)
					{
						k++;
					}

					after_from_date.tm_mday = after_from_date.tm_mday - ((7-k)%7);
					after_from_date.tm_wday = (after_from_date.tm_wday + k) % 7;
				}
				else
				{
					int valid_day_of_week=0;
					int k=0;

					after_from_date.tm_mday = 1;
					cal_service_set_day_of_week(&after_from_date);
					first_day_of_week = after_from_date.tm_wday;

					while(sch_record->start_date_time.tm_wday != valid_day_of_week)
					{
						valid_day_of_week++;
					} /* j = recurrence_day_of_the_week 0..6 */

					k=0;
					while( ((k+first_day_of_week)%7) != valid_day_of_week)
					{
						k++;
					}

					after_from_date.tm_mday = 1 + k;

					after_from_date.tm_mday = after_from_date.tm_mday + (cal_date_param->repeat_by_set_position-1)*7;
					cal_service_set_day_of_week(&after_from_date);
				}

				count_day = __cal_service_get_num_of_event_day(&(cal_date_param->start_db_date_time),&after_from_date);

				__cal_service_get_day_after_num(&(cal_date_param->start_db_date_time),&(cal_date_param->start_db_date_time),count_day);
				__cal_service_get_day_after_num(&(cal_date_param->end_db_date_time),&(cal_date_param->end_db_date_time),count_day);
				__cal_service_get_day_after_num(&(cal_date_param->start_local_date_time),&(cal_date_param->start_local_date_time),count_day);
				__cal_service_get_day_after_num(&(cal_date_param->end_local_date_time),&(cal_date_param->end_local_date_time),count_day);

			}
			g_cal_cur_id = sch_record->index;
			if(count_day == 0) return FALSE;
		}
		while((__cal_service_compare_date(&(cal_date_param->end_local_date_time),now_start_date) > 0));
	}

	//__cal_service_convert_tm_to_date(&(sch_record->repeat_end_date), &repeat_end_date);
	if ((__cal_service_compare_date(&(cal_date_param->repeat_end_date),&(cal_date_param->start_db_date_time)) > 0 )
			|| (__cal_service_compare_date(&(cal_date_param->start_local_date_time),now_end_date) < 0 ))
	{
		g_cal_cur_id = -1;
		return FALSE;
	}

	return TRUE;
}

BOOL cal_service_get_next_repeat_yearly_valid_date(const cal_sch_full_t *sch_record,
	cal_date_param_t *cal_date_param, struct tm *now_start_date, struct tm *now_end_date)
{
	int i;
	UINT32 count_day=0;

	retvm_if(NULL == sch_record, FALSE, "sch_record is NULL");
	retvm_if(NULL == cal_date_param, FALSE, "cal_date_param is NULL");
	retvm_if(NULL == now_start_date, FALSE, "now_start_date is NULL");
	retvm_if(NULL == now_end_date, FALSE, "now_end_date is NULL");
	//__cal_service_convert_tm_to_date(&(sch_record->start_date_time), &sch_start_date);
	//	repeat_by_set_position = (sch_start_date.tm_mday - 1) /7 + 1;

	if (sch_record->repeat_interval== 0)
		return FALSE; /* invalid Data */

	if ((g_cal_cur_id != sch_record->index) &&
			( !( (__cal_service_compare_date(&(cal_date_param->end_local_date_time),now_start_date) > 0 )
					|| (__cal_service_compare_date(&(cal_date_param->start_local_date_time),now_end_date) < 0 ) ) ))
	{
		g_cal_cur_id = sch_record->index;
	}
	else
	{
		do
		{
			count_day = 0;

			if (sch_record->day_date ==1)
			{
				int temp = 0;

				for (i = cal_date_param->start_db_date_time.tm_year; i < (cal_date_param->start_db_date_time.tm_year +
							sch_record->repeat_interval); i++)
				{
					temp = temp + 365;

					if(cal_date_param->start_db_date_time.tm_mon > 1)
						temp = temp + is_leap_year_tm(i+1);
					else
						temp = temp + is_leap_year_tm(i);
				}

				i = cal_date_param->start_db_date_time.tm_year;
				if (cal_date_param->start_db_date_time.tm_mon == 1)
				{
					//												cal_date_param->day_for_last_day_event,i,is_leap_year_tm(i));
					if (cal_date_param->day_for_last_day_event != 0)
					{
						if (is_leap_year_tm(i+1))
						{
							temp = temp + 1;
							cal_date_param->day_for_last_day_event = 0;
						}
						/*if (cal_date_param->start_db_date_time.tm_mday > (month_table[1]+is_leap_year_tm(i)) )
						  {
						  temp = temp - 1;
						  }
						  else if (is_leap_year_tm(i))
						  {
						  temp = temp + 1;
						  }*/
					}
					else if (cal_date_param->start_db_date_time.tm_mday == 29 )
					{
						temp = temp-1;
						cal_date_param->day_for_last_day_event = 29;
						//													temp, cal_date_param->start_db_date_time.tm_mday,(month_table[1]+is_leap_year_tm(i)));
					}
				}

				count_day = count_day + temp;
				__cal_service_get_day_after_num(&(cal_date_param->start_db_date_time),&(cal_date_param->start_db_date_time),count_day);
				__cal_service_get_day_after_num(&(cal_date_param->end_db_date_time),&(cal_date_param->end_db_date_time),count_day);
				__cal_service_get_day_after_num(&(cal_date_param->start_local_date_time),&(cal_date_param->start_local_date_time),count_day);
				__cal_service_get_day_after_num(&(cal_date_param->end_local_date_time),&(cal_date_param->end_local_date_time),count_day);

			}
			else /* Relative date check */
			{
				int first_day_of_week=0;
				int temp = 0;
				struct tm after_from_date;

				after_from_date = cal_date_param->start_db_date_time;

				for (i=after_from_date.tm_year;i<(after_from_date.tm_year+sch_record->repeat_interval);i++)
				{
					temp = temp + 365;

					if(after_from_date.tm_mon > 1)
						temp = temp + is_leap_year_tm(i+1);
					else
						temp = temp + is_leap_year_tm(i);
				}

				count_day = count_day + temp;

				__cal_service_get_day_after_num(&after_from_date,&after_from_date,count_day);

				if (cal_date_param->repeat_by_set_position == 5)
				{
					int valid_day_of_week=0;
					int k=0;

					after_from_date.tm_mday = month_table[after_from_date.tm_mon]
						+ ( (after_from_date.tm_mon==1) ? is_leap_year_tm(after_from_date.tm_year) : 0 ) ;

					cal_service_set_day_of_week(&after_from_date);

					while(sch_record->start_date_time.tm_wday != valid_day_of_week)
						valid_day_of_week++; /* j = recurrence_day_of_the_week 0..6 */

					k=0;
					while( ((k+after_from_date.tm_wday)%7) != valid_day_of_week)
						k++;

					after_from_date.tm_mday = after_from_date.tm_mday - ((7-k)%7);
					after_from_date.tm_wday = (after_from_date.tm_wday + k) % 7;
				}
				else
				{
					int valid_day_of_week=0;
					int k=0;

					after_from_date.tm_mday = 1;
					cal_service_set_day_of_week(&after_from_date);
					first_day_of_week = after_from_date.tm_wday;

					while(sch_record->start_date_time.tm_wday != valid_day_of_week)
						valid_day_of_week++; /* j = recurrence_day_of_the_week 0..6 */

					k=0;
					while( ((k+first_day_of_week)%7) != valid_day_of_week)
						k++;

					after_from_date.tm_mday = 1 + k;

					after_from_date.tm_mday = after_from_date.tm_mday + (cal_date_param->repeat_by_set_position-1)*7;
					cal_service_set_day_of_week(&after_from_date);
				}

				count_day = __cal_service_get_num_of_event_day(&(cal_date_param->start_db_date_time),&after_from_date);
				__cal_service_get_day_after_num(&(cal_date_param->start_db_date_time),&(cal_date_param->start_db_date_time),count_day);
				__cal_service_get_day_after_num(&(cal_date_param->end_db_date_time),&(cal_date_param->end_db_date_time),count_day);
				__cal_service_get_day_after_num(&(cal_date_param->start_local_date_time),&(cal_date_param->start_local_date_time),count_day);
				__cal_service_get_day_after_num(&(cal_date_param->end_local_date_time),&(cal_date_param->end_local_date_time),count_day);

			}

			g_cal_cur_id = sch_record->index;
		}
		while((__cal_service_compare_date(&(cal_date_param->end_local_date_time),now_start_date) > 0));
	}

	//__cal_service_convert_tm_to_date(&(sch_record->repeat_end_date), &repeat_end_date);
	if((__cal_service_compare_date(&(cal_date_param->repeat_end_date),&(cal_date_param->start_db_date_time)) > 0 )
			||(__cal_service_compare_date(&(cal_date_param->start_local_date_time),now_end_date) < 0 ) )
	{
		g_cal_cur_id = -1;
		return FALSE;
	}

	return TRUE;
}



int cal_db_service_get_next_valid_exception_time(const cal_sch_full_t *sch_record,
		cal_date_param_t *cal_date_param,GList *exception_date_list,
		struct tm* start_tm,struct tm* end_tm,
		struct tm* next_start_tm,struct tm* next_end_tm)
{
	cal_exception_info_t *exception_info = NULL;
	cal_value *cvalue = NULL;
	GList *head = NULL;
	bool is_exception_date = false;
	bool is_valid_date = FALSE;
	time_t temp_time = 0;

	retv_if(NULL == sch_record, CAL_ERR_ARG_NULL);
	retv_if(NULL == start_tm, CAL_ERR_ARG_NULL);
	retv_if(NULL == end_tm, CAL_ERR_ARG_NULL);
	retv_if(NULL == next_start_tm, CAL_ERR_ARG_NULL);
	retv_if(NULL == next_end_tm, CAL_ERR_ARG_NULL);

	//g_cal_cur_id = -1;
	is_valid_date = false;
	while (cal_service_get_valid_date_func[sch_record->repeat_term](sch_record, cal_date_param,
				start_tm, end_tm) == TRUE )
	{
		is_exception_date = false;
		//check exception date
		if(exception_date_list)
		{
			// __cal_service_convert_datetime_to_tm(&cal_date_param.start_db_date_time,next_start_tm);
			memcpy(next_start_tm,&cal_date_param->start_db_date_time,sizeof(struct tm));

			//if exception date, calucate again
			temp_time = cals_mktime(next_start_tm);
			//DBG("next_start_tm [%s\n", ctime(&temp_time));

			//DBG("exception_date_list = %x\n",exception_date_list);

			head = exception_date_list;

			while(head != NULL)
			{
				cvalue = head->data;
				//DBG("cvalue = %x\n",cvalue);
				if(cvalue)
				{
					exception_info = cvalue->user_data;
					temp_time = cals_mktime(&(exception_info->exception_start_time));
					//DBG("exception_start_time [%s\n", ctime(&temp_time));

					//check exception date is same or not
					if( (next_start_tm->tm_year == exception_info->exception_start_time.tm_year) &&
							(next_start_tm->tm_mon == exception_info->exception_start_time.tm_mon ) &&
							(next_start_tm->tm_mday == exception_info->exception_start_time.tm_mday) )
					{
						is_exception_date = true;
						//DBG("is_exception_date = %d\n",is_exception_date);
						break;
					}
				}
				else
				{
					break;
				}
				head = head->next;
			}

			if(is_exception_date)
			{
				continue;
			}

			//printf("is_exception_date = %d",is_exception_date);
			cal_set_time_by_dst_info(cal_date_param,sch_record,next_start_tm,next_end_tm);
			is_valid_date = true;
			break;
		}
		else
		{
			cal_set_time_by_dst_info(cal_date_param,sch_record,next_start_tm,next_end_tm);
			is_valid_date = true;
			break;
		}

	}

	if(is_valid_date == true)
	{
		return CAL_SUCCESS;
	}

	return CAL_ERR_FAIL;
}
