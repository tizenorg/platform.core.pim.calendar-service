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

#ifndef __CALENDAR_SVC_INOTIFY_H__
#define __CALENDAR_SVC_INOTIFY_H__

#include "calendar_db.h"
#include "cal_typedef.h"

#ifdef CAL_IPC_CLIENT
void cal_inotify_call_pending_callback(void);
#endif

int cal_inotify_initialize(void);
int cal_inotify_subscribe(cal_noti_type_e type, const char *path, calendar_db_changed_cb callback, void *data);
int cal_inotify_unsubscribe_with_data(const char *path, calendar_db_changed_cb callback, void *user_data);
void cal_inotify_finalize(void);

#endif /* __CALENDAR_SVC_INOTIFY_H__ */
