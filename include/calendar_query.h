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

#ifndef __TIZEN_SOCIAL_CALENDAR_QUERY_H__
#define __TIZEN_SOCIAL_CALENDAR_QUERY_H__

#include <calendar_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file calendar_query.h
 */

/**
 * @addtogroup CAPI_SOCIAL_CALENDAR_SVC_QUERY_MODULE
 * @{
 */

/**
 * @brief Creates a query handle.
 *
 * @since_tizen 2.3
 *
 * @remarks You must release @a query using calendar_query_destroy().
 *
 * @param[in]   view_uri   The view URI of a query
 * @param[out]  query	   The filter handle
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_OUT_OF_MEMORY       Out of memory
 *
 * @pre     calendar_connect() should be called to initialize.
 *
 * @see calendar_query_destroy()
 */
int calendar_query_create(const char* view_uri, calendar_query_h* query);

/**
 * @brief Destroys a query handle.
 *
 * @since_tizen 2.3
 *
 * @param[in] query   The query handle
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE               Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see calendar_query_create()
 */
int calendar_query_destroy(calendar_query_h query);

/**
 * @brief Adds property IDs for projection.
 *
 * @details Property IDs can be of one of the properties of view_uri which is used in calendar_query_create().
 *
 * @since_tizen 2.3
 *
 * @param[in]   query               The query handle
 * @param[in]   property_id_array   The property ID array
 * @param[in]   count               The number of property IDs
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 */
int calendar_query_set_projection(calendar_query_h query, unsigned int property_id_array[], int count);

/**
 * @brief Sets the "distinct" option for projection.
 *
 * @since_tizen 2.3
 *
 * @param[in]   query   The query handle
 * @param[in]   set     If @c true it is set,
 *                      otherwise if @c false it is unset
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 */
int calendar_query_set_distinct(calendar_query_h query, bool set);

/**
 * @brief Sets the filter for a query.
 *
 * @since_tizen 2.3
 *
 * @param[in]  query    The query handle
 * @param[in]  filter   The filter handle
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_NO_DATA             Requested data does not exist
 *
 * @see calendar_query_add_operator()
 */
int calendar_query_set_filter(calendar_query_h query, calendar_filter_h filter);

/**
 * @brief Sets the sort mode for a query.
 *
 * @since_tizen 2.3
 *
 * @param[in]   query           The query handle
 * @param[in]   property_id     The property ID to sort
 * @param[in]   is_ascending    If @c true it sorts in the ascending order,
 *                              otherwise if @c false it sorts in the descending order
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 */
int calendar_query_set_sort(calendar_query_h query, unsigned int property_id, bool is_ascending);


/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_SOCIAL_CALENDAR_QUERY_H__ */

