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

#ifndef __TIZEN_SOCIAL_CALENDAR_FILTER_H__
#define __TIZEN_SOCIAL_CALENDAR_FILTER_H__

#include <calendar_types.h>

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file calendar_filter.h
 */

/**
 * @addtogroup CAPI_SOCIAL_CALENDAR_SVC_FILTER_MODULE
 * @{
 */

/**
 * @brief Creates a filter handle.
 *
 * @since_tizen 2.3
 *
 * @remarks You must release @a filter using calendar_filter_destroy().
 *
 * @param[in]   view_uri     The view URI of a filter
 * @param[out]  filter       The filter handle
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_OUT_OF_MEMORY       Out of memory
 *
 * @pre     calendar_connect() should be called to initialize.
 *
 * @see calendar_filter_destroy()
 */
int calendar_filter_create(const char* view_uri, calendar_filter_h* filter);

/**
 * @brief Destroys a filter handle.
 *
 * @since_tizen 2.3
 *
 * @param[in]   filter   The filter handle
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see calendar_filter_create()
 */
int	calendar_filter_destroy(calendar_filter_h filter);

/**
 * @brief Adds a condition for the string type property.
 *
 * @since_tizen 2.3
 *
 * @param[in]   filter          The filter handle
 * @param[in]   property_id     The property ID to add a condition
 * @param[in]   match           The match flag
 * @param[in]   match_value     The match value
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see calendar_filter_add_operator()
 */
int	calendar_filter_add_str(calendar_filter_h filter, unsigned int property_id, calendar_match_str_flag_e match, const char* match_value);

/**
 * @brief Adds a condition for the integer type property.
 *
 * @since_tizen 2.3
 *
 * @param[in]   filter          The filter handle
 * @param[in]   property_id     The property ID to add a condition
 * @param[in]   match           The match flag
 * @param[in]   match_value     The match value
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see calendar_filter_add_operator()
 */
int	calendar_filter_add_int(calendar_filter_h filter, unsigned int property_id, calendar_match_int_flag_e match, int match_value);

/**
 * @brief Adds a condition for the double type property.
 *
 * @since_tizen 2.3
 *
 * @param[in]   filter          The filter handle
 * @param[in]   property_id     The property ID to add a condition
 * @param[in]   match           The match flag
 * @param[in]   match_value     The match value
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see calendar_filter_add_operator()
 */
int	calendar_filter_add_double(calendar_filter_h filter, unsigned int property_id, calendar_match_int_flag_e match, double match_value);

/**
 * @brief Adds a condition for the long long int type property.
 *
 * @since_tizen 2.3
 *
 * @param[in]   filter          The filter handle
 * @param[in]   property_id     The property ID to add a condition
 * @param[in]   match           The match flag
 * @param[in]   match_value     The match value
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see calendar_filter_add_operator()
 */
int	calendar_filter_add_lli(calendar_filter_h filter, unsigned int property_id, calendar_match_int_flag_e match, long long int match_value);

/**
 * @brief Adds a condition for the calendar_time_s type property.
 *
 * @since_tizen 2.3
 *
 * @param[in]   filter          The filter handle
 * @param[in]   property_id     The property ID to add a condition
 * @param[in]   match           The match flag
 * @param[in]   match_value     The match value
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see calendar_filter_add_operator()
 */
int	calendar_filter_add_caltime(calendar_filter_h filter, unsigned int property_id, calendar_match_int_flag_e match, calendar_time_s match_value);

/**
 * @brief Adds a child filter to a parent filter.
 *
 * @since_tizen 2.3
 *
 * @param[in]   parent_filter      The parent filter handle
 * @param[in]   child_filter       The child filter handle
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see calendar_filter_add_operator()
 */
int calendar_filter_add_filter(calendar_filter_h parent_filter, calendar_filter_h child_filter);

/**
 * @brief Adds an operator between conditions.
 *
 * @since_tizen 2.3
 *
 * @param[in]   filter          The filter handle
 * @param[in]   operator_type   The operator type
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see calendar_filter_add_str()
 * @see calendar_filter_add_int()
 * @see calendar_filter_add_double()
 * @see calendar_filter_add_caltime()
 * @see calendar_filter_add_filter()
 */
int calendar_filter_add_operator(calendar_filter_h filter, calendar_filter_operator_e operator_type);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_SOCIAL_CALENDAR_FILTER_H__ */

