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

#ifndef __TIZEN_SOCIAL_CALENDAR_RECORD_H__
#define __TIZEN_SOCIAL_CALENDAR_RECORD_H__

#include <calendar_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file calendar_record.h
 */

/**
 * @addtogroup CAPI_SOCIAL_CALENDAR_SVC_RECORD_MODULE
 * @{
 */

/**
 * @brief Creates a record handle.
 *
 * @since_tizen 2.3
 *
 * @remarks You must release @a record using calendar_record_destroy().
 *
 * @param[in]	view_uri    The view URI
 * @param[out]	out_record  The record handle
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_NOT_PERMITTED       Operation not permitted
 *
 * @pre     calendar_connect() should be called to initialize.
 *
 * @see calendar_record_destroy()
 */
int calendar_record_create(const char* view_uri, calendar_record_h* out_record);

/**
 * @brief Destroys a record handle and releases all its resources.
 *
 * @since_tizen 2.3
 *
 * @param[in]	record          The record handle
 * @param[in]	delete_child    If @c true, child records are destroyed automatically,
 *                              otherwise @c false
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                 Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER    Invalid parameter
 * @retval  #CALENDAR_ERROR_NOT_PERMITTED        Operation not permitted
 *
 * @see calendar_record_create()
 */
int calendar_record_destroy(calendar_record_h record, bool delete_child);

/**
 * @brief Makes a clone of a record handle.
 *
 * @since_tizen 2.3
 *
 * @remarks You must release @a cloned_record using calendar_record_destroy().
 *
 * @param[in]	record          The record handle
 * @param[out]	out_record      The cloned record handle
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                    Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER       Invalid parameter
 * @retval  #CALENDAR_ERROR_NOT_PERMITTED           Operation not permitted
 *
 * @see calendar_record_destroy()
 */
int calendar_record_clone(calendar_record_h record, calendar_record_h* out_record);

/**
 * @brief Gets a URI string from a record.
 *
 * @since_tizen 2.3
 *
 * @param[in]   record   The record handle
 * @param[out]  uri      The URI of the record
 *
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                 Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER    Invalid parameter
 */
int calendar_record_get_uri_p(calendar_record_h record, char** uri);

/**
 * @brief Gets a string from a record.
 *
 * @since_tizen 2.3
 *
 * @remarks You must release @a value using free().
 *
 * @param[in]   record        The record handle
 * @param[in]   property_id   The property ID
 * @param[out]  out_str       The result value
 *
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_NOT_PERMITTED       Operation not permitted
 *
 * @see calendar_record_get_str_p()
 * @see calendar_record_set_str()
 */
int calendar_record_get_str(calendar_record_h record, unsigned int property_id, char** out_str);

/**
 * @brief Gets a string pointer from a record.
 *
 * @since_tizen 2.3
 *
 * @remarks You MUST NOT release @a value.
 *
 * @param[in]   record         The record handle
 * @param[in]   property_id    The property ID
 * @param[out]  out_str        The result value
 *
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_NOT_PERMITTED       Operation not permitted
 *
 * @see calendar_record_get_str()
 * @see calendar_record_set_str()
 */
int calendar_record_get_str_p(calendar_record_h record, unsigned int property_id, char** out_str);

/**
 * @brief Gets an integer value from a record.
 *
 * @since_tizen 2.3
 *
 * @param[in]   record          The record handle
 * @param[in]   property_id     The property ID
 * @param[out]  out_value       The result value
 *
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_NOT_PERMITTED       Operation not permitted
 *
 * @see calendar_record_set_int()
 */
int calendar_record_get_int(calendar_record_h record, unsigned int property_id, int* out_value);

/**
 * @brief Gets a double value from a record.
 *
 * @since_tizen 2.3
 *
 * @param[in]   record          The record handle
 * @param[in]   property_id     The property ID
 * @param[out]  out_value       The result value
 *
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_NOT_PERMITTED       Operation not permitted
 *
 * @see calendar_record_set_double()
 */
int calendar_record_get_double(calendar_record_h record, unsigned int property_id, double* out_value);

/**
 * @brief Gets a long long integer value from a record.
 *
 * @since_tizen 2.3
 *
 * @param[in]   record          The record handle
 * @param[in]   property_id	    The property ID
 * @param[out]  out_value       The result value
 *
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_NOT_PERMITTED       Operation not permitted
 *
 * @see calendar_record_set_lli()
 */
int calendar_record_get_lli(calendar_record_h record, unsigned int property_id, long long int* out_value);

/**
 * @brief Gets a calendar_caltime_s value from a record.
 *
 * @since_tizen 2.3
 *
 * @param[in]   record          The record handle
 * @param[in]   property_id     The property ID
 * @param[out]  out_value       The result value
 *
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_NOT_PERMITTED       Operation not permitted
 *
 * @see calendar_record_set_caltime()
 */
int calendar_record_get_caltime(calendar_record_h record, unsigned int property_id, calendar_time_s* out_value);

/**
 * @brief Sets a string to a record.
 *
 * @since_tizen 2.3
 *
 * @param[in]	record          The record handle
 * @param[in]	property_id     The property ID
 * @param[in]	value           The value to be set
 *
 * @return      @c 0 on success,
 *              otherwise a negative error value
 * @retval      #CALENDAR_ERROR_NONE                  Successful
 * @retval      #CALENDAR_ERROR_INVALID_PARAMETER     Invalid parameter
 * @retval  	#CALENDAR_ERROR_NOT_PERMITTED         Operation not permitted
 *
 * @see calendar_record_get_str()
 * @see calendar_record_get_str_p()
 */
int calendar_record_set_str(calendar_record_h record, unsigned int property_id, const char* value);

/**
 * @brief Sets an integer value to a record.
 *
 * @since_tizen 2.3
 *
 * @param[in]	record          The record handle
 * @param[in]	property_id	    The property ID
 * @param[in]	value           The value to be set
 *
 * @return      @c 0 on success,
 *              otherwise a negative error value
 * @retval      #CALENDAR_ERROR_NONE                  Successful
 * @retval      #CALENDAR_ERROR_INVALID_PARAMETER     Invalid parameter
 * @retval  	#CALENDAR_ERROR_NOT_PERMITTED         Operation not permitted
 *
 * @see calendar_record_get_int()
 */
int calendar_record_set_int(calendar_record_h record, unsigned int property_id, int value);

/**
 * @brief Sets a double value to a record.
 *
 * @since_tizen 2.3
 *
 * @param[in]	record          The record handle
 * @param[in]	property_id     The property ID
 * @param[in]	value           The value to be set
 *
 * @return      @c 0 on success,
 *              otherwise a negative error value
 * @retval      #CALENDAR_ERROR_NONE                  Successful
 * @retval      #CALENDAR_ERROR_INVALID_PARAMETER     Invalid parameter
 * @retval  	#CALENDAR_ERROR_NOT_PERMITTED         Operation not permitted
 *
 * @see calendar_record_get_double()
 */
int calendar_record_set_double(calendar_record_h record, unsigned int property_id, double value);

/**
 * @brief Sets a long long integer value to a record.
 *
 * @since_tizen 2.3
 *
 * @param[in]	record          The record handle
 * @param[in]	property_id     The property ID
 * @param[in]	value           The value to be set
 *
 * @return      @c 0 on success,
 *              otherwise a negative error value
 * @retval      #CALENDAR_ERROR_NONE                  Successful
 * @retval      #CALENDAR_ERROR_INVALID_PARAMETER     Invalid parameter
 * @retval  	#CALENDAR_ERROR_NOT_PERMITTED         Operation not permitted
 *
 * @see calendar_record_get_lli()
 */
int calendar_record_set_lli(calendar_record_h record, unsigned int property_id, long long int value);

/**
 * @brief Sets a calendar_time_s value to a record.
 *
 * @since_tizen 2.3
 *
 * @param[in]	record          The record handle
 * @param[in]	property_id     The property ID
 * @param[in]	value           The value to be set
 *
 * @return      @c 0 on success,
 *              otherwise a negative error value
 * @retval      #CALENDAR_ERROR_NONE                  Successful
 * @retval      #CALENDAR_ERROR_INVALID_PARAMETER     Invalid parameter
 * @retval  	#CALENDAR_ERROR_NOT_PERMITTED         Operation not permitted
 *
 * @see calendar_record_get_caltime()
 */
int calendar_record_set_caltime(calendar_record_h record, unsigned int property_id, calendar_time_s value);

/**
 * @brief Adds a child record to the parent record.
 *
 * @since_tizen 2.3
 *
 * @param[in]	record          The parent record handle
 * @param[in]	property_id     The property ID
 * @param[in]	child_record    The handle of the child record to be added to the parent record
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                  Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER     Invalid parameter
 * @retval  #CALENDAR_ERROR_NOT_PERMITTED         Operation not permitted
 *
 * @see calendar_record_remove_child_record()
 */
int calendar_record_add_child_record(calendar_record_h record, unsigned int property_id, calendar_record_h child_record);

/**
 * @brief Removes a child record from the parent record.
 *
 * @since_tizen 2.3
 *
 * @param[in]	record        The parent record handle
 * @param[in]	property_id   The property ID
 * @param[in]	child_record  The handle of the child record to be removed from the parent record
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_NOT_PERMITTED       Operation not permitted
 *
 * @see calendar_record_add_child_record()
 */
int calendar_record_remove_child_record(calendar_record_h record, unsigned int property_id, calendar_record_h child_record);

/**
 * @brief Gets the number of child records in a record.
 *
 * @since_tizen 2.3
 *
 * @param[in]	record          The parent record handle
 * @param[in]	property_id     The property ID
 * @param[out]	count           The child record count
 *
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_NOT_PERMITTED       Operation not permitted
 *
 * @see calendar_record_add_child_record()
 * @see calendar_record_remove_child_record()
 */
int calendar_record_get_child_record_count(calendar_record_h record, unsigned int property_id,unsigned int* count);

/**
 * @brief Gets a child record handle pointer from the parent record.
 *
 * @since_tizen 2.3
 *
 * @remarks You MUST NOT release @a child_record. \n It is released when the parent record handle is destroyed.
 *
 * @param[in]   record          The record handle
 * @param[in]   property_id     The property ID
 * @param[in]   index           The index of the child record
 * @param[out]  child_record    The child record handle pointer
 *
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_NOT_PERMITTED       Operation not permitted
 *
 * @see calendar_record_add_child_record()
 * @see calendar_record_remove_child_record()
 * @see calendar_record_get_child_record_count()
 */
int calendar_record_get_child_record_at_p(calendar_record_h record, unsigned int property_id, int index, calendar_record_h* child_record);

/**
 * @brief Makes a clone of a given record's child record list.
 *
 * @since_tizen 2.3
 *
 * @remarks You must release @a cloned_list using calendar_list_destroy().
 *
 * @param[in]   record          The record handle
 * @param[in]   property_id     The property ID
 * @param[out]  out_list        The cloned list handle
 *
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_NOT_PERMITTED       Operation not permitted
 *
 * @see calendar_list_destroy()
 */
int calendar_record_clone_child_record_list(calendar_record_h record, unsigned int property_id, calendar_list_h* out_list);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_SOCIAL_CALENDAR_RECORD_H__ */

