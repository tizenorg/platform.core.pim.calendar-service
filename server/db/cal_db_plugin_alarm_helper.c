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

#include <stdlib.h>

#include "calendar_db.h"

#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_view.h"
#include "cal_time.h"
#include "cal_record.h"
#include "cal_list.h"

#include "cal_db_util.h"
#include "cal_db.h"
#include "cal_db_query.h"
#include "cal_db_instance.h"
#include "cal_db_plugin_alarm_helper.h"
#include "cal_db_util.h"
#include "cal_utils.h"

static int _cal_db_alarm_insert_record(calendar_record_h record, int parent_id)
{
	cal_alarm_s *alarm = NULL;

	alarm = (cal_alarm_s *)(record);
	RETVM_IF(NULL == alarm, CALENDAR_ERROR_INVALID_PARAMETER,
			"Invalid argument: cal_alarm_s is NULL");

	if (alarm->remind_tick_unit == CALENDAR_ALARM_NONE) {
		DBG("No alarm unit tick");
		return CALENDAR_ERROR_NONE;
	}

	int ret = 0;
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	sqlite3_stmt *stmt = NULL;

	snprintf(query, sizeof(query),
			"INSERT INTO %s ("
			"event_id, "
			"remind_tick, remind_tick_unit, "
			"alarm_description, "
			"alarm_type,  "
			"alarm_summary, alarm_action, alarm_attach, "
			"alarm_utime, alarm_datetime "
			") VALUES ("
			"%d, "
			"%d, %d, "
			"?, "
			"%d, "
			"?, %d, ?, "
			"%lld, ?)",
			CAL_TABLE_ALARM,
			parent_id,
			alarm->remind_tick, alarm->remind_tick_unit,
			alarm->alarm.type,
			alarm->alarm_action,
			alarm->alarm.time.utime);

	ret = cal_db_util_query_prepare(query, &stmt);
	if (CALENDAR_ERROR_NONE != ret) {
		/* LCOV_EXCL_START */
		ERR("cal_db_util_query_prepare() Fail(%d)", ret);
		SECURE("query[%s]", query);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	int index = 1;

	if (alarm->alarm_description) {
		cal_db_util_stmt_bind_text(stmt, index, alarm->alarm_description);
		DBG("description[%s]", alarm->alarm_description);
	}
	index++;

	if (alarm->alarm_summary) {
		cal_db_util_stmt_bind_text(stmt, index, alarm->alarm_summary);
		DBG("summary [%s]", alarm->alarm_summary);
	}
	index++;

	if (alarm->alarm_attach)
		cal_db_util_stmt_bind_text(stmt, index, alarm->alarm_attach);
	index++;

	if (CALENDAR_TIME_LOCALTIME == alarm->alarm.type) {
		char alarm_datetime[CAL_STR_SHORT_LEN32] = {0};
		snprintf(alarm_datetime, sizeof(alarm_datetime), CAL_FORMAT_LOCAL_DATETIME,
				alarm->alarm.time.date.year,
				alarm->alarm.time.date.month,
				alarm->alarm.time.date.mday,
				alarm->alarm.time.date.hour,
				alarm->alarm.time.date.minute,
				alarm->alarm.time.date.second);
		cal_db_util_stmt_bind_text(stmt, index, alarm_datetime);
		DBG("datetime [%s]", alarm_datetime);
	}
	index++;

	ret = cal_db_util_stmt_step(stmt);
	sqlite3_finalize(stmt);
	if (CALENDAR_ERROR_NONE != ret) {
		/* LCOV_EXCL_START */
		ERR("cal_db_util_stmt_step() Fail(%d)", ret);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	return CALENDAR_ERROR_NONE;
}

int cal_db_alarm_insert_records(cal_list_s *list_s, int event_id)
{
	int ret;
	int count = 0;
	calendar_record_h record = NULL;
	calendar_list_h list = (calendar_list_h)list_s;
	RETV_IF(NULL == list, CALENDAR_ERROR_INVALID_PARAMETER);

	calendar_list_get_count(list, &count);
	if (0 == count)
		return CALENDAR_ERROR_NONE;

	calendar_list_first(list);
	while (CALENDAR_ERROR_NONE == calendar_list_get_current_record_p(list, &record)) {
		ret = _cal_db_alarm_insert_record(record, event_id);
		RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_db_extended_insert_record() Fail(%d)", ret);
		calendar_list_next(list);
	}
	return CALENDAR_ERROR_NONE;
}

int cal_db_alarm_get_records(int parent, cal_list_s *list)
{
	int ret;
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	sqlite3_stmt *stmt = NULL;

	RETVM_IF(NULL == list, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter: list is NULL");

	snprintf(query, sizeof(query), "SELECT rowid,remind_tick,remind_tick_unit,"
			"alarm_description,alarm_type,alarm_summary,alarm_action,alarm_attach,"
			"alarm_utime,alarm_datetime FROM %s WHERE event_id=%d ",
			CAL_TABLE_ALARM, parent);

	ret = cal_db_util_query_prepare(query, &stmt);
	if (CALENDAR_ERROR_NONE != ret) {
		/* LCOV_EXCL_START */
		ERR("cal_db_util_query_prepare() Fail(%d)", ret);
		SECURE("query[%s]", query);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	int index = 0;
	const unsigned char *temp;
	calendar_record_h record = NULL;
	cal_alarm_s *alarm = NULL;

	while (CAL_SQLITE_ROW == cal_db_util_stmt_step(stmt)) {
		ret = calendar_record_create(_calendar_alarm._uri, &record);
		if (CALENDAR_ERROR_NONE != ret) {
			/* LCOV_EXCL_START */
			sqlite3_finalize(stmt);
			cal_list_clear(list);
			return ret;
			/* LCOV_EXCL_STOP */
		}

		index = 0;
		alarm = (cal_alarm_s *)(record);

		alarm->parent_id = parent;
		alarm->id = sqlite3_column_int(stmt, index++);
		alarm->remind_tick = sqlite3_column_int(stmt, index++);
		alarm->remind_tick_unit = sqlite3_column_int(stmt, index++);

		temp = sqlite3_column_text(stmt, index++);
		alarm->alarm_description = cal_strdup((const char*)temp);

		alarm->alarm.type = sqlite3_column_int(stmt, index++);

		temp = sqlite3_column_text(stmt, index++);
		alarm->alarm_summary = cal_strdup((const char*)temp);

		alarm->alarm_action = sqlite3_column_int(stmt, index++);

		temp = sqlite3_column_text(stmt, index++);
		alarm->alarm_attach = cal_strdup((const char*)temp);

		if (alarm->alarm.type == CALENDAR_TIME_UTIME) {
			alarm->alarm.time.utime = sqlite3_column_int64(stmt, index++);
			index++; /* datetime */
		} else {
			index++; /* utime */
			temp = sqlite3_column_text(stmt, index++);
			if (temp) {
				int y = 0, m = 0, d = 0;
				int h = 0, n = 0, s = 0;
				switch (strlen((const char*)temp)) {
				case 8:
					sscanf((const char *)temp, CAL_DATETIME_FORMAT_YYYYMMDD, &y, &m, &d);
					break;
				case 15:
					sscanf((const char *)temp, CAL_DATETIME_FORMAT_YYYYMMDDTHHMMSS, &y, &m, &d, &h, &n, &s);
					break;
				case 19:
					sscanf((const char *)temp, CAL_FORMAT_LOCAL_DATETIME, &y, &m, &d, &h, &n, &s);
					break;
				}
				alarm->alarm.time.date.year = y;
				alarm->alarm.time.date.month = m;
				alarm->alarm.time.date.mday = d;
				alarm->alarm.time.date.hour = h;
				alarm->alarm.time.date.minute = n;
				alarm->alarm.time.date.second = s;
			}
		}
		calendar_list_add((calendar_list_h)list, record);
	}
	sqlite3_finalize(stmt);
	return CALENDAR_ERROR_NONE;
}

int cal_db_alarm_delete_with_id(int parent_id)
{
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	int ret = 0;

	snprintf(query, sizeof(query), "DELETE FROM %s WHERE event_id=%d ",
			CAL_TABLE_ALARM, parent_id);

	ret = cal_db_util_query_exec(query);
	if (CALENDAR_ERROR_NONE != ret) {
		/* LCOV_EXCL_START */
		ERR("cal_db_util_query_exec() Fail(%d)", ret);
		SECURE("[%s]", query);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	return CALENDAR_ERROR_NONE;
}

int cal_db_alarm_has_alarm(cal_list_s *list_s)
{
	calendar_record_h alarm = NULL;
	int has_alarm = 0;
	calendar_list_h list = (calendar_list_h)list_s;

	calendar_list_first(list);
	while (CALENDAR_ERROR_NONE == calendar_list_get_current_record_p(list, &alarm)) {
		int unit;
		calendar_record_get_int(alarm, _calendar_alarm.tick_unit, &unit);
		if (CALENDAR_ALARM_NONE != unit) {
			has_alarm = 1;
			break;
		}
		calendar_list_next(list);
	}
	return has_alarm;
}
