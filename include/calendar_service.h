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

#ifndef __TIZEN_SOCAIL_CALENDAR_SERVICE_H__
#define __TIZEN_SOCAIL_CALENDAR_SERVICE_H__

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup CAPI_SOCIAL_CALENDAR_SVC_DATABASE_MODULE
 * @{
 */

/**
 * @brief	Connects to the calendar service.
 *
 * @remarks Connection opening is necessary to access the calendar database such as fetching, inserting, or updating records.\n
 * The execution of calendar_connect() and calendar_disconnect() could slow down your application so you are recommended not to call them frequently.
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE		Successful
 * @retval  #CALENDAR_ERROR_DB_FAILED	Database operation failure
 *
 * @see  calendar_disconnect()
 */
API int calendar_connect(void);

/**
 * @brief	Disconnects from the calendar service.
 *
 * @remarks	If there is no opened connection, this function returns #CALENDAR_ERROR_DB_FAILED.
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE		Successful
 * @retval  #CALENDAR_ERROR_DB_FAILED	Database operation failure
 *
 * @see calendar_connect()
 */
API int calendar_disconnect(void);

/**
 * @brief   Connects to the calendar service on thread.
 *
 * @remarks Connection opening is necessary to access the calendar database such as fetching, inserting, or updating records.\n
 * The execution of calendar_connect() and calendar_disconnect() could slow down your application so you are recommended not to call them frequently.
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE        Successful
 * @retval  #CALENDAR_ERROR_DB_FAILED   Database operation failure
 *
 * @see  calendar_disconnect()
 */
API int calendar_connect_on_thread(void);

/**
 * @brief   Disconnects from the calendar service on thread.
 *
 * @remarks If there is no opened connection, this function returns #CALENDAR_ERROR_DB_FAILED.
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE        Successful
 * @retval  #CALENDAR_ERROR_DB_FAILED   Database operation failure
 *
 * @see calendar_connect()
 */
API int calendar_disconnect_on_thread(void);

/**
 * @brief   Connects to the calendar service.
 *
 * @remarks Connection opening is necessary to access the calendar database such as fetching, inserting, or updating records.\n
 * The execution of calendar_connect() and calendar_disconnect() could slow down your application so you are recommended not to call them frequently.
 *
 * @param[in]   flags  calendar_connect_flag
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE        Successful
 * @retval  #CALENDAR_ERROR_DB_FAILED   Database operation failure
 *
 * @see  calendar_disconnect(), CALENDAR_CONNECT_FLAG_RETRY
 */
API int calendar_connect_with_flags(unsigned int flags);
/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_SOCAIL_CALENDAR_SERVICE_H__ */

