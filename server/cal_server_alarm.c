/*
 * Calendar Service
 *
 * Copyright (c) 2012 - 2013 Samsung Electronics Co., Ltd. All rights reserved.
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

#include <appsvc.h>
#include <alarm.h>
#include <vconf.h>

#include "cal_internal.h"
#include "calendar2.h"
#include "cal_time.h"
#include "cal_typedef.h"
#include "cal_inotify.h"

#include "cal_db_util.h"
#include "cal_db.h"
#include "cal_db_query.h"
#include "cal_server_reminder.h"

static struct timeval stv; // check time
static struct timeval etv; // check time

static int __cal_server_alarm_unset_alerted_alarmmgr_id(int alarm_id);

static int __cal_server_alarm_clear_all_cb(alarm_id_t alarm_id, void *data)
{
	int ret;

	DBG("remove alarm id(%d)", alarm_id);
	__cal_server_alarm_unset_alerted_alarmmgr_id(alarm_id);
	ret = alarmmgr_remove_alarm(alarm_id);
	if (ret != ALARMMGR_RESULT_SUCCESS)
	{
		ERR("alarmmgr_remove_alarm() failed(ret:%d)", ret);
		return ret;
	}
	return CALENDAR_ERROR_NONE;
}

struct _normal_data_s
{
	int id;
	long long int alarm_utime;
	int tick;
	int unit;
	int alarm_id;
	int record_type;
};

struct _allday_data_s
{
	int id;
	int alarm_datetime;
	int tick;
	int unit;
	int alarm_id;
	int record_type;
};

static int __cal_server_alarm_get_next_list_normal_event(long long int from_utime, long long int to_utime, GList **list, int *count)
{
	int index;
    char query[CAL_DB_SQL_MAX_LEN] = {0};
    sqlite3_stmt *stmt = NULL;

	DBG("searching normal event (%lld) ~ (%lld)", from_utime, to_utime);

	snprintf(query, sizeof(query),
			"SELECT A.event_id, B.alarm_time, "
			"B.remind_tick, B.remind_tick_unit, B.alarm_id "
			"FROM %s as A, "                       // A is normal instance
			"%s as B ON A.event_id = B.event_id, " // B is alarm
			"%s as C ON B.event_id = C.id "        // c is schedule
			"WHERE C.has_alarm == 1 AND C.type = %d "
			"AND B.remind_tick_unit = %d AND B.alarm_time = %lld "
			"UNION "
			"SELECT A.event_id, A.dtstart_utime - (B.remind_tick * B.remind_tick_unit), "
			"B.remind_tick, B.remind_tick_unit, B.alarm_id "
			"FROM %s as A, "                       // A is normal instance
			"%s as B ON A.event_id = B.event_id, " // B is alarm
			"%s as C ON B.event_id = C.id "        // c is schedule
			"WHERE C.has_alarm = 1 AND C.type = %d "
			"AND B.remind_tick_unit > %d "
			"AND (A.dtstart_utime - (B.remind_tick * B.remind_tick_unit)) >= %lld "
			"AND (A.dtstart_utime - (B.remind_tick * B.remind_tick_unit)) < %lld "
			"ORDER BY (A.dtstart_utime - (B.remind_tick * B.remind_tick_unit))",
			CAL_TABLE_NORMAL_INSTANCE, CAL_TABLE_ALARM, CAL_TABLE_SCHEDULE,
			CALENDAR_BOOK_TYPE_EVENT,
			CALENDAR_ALARM_TIME_UNIT_SPECIFIC, from_utime,
			CAL_TABLE_NORMAL_INSTANCE, CAL_TABLE_ALARM, CAL_TABLE_SCHEDULE,
			CALENDAR_BOOK_TYPE_EVENT,
			CALENDAR_ALARM_TIME_UNIT_SPECIFIC,
			from_utime, to_utime);

	stmt = _cal_db_util_query_prepare(query);
	if (NULL == stmt)
	{
		DBG("query[%s]", query);
		ERR("_cal_db_util_query_prepare() Failed");
		return CALENDAR_ERROR_DB_FAILED;
	}

	*count = 0;
	while (CAL_DB_ROW  == _cal_db_util_stmt_step(stmt))
	{
		struct _normal_data_s *nd = calloc(1, sizeof(struct _normal_data_s));

		index = 0;
		nd->id = sqlite3_column_int(stmt, index++);
		nd->alarm_utime = sqlite3_column_int64(stmt, index++);
		nd->tick = sqlite3_column_int(stmt, index++);
		nd->unit = sqlite3_column_int(stmt, index++);
		nd->alarm_id = sqlite3_column_int(stmt, index++);

		*list = g_list_append(*list, nd);
		(*count)++;
	}
	sqlite3_finalize(stmt);
	return CALENDAR_ERROR_NONE;
}

static int __cal_server_alarm_get_next_list_normal_todo(long long int from_utime, long long int to_utime, GList **list, int *count)
{
	int index;
    char query[CAL_DB_SQL_MAX_LEN] = {0};
    sqlite3_stmt *stmt = NULL;

	DBG("searching normal todo (%lld) ~ (%lld)", from_utime, to_utime);

	snprintf(query, sizeof(query),
			"SELECT B.id, A.alarm_time, "
			"A.remind_tick, A.remind_tick_unit, A.alarm_id "
			"FROM %s as A, "                          // A is alarm
			"%s as B ON A.event_id = B.id "           // B is schedule
			"WHERE B.has_alarm == 1 AND B.type = %d "
			"AND A.remind_tick_unit = %d AND A.alarm_time = %lld "
			"UNION "
			"SELECT B.id, B.dtend_utime - (A.remind_tick * A.remind_tick_unit), "
			"A.remind_tick, A.remind_tick_unit, A.alarm_id "
			"FROM %s as A, "                          // A is alarm
			"%s as B ON A.event_id = B.id "           // B is schedule
			"WHERE B.has_alarm = 1 AND B.type = %d "
			"AND A.remind_tick_unit > %d "
			"AND (B.dtend_utime - (A.remind_tick * A.remind_tick_unit)) >= %lld "
			"AND (B.dtend_utime - (A.remind_tick * A.remind_tick_unit)) < %lld "
			"ORDER BY (B.dtend_utime - (A.remind_tick * A.remind_tick_unit))",
			CAL_TABLE_ALARM, CAL_TABLE_SCHEDULE,
			CALENDAR_BOOK_TYPE_TODO,
			CALENDAR_ALARM_TIME_UNIT_SPECIFIC, from_utime,
			CAL_TABLE_ALARM, CAL_TABLE_SCHEDULE,
			CALENDAR_BOOK_TYPE_TODO,
			CALENDAR_ALARM_TIME_UNIT_SPECIFIC,
			from_utime, to_utime);

	stmt = _cal_db_util_query_prepare(query);
	if (NULL == stmt)
	{
		DBG("query[%s]", query);
		ERR("_cal_db_util_query_prepare() Failed");
		return CALENDAR_ERROR_DB_FAILED;
	}

	*count = 0;
	while (CAL_DB_ROW  == _cal_db_util_stmt_step(stmt))
	{
		struct _normal_data_s *nd = calloc(1, sizeof(struct _normal_data_s));

		index = 0;
		nd->id = sqlite3_column_int(stmt, index++);
		nd->alarm_utime = sqlite3_column_int64(stmt, index++);
		nd->tick = sqlite3_column_int(stmt, index++);
		nd->unit = sqlite3_column_int(stmt, index++);
		nd->alarm_id = sqlite3_column_int(stmt, index++);

		*list = g_list_append(*list, nd);
		(*count)++;
	}
	sqlite3_finalize(stmt);
	return CALENDAR_ERROR_NONE;
}

static int __cal_server_alarm_get_next_list_allday_event(int from_datetime, int to_datetime, GList **list, int *count)
{
	int index;
    char query[CAL_DB_SQL_MAX_LEN] = {0};
    sqlite3_stmt *stmt = NULL;

	DBG("searching allday (%d) ~ (%d)", from_datetime, to_datetime);

	snprintf(query, sizeof(query),
			"SELECT A.event_id, B.alarm_time, "
			"B.remind_tick, B.remind_tick_unit, B.alarm_id "
			"FROM %s as A, "                       // A is allday instance
			"%s as B ON A.event_id = B.event_id, " // B is alarm
			"%s as C ON B.event_id = C.id "        // C is schedule
			"WHERE C.has_alarm == 1 AND C.type = %d "
			"AND B.remind_tick_unit = %d AND B.alarm_time = %d "
			"UNION "
			"SELECT A.event_id, A.dtstart_datetime, "
			"B.remind_tick, B.remind_tick_unit, B.alarm_id "
			"FROM %s as A, "                       // A is allday instance
			"%s as B ON A.event_id = B.event_id, " // B is alarm
			"%s as C ON B.event_id = C.id "        // C is schedule
			"WHERE C.has_alarm = 1 AND C.type = %d "
			"AND B.remind_tick_unit > %d "
			"AND A.dtstart_datetime - (B.remind_tick * B.remind_tick_unit)/86400 > %d "
			"AND A.dtstart_datetime - (B.remind_tick * B.remind_tick_unit)/86400 <= %d ",
			CAL_TABLE_ALLDAY_INSTANCE, CAL_TABLE_ALARM, CAL_TABLE_SCHEDULE,
			CALENDAR_BOOK_TYPE_EVENT,
			CALENDAR_ALARM_TIME_UNIT_SPECIFIC, from_datetime,
			CAL_TABLE_ALLDAY_INSTANCE, CAL_TABLE_ALARM, CAL_TABLE_SCHEDULE,
			CALENDAR_BOOK_TYPE_EVENT,
			CALENDAR_ALARM_TIME_UNIT_SPECIFIC,
			from_datetime, to_datetime);

	stmt = _cal_db_util_query_prepare(query);
	if (NULL == stmt)
	{
		DBG("query[%s]", query);
		ERR("_cal_db_util_query_prepare() Failed");
		return CALENDAR_ERROR_DB_FAILED;
	}

	*count = 0;
	while (CAL_DB_ROW  == _cal_db_util_stmt_step(stmt))
	{
		struct _allday_data_s *ad = calloc(1, sizeof(struct _allday_data_s));

		index = 0;
		ad->id = sqlite3_column_int(stmt, index++);
		ad->alarm_datetime = sqlite3_column_int(stmt, index++);
		ad->tick = sqlite3_column_int(stmt, index++);
		ad->unit = sqlite3_column_int(stmt, index++);
		ad->alarm_id = sqlite3_column_int(stmt, index++);

		*list = g_list_append(*list, ad);
		(*count)++;
	}
	sqlite3_finalize(stmt);
	return CALENDAR_ERROR_NONE;
}

static int __cal_server_alarm_get_next_list_allday_todo(int from_datetime, int to_datetime, GList **list, int *count)
{
	int index;
    char query[CAL_DB_SQL_MAX_LEN] = {0};
    sqlite3_stmt *stmt = NULL;

	DBG("searching allday todo(%d) ~ (%d)", from_datetime, to_datetime);

	snprintf(query, sizeof(query),
			"SELECT A.event_id, A.alarm_time, "
			"A.remind_tick, A.remind_tick_unit, A.alarm_id "
			"FROM %s as A, "                // A is alarm
			"%s as B ON A.event_id = B.id " // B is schedule
			"WHERE B.has_alarm == 1 AND B.type = %d "
			"AND A.remind_tick_unit = %d AND A.alarm_time = %d "
			"UNION "
			"SELECT A.event_id, B.dtend_datetime, "
			"A.remind_tick, A.remind_tick_unit, A.alarm_id "
			"FROM %s as A, "                // A is alarm
			"%s as B ON A.event_id = B.id " // B is schedule
			"WHERE B.has_alarm = 1 AND B.type = %d "
			"AND A.remind_tick_unit > %d "
			"AND B.dtend_datetime - (A.remind_tick * A.remind_tick_unit)/86400 > %d "
			"AND B.dtend_datetime - (A.remind_tick * A.remind_tick_unit)/86400 <= %d ",
			CAL_TABLE_ALARM, CAL_TABLE_SCHEDULE,
			CALENDAR_BOOK_TYPE_TODO,
			CALENDAR_ALARM_TIME_UNIT_SPECIFIC, from_datetime,
			CAL_TABLE_ALARM, CAL_TABLE_SCHEDULE,
			CALENDAR_BOOK_TYPE_TODO,
			CALENDAR_ALARM_TIME_UNIT_SPECIFIC,
			from_datetime, to_datetime);

	stmt = _cal_db_util_query_prepare(query);
	if (NULL == stmt)
	{
		DBG("query[%s]", query);
		ERR("_cal_db_util_query_prepare() Failed");
		return CALENDAR_ERROR_DB_FAILED;
	}

	*count = 0;
	while (CAL_DB_ROW  == _cal_db_util_stmt_step(stmt))
	{
		struct _allday_data_s *ad = calloc(1, sizeof(struct _allday_data_s));

		index = 0;
		ad->id = sqlite3_column_int(stmt, index++);
		ad->alarm_datetime = sqlite3_column_int(stmt, index++);
		ad->tick = sqlite3_column_int(stmt, index++);
		ad->unit = sqlite3_column_int(stmt, index++);
		ad->alarm_id = sqlite3_column_int(stmt, index++);

		*list = g_list_append(*list, ad);
		(*count)++;
	}
	sqlite3_finalize(stmt);
	return CALENDAR_ERROR_NONE;
}

static int __cal_server_alarm_update_alarm_id(int alarm_id, int event_id, int tick, int unit)
{
    char query[CAL_DB_SQL_MAX_LEN] = {0};
    cal_db_util_error_e dbret = CAL_DB_OK;

	DBG("Update alarm_id(%d) in alarm table", alarm_id);
	snprintf(query, sizeof(query), "UPDATE %s SET "
			"alarm_id = %d "
			"WHERE event_id = %d AND remind_tick = %d AND remind_tick_unit = %d",
			CAL_TABLE_ALARM,
			alarm_id,
			event_id, tick, unit);

	dbret = _cal_db_util_query_exec(query);
	if (CAL_DB_OK != dbret)
	{
		ERR("_cal_db_util_query_exec() Failed(%d)", dbret);
		switch (dbret)
		{
		case CAL_DB_ERROR_NO_SPACE:
			return CALENDAR_ERROR_FILE_NO_SPACE;
		default:
			return CALENDAR_ERROR_DB_FAILED;
		}
	}

	return CALENDAR_ERROR_NONE;
}

static GFunc __cal_server_alarm_print_list_normal(gpointer data, gpointer user_data)
{
	struct _normal_data_s *normal = (struct _normal_data_s *)data;
	DBG("%02d:%lld", normal->id, normal->alarm_utime);
	return 0;
}

static GFunc __cal_server_alarm_print_list_allday_todo(gpointer data, gpointer user_data)
{
	struct _allday_data_s *allday = (struct _allday_data_s *)data;
	DBG("%02d:%d", allday->id, allday->alarm_datetime);
	return 0;
}

static int __cal_server_alarm_get_list_with_alarmmgr_id(int alarm_id, GList **normal_list, GList **allday_list)
{
	int index;
    char query[CAL_DB_SQL_MAX_LEN] = {0};
    sqlite3_stmt *stmt = NULL;
	struct _normal_data_s *nd;
	struct _allday_data_s *ad;

	snprintf(query, sizeof(query),
			"SELECT A.type, A.dtstart_type, A.dtend_type, B.remind_tick, B.remind_tick_unit "
			"FROM %s as A, "                // schedule
			"%s as B ON A.id = B.event_id " // alarm
			"WHERE B.alarm_id = %d",
			CAL_TABLE_SCHEDULE,
			CAL_TABLE_ALARM,
			alarm_id);

	stmt = _cal_db_util_query_prepare(query);
	if (NULL == stmt)
	{
		DBG("query[%s]", query);
		ERR("_cal_db_util_query_prepare() Failed");
		return CALENDAR_ERROR_DB_FAILED;
	}

	int type = 0;
	int dtstart_type = 0;
	int dtend_type = 0;
	int tick = 0;
	int unit = 0;
	if (CAL_DB_ROW  == _cal_db_util_stmt_step(stmt))
	{
		index = 0;
		type = sqlite3_column_int(stmt, index++);
		dtstart_type = sqlite3_column_int(stmt, index++);
		dtend_type = sqlite3_column_int(stmt, index++);
		tick = sqlite3_column_int(stmt, index++);
		unit = sqlite3_column_int(stmt, index++);
	}
	else
	{
		ERR("No record");
	}
	sqlite3_finalize(stmt);
	stmt = NULL;

	if ((type == CALENDAR_BOOK_TYPE_EVENT && dtstart_type == CALENDAR_TIME_UTIME)
			|| (type == CALENDAR_BOOK_TYPE_TODO && dtend_type == CALENDAR_TIME_UTIME))
	{
		long long int now_utime;
		now_utime = _cal_time_get_now();
		now_utime -= now_utime % 60;

		snprintf(query, sizeof(query),
				"SELECT A.event_id, A.dtstart_utime, "
				"B.remind_tick, B.remind_tick_unit, "
				"C.type "
				"FROM %s as A, "                       // A is normal instance
				"%s as B ON A.event_id = B.event_id, " // B is alarm
				"%s as C ON B.event_id = C.id "        // c is schedule
				"WHERE C.has_alarm == 1 AND C.type = %d "
				"AND B.remind_tick_unit = %d "
				"AND B.alarm_time = %lld "
				"UNION "
				"SELECT A.event_id, A.dtstart_utime, "
				"B.remind_tick, B.remind_tick_unit, "
				"C.type "
				"FROM %s as A, "                       // A is normal instance
				"%s as B ON A.event_id = B.event_id, " // B is alarm
				"%s as C ON B.event_id = C.id "        // c is schedule
				"WHERE C.has_alarm = 1 AND C.type = %d "
				"AND B.remind_tick_unit > %d "
				"AND (A.dtstart_utime - (B.remind_tick * B.remind_tick_unit)) >= %lld "
				"AND (A.dtstart_utime - (B.remind_tick * B.remind_tick_unit)) < %lld "
				"UNION "
				"SELECT B.id, B.dtend_utime, "
				"A.remind_tick, A.remind_tick_unit, "
				"B.type "
				"FROM %s as A, "                          // A is alarm
				"%s as B ON A.event_id = B.id "           // B is schedule
				"WHERE B.has_alarm == 1 AND B.type = %d "
				"AND A.remind_tick_unit = %d "
				"AND A.alarm_time = %lld "
				"UNION "
				"SELECT B.id, B.dtend_utime, "
				"A.remind_tick, A.remind_tick_unit, "
				"B.type "
				"FROM %s as A, "                          // A is alarm
				"%s as B ON A.event_id = B.id "           // B is schedule
				"WHERE B.has_alarm = 1 AND B.type = %d "
				"AND A.remind_tick_unit > %d "
				"AND (B.dtend_utime - (A.remind_tick * A.remind_tick_unit)) >= %lld "
				"AND (B.dtend_utime - (A.remind_tick * A.remind_tick_unit)) < %lld ",
			CAL_TABLE_NORMAL_INSTANCE, CAL_TABLE_ALARM, CAL_TABLE_SCHEDULE,
			CALENDAR_BOOK_TYPE_EVENT,
			CALENDAR_ALARM_TIME_UNIT_SPECIFIC,
			now_utime,
			CAL_TABLE_NORMAL_INSTANCE, CAL_TABLE_ALARM, CAL_TABLE_SCHEDULE,
			CALENDAR_BOOK_TYPE_EVENT,
			CALENDAR_ALARM_TIME_UNIT_SPECIFIC,
			now_utime, now_utime + 60,
			CAL_TABLE_ALARM, CAL_TABLE_SCHEDULE,
			CALENDAR_BOOK_TYPE_TODO,
			CALENDAR_ALARM_TIME_UNIT_SPECIFIC,
			now_utime,
			CAL_TABLE_ALARM, CAL_TABLE_SCHEDULE,
			CALENDAR_BOOK_TYPE_TODO,
			CALENDAR_ALARM_TIME_UNIT_SPECIFIC,
			now_utime, now_utime + 60);

		stmt = _cal_db_util_query_prepare(query);
		if (NULL == stmt)
		{
			DBG("query[%s]", query);
			ERR("_cal_db_util_query_prepare() Failed");
			return CALENDAR_ERROR_DB_FAILED;
		}

		while (CAL_DB_ROW == _cal_db_util_stmt_step(stmt))
		{
			index = 0;
			nd = calloc(1, sizeof(struct _normal_data_s));
			nd->id = sqlite3_column_int(stmt, index++);
			nd->alarm_utime = sqlite3_column_int64(stmt, index++); // dtstart, dtend
			nd->tick = sqlite3_column_int(stmt, index++);
			nd->unit = sqlite3_column_int(stmt, index++);
			nd->record_type = sqlite3_column_int(stmt, index++); // event, todo
			*normal_list = g_list_append(*normal_list, nd);
			DBG("(%d)", nd->id);
		}
	}
	else
	{
		int now_datetime;
		time_t now_timet = time(NULL);
		struct tm start_tm = {0};
		now_timet -= now_timet % 60;
		gmtime_r(&now_timet, &start_tm);
		now_datetime = (start_tm.tm_year + 1900) * 10000
			+ (start_tm.tm_mon + 1) * 100
			+ (start_tm.tm_mday);

		snprintf(query, sizeof(query),
				"SELECT A.event_id, A.dtstart_datetime, "
				"B.remind_tick, B.remind_tick_unit, "
				"C.type "
				"FROM %s as A, "                       // A is allday instance
				"%s as B ON A.event_id = B.event_id, " // B is alarm
				"%s as C ON B.event_id = C.id "        // C is schedule
				"WHERE C.has_alarm == 1 AND C.type = %d "
				"AND B.remind_tick_unit = %d "
				"AND B.alarm_time = %d "
				"UNION "
				"SELECT A.event_id, A.dtstart_datetime, "
				"B.remind_tick, B.remind_tick_unit, "
				"C.type "
				"FROM %s as A, "                       // A is allday instance
				"%s as B ON A.event_id = B.event_id, " // B is alarm
				"%s as C ON B.event_id = C.id "        // C is schedule
				"WHERE C.has_alarm = 1 AND C.type = %d "
				"AND B.remind_tick_unit > %d "
				"AND A.dtstart_datetime - (B.remind_tick * B.remind_tick_unit)/86400 = %d "
				"UNION "
				"SELECT A.event_id, B.dtend_datetime, "
				"A.remind_tick, A.remind_tick_unit, "
				"B.type "
				"FROM %s as A, "                // A is alarm
				"%s as B ON A.event_id = B.id " // B is schedule
				"WHERE B.has_alarm == 1 AND B.type = %d "
				"AND A.remind_tick_unit = %d AND A.alarm_time = %d "
				"UNION "
				"SELECT A.event_id, B.dtend_datetime, "
				"A.remind_tick, A.remind_tick_unit, "
				"B.type "
				"FROM %s as A, "                // A is alarm
				"%s as B ON A.event_id = B.id " // B is schedule
				"WHERE B.has_alarm = 1 AND B.type = %d "
				"AND A.remind_tick_unit > %d "
				"AND B.dtend_datetime - (A.remind_tick * A.remind_tick_unit)/86400 = %d ",
			CAL_TABLE_ALLDAY_INSTANCE, CAL_TABLE_ALARM, CAL_TABLE_SCHEDULE,
			CALENDAR_BOOK_TYPE_EVENT,
			CALENDAR_ALARM_TIME_UNIT_SPECIFIC,
			now_datetime,
			CAL_TABLE_ALLDAY_INSTANCE, CAL_TABLE_ALARM, CAL_TABLE_SCHEDULE,
			CALENDAR_BOOK_TYPE_EVENT,
			CALENDAR_ALARM_TIME_UNIT_SPECIFIC,
			now_datetime,
			CAL_TABLE_ALARM, CAL_TABLE_SCHEDULE,
			CALENDAR_BOOK_TYPE_TODO,
			CALENDAR_ALARM_TIME_UNIT_SPECIFIC,
			now_datetime,
			CAL_TABLE_ALARM, CAL_TABLE_SCHEDULE,
			CALENDAR_BOOK_TYPE_TODO,
			CALENDAR_ALARM_TIME_UNIT_SPECIFIC,
			now_datetime);

		stmt = _cal_db_util_query_prepare(query);
		if (NULL == stmt)
		{
			DBG("query[%s]", query);
			ERR("_cal_db_util_query_prepare() Failed");
			return CALENDAR_ERROR_DB_FAILED;
		}

		const unsigned char *temp;
		while (CAL_DB_ROW == _cal_db_util_stmt_step(stmt))
		{
			index = 0;
			ad = calloc(1, sizeof(struct _allday_data_s));
			ad->id = sqlite3_column_int(stmt, index++);
			temp = sqlite3_column_text(stmt, index++); // dtstart, dtend
			ad->alarm_datetime = atoi((const char *)temp);
			ad->tick = sqlite3_column_int(stmt, index++);
			ad->unit = sqlite3_column_int(stmt, index++);
			ad->record_type = sqlite3_column_int(stmt, index++);
			*allday_list = g_list_append(*allday_list, ad);
		}
	}
	sqlite3_finalize(stmt);
	return CALENDAR_ERROR_NONE;
}

// this api is necessary for repeat instance.
static int __cal_server_alarm_unset_alerted_alarmmgr_id(int alarm_id)
{
    char query[CAL_DB_SQL_MAX_LEN] = {0};
    cal_db_util_error_e dbret = CAL_DB_OK;

	DBG("alarm_id(%d)", alarm_id);

	snprintf(query, sizeof(query),
			"UPDATE %s SET alarm_id = 0 WHERE alarm_id = %d ",
			CAL_TABLE_ALARM, alarm_id);

	dbret = _cal_db_util_query_exec(query);
	if (CAL_DB_OK != dbret)
	{
		ERR("_cal_db_util_query_exec() Failed(%d)", dbret);
		switch (dbret)
		{
		case CAL_DB_ERROR_NO_SPACE:
			return CALENDAR_ERROR_FILE_NO_SPACE;
		default:
			return CALENDAR_ERROR_DB_FAILED;
		}
	}

	return CALENDAR_ERROR_NONE;
}

static int __cal_server_alarm_alert_with_pkgname(const char *pkgname, char *id, char *time, char *tick, char *unit, char *type)
{
	int ret = CALENDAR_ERROR_NONE;
	int index;
    char query[CAL_DB_SQL_MAX_LEN] = {0};
	const char *key;
	const char *value;
    sqlite3_stmt *stmt = NULL;
    cal_db_util_error_e dbret = CAL_DB_OK;

	if (NULL == pkgname)
	{
		ERR("Invalid parameter: pkgname is NULL");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	// get user key, value
	snprintf(query, sizeof(query), "SELECT key, value FROM %s WHERE pkgname = '%s' ",
			CAL_REMINDER_ALERT, pkgname);
    stmt = _cal_db_util_query_prepare(query);
    if (NULL == stmt)
    {
        ERR("_cal_db_util_query_prepare() Failed");
        return CALENDAR_ERROR_DB_FAILED;
    }

    while(CAL_DB_ROW == _cal_db_util_stmt_step(stmt))
    {
		index = 0;
		key = (const char *)sqlite3_column_text(stmt, index++);
		value = (const char *)sqlite3_column_text(stmt, index++);
	}

	// run appsvc
	DBG("[%s][%s][%s][%s]", id, time, tick, unit);

	bundle *b;
	b = bundle_create();
	appsvc_set_pkgname(b, pkgname);
	appsvc_set_operation(b, APPSVC_OPERATION_DEFAULT);
	if (key && value) appsvc_add_data(b, key, value);
	appsvc_add_data(b, "id", id);
	appsvc_add_data(b, "time", time);
	appsvc_add_data(b, "tick", tick);
	appsvc_add_data(b, "unit", unit);
	appsvc_add_data(b, "type", type);
	ret = appsvc_run_service(b, 0, NULL, NULL);
	bundle_free(b);

	sqlite3_finalize(stmt);

	// if no app, delete from the table
	if (APPSVC_RET_ELAUNCH == ret)
	{
		DBG("Deleted no responce pkg[%s]", pkgname);
		snprintf(query, sizeof(query), "DELETE FROM %s WHERE pkgname = '%s' ",
				CAL_REMINDER_ALERT, pkgname);
		dbret = _cal_db_util_query_exec(query);
		if (CAL_DB_OK != dbret)
		{
			ERR("_cal_db_util_query_exec() failed(ret:%d)", ret);
			switch (dbret)
			{
			case CAL_DB_ERROR_NO_SPACE:
				return CALENDAR_ERROR_FILE_NO_SPACE;
			default:
				return CALENDAR_ERROR_DB_FAILED;
			}
		}

	}
	return CALENDAR_ERROR_NONE;
}

static int __cal_server_alarm_alert(char *id, char *time, char *tick, char *unit, char *type)
{
	int index;
    char query[CAL_DB_SQL_MAX_LEN] = {0};
    sqlite3_stmt *stmt = NULL;

	snprintf(query, sizeof(query), "SELECT pkgname FROM %s WHERE onoff = 1 ",
			CAL_REMINDER_ALERT);
    stmt = _cal_db_util_query_prepare(query);
    if (NULL == stmt)
    {
        ERR("_cal_db_util_query_prepare() Failed");
        return CALENDAR_ERROR_DB_FAILED;
    }

    while(CAL_DB_ROW == _cal_db_util_stmt_step(stmt))
    {
		index = 0;
		const char *pkgname = (const char *)sqlite3_column_text(stmt, index++);
		DBG("pkgname[%s]", pkgname);
		__cal_server_alarm_alert_with_pkgname(pkgname, id, time, tick, unit, type);
	}
	sqlite3_finalize(stmt);

	return CALENDAR_ERROR_NONE;
}

static int __cal_server_alarm_noti_with_appsvc(int alarm_id, GList *normal_list, GList *allday_list)
{
	char buf_id[128] = {0};
	char buf_time[128] = {0};
	char buf_tick[128] = {0};
	char buf_unit[128] = {0};
	char buf_type[128] = {0};
	GList *l = NULL;

	l = g_list_first(normal_list);
	if (NULL == l) DBG("normal list is NULL");
	while (l)
	{
		struct _normal_data_s *nd = (struct _normal_data_s *)l->data;
		snprintf(buf_id, sizeof(buf_id), "%d", nd->id);
		snprintf(buf_time, sizeof(buf_time), "%lld", nd->alarm_utime);
		snprintf(buf_tick, sizeof(buf_tick), "%d", nd->tick);
		snprintf(buf_unit, sizeof(buf_unit), "%d", nd->unit);
		snprintf(buf_type, sizeof(buf_type), "%d", nd->record_type);

		__cal_server_alarm_alert(buf_id, buf_time, buf_tick, buf_unit, buf_type);
		l = g_list_next(l);
	}

	l = NULL;
	l = g_list_first(allday_list);
	if (NULL == l) DBG("allday list is NULL");
	while (l)
	{
		struct _allday_data_s *ad = (struct _allday_data_s *)l->data;
		snprintf(buf_id, sizeof(buf_id), "%d",  ad->id);
		snprintf(buf_time, sizeof(buf_time), "%d", ad->alarm_datetime);
		snprintf(buf_tick, sizeof(buf_tick), "%d", ad->tick);
		snprintf(buf_unit, sizeof(buf_unit), "%d", ad->unit);
		snprintf(buf_type, sizeof(buf_type), "%d", ad->record_type);

		__cal_server_alarm_alert(buf_id, buf_time, buf_tick, buf_unit, buf_type);
		l = g_list_next(l);
	}
	return CALENDAR_ERROR_NONE;
}

static int __cal_server_alarm_register_next_normal(long long int now_utime, GList *normal_list)
{
	int ret;
	int alarm_id;
	struct _normal_data_s *nd;

	GList *l = g_list_first(normal_list);

	// try to save the first record.
	// if alarm id exists, it means updated record is not earilier than already registerd one.
	nd = (struct _normal_data_s *)l->data;
	if (NULL == nd)
	{
		ERR("No data");
		return CALENDAR_ERROR_DB_FAILED;
	}

	if (nd->alarm_id > 0)
	{
		DBG("Already registered this id");
		return CALENDAR_ERROR_NONE;
	}

	DBG("new is earilier than registered.");

	// clear all alarm which set by mine.
	ret = alarmmgr_enum_alarm_ids(__cal_server_alarm_clear_all_cb, NULL);
	if (ret != ALARMMGR_RESULT_SUCCESS)
	{
		ERR("alarmmgr_enum_alarm_ids() failed");
		return ret;
	}

	// register new list
	long long int mod_alarm_utime; // del seconds
	mod_alarm_utime = nd->alarm_utime - (nd->alarm_utime % 60);
	ret = alarmmgr_add_alarm(ALARM_TYPE_VOLATILE,
			(time_t)(mod_alarm_utime - now_utime),
			0, NULL, &alarm_id);
	if (ret < 0)
	{
		ERR("alarmmgr_add_alarm_appsvc failed (%d)", ret);
		return ret;
	}
	DBG("Get normal alarmmgr id (%d)", alarm_id);
	__cal_server_alarm_update_alarm_id(alarm_id, nd->id, nd->tick, nd->unit);

	return CALENDAR_ERROR_NONE;
}

static int __cal_server_alarm_register_next_allday(time_t now_timet, GList *allday_list)
{
	int ret;
	int alarm_id;
	struct _allday_data_s *ad;
	time_t alarm_timet;

	GList *l = g_list_first(allday_list);
	alarm_id = 0;
	ad = (struct _allday_data_s *)l->data;
	if (NULL == ad)
	{
		ERR("No data");
		return CALENDAR_ERROR_DB_FAILED;
	}

	if (ad->alarm_id > 0)
	{
		DBG("Already registered this id");
		return CALENDAR_ERROR_NONE;
	}

	DBG("new is earilier than registered.");

	// clear all alarm which set by mine.
	ret = alarmmgr_enum_alarm_ids(__cal_server_alarm_clear_all_cb, NULL);
	if (ret != ALARMMGR_RESULT_SUCCESS)
	{
		ERR("alarmmgr_enum_alarm_ids() failed");
		return ret;
	}

	// register new list
	ret = alarmmgr_add_alarm(ALARM_TYPE_VOLATILE,
			(time_t)(alarm_timet - (ad->tick * ad->unit) - now_timet),
			0, NULL, &alarm_id);
	if (ret < 0)
	{
		ERR("alarmmgr_add_alarm_appsvc failed (%d)", ret);
		return ret;
	}
	DBG("Get allday alarmmgr id (%d)", alarm_id);
	__cal_server_alarm_update_alarm_id(alarm_id, ad->id, ad->tick, ad->unit);

	return CALENDAR_ERROR_NONE;
}

static gint __cal_server_alarm_normal_sort_cb(gconstpointer a, gconstpointer b)
{
	struct _normal_data_s *p1 = (struct _normal_data_s *)a;
	struct _normal_data_s *p2 = (struct _normal_data_s *)b;

	return p1->alarm_utime - p2->alarm_utime > 0 ? 1 : -1;
}

static gint __cal_server_alarm_allday_sort_cb(gconstpointer a, gconstpointer b)
{
	struct _allday_data_s *p1 = (struct _allday_data_s *)a;
	struct _allday_data_s *p2 = (struct _allday_data_s *)b;

	return p1->alarm_datetime - p2->alarm_datetime > 0 ? 1 : -1;
}

static int __cal_server_alarm_register_with_alarmmgr(void)
{
	GList *normal_list = NULL;
	GList *allday_list = NULL;
	int event_count;
	int todo_count;
	int loop_count;

	gettimeofday(&stv, NULL); // check time

	// for normal
	long long int now_utime;
	long long int from_utime, to_utime;
	now_utime = _cal_time_get_now();

	event_count = 0;
	todo_count = 0;
	to_utime = now_utime - (now_utime %  60) + 60;

	// searching 3months, next 6months, next 12months
	for (loop_count = 1; loop_count < 4; loop_count++)
	{
		from_utime = to_utime;
		to_utime = from_utime + (60 * 60 * 24 * 30 * 3 * loop_count);
		__cal_server_alarm_get_next_list_normal_event(from_utime, to_utime, &normal_list, &event_count);
		__cal_server_alarm_get_next_list_normal_todo(from_utime, to_utime, &normal_list, &todo_count);
		if (event_count + todo_count > 0)
		{
			break;
		}
	}
	DBG("event count(%d) todo count(%d)", event_count, todo_count);

	if (event_count + todo_count > 0)
	{
		normal_list = g_list_sort(normal_list, __cal_server_alarm_normal_sort_cb);
		DBG("after sorting");
		g_list_foreach(normal_list, (GFunc)__cal_server_alarm_print_list_normal, NULL);
		__cal_server_alarm_register_next_normal(now_utime, normal_list);

	}
	if (normal_list)
	{
		g_list_free_full(normal_list, free);
		normal_list = NULL;
	}

	// for allday
	DBG("allday");
	time_t now_timet;
	time_t from_timet, to_timet;
	int from_datetime, to_datetime;
	struct tm datetime_tm;
	now_timet = time(NULL);

	event_count = 0;
	todo_count = 0;
	to_timet = now_timet;

	// searching 3months, next 6months, next 12months
	for (loop_count = 1; loop_count < 4; loop_count++)
	{
		from_timet = to_timet;
		gmtime_r(&from_timet, &datetime_tm);
		from_datetime = (1900 + datetime_tm.tm_year) * 10000 + (datetime_tm.tm_mon + 1) * 100 + datetime_tm.tm_mday;

		to_timet = from_timet + (60 * 60 * 24 * 30 * 3 * loop_count);
		gmtime_r(&to_timet, &datetime_tm);
		to_datetime = (1900 + datetime_tm.tm_year) * 10000 + (datetime_tm.tm_mon + 1) * 100 + datetime_tm.tm_mday;

		__cal_server_alarm_get_next_list_allday_event(from_datetime, to_datetime, &allday_list, &event_count);
		__cal_server_alarm_get_next_list_allday_todo(from_datetime, to_datetime, &allday_list, &todo_count);

		if (event_count + todo_count > 0)
		{
			break;
		}
	}
	DBG("event count(%d) todo count(%d)", event_count, todo_count);

	if (event_count + todo_count > 0)
	{
		allday_list = g_list_sort(allday_list, __cal_server_alarm_allday_sort_cb);
		DBG("after sorting");
		g_list_foreach(allday_list, (GFunc)__cal_server_alarm_print_list_allday_todo, NULL);
		__cal_server_alarm_register_next_allday(now_timet, allday_list);
	}
	if (allday_list)
	{
		g_list_free_full(allday_list, free);
		allday_list = NULL;
	}

	int diff;
	gettimeofday(&etv, NULL);
	diff = ((int)etv.tv_sec *1000 + (int)etv.tv_usec/1000)
		-((int)stv.tv_sec *1000 + (int)stv.tv_usec/1000);
	DBG("registering time diff %ld(%d.%d)",diff, diff/1000, diff%1000); // time check
	return 0;
}

static int __cal_server_alarm_callback(char *id, char *time, char *tick, char *unit, char *type)
{
	_cal_server_reminder_add_callback_data("id", id);
	_cal_server_reminder_add_callback_data("time", time);
	_cal_server_reminder_add_callback_data("tick", tick);
	_cal_server_reminder_add_callback_data("unit", unit);
	_cal_server_reminder_add_callback_data("type", type);

	return CALENDAR_ERROR_NONE;
}

static int __cal_server_alarm_noti_callback(int alarm_id, GList *normal_list, GList *allday_list)
{
	char buf_id[128] = {0};
	char buf_time[128] = {0};
	char buf_tick[128] = {0};
	char buf_unit[128] = {0};
	char buf_type[128] = {0};
	GList *l = NULL;

	l = g_list_first(normal_list);
	if (NULL == l) DBG("normal list is NULL");
	while (l)
	{
		struct _normal_data_s *nd = (struct _normal_data_s *)l->data;
		snprintf(buf_id, sizeof(buf_id), "%d", nd->id);
		snprintf(buf_time, sizeof(buf_time), "%lld", nd->alarm_utime);
		snprintf(buf_tick, sizeof(buf_tick), "%d", nd->tick);
		snprintf(buf_unit, sizeof(buf_unit), "%d", nd->unit);
		snprintf(buf_type, sizeof(buf_type), "%d", nd->record_type);

		__cal_server_alarm_callback(buf_id, buf_time, buf_tick, buf_unit, buf_type);
		_cal_server_reminder_publish();
		l = g_list_next(l);
	}

	l = NULL;
	l = g_list_first(allday_list);
	if (NULL == l) DBG("allday list is NULL");
	while (l)
	{
		struct _allday_data_s *ad = (struct _allday_data_s *)l->data;
		snprintf(buf_id, sizeof(buf_id), "%d",  ad->id);
		snprintf(buf_time, sizeof(buf_time), "%d", ad->alarm_datetime);
		snprintf(buf_tick, sizeof(buf_tick), "%d", ad->tick);
		snprintf(buf_unit, sizeof(buf_unit), "%d", ad->unit);
		snprintf(buf_type, sizeof(buf_type), "%d", ad->record_type);

		__cal_server_alarm_callback(buf_id, buf_time, buf_tick, buf_unit, buf_type);
		_cal_server_reminder_publish();
		l = g_list_next(l);
	}
	return 0;
}

static int _alert_cb(alarm_id_t alarm_id, void *data)
{
	GList *normal_list = NULL;
	GList *allday_list = NULL;

	__cal_server_alarm_get_list_with_alarmmgr_id(alarm_id, &normal_list, &allday_list);
	__cal_server_alarm_unset_alerted_alarmmgr_id(alarm_id);
	__cal_server_alarm_noti_with_appsvc(alarm_id, normal_list, allday_list);
	__cal_server_alarm_noti_callback(alarm_id, normal_list, allday_list);

	if (normal_list)
	{
		g_list_free_full(normal_list, free);
		normal_list = NULL;
	}
	if (allday_list)
	{
		g_list_free_full(allday_list, free);
		allday_list = NULL;
	}

	__cal_server_alarm_register_with_alarmmgr();
	return 0;
}

////////////////////////////////////////////////////////////////////
static void __cal_server_alarm_timechange_cb(keynode_t *node, void *data)
{
	__cal_server_alarm_register_with_alarmmgr();
}

int __cal_server_alarm_set_timechange(void)
{
	int ret;

	ret = vconf_notify_key_changed(VCONFKEY_SYSTEM_TIMECHANGE,
			__cal_server_alarm_timechange_cb, NULL);
	if (ret < 0)
	{
		ERR("vconf_ignore_key_changed() failed");
		return ret;
	}
	return CALENDAR_ERROR_NONE;
}

static void __changed_cb(const char* view_uri, void* data)
{
	__cal_server_alarm_register_with_alarmmgr();
}

static int __cal_server_alarm_set_inotify(calendar_db_changed_cb callback)
{
	_cal_inotify_subscribe(CAL_NOTI_TYPE_EVENT, CAL_NOTI_EVENT_CHANGED, callback, NULL);
	_cal_inotify_subscribe(CAL_NOTI_TYPE_TODO, CAL_NOTI_TODO_CHANGED, callback, NULL);
	return 0;
}

int _cal_server_alarm(void)
{
	int ret;

	__cal_server_alarm_set_timechange();
	__cal_server_alarm_set_inotify(__changed_cb);

	ret = alarmmgr_init("calendar-service");
	retvm_if(ret < 0, ret, "alarmmgr_init() failed");

	ret = alarmmgr_set_cb(_alert_cb, NULL);
	retvm_if(ret < 0, ret, "alarmmgr_set_cb() failed");

	__cal_server_alarm_register_with_alarmmgr();

	return CALENDAR_ERROR_NONE;
}

