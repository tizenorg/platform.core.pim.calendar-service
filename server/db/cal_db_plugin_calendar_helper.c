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

#include "calendar_db.h"
#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_view.h"
#include "cal_record.h"

#include "cal_db.h"
#include "cal_db_query.h"
#include "cal_db_plugin_calendar_helper.h"
#include "cal_db_util.h"

int cal_db_delete_account(int account_id)
{
	CAL_FN_CALL();
	int ret = CALENDAR_ERROR_NONE;
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	sqlite3_stmt *stmt = NULL;
	GList *calendar_list = NULL;

	snprintf(query, sizeof(query), "SELECT id FROM %s where account_id = %d and deleted = 0",
			CAL_TABLE_CALENDAR, account_id);

	ret = cal_db_util_query_prepare(query, &stmt);
	if (CALENDAR_ERROR_NONE != ret) {
		/* LCOV_EXCL_START */
		ERR("cal_db_util_query_prepare() Fail(%d)", ret);
		SECURE("query[%s]", query);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	while (CAL_SQLITE_ROW == cal_db_util_stmt_step(stmt)) {
		int id = 0;
		id = sqlite3_column_int(stmt, 0);
		calendar_list = g_list_append(calendar_list, GINT_TO_POINTER(id));
	}

	sqlite3_finalize(stmt);
	if (calendar_list)
		DBG("calendar cnt=%d", g_list_length(calendar_list));

	ret = cal_db_util_begin_trans();
	if (CALENDAR_ERROR_NONE != ret) {
		/* LCOV_EXCL_START */
		ERR("cal_db_util_begin_trans() Fail(%d)", ret);
		g_list_free(calendar_list);
		return CALENDAR_ERROR_DB_FAILED;
		/* LCOV_EXCL_STOP */
	}

	GList* cursor = calendar_list;
	while (cursor) {
		int id = GPOINTER_TO_INT(cursor->data);

		ret = cal_db_delete_record(_calendar_book._uri, id);
		if (CALENDAR_ERROR_NONE != ret) {
		/* LCOV_EXCL_START */
			ERR("cal_db_delete_record() Fail(%d)", ret);
			SECURE("book_id(%d)", id);
		/* LCOV_EXCL_STOP */
		}
		cursor = g_list_next(cursor);
	}

	g_list_free(calendar_list);
	cal_db_util_end_trans(true);
	return CALENDAR_ERROR_NONE;
}
