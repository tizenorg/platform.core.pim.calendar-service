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
#include <errno.h>
#include <alarm.h>

#include "cals-internal.h"
#include "cals-typedef.h"
#include "cals-db-info.h"
#include "cals-sqlite.h"
#include "cals-tz-utils.h"
#include "cals-utils.h"
#include "cals-alarm.h"

#define PKG_CALENDAR_APP "org.tizen.calendar"

int cals_alarm_remove(int type, int related_id)
{
	int ret;
	sqlite3_stmt *stmt = NULL;
	char query[CALS_SQL_MAX_LEN] = {0};

	switch (type) {
	case CALS_ALARM_REMOVE_BY_EVENT_ID:
		sprintf(query, "SELECT alarm_id FROM %s "
			"WHERE event_id = %d AND alarm_id <> 0", CALS_TABLE_ALARM, related_id);
		break;
	case CALS_ALARM_REMOVE_BY_CALENDAR_ID:
		sprintf(query, "SELECT alarm_id FROM %s A, %s B ON A.id = B.event_id "
			"WHERE A.calendar_id = %d AND B.alarm_id <> 0",
			CALS_TABLE_SCHEDULE, CALS_TABLE_ALARM, related_id);
		break;
	case CALS_ALARM_REMOVE_BY_ACC_ID:
		sprintf(query, "SELECT alarm_id FROM %s A, %s B ON A.id = B.event_id "
			"WHERE A.account_id = %d AND B.alarm_id <> 0",
			CALS_TABLE_SCHEDULE, CALS_TABLE_ALARM, related_id);
		break;
	case CALS_ALARM_REMOVE_ALL:
		sprintf(query, "SELECT alarm_id FROM %s WHERE alarm_id <> 0", CALS_TABLE_ALARM);
		break;
	}

	stmt = cals_query_prepare(query);
	retvm_if(NULL == stmt, CAL_ERR_DB_FAILED, "cals_query_prepare() Failed");

	ret = cals_stmt_step(stmt);
	if (ret < CAL_SUCCESS) {
		sqlite3_finalize(stmt);
		ERR("sqlite3_step() Failed(%d)", ret);
		return CAL_ERR_DB_FAILED;
	}

	while (CAL_TRUE == ret) {
		alarmmgr_remove_alarm(sqlite3_column_int(stmt,0));
		ret = cals_stmt_step(stmt);
	}
	sqlite3_finalize(stmt);

	// TODO: If calendar service use delete_table not delete_flag, below procedure can handle by trigger.
	switch (type) {
	case CALS_ALARM_REMOVE_BY_EVENT_ID:
		sprintf(query, "DELETE FROM %s WHERE event_id = %d", CALS_TABLE_ALARM, related_id);
		break;
	case CALS_ALARM_REMOVE_BY_CALENDAR_ID:
		sprintf(query, "DELETE FROM %s "
			"WHERE event_id IN (SELECT id FROM %s WHERE calendar_id = %d)",
			CALS_TABLE_ALARM, CALS_TABLE_SCHEDULE, related_id);
		break;
	case CALS_ALARM_REMOVE_BY_ACC_ID:
		sprintf(query, "DELETE FROM %s "
			"WHERE event_id IN (SELECT id FROM %s WHERE account_id = %d)",
			CALS_TABLE_ALARM, CALS_TABLE_SCHEDULE, related_id);
		break;
	case CALS_ALARM_REMOVE_ALL:
		sprintf(query, "DELETE FROM %s", CALS_TABLE_ALARM);
		break;
	}

	stmt = cals_query_prepare(query);
	retvm_if(NULL == stmt, CAL_ERR_DB_FAILED, "cals_query_prepare() Failed");

	ret = cals_stmt_step(stmt);
	if (CAL_SUCCESS != ret) {
		sqlite3_finalize(stmt);
		ERR("sqlite3_step(%s) Failed(%d)", query, ret);
		return CAL_ERR_DB_FAILED;
	}
	sqlite3_finalize(stmt);

	return CAL_SUCCESS;
}

static inline int _cals_alarm_add(const int event_id, cal_alarm_info_t *alarm_info)
{
	int rc = -1;
	sqlite3_stmt *stmt = NULL;
	char sql_value[CALS_SQL_MAX_LEN] = {0};
	time_t conv_alarm_time;

	conv_alarm_time = cals_mktime(&alarm_info->alarm_time);

	sprintf(sql_value, "INSERT INTO %s(event_id,alarm_time,remind_tick,remind_tick_unit,alarm_tone,alarm_description,alarm_type,alarm_id) "
			"VALUES(%d,%ld,%d,%d,?,?,%d,%d)", CALS_TABLE_ALARM,
			event_id,
			conv_alarm_time,
			alarm_info->remind_tick,
			alarm_info->remind_tick_unit,
			alarm_info->alarm_type,
			alarm_info->alarm_id);

	stmt = cals_query_prepare(sql_value);
	retvm_if(NULL == stmt, CAL_ERR_DB_FAILED, "cals_query_prepare() Failed");

	if (alarm_info->alarm_tone)
		cals_stmt_bind_text(stmt, 0, alarm_info->alarm_tone);

	if (alarm_info->alarm_description)
		cals_stmt_bind_text(stmt, 1, alarm_info->alarm_description);

	rc = cals_stmt_step(stmt);
	if (CAL_SUCCESS != rc) {
		sqlite3_finalize(stmt);
		ERR("cals_stmt_step() Failed(%d)", rc);
		return rc;
	}

	sqlite3_finalize(stmt);

	return CAL_SUCCESS;
}

int cals_alarm_add(int event_id, cal_alarm_info_t *alarm_info, struct tm *start_date_time)
{
	CALS_FN_CALL;
	int ret = 0;
	alarm_id_t alarm_id;
	alarm_date_t alarm_date = {0};
	alarm_entry_t *aet = NULL;
	struct tm time;
	time_t base_tt, alarm_tt;

	if(alarm_info->remind_tick_unit == CAL_SCH_TIME_UNIT_OFF)
		return CAL_INVALID_INDEX;

	//update the alarm time
	aet = alarmmgr_create_alarm();
	retvm_if(NULL == aet, CAL_ERR_ALARMMGR_FAILED, "alarmmgr_create_alarm() Failed");

	//calculate time
	if(alarm_info->remind_tick_unit == CAL_SCH_TIME_UNIT_SPECIFIC)	{
		base_tt = cals_mktime(&alarm_info->alarm_time);
	} else {
		int sec = 0;

		switch (alarm_info->remind_tick_unit) {
		case CAL_SCH_TIME_UNIT_MIN:
			sec = 60;
			break;
		case CAL_SCH_TIME_UNIT_HOUR:
			sec = 3600;
			break;
		case CAL_SCH_TIME_UNIT_DAY:
			sec = ONE_DAY_SECONDS;
			break;
		case CAL_SCH_TIME_UNIT_WEEK:
			sec = ONE_WEEK_SECONDS;
			break;
		case CAL_SCH_TIME_UNIT_MONTH:
			sec = ONE_MONTH_SECONDS;
			break;
		case CAL_SCH_TIME_UNIT_OFF:
		default:
			alarmmgr_free_alarm(aet);
			return CAL_INVALID_INDEX;
		}

		sec = sec * alarm_info->remind_tick;

		base_tt = cals_mktime(start_date_time) - sec;
	}

	calendar_svc_util_gmt_to_local(base_tt,&alarm_tt);
	cals_tmtime_r(&alarm_tt,&time);
	TMDUMP(time);

	alarm_date.year = time.tm_year + 1900;
	alarm_date.month = time.tm_mon + 1;
	alarm_date.day = time.tm_mday;
	alarm_date.hour = time.tm_hour;
	alarm_date.min = time.tm_min;
	alarm_date.sec = time.tm_sec;

	ret = alarmmgr_set_time(aet, alarm_date);
	if (ret < 0) {
		alarmmgr_free_alarm(aet);
		ERR("alarmmgr_set_time() Failed(%d)", ret);
		return CAL_ERR_ALARMMGR_FAILED;
	}

	ret = alarmmgr_set_repeat_mode(aet, 0, 0);
	if (ret < 0) {
		alarmmgr_free_alarm(aet);
		ERR("alarmmgr_set_repeat_mode() Failed(%d)", ret);
		return CAL_ERR_ALARMMGR_FAILED;
	}

	ret = alarmmgr_set_type(aet, ALARM_TYPE_DEFAULT);
	if (ret < 0) {
		alarmmgr_free_alarm(aet);
		ERR("alarmmgr_set_type() Failed(%d)", ret);
		return CAL_ERR_ALARMMGR_FAILED;
	}

	ret = alarmmgr_add_alarm_with_localtime(aet, PKG_CALENDAR_APP, &alarm_id);
	if (ret < 0) {
		alarmmgr_free_alarm(aet);
		ERR("alarmmgr_add_alarm_with_localtime() Failed(%d)", ret);
		return CAL_ERR_ALARMMGR_FAILED;
	}

	alarmmgr_free_alarm(aet);
	alarm_info->alarm_id = alarm_id;

	_cals_alarm_add(event_id, alarm_info);

	return CAL_SUCCESS;
}


int cals_alarm_get_event_id(int alarm_id)
{
	char query[CALS_SQL_MIN_LEN];

	sprintf(query, "SELECT event_id FROM cal_alarm_table WHERE alarm_id=%d", alarm_id);

	return cals_query_get_first_int_result(query);
}


static void _cals_alarm_value_free(gpointer data, gpointer user_data)
{
	if (NULL == data)
		return;

	free(((cal_alarm_info_t*)((cal_value*)data)->user_data)->alarm_tone);
	free(((cal_alarm_info_t*)((cal_value*)data)->user_data)->alarm_description);
	free(data);
}


int cals_get_alarm_info(const int event_id, GList **alarm_list)
{
	int ret = -1;
	GList *result = NULL;
	sqlite3_stmt *stmt = NULL;
	cal_value * cvalue = NULL;
	char query[CALS_SQL_MAX_LEN] = {0};
	cal_alarm_info_t* alarm_info = NULL;

	retv_if(NULL == alarm_list, CAL_ERR_ARG_NULL);

	sprintf(query, "SELECT * FROM %s WHERE event_id=%d", CALS_TABLE_ALARM, event_id);

	stmt = cals_query_prepare(query);
	retvm_if(NULL == stmt, CAL_ERR_DB_FAILED, "cals_query_prepare() Failed");

	ret = cals_stmt_step(stmt);
	if (ret < CAL_SUCCESS) {
		sqlite3_finalize(stmt);
		ERR("cals_stmt_step() Failed(%d)", ret);
		return ret;
	}

	while (CAL_TRUE == ret)
	{
		cvalue = calloc(1, sizeof(cal_value));
		if (NULL == cvalue) {
			sqlite3_finalize(stmt);
			g_list_foreach(result, _cals_alarm_value_free, NULL);
			g_list_free(result);
			ERR("calloc() Failed(%d)", errno);
			return CAL_ERR_OUT_OF_MEMORY;
		}

		cvalue->v_type = CAL_EVENT_ALARM;
		cvalue->user_data = alarm_info = calloc(1, sizeof(cal_alarm_info_t));
		if (NULL == alarm_info) {
			sqlite3_finalize(stmt);
			g_list_foreach(result, _cals_alarm_value_free, NULL);
			g_list_free(result);
			free(cvalue);
			ERR("calloc() Failed(%d)", errno);
			return CAL_ERR_OUT_OF_MEMORY;
		}

		alarm_info->event_id = sqlite3_column_int(stmt, 0);

		long int temp = 0;
		temp = sqlite3_column_int(stmt, 1);
		cal_db_service_copy_struct_tm((struct tm*)cals_tmtime(&temp),&(alarm_info->alarm_time));
		alarm_info->remind_tick = sqlite3_column_int(stmt,2);
		alarm_info->remind_tick_unit = sqlite3_column_int(stmt, 3);
		alarm_info->alarm_tone = SAFE_STRDUP(sqlite3_column_text(stmt, 4));
		alarm_info->alarm_description = SAFE_STRDUP(sqlite3_column_text(stmt, 5));
		alarm_info->alarm_type = sqlite3_column_int(stmt, 6);
		alarm_info->alarm_id = sqlite3_column_int(stmt, 7);

		result = g_list_append(result, cvalue);

		ret = cals_stmt_step(stmt);
	}
	sqlite3_finalize(stmt);

	*alarm_list = result;

	return CAL_SUCCESS;
}


