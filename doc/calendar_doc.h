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

#ifndef __TIZEN_SOCIAL_CALENDAR_DOC_H__
#define __TIZEN_SOCIAL_CALENDAR_DOC_H__

/**
 * @ingroup CAPI_SOCIAL_FRAMEWORK
 * @defgroup CAPI_SOCIAL_CALENDAR_SVC_MODULE Calendar
 *
 * @brief The Calendar Service API provides functions for managing calendars(including events, to-dos).
 * This API allows you not only to store information about calendar but also to query calendar information.
 *
 * @section CAPI_SOCIAL_CALENDARS_SVC_MODULE_HEADER Required Header
 *  \#include <calendar.h>
 *
 *
 * @section CAPI_SOCIAL_CALENDAR_SVC_MODULE_OVERVIEW Overview
 * A calendar is a system of organizing days for social purposes and is composed of records like events and todos.
 * These records are made up of another sub records like alarms, attendees or extended ones.
 * Events could have recurrence rule, instances would be generated.
 * @image html calendar_model.png "Figure: Calendar model"
 *
 * The Calendar-Service provides an interface to manage the information of records.
 * <table>
 * <caption> Table: Calendar service properties. </caption>
 *	<tr>
 *		<td>
 *		* Managing information stored in database. <br>
 *		* Aggregating information from various accounts. <br>
 *		* Notifying changes of information. <br>
 *		* Searching information. <br>
 *		* Vcalendar  supports ver1.0(vcs) / 2.0(ics).
 *		</td>
 *	</tr>
 * </table>
 *
 * Calendar service module works in a way similar to client-service architecture.
 * In this architecture Tizen application is a client side and has to connect to service before using calendar service APIs.
 * Connection/disconnection MUST be done with use of calendar_connect() / calendar_disconnect().
 *
 * @code
 * calendar_connect();
 *
 * // jobs for records
 *
 * calendar_disconnect(); *
 * @endcode
 *
 * @section CAPI_SOCIAL_CALENDAR_SVC_MODULE_Entities Entities
 * Calendar service manages information related to following entities.
 * - Calendar books
 *    - Represents where event and todo should belong to.
 *    - Created by one of calendars sources below
 *       -# Local device, which has no account.
 *       -# Service providers such as Google or Yahoo, with account.
 *       -# Applications like ChatON, Joyn, Facebook, etc.
 *    - Have properties like name, account id, color.
 * - Events
 *    - Have properties like summary, start_time, description.
 *    - Single entry can be available on each calendar book.
 * - Todos
 *    - Similar with event entry.
 *
 * @subsection CAPI_SOCIAL_CALENDAR_SVC_MODULE_Entities_Relationship_between_entities Relationship between entities
 * @image html entities.png "Figure: Relationship between entities"
 *
 * @subsection CAPI_SOCIAL_CALENDAR_SVC_MODULE_Entities_Multiple_Address_books_from_Accounts Multiple Address books from Accounts
 * Each account can create multiple calendar books. Calendar book name does not need to be unique on the device because it is handled with id.
 * Local device address book has no account and its related account id is zero.
 *
 * @section CAPI_SOCIAL_CALENDAR_SVC_MODULE_Views Views
 * \ref CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE
 * To access and handle entities, views are provided. <br>
 * According to data-view declarations, generic access functions are used (calendar_db_insert_record(), calendar_record_get_int(), …). <br>
 * Data-View is almost same as database "VIEW" which limits access and guarantees the performance by offering various views for the proper purpose.
 *
 * _calendar_instance_utime, _calendar_instance_localtime views are not offered but combination with another view are provided.
 *
 * <table>
 * <caption> Table: Calendar views </caption>
 * <tr>
 *	<th> Editable view </th>
 *	<th> Read only view </th>
 * </tr>
 * <tr>
 *	<td>
 *		_calendar_book <br>
 *		_calendar_event <br>
 *		_calendar_todo <br>
 *		_calendar_timezone <br>
 *		_calendar_attendee <br>
 *		_calendar_alarm <br>
 *		_calendar_extended_property
 *	</td>
 *	<td>
 *		_calendar_updated_info <br>
 *		_calendar_event_calendar_book <br>
 *		_calendar_todo_calendar_book <br>
 *		_calendar_event_calendar_book_attendee <br>
 *		_calendar_instance_utime_calendar_book <br>
 *		_calendar_instance_localtime_calendar_book <br>
 *		_calendar_instance_utime_calendar_book_extended <br>
 *		_calendar_instance_localtime_calendar_book_extended

 *	</td>
 * </tr>
 * </table>
 *
 *
 * _calendar_updated_info is used when identifying record changes depending on version.
 *
 * The other read only views are combination of editable views for UI convenience. <br>
 * _calendar_event + _calendar_book = _calendar_event_calendar_book <br>
 * _calendar_instance_utime + _calendar_book = _calendar_instance_utime_calendar_book <br>
 * _calendar_event + _calendar_book + _calendar_attendee = _calendar_event_calendar_book_attendee <br>
 *
 *
 * @subsection CAPI_SOCIAL_CALENDAR_SVC_MODULE_Views_Properties Properties
 * Record types which have *_id as their properties, hold identifiers of other records - e.g. attendee and alarm views hold id of their corresponding events or to-dos in event_id or todo_id property respectively (as children of the corresponding events or to-dos record).
 * Properties of type 'record' are other records. For example, a event record has 'attendee' and 'alarm', which means that records of those types can be children of event type records.
 *
 * In calendar_view.h header file, view macros are found and below figure. show what macro means.
 * @image html view_property.png "Figure: Properties"
 *
 * There is an example how to create event with view.
 *
 * @code
 * // create an event with _calendar_event view.
 * calendar_record_h event = NULL;
 * calendar_record_create(_calendar_event._uri, &event);
 *
 * // set event summary to _calendar_event view.
 * calendar_record_set_str(event, _calendar_event.summary, "Meeting");
 * @endcode
 *
 * @subsection CAPI_SOCIAL_CALENDAR_SVC_MODULE_Views_Version Version
 *
 * Calendar service uses version system in bellow APIs.
 *
 * <table>
 * <caption> Table: Version related APIs </caption>
 * <tr>
 *  <td>
 * calendar_db_get_current_version(int* calendar_db_version) <br>
 * calendar_db_get_changes_by_version(const char *view_uri, int calendar_book_id, int calendar_db_version, calendar_list_h *record_list, int *current_calendar_db_version) <br>
 * calendar_db_get_last_change_version(int* last_change_version) <br>
 * calendar_db_get_changes_exception_by_version(const char *view_uri, int original_event_id, int calendar_db_version, calendar_list_h *list)
 *  </td>
 * </tr>
 * </table>
 *
 * Whenever modifications are in DB, version number is increased every time. <br>
 * If sync applications like google, facebook sync at version 13 and they try to sync again every 1 minute, they want to get changes from version 14 to current version. <br>
 * To get current version, calendar_db_get_current_version() is used and calendar_db_get_changes_by_version() is used to get the modified record list. calendar_db_get_changes_exception_by_version() is used to get modified instances in recurring event.
 * (see exception in Events and instances)
 *
 *
 * @section CAPI_SOCIAL_CALENDAR_SVC_MODULE_Records Records
 * An important concept in Calendar service APIs is a record.
 * It may be helpful to know that a record represents an actual record in the internal database, but in general, you can think of a record as a structure describing a single (but complex) view, like a calendar event or a time zone.
 *
 * A record has many properties, for example, a to-do record has the to-do's description, priority, progress, created, last modified and completed time, plus many others. <br>
 * A record can also contain an identifier field, which holds an identifier of another record. Setting this field's value establishes a relation between the records, for example, a calendar event contains the identifier of a calendar book to which it belongs.
 *
 * @subsection CAPI_SOCIAL_CALENDAR_SVC_MODULE_Records_URI URI
 *
 * A record's type is identified by a structure called the view.
 * For example, the _calendar_event view describes the properties of the calendar event record.
 * Every view has a special field - _uri - that uniquely identifies the view.
 * In many cases you will need to provide the _uri value to indicate what type of record you wish to create or operate on.
 * @code
 * // create an event and get handle
 * calendar_record_h event = NULL;
 * calendar_record_create(_calendar_event._uri, &event);
 * @endcode
 *
 * @subsection CAPI_SOCIAL_CALENDAR_SVC_MODULE_Records_Record_handle Record handle
 *
 * To use a record, you must obtain its handle.
 * There are many ways to obtains it, including creating a new record and referring to child records of a record.
 * @code
 * // create an event and get handle
 * calendar_record_h event = NULL;
 * calendar_record_create(_calendar_event._uri, &event);
 *
 * // get record handle with id
 * calendar_record_h event2 = NULL
 * calendar_db_get_record(_calendar_event._uri, event_id, &event2);
 * @endcode
 *
 * @subsection CAPI_SOCIAL_CALENDAR_SVC_MODULE_Records_Basic_types Basic types
 *
 * Records contain properties of basic types: integer, lli (long integer, long long int), double,
 * string and calendar_time_s.<br>
 * The calendar_time_s type holds either a long long int, or three integers (year, month, day).
 * There are setter and getter functions for each type:
 *
 * <table>
 * <caption> Table: Setter and getter functions </caption>
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
 *     <td> long long integer </td>
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
 *     <td> calendar_time_s </td>
 *     <td> calendar_record_set_caltime </td>
 *     <td> calendar_record_get_caltime </td>
 * </tr>
 * </table>
 *
 * A record's type is identified by a structure called the view. For example,
 * the _calendar_event view describes the properties of the calendar event record.
 * Every view has a special field - _uri - that uniquely identifies the view.
 * In many cases you will need to provide the _uri value to indicate what
 * type of record you wish to create or operate on.
 *
 * @subsection CAPI_SOCIAL_CALENDAR_SVC_MODULE_Records_Child Child
 *
 * Records of certain type also hold 'child list' properties.
 * If a record has property of this type, it can be a parent of other records, called child records.
 * For example, attendee records can hold an event's identifier in their event_id property.
 * The event is the parent record of the child attendee records.
 *
 * To use a record, you must obtain its handle. There are many ways to obtains it,
 * including creating a new record and referring to child records of a record.
 *
 * Sample code:<br>
 * the code below creates an event and inserts it into default event book
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
 * // add alarm as child
 * calendar_record_h alarm = NULL;
 * calendar_record_create(_calendar_alarm._uri, &alarm);
 * calendar_record_set_int(alarm, _calendar_alarm.tick_unit, CALENDAR_ALARM_TIME_UNIT_MINUTE);
 * calendar_record_set_int(alarm, _calendar_alarm.tick, 5);
 * calendar_record_add_child_record(event, _calendar_event.calendar_alarm, alarm);
 *
 * // insert calendar book into the database
 * int event_id = 0;
 * calendar_db_insert_record(event, &event_id);
 *
 * // destroy
 * calendar_record_destroy(event, true);
 * @endcode
 *
 * @section CAPI_SOCIAL_CALENDAR_SVC_MODULE_Calendar_books Calendar Books
 * A calendar book is a placeholder for other records in Calendar API.
 * Every event and to-do has to belong to a calendar book.
 * There are three built-in calendar books.
 * <table>
 * <caption> Table: Calendar books </caption>
 * <tr>
 *  <td> DEFAULT_EVENT_CALENDAR_BOOK_ID </td>
 *  <td> Event book </td>
 * </tr>
 * <tr>
 *  <td> DEFAULT_TODO_CALENDAR_BOOK_ID </td>
 *  <td> Todo book </td>
 * </tr>
 * <tr>
 *  <td> DEFAULT_BIRTHDAY_CALENDAR_BOOK_ID </td>
 *  <td> Birthday book </td>
 * </tr>
 * </table>
 *
 * There is an example how calendar book id is set.
 *
 * @code
 * calendar_record_h event = NULL;
 *
 * calendar_record_create(_calendar_event._uri, &event);
 *
 * // set default calendar book id
 * calendar_record_set_int(event, _calendar_event.calendar_id, DEFAULT_EVENT_CALENDAR_BOOK_ID);
 *
 * // set other fields
 *
 * int event_id = 0;
 * calendar_db_insert_record(event &event_id);
 *
 * // destroy
 * calendar_record_destroy(event, true);
 * @endcode
 *
 * To receive a list of existing calendar books, use the following:
 *
 * @code
 * calendar_list_h calendar_book_list = NULL;
 * calendar_db_get_all_records(_calendar_calendar_book._uri, 0, 0, &calendar_book_list);
 * @endcode
 *
 *
 * @section CAPI_SOCIAL_CALENDAR_SVC_MODULE_Events_and_instances Events and Instances
 * Two important concepts are event and instance. An event record describes
 * various properties of the event, like description, categories, priority
 * and many others. It also contains information on when the event takes place,
 * there can be more than one instance of the event. Each instance has its
 * corresponding instance record. <br>
 * If event is inserted with rrule, alarm and attendee, its data is saved to each database.
 * Generated instances based on rrule are also stored in instance database.
 * @image html view_db.png "Figure: Views and databases"
 *
 * For example, if an event has the following properties:
 * <table>
 * <caption> Table: Event and instance example </caption>
 * <tr>
 *	<th> event </th>
 *	<th> instances </th>
 * </tr>
 * <tr>
 *	<td>
 *		start date on 2012-10-09 (Tuesday) <br>
 *		frequency set to 'WEEKLY' <br>
 *		interval set to 1 <br>
 *		count set to 3
 * </td>
 *	<td>
 *		2012-10-09 Tuesday <br>
 *		2012-10-16 Tuesday <br>
 *		2012-10-23 Tuesday
 *	</td>
 * </tr>
 * </table>
 *
 * Interval is a multiplier of frequency, which means that if it is set to N,
 * instances occur every N weeks (or whatever was set in frequency attribute).
 *
 * The recurrence model in Calendar API is compliant with iCalendar specification
 * (<a href="http://www.ietf.org/rfc/rfc2445.txt">www.ietf.org/rfc/rfc2445.txt</a>).
 * The following event properties have the same functionality as their corresponding
 * values in iCalendar:
 *
 * <table>
 * <caption> Table: Recurrence rules. </caption>
 * <tr>
 *	<th> Recurrence rule property </th>
 *	<th> comment </th>
 * </tr>
 * <tr>
 *	<td> freq </td>
 *	<td> Yearly, monthly, weekly, daily </td>
 * </tr>
 * <tr>
 *	<td> count </td>
 *	<td> Until count. If count is 3, 3 instances are generated </td>
 * </tr>
 * <tr>
 *	<td> interval </td>
 *	<td> The interval is positive integer representing how often the recurrence rule repeats </td>
 * </tr>
 * <tr>
 *	<td> byday </td>
 *	<td> MO, TU, WE, TH, FR, SA, SU </td>
 * </tr>
 * <tr>
 *	<td> bymonthday </td>
 *	<td> Days of month </td>
 * </tr>
 * <tr>
 *	<td> byyearday </td>
 *	<td> Days of year </td>
 * </tr>
 * <tr>
 *	<td> byweekno </td>
 *	<td> Ordinals specifying weeks of the year </td>
 * </tr>
 * <tr>
 *	<td> bymonth </td>
 *	<td> Months of year </td>
 * </tr>
 * <tr>
 *	<td> bysetpos </td>
 *	<td> Values which corresponds to the nth occurrence within the set of events </td>
 * </tr>
 * <tr>
 *	<td> wkst </td>
 *	<td> The day on which the workweek starts </td>
 * </tr>
 * </table>
 *
 * @subsection CAPI_SOCIAL_CALENDAR_SVC_MODULE_Events_and_instances_Exceptions Exceptions
 *
 * If one of instances is modified in summary, date… or deleted, this is called exception.<br>
 * For example, if 2nd instance date is modified from 16th to 17th, 17th is the exception.
 * <table>
 * <caption> Table: Exception example </caption>
 * <tr>
 *	<th> event </th>
 *	<th> instances </th>
 *	<th> exceptions </th>
 * </tr>
 * <tr>
 *	<td>
 *		start date on 2012-10-09 (Tuesday) <br>
 *		frequency set to 'WEEKLY' <br>
 *		interval set to 1 <br>
 *		count set to 3
 * </td>
 *	<td>
 *		2012-10-09 Tuesday <br>
 *		2012-10-23 Tuesday
 *	</td>
 *  <td>
 *		2012-10-17 Tuesday <br>
 *  </td>
 * </tr>
 * </table>
 *
 * To get changes in exception, calendar_db_get_changes_exception_by_version() is called. These instances and exceptions are deleted together when original event is deleted.
 *
 *
 * @section CAPI_SOCIAL_CALENDAR_SVC_MODULE_Calendar_time_structure Calendar Time Structure
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
 * <caption> Table: Data types </caption>
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
 * @subsection CAPI_SOCIAL_CALENDAR_SVC_MODULE_Calendar_time_structure_UTC_time_usage UTC Time Usage
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
 *
 * @code
 * #define ms2sec(ms) (long long int)(ms / 1000.0)
 *
 * long long int _time_convert_itol(char *tzid, int y, int mon, int d, int h, int min, int s)
 * {
 *     int ret = 0;
 *     i18n_uchar utf16_timezone[CAL_STR_SHORT_LEN64] = {0};
 *     i18n_ustring_copy_ua_n(utf16_timezone, tzid, sizeof(utf16_timezone)/sizeof(i18n_uchar));
 *
 *     i18n_ucalendar_h ucal = NULL;
 *     char *loc_default = NULL;
 *     i18n_ulocale_get_default((const char **)&loc_default);
 *     ret = i18n_ucalendar_create(utf16_timezone, -1, loc_default, I18N_UCALENDAR_GREGORIAN, &ucal);
 *     if (I18N_ERROR_NONE != ret) {
 *		   dlog_print(DLOG_DEBUG, LOG_TAG, "i18n_ucalendar_create() Fail (%d)\n", ret);
 *         return -1;
 *     }
 *
 *     i18n_ucalendar_set_date_time(ucal, y, mon - 1, d, h, min, s);
 *
 *     i18n_udate date;
 *     ret = i18n_ucalendar_get_millisecond(ucal, &date);
 *     if (I18N_ERROR_NONE != ret) {
 *         dlog_print(DLOG_DEBUG, LOG_TAG, "i18n_ucalendar_create() Fail (%d)\n", ret);
 *         i18n_ucalendar_destroy(ucal);
 *         return -1;
 *     }
 *     i18n_ucalendar_destroy(ucal);
 *
 *     return ms2sec(date);
 * }
 * @endcode
 *
 * Sample code:<br>
 * @code
 * // fill calendar time structures (start and end time)
 * calendar_time_s st = {0};
 * calendar_time_s et = {0};
 *
 * st.type = CALENDAR_TIME_UTIME;
 * st.time.utime = _time_convert_itol("Asia/Seoul", 2012, 9, 15, 11, 0, 0);
 *
 * et.type = CALENDAR_TIME_UTIME;
 * et.time.utime = _time_convert_itol("Asia/Seoul", 2012, 9, 15, 12, 0, 0);
 *
 * // create an event record
 * // ...
 *
 * // set local time zone of start time
 * calendar_record_set_str(event, _calendar_event.start_tzid, "Asia/Seoul");
 *
 * // set start time
 * calendar_record_set_caltime(event, _calendar_event.start_time, st);
 *
 * // set local time zone of end time
 * calendar_record_set_str(event, _calendar_event.end_tzid, "Asia/Seoul");
 *
 * // set end time
 * calendar_record_set_caltime(event, _calendar_event.end_time, et);
 * @endcode
 *
 *
 * @subsection CAPI_SOCIAL_CALENDAR_SVC_MODULE_Calendar_time_structure_Date_usage Date Usage
 * Another usage of time structure is an all day event.
 * In case of such events, the structure's type field MUST be set to CALENDAR_TIME_LOCALTIME.
 * Only the date (no time) will be stored. Such structures can be used to set start and end time of an event. <br>
 * Both start and end time of the event MUST be set.
 * The end date value MUST be later in time than the value of the start date.
 *
 * @code
 * // range is 2days: 2014/02/17 00:00:00 ~ 2014/02/19 00:00:00
 *
 * calendar_time_s st = {0};
 * st.type = CALENDAR_TIME_LOCALTIME;
 * st.time.date.year = 2014;
 * st.time.date.month = 2;
 * st.time.date.mday = 17;
 *
 * calendar_time_s et = {0};
 * et.type = CALENDAR_TIME_LOCALTIME;
 * et.time.date.year = 2014;
 * et.time.date.month = 2;
 * et.time.date.mday = 19;
 *
 * // create an event record
 * // ...
 * @endcode
 *
 *
 * @section CAPI_SOCIAL_CALENDAR_SVC_MODULE_Creating_a_recurring_event Recurring Events
 * To create a recurring event in Calendar API, you MUST set frequency.
 * There is a sample code how to create recurring event. Firstly, set start and end time.
 *
 * @code
 * calendar_time_s st = {0};
 * st.type = CALENDAR_TIME_UTIME;
 * st.time.utime = _time_convert_itol("Asia/Seoul", 2012, 9, 15, 11, 0, 0);
 *
 * calendar_time_s et = {0};
 * et.type = CALENDAR_TIME_UTIME;
 * et.time.utime = _time_convert_itol("Asia/Seoul", 2012, 9, 15, 12, 0, 0);
 * @endcode
 *
 * Then, the remaining properties should be set.
 * Each frequency needs other proper fields except daily event.
 * If other values are not inserted, these values are calculated based on start time.
 *
 * <table>
 * <caption> Table: Frequency properties </caption>
 * <tr>
 *  <th> Freq </th>
 *  <th> Property </th>
 *  <th> Comment </th>
 * </tr>
 * <tr>
 *  <td> CALENDAR_RECURRENCE_YEARLY </td>
 *  <td> byyearday </td>
 *  <td> Every 100th day </td>
 * </tr>
 * <tr>
 *  <td> </td>
 *  <td> byweekno </td>
 *  <td> Every 34th week </td>
 * </tr>
 * <tr>
 *  <td> </td>
 *  <td> bymonthday </td>
 *  <td> Every 1st February (birthday)</td>
 * </tr>
 * <tr>
 *  <td> </td>
 *  <td> byday </td>
 *  <td> Every 1st Monday of May (holiday) </td>
 * </tr>
 * <tr>
 *  <td> CALENDAR_RECURRENCE_MONTHLY </td>
 *  <td> bymonthday </td>
 *  <td> Every 29th (payday) </td>
 * </tr>
 * <tr>
 *  <td> </td>
 *  <td> byday </td>
 *  <td> Every last Friday </td>
 * </tr>
 * <tr>
 *  <td> CALENDAR_RECURRENCE_WEEKLY </td>
 *  <td> byday </td>
 *  <td> Every week </td>
 * </tr>
 * <tr>
 *  <td> CALENDAR_RECURRENCE_DAILY </td>
 *  <td> - </td>
 *  <td> - </td>
 * </tr>
 * </table>
 *
 * If byday is not set in weekly event, default byday is followed start day of week.
 * By the same token, default interval is 1 and default range type is endless.
 *
 * <table>
 * <caption> Table: Range types </caption>
 * <tr>
 *  <th> Range type </th>
 *  <th> Comment </th>
 * </tr>
 * <tr>
 *  <td> CALENDAR_RANGE_NONE </td>
 *  <td> Endless(max date is 2036/12/31) </td>
 * </tr>
 * <tr>
 *  <td> CALENDAR_RANGE_UNTIL </td>
 *  <td> Should set until property </td>
 * </tr>
 * <tr>
 *  <td> CALENDAR_RANGE_COUNT </td>
 *  <td> Should set count property </td>
 * </tr>
 * </table>
 *
 * @code
 * calendar_record_h event;
 * calendar_record_create(_calendar_event._uri, &event);
 *
 * calendar_record_set_str(event, _calendar_event.start_tzid, "Asia/Seoul");
 * calendar_record_set_caltime(event, _calendar_event.start_time, st);
 * calendar_record_set_str(event, _calendar_event.end_tzid, "Asia/Seoul");
 * calendar_record_set_caltime(event, _calendar_event.end_time, et);
 *
 * calendar_record_set_int(event, _calendar_event.freq, CALENDAR_RECURRENCE_WEEKLY);
 * calendar_record_set_int(event, _calendar_event.interval, 1)
 * calendar_record_set_int(event, _calendar_event.count, 3);
 * @endcode
 *
 * The last step is inserting the event into the database.
 * Records representing instances of the event are created when the event record is inserted.
 *
 * @code
 * int event_id = 0;
 * calendar_db_insert_record(event, &event_id);
 *
 * calendar_record_destroy(event, true);
 * @endcode
 *
 *
 * @section CAPI_SOCIAL_CALENDAR_SVC_MODULE_Filters_and_queries Filters and Queries
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
 * <table>
 * <caption> Table: Filter and conditions </caption>
 * <tr>
 *	<th> Filter with conditions </th>
 *	<th> Result </th>
 * </tr>
 * <tr>
 *	<td>
 *		Contidion C1 <br>
 *		OR <br>
 *		Contidion C2 <br>
 *		AND <br>
 *		Condition C3
 *	</td>
 *	<td> (C1 OR C2) AND C3 </td>
 * </tr>
 * <tr>
 *	<td>
 *		Filter F1: <br>
 *			Condition C1 <br>
 *			OR <br>
 *			Condition C2 <br><br>
 *		Filter F2: <br>
 *			Condition C3 <br>
 *			OR <br>
 *			Condition C4 <br><br>
 *		Filter F3: <br>
 *			Condition C5 <br>
 *			AND <br>
 *			F1 <br>
 *			AND <br>
 *			F2
 *	</td>
 *	<td>
 *		(C5 AND F1) AND F2 <br>
 *		Which is: <br>
 *		(C5 AND (C1 OR C2)) AND (C3 OR C4)
 *	</td>
 * </tr>
 * </table>
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
 * calendar_filter_add_int(filter, _calendar_event.priority, CALENDAR_MATCH_EQUAL, CALENDAR_EVENT_PRIORITY_HIGH);
 *
 * // add OR operator
 * calendar_filter_add_operator(filter, CALENDAR_FILTER_OPERATOR_OR);
 *
 * // add 'description contains "meeting"' condition
 * calendar_filter_add_str(filter, _calendar_event.description, CALENDAR_MATCH_CONTAINS, "meeting");
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
 * calendar_filter_destroy(filter);
 * calendar_query_destroy(query);
 *
 * // use the list
 * // ...
 *
 * calendar_list_destroy(list, true);
 * @endcode
 *
 *
 * @section CAPI_SOCIAL_CALENDAR_SVC_MODULE_Projections Projections
 * Useful concept in Calendar service APIs is projection, related to searching with filters and queries.
 * Projection allows you to query the Data for just those specific properties of a record that you actually need, at lower latency
 * and cost than retrieving the entire properties
 *
 * Sample code:<br>
 * create filter which will get event id, summary and start time from the record with its summary has “test” (string filter).
 * Create a query and add the filter to it. Results are received in a list.
 *
 * @code
 * calendar_query_h query = NULL;
 * calendar_filter_h filter = NULL;
 *
 * // set query with filter
 * calendar_query_create(_calendar_event_calendar_book_attendee._uri, &query);
 * calendar_filter_create(_calendar_event_calendar_book_attendee._uri, &filter);
 * calendar_filter_add_str(filter, _calendar_event.summary, CALENDAR_MATCH_CONTAINS, "test");
 * calendar_query_set_filter(query, filter);
 *
 * // set projection
 * unsigned int projection[3];
 * projection[0]=_calendar_event_calendar_book_attendee.event_id;
 * projection[1]=_calendar_event_calendar_book_attendee.summary;
 * projection[2]=_calendar_event_calendar_book_attendee.start_time;
 *
 * // get list
 * calendar_query_set_projection(query, projection, 3);
 * calendar_db_get_records_with_query(query, 0, 0, &list);
 *
 * // destroy handle
 * calendar_filter_destroy(filter);
 * calendar_query_destroy(query);
 * calendar_list_destroy(list, true);
 * @endcode
 *
 *
 * @section CAPI_SOCIAL_CALENDAR_SVC_MODULE_Reminders Reminders
 * Alarm and reminder is the similar terminology but reminder is used as package name storages in below figure 5.
 * When alarm alerts, calendar-service sends notification to all packages which is registered in reminder DB table with calendar_reminder_add_receiver().
 *
 * This shows how to set alarm and how alarm works.
 *
 * @image html alarm_process.png "Figure: Alarm process"
 * After adding receiver, registered alarm would be alerted on reserved time by inserting alarm as child.
 *
 * <table>
 * <caption> Table: Alarm fields </caption>
 * <tr>
 *	<th> tick_unit </th>
 *	<th> related field </th>
 *	<th> comment </th>
 * </tr>
 * <tr>
 *	<td> CALENDAR_ALARM_TIME_UNIT_SPECIFIC </td>
 *	<td> time </td>
 *	<td> This represents the number of seconds elapsed since the Epoch, 1970-01-01 00:00:00 +0000 (UTC) </td>
 * </tr>
 * <tr>
 *	<td> CALENDAR_ALARM_TIME_UNIT_WEEK </td>
 *	<td> tick </td>
 *	<td> The number of weeks before start time </td>
 * </tr>
 * <tr>
 *	<td> CALENDAR_ALARM_TIME_UNIT_DAY </td>
 *	<td> tick </td>
 *	<td> The number of days before start time </td>
 * </tr>
 * <tr>
 *	<td> CALENDAR_ALARM_TIME_UNIT_HOUR </td>
 *	<td> tick </td>
 *	<td> The number of hours before start time </td>
 * </tr>
 * <tr>
 *	<td> CALENDAR_ALARM_TIME_UNIT_MINUTE </td>
 *	<td> tick </td>
 *	<td> The number of minutes before start time </td>
 * </tr>
 * </table>
 *
 * Below example shows the alarm which is set 1 minute before start time.
 *
 * @code
 // set alarm with normal unit
 * calendar_record_h alarm = NULL;
 * calendar_record_create(_calendar_alarm._uri, &alarm);
 * calendar_record_set_int(alarm, _calendar_alarm.tick_unit, CALENDAR_ALARM_TIME_UNIT_MINUTE);
 * calendar_record_set_int(alarm, _calendar_alarm.tick, 1); // before 1min (60secs)
 *
 * // add alarm as child
 * calendar_record_add_child_record(event, _calendar_event.calendar_alarm, alarm);
 * @endcode
 *
 * With CALENDAR_ALARM_TIME_UNIT_SPECIFIC, the alarm could be set regardless of the start time.
 *
 * @code
 // set alarm with specific unit
 * calendar_record_h alarm = NULL;
 * calendar_record_create(_calendar_alarm._uri, &alarm);
 * calendar_record_set_int(alarm, _calendar_alarm.tick_unit, CALENDAR_ALARM_TIME_UNIT_SPECIFIC);
 * // suppose start time is 1404036000(Sun, 29 Jun 2014 10:00:00 GMT) and alarm is set 60secs after from start time.
 * calendar_time_s ct;
 * ct.type = CALENDAR_TIME_UTIME;
 * ct.time.utime = 1404036000 + 60;
 * calendar_record_set_caltime(alarm, _calendar_alarm.alarm_time, ct);
 *
 * // add alarm as child
 * calendar_record_add_child_record(event, _calendar_event.calendar_alarm, alarm);
 * @endcode
 *
 * How to register package as reminder.
 *
 * In manifest.xml file, below code must be inserted.
 * @code
 * <app-control>
 *    <operation name="http://tizen.org/appcontrol/operation/view" />
 *    <mime name="application/x-tizen.calendar.reminder" />
 * </app-control>
 * @endcode
 *
 * When alarm alerts, calendar-service sends data with key, value pairs.<br>
 * With "ids" key, id array could be get.<br>
 * Each detail value could be get with "each id" key.<br>
 * The detail data is the combination of alarm values. id, time, tick, unit and type is connected with "&" charater.<br>
 * ex> id=64&time=1415106300&tick=0&unit=60&type=0
 *
 * @code
 * // "ids" string is the key, to get id array
 * char **ids = NULL;
 * int len = 0;
 * app_control_get_extra_data_array(b, "ids", &ids, &len);
 *
 * int i = 0;
 * for (i = 0; i < len; i++) {
 *	// "id" is the key to get detail value
 *	char *value = NULL;
 *	app_control_get_extra_data(b, ids[i], &value);
 *	if (NULL == value) {
 *		continue;
 *	}
 *	// parse detail data
 *
 *	// free
 *	free(ids[i]);
 *	ids[i] = NULL;
 * }
 *
 * free(ids);
 * @endcode
 *
 * @section CAPI_SOCIAL_CALENDAR_SVC_MODULE_Db_change_noties Database Change Notifications
 *
 * Applications add/remove callback function to detect/ignore the calendar DB changes
 * with calendar_db_add_changed_cb() / calendar_db_remove_changed_cb(). <br>
 * Clients wait calendar change notification on client side.
 * If calendar is changed by another module, server publishes inotify event. Inotify module broadcasts to subscribe modules.
 * Internal inotify handler is called at client side. User callback function is called with user data.
 *
 * @code
 * // add callback function
 * void __event_changed_cb(const char *view_uri, void *user_data)
 * {
 * }
 * // add changed noti callback
 * calendar_db_add_changed_cb(_calendar_event._uri, __event_changed_cb, NULL);
 * @endcode
 *
 *
 * @section CAPI_SOCIAL_CALENDAR_SVC_MODULE_Vcalendar Vcalendar
 *
 * To exchange of personal calendaring and scheduling information, vcalendar is used.
 * In order to avoid confusion with this referenced work, this is to be known as the vcalendar specification.
 * Vcalendar ver2.0 is known as icalendar.
 *
 *
 * <table>
 * <caption> Table: Vcalendar example ( http://www.ietf.org/rfc/rfc2445.txt ) </caption>
 * <tr>
 *  <td>
 *	BEGIN:VCALENDAR<br>
 *	VERSION:2.0<br>
 *	PRODID:-//hacksw/handcal//NONSGML v1.0//EN<br>
 *	BEGIN:VEVENT<br>
 *	DTSTART:19970714T170000Z<br>
 *	DTEND:19970715T035959Z<br>
 *	SUMMARY:Bastille Day Party<br>
 *	END:VEVENT<br>
 *	END:VCALENDAR<br>
 *  </td>
 * </tr>
 * </table>
 *
 * Calendar service provides APIs to compose vcalendar stream.
 * With stream, file could be made or data could be transmitted with json data.
 *
 * @code
 * calendar_list_h list = NULL;
 * // create or get list to make vcalendar stream
 *
 * char *stream = NULL;
 * calendar_vcalendar_make_from_records(list, &stream);
 *
 * // jobs for stream
 *
 * // free
 * free(stream);
 * @endcode
 *
 * Vcalendar could be parsed with calendar service APIs as well.
 *
 * @code
 * // read  stream from file
 *
 * calendar_list_h list = NULL;
 * calendar_vcalendar_parse_to_calendar(stream, &list);
 *
 * // jobs for list…
 * calendar_list_destroy(list, true);
 * @endcode
 *
 */

/**
 * @ingroup CAPI_SOCIAL_CALENDAR_SVC_MODULE
 * @defgroup CAPI_SOCIAL_CALENDAR_SVC_DATABASE_MODULE Database
 *
 * @brief The calendar database API provides the set of the definitions and interfaces that enable you to handle calendar database.
 *
 * @section CAPI_SOCIAL_CALENDAR_SVC_DATABASE_MODULE_HEADER Required Header
 *  \#include <calendar.h>
 *
 * <BR>
 */

/**
 * @ingroup CAPI_SOCIAL_CALENDAR_SVC_MODULE
 * @defgroup CAPI_SOCIAL_CALENDAR_SVC_FILTER_MODULE Filter
 *
 * @brief The calendar filter API provides the set of the definitions and interfaces that enable you to handle filter.
 *
 * @section CAPI_SOCIAL_CALENDAR_SVC_FILTER_MODULE_HEADER Required Header
 *  \#include <calendar.h>
 *
 * <BR>
 */

/**
 * @ingroup CAPI_SOCIAL_CALENDAR_SVC_MODULE
 * @defgroup CAPI_SOCIAL_CALENDAR_SVC_LIST_MODULE List
 *
 * @brief The calendar list API provides the set of the definitions and interfaces that enable you to handle list.
 *
 * @section CAPI_SOCIAL_CALENDAR_SVC_LIST_MODULE_HEADER Required Header
 *  \#include <calendar.h>
 *
 * <BR>
 */

/**
 * @ingroup CAPI_SOCIAL_CALENDAR_SVC_MODULE
 * @defgroup CAPI_SOCIAL_CALENDAR_SVC_QUERY_MODULE Query
 *
 * @brief The calendar query API provides the set of the definitions and interfaces that enable you to handle query.
 *
 * @section CAPI_SOCIAL_CALENDAR_SVC_QUERY_MODULE_HEADER Required Header
 *  \#include <calendar.h>
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
 *  \#include <calendar.h>
 *
 * <BR>
 */

/**
 * @ingroup CAPI_SOCIAL_CALENDAR_SVC_MODULE
 * @defgroup CAPI_SOCIAL_CALENDAR_SVC_REMINDER_MODULE Reminder
 *
 * @brief The calendar reminder API provides the interface to set/unset reminder callback.
 *
 * @section CAPI_SOCIAL_CALENDAR_SVC_REMINDER_MODULE_HEADER Required Header
 *  \#include <calendar.h>
 *
 * <BR>
 */

/**
 * @ingroup CAPI_SOCIAL_CALENDAR_SVC_MODULE
 * @defgroup CAPI_SOCIAL_CALENDAR_SVC_VCALENDAR_MODULE vCalendar
 *
 * @brief The calendar vcalendar API provides the set of the definitions and interfaces that enable you to get/set data from/to vCalendar.
 *
 * @section CAPI_SOCIAL_CALENDAR_SVC_VCALENDAR_MODULE_HEADER Required Header
 *  \#include <calendar.h>
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
 * A record can have basic properties of four types: integer, string, long integer. Each property
 * of basic type has functions to operate on it:
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
 *     <td> long long integer </td>
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
 *     <td> calendar_time_s </td>
 *     <td> calendar_record_set_caltime </td>
 *     <td> calendar_record_get_caltime </td>
 * </tr>
 * </table>
 *
 * For long long integer functions, "lli" stands for long long int, ususally used to hold UTC time.
 *
 * Below you can find tables with view properties.
 *
 * Properties of type 'record' are other records. For example, the @ref CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE_calendar_event
 * has a 'calendar_alarm' property of type 'record'. This means that records of type calendar_alarm (@ref CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE_calendar_alarm)
 * can be children of the event record. If a name record holds the identifier
 * of a event record in its 'event_id' property, it is the child record of the corresponding
 * event record.
 *
 * Records can have many children of a given type.
 *
 * Please refer to the main section of Calendar API for a more detailed explanation and examples.
 *
 * @section CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE_HEADER Required Header
 *  \#include <calendar.h>
 */

#endif /* __TIZEN_SOCIAL_CALENDAR_DOC_H__ */
