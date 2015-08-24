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

#ifndef __CAL_CLIENT_IPC_H__
#define __CAL_CLIENT_IPC_H__

#include <pims-ipc.h>

bool cal_client_ipc_is_call_inprogress(void);
int cal_client_ipc_call(char *module, char *function, pims_ipc_h data_in, pims_ipc_data_h *data_out);
void cal_client_ipc_set_change_version(int version);
int cal_client_ipc_get_change_version(void);
int cal_client_ipc_client_check_permission(int permission, bool *result);

#endif // __CAL_CLIENT_IPC_H__
