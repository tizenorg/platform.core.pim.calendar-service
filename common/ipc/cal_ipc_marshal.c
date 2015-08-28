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
#include <string.h>

#include "calendar.h"
#include "cal_ipc_marshal.h"
#include "cal_record.h"
#include "cal_internal.h"
#include "cal_view.h"
#include "cal_handle.h"
#include "cal_utils.h"

extern cal_ipc_marshal_record_plugin_cb_s cal_ipc_record_calendar_plugin_cb;
extern cal_ipc_marshal_record_plugin_cb_s cal_ipc_record_event_plugin_cb;
extern cal_ipc_marshal_record_plugin_cb_s cal_ipc_record_todo_plugin_cb;
extern cal_ipc_marshal_record_plugin_cb_s cal_ipc_record_alarm_plugin_cb;
extern cal_ipc_marshal_record_plugin_cb_s cal_ipc_record_attendee_plugin_cb;
extern cal_ipc_marshal_record_plugin_cb_s cal_ipc_record_timezone_plugin_cb;
extern cal_ipc_marshal_record_plugin_cb_s cal_ipc_record_updated_info_plugin_cb;
extern cal_ipc_marshal_record_plugin_cb_s cal_ipc_record_instance_normal_plugin_cb;
extern cal_ipc_marshal_record_plugin_cb_s cal_ipc_record_instance_normal_extended_plugin_cb;
extern cal_ipc_marshal_record_plugin_cb_s cal_ipc_record_instance_allday_plugin_cb;
extern cal_ipc_marshal_record_plugin_cb_s cal_ipc_record_instance_allday_extended_plugin_cb;
extern cal_ipc_marshal_record_plugin_cb_s cal_ipc_record_search_plugin_cb;
extern cal_ipc_marshal_record_plugin_cb_s cal_ipc_record_extended_plugin_cb;

static cal_ipc_marshal_record_plugin_cb_s* _cal_ipc_marshal_get_plugin_cb(cal_record_type_e type);

static int _cal_ipc_unmarshal_composite_filter(const pims_ipc_data_h ipc_data, cal_composite_filter_s* filter);
static int _cal_ipc_marshal_composite_filter(const cal_composite_filter_s* filter, pims_ipc_data_h ipc_data);
static int _cal_ipc_unmarshal_attribute_filter(const pims_ipc_data_h ipc_data, const cal_filter_type_e filter_type, cal_attribute_filter_s* filter);
static int _cal_ipc_marshal_attribute_filter(const cal_attribute_filter_s* filter, pims_ipc_data_h ipc_data);

static cal_ipc_marshal_record_plugin_cb_s* _cal_ipc_marshal_get_plugin_cb(cal_record_type_e type)
{
	switch (type) {
	case CAL_RECORD_TYPE_CALENDAR:
		return (&cal_ipc_record_calendar_plugin_cb);
	case CAL_RECORD_TYPE_EVENT:
		return (&cal_ipc_record_event_plugin_cb);
	case CAL_RECORD_TYPE_TODO:
		return (&cal_ipc_record_todo_plugin_cb);
	case CAL_RECORD_TYPE_ALARM:
		return (&cal_ipc_record_alarm_plugin_cb);
	case CAL_RECORD_TYPE_ATTENDEE:
		return (&cal_ipc_record_attendee_plugin_cb);
	case CAL_RECORD_TYPE_TIMEZONE:
		return (&cal_ipc_record_timezone_plugin_cb);
	case CAL_RECORD_TYPE_INSTANCE_NORMAL:
		return (&cal_ipc_record_instance_normal_plugin_cb);
	case CAL_RECORD_TYPE_INSTANCE_ALLDAY:
		return (&cal_ipc_record_instance_allday_plugin_cb);
	case CAL_RECORD_TYPE_INSTANCE_NORMAL_EXTENDED:
		return (&cal_ipc_record_instance_normal_extended_plugin_cb);
	case CAL_RECORD_TYPE_INSTANCE_ALLDAY_EXTENDED:
		return (&cal_ipc_record_instance_allday_extended_plugin_cb);
	case CAL_RECORD_TYPE_UPDATED_INFO:
		return (&cal_ipc_record_updated_info_plugin_cb);
	case CAL_RECORD_TYPE_SEARCH:
		return (&cal_ipc_record_search_plugin_cb);
	case CAL_RECORD_TYPE_EXTENDED:
		return (&cal_ipc_record_extended_plugin_cb);
	default:
		return NULL;
	}
}

static void _cal_ipc_unmarshal_composite_filter_free(cal_composite_filter_s* filter)
{
	if (filter->filters) {
		GSList *cursor = NULL;
		for(cursor=filter->filters;cursor;cursor=cursor->next) {
			cal_filter_s *src = (cal_filter_s*)cursor->data;
			if (src->filter_type == CAL_FILTER_COMPOSITE)
				_cal_ipc_unmarshal_composite_filter_free((cal_composite_filter_s *)src);
			else {
				cal_attribute_filter_s *attr = (cal_attribute_filter_s *)src;
				if (attr->filter_type == CAL_FILTER_STR)
					free(attr->value.s);
			}
			free(src);
		}
		g_slist_free(filter->filters);
	}

	if (filter->filter_ops) {
		g_slist_free(filter->filter_ops);
	}

	free(filter->view_uri);
}

static int _cal_ipc_unmarshal_composite_filter(const pims_ipc_data_h ipc_data, cal_composite_filter_s* filter)
{
	int ret = CALENDAR_ERROR_NONE;
	unsigned int size = 0;
	char* str = NULL;
	int count = 0;
	int i = 0;
	cal_filter_type_e filter_type = CAL_FILTER_COMPOSITE;
	calendar_filter_operator_e op = CALENDAR_FILTER_OPERATOR_AND;

	RETV_IF(NULL == filter, CALENDAR_ERROR_INVALID_PARAMETER);

	filter->filter_type = CAL_FILTER_COMPOSITE;

	str = (char*)pims_ipc_data_get(ipc_data,&size);
	CAL_FREE(filter->view_uri);
	filter->view_uri = cal_strdup(str);

	ret = cal_ipc_unmarshal_int(ipc_data, &count);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	for(i=0;i<count;i++) {
		ret = cal_ipc_unmarshal_int(ipc_data, (int*)&filter_type);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
			ret = CALENDAR_ERROR_INVALID_PARAMETER;
			goto ERROR_RETURN;
		}
		if (filter_type == CAL_FILTER_COMPOSITE) {
			cal_composite_filter_s* com_filter = NULL;
			com_filter = (cal_composite_filter_s*)calloc(1, sizeof(cal_composite_filter_s));
			if (NULL == com_filter) {
				ERR("calloc() Fail");
				ret = CALENDAR_ERROR_OUT_OF_MEMORY;
				goto ERROR_RETURN;
			}
			ret = _cal_ipc_unmarshal_composite_filter(ipc_data, com_filter);
			if (CALENDAR_ERROR_NONE != ret) {
				ERR("_cal_ipc_unmarshal_composite_filter() Fail(%d)", ret);
				ret = CALENDAR_ERROR_INVALID_PARAMETER;
				CAL_FREE(com_filter);
				goto ERROR_RETURN;
			}
			filter->filters = g_slist_append(filter->filters,com_filter);
		}
		else {
			cal_attribute_filter_s* attr_filter = NULL;
			attr_filter = (cal_attribute_filter_s*)calloc(1, sizeof(cal_attribute_filter_s));
			if (NULL == attr_filter) {
				ERR("calloc() Fail");
				ret = CALENDAR_ERROR_OUT_OF_MEMORY;
				goto ERROR_RETURN;
			}
			ret = _cal_ipc_unmarshal_attribute_filter(ipc_data, filter_type, attr_filter);
			if (CALENDAR_ERROR_NONE != ret) {
				ERR("_cal_ipc_unmarshal_attribute_filter() Fail(%d)", ret);
				ret =  CALENDAR_ERROR_INVALID_PARAMETER;
				CAL_FREE(attr_filter);
				goto ERROR_RETURN;
			}
			filter->filters = g_slist_append(filter->filters,attr_filter);
		}
	}

	ret = cal_ipc_unmarshal_int(ipc_data, &count);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)");
		ret =  CALENDAR_ERROR_INVALID_PARAMETER;
		goto ERROR_RETURN;
	}

	for(i = 0; i < count; i++) {
		ret = cal_ipc_unmarshal_int(ipc_data, (int*)&op);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
			ret =  CALENDAR_ERROR_INVALID_PARAMETER;
			goto ERROR_RETURN;
		}
		filter->filter_ops = g_slist_append(filter->filter_ops, (void*)op);
	}

	filter->properties = (cal_property_info_s *)cal_view_get_property_info(filter->view_uri, &filter->property_count);

	return CALENDAR_ERROR_NONE;

ERROR_RETURN:

	_cal_ipc_unmarshal_composite_filter_free(filter);

	return ret;
}

static int _cal_ipc_marshal_composite_filter(const cal_composite_filter_s* filter, pims_ipc_data_h ipc_data)
{
	int ret = 0;
	ret = cal_ipc_marshal_int((filter->filter_type), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	int length = strlen(filter->view_uri);
	if (pims_ipc_data_put(ipc_data, (void*)filter->view_uri,length+1) < 0) {
		ERR("pims_ipc_data_put() Fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	if (filter->filters) {
		int count = g_slist_length(filter->filters);
		GSList *cursor = filter->filters;
		cal_filter_s* child_filter;

		ret = cal_ipc_marshal_int(count, ipc_data);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_ipc_marshal_int() Fail(%d)", ret);
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}

		while (cursor) {
			child_filter = (cal_filter_s*)cursor->data;

			if (child_filter->filter_type == CAL_FILTER_COMPOSITE) {
				ret = _cal_ipc_marshal_composite_filter((cal_composite_filter_s*)child_filter, ipc_data);
				if (CALENDAR_ERROR_NONE != ret) {
					ERR("_cal_ipc_marshal_composite_filter() Fail(%d)", ret);
					return CALENDAR_ERROR_INVALID_PARAMETER;
				}
			}
			else {
				ret = _cal_ipc_marshal_attribute_filter((cal_attribute_filter_s*)child_filter, ipc_data);
				if (CALENDAR_ERROR_NONE != ret) {
					ERR("_cal_ipc_marshal_attribute_filter() Fail(%d)", ret);
					return CALENDAR_ERROR_INVALID_PARAMETER;
				}
			}
			cursor = g_slist_next(cursor);
		}
	}
	else {
		ret = cal_ipc_marshal_int(0, ipc_data);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_ipc_marshal_int() Fail(%d)", ret);
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
	}

	if (filter->filter_ops) {
		int count = g_slist_length(filter->filter_ops);
		GSList *cursor = filter->filter_ops;

		ret = cal_ipc_marshal_int(count, ipc_data);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_ipc_marshal_int() Fail(%d)", ret);
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}

		while (cursor) {
			calendar_filter_operator_e op = (calendar_filter_operator_e)cursor->data;

			ret = cal_ipc_marshal_int(op, ipc_data);
			if (CALENDAR_ERROR_NONE != ret) {
				ERR("cal_ipc_marshal_int() Fail(%d)", ret);
				return CALENDAR_ERROR_INVALID_PARAMETER;
			}

			cursor = g_slist_next(cursor);
		}
	}
	else {
		ret = cal_ipc_marshal_int(0, ipc_data);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_ipc_marshal_int() Fail(%d)", ret);
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_ipc_unmarshal_attribute_filter(const pims_ipc_data_h ipc_data, const cal_filter_type_e filter_type, cal_attribute_filter_s* filter)
{
	int ret = 0;
	filter->filter_type = filter_type;

	ret = cal_ipc_unmarshal_int(ipc_data, &filter->property_id);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	ret = cal_ipc_unmarshal_int(ipc_data,&filter->match);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	switch (filter->filter_type) {
	case CAL_FILTER_STR:
		ret = cal_ipc_unmarshal_char(ipc_data,&filter->value.s);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_ipc_unmarshal_char() Fail(%d)", ret);
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
		break;
	case CAL_FILTER_INT:
		ret = cal_ipc_unmarshal_int(ipc_data,&filter->value.i);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
		break;
	case CAL_FILTER_DOUBLE:
		ret = cal_ipc_unmarshal_double(ipc_data,&filter->value.d);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_ipc_unmarshal_double() Fail(%d)", ret);
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
		break;
	case CAL_FILTER_LLI:
		ret = cal_ipc_unmarshal_lli(ipc_data,&filter->value.lli);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_ipc_unmarshal_lli() Fail(%d)", ret);
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
		break;
	case CAL_FILTER_CALTIME:
		ret = cal_ipc_unmarshal_caltime(ipc_data,&filter->value.caltime);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_ipc_unmarshal_caltime() Fail(%d)", ret);
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
		break;
	default:
		break;
	}
	return CALENDAR_ERROR_NONE;
}

static int _cal_ipc_marshal_attribute_filter(const cal_attribute_filter_s* filter, pims_ipc_data_h ipc_data)
{
	int ret = 0;
	ret = cal_ipc_marshal_int((filter->filter_type), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	ret = cal_ipc_marshal_int((filter->property_id), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	ret = cal_ipc_marshal_int((filter->match), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	switch (filter->filter_type) {
	case CAL_FILTER_STR:
		ret = cal_ipc_marshal_char((filter->value.s), ipc_data);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_ipc_marshal_char() Fail(%d)", ret);
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
		break;
	case CAL_FILTER_INT:
		ret = cal_ipc_marshal_int((filter->value.i), ipc_data);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_ipc_marshal_int() Fail(%d)", ret);
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
		break;
	case CAL_FILTER_DOUBLE:
		ret = cal_ipc_marshal_double((filter->value.d),ipc_data);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_ipc_marshal_double() Fail(%d)", ret);
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
		break;
	case CAL_FILTER_LLI:
		ret = cal_ipc_marshal_lli((filter->value.lli), ipc_data);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_ipc_marshal_lli() Fail(%d)", ret);
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
		break;
	case CAL_FILTER_CALTIME:
		ret = cal_ipc_marshal_caltime((filter->value.caltime), ipc_data);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_ipc_marshal_caltime() Fail(%d)", ret);
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
		break;
	default:
		break;
	}

	return CALENDAR_ERROR_NONE;
}

int cal_ipc_unmarshal_record(const pims_ipc_data_h ipc_data, calendar_record_h* precord)
{
	int ret = CALENDAR_ERROR_NONE;
	cal_record_s common = {0,};
	cal_record_s *pcommon = NULL;

	RETV_IF(NULL == ipc_data, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == precord, CALENDAR_ERROR_INVALID_PARAMETER);

	ret = cal_ipc_unmarshal_record_common(ipc_data, &common);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_record_common() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	cal_ipc_marshal_record_plugin_cb_s *plugin_cb = _cal_ipc_marshal_get_plugin_cb(common.type);

	if (NULL == plugin_cb || NULL == plugin_cb->unmarshal_record) {
		ERR("Invalid parameter");
		free(common.properties_flags);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	ret = calendar_record_create(common.view_uri, precord);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "calendar_record_create() Fail(%d)", ret);

	pcommon = (cal_record_s*)(*precord);
	pcommon->properties_max_count = common.properties_max_count;
	pcommon->properties_flags = common.properties_flags;

	ret = plugin_cb->unmarshal_record(ipc_data, *precord);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("unmarshal_record() Fail(%d)", ret);
		calendar_record_destroy(*precord,true);
		*precord = NULL;
	}

	return ret;
}

int cal_ipc_marshal_record(const calendar_record_h record, pims_ipc_data_h ipc_data)
{
	int ret = CALENDAR_ERROR_NONE;
	cal_record_s *temp = (cal_record_s*)(record);

	RETV_IF(NULL == record, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == ipc_data, CALENDAR_ERROR_INVALID_PARAMETER);

	ret = cal_ipc_marshal_record_common(temp, ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_record_common() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	cal_ipc_marshal_record_plugin_cb_s *plugin_cb = _cal_ipc_marshal_get_plugin_cb(temp->type);
	RETV_IF(NULL == plugin_cb, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == plugin_cb->marshal_record, CALENDAR_ERROR_INVALID_PARAMETER);

	ret = plugin_cb->marshal_record(record, ipc_data);

	return ret;
}

int cal_ipc_unmarshal_char(const pims_ipc_data_h ipc_data, char** ppbufchar)
{
	unsigned int size = 0;
	char *str = NULL;
	int length = 0;

	RETV_IF(NULL == ipc_data, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == ppbufchar, CALENDAR_ERROR_INVALID_PARAMETER);

	void *tmp = NULL;
	tmp = pims_ipc_data_get(ipc_data,&size);
	RETVM_IF(NULL == tmp, CALENDAR_ERROR_INVALID_PARAMETER, "pims_ipc_data_get() Fail");

	length = *(int*)tmp;
	if (length == -1) {
		*ppbufchar = NULL;
		return CALENDAR_ERROR_NONE;
	}
	str = (char*)pims_ipc_data_get(ipc_data,&size);
	if (str) {
		*ppbufchar = cal_strdup(str);
	}
	return CALENDAR_ERROR_NONE;
}

int cal_ipc_unmarshal_int(const pims_ipc_data_h data, int *pout)
{
	void *tmp = NULL;
	unsigned int size = 0;

	RETV_IF(NULL == data, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == pout, CALENDAR_ERROR_INVALID_PARAMETER);

	tmp = pims_ipc_data_get(data,&size);
	if (NULL == tmp) {
		ERR("pims_ipc_data_get() Fail");
		return CALENDAR_ERROR_NO_DATA;
	}
	else {
		*pout = *(int*)tmp;
	}
	return CALENDAR_ERROR_NONE;
}

int cal_ipc_unmarshal_uint(const pims_ipc_data_h data, unsigned int *pout)
{
	void *tmp = NULL;
	unsigned int size = 0;

	RETV_IF(NULL == data, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == pout, CALENDAR_ERROR_INVALID_PARAMETER);

	tmp = pims_ipc_data_get(data,&size);
	if (NULL == tmp) {
		ERR("pims_ipc_data_get() Fail");
		return CALENDAR_ERROR_NO_DATA;
	}
	else {
		*pout = *(unsigned int*)tmp;
	}
	return CALENDAR_ERROR_NONE;
}

int cal_ipc_unmarshal_lli(const pims_ipc_data_h data, long long int *pout)
{
	void *tmp = NULL;
	unsigned int size = 0;

	RETV_IF(NULL == data, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == pout, CALENDAR_ERROR_INVALID_PARAMETER);
	tmp = pims_ipc_data_get(data,&size);
	if (NULL == tmp) {
		ERR("pims_ipc_data_get() Fail");
		return CALENDAR_ERROR_NO_DATA;
	}
	else {
		*pout = *(long long int*)tmp;
	}
	return CALENDAR_ERROR_NONE;
}

int cal_ipc_unmarshal_long(const pims_ipc_data_h data, long *pout)
{
	void *tmp = NULL;
	unsigned int size = 0;

	RETV_IF(NULL == data, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == pout, CALENDAR_ERROR_INVALID_PARAMETER);
	tmp = pims_ipc_data_get(data,&size);
	if (NULL == tmp) {
		ERR("pims_ipc_data_get() Fail");
		return CALENDAR_ERROR_NO_DATA;
	}
	else {
		*pout = *(long*)tmp;
	}
	return CALENDAR_ERROR_NONE;
}

int cal_ipc_unmarshal_double(const pims_ipc_data_h data, double *pout)
{
	void *tmp = NULL;
	unsigned int size = 0;

	RETV_IF(NULL == data, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == pout, CALENDAR_ERROR_INVALID_PARAMETER);
	tmp = pims_ipc_data_get(data,&size);
	if (NULL == tmp) {
		ERR("pims_ipc_data_get() Fail");
		return CALENDAR_ERROR_NO_DATA;
	}
	else {
		*pout = *(double*)tmp;
	}
	return CALENDAR_ERROR_NONE;
}

int cal_ipc_unmarshal_caltime(const pims_ipc_data_h data, calendar_time_s *pout)
{
	void *tmp = NULL;
	unsigned int size = 0;
	int ret = CALENDAR_ERROR_NONE;

	RETV_IF(NULL == data, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == pout, CALENDAR_ERROR_INVALID_PARAMETER);
	tmp = pims_ipc_data_get(data,&size);
	if (NULL == tmp) {
		ERR("pims_ipc_data_get() Fail");
		return CALENDAR_ERROR_NO_DATA;
	}
	else {
		pout->type = *(int*)tmp;
	}

	if (pout->type == CALENDAR_TIME_UTIME) {
		return cal_ipc_unmarshal_lli(data, &(pout->time.utime));
	}
	else {
		ret = cal_ipc_unmarshal_int(data, &(pout->time.date.year));
		RETV_IF(ret!=CALENDAR_ERROR_NONE,ret);
		ret = cal_ipc_unmarshal_int(data, &(pout->time.date.month));
		RETV_IF(ret!=CALENDAR_ERROR_NONE,ret);
		ret = cal_ipc_unmarshal_int(data, &(pout->time.date.mday));
		RETV_IF(ret!=CALENDAR_ERROR_NONE,ret);
		ret = cal_ipc_unmarshal_int(data, &(pout->time.date.hour));
		RETV_IF(ret!=CALENDAR_ERROR_NONE,ret);
		ret = cal_ipc_unmarshal_int(data, &(pout->time.date.minute));
		RETV_IF(ret!=CALENDAR_ERROR_NONE,ret);
		ret = cal_ipc_unmarshal_int(data, &(pout->time.date.second));
		RETV_IF(ret!=CALENDAR_ERROR_NONE,ret);
	}

	return CALENDAR_ERROR_NONE;
}

int cal_ipc_unmarshal_record_common(const pims_ipc_data_h ipc_data, cal_record_s* common)
{
	unsigned int size = 0;
	void *ret_pims = NULL;

	RETV_IF(NULL == ipc_data, CALENDAR_ERROR_NO_DATA);
	ret_pims = pims_ipc_data_get(ipc_data,&size);
	if (NULL == ret_pims) {
		ERR("pims_ipc_data_get() Fail");
		return CALENDAR_ERROR_NO_DATA;
	}
	common->type = *(cal_record_type_e*)ret_pims;
	common->plugin_cb = cal_record_get_plugin_cb(common->type);

	ret_pims = pims_ipc_data_get(ipc_data,&size);
	if (NULL == ret_pims) {
		ERR("pims_ipc_data_get() Fail");
		return CALENDAR_ERROR_NO_DATA;
	}
	char *uri = (char*)ret_pims;
	common->view_uri = cal_view_get_uri(uri);

	ret_pims = pims_ipc_data_get(ipc_data,&size);
	if (NULL == ret_pims) {
		ERR("pims_ipc_data_get() Fail");
		return CALENDAR_ERROR_NO_DATA;
	}
	common->properties_max_count = *(unsigned int*)ret_pims;
	if (0 < common->properties_max_count) {
		unsigned char *tmp_properties_flags;
		ret_pims = pims_ipc_data_get(ipc_data,&size);
		if (NULL == ret_pims) {
			ERR("pims_ipc_data_get() Fail");
			return CALENDAR_ERROR_NO_DATA;
		}
		tmp_properties_flags = (unsigned char*)ret_pims;
		common->properties_flags = calloc(common->properties_max_count, sizeof(char));
		if (NULL == common->properties_flags) {
			ERR("calloc() Fail");
			return CALENDAR_ERROR_OUT_OF_MEMORY;
		}
		memcpy(common->properties_flags,tmp_properties_flags, sizeof(char)*common->properties_max_count);
	}

	ret_pims = pims_ipc_data_get(ipc_data,&size);
	if (NULL == ret_pims) {
		ERR("pims_ipc_data_get() Fail");
		free(common->properties_flags);
		return CALENDAR_ERROR_IPC;
	}
	common->property_flag = *(unsigned char*)ret_pims;
	return CALENDAR_ERROR_NONE;
}

int cal_ipc_marshal_char(const char* bufchar, pims_ipc_data_h ipc_data)
{
	int ret = CALENDAR_ERROR_NONE;

	RETV_IF(NULL == ipc_data, CALENDAR_ERROR_INVALID_PARAMETER);

	if (bufchar) {
		int length = strlen(bufchar);
		if (pims_ipc_data_put(ipc_data, (void*)&length, sizeof(int)) != 0) {
			ret = CALENDAR_ERROR_OUT_OF_MEMORY;
		}

		if (pims_ipc_data_put(ipc_data, (void*)bufchar,length+1) != 0) {
			ret = CALENDAR_ERROR_OUT_OF_MEMORY;
			return ret;
		}
	}
	else {
		int length = -1;

		if (pims_ipc_data_put(ipc_data, (void*)&length, sizeof(int)) != 0) {
			ret = CALENDAR_ERROR_OUT_OF_MEMORY;
		}
	}
	return ret;
}

int cal_ipc_marshal_int(const int in, pims_ipc_data_h ipc_data)
{
	RETV_IF(NULL == ipc_data, CALENDAR_ERROR_INVALID_PARAMETER);

	if (pims_ipc_data_put(ipc_data, (void*)&in, sizeof(int)) != 0) {
		return CALENDAR_ERROR_OUT_OF_MEMORY;
	}
	return CALENDAR_ERROR_NONE;
}

int cal_ipc_marshal_uint(const unsigned int in, pims_ipc_data_h ipc_data)
{
	RETV_IF(NULL == ipc_data, CALENDAR_ERROR_INVALID_PARAMETER);

	if (pims_ipc_data_put(ipc_data, (void*)&in, sizeof(unsigned int)) != 0) {
		return CALENDAR_ERROR_OUT_OF_MEMORY;
	}
	return CALENDAR_ERROR_NONE;
}

int cal_ipc_marshal_lli(const long long int in, pims_ipc_data_h ipc_data)
{
	RETV_IF(NULL == ipc_data, CALENDAR_ERROR_INVALID_PARAMETER);

	if (pims_ipc_data_put(ipc_data, (void*)&in, sizeof(long long int)) != 0) {
		return CALENDAR_ERROR_OUT_OF_MEMORY;
	}
	return CALENDAR_ERROR_NONE;
}

int cal_ipc_marshal_long(const long in, pims_ipc_data_h ipc_data)
{
	RETV_IF(NULL == ipc_data, CALENDAR_ERROR_INVALID_PARAMETER);

	if (pims_ipc_data_put(ipc_data, (void*)&in, sizeof(long)) != 0) {
		return CALENDAR_ERROR_OUT_OF_MEMORY;
	}
	return CALENDAR_ERROR_NONE;
}

int cal_ipc_marshal_double(const double in, pims_ipc_data_h ipc_data)
{
	RETV_IF(NULL == ipc_data, CALENDAR_ERROR_INVALID_PARAMETER);

	if (pims_ipc_data_put(ipc_data, (void*)&in, sizeof(double)) != 0) {
		return CALENDAR_ERROR_OUT_OF_MEMORY;
	}
	return CALENDAR_ERROR_NONE;
}

int cal_ipc_marshal_caltime(const calendar_time_s in, pims_ipc_data_h ipc_data)
{
	int ret = CALENDAR_ERROR_NONE;
	RETV_IF(NULL == ipc_data, CALENDAR_ERROR_INVALID_PARAMETER);

	if (pims_ipc_data_put(ipc_data, (void*)&(in.type), sizeof(int)) != 0) {
		return CALENDAR_ERROR_OUT_OF_MEMORY;
	}

	if (in.type == CALENDAR_TIME_UTIME) {
		return cal_ipc_marshal_lli(in.time.utime,ipc_data);
	}
	else {
		ret = cal_ipc_marshal_int(in.time.date.year,ipc_data);
		RETV_IF(ret!=CALENDAR_ERROR_NONE,ret);
		ret = cal_ipc_marshal_int(in.time.date.month,ipc_data);
		RETV_IF(ret!=CALENDAR_ERROR_NONE,ret);
		ret = cal_ipc_marshal_int(in.time.date.mday,ipc_data);
		RETV_IF(ret!=CALENDAR_ERROR_NONE,ret);
		ret = cal_ipc_marshal_int(in.time.date.hour,ipc_data);
		RETV_IF(ret!=CALENDAR_ERROR_NONE,ret);
		ret = cal_ipc_marshal_int(in.time.date.minute,ipc_data);
		RETV_IF(ret!=CALENDAR_ERROR_NONE,ret);
		ret = cal_ipc_marshal_int(in.time.date.second,ipc_data);
		RETV_IF(ret!=CALENDAR_ERROR_NONE,ret);
	}

	return CALENDAR_ERROR_NONE;
}

int cal_ipc_marshal_record_common(const cal_record_s* common, pims_ipc_data_h ipc_data)
{

	RETV_IF(NULL == common, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == ipc_data, CALENDAR_ERROR_INVALID_PARAMETER);

	if (pims_ipc_data_put(ipc_data, (void*)&common->type, sizeof(int)) < 0)
		return CALENDAR_ERROR_NO_DATA;

	int length = strlen(common->view_uri);

	if (pims_ipc_data_put(ipc_data, (void*)common->view_uri,length+1) < 0) {
		ERR("pims_ipc_data_put() Fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	if (pims_ipc_data_put(ipc_data, (void*)&common->properties_max_count, sizeof(int)) < 0) {
		ERR("pims_ipc_data_put() Fail");
		return CALENDAR_ERROR_NO_DATA;
	}

	if (0 < common->properties_max_count) {
		if (pims_ipc_data_put(ipc_data, (void*)common->properties_flags, sizeof(char)*common->properties_max_count) < 0) {
			ERR("pims_ipc_data_put() Fail");
			return CALENDAR_ERROR_NO_DATA;
		}
	}

	if (pims_ipc_data_put(ipc_data, (void*)&common->property_flag, sizeof(char)) < 0) {
		ERR("pims_ipc_data_put() Fail");
		return CALENDAR_ERROR_NO_DATA;
	}

	return CALENDAR_ERROR_NONE;
}

int cal_ipc_unmarshal_query(const pims_ipc_data_h ipc_data, calendar_query_h *query)
{
	cal_query_s *que = NULL;
	unsigned int size = 0;
	char* str = NULL;
	int count = 0, i = 0;
	int ret = CALENDAR_ERROR_NONE;

	RETV_IF(NULL == query, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == ipc_data, CALENDAR_ERROR_INVALID_PARAMETER);

	str = (char*)pims_ipc_data_get(ipc_data,&size);

	ret = calendar_query_create(str, query);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("calendar_query_create() Fail(%d)", ret);
		return ret;
	}

	que = (cal_query_s *) *query;

	ret = cal_ipc_unmarshal_int(ipc_data, &count);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		ret = CALENDAR_ERROR_INVALID_PARAMETER;
		goto ERROR_RETURN;
	}

	if (count == 0) {
		que->filter = NULL;
	}
	else {
		calendar_filter_h filter = (calendar_filter_h)que->filter;
		ret = calendar_filter_create(que->view_uri, &filter);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("calendar_filter_create() Fail(%d)", ret);
			ret = CALENDAR_ERROR_OUT_OF_MEMORY;
			goto ERROR_RETURN;
		}
		que->filter = (cal_composite_filter_s*)filter;

		ret = cal_ipc_unmarshal_int(ipc_data, &count);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
			ret = CALENDAR_ERROR_INVALID_PARAMETER;
			goto ERROR_RETURN;
		}

		ret = _cal_ipc_unmarshal_composite_filter(ipc_data, que->filter);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("_cal_ipc_unmarshal_composite_filter() Fail(%d)", ret);
			ret = CALENDAR_ERROR_INVALID_PARAMETER;
			goto ERROR_RETURN;
		}
	}

	ret = cal_ipc_unmarshal_int(ipc_data, &(que->projection_count));
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		ret = CALENDAR_ERROR_INVALID_PARAMETER;
		goto ERROR_RETURN;
	}

	if (0 < que->projection_count) {
		que->projection = (unsigned int*)calloc(que->projection_count, sizeof(int));
		if (NULL == que->projection) {
			ERR("calloc() Fail");
			ret = CALENDAR_ERROR_OUT_OF_MEMORY;
			goto ERROR_RETURN;
		}

		for(i=0;i<que->projection_count;i++) {
			ret = cal_ipc_unmarshal_uint(ipc_data, &(que->projection[i]));
			if (CALENDAR_ERROR_NONE != ret) {
				ERR("cal_ipc_unmarshal_uint() Fail(%d)", ret);
				ret = CALENDAR_ERROR_INVALID_PARAMETER;
				goto ERROR_RETURN;
			}
		}
	}
	else {
		que->projection = NULL;
	}

	ret = cal_ipc_unmarshal_int(ipc_data, &(que->sort_property_id));
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		ret = CALENDAR_ERROR_INVALID_PARAMETER;
		goto ERROR_RETURN;
	}

	ret = cal_ipc_unmarshal_int(ipc_data, (int*)&(que->asc));
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		ret = CALENDAR_ERROR_INVALID_PARAMETER;
		goto ERROR_RETURN;
	}

	que->properties = (cal_property_info_s *)cal_view_get_property_info(que->view_uri, &que->property_count);

	ret = cal_ipc_unmarshal_int(ipc_data, (int*)&(que->distinct));
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		ret = CALENDAR_ERROR_INVALID_PARAMETER;
		goto ERROR_RETURN;
	}

	return CALENDAR_ERROR_NONE;

ERROR_RETURN:

	calendar_query_destroy(*query);
	*query = NULL;

	return ret;
}

int cal_ipc_marshal_query(const calendar_query_h query, pims_ipc_data_h ipc_data)
{
	int ret = 0;
	cal_query_s *que = NULL;
	int i = 0;
	int length = 0;

	RETV_IF(NULL == query, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == ipc_data, CALENDAR_ERROR_INVALID_PARAMETER);
	que = (cal_query_s *)query;

	length = strlen(que->view_uri);
	if (pims_ipc_data_put(ipc_data, (void*)que->view_uri,length+1) < 0) {
		ERR("pims_ipc_data_put() Fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	if (que->filter) {
		ret = cal_ipc_marshal_int(1, ipc_data);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_ipc_marshal_int() Fail(%d)", ret);
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
		ret = _cal_ipc_marshal_composite_filter(que->filter, ipc_data);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("_cal_ipc_marshal_composite_filter() Fail(%d)", ret);
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
	}
	else {
		ret = cal_ipc_marshal_int(0, ipc_data);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_ipc_marshal_int() Fail(%d)", ret);
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
	}

	ret = cal_ipc_marshal_int((que->projection_count), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	for(i=0;i<que->projection_count;i++) {
		ret = cal_ipc_marshal_uint((que->projection[i]), ipc_data);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_ipc_marshal_uint() Fail(%d)", ret);
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
	}

	ret = cal_ipc_marshal_int((que->sort_property_id), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	ret = cal_ipc_marshal_int((int)(que->asc), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	ret = cal_ipc_marshal_int((int)(que->distinct), ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}

int cal_ipc_unmarshal_list(const pims_ipc_data_h ipc_data, calendar_list_h* list)
{
	int count = 0, i = 0;
	calendar_record_h record;
	int ret = CALENDAR_ERROR_NONE;

	RETVM_IF(NULL == list, CALENDAR_ERROR_INVALID_PARAMETER, "list is NULL");
	RETVM_IF(NULL == ipc_data, CALENDAR_ERROR_INVALID_PARAMETER, "ipc_data is NULL");

	ret = calendar_list_create(list);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("calendar_list_create() Fail(%d)", ret);
		return CALENDAR_ERROR_OUT_OF_MEMORY;
	}

	ret = cal_ipc_unmarshal_int(ipc_data, &(count));
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		ret = CALENDAR_ERROR_INVALID_PARAMETER;
		goto ERROR_RETURN;
	}

	for(i=0;i<count;i++) {
		ret = cal_ipc_unmarshal_record(ipc_data, &record);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_ipc_unmarshal_record() Fail(%d)", ret);
			ret = CALENDAR_ERROR_INVALID_PARAMETER;
			goto ERROR_RETURN;
		}

		ret = calendar_list_add(*list, record);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("calendar_list_add() Fail(%d)", ret);
			ret = CALENDAR_ERROR_INVALID_PARAMETER;
			goto ERROR_RETURN;
		}
	}
	calendar_list_first(*list);

	return CALENDAR_ERROR_NONE;

ERROR_RETURN:
	if (*list) {
		calendar_list_destroy(*list, true);
		*list = NULL;
	}

	return ret;
}

int cal_ipc_marshal_list(const calendar_list_h list, pims_ipc_data_h ipc_data)
{
	int ret = 0;
	int count = 0;
	int i = 0;
	calendar_record_h record;
	RETV_IF(NULL == list, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == ipc_data, CALENDAR_ERROR_INVALID_PARAMETER);

	ret = calendar_list_get_count(list, &count);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("calendar_list_get_count() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	ret = cal_ipc_marshal_int(count,ipc_data);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	calendar_list_first(list);

	for(i=0;i<count;i++) {
		ret = calendar_list_get_current_record_p(list, &record);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("calendar_list_get_current_record_p() Fail(%d)", ret);
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}

		ret = cal_ipc_marshal_record(record, ipc_data);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_ipc_marshal_record() Fail(%d)", ret);
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
		calendar_list_next(list);
	}

	return CALENDAR_ERROR_NONE;
}

int cal_ipc_unmarshal_child_list(const pims_ipc_data_h ipc_data, calendar_list_h* list)
{
	int ret = 0;
	unsigned int i = 0;
	int count = 0;
	calendar_record_h record;

	RETV_IF(NULL == list, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == ipc_data, CALENDAR_ERROR_INVALID_PARAMETER);

	ret = cal_ipc_unmarshal_int(ipc_data, &(count));
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	for(i=0;i<count;i++) {
		ret = cal_ipc_unmarshal_record(ipc_data, &record);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_ipc_unmarshal_record() Fail(%d)", ret);
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}

		ret = calendar_list_add(*list, record);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("calendar_list_add() Fail(%d)", ret);
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
	}

	return CALENDAR_ERROR_NONE;
}

int cal_ipc_marshal_handle(calendar_h handle, const pims_ipc_data_h ipc_data)
{
	int ret = 0;
	cal_s *h = (cal_s *)handle;
	RETV_IF(NULL == handle, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == ipc_data, CALENDAR_ERROR_INVALID_PARAMETER);

	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret , "cal_ipc_marshal_char() Fail(%d)", ret);
	return CALENDAR_ERROR_NONE;
}

int cal_ipc_unmarshal_handle(const pims_ipc_data_h ipc_data, calendar_h *handle)
{
	int ret = 0;

	RETV_IF(NULL == ipc_data, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == handle, CALENDAR_ERROR_INVALID_PARAMETER);

	ret = cal_handle_create(handle);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_handle_create() Fail(%d)", ret);
	cal_s *h = (cal_s *)(*handle);

	return CALENDAR_ERROR_NONE;
}
