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
#ifndef __CALENDAR_SVC_CALENDAR_H__
#define __CALENDAR_SVC_CALENDAR_H__

int cals_insert_calendar(const calendar_t *calendar);
int cals_update_calendar(const calendar_t *calendar_info);

int cals_delete_calendars(int account_id);
int cals_delete_calendar(int index);

int cals_rearrage_calendar_field(const char *src, char *dest, int dest_size);

void cals_stmt_get_calendar(sqlite3_stmt *stmt, calendar_t *calendar_record);
int cals_stmt_get_filted_calendar(sqlite3_stmt *stmt, calendar_t *calendar_record, const char *select_field);

#endif /* __CALENDAR_SVC_CALENDAR_H__ */

