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
#include "cal_handle.h"
#include "cal_utils.h"

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
	free(h);
	return CALENDAR_ERROR_NONE;
}

int cal_handle_get_version(calendar_h handle, int *out_version)
{
	RETV_IF(NULL == handle, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_version, CALENDAR_ERROR_INVALID_PARAMETER);

	cal_s *h = (cal_s *)handle;
	*out_version = h->version;
	return CALENDAR_ERROR_NONE;
}
