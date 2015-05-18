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

#include <vasum.h>

#include "calendar_errors.h"
#include "cal_internal.h"
#include "cal_zone.h"
#include "cal_utils.h"

int cal_zone_get_root_path(const char *zone_name, char *path, int path_size)
{
	if (zone_name && '\0' == *zone_name)
		return 0;

	return snprintf(path, path_size, CAL_ZONE_BASE_PATH, zone_name);
}

int cal_zone_get_canonicalize_path(const char *zone_name, const char *path, char **canonicalize_path)
{
	RETV_IF(NULL == path, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == canonicalize_path, CALENDAR_ERROR_INVALID_PARAMETER);

	char buf[1024] = {0};
	if (zone_name) {
		if ('\0' == *zone_name)
			snprintf(buf, sizeof(buf), "%s", path);
		else
			snprintf(buf, sizeof(buf), CAL_ZONE_BASE_PATH"%s", zone_name, path);

		*canonicalize_path = cal_strdup(buf);
		return CALENDAR_ERROR_NONE;
	}

	int ret = 0;
	ret = vsm_canonicalize_path((char *)path, canonicalize_path);
	WARN_IF(0 != ret, "vsm_canonicalize_path() Fail(%d)", ret);
	return CALENDAR_ERROR_NONE;
}
