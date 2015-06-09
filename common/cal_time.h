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

void cal_time_get_registered_tzid_with_offset(int offset, char *registered_tzid, int tzid_size);
UCalendar *cal_time_open_ucal(int calendar_system_type, const char *tzid, int wkst);
char* cal_time_extract_by(int calendar_system_type, const char *tzid, int wkst, calendar_time_s *ct, int field);
char* cal_time_convert_ltos(const char *tzid, long long int lli, int is_allday);
long long int cal_time_convert_itol(const char *tzid, int y, int m, int d, int h, int min, int s);
long long int cal_time_get_now(void);
int cal_time_get_next_date(calendar_time_s *today, calendar_time_s *next);
char* cal_time_get_timezone(void);
void cal_time_u_cleanup(void);
void cal_time_get_tz_offset(const char *tz, time_t *zone_offset, time_t *dst_offset);
bool cal_time_in_dst(const char *tz, long long int t);
int cal_time_init(void);
void cal_time_fini(void);
long long int cal_time_convert_lli(char *p);
void cal_time_modify_caltime(calendar_time_s *caltime, long long int diff);
void cal_time_get_nth_wday(long long int t, int *nth, int *wday);
void cal_time_get_datetime(long long int t, int *y, int *m, int *d, int *h, int *n, int *s);
void cal_time_get_local_datetime(char *tzid, long long int t, int *y, int *m, int *d, int *h, int *n, int *s);
bool cal_time_is_available_tzid(char *tzid);

enum cal_extract_field {
	CAL_MONTH,
	CAL_DAY_OF_WEEK,
	CAL_DATE,
};

#ifdef __cplusplus
}
#endif

#endif // __CALENDAR_SVC_TIME_H__
