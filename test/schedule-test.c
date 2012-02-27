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
#include <stdio.h>
#include <calendar-svc-provider.h>

#include "test-log.h"

static inline int insert_test()
{
	int ret;
	cal_struct *event = NULL;
	cal_value *attendee1 = NULL, *attendee2 = NULL;
	cal_value *alarm_info1 = NULL, *alarm_info2 = NULL;

	GList *attendee_list=NULL;
	GList *alarm_list = NULL;

	time_t cur_time = time(NULL)+240;

	event = calendar_svc_struct_new(CAL_STRUCT_SCHEDULE);

	calendar_svc_struct_set_str(event, CAL_VALUE_TXT_SUMMARY, "weekly meeting");
	calendar_svc_struct_set_str(event, CAL_VALUE_TXT_DESCRIPTION, "review : project status");
	calendar_svc_struct_set_str(event, CAL_VALUE_TXT_LOCATION, "meeting room #1");
	//calendar_svc_struct_set_int(event, CAL_VALUE_INT_SCH_CATEGORY, CAL_SCH_NONE);
	calendar_svc_struct_set_int(event, CAL_VALUE_INT_REPEAT_TERM,1);
	calendar_svc_struct_set_int(event, CAL_VALUE_INT_REPEAT_INTERVAL,3);
	calendar_svc_struct_set_time(event, CAL_VALUE_GMT_START_DATE_TIME,CAL_TZ_FLAG_GMT,cur_time);
	calendar_svc_struct_set_time(event, CAL_VALUE_GMT_END_DATE_TIME,CAL_TZ_FLAG_GMT,cur_time+(60*60));
	calendar_svc_struct_set_time(event, CAL_VALUE_GMT_REPEAT_END_DATE,CAL_TZ_FLAG_GMT,cur_time+(60*60*24*7));

	//location
	calendar_svc_struct_set_str(event,CAL_VALUE_TXT_LOCATION,"location field");
	calendar_svc_struct_set_str(event,CAL_VALUE_TXT_LOCATION_SUMMARY,"location field");
	calendar_svc_struct_set_str(event,CAL_VALUE_TXT_TZ_NAME,"Asia/Seoul");
	calendar_svc_struct_set_double(event,CAL_VALUE_DBL_LATITUDE,0.1);
	calendar_svc_struct_set_double(event,CAL_VALUE_DBL_LONGITUDE,0.2);

	calendar_svc_struct_set_int(event, CAL_VALUE_INT_SYNC_STATUS,1);

	attendee1 = calendar_svc_value_new(CAL_VALUE_LST_ATTENDEE_LIST);
	if(attendee1) {
		calendar_svc_value_set_str(attendee1, CAL_VALUE_TXT_ATTENDEE_DETAIL_NAME, "heungjae jeong");
		calendar_svc_value_set_str(attendee1, CAL_VALUE_TXT_ATTENDEE_DETAIL_EMAIL, "id@domain.com");
		calendar_svc_value_set_int(attendee1, CAL_VALUE_INT_ATTENDEE_DETAIL_STATUS, 1);
		calendar_svc_value_set_int(attendee1, CAL_VALUE_INT_ATTENDEE_DETAIL_CT_INDEX, 1);
		attendee_list = g_list_append(attendee_list, attendee1);
	}

	attendee2 = calendar_svc_value_new(CAL_VALUE_LST_ATTENDEE_LIST);
	if(attendee2) {
		calendar_svc_value_set_str(attendee2, CAL_VALUE_TXT_ATTENDEE_DETAIL_NAME, "boncheol gu");
		calendar_svc_value_set_str(attendee2, CAL_VALUE_TXT_ATTENDEE_DETAIL_EMAIL, "id@domain.com");
		calendar_svc_value_set_int(attendee2, CAL_VALUE_INT_ATTENDEE_DETAIL_STATUS, 0);
		calendar_svc_value_set_int(attendee2, CAL_VALUE_INT_ATTENDEE_DETAIL_CT_INDEX, 2);
		attendee_list = g_list_append(attendee_list, attendee2);
	}

	ret = calendar_svc_struct_store_list(event, CAL_VALUE_LST_ATTENDEE_LIST, attendee_list);


	alarm_info1 = calendar_svc_value_new(CAL_VALUE_LST_ALARM);
	if(alarm_info1) {
		calendar_svc_value_set_int(alarm_info1, CAL_VALUE_INT_ALARMS_TICK, 1);
		calendar_svc_value_set_int(alarm_info1, CAL_VALUE_INT_ALARMS_TICK_UNIT,CAL_SCH_TIME_UNIT_MIN);
		alarm_list = g_list_append(alarm_list, alarm_info1);
	}

	alarm_info2 = calendar_svc_value_new(CAL_VALUE_LST_ALARM);
	if(alarm_info2) {
		calendar_svc_value_set_int(alarm_info2, CAL_VALUE_INT_ALARMS_TICK, 2);
		calendar_svc_value_set_int(alarm_info2, CAL_VALUE_INT_ALARMS_TICK_UNIT, CAL_SCH_TIME_UNIT_MIN);
	}
	alarm_list = g_list_append(alarm_list, alarm_info2);

	ret = calendar_svc_struct_store_list(event, CAL_VALUE_LST_ALARM, alarm_list);

	ret = calendar_svc_insert(event);
	calendar_svc_struct_free(&event);

	if (ret < CAL_SUCCESS) {
		ERR("calendar_svc_insert() Failed(%d)", ret);
		return -1;
	} else {
		DBG("calendar_svc_insert() return %d", ret);
		return ret;
	}
}

static inline void get_event(int index)
{
	cal_struct *event = NULL;
	int ct_value = 0;
	int repeat_term = 0;
	int interval = 0;
	time_t r_end_date_time = 0;
	GList *attendee_list=NULL;
	time_t cur_time = time(NULL)+240;

	event = NULL;
	calendar_svc_get(CAL_STRUCT_SCHEDULE, index, NULL, &event);
	repeat_term = calendar_svc_struct_get_int(event, CAL_VALUE_INT_REPEAT_TERM);
	interval = calendar_svc_struct_get_int(event, CAL_VALUE_INT_REPEAT_INTERVAL);
	r_end_date_time = calendar_svc_struct_get_time(event, CAL_VALUE_GMT_REPEAT_END_DATE,CAL_TZ_FLAG_LOCAL);

	//location
	char *location = calendar_svc_struct_get_str(event,CAL_VALUE_TXT_LOCATION);
	char *location_summary = calendar_svc_struct_get_str(event,CAL_VALUE_TXT_LOCATION_SUMMARY);
	double latitude = calendar_svc_struct_get_double(event,CAL_VALUE_DBL_LATITUDE);
	double longitude = calendar_svc_struct_get_double(event,CAL_VALUE_DBL_LONGITUDE);
	int sync_status =  calendar_svc_struct_get_int(event, CAL_VALUE_INT_SYNC_STATUS);
	char *tz_name = calendar_svc_struct_get_str(event,CAL_VALUE_TXT_TZ_NAME);

	DBG("tz_name:Asia/Seoul = %s",tz_name);
	DBG("location: %s,%s,%s, %lf, %lf, %d",location,location_summary,tz_name,latitude,longitude,sync_status);
	DBG("curtime:%ld, event repeat_term(%d,%d,%ld)",cur_time,repeat_term,interval,r_end_date_time);

	attendee_list = NULL;
	calendar_svc_struct_get_list(event,CAL_VALUE_LST_ATTENDEE_LIST,&attendee_list);
	ct_value = calendar_svc_value_get_int(attendee_list->data,CAL_VALUE_INT_ATTENDEE_DETAIL_CT_INDEX);

	calendar_svc_struct_free(&event);
}

int main(int argc, char **argv)
{
	int ret;

	calendar_svc_connect();

	ret = insert_test();
	if (0 < ret)
		get_event(ret);

	calendar_svc_close();

	return 0;
}

