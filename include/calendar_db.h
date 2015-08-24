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

#ifndef __TIZEN_SOCIAL_CALENDAR_DB_H__
#define __TIZEN_SOCIAL_CALENDAR_DB_H__

#include <calendar_types.h>

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file calendar_db.h
 */

/**
 * @addtogroup CAPI_SOCIAL_CALENDAR_SVC_DATABASE_MODULE
 * @{
 */

/**
 * @brief Called when a designated view changes.
 * @since_tizen 2.3
 *
 * @param[in]   view_uri   The view URI
 * @param[in]   user_data  The user data passed from the callback registration function
 *
 * @see calendar_db_add_changed_cb()
 */
typedef void (*calendar_db_changed_cb)(const char* view_uri, void* user_data);

/**
 * @brief Inserts a record into the calendar database.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/calendar.write
 *
 * @param[in]   record     The record handle
 * @param[out]  record_id  The record ID
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_FILE_NO_SPACE       File system is full
 * @retval  #CALENDAR_ERROR_NOT_PERMITTED       Operation not permitted
 * @retval  #CALENDAR_ERROR_PERMISSION_DENIED   Permission denied. This application does not have the privilege to call this method.
 * @retval  #CALENDAR_ERROR_DB_FAILED           Database operation failure
 * @retval  #CALENDAR_ERROR_IPC                 Unknown IPC error
 * @retval  #CALENDAR_ERROR_NO_DATA             Data does not exist
 *
 * @pre     calendar_connect() should be called to open a connection to the calendar service.
 *
 * @see calendar_connect()
 * @see calendar_db_update_record()
 * @see calendar_db_delete_record()
 * @see calendar_db_get_record()
 */
int calendar_db_insert_record( calendar_record_h record, int* record_id );

/**
 * @brief Gets a record from the calendar database.
 *
 * @details This function creates a new record handle from the calendar database by the given @a record_id. \n
 *          @a record will be created and filled with record information.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/calendar.read
 *
 * @remarks You must release @a record using calendar_record_destroy().
 *
 * @param[in]   view_uri    The view URI of a record
 * @param[in]   record_id   The record ID
 * @param[out]  record      The record handle associated with the record ID
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_FILE_NO_SPACE       File system is full
 * @retval  #CALENDAR_ERROR_NOT_PERMITTED       Operation not permitted
 * @retval  #CALENDAR_ERROR_PERMISSION_DENIED   Permission denied. This application does not have the privilege to call this method.
 * @retval  #CALENDAR_ERROR_DB_FAILED           Database operation failure
 * @retval  #CALENDAR_ERROR_DB_RECORD_NOT_FOUND Database not found
 * @retval  #CALENDAR_ERROR_IPC                 Unknown IPC error
 * @retval  #CALENDAR_ERROR_NO_DATA             Data does not exist
 *
 * @pre     calendar_connect() should be called to open a connection to the calendar service.
 *
 * @see calendar_connect()
 * @see calendar_record_destroy()
 */
int calendar_db_get_record( const char* view_uri, int record_id, calendar_record_h* record );

/**
 * @brief Updates a record in the calendar database.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/calendar.write
 *
 * @param[in]   record    The record handle
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_DB_FAILED           Database operation failure
 * @retval  #CALENDAR_ERROR_PERMISSION_DENIED   Permission denied. This application does not have the privilege to call this method.
 * @retval  #CALENDAR_ERROR_NOT_PERMITTED       Operation not permitted
 * @retval  #CALENDAR_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CALENDAR_ERROR_DB_RECORD_NOT_FOUND Database not found
 * @retval  #CALENDAR_ERROR_IPC                 Unknown IPC error
 * @retval  #CALENDAR_ERROR_NO_DATA             Data does not exist
 *
 * @pre     calendar_connect() should be called to open a connection to the calendar service.
 *
 * @see calendar_connect()
 * @see calendar_db_insert_record()
 * @see calendar_db_delete_record()
 * @see calendar_db_get_record()
 */
int calendar_db_update_record( calendar_record_h record );

/**
 * @brief Deletes a record from the calendar database with related child records.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/calendar.write
 *
 * @param[in]   view_uri    The view URI of a record
 * @param[in]   record_id   The record ID to be deleted
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_DB_FAILED           Database operation failure
 * @retval  #CALENDAR_ERROR_PERMISSION_DENIED   Permission denied. This application does not have the privilege to call this method.
 * @retval  #CALENDAR_ERROR_DB_RECORD_NOT_FOUND Database not found
 * @retval  #CALENDAR_ERROR_FILE_NO_SPACE       File system is full
 * @retval  #CALENDAR_ERROR_IPC                 Unknown IPC error
 * @retval  #CALENDAR_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CALENDAR_ERROR_NO_DATA             Data does not exist
 *
 * @pre     calendar_connect() should be called to open a connection to the calendar service.
 *
 * @see calendar_connect()
 * @see calendar_db_insert_record()
 */
int calendar_db_delete_record( const char* view_uri, int record_id );

/**
 * @brief Retrieves all records as a list.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/calendar.read
 *
 * @remarks You must release @a record_list using calendar_list_destroy().
 *
 * @param[in]   view_uri        The view URI to get records from
 * @param[in]   offset          The index from which results are received
 * @param[in]   limit           The maximum number of results(value 0 is used for all records)
 * @param[out]  record_list     The record list
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_DB_FAILED           Database operation failure
 * @retval  #CALENDAR_ERROR_PERMISSION_DENIED   Permission denied. This application does not have the privilege to call this method.
 * @retval  #CALENDAR_ERROR_NOT_PERMITTED       Operation not permitted
 * @retval  #CALENDAR_ERROR_IPC                 Unknown IPC error
 * @retval  #CALENDAR_ERROR_NO_DATA             Data does not exist
 *
 * @pre    calendar_connect() should be called to open a connection to the calendar service.
 *
 * @see calendar_connect()
 * @see calendar_list_destroy()
 */
int calendar_db_get_all_records( const char* view_uri, int offset, int limit, calendar_list_h* record_list );

/**
 * @brief Retrieves records using a query handle.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/calendar.read
 *
 * @remarks You must release @a record_list using calendar_list_destroy().
 *
 * @param[in]   query           The query handle used to filter results
 * @param[in]   offset          The index from which results are received
 * @param[in]   limit           The maximum number of results(value 0 is used for all records)
 * @param[out]  record_list     The record list
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE			    Successful
 * @retval  #CALENDAR_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_DB_FAILED           Database operation failure
 * @retval  #CALENDAR_ERROR_PERMISSION_DENIED   Permission denied. This application does not have the privilege to call this method.
 * @retval  #CALENDAR_ERROR_NOT_PERMITTED       Operation not permitted
 * @retval  #CALENDAR_ERROR_IPC                 Unknown IPC error
 * @retval  #CALENDAR_ERROR_NO_DATA             Data does not exist
 *
 * @pre    calendar_connect() should be called to open a connection to the calendar service.
 *
 * @see calendar_connect()
 * @see calendar_list_destroy()
 */
int calendar_db_get_records_with_query( calendar_query_h query, int offset, int limit, calendar_list_h* record_list );

/**
 * @brief Gets the record count of a specific view.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/calendar.read
 *
 * @param[in]   view_uri        The view URI to get records from
 * @param[out]  count           The number of records
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_DB_FAILED           Database operation failure
 * @retval  #CALENDAR_ERROR_PERMISSION_DENIED   Permission denied. This application does not have the privilege to call this method.
 * @retval  #CALENDAR_ERROR_NOT_PERMITTED       Operation not permitted
 * @retval  #CALENDAR_ERROR_IPC                 Unknown IPC error
 * @retval  #CALENDAR_ERROR_NO_DATA             Data does not exist
 * @retval  #CALENDAR_ERROR_OUT_OF_MEMORY       Out of memory
 *
 * @pre     This function requires an open connection to the calendar service using calendar_connect2().
 *
 * @see calendar_connect()
 */
int calendar_db_get_count( const char* view_uri, int *count );

/**
 * @brief Gets the record count with a query handle.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/calendar.read
 *
 * @param[in]   query    The query handle used for filtering the results
 * @param[out]  count    The number of records
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_DB_FAILED           Database operation failure
 * @retval  #CALENDAR_ERROR_PERMISSION_DENIED   Permission denied. This application does not have the privilege to call this method.
 * @retval  #CALENDAR_ERROR_NOT_PERMITTED       Operation not permitted
 * @retval  #CALENDAR_ERROR_IPC                 Unknown IPC error
 * @retval  #CALENDAR_ERROR_NO_DATA             Data does not exist
 * @retval  #CALENDAR_ERROR_OUT_OF_MEMORY       Out of memory
 *
 * @pre    This function requires an open connection to the calendar service using calendar_connect2().
 *
 * @see calendar_connect2()
 */
int calendar_db_get_count_with_query( calendar_query_h query, int *count );

/**
 * @brief Inserts multiple records into the calendar database as a batch operation.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/calendar.write
 *
 * @param[in]   record_list         The record list handle
 * @param[out]  record_id_array	    The array of record IDs
 * @param[out]  count			    The number of record IDs
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_DB_FAILED           Database operation failure
 * @retval  #CALENDAR_ERROR_PERMISSION_DENIED   Permission denied. This application does not have the privilege to call this method.
 * @retval  #CALENDAR_ERROR_NOT_PERMITTED       Operation not permitted
 * @retval  #CALENDAR_ERROR_FILE_NO_SPACE       File system is full
 * @retval  #CALENDAR_ERROR_IPC                 Unknown IPC error
 * @retval  #CALENDAR_ERROR_NO_DATA             Data does not exist
 * @retval  #CALENDAR_ERROR_OUT_OF_MEMORY       Out of memory
 *
 * @pre     calendar_connect() should be called to open a connection to the calendar service.
 *
 * @see calendar_connect()
 * @see calendar_db_update_records()
 * @see calendar_db_delete_records()
 */
int calendar_db_insert_records( calendar_list_h record_list, int** record_id_array, int* count);

/**
 * @brief Updates multiple records into the calendar database as a batch operation.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/calendar.write
 *
 * @param[in]   record_list       The record list handle
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_DB_FAILED           Database operation failure
 * @retval  #CALENDAR_ERROR_NO_DATA             Data does not exist
 * @retval  #CALENDAR_ERROR_NOT_PERMITTED       Operation not permitted
 * @retval  #CALENDAR_ERROR_PERMISSION_DENIED   Permission denied. This application does not have the privilege to call this method.
 * @retval  #CALENDAR_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CALENDAR_ERROR_DB_RECORD_NOT_FOUND Database not found
 * @retval  #CALENDAR_ERROR_IPC                 Unknown IPC error
 *
 * @pre     calendar_connect() should be called to open a connection to the calendar service.
 *
 * @see calendar_connect()
 * @see calendar_db_insert_records()
 * @see calendar_db_delete_records()
 */
int calendar_db_update_records( calendar_list_h record_list);

/**
 * @brief   Deletes multiple records with related child records from the calendar database as a batch operation.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/calendar.write
 *
 * @param[in]   view_uri            The view URI of the records to delete
 * @param[in]   record_id_array     The record IDs to delete
 * @param[in]   count               The number of records
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_DB_FAILED           Database operation failure
 * @retval  #CALENDAR_ERROR_PERMISSION_DENIED   Permission denied. This application does not have the privilege to call this method.
 * @retval  #CALENDAR_ERROR_NOT_PERMITTED       Operation not permitted
 * @retval  #CALENDAR_ERROR_DB_RECORD_NOT_FOUND Database not found
 * @retval  #CALENDAR_ERROR_FILE_NO_SPACE       File system is full
 * @retval  #CALENDAR_ERROR_IPC                 Unknown IPC error
 * @retval  #CALENDAR_ERROR_OUT_OF_MEMORY       Out of memory
 *
 * @pre     calendar_connect() should be called to open a connection to the calendar service.
 *
 * @see calendar_connect()
 * @see calendar_db_insert_records()
 * @see calendar_db_delete_records()
 */
int calendar_db_delete_records(const char* view_uri, int record_id_array[], int count);

/**
 * @brief	Gets the current calendar database version.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/calendar.read
 *
 * @param[out]  calendar_db_version    The calendar database version
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                 Successful
 * @retval	#CALENDAR_ERROR_INVALID_PARAMETER    Invalid parameter
 * @retval  #CALENDAR_ERROR_DB_FAILED            Database operation failure
 * @retval  #CALENDAR_ERROR_PERMISSION_DENIED    Permission denied. This application does not have the privilege to call this method.
 * @retval  ##CALENDAR_ERROR_DB_RECORD_NOT_FOUND Database not found
 * @retval  #CALENDAR_ERROR_IPC                  Unknown IPC error
 *
 * @pre     This function requires an open connection to the calendar service using calendar_connect().
 *
 * @see calendar_connect()
 * @see calendar_db_get_changes_by_version()
 */
int calendar_db_get_current_version(int* calendar_db_version);

/**
 * @brief Registers a callback function to be invoked when a record changes.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/calendar.read
 *
 * @remarks If successive change notification produced on the view_uri are identical,
 * then they are coalesced into a single notification if the older notification has not yet been called
 * because default main loop is doing something.
 * But, it means that a callback function is not called to reliably count of change.
 * This API supports only @ref CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE_calendar_book view, @ref CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE_calendar_event view,
 * @ref CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE_calendar_todo view.
 *
 * @param[in]   view_uri    The view URI of the record to subscribe for change notifications
 * @param[in]   callback    The callback function to register
 * @param[in]	user_data   The user data to be passed to the callback function
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval	#CALENDAR_ERROR_NONE                Successful
 * @retval	#CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_SYSTEM              Error from another modules
 * @retval  #CALENDAR_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CALENDAR_ERROR_PERMISSION_DENIED   Permission denied. This application does not have the privilege to call this method.
 *
 * @pre	    This function requires an open connection to the calendar service using calendar_connect().
 * @post    calendar_db_changed_cb() will be invoked when the designated view changes.
 *
 * @see calendar_connect()
 * @see calendar_db_changed_cb()
 * @see calendar_db_remove_changed_cb()
 */
int calendar_db_add_changed_cb(const char* view_uri, calendar_db_changed_cb callback, void* user_data );

/**
 * @brief Unregisters a callback function.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/calendar.read
 *
 * @param[in]   view_uri    The view URI of the record to subscribe for change notifications
 * @param[in]   callback    The callback function to register
 * @param[in]	user_data   The user data to be passed to the callback function
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval	#CALENDAR_ERROR_NONE                Successful
 * @retval	#CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_SYSTEM              Error from another modules
 * @retval  #CALENDAR_ERROR_PERMISSION_DENIED   Permission denied. This application does not have the privilege to call this method.
 *
 * @pre	    This function requires an open connection to the calendar service using calendar_connect().
 *
 * @see calendar_connect()
 * @see calendar_db_changed_cb()
 * @see calendar_db_add_changed_cb()
 */
int calendar_db_remove_changed_cb( const char* view_uri, calendar_db_changed_cb callback, void* user_data );

/**
 * @brief Retrieves records with the given calendar database version.
 *
  * @details This function finds all the changed records since the given @a calendar_db_version.
  *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/calendar.read
 *
 * @remarks You must release @a change_record_list using calendar_list_destroy().
 *
 * @param[in]   view_uri                    The view URI to get records from
 * @param[in]   calendar_book_id            The calendar book ID to filter
 * @param[in]   calendar_db_version         The calendar database version
 * @param[out]  record_list                 The record list
 * @param[out]  current_calendar_db_version The current calendar database version
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_DB_FAILED           Database operation failure
 * @retval  #CALENDAR_ERROR_PERMISSION_DENIED   Permission denied. This application does not have the privilege to call this method.
 * @retval  #CALENDAR_ERROR_DB_RECORD_NOT_FOUND Database not found
 * @retval  #CALENDAR_ERROR_IPC                 Unknown IPC error
 *
 * @pre    calendar_connect() should be called to open a connection to the calendar service.
 *
 * @see calendar_connect()
 * @see calendar_list_destroy()
 */
int calendar_db_get_changes_by_version(const char* view_uri, int calendar_book_id, int calendar_db_version, calendar_list_h* record_list, int *current_calendar_db_version );

/**
 * @brief Inserts a vcalendar stream into the calendar database.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/calendar.write
 *
 * @param[in]   vcalendar_stream     The vcalendar stream
 * @param[out]  record_id_array      The record IDs to delete
 * @param[out]  count                The number of record ID arrays
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_DB_FAILED           Database operation failure
 * @retval  #CALENDAR_ERROR_PERMISSION_DENIED   Permission denied. This application does not have the privilege to call this method.
 * @retval  #CALENDAR_ERROR_NO_DATA             Data does not exist
 * @retval  #CALENDAR_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CALENDAR_ERROR_FILE_NO_SPACE       File system is full
 * @retval  #CALENDAR_ERROR_NOT_PERMITTED       Operation not permitted
 * @retval  #CALENDAR_ERROR_IPC                 Unknown IPC error
 *
 * @pre     calendar_connect() should be called to open a connection to the calendar service.
 *
 * @see calendar_connect()
 * @see calendar_db_replace_vcalendars()
 */
int calendar_db_insert_vcalendars(const char* vcalendar_stream, int **record_id_array, int *count);

/**
 * @brief Replaces a vcalendar stream in the calendar database.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/calendar.write
 *
 * @param[in]   vcalendar_stream     The vcalendar stream
 * @param[in]   record_id_array      The record IDs to replace
 * @param[in]   count                The number of record ID arrays
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_DB_FAILED           Database operation failure
 * @retval  #CALENDAR_ERROR_PERMISSION_DENIED   Permission denied. This application does not have the privilege to call this method.
 * @retval  #CALENDAR_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CALENDAR_ERROR_NO_DATA             Data does not exist
 * @retval  #CALENDAR_ERROR_NOT_PERMITTED       Operation not permitted
 * @retval  #CALENDAR_ERROR_DB_RECORD_NOT_FOUND Database not found
 * @retval  #CALENDAR_ERROR_IPC                 Unknown IPC error
 *
 * @pre     This function requires an open connection to the calendar service by calendar_connect().
 *
 * @see calendar_connect()
 * @see calendar_db_replace_vcalendars()
 */
int calendar_db_replace_vcalendars(const char* vcalendar_stream, int *record_id_array, int count);

/**
 * @brief Replaces a record in the calendar database.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/calendar.write
 *
 * @param[in]   record        The record handle
 * @param[in]   record_id     The record ID
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_DB_FAILED           Database operation failure
 * @retval  #CALENDAR_ERROR_PERMISSION_DENIED   Permission denied. This application does not have the privilege to call this method.
 * @retval  #CALENDAR_ERROR_FILE_NO_SPACE       File system is full
 * @retval  #CALENDAR_ERROR_NOT_PERMITTED       Operation not permitted
 * @retval  #CALENDAR_ERROR_IPC                 Unknown IPC error
 * @retval  #CALENDAR_ERROR_OUT_OF_MEMORY       Out of memory
 *
 * @pre     calendar_connect() should be called to open a connection to the calendar service.
 *
 * @see calendar_connect()
 * @see calendar_db_update_record()
 * @see calendar_db_delete_record()
 * @see calendar_db_get_record()
 */
int calendar_db_replace_record(calendar_record_h record, int record_id);

/**
 * @brief Replaces multiple records in the calendar database as a batch operation.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/calendar.write
 *
 * @param[in]   record_list         The record list handle
 * @param[in]   record_id_array     The record IDs
 * @param[in]   count               The number of record ID arrays
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_DB_FAILED           Database operation failure
 * @retval  #CALENDAR_ERROR_PERMISSION_DENIED   Permission denied. This application does not have the privilege to call this method.
 * @retval  #CALENDAR_ERROR_FILE_NO_SPACE       File system is full
 * @retval  #CALENDAR_ERROR_NOT_PERMITTED       Operation not permitted
 * @retval  #CALENDAR_ERROR_IPC                 Unknown IPC error
 * @retval  #CALENDAR_ERROR_OUT_OF_MEMORY       Out of memory
 *
 * @pre     calendar_connect() should be called to open a connection to the calendar service.
 *
 * @see calendar_connect()
 * @see calendar_db_update_records()
 * @see calendar_db_delete_records()
 * @see calendar_db_replace_record()
 */
int calendar_db_replace_records(calendar_list_h record_list, int *record_id_array, int count);

/**
 * @brief Gets the last successful change version of the database on the current connection.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/calendar.read
 *
 * @param[out]  last_change_version   The calendar database version on the current connection
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_PERMISSION_DENIED   Permission denied. This application does not have the privilege to call this method.
 *
 * @pre     This function requires an open connection to the calendar service using calendar_connect().
 *
 * @see calendar_connect()
 * @see calendar_db_get_current_version()
 */
int calendar_db_get_last_change_version(int* last_change_version);

/**
 * @brief Retrieves changed exception records since the given calendar database version.
 *        Exceptions are the modified or deleted instances in a recurring event.
 *
 * @details This function finds all the changed records since the given @a calendar_db_version.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/calendar.read
 *
 * @remarks You must release @a change_record_list using calendar_list_destroy().
 *
 * @param[in]   view_uri               The view URI to get records from
 * @param[in]   original_event_id      The original event ID
 * @param[in]   calendar_db_version    The calendar database version starting from which to get records
 * @param[out]  list			       The record list
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE                Successful
 * @retval  #CALENDAR_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CALENDAR_ERROR_DB_FAILED           Database operation failure
 * @retval  #CALENDAR_ERROR_PERMISSION_DENIED   Permission denied. This application does not have the privilege to call this method.
 * @retval  #CALENDAR_ERROR_IPC                 Unknown IPC error
 *
 * @pre     calendar_connect() should be called to open a connection to the calendar service.
 *
 * @see calendar_connect()
 * @see calendar_list_destroy()
 */
int calendar_db_get_changes_exception_by_version(const char* view_uri, int original_event_id, int calendar_db_version, calendar_list_h* list);

/**
 * @brief Cleans the data after sync.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/calendar.write

 * @param[in]   calendar_book_id	The calendar book ID
 * @param[in]   calendar_db_version         The calendar database version
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 * @retval  #CALENDAR_ERROR_NONE			Successful
 * @retval  #CALENDAR_ERROR_OUT_OF_MEMORY		Out of memory
 * @retval  #CALENDAR_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval  #CALENDAR_ERROR_DB_FAILED			Database operation failure
 * @retval  #CALENDAR_ERROR_PERMISSION_DENIED   Permission denied
 * @retval  #CALENDAR_ERROR_FILE_NO_SPACE       File system is full
 * @retval  #CALENDAR_ERROR_IPC                 Unknown IPC error
 * @retval  #CALENDAR_ERROR_NO_DATA             Data does not exist
 *
 * @pre    calendar_connect() should be called to open a connection to the calendar service.
 *
 * @see calendar_connect()
 */
int calendar_db_clean_after_sync( int calendar_book_id, int calendar_db_version );

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_SOCIAL_CALENDAR_DB_H__ */

