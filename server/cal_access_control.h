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

#ifndef __CALENDAR_SVC_ACCESS_CONTROL_H__
#define __CALENDAR_SVC_ACCESS_CONTROL_H__

#include <pims-ipc.h>
#include "cal_typedef.h"

#define CAL_PRIVILEGE_READ "http://tizen.org/privilege/calendar.read"
#define CAL_PRIVILEGE_WRITE "http://tizen.org/privilege/calendar.write"

void cal_access_control_set_client_info(pims_ipc_h ipc, const char* smack_label);
void cal_access_control_unset_client_info(void);
char* cal_access_control_get_label(void);
void cal_access_control_reset(void);  // reset read_list, write_list..
bool cal_access_control_have_write_permission(int book_id);
int cal_is_owner(int book_id);

#endif /*  __CALENDAR_SVC_ACCESS_CONTROL_H__ */
