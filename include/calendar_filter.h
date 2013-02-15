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

#ifndef __TIZEN_SOCAIL_CALENDAR_FILTER_H__
#define __TIZEN_SOCAIL_CALENDAR_FILTER_H__

#include <calendar_types2.h>

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup CAPI_SOCIAL_CALENDAR_SVC_FILTER_MODULE
 * @{
 */

/**
 * @brief   Creates a handle to filter.
 *
 * @remarks		@a filter must be released with calendar_filter_destroy() by you.
 *
 * @param[in]   view_uri			The view URI of a filter
 * @param[out]  filter				The filter handle
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_OUT_OF_MEMORY       Out of memory
 *
 * @see calendar_filter_destroy()
 */
API int calendar_filter_create( const char* view_uri, calendar_filter_h* filter );

/**
 * @brief   Destroys a filter handle.
 *
 * @param[in]   filter    The filter handle
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see calendar_filter_create()
 */
API int	calendar_filter_destroy( calendar_filter_h filter );

/**
 * @brief		Adds a condition for string type property
 *
 * @param[in]   filter			The filter handle
 * @param[in]   property_id		The property ID to add a condition
 * @param[in]   match			The match flag
 * @param[in]   match_value		The match value
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see calendar_filter_add_operator()
 */
API int	calendar_filter_add_str( calendar_filter_h filter, unsigned int property_id, calendar_match_str_flag_e match, const char* match_value );

/**
 * @brief		Adds a condition for integer type property
 *
 * @param[in]   filter			The filter handle
 * @param[in]   property_id		The property ID to add a condition
 * @param[in]   match			The match flag
 * @param[in]   match_value		The match value
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see calendar_filter_add_operator()
 */
API int	calendar_filter_add_int( calendar_filter_h filter, unsigned int property_id, calendar_match_int_flag_e match, int match_value );

/**
 * @brief		Adds a condition for double type property
 *
 * @param[in]   filter			The filter handle
 * @param[in]   property_id		The property ID to add a condition
 * @param[in]   match			The match flag
 * @param[in]   match_value		The match value
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see calendar_filter_add_operator()
 */
API int	calendar_filter_add_double( calendar_filter_h filter, unsigned int property_id, calendar_match_int_flag_e match, double match_value );

/**
 * @brief		Adds a condition for long long int type property
 *
 * @param[in]   filter			The filter handle
 * @param[in]   property_id		The property ID to add a condition
 * @param[in]   match			The match flag
 * @param[in]   match_value		The match value
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see calendar_filter_add_operator()
 */
API int	calendar_filter_add_lli( calendar_filter_h filter, unsigned int property_id, calendar_match_int_flag_e match, long long int match_value );

/**
 * @brief		Adds a condition for calendar_time_s type property
 *
 * @param[in]   filter			The filter handle
 * @param[in]   property_id		The property ID to add a condition
 * @param[in]   match			The match flag
 * @param[in]   match_value		The match value
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see calendar_filter_add_operator()
 */
API int	calendar_filter_add_caltime( calendar_filter_h filter, unsigned int property_id, calendar_match_int_flag_e match, calendar_time_s match_value );

/**
 * @brief		Adds a filter handle to filter handle.
 *
 * @param[in]   parent_filter		The parent filter handle
 * @param[in]   child_filter		The child filter handle
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see calendar_filter_add_operator()
 */

API int calendar_filter_add_filter( calendar_filter_h parent_filter, calendar_filter_h child_filter);
/**
 * @brief		Adds a operator between conditions
 *
 * @param[in]   filter			The filter handle
 * @param[in]   operator_type	The operator type
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see calendar_filter_add_str()
 * @see calendar_filter_add_int()
 * @see calendar_filter_add_bool()
 */
API int calendar_filter_add_operator( calendar_filter_h filter, calendar_filter_operator_e operator_type );

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_SOCAIL_CALENDAR_FILTER_H__ */

