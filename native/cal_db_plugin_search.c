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

/*
 * db plugin function
 */
static int __cal_db_search_get_records_with_query( calendar_query_h query, int offset, int limit, calendar_list_h* out_list );
static int __cal_db_search_get_count_with_query(calendar_query_h query, int *out_count);

/*
 * static function
 */
static void __cal_db_search_get_stmt(sqlite3_stmt *stmt,calendar_query_h query,
        calendar_record_h record);
static void __cal_db_search_get_property_stmt(sqlite3_stmt *stmt,
        unsigned int property, int *stmt_count, calendar_record_h record);
static void __cal_db_search_get_projection_stmt(sqlite3_stmt *stmt,
        const unsigned int *projection, const int projection_count,
        calendar_record_h record);
static int __cal_db_search_make_projection(calendar_query_h query, char **projection);


cal_db_plugin_cb_s _cal_db_search_plugin_cb = {
    .is_query_only=true,
    .insert_record=NULL,
    .get_record=NULL,
    .update_record=NULL,
    .delete_record=NULL,
    .get_all_records=NULL,
    .get_records_with_query=__cal_db_search_get_records_with_query,
    .insert_records=NULL,
    .update_records=NULL,
    .delete_records=NULL,
    .get_count=NULL,
    .get_count_with_query=__cal_db_search_get_count_with_query,
    .replace_record=NULL,
    .replace_records=NULL
};

static int __cal_db_search_get_records_with_query( calendar_query_h query, int offset, int limit, calendar_list_h* out_list )
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

    // make filter
    if (que->filter)
    {
        ret = _cal_db_query_create_condition(query,
                &condition, &bind_text);
        if (ret != CALENDAR_ERROR_NONE)
        {
            ERR("filter create fail");
            return ret;
        }
    }

    // make projection
    if (que->projection_count > 0)
    {
        ret = _cal_db_query_create_projection(query, &projection);
    }
    else
    {
        __cal_db_search_make_projection(query, &projection);
    }

    // query - projection
    if ( 0 == strcmp(que->view_uri, CALENDAR_VIEW_EVENT_CALENDAR))
    {
        table_name = SAFE_STRDUP(CAL_VIEW_TABLE_EVENT_CALENDAR);
    }
    else if ( 0 == strcmp(que->view_uri, CALENDAR_VIEW_TODO_CALENDAR))
    {
        table_name = SAFE_STRDUP(CAL_VIEW_TABLE_TODO_CALENDAR);
    }
    else if ( 0 == strcmp(que->view_uri, CALENDAR_VIEW_EVENT_CALENDAR_ATTENDEE))
    {
        table_name = SAFE_STRDUP(CAL_VIEW_TABLE_EVENT_CALENDAR_ATTENDEE);
    }
    else if ( 0 == strcmp(que->view_uri, CALENDAR_VIEW_INSTANCE_NORMAL_CALENDAR))
    {
        table_name = SAFE_STRDUP(CAL_VIEW_TABLE_NORMAL_INSTANCE);
    }
    else if ( 0 == strcmp(que->view_uri, CALENDAR_VIEW_INSTANCE_ALLDAY_CALENDAR))
    {
        table_name = SAFE_STRDUP(CAL_VIEW_TABLE_ALLDAY_INSTANCE);
    }
    else
    {
        ERR("uri(%s) not support get records with query",que->view_uri);
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    if (que->distinct == true)
    {
        len = snprintf(strquery, sizeof(strquery), "SELECT DISTINCT %s FROM %s", projection, table_name);
    }
    else
    {
        len = snprintf(strquery, sizeof(strquery), "SELECT %s FROM %s", projection, table_name);
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
        ret = calendar_record_create(que->view_uri,&record);
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
            __cal_db_search_get_projection_stmt(stmt,que->projection,que->projection_count,
                    record);
        }
        else
        {
            __cal_db_search_get_stmt(stmt, query,record);
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

static int __cal_db_search_get_count_with_query(calendar_query_h query, int *out_count)
{
    cal_query_s *que = NULL;
    int ret = CALENDAR_ERROR_NONE;
    char *condition = NULL;
    char *projection = NULL;
    char strquery[CAL_DB_SQL_MAX_LEN] = {0};
    int len;
    char *table_name;
    int count = 0;
    GSList *bind_text = NULL;

    que = (cal_query_s *)query;

    if ( 0 == strcmp(que->view_uri, CALENDAR_VIEW_EVENT_CALENDAR))
    {
        table_name = SAFE_STRDUP(CAL_VIEW_TABLE_EVENT_CALENDAR);
        projection = SAFE_STRDUP("id");
    }
    else if ( 0 == strcmp(que->view_uri, CALENDAR_VIEW_TODO_CALENDAR))
    {
        table_name = SAFE_STRDUP(CAL_VIEW_TABLE_TODO_CALENDAR);
        projection = SAFE_STRDUP("id");
    }
    else if ( 0 == strcmp(que->view_uri, CALENDAR_VIEW_EVENT_CALENDAR_ATTENDEE))
    {
        table_name = SAFE_STRDUP(CAL_VIEW_TABLE_EVENT_CALENDAR_ATTENDEE);
        projection = SAFE_STRDUP("id");
    }
    else if ( 0 == strcmp(que->view_uri, CALENDAR_VIEW_INSTANCE_NORMAL_CALENDAR))
    {
        table_name = SAFE_STRDUP(CAL_VIEW_TABLE_NORMAL_INSTANCE);
        projection = SAFE_STRDUP("event_id");
    }
    else if ( 0 == strcmp(que->view_uri, CALENDAR_VIEW_INSTANCE_ALLDAY_CALENDAR))
    {
        table_name = SAFE_STRDUP(CAL_VIEW_TABLE_ALLDAY_INSTANCE);
        projection = SAFE_STRDUP("event_id");
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
            CAL_FREE(projection);
            return ret;
        }
    }

    // query - select from

    if (que->distinct == true)
    {
        len = snprintf(strquery, sizeof(strquery), "SELECT count(DISTINCT %s) FROM %s", projection, table_name);

    }
    else
    {
        len = snprintf(strquery, sizeof(strquery), "SELECT count(*) FROM %s", table_name);
    }

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
		CAL_FREE(projection);
		return ret;
	}
    CAL_DBG("%s=%d",strquery,count);

    *out_count = count;

    if (bind_text)
    {
        g_slist_free(bind_text);
    }
    CAL_FREE(condition);
    CAL_FREE(projection);
    return CALENDAR_ERROR_NONE;
}

static void __cal_db_search_get_stmt(sqlite3_stmt *stmt,calendar_query_h query,
        calendar_record_h record)
{
    int i=0;
    int stmt_count = 0;
    cal_query_s *query_s = NULL;
    cal_property_info_s *properties = NULL;

    query_s = (cal_query_s *)query;

    for (i=0;i<query_s->property_count;i++)
    {
        properties = &(query_s->properties[i]);

        if ( CAL_PROPERTY_CHECK_FLAGS(properties->property_id, CAL_PROPERTY_FLAGS_FILTER) == true)
        {
            break;
        }

        __cal_db_search_get_property_stmt(stmt, properties->property_id, &stmt_count,record);
    }
    return ;
}

static void __cal_db_search_get_property_stmt(sqlite3_stmt *stmt,
        unsigned int property, int *stmt_count, calendar_record_h record)
{
    cal_search_s *search = NULL;
    const unsigned char *temp;
    char *dtstart_datetime;
    char buf[8] = {0};
    int int_tmp = 0;
    double d_tmp = 0;
    long long int lli_tmp = 0;

    search = (cal_search_s*)(record);

    if (CAL_PROPERTY_CHECK_DATA_TYPE(property,CAL_PROPERTY_DATA_TYPE_INT) == true)
    {
        int_tmp = sqlite3_column_int(stmt, *stmt_count);
        _cal_record_set_int(record,property,int_tmp);
    }
    else if (CAL_PROPERTY_CHECK_DATA_TYPE(property,CAL_PROPERTY_DATA_TYPE_STR) == true)
    {
        temp = sqlite3_column_text(stmt, *stmt_count);
        _cal_record_set_str(record,property,(const char*)temp);
    }
    else if (CAL_PROPERTY_CHECK_DATA_TYPE(property,CAL_PROPERTY_DATA_TYPE_DOUBLE) == true)
    {
        d_tmp = sqlite3_column_double(stmt,*stmt_count);
        _cal_record_set_double(record,property,d_tmp);
    }
    else if (CAL_PROPERTY_CHECK_DATA_TYPE(property,CAL_PROPERTY_DATA_TYPE_LLI) == true)
    {
        lli_tmp = sqlite3_column_int64(stmt, *stmt_count);
        _cal_record_set_lli(record,property,lli_tmp);
    }
    else if (CAL_PROPERTY_CHECK_DATA_TYPE(property,CAL_PROPERTY_DATA_TYPE_CALTIME) == true)
    {
        calendar_time_s caltime_tmp;
        caltime_tmp.type = sqlite3_column_int(stmt,*stmt_count);
        if (caltime_tmp.type == CALENDAR_TIME_UTIME)
        {
            *stmt_count = *stmt_count+1;
            caltime_tmp.time.utime = sqlite3_column_int64(stmt,*stmt_count);
            *stmt_count = *stmt_count+1;
            sqlite3_column_text(stmt, *stmt_count);
        }
        else
        {
            *stmt_count = *stmt_count+1;
            sqlite3_column_int64(stmt,*stmt_count); //event->start.time.utime = sqlite3_column_int64(stmt,count++);
            *stmt_count = *stmt_count+1;
            temp = sqlite3_column_text(stmt, *stmt_count);
            if (temp) {
                dtstart_datetime = SAFE_STRDUP(temp);
                snprintf(buf, strlen("YYYY") + 1, "%s", &dtstart_datetime[0]);
                caltime_tmp.time.date.year =  atoi(buf);
                snprintf(buf, strlen("MM") + 1, "%s", &dtstart_datetime[4]);
                caltime_tmp.time.date.month = atoi(buf);
                snprintf(buf, strlen("DD") + 1, "%s", &dtstart_datetime[6]);
                caltime_tmp.time.date.mday = atoi(buf);
                if (dtstart_datetime) free(dtstart_datetime);
            }
        }
        _cal_record_set_caltime(record,property,caltime_tmp);
    }
    else
    {
        sqlite3_column_int(stmt, *stmt_count);
    }

    *stmt_count = *stmt_count+1;
}
static void __cal_db_search_get_projection_stmt(sqlite3_stmt *stmt,
        const unsigned int *projection, const int projection_count,
        calendar_record_h record)
{
    int i=0;
    int stmt_count = 0;

    for(i=0;i<projection_count;i++)
    {
        __cal_db_search_get_property_stmt(stmt,projection[i],&stmt_count,record);
    }
}

static int __cal_db_search_make_projection(calendar_query_h query, char **projection)
{
    int i = 0;
    int len = 0;
    const char *field_name;
    char out_projection[CAL_DB_SQL_MAX_LEN] = {0};
    cal_query_s *query_s = NULL;
    cal_property_info_s *properties = NULL;

    retv_if(NULL == query, CALENDAR_ERROR_INVALID_PARAMETER);
    query_s = (cal_query_s *)query;

    properties = &(query_s->properties[0]);
    field_name = properties->fields;
    if (field_name)
        len += snprintf(out_projection+len, sizeof(out_projection)-len, "%s", field_name);

    for (i=1;i<query_s->property_count;i++)
    {
        properties = &(query_s->properties[i]);
        field_name = properties->fields;

        if ( CAL_PROPERTY_CHECK_FLAGS(properties->property_id, CAL_PROPERTY_FLAGS_FILTER) == true)
        {
            break;
        }

        if (field_name)
        {
            len += snprintf(out_projection+len, sizeof(out_projection)-len, ", %s", field_name);
        }
    }

    *projection = strdup(out_projection);

    return CALENDAR_ERROR_NONE;
}
