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

#include <stdlib.h> //calloc
#include <pims-ipc.h>
#include <glib-object.h> //g_type_init
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "calendar.h"
#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_inotify.h"
#include "cal_view.h"
#include "cal_record.h"
#include "cal_list.h"
#include "cal_mutex.h"
#include "cal_ipc.h"
#include "cal_ipc_marshal.h"
#include "cal_client_ipc.h"

#define __CAL_CLIENT_ACCESS_MAX 10
#define __CAL_CLIENT_ALLOW_USEC 25000

#define CAL_LIMIT_ACCESS_FRONT(uri) \
	int is_schedule = 0; \
do { \
	if (!strncmp(uri, CALENDAR_VIEW_EVENT, strlen(CALENDAR_VIEW_EVENT))) \
	{ \
		is_schedule = 1; \
		struct timeval hold = {0}; \
		struct timeval diff = {0}; \
		gettimeofday(&hold, NULL); \
		timersub(&hold, &__g_release_time, &diff); \
		DBG("%ld.%ld sec", diff.tv_sec, diff.tv_usec); \
		if (diff.tv_sec / 1000 == 0 && diff.tv_usec < __CAL_CLIENT_ALLOW_USEC) \
		{ \
			if (__g_access_count < __CAL_CLIENT_ACCESS_MAX) \
			{ \
				__g_access_count++; \
				DBG("--count (%d)", __g_access_count); \
			} \
			else \
			{ \
				DBG("--sleep"); \
				usleep(200000); \
				__g_access_count = 0; \
				timerclear(&__g_release_time); \
			} \
		} \
		else \
		{ \
			DBG("--reset"); \
			__g_access_count = 0; \
			timerclear(&__g_release_time); \
		} \
	} \
} while(0)

#define CAL_LIMIT_ACCESS_BACK \
	do { \
		if (is_schedule) \
		{ \
			gettimeofday(&__g_release_time, NULL); \
		} \
	} while(0)


static int __g_access_count;
static struct timeval __g_release_time;

int cal_client_db_insert_record(calendar_record_h record, int* id)
{
	int ret = CALENDAR_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(NULL == record, CALENDAR_ERROR_INVALID_PARAMETER);

	CAL_RECORD_RESET_COMMON((cal_record_s*)record);

	cal_record_s *rec = (cal_record_s *)record;
	CAL_LIMIT_ACCESS_FRONT(rec->view_uri);

	// make indata
	indata = pims_ipc_data_create(0);
	if (indata == NULL) {
		ERR("ipc data created fail !");
		return CALENDAR_ERROR_OUT_OF_MEMORY;
	}

	ret = cal_ipc_marshal_record(record, indata);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_record() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
	}

	// ipc call
	ret = cal_client_ipc_call(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_INSERT_RECORD, indata, &outdata);
	pims_ipc_data_destroy(indata);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("pims_ipc_call Fail(%d)", ret);
		return CALENDAR_ERROR_IPC;
	}
	if (NULL == outdata) {
		ERR("outdata is NULL");
		return CALENDAR_ERROR_IPC;
	}

	// check outdata
	unsigned int size = 0;
	void *ret_pims = NULL;
	ret_pims = pims_ipc_data_get(outdata, &size);
	if (NULL == ret_pims) {
		ERR("pims_ipc_data_get() Fail");
		pims_ipc_data_destroy(outdata);
		CAL_LIMIT_ACCESS_BACK;
		return CALENDAR_ERROR_IPC;
	}
	ret = *(int*)ret_pims;

	if (CALENDAR_ERROR_NONE != ret) {
		ERR("pims_ipc_data_get() Fail(%d)", ret);
		pims_ipc_data_destroy(outdata);
		CAL_LIMIT_ACCESS_BACK;
		return ret;
	}

	int transaction_ver = 0;
	ret_pims = pims_ipc_data_get(outdata, &size);
	if (NULL == ret_pims) {
		ERR("pims_ipc_data_get() Fail");
		pims_ipc_data_destroy(outdata);
		CAL_LIMIT_ACCESS_BACK;
		return CALENDAR_ERROR_IPC;
	}
	transaction_ver = *(int*)ret_pims;
	cal_client_ipc_set_change_version(transaction_ver);

	int out_id = 0;
	ret_pims = pims_ipc_data_get(outdata, &size);
	if (NULL == ret_pims) {
		ERR("pims_ipc_data_get() Fail");
		pims_ipc_data_destroy(outdata);
		CAL_LIMIT_ACCESS_BACK;
		return CALENDAR_ERROR_IPC;
	}
	out_id = *(int*)ret_pims;

	if (id)
		*id = out_id;

	pims_ipc_data_destroy(outdata);
	CAL_LIMIT_ACCESS_BACK;
	return ret;
}

int cal_client_db_update_record(calendar_record_h record)
{
	int ret = CALENDAR_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(NULL == record, CALENDAR_ERROR_INVALID_PARAMETER);

	CAL_LIMIT_ACCESS_FRONT(((cal_record_s *)record)->view_uri);

	// make indata
	indata = pims_ipc_data_create(0);
	RETVM_IF(NULL == indata, CALENDAR_ERROR_OUT_OF_MEMORY, "ipc data created Fail !");

	ret = cal_ipc_marshal_record(record,indata);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_char() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
	}

	// ipc call
	ret = cal_client_ipc_call(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_UPDATE_RECORD, indata, &outdata);
	pims_ipc_data_destroy(indata);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("pims_ipc_call Fail(%d)", ret);
		return CALENDAR_ERROR_IPC;
	}
	if (NULL == outdata) {
		ERR("outdata is NULL");
		return CALENDAR_ERROR_IPC;
	}

	// check outdata
	unsigned int size = 0;
	void *ret_pims = NULL;
	ret_pims = pims_ipc_data_get(outdata, &size);
	if (NULL == ret_pims) {
		ERR("pims_ipc_data_get() Fail");
		pims_ipc_data_destroy(outdata);
		CAL_LIMIT_ACCESS_BACK;
		return CALENDAR_ERROR_IPC;
	}
	ret = *(int*)ret_pims;

	if (CALENDAR_ERROR_NONE != ret) {
		ERR("pims_ipc_data_get() Fail(%d)", ret);
		pims_ipc_data_destroy(outdata);
		CAL_LIMIT_ACCESS_BACK;
		return ret;
	}

	int transaction_ver = 0;
	ret_pims = pims_ipc_data_get(outdata, &size);
	if (NULL == ret_pims) {
		ERR("pims_ipc_data_get() Fail");
		pims_ipc_data_destroy(outdata);
		CAL_LIMIT_ACCESS_BACK;
		return CALENDAR_ERROR_IPC;
	}
	transaction_ver = *(int*)ret_pims;
	cal_client_ipc_set_change_version(transaction_ver);

	pims_ipc_data_destroy(outdata);
	CAL_LIMIT_ACCESS_BACK;
	return ret;
}

int cal_client_db_delete_record(const char* view_uri, int id)
{
	int ret = CALENDAR_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(NULL == view_uri, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(id <= 0, CALENDAR_ERROR_INVALID_PARAMETER,"id(%d) <= 0", id);

	CAL_LIMIT_ACCESS_FRONT(view_uri);

	// make indata
	indata = pims_ipc_data_create(0);
	RETVM_IF(NULL == indata, CALENDAR_ERROR_OUT_OF_MEMORY, "ipc data created Fail !");

	ret = cal_ipc_marshal_char(view_uri,indata);
	if (CALENDAR_ERROR_NONE != ret) {
        ERR("cal_ipc_marshal_char() Fail(%d)", ret);
        pims_ipc_data_destroy(indata);
		return ret;
	}
	ret = cal_ipc_marshal_int(id,indata);
	if (CALENDAR_ERROR_NONE != ret) {
        ERR("cal_ipc_marshal_char() Fail(%d)", ret);
        pims_ipc_data_destroy(indata);
		return ret;
	}

	// ipc call
	ret = cal_client_ipc_call(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_DELETE_RECORD, indata, &outdata);
	pims_ipc_data_destroy(indata);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("pims_ipc_call Fail(%d)", ret);
		return CALENDAR_ERROR_IPC;
	}
	if (NULL == outdata) {
		ERR("outdata is NULL");
		return CALENDAR_ERROR_IPC;
	}

	// check outdata
	unsigned int size = 0;
	void *ret_pims = NULL;
	ret_pims = pims_ipc_data_get(outdata, &size);
	if (NULL == ret_pims) {
		ERR("pims_ipc_data_get() Fail");
		pims_ipc_data_destroy(outdata);
		CAL_LIMIT_ACCESS_BACK;
		return CALENDAR_ERROR_IPC;
	}
	ret = *(int*)ret_pims;

	if (CALENDAR_ERROR_NONE != ret) {
		ERR("pims_ipc_data_get() Fail(%d)", ret);
		pims_ipc_data_destroy(outdata);
		CAL_LIMIT_ACCESS_BACK;
		return ret;
	}

	int transaction_ver = 0;
	ret_pims = pims_ipc_data_get(outdata, &size);
	if (NULL == ret_pims) {
		ERR("pims_ipc_data_get() Fail");
		pims_ipc_data_destroy(outdata);
		CAL_LIMIT_ACCESS_BACK;
		return CALENDAR_ERROR_IPC;
	}
	transaction_ver = *(int*)ret_pims;
	cal_client_ipc_set_change_version(transaction_ver);

	pims_ipc_data_destroy(outdata);
	CAL_LIMIT_ACCESS_BACK;
	return ret;
}

int cal_client_db_replace_record(calendar_record_h record, int record_id)
{
	int ret = CALENDAR_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(NULL == record, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(record_id < 0, CALENDAR_ERROR_INVALID_PARAMETER, "record_id(%d) < 0", record_id);

	CAL_LIMIT_ACCESS_FRONT(((cal_record_s *)record)->view_uri);

	// make indata
	indata = pims_ipc_data_create(0);
    RETVM_IF(indata == NULL, CALENDAR_ERROR_OUT_OF_MEMORY, "pims_ipc_data_create() Fail");

	ret = cal_ipc_marshal_record(record,indata);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_char() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
	}
	ret = cal_ipc_marshal_int(record_id,indata);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_char() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
	}

	// ipc call
	ret = cal_client_ipc_call(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_REPLACE_RECORD, indata, &outdata);
	pims_ipc_data_destroy(indata);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("pims_ipc_call Fail(%d)", ret);
		return CALENDAR_ERROR_IPC;
	}
	if (NULL == outdata) {
		ERR("outdata is NULL");
		return CALENDAR_ERROR_IPC;
	}

	// check outdata
	unsigned int size = 0;
	void *ret_pims = NULL;
	ret_pims = pims_ipc_data_get(outdata, &size);
	if (NULL == ret_pims) {
		ERR("pims_ipc_data_get() Fail");
		ret = CALENDAR_ERROR_IPC;
		pims_ipc_data_destroy(outdata);
		CAL_LIMIT_ACCESS_BACK;
		return CALENDAR_ERROR_IPC;
	}
	ret = *(int*)ret_pims;

	if (CALENDAR_ERROR_NONE != ret) {
		ERR("pims_ipc_data_get() Fail(%d)", ret);
		pims_ipc_data_destroy(outdata);
		CAL_LIMIT_ACCESS_BACK;
		return ret;
	}

	int transaction_ver = 0;
	ret_pims = pims_ipc_data_get(outdata, &size);
	if (NULL == ret_pims) {
		ERR("pims_ipc_data_get() Fail");
		pims_ipc_data_destroy(outdata);
		CAL_LIMIT_ACCESS_BACK;
		return CALENDAR_ERROR_IPC;
	}
	transaction_ver = *(int*)ret_pims;
	cal_client_ipc_set_change_version(transaction_ver);

	pims_ipc_data_destroy(outdata);
	CAL_LIMIT_ACCESS_BACK;
	return ret;
}

int cal_client_db_get_record( const char* view_uri, int id, calendar_record_h* out_record )
{
	int ret = CALENDAR_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(NULL == view_uri, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_record, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(id <= 0, CALENDAR_ERROR_INVALID_PARAMETER, "id(%d) <= 0", id);

	// make indata
	indata = pims_ipc_data_create(0);
    RETVM_IF(NULL == indata, CALENDAR_ERROR_OUT_OF_MEMORY, "pims_ipc_data_create() Fail");

	ret = cal_ipc_marshal_char(view_uri,indata);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_char() Fail(%d)", ret);
        pims_ipc_data_destroy(indata);
		return ret;
	}
	ret = cal_ipc_marshal_int(id,indata);
	if (CALENDAR_ERROR_NONE != ret) {
        ERR("cal_ipc_marshal_int() Fail(%d)", ret);
        pims_ipc_data_destroy(indata);
		return ret;
	}

	// ipc call
	ret = cal_client_ipc_call(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_GET_RECORD, indata, &outdata);
	pims_ipc_data_destroy(indata);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("pims_ipc_call Fail(%d)", ret);
		return CALENDAR_ERROR_IPC;
	}
	if (NULL == outdata) {
		ERR("outdata is NULL");
		return CALENDAR_ERROR_IPC;
	}

	// check outdata
	unsigned int size = 0;
	void *ret_pims = NULL;
	ret_pims = pims_ipc_data_get(outdata, &size);
	if (NULL == ret_pims) {
		ERR("pims_ipc_data_get() Fail");
		pims_ipc_data_destroy(outdata);
		return CALENDAR_ERROR_IPC;
	}
	ret = *(int*)ret_pims;

	if (CALENDAR_ERROR_NONE != ret) {
		ERR("pims_ipc_data_get() Fail(%d)", ret);
		pims_ipc_data_destroy(outdata);
		return ret;
	}

	ret = cal_ipc_unmarshal_record(outdata, out_record);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_record() Fail(%d)", ret);
		pims_ipc_data_destroy(outdata);
		return ret;
	}

	pims_ipc_data_destroy(outdata);
	return ret;
}

int cal_client_db_get_all_records( const char* view_uri, int offset, int limit, calendar_list_h* out_list )
{
	int ret = CALENDAR_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(NULL == out_list, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == view_uri, CALENDAR_ERROR_INVALID_PARAMETER);

	// make indata
	indata = pims_ipc_data_create(0);
	RETVM_IF(NULL == indata, CALENDAR_ERROR_OUT_OF_MEMORY, "ipc data created Fail !");

	ret = cal_ipc_marshal_char(view_uri,indata);
    if (CALENDAR_ERROR_NONE != ret) {
        ERR("cal_ipc_marshal_char() Fail(%d)", ret);
        pims_ipc_data_destroy(indata);
        return ret;
    }
    ret = cal_ipc_marshal_int(offset,indata);
    if (CALENDAR_ERROR_NONE != ret) {
        ERR("cal_ipc_marshal_char() Fail(%d)", ret);
        pims_ipc_data_destroy(indata);
        return ret;
    }
    ret = cal_ipc_marshal_int(limit,indata);
    if (CALENDAR_ERROR_NONE != ret) {
        ERR("cal_ipc_marshal_char() Fail(%d)", ret);
        pims_ipc_data_destroy(indata);
        return ret;
    }

	// ipc call
	ret = cal_client_ipc_call(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_GET_ALL_RECORDS, indata, &outdata);
	pims_ipc_data_destroy(indata);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("pims_ipc_call Fail(%d)", ret);
		return CALENDAR_ERROR_IPC;
	}
	if (NULL == outdata) {
		ERR("outdata is NULL");
		return CALENDAR_ERROR_IPC;
	}

	// check outdata
	unsigned int size = 0;
	void *ret_pims = NULL;
	ret_pims = pims_ipc_data_get(outdata, &size);
	if (NULL == ret_pims) {
		ERR("pims_ipc_data_get() Fail");
		pims_ipc_data_destroy(outdata);
		return CALENDAR_ERROR_IPC;
	}
	ret = *(int*)ret_pims;

	if (CALENDAR_ERROR_NONE != ret) {
		ERR("pims_ipc_data_get() Fail(%d)", ret);
		pims_ipc_data_destroy(outdata);
		return ret;
	}
	ret = cal_ipc_unmarshal_list(outdata,out_list);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_list() Fail(%d)", ret);
		pims_ipc_data_destroy(outdata);
		return ret;
	}

	pims_ipc_data_destroy(outdata);
	return ret;
}

int cal_client_db_get_records_with_query( calendar_query_h query, int offset, int limit, calendar_list_h* out_list )
{
	int ret = CALENDAR_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(NULL == query, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_list, CALENDAR_ERROR_INVALID_PARAMETER);

	// make indata
	indata = pims_ipc_data_create(0);
	RETVM_IF(NULL == indata, CALENDAR_ERROR_OUT_OF_MEMORY, "ipc data created Fail !");

	ret = cal_ipc_marshal_query(query,indata);
    if (CALENDAR_ERROR_NONE != ret) {
        ERR("cal_ipc_marshal_query() Fail(%d)", ret);
        pims_ipc_data_destroy(indata);
        return ret;
    }
    ret = cal_ipc_marshal_int(offset,indata);
    if (CALENDAR_ERROR_NONE != ret) {
        ERR("cal_ipc_marshal_int() Fail(%d)", ret);
        pims_ipc_data_destroy(indata);
        return ret;
    }
    ret = cal_ipc_marshal_int(limit,indata);
    if (CALENDAR_ERROR_NONE != ret) {
        ERR("cal_ipc_marshal_int() Fail(%d)", ret);
        pims_ipc_data_destroy(indata);
        return ret;
    }

	// ipc call
	ret = cal_client_ipc_call(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_GET_RECORDS_WITH_QUERY, indata, &outdata);
	pims_ipc_data_destroy(indata);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("pims_ipc_call Fail(%d)", ret);
		return CALENDAR_ERROR_IPC;
	}
	if (NULL == outdata) {
		ERR("outdata is NULL");
		return CALENDAR_ERROR_IPC;
	}

	// check outdata
	unsigned int size = 0;
	void *ret_pims = NULL;
	ret_pims = pims_ipc_data_get(outdata, &size);
	if (NULL == ret_pims) {
		ERR("pims_ipc_data_get() Fail");
		pims_ipc_data_destroy(outdata);
		return CALENDAR_ERROR_IPC;
	}
	ret = *(int*)ret_pims;

	if (CALENDAR_ERROR_NONE != ret) {
		ERR("pims_ipc_data_get() Fail(%d)", ret);
		pims_ipc_data_destroy(outdata);
		return ret;
	}

	ret = cal_ipc_unmarshal_list(outdata,out_list);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_list() Fail(%d)", ret);
		pims_ipc_data_destroy(outdata);
		return ret;
	}

	pims_ipc_data_destroy(outdata);
	return ret;
}

int cal_client_db_clean_after_sync(int book_id, int calendar_db_version)
{
	int ret = CALENDAR_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETVM_IF(book_id <= 0, CALENDAR_ERROR_INVALID_PARAMETER, "book_id(%d) < 0", book_id);

	// make indata
	indata = pims_ipc_data_create(0);
	RETVM_IF(NULL == indata, CALENDAR_ERROR_OUT_OF_MEMORY, "ipc data created Fail !");

	ret = cal_ipc_marshal_int(book_id,indata);
    if (CALENDAR_ERROR_NONE != ret) {
        ERR("cal_ipc_marshal_char() Fail(%d)", ret);
        pims_ipc_data_destroy(indata);
        return ret;
    }
    ret = cal_ipc_marshal_int(calendar_db_version,indata);
    if (CALENDAR_ERROR_NONE != ret) {
        ERR("cal_ipc_marshal_char() Fail(%d)", ret);
        pims_ipc_data_destroy(indata);
        return ret;
    }

	// ipc call
	ret = cal_client_ipc_call(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_CLEAN_AFTER_SYNC, indata, &outdata);
	pims_ipc_data_destroy(indata);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("pims_ipc_call Fail(%d)", ret);
		return CALENDAR_ERROR_IPC;
	}
	if (NULL == outdata) {
		ERR("outdata is NULL");
		return CALENDAR_ERROR_IPC;
	}

	// check outdata
	unsigned int size = 0;
	void *ret_pims = NULL;
	ret_pims = pims_ipc_data_get(outdata, &size);
	if (NULL == ret_pims) {
		ERR("pims_ipc_data_get() Fail");
		pims_ipc_data_destroy(outdata);
		return CALENDAR_ERROR_IPC;
	}
	ret = *(int*)ret_pims;

	if (CALENDAR_ERROR_NONE != ret) {
		ERR("pims_ipc_data_get() Fail(%d)", ret);
		pims_ipc_data_destroy(outdata);
		return ret;
	}

	pims_ipc_data_destroy(outdata);
	return ret;
}

int cal_client_db_get_count( const char* view_uri, int *out_count )
{
	int ret = CALENDAR_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(NULL == view_uri, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_count, CALENDAR_ERROR_INVALID_PARAMETER);

	// make indata
	indata = pims_ipc_data_create(0);
	RETVM_IF(NULL == indata, CALENDAR_ERROR_OUT_OF_MEMORY, "ipc data created Fail !");

	ret = cal_ipc_marshal_char(view_uri,indata);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_char() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
	}

	// ipc call
	ret = cal_client_ipc_call(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_GET_COUNT, indata, &outdata);
	pims_ipc_data_destroy(indata);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("pims_ipc_call Fail(%d)", ret);
		return CALENDAR_ERROR_IPC;
	}
	if (NULL == outdata) {
		ERR("outdata is NULL");
		return CALENDAR_ERROR_IPC;
	}

	// check outdata
	unsigned int size = 0;
	void *ret_pims = NULL;
	ret_pims = pims_ipc_data_get(outdata, &size);
	if (NULL == ret_pims) {
		ERR("pims_ipc_data_get() Fail");
		pims_ipc_data_destroy(outdata);
		return CALENDAR_ERROR_IPC;
	}
	ret = *(int*)ret_pims;

	if (CALENDAR_ERROR_NONE != ret) {
		ERR("pims_ipc_data_get() Fail(%d)", ret);
		pims_ipc_data_destroy(outdata);
		return ret;
	}

	ret = cal_ipc_unmarshal_int(outdata,out_count);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		pims_ipc_data_destroy(outdata);
		return ret;
	}

	pims_ipc_data_destroy(outdata);
	return ret;
}

int cal_client_db_get_count_with_query( calendar_query_h query, int *out_count )
{
	int ret = CALENDAR_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(NULL == query, CALENDAR_ERROR_INVALID_PARAMETER);

	// make indata
	indata = pims_ipc_data_create(0);
	RETVM_IF(NULL == indata, CALENDAR_ERROR_OUT_OF_MEMORY, "ipc data created Fail !");

	ret = cal_ipc_marshal_query(query,indata);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_char() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
	}

	// ipc call
	ret = cal_client_ipc_call(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_GET_COUNT_WITH_QUERY, indata, &outdata);
	pims_ipc_data_destroy(indata);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("pims_ipc_call Fail(%d)", ret);
		return CALENDAR_ERROR_IPC;
	}
	if (NULL == outdata) {
		ERR("outdata is NULL");
		return CALENDAR_ERROR_IPC;
	}

	// check outdata
	unsigned int size = 0;
	void *ret_pims = NULL;
	ret_pims = pims_ipc_data_get(outdata, &size);
	if (NULL == ret_pims) {
		ERR("pims_ipc_data_get() Fail");
		pims_ipc_data_destroy(outdata);
		return CALENDAR_ERROR_IPC;
	}
	ret = *(int*)ret_pims;

	if (CALENDAR_ERROR_NONE != ret) {
		ERR("pims_ipc_data_get() Fail(%d)", ret);
		pims_ipc_data_destroy(outdata);
		return ret;
	}

	ret = cal_ipc_unmarshal_int(outdata, out_count);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		pims_ipc_data_destroy(outdata);
		return ret;
	}

	pims_ipc_data_destroy(outdata);
	return ret;
}

int cal_client_db_insert_records( calendar_list_h record_list, int** record_id_array, int* count)
{
	int ret = CALENDAR_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(NULL == record_list, CALENDAR_ERROR_INVALID_PARAMETER);

	// make indata
	indata = pims_ipc_data_create(0);
	RETVM_IF(NULL == indata, CALENDAR_ERROR_OUT_OF_MEMORY, "ipc data created Fail !");

	ret = cal_ipc_marshal_list(record_list,indata);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_list() Fail(%d)", ret);
        pims_ipc_data_destroy(indata);
        return ret;
    }

	// ipc call
	ret = cal_client_ipc_call(CAL_IPC_MODULE,CAL_IPC_SERVER_DB_INSERT_RECORDS, indata,&outdata);
	pims_ipc_data_destroy(indata);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("pims_ipc_call Fail(%d)", ret);
		return CALENDAR_ERROR_IPC;
	}
	if (NULL == outdata) {
		ERR("outdata is NULL");
		return CALENDAR_ERROR_IPC;
	}

	// check outdata
	unsigned int size = 0;
	void *ret_pims = NULL;
	ret_pims = pims_ipc_data_get(outdata, &size);
	if (NULL == ret_pims) {
		ERR("pims_ipc_data_get() Fail");
		pims_ipc_data_destroy(outdata);
		return CALENDAR_ERROR_IPC;
	}
	ret = *(int*)ret_pims;

	if (CALENDAR_ERROR_NONE != ret) {
		ERR("pims_ipc_data_get() Fail(%d)", ret);
		pims_ipc_data_destroy(outdata);
		return ret;
	}

	int transaction_ver = 0;
	ret_pims = pims_ipc_data_get(outdata, &size);
	if (NULL == ret_pims) {
		ERR("pims_ipc_data_get() Fail");
		pims_ipc_data_destroy(outdata);
		return CALENDAR_ERROR_IPC;
	}
	transaction_ver = *(int*)ret_pims;
	cal_client_ipc_set_change_version(transaction_ver);

	if (NULL == count || NULL == record_id_array) {
		pims_ipc_data_destroy(outdata);
		return CALENDAR_ERROR_NONE;
	}

	*count = *(int *)pims_ipc_data_get(outdata, &size);
	if (*count <= 0) {
		ERR("count(%d) <= 0", *count);
		*count = 0;
		pims_ipc_data_destroy(outdata);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	int *ids = NULL;
	ids = calloc(1, sizeof(int)*(*count));
	if(NULL == ids) {
		ERR("calloc() Fail");
		*count = 0;
		pims_ipc_data_destroy(outdata);
		return CALENDAR_ERROR_OUT_OF_MEMORY;
	}

	int i;
	for(i = 0; i < (*count); i++) {
		ret_pims = pims_ipc_data_get(outdata, &size);
		if (NULL == ret_pims) {
			ERR("pims_ipc_data_get() Fail");
			free(ids);
			pims_ipc_data_destroy(outdata);
			return CALENDAR_ERROR_IPC;
		}
		ids[i] = *(int*)ret_pims;
	}
	*record_id_array = ids;

	pims_ipc_data_destroy(outdata);
	return ret;
}

int cal_client_db_update_records(calendar_list_h record_list)
{
	int ret = CALENDAR_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(NULL == record_list, CALENDAR_ERROR_INVALID_PARAMETER);

	// make indata
	indata = pims_ipc_data_create(0);
	RETVM_IF(NULL == indata, CALENDAR_ERROR_OUT_OF_MEMORY, "ipc data created Fail !");

	ret = cal_ipc_marshal_list(record_list,indata);
	if (CALENDAR_ERROR_NONE != ret) {
        ERR("cal_ipc_marshal_char() Fail(%d)", ret);
        pims_ipc_data_destroy(indata);
        return ret;
    }

	// ipc call
	ret = cal_client_ipc_call(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_UPDATE_RECORDS, indata, &outdata);
	pims_ipc_data_destroy(indata);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("pims_ipc_call Fail(%d)", ret);
		return CALENDAR_ERROR_IPC;
	}
	if (NULL == outdata) {
		ERR("outdata is NULL");
		return CALENDAR_ERROR_IPC;
	}

	// check outdata
	unsigned int size = 0;
	void *ret_pims = NULL;
	ret_pims = pims_ipc_data_get(outdata, &size);
	if (NULL == ret_pims) {
		ERR("pims_ipc_data_get() Fail");
		pims_ipc_data_destroy(outdata);
		return CALENDAR_ERROR_IPC;
	}
	ret = *(int*)ret_pims;

	if (CALENDAR_ERROR_NONE != ret) {
		ERR("pims_ipc_data_get() Fail(%d)", ret);
		pims_ipc_data_destroy(outdata);
		return ret;
	}

	int transaction_ver = 0;
	ret_pims = pims_ipc_data_get(outdata, &size);
	if (NULL == ret_pims) {
		ERR("pims_ipc_data_get() Fail");
		pims_ipc_data_destroy(outdata);
		return CALENDAR_ERROR_IPC;
	}
	transaction_ver = *(int*)ret_pims;
	cal_client_ipc_set_change_version(transaction_ver);

	pims_ipc_data_destroy(outdata);
	return ret;

}

int cal_client_db_delete_records(const char* view_uri, int record_id_array[], int count)
{
	int ret = CALENDAR_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;
	int i = 0;

	RETV_IF(NULL == view_uri, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == record_id_array, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(count <= 0, CALENDAR_ERROR_INVALID_PARAMETER, "count(%d) < 0", count);

	// make indata
	indata = pims_ipc_data_create(0);
	RETVM_IF(NULL == indata, CALENDAR_ERROR_OUT_OF_MEMORY, "ipc data created Fail !");

	ret = cal_ipc_marshal_char(view_uri,indata);
    if (CALENDAR_ERROR_NONE != ret) {
        ERR("cal_ipc_marshal_char() Fail(%d)", ret);
        pims_ipc_data_destroy(indata);
        return ret;
    }
    ret = cal_ipc_marshal_int(count,indata);
    if (CALENDAR_ERROR_NONE != ret) {
        ERR("cal_ipc_marshal_char() Fail(%d)", ret);
        pims_ipc_data_destroy(indata);
        return ret;
    }
    for (i = 0; i < count; i++) {
        ret = cal_ipc_marshal_int(record_id_array[i],indata);
        if (CALENDAR_ERROR_NONE != ret) {
            ERR("cal_ipc_marshal_char() Fail(%d)", ret);
            pims_ipc_data_destroy(indata);
            return ret;
        }
    }

	// ipc call
	ret = cal_client_ipc_call(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_DELETE_RECORDS, indata, &outdata);
	pims_ipc_data_destroy(indata);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("pims_ipc_call Fail(%d)", ret);
		return CALENDAR_ERROR_IPC;
	}
	if (NULL == outdata) {
		ERR("outdata is NULL");
		return CALENDAR_ERROR_IPC;
	}

	// check outdata
	unsigned int size = 0;
	void *ret_pims = NULL;
	ret_pims = pims_ipc_data_get(outdata, &size);
	if (NULL == ret_pims) {
		ERR("pims_ipc_data_get() Fail");
		pims_ipc_data_destroy(outdata);
		return CALENDAR_ERROR_IPC;
	}
	ret = *(int*)ret_pims;

	if (CALENDAR_ERROR_NONE != ret) {
		ERR("pims_ipc_data_get() Fail(%d)", ret);
		pims_ipc_data_destroy(outdata);
		return ret;
	}

	int transaction_ver = 0;
	ret_pims = pims_ipc_data_get(outdata, &size);
	if (NULL == ret_pims) {
		ERR("pims_ipc_data_get() Fail");
		pims_ipc_data_destroy(outdata);
		return CALENDAR_ERROR_IPC;
	}
	transaction_ver = *(int*)ret_pims;
	cal_client_ipc_set_change_version(transaction_ver);

	pims_ipc_data_destroy(outdata);
	return ret;

}

int cal_client_db_replace_records(calendar_list_h record_list, int *record_id_array, int count)
{
	int ret = CALENDAR_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;
	int i = 0;

	RETV_IF(NULL == record_list, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == record_id_array, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(count <= 0, CALENDAR_ERROR_INVALID_PARAMETER, "count(%d) < 0", count);

	// make indata
	indata = pims_ipc_data_create(0);
    RETVM_IF(indata == NULL, CALENDAR_ERROR_OUT_OF_MEMORY, "pims_ipc_data_create() Fail");

	ret = cal_ipc_marshal_list(record_list,indata);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_list() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
	}
	ret = cal_ipc_marshal_int(count,indata);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
	}
	for(i = 0; i < count; i++) {
		ret = cal_ipc_marshal_int(record_id_array[i],indata);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_ipc_marshal_int() Fail(%d)", ret);
			pims_ipc_data_destroy(indata);
			return ret;
		}
	}

	// ipc call
	ret = cal_client_ipc_call( CAL_IPC_MODULE, CAL_IPC_SERVER_DB_REPLACE_RECORDS, indata, &outdata);
	pims_ipc_data_destroy(indata);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("pims_ipc_call Fail(%d)", ret);
		return CALENDAR_ERROR_IPC;
	}
	if (NULL == outdata) {
		ERR("outdata is NULL");
		return CALENDAR_ERROR_IPC;
	}

	// check outdata
	unsigned int size = 0;
	void *ret_pims = NULL;
	ret_pims = pims_ipc_data_get(outdata, &size);
	if (NULL == ret_pims) {
		ERR("pims_ipc_data_get() Fail");
		pims_ipc_data_destroy(outdata);
		return CALENDAR_ERROR_IPC;
	}
	ret = *(int*)ret_pims;

	if (CALENDAR_ERROR_NONE != ret) {
		ERR("pims_ipc_data_get() Fail(%d)", ret);
		pims_ipc_data_destroy(outdata);
		return ret;
	}

	int transaction_ver = 0;
	ret_pims = pims_ipc_data_get(outdata, &size);
	if (NULL == ret_pims) {
		ERR("pims_ipc_data_get() Fail");
		pims_ipc_data_destroy(outdata);
		return CALENDAR_ERROR_IPC;
	}
	transaction_ver = *(int*)ret_pims;
	cal_client_ipc_set_change_version(transaction_ver);

	pims_ipc_data_destroy(outdata);
	return ret;
}

int cal_client_db_get_changes_by_version(const char* view_uri, int calendar_book_id, int calendar_db_version, calendar_list_h* record_list, int* current_calendar_db_version )
{
	int ret = CALENDAR_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(NULL == view_uri, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == record_list, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == current_calendar_db_version, CALENDAR_ERROR_INVALID_PARAMETER);

	// make indata
	indata = pims_ipc_data_create(0);
	RETVM_IF(NULL == indata, CALENDAR_ERROR_OUT_OF_MEMORY, "ipc data created Fail !");

	ret = cal_ipc_marshal_char(view_uri,indata);
	if (CALENDAR_ERROR_NONE != ret) {
        ERR("cal_ipc_marshal_char() Fail(%d)", ret);
        pims_ipc_data_destroy(indata);
        return ret;
    }
    ret = cal_ipc_marshal_int(calendar_book_id,indata);
    if (CALENDAR_ERROR_NONE != ret) {
        ERR("cal_ipc_marshal_int() Fail(%d)", ret);
        pims_ipc_data_destroy(indata);
        return ret;
    }
    ret = cal_ipc_marshal_int(calendar_db_version,indata);
    if (CALENDAR_ERROR_NONE != ret) {
        ERR("cal_ipc_marshal_int() Fail(%d)", ret);
        pims_ipc_data_destroy(indata);
        return ret;
    }

	// ipc call
	ret = cal_client_ipc_call(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_CHANGES_BY_VERSION, indata, &outdata);
	pims_ipc_data_destroy(indata);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("pims_ipc_call Fail(%d)", ret);
		return CALENDAR_ERROR_IPC;
	}
	if (NULL == outdata) {
		ERR("outdata is NULL");
		return CALENDAR_ERROR_IPC;
	}

	// check outdata
	unsigned int size = 0;
	void *ret_pims = NULL;
	ret_pims = pims_ipc_data_get(outdata, &size);
	if (NULL == ret_pims) {
		ERR("pims_ipc_data_get() Fail");
		pims_ipc_data_destroy(outdata);
		return CALENDAR_ERROR_IPC;
	}
	ret = *(int*)ret_pims;

	if (CALENDAR_ERROR_NONE != ret) {
		ERR("pims_ipc_data_get() Fail(%d)", ret);
		pims_ipc_data_destroy(outdata);
		return ret;
	}

	ret = cal_ipc_unmarshal_list(outdata, record_list);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_list() Fail(%d)", ret);
		pims_ipc_data_destroy(outdata);
		return ret;
	}
	ret = cal_ipc_unmarshal_int(outdata, current_calendar_db_version);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		pims_ipc_data_destroy(outdata);
		return ret;
	}

	pims_ipc_data_destroy(outdata);
	return ret;
}

int cal_client_db_get_current_version(int* calendar_db_version)
{
	int ret = CALENDAR_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(NULL == calendar_db_version, CALENDAR_ERROR_INVALID_PARAMETER);

	// ipc call
	ret = cal_client_ipc_call(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_GET_CURRENT_VERSION, indata, &outdata);
	pims_ipc_data_destroy(indata);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("pims_ipc_call Fail(%d)", ret);
		return CALENDAR_ERROR_IPC;
	}
	if (NULL == outdata) {
		ERR("outdata is NULL");
		return CALENDAR_ERROR_IPC;
	}

	// check outdata
	unsigned int size = 0;
	void *ret_pims = NULL;
	ret_pims = pims_ipc_data_get(outdata, &size);
	if (NULL == ret_pims) {
		ERR("pims_ipc_data_get() Fail");
		pims_ipc_data_destroy(outdata);
		return CALENDAR_ERROR_IPC;
	}
	ret = *(int*)ret_pims;

	if (CALENDAR_ERROR_NONE != ret) {
		ERR("pims_ipc_data_get() Fail(%d)", ret);
		pims_ipc_data_destroy(outdata);
		return ret;
	}
	ret = cal_ipc_unmarshal_int(outdata, calendar_db_version);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		pims_ipc_data_destroy(outdata);
		return ret;
	}

	pims_ipc_data_destroy(outdata);
	return ret;
}

int cal_client_db_add_changed_cb(const char* view_uri, calendar_db_changed_cb callback, void* user_data )
{
	CAL_FN_CALL();
	int ret;
	cal_record_type_e type = CAL_RECORD_TYPE_INVALID;

	RETV_IF(NULL == view_uri, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == callback, CALENDAR_ERROR_INVALID_PARAMETER);

	type = cal_view_get_type(view_uri);

	switch(type) {
	case CAL_RECORD_TYPE_CALENDAR:
		ret = cal_inotify_subscribe(CAL_NOTI_TYPE_CALENDAR, CAL_NOTI_CALENDAR_CHANGED, callback, user_data);
		break;
	case CAL_RECORD_TYPE_EVENT:
		ret = cal_inotify_subscribe(CAL_NOTI_TYPE_EVENT, CAL_NOTI_EVENT_CHANGED, callback, user_data);
		break;
	case CAL_RECORD_TYPE_TODO:
		ret = cal_inotify_subscribe(CAL_NOTI_TYPE_TODO, CAL_NOTI_TODO_CHANGED, callback, user_data);
		break;
	default:
		ERR("Invalid view_uri(%s)", view_uri);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_inotify_subscribe() Failed(%d)", ret);

	return CALENDAR_ERROR_NONE;
}

int cal_client_db_remove_changed_cb( const char* view_uri, calendar_db_changed_cb callback, void* user_data )
{
	CAL_FN_CALL();
	int ret;
	cal_record_type_e type = CAL_RECORD_TYPE_INVALID;

	RETV_IF(NULL == view_uri, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == callback, CALENDAR_ERROR_INVALID_PARAMETER);

	type = cal_view_get_type(view_uri);

	switch(type) {
	case CAL_RECORD_TYPE_CALENDAR:
		ret = cal_inotify_unsubscribe_with_data(CAL_NOTI_CALENDAR_CHANGED, callback, user_data);
		break;
	case CAL_RECORD_TYPE_EVENT:
		ret = cal_inotify_unsubscribe_with_data(CAL_NOTI_EVENT_CHANGED, callback, user_data);
		break;
	case CAL_RECORD_TYPE_TODO:
		ret = cal_inotify_unsubscribe_with_data(CAL_NOTI_TODO_CHANGED, callback, user_data);
		break;
	default:
		ERR("Invalid view_uri(%s)", view_uri);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_inotify_unsubscribe_with_data() Failed(%d)", ret);

	return CALENDAR_ERROR_NONE;
}

int cal_client_db_insert_vcalendars(const char* vcalendar_stream, int **record_id_array, int *count)
{
	int ret = CALENDAR_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(NULL == vcalendar_stream, CALENDAR_ERROR_INVALID_PARAMETER);

	// make indata
	indata = pims_ipc_data_create(0);
    RETVM_IF(indata == NULL, CALENDAR_ERROR_OUT_OF_MEMORY, "pims_ipc_data_create() Fail");

	ret = cal_ipc_marshal_char(vcalendar_stream,indata);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_char() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
	}

	// ipc call
	ret = cal_client_ipc_call(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_INSERT_VCALENDARS, indata, &outdata);
	pims_ipc_data_destroy(indata);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("pims_ipc_call Fail(%d)", ret);
		return CALENDAR_ERROR_IPC;
	}
	if (NULL == outdata) {
		ERR("outdata is NULL");
		return CALENDAR_ERROR_IPC;
	}

	// check outdata
	unsigned int size = 0;
	void *ret_pims = NULL;
	ret_pims = pims_ipc_data_get(outdata, &size);
	if (NULL == ret_pims) {
		ERR("pims_ipc_data_get() Fail");
		pims_ipc_data_destroy(outdata);
		return CALENDAR_ERROR_IPC;
	}
	ret = *(int*)ret_pims;

	if (CALENDAR_ERROR_NONE != ret) {
		ERR("pims_ipc_data_get() Fail(%d)", ret);
		pims_ipc_data_destroy(outdata);
		return ret;
	}

	int transaction_ver = 0;
	ret_pims = pims_ipc_data_get(outdata, &size);
	if (NULL == ret_pims) {
		ERR("pims_ipc_data_get() Fail");
		pims_ipc_data_destroy(outdata);
		return CALENDAR_ERROR_IPC;
	}
	transaction_ver = *(int*)ret_pims;
	cal_client_ipc_set_change_version(transaction_ver);

	if (NULL == count || NULL == record_id_array) {
		pims_ipc_data_destroy(outdata);
		return ret;
	}

	*count = *(int*) pims_ipc_data_get(outdata, &size);
	if (*count <= 0) {
		ERR("count(%d) <= 0", *count);
		*count = 0;
		pims_ipc_data_destroy(outdata);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	int *ids = NULL;
	ids = calloc(1, sizeof(int)*(*count));
	if(NULL == ids) {
		ERR("calloc() Fail");
		*count = 0;
		pims_ipc_data_destroy(outdata);
		return CALENDAR_ERROR_OUT_OF_MEMORY;
	}

	int i = 0;
	for(i = 0; i < (*count); i++) {
		ret_pims = pims_ipc_data_get(outdata, &size);
		if (NULL == ret_pims) {
			ERR("pims_ipc_data_get() Fail");
			free(ids);
			pims_ipc_data_destroy(outdata);
			return CALENDAR_ERROR_IPC;
		}
		ids[i] = *(int*)ret_pims;
	}
	*record_id_array = ids;

	pims_ipc_data_destroy(outdata);
	return ret;
}

int cal_client_db_replace_vcalendars(const char* vcalendar_stream, int *record_id_array, int count)
{
	int ret = CALENDAR_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;
	int i = 0;

	RETV_IF(NULL == vcalendar_stream, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == record_id_array, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(count <= 0, CALENDAR_ERROR_INVALID_PARAMETER, "count(%d) < 0", count);

	// make indata
	indata = pims_ipc_data_create(0);
    RETVM_IF(indata == NULL, CALENDAR_ERROR_OUT_OF_MEMORY, "pims_ipc_data_create() Fail");

	ret = cal_ipc_marshal_char(vcalendar_stream,indata);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_char() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
	}
	ret = cal_ipc_marshal_int(count,indata);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_char() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
	}
	for(i = 0; i < count; i++) {
		ret = cal_ipc_marshal_int(record_id_array[i],indata);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_ipc_marshal_char() Fail(%d)", ret);
			pims_ipc_data_destroy(indata);
			return ret;
		}
	}

	// ipc call
	ret = cal_client_ipc_call(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_REPLACE_VCALENDARS, indata, &outdata);
	pims_ipc_data_destroy(indata);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("pims_ipc_call Fail(%d)", ret);
		return CALENDAR_ERROR_IPC;
	}
	if (NULL == outdata) {
		ERR("outdata is NULL");
		return CALENDAR_ERROR_IPC;
	}

	// check outdata
	unsigned int size = 0;
	void *ret_pims = NULL;
	ret_pims = pims_ipc_data_get(outdata, &size);
	if (NULL == ret_pims) {
		ERR("pims_ipc_data_get() Fail");
		pims_ipc_data_destroy(outdata);
		return CALENDAR_ERROR_IPC;
	}
	ret = *(int*)ret_pims;

	if (CALENDAR_ERROR_NONE != ret) {
		ERR("pims_ipc_data_get() Fail(%d)", ret);
		pims_ipc_data_destroy(outdata);
		return ret;
	}

	int transaction_ver = 0;
	ret_pims = pims_ipc_data_get(outdata, &size);
	if (NULL == ret_pims) {
		ERR("pims_ipc_data_get() Fail");
		pims_ipc_data_destroy(outdata);
		return CALENDAR_ERROR_IPC;
	}
	transaction_ver = *(int*)ret_pims;
	cal_client_ipc_set_change_version(transaction_ver);

	pims_ipc_data_destroy(outdata);
	return ret;
}

int cal_client_db_get_last_change_version(int* last_version)
{
	int ret = CALENDAR_ERROR_NONE;

	RETV_IF(NULL == last_version, CALENDAR_ERROR_INVALID_PARAMETER);

	*last_version = 0;

	bool result = false;
	ret = cal_client_ipc_client_check_permission(CAL_PERMISSION_READ, &result);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_client_ipc_client_check_permission() is fail (%d)", ret);
	RETVM_IF(result == false, CALENDAR_ERROR_PERMISSION_DENIED, "Permission denied (calendar read)");

	*last_version = cal_client_ipc_get_change_version();
	return ret;
}

int cal_client_db_get_changes_exception_by_version(const char* view_uri, int original_event_id, int calendar_db_version, calendar_list_h* record_list)
{
	int ret = CALENDAR_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(NULL == view_uri, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == record_list, CALENDAR_ERROR_INVALID_PARAMETER);

	// make indata
	indata = pims_ipc_data_create(0);
    RETVM_IF(indata == NULL, CALENDAR_ERROR_OUT_OF_MEMORY, "pims_ipc_data_create() Fail");

	ret = cal_ipc_marshal_char(view_uri,indata);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_char() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
	}
	ret = cal_ipc_marshal_int(original_event_id,indata);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
	}
	ret = cal_ipc_marshal_int(calendar_db_version,indata);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
	}

	// ipc call
	ret = cal_client_ipc_call(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_CHANGES_EXCEPTION, indata, &outdata);
	pims_ipc_data_destroy(indata);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("pims_ipc_call Fail(%d)", ret);
		return CALENDAR_ERROR_IPC;
	}
	if (NULL == outdata) {
		ERR("outdata is NULL");
		return CALENDAR_ERROR_IPC;
	}

	// check outdata
	unsigned int size = 0;
	void *ret_pims = NULL;
	ret_pims = pims_ipc_data_get(outdata, &size);
	if (NULL == ret_pims) {
		ERR("pims_ipc_data_get() Fail");
		pims_ipc_data_destroy(outdata);
		return CALENDAR_ERROR_IPC;
	}
	ret = *(int*)ret_pims;

	if (CALENDAR_ERROR_NONE != ret) {
		ERR("pims_ipc_data_get() Fail(%d)", ret);
		pims_ipc_data_destroy(outdata);
		return ret;
	}

	ret = cal_ipc_unmarshal_list(outdata,record_list);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_list() Fail(%d)", ret);
		pims_ipc_data_destroy(outdata);
		return ret;
	}

	pims_ipc_data_destroy(outdata);
	return ret;
}

#ifdef CAL_MEMORY_TEST
int cal_client_destroy(void)
{
	int ret = CALENDAR_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;


	// make indata
	indata = pims_ipc_data_create(0);
	if (indata == NULL)
	{
		ERR("ipc data created fail !");
		ret = CALENDAR_ERROR_OUT_OF_MEMORY;
		return ret;
	}

	// ipc call
	if (cal_client_ipc_call(CAL_IPC_MODULE, CAL_IPC_SERVER_DESTROY, indata, &outdata) != 0)
	{
		ERR("pims_ipc_call failed");
		pims_ipc_data_destroy(outdata);
		return CALENDAR_ERROR_IPC;
	}

	pims_ipc_data_destroy(indata);

	if (NULL == outdata) {
		ERR("ipc outdata is NULL");
		return CALENDAR_ERROR_IPC;
	}

	// check outdata
	unsigned int size = 0;
	void *ret_pims = NULL;
	ret_pims = pims_ipc_data_get(outdata, &size);
	if (NULL == ret_pims) {
		ERR("pims_ipc_data_get() Fail");
		pims_ipc_data_destroy(outdata);
		return CALENDAR_ERROR_IPC;
	}
	ret = *(int*)ret_pims;

	pims_ipc_data_destroy(outdata);
	return ret;
}
#endif //#ifdef CAL_MEMORY_TEST
