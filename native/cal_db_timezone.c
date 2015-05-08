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

#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_db.h"
#include "cal_db_util.h"

void cal_db_timezone_search_with_tzid(const char *zone_name, char *tzid, int *timezone_id)
{
	RET_IF(NULL == zone_name);
	RET_IF(NULL == tzid);
	RET_IF('\0' == *tzid);
	RET_IF(NULL == timezone_id);

	char query[CAL_DB_SQL_MAX_LEN] = {0};
	snprintf(query, sizeof(query), "SELECT id FROM %s WHERE standard_name='%s'", CAL_TABLE_TIMEZONE, tzid);
	cal_db_util_query_get_first_int_result(zone_name, query, NULL, timezone_id);
}
