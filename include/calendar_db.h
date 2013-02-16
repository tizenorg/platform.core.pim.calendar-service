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

#ifndef __TIZEN_SOCAIL_CALENDAR_DB_H__
#define __TIZEN_SOCAIL_CALENDAR_DB_H__

#include <calendar_types2.h>

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
 * @brief   Inserts a record to the calendar database.
 *
 * @param[in]   record                 The record handle
 * @param[out]  record_id              The record ID
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_DB_FAILED			Database operation failure
 *
 * @pre     This function requires an open connection to calendar service by calendar_connect().
 *
 * @see calendar_connect()
 * @see calendar_db_update_record()
 * @see calendar_db_delete_record()
 * @see calendar_db_get_record()
 */
API int calendar_db_insert_record( calendar_record_h record, int* record_id );

/**
 * @brief   Gets a record from the calendar database.
 *
 * @details This function creates a new record handle from the calendar database by the given @a record_id. \n
 * @a record will be created, which is filled with record information.
 *
 * @remarks  @a record must be released with calendar_record_destroy() by you.
 *
 * @param[in]   view_uri	The view URI of a record
 * @param[in]   record_id	The record ID to get from database
 * @param[out]  record		The record handle associated with the record ID
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_DB_FAILED			Database operation failure
 *
 * @pre     This function requires an open connection to calendar service by calendar_connect().
 *
 * @see calendar_connect()
 * @see calendar_record_destroy()
 */
API int	calendar_db_get_record( const char* view_uri, int record_id, calendar_record_h* record );

/**
 * @brief Updates a record to the calendar database.
 *
 * @param[in]   record          The record handle
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_DB_FAILED			Database operation failure
 *
 * @pre     This function requires an open connection to calendar service by calendar_connect().
 *
 * @see calendar_connect()
 * @see calendar_db_insert_record()
 * @see calendar_db_delete_record()
 * @see calendar_db_get_record()
 */
API int calendar_db_update_record( calendar_record_h record );

/**
 * @brief Deletes a record from the calendar database.
 *
 * @param[in]   view_uri	The view URI of a record
 * @param[in]   record_id	The record ID to delete
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_DB_FAILED			Database operation failure
 *
 * @pre     This function requires an open connection to calendar service by calendar_connect().
 *
 * @see calendar_connect()
 * @see calendar_db_insert_record()
 */
API int calendar_db_delete_record( const char* view_uri, int record_id );

/**
 * @brief       Retrieves all record as list
 *
 * @remarks     @a record_list must be released with calendar_list_destroy() by you.
 *
 * @param[in]   view_uri		The view URI to get records
 * @param[in]   offset			The index to get results from which index
 * @param[in]   limit			The number to limit results
 * @param[out]  record_list		The record list
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE				Successful
 * @retval  #CALENDAR_ERROR_OUT_OF_MEMORY		Out of memory
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval  #CALENDAR_ERROR_DB_FAILED			Database operation failure
 *
 * @pre    This function requires an open connection to calendar service by calendar_connect().
 *
 * @see calendar_connect()
 * @see calendar_list_destroy()
 */
API int calendar_db_get_all_records( const char* view_uri, int offset, int limit, calendar_list_h* record_list );

/**
 * @brief       Retrieves records with query handle
 *
 * @remarks     @a record_list must be released with calendar_list_destroy() by you.
 *
 * @param[in]   query			The query handle to filter
 * @param[in]   offset			The index to get results from which index
 * @param[in]   limit			The number to limit results
 * @param[out]  record_list		The record list
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE				Successful
 * @retval  #CALENDAR_ERROR_OUT_OF_MEMORY		Out of memory
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval  #CALENDAR_ERROR_DB_FAILED			Database operation failure
 *
 * @pre    This function requires an open connection to calendar service by calendar_connect().
 *
 * @see calendar_connect()
 * @see calendar_list_destroy()
 */
API int calendar_db_get_records_with_query( calendar_query_h query, int offset, int limit, calendar_list_h* record_list );

/**
 * @brief       Cleans data after sync
 *
 * @param[in]   calendar_book_id			The calendar book ID
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE				Successful
 * @retval  #CALENDAR_ERROR_OUT_OF_MEMORY		Out of memory
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval  #CALENDAR_ERROR_DB_FAILED			Database operation failure
 *
 * @pre    This function requires an open connection to calendar service by calendar_connect().
 *
 * @see calendar_connect()
 */
API int calendar_db_clean_after_sync( int calendar_book_id ); // calendar_svc_clean_after_sync  for EAS sync

/**
 * @brief       Gets records count of a specific view
 *
 * @param[in]   view_uri		The view URI to get records
 * @param[out]  count			The count of records
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE				Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval  #CALENDAR_ERROR_DB_FAILED			Database operation failure
 *
 * @pre    This function requires an open connection to calendar service by calendar_connect2().
 *
 * @see calendar_connect()
 */
API int calendar_db_get_count( const char* view_uri, int *count );

/**
 * @brief       Gets records count with a query handle
 *
 * @param[in]   query			The query handle to filter
 * @param[out]  count			The count of records
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE				Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval  #CALENDAR_ERROR_DB					Database operation failure
 *
 * @pre    This function requires an open connection to calendar service by calendar_connect2().
 *
 * @see calendar_connect2()
 */
API int calendar_db_get_count_with_query( calendar_query_h query, int *count );

/**
 * @brief   Inserts multiple records as batch operation to the calendar database.
 *
 * @param[in]   record_list			The record list handle
 * @param[out]  record_id_array	    The record IDs
 * @param[out]  count			    The number of record ID array
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_DB_FAILED           Database operation failure
 *
 * @pre     This function requires an open connection to calendar service by calendar_connect().
 *
 * @see calendar_connect()
 * @see calendar_db_update_records()
 * @see calendar_db_delete_records()
 * @see calendar_db_insert_records_async()
 */
API int calendar_db_insert_records( calendar_list_h record_list, int** record_id_array, int* count);

/**
 * @brief   Inserts multiple records as batch operation to the calendar database.
 *
 * @param[in]   record_list         The record list handle
 * @param[in]   callback            The callback function to invoke which lets you know result of batch operation
 * @param[in]   user_data           The user data to be passed to the callback function
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_DB_FAILED           Database operation failure
 *
 * @pre     This function requires an open connection to calendar service by calendar_connect().
 *
 * @see calendar_connect()
 * @see calendar_db_update_records()
 * @see calendar_db_delete_records()
 * @see calendar_db_insert_result_cb
 */
API int calendar_db_insert_records_async( calendar_list_h record_list, calendar_db_insert_result_cb callback, void *user_data);

/**
 * @brief   Updates multiple records as batch operation to the calendar database.
 *
 * @param[in]   record_list			The record list handle
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_DB_FAILED           Database operation failure
 *
 * @pre     This function requires an open connection to calendar service by calendar_connect().
 *
 * @see calendar_connect()
 * @see calendar_db_insert_records()
 * @see calendar_db_delete_records()
 * @see calendar_db_update_records_async()
 */
API int calendar_db_update_records( calendar_list_h record_list);

/**
 * @brief   Updates multiple records as batch operation to the calendar database.
 *
 * @param[in]   record_list         The record list handle
 * @param[in]   callback            The callback function to invoke which lets you know result of batch operation
 * @param[in]   user_data           The user data to be passed to the callback function
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_DB_FAILED           Database operation failure
 *
 * @pre     This function requires an open connection to calendar service by calendar_connect().
 *
 * @see calendar_connect()
 * @see calendar_db_insert_records()
 * @see calendar_db_delete_records()
 * @see calendar_db_result_cb
 */
API int calendar_db_update_records_async( calendar_list_h record_list, calendar_db_result_cb callback, void *user_data);

/**
 * @brief   Deletes multiple records as batch operation to the calendar database.
 *
 * @param[in]   view_uri			The view URI of records
 * @param[in]   record_id_array		The record IDs to delete
 * @param[in]   count				The number of record ID array
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_DB_FAILED           Database operation failure
 *
 * @pre     This function requires an open connection to calendar service by calendar_connect().
 *
 * @see calendar_connect()
 * @see calendar_db_insert_records()
 * @see calendar_db_delete_records()
 * @see calendar_db_delete_records_async()
 */
API int calendar_db_delete_records(const char* view_uri, int record_id_array[], int count);

/**
 * @brief   Deletes multiple records as batch operation to the calendar database.
 *
 * @param[in]   view_uri            The view URI of records
 * @param[in]   record_id_array     The record IDs to delete
 * @param[in]   count               The number of record ID array
 * @param[in]   callback            The callback function to invoke which lets you know result of batch operation
 * @param[in]   user_data           The user data to be passed to the callback function
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_DB_FAILED           Database operation failure
 *
 * @pre     This function requires an open connection to calendar service by calendar_connect().
 *
 * @see calendar_connect()
 * @see calendar_db_insert_records()
 * @see calendar_db_delete_records()
 * @see calendar_db_result_cb()
 */
API int calendar_db_delete_records_async(const char* view_uri, int record_id_array[], int count, calendar_db_result_cb callback, void *user_data);

/**
 * @brief	Gets the current calendar database version.
 *
 * @param[out]  calendar_db_version    The calendar database version
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE				Successful
 * @retval	#CALENDAR_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval  #CALENDAR_ERROR_DB_FAILED			Database operation failure
 *
 * @pre     This function requires an open connection to the calendar service by calendar_connect().
 *
 * @see calendar_connect()
 * @see calendar_db_get_changes_by_version()
 */
API int calendar_db_get_current_version(int* calendar_db_version);

/**
 * @brief       Registers a callback function to be invoked when the record changes.
 *
 * @param[in]   view_uri	The view URI of record to subscribe to changing notifications
 * @param[in]   callback	The callback function to register
 * @param[in]	user_data	The user data to be passed to the callback function
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval	#CALENDAR_ERROR_NONE                Successful
 * @retval	#CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @pre		This function requires an open connection to the calendar service by calendar_connect().
 * @post	calendar_db_changed_cb() will be invoked when the designated view changes.
 *
 * @see calendar_connect()
 * @see calendar_db_changed_cb()
 * @see calendar_db_remove_changed_cb()
 */
API int calendar_db_add_changed_cb(const char* view_uri, calendar_db_changed_cb callback, void* user_data );

/**
 * @brief       Unregisters a callback function.
 *
 * @param[in]   view_uri	The view URI of record to subscribe to changing notifications
 * @param[in]   callback	The callback function to register
 * @param[in]	user_data	The user data to be passed to the callback function
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval	#CALENDAR_ERROR_NONE                Successful
 * @retval	#CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @pre		This function requires an open connection to the calendar service by calendar_connect().
 *
 * @see calendar_connect()
 * @see calendar_db_changed_cb()
 * @see calendar_db_add_changed_cb()
 */
API int calendar_db_remove_changed_cb( const char* view_uri, calendar_db_changed_cb callback, void* user_data );

/**
 * @brief       Retrieves records with the calendar database version.
 *
 * @details		This function will find all changed records since the given @a calendar_db_version
 *
 * @remarks     @a change_record_list must be released with calendar_list_destroy() by you.
 *
 * @param[in]   view_uri					The view URI to get records
 * @param[in]   calendar_book_id				The calendar book ID to filter
 * @param[in]   calendar_db_version			The calendar database version
 * @param[out]  record_list					The record list
 * @param[out]  current_calendar_db_version	The current calendar database version
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE				Successful
 * @retval  #CALENDAR_ERROR_OUT_OF_MEMORY		Out of memory
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval  #CALENDAR_ERROR_DB_FAILED			Database operation failure
 *
 * @pre    This function requires an open connection to calendar service by calendar_connect().
 *
 * @see calendar_connect()
 * @see calendar_list_destroy()
 */
API int calendar_db_get_changes_by_version(const char* view_uri, int calendar_book_id, int calendar_db_version, calendar_list_h* record_list, int *current_calendar_db_version );

/**
 * @brief   Inserts vcalendar stream to the calendar database.
 *
 * @param[in]   vcalendar_stream                The vcalendar stream
 * @param[out]  record_id_array                 The record IDs to delete
 * @param[out]  count                           The number of record ID array
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_DB_FAILED           Database operation failure
 *
 * @pre     This function requires an open connection to calendar service by calendar_connect().
 *
 * @see calendar_connect()
 * @see calendar_db_replace_vcalendars()
 */
API int calendar_db_insert_vcalendars(const char* vcalendar_stream, int **record_id_array, int *count);

/**
 * @brief   Inserts vcalendar stream to the calendar database.
 *
 * @param[in]   vcalendar_stream                The vcalendar stream
 * @param[in]   callback                        The callback function to invoke which lets you know result of batch operation
 * @param[in]   user_data                       The user data to be passed to the callback function
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_DB_FAILED           Database operation failure
 *
 * @pre     This function requires an open connection to calendar service by calendar_connect().
 *
 * @see calendar_connect()
 * @see calendar_db_replace_vcalendars()
 * @see calendar_db_insert_result_cb
 */
API int calendar_db_insert_vcalendars_async(const char* vcalendar_stream, calendar_db_insert_result_cb callback, void *user_data);

/**
 * @brief   Replaces vcalendar stream to the calendar database.
 *
 * @param[in]   vcalendar_stream                The vcalendar stream
 * @param[in]   record_id_array                 The record IDs to replace
 * @param[in]   count                           The number of record ID array
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_DB_FAILED           Database operation failure
 *
 * @pre     This function requires an open connection to calendar service by calendar_connect().
 *
 * @see calendar_connect()
 * @see calendar_db_replace_vcalendars()
 */
API int calendar_db_replace_vcalendars(const char* vcalendar_stream, int *record_id_array, int count);

/**
 * @brief   Replaces vcalendar stream to the calendar database.
 *
 * @param[in]   vcalendar_stream                The vcalendar stream
 * @param[in]   record_id_array                 The record IDs to replace
 * @param[in]   count                           The number of record ID array
 * @param[in]   callback                        The callback function to invoke which lets you know result of batch operation
 * @param[in]   user_data                       The user data to be passed to the callback function
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_DB_FAILED           Database operation failure
 *
 * @pre     This function requires an open connection to calendar service by calendar_connect().
 *
 * @see calendar_connect()
 * @see calendar_db_replace_vcalendars()
 */
API int calendar_db_replace_vcalendars_async(const char* vcalendar_stream, int *record_id_array, int count, calendar_db_result_cb callback, void *user_data);

/**
 * @brief   Replaces a record to the calendar database.
 *
 * @param[in]   record                 The record handle
 * @param[in]   record_id              The record ID
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_DB_FAILED           Database operation failure
 *
 * @pre     This function requires an open connection to calendar service by calendar_connect().
 *
 * @see calendar_connect()
 * @see calendar_db_update_record()
 * @see calendar_db_delete_record()
 * @see calendar_db_get_record()
 */
API int calendar_db_replace_record(calendar_record_h record, int record_id);

/**
 * @brief   Replaces multiple records as batch operation to the calendar database.
 *
 * @param[in]   record_list         The record list handle
 * @param[in]   record_id_array     The record IDs
 * @param[in]   count               The number of record ID array
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_DB_FAILED           Database operation failure
 *
 * @pre     This function requires an open connection to calendar service by calendar_connect().
 *
 * @see calendar_connect()
 * @see calendar_db_update_records()
 * @see calendar_db_delete_records()
 * @see calendar_db_replace_record()
 * @see calendar_db_replace_records_async()
 */
API int calendar_db_replace_records(calendar_list_h record_list, int *record_id_array, int count);

/**
 * @brief   Replaces multiple records as batch operation to the calendar database.
 *
 * @param[in]   record_list         The record list handle
 * @param[in]   record_id_array     The record IDs
 * @param[in]   count               The number of record ID array
 * @param[in]   callback            The callback function to invoke which lets you know result of batch operation
 * @param[in]   user_data           The user data to be passed to the callback function
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_DB_FAILED           Database operation failure
 *
 * @pre     This function requires an open connection to calendar service by calendar_connect().
 *
 * @see calendar_connect()
 * @see calendar_db_update_records()
 * @see calendar_db_delete_records()
 * @see calendar_db_replace_record()
 * @see calendar_db_result_cb
 */
API int calendar_db_replace_records_async(calendar_list_h record_list, int *record_id_array, int count, calendar_db_result_cb callback, void *user_data);

/**
 * @brief   Gets the last change calendar database version on current connection.
 *
 * @param[out]  last_change_version    The calendar database version on current connection
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_DB_FAILED           Database operation failure
 *
 * @pre     This function requires an open connection to the calendar service by calendar_connect().
 *
 * @see calendar_connect()
 * @see calendar_db_get_current_version()
 */
API int calendar_db_get_last_change_version(int* last_change_version);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_SOCAIL_CALENDAR_DB_H__ */

