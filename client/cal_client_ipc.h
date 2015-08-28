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

#ifndef __CAL_CLIENT_IPC_H__
#define __CAL_CLIENT_IPC_H__

#include <pims-ipc.h>
#include "calendar_types.h"
#include "cal_handle.h"

int cal_client_ipc_connect(calendar_h handle, unsigned int id);
int cal_client_ipc_disconnect(calendar_h handle, unsigned int id, int connection_count);

bool cal_client_ipc_is_call_inprogress(void);
int cal_client_ipc_call(char *module, char *function, pims_ipc_h data_in, pims_ipc_data_h *data_out);
int cal_client_ipc_client_check_permission(int permission, bool *result);
int cal_client_ipc_get_change_version(calendar_h handle);
void cal_client_ipc_set_change_version(calendar_h handle, int version);
void cal_client_ipc_lock(void);
void cal_client_ipc_unlock(void);

bool cal_client_ipc_get_disconnected(void);
int cal_client_ipc_set_disconnected_cb(pims_ipc_h ipc, void (*cb)(void *), void *user_data);
int cal_client_ipc_unset_disconnected_cb(pims_ipc_h ipc);
void cal_client_ipc_set_disconnected(bool is_disconnected);
void cal_client_ipc_recovery(void);

#endif /* __CAL_CLIENT_IPC_H__ */
