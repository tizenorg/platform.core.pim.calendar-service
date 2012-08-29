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
#include "cals-alarm.h"
#include "cals-utils.h"
#include "cals-schedule.h"
#include "cals-time.h"

static inline void cals_event_make_condition(int calendar_id,
		time_t start_time, time_t end_time, int all_day, char *dest, int dest_size)
{
	int ret;

	ret = snprintf(dest, dest_size, "type = %d", CALS_SCH_TYPE_EVENT);

	if (calendar_id)
		ret += snprintf(dest+ret, dest_size-ret, "AND calendar_id = %d", calendar_id);

	if (0 < start_time)
		ret += snprintf(dest+ret, dest_size-ret, "AND start_date_time >= %ld", start_time);

	if (0 < end_time)
		ret += snprintf(dest+ret, dest_size-ret, "AND start_date_time <= %ld", end_time);

	if (0 <= all_day)
		ret += snprintf(dest+ret, dest_size-ret, "AND all_day_event = %d", !!(all_day));
}


/**
 * This function gets count related with event.
 * If parameter is invalid(0, negative, etc.), it will be ignored
 * If all parameters are invalid, this function return all event count.
 *
 * @param[in] calendar_id calendar_id
 * @param[in] start_time start time
 * @param[in] end_time end time
 * @param[in] all_day TRUE(1 or positive)/FALSE(0)/IGNORE(-1 or negative)
 * @return The count number on success, Negative value(#cal_error) on error
 */
/*
API int calendar_svc_event_get_count(int calendar_id,
		time_t start_time, time_t end_time, int all_day)
{
	char query[CALS_SQL_MIN_LEN];
	char cond[CALS_SQL_MIN_LEN];

	cals_event_make_condition(calendar_id, start_time, end_time, all_day,
									cond, sizeof(cond));

	snprintf(query, sizeof(query), "SELECT COUNT(*) FROM %s WHERE is_deleted = 0 AND %s",
			CALS_TABLE_SCHEDULE, cond);

	return cals_query_get_first_int_result(query);
}
*/

API cal_iter* calendar_svc_event_get_list(int calendar_id,
	time_t start_time, time_t end_time, int all_day)
{
	cal_iter *iter;
	sqlite3_stmt *stmt = NULL;
	char query[CALS_SQL_MAX_LEN];
	char cond[CALS_SQL_MIN_LEN];

	cals_event_make_condition(calendar_id, start_time, end_time, all_day,
									cond, sizeof(cond));

	sprintf(query,"SELECT * FROM %s WHERE is_deleted = 0 AND %s ORDER BY start_date_time",
			CALS_TABLE_SCHEDULE, cond);

	stmt = cals_query_prepare(query);
	retvm_if(NULL == stmt, NULL,"cals_query_prepare() Failed");

	iter = calloc(1, sizeof(cal_iter));
	if (NULL == iter) {
		sqlite3_finalize(stmt);
		ERR("calloc() Failed(%d)", errno);
		return NULL;
	}
	iter->i_type = CAL_STRUCT_TYPE_SCHEDULE;
	iter->stmt = stmt;

	return CAL_SUCCESS;
}

API int calendar_svc_event_search(int fields, const char *keyword, cal_iter **iter)
{
	int ret;

	ret = cals_sch_search(CALS_SCH_TYPE_EVENT, fields, keyword, iter);
	retvm_if(ret < 0, ret, "cals_sch_search() failed(%d)", ret);

	return CAL_SUCCESS;
}

static inline int cals_event_get_changes(int calendar_id, int version, cal_iter *iter)
{
	char buf[64] = {0};
	char query[CALS_SQL_MIN_LEN] = {0,};
	sqlite3_stmt *stmt;
	cals_updated *last;
	cals_updated *result;

	if (calendar_id > 0) {
		snprintf(buf, sizeof(buf), "AND calendar_id = %d ", calendar_id);
	} else {
		memset(buf, 0x0, sizeof(buf));
	}

	snprintf(query, sizeof(query),
			"SELECT id, changed_ver, created_ver, is_deleted, calendar_id FROM %s "
			"WHERE changed_ver > %d AND original_event_id = %d AND type = %d %s "
			"UNION "
			"SELECT schedule_id, deleted_ver, -1, 1, calendar_id FROM %s "
			"WHERE deleted_ver > %d AND schedule_type = %d %s ",
			CALS_TABLE_SCHEDULE,
			version, CALS_INVALID_ID, CALS_SCH_TYPE_EVENT, buf,
			CALS_TABLE_DELETED,
			version, CALS_SCH_TYPE_EVENT, buf);

	DBG("query(%s)", query);
	stmt = cals_query_prepare(query);
	retvm_if (NULL == stmt, CAL_ERR_DB_FAILED, "cals_query_prepare() failed.");

	while (CAL_TRUE == cals_stmt_step(stmt)){
		result = cals_updated_schedule_add_mempool();

		result->id = sqlite3_column_int(stmt, 0);
		result->ver = sqlite3_column_int(stmt, 1);
		if (sqlite3_column_int(stmt, 3) == 1) {
			result->type = CALS_UPDATED_TYPE_DELETED;
		} else if (sqlite3_column_int(stmt, 2) == result->ver || version < sqlite3_column_int(stmt, 2)) {
			result->type = CALS_UPDATED_TYPE_INSERTED;
		} else {
			result->type = CALS_UPDATED_TYPE_MODIFIED;
		}
		result->calendar_id = sqlite3_column_int(stmt, 4);

		if (iter->info->head == NULL) {
			iter->info->head = result;
		} else {
			last->next = result;
		}
		last = result;
	}
	iter->i_type = CAL_STRUCT_TYPE_UPDATED_LIST;

	sqlite3_finalize(stmt);

	return CAL_SUCCESS;
}


API int calendar_svc_event_get_changes(int calendar_id, int version, cal_iter **iter)
{
	int ret;
	cal_iter *it;

	retv_if (NULL == iter, CAL_ERR_ARG_NULL);
	retvm_if (version < 0, CAL_ERR_ARG_INVALID, "Invalid argument");

	it = calloc(1, sizeof(cal_iter));
	retvm_if (NULL == it, CAL_ERR_OUT_OF_MEMORY, "calloc() failed");

	it->info = calloc(1, sizeof(cals_updated_info));
	if (!it->info) {
		ERR("calloc() Failed");
		free(it);
		return CAL_ERR_OUT_OF_MEMORY;
	}

	ret = cals_event_get_changes(calendar_id, version, it);
	if (ret) {
		ERR("cals_get_updated_schedules() failed(%d)", ret);
		free(it->info);
		free(it);
		return ret;
	}

	*iter = it;

	return CAL_SUCCESS;
}

API int calendar_svc_event_get_normal_list_by_period(int calendar_id, int op_code,
		long long int stime, long long int etime, cal_iter **iter)
{
	/* calendar_id: -1 means searching all calendar */
	retv_if(iter == NULL, CAL_ERR_ARG_NULL);

	char query[CALS_SQL_MIN_LEN] = {0};
	char buf[64] = {0};
	sqlite3_stmt *stmt = NULL;

	if (calendar_id > 0) {
		snprintf(buf, sizeof(buf), "AND B.calendar_id = %d", calendar_id);
	} else {
		memset(buf, 0x0, sizeof(buf));
	}

	*iter = calloc(1, sizeof(cal_iter));
	retvm_if(NULL == *iter, CAL_ERR_OUT_OF_MEMORY, "Failed to calloc(%d)", errno);
	(*iter)->is_patched = 0;

	switch (op_code) {
	case CALS_LIST_PERIOD_NORMAL_ONOFF:
		(*iter)->i_type = CALS_STRUCT_TYPE_PERIOD_NORMAL_ONOFF;
		snprintf(query, sizeof(query),
				"SELECT A.event_id, "
				"B.dtstart_type, A.dtstart_utime, "
				"B.dtend_type, A.dtend_utime "
				"FROM %s as A, %s as B, %s as C "
				"ON A.event_id = B.id AND B.calendar_id = C.rowid "
				"WHERE ((A.dtstart_utime < %lld AND A.dtend_utime > %lld) "
				"OR A.dtstart_utime = %lld) "
				"AND B.type = %d AND B.is_deleted = 0 AND C.visibility = 1 %s "
				"ORDER BY A.dtstart_utime ",
				CALS_TABLE_NORMAL_INSTANCE, CALS_TABLE_SCHEDULE, CALS_TABLE_CALENDAR,
				etime, stime,
				stime,
				CALS_SCH_TYPE_EVENT, buf);
		break;

	case CALS_LIST_PERIOD_NORMAL_BASIC:
		(*iter)->i_type = CALS_STRUCT_TYPE_PERIOD_NORMAL_BASIC;
		snprintf(query, sizeof(query),
				"SELECT A.event_id, "
				"B.dtstart_type, A.dtstart_utime, "
				"B.dtend_type, A.dtend_utime, "
				"B.summary, B.location "
				"FROM %s as A, %s as B, %s as C "
				"ON A.event_id = B.id AND B.calendar_id = C.rowid "
				"WHERE ((A.dtstart_utime < %lld AND A.dtend_utime > %lld) "
				"OR A.dtstart_utime = %lld) "
				"AND B.type = %d AND B.is_deleted = 0 AND C.visibility = 1 %s "
				"ORDER BY A.dtstart_utime ",
				CALS_TABLE_NORMAL_INSTANCE, CALS_TABLE_SCHEDULE, CALS_TABLE_CALENDAR,
				etime, stime,
				stime,
				CALS_SCH_TYPE_EVENT, buf);
		break;

	case CALS_LIST_PERIOD_NORMAL_OSP:
		(*iter)->i_type = CALS_STRUCT_TYPE_PERIOD_NORMAL_OSP;
		snprintf(query, sizeof(query),
				"SELECT A.event_id, B.calendar_id, "
				"B.dtstart_type, A.dtstart_utime, "
				"B.dtend_type, A.dtend_utime, "
				"B.summary, B.description, B.location, B.busy_status, "
				"B.meeting_status, B.priority, B.sensitivity, B.rrule_id "
				"FROM %s as A, %s as B, %s as C "
				"ON A.event_id = B.id AND B.calendar_id = C.rowid "
				"WHERE ((A.dtstart_utime < %lld AND A.dtend_utime > %lld) "
				"OR A.dtstart_utime = %lld) "
				"AND B.type = %d AND B.is_deleted = 0 AND C.visibility = 1 %s "
				"ORDER BY A.dtstart_utime ",
				CALS_TABLE_NORMAL_INSTANCE, CALS_TABLE_SCHEDULE, CALS_TABLE_CALENDAR,
				etime, stime,
				stime,
				CALS_SCH_TYPE_EVENT, buf);
		break;

	case CALS_LIST_PERIOD_NORMAL_LOCATION:
		(*iter)->i_type = CALS_STRUCT_TYPE_PERIOD_NORMAL_LOCATION;
		snprintf(query, sizeof(query),
				"SELECT A.event_id, B.calendar_id, "
				"B.dtstart_type, A.dtstart_utime, "
				"B.dtend_type, A.dtend_utime, "
				"B.summary, B.description, B.location, B.busy_status, "
				"B.meeting_status, B.priority, B.sensitivity, B.rrule_id, "
				"B.latitude, B.longitude "
				"FROM %s as A, %s as B, %s as C "
				"ON A.event_id = B.id AND B.calendar_id = C.rowid "
				"WHERE ((A.dtstart_utime < %lld AND A.dtend_utime > %lld) "
				"OR A.dtstart_utime = %lld) "
				"AND B.type = %d AND B.is_deleted = 0 AND C.visibility = 1 %s "
				"ORDER BY A.dtstart_utime ",
				CALS_TABLE_NORMAL_INSTANCE, CALS_TABLE_SCHEDULE, CALS_TABLE_CALENDAR,
				etime, stime,
				stime,
				CALS_SCH_TYPE_EVENT, buf);
		break;

	case CALS_LIST_PERIOD_NORMAL_ALARM:
		(*iter)->i_type = CALS_STRUCT_TYPE_PERIOD_NORMAL_ALARM;
		snprintf(query, sizeof(query),
				"SELECT A.event_id, C.calendar_id, "
				"C.dtstart_type, A.dtstart_utime, "
				"C.dtend_type, A.dtend_utime, "
				"(A.dtstart_utime - (B.remind_tick * B.remind_tick_unit * 60)), "
				"B.alarm_id "
				"FROM %s as A, %s as B, %s as C "
				"ON A.event_id = B.event_id AND B.event_id = C.id "
				"WHERE C.type = %d AND C.is_deleted = 0 "
				"AND A.dtstart_utime - (B.remind_tick * B.remind_tick_unit * 60) >= %lld "
				"AND A.dtstart_utime - (B.remind_tick * B.remind_tick_unit * 60) < %lld "
				"%s "
				"ORDER BY B.alarm_time ",
				CALS_TABLE_NORMAL_INSTANCE, CALS_TABLE_ALARM, CALS_TABLE_SCHEDULE,
				CALS_SCH_TYPE_EVENT,
				stime, etime,
				buf);
		break;

	}
	DBG("query(%s)", query);
	stmt = cals_query_prepare(query);
	retvm_if(NULL == stmt, CAL_ERR_DB_FAILED, "Failed to query prepare");

	(*iter)->stmt = stmt;
	return CAL_SUCCESS;
}

API int calendar_svc_event_get_allday_list_by_period(int calendar_id, int op_code,
		int dtstart_year, int dtstart_month, int dtstart_mday,
		int dtend_year, int dtend_month, int dtend_mday, cal_iter **iter)
{
	/* calendar_id -1 means searching all calendar */
	retv_if(iter == NULL, CAL_ERR_ARG_NULL);

	sqlite3_stmt *stmt = NULL;
	char query[CALS_SQL_MIN_LEN] = {0};
	char buf[64] = {0};
	char sdate[32] = {0};
	char edate[32] = {0};

	if (dtstart_year < 0 || dtstart_month < 0 || dtstart_mday < 0) {
		ERR("Check start date(%d/%d/%d)", dtstart_year, dtstart_month, dtstart_mday);
		return CAL_ERR_ARG_NULL;
	}
	if (dtend_year < 0 || dtend_month < 0 || dtend_mday < 0) {
		ERR("Check end date(%d/%d/%d)", dtend_year, dtend_month, dtend_mday);
		return CAL_ERR_ARG_NULL;
	}

	if (calendar_id > 0) {
		snprintf(buf, sizeof(buf), "AND B.calendar_id = %d", calendar_id);
	} else {
		memset(buf, 0x0, sizeof(buf));
	}

	snprintf(sdate, sizeof(sdate), "%4d%02d%02d", dtstart_year, dtstart_month, dtstart_mday);
	snprintf(edate, sizeof(edate), "%4d%02d%02d", dtend_year, dtend_month, dtend_mday);

	*iter = calloc(1, sizeof(cal_iter));
	retvm_if(NULL == *iter, CAL_ERR_OUT_OF_MEMORY, "Failed to calloc(%d)", errno);
	(*iter)->is_patched = 0;

	switch (op_code) {
	case CALS_LIST_PERIOD_ALLDAY_ONOFF:
		(*iter)->i_type = CALS_STRUCT_TYPE_PERIOD_ALLDAY_ONOFF;
		snprintf(query, sizeof(query),
				"SELECT A.event_id, "
				"B.dtstart_type, A.dtstart_datetime, "
				"B.dtend_type, A.dtend_datetime "
				"FROM %s as A, %s as B, %s as C "
				"ON A.event_id = B.id AND B.calendar_id = C.rowid "
				"WHERE A.dtstart_datetime <= %s AND A.dtend_datetime >= %s "
				"AND B.type = %d AND B.is_deleted = 0 AND C.visibility = 1 %s "
				"ORDER BY A.dtstart_datetime ",
				CALS_TABLE_ALLDAY_INSTANCE, CALS_TABLE_SCHEDULE, CALS_TABLE_CALENDAR,
				edate, sdate,
				CALS_SCH_TYPE_EVENT, buf);
		break;

	case CALS_LIST_PERIOD_ALLDAY_BASIC:
		(*iter)->i_type = CALS_STRUCT_TYPE_PERIOD_ALLDAY_BASIC;
		snprintf(query, sizeof(query),
				"SELECT A.event_id, "
				"B.dtstart_type, A.dtstart_datetime, "
				"B.dtend_type, A.dtend_datetime, "
				"B.summary, B.location "
				"FROM %s as A, %s as B, %s as C "
				"ON A.event_id = B.id AND B.calendar_id = C.rowid "
				"WHERE A.dtstart_datetime <= %s AND A.dtend_datetime >= %s "
				"AND B.type = %d AND B.is_deleted = 0 AND C.visibility = 1 %s "
				"ORDER BY A.dtstart_datetime ",
				CALS_TABLE_ALLDAY_INSTANCE, CALS_TABLE_SCHEDULE, CALS_TABLE_CALENDAR,
				edate, sdate,
				CALS_SCH_TYPE_EVENT, buf);
		break;

	case CALS_LIST_PERIOD_ALLDAY_OSP:
		(*iter)->i_type = CALS_STRUCT_TYPE_PERIOD_ALLDAY_OSP;
		snprintf(query, sizeof(query),
				"SELECT A.event_id, B.calendar_id, "
				"B.dtstart_type, A.dtstart_datetime, "
				"B.dtend_type, A.dtend_datetime, "
				"B.summary, B.description, B.location, B.busy_status, "
				"B.meeting_status, B.priority, B.sensitivity, B.rrule_id "
				"FROM %s as A, %s as B, %s as C "
				"ON A.event_id = B.id AND B.calendar_id = C.rowid "
				"WHERE A.dtstart_datetime <= %s AND A.dtend_datetime >= %s "
				"AND B.type = %d AND B.is_deleted = 0 AND C.visibility = 1 %s "
				"ORDER BY A.dtstart_datetime ",
				CALS_TABLE_ALLDAY_INSTANCE, CALS_TABLE_SCHEDULE, CALS_TABLE_CALENDAR,
				edate, sdate,
				CALS_SCH_TYPE_EVENT, buf);
		break;

	case CALS_LIST_PERIOD_ALLDAY_LOCATION:
		(*iter)->i_type = CALS_STRUCT_TYPE_PERIOD_ALLDAY_LOCATION;
		snprintf(query, sizeof(query),
				"SELECT A.event_id, B.calendar_id, "
				"B.dtstart_type, A.dtstart_datetime, "
				"B.dtend_type, A.dtend_datetime, "
				"B.summary, B.description, B.location, B.busy_status, "
				"B.meeting_status, B.priority, B.sensitivity, B.rrule_id, "
				"B.latitude, B.longitude "
				"FROM %s as A, %s as B, %s as C "
				"ON A.event_id = B.id AND B.calendar_id = C.rowid "
				"WHERE A.dtstart_datetime <= %s AND A.dtend_datetime >= %s "
				"AND B.type = %d AND B.is_deleted = 0 AND C.visibility = 1 %s "
				"ORDER BY A.dtstart_datetime ",
				CALS_TABLE_ALLDAY_INSTANCE, CALS_TABLE_SCHEDULE, CALS_TABLE_CALENDAR,
				edate, sdate,
				CALS_SCH_TYPE_EVENT, buf);
		break;
/*
	case CALS_LIST_PERIOD_ALLDAY_ALARM:
		(*iter)->i_type = CALS_STRUCT_TYPE_PERIOD_ALLDAY_ALARM;
		snprintf(query, sizeof(query),
				"SELECT B.event_id, B.alarm_time, B.alarm_id "
				"FROM %s as A, %s as B, %s as C "
				"ON A.event_id = B.event_id "
				"WHERE B.alarm_time >= %lld AND B.alarm_time < %lld "
				"AND C.type = %d AND C.is_deleted = 0 "
				"%s "
				"ORDER BY B.alarm_time ",
				CALS_TABLE_ALLDAY_INSTANCE, CALS_TABLE_ALARM, CALS_TABLE_SCHEDULE,
				sdate, edate,
				CALS_SCH_TYPE_EVENT,
				buf);
		break;
*/
	}

	stmt = cals_query_prepare(query);
	retvm_if(NULL == stmt, CAL_ERR_DB_FAILED, "Failed to query prepare");

	(*iter)->stmt = stmt;
	return CAL_SUCCESS;
}

/* delete instance from instance_table and update exdate from schedule_table */
API int calendar_svc_event_delete_normal_instance(int event_id, long long int dtstart_utime)
{
	int ret, len, len_datetime;
	char *exdate = NULL;
	char *str_datetime = NULL;
	char *p;
	char query[CALS_SQL_MIN_LEN] = {0};
	sqlite3_stmt *stmt;

	ret = cals_begin_trans();
	if (ret != CAL_SUCCESS) {
		ERR("cals_begin_trans() Failed(%d)", ret);
		cals_end_trans(false);
		return ret;
	}

	len_datetime = strlen("YYYYMMDDTHHMMSSZ");

	/* delete instance from normal_instance_table */
	snprintf(query, sizeof(query), "DELETE FROM %s "
			"WHERE event_id = %d AND dtstart_utime = %lld ",
			CALS_TABLE_NORMAL_INSTANCE,
			event_id, dtstart_utime);

	ret = cals_query_exec(query);
	if (ret != CAL_SUCCESS) {
		ERR("Failed to delete instance(errno:%d) [id:%d utime:%lld]",
				ret, event_id, dtstart_utime);
		cals_end_trans(false);
		return ret;
	}

	/* get exdate to append */
	snprintf(query, sizeof(query), "SELECT %s FROM %s "
			"WHERE id = %d ",
			CAL_VALUE_TXT_EXDATE, CALS_TABLE_SCHEDULE,
			event_id);

	stmt = cals_query_prepare(query);
	if (stmt == NULL) {
		ERR("cals_query_prepare() failed.");
		cals_end_trans(false);
		return CAL_ERR_DB_FAILED;
	}

	ret = cals_stmt_step(stmt);
	if (ret == CAL_TRUE) {
		exdate = (char *)sqlite3_column_text(stmt, 0);
		DBG("exdate(%s)", exdate);

	} else if (ret != CAL_SUCCESS) {
		ERR("Failed to step(errno:%d)", ret);
		sqlite3_finalize(stmt);
		cals_end_trans(false);
		return ret;
	}

	/* check whether exdate does already exist */
	if (exdate == NULL || strlen(exdate) == 0) {
		p = calloc(len_datetime + 1, sizeof(char));
		if (p == NULL) {
			ERR("Failed to calloc");
			sqlite3_finalize(stmt);
			cals_end_trans(false);
			return CAL_ERR_OUT_OF_MEMORY;
		}
		str_datetime =  cals_time_get_str_datetime(NULL, dtstart_utime);
		snprintf(p, len_datetime + 1, "%s", str_datetime);
		DBG("inserted exdate firstly(%s)",  str_datetime);

	} else {
		DBG("append exdate");
		len = strlen(exdate);
		p = calloc(len + strlen(",") + len_datetime + 1, sizeof(char));
		if (p == NULL) {
			ERR("Failed to calloc");
			sqlite3_finalize(stmt);
			cals_end_trans(false);
			return CAL_ERR_OUT_OF_MEMORY;
		}
		str_datetime =  cals_time_get_str_datetime(NULL, dtstart_utime);
		snprintf(p, len + strlen(",") + len_datetime + 1, "%s,%s",
				exdate, str_datetime);
	}
	if (str_datetime) free(str_datetime);
	sqlite3_finalize(stmt);

	/* updaet exdate, version, last_mod from schedule table */
	snprintf(query, sizeof(query), "UPDATE %s SET "
			"exdate = ?, "
			"changed_ver = %d, "
			"last_mod = strftime('%%s','now') "
			"WHERE id = %d ",
			CALS_TABLE_SCHEDULE,
			cals_get_next_ver(),
			event_id);

	stmt = cals_query_prepare(query);
	if (stmt == NULL) {
		ERR("cals_query_prepare() failed.");
		cals_end_trans(false);
		return CAL_ERR_DB_FAILED;
	}

	cals_stmt_bind_text(stmt, 1, p);

	ret = cals_stmt_step(stmt);
	if (CAL_SUCCESS != ret) {
		sqlite3_finalize(stmt);
		ERR("sqlite3_step() Failed(%d)", ret);
		cals_end_trans(false);
		return ret;
	}
	sqlite3_finalize(stmt);

	/* send noti */
	ret = cals_notify(CALS_NOTI_TYPE_EVENT);
	if (ret < 0) {
		WARN("cals_notify failed (%d)", ret);
	}

	cals_end_trans(true);

	return CAL_SUCCESS;
}

API int calendar_svc_event_delete_allday_instance(int event_id, int dtstart_year, int dtstart_month, int dtstart_mday)
{
	int ret, len, len_datetime;
	char *exdate = NULL;
	char *p;
	char query[CALS_SQL_MIN_LEN] = {0};
	char buf[32] = {0};
	sqlite3_stmt *stmt;

	ret = cals_begin_trans();
	if (ret != CAL_SUCCESS) {
		ERR("cals_begin_trans() Failed(%d)", ret);
		cals_end_trans(false);
		return ret;
	}

	len_datetime = strlen("YYYYMMDDTHHMMSSZ");

	/* delete instance from normal_instance_table */
	snprintf(buf, sizeof(buf), "%04d%02d%02d", dtstart_year, dtstart_month, dtstart_mday);
	DBG("allday(%s)\n", buf);
	snprintf(query, sizeof(query), "DELETE FROM %s "
			"WHERE event_id = %d AND dtstart_datetime = %s ",
			CALS_TABLE_ALLDAY_INSTANCE,
			event_id, buf);

	ret = cals_query_exec(query);
	if (ret != CAL_SUCCESS) {
		ERR("Failed to delete instance(errno:%d) [id:%d datetime:%s]",
				ret, event_id, buf);
		cals_end_trans(false);
		return ret;
	}

	/* get exdate to append */
	snprintf(query, sizeof(query), "SELECT %s FROM %s "
			"WHERE id = %d ",
			CAL_VALUE_TXT_EXDATE, CALS_TABLE_SCHEDULE,
			event_id);

	stmt = cals_query_prepare(query);
	if (stmt == NULL) {
		ERR("cals_query_prepare() failed.");
		cals_end_trans(false);
		return CAL_ERR_DB_FAILED;
	}

	ret = cals_stmt_step(stmt);
	if (ret == CAL_TRUE) {
		exdate = (char *)sqlite3_column_text(stmt, 0);
		DBG("exdate(%s)", exdate);

	} else if (ret != CAL_SUCCESS) {
		ERR("Failed to step(errno:%d)", ret);
		sqlite3_finalize(stmt);
		cals_end_trans(false);
		return ret;
	}

	/* check whether exdate does already exist */
	if (exdate == NULL || strlen(exdate) == 0) {
		p = calloc(len_datetime + 1, sizeof(char));
		if (p == NULL) {
			ERR("Failed to calloc");
			sqlite3_finalize(stmt);
			return CAL_ERR_OUT_OF_MEMORY;
		}
		snprintf(p, len_datetime + 1, "%s", buf);
		DBG("inserted exdate firstly(%s)",  buf);

	} else {
		DBG("append exdate");
		len = strlen(exdate);
		p = calloc(len + strlen(",") + len_datetime + 1, sizeof(char));
		if (p == NULL) {
			ERR("Failed to calloc");
			sqlite3_finalize(stmt);
			cals_end_trans(false);
			return CAL_ERR_OUT_OF_MEMORY;
		}
		snprintf(p, len + strlen(",") + len_datetime + 1, "%s,%s",
				exdate, buf);
	}
	sqlite3_finalize(stmt);

	/* updaet exdate from schedule table */
	snprintf(query, sizeof(query), "UPDATE %s SET "
			"exdate = ? "
			"WHERE id = %d ",
			CALS_TABLE_SCHEDULE,
			event_id);

	stmt = cals_query_prepare(query);
	if (stmt == NULL) {
		ERR("cals_query_prepare() failed.");
		cals_end_trans(false);
		return CAL_ERR_DB_FAILED;
	}

	cals_stmt_bind_text(stmt, 1, p);

	ret = cals_stmt_step(stmt);
	if (CAL_SUCCESS != ret) {
		sqlite3_finalize(stmt);
		ERR("sqlite3_step() Failed(%d)", ret);
		cals_end_trans(false);
		return ret;
	}
	sqlite3_finalize(stmt);

	/* send noti */
	ret = cals_notify(CALS_NOTI_TYPE_EVENT);
	if (ret < 0) {
		WARN("cals_notify failed (%d)", ret);
	}

	cals_end_trans(true);

	return CAL_SUCCESS;
}





