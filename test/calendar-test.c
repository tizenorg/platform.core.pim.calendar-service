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
#include <calendar-svc-provider.h>

#include "test-log.h"

static inline int insert_calendar1()
{
	int ret;
	cal_struct *calendar = NULL;

	calendar = calendar_svc_struct_new(CAL_STRUCT_CALENDAR);

	calendar_svc_struct_set_int(calendar, CAL_TABLE_INT_ACCOUNT_ID, 1);
	calendar_svc_struct_set_int(calendar, CAL_TABLE_INT_VISIBILITY, 1);
	calendar_svc_struct_set_str(calendar, CAL_TABLE_TXT_NAME, "Test Calendar1");
	calendar_svc_struct_set_str(calendar, CAL_TABLE_TXT_DESCRIPTION, "Event only calendar");
	calendar_svc_struct_set_int(calendar, CAL_TABLE_INT_STORE_TYPE, CALS_CALENDAR_TYPE_EVENT);

	ret = calendar_svc_insert(calendar);
	calendar_svc_struct_free(&calendar);

	if (ret < CAL_SUCCESS) {
		ERR("calendar_svc_insert() Failed(%d)", ret);
		return -1;
	} else {
		DBG("calendar_svc_insert() return %d", ret);
		return ret;
	}
}


static inline int insert_calendar2()
{
	int ret;
	cal_struct *calendar = NULL;

	calendar = calendar_svc_struct_new(CAL_STRUCT_CALENDAR);

	calendar_svc_struct_set_int(calendar, CAL_TABLE_INT_ACCOUNT_ID, 1);
	calendar_svc_struct_set_int(calendar, CAL_TABLE_INT_VISIBILITY, 1);
	calendar_svc_struct_set_str(calendar, CAL_TABLE_TXT_NAME, "Test Calendar2");
	calendar_svc_struct_set_str(calendar, CAL_TABLE_TXT_DESCRIPTION, "Event and Todo calendar");
	calendar_svc_struct_set_int(calendar, CAL_TABLE_INT_STORE_TYPE, CALS_CALENDAR_TYPE_EVENT|CALS_CALENDAR_TYPE_TODO);

	ret = calendar_svc_insert(calendar);
	calendar_svc_struct_free(&calendar);

	if (ret < CAL_SUCCESS) {
		ERR("calendar_svc_insert() Failed(%d)", ret);
		return -1;
	} else {
		DBG("calendar_svc_insert() return %d", ret);
		return ret;
	}
}

static inline void get_calendar(int index)
{
	int ret;
	cal_struct *calendar = NULL;

	ret = calendar_svc_get(CAL_STRUCT_CALENDAR, index, NULL, &calendar);
	if (ret < CAL_SUCCESS) {
		ERR("calendar_svc_get() Failed(%d)", ret);
		return;
	}

	printf("%s(%s)\n\t acc = %d\n\t type = %d\n",
			calendar_svc_struct_get_str(calendar, CAL_TABLE_TXT_NAME),
			calendar_svc_struct_get_str(calendar, CAL_TABLE_TXT_DESCRIPTION),
			calendar_svc_struct_get_int(calendar, CAL_TABLE_INT_ACCOUNT_ID),
			calendar_svc_struct_get_int(calendar, CAL_TABLE_INT_STORE_TYPE));

	calendar_svc_struct_free(&calendar);
}

int main(int argc, char **argv)
{
	int ret;

	calendar_svc_connect();

	ret = insert_calendar1();
	DBG("get_calendar");
	get_calendar(ret);

	ret = insert_calendar2();
	DBG("get_calendar");
	get_calendar(ret);

	calendar_svc_close();

	return 0;
}


