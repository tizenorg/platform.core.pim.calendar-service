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

#ifndef __CALENDAR_SVC_VCALENDAR_PARSE_H__
#define __CALENDAR_SVC_VCALENDAR_PARSE_H__

#include "calendar_vcalendar.h"

typedef struct {
	calendar_vcalendar_parse_cb callback;
	void *user_data;
	bool ret;
} vcalendar_foreach_s;

char* _cal_vcalendar_parse_remove_space(char *src);
int _cal_vcalendar_parse_unfolding(char *stream);
char* _cal_vcalendar_parse_read_line(char *stream, char **line);
char* _cal_vcalendar_parse_read_key_value(char *stream, char **prop, char **cont);
int _cal_vcalendar_parse_vcalendar_object(char *vcalendar_object_stream, calendar_list_h list, vcalendar_foreach_s *foreach_data);

#endif // __CALENDAR_SVC_VCALENDAR_PARSE_H__
