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
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <glib.h>
#include <glib-object.h>
#include <vconf.h>

#include "cals-internal.h"
#include "cals-typedef.h"
#include "cals-utils.h"
#include "cals-db.h"
#include "cals-db-info.h"
#include "cals-ical.h"
#include "cals-tz-utils.h"
#include "cals-recurrence-utils.h"
#include "cals-ical-codec.h"
#include "cals-alarm.h"
#include "cals-sqlite.h"
#include "cals-calendar.h"
#include "cals-schedule.h"
#include "cals-inotify.h"

extern sqlite3* calendar_db_handle;

static int db_ref_cnt = 0;
cal_svc_tm_info_t cal_svc_tm_value;

typedef enum
{
	VALUE_TYPE_TEXT,
	VALUE_TYPE_INT,
	VALUE_TYPE_TIME,
	VALUE_TYPE_DOUBLE,
	VALUE_TYPE_USER,
} cal_value_type_t;

typedef struct
{
	char *field_name;
	int type;
} __cal_field_type;

static const __cal_field_type __calendar_event_type_field[]={
	{CAL_VALUE_INT_INDEX,				VALUE_TYPE_INT},
	{CAL_VALUE_INT_ACCOUNT_ID,			VALUE_TYPE_INT},
	{CAL_VALUE_TXT_SUMMARY,				VALUE_TYPE_TEXT},
	{CAL_VALUE_TXT_DESCRIPTION,			VALUE_TYPE_TEXT},
	{CAL_VALUE_TXT_LOCATION,				VALUE_TYPE_TEXT},
	{CAL_VALUE_INT_ALL_DAY_EVENT,		VALUE_TYPE_INT},
	{CAL_VALUE_GMT_START_DATE_TIME,		VALUE_TYPE_TIME},
	{CAL_VALUE_GMT_END_DATE_TIME,		VALUE_TYPE_TIME},
	{CAL_VALUE_INT_REPEAT_TERM,			VALUE_TYPE_INT},
	{CAL_VALUE_INT_REPEAT_INTERVAL,		VALUE_TYPE_INT},
	{CAL_VALUE_INT_REPEAT_OCCURRENCES,	VALUE_TYPE_INT},
	{CAL_VALUE_GMT_REPEAT_END_DATE,		VALUE_TYPE_TIME},
	{CAL_VALUE_INT_SUN_MOON,				VALUE_TYPE_INT},
	{CAL_VALUE_INT_WEEK_START,			VALUE_TYPE_INT},
	{CAL_VALUE_TXT_WEEK_FLAG,			VALUE_TYPE_TEXT},
	{CAL_VALUE_INT_DAY_DATE,				VALUE_TYPE_INT},
	{CAL_VALUE_GMT_LAST_MODIFIED_TIME,	VALUE_TYPE_TIME},
	{CAL_VALUE_INT_MISSED,				VALUE_TYPE_INT},
	{CAL_VALUE_INT_TASK_STATUS,			VALUE_TYPE_INT},
	{CAL_VALUE_INT_PRIORITY,				VALUE_TYPE_INT},
	{CAL_VALUE_INT_TIMEZONE,				VALUE_TYPE_INT},
	{CAL_VALUE_INT_FILE_ID,				VALUE_TYPE_INT},
	{CAL_VALUE_INT_CONTACT_ID,			VALUE_TYPE_INT},
	{CAL_VALUE_INT_BUSY_STATUS,			VALUE_TYPE_INT},
	{CAL_VALUE_INT_SENSITIVITY,			VALUE_TYPE_INT},
	{CAL_VALUE_INT_MEETING_STATUS,		VALUE_TYPE_INT},
	{CAL_VALUE_TXT_UID,					VALUE_TYPE_TEXT},
	{CAL_VALUE_TXT_ORGANIZER_NAME,		VALUE_TYPE_TEXT},
	{CAL_VALUE_TXT_ORGANIZER_EMAIL,		VALUE_TYPE_TEXT},
	{CAL_VALUE_INT_CALENDAR_TYPE,		VALUE_TYPE_INT},
	{CAL_VALUE_TXT_GCAL_ID,				VALUE_TYPE_TEXT},
	{CAL_VALUE_INT_DELETED,				VALUE_TYPE_INT},
	{CAL_VALUE_TXT_UPDATED,				VALUE_TYPE_TEXT},
	{CAL_VALUE_INT_LOCATION_TYPE,		VALUE_TYPE_INT},
	{CAL_VALUE_TXT_LOCATION_SUMMARY,		VALUE_TYPE_TEXT},
	{CAL_VALUE_TXT_ETAG,					VALUE_TYPE_TEXT},
	{CAL_VALUE_INT_CALENDAR_ID,			VALUE_TYPE_INT},
	{CAL_VALUE_INT_SYNC_STATUS,			VALUE_TYPE_INT},
	{CAL_VALUE_TXT_EDIT_URL,				VALUE_TYPE_TEXT},
	{CAL_VALUE_TXT_GEDERID,				VALUE_TYPE_TEXT},
	{CAL_VALUE_INT_DST,					VALUE_TYPE_INT},
	{CAL_VALUE_INT_ORIGINAL_EVENT_ID,	VALUE_TYPE_INT},
	{CAL_VALUE_INT_CALENDAR_INDEX,		VALUE_TYPE_INT},
	{CAL_VALUE_DBL_LATITUDE,				VALUE_TYPE_DOUBLE},
	{CAL_VALUE_DBL_LONGITUDE,			VALUE_TYPE_DOUBLE},
	{CAL_VALUE_TXT_TZ_NAME,				VALUE_TYPE_TEXT},
	{CAL_VALUE_TXT_TZ_CITY_NAME,			VALUE_TYPE_TEXT},
	{CAL_VALUE_INT_EMAIL_ID,			  VALUE_TYPE_INT},
	{CAL_VALUE_INT_AVAILABILITY,			VALUE_TYPE_INT},
	{CAL_VALUE_TXT_MEETING_CATEGORY_DETAIL_NAME,			VALUE_TYPE_TEXT},
	{CAL_VALUE_GMT_CREATED_DATE_TIME,	VALUE_TYPE_TIME},
	{CAL_VALUE_GMT_COMPLETED_DATE_TIME,	VALUE_TYPE_TIME},
	{CAL_VALUE_INT_PROGRESS,			VALUE_TYPE_INT},
	{NULL,								VALUE_TYPE_USER}
};

static const __cal_field_type __calendar_table_field[] = {
	{CAL_TABLE_INT_INDEX,					VALUE_TYPE_INT},
	{CAL_TABLE_TXT_CALENDAR_ID,				VALUE_TYPE_TEXT},
	{CAL_TABLE_TXT_UID,						VALUE_TYPE_TEXT},
	{CAL_TABLE_TXT_LINK,					VALUE_TYPE_TEXT},
	{CAL_TABLE_INT_UPDATED,					VALUE_TYPE_INT},
	{CAL_TABLE_TXT_NAME,					VALUE_TYPE_TEXT},
	{CAL_TABLE_TXT_DESCRIPTION,				VALUE_TYPE_TEXT},
	{CAL_TABLE_TXT_AUTHOR,					VALUE_TYPE_TEXT},
	{CAL_TABLE_TXT_COLOR,					VALUE_TYPE_TEXT},
	{CAL_TABLE_INT_HIDDEN,					VALUE_TYPE_INT},
	{CAL_TABLE_INT_SELECTED,				VALUE_TYPE_INT},
	{CAL_TABLE_TXT_LOCATION,				VALUE_TYPE_TEXT},
	{CAL_TABLE_INT_LOCALE,					VALUE_TYPE_INT},
	{CAL_TABLE_INT_COUNTRY,					VALUE_TYPE_INT},
	{CAL_TABLE_INT_TIME_ZONE,				VALUE_TYPE_INT},
	{CAL_TABLE_TXT_TIME_ZONE_LABEL,			VALUE_TYPE_TEXT},
	{CAL_TABLE_INT_DISPLAY_ALL_TIMEZONES,	VALUE_TYPE_INT},
	{CAL_TABLE_INT_DATE_FIELD_ORDER,		VALUE_TYPE_INT},
	{CAL_TABLE_INT_FROMAT_24HOUR_TIME,		VALUE_TYPE_INT},
	{CAL_TABLE_INT_WEEK_START,				VALUE_TYPE_INT},
	{CAL_TABLE_INT_DEFAULT_CAL_MODE,		VALUE_TYPE_INT},
	{CAL_TABLE_INT_CUSTOM_CAL_MODE,			VALUE_TYPE_INT},
	{CAL_TABLE_TXT_USER_LOCATION,			VALUE_TYPE_TEXT},
	{CAL_TABLE_TXT_WEATHER,					VALUE_TYPE_TEXT},
	{CAL_TABLE_INT_SHOW_DECLINED_EVENTS,	VALUE_TYPE_INT},
	{CAL_TABLE_INT_HIDE_INVITATIONS,		VALUE_TYPE_INT},
	{CAL_TABLE_INT_ALTERNATE_CALENDAR,		VALUE_TYPE_INT},
	{CAL_TABLE_INT_VISIBILITY,				VALUE_TYPE_INT},
	{CAL_TABLE_INT_PROJECTION,				VALUE_TYPE_INT},
	{CAL_TABLE_INT_SEQUENCE,				VALUE_TYPE_INT},
	{CAL_TABLE_INT_SUPRESS_REPLY_NOTIFICATIONS,VALUE_TYPE_INT},
	{CAL_TABLE_INT_SYNC_EVENT,				VALUE_TYPE_INT},
	{CAL_TABLE_INT_TIMES_CLEANED,			VALUE_TYPE_INT},
	{CAL_TABLE_INT_GUESTS_CAN_MODIFY,		VALUE_TYPE_INT},
	{CAL_TABLE_INT_GUESTS_CAN_INVITE_OTHERS,VALUE_TYPE_INT},
	{CAL_TABLE_INT_GUESTS_CAN_SEE_GUESTS,	VALUE_TYPE_INT},
	{CAL_TABLE_INT_ACCESS_LEVEL,			VALUE_TYPE_INT},
	{CAL_TABLE_INT_SYNC_STATUS,				VALUE_TYPE_INT},
	{CAL_TABLE_INT_ACCOUNT_ID,				VALUE_TYPE_INT},
	{CAL_TABLE_INT_SENSITIVITY,				VALUE_TYPE_INT},
	{NULL,VALUE_TYPE_USER}
};


static const __cal_field_type __timezone_table_field[] = {

	{CAL_TZ_VALUE_INT_INDEX, VALUE_TYPE_INT},
	{CAL_TZ_VALUE_INT_TZ_OFFSET, VALUE_TYPE_INT},

	{CAL_TZ_VALUE_TXT_STD_NAME, VALUE_TYPE_TEXT},
	{CAL_TZ_VALUE_INT_STD_START_MONTH, VALUE_TYPE_INT},
	{CAL_TZ_VALUE_INT_STD_START_POSITION_OF_WEEK, VALUE_TYPE_INT},
	{CAL_TZ_VALUE_INT_STD_START_DAY, VALUE_TYPE_INT},
	{CAL_TZ_VALUE_INT_STD_START_HOUR, VALUE_TYPE_INT},
	{CAL_TZ_VALUE_INT_STD_BIAS, VALUE_TYPE_INT},

	{CAL_TZ_VALUE_TXT_DST_NAME, VALUE_TYPE_TEXT},
	{CAL_TZ_VALUE_INT_DST_START_MONTH, VALUE_TYPE_INT},
	{CAL_TZ_VALUE_INT_DST_START_POSITION_OF_WEEK, VALUE_TYPE_INT},
	{CAL_TZ_VALUE_INT_DST_START_DAY, VALUE_TYPE_INT},
	{CAL_TZ_VALUE_INT_DST_START_HOUR, VALUE_TYPE_INT},
	{CAL_TZ_VALUE_INT_DST_BIAS, VALUE_TYPE_INT},

	{NULL,VALUE_TYPE_USER}
};

static cal_value_type_t __calendar_svc_get_type(int type, const char *field)
{

	int index = 0;
	const __cal_field_type *field_table;

	if((type == CAL_STRUCT_TYPE_SCHEDULE)||(type == CAL_STRUCT_TYPE_TODO))
		field_table = __calendar_event_type_field;
	else if(type == CAL_STRUCT_TYPE_CALENDAR)
		field_table = __calendar_table_field;
	else //(type == CAL_STRUCT_TYPE_TIMEZONE)
		field_table = __timezone_table_field;

	index = 0;
	while ((field_table[index].field_name != NULL) &&
				(strcmp(field_table[index].field_name, field)!=0))
	{
		index++;
	}

	return field_table[index].type;
}

API int calendar_svc_connect(void)
{
	CALS_FN_CALL;
	int ret = 0;

	if(db_ref_cnt <= 0)
	{
		ret = cals_db_open();
		retvm_if(ret, ret, "cals_db_open() Failed(%d)", ret);

		ret = cals_inotify_init();
		if(CAL_SUCCESS != ret) {
			cals_db_close();
			ERR("cals_inotify_init() Failed(%d)", ret);
			return ret;
		}
		db_ref_cnt = 0;
	}
	db_ref_cnt++;

	return CAL_SUCCESS;
}

API int calendar_svc_close(void)
{
	CALS_FN_CALL;
	retvm_if(0 == db_ref_cnt, CAL_ERR_ENV_INVALID,
			"Calendar service was not connected");

	if (db_ref_cnt==1) {
		cals_db_close();
		cals_inotify_close();
	}
	db_ref_cnt--;

	return CAL_SUCCESS;
}


API int calendar_svc_subscribe_db_change(const char *noti_type, void(*calendarNotiCb)(void *), void *user_data)
{
	CALS_FN_CALL;
	int ret;
	const char *noti_path;

	retv_if(NULL == noti_type, CAL_ERR_ARG_NULL);

	if(0 == strcmp(noti_type, CAL_STRUCT_SCHEDULE))
		noti_path = cals_noti_get_file_path(CALS_NOTI_TYPE_EVENT);
	else if(0 == strcmp(noti_type, CAL_STRUCT_TODO))
		noti_path = cals_noti_get_file_path(CALS_NOTI_TYPE_TODO);
	else if(0 == strcmp(noti_type, CAL_STRUCT_CALENDAR))
		noti_path = cals_noti_get_file_path(CALS_NOTI_TYPE_CALENDAR);
	else {
		ERR("Invalid noti_type(%s)", noti_type);
		return CAL_ERR_ARG_INVALID;
	}

	ret = cals_inotify_subscribe(noti_path, calendarNotiCb, user_data);
	retvm_if(CAL_SUCCESS != ret, ret, "cals_inotify_subscribe() Failed(%d)", ret);

	return CAL_SUCCESS;
}


API int calendar_svc_unsubscribe_db_change (const char *noti_type, void(*cb)(void *))
{
	CALS_FN_CALL;
	int ret;
	const char *noti_path;

	retv_if(NULL == noti_type, CAL_ERR_ARG_NULL);
	retv_if(NULL == cb, CAL_ERR_ARG_NULL);

	if(0 == strcmp(noti_type, CAL_STRUCT_SCHEDULE))
		noti_path = cals_noti_get_file_path(CALS_NOTI_TYPE_EVENT);
	else if(0 == strcmp(noti_type, CAL_STRUCT_TODO))
		noti_path = cals_noti_get_file_path(CALS_NOTI_TYPE_TODO);
	else if(0 == strcmp(noti_type, CAL_STRUCT_CALENDAR))
		noti_path = cals_noti_get_file_path(CALS_NOTI_TYPE_CALENDAR);
	else {
		ERR("Invalid noti_type(%s)", noti_type);
		return CAL_ERR_ARG_INVALID;
	}

	ret = cals_inotify_unsubscribe(noti_path, cb);
	retvm_if(CAL_SUCCESS != ret, ret, "cals_inotify_unsubscribe() Failed(%d)", ret);

	return CAL_SUCCESS;
}

API int calendar_svc_subscribe_change (void(*calendarNotiCb)(void *), void *user_data)
{
	CALS_FN_CALL;
	calendar_svc_subscribe_db_change(CAL_STRUCT_SCHEDULE, calendarNotiCb,user_data);
	calendar_svc_subscribe_db_change(CAL_STRUCT_TODO, calendarNotiCb,user_data);
	return CAL_SUCCESS;
}

API int calendar_svc_unsubscribe_change (void(*cb)(void *))
{
	CALS_FN_CALL;
	calendar_svc_unsubscribe_db_change(CAL_STRUCT_SCHEDULE, cb);
	calendar_svc_unsubscribe_db_change(CAL_STRUCT_TODO, cb);

	return CAL_SUCCESS;
}

API int calendar_svc_insert(cal_struct *event)
{
	int ret, index = 0;
	cal_sch_full_t *sch_temp = NULL;
	calendar_t *cal_temp = NULL;
	cal_timezone_t * tz_temp = NULL;
	clock_t start_time=0,end_time = 0;

	retv_if(NULL == event, CAL_ERR_ARG_NULL);

	start_time = CAL_PROFILE_GET_TIME();

	ret = cals_begin_trans();
	retvm_if(CAL_SUCCESS != ret, ret, "cals_begin_trans() Failed(%d)", ret);

	switch(event->event_type) {
	case CAL_STRUCT_TYPE_SCHEDULE:
		sch_temp = (cal_sch_full_t*)event->user_data;
		sch_temp->cal_type = CAL_EVENT_SCHEDULE_TYPE;

		ret = cals_insert_schedule(sch_temp);
		if (ret < CAL_SUCCESS) {
			cals_end_trans(false);
			ERR("cals_insert_schedule() Failed(%d)", ret);
			return ret;
		}

		index = sch_temp->index = ret;
		break;
	case CAL_STRUCT_TYPE_CALENDAR:
		cal_temp = (calendar_t*)event->user_data;

		ret = cals_insert_calendar(cal_temp);
		if (ret < CAL_SUCCESS) {
			cals_end_trans(false);
			ERR("cals_insert_calendar() Failed(%d)", ret);
			return ret;
		}

		index = cal_temp->index = ret;
		break;
	case CAL_STRUCT_TYPE_TODO:
		sch_temp = (cal_sch_full_t*)event->user_data;

		ret = cals_insert_schedule(sch_temp);
		if (ret < CAL_SUCCESS) {
			cals_end_trans(false);
			ERR("cals_insert_schedule() Failed(%d)", ret);
			return ret;
		}

		index = sch_temp->index = ret;
		break;
	case CAL_STRUCT_TYPE_TIMEZONE:
		tz_temp = (cal_timezone_t*)event->user_data;

		ret = cals_insert_timezone(tz_temp);
		if (ret < CAL_SUCCESS) {
			cals_end_trans(false);
			ERR("cals_insert_timezone() Failed(%d)", ret);
			return ret;
		}

		index = tz_temp->index = ret;
		break;
	default:
		cals_end_trans(false);
		ERR("Unknown event type(%d)", event->event_type);
		return CAL_ERR_ARG_INVALID;
	}
	cals_end_trans(true);

	end_time = CAL_PROFILE_GET_TIME();
	CAL_PROFILE_PRINT(start_time,end_time);

	return index;
}


API int calendar_svc_get(const char *data_type,int index,const char *field_list, cal_struct **record)
{
	CALS_FN_CALL;
	int error_code = 0;
	char sql_value[CALS_SQL_MIN_LEN] = {0};
	char rearranged[CALS_SQL_MIN_LEN] = {0};
	int rc = 0;
	sqlite3_stmt *stmt = NULL;
	bool malloc_inside = false;

	retex_if(NULL == data_type,,"data_type is NULL");
	retex_if(NULL == record ,,"record is NULL");

	if((0 == strcmp(data_type,CAL_STRUCT_SCHEDULE)) || (0 == strcmp(data_type,CAL_STRUCT_TODO)))
	{
		if(NULL == *record)
		{
			*record = calendar_svc_struct_new(data_type);
			retex_if(NULL == *record,,"calendar_svc_struct_new() Failed");

			malloc_inside = true;
		}

		cal_sch_full_t *sch_record = NULL;
		sch_record = (*record)->user_data;

		if (field_list) {
			cals_rearrage_schedule_field(field_list, rearranged, sizeof(rearranged));
			sprintf(sql_value,"SELECT %s FROM %s WHERE id=%d",
						CALS_TABLE_SCHEDULE, rearranged, index);
		} else
			sprintf(sql_value,"SELECT * FROM %s WHERE id=%d;", CALS_TABLE_SCHEDULE, index);

		stmt = cals_query_prepare(sql_value);
		retex_if(NULL == stmt,,"cals_query_prepare() Failed");

		rc = cals_stmt_step(stmt);
		retex_if(CAL_TRUE != rc,,"cals_stmt_step() Failed(%d)", rc);

		if (field_list)
			cals_stmt_get_filted_schedule(stmt, sch_record, field_list);
		else
			cals_stmt_get_full_schedule(stmt, sch_record, true);

		sch_record->index = index;

		if (0 == strcmp(data_type,CAL_STRUCT_SCHEDULE)) {
			cal_db_service_get_participant_info_by_index(index,&(sch_record->attendee_list),&error_code);
			cal_db_service_get_meeting_category_info_by_index(index,&(sch_record->meeting_category),&error_code);
			cal_db_service_get_recurrency_exception(index,&(sch_record->exception_date_list),&error_code);
			cals_get_alarm_info(index, &(sch_record->alarm_list));
		}
	}
	else if(0 == strcmp(data_type,CAL_STRUCT_CALENDAR))
	{
		if(NULL == *record)
		{
			*record = calendar_svc_struct_new(CAL_STRUCT_CALENDAR);
			retex_if(NULL == *record,,"calendar_svc_struct_new() Failed");
		}

		calendar_t *calendar = NULL;
		calendar = (*record)->user_data;

		if (field_list)	{
			cals_rearrage_calendar_field(field_list, rearranged, sizeof(rearranged));
			sprintf(sql_value,"SELECT rowid,%s FROM %s WHERE rowid=%d",
				rearranged, CALS_TABLE_CALENDAR, index);
		}
		else
			sprintf(sql_value,"SELECT rowid,* FROM %s WHERE rowid=%d",
				CALS_TABLE_CALENDAR,	index);

		stmt = cals_query_prepare(sql_value);
		retex_if(NULL == stmt,,"cals_query_prepare() Failed");

		rc = cals_stmt_step(stmt);
		retex_if(CAL_TRUE != rc,,"cals_stmt_step() Failed(%d)", rc);

		if (field_list)
			cals_stmt_get_filted_calendar(stmt, calendar, field_list);
		else
			cals_stmt_get_calendar(stmt, calendar);

		calendar->index = index;
	}
	else if(0 == strcmp(data_type,CAL_STRUCT_TIMEZONE))
	{
		if(NULL == *record)
		{
			*record = calendar_svc_struct_new(CAL_STRUCT_TIMEZONE);
			retex_if(NULL == *record,,"calendar_svc_struct_new() Failed");
		}

		cal_timezone_t *detail_record = NULL;
		detail_record = (*record)->user_data;

		/*if(NULL != field_list) - not support yet
		  {
		  sprintf(sql_value,"select rowid,%s from timezone_table where rowid=%d;",field_list,index);
		  }
		  else*/
		{
			sprintf(sql_value,"select rowid,* from timezone_table where rowid=%d;",index);
		}

		stmt = cals_query_prepare(sql_value);
		retex_if(NULL == stmt,,"cals_query_prepare() Failed");

		rc = cals_stmt_step(stmt);
		retex_if(CAL_TRUE != rc,,"cals_stmt_step() Failed(%d)", rc);

		/*if(NULL != field_list) - not support yet
		  {
		  cal_db_service_convert_stmt_to_select_field_tz_info(stmt,detail_record,field_list);
		  }
		  else*/
		{
			cal_db_service_convert_stmt_to_tz_info(stmt,detail_record);
		}

		detail_record->index = index;
	}
	else
	{
		retex_if(true,,"Can not find!\n");
	}


	if (stmt != NULL)
	{
		sqlite3_finalize(stmt);
		stmt = NULL;
	}

	return CAL_SUCCESS;

CATCH:

	if (stmt != NULL)
	{
		sqlite3_finalize(stmt);
		stmt = NULL;
	}

	if(malloc_inside)
	{
		if(NULL != *record)
		{
			calendar_svc_struct_free(record);
		}
	}

	return CAL_ERR_FAIL;

}


API int calendar_svc_get_count(int account_id, int calendar_id, const char *data_type)
{
	CALS_FN_CALL;
	int count = 0;
	char sql_value[CALS_SQL_MIN_LEN] = {0};
	char condition_value[CALS_SQL_MIN_LEN] = {0};

	retv_if(NULL == data_type, CAL_ERR_ARG_NULL);

	if(0 == strcmp(data_type,CAL_STRUCT_SCHEDULE))
	{
		sprintf(condition_value,"WHERE type=%d",CAL_EVENT_SCHEDULE_TYPE);

		if(account_id == ALL_VISIBILITY_ACCOUNT || calendar_id==ALL_VISIBILITY_ACCOUNT)
		{
			sprintf(sql_value,"SELECT COUNT(*) FROM %s A, %s B %s "
					"AND A.calendar_id = B.rowid AND B.visibility = 1 AND "
					"A.is_deleted = 0 ORDER BY A.start_date_time",
					CALS_TABLE_SCHEDULE, CALS_TABLE_CALENDAR, condition_value);
		}
		else
		{
			if(account_id !=0)
			{
				sprintf(condition_value,"%s AND account_id = %d",condition_value,account_id);
			}

			if(calendar_id != 0)
			{
				sprintf(condition_value,"%s AND calendar_id = %d",condition_value,calendar_id);
			}

			sprintf(sql_value,"SELECT COUNT(*) FROM %s %s AND is_deleted = 0 ORDER BY start_date_time",
				CALS_TABLE_SCHEDULE, condition_value);
		}

	}
	else if(0 == strcmp(data_type,CAL_STRUCT_TODO))
	{
		sprintf(condition_value,"WHERE type=%d",CAL_EVENT_TODO_TYPE);

		if(account_id !=0)
		{
			sprintf(condition_value,"%s AND account_id = %d ",condition_value,account_id);
		}

		if(calendar_id != 0)
		{
			sprintf(condition_value,"%s AND calendar_id = %d ",condition_value,calendar_id);
		}

		sprintf(sql_value,"SELECT COUNT(*) FROM %s %s AND is_deleted = 0",
			CALS_TABLE_SCHEDULE, condition_value);

	}
	else if(0 == strcmp(data_type,CAL_STRUCT_CALENDAR))
	{
		condition_value[0] = '\0';

		if(account_id != 0)
		{
			sprintf(condition_value, "WHERE account_id = %d ", account_id);
		}

		sprintf(sql_value,"SELECT COUNT(*) FROM %s %s;", CALS_TABLE_CALENDAR, condition_value);

	}
	else //not support yet
	{
		ERR("Invalid type(%s)", data_type);
		return CAL_ERR_ARG_INVALID;
	}

	count = cals_query_get_first_int_result(sql_value);

	return count;
}

API int calendar_svc_get_all(int account_id, int calendar_id,const char *data_type, cal_iter **iter)
{
	CALS_FN_CALL;
	int type;
	sqlite3_stmt *stmt = NULL;
	char sql_value[CALS_SQL_MIN_LEN];

	retv_if(NULL == data_type, CAL_ERR_ARG_NULL);
	retv_if(NULL == iter, CAL_ERR_ARG_NULL);
	retvm_if(calendar_id < 0, CAL_ERR_ARG_INVALID, "calendar_id(%d) is Invalid", calendar_id);

	if(0 == strcmp(data_type,CAL_STRUCT_SCHEDULE))
	{
		if (account_id == ALL_VISIBILITY_ACCOUNT || calendar_id==ALL_VISIBILITY_ACCOUNT)
		{
			sprintf(sql_value,"SELECT A.* FROM %s A, %s B ON A.calendar_id = B.rowid "
					"WHERE type=%d AND B.visibility = 1 AND A.is_deleted = 0 "
					"ORDER BY A.start_date_time",
					CALS_TABLE_SCHEDULE, CALS_TABLE_CALENDAR, CAL_EVENT_SCHEDULE_TYPE);
		}
		else
		{
			if (calendar_id)
				sprintf(sql_value,"SELECT * FROM %s "
					"WHERE type=%d AND is_deleted = 0 AND calendar_id = %d "
					"ORDER BY start_date_time",
					CALS_TABLE_SCHEDULE, CAL_EVENT_SCHEDULE_TYPE, calendar_id);
			else if (account_id)
				sprintf(sql_value,"SELECT * FROM %s "
					"WHERE type=%d AND is_deleted = 0 AND account_id = %d "
					"ORDER BY start_date_time",
					CALS_TABLE_SCHEDULE, CAL_EVENT_SCHEDULE_TYPE, account_id);
			else
				sprintf(sql_value,"SELECT * FROM %s "
					"WHERE type=%d AND is_deleted = 0 "
					"ORDER BY start_date_time",
					CALS_TABLE_SCHEDULE, CAL_EVENT_SCHEDULE_TYPE);
		}

		type = CAL_STRUCT_TYPE_SCHEDULE;
	}
	else if(0 == strcmp(data_type,CAL_STRUCT_TODO))
	{
		if (calendar_id)
			sprintf(sql_value,"SELECT * FROM %s WHERE type=%d AND is_deleted = 0 AND calendar_id = %d ",
				CALS_TABLE_SCHEDULE, CAL_EVENT_TODO_TYPE, calendar_id);
		else if (account_id)
			sprintf(sql_value,"SELECT * FROM %s WHERE type=%d AND is_deleted = 0 AND account_id = %d ",
				CALS_TABLE_SCHEDULE, CAL_EVENT_TODO_TYPE, account_id);
		else
			sprintf(sql_value,"SELECT * FROM %s WHERE type=%d AND is_deleted = 0 ",
				CALS_TABLE_SCHEDULE, CAL_EVENT_TODO_TYPE);

		type = CAL_STRUCT_TYPE_TODO;
	}
	else if(0 == strcmp(data_type,CAL_STRUCT_CALENDAR))
	{
		if (account_id)
			sprintf(sql_value,"SELECT rowid,* FROM %s WHERE account_id = %d", CALS_TABLE_CALENDAR, account_id);
		else
			sprintf(sql_value,"SELECT rowid,* FROM %s", CALS_TABLE_CALENDAR);

		type = CAL_STRUCT_TYPE_CALENDAR;
	}
	else //not support yet
	{
		ERR("Unknown Type(%s)", data_type);
		return CAL_ERR_ARG_INVALID;
	}

	stmt = cals_query_prepare(sql_value);
	retvm_if(NULL == stmt, CAL_ERR_DB_FAILED, "cals_query_prepare() Failed");

	*iter = calloc(1, sizeof(cal_iter));
	retvm_if(NULL == *iter, CAL_ERR_OUT_OF_MEMORY, "calloc() Failed(%d)", errno);

	(*iter)->stmt = stmt;
	(*iter)->i_type = type;

	return CAL_SUCCESS;
}


API int calendar_svc_get_list(int account_id, int calendar_id,
	const char *data_type,const char *field_type, int offset,int count, cal_iter **iter)
{
	CALS_FN_CALL;
	int type;
	sqlite3_stmt *stmt = NULL;
	char sql_value[CALS_SQL_MAX_LEN] = {0};

	retv_if(NULL == data_type, CAL_ERR_ARG_NULL);
	retv_if(NULL == iter, CAL_ERR_ARG_NULL);
	retvm_if(calendar_id < 0, CAL_ERR_ARG_INVALID, "calendar_id(%d) is Invalid", calendar_id);

	if(0 == strcmp(data_type,CAL_STRUCT_SCHEDULE))
	{
		if(account_id == ALL_VISIBILITY_ACCOUNT || calendar_id==ALL_VISIBILITY_ACCOUNT)
		{
			sprintf(sql_value,"SELECT A.id, A.summary, A.location, A.all_day_event,"
					"A.start_date_time,A.end_date_time,A.repeat_item,A.week_start,A.week_flag,A.calendar_id "
					"FROM %s A, %s B "
					"WHERE A.type=%d AND B.visibility = 1 AND A.calendar_id = B.rowid AND A.is_deleted = 0 "
					"ORDER BY A.start_date_time LIMIT %d, %d",
					CALS_TABLE_SCHEDULE, CALS_TABLE_CALENDAR, CAL_EVENT_SCHEDULE_TYPE, offset, count);
		}
		else
		{
			char cond[CALS_SQL_MIN_LEN];
			cond[0] = '\0';

			if (account_id)
				sprintf(cond,"type=%d AND account_id = %d AND",CAL_EVENT_SCHEDULE_TYPE,account_id);

			if (calendar_id)
				sprintf(cond,"type=%d AND calendar_id = %d AND",CAL_EVENT_SCHEDULE_TYPE,calendar_id);

			sprintf(sql_value,"SELECT id,summary,location,all_day_event,"
					"start_date_time,end_date_time,repeat_item,week_start,week_flag,calendar_id "
					"FROM %s WHERE %s is_deleted = 0 ORDER BY start_date_time LIMIT %d,%d;",
					CALS_TABLE_SCHEDULE, cond, offset, count);
		}

		type = CAL_STRUCT_TYPE_SCHEDULE_LIST;
	}
	else if (0 == strcmp(data_type,CAL_STRUCT_TODO)) {
		//cond_size = sprintf(condition_value,"WHERE A.type=%d AND",CAL_EVENT_TODO_TYPE);
		ERR("data_type(%s) is not supported", data_type);
		return CAL_ERR_ARG_INVALID;
	}
	else {
		//cond_size = sprintf(condition_value,"WHERE ");
		ERR("data_type(%s) is not supported", data_type);
		return CAL_ERR_ARG_INVALID;
	}

	stmt = cals_query_prepare(sql_value);
	retvm_if(NULL == stmt, CAL_ERR_DB_FAILED, "cals_query_prepare() Failed");

	*iter = calloc(1, sizeof(cal_iter));
	retvm_if(NULL == *iter, CAL_ERR_OUT_OF_MEMORY, "calloc() Failed(%d)", errno);

	(*iter)->stmt = stmt;
	(*iter)->i_type = type;

	return CAL_SUCCESS;
}


API int calendar_svc_search_list(int account_id,int calendar_id,const char *data_type,const char *search_type,const void *search_value,
		int offset,int count, cal_iter **iter)
{
	CALS_FN_CALL;
	sqlite3_stmt *stmt = NULL;
	char sql_value[CALS_SQL_MAX_LEN] = {0};
	int rc = 0;
	char condition_value[CALS_SQL_MAX_LEN] = {0};
	char search_str[CALS_SQL_MAX_LEN] = {0};
	cal_value_type_t value_type = 0;

	retv_if(NULL == data_type, CAL_ERR_FAIL);
	retv_if(NULL == search_type, CAL_ERR_FAIL);
	retv_if(NULL == search_value, CAL_ERR_FAIL);
	retv_if(CALS_SQL_MIN_LEN < strlen(search_value), CAL_ERR_FAIL);
	retv_if(NULL == iter, CAL_ERR_FAIL);
	retv_if(calendar_id < 0, CAL_ERR_FAIL);

	*iter = calloc(1, sizeof(cal_iter));
	retvm_if(NULL == *iter, CAL_ERR_OUT_OF_MEMORY, "calloc() Failed(%d)", errno);

	value_type = __calendar_svc_get_type(CAL_STRUCT_TYPE_SCHEDULE, search_type);

	switch(value_type)
	{
	case VALUE_TYPE_TEXT:

		retex_if(search_value == NULL,,"search_value is NULL");
		cals_escape_like_pattern(search_value, search_str, sizeof(search_str));
		DBG("%s %s", search_value, search_str);

		if(ALL_VISIBILITY_ACCOUNT == account_id)
		{
			sprintf(condition_value,"where st.type=%d and upper(%s) like upper('%%%s%%') ESCAPE '\\'",CAL_EVENT_SCHEDULE_TYPE, search_type, search_str);
		}
		else if(0 != account_id)
		{
			sprintf(condition_value,"where st.type=%d and upper(%s) like upper('%%%s%%') ESCAPE '\\'and account_id = %d",CAL_EVENT_SCHEDULE_TYPE, search_type, search_str, account_id);
		}
		else
		{
			sprintf(condition_value,"where st.type=%d and upper(%s) like upper('%%%s%%') ESCAPE '\\'",CAL_EVENT_SCHEDULE_TYPE, search_type, search_str);
		}
		break;
	case VALUE_TYPE_INT:
		if(ALL_VISIBILITY_ACCOUNT == account_id)
		{
			sprintf(condition_value,"where st.type=%d and %s = %d ",CAL_EVENT_SCHEDULE_TYPE, search_type,(int)search_value);
		}
		else if(0 != account_id)
		{
			sprintf(condition_value,"where st.type=%d and %s = %d and account_id = %d",CAL_EVENT_SCHEDULE_TYPE, search_type, (int)search_value, account_id);
		}
		else
		{
			sprintf(condition_value,"where st.type=%d and %s = %d ",CAL_EVENT_SCHEDULE_TYPE, search_type, (int)search_value);
		}
		break;

	case VALUE_TYPE_TIME:
	case VALUE_TYPE_DOUBLE:
	case VALUE_TYPE_USER:
		retex_if(true,,"Can not find");
	default:
		break;
	}

	if(0 == strcmp(data_type,CAL_STRUCT_SCHEDULE))
	{
		if(account_id == ALL_VISIBILITY_ACCOUNT || calendar_id==ALL_VISIBILITY_ACCOUNT)
		{
			sprintf(sql_value,"select st.id,st.summary,st.location,st.all_day_event,st.start_date_time,st.end_date_time,st.repeat_item,st.week_start,st.week_flag,st.calendar_id "
					"from schedule_table as st, calendar_table as ct %s "
					"and ct.visibility = 1 and st.calendar_id = ct.rowid and "
					"st.is_deleted = 0 order by st.summary limit %d,%d ;",condition_value,offset,count);
		}
		else
		{

			sprintf(sql_value,"select st.id,st.summary,st.location,st.all_day_event,st.start_date_time,st.end_date_time,st.repeat_item,st.week_start,st.week_flag,st.calendar_id "\
					"from schedule_table as st %s and is_deleted = 0 order by st.summary limit %d,%d ;",condition_value,offset,count);
		}

		(*iter)->i_type = CAL_STRUCT_TYPE_SCHEDULE_LIST;
	}

	rc = sqlite3_prepare_v2(calendar_db_handle, sql_value, strlen(sql_value), &stmt, NULL);
	retex_if(rc != SQLITE_OK,,"[ERROR]calendar_svc_get_all:Failed to get stmt!(sql:%s)\n",sql_value);

	(*iter)->stmt = stmt;


	return CAL_SUCCESS;

CATCH:

	if (iter && *iter != NULL)
	{
		CAL_FREE(*iter);
	}

	if (stmt != NULL)
	{
		sqlite3_finalize(stmt);
		stmt = NULL;
	}

	return CAL_ERR_FAIL;
}


API int calendar_svc_update(cal_struct *record)
{
	CALS_FN_CALL;
	int ret = 0;

	retv_if(NULL == record, CAL_ERR_ARG_NULL);

	cal_sch_full_t *event_record_schedule = NULL;
	calendar_t *event_record_calendar = NULL;
	cal_timezone_t *tz_info = NULL;

	ret = cals_begin_trans();
	retvm_if(CAL_SUCCESS != ret, ret, "cals_begin_trans() Failed(%d)", ret);

	switch(record->event_type)	{
	case CAL_STRUCT_TYPE_SCHEDULE:
	case CAL_STRUCT_TYPE_TODO:
		event_record_schedule = record->user_data;
		ret = cals_update_schedule(event_record_schedule->index,event_record_schedule);
		if (CAL_SUCCESS != ret) {
			cals_end_trans(false);
			ERR("cals_update_schedule() Failed(%d)", ret);
			return ret;
		}
		break;
	case CAL_STRUCT_TYPE_CALENDAR:
		event_record_calendar = record->user_data;
		ret = cals_update_calendar(event_record_calendar);
		if (CAL_SUCCESS != ret) {
			cals_end_trans(false);
			ERR("cals_update_calendar() Failed(%d)", ret);
			return ret;
		}
		break;
	case CAL_STRUCT_TYPE_TIMEZONE:
		tz_info = record->user_data;
		ret = cals_update_timezone(tz_info);
		if (CAL_SUCCESS != ret) {
			cals_end_trans(false);
			ERR("cals_update_timezone() Failed(%d)", ret);
			return ret;
		}
		break;
	default:
		cals_end_trans(false);
		ERR("Unknown event type(%d)", record->event_type);
		return CAL_ERR_ARG_INVALID;
	}
	cals_end_trans(true);

	return CAL_SUCCESS;
}


API int calendar_svc_delete(const char *data_type, int index)
{
	CALS_FN_CALL;
	int ret = 0;

	retv_if(NULL == data_type, CAL_ERR_ARG_NULL);

	ret = cals_begin_trans();
	retvm_if(CAL_SUCCESS != ret, ret, "cals_begin_trans() Failed(%d)", ret);

	if(0 == strcmp(data_type,CAL_STRUCT_SCHEDULE) || 0 == strcmp(data_type,CAL_STRUCT_TODO)) {
		ret = cals_delete_schedule(index);
		if (ret) {
			cals_end_trans(false);
			ERR("cals_delete_schedule() Failed(%d)", ret);
			return ret;
		}
	} else if(0 == strcmp(data_type,CAL_STRUCT_CALENDAR)) {
		if(DEFAULT_CALENDAR_ID == index) {
			cals_end_trans(false);
			return CAL_ERR_FAIL;
		}

		ret = cals_delete_calendar(index);
		if (ret) {
			cals_end_trans(false);
			ERR("cals_delete_calendar() Failed(%d)", ret);
			return ret;
		}
	} else {
		cals_end_trans(false);
		ERR("Invalid data_type(%s)", data_type);
		return CAL_ERR_ARG_INVALID;
	}
	cals_end_trans(true);

	return CAL_SUCCESS;
}


static inline int cals_remove_alarms_by_period(int calendar_id, time_t start_time, time_t end_time)
{
	int ret = 0;
	char query[CALS_SQL_MIN_LEN];
	sqlite3_stmt *stmt = NULL;

	if (calendar_id)
		sprintf(query, "SELECT id FROM %s WHERE start_date_time >= %ld AND start_date_time <= %ld AND calendar_id = %d",
				CALS_TABLE_SCHEDULE, start_time, end_time, calendar_id);
	else
		sprintf(query, "SELECT id FROM %s WHERE start_date_time >= %ld AND start_date_time <= %ld",
				CALS_TABLE_SCHEDULE, start_time, end_time);

	stmt = cals_query_prepare(query);
	retvm_if(NULL == stmt, CAL_ERR_DB_FAILED, "cals_query_prepare() Failed");

	ret = cals_stmt_step(stmt);
	if (ret < CAL_SUCCESS) {
		sqlite3_finalize(stmt);
		ERR("cals_stmt_step() Failed(%d)", ret);
		return ret;
	}

	while(CAL_TRUE == ret)
	{
		cals_alarm_remove(CALS_ALARM_REMOVE_BY_EVENT_ID, sqlite3_column_int(stmt,0));
		ret = cals_stmt_step(stmt);
	}
	sqlite3_finalize(stmt);

	return CAL_SUCCESS;
}


API int calendar_svc_event_delete_by_period(int calendar_id, time_t start_time, time_t end_time)
{
	CALS_FN_CALL;
	char sql[CALS_SQL_MIN_LEN] = {0};
	int ret = 0;

	retvm_if(start_time < 0, CAL_ERR_ARG_INVALID, "start_time(%ld) Invalid", start_time);
	retvm_if(end_time < 0, CAL_ERR_ARG_INVALID, "end_time(%ld) Invalid", end_time);

	ret = cals_begin_trans();
	retvm_if(CAL_SUCCESS != ret, ret, "cals_begin_trans() Failed(%d)", ret);

	ret = cals_remove_alarms_by_period(calendar_id, start_time, end_time);
	if (CAL_SUCCESS != ret) {
		cals_end_trans(false);
		ERR("cals_remove_alarms_by_period() Failed(%d)", ret);
		return ret;
	}

	time_t current_time = time(NULL);

	if (calendar_id)
		sprintf(sql,"UPDATE %s SET is_deleted = 1,sync_status = %d,last_modified_time = %ld "
						"WHERE start_date_time >= %ld AND start_date_time <= %ld AND calendar_id = %d",
						CALS_TABLE_SCHEDULE, CAL_SYNC_STATUS_DELETED, current_time, start_time, end_time, calendar_id);
	else
		sprintf(sql,"UPDATE %s SET is_deleted = 1,sync_status = %d,last_modified_time = %ld "
						"WHERE start_date_time >= %ld AND start_date_time <= %ld",
						CALS_TABLE_SCHEDULE, CAL_SYNC_STATUS_DELETED, current_time, start_time, end_time);

	ret = cals_query_exec(sql);
	if (CAL_SUCCESS != ret) {
		cals_end_trans(false);
		ERR("cals_query_exec() Failed(%d)", ret);
		return ret;
	}
	cals_end_trans(true);

	cals_notify(CALS_NOTI_TYPE_EVENT);

	return CAL_SUCCESS;
}


API int calendar_svc_delete_account (int account_id)
{
	CALS_FN_CALL;
	int ret = 0;
	char sql[512] = {0,};

	retvm_if(account_id < 0, CAL_ERR_ARG_INVALID, "account_id(%d) is Invalid", account_id);

	ret = cals_alarm_remove(CALS_ALARM_REMOVE_BY_ACC_ID, account_id);
	retvm_if(CAL_SUCCESS != ret, ret, "cals_alarm_remove() Failed(%d)", ret);

	//delete schedule
	if (account_id)
		sprintf(sql,"DELETE FROM %s WHERE account_id = %d", CALS_TABLE_SCHEDULE, account_id);
	else
		sprintf(sql,"DELETE FROM %s", CALS_TABLE_SCHEDULE);

	ret = cals_query_exec(sql);
	retvm_if(ret, ret, "cals_query_exec() Failed(%d)", ret);

	calendar_svc_delete_all(account_id, CAL_STRUCT_CALENDAR);

	return CAL_SUCCESS;
}


API int calendar_svc_clean_after_sync(int account_id)
{
	CALS_FN_CALL;
	int ret = 0;
	char sql[256] = {0};

	retvm_if(account_id < 0, CAL_ERR_ARG_INVALID, "account_id(%d) is Invalid", account_id);

	if(account_id)
		sprintf(sql,"delete from %s where (is_deleted = 1 and account_id = %d);",
			CALS_TABLE_SCHEDULE, account_id);
	else
		sprintf(sql,"delete from %s where is_deleted = 1;", CALS_TABLE_SCHEDULE);

	ret = cals_query_exec(sql);
	retvm_if(ret, ret, "cals_query_exec() Failed(%d)", ret);

	return CAL_SUCCESS;
}


API int calendar_svc_delete_all(int account_id, const char *data_type)
{
	int ret = 0;

	retvm_if(account_id < 0, CAL_ERR_ARG_INVALID, "account_id(%d) is Invalid", account_id);

	if(data_type == NULL) //delete all data from db by account id
	{
		ret = __cal_service_delete_all_records(account_id, CAL_EVENT_NONE);
		retvm_if(CAL_SUCCESS != ret, ret, "__cal_service_delete_all_records() Failed(%d)", ret);
		ret = cals_delete_calendars(account_id);
		retvm_if(CAL_SUCCESS != ret, ret, "cals_delete_calendars() Failed(%d)", ret);
	}
	else if(0 == strcmp(data_type,CAL_STRUCT_SCHEDULE))
	{
		ret = __cal_service_delete_all_records(account_id, CAL_EVENT_SCHEDULE_TYPE);
		retvm_if(CAL_SUCCESS != ret, ret, "__cal_service_delete_all_records() Failed(%d)", ret);
	}
	else if(0 == strcmp(data_type,CAL_STRUCT_TODO))
	{
		ret = __cal_service_delete_all_records(account_id, CAL_EVENT_TODO_TYPE);
		retvm_if(CAL_SUCCESS != ret, ret, "__cal_service_delete_all_records() Failed(%d)", ret);
	}
	else if(0 == strcmp(data_type,CAL_STRUCT_CALENDAR))
	{
		ret = cals_delete_calendars(account_id);
		retvm_if(CAL_SUCCESS != ret, ret, "cals_delete_calendars() Failed(%d)", ret);
	}
	else
	{
		ERR("Unknow data_type(%s)", data_type);
		return CAL_ERR_ARG_INVALID;
	}

	return CAL_SUCCESS;
}


API int calendar_svc_find_event_list(int account_id,const char *search_type,const void *search_value, cal_iter **iter)
{
	CALS_FN_CALL;
	int ret = 0;
	sqlite3_stmt *stmt = NULL;
	char sql_value[CALS_SQL_MIN_LEN] = {0};
	cal_value_type_t value_type = 0;

	retv_if(NULL == search_type, CAL_ERR_ARG_NULL);
	retv_if(NULL == iter, CAL_ERR_ARG_NULL);

	value_type = __calendar_svc_get_type(CAL_STRUCT_TYPE_SCHEDULE,search_type);

	switch(value_type)
	{
	case VALUE_TYPE_TEXT:
		retv_if(NULL == search_value, CAL_ERR_ARG_NULL);

		if(ALL_VISIBILITY_ACCOUNT == account_id)
		{
			sprintf(sql_value,
					"SELECT * FROM %s WHERE %s like upper('%%%s%%') AND is_deleted = 0 ORDER BY start_date_time;",
					CALS_TABLE_SCHEDULE, search_type, (char*)search_value);
		}
		else if(0 != account_id)
		{
			sprintf(sql_value,
					"SELECT * FROM %s WHERE %s like upper('%%%s%%') AND is_deleted = 0 and account_id = %d ORDER BY start_date_time;",
					CALS_TABLE_SCHEDULE, search_type, (char*)search_value, account_id);
		}
		else
		{
			sprintf(sql_value,
					"SELECT * FROM %s WHERE %s like upper('%%%s%%') AND is_deleted = 0 ORDER BY start_date_time;",
					CALS_TABLE_SCHEDULE, search_type, (char*)search_value);
		}
		break;
	case VALUE_TYPE_INT:
		if(ALL_VISIBILITY_ACCOUNT == account_id)
		{
			sprintf(sql_value,
					"SELECT * FROM %s WHERE %s = %d AND is_deleted = 0 ORDER BY start_date_time;",
					CALS_TABLE_SCHEDULE, search_type,(int)search_value);
		}
		else if(0 != account_id)
		{
			sprintf(sql_value,
					"SELECT * FROM %s WHERE %s = %d AND is_deleted = 0 AND account_id = %d ORDER BY start_date_time;",
					CALS_TABLE_SCHEDULE, search_type, (int)search_value, account_id);
		}
		else
		{
			sprintf(sql_value,
					"SELECT * FROM %s WHERE %s = %d AND is_deleted = 0 ORDER BY start_date_time;",
					CALS_TABLE_SCHEDULE, search_type, (int)search_value);
		}
		break;
	case VALUE_TYPE_USER:
		if (0 == strcmp(CAL_VALUE_INT_ALARMS_ID, search_type)) {
			ret = cals_alarm_get_event_id((int)search_value);
			sprintf(sql_value,
					"SELECT * FROM %s WHERE id = %d AND is_deleted = 0 ORDER BY start_date_time",
					CALS_TABLE_SCHEDULE, ret);
			break;
		}
	case VALUE_TYPE_TIME:
	case VALUE_TYPE_DOUBLE:
	default:
		ERR("Unknown Type(%s:%d)", search_type, value_type);
		break;
	}

	stmt = cals_query_prepare(sql_value);
	retvm_if(NULL == stmt, CAL_ERR_DB_FAILED, "cals_query_prepare() Failed");

	if(NULL == *iter)
	{
		*iter = calloc(1, sizeof(cal_iter));
		if (NULL == *iter) {
			sqlite3_finalize(stmt);
			ERR("calloc() Failed(%d)", errno);
			return CAL_ERR_OUT_OF_MEMORY;
		}
	}

	(*iter)->stmt = stmt;
	(*iter)->i_type = CAL_STRUCT_TYPE_SCHEDULE;

	return CAL_SUCCESS;
}


API int calendar_svc_find_recurring_event_list(int account_id, cal_iter **iter)
{
	CALS_FN_CALL;
	sqlite3_stmt *stmt = NULL;
	char sql_value[512] = {0};

	retv_if(NULL == iter, CAL_ERR_ARG_NULL);

	*iter = calloc(1, sizeof(cal_iter));
	retvm_if(NULL == *iter, CAL_ERR_OUT_OF_MEMORY, "calloc() Failed(%d)", errno);

	if(0 != account_id)
		snprintf(sql_value, sizeof(sql_value), "select * from %s where repeat_item > 0 and is_deleted = 0 and account_id = %d",
			CALS_TABLE_SCHEDULE, account_id);
	else
		snprintf(sql_value, sizeof(sql_value), "select * from %s where repeat_item > 0 and is_deleted = 0;",
			CALS_TABLE_SCHEDULE);

	stmt = cals_query_prepare(sql_value);
	if (NULL == stmt) {
		ERR("cals_query_prepare() Failed");
		free(*iter);
		return CAL_ERR_DB_FAILED;
	}

	(*iter)->stmt = stmt;
	(*iter)->i_type = CAL_STRUCT_TYPE_SCHEDULE;

	return CAL_SUCCESS;
}


API int calendar_svc_find_event_list_by_filter(int account_id, int filter_count, const char *search_type[], const void *search_value[], cal_iter **iter)
{
	CALS_FN_CALL;

	int i, j, cond_len = 0;
	int rc = 0;
	sqlite3_stmt *stmt = NULL;
	char sql_value[2560] = {0};
	char sql_condition_value[2048] = {0};
	char sql_temp[256] = {0};
	cal_value_type_t value_type = 0;
	bool category_table_used = false;

	retex_if(0 >= filter_count,,"Invalid parameter(filter_count).\n");
	retex_if(NULL == iter,,"Invalid parameter(iter).\n");

	if(*iter == NULL)
	{
		*iter = malloc(sizeof(cal_iter));
		retex_if(NULL == *iter,,"Failed to malloc!\n");
	}
	memset(*iter,0x00,sizeof(cal_iter));

	for (i=0; i<filter_count; i++)
	{
		value_type = __calendar_svc_get_type(CAL_STRUCT_TYPE_SCHEDULE, search_type[i]);
		memset(sql_temp, 0, sizeof(sql_temp));
		switch(value_type)
		{
		case VALUE_TYPE_TEXT:
			if(search_value[i] == NULL)
			{
				return CAL_ERR_ARG_NULL;
			}
			if(!strncmp(CAL_VALUE_TXT_MEETING_CATEGORY_DETAIL_NAME, search_type[i], 128))
			{
				snprintf(sql_temp, sizeof(sql_temp), "schedule_table.id = cal_meeting_category_table.event_id and cal_meeting_category_table.category_name like upper('%s') and ", (char*)search_value[i]);
				category_table_used = true;
			}
			else
			{
				snprintf(sql_temp, sizeof(sql_temp), "upper(%s) like upper('%s') and ", search_type[i], (char*)search_value[i]);
			}
			break;
		case VALUE_TYPE_INT:
			if(!strncmp(CAL_VALUE_INT_MEETING_STATUS, search_type[i], 128))
			{
				int len;
				char *status_string = (char*)search_value[i];
				int status_length = strlen(status_string);
				len = snprintf(sql_temp, sizeof(sql_temp), "(");
				CALS_DBG("status string: %s", status_string);
				for(j=0;j<status_length;j++)
				{
					len += snprintf(sql_temp+len, sizeof(sql_temp)-len, "%s = %c ", search_type[i], status_string[j]);
					if(status_length-1 > j)
						len += snprintf(sql_temp+len, sizeof(sql_temp)-len, "or ");
				}
				len += snprintf(sql_temp+len, sizeof(sql_temp)-len, ") and ");
				if(sizeof(sql_temp) <= len) {
					ERR("condition is too long");
					free(*iter);
					return CAL_ERR_ARG_INVALID;
				}
			}
			else
			{
				snprintf(sql_temp, sizeof(sql_temp), "%s = %d and", search_type[i], (int)search_value[i]);
			}
			break;
		case VALUE_TYPE_TIME:
			// Doesn't consider the recurrence.
			if(!strncmp(CAL_VALUE_GMT_START_DATE_TIME, search_type[i], 128)) // start time min value
			{
				snprintf(sql_temp, sizeof(sql_temp), "%s >= %d and ", search_type[i], (int)search_value[i]);
			}
			if(!strncmp(CAL_VALUE_GMT_END_DATE_TIME, search_type[i], 128)) // start time max value
			{
				snprintf(sql_temp, sizeof(sql_temp), "%s <= %d and ", CAL_VALUE_GMT_START_DATE_TIME, (int)search_value[i]);
			}
			break;
		case VALUE_TYPE_DOUBLE:
		case VALUE_TYPE_USER:
			retex_if(true,,"Can not find!\n");
		default:
			break;
		}

		cond_len += snprintf(sql_condition_value+cond_len, sizeof(sql_condition_value)-cond_len, "%s", sql_temp);
		if(sizeof(sql_condition_value) <= cond_len) {
			ERR("condition is too long");
			free(*iter);
			return CAL_ERR_ARG_INVALID;
		}
	}
	if(0 != account_id)
	{
		if(false==category_table_used)
			snprintf(sql_value, sizeof(sql_value), "select schedule_table.* from schedule_table where %s is_deleted = 0 and account_id = %d;", sql_condition_value, account_id);
		else
			snprintf(sql_value, sizeof(sql_value), "select schedule_table.* from schedule_table, cal_meeting_category_table where %s is_deleted = 0 and account_id = %d;", sql_condition_value, account_id);
	}
	else
	{
		if(false==category_table_used)
			snprintf(sql_value, sizeof(sql_value), "select schedule_table.* from schedule_table where %s is_deleted = 0;", sql_condition_value);
		else
			snprintf(sql_value, sizeof(sql_value), "select schedule_table.* from schedule_table, cal_meeting_category_table where %s is_deleted = 0;", sql_condition_value);
	}

	rc = sqlite3_prepare_v2(calendar_db_handle, sql_value, strlen(sql_value), &stmt, NULL);
	retex_if(rc != SQLITE_OK,,"[ERROR]calendar_svc_find_event_list_by_filter: Failed to get stmt!(%s)\n", sql_value);

	(*iter)->stmt = stmt;
	(*iter)->i_type = CAL_STRUCT_TYPE_SCHEDULE;
	return CAL_SUCCESS;
CATCH:
	if (iter && *iter != NULL)
	{
		CAL_FREE(*iter);
	}
	if (stmt != NULL)
	{
		sqlite3_finalize(stmt);
		stmt = NULL;
	}
	return CAL_ERR_FAIL;
}


static inline int __calendar_svc_find_calendar_list(int account_id,int calendar_id,const char *search_type,const void *search_value, cal_iter **iter)
{
	sqlite3_stmt *stmt = NULL;
	char sql_value[CALS_SQL_MIN_LEN] = {0};
	int rc = 0;
	cal_value_type_t value_type = 0;

	//retex_if(0 < account_id,,"[ERROR]calendar_svc_find_event_list:Invalid parameter(account_id)");
	retex_if(NULL == search_type ,," [ERROR]calendar_svc_find_event_list:Invalid parameter(search_type).\n");
	retex_if(NULL == iter,," [ERROR]calendar_svc_find_event_list:Invalid parameter(iter).\n");

	*iter = calloc(1, sizeof(cal_iter));
	retvm_if(NULL == *iter, CAL_ERR_OUT_OF_MEMORY, "calloc() Failed(%d)", errno);

	value_type = __calendar_svc_get_type(CAL_STRUCT_TYPE_CALENDAR,search_type);

	switch(value_type)
	{
	case VALUE_TYPE_TEXT:
		if(account_id)
			sprintf(sql_value,
					"select rowid,* from calendar_table where %s like '%%%s%%' and is_deleted = 0 and account_id = %d ORDER BY start_date_time;",
					search_type, (char*)search_value, account_id);
		else
			sprintf(sql_value,
					"select rowid,* from calendar_table where %s like '%%%s%%' and is_deleted = 0 ORDER BY start_date_time;",
					search_type, (char*)search_value);
		break;
	case VALUE_TYPE_INT:
		if(account_id)
			sprintf(sql_value,
					"select rowid,* from calendar_table where %s = %d and is_deleted = 0 and account_id = %d ORDER BY start_date_time;",
					search_type, (int)search_value, account_id);
		else
			sprintf(sql_value,
					"select rowid,* from calendar_table where %s = %d and is_deleted = 0 ORDER BY start_date_time;",
					search_type, (int)search_value);
		break;
	case VALUE_TYPE_TIME:
	case VALUE_TYPE_DOUBLE:
	case VALUE_TYPE_USER:
	default:
		retex_if(true,,"Can not find!\n");
	}

	rc = sqlite3_prepare_v2(calendar_db_handle, sql_value, strlen(sql_value), &stmt, NULL);
	retex_if(rc != SQLITE_OK,,"[ERROR]calendar_svc_find_event_list:Failed to get stmt!(%s)\n",sql_value);

	(*iter)->stmt = stmt;
	(*iter)->i_type = CAL_STRUCT_TYPE_SCHEDULE;

	return CAL_SUCCESS;

CATCH:

	if (iter && *iter != NULL)
	{
		CAL_FREE(*iter);
	}

	if (stmt != NULL)
	{
		sqlite3_finalize(stmt);
		stmt = NULL;
	}


	return CAL_ERR_FAIL;

}


static inline int __calendar_svc_find_timezone_list(int account_id,int calendar_id,const char *search_type,const void *search_value, cal_iter **iter)
{
	sqlite3_stmt *stmt = NULL;
	char sql_value[CALS_SQL_MIN_LEN] = {0};
	int rc = 0;
	cal_value_type_t value_type = 0;

	//retex_if(0 < account_id,,"[ERROR]calendar_svc_find_event_list:Invalid parameter(account_id)");
	retex_if(NULL == search_type ,,"Invalid parameter(search_type)");
	retex_if(NULL == iter,,"Invalid parameter(iter)");

	*iter = calloc(1, sizeof(cal_iter));
	retvm_if(NULL == *iter, CAL_ERR_OUT_OF_MEMORY, "calloc() Failed(%d)", errno);

	value_type = __calendar_svc_get_type(CAL_STRUCT_TYPE_TIMEZONE,search_type);

	switch(value_type)
	{
	case VALUE_TYPE_TEXT:
		if(0 != account_id)
			sprintf(sql_value,"select rowid,* from timezone_table where %s like '%%%s%%' and account_id = %d;", search_type, (char*)search_value, account_id);
		else
			sprintf(sql_value,"select rowid,* from timezone_table where %s like '%%%s%%' ;", search_type, (char*)search_value);

		break;
	case VALUE_TYPE_INT:
		if(0 != account_id)
			sprintf(sql_value,"select rowid,* from timezone_table where %s = %d and account_id = %d;", search_type, (int)search_value, account_id);
		else
			sprintf(sql_value,"select rowid,* from timezone_table where %s = %d ;", search_type, (int)search_value);
		break;
	case VALUE_TYPE_TIME:
	case VALUE_TYPE_DOUBLE:
	case VALUE_TYPE_USER:
	default:
		retex_if(true,,"Can not find!\n");
	}

	rc = sqlite3_prepare_v2(calendar_db_handle, sql_value, strlen(sql_value), &stmt, NULL);
	retex_if(rc != SQLITE_OK,,"ailed to get stmt!(%s)",sql_value);

	(*iter)->stmt = stmt;
	(*iter)->i_type = CAL_STRUCT_TYPE_TIMEZONE;

	return CAL_SUCCESS;

CATCH:

	if (iter && *iter != NULL)
	{
		CAL_FREE(*iter);
	}

	if (stmt != NULL)
	{
		sqlite3_finalize(stmt);
		stmt = NULL;
	}


	return CAL_ERR_FAIL;

}


API int calendar_svc_find(int account_id,int calendar_id,const char *data_type,const char *search_type,const void *search_value, cal_iter **iter)
{
	retv_if(NULL == data_type, CAL_ERR_ARG_NULL);

	if(0 == strcmp(data_type,CAL_STRUCT_SCHEDULE))
	{
		calendar_svc_find_event_list(account_id,search_type,search_value,iter);
	}
	else if(0 == strcmp(data_type,CAL_STRUCT_CALENDAR))
	{
		__calendar_svc_find_calendar_list(account_id, calendar_id, search_type, search_value, iter);
	}
	else if(0 == strcmp(data_type,CAL_STRUCT_TIMEZONE))
	{
		__calendar_svc_find_timezone_list(account_id, calendar_id, search_type, search_value, iter);
	}
	else {
		ERR("Unknown Type(%s)", data_type);
		return CAL_ERR_FAIL;
	}

	return CAL_SUCCESS;
}


API int calendar_svc_get_updated_event_list(int account_id, time_t timestamp, cal_iter **iter)
{
	CALS_FN_CALL;
	sqlite3_stmt *stmt = NULL;
	char sql_value[CALS_SQL_MIN_LEN] = {0};

	cal_iter *iter_value = NULL;

	retv_if(NULL == iter, CAL_ERR_ARG_NULL);
	retvm_if(timestamp < 0, CAL_ERR_ARG_INVALID, "timestamp(%ld) is Invalid", timestamp);

	iter_value = calloc(1, sizeof(cal_iter));
	retvm_if(NULL == iter_value, CAL_ERR_OUT_OF_MEMORY, "calloc() Failed");

	if (account_id)
		sprintf(sql_value,"SELECT * FROM %s WHERE last_modified_time > %ld AND account_id = %d",
				CALS_TABLE_SCHEDULE, timestamp, account_id);
	else
		sprintf(sql_value,"SELECT * FROM %s WHERE last_modified_time > %ld",
				CALS_TABLE_SCHEDULE, timestamp);

	stmt = cals_query_prepare(sql_value);
	if (NULL == stmt) {
		ERR("cals_query_prepare() Failed");
		free(iter_value);
		return CAL_ERR_DB_FAILED;
	}

	iter_value->stmt = stmt;
	iter_value->i_type = CAL_STRUCT_TYPE_SCHEDULE;
	*iter = iter_value;

	return CAL_SUCCESS;
}



API int calendar_svc_get_month_event_list_by_period(int account_id,
	time_t startdate, time_t enddate, cal_iter **iter)
{
	CALS_FN_CALL;
	sqlite3_stmt *stmt = NULL;
	char sql_value[CALS_SQL_MIN_LEN] = {0};
	time_t gm_startdate = 0;
	time_t gm_enddate =0;

	//retex_if(0 > account_id,,"[ERROR]calendar_svc_get_event_list_by_period:Invalid parameter(account_id)!\n");
	retv_if(NULL == iter, CAL_ERR_ARG_NULL);
	retv_if(startdate < 0, CAL_ERR_ARG_INVALID);
	retv_if(enddate < 0, CAL_ERR_ARG_INVALID);

	//switch((*iter)->i_type)
	//{
	//	case CAL_STRUCT_TYPE_SCHEDULE:
	//calendar_svc_util_local_to_gmt(startdate,&gm_startdate);
	//calendar_svc_util_local_to_gmt(enddate,&gm_enddate);
	gm_startdate = startdate - SECSPERDAY;
	gm_enddate = enddate + SECSPERDAY;

	*iter = calloc(1, sizeof(cal_iter));
	retvm_if(NULL == *iter, CAL_ERR_OUT_OF_MEMORY,"calloc() Failed(%d)", errno);

	(*iter)->is_patched = FALSE;

	if(account_id == ALL_VISIBILITY_ACCOUNT) {
		sprintf(sql_value,"select schedule_table.* from schedule_table,calendar_table "\
				"where schedule_table.type=%d and schedule_table.is_deleted = 0 "\
				"and schedule_table.calendar_id = calendar_table.rowid and calendar_table.visibility = 1 "\
				"and ((schedule_table.repeat_item = 0 and schedule_table.start_date_time <= %d and schedule_table.end_date_time >=%d) "\
				"or (schedule_table.repeat_item<>0 and schedule_table.repeat_end_date>=%d)) limit 10;"
				,CAL_EVENT_SCHEDULE_TYPE, (int)gm_enddate,(int)gm_startdate, (int)gm_startdate);
	} else {
		if(0 != account_id) {
			sprintf(sql_value,"select * from schedule_table where type=%d and is_deleted = 0 and account_id = %d and\
					((repeat_item = 0 and start_date_time <= %d and end_date_time >=%d) \
					 or (repeat_item<>0 and repeat_end_date>=%d));"
					,CAL_EVENT_SCHEDULE_TYPE,account_id, (int)gm_enddate,(int)gm_startdate, (int)gm_startdate);
		} else {
			sprintf(sql_value,"select * from schedule_table where type=%d and is_deleted = 0 and\
					((repeat_item = 0 and start_date_time <= %d and end_date_time >=%d) \
					 or (repeat_item<>0 and repeat_end_date>=%d));"
					,CAL_EVENT_SCHEDULE_TYPE, (int)gm_enddate,(int)gm_startdate, (int)gm_startdate);
		}
	}
	stmt = cals_query_prepare(sql_value);
	if (NULL == stmt) {
		ERR("cals_query_prepare() Failed");
		free(*iter);
		return CAL_ERR_DB_FAILED;
	}

	(*iter)->stmt = stmt;
	(*iter)->i_type = CAL_STRUCT_TYPE_SCHEDULE;

	return CAL_SUCCESS;
}


API int calendar_svc_get_event_list_by_period (int account_id, time_t startdate, time_t enddate, cal_iter **iter)
{
	CALS_FN_CALL;
	sqlite3_stmt *stmt = NULL;
	char sql_value[CALS_SQL_MIN_LEN] = {0};
	int rc = 0;
	time_t gm_startdate = 0;
	time_t gm_enddate =0;

	//retex_if(0 > account_id,,"[ERROR]calendar_svc_get_event_list_by_period:Invalid parameter(account_id)!\n");
	retex_if(NULL == iter,,"[ERROR]calendar_svc_get_event_list_by_period:Invalid parameter(iter)!\n");
	retex_if(startdate < 0,,"[ERROR]calendar_svc_get_event_list_by_period:Invalid parameter(startdate)!\n");
	retex_if(enddate < 0,,"[ERROR]calendar_svc_get_event_list_by_period:Invalid parameter(enddate)!\n");

	//switch((*iter)->i_type)
	//{
	//	case CAL_STRUCT_TYPE_SCHEDULE:
	gm_startdate = startdate - SECSPERDAY;
	gm_enddate = enddate + SECSPERDAY;

	*iter = calloc(1, sizeof(cal_iter));
	retvm_if(NULL == *iter, CAL_ERR_OUT_OF_MEMORY, "calloc() Failed(%d)", errno);

	(*iter)->is_patched = FALSE;

	if(account_id == ALL_VISIBILITY_ACCOUNT)
	{
		sprintf(sql_value,"select schedule_table.* from schedule_table,calendar_table "\
				"where schedule_table.type=%d and schedule_table.is_deleted = 0 "\
				"and schedule_table.calendar_id = calendar_table.rowid and calendar_table.visibility = 1 "\
				"and ((schedule_table.repeat_item = 0 and schedule_table.start_date_time <= %d and schedule_table.end_date_time >=%d) "\
				"or (schedule_table.repeat_item<>0 and schedule_table.repeat_end_date>=%d));"
				,CAL_EVENT_SCHEDULE_TYPE, (int)gm_enddate,(int)gm_startdate, (int)gm_startdate);
	}
	else
	{

		if(0 != account_id)
		{
			sprintf(sql_value,"select * from schedule_table where type=%d and is_deleted = 0 and account_id = %d and\
					((repeat_item = 0 and start_date_time <= %d and end_date_time >=%d) \
					 or (repeat_item<>0 and repeat_end_date>=%d));"
					,CAL_EVENT_SCHEDULE_TYPE,account_id, (int)gm_enddate,(int)gm_startdate, (int)gm_startdate);
		}
		else
		{
			sprintf(sql_value,"select * from schedule_table where type=%d and is_deleted = 0 and\
					((repeat_item = 0 and start_date_time <= %d and end_date_time >=%d) \
					 or (repeat_item<>0 and repeat_end_date>=%d));"
					,CAL_EVENT_SCHEDULE_TYPE, (int)gm_enddate,(int)gm_startdate, (int)gm_startdate);
		}

	}
	rc = sqlite3_prepare_v2(calendar_db_handle, sql_value, strlen(sql_value), &stmt, NULL);
	retex_if(rc != SQLITE_OK,printf("errmsg is %s\n",sqlite3_errmsg(calendar_db_handle)),"[ERROR]calendar_svc_get_event_list_by_period:Failed to get stmt!\n");


	(*iter)->stmt = stmt;
	(*iter)->i_type = CAL_STRUCT_TYPE_SCHEDULE;

	return CAL_SUCCESS;

CATCH:

	if (iter)
	{
		CAL_FREE(*iter);
	}

	if (stmt != NULL)
	{
		sqlite3_finalize(stmt);
		stmt = NULL;
	}

	return CAL_ERR_FAIL;
}



API int calendar_svc_get_event_list_by_tm_period (int account_id,int calendar_id, struct tm* startdate, struct tm* enddate, cal_iter **iter)
{
	CALS_FN_CALL;
	sqlite3_stmt *stmt = NULL;
	char sql_value[CALS_SQL_MIN_LEN] = {0};
	int rc = 0;

	time_t gm_startdate = 0;
	time_t gm_enddate =0;
	time_t local_startdate = 0;
	time_t local_enddate =0;
	//retex_if(0 > account_id,,"[ERROR]calendar_svc_get_event_list_by_period:Invalid parameter(account_id)!\n");
	retex_if(NULL == iter,,"[ERROR]calendar_svc_get_event_list_by_period:Invalid parameter(iter)!\n");
	retex_if(startdate < 0,,"[ERROR]calendar_svc_get_event_list_by_period:Invalid parameter(startdate)!\n");
	retex_if(enddate < 0,,"[ERROR]calendar_svc_get_event_list_by_period:Invalid parameter(enddate)!\n");

	//switch((*iter)->i_type)
	//{
	//	case CAL_STRUCT_TYPE_SCHEDULE:
	local_startdate = cals_mktime(startdate);
	local_enddate = cals_mktime(enddate);

	calendar_svc_util_local_to_gmt(local_startdate,&gm_startdate);
	calendar_svc_util_local_to_gmt(local_enddate,&gm_enddate);

	*iter = calloc(1, sizeof(cal_iter));
	retvm_if(NULL == *iter, CAL_ERR_OUT_OF_MEMORY, "calloc() Failed(%d)", errno);
	(*iter)->is_patched = FALSE;

	if(account_id == ALL_VISIBILITY_ACCOUNT)
	{
		sprintf(sql_value,"select schedule_table.* from schedule_table,calendar_table "\
				"where schedule_table.type=%d and schedule_table.is_deleted = 0 "\
				"and schedule_table.calendar_id = calendar_table.rowid and calendar_table.visibility = 1 "\
				"and ((schedule_table.repeat_item = 0 and schedule_table.start_date_time <= %d and schedule_table.end_date_time >=%d) "\
				"or (schedule_table.repeat_item<>0 and schedule_table.repeat_end_date>=%d));"
				,CAL_EVENT_SCHEDULE_TYPE,(int)gm_enddate,(int)gm_startdate, (int)gm_startdate);

		DBG("%s",sql_value);
	}
	else
	{

		if(0 != calendar_id)
		{
			sprintf(sql_value,"select * from schedule_table where type=%d and is_deleted = 0 and calendar_id = %d and\
					((repeat_item = 0 and start_date_time <= %d and end_date_time >=%d) \
					 or (repeat_item<>0 and repeat_end_date>=%d));"
					,CAL_EVENT_SCHEDULE_TYPE,calendar_id, (int)gm_enddate,(int)gm_startdate, (int)gm_startdate);
		}
		else if(0 != account_id)
		{
			sprintf(sql_value,"select * from schedule_table where type=%d and is_deleted = 0 and account_id = %d and\
					((repeat_item = 0 and start_date_time <= %d and end_date_time >=%d) \
					 or (repeat_item<>0 and repeat_end_date>=%d));"
					,CAL_EVENT_SCHEDULE_TYPE,account_id, (int)gm_enddate,(int)gm_startdate, (int)gm_startdate);
		}
		else
		{
			sprintf(sql_value,"select * from schedule_table where type=%d and is_deleted = 0 and\
					((repeat_item = 0 and start_date_time <= %d and end_date_time >=%d) \
					 or (repeat_item<>0 and repeat_end_date>=%d));"
					,CAL_EVENT_SCHEDULE_TYPE, (int)gm_enddate,(int)gm_startdate, (int)gm_startdate);
		}
	}

	rc = sqlite3_prepare_v2(calendar_db_handle, sql_value, strlen(sql_value), &stmt, NULL);
	retex_if(rc != SQLITE_OK,,"[ERROR]calendar_svc_get_event_list_by_period:Failed to get stmt!\n");

	(*iter)->stmt = stmt;
	(*iter)->i_type = CAL_STRUCT_TYPE_SCHEDULE;

	return CAL_SUCCESS;

CATCH:

	if (iter)
	{
		CAL_FREE(*iter);
	}

	if (stmt != NULL)
	{
		sqlite3_finalize(stmt);
		stmt = NULL;
	}

	return CAL_ERR_FAIL;
}


API int calendar_svc_convert_id_to_uid(const char *data_type,int index,char **uid)
{
	int 	rc = -1;
	char	sql_value[CALS_SQL_MAX_LEN] = {0};
	sqlite3_stmt *stmt = NULL;
	int return_value = CAL_SUCCESS;

	retex_if(uid == NULL,return_value = CAL_ERR_ARG_NULL, "The calendar database hasn't been opened.");

	retex_if(NULL == calendar_db_handle, return_value = CAL_ERR_DB_NOT_OPENED, "The calendar database hasn't been opened.");

	//sprintf(sql_value, "select * from cal_participant_table where event_id = %d;", panticipant_index);
	// TODO: make query!!!!
	if((0 == strcmp(data_type,CAL_STRUCT_SCHEDULE)) || (0 == strcmp(data_type,CAL_STRUCT_TODO)))
	{
		sprintf(sql_value,"select uid from schedule_table where id=%d;",index);
	}
	else if(0 == strcmp(data_type,CAL_STRUCT_CALENDAR))
	{
		sprintf(sql_value,"select uid from calendar_table where rowid=%d;",index);
	}


	rc = sqlite3_prepare_v2(calendar_db_handle, sql_value, strlen(sql_value), &stmt, NULL);
	retex_if(rc != SQLITE_OK, return_value = CAL_ERR_DB_FAILED, "Failed to get stmt!!");

	rc = sqlite3_step(stmt);
	retex_if(rc!= SQLITE_ROW && rc!= SQLITE_OK && rc!= SQLITE_DONE, return_value = CAL_ERR_DB_FAILED, "[ERROR]cal_db_service_get_participant_info_by_index:Query error !!");

	cal_db_get_text_from_stmt(stmt,uid,0);

	if (stmt != NULL)
	{
		sqlite3_finalize(stmt);
		stmt = NULL;
	}

	return CAL_SUCCESS;

CATCH:

	if (stmt != NULL)
	{
		sqlite3_finalize(stmt);
		stmt = NULL;
	}

	return return_value;
}

int calendar_svc_convert_uid_to_id(const char *data_type,char *uid,int *index)
{
	int 	rc = -1;
	char	sql_value[CALS_SQL_MAX_LEN] = {0};
	sqlite3_stmt *stmt = NULL;
	int return_value = CAL_SUCCESS;

	retex_if(uid == NULL, return_value = CAL_ERR_ARG_NULL, "The calendar database hasn't been opened.");

	retex_if(NULL == calendar_db_handle, return_value = CAL_ERR_DB_NOT_OPENED, "The calendar database hasn't been opened.");

	//sprintf(sql_value, "select * from cal_participant_table where event_id = %d;", panticipant_index);
	// TODO: make query!!!!
	if((0 == strcmp(data_type,CAL_STRUCT_SCHEDULE)) || (0 == strcmp(data_type,CAL_STRUCT_TODO)))
	{
		sprintf(sql_value,"select id from schedule_table where uid=%s;",uid);
	}
	else if(0 == strcmp(data_type,CAL_STRUCT_CALENDAR))
	{
		sprintf(sql_value,"select rowid from calendar_table where uid=%s;",uid);
	}


	rc = sqlite3_prepare_v2(calendar_db_handle, sql_value, strlen(sql_value), &stmt, NULL);
	retex_if(rc != SQLITE_OK, return_value = CAL_ERR_DB_FAILED, "Failed to get stmt!!");

	rc = sqlite3_step(stmt);
	retex_if(rc!= SQLITE_ROW && rc!= SQLITE_OK && rc!= SQLITE_DONE, return_value = CAL_ERR_DB_FAILED, "[ERROR]cal_db_service_get_participant_info_by_index:Query error !!");

	*index = sqlite3_column_int(stmt,0);

	if (stmt != NULL)
	{
		sqlite3_finalize(stmt);
		stmt = NULL;
	}

	return CAL_SUCCESS;

CATCH:

	if (stmt != NULL)
	{
		sqlite3_finalize(stmt);
		stmt = NULL;
	}

	return return_value;
}


API int calendar_svc_iter_get_info(cal_iter *iter, cal_struct **row_event)
{
	cal_sch_full_t *sch_record = NULL;
	calendar_t *cal_record = NULL;
	cal_timezone_t *tz_record = NULL;

	retv_if(NULL == iter, CAL_ERR_ARG_NULL);
	retv_if(NULL == iter->stmt, CAL_ERR_ARG_INVALID);
	retv_if(NULL == row_event, CAL_ERR_ARG_NULL);

	if(iter->is_patched!=TRUE)
	{
		int ret = calendar_svc_iter_next(iter);
		if(ret == CAL_ERR_FINISH_ITER)
			return CAL_ERR_NO_DATA;
		else if(ret != CAL_SUCCESS)
			return ret;
	}

	int error_code = 0;

	switch(iter->i_type)
	{
	case CAL_STRUCT_TYPE_SCHEDULE_LIST:
		if (NULL == *row_event)
		{
			*row_event = calendar_svc_struct_new(CAL_STRUCT_SCHEDULE);
			retvm_if(NULL == *row_event, CAL_ERR_FAIL, "calendar_svc_struct_new(SCHEDULE) Failed");
		}
		sch_record = (*row_event)->user_data;
		retvm_if(NULL == sch_record, CAL_ERR_FAIL, "row_event is Invalid");

		cal_db_service_convert_stmt_to_list_field_record(iter->stmt,sch_record,true);
		break;
	case CAL_STRUCT_TYPE_SCHEDULE:
		if (NULL == *row_event)
		{
			*row_event = calendar_svc_struct_new(CAL_STRUCT_SCHEDULE);
			retvm_if(NULL == *row_event, CAL_ERR_FAIL, "calendar_svc_struct_new(SCHEDULE) Failed");
		}
		sch_record = (cal_sch_full_t*)(*row_event)->user_data;
		retvm_if(NULL == sch_record, CAL_ERR_FAIL, "row_event is Invalid");

		cals_stmt_get_full_schedule(iter->stmt,sch_record,true);

		cal_db_service_get_participant_info_by_index(sch_record->index,&(sch_record->attendee_list),&error_code);
		cal_db_service_get_meeting_category_info_by_index(sch_record->index,&(sch_record->meeting_category),&error_code);
		cal_db_service_get_recurrency_exception(sch_record->index,&(sch_record->exception_date_list),&error_code);
		cals_get_alarm_info(sch_record->index, &(sch_record->alarm_list));

		break;
	case CAL_STRUCT_TYPE_TODO:
		if (NULL == *row_event)
		{
			*row_event = calendar_svc_struct_new(CAL_STRUCT_TODO);
			retvm_if(NULL == *row_event, CAL_ERR_FAIL, "calendar_svc_struct_new(TODO) Failed");
		}
		sch_record = (cal_sch_full_t*)(*row_event)->user_data;
		retvm_if(NULL == sch_record, CAL_ERR_FAIL, "row_event is Invalid");

		cals_stmt_get_full_schedule(iter->stmt,sch_record,true);

		break;

	case CAL_STRUCT_TYPE_CALENDAR:
		if (NULL == *row_event)
		{
			*row_event = calendar_svc_struct_new(CAL_STRUCT_CALENDAR);
			retvm_if(NULL == *row_event, CAL_ERR_FAIL, "calendar_svc_struct_new(CALENDAR) Failed");
		}
		cal_record = (calendar_t*)(*row_event)->user_data;
		retvm_if(NULL == cal_record, CAL_ERR_FAIL, "row_event is Invalid");

		cals_stmt_get_calendar(iter->stmt,cal_record);

		break;
	case CAL_STRUCT_TYPE_TIMEZONE:
		if (NULL == *row_event)
		{
			*row_event = calendar_svc_struct_new(CAL_STRUCT_TIMEZONE);
			retvm_if(NULL == *row_event, CAL_ERR_FAIL, "calendar_svc_struct_new(TIMEZONE) Failed");
		}
		tz_record = (cal_timezone_t*)(*row_event)->user_data;
		retvm_if(NULL == tz_record, CAL_ERR_FAIL, "row_event is Invalid");

		cal_db_service_convert_stmt_to_tz_info(iter->stmt,tz_record);
		break;
	default:
		break;
	}

	return CAL_SUCCESS;
}


API int calendar_svc_iter_next(cal_iter *iter)
{
	int ret = 0;

	retv_if(NULL == iter, CAL_ERR_ARG_NULL);

	if (iter->is_patched == FALSE)
		iter->is_patched = TRUE;

	ret = cals_stmt_step(iter->stmt);
	retvm_if(ret < CAL_SUCCESS, ret, "cals_stmt_step() Failed(%d)", ret);

	if (CAL_SUCCESS == ret)
		return CAL_ERR_FINISH_ITER;

	return CAL_SUCCESS;
}

API int calendar_svc_iter_remove(cal_iter **iter)
{
	CALS_FN_CALL;
	retv_if(NULL == iter, CAL_ERR_ARG_NULL);
	retv_if(NULL == *iter, CAL_ERR_ARG_NULL);

	if ((*iter)->stmt)
	{
		sqlite3_finalize((*iter)->stmt);
		(*iter)->stmt = NULL;
	}

	free(*iter);
	*iter = NULL;

	return CAL_SUCCESS;
}


API int calendar_svc_util_next_valid_event(cal_struct *event, time_t start_time,
	time_t end_time, time_t *next_valid_start_time, time_t *next_valid_end_time)
{
	CALS_FN_CALL;
	int ret = 0;
	cal_sch_full_t *temp_sch_full;
	static cal_date_param_t cal_date_param;
	struct tm start_tm, end_tm;
	struct tm event_start_tm = {0};
	struct tm event_end_tm = {0};

	retv_if(NULL == event, CAL_ERR_ARG_NULL);
	retv_if(NULL == event->user_data, CAL_ERR_ARG_INVALID);
	retv_if(NULL == next_valid_start_time, CAL_ERR_ARG_NULL);
	retv_if(NULL == next_valid_end_time, CAL_ERR_ARG_NULL);

	retvm_if(start_time < 0, CAL_ERR_ARG_INVALID, "start_time(%ld) is Invalid", start_time);
	retvm_if(end_time < 0, CAL_ERR_ARG_INVALID, "end_time(%ld) is Invalid", end_time);

	temp_sch_full = event->user_data;

	memcpy(&event_start_tm,cals_tmtime(&start_time),sizeof(struct tm));
	memcpy(&event_end_tm,cals_tmtime(&end_time),sizeof(struct tm));

	if((*next_valid_start_time == 0) && (*next_valid_end_time == 0))
	{
		cal_service_set_date_param(&cal_date_param, temp_sch_full);
	}

	ret = cal_db_service_get_next_valid_exception_time(temp_sch_full,
		&cal_date_param,cal_date_param.exception_date_list,&event_start_tm,&event_end_tm,&start_tm,&end_tm);
	retvm_if(ret, ret, "cal_db_service_get_next_valid_exception_time() Failed(%d)", ret);

	*next_valid_start_time = mktime(&start_tm);
	*next_valid_end_time = mktime(&end_tm);

	//ERR("%d-start_tm(%d/%d/%d %d)",temp_sch_full->index, start_tm.tm_year+1900,start_tm.tm_mon+1,start_tm.tm_mday,start_tm.tm_hour);
	//ERR("%d-end_tm(%d/%d/%d %d)",temp_sch_full->index,end_tm.tm_year+1900,end_tm.tm_mon+1,end_tm.tm_mday,end_tm.tm_hour);

	return CAL_SUCCESS;
}


API int calendar_svc_util_next_valid_event_tm(cal_struct *event, struct tm *start_tm,
	struct tm *end_tm, struct tm *next_valid_start_tm, struct tm *next_valid_end_tm)
{
	int ret = 0;
	cal_sch_full_t *temp_sch_full;
	static cal_date_param_t cal_date_param;

	retv_if(NULL == event, CAL_ERR_ARG_NULL);
	retv_if(NULL == event->user_data, CAL_ERR_ARG_INVALID);
	retv_if(NULL == next_valid_start_tm, CAL_ERR_ARG_NULL);
	retv_if(NULL == next_valid_end_tm, CAL_ERR_ARG_NULL);
	retv_if(NULL == start_tm, CAL_ERR_ARG_NULL);
	retv_if(NULL == end_tm, CAL_ERR_ARG_NULL);

	temp_sch_full = (cal_sch_full_t *)event->user_data;

	if((next_valid_start_tm->tm_year == 0) && (next_valid_end_tm->tm_year == 0))
	{
		cal_service_set_date_param(&cal_date_param, temp_sch_full);
	}

	ret = cal_db_service_get_next_valid_exception_time(temp_sch_full,
		&cal_date_param,cal_date_param.exception_date_list,start_tm,end_tm,next_valid_start_tm,next_valid_end_tm);
	retvm_if(ret, ret, "cal_db_service_get_next_valid_exception_time() Failed(%d)", ret);

	CALS_DBG("%d-start_tm(%d/%d/%d %d)",temp_sch_full->index, next_valid_start_tm->tm_year+1900,next_valid_start_tm->tm_mon+1,next_valid_start_tm->tm_mday,next_valid_start_tm->tm_hour);
	CALS_DBG("%d-end_tm(%d/%d/%d %d)",temp_sch_full->index,next_valid_end_tm->tm_year+1900,next_valid_end_tm->tm_mon+1,next_valid_end_tm->tm_mday,next_valid_end_tm->tm_hour);

	return CAL_SUCCESS;
}


time_t calendar_svc_util_mk_localtime(struct tm* tmTime)
{
	retvm_if(NULL == tmTime,0,"Invalid parameter(tmTime)!\n");

	if(cal_svc_tm_value.is_initialize == false){
		cal_svc_set_tz_base_info(tmTime->tm_year); //local time
	}

	return cals_mktime(tmTime)-cal_svc_tm_value.localtime_offset;
}

API int calendar_svc_util_gmt_to_local (time_t fromTime, time_t *toTime)
{
	retex_if(toTime==NULL,,"[ERROR]calendar_svc_util_gmt_to_local:Invalid parameter(toTime)!\n");
	struct tm * cur_time = NULL;
	struct tm ttm;
	cur_time = gmtime_r(&fromTime,&ttm);

	retex_if(cur_time==NULL,,"[ERROR]calendar_svc_util_gmt_to_local:Invalid parameter(fromTime)!\n");

	if( (cal_svc_tm_value.is_initialize == false) ||
			((cal_svc_tm_value.local_dst_offset !=0) &&
			 (cal_svc_tm_value.start_local_dst_date_time.tm_year != cur_time->tm_year)) ){
		cal_svc_set_tz_base_info(cur_time->tm_year); //local time
	}

	if( (cal_svc_tm_value.local_dst_offset !=0) &&
			(__cal_service_compare_date(&cal_svc_tm_value.start_local_dst_date_time,cur_time) > 0) &&
			(__cal_service_compare_date(cur_time,&cal_svc_tm_value.start_local_std_date_time) > 0) )
	{
		*toTime = fromTime+cal_svc_tm_value.local_dst_offset;
	}
	else
		*toTime = fromTime+cal_svc_tm_value.localtime_offset;


	return CAL_SUCCESS;
CATCH:

	return CAL_ERR_FAIL;

}

API int calendar_svc_util_local_to_gmt (time_t fromTime, time_t *toTime)
{
	retex_if(toTime==NULL,,"[ERROR]calendar_svc_util_local_to_gmt:Invalid parameter(toTime)!\n");
	struct tm * cur_time = NULL;
	struct tm ttm;
	cur_time = gmtime_r(&fromTime,&ttm);
	retex_if(cur_time==NULL,,"[ERROR]calendar_svc_util_local_to_gmt:Invalid parameter(fromTime)!\n");


	if( (cal_svc_tm_value.is_initialize == false) ||
			((cal_svc_tm_value.local_dst_offset !=0) &&
			 (cal_svc_tm_value.start_local_dst_date_time.tm_year != cur_time->tm_year)) ){
		cal_svc_set_tz_base_info(cur_time->tm_year); //local time
	}

	if( (cal_svc_tm_value.local_dst_offset !=0) &&
			(__cal_service_compare_date(&cal_svc_tm_value.start_local_dst_date_time,cur_time) > 0) &&
			(__cal_service_compare_date(cur_time,&cal_svc_tm_value.start_local_std_date_time) > 0) )
	{
		*toTime = fromTime-cal_svc_tm_value.local_dst_offset;
	}
	else
		*toTime = fromTime-cal_svc_tm_value.localtime_offset;

	return CAL_SUCCESS;
CATCH:

	return CAL_ERR_FAIL;
}

static int get_num(const char *num)
{
	if(NULL == num)
	{
		return -1;
	}

	int temp = 0;

	if(isdigit(num[0]))
	{
		temp = num[0]-'0';
	}

	if(isdigit(num[1]))
	{
		temp *= 10;
		temp += num[1]-'0';
	}

	return temp;
}

static int get_week(const char *week)
{
	if(NULL == week)
	{
		return -1;
	}

	switch(week[0])
	{
	case 'M':
		return 1;

	case 'T':
		if(week[1] == 'u')
		{
			return 2;
		}
		else if(week[1] == 'h')
		{
			return 4;
		}
		else
		{
			return -1;
		}

	case 'W':
		return 3;

	case 'F':
		return 5;

	case 'S':
		if(week[1] == 'a')
		{
			return 6;
		}
		else if(week[1] == 'u')
		{
			return 0;
		}
		else
			return -1;
	default:
		return -1;

	}

}

static int get_month(const char *mon)
{
	if(NULL == mon)
	{
		return -1;
	}

	switch(mon[0])
	{
	case 'J':
		if(mon[1] == 'a')
		{
			return 0;
		}
		else if(mon[1] == 'u')
		{
			if(mon[2] == 'n')
			{
				return 5;
			}
			else
			{
				return 6;
			}

		}
		else
		{
			return -1;
		}

	case 'F':
		return 1;

	case 'M':
		if(mon[2] == 'r')
		{
			return 2;
		}
		else if(mon[2] == 'y')
		{
			return 4;
		}
		else
		{
			return -1;
		}

	case 'A':
		if(mon[1] == 'p')
		{
			return 3;
		}
		else if(mon[1] == 'g')
		{
			return 7;
		}
		else
		{
			return -1;
		}

	case 'S':
		return 8;

	case 'O':
		return 9;

	case 'N':
		return 10;

	case 'D':
		return 11;

	default:
		return -1;

	}

}

static int get_time(char *line, struct tm* time)
{
	if(NULL == line || NULL == time)
	{
		return CAL_ERR_FAIL;
	}

	char *p = strstr(line," = ");
	if(NULL != p)
	{
		p += 3;
		int temp = get_week(p);
		if(temp >= 0)
		{
			time->tm_wday = temp;
		}

		p += 4;
		temp = get_month(p);
		if(temp >= 0)
		{
			time->tm_mon = temp;
		}

		p += 4;
		temp = get_num(p);
		if(temp >= 0)
		{
			time->tm_mday = temp;
		}

		p += 3;
		temp = get_num(p);
		if(temp >= 0)
		{
			time->tm_hour = temp;
		}

		p += 3;
		temp = get_num(p);
		if(temp >= 0)
		{
			time->tm_min = temp;
		}

		p += 3;
		temp = get_num(p);
		if(temp >= 0)
		{
			time->tm_sec = temp;
		}

	}

	return CAL_SUCCESS;
}


static int get_offset(char *line, int *offset)
{
	if(NULL == line || NULL == time)
	{
		return CAL_ERR_FAIL;
	}

	char *p = strstr(line,"gmtoff=");
	if(NULL != p)
	{
		//ERR("%s",p);
		p=p+7;
		*offset = atoi(p);
	}

	return CAL_SUCCESS;
}


API int calendar_svc_get_tz_info(char *tz_file_name,int year, struct tm* dst_start, struct tm* dst_end,int *base_offset,int *dst_offset)
{
	CALS_FN_CALL;
	char line[1024] = {0};
	char cmd[1024] = {0};
	char year_str[10]= {0};
	bool is_start_time = true;
	bool is_set_base_offset = false;
	static FILE* fd = NULL;

	retex_if(NULL == tz_file_name || NULL == dst_start || NULL == dst_end,,"Invalid parameter!\n");

	//start->tm_year = year - 1900;
	//end->tm_year = year - 1900;

	snprintf(year_str,sizeof(year_str),"%4d UTC",year);

	snprintf(cmd,sizeof(cmd),"zdump %s -v -c %d,%d",tz_file_name,year-1,year+1);

	if(!(fd = popen(cmd, "r")))
		return CAL_ERR_FAIL;

	while(NULL != fgets(line,sizeof(line)-1,fd))
	{
		if(NULL != strstr(line,"isdst=1") && NULL != strstr(line,year_str))
		{
			if(is_start_time)
			{
				is_start_time = false;
				get_time(line,dst_start);
				dst_start->tm_year = year-1900;
				//ERR("%s %d-%d-%d %d",line,dst_start->tm_year,dst_start->tm_mon,dst_start->tm_mday,dst_start->tm_hour);
			}
			else
			{
				get_time(line,dst_end);
				get_offset(line,dst_offset);
				dst_end->tm_year = year-1900;
				//ERR("%s %d-%d-%d %d",line,dst_start->tm_year,dst_start->tm_mon,dst_start->tm_mday,dst_start->tm_hour);
			}

		}
		else if(is_set_base_offset == false)
		{
			get_offset(line,base_offset);
			//is_set_base_offset = true;
		}
	}

	pclose(fd);
	ERR("%s base:%d dst:%d",tz_file_name,*base_offset,*dst_offset);

	return CAL_SUCCESS;

CATCH:

	return CAL_ERR_FAIL;
}


API int calendar_svc_util_convert_db_time (struct tm* fromTime,char *fromTz, struct tm *toTime, char *toTz)
{
	struct tm start_dst_date_time;
	struct tm start_std_date_time;
	int tz_offset=0;
	int dst_offset=0;
	time_t base_tt;
	struct tm temp_tm;

	retex_if(fromTime==NULL,,"[ERROR]calendar_svc_util_local_to_gmt:Invalid parameter(enddate)!\n");
	retex_if(toTime==NULL,,"[ERROR]calendar_svc_util_local_to_gmt:Invalid parameter(enddate)!\n");
	retex_if(fromTz==NULL,,"[ERROR]calendar_svc_util_local_to_gmt:Invalid parameter(enddate)!\n");
	retex_if(toTz==NULL,,"[ERROR]calendar_svc_util_local_to_gmt:Invalid parameter(enddate)!\n");


	base_tt = cals_mktime(fromTime);
	if(base_tt < 0)
		return 0;
	calendar_svc_util_gmt_to_local(base_tt,&base_tt);
	cals_tmtime_r(&base_tt,&temp_tm);

	//ERR("temp_tm(%d/%d/%d %d)",temp_tm.tm_year+1900,temp_tm.tm_mon+1,temp_tm.tm_mday,temp_tm.tm_hour);

	if(strcmp(toTz,"GMT")!=0)
	{
		if(strcmp(toTz,cal_svc_tm_value.local_tz_name) == 0)
		{
			dst_offset = cal_svc_tm_value.local_dst_offset;
			tz_offset = cal_svc_tm_value.localtime_offset;
			memcpy(&start_dst_date_time,&cal_svc_tm_value.start_local_dst_date_time,sizeof(struct tm));
			memcpy(&start_std_date_time,&cal_svc_tm_value.start_local_std_date_time,sizeof(struct tm));
		}
		else if(strcmp(toTz,cal_svc_tm_value.temp_tz_name) == 0)
		{
			dst_offset = cal_svc_tm_value.temp_dst_offset;
			tz_offset = cal_svc_tm_value.temptime_offset;
			memcpy(&start_dst_date_time,&cal_svc_tm_value.start_temp_dst_date_time,sizeof(struct tm));
			memcpy(&start_std_date_time,&cal_svc_tm_value.start_temp_std_date_time,sizeof(struct tm));
		}
		else
		{
			calendar_svc_get_tz_info(toTz,fromTime->tm_year+1900,
					&start_dst_date_time,
					&start_std_date_time,
					&tz_offset,&dst_offset);
		}

		if( (dst_offset !=0) &&
				(__cal_service_compare_date(&start_dst_date_time,&temp_tm) > 0) &&
				(__cal_service_compare_date(&temp_tm,&start_std_date_time) > 0) )
		{
			base_tt = base_tt-dst_offset;
		}
		else
			base_tt = base_tt-tz_offset;
	}

	cals_tmtime_r(&base_tt,toTime);

	//ERR("tzinfo(%s:(%d->%d) %d, %d)",toTz,cals_mktime(fromTime),base_tt,tz_offset,dst_offset);
	//ERR("fromTime(%d/%d/%d %d)",fromTime->tm_year+1900,fromTime->tm_mon+1,fromTime->tm_mday,fromTime->tm_hour);
	//ERR("toTime(%d/%d/%d %d)\n\n",toTime->tm_year+1900,toTime->tm_mon+1,toTime->tm_mday,toTime->tm_hour);

	return CAL_SUCCESS;
CATCH:

	return CAL_ERR_FAIL;
}



API int calendar_svc_util_save_vcs_by_index (const int index, char *full_file_path)
{
	cal_sch_full_t sch_record_fulle = {0};
	int sch_count = 1;
	int error_code = 0;
	bool is_success = false;

	retv_if(NULL == full_file_path, CAL_ERR_ARG_NULL);

	is_success = cal_db_service_get_record_full_field_by_index(index,&sch_record_fulle,&error_code);
	retvm_if(!is_success, error_code, "cal_db_service_get_record_full_field_by_index() Failed(%d)", error_code);

	cal_convert_cal_data_to_vdata_file(&sch_record_fulle,sch_count,full_file_path,&error_code);
	cal_db_service_free_full_record(&sch_record_fulle,&error_code);

	return CAL_SUCCESS;
}

API int calendar_svc_util_register_vcs_file (const char *file_name)
{
	retv_if(NULL == file_name, CAL_ERR_ARG_NULL);

	return cal_vcalendar_register_vcs_file(file_name);
}

API int calendar_svc_util_convert_vcs_to_event (const char *raw_data,int data_size,cal_struct **record)
{
	CALS_FN_CALL;
	//cal_vcalendar_register_vcs_file(file_name);
	cal_sch_full_t *sch_array = NULL;
	int sch_count = 0;
	int error_code = 0;

	retex_if(NULL == record,,"[ERROR]calendar_svc_util_convert_vcs_to_event:Invalid parameter!\n");
	retex_if(NULL == raw_data,,"[ERROR]calendar_svc_util_convert_vcs_to_event:Invalid parameter!\n");

	bool is_success= FALSE;
	is_success = _cal_convert_vcalendar_to_cal_data(raw_data,&sch_array, &sch_count);

	if(is_success)
	{
		*record = (cal_struct*)malloc(sizeof(cal_struct));
		retex_if(NULL == *record,,"Failed to malloc!\n");

		(*record)->user_data = sch_array;
		//(*record)->event_type = CAL_STRUCT_TYPE_SCHEDULE;
		//SURC d.zakutailo 2010-06-03: (*record)->event_type must be obtained from _cal_convert_vcalendar_to_cal_data function. Don't use any predefined types directly.
		if (sch_array->cal_type == CAL_EVENT_SCHEDULE_TYPE) {
			(*record)->event_type = CAL_STRUCT_TYPE_SCHEDULE;
		} else
			if (sch_array->cal_type == CAL_EVENT_TODO_TYPE) {
				(*record)->event_type = CAL_STRUCT_TYPE_TODO;
			} else
				(*record)->event_type = sch_array->cal_type;
			return CAL_SUCCESS;
	}

CATCH:

	if(NULL != sch_array)
	{
		cal_db_service_free_full_record(sch_array,&error_code);
		CAL_FREE(sch_array);
	}

	return CAL_ERR_FAIL;
}

API int calendar_svc_util_convert_event_to_vcs (cal_struct *record,char **raw_data,int *data_size)
{
	bool is_success= FALSE;

	retv_if(NULL == record, CAL_ERR_ARG_NULL);
	retv_if(NULL == raw_data, CAL_ERR_ARG_NULL);

	is_success = _cal_convert_sch_to_vcalendar((cal_sch_full_t*)(record->user_data), 1,raw_data, CAL_VCAL_VER_1_0);
	retvm_if(!is_success, CAL_ERR_FAIL, "_cal_convert_sch_to_vcalendar() Failed");

	return CAL_SUCCESS;
}

/* will be removed */
int calendar_svc_util_set_calendar_timezone(const char *tzname)
{
	/* char zoneinfo[256]={0,};
		char cmd[256]={0,};
		sprintf(zoneinfo,"/usr/share/zoneinfo/%s",tzname);

	//calendar_svc_get_time_by_tzinfo(zoneinfo);
	//calendar_svc_get_time_by_tzinfo("/usr/share/zoneinfo/calendar_localtime");
	//sprintf(cmd,"rm -f /usr/share/zoneinfo/calendar_localtime");
	unlink("/usr/share/zoneinfo/calendar_localtime");
	sprintf(cmd,"ln -s %s /usr/share/zoneinfo/calendar_localtime",zoneinfo);
	execve(cmd);*/

	return CAL_SUCCESS;
}


/* hidden api */
API int calendar_svc_get_month_event_list (int account_id, time_t startdate, time_t enddate,int is_repeat, cal_iter **iter)
{
	CALS_FN_CALL;
	int ret;
	sqlite3_stmt *stmt = NULL;

	retv_if(NULL == iter, CAL_ERR_ARG_NULL);

	*iter = calloc(1, sizeof(cal_iter));
	retvm_if(NULL == *iter, CAL_ERR_OUT_OF_MEMORY, "calloc() Failed(%d)", errno);

	ret = cal_db_service_get_month_event(account_id, startdate, enddate, is_repeat, &stmt);
	if (CAL_SUCCESS != ret) {
		ERR("cal_db_service_get_month_event() Failed(%d)", ret);
		free(*iter);
		*iter = NULL;
		return ret;
	}

	(*iter)->is_patched = FALSE;
	(*iter)->stmt = stmt;
	(*iter)->i_type = CAL_STRUCT_TYPE_SCHEDULE;

	return CAL_SUCCESS;
}

API int calendar_svc_iter_get_month_info (cal_iter *iter, int is_repeat,cal_struct **row_event)
{
	CALS_FN_CALL;
	cal_sch_full_t *sch_record = NULL;

	retv_if(NULL == iter, CAL_ERR_ARG_NULL);
	retv_if(NULL == iter->stmt, CAL_ERR_ARG_NULL);
	retv_if(NULL == row_event, CAL_ERR_ARG_NULL);

	if(iter->is_patched!=TRUE)
	{
		int ret = calendar_svc_iter_next(iter);
		if(ret == CAL_ERR_FINISH_ITER)
			return CAL_ERR_NO_DATA;
		else if(ret != CAL_SUCCESS)
			return ret;
	}

	int error_code = 0;

	if(*row_event == NULL)
	{
		*row_event = calendar_svc_struct_new(CAL_STRUCT_SCHEDULE);
		retvm_if(NULL == *row_event, CAL_ERR_OUT_OF_MEMORY, "calendar_svc_struct_new() Failed");

		(*row_event)->event_type = CAL_STRUCT_TYPE_SCHEDULE;
	}
	sch_record = (cal_sch_full_t*)(*row_event)->user_data;
	retv_if(NULL == sch_record, CAL_ERR_FAIL);

	cal_db_service_convert_stmt_to_month_field_record(iter->stmt,is_repeat,sch_record,true);

	if(is_repeat)
		cal_db_service_get_recurrency_exception(sch_record->index,&(sch_record->exception_date_list),&error_code);


	return CAL_SUCCESS;
}

static inline void __set_day_flag(struct tm*stm,struct tm*etm,struct tm*estm,struct tm*eetm,int *day_flag)
{

	int start_day = 0;
	int end_day = 0;
	int i = 0;

	//ERR("stm(%d/%d/%d %d)",stm->tm_year+1900,stm->tm_mon+1,stm->tm_mday,stm->tm_hour);
	//ERR("etm(%d/%d/%d %d)",etm->tm_year+1900,etm->tm_mon+1,etm->tm_mday,etm->tm_hour);
	//ERR("estm(%d/%d/%d %d)",estm->tm_year+1900,estm->tm_mon+1,estm->tm_mday,estm->tm_hour);
	//ERR("eetm(%d/%d/%d %d)",eetm->tm_year+1900,eetm->tm_mon+1,eetm->tm_mday,eetm->tm_hour);

	int istm = 0;
	int ietm = 0;
	int iestm = 0;
	int ieetm = 0;

	istm = timegm(stm)/ONE_DAY_SECONDS;
	ietm = timegm(etm)/ONE_DAY_SECONDS;
	iestm = timegm(estm)/ONE_DAY_SECONDS;
	ieetm = timegm(eetm)/ONE_DAY_SECONDS;

	if(ieetm<istm)
		return; //imposible

	if(iestm>ietm)
		return;

	if(iestm<istm)
		start_day = 1;
	else
		start_day = ((iestm-istm)+1);

	if(ieetm<ietm)
		end_day = ((ieetm-istm)+1);
	else
		end_day = ((ietm-istm)+1);

	//ERR("%d,%d,%d,%d (%d,%d)",istm,ietm,iestm,ieetm,start_day,end_day);
	for(i=start_day-1;i<end_day;i++)
	{
		day_flag[i]++;
	}
}


API int calendar_svc_get_month_event_check (int account_id, time_t startdate, time_t enddate, int *day_flag)
{
	cal_iter *it = NULL;
	cal_struct *cs = NULL;
	int rc = 0;

	struct tm lstm;
	struct tm letm;

	struct tm stm ;
	struct tm etm ;

	struct tm* temp_tm = NULL;
	struct tm estm;
	struct tm eetm;

	CALS_FN_CALL;

	//retex_if(0 > account_id,,"[ERROR]calendar_svc_get_event_list_by_period:Invalid parameter(account_id)!\n");
	retvm_if(startdate < 0, CAL_ERR_ARG_INVALID, "startdate(%d) is Invalid", startdate);
	retvm_if(enddate < 0, CAL_ERR_ARG_INVALID, "enddate(%d) is Invalid", enddate);

	localtime_r(&startdate, &stm);
	localtime_r(&enddate, &etm);

	//localtime_r(&startdate,&stm);
	//localtime_r(&enddate,&etm);

	if(etm.tm_mon != stm.tm_mon)
	{
		enddate--;
		localtime_r(&enddate,&etm);
		//cals_tmtime_r(&enddate, &etm);
	}


	//no repeat event
	calendar_svc_get_month_event_list(account_id, startdate, enddate,false,&it);

	while (calendar_svc_iter_next(it) == CAL_SUCCESS) {
		rc = calendar_svc_iter_get_month_info(it, false,&cs);
		if (rc != CAL_SUCCESS || cs == NULL)
			break;

		temp_tm = calendar_svc_struct_get_tm(cs, CAL_VALUE_GMT_START_DATE_TIME,CAL_TZ_FLAG_LOCAL);
		memcpy(&estm,temp_tm,sizeof(struct tm));
		temp_tm = calendar_svc_struct_get_tm(cs, CAL_VALUE_GMT_END_DATE_TIME,CAL_TZ_FLAG_LOCAL);
		memcpy(&eetm,temp_tm,sizeof(struct tm));
		__set_day_flag(&stm,&etm,&estm,&eetm,day_flag);

		calendar_svc_struct_free(&cs);
	}

	calendar_svc_iter_remove(&it);
	it = NULL;

	//repeat event
	calendar_svc_get_month_event_list(account_id, startdate, enddate,true,&it);

	while (calendar_svc_iter_next(it) == CAL_SUCCESS) {
		rc = calendar_svc_iter_get_month_info(it,true, &cs);
		if (rc != CAL_SUCCESS || cs == NULL) {
			ERR("calendar_svc_iter_get_month_info return %d", rc);
			break;
		}

		memset(&lstm,0x00,sizeof(struct tm));
		memset(&letm,0x00,sizeof(struct tm));

		while (calendar_svc_util_next_valid_event_tm(cs,
					&stm, &etm, &lstm, &letm) == CAL_SUCCESS) {
			__set_day_flag(&stm,&etm,&lstm,&letm,day_flag);
		}
		calendar_svc_struct_free(&cs);
	}
	calendar_svc_iter_remove(&it);

	return CAL_SUCCESS;
}


API int calendar_svc_iter_get_main_info (cal_iter *iter, cal_struct **row_event)
{
	CALS_FN_CALL;
	int ret;
	calendar_t *cal_record = NULL;
	cal_sch_full_t *sch_record = NULL;
	cal_timezone_t *tz_record = NULL;
	int error_code = 0;

	retv_if(iter == NULL, CAL_ERR_ARG_NULL);
	retv_if(iter->stmt == NULL, CAL_ERR_ARG_INVALID);
	retv_if(NULL == row_event, CAL_ERR_ARG_NULL);

	if(iter->is_patched!=TRUE)
	{
		int ret = calendar_svc_iter_next(iter);
		if(ret == CAL_ERR_FINISH_ITER)
		{
			return CAL_ERR_NO_DATA;
		}
		else if(ret != CAL_SUCCESS)
		{
			return ret;
		}
	}

	switch(iter->i_type)
	{
	case CAL_STRUCT_TYPE_SCHEDULE:
		if(*row_event == NULL)
		{
			*row_event = calendar_svc_struct_new(CAL_STRUCT_SCHEDULE);
			retvm_if(NULL == *row_event, CAL_ERR_OUT_OF_MEMORY, "calendar_svc_struct_new() Failed");

			(*row_event)->event_type = CAL_STRUCT_TYPE_SCHEDULE;
		}
		sch_record = (cal_sch_full_t*)(*row_event)->user_data;
		retv_if(NULL == sch_record, CAL_ERR_ARG_NULL);

		cals_stmt_get_full_schedule(iter->stmt, sch_record, true);

		cal_db_service_get_participant_info_by_index(sch_record->index,&(sch_record->attendee_list),&error_code);
		cal_db_service_get_meeting_category_info_by_index(sch_record->index,&(sch_record->meeting_category),&error_code);
		cal_db_service_get_recurrency_exception(sch_record->index,&(sch_record->exception_date_list),&error_code);
		ret = cals_get_alarm_info(sch_record->index, &(sch_record->alarm_list));
		retvm_if(CAL_SUCCESS != ret, ret, "cals_get_alarm_info() Failed(%d)", ret);

		break;
	case CAL_STRUCT_TYPE_TODO:
		if(*row_event == NULL)
		{
			*row_event = calendar_svc_struct_new(CAL_STRUCT_TODO);
			retvm_if(NULL == *row_event, CAL_ERR_OUT_OF_MEMORY, "calendar_svc_struct_new() Failed");

			(*row_event)->event_type = CAL_STRUCT_TYPE_TODO;
		}
		sch_record = (cal_sch_full_t*)(*row_event)->user_data;
		retv_if(NULL == sch_record, CAL_ERR_ARG_NULL);

		cals_stmt_get_full_schedule(iter->stmt, sch_record, true);

		break;

	case CAL_STRUCT_TYPE_CALENDAR:
		if(*row_event == NULL)
		{
			*row_event = calendar_svc_struct_new(CAL_STRUCT_CALENDAR);
			retvm_if(NULL == *row_event, CAL_ERR_OUT_OF_MEMORY, "calendar_svc_struct_new() Failed");

			(*row_event)->event_type = CAL_STRUCT_TYPE_CALENDAR;
		}
		cal_record = (calendar_t*)(*row_event)->user_data;
		retv_if(NULL == cal_record, CAL_ERR_FAIL);

		cals_stmt_get_calendar(iter->stmt,cal_record);

		break;
	case CAL_STRUCT_TYPE_TIMEZONE:
		if(*row_event == NULL)
		{
			*row_event = calendar_svc_struct_new(CAL_STRUCT_TIMEZONE);
			retvm_if(NULL == *row_event, CAL_ERR_OUT_OF_MEMORY, "calendar_svc_struct_new() Failed");

			(*row_event)->event_type = CAL_STRUCT_TYPE_TIMEZONE;
		}
		tz_record = (cal_timezone_t*)(*row_event)->user_data;
		retv_if(NULL == tz_record, CAL_ERR_FAIL);

		cal_db_service_convert_stmt_to_tz_info(iter->stmt,tz_record);

		break;

	default:
		break;
	}

	return CAL_SUCCESS;
}
