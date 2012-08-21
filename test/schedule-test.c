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

#define ST_SUM "Weekly Meeting"
#define ST_DESC "Review : Project status"
#define ST_LOC "Meeting Room #1"
#define ST_TZ "Asia/Seoul"
#define ST_CTG "Business"
#define ST_ATT1_NAME "David Lee"
#define ST_ATT1_EMAIL "davidlee@gmail.com"
#define ST_ATT2_NAME "Brian Kim"
#define ST_ATT2_EMAIL "briankim@yahoo.com"

static inline int insert()
{
	TEST_FN_START;

	int ret;

	cal_struct *event = NULL;
	cal_value *attendee1 = NULL, *attendee2 = NULL;
	GList *attendee_list=NULL;

	time_t cur_time = time(NULL)+240;

	event = calendar_svc_struct_new(CAL_STRUCT_SCHEDULE);

	calendar_svc_struct_set_str(event, CAL_VALUE_TXT_SUMMARY, ST_SUM);
	calendar_svc_struct_set_str(event, CAL_VALUE_TXT_DESCRIPTION, ST_DESC);
	calendar_svc_struct_set_str(event, CAL_VALUE_TXT_LOCATION, ST_LOC);
	calendar_svc_struct_set_int(event, CAL_VALUE_INT_REPEAT_TERM, 1);
	calendar_svc_struct_set_int(event, CAL_VALUE_INT_REPEAT_INTERVAL, 3);
	calendar_svc_struct_set_int(event, CAL_VALUE_INT_REPEAT_UNTIL_TYPE, CALS_REPEAT_UNTIL_TYPE_DATETIME);

	calendar_svc_struct_set_time(event, CAL_VALUE_GMT_START_DATE_TIME, CAL_TZ_FLAG_GMT, cur_time);
	calendar_svc_struct_set_time(event, CAL_VALUE_GMT_END_DATE_TIME, CAL_TZ_FLAG_GMT, cur_time+(60*60));
	calendar_svc_struct_set_time(event, CAL_VALUE_GMT_REPEAT_END_DATE, CAL_TZ_FLAG_GMT, cur_time+(60*60*24*7));
	calendar_svc_struct_set_str(event,CAL_VALUE_TXT_CATEGORIES, ST_CTG);
	calendar_svc_struct_set_str(event,CAL_VALUE_TXT_TZ_NAME, ST_TZ);
	calendar_svc_struct_set_double(event,CAL_VALUE_DBL_LATITUDE, 0.1);
	calendar_svc_struct_set_double(event,CAL_VALUE_DBL_LONGITUDE, 0.2);

	calendar_svc_struct_set_int(event, CAL_VALUE_INT_SYNC_STATUS,1);

	attendee1 = calendar_svc_value_new(CAL_VALUE_LST_ATTENDEE_LIST);
	if(attendee1) {
		calendar_svc_value_set_str(attendee1, CAL_VALUE_TXT_ATTENDEE_DETAIL_NAME, ST_ATT1_NAME);
		calendar_svc_value_set_str(attendee1, CAL_VALUE_TXT_ATTENDEE_DETAIL_EMAIL, ST_ATT1_EMAIL);
		calendar_svc_value_set_int(attendee1, CAL_VALUE_INT_ATTENDEE_DETAIL_STATUS, 1);
		calendar_svc_value_set_int(attendee1, CAL_VALUE_INT_ATTENDEE_DETAIL_CT_INDEX, 1);
		attendee_list = g_list_append(attendee_list, attendee1);
	}

	attendee2 = calendar_svc_value_new(CAL_VALUE_LST_ATTENDEE_LIST);
	if(attendee2) {
		calendar_svc_value_set_str(attendee2, CAL_VALUE_TXT_ATTENDEE_DETAIL_NAME, ST_ATT2_NAME);
		calendar_svc_value_set_str(attendee2, CAL_VALUE_TXT_ATTENDEE_DETAIL_EMAIL, ST_ATT2_EMAIL);
		calendar_svc_value_set_int(attendee2, CAL_VALUE_INT_ATTENDEE_DETAIL_STATUS, 0);
		calendar_svc_value_set_int(attendee2, CAL_VALUE_INT_ATTENDEE_DETAIL_CT_INDEX, 2);
		attendee_list = g_list_append(attendee_list, attendee2);
	}

	ret = calendar_svc_struct_store_list(event, CAL_VALUE_LST_ATTENDEE_LIST, attendee_list);

	ret = calendar_svc_insert(event);
	calendar_svc_struct_free(&event);

	return ret;
}

static inline void get(int index)
{
	TEST_FN_START;

	cal_struct *event = NULL;
	int ct_value = 0;
	int repeat_term = 0;
	int interval = 0;
	int until_type = -1;
	time_t r_end_date_time = 0;
	GList *attendee_list=NULL;
	time_t cur_time = time(NULL)+240;

	event = NULL;
	calendar_svc_get(CAL_STRUCT_SCHEDULE, index, NULL, &event);
	repeat_term = calendar_svc_struct_get_int(event, CAL_VALUE_INT_REPEAT_TERM);
	interval = calendar_svc_struct_get_int(event, CAL_VALUE_INT_REPEAT_INTERVAL);
	until_type = calendar_svc_struct_get_int(event, CAL_VALUE_INT_REPEAT_UNTIL_TYPE);
	r_end_date_time = calendar_svc_struct_get_time(event, CAL_VALUE_GMT_REPEAT_END_DATE,CAL_TZ_FLAG_LOCAL);

	char *location = calendar_svc_struct_get_str(event,CAL_VALUE_TXT_LOCATION);
	char *categories = calendar_svc_struct_get_str(event,CAL_VALUE_TXT_CATEGORIES);
	char *tz_name = calendar_svc_struct_get_str(event,CAL_VALUE_TXT_TZ_NAME);

	DBG("%s = %s", ST_TZ, tz_name);
	DBG("%s = %s", ST_LOC, location);
	DBG("%s = %s", ST_CTG, categories);

	attendee_list = NULL;
	calendar_svc_struct_get_list(event,CAL_VALUE_LST_ATTENDEE_LIST,&attendee_list);
	ct_value = calendar_svc_value_get_int(attendee_list->data,CAL_VALUE_INT_ATTENDEE_DETAIL_CT_INDEX);

	calendar_svc_struct_free(&event);
}

int search()
{
	TEST_FN_START;
	int ret;
	cal_struct *cs;
	cal_iter *it;
	char *summary;
	char *location;
	char *desc;
	int id;

	calendar_svc_connect();
	int search_field;

	search_field = CALS_SEARCH_FIELD_NONE;
	search_field |= CALS_SEARCH_FIELD_SUMMARY;
	search_field |= CALS_SEARCH_FIELD_DESCRIPTION;
	search_field |= CALS_SEARCH_FIELD_LOCATION;
	search_field |= CALS_SEARCH_FIELD_ATTENDEE;

	ret = calendar_svc_event_search(search_field, "Brian", &it);
	if (ret < 0) {
		ERR("calendar_svc_event_search failed(%d)", ret);
		return ret;
	}

	while (calendar_svc_iter_next(it) == CAL_SUCCESS) {
		cs = NULL;
		ret = calendar_svc_iter_get_info(it, &cs);
		if (ret < 0) {
			ERR("calendar_svc_iter_get_info failed (%d)\n", ret);
			return ret;
		}

		id = calendar_svc_struct_get_int(cs, CAL_VALUE_INT_INDEX);
		summary = calendar_svc_struct_get_str(cs, CAL_VALUE_TXT_SUMMARY);
		desc = calendar_svc_struct_get_str(cs, CAL_VALUE_TXT_DESCRIPTION);
		location = calendar_svc_struct_get_str(cs, CAL_VALUE_TXT_LOCATION);
		DBG("\t%s = %s", ST_SUM, summary);
		DBG("\t%s = %s", ST_DESC, desc);
		DBG("\t%s = %s", ST_LOC, location);
		calendar_svc_struct_free(&cs);
	}

	calendar_svc_iter_remove(&it);

	calendar_svc_close();

	return 0;
}

int smartsearch()
{
	TEST_FN_START;
	int ret;
	cal_struct *cs;
	cal_iter *it;
	char *summary;
	char *location;
	char *desc;
	int id;

	calendar_svc_connect();

	ret = calendar_svc_smartsearch_excl("weekly", -1, 10, &it);
	if (ret < 0) {
		ERR("calendar_svc_event_search failed(%d)", ret);
		return ret;
	}

	while (calendar_svc_iter_next(it) == CAL_SUCCESS) {
		cs = NULL;
		ret = calendar_svc_iter_get_info(it, &cs);
		if (ret < 0) {
			ERR("calendar_svc_iter_get_info failed (%d)\n", ret);
			return ret;
		}

		id = calendar_svc_struct_get_int(cs, CAL_VALUE_INT_INDEX);
		summary = calendar_svc_struct_get_str(cs, CAL_VALUE_TXT_SUMMARY);
		desc = calendar_svc_struct_get_str(cs, CAL_VALUE_TXT_DESCRIPTION);
		location = calendar_svc_struct_get_str(cs, CAL_VALUE_TXT_LOCATION);
		DBG("ID = %d", id);
		DBG("\t%s = %s", ST_SUM, summary);
		DBG("\t%s = %s", ST_DESC, desc);
		DBG("\t%s = %s", ST_LOC, location);
		calendar_svc_struct_free(&cs);
	}

	calendar_svc_iter_remove(&it);

	calendar_svc_close();

	return 0;
}


int main(int argc, char **argv)
{
	int id;

	calendar_svc_connect();

	id = insert();
	if (id) {
		get(id);
	}
	search();
	smartsearch();

	calendar_svc_close();

	return 0;
}

