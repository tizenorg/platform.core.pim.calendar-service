/*
 * Calendar Service
 *
 * Copyright (c) 2000 - 2012 Samsung Electronics Co., Ltd. All rights reserved.
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
#ifndef __CALENDAR_SVC_DB_INFO_H__
#define __CALENDAR_SVC_DB_INFO_H__

#define CALS_DB_PATH "/opt/dbspace/.calendar-svc.db"
#define CALS_DB_JOURNAL_PATH "/opt/dbspace/.calendar-svc.db-journal"

// For Security
#define CALS_SECURITY_FILE_GROUP 6003
#define CALS_SECURITY_DEFAULT_PERMISSION 0660
#define CALS_SECURITY_DIR_DEFAULT_PERMISSION 0770

#define CALS_TABLE_SCHEDULE "schedule_table"
#define CALS_TABLE_ALARM "alarm_table"
#define CALS_TABLE_CALENDAR "calendar_table"
#define CALS_TABLE_PARTICIPANT "cal_participant_table"
#define CALS_TABLE_TIMEZONE "timezone_table"
#define CALS_TABLE_VERSION "version_table"
#define CALS_TABLE_DELETED "deleted_table"
#define CALS_TABLE_RRULE "rrule_table"
#define CALS_TABLE_NORMAL_INSTANCE "normal_instance_table"
#define CALS_TABLE_ALLDAY_INSTANCE "allday_instance_table"

#endif /* __CALENDAR_SVC_DB_INFO_H__ */

