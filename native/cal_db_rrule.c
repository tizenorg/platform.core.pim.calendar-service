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
#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_db_util.h"
#include "cal_db.h"
#include "cal_view.h"
#include "cal_time.h"
#include "cal_utils.h"

void cal_db_rrule_set_default(calendar_record_h record)
{
	cal_event_s *event = NULL;
	RET_IF(NULL == record);

	event = (cal_event_s *)record;

	switch (event->freq) {
	case CALENDAR_RECURRENCE_NONE:
		break;
	case CALENDAR_RECURRENCE_DAILY:
		break;
	case CALENDAR_RECURRENCE_WEEKLY:
		if (event->byday && 0 < strlen(event->byday))
			break;

		event->byday = cal_time_extract_by(event->system_type, event->start_tzid, event->wkst,
				&event->start, CAL_DAY_OF_WEEK);
		DBG("Not enough field in weekly, so set byda[%s]", event->byday);
		break;

	case CALENDAR_RECURRENCE_MONTHLY:
		if (event->bymonthday && 0 < strlen(event->bymonthday)) {
			break;
		}
		else if (event->byday && 0 < strlen(event->byday)) {
			break;
		}

		event->bymonthday = cal_time_extract_by(event->system_type, event->start_tzid, event->wkst,
				&event->start, CAL_DATE);
		DBG("Not enough field in monthly, so set bymonthday[%s]", event->bymonthday);
		break;

	case CALENDAR_RECURRENCE_YEARLY:
		if (event->bymonth && 0 < strlen(event->bymonth)) {
			break;
		}
		else if (event->byyearday && 0 < strlen(event->byyearday)) {
			break;
		}
		else if (event->byweekno && 0 < strlen(event->byweekno)) {
			break;
		}

		event->bymonth = cal_time_extract_by(event->system_type, event->start_tzid, event->wkst,
				&event->start, CAL_MONTH);
		event->bymonthday = cal_time_extract_by(event->system_type, event->start_tzid, event->wkst,
				&event->start, CAL_DATE);
		DBG("Not enough field in yearly, so set bymonth[%s] bymonthday[%s]",
				event->bymonth, event->bymonthday);
		break;

	default:
		break;
	}
}

void cal_db_rrule_get_rrule_from_event(calendar_record_h event, cal_rrule_s **rrule)
{
	cal_rrule_s *_rrule;
	cal_event_s *_event;

	RET_IF(NULL == event);
	_event = (cal_event_s *)event;
	if (_event->freq == CALENDAR_RECURRENCE_NONE) {
		return;
	}

	_rrule = calloc(1, sizeof(cal_rrule_s));
	RETM_IF(NULL == _rrule, "calloc() Fail");

	_rrule->freq = _event->freq;

	_rrule->range_type = _event->range_type;
	switch (_rrule->range_type) {
	case CALENDAR_RANGE_UNTIL:
		_rrule->until = _event->until;
		break;
	case CALENDAR_RANGE_COUNT:
		break;
	case CALENDAR_RANGE_NONE:
		break;
	}

	_rrule->count = _event->count;
	_rrule->interval = _event->interval;
	_rrule->bysecond = _event->bysecond;
	_rrule->byminute = _event->byminute;
	_rrule->byhour = _event->byhour;
	_rrule->byday = _event->byday;
	_rrule->bymonthday = _event->bymonthday;
	_rrule->byyearday = _event->byyearday;
	_rrule->byweekno = _event->byweekno;
	_rrule->bymonth = _event->bymonth;
	_rrule->bysetpos = _event->bysetpos;
	_rrule->wkst = _event->wkst;

	*rrule = _rrule;
}

void cal_db_rrule_set_rrule_to_event(cal_rrule_s *rrule, calendar_record_h event)
{
	cal_event_s *_event;

	RET_IF(NULL == rrule);
	RET_IF(NULL == event);

	_event = (cal_event_s *)event;

	_event->freq = rrule->freq;
	_event->range_type = rrule->range_type;
	_event->until = rrule->until;
	_event->count = rrule->count;
	_event->interval = rrule->interval;
	_event->bysecond = rrule->bysecond;
	_event->byminute = rrule->byminute;
	_event->byhour = rrule->byhour;
	_event->byday = rrule->byday;
	_event->bymonthday = rrule->bymonthday;
	_event->byyearday = rrule->byyearday;
	_event->byweekno = rrule->byweekno;
	_event->bymonth = rrule->bymonth;
	_event->bysetpos = rrule->bysetpos;
	_event->wkst = rrule->wkst;
}

void cal_db_rrule_set_rrule_to_todo(cal_rrule_s *rrule, calendar_record_h todo)
{
	cal_todo_s *_todo;

	RET_IF(NULL == rrule);
	RET_IF(NULL == todo);

	_todo = (cal_todo_s *)todo;

	_todo->freq = rrule->freq;
	_todo->range_type = rrule->range_type;
	_todo->until = rrule->until;
	_todo->count = rrule->count;
	_todo->interval = rrule->interval;
	_todo->bysecond = rrule->bysecond;
	_todo->byminute = rrule->byminute;
	_todo->byhour = rrule->byhour;
	_todo->byday = rrule->byday;
	_todo->bymonthday = rrule->bymonthday;
	_todo->byyearday = rrule->byyearday;
	_todo->byweekno = rrule->byweekno;
	_todo->bymonth = rrule->bymonth;
	_todo->bysetpos = rrule->bysetpos;
	_todo->wkst = rrule->wkst;
}

void cal_db_rrule_get_rrule_from_todo(calendar_record_h todo, cal_rrule_s **rrule)
{
	cal_rrule_s *_rrule;
	cal_todo_s *_todo;

	RET_IF(NULL == todo);

	_todo = (cal_todo_s *)todo;

	_rrule = calloc(1, sizeof(cal_rrule_s));
	RETM_IF(NULL == _rrule, "calloc() Fail");

	_rrule->freq = _todo->freq;
	_rrule->range_type = _todo->range_type;
	_rrule->until = _todo->until;
	_rrule->count = _todo->count;
	_rrule->interval = _todo->interval;
	_rrule->bysecond = _todo->bysecond;
	_rrule->byminute = _todo->byminute;
	_rrule->byhour = _todo->byhour;
	_rrule->byday = _todo->byday;
	_rrule->bymonthday = _todo->bymonthday;
	_rrule->byyearday = _todo->byyearday;
	_rrule->byweekno = _todo->byweekno;
	_rrule->bymonth = _todo->bymonth;
	_rrule->bysetpos = _todo->bysetpos;
	_rrule->wkst = _todo->wkst;

	*rrule = _rrule;
}

int _cal_db_rrule_insert_record(int id, cal_rrule_s *rrule)
{
	RETV_IF(NULL == rrule, CALENDAR_ERROR_INVALID_PARAMETER);

	int rrule_id;
	int index;
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	char until_datetime[32] = {0};
	sqlite3_stmt *stmt = NULL;
	int ret = 0;

	RETVM_IF(rrule == NULL, CALENDAR_ERROR_INVALID_PARAMETER,
			"Invalid argument: rrule is NULL");

	snprintf(query, sizeof(query),
			"INSERT INTO %s ("
			"event_id, freq, range_type, "
			"until_type, until_utime, until_datetime, "
			"count, interval, "
			"bysecond, byminute, byhour, byday, "
			"bymonthday, byyearday, byweekno, bymonth, "
			"bysetpos, wkst "
			") VALUES ("
			"%d, %d, %d, "
			"%d, %lld, ?, "
			"%d, %d, "
			"?, ?, ?, ?, "
			"?, ?, ?, ?, "
			"?, %d "
			") ",
			CAL_TABLE_RRULE,
			id, rrule->freq, rrule->range_type,
			rrule->until.type, rrule->until.time.utime,
			rrule->count, rrule->interval,
			rrule->wkst);

	ret = cal_db_util_query_prepare(query, &stmt);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_query_prepare() Fail(%d)", ret);
		SECURE("query[%s]", query);
		return ret;
	}

	index = 1;

	if (CALENDAR_TIME_LOCALTIME == rrule->until.type) {
		snprintf(until_datetime, sizeof(until_datetime), CAL_FORMAT_LOCAL_DATETIME,
				rrule->until.time.date.year,
				rrule->until.time.date.month,
				rrule->until.time.date.mday,
				rrule->until.time.date.hour,
				rrule->until.time.date.minute,
				rrule->until.time.date.second);
		cal_db_util_stmt_bind_text(stmt, index, until_datetime);
	}
	index++;

	if (rrule->bysecond)
		cal_db_util_stmt_bind_text(stmt, index, rrule->bysecond);
	index++;

	if (rrule->byminute)
		cal_db_util_stmt_bind_text(stmt, index, rrule->byminute);
	index++;

	if (rrule->byhour)
		cal_db_util_stmt_bind_text(stmt, index, rrule->byhour);
	index++;

	if (rrule->byday)
		cal_db_util_stmt_bind_text(stmt, index, rrule->byday);
	index++;

	if (rrule->bymonthday)
		cal_db_util_stmt_bind_text(stmt, index, rrule->bymonthday);
	index++;

	if (rrule->byyearday)
		cal_db_util_stmt_bind_text(stmt, index, rrule->byyearday);
	index++;

	if (rrule->byweekno)
		cal_db_util_stmt_bind_text(stmt, index, rrule->byweekno);
	index++;

	if (rrule->bymonth)
		cal_db_util_stmt_bind_text(stmt, index, rrule->bymonth);
	index++;

	if (rrule->bysetpos)
		cal_db_util_stmt_bind_text(stmt, index, rrule->bysetpos);
	index++;

	ret = cal_db_util_stmt_step(stmt);
	sqlite3_finalize(stmt);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_stmt_step() Fail(%d)", ret);
		return ret;
	}

	rrule_id = cal_db_util_last_insert_id();
	DBG("rrule_id(%d)", rrule_id);
	return CALENDAR_ERROR_NONE;
}

int cal_db_rrule_get_rrule(int id, cal_rrule_s **rrule)
{
	char query[CAL_DB_SQL_MAX_LEN];
	int index;
	sqlite3_stmt *stmt = NULL;
	cal_rrule_s *_rrule = NULL;
	const unsigned char *temp;
	int ret = 0;

	snprintf(query, sizeof(query),
			"SELECT id, event_id, freq, range_type, until_type, until_utime, "
			"until_datetime, count, interval, bysecond, byminute, byhour, byday, "
			"bymonthday, byyearday, byweekno, bymonth, bysetpos, wkst "
			"FROM %s WHERE event_id = %d ",
			CAL_TABLE_RRULE, id);

	ret = cal_db_util_query_prepare(query, &stmt);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_query_prepare() Fail(%d)", ret);
		SECURE("query[%s]", query);
		return ret;
	}

	ret = cal_db_util_stmt_step(stmt);
	if (CAL_SQLITE_ROW != ret) {
		ERR("cal_db_util_stmt_step() Fail(%d)", ret);
		SECURE("query[%s]", query);
		sqlite3_finalize(stmt);
		return ret;
	}

	_rrule = calloc(1, sizeof(cal_rrule_s));
	RETVM_IF(NULL == _rrule, CALENDAR_ERROR_OUT_OF_MEMORY, "calloc() Fail");

	index = 0;
	sqlite3_column_int(stmt, index++); /* id */
	sqlite3_column_int(stmt, index++); /* event_id */
	_rrule->freq = sqlite3_column_int(stmt, index++);

	_rrule->range_type = sqlite3_column_int(stmt, index++);
	_rrule->until.type = sqlite3_column_int(stmt, index++);
	_rrule->until.time.utime = sqlite3_column_int64(stmt, index++);

	temp = sqlite3_column_text(stmt, index++);
	if (CALENDAR_TIME_LOCALTIME == _rrule->until.type) {
		sscanf((const char *)temp, CAL_FORMAT_LOCAL_DATETIME,
				&_rrule->until.time.date.year,
				&_rrule->until.time.date.month,
				&_rrule->until.time.date.mday,
				&_rrule->until.time.date.hour,
				&_rrule->until.time.date.minute,
				&_rrule->until.time.date.second);
	}

	_rrule->count = sqlite3_column_int(stmt, index++);
	_rrule->interval = sqlite3_column_int(stmt, index++);

	temp = sqlite3_column_text(stmt, index++);
	_rrule->bysecond = cal_strdup((const char*)temp);

	temp = sqlite3_column_text(stmt, index++);
	_rrule->byminute = cal_strdup((const char*)temp);

	temp = sqlite3_column_text(stmt, index++);
	_rrule->byhour = cal_strdup((const char*)temp);

	temp = sqlite3_column_text(stmt, index++);
	_rrule->byday= cal_strdup((const char*)temp);

	temp = sqlite3_column_text(stmt, index++);
	_rrule->bymonthday= cal_strdup((const char*)temp);

	temp = sqlite3_column_text(stmt, index++);
	_rrule->byyearday= cal_strdup((const char*)temp);

	temp = sqlite3_column_text(stmt, index++);
	_rrule->byweekno= cal_strdup((const char*)temp);

	temp = sqlite3_column_text(stmt, index++);
	_rrule->bymonth= cal_strdup((const char*)temp);

	temp = sqlite3_column_text(stmt, index++);
	_rrule->bysetpos = cal_strdup((const char*)temp);

	_rrule->wkst = sqlite3_column_int(stmt, index++);

	sqlite3_finalize(stmt);

	*rrule = _rrule;
	return CALENDAR_ERROR_NONE;
}

static int _cal_db_rrule_delete_record(int id)
{
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	int ret = 0;

	snprintf(query, sizeof(query),
			"DELETE FROM %s WHERE event_id = %d ",
			CAL_TABLE_RRULE, id);

	ret = cal_db_util_query_exec(query);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_query_exec() Fail(%d)", ret);
		SECURE("[%s]", query);
		return ret;
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_db_rrule_has_record(int id, int *has_record)
{
	int ret = CALENDAR_ERROR_NONE;
	int count = 0;
	char query[CAL_DB_SQL_MAX_LEN] = {0};

	snprintf(query, sizeof(query),
			"SELECT count(*) FROM %s WHERE event_id = %d ",
			CAL_TABLE_RRULE, id);

	ret = cal_db_util_query_get_first_int_result(query, NULL, &count);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_query_get_first_int_result() failed");
		return ret;
	}

	*has_record = 0 < count ? 1 : 0;
	return CALENDAR_ERROR_NONE;
}

static int _cal_db_rrule_update_record(int id, cal_rrule_s *rrule)
{
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	char until_datetime[32] = {0};
	int ret = 0;
	sqlite3_stmt *stmt = NULL;

	snprintf(query, sizeof(query),
			"UPDATE %s SET "
			"freq = %d, "
			"range_type = %d, "
			"until_type = %d, "
			"until_utime = %lld, "
			"until_datetime = ?, "
			"count = %d, "
			"interval = %d, "
			"bysecond = ?, "
			"byminute= ?, "
			"byhour = ?, "
			"byday = ?, "
			"bymonthday = ?, "
			"byyearday = ?, "
			"byweekno = ?, "
			"bymonth = ?, "
			"bysetpos = ?, "
			"wkst = %d "
			"WHERE event_id = %d ",
			CAL_TABLE_RRULE,
			rrule->freq,
			rrule->range_type,
			rrule->until.type,
			rrule->until.time.utime,
			rrule->count,
			rrule->interval,
			rrule->wkst,
			id);

	ret = cal_db_util_query_prepare(query, &stmt);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_query_prepare() Fail(%d)", ret);
		SECURE("query[%s]", query);
		return ret;
	}

	int index = 1;
	if (CALENDAR_TIME_LOCALTIME == rrule->until.type) {
		snprintf(until_datetime, sizeof(until_datetime), CAL_FORMAT_LOCAL_DATETIME,
				rrule->until.time.date.year,
				rrule->until.time.date.month,
				rrule->until.time.date.mday,
				rrule->until.time.date.hour,
				rrule->until.time.date.minute,
				rrule->until.time.date.second);
		cal_db_util_stmt_bind_text(stmt, index, until_datetime);
	}
	index++;

	if (rrule->bysecond)
		cal_db_util_stmt_bind_text(stmt, index, rrule->bysecond);
	index++;

	if (rrule->byminute)
		cal_db_util_stmt_bind_text(stmt, index, rrule->byminute);
	index++;

	if (rrule->byhour)
		cal_db_util_stmt_bind_text(stmt, index, rrule->byhour);
	index++;

	if (rrule->byday)
		cal_db_util_stmt_bind_text(stmt, index, rrule->byday);
	index++;

	if (rrule->bymonthday)
		cal_db_util_stmt_bind_text(stmt, index, rrule->bymonthday);
	index++;

	if (rrule->byyearday)
		cal_db_util_stmt_bind_text(stmt, index, rrule->byyearday);
	index++;

	if (rrule->byweekno)
		cal_db_util_stmt_bind_text(stmt, index, rrule->byweekno);
	index++;

	if (rrule->bymonth)
		cal_db_util_stmt_bind_text(stmt, index, rrule->bymonth);
	index++;

	if (rrule->bysetpos)
		cal_db_util_stmt_bind_text(stmt, index, rrule->bysetpos);
	index++;

	ret = cal_db_util_stmt_step(stmt);
	sqlite3_finalize(stmt);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_stmt_step() Fail(%d)", ret);
		return ret;
	}

	return CALENDAR_ERROR_NONE;
}

int cal_db_rrule_insert_record(int id, cal_rrule_s *rrule)
{
	RETVM_IF(NULL == rrule, CALENDAR_ERROR_INVALID_PARAMETER,
			"Invalid argument: rrule is NULL");

	if (rrule->freq == CALENDAR_RECURRENCE_NONE) {
	}
	else {
		_cal_db_rrule_insert_record(id, rrule);
	}
	return CALENDAR_ERROR_NONE;
}

int cal_db_rrule_update_record(int id, cal_rrule_s *rrule)
{
	int has_record = 0;

	if (NULL == rrule || rrule->freq == CALENDAR_RECURRENCE_NONE) {
		DBG("freq is NONE");
		_cal_db_rrule_delete_record(id);
		return CALENDAR_ERROR_NONE;
	}
	else {
		_cal_db_rrule_has_record(id, &has_record);
		if (has_record) {
			_cal_db_rrule_update_record(id, rrule);
		}
		else {
			_cal_db_rrule_insert_record(id, rrule);
		}
	}
	return CALENDAR_ERROR_NONE;
}

