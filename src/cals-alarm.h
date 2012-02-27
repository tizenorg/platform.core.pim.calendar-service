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
#ifndef __CALENDAR_SVC_ALARM_H__
#define __CALENDAR_SVC_ALARM_H__


enum {
	CALS_ALARM_REMOVE_BY_EVENT_ID,
	CALS_ALARM_REMOVE_BY_CALENDAR_ID,
	CALS_ALARM_REMOVE_BY_ACC_ID,
	CALS_ALARM_REMOVE_ALL,
};
int cals_alarm_remove(int type, int related_id);
int cals_alarm_add(int event_id, cal_alarm_info_t *alarm_info, struct tm *start_date_time);
int cals_alarm_get_event_id(int alarm_id);
int cals_get_alarm_info(const int event_id, GList **alarm_list);


#endif /* __CALENDAR_SVC_ALARM_H__ */
