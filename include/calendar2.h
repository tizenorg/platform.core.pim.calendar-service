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
#ifndef __TIZEN_SOCIAL_CALENDAR_H__
#define __TIZEN_SOCIAL_CALENDAR_H__

#include <calendar_errors.h>
#include <calendar_view.h>
#include <calendar_db.h>
#include <calendar_filter.h>
#include <calendar_list.h>
#include <calendar_query.h>
#include <calendar_record.h>
#include <calendar_service.h>
#include <calendar_vcalendar.h>
#include <calendar_reminder.h>

#endif /* __TIZEN_SOCIAL_CALENDAR_H__ */

/**
 * @ingroup CAPI_SOCIAL_FRAMEWORK
 * @defgroup CAPI_SOCIAL_CALENDAR_SVC_MODULE Calendar(New)
 *
 * @brief The Calendar Service API provides functions for managing calendars(including events, to-dos).
 * This API allows you not only to store information about calendar but also to query calendar information.
 *
 * @section CAPI_SOCIAL_CALENDARS_SVC_MODULE_HEADER Required Header
 *  \#include <calendar2.h>
 *
 * @section CAPI_SOCIAL_CALENDAR_SVC_MODULE_OVERVIEW Overview
 * @section CAPI_SOCIAL_CALENDAR_SVC_MODULE_Records Records
 * An important concept in Calendar API is a record. It may be helpful to know that a record represents
 * an actual record in the internal database, but in general, you can think of a record as a structure
 * describing a single (but complex) entity, like a calendar event or a time zone. A record has
 * many properties, for example, a to-do record has the to-do's description, priority, progress,
 * created, last modified and completed time, plus many others.
 *
 * A record can also contain an identifier field, which holds an identifier of another record.
 * Setting this field's value establishes a relation between the records, for example,
 * a calendar event contains the identifier of a calendar book to which it belongs.
 *
 * Records contain properties of basic types: integer, lli (long integer, long long int), double,
 * string, bool and time. The time type holds either a long long int, or three integers
 * (year, month, day). There are setter and getter functions for each type:
 *
 * <table>
 * <tr>
 *     <th> Property </th>
 *     <th> Setter </th>
 *     <th> Getter </th>
 * </tr>
 * <tr>
 *     <td> integer </td>
 *     <td> calendar_record_set_int </td>
 *     <td> calendar_record_get_int </td>
 * </tr>
 * <tr>
 *     <td> long integer </td>
 *     <td> calendar_record_set_lli </td>
 *     <td> calendar_record_get_lli </td>
 * </tr>
 * <tr>
 *     <td> double </td>
 *     <td> calendar_record_set_double </td>
 *     <td> calendar_record_get_double </td>
 * </tr>
 * <tr>
 *     <td> string </td>
 *     <td> calendar_record_set_str </td>
 *     <td> calendar_record_get_str </td>
 * </tr>
 * <tr>
 *     <td> bool </td>
 *     <td> calendar_record_set_bool </td>
 *     <td> calendar_record_get_bool </td>
 * </tr>
 * <tr>
 *     <td> time </td>
 *     <td> calendar_record_set_time </td>
 *     <td> calendar_record_get_time </td>
 * </tr>
 * </table>
 *
 * A record's type is identified by a structure called the view. For example,
 * the _calendar_event view describes the properties of the calendar event record.
 * Every view has a special field - _uri - that uniquely identifies the view.
 * In many cases you wil need to provide the _uri value to indicate what
 * type of record you wish to create or operate on.
 *
 * To use a record, you must obtain its handle. There are many ways to obtains it,
 * including creating a new record and referring to child records of a record.
 *
 * Example: the code below creates an event and inserts it into default event book
 * (see below on calendar books).
 *
 * @code
 * // create an event
 * calendar_record_h event;
 * calendar_record_create(_calendar_event._uri, &event);
 *
 * // set event summary
 * calendar_record_set_str(event, _calendar_event.summary, "Meeting");
 *
 * // put the event into the default calendar book for events
 * calendar_record_set_int(event, _calendar_event.calendar_book_id, book_id);
 *
 * // insert calendar book into the database
 * calendar_db_insert_record(event);
 * @endcode
 *
 * Records of certain type also hold 'child list' properties. If a record has
 * property of this type, it can be a parent of other records, called child records.
 * For example, attendee records can hold an event's identifier in their event_id
 * property. The event is the parent record of the child attendee records.
 *
 *
 *
 * @section CAPI_SOCIAL_CALENDAR_SVC_MODULE_Calendar_books Calendar books
 * A calendar book is a placeholder for other records in Calendar API.
 * Every event and to-do has to belong to a calendar book.
 * There are two built-in calendar books: one for events, and one for to-dos,
 * identified by DEFAULT_EVENT_CALENDAR_BOOK_ID and DEFAULT_TODO_CALENDAR_BOOK_ID,
 * respectively.
 *
 * To receive a list of existing calendar books, use the following:
 *
 * @code
 * calendar_list_h calendar_book_list = NULL;
 * calendar_db_get_all_records(_calendar_calendar_book._uri, 0, 0, &calendar_book_list);
 * @endcode
 *
 * The parameters of calendar_db_get_all_records() are:
 * - type of records you wish to receive - _uri field of the view representing desired type,
 * - index from which results should be received (0 for all records),
 * - maximum number of results (0 means no limit),
 * - a list structure to hold the results.
 *
 * The list should be destroyed later with calendar_list_destroy().
 *
 *
 * @section CAPI_SOCIAL_CALENDAR_SVC_MODULE_Events_and_instances Events and instances
 * Two important concepts are event and instance. An event record describes
 * various properties of the event, like description, categories, priority
 * and many others. It also contains information on when the event takes place,
 * there can be more than one instance of the event. Each instance has its
 * corresponding instance record.
 *
 * For example, if an event has the following properties:
 *
 * - start date on 2012-10-09 (Tuesday),
 * - frequency set to 'WEEKLY',
 * - interval set to 1,
 * - count set to 3,
 *
 * it will generate three instances:
 *
 * - 2012-10-09
 * - 2012-10-16
 * - 2012-10-22
 *
 * Interval is a multiplier of frequency, which means that if it is set to N,
 * instances occur every N weeks (or whatever was set in frequency attribute).
 *
 * The recurrence model in Calendar API is compliant with iCalendar specification
 * (<a href="http://www.ietf.org/rfc/rfc2445.txt">www.ietf.org/rfc/rfc2445.txt</a>).
 * The following event properties have the same functionality as their corresponding
 * values in iCalendar:
 *
 * @code
 * freq
 * count
 * interval
 * bysecond
 * byminute
 * byhour
 * byday
 * bymonthday
 * byyearday
 * byweekno
 * bymonth
 * bysetpos
 * wkst
 * @endcode
 *
 *
 *
 * @section CAPI_SOCIAL_CALENDAR_SVC_MODULE_Calendar_time_structure Calendar time structure
 * The calendar time structure, calendar_caltime_s, is defined as follows:
 *
 * @code
 * typedef struct
 * {
 *     calendar_time_type_e type;
 *     union {
 *         long long int utime;
 *         struct {
 *             int year;
 *             int month;
 *             int mday;
 *         } date;
 *     } time;
 * } calendar_time_s;
 * @endcode
 *
 * The structure should be used when setting the calendar time type
 * (_CALENDAR_PROPERTY_CALTIME) properties of records.
 *
 * It can hold two types of data: UTC time (long long int) and date,
 * given as year, month and day of the month (three integers). These types
 * are identified by values of calendar_time_type_e, which are CALENDAR_TIME_UTIME
 * and CALENDAR_TIME_LOCALTIME, respectively. The data type determines the usage
 * of the structure.
 *
 * <table>
 * <tr>
 *     <th> Identifier </th>
 *     <th> Type </th>
 *     <th> Name </th>
 *     <th> Purpose </th>
 * </tr>
 * <tr>
 *     <td> CALENDAR_TIME_UTIME </td>
 *     <td> long long int </td>
 *     <td> utime </td>
 *     <td> UTC time, used to describe non-all-day events </td>
 * </tr>
 * <tr>
 *     <td> CALENDAR_TIME_LOCALTIME </td>
 *     <td> struct </td>
 *     <td> date </td>
 *     <td> date only (year, month and day of the month), used to describe all day events </td>
 * </tr>
 * </table>
 *
 *
 * 1. UTC time usage
 *
 * Structures with UTC time should be used for non-all-day events.
 * In such cases, the API user should convert local time to UTC time. The local
 * time zone identifier should be stored in the record, in the corresponding
 * property.
 *
 * For example, when setting starting time of an event, the local
 * time zone should be stored in start_tzid.
 *
 * When converting local time to UTC time, the function below can be useful.
 * The function converts the given date and time to the corresponding
 * UTC time, considering the given time zone (first argument).
 * The function uses UCalendar, see <a href="http://icu-project.org/apiref/icu4c/ucal_8h.html">
 * ucal_8h.html</a>
 *
 * @code
 * #define ms2sec(ms) (long long int)(ms / 1000.0)
 *
 * long long int _time_convert_itol(char *tzid, int y, int mon, int d, int h, int min, int s)
 * {
 *     long long int lli;
 *     UCalendar *ucal;
 *     UErrorCode status = U_ZERO_ERROR;
 *
 *     UChar *_tzid = NULL;
 *
 *     if (tzid == NULL)
 *     {
 *         tzid = "Etc/GMT";
 *     }
 *     _tzid = (UChar*)calloc(strlen(tzid) + 1, sizeof(UChar));
 *     if (_tzid == NULL)
 *     {
 *         return -1;
 *     }
 *     u_uastrcpy(_tzid, tzid);
 *
 *     ucal = ucal_open(_tzid, u_strlen(_tzid), "en_US", UCAL_TRADITIONAL, &status);
 *     if (U_FAILURE(status)) {
 *         printf("ucal_open failed (%s)\n", u_errorName(status));
 *         return -1;
 *     }
 *
 *     ucal_set(ucal, UCAL_YEAR, y);
 *     ucal_set(ucal, UCAL_MONTH, mon -1);
 *     ucal_set(ucal, UCAL_DATE, d);
 *     ucal_set(ucal, UCAL_HOUR, h);
 *     ucal_set(ucal, UCAL_MINUTE, min);
 *     ucal_set(ucal, UCAL_SECOND, s);
 *     lli = ms2sec(ucal_getMillis(ucal, &status));
 *     ucal_close(ucal);
 *     if (_tzid) free(_tzid);
 *
 *     return lli;
 * }
 * @endcode
 *
 * Sample code:
 *
 * @code
 * // fill calendar time structures (start and end time)
 * calendar_time_s st = {0};
 * calendar_time_s et = {0};
 *
 * st.type = CALENDAR_TIME_UTIME;
 * st.time.time = _time_convert_itol("Asia/Seoul", 2012, 9, 15, 11, 0, 0);
 *
 * et.type = CALENDAR_TIME_UTIME;
 * et.time.time = _time_convert_itol("Asia/Seoul", 2012, 9, 15, 12, 0, 0);
 *
 * // create an event record
 * // ...
 *
 * // set local time zone of start time
 * calendar_record_set_str(event, _calendar_event.start_tzid, "Asia/Seoul");
 *
 * // set start time
 * calendar_record_set_time(event, _calendar_event.start_time, st);
 *
 * // set local time zone of end time
 * calendar_record_set_str(event, _calendar_event.end_tzid, "Asia/Seoul");
 *
 * // set end time
 * calendar_record_set_time(event, _calendar_event.start_time, et);
 * @endcode
 *
 *
 * 2. Date usage
 *
 * Another usage of time structure is an all day event. In case of such events,
 * the structure's type field should be set to CALENDAR_TIME_LOCALTIME.
 * Only the date (no time) will be stored. Such structures can be used to set start
 * and end time of an event.
 *
 * Both start and end time of the event should be set. Start and end time
 * do not have to be equal. If they are not, the event's duration will be more
 * than one day. Note that in such cases there are no instances created,
 * as this is still a non-recurring event.
 *
 *
 *
 * @section CAPI_SOCIAL_CALENDAR_SVC_MODULE_Creating_a_recurring_event Creating a recurring event
 * To create a recurring event in Calendar API, first you need to create
 * and fill start and end time structures.
 *
 * @code
 * calendar_time_s st = {0};
 * calendar_time_s et = {0};
 *
 * st.type = CALENDAR_TIME_UTIME;
 * st.time.time = _time_convert_itol("Asia/Seoul", 2012, 9, 15, 11, 0, 0);
 *
 * et.type = CALENDAR_TIME_UTIME;
 * et.time.time = _time_convert_itol("Asia/Seoul", 2012, 9, 15, 12, 0, 0);
 * @endcode
 *
 * Then you can create and configure an event record.
 *
 * The time structures created before should be set using the corresponding setter function.
 * Then, the remaining properties should be set - frequency, interval and count.
 *
 * The last step is inserting the event into the database. Records representing
 * instances of the event are created when the event record is inserted.
 *
 * @code
 * calendar_record_h event;
 * calendar_record_create(_calendar_event._uri, &event);
 *
 * calendar_record_set_str(event, _calendar_event.start_tzid, "Asia/Seoul");
 * calendar_record_set_time(event, _calendar_event.start_time, st);
 * calendar_record_set_str(event, _calendar_event.end_tzid, "Asia/Seoul");
 * calendar_record_set_time(event, _calendar_event.start_time, et);
 *
 * calendar_record_set_int(event, _calendar_event.freq, CALENDAR_RECURRENCE_WEEKLY);
 * calendar_record_set_int(event, _calendar_event.interval, 1)
 * calendar_record_set_int(event, _calendar_event.count, 3);
 *
 * calendar_db_insert_record(event);
 * @endcode
 *
 *
 *
 * @section CAPI_SOCIAL_CALENDAR_SVC_MODULE_Filters_and_queries Filters and queries
 * Queries are used to retrieve data which satisfies given criteria, like an integer
 * property being greater than a given value, or a string property containing a given substring.
 * The criteria are defined by creating filters and adding conditions to them, joining them
 * with logical operators. Also, instead of a condition, another filter can be added,
 * which can be used to create more complex filters.
 *
 * Operator precedence in filters determined by the order in which the
 * conditions and filters are added.
 *
 * When a filter is ready, it can be set as a property of a query.
 * Other query properties allow configuring how the returned results
 * are grouped and sorted.
 *
 * Operator precedence in filters is determined by the order in which the
 * conditions and filters are added are added.
 * For example, if the following sequence is added:
 *
 * @code
 * Condition C1
 * OR
 * Condition C2
 * AND
 * Condition C3
 * @endcode
 *
 * the result is:
 *
 * @code
 * (C1 OR C2) AND C3
 * @endcode
 *
 * Another example, the sequence:
 *
 * @code
 * Filter F1:
 * Condition C1
 * OR
 * Condition C2
 *
 * Filter F2:
 * Condition C3
 * OR
 * Condition C4
 *
 * Filter F3:
 * Condition C5
 * AND
 * F1
 * AND
 * F2
 * @endcode
 *
 * results in:
 *
 * @code
 * Filter F3:
 * (C5 AND F1) AND F2
 * @endcode
 *
 * which is:
 *
 * @code
 * Filter F3:
 * (C5 AND (C1 OR C2)) AND (C3 OR C4)
 * @endcode
 *
 * The following code creates a filter, accepting events with high priority
 * or those that include the word "meeting" in their description.
 *
 * @code
 * calendar_filter_h filter = NULL;
 *
 * // create a filter returning event type records
 * calendar_filter_create(_calendar_event._uri, &filter);
 *
 * // add 'priority equals high' condition
 * calendar_filter_add_int(filter, _calendar_event.priority, CALENDAR_MATCH_EQUAL,
 *         CALENDAR_EVENT_PRIORITY_HIGH);
 *
 * // add OR operator
 * calendar_filter_add_operator(filter, CALENDAR_FILTER_OPERATOR_OR);
 *
 * // add 'description contains "meeting"' condition
 * calendar_filter_add_str(filter, _calendar_event.description, CALENDAR_MATCH_CONTAINS,
 *         "meeting");
 * @endcode
 *
 * The filter should be inserted into a query and the query should be executed:
 *
 * @code
 * calendar_query_h query = NULL;
 * calendar_list_h list = NULL;
 *
 * // create a query returning event type records
 * calendar_query_create(_calendar_event._uri, &query);
 *
 * // add the filter
 * calendar_query_set_filter(query, filter);
 *
 * // execute the query, results are returned in a list
 * calendar_db_get_records_with_query(query, 0, 0, &list);
 *
 * calendar_filter_destroy(&filter);
 * calendar_query_destroy(&query);
 *
 * // use the list
 * // ...
 *
 * calendar_list_destroy(&list);
 * @endcode
 *
 * @section CAPI_SOCIAL_CALENDAR_SVC_MODULE_View_properties View properties
 * In \ref CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE category, you can find tables with view properties. Record types which have *_id
 * as their properties, hold identifiers of other records - e.g. attendee and alarm
 * views hold id of their corresponding events or to-dos in event_id or todo_id property repectively
 * (as children of the corresponding events or to-dos record).
 *
 * Properties of type 'record' are other records. For example, a event record has 'attendee'
 * and 'alarm', which means that records of those types can be children
 * of event type records.
 */

/**
 * @ingroup CAPI_SOCIAL_CALENDAR_SVC_MODULE
 * @defgroup CAPI_SOCIAL_CALENDAR_SVC_DATABASE_MODULE Database
 *
 * @brief The calendar database API provides the set of the definitions and interfaces that enable you to handle calendar database.
 *
 * @section CAPI_SOCIAL_CALENDAR_SVC_DATABASE_MODULE_HEADER Required Header
 *  \#include <calendar2.h>
 *
 * <BR>
 */

/**
 * @ingroup CAPI_SOCIAL_CALENDAR_SVC_MODULE
 * @defgroup CAPI_SOCIAL_CALENDAR_SVC_RECORD_MODULE Record
 *
 * @brief The calendar record API provides the set of the definitions and interfaces that enable you to get/set data from/to calendar record handle.
 *
 * @section CAPI_SOCIAL_CALENDAR_SVC_RECORD_MODULE_HEADER Required Header
 *  \#include <calendar2.h>
 *
 * <BR>
 */

/**
 * @ingroup CAPI_SOCIAL_CALENDAR_SVC_MODULE
 * @defgroup CAPI_SOCIAL_CALENDAR_SVC_LIST_MODULE List
 *
 * @brief This page provides information about list.
 *
 * @brief The calendar database API provides the set of the definitions and interfaces that enable you to handle list.
 *  \#include <calendar2.h>
 *
 * <BR>
 */

/**
 * @ingroup CAPI_SOCIAL_CALENDAR_SVC_MODULE
 * @defgroup CAPI_SOCIAL_CALENDAR_SVC_FILTER_MODULE Filter
 *
 * @brief The calendar database API provides the set of the definitions and interfaces that enable you to handle filter.
 *
 * @section CAPI_SOCIAL_CALENDAR_SVC_FILTER_MODULE_HEADER Required Header
 *  \#include <calendar2.h>
 *
 * <BR>
 */

/**
 * @ingroup CAPI_SOCIAL_CALENDAR_SVC_MODULE
 * @defgroup CAPI_SOCIAL_CALENDAR_SVC_QUERY_MODULE Query
 *
 * @brief The calendar database API provides the set of the definitions and interfaces that enable you to handle query.
 *
 * @section CAPI_SOCIAL_CALENDAR_SVC_QUERY_MODULE_HEADER Required Header
 *  \#include <calendar2.h>
 *
 * <BR>
 */

/**
 * @ingroup CAPI_SOCIAL_CALENDAR_SVC_MODULE
 * @defgroup CAPI_SOCIAL_CALENDAR_SVC_VCALENDAR_MODULE vCalendar
 *
 * @brief The calendar record API provides the set of the definitions and interfaces that enable you to get/set data from/to vCalendar.
 *
 * @section CAPI_SOCIAL_CALENDAR_SVC_VCALENDAR_MODULE_HEADER Required Header
 *  \#include <calendar2.h>
 *
 * <BR>
 */

/**
 * @ingroup CAPI_SOCIAL_CALENDAR_SVC_MODULE
 * @defgroup CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE View/Property
 *
 * @brief This page provides information about views with properties.
 *
 * @section CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE_OVERVIEW Overview
 * A view is a structure which describes properties of a record.
 * A record can have basic properties of four types: integer, string, boolean, long integer. Each property
 * of basic type has functions to operate on it:
 *
 * <table>
 * <tr>
 *    <th>Property type</th>
 *    <th>Setter</th>
 *    <th>Getter</th>
 * </tr>
 * <tr>
 *    <td> string </td>
 *    <td> calendar_record_set_str </td>
 *    <td> calendar_record_get_str </td>
 * </tr>
 * <tr>
 *    <td> integer </td>
 *    <td> calendar_record_set_int </td>
 *    <td> calendar_record_get_int </td>
 * </tr>
 * <tr>
 *    <td> boolean </td>
 *    <td> calendar_record_set_bool </td>
 *    <td> calendar_record_get_bool </td>
 * </tr>
 * <tr>
 *    <td> long integer </td>
 *    <td> calendar_record_set_lli </td>
 *    <td> calendar_record_get_lli </td>
 * </tr>
 * </table>
 *
 * For long integer functions, "lli" stands for long long int, ususally used to hold UTC time.
 *
 * Below you can find tables with view properties.
 *
 * Properties of type 'record' are other records. For example, the \ref CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE_calendar_event
 * has a 'calendar_alarm' property of type 'record'. This means that records of type calendar_alarm (\ref CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE_calendar_alarm)
 * can be children of the event record. If a name record holds the identifier
 * of a event record in its 'event_id' property, it is the child record of the corresponding
 * event record.
 *
 * Records can have many children of a given type.
 *
 * Please refer to the main section of Calendar API for a more detailed explanation and examples.
 *
 * @section CAPI_SOCIAL_CCALENDAR_SVC_VIEW_MODULE_HEADER Required Header
 *  \#include <calendar2.h>
 *
 * @section CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE_calendar_book _calendar_book view
 * <table>
 *     <tr>
 *     <th> Type </th>
 *     <th> Property ID </th>
 *     <th> Read, Write </th>
 *     </tr>
 *     <tr><td> string </td><td> _uri </td><td> read only </td></tr>
 *     <tr><td> integer </td><td> id </td><td> read only </td></tr>
 *     <tr><td> string </td><td> uid </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> name </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> description </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> color </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> location </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> visibility </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> sync_event </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> is_deleted </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> account_id </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> store_type </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> sync_data1 </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> sync_data2 </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> sync_data3 </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> sync_data4 </td><td> read, write </td></tr>
 * </table>
 *
 * @section CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE_calendar_event _calendar_event view
 * <table>
 *     <tr>
 *     <th> Type </th>
 *     <th> Property ID </th>
 *     <th> Read, Write </th>
 *     </tr>
 *     <tr><td> string </td><td> _uri </td><td> read only </td></tr>
 *     <tr><td> integer </td><td> id </td><td> read only </td></tr>
 *     <tr><td> integer </td><td> calendar_book_id </td><td> read, write once </td></tr>
 *     <tr><td> string </td><td> summary </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> description </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> location </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> categories </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> exdate </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> event_status </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> priority </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> timezone </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> person_id </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> busy_status </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> sensitivity </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> uid </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> organizer_name </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> organizer_email </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> meeting_status </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> original_event_id </td><td> read, write </td></tr>
 *     <tr><td> double </td><td> latitude </td><td> read, write </td></tr>
 *     <tr><td> double </td><td> longitude </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> email_id </td><td> read, write </td></tr>
 *     <tr><td> long long int </td><td> created_time </td><td> read, write </td></tr>
 *     <tr><td> long long int </td><td> last_modified_time </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> is_deleted </td><td> read only </td></tr>
 *     <tr><td> integer </td><td> freq </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> range_type </td><td> read, write </td></tr>
 *     <tr><td> calendar time </td><td> until_time </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> count </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> interval </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> bysecond </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> byminute </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> byhour </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> byday </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> bymonthday </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> byyearday </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> byweekno </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> bymonth </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> bysetpos </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> wkst </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> recurrence_id </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> rdate </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> has_attendee </td><td> read only </td></tr>
 *     <tr><td> integer </td><td> has_alarm </td><td> read only </td></tr>
 *     <tr><td> integer </td><td> calendar_system_type </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> sync_data1 </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> sync_data2 </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> sync_data3 </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> sync_data4 </td><td> read, write </td></tr>
 *     <tr><td> calendar time </td><td> start_time </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> start_tzid </td><td> read, write </td></tr>
 *     <tr><td> calendar time </td><td> end_time </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> end_tzid </td><td> read, write </td></tr>
 *     <tr><td> child list </td><td> calendar_alarm </td><td> read, write </td></tr>
 *     <tr><td> child list </td><td> calendar_attendee </td><td> read, write </td></tr>
 * </table>
 *
 * @section CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE_calendar_todo _calendar_todo view
 * <table>
 *     <tr>
 *     <th> Type </th>
 *     <th> Property ID </th>
 *     <th> Read, Write </th>
 *     </tr>
 *     <tr><td> string </td><td> _uri </td><td> read only </td></tr>
 *     <tr><td> integer </td><td> id </td><td> read only </td></tr>
 *     <tr><td> integer </td><td> calendar_book_id </td><td> read, write once </td></tr>
 *     <tr><td> string </td><td> summary </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> description </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> location </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> categories </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> todo_status </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> priority </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> sensitivity </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> uid </td><td> read, write </td></tr>
 *     <tr><td> double </td><td> latitude </td><td> read, write </td></tr>
 *     <tr><td> double </td><td> longitude </td><td> read, write </td></tr>
 *     <tr><td> long long int </td><td> created_time </td><td> read, write </td></tr>
 *     <tr><td> long long int </td><td> last_modified_time </td><td> read, write </td></tr>
 *     <tr><td> long long int </td><td> completed_time </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> progress </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> is_deleted </td><td> read only </td></tr>
 *     <tr><td> integer </td><td> freq </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> range_type </td><td> read, write </td></tr>
 *     <tr><td> calendar time </td><td> until_time </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> count </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> interval </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> bysecond </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> byminute </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> byhour </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> byday </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> bymonthday </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> byyearday </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> byweekno </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> bymonth </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> bysetpos </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> wkst </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> has_alarm </td><td> read only </td></tr>
 *     <tr><td> string </td><td> sync_data1 </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> sync_data2 </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> sync_data3 </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> sync_data4 </td><td> read, write </td></tr>
 *     <tr><td> calendar time </td><td> start_time </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> start_tzid </td><td> read, write </td></tr>
 *     <tr><td> calendar time </td><td> due_time </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> due_tzid </td><td> read, write </td></tr>
 *     <tr><td> child list </td><td> calendar_alarm </td><td> read, write </td></tr>
 * </table>
 *
 * @section CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE_calendar_timezone _calendar_timezone view
 * <table>
 *     <tr>
 *     <th> Type </th>
 *     <th> Property ID </th>
 *     <th> Read, Write </th>
 *     </tr>
 *     <tr><td> string </td><td> _uri </td><td> read only </td></tr>
 *     <tr><td> integer </td><td> id </td><td> read only </td></tr>
 *     <tr><td> integer </td><td> calendar_book_id </td><td> read only </td></tr>
 *     <tr><td> integer </td><td> tz_offset_from_gmt </td><td> read only </td></tr>
 *     <tr><td> string </td><td> standard_name </td><td> read only </td></tr>
 *     <tr><td> integer </td><td> standard_start_month </td><td> read only </td></tr>
 *     <tr><td> integer </td><td> standard_start_position_of_week </td><td> read only </td></tr>
 *     <tr><td> integer </td><td> standard_start_day </td><td> read only </td></tr>
 *     <tr><td> integer </td><td> standard_start_hour </td><td> read only </td></tr>
 *     <tr><td> integer </td><td> standard_bias </td><td> read only </td></tr>
 *     <tr><td> string </td><td> day_light_name </td><td> read only </td></tr>
 *     <tr><td> integer </td><td> day_light_start_month </td><td> read only </td></tr>
 *     <tr><td> integer </td><td> day_light_start_position_of_week </td><td> read only </td></tr>
 *     <tr><td> integer </td><td> day_light_start_day </td><td> read only </td></tr>
 *     <tr><td> integer </td><td> day_light_start_hour </td><td> read only </td></tr>
 *     <tr><td> integer </td><td> day_light_bias </td><td> read only </td></tr>
 * </table>
 *
 * @section CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE_calendar_attendee _calendar_attendee view
 * <table>
 *     <tr>
 *     <th> Type </th>
 *     <th> Property ID </th>
 *     <th> Read, Write </th>
 *     </tr>
 *     <tr><td> string </td><td> _uri </td><td> read only </td></tr>
 *     <tr><td> integer </td><td> event_id </td><td> read only </td></tr>
 *     <tr><td> string </td><td> number </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> type </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> person_id </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> uid </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> group </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> email </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> role </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> status </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> rsvp </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> delegate_uri </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> delegator_uri </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> name </td><td> read, write </td></tr>
 * </table>
 *
 * @section CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE_calendar_alarm _calendar_alarm view
 * <table>
 *     <tr>
 *     <th> Type </th>
 *     <th> Property ID </th>
 *     <th> Read, Write </th>
 *     </tr>
 *     <tr><td> string </td><td> _uri </td><td> read only </td></tr>
 *     <tr><td> integer </td><td> event_id </td><td> read only </td></tr>
 *     <tr><td> integer </td><td> todo_id </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> type </td><td> read, write </td></tr>
 *     <tr><td> long long int </td><td> time </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> tick </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> tick_unit </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> tone </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> description </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> alarm_id </td><td> read, write </td></tr>
 * </table>
 *
 * @section CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE_calendar_updated_info _calendar_updated_info view
 * <table>
 *     <tr>
 *     <th> Type </th>
 *     <th> Property ID </th>
 *     <th> Read, Write </th>
 *     </tr>
 *     <tr><td> string </td><td> _uri </td><td> read only </td></tr>
 *     <tr><td> integer </td><td> id </td><td> read only </td></tr>
 *     <tr><td> integer </td><td> calendar_book_id </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> modified_status </td><td> read, write </td></tr>
 * </table>
 *
 * @section CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE_calendar_event_calendar_book _calendar_event_calendar_book view
 * <table>
 *     <tr>
 *     <th> Type </th>
 *     <th> Property ID </th>
 *     <th> Read, Write </th>
 *     </tr>
 *     <tr><td> string </td><td> _uri </td><td> read only </td></tr>
 *     <tr><td> integer </td><td> event_id </td><td> read only </td></tr>
 *     <tr><td> integer </td><td> calendar_book_id </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> summary </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> description </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> location </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> categories </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> exdate </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> event_status </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> priority </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> timezone </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> person_id </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> busy_status </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> sensitivity </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> uid </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> organizer_name </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> organizer_email </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> meeting_status </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> original_event_id </td><td> read, write </td></tr>
 *     <tr><td> double </td><td> latitude </td><td> read, write </td></tr>
 *     <tr><td> double </td><td> longitude </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> email_id </td><td> read, write </td></tr>
 *     <tr><td> long long int </td><td> created_time </td><td> read, write </td></tr>
 *     <tr><td> long long int </td><td> last_modified_time </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> is_deleted </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> freq </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> range_type </td><td> read, write </td></tr>
 *     <tr><td> calendar time </td><td> until_time </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> count </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> interval </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> bysecond </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> byminute </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> byhour </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> byday </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> bymonthday </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> byyearday </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> byweekno </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> bymonth </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> bysetpos </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> wkst </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> recurrence_id </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> rdate </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> has_attendee </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> has_alarm </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> calendar_system_type </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> sync_data1 </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> sync_data2 </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> sync_data3 </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> sync_data4 </td><td> read, write </td></tr>
 *     <tr><td> calendar time </td><td> start_time </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> start_tzid </td><td> read, write </td></tr>
 *     <tr><td> calendar time </td><td> end_time </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> end_tzid </td><td> read, write </td></tr>
 *     <tr><td> filter integer </td><td> calendar_book_visibility </td><td> read, write </td></tr>
 *     <tr><td> filter integer </td><td> calendar_book_account_id </td><td> read, write </td></tr>
 * </table>
 *
 * @section CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE_calendar_todo_calendar_book _calendar_todo_calendar_book view
 * <table>
 *     <tr>
 *     <th> Type </th>
 *     <th> Property ID </th>
 *     <th> Read, Write </th>
 *     </tr>
 *     <tr><td> string </td><td> _uri </td><td> read only </td></tr>
 *     <tr><td> integer </td><td> todo_id </td><td> read only </td></tr>
 *     <tr><td> integer </td><td> calendar_book_id </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> summary </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> description </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> location </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> categories </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> todo_status </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> priority </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> sensitivity </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> uid </td><td> read, write </td></tr>
 *     <tr><td> double </td><td> latitude </td><td> read, write </td></tr>
 *     <tr><td> double </td><td> longitude </td><td> read, write </td></tr>
 *     <tr><td> long long int </td><td> created_time </td><td> read, write </td></tr>
 *     <tr><td> long long int </td><td> last_modified_time </td><td> read, write </td></tr>
 *     <tr><td> long long int </td><td> completed_time </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> progress </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> is_deleted </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> freq </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> range_type </td><td> read, write </td></tr>
 *     <tr><td> calendar time </td><td> until_time </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> count </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> interval </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> bysecond </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> byminute </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> byhour </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> byday </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> bymonthday </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> byyearday </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> byweekno </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> bymonth </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> bysetpos </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> wkst </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> has_alarm </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> sync_data1 </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> sync_data2 </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> sync_data3 </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> sync_data4 </td><td> read, write </td></tr>
 *     <tr><td> calendar time </td><td> start_time </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> start_tzid </td><td> read, write </td></tr>
 *     <tr><td> calendar time </td><td> due_time </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> due_tzid </td><td> read, write </td></tr>
 *     <tr><td> filter integer </td><td> calendar_book_visibility </td><td> read, write </td></tr>
 *     <tr><td> filter integer </td><td> calendar_book_account_id </td><td> read, write </td></tr>
 * </table>
 *
 * @section CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE_calendar_event_calendar_book_attendee _calendar_event_calendar_book_attendee view
 * <table>
 *     <tr>
 *     <th> Type </th>
 *     <th> Property ID </th>
 *     <th> Read, Write </th>
 *     </tr>
 *     <tr><td> string </td><td> _uri </td><td> read only </td></tr>
 *     <tr><td> integer </td><td> event_id </td><td> read only </td></tr>
 *     <tr><td> integer </td><td> calendar_book_id </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> summary </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> description </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> location </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> categories </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> exdate </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> event_status </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> priority </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> timezone </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> person_id </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> busy_status </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> sensitivity </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> uid </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> organizer_name </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> organizer_email </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> meeting_status </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> original_event_id </td><td> read, write </td></tr>
 *     <tr><td> double </td><td> latitude </td><td> read, write </td></tr>
 *     <tr><td> double </td><td> longitude </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> email_id </td><td> read, write </td></tr>
 *     <tr><td> long long int </td><td> created_time </td><td> read, write </td></tr>
 *     <tr><td> long long int </td><td> last_modified_time </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> is_deleted </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> freq </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> range_type </td><td> read, write </td></tr>
 *     <tr><td> calendar time </td><td> until_time </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> count </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> interval </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> bysecond </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> byminute </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> byhour </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> byday </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> bymonthday </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> byyearday </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> byweekno </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> bymonth </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> bysetpos </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> wkst </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> recurrence_id </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> rdate </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> has_attendee </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> has_alarm </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> calendar_system_type </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> sync_data1 </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> sync_data2 </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> sync_data3 </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> sync_data4 </td><td> read, write </td></tr>
 *     <tr><td> calendar time </td><td> start_time </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> start_tzid </td><td> read, write </td></tr>
 *     <tr><td> calendar time </td><td> end_time </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> end_tzid </td><td> read, write </td></tr>
 *     <tr><td> filter integer </td><td> calendar_book_visibility </td><td> read, write </td></tr>
 *     <tr><td> filter integer </td><td> calendar_book_account_id </td><td> read, write </td></tr>
 *     <tr><td> filter string </td><td> attendee_email </td><td> read, write </td></tr>
 *     <tr><td> filter string </td><td> attendee_name </td><td> read, write </td></tr>
 * </table>
 *
 * @section CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE_calendar_instance_normal_calendar_book _calendar_instance_normal_calendar_book view
 * <table>
 *     <tr>
 *     <th> Type </th>
 *     <th> Property ID </th>
 *     <th> Read, Write </th>
 *     </tr>
 *     <tr><td> string </td><td> _uri </td><td> read only </td></tr>
 *     <tr><td> integer </td><td> event_id </td><td> read only </td></tr>
 *     <tr><td> calendar time </td><td> start_time </td><td> read, write </td></tr>
 *     <tr><td> calendar time </td><td> end_time </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> summary </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> location </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> calendar_book_id </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> description </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> busy_status </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> event_status </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> priority </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> sensitivity </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> has_rrule </td><td> read, write </td></tr>
 *     <tr><td> double </td><td> latitude </td><td> read, write </td></tr>
 *     <tr><td> double </td><td> longitude </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> has_alarm </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> original_event_id </td><td> read, write </td></tr>
 *     <tr><td> filter integer </td><td> calendar_book_visibility </td><td> read, write </td></tr>
 *     <tr><td> filter integer </td><td> calendar_book_account_id </td><td> read, write </td></tr>
 * </table>
 *
 * @section CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE_calendar_instance_allday_calendar_book _calendar_instance_allday_calendar_book view
 * <table>
 *     <tr>
 *     <th> Type </th>
 *     <th> Property ID </th>
 *     <th> Read, Write </th>
 *     </tr>
 *     <tr><td> string </td><td> _uri </td><td> read only </td></tr>
 *     <tr><td> integer </td><td> event_id </td><td> read only </td></tr>
 *     <tr><td> calendar time </td><td> start_time </td><td> read, write </td></tr>
 *     <tr><td> calendar time </td><td> end_time </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> summary </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> location </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> calendar_book_id </td><td> read, write </td></tr>
 *     <tr><td> string </td><td> description </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> busy_status </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> event_status </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> priority </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> sensitivity </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> has_rrule </td><td> read, write </td></tr>
 *     <tr><td> double </td><td> latitude </td><td> read, write </td></tr>
 *     <tr><td> double </td><td> longitude </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> has_alarm </td><td> read, write </td></tr>
 *     <tr><td> integer </td><td> original_event_id </td><td> read, write </td></tr>
 *     <tr><td> filter integer </td><td> calendar_book_visibility </td><td> read, write </td></tr>
 *     <tr><td> filter integer </td><td> calendar_book_account_id </td><td> read, write </td></tr>
 * </table>
 *
 */


