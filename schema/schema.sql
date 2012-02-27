--
-- Calendar Service
--
-- Copyright (c) 2000 - 2012 Samsung Electronics Co., Ltd. All rights reserved.
--
-- Licensed under the Apache License, Version 2.0 (the "License");
-- you may not use this file except in compliance with the License.
-- You may obtain a copy of the License at
--
-- http://www.apache.org/licenses/LICENSE-2.0
--
-- Unless required by applicable law or agreed to in writing, software
-- distributed under the License is distributed on an "AS IS" BASIS,
-- WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
-- See the License for the specific language governing permissions and
-- limitations under the License.
--

CREATE TABLE schedule_table
(
id INTEGER PRIMARY KEY AUTOINCREMENT,
account_id INTEGER,
type INTEGER,
category INTEGER,
summary TEXT,
description TEXT,
location TEXT,
all_day_event INTEGER,
start_date_time INTEGER,
end_date_time INTEGER,
repeat_item INTEGER,
repeat_interval INTEGER,
repeat_occurrences INTEGER,
repeat_end_date INTEGER,
sun_moon INTEGER,
week_start INTEGER,
week_flag TEXT,
day_date INTEGER,
last_modified_time INTEGER,
missed INTEGER,
task_status INTEGER,
priority INTEGER,
timezone INTEGER,
file_id INTEGER,
contact_id INTEGER,
busy_status INTEGER,
sensitivity INTEGER,
uid TEXT,
calendar_type INTEGER,
organizer_name TEXT,
organizer_email TEXT,
meeting_status INTEGER,
gcal_id TEXT,
deleted INTEGER,
updated TEXT,
location_type INTEGER,
location_summary TEXT,
etag TEXT,
calendar_id INTEGER,
sync_status INTEGER,
edit_uri TEXT,
gevent_id TEXT,
dst INTEGER,
original_event_id INTEGER,
latitude DOUBLE,
longitude DOUBLE,
is_deleted INTEGER,
tz_name TEXT,
tz_city_name TEXT,
email_id INTEGER,
availability INTEGER,
created_date_time INTEGER,
completed_date_time INTEGER,
progress INTEGER
);
CREATE INDEX sch_idx1 ON schedule_table(type, category);
CREATE TRIGGER trg_sch_del AFTER DELETE ON schedule_table
 BEGIN
   DELETE FROM cal_alarm_table WHERE event_id = old.id;
 END;

CREATE TABLE cal_participant_table
(
event_id INTEGER,
attendee_name TEXT,
attendee_email TEXT,
attendee_number TEXT,
attendee_status INTEGER,
attendee_type INTEGER,
attendee_ct_index INTEGER,
attendee_role INTEGER,
attendee_rsvp INTEGER,
attendee_group TEXT,
attendee_delegator_uri TEXT,
attendee_delegate_uri TEXT,
attendee_uid TEXT
);

CREATE TABLE cal_meeting_category_table
(
event_id INTEGER,
category_name TEXT
);

CREATE TABLE recurrency_log_table
(
uid TEXT,
start_date_time INTEGER,
end_date_time INTEGER,
event_id INTEGER,
exception_event_id INTEGER,
updated INTEGER
);

CREATE TABLE calendar_table
(
calendar_id TEXT,
uid TEXT,
link TEXT,
updated INTEGER,
name TEXT,
description TEXT,
author TEXT,
color TEXT,
hidden INTEGER,
selected INTEGER,
location TEXT,
locale INTEGER,
country INTEGER,
time_zone INTEGER,
timezone_label TEXT,
display_all_timezones INTEGER,
date_field_order INTEGER,
format_24hour_time INTEGER,
week_start INTEGER,
default_cal_mode INTEGER,
custom_cal_mode INTEGER,
user_location TEXT,
weather TEXT,
show_declined_events INTEGER,
hide_invitations INTEGER,
alternate_calendar INTEGER,
visibility INTEGER,
projection INTEGER,
sequence INTEGER,
suppress_reply_notifications INTEGER,
sync_event INTEGER,
times_cleaned INTEGER,
guests_can_modify INTEGER,
guests_can_invite_others INTEGER,
guests_can_see_guests INTEGER,
access_level INTEGER,
sync_status INTEGER,
is_deleted INTEGER,
account_id INTEGER,
sensitivity INTEGER,
store_type INTEGER
);

CREATE TABLE timezone_table
(
tz_offset_from_gmt INTEGER,
standard_name TEXT,
std_start_month INTEGER,
std_start_position_of_week INTEGER,
std_start_day INTEGER,
std_start_hour INTEGER,
standard_bias INTEGER,
day_light_name TEXT,
day_light_start_month INTEGER,
day_light_start_position_of_week INTEGER,
day_light_start_day INTEGER,
day_light_start_hour INTEGER,
day_light_bias INTEGER
);

CREATE TABLE cal_alarm_table
(
event_id INTEGER,
alarm_time INTEGER,
remind_tick INTEGER,
remind_tick_unit INTEGER,
alarm_tone TEXT,
alarm_description TEXT,
alarm_type INTEGER,
alarm_id INTEGER
);

INSERT INTO calendar_table VALUES(0,0,0,0,'default calendar',0,0,'76.198.86.255',0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,-1,0,3);
