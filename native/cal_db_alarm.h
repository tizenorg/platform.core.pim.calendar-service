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

#ifndef __CALENDAR_SVC_DB_ALARM_H__
#define __CALENDAR_SVC_DB_ALARM_H__

int _cal_db_alarm_get_records(int event_id, GList **out_list);
int _cal_db_alarm_convert_gtoh(GList *glist, int id, calendar_list_h *hlist);
int _cal_db_alarm_delete_with_id(int event_id);
/*
 * @param[in] list   The alarm list
 *
 * @return 0 on none alarm, 1 on alarm.
 */
int _cal_db_alarm_has_alarm(GList *list);

#endif  //__CALENDAR_SVC_DB_ALARM_H__
