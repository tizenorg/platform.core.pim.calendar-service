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

#ifndef __CAL_DB_PLUGIN_TIMEZONE_HELPER_H__
#define __CAL_DB_PLUGIN_TIMEZONE_HELPER_H__

void cal_db_timezone_search_with_tzid(int book_id, char *tzid, int *timezone_id);
void cal_db_timezone_get_offset(int book_id, char *tzid, int *offset);

#endif  /* __CAL_DB_PLUGIN_TIMEZONE_HELPER_H__ */
