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

#ifndef __CAL_DB_PLUGIN_EVENT_HELPER_H__
#define __CAL_DB_PLUGIN_EVENT_HELPER_H__

int cal_db_event_update_original_event_version(int original_event_id, int version);
int cal_db_event_check_value_validation(cal_event_s *event);
int cal_db_event_insert_record(calendar_record_h record, int original_event_id, int *id);
int cal_db_event_insert_records(cal_list_s *list_s, int original_event_id);
GList *cal_db_event_get_list_with_uid(char *uid, int parent_id);
void cal_db_event_update_child_origina_event_id(int child_id, int parent_id);
char *cal_db_event_get_recurrence_id_from_exception(int child_id);
void cal_db_event_apply_recurrence_id(int parent_id, cal_event_s *event, char *recurrence_id, int child_id);

#endif /* __CAL_DB_PLUGIN_EVENT_HELPER_H__ */
