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
#include "cals-typedef.h"
#include "cals-ical-codec.h"
#include "cals-ical.h"
#include "cals-utils.h"
#include "cals-db.h"
#include "cals-internal.h"
#include "cals-schedule.h"


#define VCALENDAR_HEADER "BEGIN:VCALENDAR"
#define VNOTE_HEADER "BEGIN:VNOTE"

bool cal_convert_cal_data_to_vdata_file(const cal_sch_full_t *sch_array,
	const int sch_count, const char *file_path, int *error_code)
{
	CALS_FN_CALL;
	bool is_success = false;
	char *vcal = NULL;

	cal_vCal_ver_t version = CAL_VCAL_VER_1_0;

	retvm_if(NULL == error_code, false,	"The error_code is NULL");
	if (NULL == sch_array || NULL == file_path) {
		ERR("Invalid ARG:sch_array(%p), file_path(%p)", sch_array, file_path);
		*error_code = CAL_ERR_ARG_INVALID;
		return false;
	}

	// create a vcal file
	FILE* file = fopen(file_path, "w+");
	retex_if(file == NULL, *error_code = CAL_ERR_IO_ERR, "Failed to create file!");

	// encode a sch data
	/*if (sch_array[0].cal_type == CAL_EVENT_MEMO_TYPE)
	  {
	  is_success = _cal_convert_note_to_vnote(sch_array, sch_count, &vcal, version);
	  retex_if(is_success == true, *error_code = CAL_ERR_VOBJECT_FAILED, "_cal_convert_sch_to_vcalendar error!");
	  }
	  else*/
	{
		is_success = _cal_convert_sch_to_vcalendar(sch_array, sch_count, &vcal, version);
		retex_if(is_success != true, *error_code = CAL_ERR_VOBJECT_FAILED, "_cal_convert_sch_to_vcalendar error!");
	}

	retex_if(vcal == NULL, *error_code = CAL_ERR_VOBJECT_FAILED,"cal_convert_cal_data_to_vdata_file vcal is NULL!!");


	CALS_DBG( "\n%s\n", vcal );

	CALS_DBG("------------begint to write to file_path = %s--------------------\n", file_path);

	// write vcal string to file
	if (fputs( vcal, file) == EOF)
	{
		fclose( file );

		CAL_FREE(vcal);

		*error_code = CAL_ERR_IO_ERR;
		return false;
	}

	CALS_DBG("------------success write to file_path = %s--------------------\n", file_path);

	// free buff
	free(vcal);
	fclose( file );

	return true;

CATCH:
	if (file != NULL)
	{
		fclose( file );
	}
	return false;
}

bool cal_convert_vdata_file_to_cal_data(const char *file_path,
	cal_sch_full_t **sch_array, int * sch_count, int *error_code)
{
	char raw_data[VCALENDAR_DATA_LEN + 1];
	bool is_success = false;

	retvm_if(NULL == error_code, false,	"The error_code is NULL");

	if (NULL == file_path || NULL == sch_array || NULL == sch_count) {
		ERR("Invalid ARG:file_path(%p), sch_array(%p), sch_count(%p)",
				file_path, sch_array, sch_count);
		*error_code = CAL_ERR_ARG_INVALID;
		return false;
	}

	memset(raw_data, 0, VCALENDAR_DATA_LEN + 1);

	// write vcal to file
	FILE * file = fopen(file_path, "r");
	retex_if(file == NULL, *error_code = CAL_ERR_IO_ERR, "Failed to open file!");

	// Fix for prevent - B.
	fread(raw_data, 1, (VCALENDAR_DATA_LEN), file);

	fclose( file );

	if (strncmp(raw_data, VCALENDAR_HEADER, strlen(VCALENDAR_HEADER)) == 0)
	{
		// this is a vcalendar stream
		is_success = _cal_convert_vcalendar_to_cal_data(raw_data, sch_array, sch_count);
		retex_if(is_success == false, *error_code = CAL_ERR_VOBJECT_FAILED, "_cal_convert_vcalendar_to_cal_data error!");

	}
	else
	{
		ERR( "\n-------------------------- header error --------- -------------\n");
		return false;
	}
	return true;

CATCH:

	return false;
}

int cal_vcalendar_register_vcs_file(const char * file_name)
{
	int ret, i = 0;
	bool is_success = false;
	cal_sch_full_t * 	sch_array = NULL;
	int sch_count = 0;
	int error_code = 0;
	int index = 0;
	is_success = cal_convert_vdata_file_to_cal_data( file_name, &sch_array, &sch_count, &error_code);
	retex_if(!is_success,,"[ERROR]Falied to convert to cal!\n");

	// get data
	if ((sch_array != NULL) && (sch_count != 0))
	{
		CALS_DBG("\n-------------------------get a sch from vcalendar ---------------------------------\n");

		calendar_svc_connect();

		for ( i = 0; i < sch_count; i++)
		{
			CALS_DBG("\n--------------------------begin to store it in DB--------------------------------\n");
			if (sch_array[i].cal_type == CAL_EVENT_TODO_TYPE)
				ret = cals_insert_schedule(sch_array + i);
			else
				ret = cals_insert_schedule(sch_array + i);
			retvm_if(ret < CAL_SUCCESS, ret, "cals_insert_schedule() Failed(%d)", ret);
			index = ret;

			CALS_DBG("\n--------------------------success store it in DB--------------------------------\n");
		}

	}

	if ( sch_array != NULL )
	{
		cal_db_service_free_full_record(sch_array,&error_code);
		CAL_FREE(sch_array);
	}

	CALS_DBG("-----------------exit-cal_vcalendar_register_vcs_file-------------------\n");
	calendar_svc_close();
	return index;

CATCH:

	calendar_svc_close();

	if ( sch_array != NULL )
	{
		cal_db_service_free_full_record(sch_array,&error_code);
		CAL_FREE(sch_array);
	}

	return -1;
}


