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
#include "cals-utils.h"
#include "cals-alarm.h"
#include "cals-schedule.h"
#include "cals-instance.h"
#include "cals-time.h"

int _cals_clear_instances(int id);

static inline int _cals_insert_schedule(cal_sch_full_t *record)
{
	int ret = -1;
	char query[CALS_SQL_MAX_LEN] = {0};
	char dtstart_datetime[32] = {0};
	char dtend_datetime[32] = {0};
	sqlite3_stmt *stmt = NULL;

	retv_if(NULL == record, CAL_ERR_ARG_NULL);

	int input_ver = cals_get_next_ver();

	ret = snprintf(query, sizeof(query),
		"INSERT INTO %s ("
			"account_id, type, "
			"created_ver, changed_ver, "
			"summary, description, location, categories, exdate, "
			"missed, "
			"task_status, priority, timezone, file_id, "
			"contact_id, busy_status, sensitivity, uid, "
			"calendar_type, organizer_name, organizer_email, meeting_status, "
			"gcal_id, updated, location_type, "
			"location_summary, etag, calendar_id, sync_status, "
			"edit_uri, gevent_id, dst, original_event_id, "
			"latitude, longitude, "
			"email_id, availability, "
			"created_time, completed_time, progress, "
			"dtstart_type, dtstart_utime, dtstart_datetime, dtstart_tzid, "
			"dtend_type, dtend_utime, dtend_datetime, dtend_tzid, "
			"last_mod, rrule_id "
			") VALUES ( "
			"%d, %d, "
			"%d, %d, "
			"?, ?, ?, ?, ?, "
			"%d, "
			"%d, %d, %d, %d, "
			"%d, %d, %d, ?, "
			"%d, ?, ?, %d, "
			"?, ?, %d, "
			"?, ?, %d, %d, "
			"?, ?, %d, %d, "
			"%lf, %lf, "
			"%d, %d, "
			"strftime('%%s', 'now'), %lld, %d, "
			"%d, %lld, ?, ?, "
			"%d, %lld, ?, ?, "
			"strftime('%%s', 'now'), %d ) ",
			CALS_TABLE_SCHEDULE,
			record->account_id, record->cal_type,
			input_ver, input_ver,
			record->missed,
			record->task_status, record->priority,	record->timezone, record->file_id,
			record->contact_id, record->busy_status, record->sensitivity,
			record->calendar_type, record->meeting_status,
			record->location_type,
			record->calendar_id, record->sync_status,
			record->dst, record->original_event_id,
			record->latitude, record->longitude,
			record->email_id, record->availability,
			record->completed_time, record->progress,
			record->dtstart_type, record->dtstart_utime,
			record->dtend_type, record->dtend_utime,
			record->rrule_id);

	stmt = cals_query_prepare(query);
	retvm_if(NULL == stmt, CAL_ERR_DB_FAILED, "cals_query_prepare() Failed");

	int count = 1;

	if (record->summary)
		cals_stmt_bind_text(stmt, count, record->summary);
	count++;

	if (record->description)
		cals_stmt_bind_text(stmt, count, record->description);
	count++;

	if (record->location)
		cals_stmt_bind_text(stmt, count, record->location);
	count++;

	if (record->categories)
		cals_stmt_bind_text(stmt, count, record->categories);
	count++;

	if (record->exdate)
		cals_stmt_bind_text(stmt, count, record->exdate);
	count++;

	if (record->uid)
		cals_stmt_bind_text(stmt, count, record->uid);
	count++;

	if (record->organizer_name)
		cals_stmt_bind_text(stmt, count, record->organizer_name);
	count++;

	if (record->organizer_email)
		cals_stmt_bind_text(stmt, count, record->organizer_email);
	count++;

	if (record->gcal_id)
		cals_stmt_bind_text(stmt, count, record->gcal_id);
	count++;

	if (record->updated)
		cals_stmt_bind_text(stmt, count, record->updated);
	count++;

	if (record->location_summary)
		cals_stmt_bind_text(stmt, count, record->location_summary);
	count++;

	if (record->etag)
		cals_stmt_bind_text(stmt, count, record->etag);
	count++;

	if (record->edit_uri)
		cals_stmt_bind_text(stmt, count, record->edit_uri);
	count++;

	if (record->gevent_id)
		cals_stmt_bind_text(stmt, count, record->gevent_id);
	count++;

	snprintf(dtstart_datetime, sizeof(dtstart_datetime), "%04d%02d%02dT000000",
			record->dtstart_year, record->dtstart_month, record->dtstart_mday);
	cals_stmt_bind_text(stmt, count, dtstart_datetime);
	count++;

	if (record->dtstart_tzid)
		cals_stmt_bind_text(stmt, count, record->dtstart_tzid);
	count++;

	snprintf(dtend_datetime, sizeof(dtend_datetime), "%04d%02d%02dT235959",
			record->dtend_year, record->dtend_month, record->dtend_mday);
	cals_stmt_bind_text(stmt, count, dtend_datetime);
	count++;

	if (record->dtend_tzid)
		cals_stmt_bind_text(stmt, count, record->dtend_tzid);
	count++;

	ret = cals_stmt_step(stmt);
	if (CAL_SUCCESS != ret) {
		sqlite3_finalize(stmt);
		ERR("sqlite3_step() Failed(%d)", ret);
		return ret;
	}
	sqlite3_finalize(stmt);

	return cals_last_insert_id();
}

static inline int _cals_insert_rrule_id(int index, cal_sch_full_t *record)
{
	CALS_FN_CALL;
	int ret = -1;
	char query[CALS_SQL_MAX_LEN] = {0};

	retv_if(NULL == record, CAL_ERR_ARG_NULL);

	ret = snprintf(query, sizeof(query),
			"UPDATE %s SET "
			"rrule_id = %d "
			"WHERE id = %d ",
			CALS_TABLE_SCHEDULE,
			record->rrule_id,
			index);
DBG("query(%s)", query);
	ret = cals_query_exec(query);
	if (ret) {
		ERR("cals_query_exec() failed (%d)", ret);
		return ret;
	}
	return CAL_SUCCESS;
}

static inline int _cals_insert_rrule(int index, cal_sch_full_t *record)
{
	int ret;
	int cnt;
	char query[CALS_SQL_MAX_LEN] = {0};
	char until_datetime[32] = {0};
	sqlite3_stmt *stmt = NULL;

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
			CALS_TABLE_RRULE,
			index, record->freq, record->range_type,
			record->until_type, record->until_utime,
			record->count, record->interval,
			record->wkst);

	stmt = cals_query_prepare(query);
	retvm_if(stmt == NULL, CAL_ERR_DB_FAILED, "Failed to query prepare");

	cnt = 1;
	snprintf(until_datetime, sizeof(until_datetime), "%04d%02d%02dT235959",
			record->until_year, record->until_month, record->until_mday);
	cals_stmt_bind_text(stmt, cnt, until_datetime);
	cnt++;

	if (record->bysecond)
		cals_stmt_bind_text(stmt, cnt, record->bysecond);
	cnt++;

	if (record->byminute)
		cals_stmt_bind_text(stmt, cnt, record->byminute);
	cnt++;

	if (record->byhour)
		cals_stmt_bind_text(stmt, cnt, record->byhour);
	cnt++;

	if (record->byday)
		cals_stmt_bind_text(stmt, cnt, record->byday);
	cnt++;

	if (record->bymonthday)
		cals_stmt_bind_text(stmt, cnt, record->bymonthday);
	cnt++;

	if (record->byyearday)
		cals_stmt_bind_text(stmt, cnt, record->byyearday);
	cnt++;

	if (record->byweekno)
		cals_stmt_bind_text(stmt, cnt, record->byweekno);
	cnt++;

	if (record->bymonth)
		cals_stmt_bind_text(stmt, cnt, record->bymonth);
	cnt++;

	if (record->bysetpos)
		cals_stmt_bind_text(stmt, cnt, record->bysetpos);
	cnt++;

	ret = cals_stmt_step(stmt);
	if (CAL_SUCCESS != ret) {
		sqlite3_finalize(stmt);
		ERR("sqlite3_step() Failed(%d)", ret);
		return ret;
	}
	sqlite3_finalize(stmt);

	return cals_last_insert_id();
}

int cals_insert_schedule(cal_sch_full_t *sch_record)
{
	int ret = 0;
	int index = 0;
	cal_value *cvalue = NULL;
	bool is_success = false;
	struct cals_time st;
	struct cals_time et;

	retvm_if(NULL == sch_record, CAL_ERR_ARG_INVALID, "sch_record is NULL");

	sch_record->missed = 0;

	ret = cals_begin_trans();
	retvm_if(ret, ret, "cals_begin_trans() is Failed(%d)", ret);

	ret = _cals_insert_schedule(sch_record);
	if(ret < CAL_SUCCESS) {
		ERR("_cals_insert_schedule() Failed(%d)", ret);
		cals_end_trans(false);
		return ret;
	}
	index = ret;

	if (sch_record->freq != CALS_FREQ_ONCE) {
		ret = _cals_insert_rrule(index, sch_record);
		if (ret < CAL_SUCCESS) {
			ERR("Failed in _cals_insert_rrule(%d)\n", ret);
			cals_end_trans(false);
			return ret;
		}
		sch_record->rrule_id = ret;
		DBG("added rrule_id(%d)", ret);
		ret = _cals_insert_rrule_id(index, sch_record);
		if (ret != CAL_SUCCESS) {
			ERR("Failed in _cals_insert_rrule_id(%d)\n", ret);
		}
		DBG("ended add");
	}

	st.type = sch_record->dtstart_type;
	if (st.type == CALS_TIME_UTIME)
		st.utime = sch_record->dtstart_utime;
	else {
		st.year = sch_record->dtstart_year;
		st.month = sch_record->dtstart_month;
		st.mday = sch_record->dtstart_mday;
	}

	et.type = sch_record->dtend_type;
	if (et.type == CALS_TIME_UTIME)
		et.utime = sch_record->dtend_utime;
	else {
		et.year = sch_record->dtend_year;
		et.month = sch_record->dtend_month;
		et.mday = sch_record->dtend_mday;
	}

	cals_instance_insert(index, &st, &et, sch_record);

	if (sch_record->attendee_list)
	{
		DBG("attendee exists");
		GList *list = g_list_first(sch_record->attendee_list);
		cal_participant_info_t *participant_info = NULL;

		while(list)
		{
			cvalue = list->data;
			if(cvalue)
			{
				participant_info = cvalue->user_data;
				if(participant_info->is_deleted==0)
				{
					ret = cal_service_add_participant_info(index, participant_info);
					warn_if(ret, "cal_service_add_participant_info() Failed(%d)", ret);
				}
			}
			list = g_list_next(list);
		}
	} else {
		DBG("No attendee exists");
	}

	if (sch_record->alarm_list)
	{
		DBG("alarm exists");
		GList *list = sch_record->alarm_list;
		cal_alarm_info_t *alarm_info = NULL;

		while (list)
		{
			cvalue = list->data;
			if (cvalue == NULL) {
				ERR("Failed to fine value");
				break;
			}

			alarm_info = cvalue->user_data;
			if (alarm_info == NULL) {
				ERR("Failed to find alarm info");
				break;
			}

			if(alarm_info->is_deleted == 0)
			{
				DBG("type(%d) tick(%d) unit(%d)",
						sch_record->cal_type,
						alarm_info->remind_tick,
						alarm_info->remind_tick_unit);
				if (alarm_info->remind_tick != CALS_INVALID_ID)
				{
					switch (sch_record->cal_type) {
					case CALS_SCH_TYPE_EVENT:
						ret = cals_alarm_add(index, alarm_info, &st);
						warn_if(CAL_SUCCESS != ret, "cals_alarm_add() Failed(%d)", ret);
						break;
					case CALS_SCH_TYPE_TODO:
						if (sch_record->dtend_utime == CALS_TODO_NO_DUE_DATE) {
							DBG("no due date is set");
							break;
						}
						ret = cals_alarm_add(index, alarm_info, &et);
						warn_if(CAL_SUCCESS != ret, "cals_alarm_add() Failed(%d)", ret);
						break;
					}
				}
			}
			list = list->next;
		}
	} else {
		DBG("No alarm exists");
	}

	cals_end_trans(true);
	sch_record->index = index;

	if(sch_record->cal_type == CALS_SCH_TYPE_EVENT)
		is_success= cals_notify(CALS_NOTI_TYPE_EVENT);
	else
		is_success= cals_notify(CALS_NOTI_TYPE_TODO);
	warn_if(is_success != CAL_SUCCESS, "cals_notify() Failed");

	return index;
}

static int _cals_delete_participant_info(const int index)
{
	int ret;
	char query[CALS_SQL_MIN_LEN];

	sprintf(query, "DELETE FROM %s WHERE event_id = %d", CALS_TABLE_PARTICIPANT, index);

	ret = cals_query_exec(query);
	retvm_if(CAL_SUCCESS != ret, ret, "cals_query_exec() Failed(%d)", ret);

	return CAL_SUCCESS;
}


static inline int _cals_update_schedule(const int index, cal_sch_full_t *current_record)
{
	int ret = -1;
	char query[CALS_SQL_MAX_LEN] = {0};
	char dtstart_datetime[32] = {0};
	char dtend_datetime[32] = {0};
	sqlite3_stmt *stmt = NULL;

	retv_if(NULL == current_record, CAL_ERR_ARG_NULL);

	if (CAL_SYNC_STATUS_UPDATED != current_record->sync_status)
		current_record->sync_status = CAL_SYNC_STATUS_UPDATED;

	snprintf(query, sizeof(query), "UPDATE %s SET "
			"changed_ver = %d,"
			"type = %d,"
			"summary = ?,"
			"description = ?,"
			"location = ?,"
			"categories = ?,"
			"exdate = ?,"
			"missed = %d,"
			"task_status = %d,"
			"priority = %d,"
			"timezone = %d, "
			"file_id = %d, "
			"contact_id = %d, "
			"busy_status = %d, "
			"sensitivity = %d, "
			"uid = ?, "
			"calendar_type = %d, "
			"organizer_name = ?, "
			"organizer_email = ?, "
			"meeting_status = %d, "
			"gcal_id = ?, "
			"updated = ?, "
			"location_type = %d, "
			"location_summary = ?, "
			"etag = ?, "
			"calendar_id = %d, "
			"sync_status = %d, "
			"edit_uri = ?, "
			"gevent_id = ?, "
			"dst = %d,"
			"original_event_id = %d,"
			"latitude = %lf,"
			"longitude = %lf,"
			"email_id = %d,"
			"availability = %d,"
			"completed_time = %lld,"
			"progress = %d, "
			"dtstart_type = %d, "
			"dtstart_utime = %lld, "
			"dtstart_datetime = ?, "
			"dtstart_tzid = ?, "
			"dtend_type = %d, "
			"dtend_utime = %lld, "
			"dtend_datetime = ?, "
			"dtend_tzid = ?, "
			"last_mod = strftime('%%s', 'now')"
			"WHERE id = %d;",
		CALS_TABLE_SCHEDULE,
		cals_get_next_ver(),
		current_record->cal_type,
		current_record->missed,
		current_record->task_status,
		current_record->priority,
		current_record->timezone,
		current_record->file_id,
		current_record->contact_id,
		current_record->busy_status,
		current_record->sensitivity,
		current_record->calendar_type,
		current_record->meeting_status,
		current_record->location_type,
		current_record->calendar_id,
		current_record->sync_status,
		current_record->dst,
		current_record->original_event_id,
		current_record->latitude,
		current_record->longitude,
		current_record->email_id,
		current_record->availability,
		current_record->completed_time,
		current_record->progress,
		current_record->dtstart_type,
		current_record->dtstart_utime,
		current_record->dtend_type,
		current_record->dtend_utime,
		index);

	stmt = cals_query_prepare(query);
	retvm_if(NULL == stmt, CAL_ERR_DB_FAILED, "cals_query_prepare() Failed");

	int count = 1;

	if (current_record->summary)
		cals_stmt_bind_text(stmt, count, current_record->summary);
	count++;

	if (current_record->description)
		cals_stmt_bind_text(stmt, count, current_record->description);
	count++;

	if (current_record->location)
		cals_stmt_bind_text(stmt, count, current_record->location);
	count++;

	if (current_record->categories)
		cals_stmt_bind_text(stmt, count, current_record->categories);
	count++;

	if (current_record->exdate)
		cals_stmt_bind_text(stmt, count, current_record->exdate);
	count++;

	if (current_record->uid)
		cals_stmt_bind_text(stmt, count, current_record->uid);
	count++;

	if (current_record->organizer_name)
		cals_stmt_bind_text(stmt, count, current_record->organizer_name);
	count++;

	if (current_record->organizer_email)
		cals_stmt_bind_text(stmt, count, current_record->organizer_email);
	count++;

	if (current_record->gcal_id)
		cals_stmt_bind_text(stmt, count, current_record->gcal_id);
	count++;

	if (current_record->updated)
		cals_stmt_bind_text(stmt, count, current_record->updated);
	count++;

	if (current_record->location_summary)
		cals_stmt_bind_text(stmt, count, current_record->location_summary);
	count++;

	if (current_record->etag)
		cals_stmt_bind_text(stmt, count, current_record->etag);
	count++;

	if (current_record->edit_uri)
		cals_stmt_bind_text(stmt, count, current_record->edit_uri);
	count++;

	if (current_record->gevent_id)
		cals_stmt_bind_text(stmt, count, current_record->gevent_id);
	count++;

	snprintf(dtstart_datetime, sizeof(dtstart_datetime), "%04d%02d%02d",
			current_record->dtstart_year,
			current_record->dtstart_month,
			current_record->dtstart_mday);
	cals_stmt_bind_text(stmt, count, dtstart_datetime);
	count++;

	if (current_record->dtstart_tzid)
		cals_stmt_bind_text(stmt, count, current_record->dtstart_tzid);
	count++;

	snprintf(dtend_datetime, sizeof(dtend_datetime), "%04d%02d%02d",
			current_record->dtend_year,
			current_record->dtend_month,
			current_record->dtend_mday);
	cals_stmt_bind_text(stmt, count, dtend_datetime);
	count++;

	if (current_record->dtend_tzid)
		cals_stmt_bind_text(stmt, count, current_record->dtend_tzid);
	count++;

	ret = cals_stmt_step(stmt);
	if (CAL_SUCCESS != ret) {
		sqlite3_finalize(stmt);
		ERR("sqlite3_step() Failed(%d)", ret);
		return ret;
	}
	sqlite3_finalize(stmt);

	return CAL_SUCCESS;
}

static inline int _cals_update_rrule(const int index, cal_sch_full_t *record)
{
	int ret;
	int cnt;
	char query[CALS_SQL_MAX_LEN] = {0};
	char until_datetime[32] = {0};
	sqlite3_stmt *stmt = NULL;

	retv_if(record == NULL, CAL_ERR_ARG_NULL);

	snprintf(query, sizeof(query), "UPDATE %s set "
			"freq = %d, "
			"range_type = %d, "
			"until_type = %d, "
			"until_utime = %lld, "
			"until_datetime = ?, "
			"count = %d, "
			"interval = %d, "
			"bysecond = ?, "
			"byminute = ?, "
			"byhour = ?, "
			"byday = ?, "
			"bymonthday = ?, "
			"byyearday = ?, "
			"byweekno = ?, "
			"bymonth = ?, "
			"bysetpos = ?, "
			"wkst = %d "
			"WHERE event_id = %d",
			CALS_TABLE_RRULE,
			record->freq,
			record->range_type,
			record->until_type,
			record->until_utime,
			record->count,
			record->interval,
			record->wkst,
			index);
	stmt = cals_query_prepare(query);
	retvm_if(stmt == NULL, CAL_ERR_DB_FAILED, "Failed query prepare");

	cnt = 1;
	snprintf(until_datetime, sizeof(until_datetime), "%04d%02d%02dT235959",
			record->until_year, record->until_month, record->until_mday);
	cals_stmt_bind_text(stmt, cnt, until_datetime);
	cnt++;

	if (record->bysecond)
		cals_stmt_bind_text(stmt, cnt, record->bysecond);
	cnt++;

	if (record->byminute)
		cals_stmt_bind_text(stmt, cnt, record->byminute);
	cnt++;

	if (record->byhour)
		cals_stmt_bind_text(stmt, cnt, record->byhour);
	cnt++;

	if (record->byday)
		cals_stmt_bind_text(stmt, cnt, record->byday);
	cnt++;

	if (record->bymonthday)
		cals_stmt_bind_text(stmt, cnt, record->bymonthday);
	cnt++;

	if (record->byyearday)
		cals_stmt_bind_text(stmt, cnt, record->byyearday);
	cnt++;

	if (record->byweekno)
		cals_stmt_bind_text(stmt, cnt, record->byweekno);
	cnt++;

	if (record->bymonth)
		cals_stmt_bind_text(stmt, cnt, record->bymonth);
	cnt++;

	if (record->bysetpos)
		cals_stmt_bind_text(stmt, cnt, record->bysetpos);
	cnt++;

	ret = cals_stmt_step(stmt);
	if (CAL_SUCCESS != ret) {
		sqlite3_finalize(stmt);
		ERR("sqlite3_step() Failed(%d)", ret);
		return ret;
	}
	sqlite3_finalize(stmt);

	return CAL_SUCCESS;
}


int cals_update_schedule(const int index, cal_sch_full_t *sch_record)
{
	bool is_success = false;
	cal_value * cvalue = NULL;
	int ret = 0;

	retv_if(NULL == sch_record, CAL_ERR_ARG_NULL);

	sch_record->missed = 0;

	ret = _cals_update_schedule(index, sch_record);
	retvm_if(CAL_SUCCESS != ret, ret, "_cals_update_schedule() Failed(%d)", ret);

	ret = _cals_update_rrule(index, sch_record);
	retvm_if(CAL_SUCCESS != ret, ret, "Failed in update rrule(%d)", ret);

	_cals_delete_participant_info(index);
	if (sch_record->attendee_list)
	{
		GList *list = g_list_first(sch_record->attendee_list);
		cal_participant_info_t* participant_info = NULL;

		while(list)
		{
			cvalue = (cal_value *)list->data;
			participant_info = (cal_participant_info_t*)cvalue->user_data;

			if (0 == participant_info->is_deleted) {
				ret = cal_service_add_participant_info(index, participant_info);
				warn_if(ret, "cal_service_add_participant_info() Failed(%d)", ret);
			}

			list = g_list_next(list);
		}
	}

	/* delete registered alarm */
	cals_alarm_remove(CALS_ALARM_REMOVE_BY_EVENT_ID, index);
	if (sch_record->alarm_list)
	{
		GList *list = sch_record->alarm_list;
		cal_alarm_info_t *alarm_info = NULL;
		struct cals_time dtstart;

		while (list)
		{
			cvalue = (cal_value *)list->data;
			alarm_info = (cal_alarm_info_t*)cvalue->user_data;

			if (alarm_info->is_deleted==0) {
				if (alarm_info->remind_tick != CALS_INVALID_ID) {
					dtstart.type = sch_record->dtstart_type;
					dtstart.utime = sch_record->dtstart_utime;
					dtstart.year = sch_record->dtstart_year;
					dtstart.month = sch_record->dtstart_month;
					dtstart.mday = sch_record->dtstart_mday;
					ret = cals_alarm_add(index, alarm_info, &dtstart);
					warn_if(CAL_SUCCESS != ret, "cals_alarm_add() Failed(%d)", ret);
				}
			}

			list = g_list_next(list);
		}
	}

	/* TODO: re register alarm */

	/* clear instance */
	ret = _cals_clear_instances(index);
	if (ret) {
		ERR("_cals_clear_instances failed (%d)", ret);
		return ret;
	}

	/* insert instance */
	struct cals_time st;
	struct cals_time et;

	st.type = sch_record->dtstart_type;
	if (st.type == CALS_TIME_UTIME)
		st.utime = sch_record->dtstart_utime;
	else {
		st.year = sch_record->dtstart_year;
		st.month = sch_record->dtstart_month;
		st.mday = sch_record->dtstart_mday;
	}

	et.type = sch_record->dtend_type;
	if (et.type == CALS_TIME_UTIME)
		et.utime = sch_record->dtend_utime;
	else {
		et.year = sch_record->dtend_year;
		et.month = sch_record->dtend_month;
		et.mday = sch_record->dtend_mday;
	}

	cals_instance_insert(index, &st, &et, sch_record);

	/* set notify */
	if(sch_record->cal_type == CALS_SCH_TYPE_EVENT)
		is_success= cals_notify(CALS_NOTI_TYPE_EVENT);
	else
		is_success= cals_notify(CALS_NOTI_TYPE_TODO);

	return CAL_SUCCESS;
}

int _get_sch_basic_info(int id, int *cal_id, int *sch_type, int *acc_id)
{
	int r;
	sqlite3_stmt *stmt;
	char query[CALS_SQL_MAX_LEN];

	snprintf(query, sizeof(query), "SELECT calendar_id, type, "
			"account_id FROM %s WHERE id = %d",
			CALS_TABLE_SCHEDULE, id);

	stmt = cals_query_prepare(query);

	if (!stmt) {
		ERR("cals_query_prepare failed");
		return CAL_ERR_DB_FAILED;
	}

	r = cals_stmt_step(stmt);

	if (r < 0) {
		sqlite3_finalize(stmt);
		ERR("cals_stmt_step failed (%d)", r);
		return r;
	}

	if (r == CAL_SUCCESS) {
		ERR("cals_stmt_step return no data");
		sqlite3_finalize(stmt);
		return CAL_ERR_NO_DATA;
	}

	*cal_id = sqlite3_column_int(stmt, 0);
	*sch_type = sqlite3_column_int(stmt, 1);
	*acc_id = sqlite3_column_int(stmt, 2);

	sqlite3_finalize(stmt);

	return CAL_SUCCESS;
}

int _cals_update_deleted_table(int id)
{
	int r;
	char query[CALS_SQL_MAX_LEN];

	snprintf(query, sizeof(query), "INSERT INTO %s "
				"SELECT id, type, calendar_id, %d FROM %s "
				"WHERE id = %d",
				CALS_TABLE_DELETED,
				cals_get_next_ver(), CALS_TABLE_SCHEDULE,
				id);

	r = cals_query_exec(query);
	if (r) {
		ERR("cals_query_exec() failed (%d)", r);
		return r;
	}
	return CAL_SUCCESS;
}

int _cals_delete_schedule(int id)
{
	int r;
	char query[CALS_SQL_MAX_LEN];

	snprintf(query, sizeof(query), "DELETE FROM %s WHERE id = %d",
			CALS_TABLE_SCHEDULE, id);

	r = cals_query_exec(query);
	if (r) {
		ERR("cals_query_exec() failed (%d)", r);
		return r;
	}
	return CAL_SUCCESS;
}

int _cals_clear_instances(int id)
{
	int r;
	char query[CALS_SQL_MAX_LEN];

	snprintf(query, sizeof(query), "DELETE FROM %s WHERE event_id = %d ",
			CALS_TABLE_NORMAL_INSTANCE, id);

	r = cals_query_exec(query);
	if (r) {
		ERR("cals_query_exec() failed (%d)", r);
		return r;
	}

	snprintf(query, sizeof(query), "DELETE FROM %s WHERE event_id = %d ",
			CALS_TABLE_ALLDAY_INSTANCE, id);

	r = cals_query_exec(query);
	if (r) {
		ERR("cals_query_exec() failed (%d)", r);
		return r;
	}

	return CAL_SUCCESS;
}

int _cals_delete_rrule(int id)
{
	int r;
	char query[CALS_SQL_MAX_LEN];

	snprintf(query, sizeof(query), "DELETE FROM %s WHERE event_id = %d",
			CALS_TABLE_RRULE, id);

	r = cals_query_exec(query);
	if (r) {
		ERR("cals_query_exec() failed (%d)", r);
		return r;
	}

	return CAL_SUCCESS;
}

int _cals_mark_delete_schedule(int id)
{
	int r;
	char query[CALS_SQL_MAX_LEN];

	snprintf(query, sizeof(query), "UPDATE %s "
			"SET is_deleted = 1, changed_ver = %d, "
			"last_mod = strftime('%%s','now') WHERE id = %d",
			CALS_TABLE_SCHEDULE, cals_get_next_ver(), id);

	r = cals_query_exec(query);
	if (r) {
		ERR("cals_query_exec() failed (%d)", r);
		return r;
	}

	return CAL_SUCCESS;
}

int cals_delete_schedule(int id)
{
	int r;
	int cal_id;
	int sch_type;
	int acc_id;

	r = _get_sch_basic_info(id, &cal_id, &sch_type, &acc_id);
	if (r) {
		ERR("_get_sch_basic_info failed (%d)", r);
		return r;
	}

	if (acc_id == LOCAL_ACCOUNT_ID) {
		_cals_update_deleted_table(id);
		if (r) {
			ERR("_cals_update_deleted_table failed (%d)", r);
			return r;
		}
		r = _cals_delete_schedule(id);
		if (r) {
			ERR("_cals_delete_schedule failed (%d)", r);
			return r;
		}
		_cals_delete_participant_info(id);
		if (r) {
			ERR("_cals_delete_participant_info failed (%d)", r);
			return r;
		}
		_cals_delete_rrule(id);
		if (r) {
			ERR("_cals_delete_rrule failed (%d)", r);
			return r;
		}
	}
	else {
		r = _cals_mark_delete_schedule(id);
		if (r) {
			ERR("_cals_mark_delete_schedule failed (%d)", r);
			return r;
		}
	}

	r = _cals_clear_instances(id);
	if (r) {
		ERR("_cals_clear_instances failed (%d)", r);
		return r;
	}

	r = cals_alarm_remove(CALS_ALARM_REMOVE_BY_EVENT_ID, id);
	if (r) {
		ERR("cals_alarm_remove() failed(%d)", r);
		return r;
	}

	if(sch_type == CALS_SCH_TYPE_EVENT)
		r = cals_notify(CALS_NOTI_TYPE_EVENT);
	else
		r = cals_notify(CALS_NOTI_TYPE_TODO);

	if (r)
		WARN("cals_notify failed (%d)", r);

	return CAL_SUCCESS;
}

int cals_rearrange_schedule_field(const char *src, char *dest, int dest_size)
{
	int ret = 0;
	if (strstr(src, CAL_VALUE_INT_INDEX))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_INT_INDEX);

	if (strstr(src,CAL_VALUE_INT_ACCOUNT_ID))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_INT_ACCOUNT_ID);

	if (strstr(src,CAL_VALUE_INT_TYPE))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_INT_TYPE);

	if (strstr(src,CAL_VALUE_TXT_SUMMARY))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_TXT_SUMMARY);

	if (strstr(src,CAL_VALUE_TXT_DESCRIPTION))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_TXT_DESCRIPTION);

	if (strstr(src,CAL_VALUE_TXT_LOCATION))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_TXT_LOCATION);

	if (strstr(src, CAL_VALUE_TXT_CATEGORIES))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_TXT_CATEGORIES);

	if(strstr(src,CAL_VALUE_INT_MISSED))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_INT_MISSED);

	if(strstr(src,CAL_VALUE_INT_TASK_STATUS))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_INT_TASK_STATUS);

	if(strstr(src,CAL_VALUE_INT_PRIORITY))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_INT_PRIORITY);

	if(strstr(src,CAL_VALUE_INT_TIMEZONE))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_INT_TIMEZONE);

	if(strstr(src,CAL_VALUE_INT_FILE_ID))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_INT_FILE_ID);

	if(strstr(src,CAL_VALUE_INT_CONTACT_ID))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_INT_CONTACT_ID);

	if(strstr(src,CAL_VALUE_INT_BUSY_STATUS))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_INT_BUSY_STATUS);

	if(strstr(src,CAL_VALUE_INT_SENSITIVITY))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_INT_SENSITIVITY);

	if(strstr(src,CAL_VALUE_TXT_UID))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_TXT_UID);

	if(strstr(src,CAL_VALUE_INT_CALENDAR_TYPE))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_INT_CALENDAR_TYPE);

	if(strstr(src,CAL_VALUE_TXT_ORGANIZER_NAME))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_TXT_ORGANIZER_NAME);

	if(strstr(src,CAL_VALUE_TXT_ORGANIZER_EMAIL))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_TXT_ORGANIZER_EMAIL);

	if(strstr(src, CAL_VALUE_INT_MEETING_STATUS))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_INT_MEETING_STATUS);

	if(strstr(src,CAL_VALUE_TXT_GCAL_ID))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_TXT_GCAL_ID);

	if(strstr(src,CAL_VALUE_INT_DELETED))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_INT_DELETED);

	if(strstr(src,CAL_VALUE_TXT_UPDATED))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_TXT_UPDATED);

	if(strstr(src,CAL_VALUE_INT_LOCATION_TYPE))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_INT_LOCATION_TYPE);

	if(strstr(src,CAL_VALUE_TXT_LOCATION_SUMMARY))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_TXT_LOCATION_SUMMARY);

	if(strstr(src,CAL_VALUE_TXT_ETAG))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_TXT_ETAG);

	if(strstr(src,CAL_VALUE_INT_CALENDAR_ID))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_INT_CALENDAR_ID);

	if(strstr(src,CAL_VALUE_INT_SYNC_STATUS))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_INT_SYNC_STATUS);

	if(strstr(src,CAL_VALUE_TXT_EDIT_URL))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_TXT_EDIT_URL);

	if(strstr(src,CAL_VALUE_TXT_GEDERID))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_TXT_GEDERID);

	if(strstr(src,CAL_VALUE_INT_DST))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_INT_DST);

	if(strstr(src,CAL_VALUE_INT_ORIGINAL_EVENT_ID))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_INT_ORIGINAL_EVENT_ID);

	if(strstr(src,CAL_VALUE_DBL_LATITUDE))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_DBL_LATITUDE);

	if(strstr(src,CAL_VALUE_DBL_LONGITUDE))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_DBL_LONGITUDE);

	if(strstr(src,CAL_VALUE_INT_EMAIL_ID))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_INT_EMAIL_ID);

	if(strstr(src,CAL_VALUE_INT_AVAILABILITY))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_INT_AVAILABILITY);

	if(strstr(src,CAL_VALUE_LLI_CREATED_TIME))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_LLI_CREATED_TIME);

	if(strstr(src,CAL_VALUE_LLI_COMPLETED_TIME))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_LLI_COMPLETED_TIME);

	if(strstr(src,CAL_VALUE_INT_PROGRESS))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_INT_PROGRESS);

	if(strstr(src,CAL_VALUE_INT_IS_DELETED))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_VALUE_INT_IS_DELETED);

	return CAL_SUCCESS;
}

int cals_stmt_get_filted_schedule(sqlite3_stmt *stmt,
		cal_sch_full_t *sch_record, const char *select_field)
{
	int count = 0;
	const unsigned char *temp;
	const char *start, *result;

	retv_if(NULL == stmt, CAL_ERR_ARG_NULL);
	retv_if(NULL == sch_record, CAL_ERR_ARG_NULL);
	retv_if(NULL == select_field, CAL_ERR_ARG_NULL);

	start = select_field;
	if((result = strstr(start, CAL_VALUE_INT_INDEX))) {
		sch_record->index = sqlite3_column_int(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_INT_ACCOUNT_ID))) {
		sch_record->account_id = sqlite3_column_int(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_INT_TYPE))) {
		sch_record->cal_type = sqlite3_column_int(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_TXT_SUMMARY))) {
		temp = sqlite3_column_text(stmt, count++);
		sch_record->summary = SAFE_STRDUP(temp);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_TXT_DESCRIPTION))) {
		temp = sqlite3_column_text(stmt, count++);
		sch_record->description = SAFE_STRDUP(temp);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_TXT_LOCATION))) {
		temp = sqlite3_column_text(stmt, count++);
		sch_record->location = SAFE_STRDUP(temp);
		start = result;
	}

	if((result = strstr(start, CAL_VALUE_TXT_CATEGORIES))) {
		temp = sqlite3_column_text(stmt, count++);
		sch_record->categories = SAFE_STRDUP(temp);
		start = result;
	}

	if((result = strstr(start, CAL_VALUE_TXT_EXDATE))) {
		temp = sqlite3_column_text(stmt, count++);
		sch_record->exdate = SAFE_STRDUP(temp);
		start = result;
	}
	if((result = strstr(start,CAL_VALUE_INT_MISSED))) {
		sch_record->missed = sqlite3_column_int(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_INT_TASK_STATUS))) {
		sch_record->task_status = sqlite3_column_int(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_INT_PRIORITY))) {
		sch_record->priority = sqlite3_column_int(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_INT_TIMEZONE))) {
		sch_record->timezone = sqlite3_column_int(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_INT_FILE_ID))) {
		sch_record->file_id = sqlite3_column_int(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_INT_CONTACT_ID))) {
		sch_record->contact_id = sqlite3_column_int(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_INT_BUSY_STATUS))) {
		sch_record->busy_status = sqlite3_column_int(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_INT_SENSITIVITY))) {
		sch_record->sensitivity = sqlite3_column_int(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_TXT_UID))) {
		temp = sqlite3_column_text(stmt, count++);
		sch_record->uid = SAFE_STRDUP(temp);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_INT_CALENDAR_TYPE))) {
		sch_record->calendar_type = sqlite3_column_int(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_INT_CALENDAR_TYPE))) {
		sch_record->calendar_type = sqlite3_column_int(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_TXT_ORGANIZER_NAME))) {
		temp = sqlite3_column_text(stmt, count++);
		sch_record->organizer_name = SAFE_STRDUP(temp);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_TXT_ORGANIZER_EMAIL))) {
		temp = sqlite3_column_text(stmt, count++);
		sch_record->organizer_email = SAFE_STRDUP(temp);
		start = result;
	}

	if((result = strstr(start, CAL_VALUE_INT_MEETING_STATUS))) {
		sch_record->meeting_status = sqlite3_column_int(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_TXT_GCAL_ID))) {
		temp = sqlite3_column_text(stmt, count++);
		sch_record->gcal_id = SAFE_STRDUP(temp);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_TXT_UPDATED))) {
		temp = sqlite3_column_text(stmt, count++);
		sch_record->updated = SAFE_STRDUP(temp);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_INT_LOCATION_TYPE))) {
		sch_record->location_type = sqlite3_column_int(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_TXT_LOCATION_SUMMARY))) {
		temp = sqlite3_column_text(stmt, count++);
		sch_record->location_summary = SAFE_STRDUP(temp);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_TXT_ETAG))) {
		temp = sqlite3_column_text(stmt, count++);
		sch_record->etag = SAFE_STRDUP(temp);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_INT_CALENDAR_ID))) {
		sch_record->calendar_id = sqlite3_column_int(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_INT_SYNC_STATUS))) {
		sch_record->sync_status = sqlite3_column_int(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_TXT_EDIT_URL))) {
		temp = sqlite3_column_text(stmt, count++);
		sch_record->edit_uri = SAFE_STRDUP(temp);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_TXT_GEDERID))) {
		temp = sqlite3_column_text(stmt, count++);
		sch_record->gevent_id = SAFE_STRDUP(temp);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_INT_DST))) {
		sch_record->dst = sqlite3_column_int(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_INT_ORIGINAL_EVENT_ID))) {
		sch_record->original_event_id = sqlite3_column_int(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_DBL_LATITUDE))) {
		sch_record->latitude = sqlite3_column_double(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_DBL_LONGITUDE))) {
		sch_record->longitude = sqlite3_column_double(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_INT_EMAIL_ID))) {
		sch_record->email_id = sqlite3_column_int(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_INT_AVAILABILITY))) {
		sch_record->availability = sqlite3_column_int(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_LLI_CREATED_TIME))) {
		sch_record->created_time = sqlite3_column_int64(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_LLI_COMPLETED_TIME))) {
		sch_record->completed_time = sqlite3_column_int64(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_INT_PROGRESS))) {
		sch_record->progress = sqlite3_column_int(stmt, count++);
		start = result;
	}

	if((result = strstr(start,CAL_VALUE_INT_IS_DELETED))) {
		sch_record->is_deleted = sqlite3_column_int(stmt, count++);
		start = result;
	}

	return CAL_SUCCESS;
}


void cals_stmt_get_full_schedule(sqlite3_stmt *stmt,cal_sch_full_t *sch_record, bool is_utc)
{
	int count = 0;
	char *dtstart_datetime;
	char *dtend_datetime;
	char buf[8] = {0};
	const unsigned char *temp;

	sch_record->index = sqlite3_column_int(stmt, count++);
	sch_record->account_id = sqlite3_column_int(stmt, count++);
	sch_record->cal_type = sqlite3_column_int(stmt, count++);

	temp = sqlite3_column_text(stmt, count++);
	sch_record->summary = SAFE_STRDUP(temp);

	temp = sqlite3_column_text(stmt, count++);
	sch_record->description = SAFE_STRDUP(temp);

	temp = sqlite3_column_text(stmt, count++);
	sch_record->location = SAFE_STRDUP(temp);

	temp = sqlite3_column_text(stmt, count++);
	sch_record->categories = SAFE_STRDUP(temp);

	temp = sqlite3_column_text(stmt, count++);
	sch_record->exdate = SAFE_STRDUP(temp);

	sch_record->missed = sqlite3_column_int(stmt, count++);
	sch_record->task_status = sqlite3_column_int(stmt, count++);
	sch_record->priority = sqlite3_column_int(stmt, count++);
	sch_record->timezone = sqlite3_column_int(stmt, count++);
	sch_record->file_id = sqlite3_column_int(stmt, count++);
	sch_record->contact_id = sqlite3_column_int(stmt, count++);
	sch_record->busy_status = sqlite3_column_int(stmt, count++);
	sch_record->sensitivity = sqlite3_column_int(stmt, count++);

	temp = sqlite3_column_text(stmt, count++);
	sch_record->uid = SAFE_STRDUP(temp);

	sch_record->calendar_type = sqlite3_column_int(stmt, count++);

	temp = sqlite3_column_text(stmt, count++);
	sch_record->organizer_name = SAFE_STRDUP(temp);

	temp = sqlite3_column_text(stmt, count++);
	sch_record->organizer_email = SAFE_STRDUP(temp);

	sch_record->meeting_status = sqlite3_column_int(stmt, count++);

	temp = sqlite3_column_text(stmt, count++);
	sch_record->gcal_id = SAFE_STRDUP(temp);

	temp = sqlite3_column_text(stmt, count++);
	sch_record->updated = SAFE_STRDUP(temp);

	sch_record->location_type = sqlite3_column_int(stmt, count++);

	temp = sqlite3_column_text(stmt, count++);
	sch_record->location_summary = SAFE_STRDUP(temp);

	temp = sqlite3_column_text(stmt, count++);
	sch_record->etag = SAFE_STRDUP(temp);

	sch_record->calendar_id = sqlite3_column_int(stmt, count++);

	sch_record->sync_status = sqlite3_column_int(stmt, count++);

	temp = sqlite3_column_text(stmt, count++);
	sch_record->edit_uri = SAFE_STRDUP(temp);

	temp = sqlite3_column_text(stmt, count++);
	sch_record->gevent_id = SAFE_STRDUP(temp);

	sch_record->dst = sqlite3_column_int(stmt, count++);

	sch_record->original_event_id = sqlite3_column_int(stmt, count++);

	sch_record->latitude = sqlite3_column_double(stmt,count++);
	sch_record->longitude = sqlite3_column_double(stmt,count++);

	sch_record->email_id = sqlite3_column_int(stmt, count++);

	sch_record->availability = sqlite3_column_int(stmt, count++);

	sch_record->created_time = sqlite3_column_int64(stmt, count++);

	sch_record->completed_time = sqlite3_column_int64(stmt, count++);

	sch_record->progress = sqlite3_column_int(stmt,count++);


	sqlite3_column_int(stmt,count++);
	sqlite3_column_int(stmt,count++);
	sch_record->is_deleted = sqlite3_column_int(stmt,count++);

	sch_record->dtstart_type = sqlite3_column_int(stmt,count++);
	sch_record->dtstart_utime = sqlite3_column_int64(stmt,count++);
	temp = sqlite3_column_text(stmt, count++);
	if (temp) {
		dtstart_datetime = SAFE_STRDUP(temp);
		snprintf(buf, strlen("YYYY") + 1, "%s", &dtstart_datetime[0]);
		sch_record->dtstart_year =  atoi(buf);
		snprintf(buf, strlen("MM") + 1, "%s", &dtstart_datetime[4]);
		sch_record->dtstart_month = atoi(buf);
		snprintf(buf, strlen("DD") + 1, "%s", &dtstart_datetime[6]);
		sch_record->dtstart_mday = atoi(buf);
		if (dtstart_datetime) free(dtstart_datetime);
	}
	temp = sqlite3_column_text(stmt, count++);
	sch_record->dtstart_tzid = SAFE_STRDUP(temp);

	sch_record->dtend_type = sqlite3_column_int(stmt, count++);
	sch_record->dtend_utime = sqlite3_column_int64(stmt, count++);
	temp = sqlite3_column_text(stmt, count++);
	if (temp) {
		dtend_datetime = SAFE_STRDUP(temp);
		snprintf(buf, strlen("YYYY") + 1, "%s", &dtend_datetime[0]);
		sch_record->dtend_year =  atoi(buf);
		snprintf(buf, strlen("MM") + 1, "%s", &dtend_datetime[4]);
		sch_record->dtend_month = atoi(buf);
		snprintf(buf, strlen("DD") + 1, "%s", &dtend_datetime[6]);
		sch_record->dtend_mday = atoi(buf);
		if (dtend_datetime) free(dtend_datetime);
	}
	temp = sqlite3_column_text(stmt, count++);
	sch_record->dtend_tzid = SAFE_STRDUP(temp);

	sch_record->last_mod = sqlite3_column_int64(stmt,count++);
	sch_record->rrule_id = sqlite3_column_int(stmt,count++);
}

void cals_stmt_fill_rrule(sqlite3_stmt *stmt,cal_sch_full_t *sch_record)
{
	char *until_datetime;
	char buf[8] = {0};
	const unsigned char *temp;
	int count = 0;

	sch_record->rrule_id = sqlite3_column_int(stmt,count++);
	sqlite3_column_int(stmt,count++); // event_id
	sch_record->freq = sqlite3_column_int(stmt,count++);
	sch_record->range_type = sqlite3_column_int(stmt,count++);
	sch_record->until_type = sqlite3_column_int(stmt,count++);
	sch_record->until_utime = sqlite3_column_int64(stmt, count++);

	temp = sqlite3_column_text(stmt, count++);
	if (temp) {
		until_datetime = SAFE_STRDUP(temp);
		snprintf(buf, strlen("YYYY") + 1, "%s", &until_datetime[0]);
		sch_record->until_year =  atoi(buf);
		snprintf(buf, strlen("MM") + 1, "%s", &until_datetime[4]);
		sch_record->until_month = atoi(buf);
		snprintf(buf, strlen("DD") + 1, "%s", &until_datetime[6]);
		sch_record->until_mday = atoi(buf);
		if (until_datetime) free(until_datetime);
	}

	sch_record->count = sqlite3_column_int(stmt,count++);
	sch_record->interval = sqlite3_column_int(stmt,count++);

	temp = sqlite3_column_text(stmt, count++);
	sch_record->bysecond = SAFE_STRDUP(temp);

	temp = sqlite3_column_text(stmt, count++);
	sch_record->byminute = SAFE_STRDUP(temp);

	temp = sqlite3_column_text(stmt, count++);
	sch_record->byhour = SAFE_STRDUP(temp);

	temp = sqlite3_column_text(stmt, count++);
	sch_record->byday = SAFE_STRDUP(temp);

	temp = sqlite3_column_text(stmt, count++);
	sch_record->bymonthday = SAFE_STRDUP(temp);

	temp = sqlite3_column_text(stmt, count++);
	sch_record->byyearday = SAFE_STRDUP(temp);

	temp = sqlite3_column_text(stmt, count++);
	sch_record->byweekno = SAFE_STRDUP(temp);

	temp = sqlite3_column_text(stmt, count++);
	sch_record->bymonth = SAFE_STRDUP(temp);

	temp = sqlite3_column_text(stmt, count++);
	sch_record->bysetpos = SAFE_STRDUP(temp);

	sch_record->wkst = sqlite3_column_int(stmt,count++);
}

inline char *cals_strcat(char * const dest, const char *src, int bufsize)
{
	return strncat(dest, src, bufsize - strlen(dest) -1);
}

void _cals_sch_search_get_cond(int fields, char * const buf, int bufsize)
{
	int first;

	first = 1;

	if (fields & CALS_SEARCH_FIELD_SUMMARY) {
		if (first)
			first = 0;
		else
			cals_strcat(buf, "OR ", bufsize);
		cals_strcat(buf, "A.summary LIKE ('%%' || :key || '%%') ", bufsize);
	}

	if (fields & CALS_SEARCH_FIELD_DESCRIPTION) {
		if (first)
			first = 0;
		else
			cals_strcat(buf, "OR ", bufsize);
		cals_strcat(buf, "A.description LIKE ('%%' || :key || '%%') ", bufsize);
	}

	if (fields & CALS_SEARCH_FIELD_LOCATION) {
		if (first)
			first = 0;
		else
			cals_strcat(buf, "OR ", bufsize);
		cals_strcat(buf, "A.location LIKE ('%%' || :key || '%%') ", bufsize);
	}

	if (fields & CALS_SEARCH_FIELD_ATTENDEE) {
		if (first)
			first = 0;
		else
			cals_strcat(buf, "OR ", bufsize);
		cals_strcat(buf, "B.attendee_name LIKE ('%%' || :key || '%%') ", bufsize);
	}

	return;
}

int cals_sch_search(cals_sch_type sch_type, int fields, const char *keyword, cal_iter **iter)
{
	cal_iter *it;
	sqlite3_stmt *stmt;
	char query[CALS_SQL_MAX_LEN] = {0};
	char cond[CALS_SQL_MIN_LEN] = {0};

	_cals_sch_search_get_cond(fields, cond, sizeof(cond));
	snprintf(query, sizeof(query), "SELECT A.* "
			"FROM %s A LEFT JOIN %s B ON A.id = B.event_id "
			"JOIN %s C ON A.calendar_id = C.ROWID "
			"WHERE A.type = %d AND (%s) AND C.visibility = 1",
			CALS_TABLE_SCHEDULE, CALS_TABLE_PARTICIPANT, CALS_TABLE_CALENDAR, sch_type, cond);
	DBG("QUERY [%s]", query);

	stmt = cals_query_prepare(query);
	retvm_if (!stmt, CAL_ERR_DB_FAILED, "cals_query_prepare() failed");

	sqlite3_bind_text(stmt, sqlite3_bind_parameter_index(stmt, ":key"), keyword, strlen(keyword), SQLITE_TRANSIENT);

	it = calloc(1, sizeof(cal_iter));
	if (!it) {
		sqlite3_finalize(stmt);
		ERR("calloc() failed(%d)", errno);
		return CAL_ERR_OUT_OF_MEMORY;
	}

	it->i_type = CAL_STRUCT_TYPE_SCHEDULE;
	it->stmt = stmt;
	*iter = it;

	return CAL_SUCCESS;
}

API int calendar_svc_smartsearch_excl(const char *keyword, int offset, int limit, cal_iter **iter)
{
	cal_iter *it;
	sqlite3_stmt *stmt;
	char query[CALS_SQL_MAX_LEN] = {0};
	char buf[1024] = {0};

	snprintf(query, sizeof(query), "SELECT A.* "
			"FROM %s A LEFT JOIN %s B ON A.calendar_id = B.ROWID "
			"WHERE A.summary LIKE ('%%' || :key || '%%') "
			"AND B.visibility = 1 LIMIT %d OFFSET %d",
			CALS_TABLE_SCHEDULE, CALS_TABLE_CALENDAR, limit, offset);

	stmt = cals_query_prepare(query);
	retvm_if (!stmt, CAL_ERR_DB_FAILED, "cals_query_prepare() failed");

	cals_escape_like_pattern(keyword, buf, sizeof(buf));
	sqlite3_bind_text(stmt, sqlite3_bind_parameter_index(stmt, ":key"), buf, strlen(buf), SQLITE_TRANSIENT);

	it = calloc(1, sizeof(cal_iter));
	if (!it) {
		sqlite3_finalize(stmt);
		ERR("calloc() failed(%d)", errno);
		return CAL_ERR_OUT_OF_MEMORY;
	}

	it->i_type = CAL_STRUCT_TYPE_SCHEDULE;
	it->stmt = stmt;
	*iter = it;

	return CAL_SUCCESS;
}

