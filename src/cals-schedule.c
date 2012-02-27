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
#include <errno.h>

#include "cals-internal.h"
#include "cals-typedef.h"
#include "cals-sqlite.h"
#include "cals-db-info.h"
#include "cals-db.h"
#include "cals-tz-utils.h"
#include "cals-utils.h"
#include "cals-alarm.h"
#include "cals-schedule.h"


static void _cals_adjust_time(cal_sch_full_t *current_record,
	time_t *start, time_t *end, time_t *repeat_end)
{
	TMDUMP(current_record->start_date_time);

	if ((current_record->all_day_event == true) &&
			((current_record->start_date_time.tm_hour != 0) ||
			 (current_record->start_date_time.tm_min !=0)) )
	{
		*start = cals_mktime(&(current_record->start_date_time));
		CALS_DBG("%d",*start);
		calendar_svc_util_gmt_to_local(*start, start);
		CALS_DBG("%d",*start);
		*end = cals_mktime(&(current_record->end_date_time));

		if(current_record->all_day_event == true &&
				current_record->end_date_time.tm_hour == 0 &&
				current_record->end_date_time.tm_min == 0)
			*end = *end - 1;

		CALS_DBG("%d",*end);
		calendar_svc_util_gmt_to_local(*end, end);
		CALS_DBG("%d",*end);
		*repeat_end = cals_mktime(&(current_record->repeat_end_date));

		calendar_svc_util_gmt_to_local(*repeat_end, repeat_end);
	}
	else
	{
		*start = cals_mktime(&(current_record->start_date_time));
		*end = cals_mktime(&(current_record->end_date_time));
		*repeat_end = cals_mktime(&(current_record->repeat_end_date));
	}

}

static inline int _cals_insert_schedule(cal_sch_full_t *record)
{
	int ret = -1;
	char query[CALS_SQL_MAX_LEN] = {0};
	sqlite3_stmt *stmt = NULL;
	time_t conv_start_time;
	time_t conv_end_time;
	time_t conv_repeat_end_time;
	time_t conv_last_modified_time;
	time_t conv_created_date_time;
	time_t conv_completed_date_time;
	time_t t = time(NULL);
	tzset();
	struct tm * ptm = NULL;
	struct tm ttm;

	retv_if(NULL == record, CAL_ERR_ARG_NULL);

	ptm = gmtime_r(&t,&ttm);
	retvm_if(NULL == ptm, CAL_ERR_TIME_FAILED, "gmtime_r() Failed(%d)", errno);

	memcpy(&record->last_modified_time, &ttm, sizeof(struct tm));

	conv_last_modified_time = cals_mktime(&(record->last_modified_time));

	_cals_adjust_time(record, &conv_start_time, &conv_end_time, &conv_repeat_end_time);
	conv_created_date_time = cals_mktime(&(record->created_date_time));
	conv_completed_date_time = cals_mktime(&(record->completed_date_time));

	ret = snprintf(query, sizeof(query),
		"INSERT INTO %s( "
			"account_id, type, category, "
			"summary, description, location, all_day_event, "
			"start_date_time, end_date_time, repeat_item, repeat_interval, "
			"repeat_occurrences, repeat_end_date, sun_moon, week_start, "
			"week_flag, day_date, last_modified_time, missed, "
			"task_status, priority, timezone, file_id, "
			"contact_id, busy_status, sensitivity, uid, "
			"calendar_type, organizer_name, organizer_email, meeting_status, "
			"gcal_id, deleted, updated, location_type, "
			"location_summary, etag, calendar_id, sync_status, "
			"edit_uri, gevent_id, dst, original_event_id, "
			"latitude, longitude, is_deleted, tz_name, "
			"tz_city_name, email_id, availability, "
			"created_date_time, completed_date_time, progress) "
		"VALUES( "
			"%d, %d, %d, "
			"?, ?, ?, %d, "
			"%ld, %ld, %d, %d, "
			"%d, %ld, %d, %d, "
			"?, %d, %ld, %d, "
			"%d, %d, %d, %d, "
			"%d, %d, %d, ?, "
			"%d, ?, ?, %d, "
			"?, %d, ?, %d, "
			"?, ?, %d, %d, "
			"?, ?, %d, %d, "
			"%lf, %lf, %d, ?, "
			"?, %d, %d, "
			"%ld, %ld, %d)",
			CALS_TABLE_SCHEDULE,
			record->account_id, record->cal_type, record->sch_category,
			record->all_day_event,
			conv_start_time, conv_end_time, record->repeat_term, record->repeat_interval,
			record->repeat_occurrences, conv_repeat_end_time, record->sun_moon, record->week_start,
			record->day_date,	(long int)conv_last_modified_time, record->missed,
			record->task_status,	record->priority,	record->timezone, record->file_id,
			record->contact_id, record->busy_status, record->sensitivity,
			record->calendar_type, record->meeting_status,
			record->deleted, record->location_type,
			record->calendar_id,	record->sync_status,
			record->dst, record->original_event_id,
			record->latitude, record->longitude, record->is_deleted,
			record->email_id,	record->availability,
			conv_created_date_time, conv_completed_date_time, record->progress);

	stmt = cals_query_prepare(query);
	retvm_if(NULL == stmt, CAL_ERR_DB_FAILED, "cals_query_prepare() Failed");

	int count = 1;

	if (record->summary)
		cals_stmt_bind_text(stmt, count, record->summary);
	count++;

	if (record->description)
		cals_stmt_bind_text(stmt, count, record->description);
	count++;

	if (record->location)
		cals_stmt_bind_text(stmt, count, record->location);
	count++;

	if (record->week_flag)
		cals_stmt_bind_text(stmt, count, record->week_flag);
	count++;

	if (record->uid)
		cals_stmt_bind_text(stmt, count, record->uid);
	count++;

	if (record->organizer_name)
		cals_stmt_bind_text(stmt, count, record->organizer_name);
	count++;

	if (record->organizer_email)
		cals_stmt_bind_text(stmt, count, record->organizer_email);
	count++;

	if (record->gcal_id)
		cals_stmt_bind_text(stmt, count, record->gcal_id);
	count++;

	if (record->updated)
		cals_stmt_bind_text(stmt, count, record->updated);
	count++;

	if (record->location_summary)
		cals_stmt_bind_text(stmt, count, record->location_summary);
	count++;

	if (record->etag)
		cals_stmt_bind_text(stmt, count, record->etag);
	count++;

	if (record->edit_uri)
		cals_stmt_bind_text(stmt, count, record->edit_uri);
	count++;

	if (record->gevent_id)
		cals_stmt_bind_text(stmt, count, record->gevent_id);
	count++;

	if (record->tz_name)
		cals_stmt_bind_text(stmt, count, record->tz_name);
	count++;

	if (record->tz_city_name)
		cals_stmt_bind_text(stmt, count, record->tz_city_name);
	count++;

	ret = cals_stmt_step(stmt);
	if (CAL_SUCCESS != ret) {
		sqlite3_finalize(stmt);
		ERR("sqlite3_step() Failed(%d)", ret);
		return ret;
	}
	sqlite3_finalize(stmt);

	return cals_last_insert_id();
}

int _cals_add_meeting_category_info(const int event_id, const cal_category_info_t *current_record)
{
	int ret;
	sqlite3_stmt *stmt;
	char sql_value[CALS_SQL_MAX_LEN];

	retv_if(NULL == current_record, CAL_ERR_ARG_NULL);

	sprintf(sql_value, "INSERT INTO %s(event_id, category_name) "
			"VALUES(%d, ?)", CALS_TABLE_MEETING_CATEGORY, event_id);

	stmt = cals_query_prepare(sql_value);
	retvm_if(NULL == stmt, CAL_ERR_DB_FAILED, "cals_query_prepare() Failed");

	cals_stmt_bind_text(stmt, 1, current_record->category_name);

	ret = cals_stmt_step(stmt);
	if (CAL_SUCCESS != ret) {
		sqlite3_finalize(stmt);
		ERR("sqlite3_step() Failed(%d)", ret);
		return ret;
	}
	sqlite3_finalize(stmt);

	return CAL_SUCCESS;
}

static inline int _cals_check_date_validity(struct tm *day)
{
	int month_day_count = 0;

	retvm_if(NULL == day, CAL_FALSE, "day is NULL");

	month_day_count = cal_db_service_get_day_count_in_month(day->tm_year, day->tm_mon);

	if ((day->tm_mday < 1) ||
			(day->tm_mday > month_day_count) ||
			(day->tm_mon < CAL_MONTH_CNT_MIN - 1 ) ||
			(day->tm_mon > CAL_MONTH_CNT_MAX - 1) ||
			(day->tm_year < CAL_YEAR_MIN - BENCHMARK_YEAR ) ||
			(day->tm_year > CAL_YEAR_MAX - BENCHMARK_YEAR))
	{
		day->tm_year = 137;
		day->tm_mon = 11;
		day->tm_mday = 31;
		return CAL_TRUE;
	}
	else
	{
		return CAL_TRUE;
	}
}

static inline int _cals_sch_check_validity(cal_sch_full_t *sch_record)
{
	struct tm end_date_time = {0};
	struct tm start_date = {0};
	struct tm tmp_start_date_time = {0};
	struct tm tmp_end_date_time = {0};

	time_t temp_start_time_seconds = 0;
	time_t temp_end_time_seconds = 0;
	time_t temp_repeat_until_seconds = 0;

	if(sch_record->cal_type != CAL_EVENT_SCHEDULE_TYPE)
		return CAL_SUCCESS;

	if((sch_record->repeat_term != CAL_REPEAT_NONE) &&
		(sch_record->repeat_occurrences) != 0 &&
		(sch_record->repeat_end_date.tm_year == BASE_TIME_YEAR))
	{
		cal_db_service_set_repeat_end_date(sch_record);
	}

	if (!_cals_check_date_validity(&sch_record->start_date_time))
	{
		//ERR("start_date_time is invalied: %s-%s-%s",sch_record->start_date_time.tm_year,
		//sch_record->start_date_time.tm_mon,
		//sch_record->start_date_time.tm_mday);
		return CAL_ERR_EVENT_START_DATE;
	}

	if (!_cals_check_date_validity(&sch_record->end_date_time))
	{
		//ERR("end_date_time is invalied: %s-%s-%s",sch_record->end_date_time.tm_year,
		//								 sch_record->end_date_time.tm_mon,
		//								 sch_record->end_date_time.tm_mday);
		return CAL_ERR_EVENT_END_DATE;
	}


	if (!_cals_check_date_validity(&sch_record->repeat_end_date))
	{
		//ERR("repeat_end_date is invalied: %s-%s-%s",sch_record->repeat_end_date.tm_year,
		//								 sch_record->repeat_end_date.tm_mon,
		//								 sch_record->repeat_end_date.tm_mday);
		return CAL_ERR_EVENT_REPEAT_END_DATE;
	}

	start_date.tm_year	= sch_record->start_date_time.tm_year;
	start_date.tm_mon	= sch_record->start_date_time.tm_mon;
	start_date.tm_mday	= sch_record->start_date_time.tm_mday;

	memcpy( &tmp_start_date_time, &sch_record->start_date_time, sizeof(struct tm) );
	memcpy( &tmp_end_date_time, &sch_record->end_date_time, sizeof(struct tm) );

	temp_start_time_seconds = timegm(&tmp_start_date_time);
	temp_end_time_seconds = timegm(&tmp_end_date_time);
	temp_repeat_until_seconds = timegm(&sch_record->repeat_end_date);

	if( temp_start_time_seconds > temp_end_time_seconds )
	{
		return CAL_ERR_EVENT_DURATION;
	}

	switch(sch_record->repeat_term)
	{
	case CAL_REPEAT_EVERY_DAY:
		if( temp_end_time_seconds - temp_start_time_seconds > (sch_record->repeat_interval*ONE_DAY_SECONDS))
			return CAL_ERR_EVENT_REPEAT_DURATION_TOO_SHORT;
		break;
		/*	case CAL_REPEAT_EVERY_WEEKDAYS:
			if( temp_end_time_seconds - temp_start_time_seconds > (sch_record->repeat_interval*ONE_DAY_SECONDS))
			{
		 *error_code = CAL_ERR_EVENT_REPEAT_DURATION_TOO_SHORT;
		 return false;
		 }
		 break;
		 case CAL_REPEAT_EVERY_WEEKENDS:
		 if( temp_end_time_seconds - temp_start_time_seconds > (sch_record->repeat_interval*ONE_DAY_SECONDS))
		 {
		 *error_code = CAL_ERR_EVENT_REPEAT_DURATION_TOO_SHORT;
		 return false;
		 }
		 break;*/
	case CAL_REPEAT_EVERY_WEEK:
		//if( temp_end_time_seconds - temp_start_time_seconds > (sch_record->repeat_interval*ONE_WEEK_SECONDS))
		//{
		//	*error_code = CAL_ERR_EVENT_REPEAT_DURATION_TOO_SHORT;
		//	return false;
		//}
		break;
	case CAL_REPEAT_EVERY_MONTH:
		if( temp_end_time_seconds - temp_start_time_seconds > (sch_record->repeat_interval*ONE_MONTH_SECONDS))
			return CAL_ERR_EVENT_REPEAT_DURATION_TOO_SHORT;
		break;
	case CAL_REPEAT_EVERY_YEAR:
		if( temp_end_time_seconds - temp_start_time_seconds > (sch_record->repeat_interval*ONE_YEAR_SECONDS))
			return CAL_ERR_EVENT_REPEAT_DURATION_TOO_SHORT;
		break;
	case CAL_REPEAT_NONE:
	default:
		break;
	}

	end_date_time.tm_year = sch_record->repeat_end_date.tm_year;
	end_date_time.tm_mon = sch_record->repeat_end_date.tm_mon;
	end_date_time.tm_mday = sch_record->repeat_end_date.tm_mday;
	end_date_time.tm_hour = sch_record->repeat_end_date.tm_hour;
	end_date_time.tm_min = sch_record->repeat_end_date.tm_min;
	end_date_time.tm_sec = 59;

	// If Repeat end date is earlier than end date
	if ( sch_record->repeat_term != CAL_REPEAT_NONE )
	{
		memcpy(&tmp_end_date_time, &sch_record->end_date_time, sizeof(struct tm));
		//if( timegm(&tmp_end_date_time) > timegm(&end_date_time) )
		//{
		//	*error_code = CAL_ERR_REPEAT_DURATION;
		//	return false;
		//}
	}
	return CAL_SUCCESS;
}


int cals_insert_schedule(cal_sch_full_t *sch_record)
{
	int ret = 0;
	int index = 0;
	cal_value *cvalue = NULL;
	bool is_success = false;

	retvm_if(NULL == sch_record, CAL_ERR_ARG_INVALID, "sch_record is NULL");

	ret = _cals_sch_check_validity(sch_record);
	retvm_if(CAL_SUCCESS != ret, ret, "_cals_sch_check_validity() is Failed(%d)", ret);

	switch(sch_record->sch_category)
	{
	case CAL_SCH_NONE:
		retvm_if(CAL_EVENT_TODO_TYPE != sch_record->cal_type, CAL_ERR_ARG_INVALID,
			"Invalid type(category = %d, component type = %d)", sch_record->sch_category, sch_record->cal_type);
		break;
	case CAL_SCH_APPOINTMENT:
		retvm_if(CAL_EVENT_SCHEDULE_TYPE != sch_record->cal_type, CAL_ERR_ARG_INVALID,
			"Invalid type(category = %d, component type = %d)", sch_record->sch_category, sch_record->cal_type);
		break;
	case CAL_SCH_SPECIAL_OCCASION:
		retvm_if(CAL_EVENT_SCHEDULE_TYPE != sch_record->cal_type, CAL_ERR_ARG_INVALID,
			"Invalid type(category = %d, component type = %d)", sch_record->sch_category, sch_record->cal_type);
		sch_record->all_day_event = 1;
		break;
	case CAL_SCH_BIRTHDAY:
		retvm_if(CAL_EVENT_SCHEDULE_TYPE != sch_record->cal_type, CAL_ERR_ARG_INVALID,
			"Invalid type(category = %d, component type = %d)", sch_record->sch_category, sch_record->cal_type);
		sch_record->all_day_event = 1;
		break;
	case CAL_SCH_HOLIDAY:
		retvm_if(CAL_EVENT_SCHEDULE_TYPE != sch_record->cal_type, CAL_ERR_ARG_INVALID,
			"Invalid type(category = %d, component type = %d)", sch_record->sch_category, sch_record->cal_type);
		sch_record->all_day_event = 1;
		break;
	case CAL_SCH_IMPORTANT:
	case CAL_SCH_PRIVATE:
	case CAL_SCH_BUSSINESS:
	default:
		ERR("Invalid type(category = %d, component type = %d)", sch_record->sch_category, sch_record->cal_type);
		return CAL_ERR_ARG_INVALID;
	}

	sch_record->missed = 0;

	ret = cals_begin_trans();
	retvm_if(ret, ret, "cals_begin_trans() is Failed(%d)", ret);

	ret = _cals_insert_schedule(sch_record);
	if(ret < CAL_SUCCESS) {
		ERR("_cals_insert_schedule() Failed(%d)", ret);
		cals_end_trans(false);
		return ret;
	}
	index = ret;

	if (sch_record->attendee_list)
	{
		GList *list = g_list_first(sch_record->attendee_list);
		cal_participant_info_t *participant_info = NULL;

		while(list)
		{
			cvalue = list->data;
			if(cvalue)
			{
				participant_info = cvalue->user_data;
				if(participant_info->is_deleted==0)
				{
					ret = cal_service_add_participant_info(index, participant_info);
					warn_if(ret, "cal_service_add_participant_info() Failed(%d)", ret);
				}
			}
			list = g_list_next(list);
		}
	}

	if (sch_record->meeting_category)
	{
		GList *list = g_list_first(sch_record->meeting_category);
		cal_category_info_t *category_info = NULL;

		while(list)
		{
			cvalue = list->data;
			if(cvalue)
			{
				category_info = cvalue->user_data;
				ret = _cals_add_meeting_category_info(index, category_info);
				warn_if(CAL_SUCCESS != ret, "_cals_add_meeting_category_info() Failed(%d)", ret);
			}
			list = g_list_next(list);
		}
	}

	if (sch_record->exception_date_list)
	{
		GList *list = g_list_first(sch_record->exception_date_list);
		cal_exception_info_t *exception_info = NULL;

		while(list)
		{
			cvalue = list->data;
			if(cvalue)
			{
				exception_info = cvalue->user_data;
				ret = cal_service_add_exception_info(index, exception_info, sch_record);
				warn_if(CAL_SUCCESS != ret, "cal_service_add_exception_info() Failed(%d)", ret);
			}
			list = g_list_next(list);
		}
	}

	if (sch_record->alarm_list)
	{
		GList *list = sch_record->alarm_list;
		cal_alarm_info_t *alarm_info = NULL;

		while (list)
		{
			cvalue = list->data;
			if (cvalue)
			{
				alarm_info = cvalue->user_data;
				if(alarm_info->is_deleted==0)
				{
					if (alarm_info->remind_tick != CAL_INVALID_INDEX)
					{
						ret = cals_alarm_add(index, alarm_info, &sch_record->start_date_time);
						warn_if(CAL_SUCCESS != ret, "cals_alarm_add() Failed(%d)", ret);
					}
				}
			}
			list = list->next;
		}
	}

	cals_end_trans(true);
	sch_record->index = index;

	if(sch_record->cal_type == CAL_EVENT_SCHEDULE_TYPE)
		is_success= cals_notify(CALS_NOTI_TYPE_EVENT);
	else
		is_success= cals_notify(CALS_NOTI_TYPE_TODO);
	warn_if(is_success != true, "cals_notify() Failed");

	return index;
}


static int _cals_delete_meeting_category_info(const int index)
{
	int ret;
	char query[CALS_SQL_MIN_LEN];

	sprintf(query, "DELETE FROM %s WHERE event_id = %d;", CALS_TABLE_MEETING_CATEGORY, index);

	ret = cals_query_exec(query);
	retvm_if(CAL_SUCCESS != ret, ret, "cals_query_exec() Failed(%d)", ret);

	return CAL_SUCCESS;
}


static int _cals_delete_participant_info(const int index)
{
	int ret;
	char query[CALS_SQL_MIN_LEN];

	sprintf(query, "DELETE FROM %s WHERE event_id = %d", CALS_TABLE_PARTICIPANT, index);

	ret = cals_query_exec(query);
	retvm_if(CAL_SUCCESS != ret, ret, "cals_query_exec() Failed(%d)", ret);

	return CAL_SUCCESS;
}


static int _cals_delete_recurrency_log(const int event_id)
{
	int ret;
	char query[CALS_SQL_MIN_LEN] = {0};

	sprintf(query, "DELETE FROM %s WHERE event_id = %d", CALS_TABLE_RECURRENCY_LOG, event_id);

	ret = cals_query_exec(query);
	retvm_if(CAL_SUCCESS != ret, ret, "cals_query_exec() Failed(%d)", ret);

	return CAL_SUCCESS;
}


static inline int _cals_update_schedule(const int index, cal_sch_full_t *current_record)
{
	int ret = -1;
	char sql_value[CALS_SQL_MAX_LEN] = {0};
	sqlite3_stmt *stmt = NULL;
	time_t conv_start_time;
	time_t conv_end_time;
	time_t conv_repeat_end_time;
	time_t conv_created_date_time;
	time_t conv_completed_date_time;
	time_t t = time(NULL);
	tzset();
	struct tm * ptm = NULL;
	struct tm ttm;

	retv_if(NULL == current_record, CAL_ERR_ARG_NULL);

	ptm = gmtime_r(&t, &ttm);
	retvm_if(NULL == ptm, CAL_ERR_TIME_FAILED, "gmtime_r() Failed(%d)", errno);
	memcpy(&current_record->last_modified_time, &ttm, sizeof(struct tm));

	_cals_adjust_time(current_record,&conv_start_time,&conv_end_time,&conv_repeat_end_time);
	conv_created_date_time = cals_mktime(&(current_record->created_date_time));
	conv_completed_date_time = cals_mktime(&(current_record->completed_date_time));

	if (CAL_SYNC_STATUS_UPDATED != current_record->sync_status)
		current_record->sync_status = CAL_SYNC_STATUS_UPDATED;

	snprintf(sql_value, sizeof(sql_value), "UPDATE %s set "
			"type = %d,"
			"category = %d,"
			"summary = ?,"
			"description = ?,"
			"location = ?,"
			"all_day_event = %d,"
			"start_date_time = %ld,"
			"end_date_time = %ld,"
			"repeat_item = %d,"
			"repeat_interval = %d,"
			"repeat_occurrences = %d,"
			"repeat_end_date = %ld,"
			"sun_moon = %d,"
			"week_start = %d,"
			"week_flag = ?,"
			"day_date = %d,"
			"last_modified_time = %ld,"
			"missed = %d,"
			"task_status = %d,"
			"priority = %d,"
			"timezone = %d, "
			"file_id = %d, "
			"contact_id = %d, "
			"busy_status = %d, "
			"sensitivity = %d, "
			"uid = ?, "
			"calendar_type = %d, "
			"organizer_name = ?, "
			"organizer_email = ?, "
			"meeting_status = %d, "
			"gcal_id = ?, "
			"deleted = %d, "
			"updated = ?, "
			"location_type = %d, "
			"location_summary = ?, "
			"etag = ?, "
			"calendar_id = %d, "
			"sync_status = %d, "
			"edit_uri = ?, "
			"gevent_id = ?, "
			"dst = %d,"
			"original_event_id = %d,"
			"latitude = %lf,"
			"longitude = %lf,"
			"is_deleted = %d,"
			"tz_name = ?,"
			"tz_city_name = ?,"
			"email_id = %d,"
			"availability = %d,"
			"created_date_time = %ld,"
			"completed_date_time = %ld,"
			"progress = %d "
			"WHERE id = %d;",
		CALS_TABLE_SCHEDULE,
		current_record->cal_type,
		current_record->sch_category,
		current_record->all_day_event,
		(long int)conv_start_time,
		(long int)conv_end_time,
		current_record->repeat_term,
		current_record->repeat_interval,
		current_record->repeat_occurrences,
		(long int)conv_repeat_end_time,
		current_record->sun_moon,
		current_record->week_start,
		current_record->day_date,
		(long int)cals_mktime(&(current_record->last_modified_time)),
		current_record->missed,
		current_record->task_status,
		current_record->priority,
		current_record->timezone,
		current_record->file_id,
		current_record->contact_id,
		current_record->busy_status,
		current_record->sensitivity,
		current_record->calendar_type,
		current_record->meeting_status,
		current_record->deleted,
		current_record->location_type,
		current_record->calendar_id,
		current_record->sync_status,
		current_record->dst,
		current_record->original_event_id,
		current_record->latitude,
		current_record->longitude,
		current_record->is_deleted,
		current_record->email_id,
		current_record->availability,
		(long int)conv_created_date_time,
		(long int)conv_completed_date_time,
		current_record->progress,
		index);

	stmt = cals_query_prepare(sql_value);
	retvm_if(NULL == stmt, CAL_ERR_DB_FAILED, "cals_query_prepare() Failed");

	int count = 1;

	if (current_record->summary)
		cals_stmt_bind_text(stmt, count, current_record->summary);
	count++;

	if (current_record->description)
		cals_stmt_bind_text(stmt, count, current_record->description);
	count++;

	if (current_record->location)
		cals_stmt_bind_text(stmt, count, current_record->location);
	count++;

	if (current_record->week_flag)
		cals_stmt_bind_text(stmt, count, current_record->week_flag);
	count++;

	if (current_record->uid)
		cals_stmt_bind_text(stmt, count, current_record->uid);
	count++;

	if (current_record->organizer_name)
		cals_stmt_bind_text(stmt, count, current_record->organizer_name);
	count++;

	if (current_record->organizer_email)
		cals_stmt_bind_text(stmt, count, current_record->organizer_email);
	count++;

	if (current_record->gcal_id)
		cals_stmt_bind_text(stmt, count, current_record->gcal_id);
	count++;

	if (current_record->updated)
		cals_stmt_bind_text(stmt, count, current_record->updated);
	count++;

	if (current_record->location_summary)
		cals_stmt_bind_text(stmt, count, current_record->location_summary);
	count++;

	if (current_record->etag)
		cals_stmt_bind_text(stmt, count, current_record->etag);
	count++;

	if (current_record->edit_uri)
		cals_stmt_bind_text(stmt, count, current_record->edit_uri);
	count++;

	if (current_record->gevent_id)
		cals_stmt_bind_text(stmt, count, current_record->gevent_id);
	count++;

	if (current_record->tz_name)
		cals_stmt_bind_text(stmt, count, current_record->tz_name);
	count++;

	if (current_record->tz_city_name)
		cals_stmt_bind_text(stmt, count, current_record->tz_city_name);
	count++;

	ret = cals_stmt_step(stmt);
	if (CAL_SUCCESS != ret) {
		sqlite3_finalize(stmt);
		ERR("sqlite3_step() Failed(%d)", ret);
		return ret;
	}
	sqlite3_finalize(stmt);

	return CAL_SUCCESS;
}

int cals_update_schedule(const int index, cal_sch_full_t *sch_record)
{
	bool is_success = false;
	cal_value * cvalue = NULL;
	int ret = 0;

	retv_if(NULL == sch_record, CAL_ERR_ARG_NULL);

	sch_record->missed = 0;

	if((sch_record->repeat_occurrences) != 0 && (sch_record->repeat_end_date.tm_year == BASE_TIME_YEAR))
	{
		cal_db_service_set_repeat_end_date(sch_record);
	}

	ret = _cals_update_schedule(index, sch_record);
	retvm_if(CAL_SUCCESS != ret, ret, "_cals_update_schedule() Failed(%d)", ret);

	_cals_delete_participant_info(index);
	if (sch_record->attendee_list)
	{
		GList *list = g_list_first(sch_record->attendee_list);
		cal_participant_info_t* participant_info = NULL;

		while(list)
		{
			cvalue = (cal_value *)list->data;
			participant_info = (cal_participant_info_t*)cvalue->user_data;

			if (0 == participant_info->is_deleted) {
				ret = cal_service_add_participant_info(index, participant_info);
				warn_if(ret, "cal_service_add_participant_info() Failed(%d)", ret);
			}

			list = g_list_next(list);
		}
	}

	_cals_delete_meeting_category_info(index);
	if (sch_record->meeting_category)
	{
		GList *list = g_list_first(sch_record->meeting_category);
		cal_category_info_t* category_info = NULL;

		while (list)
		{
			cvalue = (cal_value *)list->data;
			category_info = (cal_category_info_t*)cvalue->user_data;
			ret = _cals_add_meeting_category_info(index,category_info);
			warn_if(CAL_SUCCESS != ret, "_cals_add_meeting_category_info() Failed(%d)", ret);
			list = g_list_next(list);
		}
	}

	_cals_delete_recurrency_log(index);
	if (sch_record->exception_date_list)
	{
		GList *list = g_list_first(sch_record->exception_date_list);
		cal_exception_info_t* exception_info = NULL;

		while(list)
		{
			cvalue = (cal_value *)list->data;
			if(cvalue)
			{
				exception_info = (cal_exception_info_t*)cvalue->user_data;
				ret = cal_service_add_exception_info(index, exception_info, sch_record);
				warn_if(CAL_SUCCESS != ret, "cal_service_add_exception_info() Failed(%d)", ret);
			}
			list = g_list_next(list);
		}
	}

	cals_alarm_remove(CALS_ALARM_REMOVE_BY_EVENT_ID, index);
	if (sch_record->alarm_list)
	{
		GList *list = sch_record->alarm_list;
		cal_alarm_info_t *alarm_info = NULL;

		while (list)
		{
			cvalue = (cal_value *)list->data;
			alarm_info = (cal_alarm_info_t*)cvalue->user_data;

			if (alarm_info->is_deleted==0) {
				if (alarm_info->remind_tick != CAL_INVALID_INDEX) {
					ret = cals_alarm_add(index, alarm_info, &sch_record->start_date_time);
					warn_if(CAL_SUCCESS != ret, "cals_alarm_add() Failed(%d)", ret);
				}
			}

			list = g_list_next(list);
		}
	}

	if(sch_record->cal_type == CAL_EVENT_SCHEDULE_TYPE)
		is_success= cals_notify(CALS_NOTI_TYPE_EVENT);
	else
		is_success= cals_notify(CALS_NOTI_TYPE_TODO);

	return CAL_SUCCESS;
}


static inline int _cals_update_recurrency_log(const int exception_event_id)
{
	int ret;
	char query[CALS_SQL_MIN_LEN];

	sprintf(query, "UPDATE %s SET exception_event_id=-1 WHERE exception_event_id=%d",
			CALS_TABLE_RECURRENCY_LOG, exception_event_id);

	ret = cals_query_exec(query);
	retvm_if(CAL_SUCCESS != ret, ret, "cals_query_exec() Failed(%d)", ret);

	return CAL_SUCCESS;
}


int cals_delete_schedule(const int index)
{
	bool is_success = false;
	sqlite3_stmt *stmt = NULL;
	int ret = 0;
	char sql_value[CALS_SQL_MAX_LEN] = {0};
	calendar_type_t type = CAL_PHONE_CALENDAR;
	int original_event_id = CAL_INVALID_INDEX;
	int repeat_item = 0;
	time_t current_time = time(NULL);
	int child_index = 0;
	int cal_type = 0;

	sprintf(sql_value, "SELECT calendar_type,original_event_id,repeat_item "
		"FROM %s WHERE id=%d AND is_deleted = 0", CALS_TABLE_SCHEDULE, index);

	stmt = cals_query_prepare(sql_value);
	retvm_if(NULL == stmt, CAL_ERR_DB_FAILED, "cals_query_prepare() Failed");

	ret = cals_stmt_step(stmt);
	if (CAL_TRUE == ret) {
		type = sqlite3_column_int(stmt, 0);

		original_event_id = sqlite3_column_int(stmt, 1);
		repeat_item = sqlite3_column_int(stmt, 2);
	} else if (CAL_SUCCESS != ret) {
		sqlite3_finalize(stmt);
		ERR("cals_stmt_step() Failed(%d)", ret);
		return ret;
	}
	sqlite3_finalize(stmt);
	stmt = NULL;

	if(original_event_id != CAL_INVALID_INDEX) {
		_cals_update_recurrency_log(index);
	} else if(repeat_item != 0) {
		//select child list
		sprintf(sql_value, "SELECT id, type FROM %s WHERE original_event_id=%d AND is_deleted = 0",
			CALS_TABLE_SCHEDULE, index);

		stmt = cals_query_prepare(sql_value);
		retvm_if(NULL == stmt, CAL_ERR_DB_FAILED, "cals_query_prepare() Failed");

		while (CAL_TRUE == cals_stmt_step(stmt))
		{
			child_index =sqlite3_column_int(stmt, 0);
			cal_type = sqlite3_column_int(stmt,1);

			memset(sql_value,'\0',CALS_SQL_MAX_LEN);
			sprintf(sql_value, "UPDATE %s SET is_deleted = 1,sync_status = %d,last_modified_time = %ld WHERE id = %d",
					CALS_TABLE_SCHEDULE, CAL_SYNC_STATUS_DELETED, (long int)current_time, child_index);

			ret = cals_query_exec(sql_value);
			if (CAL_SUCCESS != ret) {
				sqlite3_finalize(stmt);
				ERR("cals_query_exec() Failed(%d)", ret);
				return ret;
			}

			ret = cals_alarm_remove(CALS_ALARM_REMOVE_BY_EVENT_ID, child_index);
			if (CAL_SUCCESS != ret) {
				sqlite3_finalize(stmt);
				ERR("cals_alarm_remove() Failed(%d)", ret);
				return ret;
			}
		}
		sqlite3_finalize(stmt);
	}

	//delete original event
	sprintf(sql_value, "UPDATE %s SET is_deleted = 1,sync_status = %d,last_modified_time = %ld WHERE id = %d;",
			CALS_TABLE_SCHEDULE, CAL_SYNC_STATUS_DELETED,(long int)current_time, index);

	ret = cals_query_exec(sql_value);
	retvm_if(CAL_SUCCESS != ret, ret, "cals_query_exec() Failed(%d)", ret);

	ret = cals_alarm_remove(CALS_ALARM_REMOVE_BY_EVENT_ID, index);
	retvm_if(CAL_SUCCESS != ret, ret, "cals_alarm_remove() Failed(%d)", ret);

	_cals_delete_recurrency_log(index);
	_cals_delete_participant_info(index);
	_cals_delete_meeting_category_info(index);

	if(cal_type == CAL_EVENT_SCHEDULE_TYPE)
		is_success= cals_notify(CALS_NOTI_TYPE_EVENT);
	else
		is_success= cals_notify(CALS_NOTI_TYPE_TODO);
	warn_if(is_success != true, , "cals_notify() Failed");

	return CAL_SUCCESS;
}


int cals_rearrage_schedule_field(const char *src, char *dest, int dest_size)
{
	int ret = 0;
	if (strstr(src, CAL_VALUE_INT_INDEX))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_INT_INDEX);

	if (strstr(src,CAL_VALUE_INT_ACCOUNT_ID))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_INT_ACCOUNT_ID);

	if (strstr(src,CAL_VALUE_INT_TYPE))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_INT_TYPE);

	if (strstr(src,CAL_VALUE_INT_CATEGORY))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_INT_CATEGORY);

	if (strstr(src,CAL_VALUE_TXT_SUMMARY))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_TXT_SUMMARY);

	if (strstr(src,CAL_VALUE_TXT_DESCRIPTION))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_TXT_DESCRIPTION);

	if (strstr(src,CAL_VALUE_TXT_LOCATION))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_TXT_LOCATION);

	if(strstr(src,CAL_VALUE_INT_ALL_DAY_EVENT))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_INT_ALL_DAY_EVENT);

	if(strstr(src,CAL_VALUE_GMT_START_DATE_TIME))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_GMT_START_DATE_TIME);

	if(strstr(src, CAL_VALUE_GMT_END_DATE_TIME))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_GMT_END_DATE_TIME);

	if(strstr(src, CAL_VALUE_INT_REPEAT_TERM))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_INT_REPEAT_TERM);

	if(strstr(src, CAL_VALUE_INT_REPEAT_INTERVAL))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_INT_REPEAT_INTERVAL);

	if(strstr(src, CAL_VALUE_INT_REPEAT_OCCURRENCES))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_INT_REPEAT_OCCURRENCES);

	if(strstr(src,CAL_VALUE_GMT_REPEAT_END_DATE))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_GMT_REPEAT_END_DATE);

	if(strstr(src,CAL_VALUE_INT_SUN_MOON))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_INT_SUN_MOON);

	if(strstr(src,CAL_VALUE_INT_WEEK_START))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_INT_WEEK_START);

	if(strstr(src,CAL_VALUE_TXT_WEEK_FLAG))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_TXT_WEEK_FLAG);

	if(strstr(src,CAL_VALUE_INT_DAY_DATE))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_INT_DAY_DATE);

	if(strstr(src,CAL_VALUE_GMT_LAST_MODIFIED_TIME))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_GMT_LAST_MODIFIED_TIME);

	if(strstr(src,CAL_VALUE_INT_MISSED))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_INT_MISSED);

	if(strstr(src,CAL_VALUE_INT_TASK_STATUS))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_INT_TASK_STATUS);

	if(strstr(src,CAL_VALUE_INT_PRIORITY))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_INT_PRIORITY);

	if(strstr(src,CAL_VALUE_INT_TIMEZONE))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_INT_TIMEZONE);

	if(strstr(src,CAL_VALUE_INT_FILE_ID))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_INT_FILE_ID);

	if(strstr(src,CAL_VALUE_INT_CONTACT_ID))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_INT_CONTACT_ID);

	if(strstr(src,CAL_VALUE_INT_BUSY_STATUS))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_INT_BUSY_STATUS);

	if(strstr(src,CAL_VALUE_INT_SENSITIVITY))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_INT_SENSITIVITY);

	if(strstr(src,CAL_VALUE_TXT_UID))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_TXT_UID);

	if(strstr(src,CAL_VALUE_INT_CALENDAR_TYPE))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_INT_CALENDAR_TYPE);

	if(strstr(src,CAL_VALUE_TXT_ORGANIZER_NAME))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_TXT_ORGANIZER_NAME);

	if(strstr(src,CAL_VALUE_TXT_ORGANIZER_EMAIL))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_TXT_ORGANIZER_EMAIL);

	if(strstr(src, CAL_VALUE_INT_MEETING_STATUS))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_INT_MEETING_STATUS);

	if(strstr(src,CAL_VALUE_TXT_GCAL_ID))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_TXT_GCAL_ID);

	if(strstr(src,CAL_VALUE_INT_DELETED))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_INT_DELETED);

	if(strstr(src,CAL_VALUE_TXT_UPDATED))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_TXT_UPDATED);

	if(strstr(src,CAL_VALUE_INT_LOCATION_TYPE))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_INT_LOCATION_TYPE);

	if(strstr(src,CAL_VALUE_TXT_LOCATION_SUMMARY))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_TXT_LOCATION_SUMMARY);

	if(strstr(src,CAL_VALUE_TXT_ETAG))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_TXT_ETAG);

	if(strstr(src,CAL_VALUE_INT_CALENDAR_ID))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_INT_CALENDAR_ID);

	if(strstr(src,CAL_VALUE_INT_SYNC_STATUS))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_INT_SYNC_STATUS);

	if(strstr(src,CAL_VALUE_TXT_EDIT_URL))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_TXT_EDIT_URL);

	if(strstr(src,CAL_VALUE_TXT_GEDERID))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_TXT_GEDERID);

	if(strstr(src,CAL_VALUE_INT_DST))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_INT_DST);

	if(strstr(src,CAL_VALUE_INT_ORIGINAL_EVENT_ID))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_INT_ORIGINAL_EVENT_ID);

	if(strstr(src,CAL_VALUE_DBL_LATITUDE))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_DBL_LATITUDE);

	if(strstr(src,CAL_VALUE_DBL_LONGITUDE))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_DBL_LONGITUDE);

	if(strstr(src,CAL_VALUE_INT_IS_DELETED))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_INT_IS_DELETED);

	if(strstr(src,CAL_VALUE_TXT_TZ_NAME))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_TXT_TZ_NAME);

	if(strstr(src,CAL_VALUE_TXT_TZ_CITY_NAME))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_TXT_TZ_CITY_NAME);

	if(strstr(src,CAL_VALUE_INT_EMAIL_ID))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_INT_EMAIL_ID);

	if(strstr(src,CAL_VALUE_INT_AVAILABILITY))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_INT_AVAILABILITY);

	if(strstr(src,CAL_VALUE_GMT_CREATED_DATE_TIME))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_GMT_CREATED_DATE_TIME);

	if(strstr(src,CAL_VALUE_GMT_COMPLETED_DATE_TIME))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_GMT_COMPLETED_DATE_TIME);

	if(strstr(src,CAL_VALUE_INT_PROGRESS))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_INT_PROGRESS);

	return CAL_SUCCESS;
}

int cals_stmt_get_filted_schedule(sqlite3_stmt *stmt,
		cal_sch_full_t *sch_record, const char *select_field)
{
	int count = 0;
	int ivalue = 0;
	const unsigned char *temp;
	const char *start, *result;

	retv_if(NULL == stmt, CAL_ERR_ARG_NULL);
	retv_if(NULL == sch_record, CAL_ERR_ARG_NULL);
	retv_if(NULL == select_field, CAL_ERR_ARG_NULL);

	start = select_field;
	if((result = strstr(start, CAL_VALUE_INT_INDEX))) {
		sch_record->index = sqlite3_column_int(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_INT_ACCOUNT_ID))) {
		sch_record->account_id = sqlite3_column_int(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_INT_TYPE))) {
		sch_record->cal_type = sqlite3_column_int(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_INT_CATEGORY))) {
		sch_record->sch_category = sqlite3_column_int(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_TXT_SUMMARY))) {
		temp = sqlite3_column_text(stmt, count++);
		sch_record->summary = SAFE_STRDUP(temp);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_TXT_DESCRIPTION))) {
		temp = sqlite3_column_text(stmt, count++);
		sch_record->description = SAFE_STRDUP(temp);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_TXT_LOCATION))) {
		temp = sqlite3_column_text(stmt, count++);
		sch_record->location = SAFE_STRDUP(temp);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_INT_ALL_DAY_EVENT))) {
		sch_record->all_day_event = sqlite3_column_int(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_GMT_START_DATE_TIME))) {
		ivalue = sqlite3_column_int(stmt, count++);
		cal_db_service_copy_struct_tm((struct tm*)cals_tmtime((time_t*)&ivalue ),&(sch_record->start_date_time));
		start = result;
	}

	if((result = strstr(start, CAL_VALUE_GMT_END_DATE_TIME))) {
		ivalue = sqlite3_column_int(stmt, count++);
		cal_db_service_copy_struct_tm((struct tm*)cals_tmtime((time_t*)&ivalue),&(sch_record->end_date_time));
		start = result;
	}

	if((result = strstr(start, CAL_VALUE_INT_REPEAT_TERM))) {
		sch_record->repeat_term = sqlite3_column_int(stmt, count++);
		start = result;
	}

	if((result = strstr(start, CAL_VALUE_INT_REPEAT_INTERVAL))) {
		sch_record->repeat_interval = sqlite3_column_int(stmt, count++);
		start = result;
	}

	if((result = strstr(start, CAL_VALUE_INT_REPEAT_OCCURRENCES))) {
		sch_record->repeat_occurrences = sqlite3_column_int(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_GMT_REPEAT_END_DATE))) {
		ivalue = sqlite3_column_int(stmt, count++);
		cal_db_service_copy_struct_tm((struct tm*)cals_tmtime((time_t*)&ivalue),&(sch_record->repeat_end_date));
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_INT_SUN_MOON))) {
		sch_record->sun_moon = sqlite3_column_int(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_INT_WEEK_START))) {
		sch_record->week_start = sqlite3_column_int(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_TXT_WEEK_FLAG))) {
		temp = sqlite3_column_text(stmt, count++);
		sch_record->week_flag = SAFE_STRDUP(temp);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_INT_DAY_DATE))) {
		sch_record->day_date = sqlite3_column_int(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_GMT_LAST_MODIFIED_TIME))) {
		ivalue = sqlite3_column_int(stmt, count++);
		cal_db_service_copy_struct_tm((struct tm*)cals_tmtime((time_t*)&ivalue),&(sch_record->last_modified_time));
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_INT_MISSED))) {
		sch_record->missed = sqlite3_column_int(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_INT_TASK_STATUS))) {
		sch_record->task_status = sqlite3_column_int(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_INT_PRIORITY))) {
		sch_record->priority = sqlite3_column_int(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_INT_TIMEZONE))) {
		sch_record->timezone = sqlite3_column_int(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_INT_FILE_ID))) {
		sch_record->file_id = sqlite3_column_int(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_INT_CONTACT_ID))) {
		sch_record->contact_id = sqlite3_column_int(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_INT_BUSY_STATUS))) {
		sch_record->busy_status = sqlite3_column_int(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_INT_SENSITIVITY))) {
		sch_record->sensitivity = sqlite3_column_int(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_TXT_UID))) {
		temp = sqlite3_column_text(stmt, count++);
		sch_record->uid = SAFE_STRDUP(temp);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_INT_CALENDAR_TYPE))) {
		sch_record->calendar_type = sqlite3_column_int(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_INT_CALENDAR_TYPE))) {
		sch_record->calendar_type = sqlite3_column_int(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_TXT_ORGANIZER_NAME))) {
		temp = sqlite3_column_text(stmt, count++);
		sch_record->organizer_name = SAFE_STRDUP(temp);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_TXT_ORGANIZER_EMAIL))) {
		temp = sqlite3_column_text(stmt, count++);
		sch_record->organizer_email = SAFE_STRDUP(temp);
		start = result;
	}

	if((result = strstr(start, CAL_VALUE_INT_MEETING_STATUS))) {
		sch_record->meeting_status = sqlite3_column_int(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_TXT_GCAL_ID))) {
		temp = sqlite3_column_text(stmt, count++);
		sch_record->gcal_id = SAFE_STRDUP(temp);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_INT_DELETED))) {
		sch_record->deleted = sqlite3_column_int(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_TXT_UPDATED))) {
		temp = sqlite3_column_text(stmt, count++);
		sch_record->updated = SAFE_STRDUP(temp);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_INT_LOCATION_TYPE))) {
		sch_record->location_type = sqlite3_column_int(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_TXT_LOCATION_SUMMARY))) {
		temp = sqlite3_column_text(stmt, count++);
		sch_record->location_summary = SAFE_STRDUP(temp);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_TXT_ETAG))) {
		temp = sqlite3_column_text(stmt, count++);
		sch_record->etag = SAFE_STRDUP(temp);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_INT_CALENDAR_ID))) {
		sch_record->calendar_id = sqlite3_column_int(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_INT_SYNC_STATUS))) {
		sch_record->sync_status = sqlite3_column_int(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_TXT_EDIT_URL))) {
		temp = sqlite3_column_text(stmt, count++);
		sch_record->edit_uri = SAFE_STRDUP(temp);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_TXT_GEDERID))) {
		temp = sqlite3_column_text(stmt, count++);
		sch_record->gevent_id = SAFE_STRDUP(temp);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_INT_DST))) {
		sch_record->dst = sqlite3_column_int(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_INT_ORIGINAL_EVENT_ID))) {
		sch_record->original_event_id = sqlite3_column_int(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_DBL_LATITUDE))) {
		sch_record->latitude = sqlite3_column_double(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_DBL_LONGITUDE))) {
		sch_record->longitude = sqlite3_column_double(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_INT_IS_DELETED))) {
		sch_record->is_deleted = sqlite3_column_int(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_TXT_TZ_NAME))) {
		temp = sqlite3_column_text(stmt, count++);
		sch_record->tz_name = SAFE_STRDUP(temp);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_TXT_TZ_CITY_NAME))) {
		temp = sqlite3_column_text(stmt, count++);
		sch_record->tz_city_name = SAFE_STRDUP(temp);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_INT_EMAIL_ID))) {
		sch_record->email_id = sqlite3_column_int(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_INT_AVAILABILITY))) {
		sch_record->availability = sqlite3_column_int(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_GMT_CREATED_DATE_TIME))) {
		ivalue = sqlite3_column_int(stmt, count++);
		cal_db_service_copy_struct_tm((struct tm*)cals_tmtime((time_t*)&ivalue ),&(sch_record->created_date_time));
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_GMT_COMPLETED_DATE_TIME))) {
		ivalue = sqlite3_column_int(stmt, count++);
		cal_db_service_copy_struct_tm((struct tm*)cals_tmtime((time_t*)&ivalue ),&(sch_record->completed_date_time));
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_INT_PROGRESS))) {
		sch_record->progress = sqlite3_column_int(stmt, count++);
		start = result;
	}


	return CAL_SUCCESS;
}


void cals_stmt_get_full_schedule(sqlite3_stmt *stmt,cal_sch_full_t *sch_record, bool is_utc)
{
	int count = 0;
	long int tmp_time = 0;
	const unsigned char *temp;

	sch_record->index = sqlite3_column_int(stmt, count++);
	sch_record->account_id = sqlite3_column_int(stmt, count++);
	sch_record->cal_type = sqlite3_column_int(stmt, count++);
	sch_record->sch_category = sqlite3_column_int(stmt, count++);
	temp = sqlite3_column_text(stmt, count++);
	sch_record->summary = SAFE_STRDUP(temp);
	temp = sqlite3_column_text(stmt, count++);
	sch_record->description = SAFE_STRDUP(temp);
	temp = sqlite3_column_text(stmt, count++);
	sch_record->location = SAFE_STRDUP(temp);
	sch_record->all_day_event = sqlite3_column_int(stmt, count++);

	tmp_time = sqlite3_column_int(stmt, count++);
	cal_db_service_copy_struct_tm((struct tm*)cals_tmtime(&tmp_time),&(sch_record->start_date_time));

	tmp_time = sqlite3_column_int(stmt, count++);
	cal_db_service_copy_struct_tm((struct tm*)cals_tmtime(&tmp_time),&(sch_record->end_date_time));

	sch_record->repeat_term = sqlite3_column_int(stmt, count++);
	sch_record->repeat_interval = sqlite3_column_int(stmt, count++);
	sch_record->repeat_occurrences = sqlite3_column_int(stmt, count++);

	tmp_time = sqlite3_column_int(stmt, count++);
	cal_db_service_copy_struct_tm((struct tm*)cals_tmtime(&tmp_time),&(sch_record->repeat_end_date));

	sch_record->sun_moon = sqlite3_column_int(stmt, count++);
	sch_record->week_start = sqlite3_column_int(stmt, count++);

	temp = sqlite3_column_text(stmt, count++);
	sch_record->week_flag = SAFE_STRDUP(temp);

	sch_record->day_date = sqlite3_column_int(stmt, count++);

	tmp_time = sqlite3_column_int(stmt, count++);
	cal_db_service_copy_struct_tm((struct tm*)cals_tmtime(&tmp_time), &(sch_record->last_modified_time));

	sch_record->missed = sqlite3_column_int(stmt, count++);
	sch_record->task_status = sqlite3_column_int(stmt, count++);
	sch_record->priority = sqlite3_column_int(stmt, count++);
	sch_record->timezone = sqlite3_column_int(stmt, count++);
	sch_record->file_id = sqlite3_column_int(stmt, count++);
	sch_record->contact_id = sqlite3_column_int(stmt, count++);
	sch_record->busy_status = sqlite3_column_int(stmt, count++);
	sch_record->sensitivity = sqlite3_column_int(stmt, count++);

	temp = sqlite3_column_text(stmt, count++);
	sch_record->uid = SAFE_STRDUP(temp);

	sch_record->calendar_type = sqlite3_column_int(stmt, count++);

	temp = sqlite3_column_text(stmt, count++);
	sch_record->organizer_name = SAFE_STRDUP(temp);

	temp = sqlite3_column_text(stmt, count++);
	sch_record->organizer_email = SAFE_STRDUP(temp);

	sch_record->meeting_status = sqlite3_column_int(stmt, count++);

	temp = sqlite3_column_text(stmt, count++);
	sch_record->gcal_id = SAFE_STRDUP(temp);

	sch_record->deleted = sqlite3_column_int(stmt, count++);

	temp = sqlite3_column_text(stmt, count++);
	sch_record->updated = SAFE_STRDUP(temp);

	sch_record->location_type = sqlite3_column_int(stmt, count++);

	temp = sqlite3_column_text(stmt, count++);
	sch_record->location_summary = SAFE_STRDUP(temp);

	temp = sqlite3_column_text(stmt, count++);
	sch_record->etag = SAFE_STRDUP(temp);

	sch_record->calendar_id = sqlite3_column_int(stmt, count++);

	sch_record->sync_status = sqlite3_column_int(stmt, count++);

	temp = sqlite3_column_text(stmt, count++);
	sch_record->edit_uri = SAFE_STRDUP(temp);

	temp = sqlite3_column_text(stmt, count++);
	sch_record->gevent_id = SAFE_STRDUP(temp);

	sch_record->dst = sqlite3_column_int(stmt, count++);

	sch_record->original_event_id = sqlite3_column_int(stmt, count++);

	sch_record->latitude = sqlite3_column_double(stmt,count++);
	sch_record->longitude = sqlite3_column_double(stmt,count++);
	sch_record->deleted = sqlite3_column_int(stmt, count++);

	temp = sqlite3_column_text(stmt, count++);
	sch_record->tz_name = SAFE_STRDUP(temp);

	temp = sqlite3_column_text(stmt, count++);
	sch_record->tz_city_name = SAFE_STRDUP(temp);

	sch_record->email_id = sqlite3_column_int(stmt, count++);

	sch_record->availability = sqlite3_column_int(stmt, count++);

	tmp_time = sqlite3_column_int(stmt, count++);
	cal_db_service_copy_struct_tm((struct tm*)cals_tmtime(&tmp_time),&(sch_record->created_date_time));

	tmp_time = sqlite3_column_int(stmt, count++);
	cal_db_service_copy_struct_tm((struct tm*)cals_tmtime(&tmp_time),&(sch_record->completed_date_time));

	sch_record->progress = sqlite3_column_int(stmt,count++);
}

