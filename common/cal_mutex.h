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

#ifndef __CALENDAR_SVC_MUTEX_H__
#define __CALENDAR_SVC_MUTEX_H__

enum {
	CAL_MUTEX_CONNECTION,
	CAL_MUTEX_PIMS_IPC_CALL,
	CAL_MUTEX_INOTIFY,
	CAL_MUTEX_PROPERTY_HASH,
	CAL_MUTEX_PIMS_IPC_PUBSUB,
	CAL_MUTEX_ACCESS_CONTROL,
};

void cal_mutex_lock(int type);
void cal_mutex_unlock(int type);

#endif  /*__CALENDAR_SVC_MUTEX_H__ */
