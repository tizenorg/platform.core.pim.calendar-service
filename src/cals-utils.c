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
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>

#include "cals-typedef.h"
#include "cals-utils.h"
#include "cals-db.h"
#include "cals-db-info.h"
#include "cals-internal.h"
#include "cals-sqlite.h"

#define CALS_MALLOC_DEFAULT_NUM 256 //4Kbytes

#ifdef CALS_IPC_SERVER
extern __thread sqlite3 *calendar_db_handle;
#else
extern sqlite3* calendar_db_handle;
#endif

typedef	enum
{
	CAL_SCH_TERM_ONE_DAY,
	CAL_SCH_TERM_OVER_2_DAYS,
	CAL_SCH_TERM_OVER_7_DAYS,
	CAL_SCH_TERM_OVER_1_MONTH,
	CAL_SCH_TERM_OVER_1_YEAR,
	CAL_SCH_TERM_ERROR
} __cal_sch_term_status_t;

static const char *CALS_NOTI_EVENT_CHANGED="/opt/data/calendar-svc/.CALENDAR_SVC_EVENT_CHANGED";
static const char *CALS_NOTI_TODO_CHANGED="/opt/data/calendar-svc/.CALENDAR_SVC_TODO_CHANGED";
static const char *CALS_NOTI_CALENDAR_CHANGED="/opt/data/calendar-svc/.CALENDAR_SVC_CALENDAR_CHANGED";

#ifdef CALS_IPC_SERVER
static __thread int transaction_cnt = 0;
static __thread int transaction_ver = 0;
static __thread bool version_up = false;

static __thread bool event_change=false;
static __thread bool todo_change=false;
static __thread bool calendar_change=false;
#else
static int transaction_cnt = 0;
static int transaction_ver = 0;
static bool version_up = false;

static bool event_change=false;
static bool todo_change=false;
static bool calendar_change=false;
#endif

static inline void _cals_notify_event_change(void)
{
	int fd = open(CALS_NOTI_EVENT_CHANGED, O_TRUNC | O_RDWR);
	if (0 <= fd) {
		close(fd);
		event_change = false;
	}
}

static inline void _cals_notify_todo_change(void)
{
	int fd = open(CALS_NOTI_TODO_CHANGED, O_TRUNC | O_RDWR);
	if (0 <= fd) {
		close(fd);
		todo_change = false;
	}
}

static inline void _cals_notify_calendar_change(void)
{
	int fd = open(CALS_NOTI_CALENDAR_CHANGED, O_TRUNC | O_RDWR);
	if (0 <= fd) {
		close(fd);
		calendar_change = false;
	}
}

const char* cals_noti_get_file_path(int type)
{
	const char *noti;
	switch (type)
	{
	case CALS_NOTI_TYPE_EVENT:
		noti = CALS_NOTI_EVENT_CHANGED;
		break;
	case CALS_NOTI_TYPE_TODO:
		noti = CALS_NOTI_TODO_CHANGED;
		break;
	case CALS_NOTI_TYPE_CALENDAR:
		noti = CALS_NOTI_CALENDAR_CHANGED;
		break;
	default:
		ERR("The type(%d) is not supported", type);
		return NULL;
	}

	return noti;
}

int cals_notify(cals_noti_type type)
{
	if (0 < transaction_cnt) {
		switch (type) {
		case CALS_NOTI_TYPE_EVENT:
			event_change = true;
			break;
		case CALS_NOTI_TYPE_TODO:
			todo_change = true;
			break;
		case CALS_NOTI_TYPE_CALENDAR:
			calendar_change = true;
			break;
		default:
			ERR("The type(%d) is not supported", type);
			return CAL_ERR_ARG_INVALID;
		}
		return CAL_SUCCESS;
	}

	switch(type) {
	case CALS_NOTI_TYPE_EVENT:
		_cals_notify_event_change();
		break;
	case CALS_NOTI_TYPE_TODO:
		_cals_notify_todo_change();
		break;
	case CALS_NOTI_TYPE_CALENDAR:
		_cals_notify_calendar_change();
		break;
	default:
		ERR("The type(%d) is not supported", type);
		return CAL_ERR_ARG_INVALID;
	}

	return CAL_SUCCESS;
}

#define CAL_COMMIT_TRY_MAX 500000
int cals_begin_trans(void)
{
	if(transaction_cnt <= 0)
	{
		int ret, progress;

		progress = 100000;
		ret = cals_query_exec("BEGIN IMMEDIATE TRANSACTION");
		while(CAL_ERR_DB_LOCK == ret && progress < CAL_COMMIT_TRY_MAX) {
			usleep(progress);
			ret = cals_query_exec("BEGIN IMMEDIATE TRANSACTION");
			progress *= 2;
		}
		retvm_if(CAL_SUCCESS != ret, ret, "cals_query_exec() Failed(%d)", ret);

		transaction_cnt = 0;
		const char *query = "SELECT ver FROM "CALS_TABLE_VERSION;
		transaction_ver = cals_query_get_first_int_result(query);
		version_up = false;
	}
	transaction_cnt++;
	CALS_DBG("transaction_cnt : %d", transaction_cnt);

	return CAL_SUCCESS;
}


static inline void _cals_cancel_changes(void)
{
	event_change = false;
	calendar_change = false;
	todo_change = false;
}


int cals_end_trans(bool is_success)
{
	int ret;
	int progress = 0;
	char query[CALS_SQL_MIN_LEN];

	transaction_cnt--;

	if (0 != transaction_cnt) {
		CALS_DBG("transaction_cnt : %d", transaction_cnt);
		return CAL_SUCCESS;
	}

	if (false == is_success) {
		_cals_cancel_changes();
		ret = cals_query_exec("ROLLBACK TRANSACTION");
		return CAL_SUCCESS;
	}

	if (version_up) {
		transaction_ver++;
		snprintf(query, sizeof(query), "UPDATE %s SET ver = %d",
				CALS_TABLE_VERSION, transaction_ver);
		ret = cals_query_exec(query);
		warn_if(CAL_SUCCESS != ret, "cals_query_exec(version up) Failed(%d).", ret);
	}

	progress = 400000;
	ret = cals_query_exec("COMMIT TRANSACTION");
	while (CAL_ERR_DB_LOCK == ret && progress < CAL_COMMIT_TRY_MAX) {
		usleep(progress);
		ret = cals_query_exec("COMMIT TRANSACTION");
		progress *= 2;
	}
	if (CAL_SUCCESS != ret) {
		int tmp_ret;
		ERR("cals_query_exec() Failed(%d)", ret);
		_cals_cancel_changes();
		tmp_ret = cals_query_exec("ROLLBACK TRANSACTION");
		warn_if(CAL_SUCCESS != tmp_ret, "cals_query_exec(ROLLBACK) Failed(%d).", tmp_ret);
		return ret;
	}
	if (event_change) _cals_notify_event_change();
	if (todo_change) _cals_notify_todo_change();
	if (calendar_change) _cals_notify_calendar_change();

	return transaction_ver;
}

API int calendar_svc_begin_trans(void)
{
	CALS_FN_CALL;
	return cals_begin_trans();
}


API int calendar_svc_end_trans(bool is_success)
{
	CALS_FN_CALL;
	return cals_end_trans(is_success);
}

int cals_get_next_ver(void)
{
	const char *query;

	if (0 < transaction_cnt) {
		version_up = true;
		return transaction_ver + 1;
	}

	query = "SELECT ver FROM "CALS_TABLE_VERSION;
	return (1 + cals_query_get_first_int_result(query));
}

bool cal_db_get_text_from_stmt(sqlite3_stmt *stmt,char** p_str_dst,int column)
{
	char *str_temp = NULL;
	int str_len = 0;

	str_temp = (char *)sqlite3_column_text(stmt, column);
	str_len = sqlite3_column_bytes(stmt, column);

	if(0 == str_len)
	{
		*p_str_dst = NULL;
		return true;
	}

	CAL_FREE(*p_str_dst);


	*p_str_dst = (char*)malloc(str_len+1);
	retvm_if(NULL == *p_str_dst, false, "failed to malloc str_dst");

	memcpy(*p_str_dst,str_temp,str_len);
	(*p_str_dst)[str_len] = '\0';

	return true;
}

bool cal_db_get_blob_from_stmt(sqlite3_stmt *stmt,struct tm* p_tm,int column)
{
	struct tm* temp = NULL;

	if(NULL == p_tm)
	{
		return false;
	}

	temp = (struct tm*)sqlite3_column_blob(stmt, column);
	if(NULL == temp)
	{
		return false;
	}

	memcpy(p_tm,temp,sizeof(struct tm));
	return true;
}


bool cal_db_service_convert_stmt_to_tz_info(sqlite3_stmt *stmt,cal_timezone_t * tz_info)
{
	CALS_FN_CALL;

	int count = 0;

	tz_info->index = sqlite3_column_int(stmt, count++);
	tz_info->tz_offset_from_gmt = sqlite3_column_int(stmt, count++);

	cal_db_get_text_from_stmt(stmt,&(tz_info->standard_name),count++);

	tz_info->std_start_month = sqlite3_column_int(stmt, count++);
	tz_info->std_start_position_of_week = sqlite3_column_int(stmt, count++);
	tz_info->std_start_day = sqlite3_column_int(stmt, count++);
	tz_info->std_start_hour = sqlite3_column_int(stmt, count++);
	tz_info->standard_bias = sqlite3_column_int(stmt, count++);

	cal_db_get_text_from_stmt(stmt,&(tz_info->day_light_name),count++);

	tz_info->day_light_start_month = sqlite3_column_int(stmt, count++);
	tz_info->day_light_start_position_of_week = sqlite3_column_int(stmt, count++);
	tz_info->day_light_start_day = sqlite3_column_int(stmt, count++);
	tz_info->day_light_start_hour = sqlite3_column_int(stmt, count++);
	tz_info->day_light_bias = sqlite3_column_int(stmt, count++);

	return true;
}

inline cals_updated* cals_updated_schedule_add_mempool(void)
{
	int i;
	cals_updated *mempool;

	mempool = calloc(1, sizeof(cals_updated));
//	for (i = 0; i < CALS_MALLOC_DEFAULT_NUM-1; i++) {
//		mempool[i].next = &mempool[i+1];
//	}
	return mempool;
}

inline int cals_updated_schedule_free_mempool(cals_updated *mempool)
{
	cals_updated *memseg, *tmp;

	retv_if(NULL == mempool, CAL_ERR_ARG_NULL);

	memseg = mempool;
	while (memseg) {
		tmp = memseg->next;
//		tmp = memseg[CALS_MALLOC_DEFAULT_NUM-1].next;
		free(memseg);
		memseg = tmp;
	}

	return CAL_SUCCESS;
}

char cal_month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};


//long long int _get_utime(int y, int mon, int d, int h, int min, int s)
long long int _date_to_utime(int y, int mon, int d, int h, int min, int s)
{
	int i;
	long long int t;

	t = y * 365;
	t += ((int)y/4 - (int)y/100 + (int)y/400);
	for (i = 0; i < mon-1; i++) {
		t += cal_month[i];
	}
	if (i > 2 && (y % 4 == 0)) {
		t += 1;
		if ((y % 100 == 0) && (y % 400 != 0)) {
			t -= 1;
		}
	}
	t += d;
	t *= (24 * 60 * 60);
	t += (((h * 60) + min ) * 60 + s);
	t -= D19700101;
	return t;
}

long long int _datetime_to_utime(char *datetime)
{
	int y, mon, d, h, min, s;
	char tmp[8] = {0};
	char *p;

	if (datetime == NULL || strlen(datetime) == 0) {
		ERR("Invalid argument");
		return -1;
	}

	p = datetime;
	snprintf(tmp, 5, "%s", p);
	y = atoi(tmp);
	snprintf(tmp, 3, "%s", p + 4);
	mon = atoi(tmp);
	snprintf(tmp, 3, "%s", p + 6);
	d = atoi(tmp);

	if (strlen(datetime) > 14) {
		snprintf(tmp, 3, "%s", p + 9);
		h = atoi(tmp);
		snprintf(tmp, 3, "%s", p + 11);
		min = atoi(tmp);
		snprintf(tmp, 3, "%s", p + 13);
		s = atoi(tmp);

	} else {
		h = min = s = 0;
	}

	return _date_to_utime(y, mon, d, h, min, s);
}

