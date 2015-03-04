/*
 * Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __TIZEN_SOCIAL_CALENDAR_VIEW_H__
#define __TIZEN_SOCIAL_CALENDAR_VIEW_H__

#include <calendar_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @ingroup CAPI_SOCIAL_CALENDAR_SVC_MODULE
 * @addtogroup CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE_calendar_book _calendar_book view
 * <table>
 *     <tr>
 *     <th> Type </th>
 *     <th> Property ID </th>
 *     <th> Read, Write </th>
 *     <th> Description </th>
 *     </tr>
 *     <tr><td> string </td><td> _uri </td><td> read only </td><td> Identifier of this calendar book view </td></tr>
 *     <tr><td> integer </td><td> id </td><td> read only </td><td> DB record ID of the calendar book </td></tr>
 *     <tr><td> string </td><td> uid </td><td> read, write </td><td> Unique identifier </td></tr>
 *     <tr><td> string </td><td> name </td><td> read, write </td><td> Calendar book name </td></tr>
 *     <tr><td> string </td><td> description </td><td> read, write </td><td> Calendar book description </td></tr>
 *     <tr><td> string </td><td> color </td><td> read, write </td><td> Calendar book color for UX </td></tr>
 *     <tr><td> string </td><td> location </td><td> read, write </td><td> Location of the event </td></tr>
 *     <tr><td> integer </td><td> visibility </td><td> read, write </td><td> Visibility of the calendar book for UX</td></tr>
 *     <tr><td> integer </td><td> sync_event </td><td> read, write </td><td> </td>Currently NOT Used</tr>
 *     <tr><td> integer </td><td> account_id </td><td> read, write once </td><td> Account for this calendar </td></tr>
 *     <tr><td> integer </td><td> store_type </td><td> read, write </td><td> Type of calendar contents(refer to the @ref calendar_book_type_e) </td></tr>
 *     <tr><td> string </td><td> sync_data1 </td><td> read, write </td><td> Generic data for use by syncing </td></tr>
 *     <tr><td> string </td><td> sync_data2 </td><td> read, write </td><td> Generic data for use by syncing </td></tr>
 *     <tr><td> string </td><td> sync_data3 </td><td> read, write </td><td> Generic data for use by syncing </td></tr>
 *     <tr><td> string </td><td> sync_data4 </td><td> read, write </td><td> Generic data for use by syncing </td></tr>
 *     <tr><td> integer </td><td> mode </td><td> read, write </td><td> Calendar book mode (refer to the @ref calendar_book_mode_e) </td></tr>
 * </table>
 */
_CALENDAR_BEGIN_VIEW()
	_CALENDAR_PROPERTY_INT( id )            // read_only
	_CALENDAR_PROPERTY_STR( uid )
	_CALENDAR_PROPERTY_STR( name )
	_CALENDAR_PROPERTY_STR( description )
	_CALENDAR_PROPERTY_STR( color )
	_CALENDAR_PROPERTY_STR( location )
	_CALENDAR_PROPERTY_INT( visibility )
	_CALENDAR_PROPERTY_INT( sync_event )
	_CALENDAR_PROPERTY_INT( account_id )
	_CALENDAR_PROPERTY_INT( store_type )
	_CALENDAR_PROPERTY_STR( sync_data1 )
	_CALENDAR_PROPERTY_STR( sync_data2 )
	_CALENDAR_PROPERTY_STR( sync_data3 )
	_CALENDAR_PROPERTY_STR( sync_data4 )
	_CALENDAR_PROPERTY_INT( mode )
_CALENDAR_END_VIEW( _calendar_book )

/**
 * @ingroup CAPI_SOCIAL_CALENDAR_SVC_MODULE
 * @addtogroup CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE_calendar_event _calendar_event view
 * <table>
 *     <tr>
 *     <th> Type </th>
 *     <th> Property ID </th>
 *     <th> Read, Write </th>
 *     <th> Description </th>
 *     </tr>
 *     <tr><td> string </td><td> _uri </td><td> read only </td><td>Identifier of this event view</td></tr>
 *     <tr><td> integer </td><td> id </td><td> read only </td><td>DB record ID of the event</td></tr>
 *     <tr><td> integer </td><td> calendar_book_id </td><td> read, write </td><td>ID of the calendar book to which the event belongs</td></tr>
 *     <tr><td> string </td><td> summary </td><td> read, write </td><td>The short description of the event</td></tr>
 *     <tr><td> string </td><td> description </td><td> read, write </td><td>The description of the event</td></tr>
 *     <tr><td> string </td><td> location </td><td> read, write </td><td>The location of the event</td></tr>
 *     <tr><td> string </td><td> categories </td><td> read, write </td><td>The category of the event. For example APPOINTMENT, BIRTHDAY</td></tr>
 *     <tr><td> string </td><td> exdate </td><td> read, write </td><td>The exception list of the event. If this event has a recurrence rule, the instance of the exdate is removed. Format is "YYYYMMDD"(allday event) or "YYYYMMDDTHHMMSS".  Multiple exceptions can be included with a comma  </td></tr>
 *     <tr><td> integer </td><td> event_status </td><td> read, write </td><td>Refer to the @ref calendar_event_status_e</td></tr>
 *     <tr><td> integer </td><td> priority </td><td> read, write </td><td></td>Refer to the @ref calendar_event_priority_e</tr>
 *     <tr><td> integer </td><td> timezone </td><td> read, write </td><td>The timezone_id of the event if it exists. Refer to the @ref CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE_calendar_timezone</td></tr>
 *     <tr><td> integer </td><td> person_id </td><td> read, write </td><td>The person_id of the event if the event is a birthday. Refer to the contacts-service</td></tr>
 *     <tr><td> integer </td><td> busy_status </td><td> read, write </td><td>Refer to the @ref calendar_event_busy_status_e</td></tr>
 *     <tr><td> integer </td><td> sensitivity </td><td> read, write </td><td>Refer to the @ref calendar_sensitivity_e </td></tr>
 *     <tr><td> string </td><td> uid </td><td> read, write </td><td>The unique ID of the event</td></tr>
 *     <tr><td> string </td><td> organizer_name </td><td> read, write </td><td>The name of organizer of the event</td></tr>
 *     <tr><td> string </td><td> organizer_email </td><td> read, write </td><td>The email address of the organizer of the event</td></tr>
 *     <tr><td> integer </td><td> meeting_status </td><td> read, write </td><td>Refer to the @ref calendar_meeting_status_e</td></tr>
 *     <tr><td> integer </td><td> original_event_id </td><td> read, write </td><td>The ID of the original event if the event is an exception.</td></tr>
 *     <tr><td> double </td><td> latitude </td><td> read, write </td><td> The latitude of the location of the event</td></tr>
 *     <tr><td> double </td><td> longitude </td><td> read, write </td><td> The longitude of the location of the event</td></tr>
 *     <tr><td> integer </td><td> email_id </td><td> read, write </td><td>ID of the email_id. Refer to the email-service.</td></tr>
 *     <tr><td> long long int </td><td> created_time </td><td> read, write </td><td> The time when the event is created</td></tr>
 *     <tr><td> long long int </td><td> last_modified_time </td><td> read only </td><td>The time when the event is updated</td></tr>
 *     <tr><td> integer </td><td> is_deleted </td><td> read only </td><td></td></tr>
 *     <tr><td> integer </td><td> freq </td><td> read, write </td><td>The frequent type of event recurrence. Refer to the @ref calendar_recurrence_frequency_e</td></tr>
 *     <tr><td> integer </td><td> range_type </td><td> read, write </td><td>Refer to the @ref calendar_range_type_e</td></tr>
 *     <tr><td> calendar time </td><td> until_time </td><td> read, write </td><td>The end time of the event recurrence. If the range_type is @ref CALENDAR_RANGE_UNTIL</td></tr>
 *     <tr><td> integer </td><td> count </td><td> read, write </td><td>The count of the event recurrence. If the range_type is @ref CALENDAR_RANGE_COUNT</td></tr>
 *     <tr><td> integer </td><td> interval </td><td> read, write </td><td>The interval of the event recurrence</td></tr>
 *     <tr><td> string </td><td> bysecond </td><td> read, write </td><td>The second list of the event recurrence. The value can be from 0 to 59. The list is seperated by commas</td></tr>
 *     <tr><td> string </td><td> byminute </td><td> read, write </td><td>The minute list of the event recurrence. The value can be from 0 to 59. The list is seperated by commas</td></tr>
 *     <tr><td> string </td><td> byhour </td><td> read, write </td><td>The hour list of the event recurrence. The value can be from 0 to 23. The list is seperated by commas</td></tr>
 *     <tr><td> string </td><td> byday </td><td> read, write </td><td>The day list of the event recurrence. The value can be SU, MO, TU, WE, TH, FR, SA. The list is seperated by commas</td></tr>
 *     <tr><td> string </td><td> bymonthday </td><td> read, write </td><td>The month day list of the event recurrence. The value can be from 1 to 31 and from -31 to -1. The list is seperated by commas</td></tr>
 *     <tr><td> string </td><td> byyearday </td><td> read, write </td><td>The year day list of the event recurrence. The value can be from 1 to 366 and from -366 to -1. The list is seperated by commas</td></tr>
 *     <tr><td> string </td><td> byweekno </td><td> read, write </td><td>The week number list of the event recurrence. The value can be from 1 to 53 and from -53 to -1. The list is seperated by commas</td></tr>
 *     <tr><td> string </td><td> bymonth </td><td> read, write </td><td>The month list of the event recurrence. The value can be from 1 to 12. The list is seperated by commas</td></tr>
 *     <tr><td> string </td><td> bysetpos </td><td> read, write </td><td>The position list of the event recurrence. The value can be from 1 to 366 and from -366 to -1. The list is seperated by commas</td></tr>
 *     <tr><td> integer </td><td> wkst </td><td> read, write </td><td>The start day of the week. Refer to the @ref calendar_days_of_week_e</td></tr>
 *     <tr><td> string </td><td> recurrence_id </td><td> read, write </td><td>RECURRENCE-ID of RFC #2445</td></tr>
 *     <tr><td> string </td><td> rdate </td><td> read, write </td><td>RDATE of RFC #2445</td></tr>
 *     <tr><td> integer </td><td> has_attendee </td><td> read only </td><td>Whether or not the event has an attendee list </td></tr>
 *     <tr><td> integer </td><td> has_alarm </td><td> read only </td><td>Whether or not the event has an alarm list </td></tr>
 *     <tr><td> integer </td><td> calendar_system_type </td><td> read, write </td><td>Refer to the @ref calendar_system_type_e</td></tr>
 *     <tr><td> string </td><td> sync_data1 </td><td> read, write </td><td>The sync data of the event. If developer need to save some information related to the event, they can use this property</td></tr>
 *     <tr><td> string </td><td> sync_data2 </td><td> read, write </td><td>The sync data of the event. If developer need to save some information related to the event, they can use this property</td></tr>
 *     <tr><td> string </td><td> sync_data3 </td><td> read, write </td><td>The sync data of the event. If developer need to save some information related to the event, they can use this property</td></tr>
 *     <tr><td> string </td><td> sync_data4 </td><td> read, write </td><td>The sync data of the event. If developer need to save some information related to the event, they can use this property</td></tr>
 *     <tr><td> calendar time </td><td> start_time </td><td> read, write </td><td>The start time of the event</td></tr>
 *     <tr><td> string </td><td> start_tzid </td><td> read, write </td><td></td>The timezone of the start_time</tr>
 *     <tr><td> calendar time </td><td> end_time </td><td> read, write </td><td>The end time of the event</td></tr>
 *     <tr><td> string </td><td> end_tzid </td><td> read, write </td><td>The timezone of the end_time</td></tr>
 *     <tr><td> child list </td><td> calendar_alarm </td><td> read, write </td><td>The alarm list of the event. Refer to the @ref CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE_calendar_alarm</td></tr>
 *     <tr><td> child list </td><td> calendar_attendee </td><td> read, write </td><td>The attendee list of the event. Refer to the @ref CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE_calendar_attendee</td></tr>
 *     <tr><td> child list </td><td> exception </td><td> read, write </td><td>The exception mod event list of the event</td></tr>
 *     <tr><td> child list </td><td> extended </td><td> read, write </td><td>The extended property list of the event. Refer to the @ref CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE_calendar_extended_property</td></tr>
 *     <tr><td> interger </td><td> is_allday </td><td> read only </td><td>The event is an allday event or not </td></tr>
 * </table>
 */
_CALENDAR_BEGIN_VIEW()
	_CALENDAR_PROPERTY_INT( id )                // read_only
	_CALENDAR_PROPERTY_INT( calendar_book_id )
	_CALENDAR_PROPERTY_STR( summary )
	_CALENDAR_PROPERTY_STR( description )
	_CALENDAR_PROPERTY_STR( location )
	_CALENDAR_PROPERTY_STR( categories )
	_CALENDAR_PROPERTY_STR( exdate )
	_CALENDAR_PROPERTY_INT( event_status )
	_CALENDAR_PROPERTY_INT( priority )
	_CALENDAR_PROPERTY_INT( timezone )
	_CALENDAR_PROPERTY_INT( person_id )
	_CALENDAR_PROPERTY_INT( busy_status )
	_CALENDAR_PROPERTY_INT( sensitivity )
	_CALENDAR_PROPERTY_STR( uid )
	_CALENDAR_PROPERTY_STR( organizer_name )
	_CALENDAR_PROPERTY_STR( organizer_email )
	_CALENDAR_PROPERTY_INT( meeting_status )
	_CALENDAR_PROPERTY_INT( original_event_id )
	_CALENDAR_PROPERTY_DOUBLE( latitude )
	_CALENDAR_PROPERTY_DOUBLE( longitude )
	_CALENDAR_PROPERTY_INT( email_id )
	_CALENDAR_PROPERTY_LLI( created_time )
	_CALENDAR_PROPERTY_LLI( last_modified_time ) // read_only
	_CALENDAR_PROPERTY_INT( is_deleted )        // read_only
	_CALENDAR_PROPERTY_INT( freq )
	_CALENDAR_PROPERTY_INT( range_type )
	_CALENDAR_PROPERTY_CALTIME( until_time )
	_CALENDAR_PROPERTY_INT( count )
	_CALENDAR_PROPERTY_INT( interval )
	_CALENDAR_PROPERTY_STR( bysecond )
	_CALENDAR_PROPERTY_STR( byminute )
	_CALENDAR_PROPERTY_STR( byhour )
	_CALENDAR_PROPERTY_STR( byday )
	_CALENDAR_PROPERTY_STR( bymonthday )
	_CALENDAR_PROPERTY_STR( byyearday )
	_CALENDAR_PROPERTY_STR( byweekno )
	_CALENDAR_PROPERTY_STR( bymonth )
	_CALENDAR_PROPERTY_STR( bysetpos )
	_CALENDAR_PROPERTY_INT( wkst )
	_CALENDAR_PROPERTY_STR( recurrence_id )
	_CALENDAR_PROPERTY_STR( rdate )
	_CALENDAR_PROPERTY_INT( has_attendee )      // read_only
	_CALENDAR_PROPERTY_INT( has_alarm )         // read_only
	_CALENDAR_PROPERTY_INT( calendar_system_type )
	_CALENDAR_PROPERTY_STR( sync_data1 )
	_CALENDAR_PROPERTY_STR( sync_data2 )
	_CALENDAR_PROPERTY_STR( sync_data3 )
	_CALENDAR_PROPERTY_STR( sync_data4 )
	_CALENDAR_PROPERTY_CALTIME( start_time )
	_CALENDAR_PROPERTY_STR( start_tzid )
	_CALENDAR_PROPERTY_CALTIME( end_time )
	_CALENDAR_PROPERTY_STR( end_tzid )
	_CALENDAR_PROPERTY_CHILD_MULTIPLE( calendar_alarm )
	_CALENDAR_PROPERTY_CHILD_MULTIPLE( calendar_attendee )
	_CALENDAR_PROPERTY_CHILD_MULTIPLE( exception )
	_CALENDAR_PROPERTY_CHILD_MULTIPLE( extended )
	_CALENDAR_PROPERTY_INT( is_allday )         // read only
_CALENDAR_END_VIEW( _calendar_event )

/**
 * @ingroup CAPI_SOCIAL_CALENDAR_SVC_MODULE
 * @addtogroup CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE_calendar_todo _calendar_todo view
 * <table>
 *     <tr>
 *     <th> Type </th>
 *     <th> Property ID </th>
 *     <th> Read, Write </th>
 *     <th> Description </th>
 *     </tr>
 *     <tr><td> string </td><td> _uri </td><td> read only </td><td>Identifier of this todo view</td></tr>
 *     <tr><td> integer </td><td> id </td><td> read only </td><td>DB record ID of the todo</td></tr>
 *     <tr><td> integer </td><td> calendar_book_id </td><td> read, write </td><td>ID of the calendar book to which the todo belongs</td></tr>
 *     <tr><td> string </td><td> summary </td><td> read, write </td><td>The short description of the todo</td></tr>
 *     <tr><td> string </td><td> description </td><td> read, write </td><td>The description of the todo</td></tr>
 *     <tr><td> string </td><td> location </td><td> read, write </td><td>The location of the todo</td></tr>
 *     <tr><td> string </td><td> categories </td><td> read, write </td><td>The category of the todo. For example APPOINTMENT, BIRTHDAY</td></tr>
 *     <tr><td> integer </td><td> todo_status </td><td> read, write </td><td>Refer to the @ref calendar_todo_status_e</td></tr>
 *     <tr><td> integer </td><td> priority </td><td> read, write </td><td>Refer to the @ref calendar_todo_priority_e</td></tr>
 *     <tr><td> integer </td><td> sensitivity </td><td> read, write </td><td>Refer to the @ref calendar_sensitivity_e </td></tr>
 *     <tr><td> string </td><td> uid </td><td> read, write </td><td>The unique ID of the todo</td></tr>
 *     <tr><td> double </td><td> latitude </td><td> read, write </td><td>The latitude of the location of the todo</td></tr>
 *     <tr><td> double </td><td> longitude </td><td> read, write </td><td>The longitude of the location of the todo</td></tr>
 *     <tr><td> long long int </td><td> created_time </td><td> read, write </td><td>The time when the todo is created</td></tr>
 *     <tr><td> long long int </td><td> last_modified_time </td><td> read only </td><td>The time when the todo is updated</td></tr>
 *     <tr><td> long long int </td><td> completed_time </td><td> read, write </td><td>The time when the todo is completed</td></tr>
 *     <tr><td> integer </td><td> progress </td><td> read, write </td><td> The progression of the todo. The value can be from 0 to 100</td></tr>
 *     <tr><td> integer </td><td> is_deleted </td><td> read only </td><td></td></tr>
 *     <tr><td> integer </td><td> freq </td><td> read, write </td><td>The frequent type of todo recurrence. Refer to the @ref calendar_recurrence_frequency_e</td></tr>
 *     <tr><td> integer </td><td> range_type </td><td> read, write </td><td>Refer to the @ref calendar_range_type_e</td></tr>
 *     <tr><td> calendar time </td><td> until_time </td><td> read, write </td><td>The end time of the todo recurrence. If the range_type is CALENDAR_RANGE_UNTIL</td></tr>
 *     <tr><td> integer </td><td> count </td><td> read, write </td><td>The count of the todo recurrence. If the range_type is CALENDAR_RANGE_COUNT</td></tr>
 *     <tr><td> integer </td><td> interval </td><td> read, write </td><td>The interval of the todo recurrence</td></tr>
 *     <tr><td> string </td><td> bysecond </td><td> read, write </td><td>The second list of the todo recurrence. The value can be from 0 to 59. The list is seperated by commas</td></tr>
 *     <tr><td> string </td><td> byminute </td><td> read, write </td><td>The minute list of the todo recurrence. The value can be from 0 to 59. The list is seperated by commas</td></tr>
 *     <tr><td> string </td><td> byhour </td><td> read, write </td><td>The hour list of the todo recurrence. The value can be from 0 to 23. The list is seperated by commas</td></tr>
 *     <tr><td> string </td><td> byday </td><td> read, write </td><td>The day list of the todo recurrence. The value can be SU, MO, TU, WE, TH, FR, SA. The list is seperated by commas</td></tr>
 *     <tr><td> string </td><td> bymonthday </td><td> read, write </td><td>The month day list of the todo recurrence. The value can be from 1 to 31 and from -31 to -1. The list is seperated by commas</td></tr>
 *     <tr><td> string </td><td> byyearday </td><td> read, write </td><td>The year day list of the todo recurrence. The value can be from 1 to 366 and from -366 to -1. The list is seperated by commas</td></tr>
 *     <tr><td> string </td><td> byweekno </td><td> read, write </td><td>The week number list of the todo recurrence. The value can be from 1 to 53 and from -53 to -1. The list is seperated by commas</td></tr>
 *     <tr><td> string </td><td> bymonth </td><td> read, write </td><td>The month list of the todo recurrence. The value can be from 1 to 12. The list is seperated by commas</td></tr>
 *     <tr><td> string </td><td> bysetpos </td><td> read, write </td><td>The position list of the todo recurrence. The value can be from 1 to 366 and from -366 to -1. The list is seperated by commas</td></tr>
 *     <tr><td> integer </td><td> wkst </td><td> read, write </td><td>The start day of the week. Refer to the @ref calendar_days_of_week_e</td></tr>
 *     <tr><td> integer </td><td> has_alarm </td><td> read only </td><td>Whether or not the todo has an alarm list </td></tr>
 *     <tr><td> string </td><td> sync_data1 </td><td> read, write </td><td>The sync data of the todo. If developers need to save some information related to the todo, they can use this property</td></tr>
 *     <tr><td> string </td><td> sync_data2 </td><td> read, write </td><td>The sync data of the todo. If developers need to save some information related to the todo, they can use this property</td></tr>
 *     <tr><td> string </td><td> sync_data3 </td><td> read, write </td><td>The sync data of the todo. If developers need to save some information related to the todo, they can use this property</td></tr>
 *     <tr><td> string </td><td> sync_data4 </td><td> read, write </td><td>The sync data of the todo. If developers need to save some information related to the todo, they can use this property</td></tr>
 *     <tr><td> calendar time </td><td> start_time </td><td> read, write </td><td>The start time of the todo</td></tr>
 *     <tr><td> string </td><td> start_tzid </td><td> read, write </td><td></td>The timezone of the start_time</tr>
 *     <tr><td> calendar time </td><td> due_time </td><td> read, write </td><td>The due time of the todo</td></tr>
 *     <tr><td> string </td><td> due_tzid </td><td> read, write </td><td>The timezone of the due_time</td></tr>
 *     <tr><td> child list </td><td> calendar_alarm </td><td> read, write </td><td>The alarm list of the todo. Refer to the @ref CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE_calendar_alarm</td></tr>
 *     <tr><td> string </td><td> organizer_name </td><td> read, write </td><td>The name of the organizer of the event</td></tr>
 *     <tr><td> string </td><td> organizer_email </td><td> read, write </td><td>The email address of the organizer of the event</td></tr>
 *     <tr><td> integer </td><td> has_attendee </td><td> read only </td><td>Whether or not the todo has an attendee list </td></tr>
 *     <tr><td> child list </td><td> calendar_attendee </td><td> read, write </td><td>The attendee list of the todo. Refer to the @ref CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE_calendar_attendee</td></tr>
 *     <tr><td> child list </td><td> extended </td><td> read, write </td><td>The extended property list of the todo. Refer to the @ref CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE_calendar_extended_property</td></tr>
 *     <tr><td> interger </td><td> is_allday </td><td> read only </td><td>The todo is an allday event or not </td></tr>
 * </table>
 */
_CALENDAR_BEGIN_VIEW()
	_CALENDAR_PROPERTY_INT( id )            // read_only
	_CALENDAR_PROPERTY_INT( calendar_book_id )
	_CALENDAR_PROPERTY_STR( summary )
	_CALENDAR_PROPERTY_STR( description )
	_CALENDAR_PROPERTY_STR( location )
	_CALENDAR_PROPERTY_STR( categories )
	_CALENDAR_PROPERTY_INT( todo_status )
	_CALENDAR_PROPERTY_INT( priority )
	_CALENDAR_PROPERTY_INT( sensitivity )
	_CALENDAR_PROPERTY_STR( uid )
	_CALENDAR_PROPERTY_DOUBLE( latitude )
	_CALENDAR_PROPERTY_DOUBLE( longitude )
	_CALENDAR_PROPERTY_LLI( created_time )
	_CALENDAR_PROPERTY_LLI( last_modified_time ) // read_only
	_CALENDAR_PROPERTY_LLI( completed_time )
	_CALENDAR_PROPERTY_INT( progress )
	_CALENDAR_PROPERTY_INT( is_deleted )    // read_only
	_CALENDAR_PROPERTY_INT( freq )
	_CALENDAR_PROPERTY_INT( range_type )
	_CALENDAR_PROPERTY_CALTIME( until_time )
	_CALENDAR_PROPERTY_INT( count )
	_CALENDAR_PROPERTY_INT( interval )
	_CALENDAR_PROPERTY_STR( bysecond )
	_CALENDAR_PROPERTY_STR( byminute )
	_CALENDAR_PROPERTY_STR( byhour )
	_CALENDAR_PROPERTY_STR( byday )
	_CALENDAR_PROPERTY_STR( bymonthday )
	_CALENDAR_PROPERTY_STR( byyearday )
	_CALENDAR_PROPERTY_STR( byweekno )
	_CALENDAR_PROPERTY_STR( bymonth )
	_CALENDAR_PROPERTY_STR( bysetpos )
	_CALENDAR_PROPERTY_INT( wkst )
	_CALENDAR_PROPERTY_INT( has_alarm )     // read_only
	_CALENDAR_PROPERTY_STR( sync_data1 )
	_CALENDAR_PROPERTY_STR( sync_data2 )
	_CALENDAR_PROPERTY_STR( sync_data3 )
	_CALENDAR_PROPERTY_STR( sync_data4 )
	_CALENDAR_PROPERTY_CALTIME( start_time )
	_CALENDAR_PROPERTY_STR( start_tzid )
	_CALENDAR_PROPERTY_CALTIME( due_time )
	_CALENDAR_PROPERTY_STR( due_tzid )
	_CALENDAR_PROPERTY_CHILD_MULTIPLE( calendar_alarm )
    _CALENDAR_PROPERTY_STR( organizer_name )
    _CALENDAR_PROPERTY_STR( organizer_email )
    _CALENDAR_PROPERTY_INT( has_attendee )     // read_only
    _CALENDAR_PROPERTY_CHILD_MULTIPLE( calendar_attendee )
    _CALENDAR_PROPERTY_CHILD_MULTIPLE( extended )
    _CALENDAR_PROPERTY_INT( is_allday )         // read only
_CALENDAR_END_VIEW( _calendar_todo )

/**
 * @ingroup CAPI_SOCIAL_CALENDAR_SVC_MODULE
 * @addtogroup CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE_calendar_timezone _calendar_timezone view
 * <table>
 *     <tr>
 *     <th> Type </th>
 *     <th> Property ID </th>
 *     <th> Read, Write </th>
 *     <th> Description </th>
 *     </tr>
 *     <tr><td> string </td><td> _uri </td><td> read only </td><td> Identifier of this timezone view </td></tr>
 *     <tr><td> integer </td><td> id </td><td> read only </td><td> DB record ID of the timezone </td></tr>
 *     <tr><td> integer </td><td> calendar_book_id </td><td> read, write </td><td> DB record ID of a related calendar book </td></tr>
 *     <tr><td> integer </td><td> tz_offset_from_gmt </td><td> read, write </td><td> UTC offset which is in use when the onset of this time zone observance begins. Valid values are -720(-12:00) to 840(+14:00) </td></tr>
 *     <tr><td> string </td><td> standard_name </td><td> read, write </td><td> Name of the Standard Time </td></tr>
 *     <tr><td> integer </td><td> standard_start_month </td><td> read, write </td><td> Starting month of the Standard Time. Month is 0-based. eg, 0 for January </td></tr>
 *     <tr><td> integer </td><td> standard_start_position_of_week </td><td> read, write </td><td> Starting day-of-week-in-month of the Standard Time. Day is 1-based </td></tr>
 *     <tr><td> integer </td><td> standard_start_day </td><td> read, write </td><td> Starting day-of-week of the Standard Time. Valid values are 1(SUNDAY) to 7(SATURDAY) </td></tr>
 *     <tr><td> integer </td><td> standard_start_hour </td><td> read, write </td><td> Starting hour of the Standard Time. Valid values are 0 to 23 </td></tr>
 *     <tr><td> integer </td><td> standard_bias </td><td> read, write </td><td> The number of minutes added during the Standard Time </td></tr>
 *     <tr><td> string </td><td> day_light_name </td><td> read, write </td><td> Name of Daylight </td></tr>
 *     <tr><td> integer </td><td> day_light_start_month </td><td> read, write </td><td> Starting month of Daylight. Month is 0-based. eg, 0 for January </td></tr>
 *     <tr><td> integer </td><td> day_light_start_position_of_week </td><td> read, write </td><td> Starting day-of-week-in-month of Daylight. Day is 1-based </td></tr>
 *     <tr><td> integer </td><td> day_light_start_day </td><td> read, write </td><td> Starting day-of-week of Daylight. Valid values are 1(SUNDAY) to 7(SATURDAY) </td></tr>
 *     <tr><td> integer </td><td> day_light_start_hour </td><td> read, write </td><td> Starting hour of Daylight. Valid values are 0 to 23 </td></tr>
 *     <tr><td> integer </td><td> day_light_bias </td><td> read, write </td><td> The number of minutes added during Daylight Time </td></tr>
 * </table>
 */
_CALENDAR_BEGIN_VIEW()
	_CALENDAR_PROPERTY_INT( id )                    // read_only
	_CALENDAR_PROPERTY_INT( calendar_book_id )
	_CALENDAR_PROPERTY_INT( tz_offset_from_gmt ) // offset(minute)
	_CALENDAR_PROPERTY_STR( standard_name )
	_CALENDAR_PROPERTY_INT( standard_start_month )
	_CALENDAR_PROPERTY_INT( standard_start_position_of_week ) // nth wday
	_CALENDAR_PROPERTY_INT( standard_start_day ) // wday
	_CALENDAR_PROPERTY_INT( standard_start_hour )
	_CALENDAR_PROPERTY_INT( standard_bias )
	_CALENDAR_PROPERTY_STR( day_light_name )
	_CALENDAR_PROPERTY_INT( day_light_start_month )
	_CALENDAR_PROPERTY_INT( day_light_start_position_of_week )
	_CALENDAR_PROPERTY_INT( day_light_start_day )
	_CALENDAR_PROPERTY_INT( day_light_start_hour )
	_CALENDAR_PROPERTY_INT( day_light_bias ) // diff between standard and daylight(minute)
_CALENDAR_END_VIEW( _calendar_timezone )

/**
 * @ingroup CAPI_SOCIAL_CALENDAR_SVC_MODULE
 * @addtogroup CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE_calendar_attendee _calendar_attendee view
 * <table>
 *     <tr>
 *     <th> Type </th>
 *     <th> Property ID </th>
 *     <th> Read, Write </th>
 *     <th> Description </th>
 *     </tr>
 *     <tr><td> string </td><td> _uri </td><td> read only </td><td> Identifier of this calendar attendee view </td></tr>
 *     <tr><td> integer </td><td> event_id </td><td> read only </td><td> Event/TODO that the attendee belongs to </td></tr>
 *     <tr><td> string </td><td> number </td><td> read, write </td><td> The number of the attendee </td></tr>
 *     <tr><td> integer </td><td> cutype </td><td> read, write </td><td> The type of attendee (one of CALENDAR_ATTENDEE_CUTYPE_INDIVIDUAL, CALENDAR_ATTENDEE_CUTYPE_GROUP, CALENDAR_ATTENDEE_CUTYPE_RESOURCE, CALENDAR_ATTENDEE_CUTYPE_ROOM, CALENDAR_ATTENDEE_CUTYPE_UNKNOWN) </tr>
 *     <tr><td> integer </td><td> person_id </td><td> read, write </td><td> Person ID that the attendee belongs to </td></tr>
 *     <tr><td> string </td><td> uid </td><td> read, write </td><td> Unique identifier </td></tr>
 *     <tr><td> string </td><td> email </td><td> read, write </td><td> The email address of the attendee </td></tr>
 *     <tr><td> integer </td><td> role </td><td> read, write </td><td> Attendee role (one of CALENDAR_ATTENDEE_ROLE_REQ_PARTICIPANT, CALENDAR_ATTENDEE_ROLE_OPT_PARTICIPANT, CALENDAR_ATTENDEE_ROLE_NON_PARTICIPANT, CALENDAR_ATTENDEE_ROLE_CHAIR) </td></tr>
 *     <tr><td> integer </td><td> status </td><td> read, write </td><td> Attendee status (one of CALENDAR_ATTENDEE_STATUS_PENDING, CALENDAR_ATTENDEE_STATUS_ACCEPTED, CALENDAR_ATTENDEE_STATUS_DECLINED, CALENDAR_ATTENDEE_STATUS_TENTATIVE, CALENDAR_ATTENDEE_STATUS_DELEGATED, CALENDAR_ATTENDEE_STATUS_COMPLETED, CALENDAR_ATTENDEE_STATUS_IN_PROCESS) </td></tr>
 *     <tr><td> integer </td><td> rsvp </td><td> read, write </td><td> RSVP invitation reply (one of true, false) </td></tr>
 *     <tr><td> string </td><td> delegatee_uri </td><td> read, write </td><td> Delegatee (DELEGATED-TO) </td></tr>
 *     <tr><td> string </td><td> delegator_uri </td><td> read, write </td><td> Delegator (DELEGATED-FROM) </td></tr>
 *     <tr><td> string </td><td> name </td><td> read, write </td><td> Attendee name </td></tr>
 *     <tr><td> string </td><td> member </td><td> read, write </td><td> Group that the attendee belongs to </td></tr>
 * </table>
 */
_CALENDAR_BEGIN_VIEW()
    _CALENDAR_PROPERTY_INT( parent_id ) // read_only
	_CALENDAR_PROPERTY_STR( number )
	_CALENDAR_PROPERTY_INT( cutype ) // calendar user type: INDIVIDUAL, GROUP, RESOURCE, ROOM, UNKNOWN
	_CALENDAR_PROPERTY_INT( person_id )
	_CALENDAR_PROPERTY_STR( uid )
	_CALENDAR_PROPERTY_STR( group )
	_CALENDAR_PROPERTY_STR( email )
	_CALENDAR_PROPERTY_INT( role )
	_CALENDAR_PROPERTY_INT( status )
	_CALENDAR_PROPERTY_INT( rsvp )
	_CALENDAR_PROPERTY_STR( delegatee_uri )
	_CALENDAR_PROPERTY_STR( delegator_uri )
	_CALENDAR_PROPERTY_STR( name )
	_CALENDAR_PROPERTY_STR( member )
_CALENDAR_END_VIEW( _calendar_attendee )

/**
 * @ingroup CAPI_SOCIAL_CALENDAR_SVC_MODULE
 * @addtogroup CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE_calendar_alarm _calendar_alarm view
 * <table>
 *     <tr>
 *     <th> Type </th>
 *     <th> Property ID </th>
 *     <th> Read, Write </th>
 *     <th> Description </th>
 *     </tr>
 *     <tr><td> string </td><td> _uri </td><td> read only </td><td> Identifier of this calendar alarm view </td></tr>
 *     <tr><td> integer </td><td> parent_id </td><td> read only </td><td> Event that the alarm belongs to </td></tr>
 *     <tr><td> integer </td><td> type </td><td> read, write </td><td> Currently NOT used </td></tr>
 *     <tr><td> long long int </td><td> time </td><td> read, write </td><td> The alarm time of the event(This represents the number of seconds elapsed since the Epoch, 1970-01-01 00:00:00 +0000(UTC)). This MUST be used with CALENDAR_ALARM_TIME_UNIT_SPECIFIC </td></tr>
 *     <tr><td> integer </td><td> tick </td><td> read, write </td><td> The number of unit before start time. This MUST be used with one of CALENDAR_ALARM_TIME_UNIT_MINUTE, CALENDAR_ALARM_TIME_UNIT_HOUR, CALENDAR_ALARM_TIME_UNIT_DAY, CALENDAR_ALARM_TIME_UNIT_WEEK. </td></tr>
 *     <tr><td> integer </td><td> tick_unit </td><td> read, write </td><td> Reminder tick time unit (one of CALENDAR_ALARM_NONE, CALENDAR_ALARM_TIME_UNIT_SPECIFIC, CALENDAR_ALARM_TIME_UNIT_MINUTE, CALENDAR_ALARM_TIME_UNIT_HOUR, CALENDAR_ALARM_TIME_UNIT_DAY, CALENDAR_ALARM_TIME_UNIT_WEEK) </td></tr>
 *     <tr><td> string </td><td> attach </td><td> read, write </td><td> Alarm tone path </td></tr>
 *     <tr><td> string </td><td> summary </td><td> read, write </td><td> Alarm summary </td></tr>
 *     <tr><td> string </td><td> description </td><td> read, write </td><td> Alarm description </td></tr>
 *     <tr><td> integer </td><td> action </td><td> read, write </td><td> Action of alarm (one of CALENDAR_ALARM_ACTION_AUDIO, CALENDAR_ALARM_ACTION_DISPLAY, CALENDAR_ALARM_ACTION_EMAIL) </td></tr>
 *     <tr><td> calendar time </td><td> alarm_time </td><td> read, write </td><td>The alarm time </td></tr>
 * </table>
 */
_CALENDAR_BEGIN_VIEW()
    _CALENDAR_PROPERTY_INT( parent_id ) // read_only
	_CALENDAR_PROPERTY_INT( tick )
	_CALENDAR_PROPERTY_INT( tick_unit )
	_CALENDAR_PROPERTY_STR( description )
	_CALENDAR_PROPERTY_STR( summary ) // emailprop: summary
	_CALENDAR_PROPERTY_INT( action ) // AUDIO, DISPLAY, EMAIL
	_CALENDAR_PROPERTY_STR( attach )
    _CALENDAR_PROPERTY_CALTIME( alarm_time )
_CALENDAR_END_VIEW( _calendar_alarm )

/**
 * @ingroup CAPI_SOCIAL_CALENDAR_SVC_MODULE
 * @addtogroup CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE_calendar_updated_info _calendar_updated_info view (read only)
 * <table>
 *     <tr>
 *     <th> Type </th>
 *     <th> Property ID </th>
 *     <th> Description </th>
 *     </tr>
 *     <tr><td> string </td><td> _uri </td><td> Identifier of this updated_info view </td></tr>
 *     <tr><td> integer </td><td> id </td><td> Modified event(or todo) record ID </td></tr>
 *     <tr><td> integer </td><td> calendar_book_id </td><td> Calendar book ID of the modified event(or todo) record </td></tr>
 *     <tr><td> integer </td><td> modified_status </td><td> Enumeration value of the modified status (@ref calendar_record_modified_status_e) </td></tr>
 *     <tr><td> integer </td><td> version </td><td> Version after change </td></tr>
 * </table>
 */
_CALENDAR_BEGIN_READ_ONLY_VIEW()
	_CALENDAR_PROPERTY_INT( id ) // read_only
	_CALENDAR_PROPERTY_INT( calendar_book_id )
	_CALENDAR_PROPERTY_INT( modified_status )
	_CALENDAR_PROPERTY_INT( version )
_CALENDAR_END_READ_ONLY_VIEW( _calendar_updated_info )

/**
 * @ingroup CAPI_SOCIAL_CALENDAR_SVC_MODULE
 * @addtogroup CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE_calendar_event_calendar_book _calendar_event_calendar_book view (read only)
 * <table>
 *     <tr>
 *     <th> Type </th>
 *     <th> Property ID </th>
 *     </tr>
 *     <tr><td> string </td><td> _uri </td></tr>
 *     <tr><td> integer </td><td> event_id </td></tr>
 *     <tr><td> integer </td><td> calendar_book_id </td></tr>
 *     <tr><td> string </td><td> summary </td></tr>
 *     <tr><td> string </td><td> description </td></tr>
 *     <tr><td> string </td><td> location </td></tr>
 *     <tr><td> string </td><td> categories </td></tr>
 *     <tr><td> string </td><td> exdate </td></tr>
 *     <tr><td> integer </td><td> event_status </td></tr>
 *     <tr><td> integer </td><td> priority </td></tr>
 *     <tr><td> integer </td><td> timezone </td></tr>
 *     <tr><td> integer </td><td> person_id </td></tr>
 *     <tr><td> integer </td><td> busy_status </td></tr>
 *     <tr><td> integer </td><td> sensitivity </td></tr>
 *     <tr><td> string </td><td> uid </td></tr>
 *     <tr><td> string </td><td> organizer_name </td></tr>
 *     <tr><td> string </td><td> organizer_email </td></tr>
 *     <tr><td> integer </td><td> meeting_status </td></tr>
 *     <tr><td> integer </td><td> original_event_id </td></tr>
 *     <tr><td> double </td><td> latitude </td></tr>
 *     <tr><td> double </td><td> longitude </td></tr>
 *     <tr><td> integer </td><td> email_id </td></tr>
 *     <tr><td> long long int </td><td> created_time </td></tr>
 *     <tr><td> long long int </td><td> last_modified_time </td></tr>
 *     <tr><td> integer </td><td> freq </td></tr>
 *     <tr><td> integer </td><td> range_type </td></tr>
 *     <tr><td> calendar time </td><td> until_time </td></tr>
 *     <tr><td> integer </td><td> count </td></tr>
 *     <tr><td> integer </td><td> interval </td></tr>
 *     <tr><td> string </td><td> bysecond </td></tr>
 *     <tr><td> string </td><td> byminute </td></tr>
 *     <tr><td> string </td><td> byhour </td></tr>
 *     <tr><td> string </td><td> byday </td></tr>
 *     <tr><td> string </td><td> bymonthday </td></tr>
 *     <tr><td> string </td><td> byyearday </td></tr>
 *     <tr><td> string </td><td> byweekno </td></tr>
 *     <tr><td> string </td><td> bymonth </td></tr>
 *     <tr><td> string </td><td> bysetpos </td></tr>
 *     <tr><td> integer </td><td> wkst </td></tr>
 *     <tr><td> string </td><td> recurrence_id </td></tr>
 *     <tr><td> string </td><td> rdate </td></tr>
 *     <tr><td> integer </td><td> has_attendee </td></tr>
 *     <tr><td> integer </td><td> has_alarm </td></tr>
 *     <tr><td> integer </td><td> calendar_system_type </td></tr>
 *     <tr><td> string </td><td> sync_data1 </td></tr>
 *     <tr><td> string </td><td> sync_data2 </td></tr>
 *     <tr><td> string </td><td> sync_data3 </td></tr>
 *     <tr><td> string </td><td> sync_data4 </td></tr>
 *     <tr><td> calendar time </td><td> start_time </td></tr>
 *     <tr><td> string </td><td> start_tzid </td></tr>
 *     <tr><td> calendar time </td><td> end_time </td></tr>
 *     <tr><td> string </td><td> end_tzid </td></tr>
 *     <tr><td> filter integer </td><td> calendar_book_visibility </td></tr>
 *     <tr><td> filter integer </td><td> calendar_book_account_id </td></tr>
 * </table>
 */
_CALENDAR_BEGIN_READ_ONLY_VIEW()
    _CALENDAR_PROPERTY_INT( event_id )
    _CALENDAR_PROPERTY_INT( calendar_book_id )
    _CALENDAR_PROPERTY_STR( summary )
    _CALENDAR_PROPERTY_STR( description )
    _CALENDAR_PROPERTY_STR( location )
    _CALENDAR_PROPERTY_STR( categories )
    _CALENDAR_PROPERTY_STR( exdate )
    _CALENDAR_PROPERTY_INT( event_status )
    _CALENDAR_PROPERTY_INT( priority )
    _CALENDAR_PROPERTY_INT( timezone )
    _CALENDAR_PROPERTY_INT( person_id )
    _CALENDAR_PROPERTY_INT( busy_status )
    _CALENDAR_PROPERTY_INT( sensitivity )
    _CALENDAR_PROPERTY_STR( uid )
    _CALENDAR_PROPERTY_STR( organizer_name )
    _CALENDAR_PROPERTY_STR( organizer_email )
    _CALENDAR_PROPERTY_INT( meeting_status )
    _CALENDAR_PROPERTY_INT( original_event_id )
    _CALENDAR_PROPERTY_DOUBLE( latitude )
    _CALENDAR_PROPERTY_DOUBLE( longitude )
    _CALENDAR_PROPERTY_INT( email_id )
    _CALENDAR_PROPERTY_LLI( created_time )
    _CALENDAR_PROPERTY_LLI( last_modified_time )
    _CALENDAR_PROPERTY_INT( freq )
    _CALENDAR_PROPERTY_INT( range_type )
    _CALENDAR_PROPERTY_CALTIME( until_time )
    _CALENDAR_PROPERTY_INT( count )
    _CALENDAR_PROPERTY_INT( interval )
    _CALENDAR_PROPERTY_STR( bysecond )
    _CALENDAR_PROPERTY_STR( byminute )
    _CALENDAR_PROPERTY_STR( byhour )
    _CALENDAR_PROPERTY_STR( byday )
    _CALENDAR_PROPERTY_STR( bymonthday )
    _CALENDAR_PROPERTY_STR( byyearday )
    _CALENDAR_PROPERTY_STR( byweekno )
    _CALENDAR_PROPERTY_STR( bymonth )
    _CALENDAR_PROPERTY_STR( bysetpos )
    _CALENDAR_PROPERTY_INT( wkst )
    _CALENDAR_PROPERTY_STR( recurrence_id )
    _CALENDAR_PROPERTY_STR( rdate )
    _CALENDAR_PROPERTY_INT( has_attendee )
    _CALENDAR_PROPERTY_INT( has_alarm )
    _CALENDAR_PROPERTY_INT( calendar_system_type )
    _CALENDAR_PROPERTY_STR( sync_data1 )
    _CALENDAR_PROPERTY_STR( sync_data2 )
    _CALENDAR_PROPERTY_STR( sync_data3 )
    _CALENDAR_PROPERTY_STR( sync_data4 )
    _CALENDAR_PROPERTY_CALTIME( start_time )
    _CALENDAR_PROPERTY_STR( start_tzid )
    _CALENDAR_PROPERTY_CALTIME( end_time )
    _CALENDAR_PROPERTY_STR( end_tzid )
    _CALENDAR_PROPERTY_INT( is_allday )         // read only
    _CALENDAR_PROPERTY_FILTER_INT( calendar_book_visibility )
    _CALENDAR_PROPERTY_FILTER_INT( calendar_book_account_id )
_CALENDAR_END_READ_ONLY_VIEW( _calendar_event_calendar_book )

/**
 * @ingroup CAPI_SOCIAL_CALENDAR_SVC_MODULE
 * @addtogroup CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE_calendar_todo_calendar_book _calendar_todo_calendar_book view (read only)
 * <table>
 *     <tr>
 *     <th> Type </th>
 *     <th> Property ID </th>
 *     </tr>
 *     <tr><td> string </td><td> _uri </td></tr>
 *     <tr><td> integer </td><td> todo_id </td></tr>
 *     <tr><td> integer </td><td> calendar_book_id </td></tr>
 *     <tr><td> string </td><td> summary </td></tr>
 *     <tr><td> string </td><td> description </td></tr>
 *     <tr><td> string </td><td> location </td></tr>
 *     <tr><td> string </td><td> categories </td></tr>
 *     <tr><td> integer </td><td> todo_status </td></tr>
 *     <tr><td> integer </td><td> priority </td></tr>
 *     <tr><td> integer </td><td> sensitivity </td></tr>
 *     <tr><td> string </td><td> uid </td></tr>
 *     <tr><td> double </td><td> latitude </td></tr>
 *     <tr><td> double </td><td> longitude </td></tr>
 *     <tr><td> long long int </td><td> created_time </td></tr>
 *     <tr><td> long long int </td><td> last_modified_time </td></tr>
 *     <tr><td> long long int </td><td> completed_time </td></tr>
 *     <tr><td> integer </td><td> progress </td></tr>
 *     <tr><td> integer </td><td> freq </td></tr>
 *     <tr><td> integer </td><td> range_type </td></tr>
 *     <tr><td> calendar time </td><td> until_time </td></tr>
 *     <tr><td> integer </td><td> count </td></tr>
 *     <tr><td> integer </td><td> interval </td></tr>
 *     <tr><td> string </td><td> bysecond </td></tr>
 *     <tr><td> string </td><td> byminute </td></tr>
 *     <tr><td> string </td><td> byhour </td></tr>
 *     <tr><td> string </td><td> byday </td></tr>
 *     <tr><td> string </td><td> bymonthday </td></tr>
 *     <tr><td> string </td><td> byyearday </td></tr>
 *     <tr><td> string </td><td> byweekno </td></tr>
 *     <tr><td> string </td><td> bymonth </td></tr>
 *     <tr><td> string </td><td> bysetpos </td></tr>
 *     <tr><td> integer </td><td> wkst </td></tr>
 *     <tr><td> integer </td><td> has_alarm </td></tr>
 *     <tr><td> string </td><td> sync_data1 </td></tr>
 *     <tr><td> string </td><td> sync_data2 </td></tr>
 *     <tr><td> string </td><td> sync_data3 </td></tr>
 *     <tr><td> string </td><td> sync_data4 </td></tr>
 *     <tr><td> calendar time </td><td> start_time </td></tr>
 *     <tr><td> string </td><td> start_tzid </td></tr>
 *     <tr><td> calendar time </td><td> due_time </td></tr>
 *     <tr><td> string </td><td> due_tzid </td></tr>
 *     <tr><td> filter integer </td><td> calendar_book_visibility </td></tr>
 *     <tr><td> filter integer </td><td> calendar_book_account_id </td></tr>
 * </table>
 */
_CALENDAR_BEGIN_READ_ONLY_VIEW()
    _CALENDAR_PROPERTY_INT( todo_id )
    _CALENDAR_PROPERTY_INT( calendar_book_id )
    _CALENDAR_PROPERTY_STR( summary )
    _CALENDAR_PROPERTY_STR( description )
    _CALENDAR_PROPERTY_STR( location )
    _CALENDAR_PROPERTY_STR( categories )
    _CALENDAR_PROPERTY_INT( todo_status )
    _CALENDAR_PROPERTY_INT( priority )
    _CALENDAR_PROPERTY_INT( sensitivity )
    _CALENDAR_PROPERTY_STR( uid )
    _CALENDAR_PROPERTY_DOUBLE( latitude )
    _CALENDAR_PROPERTY_DOUBLE( longitude )
    _CALENDAR_PROPERTY_LLI( created_time )
    _CALENDAR_PROPERTY_LLI( last_modified_time )
    _CALENDAR_PROPERTY_LLI( completed_time )
    _CALENDAR_PROPERTY_INT( progress )
    _CALENDAR_PROPERTY_INT( freq )
    _CALENDAR_PROPERTY_INT( range_type )
    _CALENDAR_PROPERTY_CALTIME( until_time )
    _CALENDAR_PROPERTY_INT( count )
    _CALENDAR_PROPERTY_INT( interval )
    _CALENDAR_PROPERTY_STR( bysecond )
    _CALENDAR_PROPERTY_STR( byminute )
    _CALENDAR_PROPERTY_STR( byhour )
    _CALENDAR_PROPERTY_STR( byday )
    _CALENDAR_PROPERTY_STR( bymonthday )
    _CALENDAR_PROPERTY_STR( byyearday )
    _CALENDAR_PROPERTY_STR( byweekno )
    _CALENDAR_PROPERTY_STR( bymonth )
    _CALENDAR_PROPERTY_STR( bysetpos )
    _CALENDAR_PROPERTY_INT( wkst )
    _CALENDAR_PROPERTY_INT( has_alarm )
    _CALENDAR_PROPERTY_STR( sync_data1 )
    _CALENDAR_PROPERTY_STR( sync_data2 )
    _CALENDAR_PROPERTY_STR( sync_data3 )
    _CALENDAR_PROPERTY_STR( sync_data4 )
    _CALENDAR_PROPERTY_CALTIME( start_time )
    _CALENDAR_PROPERTY_STR( start_tzid )
    _CALENDAR_PROPERTY_CALTIME( due_time )
    _CALENDAR_PROPERTY_STR( due_tzid )
    _CALENDAR_PROPERTY_STR( organizer_name )
    _CALENDAR_PROPERTY_STR( organizer_email )
    _CALENDAR_PROPERTY_INT( has_attendee )
    _CALENDAR_PROPERTY_INT( is_allday )         // read only
    _CALENDAR_PROPERTY_FILTER_INT( calendar_book_visibility )
    _CALENDAR_PROPERTY_FILTER_INT( calendar_book_account_id )
_CALENDAR_END_READ_ONLY_VIEW( _calendar_todo_calendar_book )

/**
 * @ingroup CAPI_SOCIAL_CALENDAR_SVC_MODULE
 * @addtogroup CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE_calendar_event_calendar_book_attendee _calendar_event_calendar_book_attendee view (read only)
 * <table>
 *     <tr>
 *     <th> Type </th>
 *     <th> Property ID </th>
 *     </tr>
 *     <tr><td> string </td><td> _uri </td></tr>
 *     <tr><td> integer </td><td> event_id </td></tr>
 *     <tr><td> integer </td><td> calendar_book_id </td></tr>
 *     <tr><td> string </td><td> summary </td></tr>
 *     <tr><td> string </td><td> description </td></tr>
 *     <tr><td> string </td><td> location </td></tr>
 *     <tr><td> string </td><td> categories </td></tr>
 *     <tr><td> string </td><td> exdate </td></tr>
 *     <tr><td> integer </td><td> event_status </td></tr>
 *     <tr><td> integer </td><td> priority </td></tr>
 *     <tr><td> integer </td><td> timezone </td></tr>
 *     <tr><td> integer </td><td> person_id </td></tr>
 *     <tr><td> integer </td><td> busy_status </td></tr>
 *     <tr><td> integer </td><td> sensitivity </td></tr>
 *     <tr><td> string </td><td> uid </td></tr>
 *     <tr><td> string </td><td> organizer_name </td></tr>
 *     <tr><td> string </td><td> organizer_email </td></tr>
 *     <tr><td> integer </td><td> meeting_status </td></tr>
 *     <tr><td> integer </td><td> original_event_id </td></tr>
 *     <tr><td> double </td><td> latitude </td></tr>
 *     <tr><td> double </td><td> longitude </td></tr>
 *     <tr><td> integer </td><td> email_id </td></tr>
 *     <tr><td> long long int </td><td> created_time </td></tr>
 *     <tr><td> long long int </td><td> last_modified_time </td></tr>
 *     <tr><td> integer </td><td> freq </td></tr>
 *     <tr><td> integer </td><td> range_type </td></tr>
 *     <tr><td> calendar time </td><td> until_time </td></tr>
 *     <tr><td> integer </td><td> count </td></tr>
 *     <tr><td> integer </td><td> interval </td></tr>
 *     <tr><td> string </td><td> bysecond </td></tr>
 *     <tr><td> string </td><td> byminute </td></tr>
 *     <tr><td> string </td><td> byhour </td></tr>
 *     <tr><td> string </td><td> byday </td></tr>
 *     <tr><td> string </td><td> bymonthday </td></tr>
 *     <tr><td> string </td><td> byyearday </td></tr>
 *     <tr><td> string </td><td> byweekno </td></tr>
 *     <tr><td> string </td><td> bymonth </td></tr>
 *     <tr><td> string </td><td> bysetpos </td></tr>
 *     <tr><td> integer </td><td> wkst </td></tr>
 *     <tr><td> string </td><td> recurrence_id </td></tr>
 *     <tr><td> string </td><td> rdate </td></tr>
 *     <tr><td> integer </td><td> has_attendee </td></tr>
 *     <tr><td> integer </td><td> has_alarm </td></tr>
 *     <tr><td> integer </td><td> calendar_system_type </td></tr>
 *     <tr><td> string </td><td> sync_data1 </td></tr>
 *     <tr><td> string </td><td> sync_data2 </td></tr>
 *     <tr><td> string </td><td> sync_data3 </td></tr>
 *     <tr><td> string </td><td> sync_data4 </td></tr>
 *     <tr><td> calendar time </td><td> start_time </td></tr>
 *     <tr><td> string </td><td> start_tzid </td></tr>
 *     <tr><td> calendar time </td><td> end_time </td></tr>
 *     <tr><td> string </td><td> end_tzid </td></tr>
 *     <tr><td> filter integer </td><td> calendar_book_visibility </td></tr>
 *     <tr><td> filter integer </td><td> calendar_book_account_id </td></tr>
 *     <tr><td> filter string </td><td> attendee_email </td></tr>
 *     <tr><td> filter string </td><td> attendee_name </td></tr>
 *     <tr><td> filter string </td><td> attendee_member </td></tr>
 * </table>
 */
_CALENDAR_BEGIN_READ_ONLY_VIEW()
    _CALENDAR_PROPERTY_INT( event_id )
    _CALENDAR_PROPERTY_INT( calendar_book_id )
    _CALENDAR_PROPERTY_STR( summary )
    _CALENDAR_PROPERTY_STR( description )
    _CALENDAR_PROPERTY_STR( location )
    _CALENDAR_PROPERTY_STR( categories )
    _CALENDAR_PROPERTY_STR( exdate )
    _CALENDAR_PROPERTY_INT( event_status )
    _CALENDAR_PROPERTY_INT( priority )
    _CALENDAR_PROPERTY_INT( timezone )
    _CALENDAR_PROPERTY_INT( person_id )
    _CALENDAR_PROPERTY_INT( busy_status )
    _CALENDAR_PROPERTY_INT( sensitivity )
    _CALENDAR_PROPERTY_STR( uid )
    _CALENDAR_PROPERTY_STR( organizer_name )
    _CALENDAR_PROPERTY_STR( organizer_email )
    _CALENDAR_PROPERTY_INT( meeting_status )
    _CALENDAR_PROPERTY_INT( original_event_id )
    _CALENDAR_PROPERTY_DOUBLE( latitude )
    _CALENDAR_PROPERTY_DOUBLE( longitude )
    _CALENDAR_PROPERTY_INT( email_id )
    _CALENDAR_PROPERTY_LLI( created_time )
    _CALENDAR_PROPERTY_LLI( last_modified_time )
    _CALENDAR_PROPERTY_INT( freq )
    _CALENDAR_PROPERTY_INT( range_type )
    _CALENDAR_PROPERTY_CALTIME( until_time )
    _CALENDAR_PROPERTY_INT( count )
    _CALENDAR_PROPERTY_INT( interval )
    _CALENDAR_PROPERTY_STR( bysecond )
    _CALENDAR_PROPERTY_STR( byminute )
    _CALENDAR_PROPERTY_STR( byhour )
    _CALENDAR_PROPERTY_STR( byday )
    _CALENDAR_PROPERTY_STR( bymonthday )
    _CALENDAR_PROPERTY_STR( byyearday )
    _CALENDAR_PROPERTY_STR( byweekno )
    _CALENDAR_PROPERTY_STR( bymonth )
    _CALENDAR_PROPERTY_STR( bysetpos )
    _CALENDAR_PROPERTY_INT( wkst )
    _CALENDAR_PROPERTY_STR( recurrence_id )
    _CALENDAR_PROPERTY_STR( rdate )
    _CALENDAR_PROPERTY_INT( has_attendee )
    _CALENDAR_PROPERTY_INT( has_alarm )
    _CALENDAR_PROPERTY_INT( calendar_system_type )
    _CALENDAR_PROPERTY_STR( sync_data1 )
    _CALENDAR_PROPERTY_STR( sync_data2 )
    _CALENDAR_PROPERTY_STR( sync_data3 )
    _CALENDAR_PROPERTY_STR( sync_data4 )
    _CALENDAR_PROPERTY_CALTIME( start_time )
    _CALENDAR_PROPERTY_STR( start_tzid )
    _CALENDAR_PROPERTY_CALTIME( end_time )
    _CALENDAR_PROPERTY_STR( end_tzid )
    _CALENDAR_PROPERTY_INT( is_allday )         // read only
    _CALENDAR_PROPERTY_FILTER_INT( calendar_book_visibility )
    _CALENDAR_PROPERTY_FILTER_INT( calendar_book_account_id )
    _CALENDAR_PROPERTY_FILTER_STR( attendee_email )
    _CALENDAR_PROPERTY_FILTER_STR( attendee_name )
    _CALENDAR_PROPERTY_FILTER_STR( attendee_member )
_CALENDAR_END_READ_ONLY_VIEW( _calendar_event_calendar_book_attendee )

/**
 * @ingroup CAPI_SOCIAL_CALENDAR_SVC_MODULE
 * @addtogroup CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE_calendar_instance_utime_calendar_book _calendar_instance_utime_calendar_book view
 * <table>
 *     <tr>
 *     <th> Type </th>
 *     <th> Property ID </th>
 *     </tr>
 *     <tr><td> string </td><td> _uri </td></tr>
 *     <tr><td> integer </td><td> event_id </td></tr>
 *     <tr><td> calendar time </td><td> start_time </td></tr>
 *     <tr><td> calendar time </td><td> end_time </td></tr>
 *     <tr><td> string </td><td> summary </td></tr>
 *     <tr><td> string </td><td> location </td></tr>
 *     <tr><td> integer </td><td> calendar_book_id </td></tr>
 *     <tr><td> string </td><td> description </td></tr>
 *     <tr><td> integer </td><td> busy_status </td></tr>
 *     <tr><td> integer </td><td> event_status </td></tr>
 *     <tr><td> integer </td><td> priority </td></tr>
 *     <tr><td> integer </td><td> sensitivity </td></tr>
 *     <tr><td> integer </td><td> has_rrule </td></tr>
 *     <tr><td> double </td><td> latitude </td></tr>
 *     <tr><td> double </td><td> longitude </td></tr>
 *     <tr><td> integer </td><td> has_alarm </td></tr>
 *     <tr><td> integer </td><td> original_event_id </td></tr>
 *     <tr><td> filter integer </td><td> calendar_book_visibility </td></tr>
 *     <tr><td> filter integer </td><td> calendar_book_account_id </td></tr>
 *     <tr><td> long long int </td><td> last_modified_time </td></tr>
 *     <tr><td> string </td><td> sync_data1 </td></tr>
 * </table>
 */
_CALENDAR_BEGIN_READ_ONLY_VIEW()
    _CALENDAR_PROPERTY_INT( event_id )
    _CALENDAR_PROPERTY_CALTIME( start_time )
    _CALENDAR_PROPERTY_CALTIME( end_time )
    _CALENDAR_PROPERTY_STR( summary )
    _CALENDAR_PROPERTY_STR( location )
    _CALENDAR_PROPERTY_INT( calendar_book_id )
    _CALENDAR_PROPERTY_STR( description )
    _CALENDAR_PROPERTY_INT( busy_status )
    _CALENDAR_PROPERTY_INT( event_status )
    _CALENDAR_PROPERTY_INT( priority )
    _CALENDAR_PROPERTY_INT( sensitivity )
    _CALENDAR_PROPERTY_INT( has_rrule )
    _CALENDAR_PROPERTY_DOUBLE( latitude )
    _CALENDAR_PROPERTY_DOUBLE( longitude )
    _CALENDAR_PROPERTY_INT( has_alarm )
    _CALENDAR_PROPERTY_INT( original_event_id )
    _CALENDAR_PROPERTY_FILTER_INT( calendar_book_visibility )
    _CALENDAR_PROPERTY_FILTER_INT( calendar_book_account_id )
    _CALENDAR_PROPERTY_LLI( last_modified_time )
    _CALENDAR_PROPERTY_STR( sync_data1 )
_CALENDAR_END_READ_ONLY_VIEW( _calendar_instance_utime_calendar_book )

/**
 * @ingroup CAPI_SOCIAL_CALENDAR_SVC_MODULE
 * @addtogroup CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE_calendar_instance_localtime_calendar_book _calendar_instance_localtime_calendar_book view
 * <table>
 *     <tr>
 *     <th> Type </th>
 *     <th> Property ID </th>
 *     </tr>
 *     <tr><td> string </td><td> _uri </td></tr>
 *     <tr><td> integer </td><td> event_id </td></tr>
 *     <tr><td> calendar time </td><td> start_time </td></tr>
 *     <tr><td> calendar time </td><td> end_time </td></tr>
 *     <tr><td> string </td><td> summary </td></tr>
 *     <tr><td> string </td><td> location </td></tr>
 *     <tr><td> integer </td><td> calendar_book_id </td></tr>
 *     <tr><td> string </td><td> description </td></tr>
 *     <tr><td> integer </td><td> busy_status </td></tr>
 *     <tr><td> integer </td><td> event_status </td></tr>
 *     <tr><td> integer </td><td> priority </td></tr>
 *     <tr><td> integer </td><td> sensitivity </td></tr>
 *     <tr><td> integer </td><td> has_rrule </td></tr>
 *     <tr><td> double </td><td> latitude </td></tr>
 *     <tr><td> double </td><td> longitude </td></tr>
 *     <tr><td> integer </td><td> has_alarm </td></tr>
 *     <tr><td> integer </td><td> original_event_id </td></tr>
 *     <tr><td> filter integer </td><td> calendar_book_visibility </td></tr>
 *     <tr><td> filter integer </td><td> calendar_book_account_id </td></tr>
 *     <tr><td> long long int </td><td> last_modified_time </td></tr>
 *     <tr><td> string </td><td> sync_data1 </td></tr>
 *     <tr><td> int </td><td> is_allday </td></tr>
 * </table>
 */
_CALENDAR_BEGIN_READ_ONLY_VIEW()
    _CALENDAR_PROPERTY_INT( event_id )
    _CALENDAR_PROPERTY_CALTIME( start_time )
    _CALENDAR_PROPERTY_CALTIME( end_time )
    _CALENDAR_PROPERTY_STR( summary )
    _CALENDAR_PROPERTY_STR( location )
    _CALENDAR_PROPERTY_INT( calendar_book_id )
    _CALENDAR_PROPERTY_STR( description )
    _CALENDAR_PROPERTY_INT( busy_status )
    _CALENDAR_PROPERTY_INT( event_status )
    _CALENDAR_PROPERTY_INT( priority )
    _CALENDAR_PROPERTY_INT( sensitivity )
    _CALENDAR_PROPERTY_INT( has_rrule )
    _CALENDAR_PROPERTY_DOUBLE( latitude )
    _CALENDAR_PROPERTY_DOUBLE( longitude )
    _CALENDAR_PROPERTY_INT( has_alarm )
    _CALENDAR_PROPERTY_INT( original_event_id )
    _CALENDAR_PROPERTY_FILTER_INT( calendar_book_visibility )
    _CALENDAR_PROPERTY_FILTER_INT( calendar_book_account_id )
    _CALENDAR_PROPERTY_LLI( last_modified_time )
    _CALENDAR_PROPERTY_STR( sync_data1 )
    _CALENDAR_PROPERTY_INT( is_allday )
_CALENDAR_END_READ_ONLY_VIEW( _calendar_instance_localtime_calendar_book )

/**
 * @ingroup CAPI_SOCIAL_CALENDAR_SVC_MODULE
 * @addtogroup CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE_calendar_instance_utime_calendar_book_extended _calendar_instance_utime_calendar_book_extended view
 * <table>
 *     <tr>
 *     <th> Type </th>
 *     <th> Property ID </th>
 *     </tr>
 *     <tr><td> string </td><td> _uri </td></tr>
 *     <tr><td> integer </td><td> event_id </td></tr>
 *     <tr><td> calendar time </td><td> start_time </td></tr>
 *     <tr><td> calendar time </td><td> end_time </td></tr>
 *     <tr><td> string </td><td> summary </td></tr>
 *     <tr><td> string </td><td> location </td></tr>
 *     <tr><td> integer </td><td> calendar_book_id </td></tr>
 *     <tr><td> string </td><td> description </td></tr>
 *     <tr><td> integer </td><td> busy_status </td></tr>
 *     <tr><td> integer </td><td> event_status </td></tr>
 *     <tr><td> integer </td><td> priority </td></tr>
 *     <tr><td> integer </td><td> sensitivity </td></tr>
 *     <tr><td> integer </td><td> has_rrule </td></tr>
 *     <tr><td> double </td><td> latitude </td></tr>
 *     <tr><td> double </td><td> longitude </td></tr>
 *     <tr><td> integer </td><td> has_alarm </td></tr>
 *     <tr><td> integer </td><td> original_event_id </td></tr>
 *     <tr><td> filter integer </td><td> calendar_book_visibility </td></tr>
 *     <tr><td> filter integer </td><td> calendar_book_account_id </td></tr>
 *     <tr><td> string </td><td> organizer_name </td></tr>
 *     <tr><td> string </td><td> categories </td></tr>
 *     <tr><td> integer </td><td> has_attendee </td></tr>
 *     <tr><td> string </td><td> sync_data1 </td></tr>
 *     <tr><td> string </td><td> sync_data2 </td></tr>
 *     <tr><td> string </td><td> sync_data3 </td></tr>
 *     <tr><td> string </td><td> sync_data4 </td></tr>
 * </table>
 */
_CALENDAR_BEGIN_READ_ONLY_VIEW()
    _CALENDAR_PROPERTY_INT( event_id )
    _CALENDAR_PROPERTY_CALTIME( start_time )
    _CALENDAR_PROPERTY_CALTIME( end_time )
    _CALENDAR_PROPERTY_STR( summary )
    _CALENDAR_PROPERTY_STR( location )
    _CALENDAR_PROPERTY_INT( calendar_book_id )
    _CALENDAR_PROPERTY_STR( description )
    _CALENDAR_PROPERTY_INT( busy_status )
    _CALENDAR_PROPERTY_INT( event_status )
    _CALENDAR_PROPERTY_INT( priority )
    _CALENDAR_PROPERTY_INT( sensitivity )
    _CALENDAR_PROPERTY_INT( has_rrule )
    _CALENDAR_PROPERTY_DOUBLE( latitude )
    _CALENDAR_PROPERTY_DOUBLE( longitude )
    _CALENDAR_PROPERTY_INT( has_alarm )
    _CALENDAR_PROPERTY_INT( original_event_id )
    _CALENDAR_PROPERTY_FILTER_INT( calendar_book_visibility )
    _CALENDAR_PROPERTY_FILTER_INT( calendar_book_account_id )
    _CALENDAR_PROPERTY_LLI( last_modified_time )
    _CALENDAR_PROPERTY_STR( sync_data1 )
    _CALENDAR_PROPERTY_STR( organizer_name )
    _CALENDAR_PROPERTY_STR( categories )
    _CALENDAR_PROPERTY_INT( has_attendee )
    _CALENDAR_PROPERTY_STR( sync_data2 )
    _CALENDAR_PROPERTY_STR( sync_data3 )
    _CALENDAR_PROPERTY_STR( sync_data4 )
_CALENDAR_END_READ_ONLY_VIEW( _calendar_instance_utime_calendar_book_extended )

/**
 * @ingroup CAPI_SOCIAL_CALENDAR_SVC_MODULE
 * @addtogroup CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE_calendar_instance_localtime_calendar_book_extended _calendar_instance_localtime_calendar_book_extended view
 * <table>
 *     <tr>
 *     <th> Type </th>
 *     <th> Property ID </th>
 *     </tr>
 *     <tr><td> string </td><td> _uri </td></tr>
 *     <tr><td> integer </td><td> event_id </td></tr>
 *     <tr><td> calendar time </td><td> start_time </td></tr>
 *     <tr><td> calendar time </td><td> end_time </td></tr>
 *     <tr><td> string </td><td> summary </td></tr>
 *     <tr><td> string </td><td> location </td></tr>
 *     <tr><td> integer </td><td> calendar_book_id </td></tr>
 *     <tr><td> string </td><td> description </td></tr>
 *     <tr><td> integer </td><td> busy_status </td></tr>
 *     <tr><td> integer </td><td> event_status </td></tr>
 *     <tr><td> integer </td><td> priority </td></tr>
 *     <tr><td> integer </td><td> sensitivity </td></tr>
 *     <tr><td> integer </td><td> has_rrule </td></tr>
 *     <tr><td> double </td><td> latitude </td></tr>
 *     <tr><td> double </td><td> longitude </td></tr>
 *     <tr><td> integer </td><td> has_alarm </td></tr>
 *     <tr><td> integer </td><td> original_event_id </td></tr>
 *     <tr><td> filter integer </td><td> calendar_book_visibility </td></tr>
 *     <tr><td> filter integer </td><td> calendar_book_account_id </td></tr>
 *     <tr><td> string </td><td> organizer_name </td></tr>
 *     <tr><td> string </td><td> categories </td></tr>
 *     <tr><td> integer </td><td> has_attendee </td></tr>
 *     <tr><td> string </td><td> sync_data1 </td></tr>
 *     <tr><td> string </td><td> sync_data2 </td></tr>
 *     <tr><td> string </td><td> sync_data3 </td></tr>
 *     <tr><td> string </td><td> sync_data4 </td></tr>
 * </table>
 */
_CALENDAR_BEGIN_READ_ONLY_VIEW()
    _CALENDAR_PROPERTY_INT( event_id )
    _CALENDAR_PROPERTY_CALTIME( start_time )
    _CALENDAR_PROPERTY_CALTIME( end_time )
    _CALENDAR_PROPERTY_STR( summary )
    _CALENDAR_PROPERTY_STR( location )
    _CALENDAR_PROPERTY_INT( calendar_book_id )
    _CALENDAR_PROPERTY_STR( description )
    _CALENDAR_PROPERTY_INT( busy_status )
    _CALENDAR_PROPERTY_INT( event_status )
    _CALENDAR_PROPERTY_INT( priority )
    _CALENDAR_PROPERTY_INT( sensitivity )
    _CALENDAR_PROPERTY_INT( has_rrule )
    _CALENDAR_PROPERTY_DOUBLE( latitude )
    _CALENDAR_PROPERTY_DOUBLE( longitude )
    _CALENDAR_PROPERTY_INT( has_alarm )
    _CALENDAR_PROPERTY_INT( original_event_id )
    _CALENDAR_PROPERTY_FILTER_INT( calendar_book_visibility )
    _CALENDAR_PROPERTY_FILTER_INT( calendar_book_account_id )
    _CALENDAR_PROPERTY_LLI( last_modified_time )
    _CALENDAR_PROPERTY_STR( sync_data1 )
    _CALENDAR_PROPERTY_STR( organizer_name )
    _CALENDAR_PROPERTY_STR( categories )
    _CALENDAR_PROPERTY_INT( has_attendee )
    _CALENDAR_PROPERTY_INT( is_allday )
    _CALENDAR_PROPERTY_STR( sync_data2 )
    _CALENDAR_PROPERTY_STR( sync_data3 )
    _CALENDAR_PROPERTY_STR( sync_data4 )
_CALENDAR_END_READ_ONLY_VIEW( _calendar_instance_localtime_calendar_book_extended )

/**
 * @ingroup CAPI_SOCIAL_CALENDAR_SVC_MODULE
 * @addtogroup CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CALENDAR_SVC_VIEW_MODULE_calendar_extended_property _calendar_extended_property view (read only)
 * <table>
 *     <tr>
 *     <th> Type </th>
 *     <th> Property ID </th>
 *     <th> Read, Write </th>
 *     <th> Description </th>
 *     </tr>
 *     <tr><td> string </td><td> _uri </td><td> read only </td><td></td> Identifier of this extended_property view </tr>
 *     <tr><td> integer </td><td> id </td><td> read only </td><td> DB record ID of the extended_property </td></tr>
 *     <tr><td> integer </td><td> record_id </td><td> read,write </td><td> Related record ID </td></tr>
 *     <tr><td> integer </td><td> record_type </td><td> read, write </td><td> Enumeration value of the record type (@ref calendar_record_type_e) </td></tr>
 *     <tr><td> string </td><td> key </td><td> read, write </td><td> The key of the property </td></tr>
 *     <tr><td> string </td><td> value </td><td> read, write </td><td> The value of the property </td></tr>
 * </table>
 */
_CALENDAR_BEGIN_VIEW()
    _CALENDAR_PROPERTY_INT( id ) // read_only
    _CALENDAR_PROPERTY_INT( record_id )
    _CALENDAR_PROPERTY_INT( record_type )
    _CALENDAR_PROPERTY_STR( key )
    _CALENDAR_PROPERTY_STR( value )
_CALENDAR_END_VIEW( _calendar_extended_property )

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_SOCIAL_CALENDAR_VIEW_H__ */
