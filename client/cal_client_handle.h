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

#ifndef __CAL_CLIENT_HANDLE_H__
#define __CAL_CLIENT_HANDLE_H__

#include "cal_handle.h"

int cal_client_handle_get_p(calendar_h *handle);
int cal_client_handle_get_p_with_id(unsigned int id, calendar_h *handle);
int cal_client_handle_create(unsigned int id, calendar_h *out_handle);
int cal_client_handle_remove(unsigned int id, calendar_h handle);
void cal_client_handle_set_version(calendar_h handle, int version);

#endif /* __CAL_CLIENT_HANDLE_H__ */
