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

#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_view.h"
#include "cal_filter.h"
#include "cal_query.h"

static bool _cal_query_property_check(const cal_property_info_s *properties,
		int count, unsigned int property_id)
{
	int i;

	for (i=0;i<count;i++)
	{
		cal_property_info_s *p = (cal_property_info_s*)&(properties[i]);
		if (property_id == p->property_id) {
			return true;
		}
	}
	return false;
}

API int calendar_query_create(const char* view_uri, calendar_query_h* out_query)
{
	cal_query_s *query;

	RETV_IF(NULL == view_uri, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_query, CALENDAR_ERROR_INVALID_PARAMETER);

	query = calloc(1, sizeof(cal_query_s));
	RETV_IF(NULL == query, CALENDAR_ERROR_OUT_OF_MEMORY);

	query->view_uri = strdup(view_uri);
	query->properties = (cal_property_info_s *)cal_view_get_property_info(view_uri, &query->property_count);
	*out_query = (calendar_query_h)query;

	return CALENDAR_ERROR_NONE;
}

API int calendar_query_set_projection(calendar_query_h query, unsigned int property_ids[], int count)
{
	cal_query_s *que = NULL;
	int i;
	bool find;

	RETV_IF(NULL == query, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == property_ids, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(count < 0, CALENDAR_ERROR_INVALID_PARAMETER, "count(%d) < 0", count);

	que = (cal_query_s *)query;

	for (i=0;i<count;i++)
	{
		find = _cal_query_property_check(que->properties, que->property_count, property_ids[i]);
		RETVM_IF(false == find, CALENDAR_ERROR_INVALID_PARAMETER,
				"Invalid parameter : property_id(%d) is not supported on view_uri(%s)", property_ids[i], que->view_uri);

		find = CAL_PROPERTY_CHECK_FLAGS(property_ids[i], CAL_PROPERTY_FLAGS_FILTER);
		RETVM_IF(true == find, CALENDAR_ERROR_INVALID_PARAMETER,
				"Invalid parameter : property_id(%d) is not supported on view_uri(%s)", property_ids[i], que->view_uri);
	}

	CAL_FREE(que->projection);

	que->projection = calloc(count, sizeof(unsigned int));
	RETVM_IF(NULL == que->projection, CALENDAR_ERROR_OUT_OF_MEMORY, "calloc() Fail");
	memcpy(que->projection, property_ids, sizeof(unsigned int) * count);
	que->projection_count = count;

	return CALENDAR_ERROR_NONE;
}

API int calendar_query_set_distinct(calendar_query_h query, bool set)
{
	cal_query_s *que = NULL;

	RETV_IF(NULL == query, CALENDAR_ERROR_INVALID_PARAMETER);
	que = (cal_query_s *)query;

	que->distinct = set;

	return CALENDAR_ERROR_NONE;
}

API int calendar_query_set_filter(calendar_query_h query, calendar_filter_h filter)
{
	cal_query_s *que;
	calendar_filter_h new_filter;
	int ret = CALENDAR_ERROR_NONE;

	RETV_IF(NULL == query, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == filter, CALENDAR_ERROR_INVALID_PARAMETER);

	que = (cal_query_s *)query;

	if (((cal_composite_filter_s*)filter)->filters == NULL)
	{
		ERR("Empty filter");
		return CALENDAR_ERROR_NO_DATA;
	}

	ret = cal_filter_clone(filter,&new_filter);
	RETV_IF(ret!=CALENDAR_ERROR_NONE, ret);

	if (que->filter)
	{
		calendar_filter_destroy((calendar_filter_h)que->filter);
	}

	que->filter = (cal_composite_filter_s*)new_filter;

	return ret;
}

API int calendar_query_set_sort(calendar_query_h query, unsigned int property_id, bool asc)
{
	cal_query_s *que;
	bool find = false;

	RETV_IF(NULL == query, CALENDAR_ERROR_INVALID_PARAMETER);
	que = (cal_query_s *)query;


	find = _cal_query_property_check(que->properties, que->property_count, property_id);
	RETVM_IF(false == find, CALENDAR_ERROR_INVALID_PARAMETER,
			"Invalid paramter : property_id(%d) is not supported on view_uri(%s)", property_id, que->view_uri);

	que->sort_property_id = property_id;
	que->asc = asc;

	return CALENDAR_ERROR_NONE;
}

API int calendar_query_destroy(calendar_query_h query)
{
	cal_query_s *que;

	RETV_IF(NULL == query, CALENDAR_ERROR_INVALID_PARAMETER);
	que = (cal_query_s *)query;

	if (que->filter)
	{
		calendar_filter_destroy((calendar_filter_h)que->filter);
	}

	CAL_FREE(que->view_uri);
	CAL_FREE(que->projection);
	CAL_FREE(que);

	return CALENDAR_ERROR_NONE;
}

int cal_query_clone(calendar_query_h query, calendar_query_h* out_query)
{
	cal_query_s *que;
	cal_query_s *out_que;
	cal_filter_s *out_filter = NULL;
	int ret = CALENDAR_ERROR_NONE;

	RETV_IF(NULL == query, CALENDAR_ERROR_INVALID_PARAMETER);
	que = (cal_query_s *)query;

	ret = calendar_query_create(que->view_uri, out_query);
	RETV_IF(CALENDAR_ERROR_NONE != ret, CALENDAR_ERROR_OUT_OF_MEMORY);
	out_que = (cal_query_s *)*out_query;

	if (que->filter)
	{
		cal_filter_clone((calendar_filter_h)que->filter,(calendar_filter_h*)&out_filter);
	}

	if (0 < que->projection_count)
	{
		out_que->projection = calloc(que->projection_count, sizeof(unsigned int));
		RETVM_IF(NULL == out_que->projection, CALENDAR_ERROR_OUT_OF_MEMORY, "calloc() Fail");
		memcpy(out_que->projection, que->projection , sizeof(unsigned int) * que->projection_count);
		out_que->projection_count = que->projection_count;
	}
	out_que->sort_property_id = que->sort_property_id;
	out_que->asc = que->asc;

	return CALENDAR_ERROR_NONE;
}

