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
#include "cals-internal.h"
#include "cals-typedef.h"
#include "cals-sqlite.h"
#include "cals-db-info.h"
#include "cals-utils.h"
#include "cals-alarm.h"
#include "cals-calendar.h"


int cals_insert_calendar(const calendar_t *calendar)
{
	int ret;
	sqlite3_stmt *stmt;
	char query[CALS_SQL_MAX_LEN];

	retv_if(NULL == calendar, CAL_ERR_ARG_NULL);

	sprintf(query,"INSERT INTO %s(calendar_id,uid,link,updated,name,description,author,"
			"color,hidden,selected,location,locale,country,time_zone,timezone_label,display_all_timezones,"
			"date_field_order,format_24hour_time,week_start,default_cal_mode,custom_cal_mode,user_location,"
			"weather,show_declined_events,hide_invitations,alternate_calendar,visibility,projection,"
			"sequence,suppress_reply_notifications,sync_event,times_cleaned,guests_can_modify,"
			"guests_can_invite_others,guests_can_see_guests,access_level,sync_status,account_id,sensitivity,store_type) "
			"VALUES( ?, ?, ?, %ld, ?, ?, ?, ?, %d, %d, ?, %d, %d, %ld, ?, %d, %d, %d, "
			"%d, %d, %d, ?, ?, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d)",
			CALS_TABLE_CALENDAR,
			//calendar->calendar_id,
			//calendar->uid,
			//calendar->link,
			calendar->updated,
			//calendar->name,
			//calendar->description,
			//calendar->author,
			//calendar->color,
			calendar->hidden,
			calendar->selected,
			//calendar->location,
			calendar->locale,
			calendar->country,
			calendar->time_zone,
			//calendar->timezone_label,
			calendar->display_all_timezones,
			calendar->date_field_order,
			calendar->format_24hour_time,
			calendar->week_start,
			calendar->default_cal_mode,
			calendar->custom_cal_mode,
			//calendar->user_location,
			//calendar->weather,
			calendar->show_declined_events,
			calendar->hide_invitations,
			calendar->alternate_calendar,
			calendar->visibility,
			calendar->projection,
			calendar->sequence,
			calendar->suppress_reply_notifications,
			calendar->sync_event,
			calendar->times_cleaned,
			calendar->guests_can_modify,
			calendar->guests_can_invite_others,
			calendar->guests_can_see_guests,
			calendar->access_level,
			calendar->sync_status,
			calendar->account_id,
			calendar->sensitivity,
			calendar->store_type);

	stmt = cals_query_prepare(query);
	retvm_if(NULL == stmt, CAL_ERR_DB_FAILED, "cals_query_prepare() Failed");

	if (calendar->calendar_id)
		cals_stmt_bind_text(stmt, 1, calendar->calendar_id);

	if (calendar->uid)
		cals_stmt_bind_text(stmt, 2, calendar->uid);

	if (calendar->link)
		cals_stmt_bind_text(stmt, 3, calendar->link);

	if (calendar->name)
		cals_stmt_bind_text(stmt, 4, calendar->name);

	if (calendar->description)
		cals_stmt_bind_text(stmt, 5, calendar->description);

	if (calendar->author)
		cals_stmt_bind_text(stmt, 6, calendar->author);

	if (calendar->color)
		cals_stmt_bind_text(stmt, 7, calendar->color);

	if (calendar->location)
		cals_stmt_bind_text(stmt, 8, calendar->location);

	if (calendar->timezone_label)
		cals_stmt_bind_text(stmt, 9, calendar->timezone_label);

	if (calendar->user_location)
		cals_stmt_bind_text(stmt, 10, calendar->user_location);

	if (calendar->weather)
		cals_stmt_bind_text(stmt, 11, calendar->weather);

	ret = cals_stmt_step(stmt);
	if (CAL_SUCCESS != ret) {
		sqlite3_finalize(stmt);
		ERR("cals_stmt_step() Failed(%d)", ret);
		return ret;
	}

	ret = cals_last_insert_id();
	sqlite3_finalize(stmt);

	cals_notify(CALS_NOTI_TYPE_CALENDAR);

	return ret;
}

int cals_update_calendar(const calendar_t *calendar)
{
	int rc = -1;
	char query[CALS_SQL_MAX_LEN] = {0};
	sqlite3_stmt *stmt = NULL;

	retv_if(NULL == calendar, CAL_ERR_ARG_NULL);

	snprintf(query, sizeof(query), "UPDATE %s SET "
			"calendar_id = ?,"
			"link = ?,"
			"updated = %ld,"
			"name = ?,"
			"description = ?,"
			"author = ?,"
			"color = ?,"
			"hidden= %d,"
			"selected = %d,"
			"location = ?,"
			"locale = %d,"
			"country = %d,"
			"time_zone = %ld,"
			"timezone_label = ?,"
			"display_all_timezones = %d,"
			"date_field_order = %d,"
			"format_24hour_time = %d,"
			"week_start = %d,"
			"default_cal_mode = %d,"
			"custom_cal_mode = %d,"
			"user_location = ?,"
			"weather = ?,"
			"show_declined_events = %d,"
			"hide_invitations = %d,"
			"alternate_calendar = %d,"
			"visibility = %d,"
			"projection = %d,"
			"sequence = %d,"
			"suppress_reply_notifications = %d,"
			"sync_event = %d,"
			"times_cleaned = %d,"
			"guests_can_modify = %d,"
			"guests_can_invite_others = %d,"
			"guests_can_see_guests = %d,"
			"access_level = %d,"
			"sync_status = %d,"
			"account_id = %d,"
			"sensitivity = %d, "
			"store_type = %d "
			"WHERE rowid = %d",
		CALS_TABLE_CALENDAR,
		calendar->updated,
		calendar->hidden,
		calendar->selected,
		calendar->locale,
		calendar->country,
		calendar->time_zone,
		calendar->display_all_timezones,
		calendar->date_field_order,
		calendar->format_24hour_time,
		calendar->week_start,
		calendar->default_cal_mode,
		calendar->custom_cal_mode,
		calendar->show_declined_events,
		calendar->hide_invitations,
		calendar->alternate_calendar,
		calendar->visibility,
		calendar->projection,
		calendar->sequence,
		calendar->suppress_reply_notifications,
		calendar->sync_event,
		calendar->times_cleaned,
		calendar->guests_can_modify,
		calendar->guests_can_invite_others,
		calendar->guests_can_see_guests,
		calendar->access_level,
		calendar->sync_status,
		calendar->account_id,
		calendar->sensitivity,
		calendar->store_type,
		calendar->index);

	stmt = cals_query_prepare(query);
	retvm_if(NULL == stmt, CAL_ERR_DB_FAILED, "cals_query_prepare() Failed");

	if (calendar->calendar_id)
		cals_stmt_bind_text(stmt, 1, calendar->calendar_id);

	if (calendar->link)
		cals_stmt_bind_text(stmt, 2, calendar->link);

	if (calendar->name)
		cals_stmt_bind_text(stmt, 3, calendar->name);

	if (calendar->description)
		cals_stmt_bind_text(stmt, 4, calendar->description);

	if (calendar->author)
		cals_stmt_bind_text(stmt, 5, calendar->author);

	if (calendar->color)
		cals_stmt_bind_text(stmt, 6, calendar->color);

	if (calendar->location)
		cals_stmt_bind_text(stmt, 7, calendar->location);

	if (calendar->timezone_label)
		cals_stmt_bind_text(stmt, 8, calendar->timezone_label);

	if (calendar->user_location)
		cals_stmt_bind_text(stmt, 9, calendar->user_location);

	if (calendar->weather)
		cals_stmt_bind_text(stmt, 10, calendar->weather);

	rc = cals_stmt_step(stmt);
	if (CAL_SUCCESS != rc) {
		sqlite3_finalize(stmt);
		ERR("cals_stmt_step() Failed(%d)", rc);
		return rc;
	}

	sqlite3_finalize(stmt);

	cals_notify(CALS_NOTI_TYPE_CALENDAR);

	return CAL_SUCCESS;
}

int cals_delete_calendar(int index)
{
	int ret = 0;
	char query[CALS_SQL_MAX_LEN] = {0};

	ret = cals_begin_trans();
	retvm_if(CAL_SUCCESS != ret, ret, "cals_begin_trans() Failed(%d)", ret);

	sprintf(query,"DELETE FROM %s WHERE rowid = %d", CALS_TABLE_CALENDAR, index);
	ret = cals_query_exec(query);
	if(CAL_SUCCESS != ret) {
		ERR("cals_query_exec() Failed(%d)", ret);
		cals_end_trans(false);
		return ret;
	}

	time_t current_time = time(NULL);

	sprintf(query,"UPDATE %s SET is_deleted = 1, sync_status = %d, last_modified_time = %ld WHERE calendar_id = %d",
		CALS_TABLE_SCHEDULE, CAL_SYNC_STATUS_DELETED,(long int)current_time, index);

	ret = cals_query_exec(query);
	if (CAL_SUCCESS != ret) {
		ERR("cals_query_exec() Failed(%d)", ret);
		cals_end_trans(false);
		return ret;
	}
	ret = cals_alarm_remove(CALS_ALARM_REMOVE_BY_CALENDAR_ID, index);
	if (CAL_SUCCESS != ret) {
		ERR("cals_alarm_remove() Failed(%d)", ret);
		cals_end_trans(false);
		return ret;
	}

	cals_end_trans(true);

	cals_notify(CALS_NOTI_TYPE_CALENDAR);
	return CAL_SUCCESS;
}


int cals_delete_calendars(int account_id)
{
	int ret = 0;
	char query[CALS_SQL_MIN_LEN] = {0};

	if (account_id)
		sprintf(query,"DELETE FROM calendar_table WHERE account_id = %d", account_id);
	else
		sprintf(query,"DELETE FROM calendar_table");

	ret = cals_query_exec(query);
	retvm_if(CAL_SUCCESS != ret, ret, "cals_query_exec() Failed(%d)", ret);

	cals_notify(CALS_NOTI_TYPE_CALENDAR);
	return CAL_SUCCESS;
}


void cals_stmt_get_calendar(sqlite3_stmt *stmt,calendar_t *record)
{
	int count = 0;
	const unsigned char *temp;

	record->index = sqlite3_column_int(stmt, count++);

	temp = sqlite3_column_text(stmt, count++);
	record->calendar_id = SAFE_STRDUP(temp);

	temp = sqlite3_column_text(stmt, count++);
	record->uid = SAFE_STRDUP(temp);

	temp = sqlite3_column_text(stmt, count++);
	record->link = SAFE_STRDUP(temp);

	record->updated = sqlite3_column_int(stmt, count++);

	temp = sqlite3_column_text(stmt, count++);
	record->name = SAFE_STRDUP(temp);

	temp = sqlite3_column_text(stmt, count++);
	record->description = SAFE_STRDUP(temp);

	temp = sqlite3_column_text(stmt, count++);
	record->author = SAFE_STRDUP(temp);

	temp = sqlite3_column_text(stmt, count++);
	record->color = SAFE_STRDUP(temp);

	record->hidden = sqlite3_column_int(stmt, count++);
	record->selected = sqlite3_column_int(stmt, count++);

	temp = sqlite3_column_text(stmt, count++);
	record->location = SAFE_STRDUP(temp);

	record->locale = sqlite3_column_int(stmt, count++);
	record->country = sqlite3_column_int(stmt, count++);
	record->time_zone = sqlite3_column_int(stmt, count++);

	temp = sqlite3_column_text(stmt, count++);
	record->timezone_label = SAFE_STRDUP(temp);

	record->display_all_timezones = sqlite3_column_int(stmt, count++);
	record->date_field_order = sqlite3_column_int(stmt, count++);
	record->format_24hour_time = sqlite3_column_int(stmt, count++);
	record->week_start = sqlite3_column_int(stmt, count++);
	record->default_cal_mode = sqlite3_column_int(stmt, count++);
	record->custom_cal_mode = sqlite3_column_int(stmt, count++);

	temp = sqlite3_column_text(stmt, count++);
	record->user_location = SAFE_STRDUP(temp);

	temp = sqlite3_column_text(stmt, count++);
	record->weather = SAFE_STRDUP(temp);

	record->show_declined_events = sqlite3_column_int(stmt, count++);
	record->hide_invitations = sqlite3_column_int(stmt, count++);
	record->alternate_calendar = sqlite3_column_int(stmt, count++);
	record->visibility = sqlite3_column_int(stmt, count++);
	record->projection = sqlite3_column_int(stmt, count++);
	record->sequence = sqlite3_column_int(stmt, count++);
	record->suppress_reply_notifications = sqlite3_column_int(stmt, count++);
	record->sync_event = sqlite3_column_int(stmt, count++);
	record->times_cleaned = sqlite3_column_int(stmt, count++);
	record->guests_can_modify = sqlite3_column_int(stmt, count++);
	record->guests_can_invite_others = sqlite3_column_int(stmt, count++);
	record->guests_can_see_guests = sqlite3_column_int(stmt, count++);
	record->access_level = sqlite3_column_int(stmt, count++);
	record->sync_status = sqlite3_column_int(stmt, count++);
	record->is_deleted = sqlite3_column_int(stmt, count++);
	record->account_id = sqlite3_column_int(stmt, count++);
	record->sensitivity = sqlite3_column_int(stmt, count++);
	record->store_type = sqlite3_column_int(stmt, count++);
}

int cals_rearrage_calendar_field(const char *src, char *dest, int dest_size)
{
	int ret = 0;
	if (strstr(src,CAL_TABLE_TXT_CALENDAR_ID))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_TABLE_TXT_CALENDAR_ID);

	if (strstr(src,CAL_TABLE_TXT_UID))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_TABLE_TXT_UID);

	if (strstr(src,CAL_TABLE_TXT_LINK))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_TABLE_TXT_LINK);

	if (strstr(src,CAL_TABLE_INT_UPDATED))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_TABLE_INT_UPDATED);

	if (strstr(src,CAL_TABLE_TXT_NAME))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_TABLE_TXT_NAME);

	if (strstr(src,CAL_TABLE_TXT_DESCRIPTION))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_TABLE_TXT_DESCRIPTION);

	if(strstr(src,CAL_TABLE_TXT_AUTHOR))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_TABLE_TXT_AUTHOR);

	if(strstr(src,CAL_TABLE_TXT_COLOR))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_TABLE_TXT_COLOR);

	if(strstr(src, CAL_TABLE_INT_HIDDEN))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_TABLE_INT_HIDDEN);

	if(strstr(src, CAL_TABLE_INT_SELECTED))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_TABLE_INT_SELECTED);

	if(strstr(src, CAL_TABLE_TXT_LOCATION))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_TABLE_TXT_LOCATION);

	if(strstr(src, CAL_TABLE_INT_LOCALE))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_TABLE_INT_LOCALE);

	if(strstr(src,CAL_TABLE_INT_COUNTRY))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_TABLE_INT_COUNTRY);

	if(strstr(src,CAL_TABLE_INT_TIME_ZONE))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_TABLE_INT_TIME_ZONE);

	if(strstr(src,CAL_TABLE_TXT_TIME_ZONE_LABEL))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_TABLE_TXT_TIME_ZONE_LABEL);

	if(strstr(src,CAL_TABLE_INT_DISPLAY_ALL_TIMEZONES))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_TABLE_INT_DISPLAY_ALL_TIMEZONES);

	if(strstr(src,CAL_TABLE_INT_DATE_FIELD_ORDER))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_TABLE_INT_DATE_FIELD_ORDER);

	if(strstr(src,CAL_TABLE_INT_FROMAT_24HOUR_TIME))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_TABLE_INT_FROMAT_24HOUR_TIME);

	if(strstr(src,CAL_TABLE_INT_WEEK_START))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_TABLE_INT_WEEK_START);

	if(strstr(src,CAL_TABLE_INT_DEFAULT_CAL_MODE))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_TABLE_INT_DEFAULT_CAL_MODE);

	if(strstr(src,CAL_TABLE_INT_CUSTOM_CAL_MODE))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_TABLE_INT_CUSTOM_CAL_MODE);

	if(strstr(src,CAL_TABLE_TXT_USER_LOCATION))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_TABLE_TXT_USER_LOCATION);

	if(strstr(src,CAL_TABLE_TXT_WEATHER))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_TABLE_TXT_WEATHER);

	if(strstr(src,CAL_TABLE_INT_SHOW_DECLINED_EVENTS))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_TABLE_INT_SHOW_DECLINED_EVENTS);

	if(strstr(src,CAL_TABLE_INT_HIDE_INVITATIONS))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_TABLE_INT_HIDE_INVITATIONS);

	if(strstr(src,CAL_TABLE_INT_ALTERNATE_CALENDAR))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_TABLE_INT_ALTERNATE_CALENDAR);

	if(strstr(src,CAL_TABLE_INT_VISIBILITY))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_TABLE_INT_VISIBILITY);

	if(strstr(src,CAL_TABLE_INT_PROJECTION))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_TABLE_INT_PROJECTION);

	if(strstr(src,CAL_TABLE_INT_SEQUENCE))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_TABLE_INT_SEQUENCE);

	if(strstr(src,CAL_TABLE_INT_SUPRESS_REPLY_NOTIFICATIONS))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_TABLE_INT_SUPRESS_REPLY_NOTIFICATIONS);

	if(strstr(src, CAL_TABLE_INT_SYNC_EVENT))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_TABLE_INT_SYNC_EVENT);

	if(strstr(src,CAL_TABLE_INT_TIMES_CLEANED))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_TABLE_INT_TIMES_CLEANED);

	if(strstr(src,CAL_TABLE_INT_GUESTS_CAN_MODIFY))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_TABLE_INT_GUESTS_CAN_MODIFY);

	if(strstr(src,CAL_TABLE_INT_GUESTS_CAN_INVITE_OTHERS))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_TABLE_INT_GUESTS_CAN_INVITE_OTHERS);

	if(strstr(src,CAL_TABLE_INT_GUESTS_CAN_SEE_GUESTS))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_TABLE_INT_GUESTS_CAN_SEE_GUESTS);

	if(strstr(src,CAL_TABLE_INT_ACCESS_LEVEL))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_TABLE_INT_ACCESS_LEVEL);

	if(strstr(src,CAL_TABLE_INT_SYNC_STATUS))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_TABLE_INT_SYNC_STATUS);

	if(strstr(src,CAL_TABLE_INT_IS_DELETED))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_TABLE_INT_IS_DELETED);

	if(strstr(src,CAL_TABLE_INT_ACCOUNT_ID))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_TABLE_INT_ACCOUNT_ID);

	if(strstr(src,CAL_TABLE_INT_SENSITIVITY))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_TABLE_INT_SENSITIVITY);

	if(strstr(src, CAL_TABLE_INT_STORE_TYPE))
		ret += snprintf(dest+ret, dest_size-ret, ",%s", CAL_TABLE_INT_STORE_TYPE);

	return CAL_SUCCESS;
}

int cals_stmt_get_filted_calendar(sqlite3_stmt *stmt,
	calendar_t *record, const char *select_field)
{
	int count = 0;
	const unsigned char *temp;
	const char *start, *result;

	retv_if(NULL == stmt, CAL_ERR_ARG_NULL);
	retv_if(NULL == record, CAL_ERR_ARG_NULL);
	retv_if(NULL == select_field, CAL_ERR_ARG_NULL);

	record->index = sqlite3_column_int(stmt, count++);

	start = select_field;
	if((result = strstr(start, CAL_TABLE_TXT_CALENDAR_ID))) {
		temp = sqlite3_column_text(stmt, count++);
		record->calendar_id = SAFE_STRDUP(temp);
		start = result;
	}
	if((result = strstr(start, CAL_TABLE_TXT_UID))) {
		temp = sqlite3_column_text(stmt, count++);
		record->uid = SAFE_STRDUP(temp);
		start = result;
	}
	if((result = strstr(start, CAL_TABLE_TXT_LINK))) {
		temp = sqlite3_column_text(stmt, count++);
		record->link = SAFE_STRDUP(temp);
		start = result;
	}
	if((result = strstr(start, CAL_TABLE_INT_UPDATED))) {
		record->updated = sqlite3_column_int(stmt, count++);
		start = result;
	}
	if((result = strstr(start, CAL_TABLE_TXT_NAME))) {
		temp = sqlite3_column_text(stmt, count++);
		record->name = SAFE_STRDUP(temp);
		start = result;
	}
	if((result = strstr(start, CAL_TABLE_TXT_DESCRIPTION))) {
		temp = sqlite3_column_text(stmt, count++);
		record->description = SAFE_STRDUP(temp);
		start = result;
	}
	if((result = strstr(start, CAL_TABLE_TXT_AUTHOR))) {
		temp = sqlite3_column_text(stmt, count++);
		record->author = SAFE_STRDUP(temp);
		start = result;
	}
	if((result = strstr(start, CAL_TABLE_TXT_COLOR))) {
		temp = sqlite3_column_text(stmt, count++);
		record->color = SAFE_STRDUP(temp);
		start = result;
	}
	if((result = strstr(start, CAL_TABLE_INT_HIDDEN))) {
		record->hidden = sqlite3_column_int(stmt, count++);
		start = result;
	}
	if((result = strstr(start, CAL_TABLE_INT_SELECTED))) {
		record->selected = sqlite3_column_int(stmt, count++);
		start = result;
	}
	if((result = strstr(start, CAL_TABLE_TXT_LOCATION))) {
		temp = sqlite3_column_text(stmt, count++);
		record->location = SAFE_STRDUP(temp);
		start = result;
	}
	if((result = strstr(start, CAL_TABLE_INT_LOCALE))) {
		record->locale = sqlite3_column_int(stmt, count++);
		start = result;
	}
	if((result = strstr(start, CAL_TABLE_INT_COUNTRY))) {
		record->country = sqlite3_column_int(stmt, count++);
		start = result;
	}
	if((result = strstr(start, CAL_TABLE_INT_TIME_ZONE))) {
		record->time_zone = sqlite3_column_int(stmt, count++);
		start = result;
	}
	if((result = strstr(start, CAL_TABLE_TXT_TIME_ZONE_LABEL))) {
		temp = sqlite3_column_text(stmt, count++);
		record->timezone_label = SAFE_STRDUP(temp);
		start = result;
	}
	if((result = strstr(start, CAL_TABLE_INT_DISPLAY_ALL_TIMEZONES))) {
		record->display_all_timezones = sqlite3_column_int(stmt, count++);
		start = result;
	}
	if((result = strstr(start, CAL_TABLE_INT_DATE_FIELD_ORDER))) {
		record->date_field_order = sqlite3_column_int(stmt, count++);
		start = result;
	}
	if((result = strstr(start, CAL_TABLE_INT_FROMAT_24HOUR_TIME))) {
		record->format_24hour_time = sqlite3_column_int(stmt, count++);
		start = result;
	}
	if((result = strstr(start, CAL_TABLE_INT_WEEK_START))) {
		record->week_start = sqlite3_column_int(stmt, count++);
		start = result;
	}
	if((result = strstr(start, CAL_TABLE_INT_DEFAULT_CAL_MODE))) {
		record->default_cal_mode = sqlite3_column_int(stmt, count++);
		start = result;
	}
	if((result = strstr(start, CAL_TABLE_INT_CUSTOM_CAL_MODE))) {
		record->custom_cal_mode = sqlite3_column_int(stmt, count++);
		start = result;
	}
	if((result = strstr(start, CAL_TABLE_TXT_USER_LOCATION))) {
		temp = sqlite3_column_text(stmt, count++);
		record->user_location = SAFE_STRDUP(temp);
		start = result;
	}
	if((result = strstr(start, CAL_TABLE_TXT_WEATHER))) {
		temp = sqlite3_column_text(stmt, count++);
		record->weather = SAFE_STRDUP(temp);
		start = result;
	}
	if((result = strstr(start, CAL_TABLE_INT_SHOW_DECLINED_EVENTS))) {
		record->show_declined_events = sqlite3_column_int(stmt, count++);
		start = result;
	}
	if((result = strstr(start, CAL_TABLE_INT_HIDE_INVITATIONS))) {
		record->hide_invitations = sqlite3_column_int(stmt, count++);
		start = result;
	}
	if((result = strstr(start, CAL_TABLE_INT_ALTERNATE_CALENDAR))) {
		record->alternate_calendar = sqlite3_column_int(stmt, count++);
		start = result;
	}
	if((result = strstr(start, CAL_TABLE_INT_VISIBILITY))) {
		record->visibility = sqlite3_column_int(stmt, count++);
		start = result;
	}
	if((result = strstr(start, CAL_TABLE_INT_PROJECTION))) {
		record->projection = sqlite3_column_int(stmt, count++);
		start = result;
	}
	if((result = strstr(start, CAL_TABLE_INT_SEQUENCE))) {
		record->sequence = sqlite3_column_int(stmt, count++);
		start = result;
	}
	if((result = strstr(start, CAL_TABLE_INT_SUPRESS_REPLY_NOTIFICATIONS))) {
		record->suppress_reply_notifications = sqlite3_column_int(stmt, count++);
		start = result;
	}
	if((result = strstr(start, CAL_TABLE_INT_SYNC_EVENT))) {
		record->sync_event = sqlite3_column_int(stmt, count++);
		start = result;
	}
	if((result = strstr(start, CAL_TABLE_INT_TIMES_CLEANED))) {
		record->times_cleaned = sqlite3_column_int(stmt, count++);
		start = result;
	}
	if((result = strstr(start, CAL_TABLE_INT_GUESTS_CAN_MODIFY))) {
		record->guests_can_modify = sqlite3_column_int(stmt, count++);
		start = result;
	}
	if((result = strstr(start, CAL_TABLE_INT_GUESTS_CAN_INVITE_OTHERS))) {
		record->guests_can_invite_others = sqlite3_column_int(stmt, count++);
		start = result;
	}
	if((result = strstr(start, CAL_TABLE_INT_GUESTS_CAN_SEE_GUESTS))) {
		record->guests_can_see_guests = sqlite3_column_int(stmt, count++);
		start = result;
	}
	if((result = strstr(start, CAL_TABLE_INT_ACCESS_LEVEL))) {
		record->access_level = sqlite3_column_int(stmt, count++);
		start = result;
	}
	if((result = strstr(start, CAL_TABLE_INT_SYNC_STATUS))) {
		record->sync_status = sqlite3_column_int(stmt, count++);
		start = result;
	}
	if((result = strstr(start, CAL_TABLE_INT_IS_DELETED))) {
		record->is_deleted = sqlite3_column_int(stmt, count++);
		start = result;
	}
	if((result = strstr(start, CAL_TABLE_INT_ACCOUNT_ID))) {
		record->account_id = sqlite3_column_int(stmt, count++);
		start = result;
	}
	if((result = strstr(start, CAL_TABLE_INT_SENSITIVITY))) {
		record->sensitivity = sqlite3_column_int(stmt, count++);
		start = result;
	}
	if((result = strstr(start, CAL_TABLE_INT_STORE_TYPE))) {
		record->store_type = sqlite3_column_int(stmt, count++);
		start = result;
	}
	return CAL_SUCCESS;
}

