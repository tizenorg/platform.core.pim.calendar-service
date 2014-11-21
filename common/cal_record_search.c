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

#include <stdlib.h>     //calloc
#include <stdbool.h>        //bool
#include <string.h>

#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_view.h"

#include "cal_record.h"

static int __cal_record_search_create( calendar_record_h* out_record );
static int __cal_record_search_destroy( calendar_record_h record, bool delete_child );
static int __cal_record_search_clone( calendar_record_h record, calendar_record_h* out_record );
static int __cal_record_search_get_str( calendar_record_h record, unsigned int property_id, char** out_str );
static int __cal_record_search_get_str_p( calendar_record_h record, unsigned int property_id, char** out_str );
static int __cal_record_search_get_int( calendar_record_h record, unsigned int property_id, int* out_value );
static int __cal_record_search_get_double( calendar_record_h record, unsigned int property_id, double* out_value );
static int __cal_record_search_get_lli( calendar_record_h record, unsigned int property_id, long long int* out_value );
static int __cal_record_search_get_caltime( calendar_record_h record, unsigned int property_id, calendar_time_s* out_value );
static int __cal_record_search_set_str( calendar_record_h record, unsigned int property_id, const char* value );
static int __cal_record_search_set_int( calendar_record_h record, unsigned int property_id, int value );
static int __cal_record_search_set_double( calendar_record_h record, unsigned int property_id, double value );
static int __cal_record_search_set_lli( calendar_record_h record, unsigned int property_id, long long int value );
static int __cal_record_search_set_caltime( calendar_record_h record, unsigned int property_id, calendar_time_s value );

cal_record_plugin_cb_s _cal_record_search_plugin_cb = {
    .create = __cal_record_search_create,
    .destroy = __cal_record_search_destroy,
    .clone = __cal_record_search_clone,
    .get_str = __cal_record_search_get_str,
    .get_str_p = __cal_record_search_get_str_p,
    .get_int = __cal_record_search_get_int,
    .get_double = __cal_record_search_get_double,
    .get_lli = __cal_record_search_get_lli,
    .get_caltime = __cal_record_search_get_caltime,
    .set_str = __cal_record_search_set_str,
    .set_int = __cal_record_search_set_int,
    .set_double = __cal_record_search_set_double,
    .set_lli = __cal_record_search_set_lli,
    .set_caltime = __cal_record_search_set_caltime,
    .add_child_record = NULL,
    .remove_child_record = NULL,
    .get_child_record_count = NULL,
    .get_child_record_at_p = NULL,
    .clone_child_record_list = NULL
};

static int __cal_record_search_create( calendar_record_h* out_record )
{
    cal_search_s *temp = NULL;
    int ret= CALENDAR_ERROR_NONE;
    cal_record_type_e type = CAL_RECORD_TYPE_INVALID;

    type = CAL_RECORD_TYPE_SEARCH;

    temp = (cal_search_s*)calloc(1, sizeof(cal_search_s));
    retvm_if(NULL == temp, CALENDAR_ERROR_OUT_OF_MEMORY, "calloc(cal_search_s:sch) Failed(%d)", CALENDAR_ERROR_OUT_OF_MEMORY);

    *out_record = (calendar_record_h)temp;

    return ret;
}

static int __cal_record_search_destroy( calendar_record_h record, bool delete_child )
{
    int ret = CALENDAR_ERROR_NONE;
    GSList *cursor;

    cal_search_s *temp = (cal_search_s*)(record);

    for(cursor = temp->values;cursor;cursor=cursor->next)
    {
        cal_search_value_s *data = cursor->data;
        if (CAL_PROPERTY_CHECK_DATA_TYPE(data->property_id, CAL_PROPERTY_DATA_TYPE_STR) == true)
        {
            CAL_FREE(data->value.s);
        }
        CAL_FREE(data);
    }

    g_slist_free(temp->values);
    CAL_FREE(temp);

    return ret;
}

static int __cal_record_search_clone( calendar_record_h record, calendar_record_h* out_record )
{
    int ret = CALENDAR_ERROR_NONE;
    cal_search_s *out_data = NULL;
    cal_search_s *src_data = NULL;
    GSList *cursor;

    src_data = (cal_search_s*)(record);

    out_data = calloc(1, sizeof(cal_search_s));
    retvm_if(NULL == out_data, CALENDAR_ERROR_OUT_OF_MEMORY, "calloc(cal_search_s) Failed(%d)", CALENDAR_ERROR_OUT_OF_MEMORY);

    CAL_RECORD_COPY_COMMON(&(out_data->common), &(src_data->common));

    for(cursor = src_data->values;cursor;cursor=cursor->next)
    {
        cal_search_value_s *src = cursor->data;
        cal_search_value_s *dest = calloc(1, sizeof(cal_search_value_s));
        if (dest == NULL)
        {
            CAL_FREE(out_data);
            ret = CALENDAR_ERROR_OUT_OF_MEMORY;
            return ret;
        }
        dest->property_id = src->property_id;
        if (CAL_PROPERTY_CHECK_DATA_TYPE(src->property_id, CAL_PROPERTY_DATA_TYPE_STR) == true)
        {
            dest->value.s = SAFE_STRDUP(src->value.s);
        }
        else if (CAL_PROPERTY_CHECK_DATA_TYPE(src->property_id, CAL_PROPERTY_DATA_TYPE_INT) == true)
        {
            dest->value.i = src->value.i;
        }
        else if (CAL_PROPERTY_CHECK_DATA_TYPE(src->property_id, CAL_PROPERTY_DATA_TYPE_DOUBLE) == true)
        {
            dest->value.d = src->value.d;
        }
        else if (CAL_PROPERTY_CHECK_DATA_TYPE(src->property_id, CAL_PROPERTY_DATA_TYPE_LLI) == true)
        {
            dest->value.lli = src->value.lli;
        }
        else if (CAL_PROPERTY_CHECK_DATA_TYPE(src->property_id, CAL_PROPERTY_DATA_TYPE_CALTIME) == true)
        {
            dest->value.caltime = src->value.caltime;
        }
        else
        {
            ASSERT_NOT_REACHED("invalid parameter (property:%d)",src->property_id);
            CAL_FREE(out_data);
            CAL_FREE(dest);
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }

        out_data->values = g_slist_append(out_data->values, dest);
    }

    *out_record = (calendar_record_h)out_data;

    return CALENDAR_ERROR_NONE;
}

static int __cal_record_search_get_str( calendar_record_h record, unsigned int property_id, char** out_str )
{
    cal_search_s *rec = (cal_search_s*)(record);
    GSList *cursor;

    for(cursor = rec->values;cursor;cursor=cursor->next)
    {
        cal_search_value_s *data = cursor->data;
        if (data->property_id == property_id)
        {
            if (CAL_PROPERTY_CHECK_DATA_TYPE(data->property_id, CAL_PROPERTY_DATA_TYPE_STR) == true)
            {
                *out_str = SAFE_STRDUP(data->value.s);
            }
            else
            {
                ASSERT_NOT_REACHED("invalid parameter (property:%d)",data->property_id);
                return CALENDAR_ERROR_INVALID_PARAMETER;
            }
        }
    }

    return CALENDAR_ERROR_NONE;
}

static int __cal_record_search_get_str_p( calendar_record_h record, unsigned int property_id, char** out_str )
{
    cal_search_s *rec = (cal_search_s*)(record);
    GSList *cursor;

    for(cursor = rec->values;cursor;cursor=cursor->next)
    {
        cal_search_value_s *data = cursor->data;
        if (data->property_id == property_id)
        {
            if (CAL_PROPERTY_CHECK_DATA_TYPE(data->property_id, CAL_PROPERTY_DATA_TYPE_STR) == true)
            {
                *out_str = (data->value.s);
            }
            else
            {
                ASSERT_NOT_REACHED("invalid parameter (property:%d)",data->property_id);
                return CALENDAR_ERROR_INVALID_PARAMETER;
            }
        }
    }

    return CALENDAR_ERROR_NONE;
}

static int __cal_record_search_get_int( calendar_record_h record, unsigned int property_id, int* out_value )
{
    cal_search_s *rec = (cal_search_s*)(record);
    GSList *cursor;

    for(cursor = rec->values;cursor;cursor=cursor->next)
    {
        cal_search_value_s *data = cursor->data;
        if (data->property_id == property_id)
        {
            if (CAL_PROPERTY_CHECK_DATA_TYPE(data->property_id, CAL_PROPERTY_DATA_TYPE_INT) == true)
            {
                *out_value = (data->value.i);
            }
            else
            {
                ASSERT_NOT_REACHED("invalid parameter (property:%d)",data->property_id);
                return CALENDAR_ERROR_INVALID_PARAMETER;
            }
        }
    }

    return CALENDAR_ERROR_NONE;
}

static int __cal_record_search_get_double( calendar_record_h record, unsigned int property_id, double* out_value )
{
    cal_search_s *rec = (cal_search_s*)(record);
    GSList *cursor;

    for(cursor = rec->values;cursor;cursor=cursor->next)
    {
        cal_search_value_s *data = cursor->data;
        if (data->property_id == property_id)
        {
            if (CAL_PROPERTY_CHECK_DATA_TYPE(data->property_id, CAL_PROPERTY_DATA_TYPE_DOUBLE) == true)
            {
                *out_value = (data->value.d);
            }
            else
            {
                ASSERT_NOT_REACHED("invalid parameter (property:%d)",data->property_id);
                return CALENDAR_ERROR_INVALID_PARAMETER;
            }
        }
    }

    return CALENDAR_ERROR_NONE;
}

static int __cal_record_search_get_lli( calendar_record_h record, unsigned int property_id, long long int* out_value )
{
    cal_search_s *rec = (cal_search_s*)(record);
    GSList *cursor;

    for(cursor = rec->values;cursor;cursor=cursor->next)
    {
        cal_search_value_s *data = cursor->data;
        if (data->property_id == property_id)
        {
            if (CAL_PROPERTY_CHECK_DATA_TYPE(data->property_id, CAL_PROPERTY_DATA_TYPE_LLI) == true)
            {
                *out_value = (data->value.lli);
            }
            else
            {
                ASSERT_NOT_REACHED("invalid parameter (property:%d)",data->property_id);
                return CALENDAR_ERROR_INVALID_PARAMETER;
            }
        }
    }

    return CALENDAR_ERROR_NONE;
}

static int __cal_record_search_get_caltime( calendar_record_h record, unsigned int property_id, calendar_time_s* out_value )
{
    cal_search_s *rec = (cal_search_s*)(record);
    GSList *cursor;

    for(cursor = rec->values;cursor;cursor=cursor->next)
    {
        cal_search_value_s *data = cursor->data;
        if (data->property_id == property_id)
        {
            if (CAL_PROPERTY_CHECK_DATA_TYPE(data->property_id, CAL_PROPERTY_DATA_TYPE_CALTIME) == true)
            {
                *out_value = (data->value.caltime);
            }
            else
            {
                ASSERT_NOT_REACHED("invalid parameter (property:%d)",data->property_id);
                return CALENDAR_ERROR_INVALID_PARAMETER;
            }
        }
    }

    return CALENDAR_ERROR_NONE;
}

static int __cal_record_search_set_str( calendar_record_h record, unsigned int property_id, const char* value )
{
    cal_search_s *rec = (cal_search_s*)(record);
    GSList *cursor;
    cal_search_value_s *data = NULL;

    for(cursor = rec->values;cursor;cursor=cursor->next)
    {
        data = cursor->data;
        if (data->property_id == property_id)
        {
            if (CAL_PROPERTY_CHECK_DATA_TYPE(data->property_id, CAL_PROPERTY_DATA_TYPE_STR) == true)
            {
                CAL_FREE(data->value.s);
                (data->value.s) = SAFE_STRDUP(value);
                return CALENDAR_ERROR_NONE;
            }
            else
            {
                ASSERT_NOT_REACHED("invalid parameter (property:%d)",data->property_id);
                return CALENDAR_ERROR_INVALID_PARAMETER;
            }
        }
    }

    data = calloc(1, sizeof(cal_search_value_s));
    retvm_if(NULL == data, CALENDAR_ERROR_OUT_OF_MEMORY, "calloc(cal_search_s) Failed");
    data->property_id = property_id;
    data->value.s = SAFE_STRDUP(value);
    rec->values = g_slist_append(rec->values, data);

    return CALENDAR_ERROR_NONE;
}

static int __cal_record_search_set_int( calendar_record_h record, unsigned int property_id, int value )
{
    cal_search_s *rec = (cal_search_s*)(record);
    GSList *cursor;
    cal_search_value_s *data = NULL;

    for(cursor = rec->values;cursor;cursor=cursor->next)
    {
        data = cursor->data;
        if (data->property_id == property_id)
        {
            if (CAL_PROPERTY_CHECK_DATA_TYPE(data->property_id, CAL_PROPERTY_DATA_TYPE_INT) == true)
            {
                (data->value.i) = value;
                return CALENDAR_ERROR_NONE;
            }
            else
            {
                ASSERT_NOT_REACHED("invalid parameter (property:%d)",data->property_id);
                return CALENDAR_ERROR_INVALID_PARAMETER;
            }
        }
    }

    data = calloc(1, sizeof(cal_search_value_s));
    retvm_if(NULL == data, CALENDAR_ERROR_OUT_OF_MEMORY, "calloc(cal_search_s) Failed");
    data->property_id = property_id;
    (data->value.i) = value;
    rec->values = g_slist_append(rec->values, data);

    return CALENDAR_ERROR_NONE;
}

static int __cal_record_search_set_double( calendar_record_h record, unsigned int property_id, double value )
{
    cal_search_s *rec = (cal_search_s*)(record);
    GSList *cursor;
    cal_search_value_s *data = NULL;

    for(cursor = rec->values;cursor;cursor=cursor->next)
    {
        data = cursor->data;
        if (data->property_id == property_id)
        {
            if (CAL_PROPERTY_CHECK_DATA_TYPE(data->property_id, CAL_PROPERTY_DATA_TYPE_DOUBLE) == true)
            {
                (data->value.d) = value;
                return CALENDAR_ERROR_NONE;
            }
            else
            {
                ASSERT_NOT_REACHED("invalid parameter (property:%d)",data->property_id);
                return CALENDAR_ERROR_INVALID_PARAMETER;
            }
        }
    }

    data = calloc(1, sizeof(cal_search_value_s));
    retvm_if(NULL == data, CALENDAR_ERROR_OUT_OF_MEMORY, "calloc(cal_search_s) Failed");
    data->property_id = property_id;
    (data->value.d) = value;
    rec->values = g_slist_append(rec->values, data);

    return CALENDAR_ERROR_NONE;
}

static int __cal_record_search_set_lli( calendar_record_h record, unsigned int property_id, long long int value )
{
    cal_search_s *rec = (cal_search_s*)(record);
    GSList *cursor;
    cal_search_value_s *data = NULL;

    for(cursor = rec->values;cursor;cursor=cursor->next)
    {
        data = cursor->data;
        if (data->property_id == property_id)
        {
            if (CAL_PROPERTY_CHECK_DATA_TYPE(data->property_id, CAL_PROPERTY_DATA_TYPE_LLI) == true)
            {
                (data->value.lli) = value;
                return CALENDAR_ERROR_NONE;
            }
            else
            {
                ASSERT_NOT_REACHED("invalid parameter (property:%d)",data->property_id);
                return CALENDAR_ERROR_INVALID_PARAMETER;
            }
        }
    }

    data = calloc(1, sizeof(cal_search_value_s));
    retvm_if(NULL == data, CALENDAR_ERROR_OUT_OF_MEMORY, "calloc(cal_search_s) Failed");
    data->property_id = property_id;
    (data->value.lli) = value;
    rec->values = g_slist_append(rec->values, data);

    return CALENDAR_ERROR_NONE;
}

static int __cal_record_search_set_caltime( calendar_record_h record, unsigned int property_id, calendar_time_s value )
{
    cal_search_s *rec = (cal_search_s*)(record);
    GSList *cursor;
    cal_search_value_s *data = NULL;

    for(cursor = rec->values;cursor;cursor=cursor->next)
    {
        data = cursor->data;
        if (data->property_id == property_id)
        {
            if (CAL_PROPERTY_CHECK_DATA_TYPE(data->property_id, CAL_PROPERTY_DATA_TYPE_CALTIME) == true)
            {
                (data->value.caltime) = value;
                return CALENDAR_ERROR_NONE;
            }
            else
            {
                ASSERT_NOT_REACHED("invalid parameter (property:%d)",data->property_id);
                return CALENDAR_ERROR_INVALID_PARAMETER;
            }
        }
    }

    data = calloc(1, sizeof(cal_search_value_s));
    retvm_if(NULL == data, CALENDAR_ERROR_OUT_OF_MEMORY, "calloc(cal_search_s) Failed");
    data->property_id = property_id;
    (data->value.caltime) = value;
    rec->values = g_slist_append(rec->values, data);

    return CALENDAR_ERROR_NONE;
}

