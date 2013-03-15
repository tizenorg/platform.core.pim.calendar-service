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

#include <stdlib.h> //calloc
#include <string.h>

#include "calendar_query.h"
#include "calendar_filter.h"
#include "calendar_list.h"

#include "cal_ipc_marshal.h"
#include "cal_record.h"
#include "cal_internal.h"
#include "cal_view.h"

extern cal_ipc_marshal_record_plugin_cb_s _cal_ipc_record_calendar_plugin_cb;
extern cal_ipc_marshal_record_plugin_cb_s _cal_ipc_record_event_plugin_cb;
extern cal_ipc_marshal_record_plugin_cb_s _cal_ipc_record_todo_plugin_cb;
extern cal_ipc_marshal_record_plugin_cb_s _cal_ipc_record_alarm_plugin_cb;
extern cal_ipc_marshal_record_plugin_cb_s _cal_ipc_record_attendee_plugin_cb;
extern cal_ipc_marshal_record_plugin_cb_s _cal_ipc_record_timezone_plugin_cb;
extern cal_ipc_marshal_record_plugin_cb_s _cal_ipc_record_updated_info_plugin_cb;
extern cal_ipc_marshal_record_plugin_cb_s _cal_ipc_record_instance_normal_plugin_cb;
extern cal_ipc_marshal_record_plugin_cb_s _cal_ipc_record_instance_allday_plugin_cb;
extern cal_ipc_marshal_record_plugin_cb_s _cal_ipc_record_search_plugin_cb;
extern cal_ipc_marshal_record_plugin_cb_s _cal_ipc_record_extended_plugin_cb;

static cal_ipc_marshal_record_plugin_cb_s* __cal_ipc_marshal_get_plugin_cb(cal_record_type_e type);

static int __cal_ipc_unmarshal_composite_filter(const pims_ipc_data_h ipc_data, cal_composite_filter_s* filter);
static int __cal_ipc_marshal_composite_filter(const cal_composite_filter_s* filter, pims_ipc_data_h ipc_data);
static int __cal_ipc_unmarshal_attribute_filter(const pims_ipc_data_h ipc_data, const cal_filter_type_e filter_type, cal_attribute_filter_s* filter);
static int __cal_ipc_marshal_attribute_filter(const cal_attribute_filter_s* filter, pims_ipc_data_h ipc_data);

static cal_ipc_marshal_record_plugin_cb_s* __cal_ipc_marshal_get_plugin_cb(cal_record_type_e type)
{
    switch (type)
    {
    case CAL_RECORD_TYPE_CALENDAR:
        return (&_cal_ipc_record_calendar_plugin_cb);
    case CAL_RECORD_TYPE_EVENT:
        return (&_cal_ipc_record_event_plugin_cb);
    case CAL_RECORD_TYPE_TODO:
        return (&_cal_ipc_record_todo_plugin_cb);
    case CAL_RECORD_TYPE_ALARM:
        return (&_cal_ipc_record_alarm_plugin_cb);
    case CAL_RECORD_TYPE_ATTENDEE:
        return (&_cal_ipc_record_attendee_plugin_cb);
    case CAL_RECORD_TYPE_TIMEZONE:
        return (&_cal_ipc_record_timezone_plugin_cb);
    case CAL_RECORD_TYPE_INSTANCE_NORMAL:
        return (&_cal_ipc_record_instance_normal_plugin_cb);
    case CAL_RECORD_TYPE_INSTANCE_ALLDAY:
        return (&_cal_ipc_record_instance_allday_plugin_cb);
    case CAL_RECORD_TYPE_UPDATED_INFO:
        return (&_cal_ipc_record_updated_info_plugin_cb);
    case CAL_RECORD_TYPE_SEARCH:
        return (&_cal_ipc_record_search_plugin_cb);
    case CAL_RECORD_TYPE_EXTENDED:
        return (&_cal_ipc_record_extended_plugin_cb);
    default:
        return NULL;
    }
}

static int __cal_ipc_unmarshal_composite_filter(const pims_ipc_data_h ipc_data, cal_composite_filter_s* filter)
{
    int ret = CALENDAR_ERROR_NONE;
    unsigned int size = 0;
    char* str = NULL;
    int count =0, i=0;
    cal_filter_type_e filter_type = CAL_FILTER_COMPOSITE;
    calendar_filter_operator_e op = CALENDAR_FILTER_OPERATOR_AND;

/*
    if (_cal_ipc_unmarshal_int(ipc_data,&(filter->filter_type)) != CALENDAR_ERROR_NONE)
    {
        ERR("_cal_ipc_unmarshal fail");
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }
*/
    filter->filter_type = CAL_FILTER_COMPOSITE;

    // view_uri
    str = (char*)pims_ipc_data_get(ipc_data,&size);
    filter->view_uri = strdup(str);

    // filters
    if (_cal_ipc_unmarshal_int(ipc_data,&count) != CALENDAR_ERROR_NONE)
    {
        ERR("_cal_ipc_unmarshal fail");
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    for(i=0;i<count;i++)
    {
        if (_cal_ipc_unmarshal_int(ipc_data,(int*)&filter_type) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            ret = CALENDAR_ERROR_INVALID_PARAMETER;
            goto ERROR_RETURN;
        }
        if (filter_type == CAL_FILTER_COMPOSITE)
        {
            cal_composite_filter_s* com_filter = NULL;
            com_filter = (cal_composite_filter_s*)calloc(1,sizeof(cal_composite_filter_s));
            if (com_filter == NULL)
            {
                ERR("malloc fail");
                ret = CALENDAR_ERROR_OUT_OF_MEMORY;
                goto ERROR_RETURN;
            }
            if (__cal_ipc_unmarshal_composite_filter(ipc_data, com_filter) != CALENDAR_ERROR_NONE)
            {
                ERR("_cal_ipc_unmarshal fail");
                ret = CALENDAR_ERROR_INVALID_PARAMETER;
                CAL_FREE(com_filter);
                goto ERROR_RETURN;
            }
            filter->filters = g_slist_append(filter->filters,com_filter);
        }
        else
        {
            cal_attribute_filter_s* attr_filter = NULL;
            attr_filter = (cal_attribute_filter_s*)calloc(1,sizeof(cal_attribute_filter_s));
            if (attr_filter == NULL)
            {
                ERR("malloc fail");
                ret = CALENDAR_ERROR_OUT_OF_MEMORY;
                goto ERROR_RETURN;
            }
            if (__cal_ipc_unmarshal_attribute_filter(ipc_data, filter_type, attr_filter) != CALENDAR_ERROR_NONE)
            {
                ERR("_cal_ipc_unmarshal fail");
                ret =  CALENDAR_ERROR_INVALID_PARAMETER;
                CAL_FREE(attr_filter);
                goto ERROR_RETURN;
            }
            filter->filters = g_slist_append(filter->filters,attr_filter);
        }
    }

    // filters
    if (_cal_ipc_unmarshal_int(ipc_data,&count) != CALENDAR_ERROR_NONE)
    {
        ERR("_cal_ipc_unmarshal fail");
        ret =  CALENDAR_ERROR_INVALID_PARAMETER;
        goto ERROR_RETURN;
    }

    for(i=0;i<count;i++)
    {
        if (_cal_ipc_unmarshal_int(ipc_data,(int*)&op) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            ret =  CALENDAR_ERROR_INVALID_PARAMETER;
            goto ERROR_RETURN;
        }
        filter->filter_ops = g_slist_append(filter->filter_ops, (void*)op);
    }

    // properties //property_count
    filter->properties = (cal_property_info_s *)_cal_view_get_property_info(filter->view_uri, &filter->property_count);

    return CALENDAR_ERROR_NONE;

ERROR_RETURN:

    if (filter->filters)
    {
        GSList *cursor = NULL;
        for(cursor=filter->filters;cursor;cursor=cursor->next)
        {
            cal_filter_s *src = (cal_filter_s*)cursor->data;
            CAL_FREE(src);
        }
        g_slist_free(filter->filters);
    }

    if (filter->filter_ops)
    {
        g_slist_free(filter->filter_ops);
    }

    return ret;
}

static int __cal_ipc_marshal_composite_filter(const cal_composite_filter_s* filter, pims_ipc_data_h ipc_data)
{
    if (_cal_ipc_marshal_int((filter->filter_type),ipc_data) != CALENDAR_ERROR_NONE)
    {
        ERR("_cal_ipc_marshal fail");
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    // view_uri
    int length = strlen(filter->view_uri);
    if (pims_ipc_data_put(ipc_data,(void*)filter->view_uri,length+1) < 0)
    {
        ERR("_cal_ipc_marshal fail");
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }
    // filter->filters
    if (filter->filters)
    {
        int count = g_slist_length(filter->filters);
        GSList *cursor = filter->filters;
        cal_filter_s* child_filter;

        if (_cal_ipc_marshal_int(count,ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
        while (cursor)
        {
            child_filter = (cal_filter_s*)cursor->data;

            if (child_filter->filter_type == CAL_FILTER_COMPOSITE)
            {
                if (__cal_ipc_marshal_composite_filter((cal_composite_filter_s*)child_filter, ipc_data) != CALENDAR_ERROR_NONE)
                {
                    ERR("__cal_ipc_marshal_composite_filter fail");
                    return CALENDAR_ERROR_INVALID_PARAMETER;
                }
            }
            else
            {
                if (__cal_ipc_marshal_attribute_filter((cal_attribute_filter_s*)child_filter, ipc_data) != CALENDAR_ERROR_NONE)
                {
                    ERR("__cal_ipc_marshal_attribute_filter fail");
                    return CALENDAR_ERROR_INVALID_PARAMETER;
                }
            }
            cursor = g_slist_next(cursor);
        }
    }
    else
    {
        if (_cal_ipc_marshal_int(0,ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }

    if (filter->filter_ops)
    {
        int count = g_slist_length(filter->filter_ops);
        GSList *cursor = filter->filter_ops;

        if (_cal_ipc_marshal_int(count,ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
        while (cursor)
        {
            calendar_filter_operator_e op = (calendar_filter_operator_e)cursor->data;

            if (_cal_ipc_marshal_int(op,ipc_data) != CALENDAR_ERROR_NONE)
            {
                ERR("_cal_ipc_marshal fail");
                return CALENDAR_ERROR_INVALID_PARAMETER;
            }

            cursor = g_slist_next(cursor);
        }
    }
    else
    {
        if (_cal_ipc_marshal_int(0,ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }

    // properties //property_count

    return CALENDAR_ERROR_NONE;
}

static int __cal_ipc_unmarshal_attribute_filter(const pims_ipc_data_h ipc_data, const cal_filter_type_e filter_type, cal_attribute_filter_s* filter)
{
    filter->filter_type = filter_type;
    if (_cal_ipc_unmarshal_int(ipc_data,&filter->property_id) != CALENDAR_ERROR_NONE)
    {
        ERR("_cal_ipc_unmarshal fail");
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }
    if (_cal_ipc_unmarshal_int(ipc_data,&filter->match) != CALENDAR_ERROR_NONE)
    {
        ERR("_cal_ipc_unmarshal fail");
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }
    switch(filter->filter_type)
    {
    case CAL_FILTER_STR:
        if (_cal_ipc_unmarshal_char(ipc_data,&filter->value.s) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
        break;
    case CAL_FILTER_INT:
        if (_cal_ipc_unmarshal_int(ipc_data,&filter->value.i) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
        break;
    case CAL_FILTER_DOUBLE:
        if (_cal_ipc_unmarshal_double(ipc_data,&filter->value.d) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
        break;
    case CAL_FILTER_LLI:
        if (_cal_ipc_unmarshal_lli(ipc_data,&filter->value.lli) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
        break;
    case CAL_FILTER_CALTIME:
        if (_cal_ipc_unmarshal_caltime(ipc_data,&filter->value.caltime) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
        break;
    default:
        break;
    }
    return CALENDAR_ERROR_NONE;
}

static int __cal_ipc_marshal_attribute_filter(const cal_attribute_filter_s* filter, pims_ipc_data_h ipc_data)
{
    if (_cal_ipc_marshal_int((filter->filter_type),ipc_data) != CALENDAR_ERROR_NONE)
    {
        ERR("_cal_ipc_marshal fail");
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }
    if (_cal_ipc_marshal_int((filter->property_id),ipc_data) != CALENDAR_ERROR_NONE)
    {
        ERR("_cal_ipc_marshal fail");
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }
    if (_cal_ipc_marshal_int((filter->match),ipc_data) != CALENDAR_ERROR_NONE)
    {
        ERR("_cal_ipc_marshal fail");
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }
    switch(filter->filter_type)
    {
    case CAL_FILTER_STR:
        if (_cal_ipc_marshal_char((filter->value.s),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
        break;
    case CAL_FILTER_INT:
        if (_cal_ipc_marshal_int((filter->value.i),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
        break;
    case CAL_FILTER_DOUBLE:
        if (_cal_ipc_marshal_double((filter->value.d),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
        break;
    case CAL_FILTER_LLI:
        if (_cal_ipc_marshal_lli((filter->value.lli),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
        break;
    case CAL_FILTER_CALTIME:
        if (_cal_ipc_marshal_caltime((filter->value.caltime),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
        break;
    default:
        break;
    }

    return CALENDAR_ERROR_NONE;
}

int _cal_ipc_unmarshal_record(const pims_ipc_data_h ipc_data, calendar_record_h* precord)
{
    int ret = CALENDAR_ERROR_NONE;
    cal_record_s common = {0,};
    cal_record_s *pcommon = NULL;

    if (_cal_ipc_unmarshal_record_common(ipc_data,&common) != CALENDAR_ERROR_NONE)
    {
        ERR("_cal_ipc_unmarshal fail");
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    retvm_if(NULL == precord || NULL == ipc_data, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");

    cal_ipc_marshal_record_plugin_cb_s *plugin_cb = __cal_ipc_marshal_get_plugin_cb(common.type);

    retvm_if(NULL == plugin_cb || NULL == plugin_cb->unmarshal_record, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");

    ret = calendar_record_create(common.view_uri, precord);
    retvm_if(ret != CALENDAR_ERROR_NONE, ret, "record create fail");

    pcommon = (cal_record_s*)(*precord);
    pcommon->properties_max_count = common.properties_max_count;
    pcommon->properties_flags = common.properties_flags;

    ret = plugin_cb->unmarshal_record(ipc_data, *precord);

    if (ret != CALENDAR_ERROR_NONE)
    {
        calendar_record_destroy(*precord,true);
        *precord = NULL;
        ERR("_cal_ipc_unmarshal fail");
    }

    return ret;
}

int _cal_ipc_marshal_record(const calendar_record_h record, pims_ipc_data_h ipc_data)
{
    int ret = CALENDAR_ERROR_NONE;

    cal_record_s *temp = (cal_record_s*)(record);

    retvm_if(NULL == record || NULL == ipc_data, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");

    cal_ipc_marshal_record_plugin_cb_s *plugin_cb = __cal_ipc_marshal_get_plugin_cb(temp->type);

    retvm_if(NULL == plugin_cb || NULL == plugin_cb->marshal_record, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");

    ret = plugin_cb->marshal_record(record, ipc_data);

    return ret;
}

int _cal_ipc_marshal_record_get_primary_id(const calendar_record_h record,
        unsigned int *property_id, int *id)
{
    int ret = CALENDAR_ERROR_NONE;

    cal_record_s *temp = (cal_record_s*)(record);

    retvm_if(NULL == record || NULL == property_id ||
            NULL == id, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");

    cal_ipc_marshal_record_plugin_cb_s *plugin_cb = __cal_ipc_marshal_get_plugin_cb(temp->type);

    retvm_if(NULL == plugin_cb || NULL == plugin_cb->get_primary_id, CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter");

    ret = plugin_cb->get_primary_id(record, property_id,id);

    return ret;
}

int _cal_ipc_unmarshal_char(const pims_ipc_data_h ipc_data, char** ppbufchar)
{
    int ret = CALENDAR_ERROR_NONE;

    void *tmp = NULL;
    unsigned int size = 0;
    char *str = NULL;

    int length = 0;

    retv_if(ipc_data==NULL,CALENDAR_ERROR_INVALID_PARAMETER);
    retv_if(ppbufchar==NULL,CALENDAR_ERROR_INVALID_PARAMETER);

    tmp = pims_ipc_data_get(ipc_data,&size);
    if ( tmp == NULL){
        ERR("pims_ipc_data_get fail");
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }
    length = *(int*)tmp;

    if(length == -1)
    {
        ret = CALENDAR_ERROR_NONE;
        //CAL_DBG("string is null");
        *ppbufchar = NULL;
        return ret;
    }

    str = (char*)pims_ipc_data_get(ipc_data,&size);
    if (str)
    {
        *ppbufchar = SAFE_STRDUP(str);
    }
    //CAL_DBG("string set %s",*ppbufchar);

    return ret;
}

int _cal_ipc_unmarshal_int(const pims_ipc_data_h data, int *pout)
{
    void *tmp = NULL;
    unsigned int size = 0;

    retv_if(data==NULL,CALENDAR_ERROR_INVALID_PARAMETER);
    retv_if(pout==NULL,CALENDAR_ERROR_INVALID_PARAMETER);

    tmp = pims_ipc_data_get(data,&size);
    if ( tmp == NULL)
    {
        ERR("pims_ipc_data_get fail");
        return CALENDAR_ERROR_NO_DATA;
    }
    else
    {
        *pout = *(int*)tmp;
    }
    return CALENDAR_ERROR_NONE;
}

int _cal_ipc_unmarshal_uint(const pims_ipc_data_h data, unsigned int *pout)
{
    void *tmp = NULL;
    unsigned int size = 0;

    retv_if(data==NULL,CALENDAR_ERROR_INVALID_PARAMETER);
    retv_if(pout==NULL,CALENDAR_ERROR_INVALID_PARAMETER);

    tmp = pims_ipc_data_get(data,&size);
    if ( tmp == NULL)
    {
        ERR("pims_ipc_data_get fail");
        return CALENDAR_ERROR_NO_DATA;
    }
    else
    {
        *pout = *(unsigned int*)tmp;
    }
    return CALENDAR_ERROR_NONE;
}

int _cal_ipc_unmarshal_lli(const pims_ipc_data_h data, long long int *pout)
{
    void *tmp = NULL;
    unsigned int size = 0;

    retv_if(data==NULL,CALENDAR_ERROR_INVALID_PARAMETER);
    retv_if(pout==NULL,CALENDAR_ERROR_INVALID_PARAMETER);
    tmp = pims_ipc_data_get(data,&size);
    if ( tmp == NULL)
    {
        ERR("pims_ipc_data_get fail");
        return CALENDAR_ERROR_NO_DATA;
    }
    else
    {
        *pout = *(long long int*)tmp;
    }
    return CALENDAR_ERROR_NONE;
}

int _cal_ipc_unmarshal_long(const pims_ipc_data_h data, long *pout)
{
    void *tmp = NULL;
    unsigned int size = 0;

    retv_if(data==NULL,CALENDAR_ERROR_INVALID_PARAMETER);
    retv_if(pout==NULL,CALENDAR_ERROR_INVALID_PARAMETER);
    tmp = pims_ipc_data_get(data,&size);
    if ( tmp == NULL)
    {
        ERR("pims_ipc_data_get fail");
        return CALENDAR_ERROR_NO_DATA;
    }
    else
    {
        *pout = *(long*)tmp;
    }
    return CALENDAR_ERROR_NONE;
}

int _cal_ipc_unmarshal_double(const pims_ipc_data_h data, double *pout)
{
    void *tmp = NULL;
    unsigned int size = 0;

    retv_if(data==NULL,CALENDAR_ERROR_INVALID_PARAMETER);
    retv_if(pout==NULL,CALENDAR_ERROR_INVALID_PARAMETER);
    tmp = pims_ipc_data_get(data,&size);
    if ( tmp == NULL)
    {
        ERR("pims_ipc_data_get fail");
        return CALENDAR_ERROR_NO_DATA;
    }
    else
    {
        *pout = *(double*)tmp;
    }
    return CALENDAR_ERROR_NONE;
}

int _cal_ipc_unmarshal_caltime(const pims_ipc_data_h data, calendar_time_s *pout)
{
    void *tmp = NULL;
    unsigned int size = 0;
    int ret = CALENDAR_ERROR_NONE;

    retv_if(data==NULL,CALENDAR_ERROR_INVALID_PARAMETER);
    retv_if(pout==NULL,CALENDAR_ERROR_INVALID_PARAMETER);
    tmp = pims_ipc_data_get(data,&size);
    if ( tmp == NULL)
    {
        ERR("pims_ipc_data_get fail");
        return CALENDAR_ERROR_NO_DATA;
    }
    else
    {
        pout->type = *(int*)tmp;
    }

    if (pout->type == CALENDAR_TIME_UTIME)
    {
        return _cal_ipc_unmarshal_lli(data, &(pout->time.utime));
    }
    else
    {
        ret = _cal_ipc_unmarshal_int(data, &(pout->time.date.year));
        retv_if(ret!=CALENDAR_ERROR_NONE,ret);
        ret = _cal_ipc_unmarshal_int(data, &(pout->time.date.month));
        retv_if(ret!=CALENDAR_ERROR_NONE,ret);
        ret = _cal_ipc_unmarshal_int(data, &(pout->time.date.mday));
        retv_if(ret!=CALENDAR_ERROR_NONE,ret);
    }

    return CALENDAR_ERROR_NONE;
}

int _cal_ipc_unmarshal_record_common(const pims_ipc_data_h ipc_data, cal_record_s* common)
{
    void *tmp = NULL;
    unsigned int size = 0;
    const char* str = NULL;

    retv_if(ipc_data==NULL,CALENDAR_ERROR_NO_DATA);
    tmp = pims_ipc_data_get(ipc_data,&size);
    if ( tmp == NULL)
    {
        ERR("pims_ipc_data_get fail");
        return CALENDAR_ERROR_NO_DATA;
    }
    else
    {
        common->type = *(cal_record_type_e*)tmp;
    }

    common->plugin_cb = _cal_record_get_plugin_cb(common->type);

    str = (char*)pims_ipc_data_get(ipc_data,&size);
    common->view_uri = _cal_view_get_uri(str);

    tmp = pims_ipc_data_get(ipc_data,&size);

    common->properties_max_count = *(unsigned int*)tmp;
    if (common->properties_max_count > 0)
    {
        unsigned char *tmp_properties_flags;
        tmp_properties_flags = (unsigned char*)pims_ipc_data_get(ipc_data,&size);
        common->properties_flags  = calloc(common->properties_max_count, sizeof(char));
        if (common->properties_flags == NULL)
        {
            ERR("calloc fail");
            return CALENDAR_ERROR_OUT_OF_MEMORY;
        }
        memcpy(common->properties_flags,tmp_properties_flags,sizeof(char)*common->properties_max_count);
    }

    tmp = pims_ipc_data_get(ipc_data,&size);
    common->property_flag = *(unsigned char*)tmp;

    return CALENDAR_ERROR_NONE;
}

int _cal_ipc_marshal_char(const char* bufchar, pims_ipc_data_h ipc_data)
{
    int ret = CALENDAR_ERROR_NONE;

    retv_if(ipc_data==NULL,CALENDAR_ERROR_INVALID_PARAMETER);

    if( bufchar != NULL)
    {
        int length = strlen(bufchar);
        if (pims_ipc_data_put(ipc_data,(void*)&length,sizeof(int)) != 0)
        {
            ret = CALENDAR_ERROR_OUT_OF_MEMORY;
        }

        if ( pims_ipc_data_put(ipc_data,(void*)bufchar,length+1) != 0)
        {
            ret = CALENDAR_ERROR_OUT_OF_MEMORY;
            return ret;
        }
    }
    else
    {
        int length = -1;

        if (pims_ipc_data_put(ipc_data,(void*)&length,sizeof(int)) != 0)
        {
            ret = CALENDAR_ERROR_OUT_OF_MEMORY;
        }
    }
    return ret;
}

int _cal_ipc_marshal_int(const int in, pims_ipc_data_h ipc_data)
{
    retv_if(ipc_data==NULL,CALENDAR_ERROR_INVALID_PARAMETER);

    if (pims_ipc_data_put(ipc_data,(void*)&in,sizeof(int)) != 0)
    {
        return CALENDAR_ERROR_OUT_OF_MEMORY;
    }
    return CALENDAR_ERROR_NONE;
}

int _cal_ipc_marshal_uint(const unsigned int in, pims_ipc_data_h ipc_data)
{
    retv_if(ipc_data==NULL,CALENDAR_ERROR_INVALID_PARAMETER);

    if (pims_ipc_data_put(ipc_data,(void*)&in,sizeof(unsigned int)) != 0)
    {
        return CALENDAR_ERROR_OUT_OF_MEMORY;
    }
    return CALENDAR_ERROR_NONE;
}

int _cal_ipc_marshal_lli(const long long int in, pims_ipc_data_h ipc_data)
{
    retv_if(ipc_data==NULL,CALENDAR_ERROR_INVALID_PARAMETER);

    if (pims_ipc_data_put(ipc_data,(void*)&in,sizeof(long long int)) != 0)
    {
        return CALENDAR_ERROR_OUT_OF_MEMORY;
    }
    return CALENDAR_ERROR_NONE;
}

int _cal_ipc_marshal_long(const long in, pims_ipc_data_h ipc_data)
{
    retv_if(ipc_data==NULL,CALENDAR_ERROR_INVALID_PARAMETER);

    if (pims_ipc_data_put(ipc_data,(void*)&in,sizeof(long)) != 0)
    {
        return CALENDAR_ERROR_OUT_OF_MEMORY;
    }
    return CALENDAR_ERROR_NONE;
}

int _cal_ipc_marshal_double(const double in, pims_ipc_data_h ipc_data)
{
    retv_if(ipc_data==NULL,CALENDAR_ERROR_INVALID_PARAMETER);

    if (pims_ipc_data_put(ipc_data,(void*)&in,sizeof(double)) != 0)
    {
        return CALENDAR_ERROR_OUT_OF_MEMORY;
    }
    return CALENDAR_ERROR_NONE;
}

int _cal_ipc_marshal_caltime(const calendar_time_s in, pims_ipc_data_h ipc_data)
{
    int ret = CALENDAR_ERROR_NONE;
    retv_if(ipc_data==NULL,CALENDAR_ERROR_INVALID_PARAMETER);

    if (pims_ipc_data_put(ipc_data,(void*)&(in.type),sizeof(int)) != 0)
    {
        return CALENDAR_ERROR_OUT_OF_MEMORY;
    }
    if ( in.type == CALENDAR_TIME_UTIME)
    {
        return _cal_ipc_marshal_lli(in.time.utime,ipc_data);
    }
    else
    {
        ret = _cal_ipc_marshal_int(in.time.date.year,ipc_data);
        retv_if(ret!=CALENDAR_ERROR_NONE,ret);
        ret = _cal_ipc_marshal_int(in.time.date.month,ipc_data);
        retv_if(ret!=CALENDAR_ERROR_NONE,ret);
        ret = _cal_ipc_marshal_int(in.time.date.mday,ipc_data);
        retv_if(ret!=CALENDAR_ERROR_NONE,ret);
    }

    return CALENDAR_ERROR_NONE;
}

int _cal_ipc_marshal_record_common(const cal_record_s* common, pims_ipc_data_h ipc_data)
{

    retv_if(NULL == common, CALENDAR_ERROR_INVALID_PARAMETER);
    retv_if(NULL == ipc_data, CALENDAR_ERROR_INVALID_PARAMETER);

    if(pims_ipc_data_put(ipc_data,(void*)&common->type,sizeof(int)) < 0)
    {
        return CALENDAR_ERROR_NO_DATA;
    }

    int length = strlen(common->view_uri);
    if (pims_ipc_data_put(ipc_data,(void*)common->view_uri,length+1) < 0)
    {
        ERR("_cal_ipc_marshal fail");
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }
    if(pims_ipc_data_put(ipc_data,(void*)&common->properties_max_count,sizeof(int)) < 0)
    {
        ERR("_cal_ipc_marshal fail");
        return CALENDAR_ERROR_NO_DATA;
    }
    if (common->properties_max_count > 0)
    {
        if(pims_ipc_data_put(ipc_data,(void*)common->properties_flags,sizeof(char)*common->properties_max_count) < 0)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_NO_DATA;
        }
    }
    if(pims_ipc_data_put(ipc_data,(void*)&common->property_flag,sizeof(char)) < 0)
    {
        ERR("_cal_ipc_marshal fail");
        return CALENDAR_ERROR_NO_DATA;
    }

    return CALENDAR_ERROR_NONE;
}
/*
int _cal_ipc_unmarshal_filter(const pims_ipc_data_h ipc_data, calendar_filter_h *filter)
{
    cal_composite_filter_s *com_filter = NULL;

    retv_if(NULL == filter, CALENDAR_ERROR_INVALID_PARAMETER);
    retv_if(NULL == ipc_data, CALENDAR_ERROR_INVALID_PARAMETER);

    // !! create filter...
    //calendar_query_create(filter);

    com_filter = (cal_composite_filter_s*)filter;

    return __cal_ipc_unmarshal_composite_filter(ipc_data,com_filter);
}

int _cal_ipc_marshal_filter(const calendar_filter_h filter, pims_ipc_data_h ipc_data)
{
    cal_composite_filter_s *com_filter = NULL;

    retv_if(NULL == filter, CALENDAR_ERROR_INVALID_PARAMETER);
    retv_if(NULL == ipc_data, CALENDAR_ERROR_INVALID_PARAMETER);

    com_filter = (cal_composite_filter_s*)filter;

    return __cal_ipc_marshal_composite_filter(com_filter,ipc_data);
}
*/
int _cal_ipc_unmarshal_query(const pims_ipc_data_h ipc_data, calendar_query_h *query)
{
    cal_query_s *que = NULL;
    unsigned int size = 0;
    char* str = NULL;
    int count = 0, i = 0;
    int ret = CALENDAR_ERROR_NONE;

    retv_if(NULL == query, CALENDAR_ERROR_INVALID_PARAMETER);
    retv_if(NULL == ipc_data, CALENDAR_ERROR_INVALID_PARAMETER);

    // view_uri
    str = (char*)pims_ipc_data_get(ipc_data,&size);

    ret = calendar_query_create(str, query);
    if (ret != CALENDAR_ERROR_NONE)
    {
        ERR("calendar_query_create fail");
        return ret;
    }

    que = (cal_query_s *) *query;

    if (_cal_ipc_unmarshal_int(ipc_data,&count) != CALENDAR_ERROR_NONE)
    {
        ERR("_cal_ipc_unmarshal fail");
        ret = CALENDAR_ERROR_INVALID_PARAMETER;
        goto ERROR_RETURN;
    }

    if (count == 0)
    {
        que->filter = NULL;
    }
    else
    {
        calendar_filter_h filter = (calendar_filter_h)que->filter;
        if (calendar_filter_create(que->view_uri,&filter) != CALENDAR_ERROR_NONE)
        {
            ERR("calendar_filter_create fail");
            ret = CALENDAR_ERROR_OUT_OF_MEMORY;
            goto ERROR_RETURN;
        }
        que->filter = (cal_composite_filter_s*)filter;

        // for filter_type
        if (_cal_ipc_unmarshal_int(ipc_data,&count) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            ret = CALENDAR_ERROR_INVALID_PARAMETER;
            goto ERROR_RETURN;
        }

        if (__cal_ipc_unmarshal_composite_filter(ipc_data,que->filter) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_unmarshal fail");
            ret = CALENDAR_ERROR_INVALID_PARAMETER;
            goto ERROR_RETURN;
        }
    }

    if (_cal_ipc_unmarshal_int(ipc_data,&(que->projection_count)) != CALENDAR_ERROR_NONE)
    {
        ERR("_cal_ipc_unmarshal fail");
        ret = CALENDAR_ERROR_INVALID_PARAMETER;
        goto ERROR_RETURN;
    }

    if (que->projection_count > 0)
    {
        que->projection = (unsigned int*)malloc(sizeof(int)*que->projection_count);
        if (que->projection == NULL)
        {
            ERR("malloc fail");
            ret = CALENDAR_ERROR_OUT_OF_MEMORY;
            goto ERROR_RETURN;
        }
        for(i=0;i<que->projection_count;i++)
        {
            if (_cal_ipc_unmarshal_uint(ipc_data,&(que->projection[i])) != CALENDAR_ERROR_NONE)
            {
                ERR("_cal_ipc_unmarshal fail");
                ret = CALENDAR_ERROR_INVALID_PARAMETER;
                goto ERROR_RETURN;
            }
        }
    }
    else
    {
        que->projection = NULL;
    }

    if (_cal_ipc_unmarshal_int(ipc_data,&(que->sort_property_id)) != CALENDAR_ERROR_NONE)
    {
        ERR("_cal_ipc_unmarshal fail");
        ret = CALENDAR_ERROR_INVALID_PARAMETER;
        goto ERROR_RETURN;
    }

    if (_cal_ipc_unmarshal_int(ipc_data,(int*)&(que->asc)) != CALENDAR_ERROR_NONE)
    {
        ERR("_cal_ipc_unmarshal fail");
        ret = CALENDAR_ERROR_INVALID_PARAMETER;
        goto ERROR_RETURN;
    }

    que->properties = (cal_property_info_s *)_cal_view_get_property_info(que->view_uri, &que->property_count);

    if (_cal_ipc_unmarshal_int(ipc_data,(int*)&(que->distinct)) != CALENDAR_ERROR_NONE)
    {
        ERR("_cal_ipc_unmarshal fail");
        ret = CALENDAR_ERROR_INVALID_PARAMETER;
        goto ERROR_RETURN;
    }

    return CALENDAR_ERROR_NONE;

ERROR_RETURN:

    calendar_query_destroy(*query);
    *query = NULL;

    return ret;
}

int _cal_ipc_marshal_query(const calendar_query_h query, pims_ipc_data_h ipc_data)
{
    cal_query_s *que = NULL;
    int i = 0;
    int length = 0;

    retv_if(NULL == query, CALENDAR_ERROR_INVALID_PARAMETER);
    retv_if(NULL == ipc_data, CALENDAR_ERROR_INVALID_PARAMETER);
    que = (cal_query_s *)query;

    //view_uri
    length = strlen(que->view_uri);
    if (pims_ipc_data_put(ipc_data,(void*)que->view_uri,length+1) < 0)
    {
        ERR("_cal_ipc_marshal fail");
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    if (que->filter)
    {
        if (_cal_ipc_marshal_int(1,ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
        if (__cal_ipc_marshal_composite_filter(que->filter,ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }
    else
    {
        if (_cal_ipc_marshal_int(0,ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }

    if (_cal_ipc_marshal_int((que->projection_count),ipc_data) != CALENDAR_ERROR_NONE)
    {
        ERR("_cal_ipc_marshal fail");
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }
    for(i=0;i<que->projection_count;i++)
    {
        if (_cal_ipc_marshal_uint((que->projection[i]),ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
    }

    if (_cal_ipc_marshal_int((que->sort_property_id),ipc_data) != CALENDAR_ERROR_NONE)
    {
        ERR("_cal_ipc_marshal fail");
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    if (_cal_ipc_marshal_int((int)(que->asc),ipc_data) != CALENDAR_ERROR_NONE)
    {
        ERR("_cal_ipc_marshal fail");
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }
    if (_cal_ipc_marshal_int((int)(que->distinct),ipc_data) != CALENDAR_ERROR_NONE)
    {
        ERR("_cal_ipc_marshal fail");
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    //properties // property_count

    return CALENDAR_ERROR_NONE;
}

int _cal_ipc_unmarshal_list(const pims_ipc_data_h ipc_data, calendar_list_h* list)
{
    int count = 0, i = 0;
    calendar_record_h record;
    int ret = CALENDAR_ERROR_NONE;

    retv_if(NULL == list, CALENDAR_ERROR_INVALID_PARAMETER);
    retv_if(NULL == ipc_data, CALENDAR_ERROR_INVALID_PARAMETER);

    if (calendar_list_create(list) != CALENDAR_ERROR_NONE)
    {
        ERR("calendar_list_create fail");
        return CALENDAR_ERROR_OUT_OF_MEMORY;
    }

    if (_cal_ipc_unmarshal_int(ipc_data,&(count)) != CALENDAR_ERROR_NONE)
    {
        ERR("_cal_ipc_unmarshal fail");
        ret = CALENDAR_ERROR_INVALID_PARAMETER;
        goto ERROR_RETURN;
    }

    for(i=0;i<count;i++)
    {
        if (_cal_ipc_unmarshal_record(ipc_data,&record) != CALENDAR_ERROR_NONE )
        {
            ERR("_cal_ipc_unmarshal fail");
            ret = CALENDAR_ERROR_INVALID_PARAMETER;
            goto ERROR_RETURN;
        }

        if (calendar_list_add(*list,record) != CALENDAR_ERROR_NONE)
        {
            ERR("calendar_list_add fail");
            ret = CALENDAR_ERROR_INVALID_PARAMETER;
            goto ERROR_RETURN;
        }
    }

    calendar_list_first(*list);

    return CALENDAR_ERROR_NONE;

ERROR_RETURN:
    if (*list)
    {
        calendar_list_destroy(*list, true);
        *list = NULL;
    }

    return ret;
}

int _cal_ipc_marshal_list(const calendar_list_h list, pims_ipc_data_h ipc_data)
{
    int count = 0, i = 0;
    calendar_record_h record;
    retv_if(NULL == list, CALENDAR_ERROR_INVALID_PARAMETER);
    retv_if(NULL == ipc_data, CALENDAR_ERROR_INVALID_PARAMETER);

    // count
    if (calendar_list_get_count(list, &count) != CALENDAR_ERROR_NONE)
    {
        ERR("calendar_list_get_count fail");
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }
    if (_cal_ipc_marshal_int(count,ipc_data) != CALENDAR_ERROR_NONE)
    {
        ERR("_cal_ipc_marshal fail");
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    calendar_list_first(list);

    for(i=0;i<count;i++)
    {
        if (calendar_list_get_current_record_p(list,&record) != CALENDAR_ERROR_NONE)
        {
            ERR("calendar_list_get_count fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
        if (_cal_ipc_marshal_record(record,ipc_data) != CALENDAR_ERROR_NONE)
        {
            ERR("_cal_ipc_marshal fail");
            return CALENDAR_ERROR_INVALID_PARAMETER;
        }
        calendar_list_next(list);
    }

    return CALENDAR_ERROR_NONE;
}
