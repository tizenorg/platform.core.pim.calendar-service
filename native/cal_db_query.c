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
#include "cal_db.h"
#include "cal_db_query.h"

#define CAL_DB_CALTIME_FIELD_MAX 3

static const char* __cal_db_utime_field_name[] =
{
        "dtstart_utime",
        "dtend_utime",
        "until_utime"
};
static const char* __cal_db_datetime_field_name[] =
{
        "dtstart_datetime",
        "dtend_datetime",
        "until_datetime"
};
static const char* __cal_db_timetype_field_name[] =
{
        "dtstart_type",
        "dtend_type",
        "until_type"
};

static int __cal_db_query_create_composite_condition(cal_composite_filter_s *com_filter,
        char **condition, GSList **bind_text);
static int __cal_db_query_create_attribute_condition(cal_composite_filter_s *com_filter,
        cal_attribute_filter_s *filter, char **condition, GSList **bind_text);
static int __cal_db_query_create_str_condition(cal_composite_filter_s *com_filter,
        cal_attribute_filter_s *filter, char **condition, GSList **bind_text);
static int __cal_db_query_create_int_condition(cal_composite_filter_s *com_filter,
        cal_attribute_filter_s *filter, char **condition );
static int __cal_db_query_create_double_condition(cal_composite_filter_s *com_filter,
        cal_attribute_filter_s *filter, char **condition );
static int __cal_db_query_create_lli_condition(cal_composite_filter_s *com_filter,
        cal_attribute_filter_s *filter, char **condition );
static int __cal_db_query_create_caltime_condition(cal_composite_filter_s *com_filter,
        cal_attribute_filter_s *filter, char **condition );
static const char * __cal_db_query_get_property_field_name(const cal_property_info_s *properties,
        int count, unsigned int property_id);
static const char * __cal_db_query_get_utime_field_name(const char* src);
static const char * __cal_db_query_get_datetime_field_name(const char* src);
static const char * __cal_db_query_get_timetype_field_name(const char* src);

int _cal_db_query_create_condition(calendar_query_h query, char **condition, GSList **bind_text)
{
    cal_query_s *que = NULL;
    int ret = CALENDAR_ERROR_NONE;

    retv_if(NULL == query || NULL == condition || NULL == bind_text, CALENDAR_ERROR_INVALID_PARAMETER);

    que = (cal_query_s *)query;

    ret = __cal_db_query_create_composite_condition(que->filter, condition, bind_text);

    return ret;
}

int _cal_db_query_create_projection(calendar_query_h query, char **projection)
{
    int i = 0;
    int len = 0;
    const char *field_name;
    char out_projection[CAL_DB_SQL_MAX_LEN] = {0};
    cal_query_s *query_s = NULL;
    cal_property_info_s *properties = NULL;

    retv_if(NULL == query, CALENDAR_ERROR_INVALID_PARAMETER);
    query_s = (cal_query_s *)query;

    properties = query_s->properties;

    if (query_s->projection)
    {
        field_name = __cal_db_query_get_property_field_name(properties, query_s->property_count, query_s->projection[0]);
        if (field_name)
            len += snprintf(out_projection+len, sizeof(out_projection)-len, "%s", field_name);

        for (i=1;i<query_s->projection_count;i++)
        {
            field_name = __cal_db_query_get_property_field_name(properties, query_s->property_count, query_s->projection[i]);
            if (field_name)
                len += snprintf(out_projection+len, sizeof(out_projection)-len, ", %s", field_name);
        }
    }
    else
    {
#if 0  // projection이 없을 경우 select *
        for (i=1;i<query_s->property_count;i++) {
            if (properties[i].fields)
                len += snprintf(out_projection+len, sizeof(out_projection)-len, ", %s", (char *)properties[i].fields);
            else
                len += snprintf(out_projection+len, sizeof(out_projection)-len, ", %s", ctsvc_get_display_column());
        }
#endif
        len += snprintf(out_projection+len, sizeof(out_projection)-len, " * ");
    }

	*projection = strdup(out_projection);

    return CALENDAR_ERROR_NONE;
}

int _cal_db_query_create_order(calendar_query_h query, char **order)
{
    int len = 0;
    const char *field_name;
    char out_order[CAL_DB_SQL_MAX_LEN] = {0};
    cal_query_s *query_s = NULL;
    cal_property_info_s *properties = NULL;

    retv_if(NULL == query, CALENDAR_ERROR_INVALID_PARAMETER);
    query_s = (cal_query_s *)query;
    properties = query_s->properties;

    if (query_s->sort_property_id > 0)
    {

        field_name = __cal_db_query_get_property_field_name(properties, query_s->property_count, query_s->sort_property_id);

        if (CAL_PROPERTY_CHECK_DATA_TYPE(query_s->sort_property_id, CAL_PROPERTY_DATA_TYPE_CALTIME) == true && field_name)
        {
            const char *tmp = NULL;

            if (strcmp(query_s->view_uri, CALENDAR_VIEW_INSTANCE_ALLDAY) == 0 ||
                    strcmp(query_s->view_uri, CALENDAR_VIEW_INSTANCE_ALLDAY_CALENDAR) == 0)
            {
                if(CAL_PROPERTY_INSTANCE_ALLDAY_START)
                {
                    tmp = __cal_db_datetime_field_name[0];
                }
                else
                {
                    tmp = __cal_db_datetime_field_name[1];
                }
                if (tmp != NULL)
                {
                     field_name = tmp;
                }
            }
            else
            {
                tmp = __cal_db_query_get_utime_field_name(field_name);
                if (tmp == NULL)
                {
                    tmp = __cal_db_query_get_datetime_field_name(field_name);
                }
                if (tmp != NULL)
                {
                     field_name = tmp;
                }
            }
        }

        if (field_name)
            len += snprintf(out_order+len, sizeof(out_order)-len, "ORDER BY %s", field_name);
    }
    else
    {
        return CALENDAR_ERROR_NO_DATA;
    }

    if (field_name)
    {
        if (query_s->asc == false)
        {
            len += snprintf(out_order+len, sizeof(out_order)-len, " DESC");
        }
    #if 0    // is default
        else // (query_s->asc == true)
        {
            len += snprintf(out_order+len, sizeof(out_order)-len, " ASC");
        }
    #endif
    }

	*order = strdup(out_order);

    return CALENDAR_ERROR_NONE;
}

bool _cal_db_query_find_projection_property(calendar_query_h query, unsigned int property)
{
    int i = 0;
    cal_query_s *query_s = NULL;
    cal_property_info_s *properties = NULL;

    retv_if(NULL == query, CALENDAR_ERROR_INVALID_PARAMETER);
    query_s = (cal_query_s *)query;

    properties = query_s->properties;

    if (query_s->projection)
    {
        for (i=0;i<query_s->projection_count;i++)
        {
            if (query_s->projection[i] == property)
            {
                return true;
            }
        }
    }
    else
    {
        // projection
        return true;
    }

    return false;
}

int _cal_db_query_create_projection_update_set(calendar_record_h record, char **set, GSList **bind_text)
{
    cal_record_s *_record = NULL;
    int i = 0;
    const cal_property_info_s* property_info = NULL;
    int property_info_count = 0;
    char out_set[CAL_DB_SQL_MAX_LEN] = {0};
    int len = 0;
    const char *field_name;
    int ret = CALENDAR_ERROR_NONE;

    retv_if(record == NULL, -1);

    _record = (cal_record_s *)record;

    if (_record->properties_max_count == 0 || _record->properties_flags == NULL)
    {
        CAL_DBG("record don't have properties");
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    // uri를 통해, property_info_s 가져옴
    property_info = _cal_view_get_property_info(_record->view_uri, &property_info_count);

    for(i=0;i<property_info_count;i++)
    {
        if (true == _cal_record_check_property_flag(record, property_info[i].property_id , CAL_PROPERTY_FLAG_DIRTY) )
        {
            field_name = property_info[i].fields;

            if (field_name == NULL)
            {
                continue;
            }

            if (CAL_PROPERTY_CHECK_DATA_TYPE(property_info[i].property_id,CAL_PROPERTY_DATA_TYPE_INT) == true)
            {
                int tmp=0;
                ret = calendar_record_get_int(record,property_info[i].property_id,&tmp);
                if (ret != CALENDAR_ERROR_NONE)
                    continue;
                if (strlen(out_set) != 0)
                {
                    len += snprintf(out_set+len, sizeof(out_set)-len, ", ");
                }
                len += snprintf(out_set+len, sizeof(out_set)-len, "%s=%d",field_name,tmp);
            }
            else if (CAL_PROPERTY_CHECK_DATA_TYPE(property_info[i].property_id,CAL_PROPERTY_DATA_TYPE_STR) == true)
            {
                char *tmp=NULL;
                ret = calendar_record_get_str(record,property_info[i].property_id,&tmp);
                if (ret != CALENDAR_ERROR_NONE)
                    continue;
                if (strlen(out_set) != 0)
                {
                    len += snprintf(out_set+len, sizeof(out_set)-len, ", ");
                }
                len += snprintf(out_set+len, sizeof(out_set)-len, "%s=?",field_name);
                *bind_text = g_slist_append(*bind_text, tmp);
            }
            else if (CAL_PROPERTY_CHECK_DATA_TYPE(property_info[i].property_id,CAL_PROPERTY_DATA_TYPE_DOUBLE) == true)
            {
                double tmp=0;
                ret = calendar_record_get_double(record,property_info[i].property_id,&tmp);
                if (ret != CALENDAR_ERROR_NONE)
                    continue;
                if (strlen(out_set) != 0)
                {
                    len += snprintf(out_set+len, sizeof(out_set)-len, ", ");
                }
                len += snprintf(out_set+len, sizeof(out_set)-len, "%s=%lf",field_name,tmp);
            }
            else if (CAL_PROPERTY_CHECK_DATA_TYPE(property_info[i].property_id,CAL_PROPERTY_DATA_TYPE_LLI) == true)
            {
                long long int tmp=0;
                ret = calendar_record_get_lli(record,property_info[i].property_id,&tmp);
                if (ret != CALENDAR_ERROR_NONE)
                    continue;
                if (strlen(out_set) != 0)
                {
                    len += snprintf(out_set+len, sizeof(out_set)-len, ", ");
                }
                len += snprintf(out_set+len, sizeof(out_set)-len, "%s=%lld",field_name,tmp);
            }
            else if (CAL_PROPERTY_CHECK_DATA_TYPE(property_info[i].property_id,CAL_PROPERTY_DATA_TYPE_CALTIME) == true)
            {
                /*
                 * field_name
                 * dtstart_type, dtstart_utime, dtstart_datetime
                 *      -> dtstart_type=%d, dtstart_utime=%lli, dtstart_datetime=?
                 * dtend_type, dtend_utime, dtend_datetime
                 *      -> dtend_type=%d, dtend_utime=%lli, dtend_datetime=?
                 * until_type, until_utime, until_datetime
                 *      -> until_type=%d, until_utime=%lli, until_datetime=?
                 */
                calendar_time_s tmp = {0,};
                const char *timetype_field = NULL;
                const char *utime_field = NULL;
                const char *datetime_field = NULL;
                timetype_field = __cal_db_query_get_timetype_field_name(field_name);
                utime_field = __cal_db_query_get_utime_field_name(field_name);
                datetime_field = __cal_db_query_get_datetime_field_name(field_name);
                ret = calendar_record_get_caltime(record,property_info[i].property_id,&tmp);
                if (ret != CALENDAR_ERROR_NONE)
                    continue;
                if(tmp.type == CALENDAR_TIME_UTIME)
                {
                    if (strlen(out_set) != 0)
                    {
                        len += snprintf(out_set+len, sizeof(out_set)-len, ", ");
                    }
                    len += snprintf(out_set+len, sizeof(out_set)-len, "%s=%d, %s=%lld",
                            timetype_field, CALENDAR_TIME_UTIME, utime_field, tmp.time.utime);
                }
                else
                {
                    char *bind_tmp = NULL;
                    char bind_datetime[32] = {0};
                    snprintf(bind_datetime, sizeof(bind_datetime), "%04d%02d%02d",
                            tmp.time.date.year,
                            tmp.time.date.month,
                            tmp.time.date.mday);

                    if (strlen(out_set) != 0)
                    {
                        len += snprintf(out_set+len, sizeof(out_set)-len, ", ");
                    }
                    len += snprintf(out_set+len, sizeof(out_set)-len, "%s=%d, %s=?",
							timetype_field, CALENDAR_TIME_LOCALTIME, datetime_field);

                    bind_tmp= strdup(bind_datetime);
                    *bind_text = g_slist_append(*bind_text, bind_tmp);
                }
            }
        }
    }

    *set = strdup(out_set);
    CAL_DBG("set=%s",*set);

    return CALENDAR_ERROR_NONE;
}

int _cal_db_query_create_projection_update_set_with_property(
        calendar_record_h record, unsigned int *properties, int properties_count,
        char **set, GSList **bind_text)
{
    cal_record_s *_record = NULL;
    int i = 0;
    const cal_property_info_s* property_info = NULL;
    int property_info_count = 0;
    char out_set[CAL_DB_SQL_MAX_LEN] = {0};
    int len = 0;
    const char *field_name;
    int ret = CALENDAR_ERROR_NONE;

    retv_if(record == NULL, -1);

    _record = (cal_record_s *)record;

    if (_record->properties_max_count == 0 || _record->properties_flags == NULL)
    {
        return CALENDAR_ERROR_NONE;
    }

    // uri를 통해, property_info_s 가져옴
    property_info = _cal_view_get_property_info(_record->view_uri, &property_info_count);

    for(i=0;i<property_info_count;i++)
    {
        int j=0;
        bool flag = false;
        for(j=0;j<properties_count;j++)
        {
            if( property_info[i].property_id == properties[j] )
            {
                flag = true;
                break;
            }
        }
        if (true == flag &&
                ( true == _cal_record_check_property_flag(record, property_info[i].property_id , CAL_PROPERTY_FLAG_DIRTY))  )
        {
            field_name = property_info[i].fields;

            if (field_name == NULL)
            {
                continue;
            }

            if (CAL_PROPERTY_CHECK_DATA_TYPE(property_info[i].property_id,CAL_PROPERTY_DATA_TYPE_INT) == true)
            {
                int tmp=0;
                ret = calendar_record_get_int(record,property_info[i].property_id,&tmp);
                if (ret != CALENDAR_ERROR_NONE)
                    continue;
                if (strlen(out_set) != 0)
                {
                    len += snprintf(out_set+len, sizeof(out_set)-len, ", ");
                }
                len += snprintf(out_set+len, sizeof(out_set)-len, "%s=%d",field_name,tmp);
            }
            else if (CAL_PROPERTY_CHECK_DATA_TYPE(property_info[i].property_id,CAL_PROPERTY_DATA_TYPE_STR) == true)
            {
                char *tmp=NULL;
                ret = calendar_record_get_str(record,property_info[i].property_id,&tmp);
                if (ret != CALENDAR_ERROR_NONE)
                    continue;
                if (strlen(out_set) != 0)
                {
                    len += snprintf(out_set+len, sizeof(out_set)-len, ", ");
                }
                len += snprintf(out_set+len, sizeof(out_set)-len, "%s=?",field_name);
                *bind_text = g_slist_append(*bind_text, tmp);
            }
            else if (CAL_PROPERTY_CHECK_DATA_TYPE(property_info[i].property_id,CAL_PROPERTY_DATA_TYPE_DOUBLE) == true)
            {
                double tmp=0;
                ret = calendar_record_get_double(record,property_info[i].property_id,&tmp);
                if (ret != CALENDAR_ERROR_NONE)
                    continue;
                if (strlen(out_set) != 0)
                {
                    len += snprintf(out_set+len, sizeof(out_set)-len, ", ");
                }
                len += snprintf(out_set+len, sizeof(out_set)-len, "%s=%lf",field_name,tmp);
            }
            else if (CAL_PROPERTY_CHECK_DATA_TYPE(property_info[i].property_id,CAL_PROPERTY_DATA_TYPE_LLI) == true)
            {
                long long int tmp=0;
                ret = calendar_record_get_lli(record,property_info[i].property_id,&tmp);
                if (ret != CALENDAR_ERROR_NONE)
                    continue;
                if (strlen(out_set) != 0)
                {
                    len += snprintf(out_set+len, sizeof(out_set)-len, ", ");
                }
                len += snprintf(out_set+len, sizeof(out_set)-len, "%s=%lld",field_name,tmp);
            }
            else if (CAL_PROPERTY_CHECK_DATA_TYPE(property_info[i].property_id,CAL_PROPERTY_DATA_TYPE_CALTIME) == true)
            {
                /*
                 * field_name
                 * dtstart_type, dtstart_utime, dtstart_datetime
                 *      -> dtstart_type=%d, dtstart_utime=%lli, dtstart_datetime=?
                 * dtend_type, dtend_utime, dtend_datetime
                 *      -> dtend_type=%d, dtend_utime=%lli, dtend_datetime=?
                 * until_type, until_utime, until_datetime
                 *      -> until_type=%d, until_utime=%lli, until_datetime=?
                 */
                calendar_time_s tmp = {0,};
                const char *timetype_field = NULL;
                const char *utime_field = NULL;
                const char *datetime_field = NULL;
                timetype_field = __cal_db_query_get_timetype_field_name(field_name);
                utime_field = __cal_db_query_get_utime_field_name(field_name);
                datetime_field = __cal_db_query_get_datetime_field_name(field_name);
                ret = calendar_record_get_caltime(record,property_info[i].property_id,&tmp);
                if (ret != CALENDAR_ERROR_NONE)
                    continue;
                if(tmp.type == CALENDAR_TIME_UTIME)
                {
                    if (strlen(out_set) != 0)
                    {
                        len += snprintf(out_set+len, sizeof(out_set)-len, ", ");
                    }
                    len += snprintf(out_set+len, sizeof(out_set)-len, "%s=%d, %s=%lld",
                            timetype_field,CALENDAR_TIME_UTIME,utime_field,tmp.time.utime);
                }
                else
                {
                    char *bind_tmp = NULL;
                    char bind_datetime[32] = {0};
                    snprintf(bind_datetime, sizeof(bind_datetime), "%04d%02d%02d",
                            tmp.time.date.year,
                            tmp.time.date.month,
                            tmp.time.date.mday);

                    if (strlen(out_set) != 0)
                    {
                        len += snprintf(out_set+len, sizeof(out_set)-len, ", ");
                    }
                    len += snprintf(out_set+len, sizeof(out_set)-len, "%s=%d, %s=?",
							timetype_field, CALENDAR_TIME_LOCALTIME, datetime_field);

                    bind_tmp= strdup(bind_datetime);
                    *bind_text = g_slist_append(*bind_text, bind_tmp);
                }
            }
        }
    }

    *set = strdup(out_set);

    return CALENDAR_ERROR_NONE;
}

static int __cal_db_query_create_composite_condition(cal_composite_filter_s *com_filter, char **condition, GSList **bind_text)
{
    GSList *cursor_filter = NULL;
    GSList *cursor_ops = NULL;
    calendar_filter_operator_e op;
    int len = 0;
    char *cond = NULL, out_cond[CAL_DB_SQL_MAX_LEN] = {0,};
    GSList *binds = NULL, *binds2 = NULL;
    cal_filter_s *filter;
    int ret = CALENDAR_ERROR_NONE;

	if (com_filter == NULL || com_filter->filters == NULL)
	{
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

    filter = (cal_filter_s *)com_filter->filters->data;
    if(filter->filter_type == CAL_FILTER_COMPOSITE)
    {
        ret = __cal_db_query_create_composite_condition((cal_composite_filter_s*)filter, &cond, &binds);
    }
    else
    {
        ret = __cal_db_query_create_attribute_condition(com_filter, (cal_attribute_filter_s*)filter, &cond, &binds);
    }

    if (ret != CALENDAR_ERROR_NONE)
    {
        ERR("__cal_db_query_create_attribute_condition fail");
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    cursor_filter = com_filter->filters->next;

    len = 0;
    len = snprintf(out_cond, sizeof(out_cond), "(%s)", cond);
    CAL_FREE(cond);

    for(cursor_ops=com_filter->filter_ops; cursor_ops && cursor_filter;
            cursor_filter=cursor_filter->next, cursor_ops=cursor_ops->next)
    {
        filter = (cal_filter_s *)cursor_filter->data;
        if(filter->filter_type == CAL_FILTER_COMPOSITE)
        {
            ret = __cal_db_query_create_composite_condition((cal_composite_filter_s*)filter, &cond, &binds2);
        }
        else
        {
            ret = __cal_db_query_create_attribute_condition(com_filter, (cal_attribute_filter_s*)filter, &cond, &binds2);
        }

        if (ret != CALENDAR_ERROR_NONE)
        {
            ERR("__cal_db_query_create_attribute_condition fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }

        op = (calendar_filter_operator_e)cursor_ops->data;
        if (op == CALENDAR_FILTER_OPERATOR_AND)
            len += snprintf(out_cond+len, sizeof(out_cond)-len, " AND (%s)", cond);
        else
            len += snprintf(out_cond+len, sizeof(out_cond)-len, " OR (%s)", cond);

        if(binds2)
            binds = g_slist_concat(binds, binds2);
        binds2 = NULL;

        CAL_FREE(cond);
    }

    *condition = strdup(out_cond);
    *bind_text = binds;

    return CALENDAR_ERROR_NONE;
}

static int __cal_db_query_create_attribute_condition(cal_composite_filter_s *com_filter,
        cal_attribute_filter_s *filter, char **condition, GSList **bind_text)
{
    int ret;
    char *cond = NULL;

    retv_if(NULL == filter, CALENDAR_ERROR_INVALID_PARAMETER);

    switch (filter->filter_type)
    {
    case CAL_FILTER_INT:
        ret = __cal_db_query_create_int_condition(com_filter, filter, &cond);
        break;
    case CAL_FILTER_STR:
        ret = __cal_db_query_create_str_condition(com_filter, filter, &cond, bind_text);
        break;
    case CAL_FILTER_DOUBLE:
        ret = __cal_db_query_create_double_condition(com_filter, filter, &cond);
        break;
    case CAL_FILTER_LLI:
        ret = __cal_db_query_create_lli_condition(com_filter, filter, &cond);
        break;
    case CAL_FILTER_CALTIME:
        ret = __cal_db_query_create_caltime_condition(com_filter, filter, &cond);
        break;
    default :
        ERR("The filter type is not supported (%d)", filter->filter_type);
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    if (CALENDAR_ERROR_NONE == ret)
        *condition = (cond);

    return ret;
}

static int __cal_db_query_create_int_condition(cal_composite_filter_s *com_filter,
        cal_attribute_filter_s *filter, char **condition )
{
    const char *field_name;
    char out_cond[CAL_DB_SQL_MAX_LEN] = {0};

    field_name = __cal_db_query_get_property_field_name(com_filter->properties,
                            com_filter->property_count, filter->property_id);
    retvm_if(NULL == field_name, CALENDAR_ERROR_INVALID_PARAMETER,
            "Invalid parameter : property id(%d)", filter->property_id);

    switch(filter->match)
    {
    case CALENDAR_MATCH_EQUAL:
        snprintf(out_cond, sizeof(out_cond), "%s = %d", field_name, filter->value.i);
        break;
    case CALENDAR_MATCH_GREATER_THAN:
        snprintf(out_cond, sizeof(out_cond), "%s > %d", field_name, filter->value.i);
        break;
    case CALENDAR_MATCH_GREATER_THAN_OR_EQUAL:
        snprintf(out_cond, sizeof(out_cond), "%s >= %d", field_name, filter->value.i);
        break;
    case CALENDAR_MATCH_LESS_THAN:
        snprintf(out_cond, sizeof(out_cond), "%s < %d", field_name, filter->value.i);
        break;
    case CALENDAR_MATCH_LESS_THAN_OR_EQUAL:
        snprintf(out_cond, sizeof(out_cond), "%s <= %d", field_name, filter->value.i);
        break;
    case CALENDAR_MATCH_NONE:
        snprintf(out_cond, sizeof(out_cond), "%s IS NULL", field_name);
        break;
    case CALENDAR_MATCH_NOT_EQUAL:
        snprintf(out_cond, sizeof(out_cond), "%s <> %d", field_name, filter->value.i);
        break;
    default :
        ERR("Invalid parameter : int match rule(%d) is not supported", filter->match);
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    *condition = strdup(out_cond);
    return CALENDAR_ERROR_NONE;
}

static int __cal_db_query_create_double_condition(cal_composite_filter_s *com_filter,
        cal_attribute_filter_s *filter, char **condition )
{
    const char *field_name;
    char out_cond[CAL_DB_SQL_MAX_LEN] = {0};

    field_name = __cal_db_query_get_property_field_name(com_filter->properties,
                            com_filter->property_count, filter->property_id);
    retvm_if(NULL == field_name, CALENDAR_ERROR_INVALID_PARAMETER,
            "Invalid parameter : property id(%d)", filter->property_id);

    switch(filter->match)
    {
    case CALENDAR_MATCH_EQUAL:
        snprintf(out_cond, sizeof(out_cond), "%s = %lf", field_name, filter->value.d);
        break;
    case CALENDAR_MATCH_GREATER_THAN:
        snprintf(out_cond, sizeof(out_cond), "%s > %lf", field_name, filter->value.d);
        break;
    case CALENDAR_MATCH_GREATER_THAN_OR_EQUAL:
        snprintf(out_cond, sizeof(out_cond), "%s >= %lf", field_name, filter->value.d);
        break;
    case CALENDAR_MATCH_LESS_THAN:
        snprintf(out_cond, sizeof(out_cond), "%s < %lf", field_name, filter->value.d);
        break;
    case CALENDAR_MATCH_LESS_THAN_OR_EQUAL:
        snprintf(out_cond, sizeof(out_cond), "%s <= %lf", field_name, filter->value.d);
        break;
    case CALENDAR_MATCH_NONE:
        snprintf(out_cond, sizeof(out_cond), "%s IS NULL", field_name);
        break;
    case CALENDAR_MATCH_NOT_EQUAL:
        snprintf(out_cond, sizeof(out_cond), "%s <> %lf", field_name, filter->value.d);
        break;
    default :
        ERR("Invalid parameter : int match rule(%d) is not supported", filter->match);
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    *condition = strdup(out_cond);
    return CALENDAR_ERROR_NONE;
}

static int __cal_db_query_create_lli_condition(cal_composite_filter_s *com_filter,
        cal_attribute_filter_s *filter, char **condition )
{
    const char *field_name;
    char out_cond[CAL_DB_SQL_MAX_LEN] = {0};

    field_name = __cal_db_query_get_property_field_name(com_filter->properties,
                            com_filter->property_count, filter->property_id);
    retvm_if(NULL == field_name, CALENDAR_ERROR_INVALID_PARAMETER,
            "Invalid parameter : property id(%d)", filter->property_id);

    switch(filter->match)
    {
    case CALENDAR_MATCH_EQUAL:
        snprintf(out_cond, sizeof(out_cond), "%s = %lld", field_name, filter->value.lli);
        break;
    case CALENDAR_MATCH_GREATER_THAN:
        snprintf(out_cond, sizeof(out_cond), "%s > %lld", field_name, filter->value.lli);
        break;
    case CALENDAR_MATCH_GREATER_THAN_OR_EQUAL:
        snprintf(out_cond, sizeof(out_cond), "%s >= %lld", field_name, filter->value.lli);
        break;
    case CALENDAR_MATCH_LESS_THAN:
        snprintf(out_cond, sizeof(out_cond), "%s < %lld", field_name, filter->value.lli);
        break;
    case CALENDAR_MATCH_LESS_THAN_OR_EQUAL:
        snprintf(out_cond, sizeof(out_cond), "%s <= %lld", field_name, filter->value.lli);
        break;
    case CALENDAR_MATCH_NONE:
        snprintf(out_cond, sizeof(out_cond), "%s IS NULL", field_name);
        break;
    case CALENDAR_MATCH_NOT_EQUAL:
        snprintf(out_cond, sizeof(out_cond), "%s <> %lld", field_name, filter->value.lli);
        break;
    default :
        ERR("Invalid parameter : int match rule(%d) is not supported", filter->match);
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    *condition = strdup(out_cond);
    return CALENDAR_ERROR_NONE;
}

static int __cal_db_query_create_caltime_condition(cal_composite_filter_s *com_filter,
        cal_attribute_filter_s *filter, char **condition )
{
    const char *field_name;
    char out_cond[CAL_DB_SQL_MAX_LEN] = {0};
    const char *tmp = NULL;

    field_name = __cal_db_query_get_property_field_name(com_filter->properties,
                            com_filter->property_count, filter->property_id);
    retvm_if(NULL == field_name, CALENDAR_ERROR_INVALID_PARAMETER,
            "Invalid parameter : property id(%d)", filter->property_id);

    if (filter->value.caltime.type == CALENDAR_TIME_UTIME )
    {
        tmp = __cal_db_query_get_utime_field_name(field_name);
        if (tmp == NULL)
        {
            tmp = field_name;
        }
        switch(filter->match)
        {
        case CALENDAR_MATCH_EQUAL:
            snprintf(out_cond, sizeof(out_cond), "%s = %lld", tmp, filter->value.caltime.time.utime);
            break;
        case CALENDAR_MATCH_GREATER_THAN:
            snprintf(out_cond, sizeof(out_cond), "%s > %lld", tmp, filter->value.caltime.time.utime);
            break;
        case CALENDAR_MATCH_GREATER_THAN_OR_EQUAL:
            snprintf(out_cond, sizeof(out_cond), "%s >= %lld", tmp, filter->value.caltime.time.utime);
            break;
        case CALENDAR_MATCH_LESS_THAN:
            snprintf(out_cond, sizeof(out_cond), "%s < %lld", tmp, filter->value.caltime.time.utime);
            break;
        case CALENDAR_MATCH_LESS_THAN_OR_EQUAL:
            snprintf(out_cond, sizeof(out_cond), "%s <= %lld", tmp, filter->value.caltime.time.utime);
            break;
        case CALENDAR_MATCH_NONE:
            snprintf(out_cond, sizeof(out_cond), "%s IS NULL", tmp);
            break;
        case CALENDAR_MATCH_NOT_EQUAL:
            snprintf(out_cond, sizeof(out_cond), "%s <> %lld", tmp, filter->value.caltime.time.utime);
            break;
        default :
            ERR("Invalid parameter : int match rule(%d) is not supported", filter->match);
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    else if (filter->value.caltime.type == CALENDAR_TIME_LOCALTIME )
    {
        char sdate[32] = {0};
        snprintf(sdate, sizeof(sdate), "%4d%02d%02d", filter->value.caltime.time.date.year
                , filter->value.caltime.time.date.month, filter->value.caltime.time.date.mday);
        tmp = __cal_db_query_get_datetime_field_name(field_name);
        if (tmp == NULL)
        {
            tmp = field_name;
        }
        switch(filter->match)
        {
        case CALENDAR_MATCH_EQUAL:
            snprintf(out_cond, sizeof(out_cond), "%s = %s", tmp, sdate);
            break;
        case CALENDAR_MATCH_GREATER_THAN:
            snprintf(out_cond, sizeof(out_cond), "%s > %s", tmp, sdate);
            break;
        case CALENDAR_MATCH_GREATER_THAN_OR_EQUAL:
            snprintf(out_cond, sizeof(out_cond), "%s >= %s", tmp, sdate);
            break;
        case CALENDAR_MATCH_LESS_THAN:
            snprintf(out_cond, sizeof(out_cond), "%s < %s", tmp, sdate);
            break;
        case CALENDAR_MATCH_LESS_THAN_OR_EQUAL:
            snprintf(out_cond, sizeof(out_cond), "%s <= %s", tmp, sdate);
            break;
        case CALENDAR_MATCH_NOT_EQUAL:
            snprintf(out_cond, sizeof(out_cond), "%s <> %s", tmp, sdate);
            break;
        case CALENDAR_MATCH_NONE:
        default :
            ERR("Invalid parameter : int match rule(%d) is not supported", filter->match);
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    else
    {
        ERR("Invalid parameter : property id(%d)", filter->property_id);
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    *condition = strdup(out_cond);
    return CALENDAR_ERROR_NONE;
}

static int __cal_db_query_create_str_condition(cal_composite_filter_s *com_filter,
        cal_attribute_filter_s *filter, char **condition, GSList **bind_text)
{
    const char *field_name;
    char out_cond[CAL_DB_SQL_MAX_LEN] = {0};

    field_name = __cal_db_query_get_property_field_name(com_filter->properties,
                            com_filter->property_count, filter->property_id);
    retvm_if(NULL == field_name, CALENDAR_ERROR_INVALID_PARAMETER,
            "Invalid parameter : property id(%d)", filter->property_id);

    switch(filter->match)
    {
    case CALENDAR_MATCH_EXACTLY:
        snprintf(out_cond, sizeof(out_cond), "%s = ?", field_name);
        break;
    case CALENDAR_MATCH_FULLSTRING:
        snprintf(out_cond, sizeof(out_cond), "%s LIKE ?", field_name);
        break;
    case CALENDAR_MATCH_CONTAINS:
        snprintf(out_cond, sizeof(out_cond), "%s LIKE '%%' || ? || '%%'", field_name);
        break;
    case CALENDAR_MATCH_STARTSWITH:
        snprintf(out_cond, sizeof(out_cond), "%s LIKE ? || '%%'", field_name);
        break;
    case CALENDAR_MATCH_ENDSWITH:
        snprintf(out_cond, sizeof(out_cond), "%s LIKE '%%' || ?", field_name);
        break;
    case CALENDAR_MATCH_EXISTS:
        snprintf(out_cond, sizeof(out_cond), "%s IS NOT NULL", field_name);
        break;
    default :
        ERR("Invalid paramter : int match rule (%d) is not supported", filter->match);
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    if (filter->value.s)
    {
        *bind_text = g_slist_append(*bind_text, filter->value.s);
    }
    *condition = strdup(out_cond);
    return CALENDAR_ERROR_NONE;
}

static const char * __cal_db_query_get_property_field_name(const cal_property_info_s *properties,
        int count, unsigned int property_id)
{
    int i;
    for (i=0;i<count;i++)
    {
        cal_property_info_s *p = (cal_property_info_s*)&(properties[i]);
        if (property_id == p->property_id)
        {
            if (p->fields)
                return p->fields;
            else
                return NULL; //ctsvc_get_display_column();
        }
    }
    return NULL;
}

static const char * __cal_db_query_get_utime_field_name(const char* src)
{
    char *tmp1 = NULL;
    int i=0;

    for(i=0;i<CAL_DB_CALTIME_FIELD_MAX;i++)
    {
        tmp1 = strstr(src,__cal_db_utime_field_name[i]);
        if (tmp1 != NULL)
        {
            return __cal_db_utime_field_name[i];
        }
    }

    return NULL;
}

static const char * __cal_db_query_get_datetime_field_name(const char* src)
{
    char *tmp1 = NULL;
    int i=0;

    for(i=0;i<CAL_DB_CALTIME_FIELD_MAX;i++)
    {
        tmp1 = strstr(src,__cal_db_datetime_field_name[i]);
        if (tmp1 != NULL)
        {
            return __cal_db_datetime_field_name[i];
        }
    }

    return NULL;
}

static const char * __cal_db_query_get_timetype_field_name(const char* src)
{
    char *tmp1 = NULL;
    int i=0;

    for(i=0;i<CAL_DB_CALTIME_FIELD_MAX;i++)
    {
        tmp1 = strstr(src,__cal_db_timetype_field_name[i]);
        if (tmp1 != NULL)
        {
            return __cal_db_timetype_field_name[i];
        }
    }

    return NULL;
}

