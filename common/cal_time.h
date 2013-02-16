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

#ifndef __CALENDAR_SVC_TIME_H__
#define __CALENDAR_SVC_TIME_H__

#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <unicode/ucal.h>
#include <unicode/ustring.h>
#include <unicode/ustdio.h>
#include <unicode/udat.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

int _cal_time_is_registered_tzid(const char *tzid);
UCalendar *_cal_time_get_ucal(const char *tzid, int wkst);
char * _cal_time_extract_by(const char *tzid, int wkst, calendar_time_s *ct, int field);
char * _cal_time_convert_ltos(const char *tzid, long long int lli);
long long int _cal_time_convert_itol(const char *tzid, int y, int m, int d, int h, int min, int s);
long long int _cal_time_convert_stol(char *tzid, char *datetime);
int _cal_time_ltoi(char *tzid, long long int lli, int *year, int *month, int *mday);
int _cal_time_ltoi2(char *tzid, long long int lli, int *nth, int *wday);
long long int _cal_time_get_now(void);
int _cal_time_get_timezone_from_table(const char *tzid, calendar_record_h *timezone, int *timezone_id);
int _cal_time_get_like_tzid(const char *tzid, calendar_record_h timezone, char **like_tzid);

enum cal_extract_field {
	CAL_DAY_OF_WEEK,
	CAL_DATE,
};

#ifdef __cplusplus
}
#endif

#endif // __CALENDAR_SVC_TIME_H__
