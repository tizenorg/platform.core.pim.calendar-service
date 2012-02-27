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
/**
 * @ingroup app_engine
 * @defgroup app_cal_engine Organizer
 * @{
 */

#ifndef __CALENDAR_SVC_DB_H__
#define __CALENDAR_SVC_DB_H__

#include <glib.h>
#include <sqlite3.h>

#include "cals-typedef.h"

/**
 *  This function initialize schedule record.
 *
 * @return		This function returns initialized record.
 * @param[out]	sch_record	Points the field information for schedule table' s record.
 * @param[out]	error_code	Points the error code.
 * @exception	None.
 */
bool cal_db_service_init_schedule_record(cal_sch_t *sch_record, int *error_code);

/**
 *  This function free full schedule record.
 *
 * @return		This function returns initialized record.
 * @param[in]	sch_full_record	Points the field information for schedule table' s record.
 * @param[out]	error_code	Points the error code.
 * @exception	CAL_ERR_ARG_INVALID.
 */
bool cal_db_service_free_full_record(cal_sch_full_t *sch_full_record, int *error_code);


/**
 *  This function gets the participant info by participant id from the specified table.
 *
 * @return		This function returns true on success, or false on failure.
 * @param[in]		panticipant_index		participant id from org table.
 * @param[out]	record_list		records of this particapant info.
 * @param[out]	error_code	Points the error code.
 * @exception	CAL_ERR_DB_NOT_OPENED, CAL_ERR_ARG_INVALID, CAL_ERR_DB_FAILED
 */
bool cal_db_service_get_participant_info_by_index(const int panticipant_index, GList** record_list, int *error_code);

bool cal_db_service_get_meeting_category_info_by_index(const int event_id, GList** record_list, int *error_code);

/**
 *  This function get record by index base on table type.
 *
 * @return		This function returns true on success, or false on failure.
 * @param[in]		index			specified the index.
 * @param[out]	returned_record	Points of the full field information for schedule table' s record.
 * @param[out]	error_code		Points the error code.
 * @exception	 CAL_ERR_DB_NOT_OPENED, CAL_ERR_DB_FAILED,
 *				CAL_ERR_DB_RECORD_NOT_FOUND, CAL_ERR_DB_FAILED
 */
bool cal_db_service_get_record_full_field_by_index(const int index, cal_sch_full_t *returned_record, int *error_code);

/**
 *  This function adds the participant info into the specified table.
 *
 * @return		This function returns true on success, or false on failure.
 * @param[in]		participant_id		participant id from org table.
 * @param[in]		current_record		records of this particapant info.
 * @param[out]	error_code		Points the error code.
 * @exception	CAL_ERR_DB_NOT_OPENED, CAL_ERR_ARG_INVALID, CAL_ERR_DB_FAILED
 */
int cal_service_add_participant_info(const int participant_id, const cal_participant_info_t* current_record);

int cal_service_add_exception_info(int event_id, cal_exception_info_t *exception_info, const cal_sch_full_t *sch_record);

int cal_db_service_get_month_event (int account_id, time_t startdate, time_t enddate,int is_repeat, sqlite3_stmt** stmt);

int cals_insert_timezone(cal_timezone_t *timezone_info);
int cals_update_timezone(cal_timezone_t *timezone_info);

bool cal_db_service_get_recurrency_exception(const int event_id, GList **exception_list, int *error_code);

int __cal_service_delete_all_records(const int account_id, const cal_event_type_t record_type);

/**
 * @}
 */

#endif /* __CALENDAR_SVC_DB_H__ */
