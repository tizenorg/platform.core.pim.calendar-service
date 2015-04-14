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
#include "cal_handle.h"

int cal_handle_create(calendar_h *handle)
{
	RETV_IF(NULL == handle, CALENDAR_ERROR_INVALID_PARAMETER);

	cal_s *h = calloc(1, sizeof(cal_s));
	RETVM_IF(NULL == h, CALENDAR_ERROR_OUT_OF_MEMORY, "calloc() Fail");
	*handle = (calendar_h)h;
	return CALENDAR_ERROR_NONE;
}

int cal_handle_destroy(calendar_h handle)
{
	RETV_IF(NULL == handle, CALENDAR_ERROR_INVALID_PARAMETER);

	cal_s *h = (cal_s *)handle;
	free(h->zone_name);
	free(h);
	return CALENDAR_ERROR_NONE;
}

int cal_handle_set_zone_name(calendar_h handle, const char *zone_name)
{
	RETV_IF(NULL == handle, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == zone_name, CALENDAR_ERROR_INVALID_PARAMETER);

	cal_s *h = (cal_s *)handle;
	h->zone_name = CAL_SAFE_STRDUP(zone_name);
	return CALENDAR_ERROR_NONE;
}

int cal_handle_get_zone_name(calendar_h handle, char **zone_name)
{
	RETV_IF(NULL == handle, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == zone_name, CALENDAR_ERROR_INVALID_PARAMETER);

	cal_s *h = (cal_s *)handle;
	if (h->zone_name)
		*zone_name = CAL_SAFE_STRDUP(h->zone_name);
	else
		*zone_name = strdup(""); // empty string means host
	return CALENDAR_ERROR_NONE;
}

