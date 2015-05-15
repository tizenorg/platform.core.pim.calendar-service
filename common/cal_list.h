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
#ifndef __CALENDAR_SVC_LIST_H__
#define __CALENDAR_SVC_LIST_H__

#include "calendar_list.h"

int cal_list_clone(calendar_list_h list, calendar_list_h *out_list);
int cal_list_get_nth_record_p(cal_list_s *list_s, int index, calendar_record_h *record);
int cal_list_clear(cal_list_s *list_s);

#endif /* __CALENDAR_SVC_LIST_H__ */
