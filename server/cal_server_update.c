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

#include <sqlite3.h>
#include <db-util.h>
#include <stdlib.h>

#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_db.h"

#define __USER_VERSION 105

static int _cal_server_update_get_db_version(sqlite3 *db, int *version)
{
	int ret = CALENDAR_ERROR_NONE;
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	sqlite3_stmt *stmt = NULL;

	snprintf(query, sizeof(query), "PRAGMA user_version;");
	ret = sqlite3_prepare_v2(db, query, strlen(query), &stmt, NULL);
	if (SQLITE_OK != ret)
	{
		ERR("sqlite3_prepare_v2() failed[%s]", sqlite3_errmsg(db));
		return CALENDAR_ERROR_DB_FAILED;
	}
	ret = sqlite3_step(stmt);
	if (SQLITE_ROW != ret)
	{
		ERR("sqlite3_step() failed[%s]", sqlite3_errmsg(db));
		sqlite3_finalize(stmt);
		return CALENDAR_ERROR_DB_FAILED;
	}
	if (version)
		*version = (int)sqlite3_column_int(stmt, 0);
	sqlite3_finalize(stmt);
	return CALENDAR_ERROR_NONE;
}

int cal_server_update(void)
{
	CAL_FN_CALL();

	int ret = CALENDAR_ERROR_NONE;
	int old_version = 0;
	char *errmsg = NULL;
	sqlite3 *__db;
	char query[CAL_DB_SQL_MAX_LEN] = {0};

	char db_file[256] = {0};
	snprintf(db_file, sizeof(db_file), "%s/%s", DB_PATH, CALS_DB_NAME);
	ret = db_util_open(db_file, &__db, 0);
	if (SQLITE_OK != ret) {
		ERR("db_util_open() fail(%d):[%s]", ret, db_file);
		return CALENDAR_ERROR_DB_FAILED;
	}
	_cal_server_update_get_db_version(__db, &old_version);
	DBG("[%s] old version(%d)", db_file, old_version);

	if (old_version < 100)
	{
		/* ----------------------- start modified 2013/08/22
		 * added attendee_table(cutype, delegatee_uri, member), alarm_table(summary, action, attach).
		 */
		ret = sqlite3_exec(__db, "DROP VIEW event_calendar_attendee_view", NULL, 0, &errmsg);
		if (SQLITE_OK != ret)
		{
			ERR("sqlite3_exec() failed: DROP VIEW event_calendar_attendee_view(%d) [%s]", ret, errmsg);
			sqlite3_free(errmsg);
		}

		ret = sqlite3_exec(__db, "ALTER TABLE attendee_table ADD COLUMN attendee_cutype INTEGER", NULL, 0, &errmsg);
		if (SQLITE_OK != ret)
		{
			ERR("sqlite3_exec() failed: ALTER TABLE attendee_table ADD COLUMN attendee_cutype(%d) [%s]", ret, errmsg);
			sqlite3_free(errmsg);
		}
		ret = sqlite3_exec(__db, "ALTER TABLE attendee_table ADD COLUMN attendee_delegatee_uri TEXT", NULL, 0, &errmsg);
		if (SQLITE_OK != ret)
		{
			ERR("sqlite3_exec() failed: ALTER TABLE attendee_table ADD COLUMN attendee_delegatee_uri(%d) [%s]", ret, errmsg);
			sqlite3_free(errmsg);
		}
		ret = sqlite3_exec(__db, "ALTER TABLE attendee_table ADD COLUMN attendee_member TEXT", NULL, 0, &errmsg);
		if (SQLITE_OK != ret)
		{
			ERR("sqlite3_exec() failed: ALTER TABLE attendee_table ADD COLUMN attendee_member(%d) [%s]", ret, errmsg);
			sqlite3_free(errmsg);
		}

		ret = sqlite3_exec(__db, "ALTER TABLE alarm_table ADD COLUMN alarm_summary TEXT", NULL, 0, &errmsg);
		if (SQLITE_OK != ret)
		{
			ERR("sqlite3_exec() failed: ALTER TABLE alarm_table ADD COLUMN alarm_summary(%d) [%s]", ret, errmsg);
			sqlite3_free(errmsg);
		}
		ret = sqlite3_exec(__db, "ALTER TABLE alarm_table ADD COLUMN alarm_action INTEGER", NULL, 0, &errmsg);
		if (SQLITE_OK != ret)
		{
			ERR("sqlite3_exec() failed: ALTER TABLE alarm_table ADD COLUMN alarm_action(%d) [%s]", ret, errmsg);
			sqlite3_free(errmsg);
		}
		ret = sqlite3_exec(__db, "ALTER TABLE alarm_table ADD COLUMN alarm_attach TEXT", NULL, 0, &errmsg);
		if (SQLITE_OK != ret)
		{
			ERR("sqlite3_exec() failed: ALTER TABLE alarm_table ADD COLUMN alarm_attach(%d) [%s]", ret, errmsg);
			sqlite3_free(errmsg);
		}
		old_version = 100;
		/* ----------------------- end modified 2013/08/22
		 */
	}
	if (old_version == 100)
	{
		/* ----------------------- start modified 2013/09/22
		 * added schedule_table(freq) for view table parameter.
		 */
		ret = sqlite3_exec(__db, "ALTER TABLE schedule_table ADD COLUMN freq INTEGER DEFAULT 0", NULL, 0, &errmsg);
		if (SQLITE_OK != ret)
		{
			ERR("sqlite3_exec() failed: ALTER TABLE schedule_table ADD COLUMN freq(%d) [%s]", ret, errmsg);
			sqlite3_free(errmsg);
		}
		old_version = 101;
		/* ----------------------- end modified 2013/09/22
		 */
	}
	if (old_version == 101)
	{
		/* ----------------------- start modified 2014/07/02
		 * added trigger depeding on schedule_table
		 * added original_event_id in deleted_table to check exception event.
		 */

		// rename trig -> trg
		ret = sqlite3_exec(__db, "DROP TRIGGER trig_original_mod", NULL, 0, &errmsg);
		if (SQLITE_OK != ret) {
			ERR("DROP TRIGGER trig_original_mod failed(%d:%s)", ret, errmsg);
			sqlite3_free(errmsg);
		}
		ret = sqlite3_exec(__db,
				"CREATE TRIGGER trg_original_mod AFTER UPDATE OF is_deleted ON schedule_table "
				" BEGIN "
				"   DELETE FROM normal_instance_table WHERE event_id = (SELECT rowid FROM schedule_table WHERE original_event_id = old.id);"
				"   DELETE FROM allday_instance_table WHERE event_id = (SELECT rowid FROM schedule_table WHERE original_event_id = old.id);"
				"   UPDATE schedule_table SET is_deleted = 1 WHERE original_event_id = old.id;"
				" END;",
				NULL, 0, &errmsg);
		if (SQLITE_OK != ret) {
			ERR("CREATE TRIGGER trg_original_mod failed(%d:%s)", ret, errmsg);
			sqlite3_free(errmsg);
		}

		// rename trg_sch_del -> trg_schedule_del
		ret = sqlite3_exec(__db, "DROP TRIGGER trg_sch_del", NULL, 0, &errmsg);
		if (SQLITE_OK != ret) {
			ERR("DROP TRIGGER trg_sch_del failed(%d:%s)", ret, errmsg);
			sqlite3_free(errmsg);
		}
		ret = sqlite3_exec(__db,
				"CREATE TRIGGER trg_schedule_del AFTER DELETE ON schedule_table "
				"BEGIN "
				"  DELETE FROM rrule_table WHERE event_id = old.id;"
				"  DELETE FROM alarm_table WHERE event_id = old.id;"
				"  DELETE FROM schedule_table WHERE original_event_id = old.id;"
				"  DELETE FROM normal_instance_table WHERE event_id = old.id;"
				"  DELETE FROM allday_instance_table WHERE event_id = old.id;"
				"  DELETE FROM attendee_table WHERE event_id = old.id;"
				"  DELETE FROM extended_table WHERE record_id = old.id AND record_type = 2;"
				"  DELETE FROM extended_table WHERE record_id = old.id AND record_type = 3;"
				"END;",
				NULL, 0, &errmsg);
		if (SQLITE_OK != ret) {
			ERR("CREATE TRIGGER trg_schedule_del failed(%d:%s)", ret, errmsg);
			sqlite3_free(errmsg);
		}

		// add trigger
		ret = sqlite3_exec(__db,
				"CREATE TRIGGER trg_schedule_del2 AFTER DELETE ON schedule_table "
				" WHEN old.is_deleted = 0 AND old.calendar_id = (SELECT id FROM calendar_table WHERE id = old.calendar_id) "
				" BEGIN "
				"   INSERT INTO deleted_table VALUES(old.id, old.type + 1, old.calendar_id, (SELECT ver FROM version_table) + 1, old.created_ver, old.original_event_id);"
				" END;",
				NULL, 0, &errmsg);
		if (SQLITE_OK != ret) {
			ERR("CREATE TRIGGER trg_schedule_del2 failed(%d:%s)", ret, errmsg);
			sqlite3_free(errmsg);
		}

		// add trigger
		ret = sqlite3_exec(__db,
				"CREATE TRIGGER trg_schedule_del3 AFTER DELETE ON schedule_table "
				" WHEN old.is_deleted = 1 AND old.calendar_id = (SELECT id FROM calendar_table WHERE id = old.calendar_id) "
				" BEGIN "
				"   INSERT INTO deleted_table VALUES(old.id, old.type + 1, old.calendar_id, old.changed_ver, old.created_ver, old.original_event_id);"
				" END;",
				NULL, 0, &errmsg);
		if (SQLITE_OK != ret) {
			ERR("CREATE TRIGGER trg_schedule_del3 failed(%d:%s)", ret, errmsg);
			sqlite3_free(errmsg);
		}

		// add field: original_event_id in deleted_table
		ret = sqlite3_exec(__db, "ALTER TABLE deleted_table ADD COLUMN original_event_id INTEGER", NULL, 0, &errmsg);
		if (SQLITE_OK != ret) {
			ERR("ALTER TABLE deleted_table failed(%d:%s)", ret, errmsg);
			sqlite3_free(errmsg);
		}
		/* ----------------------- end modified 2014/07/02
		 */
		old_version = 102;
	}
	if (old_version == 102)
	{
		/* ----------------------- start modified 2014/10/24
		 * added field is_alldy on schedule_table
		 */

		// add field: is_allday in deleted_table
		ret = sqlite3_exec(__db, "ALTER TABLE schedule_table ADD COLUMN is_allday INTEGER DEFAULT 0", NULL, 0, &errmsg);
		if (SQLITE_OK != ret) {
			ERR("ALTER TABLE schedule_table failed(%d:%s)", ret, errmsg);
			sqlite3_free(errmsg);
		}
		/* ----------------------- end modified 2014/10/24
		 */
		old_version = 103;
	}

	if (old_version == 103)
	{
		ret = sqlite3_exec(__db, "DROP TABLE reminder_table", NULL, 0, &errmsg);
		if (SQLITE_OK != ret) {
			ERR("DROP TABLE reminder_table failed(%d:%s)", ret, errmsg);
			sqlite3_free(errmsg);
		}
		old_version = 104;
	}
	if (old_version == 104)
	{
		ret = sqlite3_exec(__db, "ALTER TABLE alarm_table ADD COLUMN alarm_utime INTEGER", NULL, 0, &errmsg);
		if (SQLITE_OK != ret) {
			ERR("ALTER TABLE schedule_table failed(%d:%s)", ret, errmsg);
			sqlite3_free(errmsg);
		}
		ret = sqlite3_exec(__db, "ALTER TABLE alarm_table ADD COLUMN alarm_datetime DATE", NULL, 0, &errmsg);
		if (SQLITE_OK != ret) {
			ERR("ALTER TABLE schedule_table failed(%d:%s)", ret, errmsg);
			sqlite3_free(errmsg);
		}
		ret = sqlite3_exec(__db, "DROP VIEW allday_instance_view", NULL, 0, &errmsg);
		if (SQLITE_OK != ret)
		{
			ERR("sqlite3_exec() failed: DROP VIEW allday_instance_view(%d) [%s]", ret, errmsg);
			sqlite3_free(errmsg);
		}
		ret = sqlite3_exec(__db, "DROP VIEW allday_instance_view_extended", NULL, 0, &errmsg);
		if (SQLITE_OK != ret)
		{
			ERR("sqlite3_exec() failed: DROP VIEW allday_instance_view_extended(%d) [%s]", ret, errmsg);
			sqlite3_free(errmsg);
		}
		old_version = 105;
	}

	// update DB user_version
	snprintf(query, sizeof(query), "PRAGMA user_version = %d", __USER_VERSION);
	ret = sqlite3_exec(__db, query, NULL, 0, &errmsg);
	if (SQLITE_OK != ret)
	{
		ERR("sqlite3_exec() failed(%d) [%s]", ret, errmsg);
		sqlite3_free(errmsg);
	}
	db_util_close(__db);
	__db = NULL;

	return CALENDAR_ERROR_NONE;
}


