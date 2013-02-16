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

#include "cal_db_util.h"
#include "cal_db_query.h"
#include "cal_db_attendee.h"

static int __cal_db_attendee_insert_record(calendar_record_h record, int* id);
static int __cal_db_attendee_get_record(int id, calendar_record_h* out_record);
static int __cal_db_attendee_update_record(calendar_record_h record);
static int __cal_db_attendee_delete_record(int id);
static int __cal_db_attendee_get_all_records(int offset, int limit, calendar_list_h* out_list);
static int __cal_db_attendee_get_records_with_query(calendar_query_h query, int offset, int limit, calendar_list_h* out_list);
static int __cal_db_attendee_insert_records(const calendar_list_h list, int** ids);
static int __cal_db_attendee_update_records(const calendar_list_h list);
static int __cal_db_attendee_delete_records(int ids[], int count);
static int __cal_db_attendee_get_count(int *out_count);
static int __cal_db_attendee_get_count_with_query(calendar_query_h query, int *out_count);
/*
 * static function
 */
static void __cal_db_attendee_get_stmt(sqlite3_stmt *stmt,calendar_record_h record);
static void __cal_db_attendee_get_property_stmt(sqlite3_stmt *stmt,
        unsigned int property, int *stmt_count, calendar_record_h record);
static void __cal_db_attendee_get_projection_stmt(sqlite3_stmt *stmt,
        const unsigned int *projection, const int projection_count,
        calendar_record_h record);

cal_db_plugin_cb_s _cal_db_attendee_plugin_cb = {
	.is_query_only = false,
	.insert_record=__cal_db_attendee_insert_record,
	.get_record=__cal_db_attendee_get_record,
	.update_record=__cal_db_attendee_update_record,
	.delete_record=__cal_db_attendee_delete_record,
	.get_all_records=__cal_db_attendee_get_all_records,
	.get_records_with_query=__cal_db_attendee_get_records_with_query,
	.insert_records=__cal_db_attendee_insert_records,
	.update_records=__cal_db_attendee_update_records,
	.delete_records=__cal_db_attendee_delete_records,
    .get_count=__cal_db_attendee_get_count,
    .get_count_with_query=__cal_db_attendee_get_count_with_query,
    .replace_record=NULL,
    .replace_records=NULL
};

static int __cal_db_attendee_insert_record(calendar_record_h record, int* id)
{
	int index;
    cal_db_util_error_e dbret = CAL_DB_OK;
	char query[CAL_DB_SQL_MAX_LEN];
	sqlite3_stmt *stmt;
	cal_attendee_s *attendee = NULL;

	attendee = (cal_attendee_s *)(record);
	retvm_if(NULL == attendee, CALENDAR_ERROR_INVALID_PARAMETER,
			"Invalid argument: cal_alarm_s is NULL");

	snprintf(query, sizeof(query),
			"INSERT INTO %s ("
			"event_id, "
			"attendee_name, attendee_email, attendee_number, "
			"attendee_status, attendee_type, attendee_ct_index, "
			"attendee_role, attendee_rsvp, attendee_group, "
			"attendee_delegator_uri, attendee_delegate_uri, attendee_uid "
			") VALUES ( "
			"%d, "
			"?, ?, ?, "
			"%d, %d, %d, "
			"%d, %d, ?, "
			"?, ?, ? )",
			CAL_TABLE_ATTENDEE,
			attendee->event_id,
			attendee->attendee_status,
			attendee->attendee_type,
			attendee->attendee_ct_index,
			attendee->attendee_role,
			attendee->attendee_rsvp);

	stmt = _cal_db_util_query_prepare(query);
	retvm_if(NULL == stmt, CALENDAR_ERROR_DB_FAILED, "_cal_db_util_query_prepare() Failed");

	index = 1;
	if (attendee->attendee_name)
		_cal_db_util_stmt_bind_text(stmt, index, attendee->attendee_name);
	index++;

	if (attendee->attendee_email)
		_cal_db_util_stmt_bind_text(stmt, index, attendee->attendee_email);
	index++;

	if (attendee->attendee_number)
		_cal_db_util_stmt_bind_text(stmt, index, attendee->attendee_number);
	index++;

	if (attendee->attendee_group)
		_cal_db_util_stmt_bind_text(stmt, index, attendee->attendee_group);
	index++;

	if (attendee->attendee_delegator_uri)
		_cal_db_util_stmt_bind_text(stmt, index, attendee->attendee_delegator_uri);
	index++;

	if (attendee->attendee_delegate_uri)
		_cal_db_util_stmt_bind_text(stmt, index, attendee->attendee_delegate_uri);
	index++;

	if (attendee->attendee_uid)
		_cal_db_util_stmt_bind_text(stmt, index, attendee->attendee_uid);
	index++;

	dbret = _cal_db_util_stmt_step(stmt);
	sqlite3_finalize(stmt);

	if (CAL_DB_DONE != dbret)
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

	return CALENDAR_ERROR_NONE;
}

static int __cal_db_attendee_get_record(int id, calendar_record_h* out_record)
{
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	sqlite3_stmt *stmt = NULL;
	cal_db_util_error_e dbret = CAL_DB_OK;
	int ret = CALENDAR_ERROR_NONE;

	retvm_if(id < 0, CALENDAR_ERROR_INVALID_PARAMETER,
			"Invalid argument: id(%d)", id);

	ret = calendar_record_create(_calendar_attendee._uri, out_record);
    if (ret != CALENDAR_ERROR_NONE)
    {
        ERR("calendar_record_create(%d)", ret);
        return CALENDAR_ERROR_DB_FAILED;
    }

	snprintf(query, sizeof(query),
			"SELECT * FROM %s "
			"WHERE event_id = %d ",
			CAL_TABLE_ATTENDEE,
			id);
	stmt = _cal_db_util_query_prepare(query);
    if (NULL == stmt)
    {
        ERR("_cal_db_util_query_prepare() Failed");
        calendar_record_destroy(*out_record, true);
        *out_record = NULL;
        return CALENDAR_ERROR_DB_FAILED;
    }

	dbret = _cal_db_util_stmt_step(stmt);
	if (CAL_DB_ROW != dbret)
	{
		ERR(" _cal_db_util_stmt_step() Failed(%d)", dbret);
		sqlite3_finalize(stmt);
		calendar_record_destroy(*out_record, true);
		*out_record = NULL;
		switch (dbret)
		{
		case CAL_DB_ERROR_NO_SPACE:
			return CALENDAR_ERROR_FILE_NO_SPACE;
		default:
			return CALENDAR_ERROR_DB_FAILED;
		}
	}

	__cal_db_attendee_get_stmt(stmt,*out_record);

	sqlite3_finalize(stmt);
	stmt = NULL;

	return CALENDAR_ERROR_NONE;
}

static int __cal_db_attendee_update_record(calendar_record_h record)
{
	int index;
    cal_db_util_error_e dbret = CAL_DB_OK;
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	cal_attendee_s *attendee = NULL;
	sqlite3_stmt *stmt = NULL;

	attendee = (cal_attendee_s *)(record);
	retvm_if(NULL == attendee, CALENDAR_ERROR_INVALID_PARAMETER,
			"Invalid argument: record is NULL");

	snprintf(query, sizeof(query),
			"UPDATE %s SET "
			"attendee_name = ?, "
			"attendee_email = ?, "
			"attendee_number = ?, "
			"attendee_status = %d, "
			"attendee_type = %d, "
			"attendee_ct_index = %d, "
			"attendee_role = %d, "
			"attendee_rsvp = %d, "
			"attendee_group = ?, "
			"attendee_delegator_uri = ?, "
			"attendee_delegate_uri = ?, "
			"attendee_uid = ? ",
			CAL_TABLE_ATTENDEE,
			attendee->attendee_status,
			attendee->attendee_type,
			attendee->attendee_ct_index,
			attendee->attendee_role,
			attendee->attendee_rsvp);

	stmt = _cal_db_util_query_prepare(query);
	retvm_if(NULL == stmt, CALENDAR_ERROR_DB_FAILED,
			"_cal_db_util_query_prepare() Failed");

	index = 1;
	if (attendee->attendee_name)
		_cal_db_util_stmt_bind_text(stmt, index, attendee->attendee_name);
	index++;

	if (attendee->attendee_email)
		_cal_db_util_stmt_bind_text(stmt, index, attendee->attendee_email);
	index++;

	if (attendee->attendee_number)
		_cal_db_util_stmt_bind_text(stmt, index, attendee->attendee_number);
	index++;

	if (attendee->attendee_group)
		_cal_db_util_stmt_bind_text(stmt, index, attendee->attendee_group);
	index++;

	if (attendee->attendee_delegator_uri)
		_cal_db_util_stmt_bind_text(stmt, index, attendee->attendee_delegator_uri);
	index++;

	if (attendee->attendee_delegate_uri)
		_cal_db_util_stmt_bind_text(stmt, index, attendee->attendee_delegate_uri);
	index++;

	if (attendee->attendee_uid)
		_cal_db_util_stmt_bind_text(stmt, index, attendee->attendee_uid);
	index++;

	dbret = _cal_db_util_stmt_step(stmt);
	sqlite3_finalize(stmt);
	if (CAL_DB_DONE != dbret)
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

	return CALENDAR_ERROR_NONE;
}

static int __cal_db_attendee_delete_record(int id)
{
    cal_db_util_error_e dbret = CAL_DB_OK;
	char query[CAL_DB_SQL_MAX_LEN];

	snprintf(query, sizeof(query),
			"DELETE FROM %s "
			"WHERE event_id = %d ",
			CAL_TABLE_ATTENDEE,
			id);

	dbret = _cal_db_util_query_exec(query);
	if (CAL_DB_OK != dbret)
	{
		ERR("_cal_db_util_query_exec() Failed(%d)", dbret);
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

static int __cal_db_attendee_get_all_records(int offset, int limit, calendar_list_h* out_list)
{
	int ret;
    char query[CAL_DB_SQL_MAX_LEN] = {0};
    char offsetquery[CAL_DB_SQL_MAX_LEN] = {0};
    char limitquery[CAL_DB_SQL_MAX_LEN] = {0};
    sqlite3_stmt *stmt = NULL;

	retvm_if(NULL == out_list, CALENDAR_ERROR_INVALID_PARAMETER,
			"Invalid parameter: calendar_list_h is NULL");
	ret = calendar_list_create(out_list);
	retvm_if(CALENDAR_ERROR_NONE != ret, CALENDAR_ERROR_DB_FAILED,
			"calendar_list_create() failed");

	if (limit > 0)
	{
		snprintf(limitquery, sizeof(limitquery), "LIMIT %d ", limit);
	}
	if (offset > 0)
	{
		snprintf(offsetquery, sizeof(offsetquery), "OFFSET %d ", offset);
	}
	snprintf(query, sizeof(query),
			"SELECT * FROM %s %s %s ",
			CAL_TABLE_ATTENDEE, limitquery, offsetquery);

	stmt = _cal_db_util_query_prepare(query);
	if (NULL == stmt)
	{
		ERR("_cal_db_util_query_prepare() failed");
		calendar_list_destroy(*out_list, true);
		*out_list = NULL;
		return CALENDAR_ERROR_DB_FAILED;
	}

	calendar_record_h record = NULL;

	while (CAL_DB_ROW == _cal_db_util_stmt_step(stmt))
	{
		ret = calendar_record_create(_calendar_attendee._uri, &record);
		if (CALENDAR_ERROR_NONE != ret)
		{
			calendar_list_destroy(*out_list, true);
			*out_list = NULL;
			sqlite3_finalize(stmt);
			return ret;
		}
		//
		__cal_db_attendee_get_stmt(stmt,record);

		ret = calendar_list_add(*out_list, record);
		if (CALENDAR_ERROR_NONE != ret)
		{
			calendar_list_destroy(*out_list, true);
			*out_list = NULL;
			calendar_record_destroy(record, true);
			sqlite3_finalize(stmt);
			return ret;
		}
	}
	sqlite3_finalize(stmt);

    return CALENDAR_ERROR_NONE;
}

static int __cal_db_attendee_get_records_with_query(calendar_query_h query, int offset, int limit, calendar_list_h* out_list)
{
    cal_query_s *que = NULL;
    int ret = CALENDAR_ERROR_NONE;
    char *condition = NULL;
    char *projection = NULL;
    char *order = NULL;
    GSList *bind_text = NULL, *cursor = NULL;
    char strquery[CAL_DB_SQL_MAX_LEN] = {0};
    int len;
    sqlite3_stmt *stmt = NULL;
    int i = 0;
    char *table_name;

    que = (cal_query_s *)query;

    if ( 0 == strcmp(que->view_uri, CALENDAR_VIEW_ATTENDEE))
    {
        table_name = SAFE_STRDUP(CAL_TABLE_ATTENDEE);
    }
    else
    {
        ERR("uri(%s) not support get records with query",que->view_uri);
        return CALENDAR_ERROR_INVALID_PARAMETER;
        //table_name = SAFE_STRDUP(CAL_TABLE_NORMAL_INSTANCE);
    }

    // make filter
    if (que->filter)
    {
        ret = _cal_db_query_create_condition(query,
                &condition, &bind_text);
        if (ret != CALENDAR_ERROR_NONE)
        {
            CAL_FREE(table_name);
            ERR("filter create fail");
            return ret;
        }
    }

    // make projection
    ret = _cal_db_query_create_projection(query, &projection);

    // query - projection
    if (projection)
    {
        len = snprintf(strquery, sizeof(strquery), "SELECT %s FROM %s", projection, table_name);
    }
    else
    {
        len = snprintf(strquery, sizeof(strquery), "SELECT * FROM %s", table_name);
    }
    CAL_FREE(table_name);

    // query - condition
    if (condition)
    {
        len += snprintf(strquery+len, sizeof(strquery)-len, " WHERE %s", condition);
    }

    // ORDER
    ret = _cal_db_query_create_order(query, &order);
    if (order)
    {
        len += snprintf(strquery+len, sizeof(strquery)-len, " %s", order);
    }

    if (0 < limit)
    {
        len += snprintf(strquery+len, sizeof(strquery)-len, " LIMIT %d", limit);
        if (0 < offset)
            len += snprintf(strquery+len, sizeof(strquery)-len, " OFFSET %d", offset);
    }

    // query
    stmt = _cal_db_util_query_prepare(strquery);
    if (NULL == stmt)
    {
        if (bind_text)
        {
            g_slist_free(bind_text);
        }
        CAL_FREE(condition);
        CAL_FREE(projection);
        ERR("_cal_db_util_query_prepare() Failed");
        return CALENDAR_ERROR_DB_FAILED;
    }
    CAL_DBG("%s",strquery);

    // bind text
    if (bind_text)
    {
        len = g_slist_length(bind_text);
        for (cursor=bind_text, i=1; cursor;cursor=cursor->next, i++)
        {
            _cal_db_util_stmt_bind_text(stmt, i, cursor->data);
        }
    }

    //
    ret = calendar_list_create(out_list);
    if (ret != CALENDAR_ERROR_NONE)
    {
        if (bind_text)
        {
            g_slist_free(bind_text);
        }
        CAL_FREE(condition);
        CAL_FREE(projection);
        ERR("calendar_list_create() Failed");
        sqlite3_finalize(stmt);
        return ret;
    }

    while(CAL_DB_ROW == _cal_db_util_stmt_step(stmt))
    {
        calendar_record_h record;
        // stmt -> record
        ret = calendar_record_create(_calendar_attendee._uri,&record);
        if( ret != CALENDAR_ERROR_NONE )
        {
            calendar_list_destroy(*out_list, true);
            *out_list = NULL;

            if (bind_text)
            {
                g_slist_free(bind_text);
            }
            CAL_FREE(condition);
            CAL_FREE(projection);
            sqlite3_finalize(stmt);
            return ret;
        }
        if (que->projection_count > 0)
        {
			_cal_record_set_projection(record,
					que->projection, que->projection_count, que->property_count);

            __cal_db_attendee_get_projection_stmt(stmt,
					que->projection, que->projection_count,
                    record);
        }
        else
        {
            __cal_db_attendee_get_stmt(stmt,record);
        }

        ret = calendar_list_add(*out_list,record);
        if( ret != CALENDAR_ERROR_NONE )
        {
            calendar_list_destroy(*out_list, true);
            *out_list = NULL;
            calendar_record_destroy(record, true);

            if (bind_text)
            {
                g_slist_free(bind_text);
            }
            CAL_FREE(condition);
            CAL_FREE(projection);
            sqlite3_finalize(stmt);
            return ret;
        }
    }

    if (bind_text)
    {
        g_slist_free(bind_text);
    }
    CAL_FREE(condition);
    CAL_FREE(projection);
    sqlite3_finalize(stmt);

    return CALENDAR_ERROR_NONE;
}

static int __cal_db_attendee_insert_records(const calendar_list_h list, int** ids)
{
    calendar_record_h record;
    int ret = 0;

    ret = calendar_list_first(list);
    if (ret != CALENDAR_ERROR_NONE)
    {
        ERR("list first error");
        return ret;
    }
    do
    {
        if( calendar_list_get_current_record_p(list, &record) == CALENDAR_ERROR_NONE)
        {
            if( __cal_db_attendee_insert_record(record, NULL) != CALENDAR_ERROR_NONE)
            {
                ERR("db insert error");
                return CALENDAR_ERROR_DB_FAILED;
            }
        }
    } while(calendar_list_next(list) != CALENDAR_ERROR_NO_DATA);

    return CALENDAR_ERROR_NONE;
}

static int __cal_db_attendee_update_records(const calendar_list_h list)
{
    calendar_record_h record;
    int ret = 0;

    ret = calendar_list_first(list);
    if (ret != CALENDAR_ERROR_NONE)
    {
        ERR("list first error");
        return ret;
    }
    do
    {
        if( calendar_list_get_current_record_p(list, &record) == CALENDAR_ERROR_NONE)
        {
            if( __cal_db_attendee_update_record(record) != CALENDAR_ERROR_NONE)
            {
                ERR("db insert error");
                return CALENDAR_ERROR_DB_FAILED;
            }
        }
    } while(calendar_list_next(list) != CALENDAR_ERROR_NO_DATA);

    return CALENDAR_ERROR_NONE;
}

static int __cal_db_attendee_delete_records(int ids[], int count)
{
    int i=0;
    for(i=0;i<count;i++)
    {
        if (__cal_db_attendee_delete_record(ids[i]) != CALENDAR_ERROR_NONE)
        {
            ERR("delete failed");
            return CALENDAR_ERROR_DB_FAILED;
        }
    }
    return CALENDAR_ERROR_NONE;
}

static int __cal_db_attendee_get_count(int *out_count)
{
	int ret;
    char query[CAL_DB_SQL_MAX_LEN] = {0};
    int count = 0;

    retvm_if(NULL == out_count, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");

    snprintf(query, sizeof(query), "SELECT count(*) FROM %s ", CAL_TABLE_ATTENDEE);

    ret = _cal_db_util_query_get_first_int_result(query, NULL, &count);
	if (CALENDAR_ERROR_NONE != ret)
	{
		ERR("_cal_db_util_query_get_first_int_result() failed");
		return ret;
	}
    CAL_DBG("%s=%d",query,count);

    *out_count = count;
    return CALENDAR_ERROR_NONE;
}

static int __cal_db_attendee_get_count_with_query(calendar_query_h query, int *out_count)
{
    cal_query_s *que = NULL;
    int ret = CALENDAR_ERROR_NONE;
    char *condition = NULL;
    char strquery[CAL_DB_SQL_MAX_LEN] = {0};
    int len;
    char *table_name;
    int count = 0;
    GSList *bind_text = NULL;

    que = (cal_query_s *)query;

    if ( 0 == strcmp(que->view_uri, CALENDAR_VIEW_ATTENDEE))
    {
        table_name = SAFE_STRDUP(CAL_TABLE_ATTENDEE);
    }
    else
    {
        ERR("uri(%s) not support get records with query",que->view_uri);
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    // make filter
    if (que->filter)
    {
        ret = _cal_db_query_create_condition(query, &condition, &bind_text);
        if (ret != CALENDAR_ERROR_NONE)
        {
            CAL_FREE(table_name);
            ERR("filter create fail");
            return ret;
        }
    }

    // query - select from

    len = snprintf(strquery, sizeof(strquery), "SELECT count(*) FROM %s", table_name);

    CAL_FREE(table_name);

    // query - condition
    if (condition)
    {
        len += snprintf(strquery+len, sizeof(strquery)-len, " WHERE %s", condition);
    }

    // query
    ret = _cal_db_util_query_get_first_int_result(strquery, bind_text, &count);
	if (CALENDAR_ERROR_NONE != ret)
	{
		ERR("_cal_db_util_query_get_first_int_result() failed");
		if (bind_text)
		{
			g_slist_free(bind_text);
		}
		CAL_FREE(condition);
		return ret;
	}
    CAL_DBG("%s=%d",strquery,count);

    *out_count = count;

    if (bind_text)
    {
        g_slist_free(bind_text);
    }
	CAL_FREE(condition);
    return CALENDAR_ERROR_NONE;
}

int _cal_db_attendee_get_records(int event_id, GList **out_list)
{
	int ret;
    char query[CAL_DB_SQL_MAX_LEN] = {0};
    sqlite3_stmt *stmt = NULL;
	GList *list = NULL;

	retvm_if(NULL == out_list, CALENDAR_ERROR_INVALID_PARAMETER,
			"Invalid parameter: GList is NULL");

	snprintf(query, sizeof(query),
			"SELECT * FROM %s "
			"WHERE event_id = %d ",
			CAL_TABLE_ATTENDEE,
			event_id);

	stmt = _cal_db_util_query_prepare(query);
	if (NULL == stmt)
	{
		ERR("_cal_db_util_query_prepare() failed");
		return CALENDAR_ERROR_DB_FAILED;
	}

	int index;
	const unsigned char *temp;
	calendar_record_h record = NULL;
	cal_attendee_s *attendee = NULL;

	while (CAL_DB_ROW == _cal_db_util_stmt_step(stmt))
	{
		ret = calendar_record_create(_calendar_attendee._uri, &record);
		if (CALENDAR_ERROR_NONE != ret)
		{
            sqlite3_finalize(stmt);
			return ret;
		}

		index = 0;
		attendee = (cal_attendee_s *)(record);

		attendee->event_id = sqlite3_column_int(stmt, index++);

		temp = sqlite3_column_text(stmt, index++);
		attendee->attendee_name = SAFE_STRDUP(temp);

		temp = sqlite3_column_text(stmt, index++);
		attendee->attendee_email = SAFE_STRDUP(temp);

		temp = sqlite3_column_text(stmt, index++);
		attendee->attendee_number = SAFE_STRDUP(temp);

		attendee->attendee_status = sqlite3_column_int(stmt, index++);
		attendee->attendee_type = sqlite3_column_int(stmt, index++);
		attendee->attendee_ct_index = sqlite3_column_int(stmt, index++);
		attendee->attendee_role = sqlite3_column_int(stmt, index++);
		attendee->attendee_rsvp = sqlite3_column_int(stmt, index++);

		temp = sqlite3_column_text(stmt, index++);
		attendee->attendee_group = SAFE_STRDUP(temp);

		temp = sqlite3_column_text(stmt, index++);
		attendee->attendee_delegator_uri = SAFE_STRDUP(temp);

		temp = sqlite3_column_text(stmt, index++);
		attendee->attendee_delegate_uri = SAFE_STRDUP(temp);

		temp = sqlite3_column_text(stmt, index++);
		attendee->attendee_uid = SAFE_STRDUP(temp);

		list = g_list_append(list, record);
	}

	sqlite3_finalize(stmt);
	*out_list = list;
    return CALENDAR_ERROR_NONE;
}

int _cal_db_attendee_convert_gtoh(GList *glist, int id, calendar_list_h *hlist)
{
	int ret;
	GList *g = NULL;
	calendar_list_h h = NULL;
	calendar_record_h attendee = NULL;

	if (glist == NULL)
	{
		DBG("No attendee");
		return CALENDAR_ERROR_NO_DATA;
	}
	ret = calendar_list_create(&h);

	g = g_list_first(glist);
	while (g)
	{
		attendee = (calendar_record_h)g->data;
		if (attendee)
		{
		    ret = _cal_record_set_int(attendee,_calendar_attendee.event_id,id);
		    ret = calendar_list_add(h, attendee);

		}
		g = g_list_next(g);
	}

	*hlist = h;
	return CALENDAR_ERROR_NONE;
}

int _cal_db_attendee_delete_with_id(int event_id)
{
    char query[CAL_DB_SQL_MAX_LEN] = {0};
    cal_db_util_error_e dbret = CAL_DB_OK;

    snprintf(query, sizeof(query), "DELETE FROM %s WHERE event_id=%d ",
            CAL_TABLE_ATTENDEE, event_id);

    dbret = _cal_db_util_query_exec(query);
    if (dbret != CAL_DB_OK)
	{
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

static void __cal_db_attendee_get_stmt(sqlite3_stmt *stmt,calendar_record_h record)
{
    cal_attendee_s *attendee = NULL;
    int index;
    const unsigned char *temp;

    attendee = (cal_attendee_s*)(record);
    index = 0;

    attendee->event_id = sqlite3_column_int(stmt, index++);

    temp = sqlite3_column_text(stmt, index++);
    attendee->attendee_name = SAFE_STRDUP(temp);

    temp = sqlite3_column_text(stmt, index++);
    attendee->attendee_email = SAFE_STRDUP(temp);

    temp = sqlite3_column_text(stmt, index++);
    attendee->attendee_number = SAFE_STRDUP(temp);

    attendee->attendee_status = sqlite3_column_int(stmt, index++);
    attendee->attendee_type = sqlite3_column_int(stmt, index++);
    attendee->attendee_ct_index = sqlite3_column_int(stmt, index++);
    attendee->attendee_role = sqlite3_column_int(stmt, index++);
    attendee->attendee_rsvp = sqlite3_column_int(stmt, index++);

    temp = sqlite3_column_text(stmt, index++);
    attendee->attendee_group = SAFE_STRDUP(temp);

    temp = sqlite3_column_text(stmt, index++);
    attendee->attendee_delegator_uri = SAFE_STRDUP(temp);

    temp = sqlite3_column_text(stmt, index++);
    attendee->attendee_delegate_uri = SAFE_STRDUP(temp);

    temp = sqlite3_column_text(stmt, index++);
    attendee->attendee_uid = SAFE_STRDUP(temp);
}

static void __cal_db_attendee_get_property_stmt(sqlite3_stmt *stmt,
        unsigned int property, int *stmt_count, calendar_record_h record)
{
    cal_attendee_s *attendee = NULL;
    const unsigned char *temp;

    attendee = (cal_attendee_s*)(record);

    switch(property)
    {
    case CAL_PROPERTY_ATTENDEE_NUMBER:
        temp = sqlite3_column_text(stmt, *stmt_count);
        attendee->attendee_number = SAFE_STRDUP(temp);
        break;
    case CAL_PROPERTY_ATTENDEE_TYPE:
        attendee->attendee_type = sqlite3_column_int(stmt, *stmt_count);
        break;
    case CAL_PROPERTY_ATTENDEE_CT_INDEX:
        attendee->attendee_ct_index = sqlite3_column_int(stmt, *stmt_count);
        break;
    case CAL_PROPERTY_ATTENDEE_UID:
        temp = sqlite3_column_text(stmt, *stmt_count);
        attendee->attendee_uid = SAFE_STRDUP(temp);
        break;
    case CAL_PROPERTY_ATTENDEE_GROUP:
        temp = sqlite3_column_text(stmt, *stmt_count);
        attendee->attendee_group = SAFE_STRDUP(temp);
        break;
    case CAL_PROPERTY_ATTENDEE_EMAIL:
        temp = sqlite3_column_text(stmt, *stmt_count);
        attendee->attendee_email = SAFE_STRDUP(temp);
        break;
    case CAL_PROPERTY_ATTENDEE_ROLE:
        attendee->attendee_role = sqlite3_column_int(stmt, *stmt_count);
        break;
    case CAL_PROPERTY_ATTENDEE_STATUS:
        attendee->attendee_status = sqlite3_column_int(stmt, *stmt_count);
        break;
    case CAL_PROPERTY_ATTENDEE_RSVP:
        attendee->attendee_rsvp = sqlite3_column_int(stmt, *stmt_count);
        break;
    case CAL_PROPERTY_ATTENDEE_DELEGATE_URI:
        temp = sqlite3_column_text(stmt, *stmt_count);
        attendee->attendee_delegate_uri = SAFE_STRDUP(temp);
        break;
    case CAL_PROPERTY_ATTENDEE_DELEGATOR_URI:
        temp = sqlite3_column_text(stmt, *stmt_count);
        attendee->attendee_delegator_uri = SAFE_STRDUP(temp);
        break;
    case CAL_PROPERTY_ATTENDEE_NAME:
        temp = sqlite3_column_text(stmt, *stmt_count);
        attendee->attendee_name = SAFE_STRDUP(temp);
        break;
    case CAL_PROPERTY_ATTENDEE_EVENT_ID:
        attendee->event_id = sqlite3_column_int(stmt, *stmt_count);
        break;
    default:
        sqlite3_column_int(stmt, *stmt_count);
        break;
    }
    *stmt_count = *stmt_count+1;
}

static void __cal_db_attendee_get_projection_stmt(sqlite3_stmt *stmt,
        const unsigned int *projection, const int projection_count,
        calendar_record_h record)
{
    int i=0;
    int stmt_count = 0;

    for(i=0;i<projection_count;i++)
    {
        __cal_db_attendee_get_property_stmt(stmt,projection[i],&stmt_count,record);
    }
}
