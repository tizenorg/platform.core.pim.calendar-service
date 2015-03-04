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
#ifndef __TIZEN_SOCIAL_CALENDAR_VCALENDAR_H__
#define __TIZEN_SOCIAL_CALENDAR_VCALENDAR_H__

#include <calendar_view.h>

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file calendar_vcalendar.h
 */

/**
 * @addtogroup CAPI_SOCIAL_CALENDAR_SVC_VCALENDAR_MODULE
 * @{
 */

/**
 * @brief Retrieves a vcalendar stream from a calendar list.
 *
 * @since_tizen 2.3
 *
 * @param[in]	calendar_list       The calendar list handle
 * @param[out]	vcalendar_stream    The vcalendar stream
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_NO_DATA             Requested data does not exist
 */
int calendar_vcalendar_make_from_records(calendar_list_h calendar_list, char **vcalendar_stream);

/**
 * @brief Retrieves all calendars from a vcalendar stream.
 *
 * @since_tizen 2.3
 *
 * @param[in]	vcalendar_stream    The vcalendar stream
 * @param[out]	calendar_list       The calendar list handle
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_NO_DATA             Requested data does not exist
 *
 * @pre     calendar_connect() should be called to initialize.
 *
 */
int calendar_vcalendar_parse_to_calendar(const char* vcalendar_stream, calendar_list_h *calendar_list);

/**
 * @brief Called to get a record handle of
 * \ref CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE_calendar_event or \ref CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE_calendar_todo.
 *
 * @since_tizen 2.3
 *
 * @param[in]	record	   The record handle (\ref CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE_calendar_event or \ref CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE_calendar_todo)
 * @param[in]	user_data  The user data passed from the foreach function
 *
 * @return	@c true to continue with the next iteration of the loop,
 *          otherwise @c false to break out of the loop
 *
 * @pre calendar_vcalendar_parse_to_calendar_foreach() will invoke this callback.
 *
 * @see calendar_vcalendar_parse_to_calendar_foreach()
 * @see calendar_record_get_uri_p()
 */
typedef bool (*calendar_vcalendar_parse_cb)(calendar_record_h record, void *user_data);

/**
 * @brief Retrieves all events or to-dos
 * (\ref CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE_calendar_event or \ref CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE_calendar_todo) from a vCalendar file.
 *
 * @since_tizen 2.3
 *
 * @param[in]	vcalendar_file_path     The file path of the vCalendar stream file
 * @param[in]	callback                The callback function to be invoked
 * @param[in]	user_data               The user data to be passed to the callback function
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @pre  calendar_connect() should be called to initialize.
 *
 * @post This function invokes calendar_vcalendar_parse_cb().
 *
 * @see  calendar_vcalendar_parse_cb()
 * @see  calendar_record_get_uri_p()
 */
int calendar_vcalendar_parse_to_calendar_foreach(const char *vcalendar_file_path, calendar_vcalendar_parse_cb callback, void *user_data);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_SOCIAL_CALENDAR_VCALENDAR_H__ */

