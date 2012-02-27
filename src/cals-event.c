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

static inline void cals_event_make_condition(int calendar_id,
		time_t start_time, time_t end_time, cal_sch_category_t category, int all_day, char *dest, int dest_size)
{
	int ret;

	ret = snprintf(dest, dest_size, "type = %d", CAL_EVENT_SCHEDULE_TYPE);

	if (calendar_id)
		ret += snprintf(dest+ret, dest_size-ret, "AND calendar_id = %d", calendar_id);

	if (0 < start_time)
		ret += snprintf(dest+ret, dest_size-ret, "AND start_date_time >= %ld", start_time);

	if (0 < end_time)
		ret += snprintf(dest+ret, dest_size-ret, "AND start_date_time <= %ld", end_time);

	if (0 < category)
		ret += snprintf(dest+ret, dest_size-ret, "AND category = %d", category);

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
 * @param[in] category #cal_sch_category_t
 * @param[in] all_day TRUE(1 or positive)/FALSE(0)/IGNORE(-1 or negative)
 * @return The count number on success, Negative value(#cal_error) on error
 */
API int calendar_svc_event_get_count(int calendar_id,
		time_t start_time, time_t end_time, cal_sch_category_t category, int all_day)
{
	char query[CALS_SQL_MIN_LEN];
	char cond[CALS_SQL_MIN_LEN];

	cals_event_make_condition(calendar_id, start_time, end_time, category, all_day,
									cond, sizeof(cond));

	snprintf(query, sizeof(query), "SELECT COUNT(*) FROM %s WHERE is_deleted = 0 AND %s",
			CALS_TABLE_SCHEDULE, cond);

	return cals_query_get_first_int_result(query);
}


API cal_iter* calendar_svc_event_get_list(int calendar_id,
	time_t start_time, time_t end_time, cal_sch_category_t category, int all_day)
{
	cal_iter *iter;
	sqlite3_stmt *stmt = NULL;
	char query[CALS_SQL_MAX_LEN];
	char cond[CALS_SQL_MIN_LEN];

	cals_event_make_condition(calendar_id, start_time, end_time, category, all_day,
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

API int calendar_svc_event_delete_all(int calendar_id)
{
	int ret;
	char query[CALS_SQL_MAX_LEN] = {0};

	sprintf(query, "UPDATE %s SET is_deleted = 1, sync_status = %d, last_modified_time = %ld "
		"WHERE type = %d and calendar_id = %d",
		CALS_TABLE_SCHEDULE, CAL_SYNC_STATUS_DELETED, time(NULL),
		CAL_EVENT_SCHEDULE_TYPE, calendar_id);

	ret = cals_begin_trans();
	retvm_if(CAL_SUCCESS != ret, ret, "cals_begin_trans() Failed(%d)", ret);

	ret = cals_alarm_remove(CALS_ALARM_REMOVE_BY_CALENDAR_ID, calendar_id);
	if (CAL_SUCCESS != ret) {
		cals_end_trans(false);
		ERR("cals_alarm_remove() Failed(%d)", ret);
		return ret;
	}

	ret = cals_query_exec(query);
	if (CAL_SUCCESS != ret) {
		cals_end_trans(false);
		ERR("cals_query_exec() Failed(%d)", ret);
		return ret;
	}
	cals_end_trans(true);

	cals_notify(CALS_NOTI_TYPE_EVENT);

	return CAL_SUCCESS;
}

API cal_iter* calendar_svc_event_get_updated_list(int calendar_id, time_t timestamp)
{
	return cals_get_updated_list(CAL_EVENT_SCHEDULE_TYPE, calendar_id, timestamp);
}


