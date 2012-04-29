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
#include <time.h>
#include <errno.h>
#include <dlfcn.h>
#include <sqlite3.h>
#include <db-util.h>

#include "cals-internal.h"
#include "calendar-svc-provider.h"
#include "calendar-svc-errors.h"
#include "cals-db-info.h"
#include "cals-typedef.h"
#include "cals-utils.h"
#include "cals-tz-utils.h"
#include "cals-ical.h"
#include "cals-recurrence-utils.h"
#include "cals-alarm.h"
#include "cals-sqlite.h"
#include "cals-schedule.h"
#include "cals-db.h"

extern sqlite3 *calendar_db_handle;

static bool __check_record_index_valid(int index)
{
	if ((index < 0) || (index >= (0x7fffffff)) )
	{
		return false;
	}
	return true;
}

/************************************************************************************************
 *                                                                                               *
 *                               org engine init APIs                                            *
 *                                                                                               *
 ************************************************************************************************/

bool cal_db_service_init_schedule_record(cal_sch_t *sch_record, int *error_code)
{
	time_t t = time(NULL);
	tzset();
	struct tm *tm = NULL; //localtime(&t);
	struct tm ttm;

	tm = localtime_r(&t,&ttm);

	if(NULL == tm)
	{
		return false;
	}
	struct tm temp_date_time = {0};
	struct tm current_time = {0};

	retvm_if(NULL == error_code, false,	"The error_code is NULL");

	if(NULL == sch_record) {
		ERR("The sch_record is NULL");
		*error_code = CAL_ERR_ARG_INVALID;
		return false;
	}

	memset(sch_record,0,sizeof(cal_sch_t));

	sch_record->index = CAL_INVALID_INDEX;
	sch_record->cal_type = CAL_EVENT_SCHEDULE_TYPE;
	sch_record->sch_category = CAL_SCH_APPOINTMENT;
	memset(&temp_date_time, 0 ,sizeof(struct tm));
	memset(&sch_record->start_date_time, 0 ,sizeof(struct tm));
	memset(&sch_record->end_date_time, 0 ,sizeof(struct tm));
	memset(&sch_record->repeat_end_date, 0 ,sizeof(struct tm));
	memset(&sch_record->alarm_time, 0 ,sizeof(struct tm));
	memcpy(&current_time,tm,sizeof(struct tm));
	//CALS_DBG("!!!!!!!!!!!----------current hour is %d, minutes %d----------------",current_time.tm_hour,current_time.tm_min);

	memcpy(&sch_record->start_date_time,&current_time,sizeof(struct tm));
	memcpy(&sch_record->alarm_time,&current_time,sizeof(struct tm));
	memcpy(&sch_record->end_date_time,&current_time,sizeof(struct tm));
	sch_record->start_date_time.tm_sec = 0;
	sch_record->alarm_time.tm_sec = 0;
	sch_record->end_date_time.tm_sec = 0;
	sch_record->end_date_time.tm_hour ++;
	if (sch_record->end_date_time.tm_hour > 23)
	{
		sch_record->end_date_time.tm_hour = 0;
		cal_db_service_get_tomorrow(&sch_record->end_date_time);
	}

	temp_date_time.tm_year = 137;
	temp_date_time.tm_mon = 11;
	temp_date_time.tm_mday = 31;
	memcpy(&sch_record->repeat_end_date,&temp_date_time,sizeof(struct tm));

	sch_record->remind_tick = CAL_INVALID_INDEX;    //inintial
	sch_record->repeat_term = CAL_REPEAT_NONE;
	sch_record->day_date = 1;
	sch_record->calendar_type = CAL_PHONE_CALENDAR;
	return true;
}

bool cal_db_service_free_participant(cal_participant_info_t* paritcipant_info, int *error_code)
{
	if(NULL == paritcipant_info)
	{
		return true;
	}

	CAL_FREE(paritcipant_info->attendee_email);
	CAL_FREE(paritcipant_info->attendee_number);
	CAL_FREE(paritcipant_info->attendee_name);

	return true;
}

bool cal_db_service_free_category(cal_category_info_t* category_info, int *error_code)
{
	if(NULL == category_info)
	{
		return true;
	}
	CAL_FREE(category_info->category_name);
	return true;
}


bool cal_db_service_free_full_record(cal_sch_full_t *sch_full_record, int *error_code)
{
	retex_if(error_code == NULL, ,"error_code is NULL.");
	retex_if(sch_full_record == NULL, *error_code = CAL_ERR_ARG_INVALID,"sch_full_record is NULL.");
	cal_value *value = NULL;
	GList *head;

	CAL_FREE(sch_full_record->summary);
	CAL_FREE(sch_full_record->description);
	CAL_FREE(sch_full_record->location);
	CAL_FREE(sch_full_record->week_flag);
	CAL_FREE(sch_full_record->uid);
	CAL_FREE(sch_full_record->organizer_name);
	CAL_FREE(sch_full_record->organizer_email);
	CAL_FREE(sch_full_record->gcal_id);
	CAL_FREE(sch_full_record->updated);
	CAL_FREE(sch_full_record->location_summary);
	CAL_FREE(sch_full_record->etag);
	CAL_FREE(sch_full_record->edit_uri);
	CAL_FREE(sch_full_record->gevent_id);
	CAL_FREE(sch_full_record->tz_name);
	CAL_FREE(sch_full_record->tz_city_name);

	if(sch_full_record->exception_date_list)
	{
		head = sch_full_record->exception_date_list;
		while (sch_full_record->exception_date_list)
		{
			value = sch_full_record->exception_date_list->data;
			if(value)
			{
				free(value->user_data);
				free(value);
			}
			sch_full_record->exception_date_list = sch_full_record->exception_date_list->next;
		}
		g_list_free(head);
		sch_full_record->exception_date_list = NULL;
	}

	if (sch_full_record->attendee_list)
	{
		head = sch_full_record->attendee_list;
		while (sch_full_record->attendee_list)
		{
			value = sch_full_record->attendee_list->data;
			if(NULL != value)
			{
				if(NULL != value->user_data)
				{
					cal_db_service_free_participant((cal_participant_info_t*)value->user_data,error_code);
					CAL_FREE(value->user_data);

				}
				CAL_FREE(value);
			}
			sch_full_record->attendee_list = sch_full_record->attendee_list->next;
		}
		g_list_free(head);
		sch_full_record->attendee_list = NULL;
	}

	if (sch_full_record->meeting_category)
	{
		head = sch_full_record->meeting_category;
		while (sch_full_record->meeting_category)
		{
			value = sch_full_record->meeting_category->data;
			if(NULL != value)
			{
				if(NULL != value->user_data)
				{
					cal_db_service_free_category((cal_category_info_t*)value->user_data,error_code);
					CAL_FREE(value->user_data);
				}
				CAL_FREE(value);
			}
			sch_full_record->meeting_category = sch_full_record->meeting_category->next;
		}
		g_list_free(head);
		sch_full_record->meeting_category = NULL;
	}

	return true;

CATCH:

	return false;
}


bool cal_db_service_free_sch_record(cal_sch_t *sch_record, int *error_code)
{
	retex_if(error_code == NULL, ,"error_code is NULL.");
	retex_if(sch_record == NULL, *error_code = CAL_ERR_ARG_INVALID,"sch_record is NULL.");

	CAL_FREE(sch_record->summary);
	CAL_FREE(sch_record->description);
	CAL_FREE(sch_record->location);
	CAL_FREE(sch_record->week_flag);

	return true;

CATCH:

	return false;
}

/************************************************************************************************
 *                                                                                               *
 *                           calendar event table add/edit APIs                                  *
 *                                                                                               *
 ************************************************************************************************/
bool cal_db_service_get_meeting_category_info_by_index(const int event_id, GList** record_list, int *error_code)
{
	int 	rc = -1;
	char	sql_value[CALS_SQL_MAX_LEN] = {0};
	cal_category_info_t* category_info = NULL;
	sqlite3_stmt *stmt = NULL;
	cal_value *cvalue = NULL;

	retex_if(error_code == NULL, ,"cal_db_service_get_record_by_index: The error_code is NULL.");

	//check if db opened
	retex_if(NULL == calendar_db_handle, *error_code = CAL_ERR_DB_NOT_OPENED, "The calendar database hasn't been opened.");

	sprintf(sql_value, "select * from cal_meeting_category_table where event_id = %d;", event_id);

	rc = sqlite3_prepare_v2(calendar_db_handle, sql_value, strlen(sql_value), &stmt, NULL);
	retex_if(rc != SQLITE_OK, *error_code = CAL_ERR_DB_FAILED, "EDBStmtCreate error!!");

	rc = sqlite3_step(stmt);
	while(rc == SQLITE_ROW)
	{
		cvalue = (cal_value*)malloc(sizeof(cal_value));
		retex_if(NULL == cvalue,,"[ERROR]cal_db_service_get_participant_info_by_index:Failed to malloc!");

		cvalue->v_type = CAL_EVENT_CATEGORY;
		cvalue->user_data = category_info = (cal_category_info_t*)malloc(sizeof(cal_category_info_t));
		retex_if(category_info == NULL, *error_code = CAL_ERR_DB_FAILED, "cal_category_info_t malloc error!!");

		memset(category_info,0x00,sizeof(cal_category_info_t));

		category_info->event_id = sqlite3_column_int(stmt, 0);

		cal_db_get_text_from_stmt(stmt,&(category_info->category_name),1);

		*record_list = g_list_append(*record_list, (gpointer)cvalue);

		rc = sqlite3_step(stmt);
		retex_if(SQLITE_ROW != rc && SQLITE_OK != rc && SQLITE_DONE != rc,*error_code = CAL_ERR_DB_FAILED,"__cal_db_service_get_meeting_category_info_by_index:Failed to query.");
	}

	if(NULL != stmt)
	{
		sqlite3_finalize(stmt);
		stmt = NULL;
	}

	return true;

CATCH:

	if(cvalue){
		CAL_FREE(cvalue->user_data);
		CAL_FREE(cvalue);
	}

	if (stmt)
	{
		sqlite3_finalize(stmt);
		stmt = NULL;

	}

	return false;
}


int cal_service_add_participant_info(const int event_id, const cal_participant_info_t* current_record)
{
	int ret = -1;
	sqlite3_stmt *stmt = NULL;
	char sql_value[CALS_SQL_MAX_LEN];

	//check if db opened

	sprintf(sql_value,"INSERT INTO %s(event_id,attendee_name,attendee_email,attendee_number,attendee_status,attendee_type,attendee_ct_index,"
			"attendee_role,attendee_rsvp,attendee_group,attendee_delegator_uri,attendee_delegate_uri,attendee_uid) "
			"VALUES(%d, ?, ?, ?, %d, %d, %d,%d,%d,?,?,?,?)", CALS_TABLE_PARTICIPANT,
			event_id,
			current_record->attendee_status,
			current_record->attendee_type,
			current_record->attendee_ct_index,
			current_record->attendee_role,
			current_record->attendee_rsvp);

	stmt = cals_query_prepare(sql_value);
	retvm_if(NULL == stmt, CAL_ERR_DB_FAILED, "cals_query_prepare() Failed");

	if (current_record->attendee_name)
		cals_stmt_bind_text(stmt, 1, current_record->attendee_name);

	if (current_record->attendee_email)
		cals_stmt_bind_text(stmt, 2, current_record->attendee_email);

	if (current_record->attendee_number)
		cals_stmt_bind_text(stmt, 3, current_record->attendee_number);

	if (current_record->attendee_group)
		cals_stmt_bind_text(stmt, 4, current_record->attendee_group);

	if (current_record->attendee_delegator_uri)
		cals_stmt_bind_text(stmt, 5, current_record->attendee_delegator_uri);

	if (current_record->attendee_delegate_uri)
		cals_stmt_bind_text(stmt, 6, current_record->attendee_delegate_uri);

	if (current_record->attendee_uid)
		cals_stmt_bind_text(stmt, 7, current_record->attendee_uid);


	ret = cals_stmt_step(stmt);
	if (CAL_SUCCESS != ret) {
		sqlite3_finalize(stmt);
		ERR("cals_stmt_step() Failed(%d)", ret);
		return ret;
	}
	sqlite3_finalize(stmt);

	return CAL_SUCCESS;
}

/************************************************************************************************
 *                                                                                               *
 *                               calendar event table get APIs                                   *
 *                                                                                               *
 ************************************************************************************************/

bool cal_db_service_get_participant_info_by_index(const int panticipant_index, GList** record_list, int *error_code)
{
	int 	rc = -1;
	char	sql_value[CALS_SQL_MAX_LEN] = {0};
	cal_participant_info_t* participant_info = NULL;
	sqlite3_stmt *stmt = NULL;
	cal_value *cvalue = NULL;

	retex_if(error_code == NULL, ,"cal_db_service_get_record_by_index: The error_code is NULL.\n");

	//check input parameter
	retex_if (!__check_record_index_valid(panticipant_index),*error_code = CAL_ERR_ARG_INVALID, "The index is invalid." );

	//check if db opened
	retex_if(NULL == calendar_db_handle, *error_code = CAL_ERR_DB_NOT_OPENED, "The calendar database hasn't been opened.");

	sprintf(sql_value, "select * from cal_participant_table where event_id = %d;", panticipant_index);

	rc = sqlite3_prepare_v2(calendar_db_handle, sql_value, strlen(sql_value), &stmt, NULL);
	retex_if(rc != SQLITE_OK, *error_code = CAL_ERR_DB_FAILED, "Failed to get stmt!!");

	rc = sqlite3_step(stmt);
	retex_if(rc!= SQLITE_ROW && rc!= SQLITE_OK && rc!= SQLITE_DONE, *error_code = CAL_ERR_DB_FAILED, "[ERROR]cal_db_service_get_participant_info_by_index:Query error !!");
	while(rc == SQLITE_ROW)
	{
		cvalue = (cal_value*)malloc(sizeof(cal_value));
		retex_if(NULL == cvalue,,"[ERROR]cal_db_service_get_participant_info_by_index:Failed to malloc!\n");

		cvalue->v_type = CAL_EVENT_PATICIPANT;
		cvalue->user_data = (cal_participant_info_t*)malloc(sizeof(cal_participant_info_t));
		retex_if(NULL == cvalue->user_data,,"[ERROR]cal_db_service_get_participant_info_by_index:Failed to malloc!\n");

		participant_info = cvalue->user_data;
		memset(participant_info, 0x00, sizeof(cal_participant_info_t));

		participant_info->event_id = sqlite3_column_int(stmt, 0);

		cal_db_get_text_from_stmt(stmt,&(participant_info->attendee_name),1);
		cal_db_get_text_from_stmt(stmt,&(participant_info->attendee_email),2);
		cal_db_get_text_from_stmt(stmt,&(participant_info->attendee_number),3);
		participant_info->attendee_status = sqlite3_column_int(stmt, 4);
		participant_info->attendee_type = sqlite3_column_int(stmt, 5);
		participant_info->attendee_ct_index = sqlite3_column_int(stmt, 6);
		participant_info->attendee_role = sqlite3_column_int(stmt, 7);
		participant_info->attendee_rsvp = sqlite3_column_int(stmt, 8);
		cal_db_get_text_from_stmt(stmt,&(participant_info->attendee_group),9);
		cal_db_get_text_from_stmt(stmt,&(participant_info->attendee_delegator_uri),10);
		cal_db_get_text_from_stmt(stmt,&(participant_info->attendee_delegate_uri),11);
		cal_db_get_text_from_stmt(stmt,&(participant_info->attendee_uid),12);

		*record_list = g_list_append(*record_list, (gpointer)cvalue);

		cvalue = NULL;

		rc = sqlite3_step(stmt);

		if(rc == SQLITE_DONE)
		{
			break;
		}

		retex_if(rc != SQLITE_ROW && rc != SQLITE_OK && rc != SQLITE_DONE, *error_code = CAL_ERR_DB_FAILED, "Query error!!");

	}
	//DBG("Get that\n");
	if (stmt)
	{
		sqlite3_finalize(stmt);
		stmt = NULL;
	}

	return true;

CATCH:
	if (cvalue)
	{
		if (cvalue->user_data)
		{
			CAL_FREE(participant_info->attendee_name);
			CAL_FREE(participant_info->attendee_email);
			CAL_FREE(participant_info->attendee_number);
			CAL_FREE(cvalue->user_data);
		}
		CAL_FREE(cvalue);
	}

	if (stmt)
	{
		sqlite3_finalize(stmt);
		stmt = NULL;
	}

	return false;
}

bool cal_db_service_get_record_full_field_by_index(const int index, cal_sch_full_t *returned_record, int *error_code)
{
	int rc = -1;
	char sql_value[CALS_SQL_MAX_LEN] = {0};
	sqlite3_stmt *stmt = NULL;

	retex_if(error_code == NULL, ,"The error_code is NULL.\n");
	retex_if(returned_record == NULL, *error_code = CAL_ERR_ARG_INVALID,"The returned_record is NULL.\n");

	//check input parameter
	retex_if (!__check_record_index_valid(index),*error_code = CAL_ERR_ARG_INVALID,"The index is invalid." );

	//check if db opened
	retex_if(NULL == calendar_db_handle, *error_code = CAL_ERR_DB_NOT_OPENED, "The calendar database hasn't been opened.");

	sprintf(sql_value, "SELECT * FROM %s WHERE id = %d and is_deleted = 0;", CALS_TABLE_SCHEDULE, index);

	rc = sqlite3_prepare_v2(calendar_db_handle, sql_value, strlen(sql_value), &stmt, NULL);
	retex_if(rc != SQLITE_OK, *error_code = CAL_ERR_DB_FAILED, "Failed to get stmt!!");

	rc = sqlite3_step(stmt);
	retex_if(rc != SQLITE_ROW && rc != SQLITE_OK && rc != SQLITE_DONE, *error_code = CAL_ERR_DB_FAILED, "Failed to query!!");

	cals_stmt_get_full_schedule(stmt,returned_record,false);

	cal_db_service_get_participant_info_by_index(index, &(returned_record->attendee_list), error_code);
	cal_db_service_get_meeting_category_info_by_index(index, &(returned_record->meeting_category), error_code);

	if(NULL!= stmt)
	{
		sqlite3_finalize(stmt);
		stmt=NULL;
	}

	//calendar_svc_end_trans();

	return true;

CATCH:

	if(NULL!= stmt)
	{
		sqlite3_finalize(stmt);
		stmt=NULL;
	}

	//calendar_svc_end_trans();

	return false;
}

int __cal_service_delete_all_records(const int account_id, const cal_event_type_t record_type)
{
	int ret = 0;
	char query[CALS_SQL_MAX_LEN] = {0};

	time_t current_time = time(NULL);
	if(account_id)
	{
		switch (record_type)
		{
		case CAL_EVENT_NONE:
			sprintf(query, "UPDATE %s SET is_deleted = 1, sync_status = %d, last_modified_time = %ld WHERE account_id = %d",
				CALS_TABLE_SCHEDULE, CAL_SYNC_STATUS_DELETED,(long int)current_time,account_id);
			break;
		case CAL_EVENT_SCHEDULE_TYPE:
			sprintf(query, "UPDATE %s SET is_deleted = 1, sync_status = %d, last_modified_time = %ld WHERE (type = 1 and account_id = %d)",
				CALS_TABLE_SCHEDULE, CAL_SYNC_STATUS_DELETED,(long int)current_time,account_id);
			break;
		case CAL_EVENT_TODO_TYPE:
			sprintf(query, "UPDATE %s SET is_deleted = 1, sync_status = %d, last_modified_time = %ld WHERE (type = 2 and account_id = %d)",
				CALS_TABLE_SCHEDULE, CAL_SYNC_STATUS_DELETED,(long int)current_time,account_id);
			break;
		case CAL_EVENT_MEMO_TYPE:
			sprintf(query, "UPDATE %s SET is_deleted = 1, sync_status = %d, last_modified_time = %ld WHERE (type = 3 and locked_flag = 0 and account_id = %d)",
				CALS_TABLE_SCHEDULE, CAL_SYNC_STATUS_DELETED,(long int)current_time,account_id);
			break;
		default:
			ERR("Unknown type(%d)", record_type);
			return CAL_ERR_ARG_INVALID;
		}
	}
	else
	{
		switch (record_type)
		{
		case CAL_EVENT_NONE:
			sprintf(query, "UPDATE %s SET is_deleted = 1, sync_status = %d, last_modified_time = %ld",
				CALS_TABLE_SCHEDULE, CAL_SYNC_STATUS_DELETED,(long int)current_time);
			break;
		case CAL_EVENT_SCHEDULE_TYPE:
			sprintf(query, "UPDATE %s SET is_deleted = 1, sync_status = %d, last_modified_time = %ld WHERE type = 1",
				CALS_TABLE_SCHEDULE, CAL_SYNC_STATUS_DELETED,(long int)current_time);
			break;
		case CAL_EVENT_TODO_TYPE:
			sprintf(query, "UPDATE %s SET is_deleted = 1, sync_status = %d, last_modified_time = %ld WHERE type = 2",
				CALS_TABLE_SCHEDULE, CAL_SYNC_STATUS_DELETED,(long int)current_time);
			break;
		case CAL_EVENT_MEMO_TYPE:
			sprintf(query, "UPDATE %s SET is_deleted = 1, sync_status = %d, last_modified_time = %ld WHERE (type = 3 and locked_flag = 0)",
				CALS_TABLE_SCHEDULE, CAL_SYNC_STATUS_DELETED,(long int)current_time);
			break;
		default:
			ERR("Unknown type(%d)", record_type);
			return CAL_ERR_ARG_INVALID;
		}
	}

	ret = cals_begin_trans();
	retvm_if(CAL_SUCCESS != ret, ret, "cals_begin_trans() Failed(%d)", ret);

	if (CAL_EVENT_NONE == record_type || CAL_EVENT_SCHEDULE_TYPE == record_type) {
		if (account_id)
			cals_alarm_remove(CALS_ALARM_REMOVE_BY_ACC_ID, account_id);
		else
			cals_alarm_remove(CALS_ALARM_REMOVE_ALL, account_id);
	}

	ret = cals_query_exec(query);
	if (CAL_SUCCESS != ret) {
		cals_end_trans(false);
		ERR("cals_query_exec() Failed(%d)", ret);
		return ret;
	}
	cals_end_trans(true);

	if(CAL_EVENT_SCHEDULE_TYPE == record_type)
		cals_notify(CALS_NOTI_TYPE_EVENT);
	else
		cals_notify(CALS_NOTI_TYPE_TODO);

	return CAL_SUCCESS;
}


/************************************************************************************************
 *                                                                                               *
 *                               event exception table APIs                                      *
 *                                                                                               *
 ************************************************************************************************/

int cal_service_add_exception_info(int event_id, cal_exception_info_t *exception_info,
	const cal_sch_full_t *sch_record)
{
	int ret;
	sqlite3_stmt *stmt = NULL;
	char sql_value[CALS_SQL_MIN_LEN];
	struct tm* recurrency_tm = &exception_info->exception_start_time;
	time_t start_time = cals_mktime(recurrency_tm);

	DBG("next_start_tm [%s\n", ctime(&start_time));

	sprintf(sql_value,"INSERT INTO %s(start_date_time,end_date_time,event_id,exception_event_id,updated) "
			"VALUES(%ld, %ld, %d, %d, ?)", CALS_TABLE_RECURRENCY_LOG,
			cals_mktime(recurrency_tm),
			cals_mktime(recurrency_tm),
			event_id,
			exception_info->event_id);

	stmt = cals_query_prepare(sql_value);
	retvm_if(NULL == stmt, CAL_ERR_DB_FAILED, "cals_query_prepare() Failed");

	ret = cals_stmt_step(stmt);
	if (CAL_SUCCESS != ret) {
		sqlite3_finalize(stmt);
		ERR("cals_stmt_step() Failed(%d)", ret);
		return ret;
	}
	sqlite3_finalize(stmt);

	return CAL_SUCCESS;
}


int cals_insert_timezone(cal_timezone_t *timezone_info)
{
	int ret;
	sqlite3_stmt *stmt;
	char query[CALS_SQL_MAX_LEN];

	retv_if(NULL == timezone_info, CAL_ERR_ARG_NULL);

	sprintf(query,"INSERT INTO %s(tz_offset_from_gmt ,standard_name, "
			"std_start_month ,std_start_position_of_week ,std_start_day, "
			"std_start_hour ,standard_bias ,day_light_name ,day_light_start_month, "
			"day_light_start_position_of_week ,day_light_start_day, "
			"day_light_start_hour ,day_light_bias) "
			"VALUES(%d,?,%d,%d,%d,%d,%d,?,%d,%d,%d,%d,%d)", CALS_TABLE_TIMEZONE,
			timezone_info->tz_offset_from_gmt,
			timezone_info->std_start_month,
			timezone_info->std_start_position_of_week,
			timezone_info->std_start_day,
			timezone_info->std_start_hour,
			timezone_info->standard_bias,
			timezone_info->day_light_start_month,
			timezone_info->day_light_start_position_of_week,
			timezone_info->day_light_start_day,
			timezone_info->day_light_start_hour,
			timezone_info->day_light_bias);

	stmt = cals_query_prepare(query);
	retvm_if(NULL == stmt, CAL_ERR_DB_FAILED, "cals_query_prepare() Failed");

	if (timezone_info->standard_name)
		cals_stmt_bind_text(stmt, 1, timezone_info->standard_name);

	if (timezone_info->day_light_name)
		cals_stmt_bind_text(stmt, 2, timezone_info->day_light_name);

	ret = cals_stmt_step(stmt);
	if (CAL_SUCCESS != ret) {
		sqlite3_finalize(stmt);
		ERR("cals_stmt_step() Failed(%d)", ret);
		return ret;
	}
	ret = cals_last_insert_id();
	sqlite3_finalize(stmt);

	return ret;
}


int cals_update_timezone(cal_timezone_t *timezone_info)
{
	int ret;
	sqlite3_stmt *stmt;
	char query[CALS_SQL_MAX_LEN];

	retv_if(NULL == timezone_info, CAL_ERR_ARG_NULL);

	sprintf(query, "UPDATE %s SET "
			"tz_offset_from_gmt=%d,"
			"standard_name=?,"
			"std_start_month=%d,"
			"std_start_position_of_week=%d,"
			"std_start_day=%d,"
			"std_start_hour=%d,"
			"standard_bias=%d,"
			"day_light_name=?,"
			"day_light_start_month=%d,"
			"day_light_start_position_of_week=%d,"
			"day_light_start_day=%d,"
			"day_light_start_hour=%d,"
			"day_light_bias=%d "
			"WHERE rowid = %d",
			CALS_TABLE_TIMEZONE,
			timezone_info->tz_offset_from_gmt,
			timezone_info->std_start_month,
			timezone_info->std_start_position_of_week,
			timezone_info->std_start_day,
			timezone_info->std_start_hour,
			timezone_info->standard_bias,
			timezone_info->day_light_start_month,
			timezone_info->day_light_start_position_of_week,
			timezone_info->day_light_start_day,
			timezone_info->day_light_start_hour,
			timezone_info->day_light_bias,
			timezone_info->index);

	stmt = cals_query_prepare(query);
	retvm_if(NULL == stmt, CAL_ERR_DB_FAILED, "cals_query_prepare() Failed");

	if (timezone_info->standard_name)
		cals_stmt_bind_text(stmt, 1, timezone_info->standard_name);

	if (timezone_info->day_light_name)
		cals_stmt_bind_text(stmt, 2, timezone_info->day_light_name);

	ret = cals_stmt_step(stmt);
	if (CAL_SUCCESS != ret) {
		sqlite3_finalize(stmt);
		ERR("cals_stmt_step() Failed(%d)", ret);
		return ret;
	}
	sqlite3_finalize(stmt);

	return CAL_SUCCESS;
}


/************************************************************************************************
 *                                                                                               *
 *                            Recurrence exception API for activesync.                           *
 *                                                                                               *
 ************************************************************************************************/
	bool
cal_db_service_get_recurrency_exception(const int event_id, GList **exception_list, int *error_code)
{
	int		rc = -1;
	char	sql_value[CALS_SQL_MAX_LEN] = {0};
	sqlite3_stmt *stmt = NULL;
	cal_exception_info_t* exception_info = NULL;
	cal_value * cvalue = NULL;
	retex_if(error_code == NULL, ,"cal_db_service_get_recurrency_exception_for_acs: The error_code is NULL.\n");
	retex_if (!__check_record_index_valid(event_id),*error_code = CAL_ERR_ARG_INVALID, "The index is invalid." );

	sprintf(sql_value, "select start_date_time, exception_event_id from recurrency_log_table where event_id=%d;", event_id);

	rc = sqlite3_prepare_v2(calendar_db_handle, sql_value, strlen(sql_value), &stmt, NULL);
	retex_if(rc != SQLITE_OK, *error_code = CAL_ERR_DB_FAILED, "Failed to get stmt!!");

	rc = sqlite3_step(stmt);
	retex_if(rc == SQLITE_DONE, *error_code = CAL_ERR_DB_FAILED, "cal_db_service_get_recurrency_exception no result!!");
	retex_if(rc != SQLITE_ROW, *error_code = CAL_ERR_DB_FAILED, "Query error!!");

	while (rc == SQLITE_ROW)
	{
		cvalue = (cal_value*)malloc(sizeof(cal_value));
		retex_if(NULL == cvalue,,"Failed to malloc!\n");

		cvalue->v_type = CAL_EVENT_RECURRENCY;

		cvalue->user_data = exception_info = (cal_exception_info_t*)malloc(sizeof(cal_exception_info_t));
		retex_if(NULL == exception_info,,"Failed to malloc!\n");

		memset(exception_info, 0x00, sizeof(cal_exception_info_t));

		time_t temp_time = sqlite3_column_int(stmt, 0);
		//cal_db_service_copy_struct_tm((struct tm*)gmtime(&temp_time),&(exception_info->exception_start_time));
		gmtime_r(&temp_time,&(exception_info->exception_start_time));

		exception_info->event_id = sqlite3_column_int(stmt, 1);
		//		exception_info->index = exception_event_id;
		/*
			if(exception_event_id != CAL_INVALID_INDEX)
			{
			sch_record = (cal_sch_full_t*)malloc(sizeof(cal_sch_full_t));
			cals_init_full_record(sch_record, error_code);
			cal_db_service_get_record_full_field_by_index(exception_event_id,"exception",sch_record, error_code);
			exception_info->exception_record = sch_record;
			INFO( "Exception record is here : %d !!\n", exception_event_id);
			}
			else*/
		{
			exception_info->exception_record = NULL;
		}

		*exception_list = g_list_append(*exception_list, (gpointer)cvalue);

		rc = sqlite3_step(stmt);
		if(rc == SQLITE_DONE)
		{
			break;
		}
		retex_if(rc != SQLITE_ROW, *error_code = CAL_ERR_DB_FAILED, "Query error!!");


	}

	CALS_DBG( "exception_info_length = %d\n", g_list_length(*exception_list));

	/*if(g_list_length(*exception_list)>0)
	  {
	  time_t temp_time = mktime(&(exception_info->exception_start_time));
	//DBG("exception_start_time [%s\n", ctime(&temp_time));
	}*/


	if(NULL!= stmt)
	{
		sqlite3_finalize(stmt);
		stmt = NULL;
	}
	return true;

CATCH:
	if (cvalue)
	{
		CAL_FREE(cvalue->user_data);
		CAL_FREE(cvalue);
	}
	if(NULL!= stmt)
	{
		sqlite3_finalize(stmt);
		stmt = NULL;
	}

	return false;
}


int cal_db_service_get_month_event(int account_id, time_t startdate, time_t enddate,int is_repeat, sqlite3_stmt** stmt)
{

	char condition[1024] = {0};
	char select_value[1024] = {0};
	char from_vlaue[1024] = {0};
	char sql_value[1024] = {0};
	time_t gm_startdate = 0;
	time_t gm_enddate =0;

	calendar_svc_util_local_to_gmt(startdate,&gm_startdate);
	calendar_svc_util_local_to_gmt(enddate,&gm_enddate);

	gm_startdate = gm_startdate - SECSPERDAY;
	gm_enddate = gm_enddate + SECSPERDAY;

	if(account_id == ALL_VISIBILITY_ACCOUNT)
	{
		sprintf(from_vlaue,"from schedule_table as st, calendar_table as ct "\
				"where st.calendar_id = ct.rowid and ct.visibility = 1 "\
				"and ");
	}
	else
	{
		sprintf(from_vlaue,"from schedule_table as st where st.account_id = %d and ",account_id);
	}

	if(is_repeat == FALSE)
	{
		sprintf(select_value,"select st.id, "\
				"st.all_day_event, "\
				"st.start_date_time, "\
				"st.end_date_time ");

		sprintf(condition, "st.type=%d and st.is_deleted = 0 "\
				"and (st.repeat_item = 0 and st.start_date_time <= %d and st.end_date_time >=%d) ",
				CAL_EVENT_SCHEDULE_TYPE,(int)gm_enddate,(int)gm_startdate);
	}
	else
	{
		sprintf(select_value,"select st.id, "\
				"st.all_day_event, "\
				"st.start_date_time, "\
				"st.end_date_time, "\
				"st.repeat_item, "\
				"st.repeat_interval, "\
				"st.repeat_until_type, "\
				"st.repeat_occurrences, "\
				"st.repeat_end_date, "\
				"st.week_start, "\
				"st.week_flag, "\
				"st.day_date, "\
				"st.timezone, "\
				"st.tz_name ");

		sprintf(condition, "st.type=%d and st.is_deleted = 0 "\
				"and (st.repeat_item>0 and st.repeat_end_date>=%d) ",
				CAL_EVENT_SCHEDULE_TYPE,(int)gm_startdate );
	}

	sprintf(sql_value,"%s %s %s",select_value,from_vlaue,condition);

	*stmt = cals_query_prepare(sql_value);
	retvm_if(NULL == *stmt, CAL_ERR_DB_FAILED, "cals_query_prepare() Failed");

	return CAL_SUCCESS;
}
