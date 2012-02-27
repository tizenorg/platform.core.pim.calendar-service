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


