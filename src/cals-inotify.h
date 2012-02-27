/*
 * Calendar Service
 *
 * Copyright (c) 2000 - 2012 Samsung Electronics Co., Ltd. All rights reserved.
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

int cals_inotify_init(void);
void cals_inotify_close(void);
int cals_inotify_subscribe(const char *path, void (*cb)(void *), void *data);
int cals_inotify_unsubscribe(const char *path, void (*cb)(void *));
int cals_inotify_unsubscribe_with_data(const char *path,
		void (*cb)(void *), void *user_data);


#endif //__CALENDAR_SVC_INOTIFY_H__