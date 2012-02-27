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
#ifndef __CALENDAR_SVC_ICAL_H__
#define __CALENDAR_SVC_ICAL_H__
/**
* @ingroup app_engine
* @addtogroup	app_cal_engine
* @{
*/

#define TM_YEAR_BASE 1900
#define VCALENDAR_TIME_STR_LEN 16
#define VCALENDAR_DATA_LEN 3000

#define SCH_LIST_LEN 10

//#include "org-engine-typedef.h"

/**
* @fn bool cal_convert_cal_data_to_vdata_file( const cal_sch_full_t * sch_array, const int sch_count, const char * file_path, int * error_code);
*  This function convert a record to vcal file.
*
* @return		This function returns true on success, or false on failure.
* @param[in]	sch_array			Points the field information for schedule table' s record.
* @param[in]	sch_count			Points the count of records.
* @param[in]	file_path	Points the file path.
* @param[out]	error_code	Points the error code.
* @exception	#CAL_ERR_VOBJECT_FAILED - Encode vcal error.
* @exception	#CAL_ERR_FILE_CREATE_ERROR - Create file error.
* @exception	#CAL_ERR_FILE_WRITINGNG_ERROR - Write file error.
*/
bool cal_convert_cal_data_to_vdata_file( const cal_sch_full_t * sch_array, const int sch_count, const char * file_path, int * error_code);

/**
* @fn bool cal_convert_vdata_file_to_cal_data(const char * file_path, cal_sch_full_t ** sch_array, int * sch_count, int* error_code );
*  This function convert a record to vcal file.
*
* @return		This function returns true on success, or false on failure.
* @param[in]		file_path	Points the file path.
* @param[out]	sch_array		Points the field information for schedule table' s record.
* @param[out]	sch_count			Points the count of records.
* @param[out]	error_code	Points the error code.
* @exception	#CAL_ERR_FILE_OPEN_ERROR - Open file error.
* @exception	#CAL_ERR_VOBJECT_FAILED - Decode vcal error.
*/
bool cal_convert_vdata_file_to_cal_data(const char * file_path, cal_sch_full_t ** sch_array, int * sch_count, int *error_code );

/**
* @fn int cal_vcalendar_register_vcs_file(const char * file_name);
*  This function make vcal file by record index.
*
* @return		This function returns true on success, or false on failure.
* @param[in]	file_name	        vcalendar's file name
* @exception	#CAL_ERR_FILE_OPEN_ERROR - Open file error.
* @exception	#CAL_ERR_VOBJECT_FAILED - Decode vcal error.
* @exception    #CAL_ERR_DB_NOT_OPENED  - org db not opended.
*/
int cal_vcalendar_register_vcs_file(const char * file_name);

/**
* @}
*/
#endif	/* __CALENDAR_SVC_ICAL_H__ */
