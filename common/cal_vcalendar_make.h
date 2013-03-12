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

#ifndef __CALENDAR_SVC_VCALENDAR_MAKE_H__
#define __CALENDAR_SVC_VCALENDAR_MAKE_H__

#include "calendar_vcalendar.h"

typedef struct {
	int size;
	char *data;
	char lbuf[76];
} cal_make_s ;

cal_make_s *_cal_vcalendar_make_new(void);
int _cal_vcalendar_make_vcalendar(cal_make_s *b, calendar_list_h list);
char *_cal_vcalendar_make_get_data(cal_make_s *b);
void _cal_vcalendar_make_free(cal_make_s **b);

#endif // __CALENDAR_SVC_VCALENDAR_MAKE_H__
