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
summary TEXT,
description TEXT,
location TEXT,
categories TEXT,
exdate TEXT,
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
email_id INTEGER,
availability INTEGER,
created_time INTEGER,
completed_time INTEGER,
progress INTEGER,
changed_ver INTEGER,
created_ver INTEGER,
is_deleted INTEGER DEFAULT 0,
dtstart_type INTEGER,
dtstart_utime INTEGER,
dtstart_datetime TEXT,
dtstart_tzid TEXT,
dtend_type INTEGER,
dtend_utime INTEGER,
dtend_datetime TEXT,
dtend_tzid TEXT,
last_mod INTEGER,
rrule_id INTEGER DEFAULT 0
);
CREATE INDEX sch_idx1 ON schedule_table(type);
CREATE TRIGGER trg_sch_del AFTER DELETE ON schedule_table
 BEGIN
   DELETE FROM alarm_table WHERE event_id = old.id;
 END;

CREATE TABLE rrule_table
(
id INTEGER PRIMARY KEY AUTOINCREMENT,
event_id INTEGER,
freq INTEGER DEFAULT 0,
range_type INTEGER,
until_type INTEGER,
until_utime INTEGER,
until_datetime TEXT,
count INTEGER,
interval INTEGER,
bysecond TEXT,
byminute TEXT,
byhour TEXT,
byday TEXT,
bymonthday TEXT,
byyearday TEXT,
byweekno TEXT,
bymonth TEXT,
bysetpos TEXT,
wkst INTEGER
);

CREATE TABLE normal_instance_table
(
event_id INTEGER,
dtstart_utime INTEGER,
dtend_utime INTEGER
);

CREATE TABLE allday_instance_table
(
event_id INTEGER,
dtstart_datetime TEXT,
dtend_datetime TEXT
);

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

CREATE TABLE alarm_table
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

CREATE TABLE deleted_table
(
schedule_id INTEGER,
schedule_type INTEGER,
calendar_id INTEGER,
deleted_ver INTEGER
);
CREATE INDEX deleted_schedule_ver_idx ON deleted_table(deleted_ver);

CREATE TABLE version_table
(
ver INTEGER PRIMARY KEY
);
INSERT INTO version_table VALUES(0);

INSERT INTO calendar_table VALUES(0,0,0,0,'Default event calendar',0,0,'224.167.79.255',0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,-1,0,3);
INSERT INTO calendar_table VALUES(0,0,0,0,'Default todo calendar',0,0,'224.167.79.255',0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,-1,0,3);
