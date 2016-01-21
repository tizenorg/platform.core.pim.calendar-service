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

#ifndef __TIZEN_SOCIAL_CALENDAR_REMINDER_H__
#define __TIZEN_SOCIAL_CALENDAR_REMINDER_H__

#include <calendar_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file calendar_reminder.h
 */

/**
 * @addtogroup CAPI_SOCIAL_CALENDAR_SVC_REMINDER_MODULE
 * @{
 */

/**
 * @brief Called when an alarm is alerted.
 *
 * @since_tizen 2.3
 *
 * @param[in]   param  Value string like id=value&time=value&tick=value&unit=value&type=value
 * @param[in]   user_data   The user data passed from the callback registration function
 *
 * @see calendar_reminder_add_cb()
 */
typedef void (*calendar_reminder_cb)(const char *param, void* user_data);

/**
 * @brief Adds a callback to get a notification when an alarm alerts.
 *
 * @since_tizen 2.3
 *
 * @privlevel public
 * @privilege %http://tizen.org/privilege/calendar.read
 *
 * @param[in]   callback     The callback to be added
 * @param[in]   user_data	 The user data
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE            Successful
 * @retval  #CALENDAR_ERROR_OUT_OF_MEMORY   Out of memory
 * @retval  #CALENDAR_ERROR_NOT_PERMITTED   Operation not permitted
 * @retval  #CALENDAR_ERROR_IPC             Unknown IPC error
 *
 * @see calendar_reminder_remove_cb()
 */
int calendar_reminder_add_cb(calendar_reminder_cb callback, void *user_data);

/**
 * @brief Removes a callback to get a notification when an alarm alerts.
 *
 * @since_tizen 2.3
 *
 * @param[in]   callback       The callback to be removed
 * @param[in]   user_data      The user data
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE        Successful
 * @retval  #CALENDAR_ERROR_DB_FAILED   Database operation failure
 *
 * @see calendar_reminder_add_cb()
 */
int calendar_reminder_remove_cb(calendar_reminder_cb callback, void *user_data);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_SOCIAL_CALENDAR_REMINDER_H__ */

