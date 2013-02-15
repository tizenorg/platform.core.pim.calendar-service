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

#ifndef __TIZEN_SOCAIL_CALENDAR_ERROR_H__
#define __TIZEN_SOCAIL_CALENDAR_ERROR_H__

#include <tizen.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup CAPI_SOCIAL_CALENDAR_SVC_LIST_MODULE
 * @{
 */

typedef enum
{
    CALENDAR_ERROR_NONE                 = TIZEN_ERROR_NONE,                     /**< Successful */
    CALENDAR_ERROR_DB_FAILED            = TIZEN_ERROR_SOCIAL_CLASS | 0x02,      /**< No access to database */
    CALENDAR_ERROR_DB_RECORD_NOT_FOUND  = TIZEN_ERROR_SOCIAL_CLASS | 0x05,      /**< Not found database */
    CALENDAR_ERROR_OUT_OF_MEMORY        = TIZEN_ERROR_OUT_OF_MEMORY,            /**< Out of memory */
    CALENDAR_ERROR_INVALID_PARAMETER    = TIZEN_ERROR_INVALID_PARAMETER,        /**< Invalid parameter */
    CALENDAR_ERROR_NO_DATA              = TIZEN_ERROR_SOCIAL_CLASS | 0x03,      /**< Requested data does not exist */
    CALENDAR_ERROR_ITERATOR_END         = TIZEN_ERROR_SOCIAL_CLASS | 0x04,      /**< Iterator is on last position */
    CALENDAR_ERROR_NOW_IN_PROGRESS      = TIZEN_ERROR_NOW_IN_PROGRESS,          /**< Operation now in progress */
    CALENDAR_ERROR_ALREADY_IN_PROGRESS  = TIZEN_ERROR_ALREADY_IN_PROGRESS,      /**< Operation already in progress */
    CALENDAR_ERROR_NOT_PERMITTED        = TIZEN_ERROR_NOT_PERMITTED,            /**< Operation not permitted */
    CALENDAR_ERROR_IPC                  = TIZEN_ERROR_SOCIAL_CLASS | 0xBF,      /**< Unknown IPC error */
	CALENDAR_ERROR_FILE_NO_SPACE        = TIZEN_ERROR_FILE_NO_SPACE_ON_DEVICE,  /**< FS Full */
} calendar_error_e;

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_SOCAIL_CALENDAR_ERROR_H__ */

