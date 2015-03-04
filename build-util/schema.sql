--
-- Calendar Service
--
-- Copyright (c) 2012 - 2013 Samsung Electronics Co., Ltd. All rights reserved.
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
PRAGMA user_version = 105;

CREATE TABLE schedule_table
(
id INTEGER PRIMARY KEY AUTOINCREMENT,
type INTEGER,
summary TEXT,
description TEXT,
location TEXT,
categories TEXT,
exdate TEXT,
task_status INTEGER,
priority INTEGER,
timezone INTEGER DEFAULT 0,
contact_id INTEGER,
busy_status INTEGER,
sensitivity INTEGER,
uid TEXT,
organizer_name TEXT,
organizer_email TEXT,
meeting_status INTEGER,
calendar_id INTEGER,
original_event_id INTEGER DEFAULT -1,
latitude DOUBLE,
longitude DOUBLE,
email_id INTEGER,
created_time INTEGER,
completed_time INTEGER,
progress INTEGER,
changed_ver INTEGER,
created_ver INTEGER,
is_deleted INTEGER DEFAULT 0,
dtstart_type INTEGER,
dtstart_utime INTEGER,
dtstart_datetime DATE,
dtstart_tzid TEXT,
dtend_type INTEGER,
dtend_utime INTEGER,
dtend_datetime DATE,
dtend_tzid TEXT,
last_mod INTEGER,
rrule_id INTEGER DEFAULT 0,
recurrence_id TEXT,
rdate TEXT,
has_attendee INTEGER,
has_alarm INTEGER,
system_type INTEGER,
updated INTEGER,
sync_data1 TEXT,
sync_data2 TEXT,
sync_data3 TEXT,
sync_data4 TEXT,
has_exception INTEGER DEFAULT 0,
has_extended INTEGER DEFAULT 0,
freq INTEGER DEFAULT 0,
is_allday INTEGER DEFAULT 0
);
CREATE INDEX sch_idx1 ON schedule_table(type);
CREATE INDEX sch_idx2 ON schedule_table(calendar_id);
CREATE INDEX sch_idx3 ON schedule_table(sync_data4);

CREATE TRIGGER trg_original_mod AFTER UPDATE OF is_deleted ON schedule_table
 BEGIN
   DELETE FROM normal_instance_table WHERE event_id = (SELECT rowid FROM schedule_table WHERE original_event_id = old.id);
   DELETE FROM allday_instance_table WHERE event_id = (SELECT rowid FROM schedule_table WHERE original_event_id = old.id);
   UPDATE schedule_table SET is_deleted = 1 WHERE original_event_id = old.id;
 END;

CREATE TRIGGER trg_schedule_del AFTER DELETE ON schedule_table
 BEGIN
   DELETE FROM rrule_table WHERE event_id = old.id;
   DELETE FROM alarm_table WHERE event_id = old.id;
   DELETE FROM schedule_table WHERE original_event_id = old.id;
   DELETE FROM normal_instance_table WHERE event_id = old.id;
   DELETE FROM allday_instance_table WHERE event_id = old.id;
   DELETE FROM attendee_table WHERE event_id = old.id;
   DELETE FROM extended_table WHERE record_id = old.id AND record_type = 2;
   DELETE FROM extended_table WHERE record_id = old.id AND record_type = 3;
 END;

-- type + 1: is cal_record_type_e = cal_sch_type_e + 1
CREATE TRIGGER trg_schedule_del2 AFTER DELETE ON schedule_table
 WHEN old.is_deleted = 0 AND old.calendar_id = (SELECT id FROM calendar_table WHERE id = old.calendar_id)
 BEGIN
   INSERT INTO deleted_table VALUES(old.id, old.type + 1, old.calendar_id, (SELECT ver FROM version_table) + 1, old.created_ver, old.original_event_id);
 END;

CREATE TRIGGER trg_schedule_del3 AFTER DELETE ON schedule_table
 WHEN old.is_deleted = 1 AND old.calendar_id = (SELECT id FROM calendar_table WHERE id = old.calendar_id)
 BEGIN
   INSERT INTO deleted_table VALUES(old.id, old.type + 1, old.calendar_id, old.changed_ver, old.created_ver, old.original_event_id);
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
CREATE INDEX n_i_idx1 ON normal_instance_table(event_id, dtstart_utime);
--CREATE INDEX n_i_idx2 ON normal_instance_table(dtstart_utime);
CREATE INDEX n_i_idx3 ON normal_instance_table (dtend_utime, dtstart_utime);
CREATE INDEX n_i_idx4 ON normal_instance_table (dtstart_utime, dtend_utime);

CREATE TABLE allday_instance_table
(
event_id INTEGER,
dtstart_datetime DATE,
dtend_datetime DATE
);
CREATE INDEX a_i_idx1 ON allday_instance_table(event_id, dtstart_datetime);
--CREATE INDEX a_i_idx2 ON allday_instance_table(dtstart_datetime);
CREATE INDEX a_i_idx3 ON allday_instance_table (dtend_datetime, dtstart_datetime);
CREATE INDEX a_i_idx4 ON allday_instance_table (dtstart_datetime, dtend_datetime);

CREATE TABLE attendee_table
(
event_id INTEGER,
attendee_name TEXT,
attendee_email TEXT,
attendee_number TEXT,
attendee_status INTEGER,
attendee_ct_index INTEGER,
attendee_role INTEGER,
attendee_rsvp INTEGER,
attendee_group TEXT,
attendee_delegator_uri TEXT,
attendee_uid TEXT,
attendee_cutype INTEGER,
attendee_delegatee_uri TEXT,
attendee_member TEXT
);

CREATE INDEX attendee_idx1 on attendee_table(event_id);

CREATE TABLE calendar_table
(
id INTEGER PRIMARY KEY AUTOINCREMENT,
uid TEXT,
updated INTEGER,
name TEXT,
description TEXT,
color TEXT,
location TEXT,
visibility INTEGER,
sync_event INTEGER,
is_deleted INTEGER DEFAULT 0,
account_id INTEGER,
store_type INTEGER,
sync_data1 TEXT,
sync_data2 TEXT,
sync_data3 TEXT,
sync_data4 TEXT,
deleted INTEGER DEFAULT 0,
mode INTEGER DEFAULT 0,
owner_label TEXT
);

CREATE TABLE timezone_table
(
id INTEGER PRIMARY KEY AUTOINCREMENT,
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
day_light_bias INTEGER,
calendar_id INTEGER
);

CREATE TRIGGER trg_cal_del AFTER DELETE ON calendar_table
 BEGIN
   DELETE FROM timezone_table WHERE calendar_id = old.id;
   DELETE FROM schedule_table WHERE calendar_id = old.id;
   DELETE FROM extended_table WHERE record_id = old.id AND record_type = 1;
 END;

CREATE TABLE alarm_table
(
event_id INTEGER,
remind_tick INTEGER,
remind_tick_unit INTEGER,
alarm_description TEXT,
alarm_type INTEGER,
alarm_id INTEGER DEFAULT 0,
alarm_summary TEXT,
alarm_action INTEGER DEFAULT 0,
alarm_attach TEXT,
alarm_utime INTEGER,
alarm_datetime DATE
);

CREATE TABLE extended_table
(
id INTEGER PRIMARY KEY AUTOINCREMENT,
record_id INTEGER,
record_type INTEGER,
key TEXT,
value TEXT
);

CREATE TABLE deleted_table
(
schedule_id INTEGER,
schedule_type INTEGER,
calendar_id INTEGER,
deleted_ver INTEGER,
created_ver INTEGER,
original_event_id INTEGER DEFAULT -1
);
CREATE INDEX deleted_schedule_ver_idx ON deleted_table(deleted_ver);

CREATE TABLE version_table
(
ver INTEGER PRIMARY KEY,
contacts_ver INTEGER DEFAULT 0
);
INSERT INTO version_table VALUES(0, 0);

INSERT INTO calendar_table VALUES(1,0,0,'Default event calendar'   ,0,'224.167.79.255',0,1,1,0,-1,1,0,0,0,0,0,0,'calendar');
INSERT INTO calendar_table VALUES(2,0,0,'Default todo calendar'    ,0,'41.177.227.255',0,1,1,0,-1,2,0,0,0,0,0,0,'calendar');
INSERT INTO calendar_table VALUES(3,0,0,'Default birthday calendar',0,'141.17.27.255' ,0,1,0,0,-1,1,0,0,0,0,0,0,'calendar');
