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

#include <glib.h>
#include <stdlib.h>

#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_view.h"
#include "cal_mutex.h"

// !! TODO : please check ids number
API const _calendar_book_property_ids   _calendar_book =
{
	._uri = CALENDAR_VIEW_CALENDAR,
	.id = CAL_PROPERTY_CALENDAR_ID,
	.uid = CAL_PROPERTY_CALENDAR_UID,
	.name = CAL_PROPERTY_CALENDAR_NAME,
	.description = CAL_PROPERTY_CALENDAR_DESCRIPTION,
	.color = CAL_PROPERTY_CALENDAR_COLOR,
	.location = CAL_PROPERTY_CALENDAR_LOCATION,
	.visibility = CAL_PROPERTY_CALENDAR_VISIBILITY,
	.sync_event = CAL_PROPERTY_CALENDAR_SYNC_EVENT,
	.account_id = CAL_PROPERTY_CALENDAR_ACCOUNT_ID,
	.store_type = CAL_PROPERTY_CALENDAR_STORE_TYPE,
	.sync_data1 = CAL_PROPERTY_CALENDAR_SYNC_DATA1,
	.sync_data2 = CAL_PROPERTY_CALENDAR_SYNC_DATA2,
	.sync_data3 = CAL_PROPERTY_CALENDAR_SYNC_DATA3,
	.sync_data4 = CAL_PROPERTY_CALENDAR_SYNC_DATA4,
	.mode = CAL_PROPERTY_CALENDAR_MODE
};

API const _calendar_event_property_ids   _calendar_event =
{
	._uri = CALENDAR_VIEW_EVENT,
	.id = CAL_PROPERTY_EVENT_ID,
	.calendar_book_id = CAL_PROPERTY_EVENT_CALENDAR_ID,
	.summary = CAL_PROPERTY_EVENT_SUMMARY,
	.description = CAL_PROPERTY_EVENT_DESCRIPTION,
	.location = CAL_PROPERTY_EVENT_LOCATION,
	.categories = CAL_PROPERTY_EVENT_CATEGORIES,
	.exdate = CAL_PROPERTY_EVENT_EXDATE,
	.event_status = CAL_PROPERTY_EVENT_EVENT_STATUS,
	.priority = CAL_PROPERTY_EVENT_PRIORITY,
	.timezone = CAL_PROPERTY_EVENT_TIMEZONE,
	.person_id = CAL_PROPERTY_EVENT_CONTACT_ID,
	.busy_status = CAL_PROPERTY_EVENT_BUSY_STATUS,
	.sensitivity = CAL_PROPERTY_EVENT_SENSITIVITY,
	.uid = CAL_PROPERTY_EVENT_UID,
	.organizer_name = CAL_PROPERTY_EVENT_ORGANIZER_NAME,
	.organizer_email = CAL_PROPERTY_EVENT_ORGANIZER_EMAIL,
	.meeting_status = CAL_PROPERTY_EVENT_MEETING_STATUS,
	.original_event_id = CAL_PROPERTY_EVENT_ORIGINAL_EVENT_ID,
	.latitude = CAL_PROPERTY_EVENT_LATITUDE,
	.longitude = CAL_PROPERTY_EVENT_LONGITUDE,
	.email_id = CAL_PROPERTY_EVENT_EMAIL_ID,
	.created_time = CAL_PROPERTY_EVENT_CREATED_TIME,
	.last_modified_time = CAL_PROPERTY_EVENT_LAST_MODIFIED_TIME,
	.is_deleted = CAL_PROPERTY_EVENT_IS_DELETED,
	.freq = CAL_PROPERTY_EVENT_FREQ,
	.range_type = CAL_PROPERTY_EVENT_RANGE_TYPE,
	.until_time = CAL_PROPERTY_EVENT_UNTIL,
	.count = CAL_PROPERTY_EVENT_COUNT,
	.interval = CAL_PROPERTY_EVENT_INTERVAL,
	.bysecond = CAL_PROPERTY_EVENT_BYSECOND,
	.byminute = CAL_PROPERTY_EVENT_BYMINUTE,
	.byhour = CAL_PROPERTY_EVENT_BYHOUR,
	.byday = CAL_PROPERTY_EVENT_BYDAY,
	.bymonthday = CAL_PROPERTY_EVENT_BYMONTHDAY,
	.byyearday = CAL_PROPERTY_EVENT_BYYEARDAY,
	.byweekno = CAL_PROPERTY_EVENT_BYWEEKNO,
	.bymonth = CAL_PROPERTY_EVENT_BYMONTH,
	.bysetpos = CAL_PROPERTY_EVENT_BYSETPOS,
	.wkst = CAL_PROPERTY_EVENT_WKST,
	.recurrence_id = CAL_PROPERTY_EVENT_RECURRENCE_ID,
	.rdate = CAL_PROPERTY_EVENT_RDATE,
	.has_attendee = CAL_PROPERTY_EVENT_HAS_ATTENDEE,
	.has_alarm = CAL_PROPERTY_EVENT_HAS_ALARM,
	.calendar_system_type = CAL_PROPERTY_EVENT_CALENDAR_SYSTEM_TYPE,
	.sync_data1 = CAL_PROPERTY_EVENT_SYNC_DATA1,
	.sync_data2 = CAL_PROPERTY_EVENT_SYNC_DATA2,
	.sync_data3 = CAL_PROPERTY_EVENT_SYNC_DATA3,
	.sync_data4 = CAL_PROPERTY_EVENT_SYNC_DATA4,
	.start_time = CAL_PROPERTY_EVENT_START,
	.start_tzid = CAL_PROPERTY_EVENT_START_TZID,
	.end_time = CAL_PROPERTY_EVENT_END,
	.end_tzid = CAL_PROPERTY_EVENT_END_TZID,
	.calendar_alarm = CAL_PROPERTY_EVENT_CALENDAR_ALARM,
	.calendar_attendee = CAL_PROPERTY_EVENT_CALENDAR_ATTENDEE,
	.exception = CAL_PROPERTY_EVENT_EXCEPTION,
	.extended = CAL_PROPERTY_EVENT_EXTENDED,
	.is_allday = CAL_PROPERTY_EVENT_IS_ALLDAY
};

API const _calendar_todo_property_ids   _calendar_todo =
{
	._uri = CALENDAR_VIEW_TODO,
	.id = CAL_PROPERTY_TODO_ID,
	.calendar_book_id = CAL_PROPERTY_TODO_CALENDAR_ID,
	.summary = CAL_PROPERTY_TODO_SUMMARY,
	.description = CAL_PROPERTY_TODO_DESCRIPTION,
	.location = CAL_PROPERTY_TODO_LOCATION,
	.categories = CAL_PROPERTY_TODO_CATEGORIES,
	.todo_status = CAL_PROPERTY_TODO_TODO_STATUS,
	.priority = CAL_PROPERTY_TODO_PRIORITY,
	.sensitivity = CAL_PROPERTY_TODO_SENSITIVITY,
	.uid = CAL_PROPERTY_TODO_UID,
	.latitude = CAL_PROPERTY_TODO_LATITUDE,
	.longitude = CAL_PROPERTY_TODO_LONGITUDE,
	.created_time = CAL_PROPERTY_TODO_CREATED_TIME,
	.last_modified_time = CAL_PROPERTY_TODO_LAST_MODIFIED_TIME,
	.completed_time = CAL_PROPERTY_TODO_COMPLETED_TIME,
	.progress = CAL_PROPERTY_TODO_PROGRESS,
	.is_deleted = CAL_PROPERTY_TODO_IS_DELETED,
	.freq = CAL_PROPERTY_TODO_FREQ,
	.range_type = CAL_PROPERTY_TODO_RANGE_TYPE,
	.until_time = CAL_PROPERTY_TODO_UNTIL,
	.count = CAL_PROPERTY_TODO_COUNT,
	.interval = CAL_PROPERTY_TODO_INTERVAL,
	.bysecond = CAL_PROPERTY_TODO_BYSECOND,
	.byminute = CAL_PROPERTY_TODO_BYMINUTE,
	.byhour = CAL_PROPERTY_TODO_BYHOUR,
	.byday = CAL_PROPERTY_TODO_BYDAY,
	.bymonthday = CAL_PROPERTY_TODO_BYMONTHDAY,
	.byyearday = CAL_PROPERTY_TODO_BYYEARDAY,
	.byweekno = CAL_PROPERTY_TODO_BYWEEKNO,
	.bymonth = CAL_PROPERTY_TODO_BYMONTH,
	.bysetpos = CAL_PROPERTY_TODO_BYSETPOS,
	.wkst = CAL_PROPERTY_TODO_WKST,
	.has_alarm = CAL_PROPERTY_TODO_HAS_ALARM,
	.sync_data1 = CAL_PROPERTY_TODO_SYNC_DATA1,
	.sync_data2 = CAL_PROPERTY_TODO_SYNC_DATA2,
	.sync_data3 = CAL_PROPERTY_TODO_SYNC_DATA3,
	.sync_data4 = CAL_PROPERTY_TODO_SYNC_DATA4,
	.start_time = CAL_PROPERTY_TODO_START,
	.start_tzid = CAL_PROPERTY_TODO_START_TZID,
	.due_time = CAL_PROPERTY_TODO_DUE,
	.due_tzid = CAL_PROPERTY_TODO_DUE_TZID,
	.calendar_alarm = CAL_PROPERTY_TODO_CALENDAR_ALARM,
	.organizer_name = CAL_PROPERTY_TODO_ORGANIZER_NAME,
	.organizer_email = CAL_PROPERTY_TODO_ORGANIZER_EMAIL,
	.has_attendee = CAL_PROPERTY_TODO_HAS_ATTENDEE,
	.calendar_attendee = CAL_PROPERTY_TODO_CALENDAR_ATTENDEE,
	.extended = CAL_PROPERTY_TODO_EXTENDED,
	.is_allday = CAL_PROPERTY_TODO_IS_ALLDAY
};

API const _calendar_timezone_property_ids   _calendar_timezone =
{
	._uri = CALENDAR_VIEW_TIMEZONE,
	.id = CAL_PROPERTY_TIMEZONE_ID,
	.calendar_book_id = CAL_PROPERTY_TIMEZONE_CALENDAR_ID,
	.tz_offset_from_gmt = CAL_PROPERTY_TIMEZONE_TZ_OFFSET_FROM_GMT,
	.standard_name = CAL_PROPERTY_TIMEZONE_STANDARD_NAME,
	.standard_start_month = CAL_PROPERTY_TIMEZONE_STD_START_MONTH,
	.standard_start_position_of_week = CAL_PROPERTY_TIMEZONE_STD_START_POSITION_OF_WEEK,
	.standard_start_day = CAL_PROPERTY_TIMEZONE_STD_START_DAY,
	.standard_start_hour = CAL_PROPERTY_TIMEZONE_STD_START_HOUR,
	.standard_bias = CAL_PROPERTY_TIMEZONE_STANDARD_BIAS,
	.day_light_name = CAL_PROPERTY_TIMEZONE_DAY_LIGHT_NAME,
	.day_light_start_month = CAL_PROPERTY_TIMEZONE_DAY_LIGHT_START_MONTH,
	.day_light_start_position_of_week = CAL_PROPERTY_TIMEZONE_DAY_LIGHT_START_POSITION_OF_WEEK,
	.day_light_start_day = CAL_PROPERTY_TIMEZONE_DAY_LIGHT_START_DAY,
	.day_light_start_hour = CAL_PROPERTY_TIMEZONE_DAY_LIGHT_START_HOUR,
	.day_light_bias = CAL_PROPERTY_TIMEZONE_DAY_LIGHT_BIAS
};

API const _calendar_attendee_property_ids   _calendar_attendee =
{
	._uri = CALENDAR_VIEW_ATTENDEE,
	.parent_id = CAL_PROPERTY_ATTENDEE_PARENT_ID,
	.number = CAL_PROPERTY_ATTENDEE_NUMBER,
	.cutype = CAL_PROPERTY_ATTENDEE_CUTYPE,
	.person_id = CAL_PROPERTY_ATTENDEE_CT_INDEX,
	.uid = CAL_PROPERTY_ATTENDEE_UID,
	.group = CAL_PROPERTY_ATTENDEE_GROUP,
	.email = CAL_PROPERTY_ATTENDEE_EMAIL,
	.role = CAL_PROPERTY_ATTENDEE_ROLE,
	.status = CAL_PROPERTY_ATTENDEE_STATUS,
	.rsvp = CAL_PROPERTY_ATTENDEE_RSVP,
	.delegatee_uri = CAL_PROPERTY_ATTENDEE_DELEGATEE_URI,
	.delegator_uri = CAL_PROPERTY_ATTENDEE_DELEGATOR_URI,
	.name = CAL_PROPERTY_ATTENDEE_NAME,
	.member = CAL_PROPERTY_ATTENDEE_MEMBER
};

API const _calendar_alarm_property_ids   _calendar_alarm =
{
	._uri = CALENDAR_VIEW_ALARM,
	.parent_id = CAL_PROPERTY_ALARM_PARENT_ID,
	.tick = CAL_PROPERTY_ALARM_TICK,
	.tick_unit = CAL_PROPERTY_ALARM_TICK_UNIT,
	.description = CAL_PROPERTY_ALARM_DESCRIPTION,
	.summary = CAL_PROPERTY_ALARM_SUMMARY,
	.action = CAL_PROPERTY_ALARM_ACTION,
	.attach = CAL_PROPERTY_ALARM_ATTACH,
	.alarm_time = CAL_PROPERTY_ALARM_ALARM,
};

API const _calendar_updated_info_property_ids   _calendar_updated_info =
{
	._uri = CALENDAR_VIEW_UPDATED_INFO,
	.id = CAL_PROPERTY_UPDATED_INFO_ID,
	.calendar_book_id = CAL_PROPERTY_UPDATED_INFO_CALENDAR_ID,
	.modified_status = CAL_PROPERTY_UPDATED_INFO_TYPE,
	.version = CAL_PROPERTY_UPDATED_INFO_VERSION
};

API const _calendar_event_calendar_book_property_ids   _calendar_event_calendar_book =
{
	._uri = CALENDAR_VIEW_EVENT_CALENDAR,
	.event_id = CAL_PROPERTY_EVENT_ID,
	.calendar_book_id = CAL_PROPERTY_EVENT_CALENDAR_ID,
	.summary = CAL_PROPERTY_EVENT_SUMMARY,
	.description = CAL_PROPERTY_EVENT_DESCRIPTION,
	.location = CAL_PROPERTY_EVENT_LOCATION,
	.categories = CAL_PROPERTY_EVENT_CATEGORIES,
	.exdate = CAL_PROPERTY_EVENT_EXDATE,
	.event_status = CAL_PROPERTY_EVENT_EVENT_STATUS,
	.priority = CAL_PROPERTY_EVENT_PRIORITY,
	.timezone = CAL_PROPERTY_EVENT_TIMEZONE,
	.person_id = CAL_PROPERTY_EVENT_CONTACT_ID,
	.busy_status = CAL_PROPERTY_EVENT_BUSY_STATUS,
	.sensitivity = CAL_PROPERTY_EVENT_SENSITIVITY,
	.uid = CAL_PROPERTY_EVENT_UID,
	.organizer_name = CAL_PROPERTY_EVENT_ORGANIZER_NAME,
	.organizer_email = CAL_PROPERTY_EVENT_ORGANIZER_EMAIL,
	.meeting_status = CAL_PROPERTY_EVENT_MEETING_STATUS,
	.original_event_id = CAL_PROPERTY_EVENT_ORIGINAL_EVENT_ID,
	.latitude = CAL_PROPERTY_EVENT_LATITUDE,
	.longitude = CAL_PROPERTY_EVENT_LONGITUDE,
	.email_id = CAL_PROPERTY_EVENT_EMAIL_ID,
	.created_time = CAL_PROPERTY_EVENT_CREATED_TIME,
	.last_modified_time = CAL_PROPERTY_EVENT_LAST_MODIFIED_TIME,
	.freq = CAL_PROPERTY_EVENT_FREQ,
	.range_type = CAL_PROPERTY_EVENT_RANGE_TYPE,
	.until_time = CAL_PROPERTY_EVENT_UNTIL,
	.count = CAL_PROPERTY_EVENT_COUNT,
	.interval = CAL_PROPERTY_EVENT_INTERVAL,
	.bysecond = CAL_PROPERTY_EVENT_BYSECOND,
	.byminute = CAL_PROPERTY_EVENT_BYMINUTE,
	.byhour = CAL_PROPERTY_EVENT_BYHOUR,
	.byday = CAL_PROPERTY_EVENT_BYDAY,
	.bymonthday = CAL_PROPERTY_EVENT_BYMONTHDAY,
	.byyearday = CAL_PROPERTY_EVENT_BYYEARDAY,
	.byweekno = CAL_PROPERTY_EVENT_BYWEEKNO,
	.bymonth = CAL_PROPERTY_EVENT_BYMONTH,
	.bysetpos = CAL_PROPERTY_EVENT_BYSETPOS,
	.wkst = CAL_PROPERTY_EVENT_WKST,
	.recurrence_id = CAL_PROPERTY_EVENT_RECURRENCE_ID,
	.rdate = CAL_PROPERTY_EVENT_RDATE,
	.has_attendee = CAL_PROPERTY_EVENT_HAS_ATTENDEE,
	.has_alarm = CAL_PROPERTY_EVENT_HAS_ALARM,
	.calendar_system_type = CAL_PROPERTY_EVENT_CALENDAR_SYSTEM_TYPE,
	.sync_data1 = CAL_PROPERTY_EVENT_SYNC_DATA1,
	.sync_data2 = CAL_PROPERTY_EVENT_SYNC_DATA2,
	.sync_data3 = CAL_PROPERTY_EVENT_SYNC_DATA3,
	.sync_data4 = CAL_PROPERTY_EVENT_SYNC_DATA4,
	.start_time = CAL_PROPERTY_EVENT_START,
	.start_tzid = CAL_PROPERTY_EVENT_START_TZID,
	.end_time = CAL_PROPERTY_EVENT_END,
	.end_tzid = CAL_PROPERTY_EVENT_END_TZID,
	.is_allday = CAL_PROPERTY_EVENT_IS_ALLDAY,
	.calendar_book_visibility = CAL_PROPERTY_CALENDAR_VISIBILITY | CAL_PROPERTY_FLAGS_FILTER,
	.calendar_book_account_id = CAL_PROPERTY_CALENDAR_ACCOUNT_ID | CAL_PROPERTY_FLAGS_FILTER
};

API const _calendar_todo_calendar_book_property_ids   _calendar_todo_calendar_book =
{
	._uri = CALENDAR_VIEW_TODO_CALENDAR,
	.todo_id = CAL_PROPERTY_TODO_ID,
	.calendar_book_id = CAL_PROPERTY_TODO_CALENDAR_ID,
	.summary = CAL_PROPERTY_TODO_SUMMARY,
	.description = CAL_PROPERTY_TODO_DESCRIPTION,
	.location = CAL_PROPERTY_TODO_LOCATION,
	.categories = CAL_PROPERTY_TODO_CATEGORIES,
	.todo_status = CAL_PROPERTY_TODO_TODO_STATUS,
	.priority = CAL_PROPERTY_TODO_PRIORITY,
	.sensitivity = CAL_PROPERTY_TODO_SENSITIVITY,
	.uid = CAL_PROPERTY_TODO_UID,
	.latitude = CAL_PROPERTY_TODO_LATITUDE,
	.longitude = CAL_PROPERTY_TODO_LONGITUDE,
	.created_time = CAL_PROPERTY_TODO_CREATED_TIME,
	.last_modified_time = CAL_PROPERTY_TODO_LAST_MODIFIED_TIME,
	.completed_time = CAL_PROPERTY_TODO_COMPLETED_TIME,
	.progress = CAL_PROPERTY_TODO_PROGRESS,
	.freq = CAL_PROPERTY_TODO_FREQ,
	.range_type = CAL_PROPERTY_TODO_RANGE_TYPE,
	.until_time = CAL_PROPERTY_TODO_UNTIL,
	.count = CAL_PROPERTY_TODO_COUNT,
	.interval = CAL_PROPERTY_TODO_INTERVAL,
	.bysecond = CAL_PROPERTY_TODO_BYSECOND,
	.byminute = CAL_PROPERTY_TODO_BYMINUTE,
	.byhour = CAL_PROPERTY_TODO_BYHOUR,
	.byday = CAL_PROPERTY_TODO_BYDAY,
	.bymonthday = CAL_PROPERTY_TODO_BYMONTHDAY,
	.byyearday = CAL_PROPERTY_TODO_BYYEARDAY,
	.byweekno = CAL_PROPERTY_TODO_BYWEEKNO,
	.bymonth = CAL_PROPERTY_TODO_BYMONTH,
	.bysetpos = CAL_PROPERTY_TODO_BYSETPOS,
	.wkst = CAL_PROPERTY_TODO_WKST,
	.has_alarm = CAL_PROPERTY_TODO_HAS_ALARM,
	.sync_data1 = CAL_PROPERTY_TODO_SYNC_DATA1,
	.sync_data2 = CAL_PROPERTY_TODO_SYNC_DATA2,
	.sync_data3 = CAL_PROPERTY_TODO_SYNC_DATA3,
	.sync_data4 = CAL_PROPERTY_TODO_SYNC_DATA4,
	.start_time = CAL_PROPERTY_TODO_START,
	.start_tzid = CAL_PROPERTY_TODO_START_TZID,
	.due_time = CAL_PROPERTY_TODO_DUE,
	.due_tzid = CAL_PROPERTY_TODO_DUE_TZID,
	.organizer_name = CAL_PROPERTY_TODO_ORGANIZER_NAME,
	.organizer_email = CAL_PROPERTY_TODO_ORGANIZER_EMAIL,
	.has_attendee = CAL_PROPERTY_TODO_HAS_ATTENDEE,
	.is_allday = CAL_PROPERTY_TODO_IS_ALLDAY,
	.calendar_book_visibility = CAL_PROPERTY_CALENDAR_VISIBILITY | CAL_PROPERTY_FLAGS_FILTER,
	.calendar_book_account_id = CAL_PROPERTY_CALENDAR_ACCOUNT_ID | CAL_PROPERTY_FLAGS_FILTER
};

API const _calendar_event_calendar_book_attendee_property_ids   _calendar_event_calendar_book_attendee =
{
	._uri = CALENDAR_VIEW_EVENT_CALENDAR_ATTENDEE,
	.event_id = CAL_PROPERTY_EVENT_ID,
	.calendar_book_id = CAL_PROPERTY_EVENT_CALENDAR_ID,
	.summary = CAL_PROPERTY_EVENT_SUMMARY,
	.description = CAL_PROPERTY_EVENT_DESCRIPTION,
	.location = CAL_PROPERTY_EVENT_LOCATION,
	.categories = CAL_PROPERTY_EVENT_CATEGORIES,
	.exdate = CAL_PROPERTY_EVENT_EXDATE,
	.event_status = CAL_PROPERTY_EVENT_EVENT_STATUS,
	.priority = CAL_PROPERTY_EVENT_PRIORITY,
	.timezone = CAL_PROPERTY_EVENT_TIMEZONE,
	.person_id = CAL_PROPERTY_EVENT_CONTACT_ID,
	.busy_status = CAL_PROPERTY_EVENT_BUSY_STATUS,
	.sensitivity = CAL_PROPERTY_EVENT_SENSITIVITY,
	.uid = CAL_PROPERTY_EVENT_UID,
	.organizer_name = CAL_PROPERTY_EVENT_ORGANIZER_NAME,
	.organizer_email = CAL_PROPERTY_EVENT_ORGANIZER_EMAIL,
	.meeting_status = CAL_PROPERTY_EVENT_MEETING_STATUS,
	.original_event_id = CAL_PROPERTY_EVENT_ORIGINAL_EVENT_ID,
	.latitude = CAL_PROPERTY_EVENT_LATITUDE,
	.longitude = CAL_PROPERTY_EVENT_LONGITUDE,
	.email_id = CAL_PROPERTY_EVENT_EMAIL_ID,
	.created_time = CAL_PROPERTY_EVENT_CREATED_TIME,
	.last_modified_time = CAL_PROPERTY_EVENT_LAST_MODIFIED_TIME,
	.freq = CAL_PROPERTY_EVENT_FREQ,
	.range_type = CAL_PROPERTY_EVENT_RANGE_TYPE,
	.until_time = CAL_PROPERTY_EVENT_UNTIL,
	.count = CAL_PROPERTY_EVENT_COUNT,
	.interval = CAL_PROPERTY_EVENT_INTERVAL,
	.bysecond = CAL_PROPERTY_EVENT_BYSECOND,
	.byminute = CAL_PROPERTY_EVENT_BYMINUTE,
	.byhour = CAL_PROPERTY_EVENT_BYHOUR,
	.byday = CAL_PROPERTY_EVENT_BYDAY,
	.bymonthday = CAL_PROPERTY_EVENT_BYMONTHDAY,
	.byyearday = CAL_PROPERTY_EVENT_BYYEARDAY,
	.byweekno = CAL_PROPERTY_EVENT_BYWEEKNO,
	.bymonth = CAL_PROPERTY_EVENT_BYMONTH,
	.bysetpos = CAL_PROPERTY_EVENT_BYSETPOS,
	.wkst = CAL_PROPERTY_EVENT_WKST,
	.recurrence_id = CAL_PROPERTY_EVENT_RECURRENCE_ID,
	.rdate = CAL_PROPERTY_EVENT_RDATE,
	.has_attendee = CAL_PROPERTY_EVENT_HAS_ATTENDEE,
	.has_alarm = CAL_PROPERTY_EVENT_HAS_ALARM,
	.calendar_system_type = CAL_PROPERTY_EVENT_CALENDAR_SYSTEM_TYPE,
	.sync_data1 = CAL_PROPERTY_EVENT_SYNC_DATA1,
	.sync_data2 = CAL_PROPERTY_EVENT_SYNC_DATA2,
	.sync_data3 = CAL_PROPERTY_EVENT_SYNC_DATA3,
	.sync_data4 = CAL_PROPERTY_EVENT_SYNC_DATA4,
	.start_time = CAL_PROPERTY_EVENT_START,
	.start_tzid = CAL_PROPERTY_EVENT_START_TZID,
	.end_time = CAL_PROPERTY_EVENT_END,
	.end_tzid = CAL_PROPERTY_EVENT_END_TZID,
	.is_allday = CAL_PROPERTY_EVENT_IS_ALLDAY,
	.calendar_book_visibility = CAL_PROPERTY_CALENDAR_VISIBILITY | CAL_PROPERTY_FLAGS_FILTER,
	.calendar_book_account_id = CAL_PROPERTY_CALENDAR_ACCOUNT_ID | CAL_PROPERTY_FLAGS_FILTER,
	.attendee_email = CAL_PROPERTY_ATTENDEE_EMAIL | CAL_PROPERTY_FLAGS_FILTER,
	.attendee_name = CAL_PROPERTY_ATTENDEE_NAME | CAL_PROPERTY_FLAGS_FILTER,
	.attendee_member = CAL_PROPERTY_ATTENDEE_MEMBER | CAL_PROPERTY_FLAGS_FILTER
};

API const _calendar_instance_utime_calendar_book_property_ids   _calendar_instance_utime_calendar_book =
{
	._uri = CALENDAR_VIEW_INSTANCE_UTIME_CALENDAR,
	.event_id = CAL_PROPERTY_INSTANCE_NORMAL_EVENT_ID,
	.start_time = CAL_PROPERTY_INSTANCE_NORMAL_START,
	.end_time = CAL_PROPERTY_INSTANCE_NORMAL_END,
	.summary = CAL_PROPERTY_INSTANCE_NORMAL_SUMMARY,
	.location = CAL_PROPERTY_INSTANCE_NORMAL_LOCATION,
	.calendar_book_id = CAL_PROPERTY_INSTANCE_NORMAL_CALENDAR_ID,
	.description = CAL_PROPERTY_INSTANCE_NORMAL_DESCRIPTION,
	.busy_status = CAL_PROPERTY_INSTANCE_NORMAL_BUSY_STATUS,
	.event_status = CAL_PROPERTY_INSTANCE_NORMAL_EVENT_STATUS,
	.priority = CAL_PROPERTY_INSTANCE_NORMAL_PRIORITY,
	.sensitivity = CAL_PROPERTY_INSTANCE_NORMAL_SENSITIVITY,
	.has_rrule = CAL_PROPERTY_INSTANCE_NORMAL_HAS_RRULE,
	.latitude = CAL_PROPERTY_INSTANCE_NORMAL_LATITUDE,
	.longitude = CAL_PROPERTY_INSTANCE_NORMAL_LONGITUDE,
	.has_alarm = CAL_PROPERTY_INSTANCE_NORMAL_HAS_ALARM,
	.original_event_id = CAL_PROPERTY_INSTANCE_NORMAL_ORIGINAL_EVENT_ID,
	.calendar_book_visibility = CAL_PROPERTY_CALENDAR_VISIBILITY | CAL_PROPERTY_FLAGS_FILTER,
	.calendar_book_account_id = CAL_PROPERTY_CALENDAR_ACCOUNT_ID | CAL_PROPERTY_FLAGS_FILTER,
	.last_modified_time = CAL_PROPERTY_INSTANCE_NORMAL_LAST_MODIFIED_TIME,
	.sync_data1 = CAL_PROPERTY_INSTANCE_NORMAL_SYNC_DATA1
};

API const _calendar_instance_localtime_calendar_book_property_ids   _calendar_instance_localtime_calendar_book =
{
	._uri = CALENDAR_VIEW_INSTANCE_LOCALTIME_CALENDAR,
	.event_id = CAL_PROPERTY_INSTANCE_ALLDAY_EVENT_ID,
	.start_time = CAL_PROPERTY_INSTANCE_ALLDAY_START,
	.end_time = CAL_PROPERTY_INSTANCE_ALLDAY_END,
	.summary = CAL_PROPERTY_INSTANCE_ALLDAY_SUMMARY,
	.location = CAL_PROPERTY_INSTANCE_ALLDAY_LOCATION,
	.calendar_book_id = CAL_PROPERTY_INSTANCE_ALLDAY_CALENDAR_ID,
	.description = CAL_PROPERTY_INSTANCE_ALLDAY_DESCRIPTION,
	.busy_status = CAL_PROPERTY_INSTANCE_ALLDAY_BUSY_STATUS,
	.event_status = CAL_PROPERTY_INSTANCE_ALLDAY_EVENT_STATUS,
	.priority = CAL_PROPERTY_INSTANCE_ALLDAY_PRIORITY,
	.sensitivity = CAL_PROPERTY_INSTANCE_ALLDAY_SENSITIVITY,
	.has_rrule = CAL_PROPERTY_INSTANCE_ALLDAY_HAS_RRULE,
	.latitude = CAL_PROPERTY_INSTANCE_ALLDAY_LATITUDE,
	.longitude = CAL_PROPERTY_INSTANCE_ALLDAY_LONGITUDE,
	.has_alarm = CAL_PROPERTY_INSTANCE_ALLDAY_HAS_ALARM,
	.original_event_id = CAL_PROPERTY_INSTANCE_ALLDAY_ORIGINAL_EVENT_ID,
	.calendar_book_visibility = CAL_PROPERTY_CALENDAR_VISIBILITY | CAL_PROPERTY_FLAGS_FILTER,
	.calendar_book_account_id = CAL_PROPERTY_CALENDAR_ACCOUNT_ID | CAL_PROPERTY_FLAGS_FILTER,
	.last_modified_time = CAL_PROPERTY_INSTANCE_ALLDAY_LAST_MODIFIED_TIME,
	.sync_data1 = CAL_PROPERTY_INSTANCE_ALLDAY_SYNC_DATA1,
	.is_allday = CAL_PROPERTY_INSTANCE_ALLDAY_IS_ALLDAY
};

API const _calendar_instance_utime_calendar_book_extended_property_ids   _calendar_instance_utime_calendar_book_extended =
{
	._uri = CALENDAR_VIEW_INSTANCE_UTIME_CALENDAR_EXTENDED,
	.event_id = CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_EVENT_ID,
	.start_time = CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_START,
	.end_time = CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_END,
	.summary = CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_SUMMARY,
	.location = CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_LOCATION,
	.calendar_book_id = CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_CALENDAR_ID,
	.description = CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_DESCRIPTION,
	.busy_status = CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_BUSY_STATUS,
	.event_status = CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_EVENT_STATUS,
	.priority = CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_PRIORITY,
	.sensitivity = CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_SENSITIVITY,
	.has_rrule = CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_HAS_RRULE,
	.latitude = CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_LATITUDE,
	.longitude = CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_LONGITUDE,
	.has_alarm = CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_HAS_ALARM,
	.original_event_id = CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_ORIGINAL_EVENT_ID,
	.calendar_book_visibility = CAL_PROPERTY_CALENDAR_VISIBILITY | CAL_PROPERTY_FLAGS_FILTER,
	.calendar_book_account_id = CAL_PROPERTY_CALENDAR_ACCOUNT_ID | CAL_PROPERTY_FLAGS_FILTER,
	.last_modified_time = CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_LAST_MODIFIED_TIME,
	.sync_data1 = CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_SYNC_DATA1,
	.organizer_name = CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_ORGANIZER_NAME,
	.categories = CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_CATEGORIES,
	.has_attendee = CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_HAS_ATTENDEE,
	.sync_data2 = CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_SYNC_DATA2,
	.sync_data3 = CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_SYNC_DATA3,
	.sync_data4 = CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_SYNC_DATA4
};

API const _calendar_instance_localtime_calendar_book_extended_property_ids   _calendar_instance_localtime_calendar_book_extended =
{
	._uri = CALENDAR_VIEW_INSTANCE_LOCALTIME_CALENDAR_EXTENDED,
	.event_id = CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_EVENT_ID,
	.start_time = CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_START,
	.end_time = CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_END,
	.summary = CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_SUMMARY,
	.location = CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_LOCATION,
	.calendar_book_id = CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_CALENDAR_ID,
	.description = CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_DESCRIPTION,
	.busy_status = CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_BUSY_STATUS,
	.event_status = CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_EVENT_STATUS,
	.priority = CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_PRIORITY,
	.sensitivity = CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_SENSITIVITY,
	.has_rrule = CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_HAS_RRULE,
	.latitude = CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_LATITUDE,
	.longitude = CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_LONGITUDE,
	.has_alarm = CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_HAS_ALARM,
	.original_event_id = CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_ORIGINAL_EVENT_ID,
	.calendar_book_visibility = CAL_PROPERTY_CALENDAR_VISIBILITY | CAL_PROPERTY_FLAGS_FILTER,
	.calendar_book_account_id = CAL_PROPERTY_CALENDAR_ACCOUNT_ID | CAL_PROPERTY_FLAGS_FILTER,
	.last_modified_time = CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_LAST_MODIFIED_TIME,
	.sync_data1 = CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_SYNC_DATA1,
	.organizer_name= CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_ORGANIZER_NAME,
	.categories = CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_CATEGORIES,
	.has_attendee = CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_HAS_ATTENDEE,
	.sync_data2 = CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_SYNC_DATA2,
	.sync_data3 = CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_SYNC_DATA3,
	.sync_data4 = CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_SYNC_DATA4,
	.is_allday = CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_IS_ALLDAY
};

API const _calendar_extended_property_property_ids   _calendar_extended_property =
{
	._uri = CALENDAR_VIEW_EXTENDED,
	.id = CAL_PROPERTY_EXTENDED_ID,
	.record_id = CAL_PROPERTY_EXTENDED_RECORD_ID,
	.record_type = CAL_PROPERTY_EXTENDED_RECORD_TYPE,
	.key = CAL_PROPERTY_EXTENDED_KEY,
	.value = CAL_PROPERTY_EXTENDED_VALUE
};

//////////////////////
// cal_property_info_s

const cal_property_info_s  __property_calendar_book[] = {
	{CAL_PROPERTY_CALENDAR_ID           ,       "id"},
	{CAL_PROPERTY_CALENDAR_UID          ,       "uid"},
	{CAL_PROPERTY_CALENDAR_NAME         ,       "name"},
	{CAL_PROPERTY_CALENDAR_DESCRIPTION  ,       "description"},
	{CAL_PROPERTY_CALENDAR_COLOR        ,       "color"},
	{CAL_PROPERTY_CALENDAR_LOCATION     ,       "location"},
	{CAL_PROPERTY_CALENDAR_VISIBILITY   ,       "visibility"},
	{CAL_PROPERTY_CALENDAR_SYNC_EVENT   ,       "sync_event"},
	{CAL_PROPERTY_CALENDAR_ACCOUNT_ID   ,       "account_id"},
	{CAL_PROPERTY_CALENDAR_STORE_TYPE   ,       "store_type"},
	{CAL_PROPERTY_CALENDAR_SYNC_DATA1   ,       "sync_data1"},
	{CAL_PROPERTY_CALENDAR_SYNC_DATA2   ,       "sync_data2"},
	{CAL_PROPERTY_CALENDAR_SYNC_DATA3   ,       "sync_data3"},
	{CAL_PROPERTY_CALENDAR_SYNC_DATA4   ,       "sync_data4"},
	{CAL_PROPERTY_CALENDAR_MODE,                "mode"},
};

const cal_property_info_s   __property_event[] =
{
	{CAL_PROPERTY_EVENT_ID,                     "id"},
	{CAL_PROPERTY_EVENT_CALENDAR_ID,            "calendar_id"},
	{CAL_PROPERTY_EVENT_SUMMARY,                "summary"},
	{CAL_PROPERTY_EVENT_DESCRIPTION,            "description"},
	{CAL_PROPERTY_EVENT_LOCATION,               "location"},
	{CAL_PROPERTY_EVENT_CATEGORIES,             "categories"},
	{CAL_PROPERTY_EVENT_EXDATE,                 "exdate"},
	{CAL_PROPERTY_EVENT_EVENT_STATUS,           "task_status"},
	{CAL_PROPERTY_EVENT_PRIORITY,               "priority"},
	{CAL_PROPERTY_EVENT_TIMEZONE,               "timezone"},
	{CAL_PROPERTY_EVENT_CONTACT_ID,             "contact_id"},
	{CAL_PROPERTY_EVENT_BUSY_STATUS,            "busy_status"},
	{CAL_PROPERTY_EVENT_SENSITIVITY,            "sensitivity"},
	{CAL_PROPERTY_EVENT_UID,                    "uid"},
	{CAL_PROPERTY_EVENT_ORGANIZER_NAME,         "organizer_name"},
	{CAL_PROPERTY_EVENT_ORGANIZER_EMAIL,        "organizer_email"},
	{CAL_PROPERTY_EVENT_MEETING_STATUS,         "meeting_status"},
	{CAL_PROPERTY_EVENT_ORIGINAL_EVENT_ID,      "original_event_id"},
	{CAL_PROPERTY_EVENT_LATITUDE,               "latitude"},
	{CAL_PROPERTY_EVENT_LONGITUDE,              "longitude"},
	{CAL_PROPERTY_EVENT_EMAIL_ID,               "email_id"},
	{CAL_PROPERTY_EVENT_CREATED_TIME,           "created_time"},
	{CAL_PROPERTY_EVENT_LAST_MODIFIED_TIME,     "last_mod"},
	{CAL_PROPERTY_EVENT_IS_DELETED,             "is_deleted"},
	{CAL_PROPERTY_EVENT_FREQ,                   "freq"},
	{CAL_PROPERTY_EVENT_RANGE_TYPE,             "range_type"},
	{CAL_PROPERTY_EVENT_UNTIL,                  "until_type,until_utime,until_datetime"},
	{CAL_PROPERTY_EVENT_COUNT,                  "count"},
	{CAL_PROPERTY_EVENT_INTERVAL,               "interval"},
	{CAL_PROPERTY_EVENT_BYSECOND,               "bysecond"},
	{CAL_PROPERTY_EVENT_BYMINUTE,               "byminute"},
	{CAL_PROPERTY_EVENT_BYHOUR,                 "byhour"},
	{CAL_PROPERTY_EVENT_BYDAY,                  "byday"},
	{CAL_PROPERTY_EVENT_BYMONTHDAY,             "bymonthday"},
	{CAL_PROPERTY_EVENT_BYYEARDAY,              "byyearday"},
	{CAL_PROPERTY_EVENT_BYWEEKNO,               "byweekno"},
	{CAL_PROPERTY_EVENT_BYMONTH,                "bymonth"},
	{CAL_PROPERTY_EVENT_BYSETPOS,               "bysetpos"},
	{CAL_PROPERTY_EVENT_WKST,                   "wkst"},
	{CAL_PROPERTY_EVENT_RECURRENCE_ID,          "recurrence_id"},
	{CAL_PROPERTY_EVENT_RDATE,                  "rdate"},
	{CAL_PROPERTY_EVENT_HAS_ATTENDEE,           "has_attendee"},
	{CAL_PROPERTY_EVENT_HAS_ALARM,              "has_alarm"},
	{CAL_PROPERTY_EVENT_SYNC_DATA1,             "sync_data1"},
	{CAL_PROPERTY_EVENT_SYNC_DATA2,             "sync_data2"},
	{CAL_PROPERTY_EVENT_SYNC_DATA3,             "sync_data3"},
	{CAL_PROPERTY_EVENT_SYNC_DATA4,             "sync_data4"},
	{CAL_PROPERTY_EVENT_START,                  "dtstart_type,dtstart_utime,dtstart_datetime"},
	{CAL_PROPERTY_EVENT_END,                    "dtend_type,dtend_utime,dtend_datetime"},
	{CAL_PROPERTY_EVENT_CALENDAR_ALARM,         NULL},
	{CAL_PROPERTY_EVENT_CALENDAR_ATTENDEE,      NULL},
	{CAL_PROPERTY_EVENT_CALENDAR_SYSTEM_TYPE,   "system_type"},
	{CAL_PROPERTY_EVENT_START_TZID,             "dtstart_tzid"},
	{CAL_PROPERTY_EVENT_END_TZID,               "dtend_tzid"},
	{CAL_PROPERTY_EVENT_EXCEPTION,              NULL},
	{CAL_PROPERTY_EVENT_EXTENDED,               NULL},
	{CAL_PROPERTY_EVENT_IS_ALLDAY,              "is_allday"},
};

const cal_property_info_s   __property_todo[] =
{
	{CAL_PROPERTY_TODO_ID,                      "id"},
	{CAL_PROPERTY_TODO_CALENDAR_ID,             "calendar_id"},
	{CAL_PROPERTY_TODO_SUMMARY,                 "summary"},
	{CAL_PROPERTY_TODO_DESCRIPTION,             "description"},
	{CAL_PROPERTY_TODO_LOCATION,                "location"},
	{CAL_PROPERTY_TODO_CATEGORIES,              "categories"},
	{CAL_PROPERTY_TODO_TODO_STATUS,             "task_status"},
	{CAL_PROPERTY_TODO_PRIORITY,                "priority"},
	{CAL_PROPERTY_TODO_SENSITIVITY,             "sensitivity"},
	{CAL_PROPERTY_TODO_UID,                     "uid"},
	{CAL_PROPERTY_TODO_LATITUDE,                "latitude"},
	{CAL_PROPERTY_TODO_LONGITUDE,               "longitude"},
	{CAL_PROPERTY_TODO_PROGRESS,                "progress"},
	{CAL_PROPERTY_TODO_CREATED_TIME,            "created_time"},
	{CAL_PROPERTY_TODO_LAST_MODIFIED_TIME,      "last_mod"},
	{CAL_PROPERTY_TODO_COMPLETED_TIME,          "completed_time"},
	{CAL_PROPERTY_TODO_IS_DELETED,              "is_deleted"},
	{CAL_PROPERTY_TODO_FREQ,                    "freq"},
	{CAL_PROPERTY_TODO_RANGE_TYPE,              "range_type"},
	{CAL_PROPERTY_TODO_UNTIL,                   "until_type,until_utime,until_datetime"},
	{CAL_PROPERTY_TODO_COUNT,                   "count"},
	{CAL_PROPERTY_TODO_INTERVAL,                "interval"},
	{CAL_PROPERTY_TODO_BYSECOND,                "bysecond"},
	{CAL_PROPERTY_TODO_BYMINUTE,                "byminute"},
	{CAL_PROPERTY_TODO_BYHOUR,                  "byhour"},
	{CAL_PROPERTY_TODO_BYDAY,                   "byday"},
	{CAL_PROPERTY_TODO_BYMONTHDAY,              "bymonthday"},
	{CAL_PROPERTY_TODO_BYYEARDAY,               "byyearday"},
	{CAL_PROPERTY_TODO_BYWEEKNO,                "byweekno"},
	{CAL_PROPERTY_TODO_BYMONTH,                 "bymonth"},
	{CAL_PROPERTY_TODO_BYSETPOS,                "bysetpos"},
	{CAL_PROPERTY_TODO_WKST,                    "wkst"},
	{CAL_PROPERTY_TODO_HAS_ALARM,               "has_alarm"},
	{CAL_PROPERTY_TODO_SYNC_DATA1,              "sync_data1"},
	{CAL_PROPERTY_TODO_SYNC_DATA2,              "sync_data2"},
	{CAL_PROPERTY_TODO_SYNC_DATA3,              "sync_data3"},
	{CAL_PROPERTY_TODO_SYNC_DATA4,              "sync_data4"},
	{CAL_PROPERTY_TODO_START,                   "dtstart_type,dtstart_utime,dtstart_datetime"},
	{CAL_PROPERTY_TODO_DUE,                     "dtend_type,dtend_utime,dtend_datetime"},
	{CAL_PROPERTY_TODO_CALENDAR_ALARM,          NULL},
	{CAL_PROPERTY_TODO_START_TZID,              "dtstart_tzid"},
	{CAL_PROPERTY_TODO_DUE_TZID,                "dtend_tzid"},
	{CAL_PROPERTY_TODO_ORGANIZER_NAME,          "organizer_name"},
	{CAL_PROPERTY_TODO_ORGANIZER_EMAIL,         "organizer_email"},
	{CAL_PROPERTY_TODO_HAS_ATTENDEE,            "has_attendee"},
	{CAL_PROPERTY_TODO_CALENDAR_ATTENDEE,       NULL},
	{CAL_PROPERTY_TODO_EXTENDED,                NULL},
	{CAL_PROPERTY_TODO_IS_ALLDAY,               "dtend_type"},
};

const cal_property_info_s   __property_timezone[] =
{
	{CAL_PROPERTY_TIMEZONE_ID,                                  "id"},
	{CAL_PROPERTY_TIMEZONE_TZ_OFFSET_FROM_GMT,                  "tz_offset_from_gmt"},
	{CAL_PROPERTY_TIMEZONE_STANDARD_NAME,                       "standard_name"},
	{CAL_PROPERTY_TIMEZONE_STD_START_MONTH,                     "std_start_month"},
	{CAL_PROPERTY_TIMEZONE_STD_START_POSITION_OF_WEEK,          "std_start_position_of_week"},
	{CAL_PROPERTY_TIMEZONE_STD_START_DAY,                       "std_start_day"},
	{CAL_PROPERTY_TIMEZONE_STD_START_HOUR,                      "std_start_hour"},
	{CAL_PROPERTY_TIMEZONE_STANDARD_BIAS,                       "standard_bias"},
	{CAL_PROPERTY_TIMEZONE_DAY_LIGHT_NAME,                      "day_light_name"},
	{CAL_PROPERTY_TIMEZONE_DAY_LIGHT_START_MONTH,               "day_light_start_month"},
	{CAL_PROPERTY_TIMEZONE_DAY_LIGHT_START_POSITION_OF_WEEK,    "day_light_start_position_of_week"},
	{CAL_PROPERTY_TIMEZONE_DAY_LIGHT_START_DAY,                 "day_light_start_day"},
	{CAL_PROPERTY_TIMEZONE_DAY_LIGHT_START_HOUR,                "day_light_start_hour"},
	{CAL_PROPERTY_TIMEZONE_DAY_LIGHT_BIAS,                      "day_light_bias"},
	{CAL_PROPERTY_TIMEZONE_CALENDAR_ID,                         "calendar_id"},
};

const cal_property_info_s   __property_attendee[] =
{
	{CAL_PROPERTY_ATTENDEE_NUMBER,              "attendee_number"},
	{CAL_PROPERTY_ATTENDEE_CUTYPE,              "attendee_cutype"},
	{CAL_PROPERTY_ATTENDEE_CT_INDEX,            "attendee_ct_index"},
	{CAL_PROPERTY_ATTENDEE_UID,                 "attendee_uid"},
	{CAL_PROPERTY_ATTENDEE_GROUP,               "attendee_group"},
	{CAL_PROPERTY_ATTENDEE_EMAIL,               "attendee_email"},
	{CAL_PROPERTY_ATTENDEE_ROLE,                "attendee_role"},
	{CAL_PROPERTY_ATTENDEE_STATUS,              "attendee_status"},
	{CAL_PROPERTY_ATTENDEE_RSVP,                "attendee_rsvp"},
	{CAL_PROPERTY_ATTENDEE_DELEGATEE_URI,       "attendee_delegatee_uri"},
	{CAL_PROPERTY_ATTENDEE_DELEGATOR_URI,       "attendee_delegator_uri"},
	{CAL_PROPERTY_ATTENDEE_NAME,                "attendee_name"},
	{CAL_PROPERTY_ATTENDEE_MEMBER,              "attendee_member"},
	{CAL_PROPERTY_ATTENDEE_PARENT_ID,           "event_id"},
};

const cal_property_info_s   __property_alarm[] =
{
	{CAL_PROPERTY_ALARM_TICK,               "remind_tick"},
	{CAL_PROPERTY_ALARM_TICK_UNIT,          "remind_tick_unit"},
	{CAL_PROPERTY_ALARM_DESCRIPTION,        "alarm_description"},
	{CAL_PROPERTY_ALARM_PARENT_ID,          "event_id"},
	{CAL_PROPERTY_ALARM_SUMMARY,            "alarm_summary"},
	{CAL_PROPERTY_ALARM_ACTION,             "alarm_action"},
	{CAL_PROPERTY_ALARM_ATTACH,             "alarm_attach"},
	{CAL_PROPERTY_ALARM_ALARM,              "alarm_type,alarm_utime,alarm_datetime"},
};

const cal_property_info_s   __property_updated_info[] =
{
	{CAL_PROPERTY_UPDATED_INFO_ID,                  "id"},
	{CAL_PROPERTY_UPDATED_INFO_CALENDAR_ID,         "calendar_id"},
	{CAL_PROPERTY_UPDATED_INFO_TYPE,                "type"},
	{CAL_PROPERTY_UPDATED_INFO_VERSION,             "ver"},
};

const cal_property_info_s   __property_search_event_calendar[] =
{
	{CAL_PROPERTY_EVENT_ID,                     "id"},
	{CAL_PROPERTY_EVENT_CALENDAR_ID,            "calendar_id"},
	{CAL_PROPERTY_EVENT_SUMMARY,                "summary"},
	{CAL_PROPERTY_EVENT_DESCRIPTION,            "description"},
	{CAL_PROPERTY_EVENT_LOCATION,               "location"},
	{CAL_PROPERTY_EVENT_CATEGORIES,             "categories"},
	{CAL_PROPERTY_EVENT_EXDATE,                 "exdate"},
	{CAL_PROPERTY_EVENT_EVENT_STATUS,           "task_status"},
	{CAL_PROPERTY_EVENT_PRIORITY,               "priority"},
	{CAL_PROPERTY_EVENT_TIMEZONE,               "timezone"},
	{CAL_PROPERTY_EVENT_CONTACT_ID,             "contact_id"},
	{CAL_PROPERTY_EVENT_BUSY_STATUS,            "busy_status"},
	{CAL_PROPERTY_EVENT_SENSITIVITY,            "sensitivity"},
	{CAL_PROPERTY_EVENT_UID,                    "uid"},
	{CAL_PROPERTY_EVENT_ORGANIZER_NAME,         "organizer_name"},
	{CAL_PROPERTY_EVENT_ORGANIZER_EMAIL,        "organizer_email"},
	{CAL_PROPERTY_EVENT_MEETING_STATUS,         "meeting_status"},
	{CAL_PROPERTY_EVENT_ORIGINAL_EVENT_ID,      "original_event_id"},
	{CAL_PROPERTY_EVENT_LATITUDE,               "latitude"},
	{CAL_PROPERTY_EVENT_LONGITUDE,              "longitude"},
	{CAL_PROPERTY_EVENT_EMAIL_ID,               "email_id"},
	{CAL_PROPERTY_EVENT_CREATED_TIME,           "created_time"},
	{CAL_PROPERTY_EVENT_LAST_MODIFIED_TIME,     "last_mod"},
	{CAL_PROPERTY_EVENT_FREQ,                   "freq"},
	{CAL_PROPERTY_EVENT_RANGE_TYPE,             "range_type"},
	{CAL_PROPERTY_EVENT_UNTIL,                  "until_type,until_utime,until_datetime"},
	{CAL_PROPERTY_EVENT_COUNT,                  "count"},
	{CAL_PROPERTY_EVENT_INTERVAL,               "interval"},
	{CAL_PROPERTY_EVENT_BYSECOND,               "bysecond"},
	{CAL_PROPERTY_EVENT_BYMINUTE,               "byminute"},
	{CAL_PROPERTY_EVENT_BYHOUR,                 "byhour"},
	{CAL_PROPERTY_EVENT_BYDAY,                  "byday"},
	{CAL_PROPERTY_EVENT_BYMONTHDAY,             "bymonthday"},
	{CAL_PROPERTY_EVENT_BYYEARDAY,              "byyearday"},
	{CAL_PROPERTY_EVENT_BYWEEKNO,               "byweekno"},
	{CAL_PROPERTY_EVENT_BYMONTH,                "bymonth"},
	{CAL_PROPERTY_EVENT_BYSETPOS,               "bysetpos"},
	{CAL_PROPERTY_EVENT_WKST,                   "wkst"},
	{CAL_PROPERTY_EVENT_RECURRENCE_ID,          "recurrence_id"},
	{CAL_PROPERTY_EVENT_RDATE,                  "rdate"},
	{CAL_PROPERTY_EVENT_HAS_ATTENDEE,           "has_attendee"},
	{CAL_PROPERTY_EVENT_HAS_ALARM,              "has_alarm"},
	{CAL_PROPERTY_EVENT_SYNC_DATA1,             "sync_data1"},
	{CAL_PROPERTY_EVENT_SYNC_DATA2,             "sync_data2"},
	{CAL_PROPERTY_EVENT_SYNC_DATA3,             "sync_data3"},
	{CAL_PROPERTY_EVENT_SYNC_DATA4,             "sync_data4"},
	{CAL_PROPERTY_EVENT_START,                  "dtstart_type,dtstart_utime,dtstart_datetime"},
	{CAL_PROPERTY_EVENT_END,                    "dtend_type,dtend_utime,dtend_datetime"},
	{CAL_PROPERTY_EVENT_CALENDAR_SYSTEM_TYPE,   "system_type"},
	{CAL_PROPERTY_EVENT_START_TZID,             "dtstart_tzid"},
	{CAL_PROPERTY_EVENT_END_TZID,               "dtend_tzid"},
	{CAL_PROPERTY_EVENT_IS_ALLDAY,              "is_allday"},
	{(CAL_PROPERTY_CALENDAR_VISIBILITY|CAL_PROPERTY_FLAGS_FILTER),       "visibility"},
	{(CAL_PROPERTY_CALENDAR_ACCOUNT_ID|CAL_PROPERTY_FLAGS_FILTER),       "account_id"},
};

const cal_property_info_s   __property_search_todo_calendar[] =
{
	{CAL_PROPERTY_TODO_ID,                      "id"},
	{CAL_PROPERTY_TODO_CALENDAR_ID,             "calendar_id"},
	{CAL_PROPERTY_TODO_SUMMARY,                 "summary"},
	{CAL_PROPERTY_TODO_DESCRIPTION,             "description"},
	{CAL_PROPERTY_TODO_LOCATION,                "location"},
	{CAL_PROPERTY_TODO_CATEGORIES,              "categories"},
	{CAL_PROPERTY_TODO_TODO_STATUS,             "task_status"},
	{CAL_PROPERTY_TODO_PRIORITY,                "priority"},
	{CAL_PROPERTY_TODO_SENSITIVITY,             "sensitivity"},
	{CAL_PROPERTY_TODO_UID,                     "uid"},
	{CAL_PROPERTY_TODO_LATITUDE,                "latitude"},
	{CAL_PROPERTY_TODO_LONGITUDE,               "longitude"},
	{CAL_PROPERTY_TODO_PROGRESS,                "progress"},
	{CAL_PROPERTY_TODO_CREATED_TIME,            "created_time"},
	{CAL_PROPERTY_TODO_LAST_MODIFIED_TIME,      "last_mod"},
	{CAL_PROPERTY_TODO_COMPLETED_TIME,          "completed_time"},
	{CAL_PROPERTY_TODO_FREQ,                    "freq"},
	{CAL_PROPERTY_TODO_RANGE_TYPE,              "range_type"},
	{CAL_PROPERTY_TODO_UNTIL,                   "until_type,until_utime,until_datetime"},
	{CAL_PROPERTY_TODO_COUNT,                   "count"},
	{CAL_PROPERTY_TODO_INTERVAL,                "interval"},
	{CAL_PROPERTY_TODO_BYSECOND,                "bysecond"},
	{CAL_PROPERTY_TODO_BYMINUTE,                "byminute"},
	{CAL_PROPERTY_TODO_BYHOUR,                  "byhour"},
	{CAL_PROPERTY_TODO_BYDAY,                   "byday"},
	{CAL_PROPERTY_TODO_BYMONTHDAY,              "bymonthday"},
	{CAL_PROPERTY_TODO_BYYEARDAY,               "byyearday"},
	{CAL_PROPERTY_TODO_BYWEEKNO,                "byweekno"},
	{CAL_PROPERTY_TODO_BYMONTH,                 "bymonth"},
	{CAL_PROPERTY_TODO_BYSETPOS,                "bysetpos"},
	{CAL_PROPERTY_TODO_WKST,                    "wkst"},
	{CAL_PROPERTY_TODO_HAS_ALARM,               "has_alarm"},
	{CAL_PROPERTY_TODO_SYNC_DATA1,              "sync_data1"},
	{CAL_PROPERTY_TODO_SYNC_DATA2,              "sync_data2"},
	{CAL_PROPERTY_TODO_SYNC_DATA3,              "sync_data3"},
	{CAL_PROPERTY_TODO_SYNC_DATA4,              "sync_data4"},
	{CAL_PROPERTY_TODO_START,                   "dtstart_type,dtstart_utime,dtstart_datetime"},
	{CAL_PROPERTY_TODO_DUE,                     "dtend_type,dtend_utime,dtend_datetime"},
	{CAL_PROPERTY_TODO_START_TZID,              "dtstart_tzid"},
	{CAL_PROPERTY_TODO_DUE_TZID,                "dtend_tzid"},
	{CAL_PROPERTY_TODO_ORGANIZER_NAME,          "organizer_name"},
	{CAL_PROPERTY_TODO_ORGANIZER_EMAIL,         "organizer_email"},
	{CAL_PROPERTY_TODO_HAS_ATTENDEE,            "has_attendee"},
	{CAL_PROPERTY_TODO_IS_ALLDAY,               "dtend_type"},
	{(CAL_PROPERTY_CALENDAR_VISIBILITY|CAL_PROPERTY_FLAGS_FILTER),       "visibility"},
	{(CAL_PROPERTY_CALENDAR_ACCOUNT_ID|CAL_PROPERTY_FLAGS_FILTER),       "account_id"},
};

const cal_property_info_s   __property_search_event_calendar_attendee[] =
{
	{CAL_PROPERTY_EVENT_ID,                     "id"},
	{CAL_PROPERTY_EVENT_CALENDAR_ID,            "calendar_id"},
	{CAL_PROPERTY_EVENT_SUMMARY,                "summary"},
	{CAL_PROPERTY_EVENT_DESCRIPTION,            "description"},
	{CAL_PROPERTY_EVENT_LOCATION,               "location"},
	{CAL_PROPERTY_EVENT_CATEGORIES,             "categories"},
	{CAL_PROPERTY_EVENT_EXDATE,                 "exdate"},
	{CAL_PROPERTY_EVENT_EVENT_STATUS,           "task_status"},
	{CAL_PROPERTY_EVENT_PRIORITY,               "priority"},
	{CAL_PROPERTY_EVENT_TIMEZONE,               "timezone"},
	{CAL_PROPERTY_EVENT_CONTACT_ID,             "contact_id"},
	{CAL_PROPERTY_EVENT_BUSY_STATUS,            "busy_status"},
	{CAL_PROPERTY_EVENT_SENSITIVITY,            "sensitivity"},
	{CAL_PROPERTY_EVENT_UID,                    "uid"},
	{CAL_PROPERTY_EVENT_ORGANIZER_NAME,         "organizer_name"},
	{CAL_PROPERTY_EVENT_ORGANIZER_EMAIL,        "organizer_email"},
	{CAL_PROPERTY_EVENT_MEETING_STATUS,         "meeting_status"},
	{CAL_PROPERTY_EVENT_ORIGINAL_EVENT_ID,      "original_event_id"},
	{CAL_PROPERTY_EVENT_LATITUDE,               "latitude"},
	{CAL_PROPERTY_EVENT_LONGITUDE,              "longitude"},
	{CAL_PROPERTY_EVENT_EMAIL_ID,               "email_id"},
	{CAL_PROPERTY_EVENT_CREATED_TIME,           "created_time"},
	{CAL_PROPERTY_EVENT_LAST_MODIFIED_TIME,     "last_mod"},
	{CAL_PROPERTY_EVENT_FREQ,                   "freq"},
	{CAL_PROPERTY_EVENT_RANGE_TYPE,             "range_type"},
	{CAL_PROPERTY_EVENT_UNTIL,                  "until_type,until_utime,until_datetime"},
	{CAL_PROPERTY_EVENT_COUNT,                  "count"},
	{CAL_PROPERTY_EVENT_INTERVAL,               "interval"},
	{CAL_PROPERTY_EVENT_BYSECOND,               "bysecond"},
	{CAL_PROPERTY_EVENT_BYMINUTE,               "byminute"},
	{CAL_PROPERTY_EVENT_BYHOUR,                 "byhour"},
	{CAL_PROPERTY_EVENT_BYDAY,                  "byday"},
	{CAL_PROPERTY_EVENT_BYMONTHDAY,             "bymonthday"},
	{CAL_PROPERTY_EVENT_BYYEARDAY,              "byyearday"},
	{CAL_PROPERTY_EVENT_BYWEEKNO,               "byweekno"},
	{CAL_PROPERTY_EVENT_BYMONTH,                "bymonth"},
	{CAL_PROPERTY_EVENT_BYSETPOS,               "bysetpos"},
	{CAL_PROPERTY_EVENT_WKST,                   "wkst"},
	{CAL_PROPERTY_EVENT_RECURRENCE_ID,          "recurrence_id"},
	{CAL_PROPERTY_EVENT_RDATE,                  "rdate"},
	{CAL_PROPERTY_EVENT_HAS_ATTENDEE,           "has_attendee"},
	{CAL_PROPERTY_EVENT_HAS_ALARM,              "has_alarm"},
	{CAL_PROPERTY_EVENT_SYNC_DATA1,             "sync_data1"},
	{CAL_PROPERTY_EVENT_SYNC_DATA2,             "sync_data2"},
	{CAL_PROPERTY_EVENT_SYNC_DATA3,             "sync_data3"},
	{CAL_PROPERTY_EVENT_SYNC_DATA4,             "sync_data4"},
	{CAL_PROPERTY_EVENT_START,                  "dtstart_type,dtstart_utime,dtstart_datetime"},
	{CAL_PROPERTY_EVENT_END,                    "dtend_type,dtend_utime,dtend_datetime"},
	{CAL_PROPERTY_EVENT_CALENDAR_SYSTEM_TYPE,   "system_type"},
	{CAL_PROPERTY_EVENT_START_TZID,             "dtstart_tzid"},
	{CAL_PROPERTY_EVENT_END_TZID,               "dtend_tzid"},
	{CAL_PROPERTY_EVENT_IS_ALLDAY,              "is_allday"},
	{(CAL_PROPERTY_CALENDAR_VISIBILITY|CAL_PROPERTY_FLAGS_FILTER),       "visibility"},
	{(CAL_PROPERTY_CALENDAR_ACCOUNT_ID|CAL_PROPERTY_FLAGS_FILTER),       "account_id"},
	{(CAL_PROPERTY_ATTENDEE_EMAIL|CAL_PROPERTY_FLAGS_FILTER),            "attendee_email"},
	{(CAL_PROPERTY_ATTENDEE_NAME|CAL_PROPERTY_FLAGS_FILTER),             "attendee_name"},
	{(CAL_PROPERTY_ATTENDEE_MEMBER|CAL_PROPERTY_FLAGS_FILTER),           "attendee_member"},
};

const cal_property_info_s   __property_search_instance_utime_calendar[] =
{
	{CAL_PROPERTY_INSTANCE_NORMAL_EVENT_ID,             "event_id"},
	{CAL_PROPERTY_INSTANCE_NORMAL_START,                "dtstart_type,dtstart_utime,dtstart_datetime"},
	{CAL_PROPERTY_INSTANCE_NORMAL_END,                  "dtend_type,dtend_utime,dtend_datetime"},
	{CAL_PROPERTY_INSTANCE_NORMAL_SUMMARY,              "summary"},
	{CAL_PROPERTY_INSTANCE_NORMAL_LOCATION,             "location"},
	{CAL_PROPERTY_INSTANCE_NORMAL_CALENDAR_ID,          "calendar_id"},
	{CAL_PROPERTY_INSTANCE_NORMAL_DESCRIPTION,          "description"},
	{CAL_PROPERTY_INSTANCE_NORMAL_BUSY_STATUS,          "busy_status"},
	{CAL_PROPERTY_INSTANCE_NORMAL_EVENT_STATUS,         "task_status"},
	{CAL_PROPERTY_INSTANCE_NORMAL_PRIORITY,             "priority"},
	{CAL_PROPERTY_INSTANCE_NORMAL_SENSITIVITY,          "sensitivity"},
	{CAL_PROPERTY_INSTANCE_NORMAL_HAS_RRULE,             "rrule_id"},
	{CAL_PROPERTY_INSTANCE_NORMAL_LATITUDE,             "latitude"},
	{CAL_PROPERTY_INSTANCE_NORMAL_LONGITUDE,            "longitude"},
	{CAL_PROPERTY_INSTANCE_NORMAL_HAS_ALARM,            "has_alarm"},
	{CAL_PROPERTY_INSTANCE_NORMAL_ORIGINAL_EVENT_ID,    "original_event_id"},
	{(CAL_PROPERTY_CALENDAR_VISIBILITY|CAL_PROPERTY_FLAGS_FILTER),       "visibility"},
	{(CAL_PROPERTY_CALENDAR_ACCOUNT_ID|CAL_PROPERTY_FLAGS_FILTER),       "account_id"},
	{CAL_PROPERTY_INSTANCE_NORMAL_LAST_MODIFIED_TIME,   "last_mod"},
	{CAL_PROPERTY_INSTANCE_NORMAL_SYNC_DATA1,   "sync_data1"},
};

const cal_property_info_s   __property_search_instance_localtime_calendar[] =
{
	{CAL_PROPERTY_INSTANCE_ALLDAY_EVENT_ID,             "event_id"},
	{CAL_PROPERTY_INSTANCE_ALLDAY_START,                "dtstart_type,dtstart_utime,dtstart_datetime"},
	{CAL_PROPERTY_INSTANCE_ALLDAY_END,                  "dtend_type,dtend_utime,dtend_datetime"},
	{CAL_PROPERTY_INSTANCE_ALLDAY_SUMMARY,              "summary"},
	{CAL_PROPERTY_INSTANCE_ALLDAY_LOCATION,             "location"},
	{CAL_PROPERTY_INSTANCE_ALLDAY_CALENDAR_ID,          "calendar_id"},
	{CAL_PROPERTY_INSTANCE_ALLDAY_DESCRIPTION,          "description"},
	{CAL_PROPERTY_INSTANCE_ALLDAY_BUSY_STATUS,          "busy_status"},
	{CAL_PROPERTY_INSTANCE_ALLDAY_EVENT_STATUS,         "task_status"},
	{CAL_PROPERTY_INSTANCE_ALLDAY_PRIORITY,             "priority"},
	{CAL_PROPERTY_INSTANCE_ALLDAY_SENSITIVITY,          "sensitivity"},
	{CAL_PROPERTY_INSTANCE_ALLDAY_HAS_RRULE,             "rrule_id"},
	{CAL_PROPERTY_INSTANCE_ALLDAY_LATITUDE,             "latitude"},
	{CAL_PROPERTY_INSTANCE_ALLDAY_LONGITUDE,            "longitude"},
	{CAL_PROPERTY_INSTANCE_ALLDAY_HAS_ALARM,            "has_alarm"},
	{CAL_PROPERTY_INSTANCE_ALLDAY_ORIGINAL_EVENT_ID,    "original_event_id"},
	{(CAL_PROPERTY_CALENDAR_VISIBILITY|CAL_PROPERTY_FLAGS_FILTER),       "visibility"},
	{(CAL_PROPERTY_CALENDAR_ACCOUNT_ID|CAL_PROPERTY_FLAGS_FILTER),       "account_id"},
	{CAL_PROPERTY_INSTANCE_ALLDAY_LAST_MODIFIED_TIME,   "last_mod"},
	{CAL_PROPERTY_INSTANCE_ALLDAY_SYNC_DATA1,   "sync_data1"},
	{CAL_PROPERTY_INSTANCE_ALLDAY_IS_ALLDAY,   "is_allday"},
};

const cal_property_info_s   __property_search_instance_utime_calendar_extended[] =
{
	{CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_EVENT_ID,             "event_id"},
	{CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_START,                "dtstart_type,dtstart_utime,dtstart_datetime"},
	{CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_END,                  "dtend_type,dtend_utime,dtend_datetime"},
	{CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_SUMMARY,              "summary"},
	{CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_LOCATION,             "location"},
	{CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_CALENDAR_ID,          "calendar_id"},
	{CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_DESCRIPTION,          "description"},
	{CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_BUSY_STATUS,          "busy_status"},
	{CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_EVENT_STATUS,         "task_status"},
	{CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_PRIORITY,             "priority"},
	{CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_SENSITIVITY,          "sensitivity"},
	{CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_HAS_RRULE,            "rrule_id"},
	{CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_LATITUDE,             "latitude"},
	{CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_LONGITUDE,            "longitude"},
	{CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_HAS_ALARM,            "has_alarm"},
	{CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_ORIGINAL_EVENT_ID,    "original_event_id"},
	{(CAL_PROPERTY_CALENDAR_VISIBILITY|CAL_PROPERTY_FLAGS_FILTER),       "visibility"},
	{(CAL_PROPERTY_CALENDAR_ACCOUNT_ID|CAL_PROPERTY_FLAGS_FILTER),       "account_id"},
	{CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_LAST_MODIFIED_TIME,   "last_mod"},
	{CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_SYNC_DATA1,   "sync_data1"},
	{CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_ORGANIZER_NAME,   "organizer_name"},
	{CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_CATEGORIES,   "categories"},
	{CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_HAS_ATTENDEE,   "has_attendee"},
	{CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_SYNC_DATA2,   "sync_data2"},
	{CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_SYNC_DATA3,   "sync_data3"},
	{CAL_PROPERTY_INSTANCE_NORMAL_EXTENDED_SYNC_DATA4,   "sync_data4"},
};


const cal_property_info_s   __property_search_instance_localtime_calendar_extended[] =
{
	{CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_EVENT_ID,             "event_id"},
	{CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_START,                "dtstart_type,dtstart_utime,dtstart_datetime"},
	{CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_END,                  "dtend_type,dtend_utime,dtend_datetime"},
	{CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_SUMMARY,              "summary"},
	{CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_LOCATION,             "location"},
	{CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_CALENDAR_ID,          "calendar_id"},
	{CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_DESCRIPTION,          "description"},
	{CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_BUSY_STATUS,          "busy_status"},
	{CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_EVENT_STATUS,         "task_status"},
	{CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_PRIORITY,             "priority"},
	{CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_SENSITIVITY,          "sensitivity"},
	{CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_HAS_RRULE,            "rrule_id"},
	{CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_LATITUDE,             "latitude"},
	{CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_LONGITUDE,            "longitude"},
	{CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_HAS_ALARM,            "has_alarm"},
	{CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_ORIGINAL_EVENT_ID,    "original_event_id"},
	{(CAL_PROPERTY_CALENDAR_VISIBILITY|CAL_PROPERTY_FLAGS_FILTER),       "visibility"},
	{(CAL_PROPERTY_CALENDAR_ACCOUNT_ID|CAL_PROPERTY_FLAGS_FILTER),       "account_id"},
	{CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_LAST_MODIFIED_TIME,   "last_mod"},
	{CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_SYNC_DATA1,   "sync_data1"},
	{CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_ORGANIZER_NAME,   "organizer_name"},
	{CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_CATEGORIES,   "categories"},
	{CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_HAS_ATTENDEE,   "has_attendee"},
	{CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_SYNC_DATA2,   "sync_data2"},
	{CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_SYNC_DATA3,   "sync_data3"},
	{CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_SYNC_DATA4,   "sync_data4"},
	{CAL_PROPERTY_INSTANCE_ALLDAY_EXTENDED_IS_ALLDAY,    "is_allday"},
};

const cal_property_info_s   __property_extended[] =
{
	{CAL_PROPERTY_EXTENDED_ID,              "id"},
	{CAL_PROPERTY_EXTENDED_RECORD_ID,       "record_id"},
	{CAL_PROPERTY_EXTENDED_RECORD_TYPE,     "record_type"},
	{CAL_PROPERTY_EXTENDED_KEY,             "key"},
	{CAL_PROPERTY_EXTENDED_VALUE,           "value"},
};

typedef struct {
	char *view_uri;
	cal_record_type_e type;
	cal_property_info_s *properties;
	int property_count;
}cal_view_uri_info_s;

#define PTR_COUNT(X)    (void*)(X), sizeof(X)/sizeof(cal_property_info_s)


static const cal_view_uri_info_s __tables[] = {
	{CALENDAR_VIEW_CALENDAR,        CAL_RECORD_TYPE_CALENDAR,           PTR_COUNT(__property_calendar_book) },
	{CALENDAR_VIEW_EVENT,           CAL_RECORD_TYPE_EVENT,              PTR_COUNT(__property_event) },
	{CALENDAR_VIEW_TODO,            CAL_RECORD_TYPE_TODO,               PTR_COUNT(__property_todo) },
	{CALENDAR_VIEW_TIMEZONE,        CAL_RECORD_TYPE_TIMEZONE,           PTR_COUNT(__property_timezone) },
	{CALENDAR_VIEW_ATTENDEE,        CAL_RECORD_TYPE_ATTENDEE,           PTR_COUNT(__property_attendee) },
	{CALENDAR_VIEW_ALARM,           CAL_RECORD_TYPE_ALARM,              PTR_COUNT(__property_alarm) },
	{CALENDAR_VIEW_UPDATED_INFO,    CAL_RECORD_TYPE_UPDATED_INFO,       PTR_COUNT(__property_updated_info) },
	{CALENDAR_VIEW_EVENT_CALENDAR,  CAL_RECORD_TYPE_SEARCH,             PTR_COUNT(__property_search_event_calendar) },
	{CALENDAR_VIEW_TODO_CALENDAR,   CAL_RECORD_TYPE_SEARCH,             PTR_COUNT(__property_search_todo_calendar ) },
	{CALENDAR_VIEW_EVENT_CALENDAR_ATTENDEE,  CAL_RECORD_TYPE_SEARCH,    PTR_COUNT(__property_search_event_calendar_attendee) },
	{CALENDAR_VIEW_INSTANCE_UTIME_CALENDAR, CAL_RECORD_TYPE_INSTANCE_NORMAL, PTR_COUNT(__property_search_instance_utime_calendar) },
	{CALENDAR_VIEW_INSTANCE_LOCALTIME_CALENDAR, CAL_RECORD_TYPE_INSTANCE_ALLDAY, PTR_COUNT(__property_search_instance_localtime_calendar) },
	{CALENDAR_VIEW_INSTANCE_UTIME_CALENDAR_EXTENDED, CAL_RECORD_TYPE_INSTANCE_NORMAL_EXTENDED, PTR_COUNT(__property_search_instance_utime_calendar_extended) },
	{CALENDAR_VIEW_INSTANCE_LOCALTIME_CALENDAR_EXTENDED, CAL_RECORD_TYPE_INSTANCE_ALLDAY_EXTENDED, PTR_COUNT(__property_search_instance_localtime_calendar_extended) },
	{CALENDAR_VIEW_EXTENDED, CAL_RECORD_TYPE_EXTENDED, PTR_COUNT(__property_extended) },
};

//////////////////////

static bool cal_uri_property_flag = false;
static GHashTable *cal_uri_property_hash = NULL;
#ifdef CAL_IPC_CLIENT
static int calendar_view_count = 0;
#endif

void _cal_view_initialize(void)
{
	bool bmutex = false;

#ifdef CAL_IPC_CLIENT
	_cal_mutex_lock(CAL_MUTEX_PROPERTY_HASH);
	calendar_view_count++;
	_cal_mutex_unlock(CAL_MUTEX_PROPERTY_HASH);
#endif

	if (cal_uri_property_flag == false)
	{
		_cal_mutex_lock(CAL_MUTEX_PROPERTY_HASH);
		bmutex = true;
	}

	if ( cal_uri_property_hash == NULL)
	{
		cal_uri_property_hash = g_hash_table_new(g_str_hash, g_str_equal);
		if (cal_uri_property_hash)
		{
			g_hash_table_insert(cal_uri_property_hash, CALENDAR_VIEW_CALENDAR, GINT_TO_POINTER(&(__tables[0])) );
			g_hash_table_insert(cal_uri_property_hash, CALENDAR_VIEW_EVENT, GINT_TO_POINTER(&(__tables[1])) );
			g_hash_table_insert(cal_uri_property_hash, CALENDAR_VIEW_TODO, GINT_TO_POINTER(&(__tables[2])) );
			g_hash_table_insert(cal_uri_property_hash, CALENDAR_VIEW_TIMEZONE, GINT_TO_POINTER(&(__tables[3])));
			g_hash_table_insert(cal_uri_property_hash, CALENDAR_VIEW_ATTENDEE, GINT_TO_POINTER(&(__tables[4])));
			g_hash_table_insert(cal_uri_property_hash, CALENDAR_VIEW_ALARM, GINT_TO_POINTER(&(__tables[5])));
			g_hash_table_insert(cal_uri_property_hash, CALENDAR_VIEW_UPDATED_INFO, GINT_TO_POINTER(&(__tables[6])));
			g_hash_table_insert(cal_uri_property_hash, CALENDAR_VIEW_EVENT_CALENDAR, GINT_TO_POINTER(&(__tables[7])));
			g_hash_table_insert(cal_uri_property_hash, CALENDAR_VIEW_TODO_CALENDAR, GINT_TO_POINTER(&(__tables[8])));
			g_hash_table_insert(cal_uri_property_hash, CALENDAR_VIEW_EVENT_CALENDAR_ATTENDEE, GINT_TO_POINTER(&(__tables[9])));
			g_hash_table_insert(cal_uri_property_hash, CALENDAR_VIEW_INSTANCE_UTIME_CALENDAR, GINT_TO_POINTER(&(__tables[10])));
			g_hash_table_insert(cal_uri_property_hash, CALENDAR_VIEW_INSTANCE_LOCALTIME_CALENDAR, GINT_TO_POINTER(&(__tables[11])));
			g_hash_table_insert(cal_uri_property_hash, CALENDAR_VIEW_INSTANCE_UTIME_CALENDAR_EXTENDED, GINT_TO_POINTER(&(__tables[12])));
			g_hash_table_insert(cal_uri_property_hash, CALENDAR_VIEW_INSTANCE_LOCALTIME_CALENDAR_EXTENDED, GINT_TO_POINTER(&(__tables[13])));
			g_hash_table_insert(cal_uri_property_hash, CALENDAR_VIEW_EXTENDED, GINT_TO_POINTER(&(__tables[14])));
		}
	}

	if (bmutex == true)
	{
		cal_uri_property_flag = true;
		_cal_mutex_unlock(CAL_MUTEX_PROPERTY_HASH);
	}

	return ;
}

cal_record_type_e _cal_view_get_type(const char *view_uri)
{
	cal_view_uri_info_s* view_uri_info = NULL;
	cal_record_type_e type = CAL_RECORD_TYPE_INVALID;

	if(cal_uri_property_hash){
		view_uri_info = g_hash_table_lookup(cal_uri_property_hash, view_uri);
		if( view_uri_info ) {
			type = view_uri_info->type;
		}
		else
		{
			ERR("g_hash_table_lookup() failed");
		}
	}
	else
	{
		ERR("Unable to get cal_uri_property_hash[%s]", view_uri);
	}

	return type;
}

void _cal_view_finalize(void)
{
#ifdef CAL_IPC_CLIENT
	_cal_mutex_lock(CAL_MUTEX_PROPERTY_HASH);
	if (calendar_view_count <= 0)
	{
		_cal_mutex_unlock(CAL_MUTEX_PROPERTY_HASH);
		return ;
	}
	calendar_view_count--;

	if (calendar_view_count == 0)
	{
		if (cal_uri_property_hash != NULL) {
			g_hash_table_destroy(cal_uri_property_hash);
			cal_uri_property_hash = NULL;
		}
	}
	_cal_mutex_unlock(CAL_MUTEX_PROPERTY_HASH);
#endif
}

const cal_property_info_s* _cal_view_get_property_info(const char *view_uri, int *count)
{
	cal_property_info_s* tmp = NULL;
	cal_view_uri_info_s* view_uri_info = NULL;

	if(cal_uri_property_hash){
		view_uri_info = g_hash_table_lookup(cal_uri_property_hash, view_uri);
		if( view_uri_info ) {
			tmp = view_uri_info->properties;
			*count = view_uri_info->property_count;
		}
	}

	return tmp;
}

const char* _cal_view_get_uri(const char *view_uri)
{
	cal_view_uri_info_s* view_uri_info = NULL;

	if(cal_uri_property_hash){
		view_uri_info = g_hash_table_lookup(cal_uri_property_hash, view_uri);
		if( view_uri_info ) {
			return view_uri_info->view_uri;
		}
	}

	return NULL;
}
