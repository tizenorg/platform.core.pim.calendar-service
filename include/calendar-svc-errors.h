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
/**
 * @defgroup return_value return_value
 * @ingroup CALENDAR_SVC
 * @brief
 *		return value with api call
 */

#ifndef __CALENDAR_SVC_ERRORS_H__
#define __CALENDAR_SVC_ERRORS_H__

typedef enum
{
	CAL_ERR_EVENT_START_DATE = -405,
	CAL_ERR_EVENT_END_DATE = -404,
	CAL_ERR_EVENT_REPEAT_END_DATE = -403,
	CAL_ERR_EVENT_DURATION = -402,
	CAL_ERR_EVENT_REPEAT_DURATION_TOO_SHORT = -401,

	CAL_ERR_INVALID_DATA_TYPE	=-301,

	CAL_ERR_DB_LOCK = -204,
	CAL_ERR_DB_RECORD_NOT_FOUND = -203,
	CAL_ERR_DB_FAILED = -202,
	CAL_ERR_DB_NOT_OPENED= -201,

	CAL_ERR_ALARMMGR_FAILED = -105,
	CAL_ERR_TIME_FAILED = -104,
	CAL_ERR_INOTIFY_FAILED = -103,
	CAL_ERR_VCONF_FAILED = -102,
	CAL_ERR_VOBJECT_FAILED = -101,

	CAL_ERR_NO_SPACE = -11,
	CAL_ERR_IO_ERR = -10,
	CAL_ERR_EXCEEDED_LIMIT = -9,
	CAL_ERR_OUT_OF_MEMORY = -8,
	CAL_ERR_ALREADY_EXIST = -7,
	CAL_ERR_ENV_INVALID = -6,
	CAL_ERR_ARG_NULL = -5,
	CAL_ERR_ARG_INVALID = -4,
	CAL_ERR_NO_DATA = -3,
	CAL_ERR_FINISH_ITER= -2,
	CAL_ERR_FAIL= -1,
	CAL_SUCCESS = 0,
	CAL_FALSE = 0,
	CAL_TRUE
}cal_error;

#endif /* __CALENDAR_SVC_ERRORS_H__ */

