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
#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_db_util.h"
#include "cal_db.h"
#include "cal_view.h"
#include "cal_time.h"

void _cal_db_rrule_set_default(calendar_record_h event)
{
	cal_event_s *_event = NULL;
	retm_if(event == NULL, "Invalid argument: rrule is NULL");

	_event = (cal_event_s *)event;

	switch (_event->freq)
	{
	case CALENDAR_RECURRENCE_NONE:
		break;
	case CALENDAR_RECURRENCE_DAILY:
		break;
	case CALENDAR_RECURRENCE_WEEKLY:
		if (_event->bymonthday || _event->byday)
			break;

		_event->byday = _cal_time_extract_by(_event->start_tzid,
				_event->wkst, &_event->start, CAL_DAY_OF_WEEK);
		DBG("No byday so set default[%s]", _event->byday);
		break;

	case CALENDAR_RECURRENCE_MONTHLY:
		if (_event->bymonthday || _event->byday)
			break;

		_event->bymonthday = _cal_time_extract_by(_event->start_tzid,
				_event->wkst, &_event->start, CAL_DATE);
		DBG("No bymonthday so set default[%s]", _event->bymonthday);
		break;

	case CALENDAR_RECURRENCE_YEARLY:
		break;
	default:
		break;
	}
}

void _cal_db_rrule_get_rrule_from_event(calendar_record_h event, cal_rrule_s **rrule)
{
	cal_rrule_s *_rrule;
	cal_event_s *_event;

	retm_if(event == NULL, "Invalid argument: rrule is NULL");

	_event = (cal_event_s *)event;

	_rrule = calloc(1, sizeof(cal_rrule_s));
	retm_if(_rrule == NULL, "Failed to calloc");

	_rrule->freq = _event->freq;

	_rrule->range_type = _event->range_type;
	switch (_rrule->range_type)
	{
	case CALENDAR_RANGE_UNTIL:
		_rrule->until_type = _event->until_type;
		switch (_rrule->until_type)
		{
		case CALENDAR_TIME_UTIME:
			_rrule->until_utime = _event->until_utime;
			break;

		case CALENDAR_TIME_LOCALTIME:
			_rrule->until_year = _event->until_year;
			_rrule->until_month = _event->until_month;
			_rrule->until_mday = _event->until_mday;
			break;
		}
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

void _cal_db_rrule_set_rrule_to_event(cal_rrule_s *rrule, calendar_record_h event)
{
	cal_event_s *_event;

	retm_if(rrule == NULL, "Invalid argument: rrule is NULL");
	retm_if(event == NULL, "Invalid argument: rrule is NULL");

	_event = (cal_event_s *)event;

	_event->freq = rrule->freq;
	_event->range_type = rrule->range_type;
	_event->until_type = rrule->until_type;
	_event->until_utime = rrule->until_utime;
	_event->until_year = rrule->until_year;
	_event->until_month = rrule->until_month;
	_event->until_mday = rrule->until_mday;
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

void _cal_db_rrule_set_rrule_to_todo(cal_rrule_s *rrule, calendar_record_h todo)
{
	cal_todo_s *_todo;

	retm_if(rrule == NULL, "Invalid argument: rrule is NULL");
	retm_if(todo == NULL, "Invalid argument: todo is NULL");

	_todo = (cal_todo_s *)todo;

	_todo->freq = rrule->freq;
	_todo->range_type = rrule->range_type;
	_todo->until_type = rrule->until_type;
	_todo->until_utime = rrule->until_utime;
	_todo->until_year = rrule->until_year;
	_todo->until_month = rrule->until_month;
	_todo->until_mday = rrule->until_mday;
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

void _cal_db_rrule_get_rrule_from_todo(calendar_record_h todo, cal_rrule_s **rrule)
{
	cal_rrule_s *_rrule;
	cal_todo_s *_todo;

	retm_if(todo == NULL, "Invalid argument: rrule is NULL");

	_todo = (cal_todo_s *)todo;

	_rrule = calloc(1, sizeof(cal_rrule_s));
	retm_if(_rrule == NULL,	"Failed to calloc");

	_rrule->freq = _todo->freq;
	_rrule->range_type = _todo->range_type;
	_rrule->until_type = _todo->until_type;
	_rrule->until_utime = _todo->until_utime;
	_rrule->until_year = _todo->until_year;
	_rrule->until_month = _todo->until_month;
	_rrule->until_mday = _todo->until_mday;
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
	int rrule_id;
	int index;
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	char until_datetime[32] = {0};
	sqlite3_stmt *stmt = NULL;
    cal_db_util_error_e dbret = CAL_DB_OK;

	retvm_if(rrule == NULL, CALENDAR_ERROR_INVALID_PARAMETER,
			"Invalid argument: rrule is NULL");

	snprintf(query, sizeof(query),
			"INSERT INTO %s ( "
			"event_id, freq, range_type, "
			"until_type, until_utime, until_datetime, "
			"count, interval, "
			"bysecond, byminute, byhour, byday, "
			"bymonthday, byyearday, byweekno, bymonth, "
			"bysetpos, wkst "
			") VALUES ( "
			"%d, %d, %d, "
			"%d, %lld, ?, "
			"%d, %d, "
			"?, ?, ?, ?, "
			"?, ?, ?, ?, "
			"?, %d "
			") ",
			CAL_TABLE_RRULE,
			id, rrule->freq, rrule->range_type,
			rrule->until_type, rrule->until_type == CALENDAR_TIME_UTIME ? rrule->until_utime : 0,
			rrule->count, rrule->interval,
			rrule->wkst);

	DBG("[%s]", query);
	stmt = _cal_db_util_query_prepare(query);
	retvm_if(stmt == NULL, CALENDAR_ERROR_DB_FAILED, "Failed to query prepare");

	index = 1;

	if (CALENDAR_TIME_LOCALTIME == rrule->until_type)
	{
		snprintf(until_datetime, sizeof(until_datetime), "%04d%02d%02d",
				rrule->until_year, rrule->until_month, rrule->until_mday);
		_cal_db_util_stmt_bind_text(stmt, index, until_datetime);
	}
	index++;

	if (rrule->bysecond)
		_cal_db_util_stmt_bind_text(stmt, index, rrule->bysecond);
	index++;

	if (rrule->byminute)
		_cal_db_util_stmt_bind_text(stmt, index, rrule->byminute);
	index++;

	if (rrule->byhour)
		_cal_db_util_stmt_bind_text(stmt, index, rrule->byhour);
	index++;

	if (rrule->byday)
		_cal_db_util_stmt_bind_text(stmt, index, rrule->byday);
	index++;

	if (rrule->bymonthday)
		_cal_db_util_stmt_bind_text(stmt, index, rrule->bymonthday);
	index++;

	if (rrule->byyearday)
		_cal_db_util_stmt_bind_text(stmt, index, rrule->byyearday);
	index++;

	if (rrule->byweekno)
		_cal_db_util_stmt_bind_text(stmt, index, rrule->byweekno);
	index++;

	if (rrule->bymonth)
		_cal_db_util_stmt_bind_text(stmt, index, rrule->bymonth);
	index++;

	if (rrule->bysetpos)
		_cal_db_util_stmt_bind_text(stmt, index, rrule->bysetpos);
	index++;

	dbret = _cal_db_util_stmt_step(stmt);
	sqlite3_finalize(stmt);
	if (CAL_DB_OK != dbret)
	{
		ERR("_cal_db_util_stmt_step() Failed(%d)", dbret);
		switch (dbret)
		{
		case CAL_DB_ERROR_NO_SPACE:
			return CALENDAR_ERROR_FILE_NO_SPACE;
		default:
			return CALENDAR_ERROR_DB_FAILED;
		}
	}

	rrule_id = _cal_db_util_last_insert_id();
	DBG("rrule_id(%d)", rrule_id);
	return CALENDAR_ERROR_NONE;
}

int _cal_db_rrule_get_rrule(int id, cal_rrule_s **rrule)
{
	char query[CAL_DB_SQL_MAX_LEN];
	int index;
	sqlite3_stmt *stmt = NULL;
	cal_rrule_s *_rrule = NULL;
	const unsigned char *temp;
	cal_db_util_error_e dbret = CAL_DB_OK;

	_rrule = calloc(1, sizeof(cal_rrule_s));
	retvm_if(_rrule == NULL, CALENDAR_ERROR_OUT_OF_MEMORY,
			"Failed to calloc");

	snprintf(query, sizeof(query),
			"SELECT * FROM %s "
			"WHERE event_id = %d ",
			CAL_TABLE_RRULE,
			id);

	stmt = _cal_db_util_query_prepare(query);
	if (NULL == stmt)
	{
		ERR("_cal_db_util_query_prepare() Failed");
		CAL_FREE(_rrule);
		return CALENDAR_ERROR_DB_FAILED;
	}

	dbret = _cal_db_util_stmt_step(stmt);
	if (dbret != CAL_DB_ROW) {
		ERR("Failed to step stmt(%d)", dbret);
		sqlite3_finalize(stmt);
		CAL_FREE(_rrule);
		switch (dbret)
		{
		case CAL_DB_ERROR_NO_SPACE:
			return CALENDAR_ERROR_FILE_NO_SPACE;
		default:
			return CALENDAR_ERROR_DB_FAILED;
		}
	}

	index = 0;
	sqlite3_column_int(stmt, index++); // id
	sqlite3_column_int(stmt, index++); // event_id
	_rrule->freq = sqlite3_column_int(stmt, index++);

	//rrule->_rrule_id = 1;
	_rrule->range_type = sqlite3_column_int(stmt, index++);
	_rrule->until_type = sqlite3_column_int(stmt, index++);
	_rrule->until_utime = sqlite3_column_int64(stmt, index++);

	temp = sqlite3_column_text(stmt, index++);
	if (CALENDAR_TIME_LOCALTIME == _rrule->until_type)
	{
		sscanf((const char *)temp, "%04d%02d%02d",
				&_rrule->until_year, &_rrule->until_month, &_rrule->until_mday);
	}

	_rrule->count = sqlite3_column_int(stmt, index++);
	_rrule->interval = sqlite3_column_int(stmt, index++);

	temp = sqlite3_column_text(stmt, index++);
	_rrule->bysecond = SAFE_STRDUP(temp);

	temp = sqlite3_column_text(stmt, index++);
	_rrule->byminute = SAFE_STRDUP(temp);

	temp = sqlite3_column_text(stmt, index++);
	_rrule->byhour = SAFE_STRDUP(temp);

	temp = sqlite3_column_text(stmt, index++);
	_rrule->byday= SAFE_STRDUP(temp);

	temp = sqlite3_column_text(stmt, index++);
	_rrule->bymonthday= SAFE_STRDUP(temp);

	temp = sqlite3_column_text(stmt, index++);
	_rrule->byyearday= SAFE_STRDUP(temp);

	temp = sqlite3_column_text(stmt, index++);
	_rrule->byweekno= SAFE_STRDUP(temp);

	temp = sqlite3_column_text(stmt, index++);
	_rrule->bymonth= SAFE_STRDUP(temp);

	temp = sqlite3_column_text(stmt, index++);
	_rrule->bysetpos = SAFE_STRDUP(temp);

	_rrule->wkst = sqlite3_column_int(stmt, index++);

	sqlite3_finalize(stmt);

	*rrule = _rrule;
	return CALENDAR_ERROR_NONE;
}

int _cal_db_rrule_update_record(int id, cal_rrule_s *rrule)
{
	int dbret;
	int index;
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	char until_datetime[32] = {0};
	sqlite3_stmt *stmt = NULL;

	retvm_if(rrule == NULL, CALENDAR_ERROR_INVALID_PARAMETER,
			"Invalid argument: rrule is NULL");

	if (rrule->freq == CALENDAR_RECURRENCE_NONE)
	{
		DBG("freq is NONE");
		return CALENDAR_ERROR_NONE;
	}

	DBG("freq exist, so update rrule");
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
			rrule->until_type,
			rrule->until_type == CALENDAR_TIME_UTIME ? rrule->until_utime : 0,
			rrule->count,
			rrule->interval,
			rrule->wkst,
			id);

	DBG("query[%s]", query);
	stmt = _cal_db_util_query_prepare(query);
	retvm_if(stmt == NULL, CALENDAR_ERROR_DB_FAILED,
			"_cal_db_util_query_prepare() Failed");

	index = 1;

	if (CALENDAR_TIME_LOCALTIME == rrule->until_type)
	{
		snprintf(until_datetime, sizeof(until_datetime), "%04d%02d%02d",
				rrule->until_year, rrule->until_month, rrule->until_mday);
		_cal_db_util_stmt_bind_text(stmt, index, until_datetime);
	}
	index++;

	if (rrule->bysecond)
		_cal_db_util_stmt_bind_text(stmt, index, rrule->bysecond);
	index++;

	if (rrule->byminute)
		_cal_db_util_stmt_bind_text(stmt, index, rrule->byminute);
	index++;

	if (rrule->byhour)
		_cal_db_util_stmt_bind_text(stmt, index, rrule->byhour);
	index++;

	if (rrule->byday)
		_cal_db_util_stmt_bind_text(stmt, index, rrule->byday);
	index++;

	if (rrule->bymonthday)
		_cal_db_util_stmt_bind_text(stmt, index, rrule->bymonthday);
	index++;

	if (rrule->byyearday)
		_cal_db_util_stmt_bind_text(stmt, index, rrule->byyearday);
	index++;

	if (rrule->byweekno)
		_cal_db_util_stmt_bind_text(stmt, index, rrule->byweekno);
	index++;

	if (rrule->bymonth)
		_cal_db_util_stmt_bind_text(stmt, index, rrule->bymonth);
	index++;

	if (rrule->bysetpos)
		_cal_db_util_stmt_bind_text(stmt, index, rrule->bysetpos);
	index++;

	dbret = _cal_db_util_stmt_step(stmt);
	sqlite3_finalize(stmt);
	if (CAL_DB_DONE != dbret) {
		ERR("sqlite3_step() Failed(%d)", dbret);
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

