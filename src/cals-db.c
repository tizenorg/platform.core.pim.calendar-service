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
#include "cals-ical.h"
#include "cals-alarm.h"
#include "cals-sqlite.h"
#include "cals-schedule.h"
#include "cals-db.h"

#ifdef CALS_IPC_SERVER
extern __thread sqlite3 *calendar_db_handle;
#else
extern sqlite3 *calendar_db_handle;
#endif

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

int cals_db_free_alarm(cal_sch_full_t *record)
{
	CALS_FN_CALL;
	GList *l;
	cal_value *cv;
	cal_alarm_info_t *ai;

	if (record == NULL) {
		ERR("Invalid argument: record is NULL");
		return -1;
	}

	if (record->alarm_list == NULL) {
		DBG("No alarm list to free");
		return 0;
	}

	l = record->alarm_list;
	while (l) {
		cv = (cal_value *)l->data;
		if (cv == NULL) {
			l = g_list_next(l);
			continue;
		}

		ai = (cal_alarm_info_t *)cv->user_data;
		if (ai == NULL) {
			l = g_list_next(l);
			continue;
		}

		CAL_FREE(ai->alarm_tone);
		CAL_FREE(ai->alarm_description);
		CAL_FREE(ai);

		CAL_FREE(cv);

		l = g_list_next(l);
	}
	g_list_free(record->alarm_list);
	record->alarm_list = NULL;

	return 0;
}

int cals_db_free_attendee(cal_sch_full_t *record)
{
	CALS_FN_CALL;
	GList *l;
	cal_value *cv;
	cal_participant_info_t *pi;

	if (record == NULL) {
		ERR("Invalid argument: record is NULL");
		return -1;
	}

	if (record->attendee_list == NULL) {
		DBG("No attendee list to free");
		return 0;
	}

	l = record->attendee_list;
	while (l) {
		cv = (cal_value *)l->data;
		if (cv == NULL) {
			l = g_list_next(l);
			continue;
		}

		pi = (cal_participant_info_t *)cv->user_data;
		if (pi == NULL) {
			l = g_list_next(l);
			continue;
		}

		CAL_FREE(pi->attendee_email);
		CAL_FREE(pi->attendee_number);
		CAL_FREE(pi->attendee_name);
		CAL_FREE(pi);

		CAL_FREE(cv);

		l = g_list_next(l);
	}
	g_list_free(record->attendee_list);
	record->attendee_list = NULL;

	return 0;
}

int cal_db_service_free_full_record(cal_sch_full_t *record)
{
	if (record == NULL) {
		ERR("Invalid argument: record is NULL");
		return -1;
	}

	CAL_FREE(record->dtstart_tzid);
	CAL_FREE(record->dtend_tzid);
	CAL_FREE(record->summary);
	CAL_FREE(record->description);
	CAL_FREE(record->location);
	CAL_FREE(record->categories);
	CAL_FREE(record->uid);
	CAL_FREE(record->organizer_name);
	CAL_FREE(record->organizer_email);
	CAL_FREE(record->gcal_id);
	CAL_FREE(record->updated);
	CAL_FREE(record->location_summary);
	CAL_FREE(record->etag);
	CAL_FREE(record->edit_uri);
	CAL_FREE(record->gevent_id);

	cals_db_free_alarm(record);
	cals_db_free_attendee(record);

	return 0;
}

/************************************************************************************************
 *                                                                                               *
 *                           calendar event table add/edit APIs                                  *
 *                                                                                               *
 ************************************************************************************************/
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
	int	rc = -1;
	char sql_value[CALS_SQL_MAX_LEN] = {0};
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

int __cal_service_delete_all_records(const int account_id, const cals_sch_type record_type)
{
	int ret = 0;
	char query_logging[CALS_SQL_MIN_LEN] = {0};
	char query_delete[CALS_SQL_MIN_LEN] = {0};

	if (account_id == LOCAL_ACCOUNT_ID) {
		snprintf(query_logging, sizeof(query_logging), "INSERT INTO %s "
				"SELECT id, type, calendar_id, %d FROM %s "
				"WHERE type = %d AND account_id = %d",
				CALS_TABLE_DELETED,
				cals_get_next_ver(), CALS_TABLE_SCHEDULE,
				record_type, account_id);

		snprintf(query_delete, sizeof(query_delete), "DELETE FROM %s "
				"WHERE type = %d AND account_id = %d",
				CALS_TABLE_SCHEDULE,
				record_type, account_id);

	} else {
		snprintf(query_logging, sizeof(query_logging), "UPDATE %s "
				"SET is_deleted = 1, last_modified_time = %ld, changed_ver = %d "
				"WHERE type = %d AND accoutn_id = %d ",
				CALS_TABLE_SCHEDULE,
				time(NULL), cals_get_next_ver(),
				record_type, account_id);

	}

	ret = cals_begin_trans();
	retvm_if(CAL_SUCCESS != ret, ret, "cals_begin_trans() Failed(%d)", ret);

	if (CALS_SCH_TYPE_NONE == record_type || CALS_SCH_TYPE_EVENT == record_type) {
		if (account_id)
			cals_alarm_remove(CALS_ALARM_REMOVE_BY_ACC_ID, account_id);
		else
			cals_alarm_remove(CALS_ALARM_REMOVE_ALL, account_id);
	}

	ret = cals_query_exec(query_logging);
	if (CAL_SUCCESS != ret) {
		cals_end_trans(false);
		ERR("cals_query_exec() Failed(%d)", ret);
		return ret;
	}

	ret = cals_query_exec(query_delete);
	if (CAL_SUCCESS != ret) {
		cals_end_trans(false);
		ERR("cals_query_exec() Failed(%d)", ret);
		return ret;
	}

	cals_end_trans(true);

	if(CALS_SCH_TYPE_EVENT == record_type)
		cals_notify(CALS_NOTI_TYPE_EVENT);
	else
		cals_notify(CALS_NOTI_TYPE_TODO);

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

