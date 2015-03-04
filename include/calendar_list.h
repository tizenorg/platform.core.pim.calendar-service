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

#ifndef __TIZEN_SOCIAL_CALENDAR_LIST_H__
#define __TIZEN_SOCIAL_CALENDAR_LIST_H__

#include <calendar_types.h>

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
 * @brief Creates a calendar list handle.
 *
 * @since_tizen 2.3
 *
 * @remarks You must release @a calendar_list using calendar_list_destroy().
 *
 * @param[out]  out_list   The calendar list handle
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see calendar_list_destroy()
 */
int calendar_list_create( calendar_list_h* out_list );

/**
 * @brief Destroys a calendar list handle and releases all its resources.
 *
 * @since_tizen 2.3
 *
 * @param[in]   list            The calendar list handle
 * @param[in]   delete_record   If @c true, child records are destroyed automatically,
 *                              otherwise @c false
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                   Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER      Invalid parameter
 *
 * @see calendar_list_create()
 */
int calendar_list_destroy( calendar_list_h list, bool delete_record );


/**
 * @brief Retrieves the number of calendar entities in a calendar list.
 *
 * @since_tizen 2.3
 *
 * @param[in]	list     The calendar list handle
 * @param[out]	count    The count of the calendar entity
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see calendar_list_add()
 */
int calendar_list_get_count( calendar_list_h list, int *count );

/**
 * @brief Adds a record to the calendar list.
 *
 * @since_tizen 2.3
 *
 * @param[in]	list    The calendar list handle
 * @param[in]	record  The record handle
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see calendar_list_remove()
 */
int calendar_list_add( calendar_list_h list, calendar_record_h record );

/**
 * @brief Removes a record from the calendar list.
 * @details If the record is the current record, then the current record is changed to the next record.\n
 *          If the record is the last record then the current record will be @c NULL.
 *
 * @since_tizen 2.3
 *
 * @param[in]	list    The calendar list handle
 * @param[in]	record  The record handle
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_NO_DATA             Requested data does not exist
 *
 * @see calendar_list_add()
 */
int calendar_list_remove( calendar_list_h list, calendar_record_h record );

/**
 * @brief Retrieves a record from the calendar list.
 * @details The default current record is the first record.
 *
 * @since_tizen 2.3
 *
 * @remarks You MUST NOT destroy the @a record handle.
 *          It is destroyed automatically when the @a list is destroyed.
 *
 * @param[in]	list        The calendar list handle
 * @param[out]	record      The record handle
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_NO_DATA             Requested data does not exist
 */
int calendar_list_get_current_record_p( calendar_list_h list, calendar_record_h* record );

/**
 * @brief Moves a calendar list to the previous position.
 *
 * @since_tizen 2.3
 *
 * @param[in]  list  The calendar list handle
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_NO_DATA             Requested data does not exist
 *
 * @see calendar_list_next()
 */
int calendar_list_prev( calendar_list_h list );

/**
 * @brief Moves a calendar list to the next position.
 *
 * @since_tizen 2.3
 *
 * @param[in]  list  The calendar list handle
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_NO_DATA             Requested data does not exist
 *
 * @see calendar_list_prev()
 */
int calendar_list_next( calendar_list_h list );

/**
 * @brief Moves a calendar list to the first position.
 *
 * @since_tizen 2.3
 *
 * @param[in]  list  The calendar list handle
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see calendar_list_last()
 */
int calendar_list_first( calendar_list_h list );

/**
 * @brief Moves a calendar list to the last position.
 *
 * @since_tizen 2.3
 *
 * @param[in]  list  The calendar list handle
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see calendar_list_first()
 */
int calendar_list_last( calendar_list_h list );

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_SOCIAL_CALENDAR_LIST_H__ */

