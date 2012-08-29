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
#include "cals-alarm.h"
#include "cals-sqlite.h"
#include "cals-calendar.h"
#include "cals-schedule.h"
#include "cals-inotify.h"

#ifdef CALS_IPC_SERVER
extern __thread sqlite3 *calendar_db_handle;
#else
extern sqlite3* calendar_db_handle;
#endif

#ifdef CALS_IPC_SERVER
static __thread int db_ref_cnt = 0;
#else
static int db_ref_cnt = 0;
#endif
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
	{CAL_VALUE_TXT_LOCATION,			VALUE_TYPE_TEXT},
	{CAL_VALUE_TXT_CATEGORIES,			VALUE_TYPE_TEXT},
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
	{CAL_VALUE_INT_EMAIL_ID,			  VALUE_TYPE_INT},
	{CAL_VALUE_INT_AVAILABILITY,			VALUE_TYPE_INT},
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

#ifdef CALS_IPC_SERVER
	DBG("pthread_self=%x, db_ref_cnt=%p", pthread_self(),&db_ref_cnt );
#endif
	g_type_init();	// added for alarmmgr

	if(db_ref_cnt <= 0)
	{
		ret = cals_db_open();
		retvm_if(ret, ret, "cals_db_open() Failed(%d)", ret);
#ifdef CALS_IPC_SERVER
#else
		ret = cals_inotify_init();
		if(CAL_SUCCESS != ret) {
			cals_db_close();
			ERR("cals_inotify_init() Failed(%d)", ret);
			return ret;
		}
#endif
		db_ref_cnt = 0;
	}
	db_ref_cnt++;
#ifdef CALS_IPC_SERVER
    DBG("db_ref_cnt(%d)", db_ref_cnt);
#endif
	return CAL_SUCCESS;
}

API int calendar_svc_close(void)
{
	CALS_FN_CALL;
#ifdef CALS_IPC_SERVER
    DBG("db_ref_cnt(%d)", db_ref_cnt);
    DBG("pthread_self=%x, db_ref_cnt=%p", pthread_self(),&db_ref_cnt );
#endif
	retvm_if(0 == db_ref_cnt, CAL_ERR_ENV_INVALID,
			"Calendar service was not connected");

	if (db_ref_cnt==1) {
		cals_db_close();
#ifdef CALS_IPC_SERVER
		db_ref_cnt = 0;
		return 1;
#else
		cals_inotify_close();
#endif
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
	DBG("insert cal type(%d)", ((cal_sch_full_t*)event->user_data)->cal_type);

	switch(event->event_type) {
	case CAL_STRUCT_TYPE_SCHEDULE:
		sch_temp = (cal_sch_full_t*)event->user_data;
		if (sch_temp->cal_type != CALS_SCH_TYPE_EVENT) {
			ERR("Invalid type check (%d)", sch_temp->cal_type);
			return CAL_ERR_FAIL;
		}
/*
		if (sch_temp->calendar_id == DEFAULT_TODO_CALENDAR_ID) {
			ERR("Error, tried to insert TODO calendar id.");
			return CAL_ERR_FAIL;
		}
*/

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
		if (sch_temp->cal_type != CALS_SCH_TYPE_TODO) {
			ERR("Invalid type check (%d)", sch_temp->cal_type);
			return CAL_ERR_FAIL;
		}
/*
		if (sch_temp->calendar_id == DEFAULT_EVENT_CALENDAR_ID) {
			ERR("Error, tried to insert TODO calendar id.");
			return CAL_ERR_FAIL;
		}
*/
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

		/*
		 * is_deleted = 0 is not included in query.
		 * instead, developer should check after getting data.
		 */
		if (field_list) {
			cals_rearrange_schedule_field(field_list, rearranged, sizeof(rearranged));
			snprintf(sql_value, sizeof(sql_value),
					"SELECT %s FROM %s WHERE id = %d ",
					CALS_TABLE_SCHEDULE, rearranged, index);
		} else {
			snprintf(sql_value, sizeof(sql_value),
					"SELECT * FROM %s WHERE id = %d ",
					CALS_TABLE_SCHEDULE, index);
		}

		DBG("query(%s)", sql_value);
		stmt = cals_query_prepare(sql_value);
		retex_if(NULL == stmt,,"cals_query_prepare() Failed");

		rc = cals_stmt_step(stmt);
		if (rc == CAL_SUCCESS) {
			DBG("stmt done is called. No data(%d)", rc);
			sqlite3_finalize(stmt);
			if (malloc_inside && *record != NULL) {
				calendar_svc_struct_free(record);
			}
			return CAL_ERR_NO_DATA;

		} else if (rc != CAL_TRUE) {
			ERR("Failed to step stmt(%d)", rc);
			sqlite3_finalize(stmt);
			if (malloc_inside && *record != NULL) {
				calendar_svc_struct_free(record);
			}
			return CAL_ERR_FAIL;
		}

		if (field_list)
			cals_stmt_get_filted_schedule(stmt, sch_record, field_list);
		else
			cals_stmt_get_full_schedule(stmt, sch_record, true);

		sqlite3_finalize(stmt);
		stmt = NULL;

		if (sch_record->rrule_id > 0) {

			snprintf(sql_value, sizeof(sql_value),
					"SELECT * FROM %s WHERE event_id = %d ",
					CALS_TABLE_RRULE, index);

			DBG("has rrule_is query(%s)", sql_value);

			stmt = cals_query_prepare(sql_value);
			retex_if(NULL == stmt,,"cals_query_prepare() Failed");

			rc = cals_stmt_step(stmt);
			if (rc != CAL_TRUE) {
				ERR("Failed to step stmt(%d)", rc);
				sqlite3_finalize(stmt);
				if (malloc_inside && *record != NULL) {
					calendar_svc_struct_free(record);
				}
				return CAL_ERR_FAIL;
			}

			cals_stmt_fill_rrule(stmt, sch_record);
			sqlite3_finalize(stmt);
			stmt = NULL;
		}

		sch_record->index = index;

		cal_db_service_get_participant_info_by_index(index,
				&(sch_record->attendee_list),&error_code);
		cals_get_alarm_info(index, &(sch_record->alarm_list));
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
			snprintf(sql_value, sizeof(sql_value), "SELECT rowid %s FROM %s WHERE rowid=%d",
				rearranged, CALS_TABLE_CALENDAR, index);
		}
		else
			snprintf(sql_value, sizeof(sql_value), "SELECT rowid,* FROM %s WHERE rowid=%d",
				CALS_TABLE_CALENDAR,	index);

		stmt = cals_query_prepare(sql_value);
		retex_if(NULL == stmt,,"cals_query_prepare() Failed");

		rc = cals_stmt_step(stmt);
		if (rc != CAL_TRUE) {
			ERR("Failed to step stmt(%d)", rc);
			sqlite3_finalize(stmt);
			if (malloc_inside && *record != NULL) {
				calendar_svc_struct_free(record);
			}
			return CAL_ERR_FAIL;
		}

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
			snprintf(sql_value, sizeof(sql_value), "select rowid,* from timezone_table where rowid=%d;",index);
		}

		stmt = cals_query_prepare(sql_value);
		retex_if(NULL == stmt,,"cals_query_prepare() Failed");

		rc = cals_stmt_step(stmt);
		if (rc != CAL_TRUE) {
			ERR("Failed to step stmt(%d)", rc);
			sqlite3_finalize(stmt);
			if (malloc_inside && *record != NULL) {
				calendar_svc_struct_free(record);
			}
			return CAL_ERR_FAIL;
		}

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
		snprintf(condition_value, sizeof(condition_value), "WHERE type=%d",CALS_SCH_TYPE_EVENT);

		if(account_id == ALL_VISIBILITY_ACCOUNT || calendar_id==ALL_VISIBILITY_ACCOUNT)
		{
			snprintf(sql_value, sizeof(sql_value), "SELECT COUNT(*) "
					"FROM %s A, %s B %s "
					"AND A.calendar_id = B.rowid AND B.visibility = 1 AND A.is_deleted = 0 "
					"ORDER BY A.dtstart_utime",
					CALS_TABLE_SCHEDULE, CALS_TABLE_CALENDAR, condition_value);
		}
		else
		{
			if(account_id !=0)
			{
				snprintf(condition_value, sizeof(condition_value), "%s AND account_id = %d",condition_value,account_id);
			}

			if(calendar_id != 0)
			{
				snprintf(condition_value, sizeof(condition_value), "%s AND calendar_id = %d",condition_value,calendar_id);
			}

			snprintf(sql_value, sizeof(sql_value), "SELECT COUNT(*) FROM %s %s "
					"AND is_deleted = 0 "
					"ORDER BY dtstart_utime",
					CALS_TABLE_SCHEDULE, condition_value);
		}

	}
	else if(0 == strcmp(data_type,CAL_STRUCT_TODO))
	{
		snprintf(condition_value, sizeof(condition_value), "WHERE type=%d",CALS_SCH_TYPE_TODO);

		if(account_id !=0)
		{
			snprintf(condition_value, sizeof(condition_value), "%s AND account_id = %d ",condition_value,account_id);
		}

		if(calendar_id != 0)
		{
			snprintf(condition_value, sizeof(condition_value), "%s AND calendar_id = %d ",condition_value,calendar_id);
		}

		snprintf(sql_value, sizeof(sql_value), "SELECT COUNT(*) FROM %s %s "
				"AND is_deleted = 0 ",
				CALS_TABLE_SCHEDULE, condition_value);

	}
	else if(0 == strcmp(data_type,CAL_STRUCT_CALENDAR))
	{
		condition_value[0] = '\0';

		if(account_id != 0)
		{
			snprintf(condition_value, sizeof(condition_value), "WHERE account_id = %d ", account_id);
		}

		snprintf(sql_value, sizeof(sql_value), "SELECT COUNT(*) FROM %s %s;", CALS_TABLE_CALENDAR, condition_value);

	}
	else //not support yet
	{
		ERR("Invalid type(%s)", data_type);
		return CAL_ERR_ARG_INVALID;
	}

	count = cals_query_get_first_int_result(sql_value);

	return count;
}

/* get entry */
API int calendar_svc_get_all(int account_id, int calendar_id,const char *data_type, cal_iter **iter)
{
	CALS_FN_CALL;
	int type;
	sqlite3_stmt *stmt = NULL;
	char sql_value[CALS_SQL_MIN_LEN];

	retvm_if(NULL == data_type, CAL_ERR_ARG_NULL, "Invalid argument: data type is NULL");
	retvm_if(NULL == iter, CAL_ERR_ARG_NULL, "Invalid argument: iter is not NULL");

	if(0 == strcmp(data_type,CAL_STRUCT_SCHEDULE))
	{
		if (account_id == ALL_VISIBILITY_ACCOUNT || calendar_id==ALL_VISIBILITY_ACCOUNT)
		{
			snprintf(sql_value, sizeof(sql_value), "SELECT A.* "
					"FROM %s A, %s B ON A.calendar_id = B.rowid "
					"WHERE A.type=%d AND B.visibility = 1 AND A.is_deleted = 0 "
					"ORDER BY id",
					CALS_TABLE_SCHEDULE, CALS_TABLE_CALENDAR, CALS_SCH_TYPE_EVENT);
		}
		else
		{
			if (calendar_id > 0)
				snprintf(sql_value, sizeof(sql_value), "SELECT * FROM %s "
					"WHERE type=%d AND calendar_id = %d AND is_deleted = 0 "
					"ORDER BY id",
					CALS_TABLE_SCHEDULE, CALS_SCH_TYPE_EVENT, calendar_id);
			else if (account_id)
				snprintf(sql_value, sizeof(sql_value), "SELECT * FROM %s "
					"WHERE type=%d AND account_id = %d AND is_deleted = 0 "
					"ORDER BY id",
					CALS_TABLE_SCHEDULE, CALS_SCH_TYPE_EVENT, account_id);
			else
				snprintf(sql_value, sizeof(sql_value), "SELECT * FROM %s "
					"WHERE type=%d AND is_deleted = 0 "
					"ORDER BY id",
					CALS_TABLE_SCHEDULE, CALS_SCH_TYPE_EVENT);
		}

		type = CAL_STRUCT_TYPE_SCHEDULE;
	}
	else if(0 == strcmp(data_type,CAL_STRUCT_TODO))
	{
		if (calendar_id > 0)
			snprintf(sql_value, sizeof(sql_value), "SELECT * FROM %s "
					"WHERE type=%d AND calendar_id = %d AND is_deleted = 0 ORDER BY id",
					CALS_TABLE_SCHEDULE, CALS_SCH_TYPE_TODO, calendar_id);
		else if (account_id)
			snprintf(sql_value, sizeof(sql_value), "SELECT * FROM %s "
					"WHERE type=%d AND account_id = %d AND is_deleted = 0  ORDER BY id",
					CALS_TABLE_SCHEDULE, CALS_SCH_TYPE_TODO, account_id);
		else
			snprintf(sql_value, sizeof(sql_value), "SELECT * FROM %s "
					"WHERE type=%d AND is_deleted = 0  ORDER BY id",
					CALS_TABLE_SCHEDULE, CALS_SCH_TYPE_TODO);

		type = CAL_STRUCT_TYPE_TODO;
	}
	else if(0 == strcmp(data_type,CAL_STRUCT_CALENDAR))
	{
		if (account_id)
			snprintf(sql_value, sizeof(sql_value), "SELECT rowid,* FROM %s WHERE account_id = %d", CALS_TABLE_CALENDAR, account_id);
		else
			snprintf(sql_value, sizeof(sql_value), "SELECT rowid,* FROM %s", CALS_TABLE_CALENDAR);

		type = CAL_STRUCT_TYPE_CALENDAR;
	}
	else //not support yet
	{
		ERR("Unknown Type(%s)", data_type);
		return CAL_ERR_ARG_INVALID;
	}

	DBG("query(%s)", sql_value);
	stmt = cals_query_prepare(sql_value);
	retvm_if(NULL == stmt, CAL_ERR_DB_FAILED, "cals_query_prepare() Failed");

	*iter = calloc(1, sizeof(cal_iter));
	retvm_if(NULL == *iter, CAL_ERR_OUT_OF_MEMORY, "calloc() Failed(%d)", errno);

	(*iter)->stmt = stmt;
	(*iter)->i_type = type;

	return CAL_SUCCESS;
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

	if(0 == strcmp(data_type, CAL_STRUCT_SCHEDULE) || 0 == strcmp(data_type, CAL_STRUCT_TODO)) {
		ret = cals_delete_schedule(index);
		if (ret) {
			cals_end_trans(false);
			ERR("cals_delete_schedule() Failed(%d)", ret);
			return ret;
		}
	} else if(0 == strcmp(data_type, CAL_STRUCT_CALENDAR)) {
		if(DEFAULT_EVENT_CALENDAR_ID == index || (DEFAULT_TODO_CALENDAR_ID == index)) {
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

API int calendar_svc_clean_after_sync(int calendar_id)
{
	int ret;
	int id;
	char query[CALS_SQL_MIN_LEN] = {0};
	sqlite3_stmt *stmt = NULL;

	retvm_if(calendar_id < 0, CAL_ERR_ARG_INVALID, "calendar_id(%d) is Invalid", calendar_id);

	/* delete recur table */
	/* get event id which id_delete = 1 */
	snprintf(query, sizeof(query), "SELECT id FROM %s "
			"WHERE is_deleted = 1 AND calendar_id = %d",
			CALS_TABLE_SCHEDULE,
			calendar_id);
	stmt = cals_query_prepare(query);
	retvm_if(stmt == NULL, CAL_ERR_DB_FAILED, "Failed to query prepare");

	/* delete all recur data */
	while (sqlite3_step(stmt) == SQLITE_ROW) {
		id = sqlite3_column_int(stmt, 0);
		if (id < 0) {
			ERR("FAiled to get rruel_id");
			break;
		}

		snprintf(query, sizeof(query), "DELETE FROM %s WHERE rrule_id = %d",
				CALS_TABLE_RRULE, id);
		ret = cals_query_exec(query);
		if (ret != CAL_SUCCESS) {
			ERR("Failed to query exit");
			break;
		}
	}
	sqlite3_finalize(stmt);

	/* delete event table */
	snprintf(query, sizeof(query), "DELETE FROM %s "
			"WHERE is_deleted = 1 AND calendar_id = %d",
			CALS_TABLE_SCHEDULE,
			calendar_id);

	ret = cals_query_exec(query);
	retvm_if(ret < 0, ret, "cals_query_exec() failed (%d)", ret);

	/* delete delete table */
	snprintf(query, sizeof(query), "DELETE FROM %s "
			"WHERE calendar_id = %d",
			CALS_TABLE_DELETED,
			calendar_id);

	ret = cals_query_exec(query);
	retvm_if(ret < 0, ret, "cals_query_exec() failed (%d)", ret);

	return CAL_SUCCESS;
}

API int calendar_svc_delete_all(int account_id, const char *data_type)
{
	int ret = 0;

	retvm_if(account_id < 0, CAL_ERR_ARG_INVALID, "account_id(%d) is Invalid", account_id);

	if(data_type == NULL) //delete all data from db by account id
	{
		ret = __cal_service_delete_all_records(account_id, CALS_SCH_TYPE_NONE);
		retvm_if(CAL_SUCCESS != ret, ret, "__cal_service_delete_all_records() Failed(%d)", ret);
		ret = cals_delete_calendars(account_id);
		retvm_if(CAL_SUCCESS != ret, ret, "cals_delete_calendars() Failed(%d)", ret);
	}
	else if(0 == strcmp(data_type,CAL_STRUCT_SCHEDULE))
	{
		ret = __cal_service_delete_all_records(account_id, CALS_SCH_TYPE_EVENT);
		retvm_if(CAL_SUCCESS != ret, ret, "__cal_service_delete_all_records() Failed(%d)", ret);
	}
	else if(0 == strcmp(data_type,CAL_STRUCT_TODO))
	{
		ret = __cal_service_delete_all_records(account_id, CALS_SCH_TYPE_TODO);
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
			snprintf(sql_value, sizeof(sql_value),
					"SELECT * FROM %s "
					"WHERE %s like upper('%%%s%%') AND is_deleted = 0 "
					"ORDER BY dtstart_utime;",
					CALS_TABLE_SCHEDULE, search_type, (char*)search_value);
		}
		else if(0 != account_id)
		{
			snprintf(sql_value, sizeof(sql_value),
					"SELECT * FROM %s "
					"WHERE %s like upper('%%%s%%') AND account_id = %d AND is_deleted = 0 "
					"ORDER BY dtstart_utime;",
					CALS_TABLE_SCHEDULE, search_type, (char*)search_value, account_id);
		}
		else
		{
			snprintf(sql_value, sizeof(sql_value),
					"SELECT * FROM %s "
					"WHERE %s like upper('%%%s%%') AND is_deleted = 0 "
					"ORDER BY dtstart_utime;",
					CALS_TABLE_SCHEDULE, search_type, (char*)search_value);
		}
		break;
	case VALUE_TYPE_INT:
		if(ALL_VISIBILITY_ACCOUNT == account_id)
		{
			snprintf(sql_value, sizeof(sql_value),
					"SELECT * FROM %s "
					"WHERE %s = %d AND is_deleted = 0 "
					"ORDER BY dtstart_utime;",
					CALS_TABLE_SCHEDULE, search_type,(int)search_value);
		}
		else if(0 != account_id)
		{
			snprintf(sql_value, sizeof(sql_value),
					"SELECT * FROM %s "
					"WHERE %s = %d AND account_id = %d AND is_deleted = 0 "
					"ORDER BY dtstart_utime;",
					CALS_TABLE_SCHEDULE, search_type, (int)search_value, account_id);
		}
		else
		{
			snprintf(sql_value, sizeof(sql_value),
					"SELECT * FROM %s "
					"WHERE %s = %d AND is_deleted = 0 "
					"ORDER BY dtstart_utime;",
					CALS_TABLE_SCHEDULE, search_type, (int)search_value);
		}
		break;
	case VALUE_TYPE_USER:
		if (0 == strcmp(CAL_VALUE_INT_ALARMS_ID, search_type)) {
			ret = cals_alarm_get_event_id((int)search_value);
			snprintf(sql_value, sizeof(sql_value),
					"SELECT * FROM %s "
					"WHERE id = %d AND is_deleted = 0 "
					"ORDER BY dtstart_utime",
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

API int calendar_svc_convert_id_to_uid(const char *data_type,int index,char **uid)
{
	int 	rc = -1;
	char	sql_value[CALS_SQL_MAX_LEN] = {0};
	sqlite3_stmt *stmt = NULL;
	int return_value = CAL_SUCCESS;

	retex_if(uid == NULL,return_value = CAL_ERR_ARG_NULL, "The calendar database hasn't been opened.");

	retex_if(NULL == calendar_db_handle, return_value = CAL_ERR_DB_NOT_OPENED, "The calendar database hasn't been opened.");

	// TODO: make query!!!!
	if((0 == strcmp(data_type,CAL_STRUCT_SCHEDULE)) || (0 == strcmp(data_type,CAL_STRUCT_TODO)))
	{
		snprintf(sql_value, sizeof(sql_value), "select uid from schedule_table where id=%d;",index);
	}
	else if(0 == strcmp(data_type,CAL_STRUCT_CALENDAR))
	{
		snprintf(sql_value, sizeof(sql_value), "select uid from calendar_table where rowid=%d;",index);
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

	// TODO: make query!!!!
	if((0 == strcmp(data_type,CAL_STRUCT_SCHEDULE)) || (0 == strcmp(data_type,CAL_STRUCT_TODO)))
	{
		snprintf(sql_value, sizeof(sql_value), "select id from schedule_table where uid=%s;",uid);
	}
	else if(0 == strcmp(data_type,CAL_STRUCT_CALENDAR))
	{
		snprintf(sql_value, sizeof(sql_value), "select rowid from calendar_table where uid=%s;",uid);
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

static void cals_iter_get_info_change(cals_updated *cursor, cals_updated *result)
{
	result->type = cursor->type;
	result->id = cursor->id;
	result->ver = cursor->ver;
	result->calendar_id = cursor->calendar_id;
	return;
}


API int calendar_svc_iter_get_info(cal_iter *iter, cal_struct **row_event)
{
	int cnt;
	int rc = 0;
	char *s_datetime;
	char *e_datetime;
	char buf[8] = {0};
	char sql_value[CALS_SQL_MIN_LEN] = {0};
	cal_sch_full_t *sch_record = NULL;
	calendar_t *cal_record = NULL;
	cal_timezone_t *tz_record = NULL;
	cals_updated *cal_updated = NULL;
	sqlite3_stmt *stmt = NULL;

	retv_if(NULL == iter, CAL_ERR_ARG_NULL);
	retv_if(NULL == iter->stmt && NULL == iter->info, CAL_ERR_ARG_INVALID);
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
	case CAL_STRUCT_TYPE_SCHEDULE:
		if (NULL == *row_event)
		{
			*row_event = calendar_svc_struct_new(CAL_STRUCT_SCHEDULE);
			retvm_if(NULL == *row_event, CAL_ERR_FAIL, "calendar_svc_struct_new(SCHEDULE) Failed");
		}
		sch_record = (cal_sch_full_t*)(*row_event)->user_data;
		retvm_if(NULL == sch_record, CAL_ERR_FAIL, "row_event is Invalid");

		cals_stmt_get_full_schedule(iter->stmt,sch_record,true);

		if (sch_record->rrule_id > 0) {
			snprintf(sql_value, sizeof(sql_value),
					"SELECT * FROM %s WHERE event_id = %d ",
					CALS_TABLE_RRULE, sch_record->index);

			stmt = cals_query_prepare(sql_value);
			retvm_if(NULL == stmt, CAL_ERR_FAIL, "cals_query_prepare() Failed");

			rc = cals_stmt_step(stmt);
			retvm_if(CAL_TRUE != rc, CAL_ERR_FAIL, "cals_stmt_step() Failed(%d)", rc);


			cals_stmt_fill_rrule(stmt, sch_record);
			sqlite3_finalize(stmt);
			stmt = NULL;
		}

		cal_db_service_get_participant_info_by_index(sch_record->index,&(sch_record->attendee_list),&error_code);
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

		if (sch_record->rrule_id > 0) {
			snprintf(sql_value, sizeof(sql_value),
					"SELECT * FROM %s WHERE event_id = %d ",
					CALS_TABLE_RRULE, sch_record->index);

			stmt = cals_query_prepare(sql_value);
			retvm_if(NULL == stmt, CAL_ERR_FAIL, "cals_query_prepare() Failed");

			rc = cals_stmt_step(stmt);
			retvm_if(CAL_TRUE != rc, CAL_ERR_FAIL, "cals_stmt_step() Failed(%d)", rc);

			cals_stmt_fill_rrule(stmt, sch_record);
			sqlite3_finalize(stmt);
			stmt = NULL;
		}
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

	case CAL_STRUCT_TYPE_UPDATED_LIST:
		if (NULL == *row_event)
		{
			*row_event = calendar_svc_struct_new(CAL_STRUCT_UPDATED);
			retvm_if (NULL == *row_event, CAL_ERR_FAIL, "calendar_svc_struct_new(CAL_STRUCT_UPDATE) Failed");
		}
		cal_updated = (cals_updated *)(*row_event)->user_data;
		retvm_if(NULL == cal_updated, CAL_ERR_FAIL, "row_event is Invalid");

		cals_iter_get_info_change(iter->info->cursor, cal_updated);
		break;

	case CALS_STRUCT_TYPE_PERIOD_NORMAL_ONOFF:
		if (NULL == *row_event) {
			*row_event = calendar_svc_struct_new(CALS_STRUCT_PERIOD_NORMAL_ONOFF);
			retvm_if(NULL == *row_event, CAL_ERR_FAIL,
					"Failed to new CALS_STRUCT_PERIOD_NORMAL_ONOFF");
		}
		cals_struct_period_normal_onoff *nof;
		nof = (cals_struct_period_normal_onoff *)(*row_event)->user_data;
		retvm_if(NULL == nof, CAL_ERR_FAIL, "user_data is NULL");

		cnt = 0;
		nof->index = sqlite3_column_int(iter->stmt, cnt++);
		nof->dtstart_type = sqlite3_column_int(iter->stmt, cnt++);
		nof->dtstart_utime = sqlite3_column_int64(iter->stmt, cnt++);
		nof->dtend_type = sqlite3_column_int(iter->stmt, cnt++);
		nof->dtend_utime = sqlite3_column_int64(iter->stmt, cnt++);
		break;

	case CALS_STRUCT_TYPE_PERIOD_ALLDAY_ONOFF:
		s_datetime = NULL;
		if (NULL == *row_event) {
			*row_event = calendar_svc_struct_new(CALS_STRUCT_PERIOD_ALLDAY_ONOFF);
			retvm_if(NULL == *row_event, CAL_ERR_FAIL,
					"Failed to new CALS_STRUCT_TYPE_PERIOD_ALLDAY_ONOFF");
		}
		cals_struct_period_allday_onoff *aof;
		aof = (cals_struct_period_allday_onoff *)(*row_event)->user_data;
		retvm_if(NULL == aof, CAL_ERR_FAIL, "user_data is NULL");

		cnt = 0;
		aof->index = sqlite3_column_int(iter->stmt, cnt++);
		aof->dtstart_type = sqlite3_column_int(iter->stmt, cnt++);
		cal_db_get_text_from_stmt(iter->stmt,&(s_datetime),cnt++);
		memset(buf, 0x0, sizeof(buf));
		snprintf(buf, strlen("YYYY") + 1, "%s", &s_datetime[0]);
		aof->dtstart_year = atoi(buf);
		memset(buf, 0x0, sizeof(buf));
		snprintf(buf, strlen("MM") + 1, "%s", &s_datetime[4]);
		aof->dtstart_month = atoi(buf);
		memset(buf, 0x0, sizeof(buf));
		snprintf(buf, strlen("DD") + 1, "%s", &s_datetime[6]);
		aof->dtstart_mday = atoi(buf);

		aof->dtend_type = sqlite3_column_int(iter->stmt, cnt++);
		cal_db_get_text_from_stmt(iter->stmt,&(s_datetime), cnt++);
		memset(buf, 0x0, sizeof(buf));
		snprintf(buf, strlen("YYYY") + 1, "%s", &s_datetime[0]);
		aof->dtend_year = atoi(buf);
		memset(buf, 0x0, sizeof(buf));
		snprintf(buf, strlen("MM") + 1, "%s", &s_datetime[4]);
		aof->dtend_month = atoi(buf);
		memset(buf, 0x0, sizeof(buf));
		snprintf(buf, strlen("DD") + 1, "%s", &s_datetime[6]);
		aof->dtend_mday = atoi(buf);
		break;

	case CALS_STRUCT_TYPE_PERIOD_NORMAL_BASIC:
		if (NULL == *row_event) {
			*row_event = calendar_svc_struct_new(CALS_STRUCT_PERIOD_NORMAL_BASIC);
			retvm_if(NULL == *row_event, CAL_ERR_FAIL,
					"Failed to new CALS_STRUCT_TYPE_PERIOD_NORMAL_BASIC");
		}
		cals_struct_period_normal_basic *nb;
		nb = (cals_struct_period_normal_basic *)(*row_event)->user_data;
		retvm_if(NULL == nb, CAL_ERR_FAIL, "user_data is NULL");

		cnt = 0;
		nb->index = sqlite3_column_int(iter->stmt, 0);
		nb->dtstart_type = sqlite3_column_int(iter->stmt, 1);
		nb->dtstart_utime = sqlite3_column_int64(iter->stmt, 2);
		nb->dtend_type = sqlite3_column_int(iter->stmt, 3);
		nb->dtend_utime = sqlite3_column_int64(iter->stmt, 4);
		cal_db_get_text_from_stmt(iter->stmt,&(nb->summary), 5);
		cal_db_get_text_from_stmt(iter->stmt,&(nb->location), 6);
		break;

	case CALS_STRUCT_TYPE_PERIOD_ALLDAY_BASIC:
		s_datetime = NULL;
		e_datetime = NULL;
		if (NULL == *row_event) {
			*row_event = calendar_svc_struct_new(CALS_STRUCT_PERIOD_ALLDAY_BASIC);
			retvm_if(NULL == *row_event, CAL_ERR_FAIL,
					"Failed to new CALS_STRUCT_TYPE_PERIOD_ALLDAY_BASIC");
		}
		cals_struct_period_allday_basic *ab;
		ab = (cals_struct_period_allday_basic *)(*row_event)->user_data;
		retvm_if(NULL == ab, CAL_ERR_FAIL, "user_data is NULL");

		cnt = 0;
		ab->index = sqlite3_column_int(iter->stmt, cnt++);
		ab->dtstart_type = sqlite3_column_int(iter->stmt, cnt++);
		cal_db_get_text_from_stmt(iter->stmt,&(s_datetime), cnt++);
		memset(buf, 0x0, sizeof(buf));
		snprintf(buf, strlen("YYYY") + 1, "%s", &s_datetime[0]);
		ab->dtstart_year = atoi(buf);
		memset(buf, 0x0, sizeof(buf));
		snprintf(buf, strlen("MM") + 1, "%s", &s_datetime[4]);
		ab->dtstart_month = atoi(buf);
		memset(buf, 0x0, sizeof(buf));
		snprintf(buf, strlen("DD") + 1, "%s", &s_datetime[6]);
		ab->dtstart_mday = atoi(buf);

		ab->dtend_type = sqlite3_column_int(iter->stmt, cnt++);
		cal_db_get_text_from_stmt(iter->stmt,&(e_datetime),cnt++);
		memset(buf, 0x0, sizeof(buf));
		snprintf(buf, strlen("YYYY") + 1, "%s", &e_datetime[0]);
		ab->dtend_year = atoi(buf);
		memset(buf, 0x0, sizeof(buf));
		snprintf(buf, strlen("MM") + 1, "%s", &e_datetime[4]);
		ab->dtend_month = atoi(buf);
		memset(buf, 0x0, sizeof(buf));
		snprintf(buf, strlen("DD") + 1, "%s", &e_datetime[6]);
		ab->dtend_mday = atoi(buf);
		cal_db_get_text_from_stmt(iter->stmt,&(ab->summary), cnt++);
		cal_db_get_text_from_stmt(iter->stmt,&(ab->location), cnt++);
		break;

	case CALS_STRUCT_TYPE_PERIOD_NORMAL_OSP:
		if (NULL == *row_event) {
			*row_event = calendar_svc_struct_new(CALS_STRUCT_PERIOD_NORMAL_OSP);
			retvm_if(NULL == *row_event, CAL_ERR_FAIL,
					"Failed to new CALS_STRUCT_TYPE_PERIOD_NORMAL_OSP");
		}
		cals_struct_period_normal_osp *nosp;
		nosp = (cals_struct_period_normal_osp *)(*row_event)->user_data;
		retvm_if(NULL == nosp, CAL_ERR_FAIL, "user_data is NULL");

		cnt = 0;
		nosp->index = sqlite3_column_int(iter->stmt, cnt++);
		nosp->calendar_id = sqlite3_column_int(iter->stmt, cnt++);
		nosp->dtstart_type = sqlite3_column_int(iter->stmt, cnt++);
		nosp->dtstart_utime = sqlite3_column_int64(iter->stmt, cnt++);
		nosp->dtend_type = sqlite3_column_int(iter->stmt, cnt++);
		nosp->dtend_utime = sqlite3_column_int64(iter->stmt, cnt++);
		cal_db_get_text_from_stmt(iter->stmt,&(nosp->summary), cnt++);
		cal_db_get_text_from_stmt(iter->stmt,&(nosp->description), cnt++);
		cal_db_get_text_from_stmt(iter->stmt,&(nosp->location), cnt++);
		nosp->busy_status= sqlite3_column_int(iter->stmt, cnt++);
		nosp->meeting_status= sqlite3_column_int(iter->stmt, cnt++);
		nosp->priority= sqlite3_column_int(iter->stmt, cnt++);
		nosp->sensitivity= sqlite3_column_int(iter->stmt, cnt++);
		nosp->rrule_id = sqlite3_column_int(iter->stmt, cnt++);
		break;

	case CALS_STRUCT_TYPE_PERIOD_ALLDAY_OSP:
		s_datetime = NULL;
		e_datetime = NULL;
		if (NULL == *row_event) {
			*row_event = calendar_svc_struct_new(CALS_STRUCT_PERIOD_ALLDAY_OSP);
			retvm_if(NULL == *row_event, CAL_ERR_FAIL,
					"Failed to new CALS_STRUCT_TYPE_PERIOD_ALLDAY_OSP");
		}
		cals_struct_period_allday_osp *aosp;
		aosp = (cals_struct_period_allday_osp *)(*row_event)->user_data;
		retvm_if(NULL == aosp, CAL_ERR_FAIL, "user_data is NULL");

		cnt = 0;
		aosp->index = sqlite3_column_int(iter->stmt, cnt++);
		aosp->calendar_id = sqlite3_column_int(iter->stmt, cnt++);
		aosp->dtstart_type = sqlite3_column_int(iter->stmt, cnt++);
		cal_db_get_text_from_stmt(iter->stmt,&(s_datetime),cnt++);
		memset(buf, 0x0, sizeof(buf));
		snprintf(buf, strlen("YYYY") + 1, "%s", &s_datetime[0]);
		aosp->dtstart_year = atoi(buf);
		memset(buf, 0x0, sizeof(buf));
		snprintf(buf, strlen("MM") + 1, "%s", &s_datetime[4]);
		aosp->dtstart_month = atoi(buf);
		memset(buf, 0x0, sizeof(buf));
		snprintf(buf, strlen("DD") + 1, "%s", &s_datetime[6]);
		aosp->dtstart_mday = atoi(buf);

		aosp->dtend_type = sqlite3_column_int(iter->stmt, cnt++);
		cal_db_get_text_from_stmt(iter->stmt,&(e_datetime),cnt++);
		memset(buf, 0x0, sizeof(buf));
		snprintf(buf, strlen("YYYY") + 1, "%s", &e_datetime[0]);
		aosp->dtend_year = atoi(buf);
		memset(buf, 0x0, sizeof(buf));
		snprintf(buf, strlen("MM") + 1, "%s", &e_datetime[4]);
		aosp->dtend_month = atoi(buf);
		memset(buf, 0x0, sizeof(buf));
		snprintf(buf, strlen("DD") + 1, "%s", &e_datetime[6]);
		aosp->dtend_mday = atoi(buf);
		cal_db_get_text_from_stmt(iter->stmt,&(aosp->summary), cnt++);
		cal_db_get_text_from_stmt(iter->stmt,&(aosp->description), cnt++);
		cal_db_get_text_from_stmt(iter->stmt,&(aosp->location), cnt++);
		aosp->busy_status= sqlite3_column_int(iter->stmt, cnt++);
		aosp->meeting_status= sqlite3_column_int(iter->stmt, cnt++);
		aosp->priority= sqlite3_column_int(iter->stmt, cnt++);
		aosp->sensitivity= sqlite3_column_int(iter->stmt, cnt++);
		aosp->rrule_id = sqlite3_column_int(iter->stmt, cnt++);
		break;

	case CALS_STRUCT_TYPE_PERIOD_NORMAL_LOCATION:
		if (NULL == *row_event) {
			*row_event = calendar_svc_struct_new(CALS_STRUCT_PERIOD_NORMAL_LOCATION);
			retvm_if(NULL == *row_event, CAL_ERR_FAIL,
					"Failed to new CALS_STRUCT_TYPE_PERIOD_NORMAL_OSP");
		}
		cals_struct_period_normal_location *nosl;
		nosl = (cals_struct_period_normal_location *)(*row_event)->user_data;
		retvm_if(NULL == nosl, CAL_ERR_FAIL, "user_data is NULL");

		cnt = 0;
		nosl->index = sqlite3_column_int(iter->stmt, cnt++);
		nosl->calendar_id = sqlite3_column_int(iter->stmt, cnt++);
		nosl->dtstart_type = sqlite3_column_int(iter->stmt, cnt++);
		nosl->dtstart_utime = sqlite3_column_int64(iter->stmt, cnt++);
		nosl->dtend_type = sqlite3_column_int(iter->stmt, cnt++);
		nosl->dtend_utime = sqlite3_column_int64(iter->stmt, cnt++);
		cal_db_get_text_from_stmt(iter->stmt,&(nosl->summary), cnt++);
		cal_db_get_text_from_stmt(iter->stmt,&(nosl->description), cnt++);
		cal_db_get_text_from_stmt(iter->stmt,&(nosl->location), cnt++);
		nosl->busy_status= sqlite3_column_int(iter->stmt, cnt++);
		nosl->meeting_status= sqlite3_column_int(iter->stmt, cnt++);
		nosl->priority= sqlite3_column_int(iter->stmt, cnt++);
		nosl->sensitivity= sqlite3_column_int(iter->stmt, cnt++);
		nosl->rrule_id = sqlite3_column_int(iter->stmt, cnt++);
		nosl->latitude = sqlite3_column_double(iter->stmt, cnt++);
		nosl->longitude = sqlite3_column_double(iter->stmt, cnt++);
		break;

	case CALS_STRUCT_TYPE_PERIOD_ALLDAY_LOCATION:
		s_datetime = NULL;
		e_datetime = NULL;
		if (NULL == *row_event) {
			*row_event = calendar_svc_struct_new(CALS_STRUCT_PERIOD_ALLDAY_LOCATION);
			retvm_if(NULL == *row_event, CAL_ERR_FAIL,
					"Failed to new CALS_STRUCT_TYPE_PERIOD_ALLDAY_OSP");
		}
		cals_struct_period_allday_location *aosl;
		aosl = (cals_struct_period_allday_location *)(*row_event)->user_data;
		retvm_if(NULL == aosl, CAL_ERR_FAIL, "user_data is NULL");

		cnt = 0;
		aosl->index = sqlite3_column_int(iter->stmt, cnt++);
		aosl->calendar_id = sqlite3_column_int(iter->stmt, cnt++);
		aosl->dtstart_type = sqlite3_column_int(iter->stmt, cnt++);
		cal_db_get_text_from_stmt(iter->stmt,&(s_datetime),cnt++);
		memset(buf, 0x0, sizeof(buf));
		snprintf(buf, strlen("YYYY") + 1, "%s", &s_datetime[0]);
		aosl->dtstart_year = atoi(buf);
		memset(buf, 0x0, sizeof(buf));
		snprintf(buf, strlen("MM") + 1, "%s", &s_datetime[4]);
		aosl->dtstart_month = atoi(buf);
		memset(buf, 0x0, sizeof(buf));
		snprintf(buf, strlen("DD") + 1, "%s", &s_datetime[6]);
		aosl->dtstart_mday = atoi(buf);

		aosl->dtend_type = sqlite3_column_int(iter->stmt, cnt++);
		cal_db_get_text_from_stmt(iter->stmt,&(e_datetime),cnt++);
		memset(buf, 0x0, sizeof(buf));
		snprintf(buf, strlen("YYYY") + 1, "%s", &e_datetime[0]);
		aosl->dtend_year = atoi(buf);
		memset(buf, 0x0, sizeof(buf));
		snprintf(buf, strlen("MM") + 1, "%s", &e_datetime[4]);
		aosl->dtend_month = atoi(buf);
		memset(buf, 0x0, sizeof(buf));
		snprintf(buf, strlen("DD") + 1, "%s", &e_datetime[6]);
		aosl->dtend_mday = atoi(buf);
		cal_db_get_text_from_stmt(iter->stmt,&(aosl->summary), cnt++);
		cal_db_get_text_from_stmt(iter->stmt,&(aosl->description), cnt++);
		cal_db_get_text_from_stmt(iter->stmt,&(aosl->location), cnt++);
		aosl->busy_status= sqlite3_column_int(iter->stmt, cnt++);
		aosl->meeting_status= sqlite3_column_int(iter->stmt, cnt++);
		aosl->priority= sqlite3_column_int(iter->stmt, cnt++);
		aosl->sensitivity= sqlite3_column_int(iter->stmt, cnt++);
		aosl->rrule_id = sqlite3_column_int(iter->stmt, cnt++);
		aosl->latitude = sqlite3_column_double(iter->stmt, cnt++);
		aosl->longitude = sqlite3_column_double(iter->stmt, cnt++);
		break;

	case CALS_STRUCT_TYPE_PERIOD_NORMAL_ALARM:
		if (NULL == *row_event) {
			*row_event = calendar_svc_struct_new(CALS_STRUCT_PERIOD_NORMAL_ALARM);
			retvm_if(NULL == *row_event, CAL_ERR_FAIL,
					"Failed to new CALS_STRUCT_TYPE_PERIOD_NORMAL_ALARM");
		}
		cals_struct_period_normal_alarm *nosa;
		nosa = (cals_struct_period_normal_alarm *)(*row_event)->user_data;
		retvm_if(NULL == nosa, CAL_ERR_FAIL, "user_data is NULL");

		cnt = 0;
		nosa->index = sqlite3_column_int(iter->stmt, cnt++);
		nosa->calendar_id = sqlite3_column_int(iter->stmt, cnt++);
		nosa->dtstart_type = sqlite3_column_int(iter->stmt, cnt++);
		nosa->dtstart_utime = sqlite3_column_int64(iter->stmt, cnt++);
		nosa->dtend_type = sqlite3_column_int(iter->stmt, cnt++);
		nosa->dtend_utime = sqlite3_column_int64(iter->stmt, cnt++);
		nosa->alarm_utime = sqlite3_column_int64(iter->stmt, cnt++);
		nosa->alarm_id = sqlite3_column_int(iter->stmt, cnt++);
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

	if (CAL_STRUCT_TYPE_UPDATED_LIST == iter->i_type) {
		retv_if(NULL == iter->info, CAL_ERR_ARG_INVALID);

		if (NULL == iter->info->cursor)
			iter->info->cursor = iter->info->head;
		else
			iter->info->cursor = iter->info->cursor->next;

		if (NULL == iter->info->cursor || 0 == iter->info->cursor->id) {
			iter->info->cursor = NULL;
			cals_updated_schedule_free_mempool(iter->info->head);
			iter->info->head = NULL;
			return CAL_ERR_FINISH_ITER;
		}
	}
	else {
		ret = cals_stmt_step(iter->stmt);
		retvm_if(ret < CAL_SUCCESS, ret, "cals_stmt_step() Failed(%d)", ret);

		if (CAL_SUCCESS == ret)
			return CAL_ERR_FINISH_ITER;
	}

	return CAL_SUCCESS;
}

API int calendar_svc_iter_remove(cal_iter **iter)
{
	CALS_FN_CALL;
	retv_if(NULL == iter, CAL_ERR_ARG_NULL);
	retv_if(NULL == *iter, CAL_ERR_ARG_NULL);

	if (CAL_STRUCT_TYPE_UPDATED_LIST == (*iter)->i_type) {
		retv_if(NULL == (*iter)->info, CAL_ERR_ARG_INVALID);
		if ((*iter)->info->head) {
			cals_updated_schedule_free_mempool((*iter)->info->head);
		}
		free((*iter)->info);

	} else {
		if ((*iter)->stmt)
		{
			sqlite3_finalize((*iter)->stmt);
			(*iter)->stmt = NULL;
		}
	}
	free(*iter);
	*iter = NULL;

	return CAL_SUCCESS;
}


API int calendar_svc_iter_get_main_info (cal_iter *iter, cal_struct **row_event)
{
	CALS_FN_CALL;
	int ret;
	int rc;
	int error_code = 0;
	char sql_value[CALS_SQL_MIN_LEN] = {0};
	calendar_t *cal_record = NULL;
	cal_sch_full_t *sch_record = NULL;
	cal_timezone_t *tz_record = NULL;
	sqlite3_stmt *stmt = NULL;

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

		if (sch_record->rrule_id > 0) {
			snprintf(sql_value, sizeof(sql_value),
					"SELECT * FROM %s WHERE event_id = %d ",
					CALS_TABLE_RRULE, sch_record->index);

			stmt = cals_query_prepare(sql_value);
			retvm_if(NULL == stmt, CAL_ERR_FAIL, "cals_query_prepare() Failed");

			rc = cals_stmt_step(stmt);
			retvm_if(CAL_TRUE != rc, CAL_ERR_FAIL, "cals_stmt_step() Failed(%d)", rc);


			cals_stmt_fill_rrule(stmt, sch_record);
			sqlite3_finalize(stmt);
			stmt = NULL;
		}

		cal_db_service_get_participant_info_by_index(sch_record->index,&(sch_record->attendee_list),&error_code);
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

		if (sch_record->rrule_id > 0) {
			snprintf(sql_value, sizeof(sql_value),
					"SELECT * FROM %s WHERE event_id = %d ",
					CALS_TABLE_RRULE, sch_record->index);

			stmt = cals_query_prepare(sql_value);
			retvm_if(NULL == stmt, CAL_ERR_FAIL, "cals_query_prepare() Failed");

			rc = cals_stmt_step(stmt);
			retvm_if(CAL_TRUE != rc, CAL_ERR_FAIL, "cals_stmt_step() Failed(%d)", rc);

			cals_stmt_fill_rrule(stmt, sch_record);
			sqlite3_finalize(stmt);
			stmt = NULL;
		}

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
