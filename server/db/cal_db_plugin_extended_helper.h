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

#ifndef __CAL_DB_PLUGIN_EXTENDED_HELPER_H__
#define __CAL_DB_PLUGIN_EXTENDED_HELPER_H__

int cal_db_extended_get_records(int record_id, calendar_record_type_e record_type, cal_list_s *list);
int cal_db_extended_delete_with_id(int record_id, calendar_record_type_e record_type);
int cal_db_extended_insert_record(calendar_record_h record, int record_id, calendar_record_type_e record_type, int *id);
int cal_db_extended_insert_records(cal_list_s *list_s, int record_id, calendar_record_type_e record_type);

#endif  /* __CAL_DB_PLUGIN_EXTENDED_HELPER_H__ */
