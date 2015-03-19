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

#include <stdlib.h>
#include <malloc.h>		// malloc_trim
#include <pims-ipc-svc.h>

#include "calendar_service.h"
#include "calendar_db.h"
#include "calendar_query.h"
#include "calendar_vcalendar.h"

#include "cal_typedef.h"
#include "cal_db.h"
#include "cal_db_util.h"
#include "cal_ipc_marshal.h"
#include "cal_internal.h"
#include "cal_server_ipc.h"
#include "cal_access_control.h"

void cal_server_ipc_connect(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CALENDAR_ERROR_NONE;

	ret = calendar_connect();
	if (ret == CALENDAR_ERROR_NONE) {
		char *smack_label = NULL;
		if (0 != pims_ipc_svc_get_smack_label(ipc, &smack_label))
			ERR("pims_ipc_svc_get_smack_label() Fail");
		cal_access_control_set_client_info(ipc, smack_label);
	}

	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (!*outdata) {
			ERR("pims_ipc_data_create fail");
			return;
		}

		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0) {
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("pims_ipc_data_put fail");
			return;
		}
	} else {
		ERR("outdata is NULL");
	}
}

void cal_server_ipc_disconnect(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CALENDAR_ERROR_NONE;

	ret = calendar_disconnect();

	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			ERR("pims_ipc_data_create fail");
			return;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("pims_ipc_data_put fail");
			return;
		}
	}
	else
	{
		ERR("outdata is NULL");
	}
	return;
}

void cal_server_ipc_check_permission(pims_ipc_h ipc, pims_ipc_data_h indata,
		pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CALENDAR_ERROR_NONE;
	int permission;
	bool result;

	if (NULL == indata) {
		ret = CALENDAR_ERROR_INVALID_PARAMETER;
		ERR("check permission fail.");
		goto ERROR_RETURN;
	}

	ret = cal_ipc_unmarshal_int(indata, &permission);
	if (ret != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_unmarshal_int fail");
		goto ERROR_RETURN;
	}

	result = cal_access_control_have_permission(ipc, permission);

ERROR_RETURN:
	*outdata = pims_ipc_data_create(0);
	if (!*outdata) {
		ERR("pims_ipc_data_create fail");
		return;
	}

	if (pims_ipc_data_put(*outdata, (void*)&ret, sizeof(int)) != 0) {
		pims_ipc_data_destroy(*outdata);
		*outdata = NULL;
		ERR("pims_ipc_data_put fail (return value)");
		return;
	}
	if (CALENDAR_ERROR_NONE == ret) {
		if (pims_ipc_data_put(*outdata, (void*)&result, sizeof(bool)) != 0) {
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("pims_ipc_data_put fail");
			return;
		}
	}
}

void cal_server_ipc_db_insert_record(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CALENDAR_ERROR_NONE;
	calendar_record_h record = NULL;
	int id = 0;

	if (indata) {
		ret = cal_ipc_unmarshal_record(indata,&record);
		if (ret != CALENDAR_ERROR_NONE) {
			ERR("_cal_ipc_unmarshal_record fail");
			record = NULL;
			goto ERROR_RETURN;
		}
	} else {
		ret = CALENDAR_ERROR_INVALID_PARAMETER;
		ERR("cal_server_ipc_db_insert_record fail");
		goto ERROR_RETURN;
	}
	if (false == cal_access_control_have_permission(ipc, CAL_PERMISSION_WRITE)) {
		ret = CALENDAR_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
	}

	ret = calendar_db_insert_record(record, &id);

	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}
		if (CALENDAR_ERROR_NONE == ret)
		{
			int transaction_ver = cal_db_util_get_transaction_ver();
			if (cal_ipc_marshal_int(transaction_ver,*outdata) != CALENDAR_ERROR_NONE)
			{
				pims_ipc_data_destroy(*outdata);
				*outdata = NULL;
				ERR("cal_ipc_marshal fail");
				ret = CALENDAR_ERROR_OUT_OF_MEMORY;
				goto ERROR_RETURN;
			}
			if (cal_ipc_marshal_int(id,*outdata) != CALENDAR_ERROR_NONE)
			{
				pims_ipc_data_destroy(*outdata);
				*outdata = NULL;
				ERR("cal_ipc_marshal fail");
				ret = CALENDAR_ERROR_OUT_OF_MEMORY;
				goto ERROR_RETURN;
			}
		}
	}
	else
	{
		ERR("outdata is NULL");
	}
	goto DATA_FREE;

ERROR_RETURN:
	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}
	}
	else
	{
		ERR("outdata is NULL");
	}

DATA_FREE:
	if (record)
	{
		calendar_record_destroy(record,true);
	}
	return;
}

void cal_server_ipc_db_get_record(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CALENDAR_ERROR_NONE;
	char* view_uri = NULL;
	int id = 0;
	calendar_record_h record = NULL;

	if (indata)
	{
		ret = cal_ipc_unmarshal_char(indata,&view_uri);
		if (ret != CALENDAR_ERROR_NONE)
		{
			ERR("cal_ipc_unmarshal_char fail");
			goto ERROR_RETURN;
		}
		ret = cal_ipc_unmarshal_int(indata,&id);
		if (ret != CALENDAR_ERROR_NONE)
		{
			ERR("cal_ipc_unmarshal_int fail");
			goto ERROR_RETURN;
		}
	}
	else
	{
		ret = CALENDAR_ERROR_INVALID_PARAMETER;
		ERR("cal_server_ipc_db_insert_record fail");
		goto ERROR_RETURN;
	}

	if (false == cal_access_control_have_permission(ipc, CAL_PERMISSION_READ)) {
		ret = CALENDAR_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
	}

	ret = calendar_db_get_record(view_uri,id,&record);

ERROR_RETURN:
	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}
		if (CALENDAR_ERROR_NONE == ret) {
			if (cal_ipc_marshal_record(record, *outdata) != CALENDAR_ERROR_NONE) {
				pims_ipc_data_destroy(*outdata);
				*outdata = NULL;
				ERR("cal_ipc_marshal_record fail");
				goto DATA_FREE;
			}
		}
	}
	else
	{
		ERR("outdata is NULL");
	}
DATA_FREE:
	if (record)
	{
		calendar_record_destroy(record,true);
	}
	CAL_FREE(view_uri);
	return;
}

void cal_server_ipc_db_update_record(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CALENDAR_ERROR_NONE;
	calendar_record_h record = NULL;

	if (indata)
	{
		ret = cal_ipc_unmarshal_record(indata,&record);
		if (ret != CALENDAR_ERROR_NONE)
		{
			ERR("cal_ipc_unmarshal_record fail");
			goto ERROR_RETURN;
		}
	}
	else
	{
		ret = CALENDAR_ERROR_INVALID_PARAMETER;
		ERR("cal_server_ipc_db_insert_record fail");
		goto ERROR_RETURN;
	}

	if (false == cal_access_control_have_permission(ipc, CAL_PERMISSION_WRITE)) {
		ret = CALENDAR_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
	}

	ret = calendar_db_update_record(record);

ERROR_RETURN:
	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}
		if (CALENDAR_ERROR_NONE == ret)
		{
			int transaction_ver = cal_db_util_get_transaction_ver();
			if (cal_ipc_marshal_int(transaction_ver,*outdata) != CALENDAR_ERROR_NONE)
			{
				pims_ipc_data_destroy(*outdata);
				ERR("cal_ipc_marshal fail");
				ret = CALENDAR_ERROR_OUT_OF_MEMORY;
				goto DATA_FREE;
			}
		}
	}
	else
	{
		ERR("outdata is NULL");
	}
DATA_FREE:
	if (record)
	{
		calendar_record_destroy(record,true);
	}
	return;
}

void cal_server_ipc_db_delete_record(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CALENDAR_ERROR_NONE;
	char* view_uri = NULL;
	int id = 0;

	if (indata)
	{
		ret = cal_ipc_unmarshal_char(indata,&view_uri);
		if (ret != CALENDAR_ERROR_NONE)
		{
			ERR("cal_ipc_unmarshal_record fail");
			goto ERROR_RETURN;
		}
		ret = cal_ipc_unmarshal_int(indata,&id);
		if (ret != CALENDAR_ERROR_NONE)
		{
			ERR("cal_ipc_unmarshal_int fail");
			goto ERROR_RETURN;
		}
	}
	else
	{
		ret = CALENDAR_ERROR_INVALID_PARAMETER;
		ERR("cal_server_ipc_db_insert_record fail");
		goto ERROR_RETURN;
	}

	if (false == cal_access_control_have_permission(ipc, CAL_PERMISSION_WRITE)) {
		ret = CALENDAR_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
	}

	ret = calendar_db_delete_record(view_uri,id);

ERROR_RETURN:
	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}
		if (CALENDAR_ERROR_NONE == ret)
		{
			int transaction_ver = cal_db_util_get_transaction_ver();
			if (cal_ipc_marshal_int(transaction_ver,*outdata) != CALENDAR_ERROR_NONE)
			{
				pims_ipc_data_destroy(*outdata);
				ERR("cal_ipc_marshal fail");
				ret = CALENDAR_ERROR_OUT_OF_MEMORY;
				goto DATA_FREE;
			}
		}
	}
	else
	{
		ERR("outdata is NULL");
	}
DATA_FREE:

	CAL_FREE(view_uri);
	return;
}

void cal_server_ipc_db_get_all_records(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CALENDAR_ERROR_NONE;
	char* view_uri = NULL;
	int offset = 0;
	int limit = 0;
	calendar_list_h list = NULL;

	if (indata)
	{
		ret = cal_ipc_unmarshal_char(indata,&view_uri);
		if (ret != CALENDAR_ERROR_NONE)
		{
			ERR("cal_ipc_unmarshal_record fail");
			goto ERROR_RETURN;
		}
		ret = cal_ipc_unmarshal_int(indata,&offset);
		if (ret != CALENDAR_ERROR_NONE)
		{
			ERR("cal_ipc_unmarshal_int fail");
			goto ERROR_RETURN;
		}
		ret = cal_ipc_unmarshal_int(indata,&limit);
		if (ret != CALENDAR_ERROR_NONE)
		{
			ERR("cal_ipc_unmarshal_int fail");
			goto ERROR_RETURN;
		}
	}
	else
	{
		ret = CALENDAR_ERROR_INVALID_PARAMETER;
		ERR("cal_server_ipc_db_insert_record fail");
		goto ERROR_RETURN;
	}

	if (false == cal_access_control_have_permission(ipc, CAL_PERMISSION_READ)) {
		ret = CALENDAR_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
	}

	ret = calendar_db_get_all_records(view_uri,offset,limit,&list);

	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}
		if (ret != CALENDAR_ERROR_NONE)
		{
			ERR("cal_ipc_unmarshal_int fail");
			goto DATA_FREE;
		}
		ret = cal_ipc_marshal_list(list,*outdata);
		if (ret != CALENDAR_ERROR_NONE)
		{
			ERR("cal_ipc_unmarshal_int fail");
			goto ERROR_RETURN;
		}
	}
	else
	{
		ERR("outdata is NULL");
	}

	goto DATA_FREE;

ERROR_RETURN:
	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}
	}
	else
	{
		ERR("outdata is NULL");
	}
DATA_FREE:

	if (list)
	{
		calendar_list_destroy(list,true);
	}
	CAL_FREE(view_uri);
	return;
}

void cal_server_ipc_db_get_records_with_query(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CALENDAR_ERROR_NONE;
	calendar_query_h query = NULL;
	int offset = 0;
	int limit = 0;
	calendar_list_h list = NULL;

	if (indata)
	{
		ret = cal_ipc_unmarshal_query(indata,&query);
		if (ret != CALENDAR_ERROR_NONE)
		{
			ERR("cal_ipc_unmarshal_record fail");
			goto ERROR_RETURN;
		}
		ret = cal_ipc_unmarshal_int(indata,&offset);
		if (ret != CALENDAR_ERROR_NONE)
		{
			ERR("cal_ipc_unmarshal_int fail");
			goto ERROR_RETURN;
		}
		ret = cal_ipc_unmarshal_int(indata,&limit);
		if (ret != CALENDAR_ERROR_NONE)
		{
			ERR("cal_ipc_unmarshal_int fail");
			goto ERROR_RETURN;
		}
	}
	else
	{
		ret = CALENDAR_ERROR_INVALID_PARAMETER;
		ERR("cal_server_ipc_db_insert_record fail");
		goto ERROR_RETURN;
	}

	if (false == cal_access_control_have_permission(ipc, CAL_PERMISSION_READ)) {
		ret = CALENDAR_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
	}

	ret = calendar_db_get_records_with_query(query,offset,limit,&list);

	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}
		if (ret != CALENDAR_ERROR_NONE)
		{
			ERR("cal_ipc_unmarshal_int fail");
			goto DATA_FREE;
		}
		ret = cal_ipc_marshal_list(list,*outdata);
		if (ret != CALENDAR_ERROR_NONE)
		{
			ERR("cal_ipc_unmarshal_int fail");
			goto DATA_FREE;
		}
	}
	else
	{
		ERR("outdata is NULL");
	}
	goto DATA_FREE;

ERROR_RETURN:
	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}
	}
	else
	{
		ERR("outdata is NULL");
	}
DATA_FREE:

	if (list)
	{
		calendar_list_destroy(list,true);
	}
	if (query)
	{
		calendar_query_destroy(query);
	}
	return;
}

void cal_server_ipc_db_clean_after_sync(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CALENDAR_ERROR_NONE;
	int calendar_book_id = 0;
	int calendar_db_version = 0;

	if (indata)
	{
		ret = cal_ipc_unmarshal_int(indata,&calendar_book_id);
		if (ret != CALENDAR_ERROR_NONE)
		{
			ERR("cal_ipc_unmarshal_int fail");
			goto ERROR_RETURN;
		}
		ret = cal_ipc_unmarshal_int(indata,&calendar_db_version);
		if (ret != CALENDAR_ERROR_NONE)
		{
			ERR("cal_ipc_unmarshal_int fail");
			goto ERROR_RETURN;
		}
	}
	else
	{
		ret = CALENDAR_ERROR_INVALID_PARAMETER;
		ERR("cal_server_ipc_db_insert_record fail");
		goto ERROR_RETURN;
	}

	if (false == cal_access_control_have_permission(ipc, CAL_PERMISSION_WRITE)) {
		ret = CALENDAR_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
	}

	ret = calendar_db_clean_after_sync(calendar_book_id, calendar_db_version);

ERROR_RETURN:
	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			ERR("pims_ipc_data_create fail");
			return;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("pims_ipc_data_put fail");
			return;
		}
	}
	else
	{
		ERR("outdata is NULL");
	}

	return;
}

void cal_server_ipc_db_get_count(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CALENDAR_ERROR_NONE;
	char* view_uri = NULL;
	int count = 0;

	if (indata)
	{
		ret = cal_ipc_unmarshal_char(indata,&view_uri);
		if (ret != CALENDAR_ERROR_NONE)
		{
			ERR("cal_ipc_unmarshal_record fail");
			goto ERROR_RETURN;
		}
	}
	else
	{
		ret = CALENDAR_ERROR_INVALID_PARAMETER;
		ERR("cal_server_ipc_db_insert_record fail");
		goto ERROR_RETURN;
	}

	if (false == cal_access_control_have_permission(ipc, CAL_PERMISSION_READ)) {
		ret = CALENDAR_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
	}

	ret = calendar_db_get_count(view_uri,&count);

	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}
		if (ret != CALENDAR_ERROR_NONE)
		{
			goto DATA_FREE;
		}
		ret = cal_ipc_marshal_int(count,*outdata);
		if (ret != CALENDAR_ERROR_NONE)
		{
			ERR("cal_ipc_unmarshal_int fail");
			goto ERROR_RETURN;
		}
	}
	else
	{
		ERR("outdata is NULL");
	}
	goto DATA_FREE;

ERROR_RETURN:
	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}
	}
	else
	{
		ERR("outdata is NULL");
	}
DATA_FREE:
	CAL_FREE(view_uri);
	return;
}

void cal_server_ipc_db_get_count_with_query(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CALENDAR_ERROR_NONE;
	calendar_query_h query = NULL;
	int count = 0;

	if (indata)
	{
		ret = cal_ipc_unmarshal_query(indata,&query);
		if (ret != CALENDAR_ERROR_NONE)
		{
			ERR("cal_ipc_unmarshal_record fail");
			goto ERROR_RETURN;
		}
	}
	else
	{
		ret = CALENDAR_ERROR_INVALID_PARAMETER;
		ERR("cal_server_ipc_db_insert_record fail");
		goto ERROR_RETURN;
	}

	if (false == cal_access_control_have_permission(ipc, CAL_PERMISSION_READ)) {
		ret = CALENDAR_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
	}

	ret = calendar_db_get_count_with_query(query,&count);

	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}
		if (CALENDAR_ERROR_NONE != ret) {
			goto DATA_FREE;
		}
		ret = cal_ipc_marshal_int(count,*outdata);
		if (ret != CALENDAR_ERROR_NONE)
		{
			ERR("cal_ipc_unmarshal_int fail");
			goto ERROR_RETURN;
		}
	}
	else
	{
		ERR("outdata is NULL");
	}
	goto DATA_FREE;

ERROR_RETURN:
	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}
	}
	else
	{
		ERR("outdata is NULL");
	}
DATA_FREE:
	if (query)
	{
		calendar_query_destroy(query);
	}
	return;
}

void cal_server_ipc_db_insert_records(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CALENDAR_ERROR_NONE;
	calendar_list_h list = NULL;
	int id_count = 0;
	int *ids = NULL;
	int i=0;

	if (indata)
	{
		ret = cal_ipc_unmarshal_list(indata,&list);
		if (ret != CALENDAR_ERROR_NONE)
		{
			ERR("cal_ipc_unmarshal_list fail");
			goto ERROR_RETURN;
		}
	}
	else
	{
		ret = CALENDAR_ERROR_INVALID_PARAMETER;
		ERR("cal_server_ipc_db_insert_record fail");
		goto ERROR_RETURN;
	}

	if (false == cal_access_control_have_permission(ipc, CAL_PERMISSION_WRITE)) {
		ret = CALENDAR_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
	}

	ret = calendar_db_insert_records(list, &ids, &id_count);

	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}
		if(CALENDAR_ERROR_NONE == ret)
		{
			int transaction_ver = cal_db_util_get_transaction_ver();
			if (cal_ipc_marshal_int(transaction_ver,*outdata) != CALENDAR_ERROR_NONE)
			{
				pims_ipc_data_destroy(*outdata);
				ERR("cal_ipc_marshal fail");
				ret = CALENDAR_ERROR_OUT_OF_MEMORY;
				goto ERROR_RETURN;
			}
			// marshal : id_count+[ids]*id_count
			// id_count
			if (pims_ipc_data_put(*outdata,(void*)&id_count,sizeof(int)) != 0)
			{
				pims_ipc_data_destroy(*outdata);
				*outdata = NULL;
				ERR("pims_ipc_data_put fail");
				ret = CALENDAR_ERROR_INVALID_PARAMETER;
				goto ERROR_RETURN;
			}

			for(i=0;i<id_count;i++)
			{
				// marshal ids
				if (pims_ipc_data_put(*outdata,(void*)&ids[i],sizeof(int)) != 0)
				{
					pims_ipc_data_destroy(*outdata);
					*outdata = NULL;
					ERR("pims_ipc_data_put fail");
					ret = CALENDAR_ERROR_INVALID_PARAMETER;
					goto ERROR_RETURN;
				}
			}
		}
	}
	else
	{
		ERR("outdata is NULL");
	}
	goto DATA_FREE;

ERROR_RETURN:
	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}
	}
	else
	{
		ERR("outdata is NULL");
	}
DATA_FREE:
	if (list)
	{
		calendar_list_destroy(list,true);
	}
	CAL_FREE(ids);
	return;
}

void cal_server_ipc_db_update_records(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CALENDAR_ERROR_NONE;
	calendar_list_h list = NULL;

	if (indata)
	{
		ret = cal_ipc_unmarshal_list(indata,&list);
		if (ret != CALENDAR_ERROR_NONE)
		{
			ERR("cal_ipc_unmarshal_list fail");
			goto ERROR_RETURN;
		}
	}
	else
	{
		ret = CALENDAR_ERROR_INVALID_PARAMETER;
		ERR("cal_server_ipc_db_insert_record fail");
		goto ERROR_RETURN;
	}

	if (false == cal_access_control_have_permission(ipc, CAL_PERMISSION_WRITE)) {
		ret = CALENDAR_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
	}

	ret = calendar_db_update_records(list);

	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}
		if (CALENDAR_ERROR_NONE == ret)
		{
			int transaction_ver = cal_db_util_get_transaction_ver();
			if (cal_ipc_marshal_int(transaction_ver,*outdata) != CALENDAR_ERROR_NONE)
			{
				pims_ipc_data_destroy(*outdata);
				ERR("cal_ipc_marshal fail");
				ret = CALENDAR_ERROR_OUT_OF_MEMORY;
				goto DATA_FREE;
			}
		}
	}
	else
	{
		ERR("outdata is NULL");
	}
	goto DATA_FREE;

ERROR_RETURN:
	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}
	}
	else
	{
		ERR("outdata is NULL");
	}
DATA_FREE:
	if (list)
	{
		calendar_list_destroy(list,true);
	}
	return;
}

void cal_server_ipc_db_delete_records(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CALENDAR_ERROR_NONE;
	int count = 0;
	int *ids = NULL;
	char *uri = NULL;
	int i = 0;

	if (indata)
	{
		ret = cal_ipc_unmarshal_char(indata,&uri);
		if (ret != CALENDAR_ERROR_NONE)
		{
			ERR("cal_ipc_unmarshal_char fail");
			goto ERROR_RETURN;
		}
		ret = cal_ipc_unmarshal_int(indata,&count);
		if (ret != CALENDAR_ERROR_NONE)
		{
			ERR("cal_ipc_unmarshal_int fail");
			goto ERROR_RETURN;
		}
		if (count <=0)
		{
			goto ERROR_RETURN;
		}
		ids = (int*)malloc(sizeof(int)*count);
		for(i=0;i<count;i++)
		{
			ret = cal_ipc_unmarshal_int(indata,&ids[i]);
			if (ret != CALENDAR_ERROR_NONE)
			{
				ERR("cal_ipc_unmarshal_int fail");
				goto ERROR_RETURN;
			}
		}
	}
	else
	{
		ret = CALENDAR_ERROR_INVALID_PARAMETER;
		ERR("cal_server_ipc_db_insert_record fail");
		goto ERROR_RETURN;
	}

	if (false == cal_access_control_have_permission(ipc, CAL_PERMISSION_WRITE)) {
		ret = CALENDAR_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
	}

	ret = calendar_db_delete_records(uri,ids,count);

	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}
		if (CALENDAR_ERROR_NONE == ret)
		{
			int transaction_ver = cal_db_util_get_transaction_ver();
			if (cal_ipc_marshal_int(transaction_ver,*outdata) != CALENDAR_ERROR_NONE)
			{
				pims_ipc_data_destroy(*outdata);
				ERR("cal_ipc_marshal fail");
				ret = CALENDAR_ERROR_OUT_OF_MEMORY;
				goto DATA_FREE;
			}
		}
	}
	else
	{
		ERR("outdata is NULL");
	}
	goto DATA_FREE;

ERROR_RETURN:
	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}
	}
	else
	{
		ERR("outdata is NULL");
	}
DATA_FREE:
	CAL_FREE(uri);
	CAL_FREE(ids);
	return;
}

void cal_server_ipc_db_get_changes_by_version(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CALENDAR_ERROR_NONE;
	char* view_uri = NULL;
	int calendar_book_id = 0;
	int calendar_db_version = 0;
	calendar_list_h record_list = NULL;
	int current_calendar_db_version = 0;

	if (indata)
	{
		ret = cal_ipc_unmarshal_char(indata,&view_uri);
		if (ret != CALENDAR_ERROR_NONE)
		{
			ERR("cal_ipc_unmarshal_char fail");
			goto ERROR_RETURN;
		}
		ret = cal_ipc_unmarshal_int(indata,&calendar_book_id);
		if (ret != CALENDAR_ERROR_NONE)
		{
			ERR("cal_ipc_unmarshal_int fail");
			goto ERROR_RETURN;
		}
		ret = cal_ipc_unmarshal_int(indata,&calendar_db_version);
		if (ret != CALENDAR_ERROR_NONE)
		{
			ERR("cal_ipc_unmarshal_int fail");
			goto ERROR_RETURN;
		}
	}
	else
	{
		ret = CALENDAR_ERROR_INVALID_PARAMETER;
		ERR("cal_server_ipc_db_insert_record fail");
		goto ERROR_RETURN;
	}

	if (false == cal_access_control_have_permission(ipc, CAL_PERMISSION_READ)) {
		ret = CALENDAR_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
	}

	ret = calendar_db_get_changes_by_version(view_uri,calendar_book_id,calendar_db_version,&record_list,&current_calendar_db_version);

	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}
		if (CALENDAR_ERROR_NONE != ret) {
			goto DATA_FREE;
		}
		ret = cal_ipc_marshal_list(record_list,*outdata);
		if (ret != CALENDAR_ERROR_NONE)
		{
			ERR("cal_ipc_marshal_list fail");
			goto ERROR_RETURN;
		}
		ret = cal_ipc_marshal_int(current_calendar_db_version,*outdata);
		if (ret != CALENDAR_ERROR_NONE)
		{
			ERR("cal_ipc_unmarshal_int fail");
			goto ERROR_RETURN;
		}
	}
	else
	{
		ERR("outdata is NULL");
	}
	goto DATA_FREE;

ERROR_RETURN:
	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}
	}
	else
	{
		ERR("outdata is NULL");
	}
DATA_FREE:
	if (record_list)
	{
		calendar_list_destroy(record_list,true);
	}
	CAL_FREE(view_uri);
	return;
}

void cal_server_ipc_db_get_current_version(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CALENDAR_ERROR_NONE;
	int calendar_db_version = 0;

	if (false == cal_access_control_have_permission(ipc, CAL_PERMISSION_READ)) {
		ret = CALENDAR_ERROR_PERMISSION_DENIED;
		return;
		goto ERROR_RETURN;
	}

	ret = calendar_db_get_current_version(&calendar_db_version);

ERROR_RETURN:
	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			ERR("pims_ipc_data_create fail");
			return;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("pims_ipc_data_put fail");
			return;
		}
		if (CALENDAR_ERROR_NONE == ret) {
			ret = cal_ipc_marshal_int(calendar_db_version,*outdata);
			if (ret != CALENDAR_ERROR_NONE) {
				ERR("cal_ipc_marshal_int fail");
				return;
			}
		}
	}
	else
	{
		ERR("outdata is NULL");
	}
	return;
}

void cal_server_ipc_db_insert_vcalendars(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CALENDAR_ERROR_NONE;
	char *stream = NULL;
	int count = 0;
	int *ids = NULL;
	int i = 0;

	if (indata)
	{
		ret = cal_ipc_unmarshal_char(indata, &stream);
		if (ret != CALENDAR_ERROR_NONE)
		{
			ERR("unmarshal fail");
			goto ERROR_RETURN;
		}
	}
	else
	{
		ret = CALENDAR_ERROR_INVALID_PARAMETER;
		ERR("db_insert_vcalendars fail.");
		goto ERROR_RETURN;
	}

	if (false == cal_access_control_have_permission(ipc, CAL_PERMISSION_WRITE)) {
		ret = CALENDAR_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
	}

	ret = calendar_db_insert_vcalendars(stream, &ids, &count);

	if (ret != CALENDAR_ERROR_NONE)
	{
		ERR("calendar_db_insert_vcalendars fail");
		goto ERROR_RETURN;
	}

	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		// return
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}
		if (CALENDAR_ERROR_NONE == ret)
		{
			int transaction_ver = cal_db_util_get_transaction_ver();
			if (cal_ipc_marshal_int(transaction_ver,*outdata) != CALENDAR_ERROR_NONE)
			{
				pims_ipc_data_destroy(*outdata);
				*outdata = NULL;
				ERR("cal_ipc_marshal fail");
				ret = CALENDAR_ERROR_OUT_OF_MEMORY;
				goto ERROR_RETURN;
			}
		}
		// count
		ret = cal_ipc_marshal_int(count,*outdata);
		if (ret != CALENDAR_ERROR_NONE)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("cal_ipc_marshal_list fail");
			goto ERROR_RETURN;
		}
		for(i=0;i<count;i++)
		{
			ret = cal_ipc_marshal_int(ids[i],*outdata);
			if (ret != CALENDAR_ERROR_NONE)
			{
				pims_ipc_data_destroy(*outdata);
				*outdata = NULL;
				ERR("cal_ipc_marshal_list fail");
				goto ERROR_RETURN;
			}
		}
	}
	else
	{
		ERR("outdata is NULL");
	}
	goto DATA_FREE;

ERROR_RETURN:
	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}
	}
	else
	{
		ERR("outdata is NULL");
	}
DATA_FREE:
	CAL_FREE(stream);
	CAL_FREE(ids);
	return;
}

void cal_server_ipc_db_replace_vcalendars(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CALENDAR_ERROR_NONE;
	char *stream = NULL;
	int count = 0;
	int *ids = NULL;
	int i = 0;

	if (indata)
	{
		ret = cal_ipc_unmarshal_char(indata, &stream);
		if (ret != CALENDAR_ERROR_NONE)
		{
			ERR("unmarshal fail");
			goto ERROR_RETURN;
		}
		ret = cal_ipc_unmarshal_int(indata, &count);
		if (ret != CALENDAR_ERROR_NONE)
		{
			ERR("unmarshal fail");
			goto ERROR_RETURN;
		}
		ids = (int*)malloc(sizeof(int)*count);
		if (ids == NULL)
		{
			ERR("malloc fail");
			goto ERROR_RETURN;
		}
		for(i=0;i<count;i++)
		{
			ret = cal_ipc_unmarshal_int(indata, &ids[i]);
			if (ret != CALENDAR_ERROR_NONE)
			{
				ERR("unmarshal fail");
				goto ERROR_RETURN;
			}
		}
	}
	else
	{
		ret = CALENDAR_ERROR_INVALID_PARAMETER;
		ERR("db_replace_vcalendars fail.");
		goto ERROR_RETURN;
	}

	if (false == cal_access_control_have_permission(ipc, CAL_PERMISSION_WRITE)) {
		ret = CALENDAR_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
	}

	ret = calendar_db_replace_vcalendars(stream, ids, count);

	if (ret != CALENDAR_ERROR_NONE)
	{
		ERR("calendar_db_insert_vcalendars fail");
		goto ERROR_RETURN;
	}

	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		// return
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}
		if (CALENDAR_ERROR_NONE == ret)
		{
			int transaction_ver = cal_db_util_get_transaction_ver();
			if (cal_ipc_marshal_int(transaction_ver,*outdata) != CALENDAR_ERROR_NONE)
			{
				pims_ipc_data_destroy(*outdata);
				*outdata = NULL;
				ERR("cal_ipc_marshal fail");
				ret = CALENDAR_ERROR_OUT_OF_MEMORY;
				goto DATA_FREE;
			}
		}
	}
	else
	{
		ERR("outdata is NULL");
	}
	goto DATA_FREE;

ERROR_RETURN:
	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}
	}
	else
	{
		ERR("outdata is NULL");
	}
DATA_FREE:
	CAL_FREE(stream);
	CAL_FREE(ids);
	return;
}

void cal_server_ipc_db_replace_record(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CALENDAR_ERROR_NONE;
	int id = 0;
	calendar_record_h record = NULL;

	if (indata)
	{
		ret = cal_ipc_unmarshal_record(indata,&record);
		if (ret != CALENDAR_ERROR_NONE)
		{
			ERR("cal_ipc_unmarshal_record fail");
			goto ERROR_RETURN;
		}
		ret = cal_ipc_unmarshal_int(indata,&id);
		if (ret != CALENDAR_ERROR_NONE)
		{
			ERR("cal_ipc_unmarshal_record fail");
			goto ERROR_RETURN;
		}
	}

	else
	{
		ret = CALENDAR_ERROR_INVALID_PARAMETER;
		ERR("cal_server_ipc_db_insert_record fail");
		goto ERROR_RETURN;
	}

	if (false == cal_access_control_have_permission(ipc, CAL_PERMISSION_WRITE)) {
		ret = CALENDAR_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
	}

	ret = calendar_db_replace_record(record, id);

ERROR_RETURN:
	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}
		if (CALENDAR_ERROR_NONE == ret)
		{
			int transaction_ver = cal_db_util_get_transaction_ver();
			if (cal_ipc_marshal_int(transaction_ver,*outdata) != CALENDAR_ERROR_NONE)
			{
				pims_ipc_data_destroy(*outdata);
				*outdata = NULL;
				ERR("cal_ipc_marshal fail");
				ret = CALENDAR_ERROR_OUT_OF_MEMORY;
				goto DATA_FREE;
			}
		}
	}
	else
	{
		ERR("outdata is NULL");
	}
DATA_FREE:
	if (record)
	{
		calendar_record_destroy(record,true);
	}
	return;
}

void cal_server_ipc_db_replace_records(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CALENDAR_ERROR_NONE;
	int i;
	int count = 0;
	int *ids = NULL;
	calendar_list_h list = NULL;

	if (indata)
	{
		ret = cal_ipc_unmarshal_list(indata,&list);
		if (ret != CALENDAR_ERROR_NONE)
		{
			ERR("cal_ipc_unmarshal_list fail");
			goto ERROR_RETURN;
		}
		ret = cal_ipc_unmarshal_int(indata, &count);
		if (ret != CALENDAR_ERROR_NONE)
		{
			ERR("cal_ipc_unmarshal_int fail");
			goto ERROR_RETURN;
		}
		if (count <= 0)
		{
			goto ERROR_RETURN;
		}
		ids = (int*)malloc(sizeof(int) * count);
		for(i = 0; i < count; i++)
		{
			ret = cal_ipc_unmarshal_int(indata,&ids[i]);
			if (ret != CALENDAR_ERROR_NONE)
			{
				ERR("cal_ipc_unmarshal_int fail");
				goto ERROR_RETURN;
			}
		}
	}
	else
	{
		ret = CALENDAR_ERROR_INVALID_PARAMETER;
		ERR("cal_server_ipc_db_insert_record fail");
		goto ERROR_RETURN;
	}

	if (false == cal_access_control_have_permission(ipc, CAL_PERMISSION_WRITE)) {
		ret = CALENDAR_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
	}

	ret = calendar_db_replace_records(list, ids, count);

	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}
		if (CALENDAR_ERROR_NONE == ret)
		{
			int transaction_ver = cal_db_util_get_transaction_ver();
			if (cal_ipc_marshal_int(transaction_ver,*outdata) != CALENDAR_ERROR_NONE)
			{
				pims_ipc_data_destroy(*outdata);
				*outdata = NULL;
				ERR("cal_ipc_marshal fail");
				ret = CALENDAR_ERROR_OUT_OF_MEMORY;
				goto DATA_FREE;
			}
		}
	}
	else
	{
		ERR("outdata is NULL");
	}
	goto DATA_FREE;

ERROR_RETURN:
	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}
	}
	else
	{
		ERR("outdata is NULL");
	}
DATA_FREE:
	CAL_FREE(ids);
	if (list)
	{
		calendar_list_destroy(list,true);
	}
	return;
}

void cal_server_ipc_db_changes_exception(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CALENDAR_ERROR_NONE;
	char* view_uri = NULL;
	int original_event_id = 0;
	int calendar_db_version = 0;
	calendar_list_h record_list = NULL;

	if (indata)
	{
		ret = cal_ipc_unmarshal_char(indata,&view_uri);
		if (ret != CALENDAR_ERROR_NONE)
		{
			ERR("cal_ipc_unmarshal_char fail");
			goto ERROR_RETURN;
		}
		ret = cal_ipc_unmarshal_int(indata,&original_event_id);
		if (ret != CALENDAR_ERROR_NONE)
		{
			ERR("cal_ipc_unmarshal_int fail");
			goto ERROR_RETURN;
		}
		ret = cal_ipc_unmarshal_int(indata,&calendar_db_version);
		if (ret != CALENDAR_ERROR_NONE)
		{
			ERR("cal_ipc_unmarshal_int fail");
			goto ERROR_RETURN;
		}
	}
	else
	{
		ret = CALENDAR_ERROR_INVALID_PARAMETER;
		ERR("cal_server_ipc_db_insert_record fail");
		goto ERROR_RETURN;
	}

	if (false == cal_access_control_have_permission(ipc, CAL_PERMISSION_READ)) {
		ret = CALENDAR_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
	}

	ret = calendar_db_get_changes_exception_by_version(view_uri,original_event_id,calendar_db_version,&record_list);

	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}
		if (CALENDAR_ERROR_NONE != ret) {
			goto DATA_FREE;
		}
		ret = cal_ipc_marshal_list(record_list,*outdata);
		if (ret != CALENDAR_ERROR_NONE)
		{
			ERR("cal_ipc_marshal_list fail");
			goto ERROR_RETURN;
		}
	}
	else
	{
		ERR("outdata is NULL");
	}
	goto DATA_FREE;

ERROR_RETURN:
	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}
	}
	else
	{
		ERR("outdata is NULL");
	}
DATA_FREE:
	if (record_list)
	{
		calendar_list_destroy(record_list,true);
	}
	CAL_FREE(view_uri);
	return;
}
