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

#include "cal_filter.h"

static int _cal_filter_create_attribute(cal_composite_filter_s *com_filter, unsigned int property_id,
		int match, int filter_type, cal_attribute_filter_s **out_filter);

static int _cal_filter_destroy_composite(cal_composite_filter_s* filter);
static int _cal_filter_destroy_attribute(cal_attribute_filter_s* filter);

static int _cal_filter_clone_composite(cal_composite_filter_s* filter,
		cal_composite_filter_s **out_filter);
static int _cal_filter_clone_attribute(cal_attribute_filter_s* filter,
		cal_attribute_filter_s **out_filter);

API int calendar_filter_create(const char* view_uri, calendar_filter_h* out_filter)
{
	cal_composite_filter_s *com_filter;

	RETV_IF(NULL == view_uri, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_filter, CALENDAR_ERROR_INVALID_PARAMETER);
	com_filter = calloc(1, sizeof(cal_composite_filter_s));
	RETV_IF(NULL == com_filter, CALENDAR_ERROR_OUT_OF_MEMORY);

	com_filter->filter_type = CAL_FILTER_COMPOSITE;
	com_filter->view_uri = strdup(view_uri);
	com_filter->properties = (cal_property_info_s *)cal_view_get_property_info(view_uri, &com_filter->property_count);
	*out_filter = (calendar_filter_h)com_filter;
	return CALENDAR_ERROR_NONE;
}

API int calendar_filter_add_operator(calendar_filter_h filter, calendar_filter_operator_e op)
{
	cal_composite_filter_s *com_filter;

	RETV_IF(NULL == filter, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(CALENDAR_FILTER_OPERATOR_MAX <= op, CALENDAR_ERROR_INVALID_PARAMETER, "max(%d) <= operator(%d)", CALENDAR_FILTER_OPERATOR_MAX, op);

	com_filter = (cal_composite_filter_s*)filter;

	RETVM_IF(g_slist_length(com_filter->filter_ops) != (g_slist_length(com_filter->filters)-1),
			CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter : Please check the operator of filter");
	com_filter->filter_ops = g_slist_append(com_filter->filter_ops, (void*)op);
	return CALENDAR_ERROR_NONE;
}

API int calendar_filter_add_filter(calendar_filter_h filter, calendar_filter_h add_filter)
{
	cal_composite_filter_s *com_filter;
	cal_composite_filter_s *com_filter2;
	calendar_filter_h f = NULL;
	int ret = CALENDAR_ERROR_NONE;

	RETV_IF(NULL == filter, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == add_filter, CALENDAR_ERROR_INVALID_PARAMETER);

	com_filter = (cal_composite_filter_s*)filter;
	com_filter2 = (cal_composite_filter_s*)add_filter;

	RETVM_IF(g_slist_length(com_filter->filter_ops) != g_slist_length(com_filter->filters),
			CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter :Please check the operator of filter");
	RETVM_IF(0 != strcmp(com_filter->view_uri, com_filter2->view_uri), CALENDAR_ERROR_INVALID_PARAMETER,
			"The filter view_uri is different (filter1:%s, filter2:%s)", com_filter->view_uri, com_filter2->view_uri);

	ret = cal_filter_clone(add_filter, &f);
	RETV_IF(ret != CALENDAR_ERROR_NONE, ret);

	com_filter->filters = g_slist_append(com_filter->filters, f);

	return CALENDAR_ERROR_NONE;
}

static int _cal_filter_create_attribute(cal_composite_filter_s *com_filter, unsigned int property_id, int match, int filter_type, cal_attribute_filter_s **out_filter)
{
	cal_attribute_filter_s *filter;
	//int type;
	//bool find = false;

	RETVM_IF(g_slist_length(com_filter->filter_ops) != g_slist_length(com_filter->filters),
			CALENDAR_ERROR_INVALID_PARAMETER, "Invalid parameter :Please check the operator of filter");

	filter = calloc(1, sizeof(cal_attribute_filter_s));
	RETVM_IF(NULL == filter, CALENDAR_ERROR_OUT_OF_MEMORY, "calloc() Fail");
	filter->filter_type = filter_type;
	filter->property_id = property_id;
	filter->match = match;

	com_filter->filters = g_slist_append(com_filter->filters, filter);
	*out_filter = filter;
	return CALENDAR_ERROR_NONE;
}

API int calendar_filter_add_str(calendar_filter_h filter, unsigned int property_id, calendar_match_str_flag_e match, const char* match_value)
{
	cal_composite_filter_s *com_filter;
	cal_attribute_filter_s *str_filter;
	int ret;
	bool bcheck;

	RETV_IF(NULL == filter, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == match_value, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(CALENDAR_MATCH_STR_MAX <= match, CALENDAR_ERROR_INVALID_PARAMETER, "max(%d) <= match(%d)", CALENDAR_MATCH_STR_MAX, match);

	bcheck = CAL_PROPERTY_CHECK_DATA_TYPE(property_id,CAL_PROPERTY_DATA_TYPE_STR);
	RETVM_IF(false == bcheck, CALENDAR_ERROR_INVALID_PARAMETER, "property_id(%d)", property_id);

	bcheck = CAL_PROPERTY_CHECK_FLAGS(property_id,CAL_PROPERTY_FLAGS_PROJECTION);
	RETVM_IF(true == bcheck, CALENDAR_ERROR_INVALID_PARAMETER, "property_id(%d)", property_id);

	com_filter = (cal_composite_filter_s*)filter;
	ret = _cal_filter_create_attribute(com_filter, property_id, match, CAL_FILTER_STR, &str_filter);
	RETVM_IF(CALENDAR_ERROR_NONE !=ret, ret, "property_id(%d), match(%d), match_value(%s)", property_id, match, match_value);

	str_filter->value.s = SAFE_STRDUP(match_value);
	return CALENDAR_ERROR_NONE;
}

API int calendar_filter_add_int(calendar_filter_h filter, unsigned int property_id, calendar_match_int_flag_e match, int match_value)
{
	cal_composite_filter_s *com_filter;
	cal_attribute_filter_s *int_filter;
	int ret;
	bool bcheck;

	RETV_IF(NULL == filter, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(CALENDAR_MATCH_INT_MAX <= match, CALENDAR_ERROR_INVALID_PARAMETER, "max(%d) <= match(%d)", CALENDAR_MATCH_INT_MAX, match);

	bcheck = CAL_PROPERTY_CHECK_DATA_TYPE(property_id,CAL_PROPERTY_DATA_TYPE_INT);
	RETVM_IF(false == bcheck, CALENDAR_ERROR_INVALID_PARAMETER, "property_id(%d)", property_id);

	bcheck = CAL_PROPERTY_CHECK_FLAGS(property_id,CAL_PROPERTY_FLAGS_PROJECTION);
	RETVM_IF(true == bcheck, CALENDAR_ERROR_INVALID_PARAMETER, "property_id(%d)", property_id);

	com_filter = (cal_composite_filter_s*)filter;
	ret = _cal_filter_create_attribute(com_filter, property_id, match, CAL_FILTER_INT, &int_filter);
	RETVM_IF(CALENDAR_ERROR_NONE !=ret, ret, "property_id(%d), match(%d), match_value(%d)", property_id, match, match_value);

	int_filter->value.i = match_value;

	return CALENDAR_ERROR_NONE;
}

API int calendar_filter_add_double(calendar_filter_h filter, unsigned int property_id, calendar_match_int_flag_e match, double match_value)
{
	cal_composite_filter_s *com_filter;
	cal_attribute_filter_s *int_filter;
	int ret;
	bool bcheck;

	RETV_IF(NULL == filter, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(CALENDAR_MATCH_INT_MAX <= match, CALENDAR_ERROR_INVALID_PARAMETER, "max(%d) <= match(%d)", CALENDAR_MATCH_INT_MAX, match);

	bcheck = CAL_PROPERTY_CHECK_DATA_TYPE(property_id,CAL_PROPERTY_DATA_TYPE_DOUBLE);
	RETVM_IF(false == bcheck, CALENDAR_ERROR_INVALID_PARAMETER, "property_id(%d)", property_id);

	bcheck = CAL_PROPERTY_CHECK_FLAGS(property_id,CAL_PROPERTY_FLAGS_PROJECTION);
	RETVM_IF(true == bcheck, CALENDAR_ERROR_INVALID_PARAMETER, "property_id(%d)", property_id);

	com_filter = (cal_composite_filter_s*)filter;
	ret = _cal_filter_create_attribute(com_filter, property_id, match, CAL_FILTER_DOUBLE, &int_filter);
	RETVM_IF(CALENDAR_ERROR_NONE !=ret, ret, "property_id(%d), match(%d), match_value(%d)", property_id, match, match_value);

	int_filter->value.d = match_value;

	return CALENDAR_ERROR_NONE;
}

API int calendar_filter_add_lli(calendar_filter_h filter, unsigned int property_id, calendar_match_int_flag_e match, long long int match_value)
{
	cal_composite_filter_s *com_filter;
	cal_attribute_filter_s *int_filter;
	int ret;
	bool bcheck;

	RETV_IF(NULL == filter, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(CALENDAR_MATCH_INT_MAX <= match, CALENDAR_ERROR_INVALID_PARAMETER, "max(%d) <= match(%d)", CALENDAR_MATCH_INT_MAX, match);

	bcheck = CAL_PROPERTY_CHECK_DATA_TYPE(property_id,CAL_PROPERTY_DATA_TYPE_LLI);
	RETVM_IF(false == bcheck, CALENDAR_ERROR_INVALID_PARAMETER, "property_id(%d)", property_id);

	bcheck = CAL_PROPERTY_CHECK_FLAGS(property_id,CAL_PROPERTY_FLAGS_PROJECTION);
	RETVM_IF(true == bcheck, CALENDAR_ERROR_INVALID_PARAMETER, "property_id(%d)", property_id);

	com_filter = (cal_composite_filter_s*)filter;
	ret = _cal_filter_create_attribute(com_filter, property_id, match, CAL_FILTER_LLI, &int_filter);
	RETVM_IF(CALENDAR_ERROR_NONE !=ret, ret, "property_id(%d), match(%d), match_value(%d)", property_id, match, match_value);

	int_filter->value.lli = match_value;

	return CALENDAR_ERROR_NONE;
}

API int calendar_filter_add_caltime(calendar_filter_h filter, unsigned int property_id, calendar_match_int_flag_e match, calendar_time_s match_value)
{
	cal_composite_filter_s *com_filter;
	cal_attribute_filter_s *int_filter;
	int ret;
	bool bcheck;

	RETV_IF(NULL == filter, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(CALENDAR_MATCH_INT_MAX <= match, CALENDAR_ERROR_INVALID_PARAMETER, "max(%d) <= match(%d)", CALENDAR_MATCH_INT_MAX, match);

	bcheck = CAL_PROPERTY_CHECK_DATA_TYPE(property_id,CAL_PROPERTY_DATA_TYPE_CALTIME);
	RETVM_IF(false == bcheck, CALENDAR_ERROR_INVALID_PARAMETER, "property_id(%d)", property_id);

	bcheck = CAL_PROPERTY_CHECK_FLAGS(property_id,CAL_PROPERTY_FLAGS_PROJECTION);
	RETVM_IF(true == bcheck, CALENDAR_ERROR_INVALID_PARAMETER, "property_id(%d)", property_id);

	com_filter = (cal_composite_filter_s*)filter;
	ret = _cal_filter_create_attribute(com_filter, property_id, match, CAL_FILTER_CALTIME, &int_filter);
	RETVM_IF(CALENDAR_ERROR_NONE !=ret, ret, "property_id(%d), match(%d), match_value(%d)", property_id, match, match_value);

	int_filter->value.caltime = match_value;

	return CALENDAR_ERROR_NONE;
}

API int calendar_filter_destroy(calendar_filter_h filter)
{
	RETV_IF(NULL == filter, CALENDAR_ERROR_INVALID_PARAMETER);

	return _cal_filter_destroy_composite((cal_composite_filter_s*)filter);
}

int cal_filter_clone(calendar_filter_h filter, calendar_filter_h* out_filter)
{
	RETV_IF(NULL == filter, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_filter, CALENDAR_ERROR_INVALID_PARAMETER);

	return _cal_filter_clone_composite((cal_composite_filter_s*)filter, (cal_composite_filter_s**)out_filter);
}

static int _cal_filter_destroy_composite(cal_composite_filter_s* filter)
{
	GSList *cursor = NULL;

	RETV_IF(NULL == filter, CALENDAR_ERROR_INVALID_PARAMETER);
	for(cursor=filter->filters;cursor;cursor=cursor->next)
	{
		cal_filter_s *src = (cal_filter_s *)cursor->data;
		if (src == NULL)
			continue;
		if (src->filter_type == CAL_FILTER_COMPOSITE)
		{
			_cal_filter_destroy_composite((cal_composite_filter_s*)src);
		}
		else
		{
			_cal_filter_destroy_attribute((cal_attribute_filter_s*)src);
		}

	}
	CAL_FREE(filter->view_uri);
	g_slist_free(filter->filters);
	g_slist_free(filter->filter_ops);
	CAL_FREE(filter);

	return CALENDAR_ERROR_NONE;
}

static int _cal_filter_destroy_attribute(cal_attribute_filter_s* filter)
{
	RETV_IF(NULL == filter, CALENDAR_ERROR_INVALID_PARAMETER);

	if (filter->filter_type == CAL_FILTER_STR)
	{
		CAL_FREE(filter->value.s);
	}
	CAL_FREE(filter);
	return CALENDAR_ERROR_NONE;
}

static int _cal_filter_clone_composite(cal_composite_filter_s* filter,
		cal_composite_filter_s **out_filter)
{
	GSList *cursor;
	cal_composite_filter_s *out;
	int ret = CALENDAR_ERROR_NONE;

	RETV_IF(NULL == filter, CALENDAR_ERROR_INVALID_PARAMETER);

	ret = calendar_filter_create(filter->view_uri, (calendar_filter_h *)&out);
	RETV_IF(CALENDAR_ERROR_NONE != ret, CALENDAR_ERROR_OUT_OF_MEMORY);

	for(cursor=filter->filters; cursor ; cursor=cursor->next)
	{
		cal_filter_s *src = (cal_filter_s *)cursor->data;
		cal_filter_s *dest = NULL;

		if (src == NULL)
			continue;

		if (src->filter_type == CAL_FILTER_COMPOSITE)
		{
			ret = _cal_filter_clone_composite((cal_composite_filter_s *)src,
					(cal_composite_filter_s **)&dest);
		}
		else
		{
			ret = _cal_filter_clone_attribute((cal_attribute_filter_s *)src,
					(cal_attribute_filter_s **)&dest);
		}
		if (CALENDAR_ERROR_NONE == ret)
		{
			out->filters = g_slist_append(out->filters, dest);
		}
		else
		{
			calendar_filter_destroy((calendar_filter_h)out);
			return ret;
		}
	}

	out->filter_ops = g_slist_copy(filter->filter_ops);
	*out_filter = out;

	return CALENDAR_ERROR_NONE;
}

static int _cal_filter_clone_attribute(cal_attribute_filter_s* filter,
		cal_attribute_filter_s **out_filter)
{
	cal_attribute_filter_s *out;
	out = calloc(1, sizeof(cal_attribute_filter_s));
	RETV_IF(NULL == out, CALENDAR_ERROR_OUT_OF_MEMORY);

	out->filter_type = filter->filter_type;
	out->match = filter->match;
	out->property_id = filter->property_id;
	switch(filter->filter_type)
	{
	case CAL_FILTER_STR:
		out->value.s = SAFE_STRDUP(filter->value.s);
		break;
	case CAL_FILTER_INT:
		out->value.i = filter->value.i;
		break;
	case CAL_FILTER_DOUBLE:
		out->value.d = filter->value.d;
		break;
	case CAL_FILTER_LLI:
		out->value.lli = filter->value.lli;
		break;
	case CAL_FILTER_CALTIME:
		out->value.caltime = filter->value.caltime;
		break;
	default:
		break;
	}

	*out_filter = out;
	return CALENDAR_ERROR_NONE;
}
