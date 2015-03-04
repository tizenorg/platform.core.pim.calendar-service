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

#include <stdlib.h>

#include "calendar_reminder.h"

#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_view.h"
#include "cal_time.h"
#include "cal_record.h"
#include "cal_access_control.h"

#include "cal_db_util.h"
#include "cal_db.h"

API int calendar_reminder_add_cb(calendar_reminder_cb callback, void *user_data)
{
	ERR("This API[%s] is not valid in native library.", __func__);
	ERR("If you want to use this API, please use in client library.");
	return CALENDAR_ERROR_NOT_PERMITTED;
}

API int calendar_reminder_remove_cb(calendar_reminder_cb callback, void *user_data)
{
	ERR("This API[%s] is not valid in native library.", __func__);
	ERR("If you want to use this API, please use in client library.");
	return CALENDAR_ERROR_NOT_PERMITTED;
}

