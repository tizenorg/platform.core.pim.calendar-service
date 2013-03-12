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

static int __cal_record_updated_info_create( calendar_record_h* out_record );
static int __cal_record_updated_info_destroy( calendar_record_h record, bool delete_child );
static int __cal_record_updated_info_clone( calendar_record_h record, calendar_record_h* out_record );
static int __cal_record_updated_info_get_int( calendar_record_h record, unsigned int property_id, int* out_value );
static int __cal_record_updated_info_set_int( calendar_record_h record, unsigned int property_id, int value );

cal_record_plugin_cb_s _cal_record_updated_info_plugin_cb = {
    .create = __cal_record_updated_info_create,
    .destroy = __cal_record_updated_info_destroy,
    .clone = __cal_record_updated_info_clone,
    .get_str = NULL,
    .get_str_p = NULL,
    .get_int = __cal_record_updated_info_get_int,
    .get_double = NULL,
    .get_lli = NULL,
    .get_caltime = NULL,
    .set_str = NULL,
    .set_int = __cal_record_updated_info_set_int,
    .set_double = NULL,
    .set_lli = NULL,
    .set_caltime = NULL,
    .add_child_record = NULL,
    .remove_child_record = NULL,
    .get_child_record_count = NULL,
    .get_child_record_at_p = NULL,
    .clone_child_record_list = NULL
};

static int __cal_record_updated_info_create( calendar_record_h* out_record )
{
    cal_updated_info_s *temp = NULL;
    int ret= CALENDAR_ERROR_NONE, type = 0;

    type = CAL_RECORD_TYPE_UPDATED_INFO;

    temp = (cal_updated_info_s*)calloc(1,sizeof(cal_updated_info_s));
    retvm_if(NULL == temp, CALENDAR_ERROR_OUT_OF_MEMORY, "malloc(cal_updated_info_s:sch) Failed(%d)", CALENDAR_ERROR_OUT_OF_MEMORY);

    *out_record = (calendar_record_h)temp;

    return ret;
}

static int __cal_record_updated_info_destroy( calendar_record_h record, bool delete_child )
{
    int ret = CALENDAR_ERROR_NONE;

    cal_updated_info_s *temp = (cal_updated_info_s*)(record);

    CAL_FREE(temp);

    return ret;
}

static int __cal_record_updated_info_clone( calendar_record_h record, calendar_record_h* out_record )
{
    cal_updated_info_s *out_data = NULL;
    cal_updated_info_s *src_data = NULL;

    src_data = (cal_updated_info_s*)(record);

    out_data = calloc(1, sizeof(cal_updated_info_s));
    retvm_if(NULL == out_data, CALENDAR_ERROR_OUT_OF_MEMORY, "calloc(cal_updated_info_s) Failed(%d)", CALENDAR_ERROR_OUT_OF_MEMORY);

    CAL_RECORD_COPY_COMMON(&(out_data->common), &(src_data->common));

    out_data->type = src_data->type;
    out_data->id = src_data->id;
    out_data->calendar_id = src_data->calendar_id;
    out_data->version = src_data->version;

    *out_record = (calendar_record_h)out_data;

    return CALENDAR_ERROR_NONE;
}

static int __cal_record_updated_info_get_int( calendar_record_h record, unsigned int property_id, int* out_value )
{
    cal_updated_info_s *rec = (cal_updated_info_s*)(record);

    switch( property_id )
    {
    case CAL_PROPERTY_UPDATED_INFO_ID:
        *out_value = (rec->id);
        break;
    case CAL_PROPERTY_UPDATED_INFO_CALENDAR_ID:
        *out_value = (rec->calendar_id);
        break;
    case CAL_PROPERTY_UPDATED_INFO_TYPE:
        *out_value = (rec->type);
        break;
    case CAL_PROPERTY_UPDATED_INFO_VERSION:
        *out_value = (rec->version);
        break;
    default:
        ASSERT_NOT_REACHED("invalid parameter (property:%d)",property_id);
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    return CALENDAR_ERROR_NONE;
}

static int __cal_record_updated_info_set_int( calendar_record_h record, unsigned int property_id, int value )
{
    cal_updated_info_s *rec = (cal_updated_info_s*)(record);
    switch( property_id )
    {
    case CAL_PROPERTY_UPDATED_INFO_ID:
        (rec->id) = value;
        break;
    case CAL_PROPERTY_UPDATED_INFO_CALENDAR_ID:
        (rec->calendar_id) = value;
        break;
    case CAL_PROPERTY_UPDATED_INFO_TYPE:
        (rec->type) = value;
        break;
    case CAL_PROPERTY_UPDATED_INFO_VERSION:
        (rec->version) = value;
        break;
    default:
        ASSERT_NOT_REACHED("invalid parameter (property:%d)",property_id);
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    return CALENDAR_ERROR_NONE;
}
