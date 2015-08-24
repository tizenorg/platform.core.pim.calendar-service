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
#include "cal_list.h"

API int calendar_list_create(calendar_list_h* out_list)
{
	if (NULL == out_list)
	{
		ERR("Invalid parameter: list is NULL");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	cal_list_s *l;

	l = calloc(1, sizeof(cal_list_s));
	if (l == NULL) {
		ERR("calloc() Fail");
		return CALENDAR_ERROR_OUT_OF_MEMORY;
	}

	l->count = 0;
	l->record = NULL;
	l->cursor = NULL;

	*out_list = (calendar_list_h)l;
	return CALENDAR_ERROR_NONE;
}

API int calendar_list_get_count(calendar_list_h list, int *count)
{
	cal_list_s *l;

	if (list == NULL) {
		ERR("Invalid argument: calendar_list_h is NULL");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (count == NULL) {
		ERR("Invalid argument: count is NULL");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	l = (cal_list_s *)list;

	*count = l->count;
	return CALENDAR_ERROR_NONE;
}

API int calendar_list_add(calendar_list_h list, calendar_record_h record)
{
	cal_list_s *l;

	if (list == NULL) {
		ERR("Invalid argument: calendar_list_h is NULL");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (record == NULL) {
		ERR("Invalid argument: calendar_record_h is NULL");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	l = (cal_list_s *)list;

	l->count++;
	l->record = g_list_append(l->record, record);
	//l->cursor = g_list_nth(l->record, (l->count) -1);

	return CALENDAR_ERROR_NONE;
}

API int calendar_list_remove(calendar_list_h list, calendar_record_h record)
{
	GList *cursor;
	cal_list_s *l;

	if (list == NULL) {
		ERR("Invalid argument: calendar_list_h is NULL");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (record == NULL) {
		ERR("Invalid argument: calendar_record_h is NULL");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	l = (cal_list_s *)list;

	cursor = l->record;
	while (cursor) {
		if (cursor->data == record) {
			l->cursor = cursor->next;
			l->record = g_list_remove(l->record, cursor->data);
			l->count--;
			return CALENDAR_ERROR_NONE;
		}
		cursor = cursor->next;
	}
	return CALENDAR_ERROR_NO_DATA;
}

API int calendar_list_get_current_record_p(calendar_list_h list, calendar_record_h* record)
{
	cal_list_s *l;

	if (list == NULL) {
		ERR("Invalid argument: calendar_list_h is NULL");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (record == NULL) {
		ERR("Invalid argument: calendar_record_h is NULL");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	l = (cal_list_s *)list;
	if (l->cursor == NULL) {
		*record = NULL;
		return CALENDAR_ERROR_NO_DATA;
	}

	*record = l->cursor->data;

	return CALENDAR_ERROR_NONE;
}

API int calendar_list_prev(calendar_list_h list)
{
	cal_list_s *l;

	if (list == NULL) {
		ERR("Invalid argument: calendar_list_h is NULL");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	l = (cal_list_s *)list;
	l->cursor = g_list_previous(l->cursor);
	if (l->cursor == NULL) {
		DBG("No prev list");
		return CALENDAR_ERROR_NO_DATA;
	}

	return CALENDAR_ERROR_NONE;
}

API int calendar_list_next(calendar_list_h list)
{
	cal_list_s *l;

	if (list == NULL) {
		ERR("Invalid argument: calendar_list_h is NULL");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	l = (cal_list_s *)list;
	l->cursor = g_list_next(l->cursor);
	if (l->cursor == NULL) {
		//DBG("No next list");
		return CALENDAR_ERROR_NO_DATA;
	}

	return CALENDAR_ERROR_NONE;
}

API int calendar_list_first(calendar_list_h list)
{
	cal_list_s *l;

	if (list == NULL) {
		ERR("Invalid argument: calendar_list_h is NULL");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	l = (cal_list_s *)list;
	l->cursor = g_list_first(l->record);

	return CALENDAR_ERROR_NONE;
}

API int calendar_list_last(calendar_list_h list)
{
	cal_list_s *l;

	if (list == NULL) {
		ERR("Invalid argument: calendar_list_h is NULL");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	l = (cal_list_s *)list;
	l->cursor = g_list_last(l->record);

	return CALENDAR_ERROR_NONE;
}

API int calendar_list_destroy(calendar_list_h list, bool delete_record)
{
	GList *cursor;
	cal_list_s *l = NULL;

	if (list == NULL) {
		ERR("Invalid argument: calendar_list_h is NULL");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	l = (cal_list_s *)list;

	if (delete_record == true)
	{
		cursor = l->record;

		while (cursor)
		{
			if (cursor->data)
			{
				calendar_record_destroy((calendar_record_h)(cursor->data), true);
			}
			cursor = cursor->next;
		}
	}
	if(l->record)
	{
		g_list_free(l->record);
	}
	CAL_FREE(l);

	return CALENDAR_ERROR_NONE;
}

int cal_list_clone(calendar_list_h list, calendar_list_h *out_list)
{
	int ret = CALENDAR_ERROR_NONE;
	int count = 0, i = 0;
	calendar_list_h l = NULL;

	if (NULL == list || NULL == out_list)
	{
		ERR("Invalid parameter");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	ret = calendar_list_get_count(list, &count);
	if (CALENDAR_ERROR_NONE != ret)
	{
		return ret;
	}

	ret = calendar_list_first(list);
	if (CALENDAR_ERROR_NONE != ret)
	{
		return ret;
	}

	ret = calendar_list_create(&l);
	if (CALENDAR_ERROR_NONE != ret)
	{
		return ret;
	}

	for(i = 0; i < count; i++)
	{
		calendar_record_h record = NULL;
		calendar_record_h clone_record = NULL;
		if (calendar_list_get_current_record_p(list,&record) != CALENDAR_ERROR_NONE)
		{
			ERR("calendar_list_get_count fail");
			calendar_list_destroy(l, true);
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}

		if (calendar_record_clone(record, &clone_record) != CALENDAR_ERROR_NONE)
		{
			ERR("calendar_list_get_count fail");
			calendar_list_destroy(l, true);
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}

		if (calendar_list_add(l, clone_record) != CALENDAR_ERROR_NONE)
		{
			ERR("calendar_list_get_count fail");
			calendar_list_destroy(l, true);
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}

		calendar_list_next(list);
	}

	*out_list = l;

	return CALENDAR_ERROR_NONE;
}

int cal_list_get_nth_record_p(cal_list_s *list_s, int index, calendar_record_h *record)
{
	RETV_IF(index < 0, CALENDAR_ERROR_INVALID_PARAMETER);

	RETV_IF(NULL == record, CALENDAR_ERROR_INVALID_PARAMETER);
	*record = NULL;

	RETV_IF(NULL == list_s, CALENDAR_ERROR_INVALID_PARAMETER);

	if (index < list_s->count) {
		*record = g_list_nth_data(list_s->record, index);
		return CALENDAR_ERROR_NONE;
	}

	ERR("Check index(%d) > count(%d)", index, list_s->count);
	return CALENDAR_ERROR_NO_DATA;
}

int cal_list_clear(cal_list_s *list_s)
{
	int ret = CALENDAR_ERROR_NONE;
	calendar_record_h record = NULL;
	calendar_list_h list = (calendar_list_h)list_s;
	RETV_IF(NULL == list, CALENDAR_ERROR_INVALID_PARAMETER);

	calendar_list_first(list);
	while (CALENDAR_ERROR_NONE == calendar_list_get_current_record_p(list, &record)) {
		ret = calendar_list_remove(list, record);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("calendar_list_remove() Fail(%d)", ret);
			break;
		}
	}
	return ret;
}

