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
#ifndef __CALENDAR_SVC_SCHEDULE_H__
#define __CALENDAR_SVC_SCHEDULE_H__

/**
 *  This function add full field of calendar info to DB.
 *
 * @return		This function returns true on success, or false on failure.
 * @param[in]	sch_record		Points of the full field information for schedule table' s record.
 * @exception	CAL_ERR_DB_FAILED, CAL_ERR_ARG_INVALID, CAL_ERR_DB_NOT_OPENED,
 *				CAL_ERR_DB_RECORD_NOT_FOUND, CAL_ERR_DB_FAILED
 */
int cals_insert_schedule(cal_sch_full_t *sch_record);
int cals_update_schedule(const int index, cal_sch_full_t *sch_record);
int cals_delete_schedule(const int index);

int cals_rearrage_schedule_field(const char *src, char *dest, int dest_size);

int cals_stmt_get_filted_schedule(sqlite3_stmt *stmt,cal_sch_full_t *sch_record, const char *select_field);
void cals_stmt_get_full_schedule(sqlite3_stmt *stmt,cal_sch_full_t *sch_record, bool is_utc);


#endif /* __CALENDAR_SVC_SCHEDULE_H__ */


