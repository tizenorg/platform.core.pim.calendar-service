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

#ifndef __CALENDAR_SVC_DB_INSTANCE_HELPER_H__
#define __CALENDAR_SVC_DB_INSTANCE_HELPER_H__

int cal_db_instance_normal_insert_record(cal_instance_normal_s *normal, int* id);
int cal_db_instance_allday_insert_record(cal_instance_allday_s *normal, int* id);

#endif // __CALENDAR_SVC_DB_INSTANCE_HELPER_H__
