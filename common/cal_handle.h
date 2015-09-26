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
#ifndef __CAL_HANDLE_H__
#define __CAL_HANDLE_H__

#include "calendar_types.h"

typedef struct {
	int version;
	int connection_count;
} cal_s;

int cal_handle_create(calendar_h *handle);
int cal_handle_destroy(calendar_h handle);
int cal_handle_get_version(calendar_h handle, int *out_version);

#endif /* __CAL_HANDLE_H__ */
