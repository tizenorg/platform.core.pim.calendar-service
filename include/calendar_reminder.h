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

#ifndef __TIZEN_SOCAIL_CALENDAR_REMINDER_H__
#define __TIZEN_SOCAIL_CALENDAR_REMINDER_H__

#include <calendar_types2.h>

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup CAPI_SOCIAL_CALENDAR_SVC_REMINDER_MODULE
 * @{
 */

/**
 * @brief   Adds a receiver to get noti when alarm alerts.
 *
 * @remarks If failed to run appsvc, added receiver will be removed from the table.
 *
 * @param[in]   pkgname			    The package name to add.
 * @param[in]   extra_data_key		The user defined key to be passed via appsvc.
 * @param[in]   extra_data_value	THe user defined value to be passed via appsvc.
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE        Successful
 * @retval  #CALENDAR_ERROR_DB_FAILED   Database operation failure
 *
 * @see calendar_reminder_remove_receiver()
 */
API int calendar_reminder_add_receiver(const char *pkgname, const char *extra_data_key, const char *extra_data_value);

/**
 * @brief   Removes a receiver to get noti when alarm alerts.
 *
 * @param[in]   pkgname			    The package name to add.
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE        Successful
 * @retval  #CALENDAR_ERROR_DB_FAILED   Database operation failure
 *
 * @see calendar_reminder_remove_receiver()
 */
API int calendar_reminder_remove_receiver(const char *pkgname);

/**
 * @brief   Activates a receiver to get noti when alarm alerts.
 *
 * @param[in]   pkgname			    The package name to add.
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE        Successful
 * @retval  #CALENDAR_ERROR_DB_FAILED   Database operation failure
 *
 * @see calendar_reminder_remove_receiver()
 */
API int calendar_reminder_activate_receiver(const char *pkgname);

/**
 * @brief   Deactivates a receiver to get noti when alarm alerts.
 *
 * @param[in]   pkgname			    The package name to add.
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE        Successful
 * @retval  #CALENDAR_ERROR_DB_FAILED   Database operation failure
 *
 * @see calendar_reminder_remove_receiver()
 */
API int calendar_reminder_deactivate_receiver(const char *pkgname);

/**
 * @brief   Check whether receiver exist in the table or not.
 *
 * @param[in]   pkgname			    The package name to add.
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE        Successful
 * @retval  #CALENDAR_ERROR_DB_FAILED   Database operation failure
 *
 * @see calendar_reminder_remove_receiver()
 */
API int calendar_reminder_has_receiver(const char *pkgname);

/**
 * @brief   Adds callback to get noti when alarm alerts.
 *
 * @param[in]   callback			    Callback to add.
 * @param[in]   user_data				The user data
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE        Successful
 * @retval  #CALENDAR_ERROR_DB_FAILED   Database operation failure
 *
 * @see calendar_reminder_remove_cb()
 */
API int calendar_reminder_add_cb(calendar_reminder_cb callback, void *user_data);

/**
 * @brief   Removes callback to get noti when alarm alerts.
 *
 * @param[in]   callback			    Callback to remove.
 * @param[in]   user_data				The user data
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE        Successful
 * @retval  #CALENDAR_ERROR_DB_FAILED   Database operation failure
 *
 * @see calendar_reminder_add_cb()
 */
API int calendar_reminder_remove_cb(calendar_reminder_cb callback, void *user_data);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_SOCAIL_CALENDAR_REMINDER_H__ */

