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
#include "cals-schedule.h"

static const char *_todo_list_order[] = {
	[CALS_TODO_LIST_ORDER_END_DATE] = "dtend_utime DESC",
	[CALS_TODO_LIST_ORDER_PRIORITY] = "priority DESC, dtend_utime DESC",
	[CALS_TODO_LIST_ORDER_STATUS] = "task_status ASC, dtend_utime DESC",
};

static inline const char *cals_todo_get_order(cals_todo_list_order_t order)
{
	return _todo_list_order[order];
}

int cals_todo_init(cal_sch_full_t *sch_full_record)
{
	retvm_if(NULL == sch_full_record, CAL_ERR_ARG_INVALID , "sch_full_record is NULL");

	memset(sch_full_record,0,sizeof(cal_sch_full_t));

	sch_full_record->cal_type = CALS_SCH_TYPE_TODO;
	sch_full_record->task_status = CALS_TODO_STATUS_NONE;
	sch_full_record->calendar_id = DEFAULT_TODO_CALENDAR_ID;

	sch_full_record->index = CALS_INVALID_ID;
	sch_full_record->timezone = -1;
	sch_full_record->contact_id = CALS_INVALID_ID;
	sch_full_record->calendar_type = CAL_PHONE_CALENDAR;
	sch_full_record->attendee_list = NULL;
	sch_full_record->busy_status = 2;
	sch_full_record->summary = NULL;
	sch_full_record->description = NULL;
	sch_full_record->location= NULL;
	sch_full_record->categories = NULL;
	sch_full_record->exdate = NULL;
	sch_full_record->organizer_email = NULL;
	sch_full_record->organizer_name = NULL;
	sch_full_record->uid= NULL;
	sch_full_record->gcal_id = NULL;
	sch_full_record->location_summary = NULL;
	sch_full_record->etag = NULL;
	sch_full_record->edit_uri = NULL;
	sch_full_record->gevent_id = NULL;
	sch_full_record->original_event_id = CALS_INVALID_ID;

	sch_full_record->sync_status = CAL_SYNC_STATUS_NEW;
	sch_full_record->account_id = -1;
	sch_full_record->is_deleted = 0;
	sch_full_record->latitude = 1000; // set default 1000 out of range(-180 ~ 180)
	sch_full_record->longitude = 1000; // set default 1000 out of range(-180 ~ 180)
	sch_full_record->freq = CALS_FREQ_ONCE;

	return CAL_SUCCESS;
}


static int __todo_get_query_priority(int priority, char *query, int len)
{
	switch (priority) {
	case CALS_TODO_PRIORITY_NONE: // 0x01
		snprintf(query, len, "AND priority = %d ", 0);
		break;

	case CALS_TODO_PRIORITY_HIGH: // 0x020
		snprintf(query, len, "AND priority < %d ", CALS_TODO_PRIORITY_MID);
		break;

	case CALS_TODO_PRIORITY_MID: // 0x04
		snprintf(query, len, "AND priority = %d ", CALS_TODO_PRIORITY_MID);
		break;

	case CALS_TODO_PRIORITY_LOW: // 0x08
		snprintf(query, len, "AND priority > %d ", CALS_TODO_PRIORITY_MID);
		break;

	case 0x03: /* NONE | HIGH */
		snprintf(query, len, "AND (priority = %d OR priority < %d) ", 0, CALS_TODO_PRIORITY_MID);
		break;

	case 0x05: /* NONE | MID */
		snprintf(query, len, "AND (priority = %d OR priority = %d) ", 0, CALS_TODO_PRIORITY_MID);
		break;

	case 0x09: /* NONE | LOW */
		snprintf(query, len, "AND (priority = %d OR priority > %d) ", 0, CALS_TODO_PRIORITY_MID);
		break;

	case 0x06: /* HIGH |MID */
		snprintf(query, len, "AND priority > %d AND priority <= %d ", 0, CALS_TODO_PRIORITY_MID);
		break;

	case 0x0A: /* HIGH |LOW */
		snprintf(query, len, "AND priority > %d AND priority != %d ", 0, CALS_TODO_PRIORITY_MID);
		break;

	case 0x0C: /* MID |LOW */
		snprintf(query, len, "AND priority >= %d ", CALS_TODO_PRIORITY_MID);
		break;

	case 0x07: /* NONE | HIGH | MID */
		snprintf(query, len, "AND priority <= %d ", CALS_TODO_PRIORITY_MID);
		break;

	case 0x0B: /* NONE | HIGH | LOW */
		snprintf(query, len, "AND priority <> %d ", CALS_TODO_PRIORITY_MID);
		break;

	case 0x0D: /* NONE | MID | LOW */
		snprintf(query, len, "AND priority >= %d ", CALS_TODO_PRIORITY_MID);
		break;

	case 0x0E: /* HIGH | MID |LOW */
		snprintf(query, len, "AND priority > %d ", 0);
		break;

	case 0x0F: /* NONE | HIGH | MID | LOW */
		memset(query, 0x0, len);
	default:
		break;
	}
	return 0;
}

static int __todo_get_query_status(int status, char *query, int len)
{
	int i, conj;
	char buf[64] = {0};

	if (status) {
		if (status & 0xff) {
			DBG("check status:%x", status);
			return -1;
		}
		conj = 0;
		snprintf(query, len, "%s", "AND ( ");

		for (i = 0; i < 5; i++) {
			if (status & (1 << (i + 8))) {
				if (conj) {
					strcat(query,  "OR ");
					conj = 0;
				}

				snprintf(buf, sizeof(buf), "task_status = %d ", (1 << (i + 8)));
				strcat(query,  buf);
				conj = 1;
			}
		}
		strcat(query,  ") ");

	} else {
		memset(query, 0x0, len);

	}
	return 0;
}

/**
 * This function gets count related with todo.
 * If parameter is invalid(0, negative, etc.), it will be ignored
 * If all parameters are invalid, this function return all todo count.
 *
 * @param[in] calendar_id calendar_id
 * @param[in] start_time start time
 * @param[in] end_time end time
 * @param[in] priority priority(0~9)
 * @param[in] status #cals_status_t
 * @return The count number on success, Negative value(#cal_error) on error
 */
/*
API int calendar_svc_todo_get_count(int calendar_id,
		long long int dtstart_utime, long long int dtend_utime, int priority, cals_status_t status)
{
	char query[CALS_SQL_MIN_LEN];
	char cond[CALS_SQL_MIN_LEN];

	cals_todo_get_condition(calendar_id, start_time, end_time, priority, status, cond, sizeof(cond));

	snprintf(query, sizeof(query), "SELECT COUNT(*) FROM %s WHERE is_deleted = 0 AND %s",
			CALS_TABLE_SCHEDULE, cond);

	return cals_query_get_first_int_result(query);
}
*/
API int calendar_svc_todo_get_list(int calendar_id, long long int dtend_from, long long int dtend_to,
		int priority,cals_status_t status, cals_todo_list_order_t order, cal_iter **iter)
{
	cal_iter *it;
	sqlite3_stmt *stmt = NULL;
	char buf_calendar_id[256] = {0};
	char buf_dtend_from[256] = {0};
	char buf_dtend_to[256] = {0};
	char buf_priority[256] = {0};
	char buf_status[256] = {0};
	char query[CALS_SQL_MAX_LEN];

	retv_if(NULL == iter, CAL_ERR_ARG_NULL);
	retv_if(order < CALS_TODO_LIST_ORDER_END_DATE || order > CALS_TODO_LIST_ORDER_STATUS, CAL_ERR_ARG_INVALID);

	if (calendar_id > 0) {
		snprintf(buf_calendar_id, sizeof(buf_calendar_id), "AND calendar_id = %d ", calendar_id);
	} else {
		memset(buf_calendar_id, 0x0, sizeof(buf_calendar_id));
	}

	if (dtend_from >= 0) {
		snprintf(buf_dtend_from, sizeof(buf_dtend_from), "AND dtend_utime >= %lld ", dtend_from);
	} else {
		memset(buf_dtend_from, 0x0, sizeof(buf_dtend_from));
	}

	if (dtend_to >= 0) {
		snprintf(buf_dtend_to, sizeof(buf_dtend_to), "AND dtend_utime <= %lld ", dtend_to);
	} else {
		memset(buf_dtend_to, 0x0, sizeof(buf_dtend_to));
	}

	__todo_get_query_priority(priority, buf_priority, sizeof(buf_priority));
	__todo_get_query_status(status, buf_status, sizeof(buf_status));

	snprintf(query, sizeof(query),
			"SELECT * FROM %s "
			"WHERE is_deleted = 0 AND type = %d "
			"%s %s %s %s %s "
			"ORDER BY %s",
			CALS_TABLE_SCHEDULE,
			CAL_STRUCT_TYPE_TODO,
			buf_calendar_id, buf_dtend_from, buf_dtend_to, buf_priority, buf_status,
			cals_todo_get_order(order));

	stmt = cals_query_prepare(query);
	retvm_if(NULL == stmt, CAL_ERR_DB_FAILED, "cals_query_prepare() Failed");

	it = calloc(1, sizeof(cal_iter));
	if (NULL == it) {
		sqlite3_finalize(stmt);
		ERR("calloc() Failed(%d)", errno);
		return CAL_ERR_OUT_OF_MEMORY;
	}
	it->i_type = CAL_STRUCT_TYPE_TODO;
	it->stmt = stmt;
	*iter = it;

	return CAL_SUCCESS;
}

API int calendar_svc_todo_search(int fields, const char *keyword, cal_iter **iter)
{
	int ret;

	ret = cals_sch_search(CALS_SCH_TYPE_TODO, fields, keyword, iter);
	retvm_if(ret < 0, ret, "cals_sch_search() failed(%d)", ret);

	return CAL_SUCCESS;
}

static inline int cals_todo_get_changes(int calendar_id, int version, cal_iter *iter)
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
			"SELECT id, changed_ver, created_ver, is_deleted FROM %s "
			"WHERE changed_ver > %d AND original_event_id = %d AND type = %d %s "
			"UNION "
			"SELECT schedule_id, deleted_ver, -1, 1 FROM %s "
			"WHERE deleted_ver > %d AND schedule_type = %d %s",
			CALS_TABLE_SCHEDULE,
			version, CALS_INVALID_ID, CALS_SCH_TYPE_TODO, buf,
			CALS_TABLE_DELETED,
			version, CALS_SCH_TYPE_TODO, buf);

	DBG("query(%s)", query);
	stmt = cals_query_prepare(query);
	retvm_if (NULL == stmt, CAL_ERR_DB_FAILED, "cals_query_prepare() failed.");

	while (CAL_TRUE == cals_stmt_step(stmt)){
		result = cals_updated_schedule_add_mempool();

		result->id = sqlite3_column_int(stmt, 0);
		result->ver = sqlite3_column_int(stmt, 1);
		if (sqlite3_column_int(stmt, 3) == 1)
			result->type = CALS_UPDATED_TYPE_DELETED;
		else if (sqlite3_column_int(stmt, 2) == result->ver || version < sqlite3_column_int(stmt, 2))
			result->type = CALS_UPDATED_TYPE_INSERTED;
		else
			result->type = CALS_UPDATED_TYPE_MODIFIED;

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

API int calendar_svc_todo_get_changes(int calendar_id, int version, cal_iter **iter)
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

	ret = cals_todo_get_changes(calendar_id, version, it);
	if (ret) {
		ERR("cals_todo_get_changes() failed(%d", ret);
		free(it->info);
		free(it);
		return ret;
	}

	*iter = it;

	return CAL_SUCCESS;
}


API int calendar_svc_todo_get_iter(int calendar_id, int priority, int status,
		cals_todo_list_order_t order, cal_iter **iter)
{
	char query[CALS_SQL_MIN_LEN] = {0};
	char buf_id[64] = {0};
	char buf_prio[64] = {0};
	char buf_stat[256] = {0};
	cal_iter *it;
	sqlite3_stmt *stmt = NULL;

	it = calloc(1, sizeof(cal_iter));
	retvm_if(NULL == it, CAL_ERR_OUT_OF_MEMORY, "Failed to calloc(%d)", errno);
	it->is_patched = 0;

	if (calendar_id > 0) {
		snprintf(buf_id, sizeof(buf_id), "AND calendar_id = %d ", calendar_id);
	} else {
		memset(buf_id, 0x0, sizeof(buf_id));
	}

	/* priority */
	__todo_get_query_priority(priority, buf_prio, sizeof(buf_prio));

	/* status */
	__todo_get_query_status(status, buf_stat, sizeof(buf_stat));

	/* dtend means due */
	it->i_type = CAL_STRUCT_TYPE_TODO;
	snprintf(query, sizeof(query),
			"SELECT * FROM %s "
			"WHERE type = %d %s %s %s ",
			CALS_TABLE_SCHEDULE,
			CALS_SCH_TYPE_TODO, buf_id, buf_prio, buf_stat);
DBG("%s\n", query);
	stmt = cals_query_prepare(query);
	retvm_if(NULL == stmt, CAL_ERR_DB_FAILED, "Failed to query prepare");

	it->stmt = stmt;
	*iter = it;
	return CAL_SUCCESS;
}

API int calendar_svc_todo_get_list_by_period(int calendar_id,
		long long int due_from, long long int dueto, int priority, int status, cal_iter **iter)
{
	char query[CALS_SQL_MIN_LEN] = {0};
	char buf_id[64] = {0};
	char buf_prio[64] = {0};
	char buf_stat[256] = {0};
	cal_iter *it;
	sqlite3_stmt *stmt = NULL;

	it = calloc(1, sizeof(cal_iter));
	retvm_if(NULL == it, CAL_ERR_OUT_OF_MEMORY, "Failed to calloc(%d)", errno);
	it->is_patched = 0;

	if (calendar_id > 0) {
		snprintf(buf_id, sizeof(buf_id), "AND calendar_id = %d ", calendar_id);
	} else {
		memset(query, 0x0, sizeof(buf_id));
	}

	/* priority */
	__todo_get_query_priority(priority, buf_prio, sizeof(buf_prio));

	/* status */
	__todo_get_query_status(status, buf_stat, sizeof(buf_stat));

	/* dtend means due */
	it->i_type = CAL_STRUCT_TYPE_TODO;
	snprintf(query, sizeof(query),
			"SELECT * FROM %s "
			"WHERE dtend_utime >= %lld AND dtend_utime <= %lld "
			"AND type = %d %s %s ",
			CALS_TABLE_SCHEDULE,
			due_from, dueto,
			CALS_SCH_TYPE_TODO, buf_prio, buf_stat);
DBG("%s\n", query);
	stmt = cals_query_prepare(query);
	retvm_if(NULL == stmt, CAL_ERR_DB_FAILED, "Failed to query prepare");

	it->stmt = stmt;
	*iter = it;
	return CAL_SUCCESS;
}

API int calendar_svc_todo_get_count_by_period(int calendar_id,
		long long int due_from, long long int dueto, int priority, int status, int *count)
{
	int ret, cnt = 0;
	char query[CALS_SQL_MIN_LEN] = {0};
	char buf_prio[64] = {0};
	char buf_stat[256] = {0};

	retvm_if(priority < 0, CAL_ERR_ARG_INVALID, "Invalid argument: priorit(%d)", priority);
	retvm_if(status < 0, CAL_ERR_ARG_INVALID, "Invalid argument: status(%d)", priority);

	DBG("priority(%d) status(%d)", priority, status);
	sqlite3_stmt *stmt = NULL;

	/* priority */
	__todo_get_query_priority(priority, buf_prio, sizeof(buf_prio));

	/* status */
	__todo_get_query_status(status, buf_stat, sizeof(buf_stat));

	snprintf(query, sizeof(query),
			"SELECT count(*) FROM %s "
			"WHERE dtend_utime >= %lld AND dtend_utime <= %lld "
			"AND type = %d %s %s ",
			CALS_TABLE_SCHEDULE,
			due_from, dueto,
			CALS_SCH_TYPE_TODO, buf_prio, buf_stat);

DBG("%s\n", query);
	stmt = cals_query_prepare(query);
	retvm_if(NULL == stmt, CAL_ERR_DB_FAILED, "Failed to query prepare");

	ret = cals_stmt_step(stmt);
	if (CAL_TRUE == ret) {
		cnt = sqlite3_column_int(stmt, 0);

	} else if (CAL_SUCCESS != ret) {
		sqlite3_finalize(stmt);
		ERR("Failed to step(errno:%d)", ret);
		return ret;
	}
	sqlite3_finalize(stmt);
	*count = cnt;

	return CAL_SUCCESS;
}

