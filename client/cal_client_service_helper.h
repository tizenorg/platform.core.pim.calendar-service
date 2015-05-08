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

#ifndef __CAL_CLIENT_SERVER_HELPER_H__
#define __CAL_CLIENT_SERVER_HELPER_H__

int cal_client_connect(void);
int cal_client_disconnect(void);
int cal_client_connect_on_thread(void);
int cal_client_disconnect_on_thread(void);
int cal_client_connect_with_flags(unsigned int flags);

#endif // __CAL_CLIENT_SERVER_HELPER_H__
