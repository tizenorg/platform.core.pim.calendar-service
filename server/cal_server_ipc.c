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

#include <stdlib.h>
#include <malloc.h>
#include <glib.h>
#include <pims-ipc-data.h>
#include <pims-ipc-svc.h>

#include "calendar.h"
#include "cal_typedef.h"
#include "cal_internal.h"
#include "cal_db_util.h"
#include "cal_ipc.h"
#include "cal_ipc_marshal.h"
#include "cal_server_ipc.h"
#include "cal_access_control.h"
#include "cal_handle.h"
#include "cal_server_service.h"
#include "cal_db.h"
#include "cal_server.h"
#include "cal_server_ondemand.h"


static void _cal_server_ipc_return(pims_ipc_data_h *outdata, int ret)
{
	RET_IF(NULL == outdata);

	*outdata = pims_ipc_data_create(0);
	RETM_IF(NULL == *outdata, "pims_ipc_data_create() Fail");

	if (0 != pims_ipc_data_put(*outdata, (void *)&ret, sizeof(int))) {
		ERR("pims_ipc_data_put() Fail");
		pims_ipc_data_destroy(*outdata);
		*outdata = NULL;
	}
}

void cal_server_ipc_connect(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CALENDAR_ERROR_NONE;

	if (NULL == indata) {
		ERR("No indata");
		ret = CALENDAR_ERROR_INVALID_PARAMETER;
		_cal_server_ipc_return(outdata, ret);
		return;
	}

	calendar_h handle = NULL;
	ret = cal_ipc_unmarshal_handle(indata, &handle);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_handle() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		return;
	}

	ret = cal_connect();
	if (CALENDAR_ERROR_NONE == ret) {
		char *smack_label = NULL;
		if (0 != pims_ipc_svc_get_smack_label(ipc, &smack_label))
			ERR("pims_ipc_svc_get_smack_label() Fail");
		cal_access_control_set_client_info(ipc, smack_label);
	}
	_cal_server_ipc_return(outdata, ret);
	cal_handle_destroy(handle);
}

void cal_server_ipc_disconnect(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CALENDAR_ERROR_NONE;

	if (NULL == indata) {
		ERR("No indata");
		ret = CALENDAR_ERROR_INVALID_PARAMETER;
		_cal_server_ipc_return(outdata, ret);
		return;
	}

	calendar_h handle = NULL;
	ret = cal_ipc_unmarshal_handle(indata, &handle);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_handle() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		return;
	}

	ret = cal_disconnect();
	_cal_server_ipc_return(outdata, ret);
	cal_handle_destroy(handle);
	cal_server_ondemand_start();
}

void cal_server_ipc_check_permission(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CALENDAR_ERROR_NONE;

	if (NULL == indata) {
		ERR("No indata");
		ret = CALENDAR_ERROR_INVALID_PARAMETER;
		_cal_server_ipc_return(outdata, ret);
		return;
	}

	calendar_h handle = NULL;
	ret = cal_ipc_unmarshal_handle(indata, &handle);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_handle() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		return;
	}

	int permission = 0;
	ret = cal_ipc_unmarshal_int(indata, &permission);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		cal_handle_destroy(handle);
		return;
	}

	bool result = cal_access_control_have_permission(ipc, permission);
	_cal_server_ipc_return(outdata, ret);

	if (0 != pims_ipc_data_put(*outdata, (void *)&result, sizeof(bool))) {
		ERR("pims_ipc_data_put() Fail");
		pims_ipc_data_destroy(*outdata);
		*outdata = NULL;
		ret = CALENDAR_ERROR_OUT_OF_MEMORY;
		_cal_server_ipc_return(outdata, ret);
	}

	cal_handle_destroy(handle);
}

void cal_server_ipc_db_insert_record(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CALENDAR_ERROR_NONE;

	if (NULL == indata) {
		ERR("No indata");
		ret = CALENDAR_ERROR_INVALID_PARAMETER;
		_cal_server_ipc_return(outdata, ret);
		return;
	}

	calendar_h handle = NULL;
	ret = cal_ipc_unmarshal_handle(indata, &handle);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_handle() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		return;
	}

	calendar_record_h record = NULL;
	ret = cal_ipc_unmarshal_record(indata, &record);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_record() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		cal_handle_destroy(handle);
		return;
	}

	if (false == cal_access_control_have_permission(ipc, CAL_PERMISSION_WRITE)) {
		ret = CALENDAR_ERROR_PERMISSION_DENIED;
		_cal_server_ipc_return(outdata, ret);
		cal_handle_destroy(handle);
		return;
	}

	int id = 0;
	ret = cal_db_insert_record(record, &id);
	_cal_server_ipc_return(outdata, ret);

	int transaction_ver = 0;
	transaction_ver = cal_db_util_get_transaction_ver();
	ret = cal_ipc_marshal_int(transaction_ver, *outdata);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		pims_ipc_data_destroy(*outdata);
		*outdata = NULL;
		_cal_server_ipc_return(outdata, ret);
		calendar_record_destroy(record, true);
		cal_handle_destroy(handle);
		return;
	}

	ret = cal_ipc_marshal_int(id, *outdata);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_marshal_int() Fail(%d)", ret);
		pims_ipc_data_destroy(*outdata);
		*outdata = NULL;
		_cal_server_ipc_return(outdata, ret);
	}

	calendar_record_destroy(record, true);
	cal_handle_destroy(handle);
	cal_server_ondemand_start();
	return;
}

void cal_server_ipc_db_get_record(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CALENDAR_ERROR_NONE;

	if (NULL == indata) {
		ERR("No indata");
		ret = CALENDAR_ERROR_INVALID_PARAMETER;
		_cal_server_ipc_return(outdata, ret);
		return;
	}

	calendar_h handle = NULL;
	ret = cal_ipc_unmarshal_handle(indata, &handle);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_handle() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		return;
	}

	char* view_uri = NULL;
	ret = cal_ipc_unmarshal_char(indata, &view_uri);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_char() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		free(view_uri);
		cal_handle_destroy(handle);
		return;
	}

	int id = 0;
	ret = cal_ipc_unmarshal_int(indata, &id);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		free(view_uri);
		cal_handle_destroy(handle);
		return;
	}

	if (false == cal_access_control_have_permission(ipc, CAL_PERMISSION_READ)) {
		ret = CALENDAR_ERROR_PERMISSION_DENIED;
		_cal_server_ipc_return(outdata, ret);
		free(view_uri);
		cal_handle_destroy(handle);
		return;
	}

	calendar_record_h record = NULL;
	ret = cal_db_get_record(view_uri, id, &record);
	_cal_server_ipc_return(outdata, ret);

	if (CALENDAR_ERROR_NONE == ret) {
		ret = cal_ipc_marshal_record(record, *outdata);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_ipc_marshal_record() Fail(%d)", ret);
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			_cal_server_ipc_return(outdata, ret);
		}
	}

	calendar_record_destroy(record, true);
	free(view_uri);
	cal_handle_destroy(handle);
	cal_server_ondemand_start();
}

void cal_server_ipc_db_update_record(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CALENDAR_ERROR_NONE;

	if (NULL == indata) {
		ERR("No indata");
		ret = CALENDAR_ERROR_INVALID_PARAMETER;
		_cal_server_ipc_return(outdata, ret);
		return;
	}

	calendar_h handle = NULL;
	ret = cal_ipc_unmarshal_handle(indata, &handle);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_handle() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		return;
	}

	calendar_record_h record = NULL;
	ret = cal_ipc_unmarshal_record(indata, &record);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_record() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		cal_handle_destroy(handle);
		return;
	}

	if (false == cal_access_control_have_permission(ipc, CAL_PERMISSION_WRITE)) {
		ret = CALENDAR_ERROR_PERMISSION_DENIED;
		_cal_server_ipc_return(outdata, ret);
		cal_handle_destroy(handle);
		return;
	}

	ret = cal_db_update_record(record);
	_cal_server_ipc_return(outdata, ret);

	if (CALENDAR_ERROR_NONE == ret) {
		int transaction_ver = 0;
		transaction_ver = cal_db_util_get_transaction_ver();
		ret = cal_ipc_marshal_int(transaction_ver, *outdata);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_ipc_marshal_int() Fail(%d)", ret);
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			_cal_server_ipc_return(outdata, ret);
		}
	}

	calendar_record_destroy(record, true);
	cal_handle_destroy(handle);
	cal_server_ondemand_start();
}

void cal_server_ipc_db_delete_record(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CALENDAR_ERROR_NONE;

	if (NULL == indata) {
		ERR("No indata");
		ret = CALENDAR_ERROR_INVALID_PARAMETER;
		_cal_server_ipc_return(outdata, ret);
		return;
	}

	calendar_h handle = NULL;
	ret = cal_ipc_unmarshal_handle(indata, &handle);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_handle() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		return;
	}

	char* view_uri = NULL;
	ret = cal_ipc_unmarshal_char(indata, &view_uri);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_char() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		cal_handle_destroy(handle);
		return;
	}

	int id = 0;
	ret = cal_ipc_unmarshal_int(indata, &id);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		free(view_uri);
		cal_handle_destroy(handle);
		return;
	}

	if (false == cal_access_control_have_permission(ipc, CAL_PERMISSION_WRITE)) {
		ret = CALENDAR_ERROR_PERMISSION_DENIED;
		_cal_server_ipc_return(outdata, ret);
		free(view_uri);
		cal_handle_destroy(handle);
		return;
	}

	ret = cal_db_delete_record(view_uri,id);
	_cal_server_ipc_return(outdata, ret);

	if (CALENDAR_ERROR_NONE == ret) {
		int transaction_ver = 0;
		transaction_ver = cal_db_util_get_transaction_ver();
		ret = cal_ipc_marshal_int(transaction_ver, *outdata);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_ipc_marshal_int() Fail(%d)", ret);
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			_cal_server_ipc_return(outdata, ret);
		}
	}

	free(view_uri);
	cal_handle_destroy(handle);
	cal_server_ondemand_start();
}

void cal_server_ipc_db_get_all_records(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CALENDAR_ERROR_NONE;

	if (NULL == indata) {
		ERR("No indata");
		ret = CALENDAR_ERROR_INVALID_PARAMETER;
		_cal_server_ipc_return(outdata, ret);
		return;
	}

	calendar_h handle = NULL;
	ret = cal_ipc_unmarshal_handle(indata, &handle);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_handle() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		return;
	}

	char* view_uri = NULL;
	ret = cal_ipc_unmarshal_char(indata, &view_uri);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_char() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		cal_handle_destroy(handle);
		return;
	}

	int offset = 0;
	ret = cal_ipc_unmarshal_int(indata, &offset);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		free(view_uri);
		cal_handle_destroy(handle);
		return;
	}

	int limit = 0;
	ret = cal_ipc_unmarshal_int(indata, &limit);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		free(view_uri);
		cal_handle_destroy(handle);
		return;
	}

	if (false == cal_access_control_have_permission(ipc, CAL_PERMISSION_READ)) {
		ret = CALENDAR_ERROR_PERMISSION_DENIED;
		_cal_server_ipc_return(outdata, ret);
		free(view_uri);
		cal_handle_destroy(handle);
		return;
	}

	calendar_list_h list = NULL;
	ret = cal_db_get_all_records(view_uri, offset, limit, &list);
	_cal_server_ipc_return(outdata, ret);

	if (CALENDAR_ERROR_NONE == ret) {
		ret = cal_ipc_marshal_list(list, *outdata);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_ipc_marshal_list() Fail(%d)", ret);
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			_cal_server_ipc_return(outdata, ret);
		}
	}

	calendar_list_destroy(list, true);
	free(view_uri);
	cal_handle_destroy(handle);
	cal_server_ondemand_start();
}

void cal_server_ipc_db_get_records_with_query(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CALENDAR_ERROR_NONE;

	if (NULL == indata) {
		ERR("No indata");
		ret = CALENDAR_ERROR_INVALID_PARAMETER;
		_cal_server_ipc_return(outdata, ret);
		return;
	}

	calendar_h handle = NULL;
	ret = cal_ipc_unmarshal_handle(indata, &handle);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_handle() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		return;
	}

	calendar_query_h query = NULL;
	ret = cal_ipc_unmarshal_query(indata, &query);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_query() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		cal_handle_destroy(handle);
		return;
	}

	int offset = 0;
	ret = cal_ipc_unmarshal_int(indata, &offset);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		calendar_query_destroy(query);
		cal_handle_destroy(handle);
		return;
	}

	int limit = 0;
	ret = cal_ipc_unmarshal_int(indata, &limit);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		calendar_query_destroy(query);
		cal_handle_destroy(handle);
		return;
	}

	if (false == cal_access_control_have_permission(ipc, CAL_PERMISSION_READ)) {
		ret = CALENDAR_ERROR_PERMISSION_DENIED;
		_cal_server_ipc_return(outdata, ret);
		calendar_query_destroy(query);
		cal_handle_destroy(handle);
		return;
	}

	calendar_list_h list = NULL;
	ret = cal_db_get_records_with_query(query, offset, limit, &list);
	_cal_server_ipc_return(outdata, ret);

	if (CALENDAR_ERROR_NONE == ret) {
		ret = cal_ipc_marshal_list(list, *outdata);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_ipc_marshal_list() Fail(%d)", ret);
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ret = CALENDAR_ERROR_OUT_OF_MEMORY;
			_cal_server_ipc_return(outdata, ret);
		}
	}

	calendar_list_destroy(list, true);
	calendar_query_destroy(query);
	cal_handle_destroy(handle);
	cal_server_ondemand_start();
}

void cal_server_ipc_db_clean_after_sync(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CALENDAR_ERROR_NONE;

	if (NULL == indata) {
		ERR("No indata");
		ret = CALENDAR_ERROR_INVALID_PARAMETER;
		_cal_server_ipc_return(outdata, ret);
		return;
	}

	calendar_h handle = NULL;
	ret = cal_ipc_unmarshal_handle(indata, &handle);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_handle() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		return;
	}

	int calendar_book_id = 0;
	ret = cal_ipc_unmarshal_int(indata,&calendar_book_id);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		cal_handle_destroy(handle);
		return;
	}

	int calendar_db_version = 0;
	ret = cal_ipc_unmarshal_int(indata, &calendar_db_version);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		cal_handle_destroy(handle);
		return;
	}

	if (false == cal_access_control_have_permission(ipc, CAL_PERMISSION_WRITE)) {
		ret = CALENDAR_ERROR_PERMISSION_DENIED;
		_cal_server_ipc_return(outdata, ret);
		cal_handle_destroy(handle);
		return;
	}

	ret = cal_db_clean_after_sync(calendar_book_id, calendar_db_version);
	_cal_server_ipc_return(outdata, ret);
	cal_handle_destroy(handle);
	cal_server_ondemand_start();
}

void cal_server_ipc_db_get_count(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CALENDAR_ERROR_NONE;

	if (NULL == indata) {
		ERR("No indata");
		ret = CALENDAR_ERROR_INVALID_PARAMETER;
		_cal_server_ipc_return(outdata, ret);
		return;
	}

	calendar_h handle = NULL;
	ret = cal_ipc_unmarshal_handle(indata, &handle);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_handle() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		return;
	}

	char *view_uri = NULL;
	ret = cal_ipc_unmarshal_char(indata, &view_uri);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_char() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		cal_handle_destroy(handle);
		return;
	}

	if (false == cal_access_control_have_permission(ipc, CAL_PERMISSION_READ)) {
		ret = CALENDAR_ERROR_PERMISSION_DENIED;
		_cal_server_ipc_return(outdata, ret);
		cal_handle_destroy(handle);
		return;
	}

	int count = 0;
	ret = cal_db_get_count(view_uri, &count);
	_cal_server_ipc_return(outdata, ret);

	if (CALENDAR_ERROR_NONE == ret) {
		ret = cal_ipc_marshal_int(count, *outdata);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_ipc_marshal_int() Fail(%d)", ret);
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			_cal_server_ipc_return(outdata, ret);
		}
	}

	free(view_uri);
	cal_handle_destroy(handle);
	cal_server_ondemand_start();
}

void cal_server_ipc_db_get_count_with_query(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CALENDAR_ERROR_NONE;

	if (NULL == indata) {
		ERR("No indata");
		ret = CALENDAR_ERROR_INVALID_PARAMETER;
		_cal_server_ipc_return(outdata, ret);
		return;
	}

	calendar_h handle = NULL;
	ret = cal_ipc_unmarshal_handle(indata, &handle);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_handle() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		return;
	}

	calendar_query_h query = NULL;
	ret = cal_ipc_unmarshal_query(indata,&query);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_query() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		cal_handle_destroy(handle);
		return;
	}

	if (false == cal_access_control_have_permission(ipc, CAL_PERMISSION_READ)) {
		ret = CALENDAR_ERROR_PERMISSION_DENIED;
		_cal_server_ipc_return(outdata, ret);
		cal_handle_destroy(handle);
		return;
	}

	int count = 0;
	ret = cal_db_get_count_with_query(query, &count);
	_cal_server_ipc_return(outdata, ret);

	if (CALENDAR_ERROR_NONE == ret) {
		ret = cal_ipc_marshal_int(count, *outdata);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_ipc_marshal_int() Fail(%d)", ret);
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			_cal_server_ipc_return(outdata, ret);
		}
	}

	calendar_query_destroy(query);
	cal_handle_destroy(handle);
	cal_server_ondemand_start();
}

void cal_server_ipc_db_insert_records(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CALENDAR_ERROR_NONE;

	if (NULL == indata) {
		ERR("No indata");
		ret = CALENDAR_ERROR_INVALID_PARAMETER;
		_cal_server_ipc_return(outdata, ret);
		return;
	}

	calendar_h handle = NULL;
	ret = cal_ipc_unmarshal_handle(indata, &handle);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_handle() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		return;
	}

	calendar_list_h list = NULL;
	ret = cal_ipc_unmarshal_list(indata, &list);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_list() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		cal_handle_destroy(handle);
		return;
	}

	if (false == cal_access_control_have_permission(ipc, CAL_PERMISSION_WRITE)) {
		ret = CALENDAR_ERROR_PERMISSION_DENIED;
		_cal_server_ipc_return(outdata, ret);
		cal_handle_destroy(handle);
		return;
	}

	int *ids = NULL;
	int id_count = 0;
	ret = cal_db_insert_records(list, &ids, &id_count);
	_cal_server_ipc_return(outdata, ret);

	if (CALENDAR_ERROR_NONE == ret) {
		int transaction_ver = 0;
		transaction_ver = cal_db_util_get_transaction_ver();
		ret = cal_ipc_marshal_int(transaction_ver, *outdata);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_ipc_marshal_int() Fail(%d)", ret);
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			_cal_server_ipc_return(outdata, ret);
			free(ids);
			calendar_list_destroy(list, true);
			cal_handle_destroy(handle);
			return;
		}
		/*
		 * marshal : id_count+[ids]*id_count
		 * id_count
		 */
		if (pims_ipc_data_put(*outdata, (void*)&id_count, sizeof(int)) != 0) {
			ERR("pims_ipc_data_put() Fail");
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ret = CALENDAR_ERROR_INVALID_PARAMETER;
			_cal_server_ipc_return(outdata, ret);
			free(ids);
			calendar_list_destroy(list, true);
			cal_handle_destroy(handle);
			return;
		}

		/* marshal ids */
		int i=0;
		for(i = 0; i < id_count; i++) {
			if (pims_ipc_data_put(*outdata, (void*)&ids[i], sizeof(int)) != 0) {
				ERR("pims_ipc_data_put() Fail");
				pims_ipc_data_destroy(*outdata);
				*outdata = NULL;
				ret = CALENDAR_ERROR_INVALID_PARAMETER;
				_cal_server_ipc_return(outdata, ret);
				free(ids);
				calendar_list_destroy(list, true);
				cal_handle_destroy(handle);
				return;
			}
		}
	}

	free(ids);
	calendar_list_destroy(list, true);
	cal_handle_destroy(handle);
	cal_server_ondemand_start();
}

void cal_server_ipc_db_update_records(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CALENDAR_ERROR_NONE;

	if (NULL == indata) {
		ERR("No indata");
		ret = CALENDAR_ERROR_INVALID_PARAMETER;
		_cal_server_ipc_return(outdata, ret);
		return;
	}

	calendar_h handle = NULL;
	ret = cal_ipc_unmarshal_handle(indata, &handle);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_handle() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		return;
	}

	calendar_list_h list = NULL;
	ret = cal_ipc_unmarshal_list(indata, &list);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_list() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		cal_handle_destroy(handle);
		return;
	}

	if (false == cal_access_control_have_permission(ipc, CAL_PERMISSION_WRITE)) {
		ret = CALENDAR_ERROR_PERMISSION_DENIED;
		_cal_server_ipc_return(outdata, ret);
		cal_handle_destroy(handle);
		return;
	}

	ret = cal_db_update_records(list);
	_cal_server_ipc_return(outdata, ret);

	if (CALENDAR_ERROR_NONE == ret) {
		int transaction_ver = 0;
		transaction_ver = cal_db_util_get_transaction_ver();
		ret = cal_ipc_marshal_int(transaction_ver, *outdata);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_ipc_marshal_int() Fail(%d)", ret);
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			_cal_server_ipc_return(outdata, ret);
		}
	}

	calendar_list_destroy(list,true);
	cal_handle_destroy(handle);
	cal_server_ondemand_start();
}

void cal_server_ipc_db_delete_records(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CALENDAR_ERROR_NONE;

	if (NULL == indata) {
		ERR("No indata");
		ret = CALENDAR_ERROR_INVALID_PARAMETER;
		_cal_server_ipc_return(outdata, ret);
		return;
	}

	calendar_h handle = NULL;
	ret = cal_ipc_unmarshal_handle(indata, &handle);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_handle() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		return;
	}

	char *view_uri = NULL;
	ret = cal_ipc_unmarshal_char(indata, &view_uri);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_char() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		cal_handle_destroy(handle);
		return;
	}

	int count = 0;
	ret = cal_ipc_unmarshal_int(indata, &count);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		g_free(view_uri);
		cal_handle_destroy(handle);
		return;
	}

	if (count <= 0) {
		ERR("count(%d)", count);
		ret = CALENDAR_ERROR_NO_DATA;
		_cal_server_ipc_return(outdata, ret);
		g_free(view_uri);
		cal_handle_destroy(handle);
		return;
	}

	int *ids = NULL;
	ids = (int*)calloc(count, sizeof(int));
	if (NULL == ids) {
		ERR("calloc() Fail");
		ret = CALENDAR_ERROR_OUT_OF_MEMORY;
		_cal_server_ipc_return(outdata, ret);
		free(view_uri);
		cal_handle_destroy(handle);
		return;
	}

	int i = 0;
	for(i = 0; i < count; i++) {
		ret = cal_ipc_unmarshal_int(indata, &ids[i]);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
			ret = CALENDAR_ERROR_IPC;
			_cal_server_ipc_return(outdata, ret);
			free(ids);
			free(view_uri);
			cal_handle_destroy(handle);
			return;
		}
	}

	if (false == cal_access_control_have_permission(ipc, CAL_PERMISSION_WRITE)) {
		ret = CALENDAR_ERROR_PERMISSION_DENIED;
		_cal_server_ipc_return(outdata, ret);
		free(ids);
		free(view_uri);
		cal_handle_destroy(handle);
		return;
	}

	ret = cal_db_delete_records(view_uri, ids, count);
	_cal_server_ipc_return(outdata, ret);

	if (CALENDAR_ERROR_NONE == ret) {
		int transaction_ver = 0;
		transaction_ver = cal_db_util_get_transaction_ver();
		ret = cal_ipc_marshal_int(transaction_ver, *outdata);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_ipc_marshal_int() Fail(%d)", ret);
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ret = CALENDAR_ERROR_OUT_OF_MEMORY;
			_cal_server_ipc_return(outdata, ret);
		}
	}

	free(ids);
	free(view_uri);
	cal_handle_destroy(handle);
	cal_server_ondemand_start();
	return;
}

void cal_server_ipc_db_get_changes_by_version(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CALENDAR_ERROR_NONE;

	if (NULL == indata) {
		ERR("No indata");
		ret = CALENDAR_ERROR_INVALID_PARAMETER;
		_cal_server_ipc_return(outdata, ret);
		return;
	}

	calendar_h handle = NULL;
	ret = cal_ipc_unmarshal_handle(indata, &handle);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_handle() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		return;
	}

	char *view_uri = NULL;
	ret = cal_ipc_unmarshal_char(indata, &view_uri);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_char() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		cal_handle_destroy(handle);
		return;
	}

	int calendar_book_id = 0;
	ret = cal_ipc_unmarshal_int(indata, &calendar_book_id);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		free(view_uri);
		cal_handle_destroy(handle);
		return;
	}

	int calendar_db_version = 0;
	ret = cal_ipc_unmarshal_int(indata, &calendar_db_version);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		free(view_uri);
		cal_handle_destroy(handle);
		return;
	}

	if (false == cal_access_control_have_permission(ipc, CAL_PERMISSION_READ)) {
		ret = CALENDAR_ERROR_PERMISSION_DENIED;
		_cal_server_ipc_return(outdata, ret);
		free(view_uri);
		cal_handle_destroy(handle);
		return;
	}

	calendar_list_h list = NULL;
	int current_calendar_db_version = 0;
	ret = cal_db_get_changes_by_version(view_uri, calendar_book_id, calendar_db_version, &list, &current_calendar_db_version);
	_cal_server_ipc_return(outdata, ret);

	if (CALENDAR_ERROR_NONE == ret) {
		ret = cal_ipc_marshal_list(list,*outdata);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_ipc_marshal_list() Fail(%d)", ret);
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			_cal_server_ipc_return(outdata, ret);
			calendar_list_destroy(list, true);
			free(view_uri);
			cal_handle_destroy(handle);
			return;
		}

		ret = cal_ipc_marshal_int(current_calendar_db_version,*outdata);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_ipc_marshal_int() Fail(%d)", ret);
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			_cal_server_ipc_return(outdata, ret);
			calendar_list_destroy(list, true);
			free(view_uri);
			cal_handle_destroy(handle);
			return;
		}
	}

	calendar_list_destroy(list, true);
	free(view_uri);
	cal_handle_destroy(handle);
	cal_server_ondemand_start();
}

void cal_server_ipc_db_get_current_version(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CALENDAR_ERROR_NONE;

	if (NULL == indata) {
		ERR("No indata");
		ret = CALENDAR_ERROR_INVALID_PARAMETER;
		_cal_server_ipc_return(outdata, ret);
		return;
	}

	calendar_h handle = NULL;
	ret = cal_ipc_unmarshal_handle(indata, &handle);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_handle() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		return;
	}

	if (false == cal_access_control_have_permission(ipc, CAL_PERMISSION_READ)) {
		ret = CALENDAR_ERROR_PERMISSION_DENIED;
		_cal_server_ipc_return(outdata, ret);
		cal_handle_destroy(handle);
		return;
	}

	int calendar_db_version = 0;
	ret = cal_db_get_current_version(&calendar_db_version);
	_cal_server_ipc_return(outdata, ret);

	if (CALENDAR_ERROR_NONE == ret) {
		ret = cal_ipc_marshal_int(calendar_db_version, *outdata);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_ipc_marshal_int() Fail(%d)", ret);
			_cal_server_ipc_return(outdata, ret);
		}
	}
	cal_handle_destroy(handle);
	cal_server_ondemand_start();
	return;
}

void cal_server_ipc_db_insert_vcalendars(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CALENDAR_ERROR_NONE;

	if (NULL == indata) {
		ERR("No indata");
		ret = CALENDAR_ERROR_INVALID_PARAMETER;
		_cal_server_ipc_return(outdata, ret);
		return;
	}

	calendar_h handle = NULL;
	ret = cal_ipc_unmarshal_handle(indata, &handle);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_handle() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		return;
	}

	char *stream = NULL;
	ret = cal_ipc_unmarshal_char(indata, &stream);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_char() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		cal_handle_destroy(handle);
		return;
	}

	if (false == cal_access_control_have_permission(ipc, CAL_PERMISSION_WRITE)) {
		ret = CALENDAR_ERROR_PERMISSION_DENIED;
		_cal_server_ipc_return(outdata, ret);
		cal_handle_destroy(handle);
		return;
	}

	int count = 0;
	int *ids = NULL;
	ret = cal_db_insert_vcalendars(stream, &ids, &count);
	_cal_server_ipc_return(outdata, ret);

	if (CALENDAR_ERROR_NONE == ret) {
		int transaction_ver = 0;
		transaction_ver = cal_db_util_get_transaction_ver();
		ret = cal_ipc_marshal_int(transaction_ver, *outdata);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_ipc_marshal() Fail(%d)", ret);
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			_cal_server_ipc_return(outdata, ret);
			free(ids);
			free(stream);
			cal_handle_destroy(handle);
			return;
		}

		ret = cal_ipc_marshal_int(count, *outdata);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_ipc_marshal_int() Fail");
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			_cal_server_ipc_return(outdata, ret);
			free(ids);
			free(stream);
			cal_handle_destroy(handle);
			return;
		}

		int i;
		for(i = 0; i < count; i++) {
			ret = cal_ipc_marshal_int(ids[i], *outdata);
			if (CALENDAR_ERROR_NONE != ret) {
				ERR("cal_ipc_marshal_int() Fail(%d)", ret);
				pims_ipc_data_destroy(*outdata);
				*outdata = NULL;
				_cal_server_ipc_return(outdata, ret);
				free(ids);
				free(stream);
				cal_handle_destroy(handle);
				return;
			}
		}
	}

	free(ids);
	free(stream);
	cal_handle_destroy(handle);
	cal_server_ondemand_start();
}

void cal_server_ipc_db_replace_vcalendars(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CALENDAR_ERROR_NONE;

	if (NULL == indata) {
		ERR("No indata");
		ret = CALENDAR_ERROR_INVALID_PARAMETER;
		_cal_server_ipc_return(outdata, ret);
		return;
	}

	calendar_h handle = NULL;
	ret = cal_ipc_unmarshal_handle(indata, &handle);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_handle() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		return;
	}

	char *stream = NULL;
	ret = cal_ipc_unmarshal_char(indata, &stream);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_char() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		cal_handle_destroy(handle);
		return;
	}

	int count = 0;
	ret = cal_ipc_unmarshal_int(indata, &count);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		free(stream);
		cal_handle_destroy(handle);
		return;
	}

	int *ids = NULL;
	ids = (int*)calloc(count, sizeof(int));
	if (NULL == ids) {
		ERR("calloc() Fail");
		ret = CALENDAR_ERROR_OUT_OF_MEMORY;
		_cal_server_ipc_return(outdata, ret);
		free(stream);
		cal_handle_destroy(handle);
		return;
	}

	int i = 0;
	for(i = 0; i < count; i++) {
		ret = cal_ipc_unmarshal_int(indata, &ids[i]);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
			_cal_server_ipc_return(outdata, ret);
			free(ids);
			free(stream);
			cal_handle_destroy(handle);
			return;
		}
	}

	if (false == cal_access_control_have_permission(ipc, CAL_PERMISSION_WRITE)) {
		ret = CALENDAR_ERROR_PERMISSION_DENIED;
		_cal_server_ipc_return(outdata, ret);
		free(ids);
		free(stream);
		cal_handle_destroy(handle);
		return;
	}


	ret = cal_db_replace_vcalendars(stream, ids, count);
	_cal_server_ipc_return(outdata, ret);

	if (CALENDAR_ERROR_NONE == ret) {
		int transaction_ver = 0;
		transaction_ver = cal_db_util_get_transaction_ver();
		ret = cal_ipc_marshal_int(transaction_ver, *outdata);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_ipc_marshal_int() Fail(%d)", ret);
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			_cal_server_ipc_return(outdata, ret);
		}
	}

	free(ids);
	free(stream);
	cal_handle_destroy(handle);
	cal_server_ondemand_start();
}

void cal_server_ipc_db_replace_record(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CALENDAR_ERROR_NONE;

	if (NULL == indata) {
		ERR("No indata");
		ret = CALENDAR_ERROR_INVALID_PARAMETER;
		_cal_server_ipc_return(outdata, ret);
		return;
	}

	calendar_h handle = NULL;
	ret = cal_ipc_unmarshal_handle(indata, &handle);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_handle() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		return;
	}

	calendar_record_h record = NULL;
	ret = cal_ipc_unmarshal_record(indata, &record);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_record() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		cal_handle_destroy(handle);
		return;
	}

	int id = 0;
	ret = cal_ipc_unmarshal_int(indata, &id);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		calendar_record_destroy(record, true);
		cal_handle_destroy(handle);
		return;
	}

	if (false == cal_access_control_have_permission(ipc, CAL_PERMISSION_WRITE)) {
		ret = CALENDAR_ERROR_PERMISSION_DENIED;
		calendar_record_destroy(record, true);
		cal_handle_destroy(handle);
		return;
	}

	ret = cal_db_replace_record(record, id);
	_cal_server_ipc_return(outdata, ret);

	if (CALENDAR_ERROR_NONE == ret) {
		int transaction_ver = 0;
		transaction_ver = cal_db_util_get_transaction_ver();
		ret = cal_ipc_marshal_int(transaction_ver, *outdata);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_ipc_marshal_int() Fail");
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			_cal_server_ipc_return(outdata, ret);
		}
	}

	calendar_record_destroy(record, true);
	cal_handle_destroy(handle);
	cal_server_ondemand_start();
}

void cal_server_ipc_db_replace_records(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CALENDAR_ERROR_NONE;

	if (NULL == indata) {
		ERR("No indata");
		ret = CALENDAR_ERROR_INVALID_PARAMETER;
		_cal_server_ipc_return(outdata, ret);
		return;
	}

	calendar_h handle = NULL;
	ret = cal_ipc_unmarshal_handle(indata, &handle);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_handle() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		return;
	}

	calendar_list_h list = NULL;
	ret = cal_ipc_unmarshal_list(indata, &list);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_list() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		cal_handle_destroy(handle);
		return;
	}

	int count = 0;
	ret = cal_ipc_unmarshal_int(indata, &count);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		calendar_list_destroy(list, true);
		cal_handle_destroy(handle);
		return;
	}

	if (count <= 0) {
		ERR("count(%d) <= 0", count);
		ret = CALENDAR_ERROR_NO_DATA;
		_cal_server_ipc_return(outdata, ret);
		calendar_list_destroy(list, true);
		cal_handle_destroy(handle);
		return;
	}

	int *ids = NULL;
	ids = (int*)calloc(count, sizeof(int));
	if (NULL == ids) {
		ERR("calloc() Fail");
		ret = CALENDAR_ERROR_OUT_OF_MEMORY;
		_cal_server_ipc_return(outdata, ret);
		calendar_list_destroy(list, true);
		cal_handle_destroy(handle);
		return;
	}

	int i;
	for(i = 0; i < count; i++) {
		ret = cal_ipc_unmarshal_int(indata, &ids[i]);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
			ret = CALENDAR_ERROR_IPC;
			_cal_server_ipc_return(outdata, ret);
			free(ids);
			calendar_list_destroy(list, true);
			cal_handle_destroy(handle);
			return;
		}
	}

	if (false == cal_access_control_have_permission(ipc, CAL_PERMISSION_WRITE)) {
		ret = CALENDAR_ERROR_PERMISSION_DENIED;
		_cal_server_ipc_return(outdata, ret);
		free(ids);
		calendar_list_destroy(list, true);
		cal_handle_destroy(handle);
		return;
	}

	ret = cal_db_replace_records(list, ids, count);
	_cal_server_ipc_return(outdata, ret);

	if (CALENDAR_ERROR_NONE == ret) {
		int transaction_ver = 0;
		transaction_ver = cal_db_util_get_transaction_ver();
		ret = cal_ipc_marshal_int(transaction_ver,*outdata);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_ipc_marshal_int() Fail");
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			_cal_server_ipc_return(outdata, ret);
		}
	}

	free(ids);
	calendar_list_destroy(list, true);
	cal_handle_destroy(handle);
	cal_server_ondemand_start();
}

void cal_server_ipc_db_changes_exception(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CALENDAR_ERROR_NONE;

	if (NULL == indata) {
		ERR("No indata");
		ret = CALENDAR_ERROR_INVALID_PARAMETER;
		_cal_server_ipc_return(outdata, ret);
		return;
	}

	calendar_h handle = NULL;
	ret = cal_ipc_unmarshal_handle(indata, &handle);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_handle() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		return;
	}

	char *view_uri = NULL;
	ret = cal_ipc_unmarshal_char(indata, &view_uri);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_char() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		cal_handle_destroy(handle);
		return;
	}

	int original_event_id = 0;
	ret = cal_ipc_unmarshal_int(indata, &original_event_id);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		free(view_uri);
		cal_handle_destroy(handle);
		return;
	}

	int calendar_db_version = 0;
	ret = cal_ipc_unmarshal_int(indata, &calendar_db_version);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_ipc_unmarshal_int() Fail(%d)", ret);
		_cal_server_ipc_return(outdata, ret);
		free(view_uri);
		cal_handle_destroy(handle);
		return;
	}

	if (false == cal_access_control_have_permission(ipc, CAL_PERMISSION_READ)) {
		ret = CALENDAR_ERROR_PERMISSION_DENIED;
		_cal_server_ipc_return(outdata, ret);
		free(view_uri);
		cal_handle_destroy(handle);
		return;
	}

	calendar_list_h list = NULL;
	ret = cal_db_get_changes_exception_by_version(view_uri, original_event_id, calendar_db_version, &list);
	_cal_server_ipc_return(outdata, ret);

	if (CALENDAR_ERROR_NONE == ret) {
		ret = cal_ipc_marshal_list(list, *outdata);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_ipc_marshal_list() Fail(%d)", ret);
			ret = CALENDAR_ERROR_IPC;
			_cal_server_ipc_return(outdata, ret);
		}
	}

	calendar_list_destroy(list, true);
	free(view_uri);
	cal_handle_destroy(handle);
	cal_server_ondemand_start();
}

#ifdef CAL_MEMORY_TEST
static gboolean  _cal_server_ipc_destroy_idle(void* user_data)
{
	g_main_loop_quit(main_loop);
}
void cal_server_ipc_destroy(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CALENDAR_ERROR_NONE;

	/* kill daemon destroy */
	g_timeout_add_seconds(1, &_cal_server_ipc_destroy_idle, NULL);

	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (!*outdata) {
			ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata, (void*)&ret, sizeof(int)) != 0) {
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("pims_ipc_data_put() Fail");
			goto DATA_FREE;
		}

	}
	else {
		ERR("outdata is NULL");
	}
	goto DATA_FREE;

ERROR_RETURN:
	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (!*outdata) {
			ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata, (void*)&ret, sizeof(int)) != 0) {
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("pims_ipc_data_put() Fail");
			goto DATA_FREE;
		}
	}
	else {
		ERR("outdata is NULL");
	}
DATA_FREE:

	return;
}
#endif /* CAL_MEMORY_TEST */

int cal_server_ipc_init(void)
{
	g_type_init();

	char sock_file[CAL_STR_MIDDLE_LEN] = {0};
	snprintf(sock_file, sizeof(sock_file), CAL_SOCK_PATH"/.%s", getuid(), CAL_IPC_SERVICE);
	pims_ipc_svc_init(sock_file,CAL_SECURITY_FILE_GROUP, 0777);

	RETV_IF(pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_CONNECT, cal_server_ipc_connect, NULL) != 0, -1);
	RETV_IF(pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DISCONNECT, cal_server_ipc_disconnect, NULL) != 0, -1);
	RETV_IF(pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_CHECK_PERMISSION, cal_server_ipc_check_permission, NULL) != 0, -1);
	RETV_IF(pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_INSERT_RECORD, cal_server_ipc_db_insert_record, NULL) != 0, -1);
	RETV_IF(pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_GET_RECORD, cal_server_ipc_db_get_record, NULL) != 0, -1);
	RETV_IF(pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_UPDATE_RECORD, cal_server_ipc_db_update_record, NULL) != 0, -1);
	RETV_IF(pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_DELETE_RECORD, cal_server_ipc_db_delete_record, NULL) != 0, -1);
	RETV_IF(pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_GET_ALL_RECORDS, cal_server_ipc_db_get_all_records, NULL) != 0, -1);
	RETV_IF(pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_GET_RECORDS_WITH_QUERY, cal_server_ipc_db_get_records_with_query, NULL) != 0, -1);
	RETV_IF(pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_CLEAN_AFTER_SYNC, cal_server_ipc_db_clean_after_sync, NULL) != 0, -1);
	RETV_IF(pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_GET_COUNT, cal_server_ipc_db_get_count, NULL) != 0, -1);
	RETV_IF(pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_GET_COUNT_WITH_QUERY, cal_server_ipc_db_get_count_with_query, NULL) != 0, -1);
	RETV_IF(pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_INSERT_RECORDS, cal_server_ipc_db_insert_records, NULL) != 0, -1);
	RETV_IF(pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_UPDATE_RECORDS, cal_server_ipc_db_update_records, NULL) != 0, -1);
	RETV_IF(pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_DELETE_RECORDS, cal_server_ipc_db_delete_records, NULL) != 0, -1);
	RETV_IF(pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_CHANGES_BY_VERSION, cal_server_ipc_db_get_changes_by_version, NULL) != 0, -1);
	RETV_IF(pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_GET_CURRENT_VERSION, cal_server_ipc_db_get_current_version, NULL) != 0, -1);
	RETV_IF(pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_INSERT_VCALENDARS, cal_server_ipc_db_insert_vcalendars, NULL) != 0, -1);
	RETV_IF(pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_REPLACE_VCALENDARS, cal_server_ipc_db_replace_vcalendars, NULL) != 0, -1);
	RETV_IF(pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_REPLACE_RECORD, cal_server_ipc_db_replace_record, NULL) != 0, -1);
	RETV_IF(pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_REPLACE_RECORDS, cal_server_ipc_db_replace_records, NULL) != 0, -1);
	RETV_IF(pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DB_CHANGES_EXCEPTION, cal_server_ipc_db_changes_exception, NULL) != 0, -1);
#ifdef CAL_MEMORY_TEST
	RETV_IF(pims_ipc_svc_register(CAL_IPC_MODULE, CAL_IPC_SERVER_DESTROY, cal_server_ipc_destroy, NULL) != 0, -1);
#endif /* CAL_MEMORY_TEST */

	/* for subscribe */
	snprintf(sock_file, sizeof(sock_file), CAL_SOCK_PATH"/.%s_for_subscribe", getuid(), CAL_IPC_SERVICE);
	pims_ipc_svc_init_for_publish(sock_file, CAL_SECURITY_FILE_GROUP, CAL_SECURITY_DEFAULT_PERMISSION);
	return CALENDAR_ERROR_NONE;
}

void cal_server_ipc_deinit(void)
{
	pims_ipc_svc_deinit_for_publish();
	pims_ipc_svc_deinit();
}

void cal_server_ipc_run(GMainLoop *loop)
{
	pims_ipc_svc_run_main_loop(loop);
}
