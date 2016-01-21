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

#ifndef __TIZEN_SOCIAL_CALENDAR_SERVICE_H__
#define __TIZEN_SOCIAL_CALENDAR_SERVICE_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file calendar_service.h
 */

/**
 * @ingroup CAPI_SOCIAL_CALENDAR_SVC_MODULE
 * @defgroup CAPI_SOCIAL_CALENDAR_SVC_COMMON_MODULE Common
 * @brief The calendar common API provides the set of definitions and interfaces to initialize and deinitialize.
 *
 * @section CAPI_SOCIAL_CALENDAR_SVC_COMMON_MODULE_HEADER Required Header
 *  \#include <calendar.h>
 *
 * <BR>
 * @{
 */

/**
 * @brief Connects to the calendar service.
 *
 * @since_tizen 2.3
 *
 * @remarks Opening the connection is necessary to access the calendar database and perform operations such as fetching, inserting, or updating records.\n
 *          The execution of calendar_connect() and calendar_disconnect() could slow down your application, so you are recommended not to call them frequently.
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_DB_FAILED           Database operation failure
 * @retval  #CALENDAR_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_NOT_PERMITTED       Operation not permitted
 * @retval  #CALENDAR_ERROR_PERMISSION_DENIED   Permission denied. This application does not have the privilege to call this method
 * @retval  #CALENDAR_ERROR_IPC                 Unknown IPC error
 * @retval  #CALENDAR_ERROR_SYSTEM              Error from another modules
 *
 * @see  calendar_disconnect()
 */
int calendar_connect(void);

/**
 * @brief Disconnects from the calendar service.
 *
 * @since_tizen 2.3
 *
 * @remarks If there is no opened connection, this function returns #CALENDAR_ERROR_DB_FAILED.
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_NOT_PERMITTED       Operation not permitted
 * @retval  #CALENDAR_ERROR_IPC                 Unknown IPC error
 *
 * @see calendar_connect()
 */
int calendar_disconnect(void);

/**
 * @brief Connects to the calendar service on a thread.
 *
 * @since_tizen 2.3
 *
 * @remarks Opening a connection is necessary to access the calendar database and perform operations such as fetching, inserting, or updating records.\n
 *          On a thread environment with calendar_connect(), request in one thread could fail, while another request connection is working in the other thread.
 *          To prevent request failure, calendar_connect_on_thread() is recommended.
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE               Successful
 * @retval  #CALENDAR_ERROR_DB_FAILED          Database operation failure
 * @retval  #CALENDAR_ERROR_PERMISSION_DENIED  Permission denied. This application does not have the privilege to call this method
 *
 * @see  calendar_disconnect_on_thread()
 */
int calendar_connect_on_thread(void);

/**
 * @brief Disconnects from the calendar service on a thread.
 *
 * @since_tizen 2.3
 *
 * @remarks If there is no opened connection, this function returns #CALENDAR_ERROR_DB_FAILED.
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_DB_FAILED           Database operation failure
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_NOT_PERMITTED       Operation not permitted
 *
 * @see calendar_connect_on_thread()
 */
int calendar_disconnect_on_thread(void);

/**
 * @brief Connects to the calendar service.
 *
 * @since_tizen 2.3
 *
 * @remarks Opening a connection is necessary to access the calendar database and perform operations such as fetching, inserting, or updating records.\n
 *          Before the calendar-service daemon is ready, if you call calendar_connect(), it could fail.
 *          It is recommended to call this API with #CALENDAR_CONNECT_FLAG_RETRY flags in such a situation.
 *
 * @param[in]   flags  calendar_connect_flag
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE			    Successful
 * @retval  #CALENDAR_ERROR_DB_FAILED           Database operation failure
 * @retval  #CALENDAR_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_NOT_PERMITTED       Operation not permitted
 * @retval  #CALENDAR_ERROR_PERMISSION_DENIED   Permission denied. This application does not have the privilege to call this method.
 * @retval  #CALENDAR_ERROR_IPC                 Unknown IPC error
 * @retval  #CALENDAR_ERROR_SYSTEM              Error from another modules
 *
 * @see  calendar_disconnect(), CALENDAR_CONNECT_FLAG_RETRY
 */
int calendar_connect_with_flags(unsigned int flags);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_SOCIAL_CALENDAR_SERVICE_H__ */

