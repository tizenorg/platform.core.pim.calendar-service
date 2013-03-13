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

#ifndef __CALENDAR_SVC_DB_ATTENDEE_H__
#define __CALENDAR_SVC_DB_ATTENDEE_H__

#include "cal_db.h"

int _cal_db_attendee_get_records(int event_id, GList **out_list);
int _cal_db_attendee_convert_gtoh(GList *glist, int id, calendar_list_h *hlist);
int _cal_db_attendee_delete_with_id(int event_id);

#endif  //__CALENDAR_SVC_DB_ATTENDEE_H__