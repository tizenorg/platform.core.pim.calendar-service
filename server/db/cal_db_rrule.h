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

#ifndef __CAL_DB_RRULE_H__
#define __CAL_DB_RRULE_H__

void cal_db_rrule_set_default(calendar_record_h event);
int cal_db_rrule_insert_record(int id, cal_rrule_s *rrule);
int cal_db_rrule_update_record(int id, cal_rrule_s *rrule);
int cal_db_rrule_get_rrule(int id, cal_rrule_s **rrule);
void cal_db_rrule_get_rrule_from_event(calendar_record_h event, cal_rrule_s **rrule);
void cal_db_rrule_set_rrule_to_event(cal_rrule_s *rrule, calendar_record_h event);
void cal_db_rrule_get_rrule_from_todo(calendar_record_h todo, cal_rrule_s **rrule);
void cal_db_rrule_set_rrule_to_todo(cal_rrule_s *rrule, calendar_record_h todo);

#endif  /* __CAL_DB_RRULE_H__ */
