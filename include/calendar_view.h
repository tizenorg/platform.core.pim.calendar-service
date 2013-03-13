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

#include <calendar_types2.h>

#ifdef __cplusplus
extern "C" {
#endif

_CALENDAR_BEGIN_VIEW()
	_CALENDAR_PROPERTY_INT( id )            // read_only
	_CALENDAR_PROPERTY_STR( uid )
	_CALENDAR_PROPERTY_STR( name )
	_CALENDAR_PROPERTY_STR( description )
	_CALENDAR_PROPERTY_STR( color )
	_CALENDAR_PROPERTY_STR( location )
	_CALENDAR_PROPERTY_INT( visibility )
	_CALENDAR_PROPERTY_INT( sync_event )
	_CALENDAR_PROPERTY_INT( is_deleted )
	_CALENDAR_PROPERTY_INT( account_id )
	_CALENDAR_PROPERTY_INT( store_type )
	_CALENDAR_PROPERTY_STR( sync_data1 )
	_CALENDAR_PROPERTY_STR( sync_data2 )
	_CALENDAR_PROPERTY_STR( sync_data3 )
	_CALENDAR_PROPERTY_STR( sync_data4 )
_CALENDAR_END_VIEW( _calendar_book )

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
	_CALENDAR_PROPERTY_LLI( last_modified_time )
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
	_CALENDAR_PROPERTY_LLI( last_modified_time )
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

_CALENDAR_BEGIN_VIEW()
    _CALENDAR_PROPERTY_INT( event_id )
	_CALENDAR_PROPERTY_STR( number )
	_CALENDAR_PROPERTY_INT( type )
	_CALENDAR_PROPERTY_INT( person_id )
	_CALENDAR_PROPERTY_STR( uid )
	_CALENDAR_PROPERTY_STR( group )
	_CALENDAR_PROPERTY_STR( email )
	_CALENDAR_PROPERTY_INT( role )
	_CALENDAR_PROPERTY_INT( status )
	_CALENDAR_PROPERTY_INT( rsvp )
	_CALENDAR_PROPERTY_STR( delegate_uri )
	_CALENDAR_PROPERTY_STR( delegator_uri )
	_CALENDAR_PROPERTY_STR( name )
_CALENDAR_END_VIEW( _calendar_attendee )

_CALENDAR_BEGIN_VIEW()
    _CALENDAR_PROPERTY_INT( event_id )
    _CALENDAR_PROPERTY_INT( todo_id )
	_CALENDAR_PROPERTY_INT( type )
	_CALENDAR_PROPERTY_LLI( time )
	_CALENDAR_PROPERTY_INT( tick )
	_CALENDAR_PROPERTY_INT( tick_unit )
	_CALENDAR_PROPERTY_STR( tone )
	_CALENDAR_PROPERTY_STR( description )
	_CALENDAR_PROPERTY_INT( alarm_id )
_CALENDAR_END_VIEW( _calendar_alarm )

_CALENDAR_BEGIN_READ_ONLY_VIEW()
	_CALENDAR_PROPERTY_INT( id )
	_CALENDAR_PROPERTY_INT( calendar_book_id )
	_CALENDAR_PROPERTY_INT( modified_status )
	_CALENDAR_PROPERTY_INT( version )
_CALENDAR_END_READ_ONLY_VIEW( _calendar_updated_info )

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
    _CALENDAR_PROPERTY_INT( is_deleted )
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
    _CALENDAR_PROPERTY_INT( is_deleted )
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
    _CALENDAR_PROPERTY_INT( is_deleted )
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
_CALENDAR_END_READ_ONLY_VIEW( _calendar_event_calendar_book_attendee )

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
_CALENDAR_END_READ_ONLY_VIEW( _calendar_instance_normal_calendar_book )

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
_CALENDAR_END_READ_ONLY_VIEW( _calendar_instance_allday_calendar_book )

_CALENDAR_BEGIN_VIEW()
    _CALENDAR_PROPERTY_INT( id )
    _CALENDAR_PROPERTY_INT( record_id )
    _CALENDAR_PROPERTY_INT( record_type )
    _CALENDAR_PROPERTY_STR( key )
    _CALENDAR_PROPERTY_STR( value )
_CALENDAR_END_VIEW( _calendar_extended_property )

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_SOCIAL_CALENDAR_VIEW_H__ */