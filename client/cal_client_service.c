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

#include "calendar.h"
#include "cal_internal.h"
#include "cal_client_handle.h"
#include "cal_client_service_helper.h"
#include "cal_client_utils.h"

static int cal_connection = 0; /* total connection count: each count from zone */
static TLS int cal_connection_on_thread = 0;

API int calendar_connect(void)
{
	int ret;
	calendar_h handle = NULL;
	unsigned int id = cal_client_get_pid();

	ret = cal_client_handle_get_p_with_id(id, &handle);
	if (CALENDAR_ERROR_NO_DATA == ret) {
		ret = cal_client_handle_create(id, &handle);
		RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_client_handle_create() Fail(%d)", ret);
	}
	else if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_client_handle_get_p_with_id() Fail(%d)", ret);
		return ret;
	}
	ret = cal_client_connect(handle, id, &cal_connection);
	return ret;
}

API int calendar_disconnect(void)
{
	int ret;
	calendar_h handle = NULL;
	unsigned int id = cal_client_get_pid();

	ret = cal_client_handle_get_p_with_id(id, &handle);
	if (CALENDAR_ERROR_NO_DATA == ret) {
		return CALENDAR_ERROR_NONE;
	}
	else if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_client_handle_get_p_with_id() Fail(%d)", ret);
		return ret;
	}
	ret = cal_client_disconnect(handle, id, &cal_connection);
	WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_client_disconnect() Fail(%d)", ret);
	return ret;
}

API int calendar_connect_on_thread(void)
{
	int ret;
	calendar_h handle = NULL;
	unsigned int id = cal_client_get_tid();

	ret = cal_client_handle_get_p_with_id(id, &handle);
	if (CALENDAR_ERROR_NO_DATA == ret) {
		ret = cal_client_handle_create(id, &handle);
		RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_client_handle_create() Fail(%d)", ret);
	}
	else if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_client_handle_get_p_with_id() Fail(%d)", ret);
		return ret;
	}
	ret = cal_client_connect(handle, id, &cal_connection_on_thread);
	return ret;
}

API int calendar_disconnect_on_thread(void)
{
	int ret;
	calendar_h handle = NULL;
	unsigned int id = cal_client_get_tid();

	ret = cal_client_handle_get_p_with_id(id, &handle);
	if (CALENDAR_ERROR_NO_DATA == ret) {
		return CALENDAR_ERROR_NONE;
	}
	else if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_client_handle_get_p_with_id() Fail(%d)", ret);
		return ret;
	}
	ret = cal_client_disconnect(handle, id, &cal_connection_on_thread);
	WARN_IF(CALENDAR_ERROR_NONE != ret, "cal_client_disconnect() Fail(%d)", ret);
	return ret;
}

API int calendar_connect_with_flags(unsigned int flags)
{
	int ret;
	calendar_h handle = NULL;
	unsigned int id = cal_client_get_pid();

	ret = cal_client_handle_get_p_with_id(id, &handle);
	if (CALENDAR_ERROR_NO_DATA == ret) {
		ret = cal_client_handle_create(id, &handle);
		RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_client_handle_create() Fail(%d)", ret);
	}
	else if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_client_handle_get_p_with_id() Fail(%d)", ret);
		return ret;
	}
	ret = cal_client_connect_with_flags(handle, id, &cal_connection, flags);
	return ret;
}


int cal_client_get_thread_connection_count(void)
{
	return cal_connection_on_thread;
}
