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
#include "cal_view.h"
#include "cal_record.h"

#include "cal_db.h"
#include "cal_db_util.h"
#include "cal_db_query.h"
#include "cal_db_extended.h"

int _cal_db_extended_get_records(int record_id, calendar_record_type_e record_type, GList **out_list)
{
    int ret;
    char query[CAL_DB_SQL_MAX_LEN] = {0};
    sqlite3_stmt *stmt = NULL;
    GList *list = NULL;

    retvm_if(NULL == out_list, CALENDAR_ERROR_INVALID_PARAMETER,
            "Invalid parameter: GList is NULL");

    snprintf(query, sizeof(query),
            "SELECT * FROM %s "
            "WHERE record_id = %d AND "
            "record_type = %d",
            CAL_TABLE_EXTENDED,
            record_id,
            record_type);

    stmt = _cal_db_util_query_prepare(query);
    if (NULL == stmt)
    {
        ERR("_cal_db_util_query_prepare() failed");
        return CALENDAR_ERROR_DB_FAILED;
    }

    int count = 0;
    const unsigned char *temp;
    calendar_record_h record = NULL;
    cal_extended_s *extended = NULL;

    while (CAL_DB_ROW == _cal_db_util_stmt_step(stmt))
    {
        ret = calendar_record_create(_calendar_extended_property._uri, &record);
        if (CALENDAR_ERROR_NONE != ret)
        {
            sqlite3_finalize(stmt);
            return ret;
        }

        count = 0;
        extended = (cal_extended_s *)(record);

        extended->id = sqlite3_column_int(stmt, count++);
        extended->record_id = sqlite3_column_int(stmt, count++);
        extended->record_type = sqlite3_column_int(stmt, count++);
        temp = sqlite3_column_text(stmt, count++);
        extended->key = SAFE_STRDUP(temp);
        temp = sqlite3_column_text(stmt, count++);
        extended->value = SAFE_STRDUP(temp);

        list = g_list_append(list, record);
    }
    sqlite3_finalize(stmt);

    *out_list = list;
    return CALENDAR_ERROR_NONE;
}

int _cal_db_extended_convert_gtoh(GList *glist, int record_id, calendar_record_type_e record_type, calendar_list_h *hlist)
{
    int ret;
    GList *g = NULL;
    calendar_list_h h = NULL;
    calendar_record_h extended = NULL;

    if (glist == NULL)
    {
        DBG("No attendee");
        return CALENDAR_ERROR_NO_DATA;
    }
    ret = calendar_list_create(&h);

    g = g_list_first(glist);
    while (g)
    {
        extended = (calendar_record_h)g->data;
        if (extended)
        {
            ret = _cal_record_set_int(extended,_calendar_extended_property.record_id,record_id);
            ret = _cal_record_set_int(extended,_calendar_extended_property.record_type,record_type);
            ret = calendar_list_add(h, extended);
        }
        g = g_list_next(g);
    }

    *hlist = h;
    return CALENDAR_ERROR_NONE;
}

int _cal_db_extended_delete_with_id(int record_id, calendar_record_type_e record_type)
{
    char query[CAL_DB_SQL_MAX_LEN] = {0};
    cal_db_util_error_e dbret = CAL_DB_OK;

    snprintf(query, sizeof(query), "DELETE FROM %s WHERE record_id=%d AND record_type=%d",
            CAL_TABLE_EXTENDED, record_id, record_type);

    dbret = _cal_db_util_query_exec(query);
    if (dbret != CAL_DB_OK) {
        ERR("_cal_db_util_query_exec() failed (%d)", dbret);
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
