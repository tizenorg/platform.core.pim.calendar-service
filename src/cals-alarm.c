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
#include <appsvc.h>

#include "cals-internal.h"
#include "cals-typedef.h"
#include "cals-db-info.h"
#include "cals-sqlite.h"
#include "cals-utils.h"
#include "cals-alarm.h"
#include "cals-time.h"

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

int _cals_alarm_add_to_db(const int event_id, cal_alarm_info_t *alarm_info)
{
	CALS_FN_CALL;
	int r;
	sqlite3_stmt *stmt = NULL;
	char query[CALS_SQL_MAX_LEN] = {0};

	sprintf(query, "INSERT INTO %s ("
			"event_id, "
			"alarm_time, remind_tick, remind_tick_unit, alarm_tone, "
			"alarm_description, alarm_type, alarm_id "
			") VALUES ( "
			"%d, "
			"%lld, %d, %d, ?, "
			"?, %d, %d )",
			CALS_TABLE_ALARM,
			event_id,
			alarm_info->alarm_time, alarm_info->remind_tick, alarm_info->remind_tick_unit,
			alarm_info->alarm_type, alarm_info->alarm_id);

	DBG("query(%s)\n", query);
	stmt = cals_query_prepare(query);
	retvm_if(NULL == stmt, CAL_ERR_DB_FAILED, "cals_query_prepare() Failed");

	if (alarm_info->alarm_tone)
		cals_stmt_bind_text(stmt, 1, alarm_info->alarm_tone);

	if (alarm_info->alarm_description)
		cals_stmt_bind_text(stmt, 2, alarm_info->alarm_description);

	r = cals_stmt_step(stmt);
	sqlite3_finalize(stmt);

	if (CAL_SUCCESS != r) {
		ERR("cals_stmt_step() Failed(%d)", r);
		return r;
	}

	return CAL_SUCCESS;
}

static long long int _cals_get_interval(cal_alarm_info_t *alarm_info, struct cals_time *start_time)
{
	long long int iv, diff;
	int sec;
	struct cals_time at;

	if (alarm_info->remind_tick_unit == CAL_SCH_TIME_UNIT_SPECIFIC) {
		at.type = CALS_TIME_UTIME;
		at.utime = alarm_info->alarm_time;
		return cals_time_diff_with_now(&at);
	}

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
		return 0;
	}

	sec = sec * alarm_info->remind_tick;
	if (start_time->type == CALS_TIME_UTIME) {
		alarm_info->alarm_time = start_time->utime - sec;

	} else {
		alarm_info->alarm_time = cals_time_convert_to_lli(start_time) - sec;

	}

	diff =  cals_time_diff_with_now(start_time);
	iv = diff - (long long int)sec;
	DBG("tick(%d) tick unit(%d) "
			",so sets (%lld) = diff(%lld) - sec(%lld)",
			alarm_info->remind_tick, alarm_info->remind_tick_unit,
			iv, diff, (long long int)sec);

	return iv;
}

bundle *_get_appsvc(const char *pkg)
{
	int r;
	bundle *b;

	b = bundle_create();
	if (!b) {
		ERR("bundle_create failed");
		return NULL;
	}

	r = appsvc_set_pkgname(b, pkg);
	appsvc_set_operation(b, APPSVC_OPERATION_DEFAULT);
	if (r) {
		bundle_free(b);
		ERR("appsvc_set_pkgname failed (%d)", r);
		return NULL;
	}

	return b;
}

int _cals_alarm_add_to_alarmmgr(struct cals_time *start_time, cal_alarm_info_t *alarm_info)
{
	int ret;
	long long int iv;
	bundle *b;
	alarm_id_t alarm_id = 0;

	iv = _cals_get_interval(alarm_info, start_time);
	if (iv < 0) {
		DBG("Tried to register past event, so passed registration");
		return 0;

	} else if (iv == 0) {
		DBG("Set no alarm");
		return 0;
	}

	b = _get_appsvc(PKG_CALENDAR_APP);
	if (!b) {
		ERR("_get_appsvc failed");
		return CAL_ERR_FAIL;
	}

	ret = alarmmgr_add_alarm_appsvc(ALARM_TYPE_DEFAULT, (long int)iv, 0, b, &alarm_id);
	bundle_free(b);

	if (ret) {
		ERR("alarmmgr_add_alarm_appsvc failed (%d)", ret);
		return CAL_ERR_ALARMMGR_FAILED;
	}
	DBG("Set alarm id(%d)", alarm_id);
	alarm_info->alarm_id = alarm_id;

	return 0;
}

int cals_alarm_add(int event_id, cal_alarm_info_t *alarm_info, struct cals_time *start_time)
{
	CALS_FN_CALL;
	int ret;

	if(alarm_info->remind_tick_unit == CAL_SCH_TIME_UNIT_OFF)
		return CAL_SUCCESS;

	ret = _cals_alarm_add_to_alarmmgr(start_time, alarm_info);
	if (ret) {
		ERR("Failed to register alarm");
		return CAL_ERR_FAIL;
	}

	ret = _cals_alarm_add_to_db(event_id, alarm_info);
	if (ret) {
		ERR("Failed to add alarm to db");
		return CAL_ERR_FAIL;
	}

	return CAL_SUCCESS;
}


int cals_alarm_get_event_id(int alarm_id)
{
	char query[CALS_SQL_MIN_LEN];

	sprintf(query, "SELECT event_id FROM %s WHERE alarm_id=%d", CALS_TABLE_ALARM, alarm_id);

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

		alarm_info->alarm_time = sqlite3_column_int64(stmt, 1);
		alarm_info->remind_tick = sqlite3_column_int(stmt, 2);
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

