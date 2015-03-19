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

#include <stdlib.h>     //calloc
#include <pims-ipc.h>
#include <glib-object.h>    //g_type_init
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "calendar_service.h"
#include "calendar_db.h"
#include "calendar_types.h"

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

typedef struct {
	calendar_db_result_cb callback;
	void *user_data;
}cal_client_db_async_userdata_s;

typedef struct {
	calendar_db_insert_result_cb callback;
	void *user_data;
}cal_client_db_async_insert_userdata_s;

#define CAL_IPC_DATA_FREE(ptr) \
	do { \
		if (ptr) \
		pims_ipc_data_destroy(ptr); \
		ptr = NULL; \
	} while(0)

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


void _cal_client_db_insert_records_cb(pims_ipc_h ipc, pims_ipc_data_h data_out, void *userdata);
void _cal_client_db_update_records_cb(pims_ipc_h ipc, pims_ipc_data_h data_out, void *userdata);
void _cal_client_db_delete_records_cb(pims_ipc_h ipc, pims_ipc_data_h data_out, void *userdata);
void _cal_client_db_insert_vcalendars_cb(pims_ipc_h ipc, pims_ipc_data_h data_out, void *userdata);
void _cal_client_db_replace_vcalendars_cb(pims_ipc_h ipc, pims_ipc_data_h data_out, void *userdata);
void _cal_client_db_replace_records_cb(pims_ipc_h ipc, pims_ipc_data_h data_out, void *userdata);

void _cal_client_db_insert_records_cb(pims_ipc_h ipc, pims_ipc_data_h data_out, void *userdata)
{
	cal_client_db_async_insert_userdata_s *sync_data = (cal_client_db_async_insert_userdata_s *)userdata;
	int ret = CALENDAR_ERROR_NONE;

	int count = 0;
	int *id = 0;

	if (sync_data == NULL)
	{
		ERR("sync_data is NULL");
		return;
	}

	if (data_out)
	{
		// check outdata
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(data_out,&size);

		if (CALENDAR_ERROR_NONE == ret)
		{
			int i=0;
			unsigned int size = 0;

			int transaction_ver = 0;
			transaction_ver = *(int*)pims_ipc_data_get(data_out,&size);
			cal_client_ipc_set_change_version(transaction_ver);

			count = *(int*) pims_ipc_data_get(data_out,&size);

			id = calloc(1, sizeof(int)*count);

			if (id)
			{
				for(i=0;i<count;i++)
				{
					id[i] = *(unsigned int*) pims_ipc_data_get(data_out,&size);
				}
			}
			else
			{
				count = 0;
			}
		}

	}
	else
	{
		ret = CALENDAR_ERROR_IPC;
		ERR("async cb is no data");
	}

	if (sync_data->callback)
	{
		sync_data->callback(ret, id, count, sync_data->user_data);
	}

	cal_inotify_call_pending_callback();

	CAL_FREE(id);

	CAL_FREE(sync_data);

	return ;
}

void _cal_client_db_update_records_cb(pims_ipc_h ipc, pims_ipc_data_h data_out, void *userdata)
{
	cal_client_db_async_userdata_s *sync_data = (cal_client_db_async_userdata_s *)userdata;
	int ret = CALENDAR_ERROR_NONE;

	if (sync_data == NULL)
	{
		ERR("sync_data is NULL");
		return;
	}

	if (data_out)
	{
		// check outdata
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(data_out,&size);

		if (CALENDAR_ERROR_NONE == ret)
		{
			int transaction_ver = 0;
			transaction_ver = *(int*)pims_ipc_data_get(data_out,&size);
			cal_client_ipc_set_change_version(transaction_ver);
		}
	}
	else
	{
		ret = CALENDAR_ERROR_IPC;
		ERR("async cb is no data");
	}

	if (sync_data->callback)
	{
		sync_data->callback(ret, sync_data->user_data);
	}

	cal_inotify_call_pending_callback();

	CAL_FREE(sync_data);

	return ;
}
void _cal_client_db_delete_records_cb(pims_ipc_h ipc, pims_ipc_data_h data_out, void *userdata)
{
	cal_client_db_async_userdata_s *sync_data = (cal_client_db_async_userdata_s *)userdata;
	int ret = CALENDAR_ERROR_NONE;

	if (sync_data == NULL)
	{
		ERR("sync_data is NULL");
		return;
	}

	if (data_out)
	{
		// check outdata
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(data_out,&size);

		if (CALENDAR_ERROR_NONE == ret)
		{
			int transaction_ver = 0;
			transaction_ver = *(int*)pims_ipc_data_get(data_out,&size);
			cal_client_ipc_set_change_version(transaction_ver);
		}
	}
	else
	{
		ret = CALENDAR_ERROR_IPC;
		ERR("async cb is no data");
	}

	if (sync_data->callback)
	{
		sync_data->callback(ret, sync_data->user_data);
	}

	cal_inotify_call_pending_callback();

	CAL_FREE(sync_data);

	return ;
}

void _cal_client_db_insert_vcalendars_cb(pims_ipc_h ipc, pims_ipc_data_h data_out, void *userdata)
{
	cal_client_db_async_insert_userdata_s *sync_data = (cal_client_db_async_insert_userdata_s *)userdata;
	int ret = CALENDAR_ERROR_NONE;

	int count = 0;
	int *id = 0;

	if (sync_data == NULL)
	{
		ERR("sync_data is NULL");
		return;
	}

	if (data_out)
	{
		// check outdata
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(data_out,&size);

		if (CALENDAR_ERROR_NONE == ret)
		{
			int i=0;
			unsigned int size = 0;

			int transaction_ver = 0;
			transaction_ver = *(int*)pims_ipc_data_get(data_out,&size);
			cal_client_ipc_set_change_version(transaction_ver);

			count = *(int*) pims_ipc_data_get(data_out,&size);

			id = calloc(1, sizeof(int)*count);

			if (id)
			{
				for(i=0;i<count;i++)
				{
					id[i] = *(unsigned int*) pims_ipc_data_get(data_out,&size);
				}
			}
			else
			{
				count = 0;
			}
		}
	}
	else
	{
		ret = CALENDAR_ERROR_IPC;
		ERR("async cb is no data");
	}

	if (sync_data->callback)
	{
		sync_data->callback(ret, id, count, sync_data->user_data);
	}

	cal_inotify_call_pending_callback();

	CAL_FREE(id);

	CAL_FREE(sync_data);

	return ;
}

void _cal_client_db_replace_vcalendars_cb(pims_ipc_h ipc, pims_ipc_data_h data_out, void *userdata)
{
	cal_client_db_async_userdata_s *sync_data = (cal_client_db_async_userdata_s *)userdata;
	int ret = CALENDAR_ERROR_NONE;

	if (sync_data == NULL)
	{
		ERR("sync_data is NULL");
		return;
	}

	if (data_out)
	{
		// check outdata
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(data_out,&size);

		if (CALENDAR_ERROR_NONE == ret)
		{
			int transaction_ver = 0;
			transaction_ver = *(int*)pims_ipc_data_get(data_out,&size);
			cal_client_ipc_set_change_version(transaction_ver);
		}
	}
	else
	{
		ret = CALENDAR_ERROR_IPC;
		ERR("async cb is no data");
	}

	if (sync_data->callback)
	{
		sync_data->callback(ret, sync_data->user_data);
	}

	cal_inotify_call_pending_callback();

	CAL_FREE(sync_data);

	return ;
}

void _cal_client_db_replace_records_cb(pims_ipc_h ipc, pims_ipc_data_h data_out, void *userdata)
{
	cal_client_db_async_userdata_s *sync_data = (cal_client_db_async_userdata_s *)userdata;
	int ret = CALENDAR_ERROR_NONE;

	if (sync_data == NULL)
	{
		ERR("sync_data is NULL");
		return;
	}

	if (data_out)
	{
		// check outdata
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(data_out,&size);

		if (CALENDAR_ERROR_NONE == ret)
		{
			int transaction_ver = 0;
			transaction_ver = *(int*)pims_ipc_data_get(data_out,&size);
			cal_client_ipc_set_change_version(transaction_ver);
		}
	}
	else
	{
		ret = CALENDAR_ERROR_IPC;
		ERR("async cb is no data");
	}

	if (sync_data->callback)
	{
		sync_data->callback(ret, sync_data->user_data);
	}

	cal_inotify_call_pending_callback();

	CAL_FREE(sync_data);

	return ;
}

API int calendar_db_insert_record( calendar_record_h record, int* id )
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
	if (indata == NULL)
	{
		ERR("ipc data created fail !");
		return CALENDAR_ERROR_OUT_OF_MEMORY;
	}
	ret = cal_ipc_marshal_record(record,indata);
	if (ret != CALENDAR_ERROR_NONE)
	{
		ERR("marshal fail");
		CAL_IPC_DATA_FREE(indata);
		return ret;
	}

	// ipc call
	if (cal_client_ipc_call(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_INSERT_RECORD, indata, &outdata) != 0)
	{
		ERR("pims_ipc_call failed");
		CAL_IPC_DATA_FREE(indata);
		return CALENDAR_ERROR_IPC;
	}

	CAL_IPC_DATA_FREE(indata);

	if (outdata)
	{
		//int id = 0;
		// check outdata
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata,&size);

		if (CALENDAR_ERROR_NONE == ret)
		{
			int transaction_ver = 0;
			transaction_ver = *(int*)pims_ipc_data_get(outdata,&size);
			cal_client_ipc_set_change_version(transaction_ver);
			//unsigned int property_id = 0;
			int out_id = 0;
			//property_id = *(unsigned int*)pims_ipc_data_get(outdata,&size);
			out_id = *(int*)pims_ipc_data_get(outdata,&size);
			//cal_record_set_int(record,property_id,id);
			if (id)
			{
				*id = out_id;
			}
		}

		pims_ipc_data_destroy(outdata);
	}
	else
	{
		ERR("ipc outdata is NULL");
		return CALENDAR_ERROR_IPC;
	}

	CAL_LIMIT_ACCESS_BACK;
	return ret;
}

API int calendar_db_get_record( const char* view_uri, int id, calendar_record_h* out_record )
{
	int ret = CALENDAR_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(NULL == view_uri, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_record, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(id <= 0, CALENDAR_ERROR_INVALID_PARAMETER, "id(%d) <= 0", id);

	// make indata
	indata = pims_ipc_data_create(0);
	if (indata == NULL)
	{
		ERR("ipc data created fail !");
		ret = CALENDAR_ERROR_OUT_OF_MEMORY;
		return ret;
	}
	ret = cal_ipc_marshal_char(view_uri,indata);
	if (ret != CALENDAR_ERROR_NONE)
	{
		ERR("marshal fail");
		CAL_IPC_DATA_FREE(indata);
		return ret;
	}
	ret = cal_ipc_marshal_int(id,indata);
	if (ret != CALENDAR_ERROR_NONE)
	{
		ERR("marshal fail");
		CAL_IPC_DATA_FREE(indata);
		return ret;
	}

	// ipc call
	if (cal_client_ipc_call(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_GET_RECORD, indata, &outdata) != 0)
	{
		ERR("pims_ipc_call failed");
		CAL_IPC_DATA_FREE(indata);
		return CALENDAR_ERROR_IPC;
	}

	CAL_IPC_DATA_FREE(indata);

	if (outdata)
	{
		// check outdata
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata,&size);

		if (CALENDAR_ERROR_NONE == ret)
		{
			ret = cal_ipc_unmarshal_record(outdata,out_record);
		}

		pims_ipc_data_destroy(outdata);
	}
	else
	{
		ERR("ipc outdata is NULL");
		return CALENDAR_ERROR_IPC;
	}

	return ret;
}

API int calendar_db_update_record( calendar_record_h record )
{
	int ret = CALENDAR_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(NULL == record, CALENDAR_ERROR_INVALID_PARAMETER);

	cal_record_s *rec = (cal_record_s *)record;
	CAL_LIMIT_ACCESS_FRONT(rec->view_uri);

	// make indata
	indata = pims_ipc_data_create(0);
	if (indata == NULL)
	{
		ERR("ipc data created fail !");
		ret = CALENDAR_ERROR_OUT_OF_MEMORY;
		return ret;
	}
	ret = cal_ipc_marshal_record(record,indata);
	if (ret != CALENDAR_ERROR_NONE)
	{
		ERR("marshal fail");
		CAL_IPC_DATA_FREE(indata);
		return ret;
	}

	// ipc call
	if (cal_client_ipc_call(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_UPDATE_RECORD, indata, &outdata) != 0)
	{
		ERR("pims_ipc_call failed");
		CAL_IPC_DATA_FREE(indata);
		return CALENDAR_ERROR_IPC;
	}

	CAL_IPC_DATA_FREE(indata);

	if (outdata)
	{
		// check outdata
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata,&size);

		if (CALENDAR_ERROR_NONE == ret)
		{
			int transaction_ver = 0;
			transaction_ver = *(int*)pims_ipc_data_get(outdata,&size);
			cal_client_ipc_set_change_version(transaction_ver);
		}

		pims_ipc_data_destroy(outdata);
	}
	else
	{
		ERR("ipc outdata is NULL");
		return CALENDAR_ERROR_IPC;
	}

	CAL_LIMIT_ACCESS_BACK;
	return ret;
}

API int calendar_db_delete_record( const char* view_uri, int id )
{
	int ret = CALENDAR_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(NULL == view_uri, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(id <= 0, CALENDAR_ERROR_INVALID_PARAMETER,"id(%d) <= 0", id);

	CAL_LIMIT_ACCESS_FRONT(view_uri);

	// make indata
	indata = pims_ipc_data_create(0);
	if (indata == NULL)
	{
		ERR("ipc data created fail !");
		ret = CALENDAR_ERROR_OUT_OF_MEMORY;
		return ret;
	}
	ret = cal_ipc_marshal_char(view_uri,indata);
	if (ret != CALENDAR_ERROR_NONE)
	{
		ERR("marshal fail");
		CAL_IPC_DATA_FREE(indata);
		return ret;
	}
	ret = cal_ipc_marshal_int(id,indata);
	if (ret != CALENDAR_ERROR_NONE)
	{
		ERR("marshal fail");
		CAL_IPC_DATA_FREE(indata);
		return ret;
	}

	// ipc call
	if (cal_client_ipc_call(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_DELETE_RECORD, indata, &outdata) != 0)
	{
		ERR("pims_ipc_call failed");
		CAL_IPC_DATA_FREE(indata);
		return CALENDAR_ERROR_IPC;
	}

	CAL_IPC_DATA_FREE(indata);

	if (outdata)
	{
		// check outdata
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata,&size);

		if (CALENDAR_ERROR_NONE == ret)
		{
			int transaction_ver = 0;
			transaction_ver = *(int*)pims_ipc_data_get(outdata,&size);
			cal_client_ipc_set_change_version(transaction_ver);
		}

		pims_ipc_data_destroy(outdata);
	}
	else
	{
		ERR("ipc outdata is NULL");
		return CALENDAR_ERROR_IPC;
	}

	CAL_LIMIT_ACCESS_BACK;

	return ret;
}

API int calendar_db_get_all_records( const char* view_uri, int offset, int limit, calendar_list_h* out_list )
{
	int ret = CALENDAR_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(NULL == out_list, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == view_uri, CALENDAR_ERROR_INVALID_PARAMETER);

	// make indata
	indata = pims_ipc_data_create(0);
	if (indata == NULL)
	{
		ERR("ipc data created fail !");
		ret = CALENDAR_ERROR_OUT_OF_MEMORY;
		return ret;
	}

	ret = cal_ipc_marshal_char(view_uri,indata);
	if (ret != CALENDAR_ERROR_NONE)
	{
		ERR("marshal fail");
		CAL_IPC_DATA_FREE(indata);
		return ret;
	}
	ret = cal_ipc_marshal_int(offset,indata);
	if (ret != CALENDAR_ERROR_NONE)
	{
		ERR("marshal fail");
		CAL_IPC_DATA_FREE(indata);
		return ret;
	}
	ret = cal_ipc_marshal_int(limit,indata);
	if (ret != CALENDAR_ERROR_NONE)
	{
		ERR("marshal fail");
		CAL_IPC_DATA_FREE(indata);
		return ret;
	}

	// ipc call
	if (cal_client_ipc_call(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_GET_ALL_RECORDS, indata, &outdata) != 0)
	{
		ERR("pims_ipc_call failed");
		CAL_IPC_DATA_FREE(indata);
		return CALENDAR_ERROR_IPC;
	}

	CAL_IPC_DATA_FREE(indata);

	if (outdata)
	{
		// check outdata
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata,&size);

		if (CALENDAR_ERROR_NONE == ret)
		{
			ret = cal_ipc_unmarshal_list(outdata,out_list);
		}
		pims_ipc_data_destroy(outdata);
	}
	else
	{
		ERR("ipc outdata is NULL");
		return CALENDAR_ERROR_IPC;
	}

	return ret;
}

API int calendar_db_get_records_with_query( calendar_query_h query, int offset, int limit, calendar_list_h* out_list )
{
	int ret = CALENDAR_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(NULL == query, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_list, CALENDAR_ERROR_INVALID_PARAMETER);

	// make indata
	indata = pims_ipc_data_create(0);
	if (indata == NULL)
	{
		ERR("ipc data created fail !");
		ret = CALENDAR_ERROR_OUT_OF_MEMORY;
		return ret;
	}
	ret = cal_ipc_marshal_query(query,indata);
	if (ret != CALENDAR_ERROR_NONE)
	{
		ERR("marshal fail");
		CAL_IPC_DATA_FREE(indata);
		return ret;
	}
	ret = cal_ipc_marshal_int(offset,indata);
	if (ret != CALENDAR_ERROR_NONE)
	{
		ERR("marshal fail");
		CAL_IPC_DATA_FREE(indata);
		return ret;
	}
	ret = cal_ipc_marshal_int(limit,indata);
	if (ret != CALENDAR_ERROR_NONE)
	{
		ERR("marshal fail");
		CAL_IPC_DATA_FREE(indata);
		return ret;
	}

	// ipc call
	if (cal_client_ipc_call(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_GET_RECORDS_WITH_QUERY, indata, &outdata) != 0)
	{
		ERR("pims_ipc_call failed");
		CAL_IPC_DATA_FREE(indata);
		return CALENDAR_ERROR_IPC;
	}

	CAL_IPC_DATA_FREE(indata);

	if (outdata)
	{
		// check outdata
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata,&size);

		if (CALENDAR_ERROR_NONE == ret)
		{
			ret = cal_ipc_unmarshal_list(outdata,out_list);
		}

		pims_ipc_data_destroy(outdata);
	}
	else
	{
		ERR("ipc outdata is NULL");
		return CALENDAR_ERROR_IPC;
	}

	return ret;
}

API int calendar_db_clean_after_sync(int book_id, int calendar_db_version)
{
	int ret = CALENDAR_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETVM_IF(book_id <= 0, CALENDAR_ERROR_INVALID_PARAMETER, "book_id(%d) < 0", book_id);

	// make indata
	indata = pims_ipc_data_create(0);
	if (indata == NULL)
	{
		ERR("ipc data created fail !");
		ret = CALENDAR_ERROR_OUT_OF_MEMORY;
		return ret;
	}
	ret = cal_ipc_marshal_int(book_id, indata);
	if (ret != CALENDAR_ERROR_NONE)
	{
		ERR("marshal fail");
		CAL_IPC_DATA_FREE(indata);
		return ret;
	}
	ret = cal_ipc_marshal_int(calendar_db_version,indata);
	if (ret != CALENDAR_ERROR_NONE)
	{
		ERR("marshal fail");
		CAL_IPC_DATA_FREE(indata);
		return ret;
	}

	// ipc call
	if (cal_client_ipc_call(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_CLEAN_AFTER_SYNC, indata, &outdata) != 0)
	{
		ERR("pims_ipc_call failed");
		CAL_IPC_DATA_FREE(indata);
		return CALENDAR_ERROR_IPC;
	}

	CAL_IPC_DATA_FREE(indata);

	if (outdata)
	{
		// check outdata
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata,&size);

		pims_ipc_data_destroy(outdata);
	}
	else
	{
		ERR("ipc outdata is NULL");
		return CALENDAR_ERROR_IPC;
	}

	return ret;
}

API int calendar_db_get_count( const char* view_uri, int *out_count )
{
	int ret = CALENDAR_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(NULL == view_uri, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_count, CALENDAR_ERROR_INVALID_PARAMETER);

	// make indata
	indata = pims_ipc_data_create(0);
	if (indata == NULL)
	{
		ERR("ipc data created fail !");
		ret = CALENDAR_ERROR_OUT_OF_MEMORY;
		return ret;
	}
	ret = cal_ipc_marshal_char(view_uri,indata);
	if (ret != CALENDAR_ERROR_NONE)
	{
		ERR("marshal fail");
		CAL_IPC_DATA_FREE(indata);
		return ret;
	}

	// ipc call
	if (cal_client_ipc_call(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_GET_COUNT, indata, &outdata) != 0)
	{
		ERR("pims_ipc_call failed");
		CAL_IPC_DATA_FREE(indata);
		return CALENDAR_ERROR_IPC;
	}

	CAL_IPC_DATA_FREE(indata);

	if (outdata)
	{
		// check outdata
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata,&size);

		if (CALENDAR_ERROR_NONE == ret)
		{
			ret = cal_ipc_unmarshal_int(outdata,out_count);
		}

		pims_ipc_data_destroy(outdata);
	}
	else
	{
		ERR("ipc outdata is NULL");
		return CALENDAR_ERROR_IPC;
	}

	return ret;
}

API int calendar_db_get_count_with_query( calendar_query_h query, int *out_count )
{
	int ret = CALENDAR_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(NULL == query, CALENDAR_ERROR_INVALID_PARAMETER);

	// make indata
	indata = pims_ipc_data_create(0);
	if (indata == NULL)
	{
		ERR("ipc data created fail !");
		ret = CALENDAR_ERROR_OUT_OF_MEMORY;
		return ret;
	}
	ret = cal_ipc_marshal_query(query,indata);
	if (ret != CALENDAR_ERROR_NONE)
	{
		ERR("marshal fail");
		CAL_IPC_DATA_FREE(indata);
		return ret;
	}

	// ipc call
	if (cal_client_ipc_call(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_GET_COUNT_WITH_QUERY, indata, &outdata) != 0)
	{
		ERR("pims_ipc_call failed");
		CAL_IPC_DATA_FREE(indata);
		return CALENDAR_ERROR_IPC;
	}

	CAL_IPC_DATA_FREE(indata);

	if (outdata)
	{
		// check outdata
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata,&size);

		if (CALENDAR_ERROR_NONE == ret)
		{
			ret = cal_ipc_unmarshal_int(outdata,out_count);
		}

		pims_ipc_data_destroy(outdata);
	}
	else
	{
		ERR("ipc outdata is NULL");
		return CALENDAR_ERROR_IPC;
	}

	return ret;
}

API int calendar_db_insert_records( calendar_list_h record_list, int** record_id_array, int* count)
{
	int ret = CALENDAR_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(NULL == record_list, CALENDAR_ERROR_INVALID_PARAMETER);

	// make indata
	indata = pims_ipc_data_create(0);
	if (indata == NULL)
	{
		ERR("ipc data created fail !");
		ret = CALENDAR_ERROR_OUT_OF_MEMORY;
		return ret;
	}
	ret = cal_ipc_marshal_list(record_list,indata);
	if (ret != CALENDAR_ERROR_NONE)
	{
		ERR("marshal fail");
		CAL_IPC_DATA_FREE(indata);
		return ret;
	}

	if (cal_client_ipc_call(CAL_IPC_MODULE,CAL_IPC_SERVER_DB_INSERT_RECORDS,
				indata,&outdata) != 0)
	{
		ERR("pims_ipc_call failed");
		CAL_IPC_DATA_FREE(indata);
		return CALENDAR_ERROR_IPC;
	}

	CAL_IPC_DATA_FREE(indata);
	if (outdata)
	{
		// check outdata
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata,&size);

		if (CALENDAR_ERROR_NONE == ret)
		{
			int transaction_ver = 0;
			transaction_ver = *(int*)pims_ipc_data_get(outdata,&size);
			cal_client_ipc_set_change_version(transaction_ver);
			goto SET_DATA;
		}

		pims_ipc_data_destroy(outdata);
	}
	else
	{
		ERR("ipc outdata is NULL");
		return CALENDAR_ERROR_IPC;
	}

	return ret;

SET_DATA:
	if (outdata)
	{
		int i=0;
		unsigned int size = 0;
		int *ids = NULL;

		if (count && record_id_array)
		{
			*count = *(int*) pims_ipc_data_get(outdata,&size);

			if (*count <=0)
			{
				ERR("count is %d",*count);
				count = 0;
				pims_ipc_data_destroy(outdata);
				return CALENDAR_ERROR_INVALID_PARAMETER;
			}
			ids = calloc(1, sizeof(int)*(*count));

			if(ids == NULL)
			{
				count = 0;
				ERR("calloc fail");
				pims_ipc_data_destroy(outdata);
				return CALENDAR_ERROR_OUT_OF_MEMORY;
			}
			for(i=0;i<(*count);i++)
			{
				ids[i] = *(int*) pims_ipc_data_get(outdata,&size);
			}
			*record_id_array = ids;
		}
		pims_ipc_data_destroy(outdata);
	}

	return ret;
}

API int calendar_db_insert_records_async(calendar_list_h list, calendar_db_insert_result_cb callback, void *user_data)
{
	int ret = CALENDAR_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	cal_client_db_async_insert_userdata_s *async_data = NULL;
	calendar_list_h clone_list = NULL;
	bool result = false;

	RETV_IF(NULL == list, CALENDAR_ERROR_INVALID_PARAMETER);

	ret = cal_client_ipc_client_check_permission(CAL_PERMISSION_WRITE, &result);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "ctsvc_ipc_client_check_permission fail (%d)", ret);
	RETVM_IF(result == false, CALENDAR_ERROR_PERMISSION_DENIED, "Permission denied (calendar read)");

	ret = cal_list_clone(list, &clone_list);
	if (CALENDAR_ERROR_NONE != ret)
	{
		ERR("cal_list_clone() failed");
		return ret;
	}

	// make indata
	indata = pims_ipc_data_create(0);
	if (indata == NULL)
	{
		ERR("ipc data created fail !");
		ret = CALENDAR_ERROR_OUT_OF_MEMORY;
		calendar_list_destroy(clone_list, true);
		return ret;
	}
	ret = cal_ipc_marshal_list(clone_list,indata);
	if (ret != CALENDAR_ERROR_NONE)
	{
		ERR("marshal fail");
		CAL_IPC_DATA_FREE(indata);
		calendar_list_destroy(clone_list, true);
		return ret;
	}

	async_data = (cal_client_db_async_insert_userdata_s*)malloc(sizeof(cal_client_db_async_insert_userdata_s));
	if (async_data == NULL)
	{
		ERR("malloc fail !");
		ret = CALENDAR_ERROR_OUT_OF_MEMORY;
		calendar_list_destroy(clone_list, true);
		CAL_IPC_DATA_FREE(indata);
		return ret;
	}
	async_data->callback = callback;
	async_data->user_data = user_data;

	if (cal_client_ipc_call_async(CAL_IPC_MODULE,CAL_IPC_SERVER_DB_INSERT_RECORDS,
				indata,_cal_client_db_insert_records_cb,async_data) != 0)
	{
		ERR("pims_ipc_call_async failed");
		calendar_list_destroy(clone_list, true);
		CAL_IPC_DATA_FREE(indata);
		CAL_FREE(async_data);
		return CALENDAR_ERROR_IPC;
	}

	CAL_IPC_DATA_FREE(indata);

	calendar_list_destroy(clone_list, true);

	return ret;
}

API int calendar_db_update_records( calendar_list_h record_list)
{
	int ret = CALENDAR_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(NULL == record_list, CALENDAR_ERROR_INVALID_PARAMETER);

	// make indata
	indata = pims_ipc_data_create(0);
	if (indata == NULL)
	{
		ERR("ipc data created fail !");
		ret = CALENDAR_ERROR_OUT_OF_MEMORY;
		return ret;
	}
	ret = cal_ipc_marshal_list(record_list,indata);
	if (ret != CALENDAR_ERROR_NONE)
	{
		ERR("marshal fail");
		CAL_IPC_DATA_FREE(indata);
		return ret;
	}

	// ipc call
	if (cal_client_ipc_call(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_UPDATE_RECORDS, indata, &outdata) != 0)
	{
		ERR("pims_ipc_call failed");
		CAL_IPC_DATA_FREE(indata);
		return CALENDAR_ERROR_IPC;
	}

	CAL_IPC_DATA_FREE(indata);

	if (outdata)
	{
		// check outdata
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata,&size);

		if (CALENDAR_ERROR_NONE == ret)
		{
			int transaction_ver = 0;
			transaction_ver = *(int*)pims_ipc_data_get(outdata,&size);
			cal_client_ipc_set_change_version(transaction_ver);
		}

		pims_ipc_data_destroy(outdata);
	}
	else
	{
		ERR("ipc outdata is NULL");
		return CALENDAR_ERROR_IPC;
	}

	return ret;

}

API int calendar_db_update_records_async( calendar_list_h list, calendar_db_result_cb callback, void *user_data)
{
	int ret = CALENDAR_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	cal_client_db_async_userdata_s *async_data = NULL;
	calendar_list_h clone_list = NULL;
	bool result = false;

	RETV_IF(NULL == list, CALENDAR_ERROR_INVALID_PARAMETER);

	ret = cal_client_ipc_client_check_permission(CAL_PERMISSION_WRITE, &result);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "ctsvc_ipc_client_check_permission fail (%d)", ret);
	RETVM_IF(result == false, CALENDAR_ERROR_PERMISSION_DENIED, "Permission denied (calendar read)");

	ret = cal_list_clone(list, &clone_list);
	if (CALENDAR_ERROR_NONE != ret)
	{
		ERR("cal_list_clone() failed");
		return ret;
	}

	// make indata
	indata = pims_ipc_data_create(0);
	if (indata == NULL)
	{
		ERR("ipc data created fail !");
		ret = CALENDAR_ERROR_OUT_OF_MEMORY;
		calendar_list_destroy(clone_list, true);
		return ret;
	}
	ret = cal_ipc_marshal_list(clone_list,indata);
	if (ret != CALENDAR_ERROR_NONE)
	{
		ERR("marshal fail");
		calendar_list_destroy(clone_list, true);
		CAL_IPC_DATA_FREE(indata);
		return ret;
	}

	async_data = (cal_client_db_async_userdata_s*)malloc(sizeof(cal_client_db_async_userdata_s));
	if (async_data == NULL)
	{
		ERR("malloc fail !");
		ret = CALENDAR_ERROR_OUT_OF_MEMORY;
		calendar_list_destroy(clone_list, true);
		CAL_IPC_DATA_FREE(indata);
		return ret;
	}
	async_data->callback = callback;
	async_data->user_data = user_data;

	if (cal_client_ipc_call_async(CAL_IPC_MODULE,CAL_IPC_SERVER_DB_UPDATE_RECORDS,
				indata,_cal_client_db_update_records_cb,async_data) != 0)
	{
		ERR("pims_ipc_call_async failed");
		calendar_list_destroy(clone_list, true);
		CAL_IPC_DATA_FREE(indata);
		CAL_FREE(async_data);
		return CALENDAR_ERROR_IPC;
	}

	CAL_IPC_DATA_FREE(indata);

	calendar_list_destroy(clone_list, true);
	return ret;
}

API int calendar_db_delete_records(const char* view_uri, int record_id_array[], int count)
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
	if (indata == NULL)
	{
		ERR("ipc data created fail !");
		ret = CALENDAR_ERROR_OUT_OF_MEMORY;
		return ret;
	}
	ret = cal_ipc_marshal_char(view_uri,indata);
	if (ret != CALENDAR_ERROR_NONE)
	{
		ERR("marshal fail");
		CAL_IPC_DATA_FREE(indata);
		return ret;
	}
	ret = cal_ipc_marshal_int(count,indata);
	if (ret != CALENDAR_ERROR_NONE)
	{
		ERR("marshal fail");
		CAL_IPC_DATA_FREE(indata);
		return ret;
	}
	for (i=0;i<count;i++)
	{
		ret = cal_ipc_marshal_int(record_id_array[i],indata);
		if (ret != CALENDAR_ERROR_NONE)
		{
			ERR("marshal fail");
			CAL_IPC_DATA_FREE(indata);
			return ret;
		}
	}

	// ipc call
	if (cal_client_ipc_call(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_DELETE_RECORDS, indata, &outdata) != 0)
	{
		ERR("pims_ipc_call failed");
		CAL_IPC_DATA_FREE(indata);
		return CALENDAR_ERROR_IPC;
	}

	CAL_IPC_DATA_FREE(indata);

	if (outdata)
	{
		// check outdata
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata,&size);

		if (CALENDAR_ERROR_NONE == ret)
		{
			int transaction_ver = 0;
			transaction_ver = *(int*)pims_ipc_data_get(outdata,&size);
			cal_client_ipc_set_change_version(transaction_ver);
		}

		pims_ipc_data_destroy(outdata);
	}
	else
	{
		ERR("ipc outdata is NULL");
		return CALENDAR_ERROR_IPC;
	}

	return ret;

}

API int calendar_db_delete_records_async(const char* view_uri, int ids[], int count, calendar_db_result_cb callback, void *user_data)
{
	int ret = CALENDAR_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	int i = 0;
	cal_client_db_async_userdata_s *async_data = NULL;
	bool result = false;

	RETV_IF(NULL == view_uri, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == ids, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(count <= 0, CALENDAR_ERROR_INVALID_PARAMETER, "count(%d) < 0", count);

	ret = cal_client_ipc_client_check_permission(CAL_PERMISSION_WRITE, &result);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "ctsvc_ipc_client_check_permission fail (%d)", ret);
	RETVM_IF(result == false, CALENDAR_ERROR_PERMISSION_DENIED, "Permission denied (calendar read)");

	// make indata
	indata = pims_ipc_data_create(0);
	if (indata == NULL)
	{
		ERR("ipc data created fail !");
		ret = CALENDAR_ERROR_OUT_OF_MEMORY;
		return ret;
	}
	ret = cal_ipc_marshal_char(view_uri,indata);
	if (ret != CALENDAR_ERROR_NONE)
	{
		ERR("marshal fail");
		CAL_IPC_DATA_FREE(indata);
		return ret;
	}
	ret = cal_ipc_marshal_int(count,indata);
	if (ret != CALENDAR_ERROR_NONE)
	{
		ERR("marshal fail");
		CAL_IPC_DATA_FREE(indata);
		return ret;
	}
	for (i=0;i<count;i++)
	{
		ret = cal_ipc_marshal_int(ids[i],indata);
		if (ret != CALENDAR_ERROR_NONE)
		{
			ERR("marshal fail");
			CAL_IPC_DATA_FREE(indata);
			return ret;
		}
	}

	async_data = (cal_client_db_async_userdata_s*)malloc(sizeof(cal_client_db_async_userdata_s));
	if (async_data == NULL)
	{
		ERR("malloc fail !");
		ret = CALENDAR_ERROR_OUT_OF_MEMORY;
		CAL_IPC_DATA_FREE(indata);
		return ret;
	}
	async_data->callback = callback;
	async_data->user_data = user_data;
	if (cal_client_ipc_call_async(CAL_IPC_MODULE,CAL_IPC_SERVER_DB_DELETE_RECORDS,
				indata,_cal_client_db_delete_records_cb,async_data) != 0)
	{
		ERR("pims_ipc_call_async failed");
		CAL_IPC_DATA_FREE(indata);
		CAL_FREE(async_data);
		return CALENDAR_ERROR_IPC;
	}

	CAL_IPC_DATA_FREE(indata);

	return ret;
}

API int calendar_db_get_changes_by_version(const char* view_uri, int calendar_book_id, int calendar_db_version, calendar_list_h* record_list, int* current_calendar_db_version )
{
	int ret = CALENDAR_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(NULL == view_uri, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == record_list, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == current_calendar_db_version, CALENDAR_ERROR_INVALID_PARAMETER);

	// make indata
	indata = pims_ipc_data_create(0);
	if (indata == NULL)
	{
		ERR("ipc data created fail !");
		ret = CALENDAR_ERROR_OUT_OF_MEMORY;
		return ret;
	}
	ret = cal_ipc_marshal_char(view_uri,indata);
	if (ret != CALENDAR_ERROR_NONE)
	{
		ERR("marshal fail");
		CAL_IPC_DATA_FREE(indata);
		return ret;
	}
	ret = cal_ipc_marshal_int(calendar_book_id,indata);
	if (ret != CALENDAR_ERROR_NONE)
	{
		ERR("marshal fail");
		CAL_IPC_DATA_FREE(indata);
		return ret;
	}
	ret = cal_ipc_marshal_int(calendar_db_version,indata);
	if (ret != CALENDAR_ERROR_NONE)
	{
		ERR("marshal fail");
		CAL_IPC_DATA_FREE(indata);
		return ret;
	}

	// ipc call
	if (cal_client_ipc_call(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_CHANGES_BY_VERSION, indata, &outdata) != 0)
	{
		ERR("pims_ipc_call failed");
		CAL_IPC_DATA_FREE(indata);
		return CALENDAR_ERROR_IPC;
	}

	CAL_IPC_DATA_FREE(indata);

	if (outdata)
	{
		// check outdata
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata,&size);

		if (CALENDAR_ERROR_NONE == ret)
		{
			ret = cal_ipc_unmarshal_list(outdata,record_list);

			if (CALENDAR_ERROR_NONE == ret)
			{
				ret = cal_ipc_unmarshal_int(outdata,current_calendar_db_version);
			}
		}

		pims_ipc_data_destroy(outdata);
	}
	else
	{
		ERR("ipc outdata is NULL");
		return CALENDAR_ERROR_IPC;
	}

	return ret;
}

API int calendar_db_get_current_version(int* calendar_db_version)
{
	int ret = CALENDAR_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(NULL == calendar_db_version, CALENDAR_ERROR_INVALID_PARAMETER);

	// ipc call
	if (cal_client_ipc_call(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_GET_CURRENT_VERSION, indata, &outdata) != 0)
	{
		ERR("pims_ipc_call failed");
		return CALENDAR_ERROR_IPC;
	}

	if (outdata)
	{
		// check outdata
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata,&size);
		if (CALENDAR_ERROR_NONE == ret)
		{
			ret = cal_ipc_unmarshal_int(outdata,calendar_db_version);
		}
		pims_ipc_data_destroy(outdata);
	}
	else
	{
		ERR("ipc outdata is NULL");
		return CALENDAR_ERROR_IPC;
	}

	return ret;
}

API int calendar_db_add_changed_cb(const char* view_uri, calendar_db_changed_cb callback, void* user_data )
{
	CAL_FN_CALL();
	int ret;
	cal_record_type_e type = CAL_RECORD_TYPE_INVALID;

	RETV_IF(NULL == view_uri, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == callback, CALENDAR_ERROR_INVALID_PARAMETER);

	type = cal_view_get_type(view_uri);

	switch(type)
	{
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

API int calendar_db_remove_changed_cb( const char* view_uri, calendar_db_changed_cb callback, void* user_data )
{
	CAL_FN_CALL();
	int ret;
	cal_record_type_e type = CAL_RECORD_TYPE_INVALID;

	RETV_IF(NULL == view_uri, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == callback, CALENDAR_ERROR_INVALID_PARAMETER);

	type = cal_view_get_type(view_uri);

	switch(type)
	{
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

API int calendar_db_insert_vcalendars(const char* vcalendar_stream, int **record_id_array, int *count)
{
	int ret = CALENDAR_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;
	int i = 0;

	RETV_IF(NULL == vcalendar_stream, CALENDAR_ERROR_INVALID_PARAMETER);

	// make indata
	indata = pims_ipc_data_create(0);
	if (indata == NULL)
	{
		ERR("ipc data created fail !");
		ret = CALENDAR_ERROR_OUT_OF_MEMORY;
		return ret;
	}
	ret = cal_ipc_marshal_char(vcalendar_stream,indata);
	if (ret != CALENDAR_ERROR_NONE)
	{
		ERR("marshal fail");
		CAL_IPC_DATA_FREE(indata);
		return ret;
	}
	// ipc call
	if (cal_client_ipc_call(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_INSERT_VCALENDARS, indata, &outdata) != 0)
	{
		ERR("pims_ipc_call failed");
		CAL_IPC_DATA_FREE(indata);
		return CALENDAR_ERROR_IPC;
	}
	CAL_IPC_DATA_FREE(indata);
	if (outdata)
	{
		// check outdata
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata,&size);

		if (CALENDAR_ERROR_NONE == ret)
		{
			int transaction_ver = 0;
			transaction_ver = *(int*)pims_ipc_data_get(outdata,&size);
			cal_client_ipc_set_change_version(transaction_ver);
		}

		if (CALENDAR_ERROR_NONE == ret&& count != NULL && record_id_array != NULL)
		{
			int *ids = NULL;

			*count = *(int*) pims_ipc_data_get(outdata,&size);

			ids = (int*)malloc(sizeof(int)*(*count));
			if(ids == NULL)
			{
				pims_ipc_data_destroy(outdata);
				ERR("malloc fail");
				return CALENDAR_ERROR_OUT_OF_MEMORY;
			}
			for(i=0;i<(*count);i++)
			{
				ids[i] = *(int*) pims_ipc_data_get(outdata,&size);
			}

			*record_id_array = ids;
		}
		pims_ipc_data_destroy(outdata);
	}
	else
	{
		ERR("ipc outdata is NULL");
		return CALENDAR_ERROR_IPC;
	}

	return ret;
}

API int calendar_db_insert_vcalendars_async(const char* vcalendar_stream, calendar_db_insert_result_cb callback, void *user_data)
{
	int ret = CALENDAR_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	cal_client_db_async_insert_userdata_s *async_data = NULL;
	bool result = false;

	RETV_IF(NULL == vcalendar_stream, CALENDAR_ERROR_INVALID_PARAMETER);

	ret = cal_client_ipc_client_check_permission(CAL_PERMISSION_WRITE, &result);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "ctsvc_ipc_client_check_permission fail (%d)", ret);
	RETVM_IF(result == false, CALENDAR_ERROR_PERMISSION_DENIED, "Permission denied (calendar read)");

	// make indata
	indata = pims_ipc_data_create(0);
	if (indata == NULL)
	{
		ERR("ipc data created fail !");
		ret = CALENDAR_ERROR_OUT_OF_MEMORY;
		return ret;
	}
	ret = cal_ipc_marshal_char(vcalendar_stream,indata);
	if (ret != CALENDAR_ERROR_NONE)
	{
		ERR("marshal fail");
		CAL_IPC_DATA_FREE(indata);

		return ret;
	}

	async_data = (cal_client_db_async_insert_userdata_s*)malloc(sizeof(cal_client_db_async_insert_userdata_s));
	if (async_data == NULL)
	{
		ERR("malloc fail !");
		ret = CALENDAR_ERROR_OUT_OF_MEMORY;
		CAL_IPC_DATA_FREE(indata);
		return ret;
	}
	async_data->callback = callback;
	async_data->user_data = user_data;

	if (cal_client_ipc_call_async(CAL_IPC_MODULE,CAL_IPC_SERVER_DB_INSERT_VCALENDARS,
				indata,_cal_client_db_insert_vcalendars_cb,async_data) != 0)
	{
		ERR("pims_ipc_call_async failed");
		CAL_IPC_DATA_FREE(indata);
		CAL_FREE(async_data);
		return CALENDAR_ERROR_IPC;
	}

	CAL_IPC_DATA_FREE(indata);

	return ret;
}

API int calendar_db_replace_vcalendars(const char* vcalendar_stream, int *record_id_array, int count)
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
	if (indata == NULL)
	{
		ERR("ipc data created fail !");
		ret = CALENDAR_ERROR_OUT_OF_MEMORY;
		return ret;
	}
	ret = cal_ipc_marshal_char(vcalendar_stream,indata);
	if (ret != CALENDAR_ERROR_NONE)
	{
		ERR("marshal fail");
		CAL_IPC_DATA_FREE(indata);
		return ret;
	}
	ret = cal_ipc_marshal_int(count,indata);
	if (ret != CALENDAR_ERROR_NONE)
	{
		ERR("marshal fail");
		CAL_IPC_DATA_FREE(indata);
		return ret;
	}
	for(i=0;i<count;i++)
	{
		ret = cal_ipc_marshal_int(record_id_array[i],indata);
		if (ret != CALENDAR_ERROR_NONE)
		{
			ERR("marshal fail");
			CAL_IPC_DATA_FREE(indata);
			return ret;
		}
	}

	// ipc call
	if (cal_client_ipc_call(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_REPLACE_VCALENDARS, indata, &outdata) != 0)
	{
		ERR("pims_ipc_call failed");
		CAL_IPC_DATA_FREE(indata);
		return CALENDAR_ERROR_IPC;
	}
	CAL_IPC_DATA_FREE(indata);
	if (outdata)
	{
		// check outdata
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata,&size);

		if (CALENDAR_ERROR_NONE == ret)
		{
			int transaction_ver = 0;
			transaction_ver = *(int*)pims_ipc_data_get(outdata,&size);
			cal_client_ipc_set_change_version(transaction_ver);
		}

		pims_ipc_data_destroy(outdata);
	}
	else
	{
		ERR("ipc outdata is NULL");
		return CALENDAR_ERROR_IPC;
	}

	return ret;
}

API int calendar_db_replace_vcalendars_async(const char* vcalendar_stream, int *record_id_array, int count, calendar_db_result_cb callback, void *user_data)
{
	int ret = CALENDAR_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	cal_client_db_async_userdata_s *async_data = NULL;
	int i = 0;
	bool result = false;

	RETV_IF(NULL == vcalendar_stream, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == record_id_array, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(count <= 0, CALENDAR_ERROR_INVALID_PARAMETER, "count(%d) < 0", count);

	ret = cal_client_ipc_client_check_permission(CAL_PERMISSION_WRITE, &result);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "ctsvc_ipc_client_check_permission fail (%d)", ret);
	RETVM_IF(result == false, CALENDAR_ERROR_PERMISSION_DENIED, "Permission denied (calendar read)");

	// make indata
	indata = pims_ipc_data_create(0);
	if (indata == NULL)
	{
		ERR("ipc data created fail !");
		ret = CALENDAR_ERROR_OUT_OF_MEMORY;
		return ret;
	}
	ret = cal_ipc_marshal_char(vcalendar_stream,indata);
	if (ret != CALENDAR_ERROR_NONE)
	{
		ERR("marshal fail");
		CAL_IPC_DATA_FREE(indata);
		return ret;
	}
	ret = cal_ipc_marshal_int(count,indata);
	if (ret != CALENDAR_ERROR_NONE)
	{
		ERR("marshal fail");
		CAL_IPC_DATA_FREE(indata);
		return ret;
	}
	for(i=0;i<count;i++)
	{
		ret = cal_ipc_marshal_int(record_id_array[i],indata);
		if (ret != CALENDAR_ERROR_NONE)
		{
			ERR("marshal fail");
			CAL_IPC_DATA_FREE(indata);
			return ret;
		}
	}

	async_data = (cal_client_db_async_userdata_s*)malloc(sizeof(cal_client_db_async_userdata_s));
	if (async_data == NULL)
	{
		ERR("malloc fail !");
		ret = CALENDAR_ERROR_OUT_OF_MEMORY;
		CAL_IPC_DATA_FREE(indata);
		return ret;
	}
	async_data->callback = callback;
	async_data->user_data = user_data;

	if (cal_client_ipc_call_async(CAL_IPC_MODULE,CAL_IPC_SERVER_DB_REPLACE_VCALENDARS,
				indata,_cal_client_db_replace_vcalendars_cb,async_data) != 0)
	{
		ERR("pims_ipc_call_async failed");
		CAL_IPC_DATA_FREE(indata);
		CAL_FREE(async_data);
		return CALENDAR_ERROR_IPC;
	}

	CAL_IPC_DATA_FREE(indata);

	return ret;
}

API int calendar_db_replace_record(calendar_record_h record, int record_id)
{
	int ret = CALENDAR_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(NULL == record, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(record_id < 0, CALENDAR_ERROR_INVALID_PARAMETER, "record_id(%d) < 0", record_id);

	cal_record_s *rec = (cal_record_s *)record;
	CAL_LIMIT_ACCESS_FRONT(rec->view_uri);

	// make indata
	indata = pims_ipc_data_create(0);
	if (indata == NULL)
	{
		ERR("ipc data created fail !");
		ret = CALENDAR_ERROR_OUT_OF_MEMORY;
		return ret;
	}
	ret = cal_ipc_marshal_record(record,indata);
	if (ret != CALENDAR_ERROR_NONE)
	{
		ERR("marshal fail");
		CAL_IPC_DATA_FREE(indata);
		return ret;
	}
	ret = cal_ipc_marshal_int(record_id,indata);
	if (ret != CALENDAR_ERROR_NONE)
	{
		ERR("marshal fail");
		CAL_IPC_DATA_FREE(indata);
		return ret;
	}

	// ipc call
	if (cal_client_ipc_call(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_REPLACE_RECORD, indata, &outdata) != 0)
	{
		ERR("pims_ipc_call failed");
		CAL_IPC_DATA_FREE(indata);
		return CALENDAR_ERROR_IPC;
	}

	CAL_IPC_DATA_FREE(indata);

	if (outdata)
	{
		// check outdata
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata,&size);
		if (CALENDAR_ERROR_NONE == ret)
		{
			int transaction_ver = 0;
			transaction_ver = *(int*)pims_ipc_data_get(outdata,&size);
			cal_client_ipc_set_change_version(transaction_ver);
		}
		pims_ipc_data_destroy(outdata);
	}
	else
	{
		ERR("ipc outdata is NULL");
		return CALENDAR_ERROR_IPC;
	}

	CAL_LIMIT_ACCESS_BACK;

	return ret;
}

API int calendar_db_replace_records(calendar_list_h record_list, int *record_id_array, int count)
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
	if (indata == NULL)
	{
		ERR("ipc data created fail !");
		ret = CALENDAR_ERROR_OUT_OF_MEMORY;
		return ret;
	}
	ret = cal_ipc_marshal_list(record_list,indata);
	if (ret != CALENDAR_ERROR_NONE)
	{
		ERR("marshal fail");
		CAL_IPC_DATA_FREE(indata);
		return ret;
	}
	ret = cal_ipc_marshal_int(count,indata);
	if (ret != CALENDAR_ERROR_NONE)
	{
		ERR("marshal fail");
		CAL_IPC_DATA_FREE(indata);
		return ret;
	}
	for(i=0;i<count;i++)
	{
		ret = cal_ipc_marshal_int(record_id_array[i],indata);
		if (ret != CALENDAR_ERROR_NONE)
		{
			ERR("marshal fail");
			CAL_IPC_DATA_FREE(indata);
			return ret;
		}
	}

	// ipc call
	if (cal_client_ipc_call( CAL_IPC_MODULE, CAL_IPC_SERVER_DB_REPLACE_RECORDS, indata, &outdata) != 0)
	{
		ERR("pims_ipc_call failed");
		CAL_IPC_DATA_FREE(indata);
		return CALENDAR_ERROR_IPC;
	}

	CAL_IPC_DATA_FREE(indata);

	if (outdata)
	{
		// check outdata
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata,&size);
		if (CALENDAR_ERROR_NONE == ret)
		{
			int transaction_ver = 0;
			transaction_ver = *(int*)pims_ipc_data_get(outdata,&size);
			cal_client_ipc_set_change_version(transaction_ver);
		}
		pims_ipc_data_destroy(outdata);
	}
	else
	{
		ERR("ipc outdata is NULL");
		return CALENDAR_ERROR_IPC;
	}

	return ret;
}

API int calendar_db_replace_records_async(calendar_list_h record_list, int *record_id_array, int count, calendar_db_result_cb callback, void *user_data)
{
	int ret = CALENDAR_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	cal_client_db_async_userdata_s *async_data = NULL;
	int i = 0;
	bool result = false;

	RETV_IF(NULL == record_list, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == record_id_array, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(count <= 0, CALENDAR_ERROR_INVALID_PARAMETER, "count(%d) < 0", count);

	ret = cal_client_ipc_client_check_permission(CAL_PERMISSION_WRITE, &result);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "ctsvc_ipc_client_check_permission fail (%d)", ret);
	RETVM_IF(result == false, CALENDAR_ERROR_PERMISSION_DENIED, "Permission denied (calendar read)");

	// make indata
	indata = pims_ipc_data_create(0);
	if (indata == NULL)
	{
		ERR("ipc data created fail !");
		ret = CALENDAR_ERROR_OUT_OF_MEMORY;
		return ret;
	}
	ret = cal_ipc_marshal_list(record_list,indata);
	if (ret != CALENDAR_ERROR_NONE)
	{
		ERR("marshal fail");
		CAL_IPC_DATA_FREE(indata);
		return ret;
	}
	ret = cal_ipc_marshal_int(count,indata);
	if (ret != CALENDAR_ERROR_NONE)
	{
		ERR("marshal fail");
		CAL_IPC_DATA_FREE(indata);
		return ret;
	}
	for(i=0;i<count;i++)
	{
		ret = cal_ipc_marshal_int(record_id_array[i],indata);
		if (ret != CALENDAR_ERROR_NONE)
		{
			ERR("marshal fail");
			CAL_IPC_DATA_FREE(indata);
			return ret;
		}
	}

	async_data = (cal_client_db_async_userdata_s*)malloc(sizeof(cal_client_db_async_userdata_s));
	if (async_data == NULL)
	{
		ERR("malloc fail !");
		ret = CALENDAR_ERROR_OUT_OF_MEMORY;
		CAL_IPC_DATA_FREE(indata);
		return ret;
	}
	async_data->callback = callback;
	async_data->user_data = user_data;

	if (cal_client_ipc_call_async(CAL_IPC_MODULE,CAL_IPC_SERVER_DB_REPLACE_RECORDS,
				indata,_cal_client_db_replace_records_cb,async_data) != 0)
	{
		ERR("pims_ipc_call_async failed");
		CAL_IPC_DATA_FREE(indata);
		CAL_FREE(async_data);
		return CALENDAR_ERROR_IPC;
	}

	CAL_IPC_DATA_FREE(indata);

	return ret;
}

API int calendar_db_get_last_change_version(int* last_version)
{
	int ret = CALENDAR_ERROR_NONE;
	bool result = false;
	RETV_IF(NULL == last_version, CALENDAR_ERROR_INVALID_PARAMETER);
	*last_version = 0;

	ret = cal_client_ipc_client_check_permission(CAL_PERMISSION_READ, &result);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_client_ipc_client_check_permission() is fail (%d)", ret);
	RETVM_IF(result == false, CALENDAR_ERROR_PERMISSION_DENIED, "Permission denied (calendar read)");

	*last_version = cal_client_ipc_get_change_version();
	return ret;
}

API int calendar_db_get_changes_exception_by_version(const char* view_uri, int original_event_id, int calendar_db_version, calendar_list_h* record_list)
{
	int ret = CALENDAR_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(NULL == view_uri, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == record_list, CALENDAR_ERROR_INVALID_PARAMETER);

	// make indata
	indata = pims_ipc_data_create(0);
	if (indata == NULL)
	{
		ERR("ipc data created fail !");
		ret = CALENDAR_ERROR_OUT_OF_MEMORY;
		return ret;
	}
	ret = cal_ipc_marshal_char(view_uri,indata);
	if (ret != CALENDAR_ERROR_NONE)
	{
		ERR("marshal fail");
		CAL_IPC_DATA_FREE(indata);
		return ret;
	}
	ret = cal_ipc_marshal_int(original_event_id,indata);
	if (ret != CALENDAR_ERROR_NONE)
	{
		ERR("marshal fail");
		CAL_IPC_DATA_FREE(indata);
		return ret;
	}
	ret = cal_ipc_marshal_int(calendar_db_version,indata);
	if (ret != CALENDAR_ERROR_NONE)
	{
		ERR("marshal fail");
		CAL_IPC_DATA_FREE(indata);
		return ret;
	}

	// ipc call
	if (cal_client_ipc_call(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_CHANGES_EXCEPTION, indata, &outdata) != 0)
	{
		ERR("pims_ipc_call failed");
		CAL_IPC_DATA_FREE(indata);
		return CALENDAR_ERROR_IPC;
	}

	CAL_IPC_DATA_FREE(indata);

	if (outdata)
	{
		// check outdata
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata,&size);

		if (CALENDAR_ERROR_NONE == ret)
		{
			ret = cal_ipc_unmarshal_list(outdata,record_list);
		}

		pims_ipc_data_destroy(outdata);
	}
	else
	{
		ERR("ipc outdata is NULL");
		return CALENDAR_ERROR_IPC;
	}

	return ret;
}

#ifdef CAL_MEMORY_TEST
API int calendar_destroy(void)
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
		CAL_IPC_DATA_FREE(indata);
		return CALENDAR_ERROR_IPC;
	}

	CAL_IPC_DATA_FREE(indata);

	if (outdata)
	{
		// check outdata
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata,&size);

		pims_ipc_data_destroy(outdata);
	}
	else
	{
		ERR("ipc outdata is NULL");
		return CALENDAR_ERROR_IPC;
	}

	return ret;
}
#endif //#ifdef CAL_MEMORY_TEST
