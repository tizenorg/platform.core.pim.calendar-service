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

#include <stdlib.h>     //calloc
#include <glib.h>
#include <unistd.h> //sleep

#include "calendar.h"
#include "cal_typedef.h"
#include "cal_internal.h"
#include "cal_server_calendar_delete.h"
#include "cal_db.h"
#include "cal_db_util.h"

#define CAL_SERVER_CALENDAR_DELETE_COUNT 50
#define CAL_SERVER_CALENDAR_DELETE_STEP_TIME 1
#define CAL_SERVER_CALENDAR_DELETE_THREAD_NAME "cal_server_calendar_delete"

typedef enum
{
	STEP_1, // create calendar_id_list
	STEP_2, // delete schedule_table <-- CAL_SERVER_CALENDAR_DELETE_COUNT
	// loop STEP_2
	STEP_3, // delete calendar_table
} __calendar_delete_step_e;

typedef struct {
	GList *calendar_id_list;
	int current_calendar_id;
	__calendar_delete_step_e step;
} __calendar_delete_data_s;

GThread *__cal_server_calendar_delete_thread = NULL;
GCond __cal_server_calendar_delete_cond;
GMutex __cal_server_calendar_delete_mutex;

static bool __cal_server_calendar_delete_step(int ret, __calendar_delete_data_s* data);
static int __cal_server_calendar_delete_step1(__calendar_delete_data_s* data);
static int __cal_server_calendar_delete_step2(__calendar_delete_data_s* data);
static int __cal_server_calendar_delete_step3(__calendar_delete_data_s* data);
static bool  __cal_server_calendar_run(__calendar_delete_data_s* data);
static gpointer  __cal_server_calendar_main(gpointer user_data);

static bool __cal_server_calendar_delete_step(int ret, __calendar_delete_data_s* data)
{
	if (ret != CALENDAR_ERROR_NONE && ret != CALENDAR_ERROR_NO_DATA)
	{
		if(data->calendar_id_list)
		{
			g_list_free(data->calendar_id_list);
		}
		CAL_FREE(data);
		ERR("fail (%d)",ret);

		return false;
	}
	switch (data->step)
	{
	case STEP_1:
		if (ret == CALENDAR_ERROR_NO_DATA)
		{
			if(data->calendar_id_list)
			{
				g_list_free(data->calendar_id_list);
			}
			CAL_FREE(data);
			ERR("step_1 no_data");

			return false;
		}
		data->step = STEP_2;
		break;
	case STEP_2:
		if (ret == CALENDAR_ERROR_NO_DATA)
		{
			data->step = STEP_3;
		}
		break;
	case STEP_3:
		data->step = STEP_1;
		break;
	default:
		data->step = STEP_1;
		break;
	}

	// go next step
	return true;
}

static int __cal_server_calendar_delete_step1(__calendar_delete_data_s* data)
{
	char query[CAL_DB_SQL_MIN_LEN] = {0,};
	sqlite3_stmt *stmt = NULL;
	int count = 0;

	CAL_FN_CALL;

	if (data->calendar_id_list == NULL)
	{
		// get event_list
		snprintf(query, sizeof(query), "SELECT id FROM %s "
				"WHERE deleted = 1",
				CAL_TABLE_CALENDAR);

		stmt = _cal_db_util_query_prepare(query);
		if (NULL == stmt)
		{
			ERR("_cal_db_util_query_prepare() Failed");
			return CALENDAR_ERROR_DB_FAILED;
		}
		while(CAL_DB_ROW == _cal_db_util_stmt_step(stmt))
		{
			int id = 0;
			id = sqlite3_column_int(stmt, 0);
			data->calendar_id_list = g_list_append(data->calendar_id_list, GINT_TO_POINTER(id));
		}
		sqlite3_finalize(stmt);
	}

	count = g_list_length(data->calendar_id_list);
	if ( count <= 0)
	{
		return CALENDAR_ERROR_NO_DATA;
	}

	GList *cursor = g_list_first(data->calendar_id_list);
	if (cursor)
	{
		data->current_calendar_id = GPOINTER_TO_INT(cursor->data);
		data->calendar_id_list = g_list_remove(data->calendar_id_list, GINT_TO_POINTER(data->current_calendar_id));

		return CALENDAR_ERROR_NONE;
	}
	else
	{
		return CALENDAR_ERROR_NO_DATA;
	}
}

static int __cal_server_calendar_delete_step2(__calendar_delete_data_s* data)
{
	int ret = CALENDAR_ERROR_NONE;
	char query[CAL_DB_SQL_MIN_LEN] = {0};
	cal_db_util_error_e dbret = CAL_DB_OK;
	GList *list = NULL;
	sqlite3_stmt *stmt = NULL;
	int count = 0;

	CAL_FN_CALL;

	ret = _cal_db_util_begin_trans();
	if (CALENDAR_ERROR_NONE != ret)
	{
		ERR("_cal_db_util_begin_trans() failed");
		return CALENDAR_ERROR_DB_FAILED;
	}

	// get event_list
	snprintf(query, sizeof(query), "SELECT id FROM %s "
			"WHERE calendar_id = %d LIMIT %d",
			CAL_TABLE_SCHEDULE,
			data->current_calendar_id, CAL_SERVER_CALENDAR_DELETE_COUNT);

	stmt = _cal_db_util_query_prepare(query);
	if (NULL == stmt)
	{
		ERR("_cal_db_util_query_prepare() Failed");
		_cal_db_util_end_trans(false);
		return CALENDAR_ERROR_DB_FAILED;
	}
	while(CAL_DB_ROW == _cal_db_util_stmt_step(stmt))
	{
		int id = 0;
		id = sqlite3_column_int(stmt, 0);
		list = g_list_append(list, GINT_TO_POINTER(id));
	}

	sqlite3_finalize(stmt);
	stmt = NULL;

	count = g_list_length(list);
	if ( count <= 0)
	{
		_cal_db_util_end_trans(false);
		return CALENDAR_ERROR_NO_DATA;
	}

	GList *cursor = g_list_first(list);
	while(cursor)
	{
		int id = GPOINTER_TO_INT(cursor->data);
		/* delete event table */
		snprintf(query, sizeof(query), "DELETE FROM %s "
				"WHERE id = %d",
				CAL_TABLE_SCHEDULE,
				id);

		dbret = _cal_db_util_query_exec(query);
		CAL_DBG("%s",query);
		if (CAL_DB_OK != dbret)
		{
			ERR("_cal_db_util_query_exec() failed (%d)", dbret);
			_cal_db_util_end_trans(false);
			g_list_free(list);
			switch (dbret)
			{
			case CAL_DB_ERROR_NO_SPACE:
				_cal_db_util_end_trans(false);
				return CALENDAR_ERROR_FILE_NO_SPACE;
			default:
				_cal_db_util_end_trans(false);
				return CALENDAR_ERROR_DB_FAILED;
			}
		}
		cursor = g_list_next(cursor);
	}

	g_list_free(list);

	_cal_db_util_end_trans(true);

	return CALENDAR_ERROR_NONE;
}

static int __cal_server_calendar_delete_step3(__calendar_delete_data_s* data)
{
	int ret = CALENDAR_ERROR_NONE;
	char query[CAL_DB_SQL_MIN_LEN] = {0};
	cal_db_util_error_e dbret = CAL_DB_OK;

	ret = _cal_db_util_begin_trans();
	if (CALENDAR_ERROR_NONE != ret)
	{
		ERR("_cal_db_util_begin_trans() failed");
		return CALENDAR_ERROR_DB_FAILED;
	}

	CAL_FN_CALL;

	/* delete event table */
	snprintf(query, sizeof(query), "DELETE FROM %s "
			"WHERE id = %d",
			CAL_TABLE_CALENDAR,
			data->current_calendar_id);

	dbret = _cal_db_util_query_exec(query);
	if (CAL_DB_OK != dbret)
	{
		ERR("_cal_db_util_query_exec() failed (%d)", dbret);
		_cal_db_util_end_trans(false);
		switch (dbret)
		{
		case CAL_DB_ERROR_NO_SPACE:
			_cal_db_util_end_trans(false);
			return CALENDAR_ERROR_FILE_NO_SPACE;
		default:
			_cal_db_util_end_trans(false);
			return CALENDAR_ERROR_DB_FAILED;
		}
	}

	_cal_db_util_end_trans(true);

	return CALENDAR_ERROR_NONE;
}

static bool  __cal_server_calendar_run(__calendar_delete_data_s* data)
{
	int ret = CALENDAR_ERROR_NONE;

	CAL_FN_CALL;

	if(data == NULL)
	{
		ERR("data is NULL");

		return false;
	}

	switch (data->step)
	{
	case STEP_1:
		ret = __cal_server_calendar_delete_step1(data);
		break;
	case STEP_2:
		ret = __cal_server_calendar_delete_step2(data);
		break;
	case STEP_3:
		ret = __cal_server_calendar_delete_step3(data);
		break;
	default:
		ERR("invalid step");
		if(data->calendar_id_list)
		{
			g_list_free(data->calendar_id_list);
		}
		CAL_FREE(data);

		return false;
	}

	return __cal_server_calendar_delete_step(ret, data);

}

static gpointer  __cal_server_calendar_main(gpointer user_data)
{
	__calendar_delete_data_s *callback_data = NULL;
	int ret = CALENDAR_ERROR_NONE;
	CAL_FN_CALL;

	while(1)
	{
		callback_data = calloc(1,sizeof(__calendar_delete_data_s));

		if (callback_data != NULL)
		{
			callback_data->step = STEP_1;

			// delete
			while(1)
			{
				ret = calendar_connect();
				if (CALENDAR_ERROR_NONE != ret)
				{
					break;
				}
				sleep(CAL_SERVER_CALENDAR_DELETE_STEP_TIME); // sleep 1 sec.
				if (__cal_server_calendar_run(callback_data) == false)
				{
					callback_data = NULL;
					CAL_DBG("end");
					break;
				}
			}
			calendar_disconnect();
			CAL_FREE(callback_data);
		}
		else
		{
			ERR("calloc fail");
		}

		g_mutex_lock(&__cal_server_calendar_delete_mutex);
		CAL_DBG("wait");
		g_cond_wait(&__cal_server_calendar_delete_cond, &__cal_server_calendar_delete_mutex);
		g_mutex_unlock(&__cal_server_calendar_delete_mutex);
	}

	return NULL;
}

void _cal_server_calendar_delete_start(void)
{
	CAL_FN_CALL;

	if (__cal_server_calendar_delete_thread == NULL)
	{
		g_mutex_init(&__cal_server_calendar_delete_mutex);
		g_cond_init(&__cal_server_calendar_delete_cond);
		__cal_server_calendar_delete_thread = g_thread_new(CAL_SERVER_CALENDAR_DELETE_THREAD_NAME,__cal_server_calendar_main,NULL);
	}

	// don't use mutex.
	g_cond_signal(&__cal_server_calendar_delete_cond);

	return ;

}
