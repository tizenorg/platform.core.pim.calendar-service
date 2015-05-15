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

#include "cal_typedef.h"
#include "cal_db.h"
#include "cal_db_util.h"
#include "cal_internal.h"
#include "calendar_errors.h"
#include "cal_db_instance_helper.h"

int cal_db_instance_normal_insert_record(cal_instance_normal_s *normal, int* id)
{
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	RETV_IF(NULL == normal, CALENDAR_ERROR_INVALID_PARAMETER);

	snprintf(query, sizeof(query),
			"INSERT INTO %s ("
			"event_id, "
			"dtstart_utime, dtend_utime"
			") VALUES ( "
			"%d, "
			"%lld, %lld) ",
			CAL_TABLE_NORMAL_INSTANCE,
			normal->event_id,
			normal->start.time.utime, normal->end.time.utime);

	cal_db_util_error_e dbret = CAL_DB_OK;
	dbret = cal_db_util_query_exec(query);
	if (dbret != CAL_DB_OK) {
		ERR("cal_db_util_query_exec() failed (%d)", dbret);
		switch (dbret) {
		case CAL_DB_ERROR_NO_SPACE:
			return CALENDAR_ERROR_FILE_NO_SPACE;
		default:
			return CALENDAR_ERROR_DB_FAILED;
		}
	}
	if (id) *id = cal_db_util_last_insert_id();

	return CALENDAR_ERROR_NONE;
}

int cal_db_instance_allday_insert_record(cal_instance_allday_s *allday, int* id)
{
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	RETV_IF(NULL == allday, CALENDAR_ERROR_INVALID_PARAMETER);

	snprintf(query, sizeof(query),
			"INSERT INTO %s ("
			"event_id, "
			"dtstart_datetime, dtend_datetime"
			") VALUES ( "
			"%d, "
			"'%04d-%02d-%02dT%02d:%02d:%02d', '%04d-%02d-%02dT%02d:%02d:%02d') ",
			CAL_TABLE_ALLDAY_INSTANCE,
			allday->event_id,
			allday->start.time.date.year, allday->start.time.date.month, allday->start.time.date.mday,
			allday->start.time.date.hour, allday->start.time.date.minute, allday->start.time.date.second,
			allday->end.time.date.year, allday->end.time.date.month, allday->end.time.date.mday,
			allday->end.time.date.hour, allday->end.time.date.minute, allday->end.time.date.second);

	cal_db_util_error_e dbret = CAL_DB_OK;
	dbret = cal_db_util_query_exec(query);
	if (dbret != CAL_DB_OK) {
		ERR("cal_db_util_query_exec() failed (%d)", dbret);
		switch (dbret) {
		case CAL_DB_ERROR_NO_SPACE:
			return CALENDAR_ERROR_FILE_NO_SPACE;
		default:
			return CALENDAR_ERROR_DB_FAILED;
		}
	}
	if (id) *id = cal_db_util_last_insert_id();

	return CALENDAR_ERROR_NONE;
}

int cal_db_instance_helper_insert_utime_instance(int event_id, long long int s, long long int e)
{
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	snprintf(query, sizeof(query), "INSERT INTO %s (event_id, dtstart_utime, dtend_utime) "
			"VALUES (%d, %lld, %lld) ", CAL_TABLE_NORMAL_INSTANCE, event_id, s, e);

	cal_db_util_error_e dbret = CAL_DB_OK;
	dbret = cal_db_util_query_exec(query);
	if (dbret != CAL_DB_OK) {
		ERR("cal_db_util_query_exec() failed (%d)", dbret);
		SECURE("[%s]", query);
		switch (dbret) {
		case CAL_DB_ERROR_NO_SPACE:
			return CALENDAR_ERROR_FILE_NO_SPACE;
		default:
			return CALENDAR_ERROR_DB_FAILED;
		}
	}
	return CALENDAR_ERROR_NONE;
}

int cal_db_instance_helper_insert_localtime_instance(int event_id, const char *s, const char *e)
{
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	snprintf(query, sizeof(query), "INSERT INTO %s (event_id, dtstart_datetime, dtend_datetime) "
			"VALUES (%d, '%s', '%s') ", CAL_TABLE_ALLDAY_INSTANCE, event_id, s, e);

	cal_db_util_error_e dbret = CAL_DB_OK;
	dbret = cal_db_util_query_exec(query);
	if (dbret != CAL_DB_OK) {
		ERR("cal_db_util_query_exec() failed (%d)", dbret);
		SECURE("[%s]", query);
		switch (dbret) {
		case CAL_DB_ERROR_NO_SPACE:
			return CALENDAR_ERROR_FILE_NO_SPACE;
		default:
			return CALENDAR_ERROR_DB_FAILED;
		}
	}
	return CALENDAR_ERROR_NONE;
}

