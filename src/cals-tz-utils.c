/*
 * Calendar Service
 *
 * Copyright (c) 2000 - 2012 Samsung Electronics Co., Ltd. All rights reserved.
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
#include <time.h>
#include <sys/types.h>
#include <vconf.h>
#include <stdlib.h>

#include "cals-internal.h"
#include "cals-tz-utils.h"

struct tm cal_tm_value = {0};

struct tm* cals_tmtime(time_t *sec)
{
	struct tm* temp = gmtime_r(sec,&cal_tm_value);

	if(temp)
		return &cal_tm_value;
	else
		return NULL;
}

time_t cals_mktime(struct tm *date_time)
{
	return timegm(date_time);
}

struct tm* cals_tmtime_r(time_t *ttTime, struct tm *tmTime)
{
	struct tm* pTmTime = NULL;
	retvm_if(NULL == ttTime, NULL,"ttTime is NULL");
	retvm_if(NULL == tmTime, NULL,"tmTime is NULL");

	pTmTime = (struct tm*)cals_tmtime(ttTime);
	memcpy(tmTime,pTmTime,sizeof(struct tm));
	return pTmTime;
}

static bool _get_tz_path(const char *city, char **tz_path)
{
	int left = 0;
	int right = sizeof(time_zone_array)/sizeof(Time_Zone) -1;
	int index = 0;
	int result = 0;
	int max_count = right;


	while(true)
	{
		index = ( left + right ) / 2;

		result = strcmp(time_zone_array[index].city, city);

		if(result == 0)
		{
			*tz_path = strdup(time_zone_array[index].tz_path);
		}
		else if(result < 0)
			left = index+1;
		else
			right = index-1;

		if(left <0 || left >max_count)
			goto error;
		if(right <0 || right >max_count)
			goto error;
		if(left > right)
			goto error;

	}

	if(*tz_path == NULL)
		return false;
	else
		return true;
error:
	*tz_path = NULL;
	return false;
}


char *cals_tzutil_get_tz_path(void)
{
	int lock_on;
	char *tz_path;
	char *city;
	bool ret;

	tz_path = NULL;
	lock_on = 0;

	vconf_get_int("db/calendar/timezone_on_off", &lock_on);

	if (lock_on) {
		tz_path = vconf_get_str("db/calendar/timezone_path");
		return tz_path;
	}

	city = vconf_get_str(VCONFKEY_SETAPPL_CITYNAME_INDEX_INT);

	if (!city)
		return NULL;

	ret = _get_tz_path(city, &tz_path);
	free(city);

	if (ret)
		return tz_path;

	return strdup("localtime");
}


