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

int cal_db_instance_helper_insert_utime_instance(int event_id, long long int s, long long int e)
{
	int ret = 0;
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	snprintf(query, sizeof(query), "INSERT INTO %s (event_id, dtstart_utime, dtend_utime) "
			"VALUES (%d, %lld, %lld) ", CAL_TABLE_NORMAL_INSTANCE, event_id, s, e);

	ret = cal_db_util_query_exec(query);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_query_exec() Fail(%d)", ret);
		SECURE("[%s]", query);
		return ret;
	}
	return CALENDAR_ERROR_NONE;
}

int cal_db_instance_helper_insert_localtime_instance(int event_id, const char *s, const char *e)
{
	int ret = 0;
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	snprintf(query, sizeof(query), "INSERT INTO %s (event_id, dtstart_datetime, dtend_datetime) "
			"VALUES (%d, '%s', '%s') ", CAL_TABLE_ALLDAY_INSTANCE, event_id, s, e);

	ret = cal_db_util_query_exec(query);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_query_exec() Fail(%d)", ret);
		SECURE("[%s]", query);
		return ret;
	}
	return CALENDAR_ERROR_NONE;
}

