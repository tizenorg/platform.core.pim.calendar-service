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

#ifndef __TIZEN_SOCAIL_CALENDAR_RECORD_H__
#define __TIZEN_SOCAIL_CALENDAR_RECORD_H__

#include <calendar_types2.h>

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup CAPI_SOCIAL_CALENDAR_SVC_RECORD_MODULE
 * @{
 */

/**
 * @brief Creates a handle to the record.
 *
 * @remarks @a record must be released with calendar_record_destroy() by you.
 *
 * @param[in]	view_uri	The view uri
 * @param[out]	record    	The record handle
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER       Invalid parameter
 *
 * @see calendar_record_destroy()
 */
API int calendar_record_create( const char* view_uri, calendar_record_h* out_record );

/**
 * @brief Destroys a record handle and releases all its resources.
 *
 * @param[in]	record  	The record handle
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE                    Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER       Invalid parameter
 *
 * @see calendar_list_create()
 */
API int calendar_record_destroy( calendar_record_h record, bool delete_child );

/**
 * @brief	Makes a clone of a record handle.
 *
 * @remarks @a cloned_record must be released with calendar_record_destroy() by you.
 *
 * @param[in]	record  			The record handle
 * @param[out]	cloned_record    	The cloned record handle
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE                    Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER       Invalid parameter
 *
 * @see calendar_record_destroy()
 */
API int calendar_record_clone( calendar_record_h record, calendar_record_h* out_record );

/**
 * @brief	Gets uri string from a record handle.
 *
 * @param[in]   record			The record handle
 * @param[out]  uri  			The uri of record
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE                 Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER    Invalid parameter
 */
API int calendar_record_get_uri_p( calendar_record_h record, char** uri );

/**
 * @brief	Gets a string from a record handle.
 *
 * @remarks   @a value must be released with free() by you.
 *
 * @param[in]   record			The record handle
 * @param[in]   property_id		The property ID
 * @param[out]  value  			The value to be returned
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE                 Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER    Invalid parameter
 *
 * @see calendar_record_get_str_p()
 * @see calendar_record_set_str()
 */
API int calendar_record_get_str( calendar_record_h record, unsigned int property_id, char** out_str );

/**
 * @brief	Gets a string pointer from a record handle.
 *
 * @remarks   @a value MUST NOT be released by you.
 *
 * @param[in]   record			The record handle
 * @param[in]   property_id		The property ID
 * @param[out]  value  			The value to be returned
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE                 Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER    Invalid parameter
 *
 * @see calendar_record_get_str()
 * @see calendar_record_set_str()
 */
API int calendar_record_get_str_p( calendar_record_h record, unsigned int property_id, char** out_str );

/**
 * @brief   Gets a integer from a record handle.
 *
 * @param[in]   record			The record handle
 * @param[in]   property_id		The property ID
 * @param[out]  value  			The value to be returned
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see calendar_record_set_int()
 */
API int calendar_record_get_int( calendar_record_h record, unsigned int property_id, int* out_value );

/**
 * @brief   Gets a double from a record handle.
 *
 * @param[in]   record			The record handle
 * @param[in]   property_id		The property ID
 * @param[out]  value  			The value to be returned
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see calendar_record_set_double()
 */
API int calendar_record_get_double( calendar_record_h record, unsigned int property_id, double* out_value );

/**
 * @brief   Gets a long long integer from a record handle.
 *
 * @param[in]   record			The record handle
 * @param[in]   property_id		The property ID
 * @param[out]  value  			The value to be returned
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see calendar_record_set_lli()
 */
API int calendar_record_get_lli( calendar_record_h record, unsigned int property_id, long long int* out_value );

/**
 * @brief   Gets a caltime_caltime_s from a record handle.
 *
 * @param[in]   record			The record handle
 * @param[in]   property_id		The property ID
 * @param[out]  value  			The value to be returned
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see calendar_record_set_caltime()
 */
API int calendar_record_get_caltime( calendar_record_h record, unsigned int property_id, calendar_time_s* out_value );

/**
 * @brief   Sets a string to a record handle.
 *
 * @param[in]	record			The record handle
 * @param[in]	property_id		The property ID
 * @param[in]	value  			The value to set
 *
 * @return      0 on success, otherwise a negative error value.
 * @retval      #CALENDAR_ERROR_NONE                    Successful
 * @retval      #CALENDAR_ERROR_INVALID_PARAMETER       Invalid parameter
 *
 * @see calendar_record_get_str()
 * @see calendar_record_get_str_p()
 */
API int calendar_record_set_str( calendar_record_h record, unsigned int property_id, const char* value );

/**
 * @brief   Sets a integer to a record handle.
 *
 * @param[in]	record			The record handle
 * @param[in]	property_id		The property ID
 * @param[in]	value  			The value to set
 *
 * @return      0 on success, otherwise a negative error value.
 * @retval      #CALENDAR_ERROR_NONE                    Successful
 * @retval      #CALENDAR_ERROR_INVALID_PARAMETER       Invalid parameter
 *
 * @see calendar_record_get_double()
 */
API int calendar_record_set_int( calendar_record_h record, unsigned int property_id, int value );

/**
 * @brief   Sets a double to a record handle.
 *
 * @param[in]	record			The record handle
 * @param[in]	property_id		The property ID
 * @param[in]	value  			The value to set
 *
 * @return      0 on success, otherwise a negative error value.
 * @retval      #CALENDAR_ERROR_NONE                    Successful
 * @retval      #CALENDAR_ERROR_INVALID_PARAMETER       Invalid parameter
 *
 * @see calendar_record_get_int()
 */
API int calendar_record_set_double( calendar_record_h record, unsigned int property_id, double value );

/**
 * @brief   Sets a long long integer to a record handle.
 *
 * @param[in]	record			The record handle
 * @param[in]	property_id		The property ID
 * @param[in]	value  			The value to set
 *
 * @return      0 on success, otherwise a negative error value.
 * @retval      #CALENDAR_ERROR_NONE                    Successful
 * @retval      #CALENDAR_ERROR_INVALID_PARAMETER       Invalid parameter
 *
 * @see calendar_record_get_lli()
 */
API int calendar_record_set_lli( calendar_record_h record, unsigned int property_id, long long int value );

/**
 * @brief   Sets a long calendar_time_s to a record handle.
 *
 * @param[in]	record			The record handle
 * @param[in]	property_id		The property ID
 * @param[in]	value  			The value to set
 *
 * @return      0 on success, otherwise a negative error value.
 * @retval      #CALENDAR_ERROR_NONE                    Successful
 * @retval      #CALENDAR_ERROR_INVALID_PARAMETER       Invalid parameter
 *
 * @see calendar_record_get_caltime()
 */
API int calendar_record_set_caltime( calendar_record_h record, unsigned int property_id, calendar_time_s value );

/**
 * @brief       Adds a child record handle to a parent record handle.
 *
 * @param[in]	record          The parent record handle
 * @param[in]	property_id		The property ID
 * @param[in]	child_record	The child record handle to be added to parent record handle
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE                    Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER       Invalid parameter
 *
 * @see calendar_record_remove_child_record()
 */
API int calendar_record_add_child_record( calendar_record_h record, unsigned int property_id, calendar_record_h child_record );

/**
 * @brief       Removes a child record handle from a parent record handle.
 *
 * @param[in]	record          The parent record handle
 * @param[in]	property_id		The property ID
 * @param[in]	child_record	The child record handle to be removed from parent record handle
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE                    Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER       Invalid parameter
 *
 * @see calendar_record_add_child_record()
 */
API int calendar_record_remove_child_record( calendar_record_h record, unsigned int property_id, calendar_record_h child_record );

/**
 * @brief   Gets a number of child record handle from a parent record handle.
 *
 * @param[in]	record          The parent record handle
 * @param[in]	property_id		The property ID
 * @param[out]	count			The child record count
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see calendar_record_add_child_record()
 * @see calendar_record_remove_child_record()
 */
API int calendar_record_get_child_record_count( calendar_record_h record, unsigned int property_id,unsigned int* count );

/**
 * @brief	Gets a child record handle pointer from a parent record handle.
 *
 * @remarks   @a child_record MUST NOT be released by you. \n It is released when the parent record handle destroyed.
 *
 * @param[in]   record			The record handle
 * @param[in]   property_id		The property ID
 * @param[in]   index			The index of child record
 * @param[out]  child_record  	The child record handle pointer to be returned
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE                 Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER    Invalid parameter
 *
 * @see calendar_record_add_child_record()
 * @see calendar_record_remove_child_record()
 * @see calendar_record_get_child_record_count()
 */
API int calendar_record_get_child_record_at_p( calendar_record_h record, unsigned int property_id, int index, calendar_record_h* child_record );

/**
 * @brief	Makes a clone of a child record list handle from a parent record handle.
 *
 * @remarks   @a cloned_list MUST be released with calendar_list_destroy() by you.
 *
 * @param[in]   record			The record handle
 * @param[in]   property_id		The property ID
 * @param[out]  cloned_list  	The cloned list handle
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE                 Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER    Invalid parameter
 *
 * @see calendar_list_destroy()
 */
API int calendar_record_clone_child_record_list( calendar_record_h record, unsigned int property_id, calendar_list_h* out_list );

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_SOCAIL_CALENDAR_RECORD_H__ */

