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

#ifndef __TIZEN_SOCAIL_CALENDAR_LIST_H__
#define __TIZEN_SOCAIL_CALENDAR_LIST_H__

#include <calendar_types2.h>

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup CAPI_SOCIAL_CALENDAR_SVC_LIST_MODULE
 * @{
 */

/**
 * @brief Creates a handle to the calendar list.
 *
 * @remarks @a calendar_list must be released with calendar_list_destroy() by you.
 *
 * @param[out]  calendar_list    The calendar list handle
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER       Invalid parameter
 *
 * @see calendar_list_destroy()
 */
API int calendar_list_create( calendar_list_h* out_list );

/**
 * @brief Destroys a calendar list handle and releases all its resources.
 *
 * @param[in]   calendar_list  The calendar list handle
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE                    Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER       Invalid parameter
 *
 * @see calendar_list_create()
 */
API int calendar_list_destroy( calendar_list_h list, bool delete_record );


/**
 * @brief      Retrieves count of calendar entity from a calendar list.
 *
 * @param[in]	calendar_list				The calendar list handle
 * @param[out]	count						The count of calendar entity
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see calendar_list_add()
 */
API int calendar_list_get_count( calendar_list_h list, int *count );

/**
 * @brief      Adds a record handle to calendar list handle.
 *
 * @param[in]	calendar_list				The calendar list handle
 * @param[in]	record						The record handle
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see calendar_list_remove()
 */
API int calendar_list_add( calendar_list_h list, calendar_record_h record );

/**
 * @brief      Removes a record handle to calendar list handle.
 * @details    If the record is current record then current record is changed the next record.\n
 * If the record is the last record then current record will be NULL.
 *
 * @param[in]	calendar_list				The calendar list handle
 * @param[in]	record						The record handle
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see calendar_list_add()
 */
API int calendar_list_remove( calendar_list_h list, calendar_record_h record );

/**
 * @brief		Retrieves a record handle from calendar list handle.
 * @details		The default current record is the first record
 * @remarks		The @a record handle MUST NOT destroyed by you.
 * It is destroyed automatically when the @a calendar_list is destroyed.
 *
 * @param[in]	calendar_list				The calendar list handle
 * @param[out]	record						The record handle
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 */
API int calendar_list_get_current_record_p( calendar_list_h list, calendar_record_h* record );

/**
 * @brief		Moves a calendar list to previous position.
 *
 * @param[in]	calendar_list				The calendar list handle
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see calendar_list_next()
 */
API int calendar_list_prev( calendar_list_h list );

/**
 * @brief		Moves a calendar list to next position.
 *
 * @param[in]	calendar_list				The calendar list handle
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see calendar_list_prev()
 */
API int calendar_list_next( calendar_list_h list );

/**
 * @brief		Moves a calendar list to the first position.
 *
 * @param[in]	calendar_list				The calendar list handle
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see calendar_list_last()
 */
API int calendar_list_first( calendar_list_h list );

/**
 * @brief		Moves a calendar lis tto the last position.
 *
 * @param[in]	calendar_list				The calendar list handle
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see calendar_list_first()
 */
API int calendar_list_last( calendar_list_h list );

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_SOCAIL_CALENDAR_LIST_H__ */

