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
void _cal_time_get_registered_tzid_with_offset(int offset, char *registered_tzid, int tzid_size);
UCalendar *_cal_time_get_ucal(const char *tzid, int wkst);
char* _cal_time_extract_by(int calendar_system_type, const char *tzid, int wkst, calendar_time_s *ct, int field);
char* _cal_time_convert_ltos(const char *tzid, long long int lli, int is_allday);
long long int _cal_time_convert_itol(const char *tzid, int y, int m, int d, int h, int min, int s);
int _cal_time_utoi(long long int utime, char *tzid, int *y, int *m, int *d, int *h, int *min, int *s);
long long int _cal_time_convert_stol(char *tzid, char *datetime);
int _cal_time_ltoi(char *tzid, long long int lli, int *year, int *month, int *mday);
int _cal_time_ltoi2(char *tzid, long long int lli, int *nth, int *wday);
long long int _cal_time_get_now(void);
int _cal_time_get_timezone_from_table(const char *tzid, calendar_record_h *timezone, int *timezone_id);
int _cal_time_get_like_tzid(const char *tzid, calendar_record_h timezone, char **like_tzid);
int _cal_time_get_next_date(calendar_time_s *today, calendar_time_s *next);
char* _cal_time_get_timezone(void);
long long int _cal_time_get_utime(UCalendar *ucal, int y, int mon, int d, int h, int min, int s);
int _cal_time_get_component_from_lli(UCalendar *ucal, long long int lli, int *year, int *month, int *mday, int *hour, int *minute, int *second);
int _cal_time_get_next_time(UCalendar *ucal, int offset, int freq, calendar_time_s *next);
void _cal_time_u_cleanup(void);
void _cal_time_get_tz_offset(const char *tz, time_t *zone_offset, time_t *dst_offset);
bool _cal_time_in_dst(const char *tz, long long int t);
UCalendar* _cal_time_open_ucal(int calendar_system_type, const char *tzid, int wkst);
int _cal_time_init(void);
void _cal_time_fini(void);
long long int _cal_time_convert_lli(char *p);
void _cal_time_modify_caltime(calendar_time_s *caltime, long long int diff);
void _cal_time_get_nth_wday(long long int t, int *nth, int *wday);
void _cal_time_get_datetime(long long int t, int *y, int *m, int *d, int *h, int *n, int *s);
void _cal_time_get_local_datetime(char *tzid, long long int t, int *y, int *m, int *d, int *h, int *n, int *s);
bool _cal_time_is_available_tzid(char *tzid);

enum cal_extract_field {
	CAL_MONTH,
	CAL_DAY_OF_WEEK,
	CAL_DATE,
};

#ifdef __cplusplus
}
#endif

#endif // __CALENDAR_SVC_TIME_H__
