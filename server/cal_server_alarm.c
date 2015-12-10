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

#include <sys/time.h>
#include <unistd.h>
#include <alarm.h>
#include <vconf.h>
#include <app.h>

#include "calendar.h"
#include "cal_typedef.h"
#include "cal_internal.h"
#include "cal_time.h"
#include "cal_inotify.h"
#include "cal_db_util.h"
#include "cal_db.h"
#include "cal_db_query.h"
#include "cal_server_dbus.h"

#define CAL_SEARCH_LOOP_MAX 4

struct _alarm_data_s {
	int event_id;
	long long int alert_utime; /* to compare */
	int unit;
	int tick;
	int type; /* utime, localtime */
	long long int time;
	int record; /* todo, event */
	char datetime[CAL_STR_SHORT_LEN32];
	int system_type;
};

/* this api is necessary for repeat instance. */
static int _cal_server_alarm_unset_alerted_alarmmgr_id(int alarm_id)
{
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	int ret = 0;

	ret = cal_db_util_begin_trans();
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_begin_trans() Fail");
		return CALENDAR_ERROR_DB_FAILED;
	}

	DBG("alarm_id(%d)", alarm_id);

	snprintf(query, sizeof(query), "UPDATE %s SET alarm_id = 0 WHERE alarm_id =%d ",
			CAL_TABLE_ALARM, alarm_id);
	ret = cal_db_util_query_exec(query);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_query_exec() Fail(%d)", ret);
		SECURE("[%s]", query);
		cal_db_util_end_trans(false);
		return ret;
	}
	cal_db_util_end_trans(true);
	return CALENDAR_ERROR_NONE;
}

static int _cal_server_alarm_clear_all_cb(alarm_id_t alarm_id, void *data)
{
	int ret;

	DBG("remove alarm id(%d)", alarm_id);
	_cal_server_alarm_unset_alerted_alarmmgr_id(alarm_id);
	ret = alarmmgr_remove_alarm(alarm_id);
	if (ret != ALARMMGR_RESULT_SUCCESS) {
		ERR("alarmmgr_remove_alarm() Fail(ret:%d)", ret);
		return ret;
	}
	return CALENDAR_ERROR_NONE;
}

static int _cal_server_alarm_update_alarm_id(int alarm_id, int event_id, int tick, int unit)
{
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	int ret = 0;

	ret = cal_db_util_begin_trans();
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_begin_trans() Fail");
		return CALENDAR_ERROR_DB_FAILED;
	}

	DBG("Update alarm_id(%d) in alarm table", alarm_id);
	snprintf(query, sizeof(query), "UPDATE %s SET alarm_id =%d "
			"WHERE event_id =%d AND remind_tick =%d AND remind_tick_unit =%d",
			CAL_TABLE_ALARM, alarm_id, event_id, tick, unit);
	ret = cal_db_util_query_exec(query);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_query_exec() Fail(%d)", ret);
		SECURE("[%s]", query);
		cal_db_util_end_trans(false);
		return ret;
	}
	cal_db_util_end_trans(true);
	return CALENDAR_ERROR_NONE;
}

static long long int _cal_server_alarm_get_alert_utime(const char *field, int event_id, time_t current)
{
	int ret = 0;
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	snprintf(query, sizeof(query), "SELECT %s FROM %s "
			"WHERE event_id=%d AND %s>%ld ORDER BY %s LIMIT 1",
			field, CAL_TABLE_NORMAL_INSTANCE, event_id, field, current, field);

	sqlite3_stmt *stmt = NULL;
	ret = cal_db_util_query_prepare(query, &stmt);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_query_prepare() Fail(%d)", ret);
		SECURE("query[%s]", query);
		return ret;
	}

	long long int utime = 0;
	if (CAL_SQLITE_ROW == cal_db_util_stmt_step(stmt))
		utime = sqlite3_column_int(stmt, 0);

	sqlite3_finalize(stmt);
	return utime;
}

static int _cal_server_alarm_get_alert_localtime(const char *field, int event_id, time_t current)
{
	int ret = 0;
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	struct tm st = {0};
	tzset();
	localtime_r(&current, &st);
	time_t mod_current = timegm(&st);
	snprintf(query, sizeof(query), "SELECT %s FROM %s "
			"WHERE event_id=%d AND strftime('%%s', %s)>%ld ORDER BY %s LIMIT 1",
			field, CAL_TABLE_ALLDAY_INSTANCE, event_id, field, mod_current, field);

	sqlite3_stmt *stmt = NULL;
	ret = cal_db_util_query_prepare(query, &stmt);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_query_prepare() Fail(%d)", ret);
		SECURE("query[%s]", query);
		return ret;
	}

	const char *datetime = NULL;
	if (CAL_SQLITE_ROW == cal_db_util_stmt_step(stmt))
		datetime = (const char *)sqlite3_column_text(stmt, 0);

	if (NULL == datetime || '\0' == *datetime) {
		ERR("Invalid datetime [%s]", datetime);
		sqlite3_finalize(stmt);
		return 0;
	}

	int y = 0, m = 0, d = 0;
	int h = 0, n = 0, s = 0;
	sscanf(datetime, CAL_FORMAT_LOCAL_DATETIME, &y, &m, &d, &h, &n, &s);
	sqlite3_finalize(stmt);

	st.tm_year = y - 1900;
	st.tm_mon = m - 1;
	st.tm_mday = d;
	st.tm_hour = h;
	st.tm_min = n;
	st.tm_sec = s;

	return (long long int)mktime(&st);
}

/*
 * to get aler time, time(NULL) is not accurate. 1 secs diff could be occured.
 * so, searching DB is neccessary to find alert time.
 */
int cal_server_alarm_get_alert_time(int alarm_id, time_t *tt_alert)
{
	int ret = 0;
	RETV_IF(NULL == tt_alert, CALENDAR_ERROR_INVALID_PARAMETER);

	char query[CAL_DB_SQL_MAX_LEN] = {0};
	snprintf(query, sizeof(query), "SELECT A.event_id, A.remind_tick_unit, A.remind_tick, "
			"A.alarm_type, A.alarm_utime, A.alarm_datetime, B.type, B.dtstart_type, "
			"B.dtend_type FROM %s as A, %s as B ON A.event_id =B.id WHERE alarm_id =%d ",
			CAL_TABLE_ALARM, CAL_TABLE_SCHEDULE, alarm_id);

	sqlite3_stmt *stmt = NULL;
	ret = cal_db_util_query_prepare(query, &stmt);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_query_prepare() Fail(%d)", ret);
		SECURE("query[%s]", query);
		return ret;
	}

	int event_id = 0;
	int unit = 0;
	int tick = 0;
	int type = 0;
	long long int utime = 0;
	const char *datetime = NULL;
	int record_type = 0;
	int dtstart_type = 0;
	int dtend_type = 0;
	struct tm st = {0};

	if (CAL_SQLITE_ROW == cal_db_util_stmt_step(stmt)) {
		event_id = sqlite3_column_int(stmt, 0);
		unit = sqlite3_column_int(stmt, 1);
		tick = sqlite3_column_int(stmt, 2);
		type = sqlite3_column_int(stmt, 3);
		utime = sqlite3_column_int64(stmt, 4);
		datetime = (const char *)sqlite3_column_text(stmt, 5);
		record_type = sqlite3_column_int(stmt, 6);
		dtstart_type = sqlite3_column_int(stmt, 7);
		dtend_type = sqlite3_column_int(stmt, 8);
	}

	if (NULL == tt_alert) {
		ERR("Invalid parameter: tt_alert is NULL");
		sqlite3_finalize(stmt);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	if (CALENDAR_ALARM_TIME_UNIT_SPECIFIC == unit) {
		if (CALENDAR_TIME_UTIME == type) {
			*tt_alert = utime;
		} else {
			int y = 0, m = 0, d = 0;
			int h = 0, n = 0, s = 0;
			sscanf(datetime, CAL_FORMAT_LOCAL_DATETIME, &y, &m, &d, &h, &n, &s);

			st.tm_year = y - 1900;
			st.tm_mon = m - 1;
			st.tm_mday = d;
			st.tm_hour = h;
			st.tm_min = n;
			st.tm_sec = s;
			*tt_alert = mktime(&st);
			DBG("datetime[%s] to %02d:%02d:%02d (%d)", datetime, h, n, s, *tt_alert);
		}
		sqlite3_finalize(stmt);
		return CALENDAR_ERROR_NONE;
	}
	sqlite3_finalize(stmt);

	time_t current = time(NULL);
	current += (tick * unit);
	current -= 2; /* in case time passed */

	switch (record_type) {
	case CALENDAR_BOOK_TYPE_EVENT:
		switch (dtstart_type) {
		case CALENDAR_TIME_UTIME:
			utime = _cal_server_alarm_get_alert_utime("dtstart_utime", event_id, current);
			break;

		case CALENDAR_TIME_LOCALTIME:
			utime = _cal_server_alarm_get_alert_localtime("dtstart_datetime", event_id, current);
			break;
		}
		break;

	case CALENDAR_BOOK_TYPE_TODO:
		switch (dtend_type) {
		case CALENDAR_TIME_UTIME:
			utime = _cal_server_alarm_get_alert_utime("dtend_utime", event_id, current);
			break;

		case CALENDAR_TIME_LOCALTIME:
			utime = _cal_server_alarm_get_alert_localtime("dtend_datetime", event_id, current);
			break;
		}
		break;
	}
	DBG("alert_time(%d) = utime(%lld) - (tick(%d) * unit(%d))", *tt_alert, utime, tick, unit);

	*tt_alert = utime - (tick * unit);
	return CALENDAR_ERROR_NONE;
}

/*
 * bool get_all is
 * true : to get all alarms including same time event.
 * (ig. if 3 diffrent alarms exist at 06:30, list has 3 data)
 * false : to get only one alarm to register in alarm-manager.
 * (ig. if 3 diffrent alarms exist at 06:30, list has only one)
 */
static void _cal_server_alarm_get_upcoming_specific_utime(time_t utime, bool get_all, GList **l)
{
	int ret = 0;
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	snprintf(query, sizeof(query), "SELECT event_id,remind_tick_unit,remind_tick,"
			"alarm_type,alarm_utime,alarm_datetime "
			"FROM %s WHERE remind_tick_unit =%d AND alarm_type =%d AND alarm_utime %s %ld %s",
			CAL_TABLE_ALARM, CALENDAR_ALARM_TIME_UNIT_SPECIFIC, CALENDAR_TIME_UTIME,
			true == get_all ? "=" : ">", utime,
			true == get_all ? "" : "ORDER BY alarm_utime ASC LIMIT 1");

	sqlite3_stmt *stmt = NULL;
	ret = cal_db_util_query_prepare(query, &stmt);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_query_prepare() Fail(%d)", ret);
		SECURE("query[%s]", query);
		return;
	}

	while (CAL_SQLITE_ROW == cal_db_util_stmt_step(stmt)) {
		struct _alarm_data_s *ad = calloc(1, sizeof(struct _alarm_data_s));
		if (NULL == ad) {
			ERR("calloc() Fail");
			sqlite3_finalize(stmt);
			return;
		}

		ad->event_id = sqlite3_column_int(stmt, 0);
		ad->unit = sqlite3_column_int(stmt, 1);
		ad->tick = sqlite3_column_int(stmt, 2);
		ad->type = sqlite3_column_int(stmt, 3);
		ad->time = (long long int)sqlite3_column_int64(stmt, 4);
		snprintf(ad->datetime, sizeof(ad->datetime), "%s", (const char *)sqlite3_column_text(stmt, 5));
		*l = g_list_append(*l, ad);
		DBG("found id(%d) unit(%d) tick(%d) type(%d) time(%lld) [%s]",
				ad->event_id, ad->unit, ad->tick, ad->type, ad->time, ad->datetime);
		ad->alert_utime = ad->time;
		if (false == get_all) break;
	}
	sqlite3_finalize(stmt);
}

static void _cal_server_alarm_get_upcoming_specific_localtime(const char *datetime, bool get_all, GList **l)
{
	int ret = 0;
	char query[CAL_DB_SQL_MAX_LEN] = {0};
snprintf(query, sizeof(query), "SELECT event_id,remind_tick_unit,remind_tick,"
			"alarm_type,alarm_utime,alarm_datetime "
			"FROM %s WHERE remind_tick_unit=%d AND alarm_type=%d AND alarm_datetime %s '%s' %s",
			CAL_TABLE_ALARM, CALENDAR_ALARM_TIME_UNIT_SPECIFIC, CALENDAR_TIME_LOCALTIME,
			true == get_all ? "=" : ">", datetime,
			true == get_all ? "" : "ORDER BY alarm_datetime ASC LIMIT 1");

	sqlite3_stmt *stmt = NULL;
	ret = cal_db_util_query_prepare(query, &stmt);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_query_prepare() Fail(%d)", ret);
		SECURE("query[%s]", query);
		return;
	}

	while (CAL_SQLITE_ROW == cal_db_util_stmt_step(stmt)) {
		struct _alarm_data_s *ad = calloc(1, sizeof(struct _alarm_data_s));
		if (NULL == ad) {
			ERR("calloc() Fail");
			sqlite3_finalize(stmt);
			return;
		}

		ad->event_id = sqlite3_column_int(stmt, 0);
		ad->unit = sqlite3_column_int(stmt, 1);
		ad->tick = sqlite3_column_int(stmt, 2);
		ad->type = sqlite3_column_int(stmt, 3);
		ad->time = (long long int)sqlite3_column_int64(stmt, 4);
		snprintf(ad->datetime, sizeof(ad->datetime), "%s", (const char *)sqlite3_column_text(stmt, 5));
		*l = g_list_append(*l, ad);
		DBG("found id(%d) unit(%d) tick(%d) type(%d) time(%lld) [%s]",
				ad->event_id, ad->unit, ad->tick, ad->type, ad->time, ad->datetime);

		int y = 0, m = 0, d = 0;
		int h = 0, n = 0, s = 0;
		sscanf(ad->datetime, CAL_FORMAT_LOCAL_DATETIME, &y, &m, &d, &h, &n, &s);

		struct tm st = {0};
		st.tm_year = y - 1900;
		st.tm_mon = m -1;
		st.tm_mday = d;
		st.tm_hour = h;
		st.tm_min = n;
		st.tm_sec = s;
		ad->alert_utime = (long long int)mktime(&st);
		if (false == get_all) break;
	}
	sqlite3_finalize(stmt);
}

static void _cal_server_alarm_get_upcoming_nonspecific_event_utime(time_t utime, bool get_all, GList **l)
{
	int ret = 0;
	/*
	 * A:alarm
	 * B:normal instance
	 */
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	snprintf(query, sizeof(query), "SELECT A.event_id,A.remind_tick_unit,A.remind_tick, "
			"A.alarm_type,B.dtstart_utime,A.alarm_datetime "
			"FROM %s as A, %s as B ON A.event_id = B.event_id "
			"WHERE A.remind_tick_unit >%d AND (B.dtstart_utime - (A.remind_tick_unit * A.remind_tick)) %s %ld %s",
			CAL_TABLE_ALARM, CAL_TABLE_NORMAL_INSTANCE, CALENDAR_ALARM_TIME_UNIT_SPECIFIC,
			true == get_all ? "=" : ">", utime,
			true == get_all ? "" : "ORDER BY (B.dtstart_utime - (A.remind_tick_unit * A.remind_tick)) LIMIT 1");

	sqlite3_stmt *stmt = NULL;
	ret = cal_db_util_query_prepare(query, &stmt);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_query_prepare() Fail(%d)", ret);
		SECURE("query[%s]", query);
		return;
	}

	while (CAL_SQLITE_ROW == cal_db_util_stmt_step(stmt)) {
		struct _alarm_data_s *ad = calloc(1, sizeof(struct _alarm_data_s));
		if (NULL == ad) {
			ERR("calloc() Fail");
			sqlite3_finalize(stmt);
			return;
		}

		ad->event_id = sqlite3_column_int(stmt, 0);
		ad->unit = sqlite3_column_int(stmt, 1);
		ad->tick = sqlite3_column_int(stmt, 2);
		ad->type = sqlite3_column_int(stmt, 3);
		ad->time = (long long int)sqlite3_column_int64(stmt, 4);
		snprintf(ad->datetime, sizeof(ad->datetime), "%s", (const char *)sqlite3_column_text(stmt, 5));
		*l = g_list_append(*l, ad);
		DBG("found id(%d) unit(%d) tick(%d) type(%d) time(%lld) [%s]",
				ad->event_id, ad->unit, ad->tick, ad->type, ad->time, ad->datetime);

		ad->alert_utime = ad->time - (ad->tick * ad->unit);
		if (false == get_all) break;
	}
	sqlite3_finalize(stmt);
}

static void _cal_server_alarm_get_upcoming_nonspecific_event_localtime(const char *datetime, bool get_all, GList **l)
{
	int ret = 0;
	/*
	 * A:alarm
	 * B:allday
	 */
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	snprintf(query, sizeof(query), "SELECT A.event_id,A.remind_tick_unit,A.remind_tick, "
			"A.alarm_type,A.alarm_utime,B.dtstart_datetime "
			"FROM %s as A, %s as B ON A.event_id = B.event_id "
			"WHERE A.remind_tick_unit >%d AND "
			"(strftime('%%s', B.dtstart_datetime) - (A.remind_tick_unit * A.remind_tick) - strftime('%%s', '%s') %s 0) %s",
			CAL_TABLE_ALARM, CAL_TABLE_ALLDAY_INSTANCE, CALENDAR_ALARM_TIME_UNIT_SPECIFIC,
			datetime, true == get_all ? "=" : ">",
			true == get_all ? "" : "ORDER BY (strftime('%s', B.dtstart_datetime) - (A.remind_tick_unit * A.remind_tick)) LIMIT 1 ");
	sqlite3_stmt *stmt = NULL;
	ret = cal_db_util_query_prepare(query, &stmt);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_query_prepare() Fail(%d)", ret);
		SECURE("query[%s]", query);
		return;
	}

	while (CAL_SQLITE_ROW == cal_db_util_stmt_step(stmt)) {
		struct _alarm_data_s *ad = calloc(1, sizeof(struct _alarm_data_s));
		if (NULL == ad) {
			ERR("calloc() Fail");
			sqlite3_finalize(stmt);
			return;
		}

		ad->event_id = sqlite3_column_int(stmt, 0);
		ad->unit = sqlite3_column_int(stmt, 1);
		ad->tick = sqlite3_column_int(stmt, 2);
		ad->type = sqlite3_column_int(stmt, 3);
		ad->time = (long long int)sqlite3_column_int64(stmt, 4);
		snprintf(ad->datetime, sizeof(ad->datetime), "%s", (const char *)sqlite3_column_text(stmt, 5));
		*l = g_list_append(*l, ad);
		DBG("found id(%d) unit(%d) tick(%d) type(%d) time(%lld) [%s]",
				ad->event_id, ad->unit, ad->tick, ad->type, ad->time, ad->datetime);

		int y = 0, m = 0, d = 0;
		int h = 0, n = 0, s = 0;
		sscanf(ad->datetime, CAL_FORMAT_LOCAL_DATETIME, &y, &m, &d, &h, &n, &s);

		struct tm st = {0};
		st.tm_year = y - 1900;
		st.tm_mon = m -1;
		st.tm_mday = d;
		st.tm_hour = h;
		st.tm_min = n;
		st.tm_sec = s;
		ad->alert_utime = (long long int)mktime(&st) - (ad->tick * ad->unit);
		if (false == get_all) break;
	}
	sqlite3_finalize(stmt);
}

static void _cal_server_alarm_get_upcoming_nonspecific_todo_utime(time_t utime, bool get_all, GList **l)
{
	int ret = 0;
	/*
	 * A:alarm
	 * B:todo(normal)
	 */
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	snprintf(query, sizeof(query), "SELECT A.event_id,A.remind_tick_unit,A.remind_tick,"
			"A.alarm_type,B.dtend_utime,A.alarm_datetime "
			"FROM %s as A, %s as B ON A.event_id = B.id "
			"WHERE A.remind_tick_unit >%d AND B.type =%d "
			"AND (B.dtend_utime - (A.remind_tick_unit * A.remind_tick)) %s %ld %s",
			CAL_TABLE_ALARM, CAL_TABLE_SCHEDULE,
			CALENDAR_ALARM_TIME_UNIT_SPECIFIC, CALENDAR_BOOK_TYPE_TODO,
			true == get_all ? "=" : ">", utime,
			true == get_all ? "" : "ORDER BY (B.dtend_utime - (A.remind_tick_unit * A.remind_tick)) LIMIT 1 ");

	sqlite3_stmt *stmt = NULL;
	ret = cal_db_util_query_prepare(query, &stmt);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_query_prepare() Fail(%d)", ret);
		SECURE("query[%s]", query);
		return;
	}

	while (CAL_SQLITE_ROW == cal_db_util_stmt_step(stmt)) {
		struct _alarm_data_s *ad = calloc(1, sizeof(struct _alarm_data_s));
		if (NULL == ad) {
			ERR("calloc() Fail");
			sqlite3_finalize(stmt);
			return;
		}

		ad->event_id = sqlite3_column_int(stmt, 0);
		ad->unit = sqlite3_column_int(stmt, 1);
		ad->tick = sqlite3_column_int(stmt, 2);
		ad->type = sqlite3_column_int(stmt, 3);
		ad->time = (long long int)sqlite3_column_int64(stmt, 4);
		snprintf(ad->datetime, sizeof(ad->datetime), "%s", (const char *)sqlite3_column_text(stmt, 5));
		*l = g_list_append(*l, ad);
		DBG("found id(%d) unit(%d) tick(%d) type(%d) time(%lld) [%s]",
				ad->event_id, ad->unit, ad->tick, ad->type, ad->time, ad->datetime);

		ad->alert_utime = ad->time - (ad->tick * ad->unit);
		if (false == get_all) break;
	}
	sqlite3_finalize(stmt);
}

static void _cal_server_alarm_get_upcoming_nonspecific_todo_localtime(const char *datetime, bool get_all, GList **l)
{
	int ret = 0;
	/*
	 * A:alarm
	 * B:todo(allday)
	 */
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	snprintf(query, sizeof(query), "SELECT A.event_id,A.remind_tick_unit,A.remind_tick,"
			"A.alarm_type,A.alarm_utime,B.dtend_datetime "
			"FROM %s as A, %s as B ON A.event_id = B.id "
			"WHERE A.remind_tick_unit >%d AND B.type =%d "
			"AND (strftime('%%s', B.dtend_datetime) - (A.remind_tick_unit * A.remind_tick) - strftime('%%s', '%s') %s 0) %s",
			CAL_TABLE_ALARM, CAL_TABLE_SCHEDULE,
			CALENDAR_ALARM_TIME_UNIT_SPECIFIC, CALENDAR_BOOK_TYPE_TODO,
			datetime, true == get_all ? "=" : ">",
			true == get_all ? "" : "ORDER BY (strftime('%s', B.dtend_datetime) - (A.remind_tick_unit * A.remind_tick)) LIMIT 1 ");

	sqlite3_stmt *stmt = NULL;
	ret = cal_db_util_query_prepare(query, &stmt);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_query_prepare() Fail(%d)", ret);
		SECURE("query[%s]", query);
		return;
	}

	while (CAL_SQLITE_ROW == cal_db_util_stmt_step(stmt)) {
		struct _alarm_data_s *ad = calloc(1, sizeof(struct _alarm_data_s));
		if (NULL == ad) {
			ERR("calloc() Fail");
			sqlite3_finalize(stmt);
			return;
		}

		ad->event_id = sqlite3_column_int(stmt, 0);
		ad->unit = sqlite3_column_int(stmt, 1);
		ad->tick = sqlite3_column_int(stmt, 2);
		ad->type = sqlite3_column_int(stmt, 3);
		ad->time = (long long int)sqlite3_column_int64(stmt, 4);
		snprintf(ad->datetime, sizeof(ad->datetime), "%s", (const char *)sqlite3_column_text(stmt, 5));
		*l = g_list_append(*l, ad);
		DBG("found id(%d) unit(%d) tick(%d) type(%d) time(%lld) [%s]",
				ad->event_id, ad->unit, ad->tick, ad->type, ad->time, ad->datetime);

		int y = 0, m = 0, d = 0;
		int h = 0, n = 0, s = 0;
		sscanf(ad->datetime, CAL_FORMAT_LOCAL_DATETIME, &y, &m, &d, &h, &n, &s);

		struct tm st = {0};
		st.tm_year = y - 1900;
		st.tm_mon = m -1;
		st.tm_mday = d;
		st.tm_hour = h;
		st.tm_min = n;
		st.tm_sec = s;
		ad->alert_utime = (long long int)mktime(&st) - (ad->tick * ad->unit);
		if (false == get_all) break;
	}
	sqlite3_finalize(stmt);
}

static void _cal_server_alarm_get_latest(time_t utime, bool get_all, GList **out_l)
{
	CAL_FN_CALL();
	RET_IF(NULL == out_l);

	tzset();
	struct tm st_local = {0};
	localtime_r(&utime, &st_local);

	char datetime[CAL_STR_SHORT_LEN32] = {0};
	snprintf(datetime, sizeof(datetime), CAL_FORMAT_LOCAL_DATETIME,
			st_local.tm_year +1900, st_local.tm_mon + 1, st_local.tm_mday,
			st_local.tm_hour, st_local.tm_min, st_local.tm_sec);
	DBG("get alert to register with given time (%ld) datetime[%s]", utime, datetime);

	GList *l = NULL;

	_cal_server_alarm_get_upcoming_specific_utime(utime, get_all, &l);
	_cal_server_alarm_get_upcoming_nonspecific_event_utime(utime, get_all, &l);
	_cal_server_alarm_get_upcoming_nonspecific_todo_utime(utime, get_all, &l);

	_cal_server_alarm_get_upcoming_specific_localtime(datetime, get_all, &l);
	_cal_server_alarm_get_upcoming_nonspecific_event_localtime(datetime, get_all, &l);
	_cal_server_alarm_get_upcoming_nonspecific_todo_localtime(datetime, get_all, &l);

	*out_l = l;
}

static gint _cal_server_alarm_sort_cb(gconstpointer a, gconstpointer b)
{
	struct _alarm_data_s *p1 = (struct _alarm_data_s *)a;
	struct _alarm_data_s *p2 = (struct _alarm_data_s *)b;
	DBG("%lld) > (%lld)", p1->alert_utime, p2->alert_utime);

	return p1->alert_utime < p2->alert_utime ? -1 : 1;
}

static GFunc _cal_server_alarm_print_cb(gpointer data, gpointer user_data)
{
	struct _alarm_data_s *ad = (struct _alarm_data_s *)data;
	DBG("id(%d) unit(%d) tick(%d) type(%d) time(%lld) datetime[%s]",
			ad->event_id, ad->unit, ad->tick, ad->type, ad->time, ad->datetime);
	return 0;
}

static int _cal_server_alarm_register(GList *alarm_list)
{
	CAL_FN_CALL();
	RETV_IF(NULL == alarm_list, CALENDAR_ERROR_INVALID_PARAMETER);

	int ret = CALENDAR_ERROR_NONE;
	GList *l = g_list_first(alarm_list);
	struct _alarm_data_s *ad = (struct _alarm_data_s *)l->data;
	RETVM_IF(NULL == ad, CALENDAR_ERROR_DB_FAILED, "No data");

	/* clear all alarm which set by mine. */
	ret = alarmmgr_enum_alarm_ids(_cal_server_alarm_clear_all_cb, NULL);
	if (ret != ALARMMGR_RESULT_SUCCESS) {
		ERR("alarmmgr_enum_alarm_ids() Fail");
		return ret;
	}

	time_t mod_time = (time_t)ad->alert_utime;
	alarm_entry_t *alarm_info = NULL;
	alarm_info = alarmmgr_create_alarm();
	if (NULL == alarm_info) {
		ERR("Failed to create alarm");
		return CALENDAR_ERROR_DB_FAILED;
	}
	tzset();
	struct tm st_alarm = {0};

	switch (ad->system_type) {
	case CALENDAR_SYSTEM_EAST_ASIAN_LUNISOLAR:
		gmtime_r(&mod_time, &st_alarm);
		break;

	default:
		tzset();
		localtime_r(&mod_time, &st_alarm);
		break;
	}

	alarm_date_t date = {0};
	date.year = st_alarm.tm_year + 1900;
	date.month = st_alarm.tm_mon + 1;
	date.day = st_alarm.tm_mday;
	date.hour = st_alarm.tm_hour;
	date.min = st_alarm.tm_min;
	date.sec = st_alarm.tm_sec;
	alarmmgr_set_time(alarm_info, date);
	DBG(COLOR_CYAN" >>> registered record id (%d) at %04d/%02d/%02d %02d:%02d:%02d "COLOR_END" (utime(%lld), datetime[%s], tick(%d) unit(%d))",
			ad->event_id, date.year, date.month, date.day, date.hour, date.min, date.sec,
			ad->time, ad->datetime, ad->tick, ad->unit);

	int alarm_id = 0;
	ret = alarmmgr_add_alarm_with_localtime(alarm_info, NULL, &alarm_id);
	if (ret < 0) {
		ERR("alarmmgr_add_alarm_with_localtime Fail (%d)", ret);
		alarmmgr_free_alarm(alarm_info);
		return ret;
	}
	DBG("alarmmgr id (%d)", alarm_id);
	_cal_server_alarm_update_alarm_id(alarm_id, ad->event_id, ad->tick, ad->unit);
	alarmmgr_free_alarm(alarm_info);
	return CALENDAR_ERROR_NONE;
}

struct alarm_ud {
	GList *alarm_list;
};

static bool __app_matched_cb(app_control_h app_control, const char *package, void *user_data)
{
	CAL_FN_CALL();

	int ret = 0;

	RETV_IF(NULL == user_data, true);

	char *mime = NULL;
	ret = app_control_get_mime(app_control, &mime);
	RETVM_IF(APP_CONTROL_ERROR_NONE != ret, true, "app_control_get_mime() Fail(%d)", ret);

	const char *reminder_mime = "application/x-tizen.calendar.reminder";
	if (strncmp(mime, reminder_mime, strlen(reminder_mime))) { /* not same */
		DBG("get mime[%s] is not [%s]", mime, reminder_mime);
		free(mime);
		return true;
	}
	free(mime);

	GList *alarm_list = (GList *)user_data;
	int len = 0;
	len = g_list_length(alarm_list);
	if (0 == len) {
		DBG("len is 0, no alarm list");
		return true;
	}

	app_control_h b = NULL;
	app_control_create(&b);
	app_control_set_operation(b,  APP_CONTROL_OPERATION_DEFAULT);
	app_control_set_app_id(b, package);

	char **ids = NULL;
	ids = calloc(len, sizeof(char *));
	if (NULL == ids) {
		ERR("calloc() Fail");
		app_control_destroy(b);
		return CALENDAR_ERROR_OUT_OF_MEMORY;
	}
	GList *l = g_list_first(alarm_list);
	int i;
	for (i = 0; i < len; i++) {
		struct _alarm_data_s *ad = (struct _alarm_data_s *)l->data;
		if (NULL == ad) {
			ERR("No data");
			l = g_list_next(l);
			continue;
		}
		DBG("pkg[%s] time[%lld] tick[%d] unit[%d] record_type[%d]",
				package, ad->time, ad->tick, ad->unit, ad->record);

		int len = 0;
		char extra[CAL_STR_MIDDLE_LEN] = {0};
		len = snprintf(extra, sizeof(extra), "%s=%d", "id", ad->event_id);
		len += snprintf(extra+len, sizeof(extra)-len, "&%s=%lld", "time", ad->time);
		len += snprintf(extra+len, sizeof(extra)-len, "&%s=%d", "tick", ad->tick);
		len += snprintf(extra+len, sizeof(extra)-len, "&%s=%d", "unit", ad->unit);
		len += snprintf(extra+len, sizeof(extra)-len, "&%s=%d", "type", ad->record);

		char buf_id[CAL_STR_MIDDLE_LEN] = {0};
		snprintf(buf_id, sizeof(buf_id), "%d", ad->event_id);
		app_control_add_extra_data(b, buf_id, extra); /* key: id, value: id=4&time=123123&.. */
		DBG("value[%s]", extra);

		/* append ids */
		ids[i] = strdup(buf_id);

		l = g_list_next(l);
	}
	app_control_add_extra_data_array(b, "ids", (const char **)ids, len);
	app_control_send_launch_request(b, NULL, NULL);
	app_control_destroy(b);

	for (i = 0; i < len; i++) {
		free(ids[i]);
		ids[i] = NULL;
	}
	free(ids);

	return true;
}

static void _cal_server_alarm_noti_with_control(GList *alarm_list)
{
	CAL_FN_CALL();
	RETM_IF(NULL == alarm_list, "No alarm list");

	app_control_h app_control = NULL;
	app_control_create(&app_control);
	app_control_set_operation(app_control, APP_CONTROL_OPERATION_VIEW);
	app_control_set_mime(app_control, "application/x-tizen.calendar.reminder");

	struct alarm_ud *au = calloc(1, sizeof(struct alarm_ud));
	if (NULL == au) {
		ERR("calloc() Fail");
		app_control_destroy(app_control);
		return;
	}
	au->alarm_list = alarm_list;
	app_control_foreach_app_matched(app_control, __app_matched_cb, au);
	app_control_destroy(app_control);
}

static void _cal_server_alarm_noti_with_callback(GList *alarm_list)
{
	RETM_IF(NULL == alarm_list, "No alarm list");

	GList *l = g_list_first(alarm_list);
	while (l) {
		struct _alarm_data_s *ad = (struct _alarm_data_s *)l->data;
		if (NULL == ad) {
			ERR("No data");
			l = g_list_next(l);
			continue;
		}
		DBG("callback time[%lld] tick[%d] unit[%d] record_type[%d]",
				ad->time, ad->tick, ad->unit, ad->record);

		int len = 0;
		char extra[CAL_STR_MIDDLE_LEN] = {0};
		len = snprintf(extra, sizeof(extra), "%s=%d", "id", ad->event_id);
		len += snprintf(extra+len, sizeof(extra)-len, "&%s=%lld", "time", ad->time);
		len += snprintf(extra+len, sizeof(extra)-len, "&%s=%d", "tick", ad->tick);
		len += snprintf(extra+len, sizeof(extra)-len, "&%s=%d", "unit", ad->unit);
		len += snprintf(extra+len, sizeof(extra)-len, "&%s=%d", "type", ad->record);
		cal_dbus_publish_reminder(len, extra);

		l = g_list_next(l);
	}
}

int cal_server_alarm_register_next_alarm(time_t utime)
{
	GList *l = NULL;
	_cal_server_alarm_get_latest(utime, false, &l);
	if (NULL == l)
		return CALENDAR_ERROR_NONE;

	l = g_list_sort(l, _cal_server_alarm_sort_cb);
	g_list_foreach(l, (GFunc)_cal_server_alarm_print_cb, NULL);
	_cal_server_alarm_register(l);

	g_list_free_full(l, free);
	return CALENDAR_ERROR_NONE;
}

void cal_server_alarm_alert(time_t tt_alert)
{
	GList *l = NULL;
	_cal_server_alarm_get_latest(tt_alert, true, &l);
	_cal_server_alarm_noti_with_callback(l);
	_cal_server_alarm_noti_with_control(l);
}

static int _alert_cb(alarm_id_t alarm_id, void *data)
{
	CAL_FN_CALL();
	DBG("alarm_id (%ld)", alarm_id);

	time_t tt_alert = 0;
	cal_server_alarm_get_alert_time(alarm_id, &tt_alert);
	cal_server_alarm_alert(tt_alert);
	_cal_server_alarm_unset_alerted_alarmmgr_id(alarm_id);
	cal_server_alarm_register_next_alarm(tt_alert);
	return 0;
}

static void _cal_server_alarm_timechange_cb(keynode_t *node, void *data)
{
	time_t t = time(NULL);
	cal_server_alarm_alert(t);
	cal_server_alarm_register_next_alarm(t);
}

void _cal_server_alarm_set_timechange(void)
{
	vconf_notify_key_changed(VCONFKEY_SYSTEM_TIME_CHANGED,
			_cal_server_alarm_timechange_cb, NULL);

	vconf_notify_key_changed(VCONFKEY_TELEPHONY_NITZ_GMT,
			_cal_server_alarm_timechange_cb, NULL);
	vconf_notify_key_changed(VCONFKEY_TELEPHONY_NITZ_EVENT_GMT,
			_cal_server_alarm_timechange_cb, NULL);
	vconf_notify_key_changed(VCONFKEY_TELEPHONY_NITZ_ZONE,
			_cal_server_alarm_timechange_cb, NULL);
}

static void _changed_cb(const char* view_uri, void* data)
{
	CAL_FN_CALL();
	cal_server_alarm_register_next_alarm(time(NULL));
}

static int _cal_server_alarm_set_inotify(calendar_db_changed_cb callback)
{
	cal_inotify_subscribe(CAL_NOTI_TYPE_EVENT, CAL_NOTI_EVENT_CHANGED, callback, NULL);
	cal_inotify_subscribe(CAL_NOTI_TYPE_TODO, CAL_NOTI_TODO_CHANGED, callback, NULL);
	return 0;
}

int cal_server_alarm_init(void)
{
	CAL_FN_CALL();

	int ret = 0;
	_cal_server_alarm_set_timechange();
	_cal_server_alarm_set_inotify(_changed_cb);

	ret = alarmmgr_init("calendar-service");
	if (ret < 0) {
		ERR("alarmmgr_init() Fail(%d)", ret);
		return CALENDAR_ERROR_SYSTEM;
	}

	ret = alarmmgr_set_cb(_alert_cb, NULL);
	if (ret < 0) {
		ERR("alarmmgr_set_cb() Fail(%d)", ret);
		return CALENDAR_ERROR_SYSTEM;
	}

	cal_server_alarm_register_next_alarm(time(NULL));
	return CALENDAR_ERROR_NONE;
}

void cal_server_alarm_deinit(void)
{
	alarmmgr_fini();
}

