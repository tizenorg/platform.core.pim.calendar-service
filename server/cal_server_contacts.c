/*
 * Calendar Service
 *
 * Copyright (c) 2012 - 2015 Samsung Electronics Co., Ltd. All rights reserved.
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

#include <stdlib.h>
#include <sys/time.h>
#include <contacts.h>
#include <unistd.h>	/* usleep */

#include "calendar.h"
#include "cal_typedef.h"
#include "cal_internal.h"
#include "cal_db.h"
#include "cal_db_util.h"
#include "cal_time.h"
#include "cal_access_control.h"
#include "cal_server_contacts.h"
#include "cal_server_service.h"
#include "cal_server_ondemand.h"

#define CAL_SERVER_CONTACTS_SYNC_THREAD_NAME "cal_server_contacts_sync"
#define BULK_MAX_COUNT 100
#define SYNC_USLEEP 500

GThread *_cal_server_contacts_sync_thread = NULL;
GCond _cal_server_contacts_sync_cond;
GMutex _cal_server_contacts_sync_mutex;

static int _cal_server_contacts_set_new_event(int id, char *label, int date, char *type, int account_id, calendar_record_h *out_event)
{
	int ret;
	char buf[4] = {0};
	calendar_record_h event = NULL;
	calendar_time_s st = {0};
	calendar_time_s et = {0};

	*out_event = NULL;

	DBG("date(%d)", date);
	st.type = CALENDAR_TIME_LOCALTIME;
	st.time.date.year = date / 10000;
	st.time.date.month = (date % 10000) / 100;
	st.time.date.mday = date % 100;
	st.time.date.hour = 0;
	st.time.date.minute = 0;
	st.time.date.second = 0;

	et.type = CALENDAR_TIME_LOCALTIME;
	cal_time_get_next_date(&st, &et);

	snprintf(buf, sizeof(buf), "%d", st.time.date.mday);

	ret = calendar_record_create(_calendar_event._uri, &event);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "calendar_record_create() Fail");
	ret = calendar_record_set_str(event, _calendar_event.summary, label);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("calendar_record_set_str() Fail:summary");
		calendar_record_destroy(event, true);
		return ret;
	}

	ret = calendar_record_set_int(event, _calendar_event.calendar_book_id, DEFAULT_BIRTHDAY_CALENDAR_BOOK_ID);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("calendar_record_set_int() Fail:calendar_book_id");
		calendar_record_destroy(event, true);
		return ret;
	}
	ret = calendar_record_set_caltime(event, _calendar_event.start_time, st);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("calendar_record_set_caltime() Fail:start_time");
		calendar_record_destroy(event, true);
		return ret;
	}
	ret = calendar_record_set_caltime(event, _calendar_event.end_time, et);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("calendar_record_set_caltime() Fail:end_time");
		calendar_record_destroy(event, true);
		return ret;
	}
	ret = calendar_record_set_int(event, _calendar_event.person_id, id);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("calendar_record_set_int() Fail:person_id");
		calendar_record_destroy(event, true);
		return ret;
	}
	ret = calendar_record_set_str(event, _calendar_event.sync_data1, type);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("calendar_record_set_str() Fail:sync_data1");
		calendar_record_destroy(event, true);
		return ret;
	}
	ret = calendar_record_set_int(event, _calendar_event.freq, CALENDAR_RECURRENCE_YEARLY);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("calendar_record_set_int() Fail:freq");
		calendar_record_destroy(event, true);
		return ret;
	}
	ret = calendar_record_set_int(event, _calendar_event.interval, 1);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("calendar_record_set_int() Fail:interval");
		calendar_record_destroy(event, true);
		return ret;
	}
	ret = calendar_record_set_str(event, _calendar_event.bymonthday, buf);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("calendar_record_set_str() Fail:bymonthday");
		calendar_record_destroy(event, true);
		return ret;
	}
	ret = calendar_record_set_int(event, _calendar_event.range_type, CALENDAR_RANGE_NONE);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("calendar_record_set_int() Fail:interval");
		calendar_record_destroy(event, true);
		return ret;
	}

	if (0 < account_id) {
		snprintf(buf, sizeof(buf), "%d", account_id);
		ret = calendar_record_set_str(event, _calendar_event.sync_data4, buf);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("calendar_record_set_str() Fail:interval");
			calendar_record_destroy(event, true);
			return ret;
		}
	}

	*out_event = event;

	return CALENDAR_ERROR_NONE;
}

static int cal_server_contacts_delete_event(int contact_id, int **out_array, int *out_count)
{
	int ret = 0;
	int *array = NULL;
	int max_count = BULK_MAX_COUNT;

	RETV_IF(NULL == out_array, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_count, CALENDAR_ERROR_INVALID_PARAMETER);

	array = calloc(max_count, sizeof(int));
	if (NULL == array) {
		ERR("calloc() Fail");
		return CALENDAR_ERROR_OUT_OF_MEMORY;
	}

	char query[CAL_DB_SQL_MAX_LEN] = {0};
	sqlite3_stmt *stmt = NULL;
	snprintf(query, sizeof(query), "SELECT id FROM %s WHERE contact_id=%d",
			CAL_TABLE_SCHEDULE, contact_id);
	ret = cal_db_util_query_prepare(query, &stmt);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_query_prepare() Fail(%d)", ret);
		SECURE("query[%s]", query);
		free(array);
		return ret;
	}

	int index = 0;
	while (CAL_SQLITE_ROW == cal_db_util_stmt_step(stmt)) {
		int event_id = sqlite3_column_int(stmt, 0);
		if (0 == event_id) {
			ERR("event id is invalid");
			break;
		}

		if (max_count <= index) {
			max_count *= 2;
			array = realloc(array, max_count *sizeof(int));
			if (NULL == array) {
				ERR("realloc() Fail");
				break;
			}
		}
		array[index] = event_id;
		index++;
	}
	sqlite3_finalize(stmt);

	*out_array = array;
	*out_count = index;

	return CALENDAR_ERROR_NONE;
}

static int cal_server_contacts_insert_event(int id, calendar_list_h out_insert)
{
	int ret = 0;
	int account_id = 0;

	RETV_IF(NULL == out_insert, CALENDAR_ERROR_INVALID_PARAMETER);

	contacts_record_h contact = NULL;
	ret = contacts_db_get_record(_contacts_contact._uri, id, &contact);
	if (CONTACTS_ERROR_NONE != ret) {
		ERR("contacts_db_get_record() Fail(%d)", ret);
		return CALENDAR_ERROR_SYSTEM;
	}
	int address_book_id = 0;
	ret = contacts_record_get_int(contact, _contacts_contact.address_book_id, &address_book_id);
	if (CONTACTS_ERROR_NONE != ret) {
		ERR("contacts_record_get_int() Fail(%d)", ret);
		contacts_record_destroy(contact, true);
		return CALENDAR_ERROR_SYSTEM;
	}
	if (0 < address_book_id) { /* default phone addressbook is 0 */
		DBG("address_book_id(%d)", address_book_id);
		contacts_record_h address_book = NULL;
		ret = contacts_db_get_record(_contacts_address_book._uri, address_book_id, &address_book);
		if (CONTACTS_ERROR_NONE != ret) {
			DBG("contacts_db_get_record() Fail(%d)", ret);
			contacts_record_destroy(contact, true);
			return CALENDAR_ERROR_SYSTEM;
		}
		ret = contacts_record_get_int(address_book, _contacts_address_book.account_id, &account_id);
		contacts_record_destroy(address_book, true);
		if (CONTACTS_ERROR_NONE != ret) {
			DBG("contacts_record_get_inti() Fail(%d)", ret);
			contacts_record_destroy(contact, true);
			return CALENDAR_ERROR_SYSTEM;
		}
		DBG("account_id[%d]", account_id);
	}

	int index = 0;
	contacts_record_h contact_event = NULL;
	while (CONTACTS_ERROR_NONE == contacts_record_get_child_record_at_p(contact, _contacts_contact.event, index++, &contact_event)) {
		int type = 0;
		ret = contacts_record_get_int(contact_event, _contacts_event.type, &type);
		BREAK_IF(CONTACTS_ERROR_NONE != ret, "contacts_record_get_int() Fail(%d)", ret);

		int date = 0;
		ret = contacts_record_get_int(contact_event, _contacts_event.date, &date);
		BREAK_IF(CONTACTS_ERROR_NONE != ret, "contacts_record_get_int() Fail(%d)", ret);

		bool is_proper_type = true;
		char *caltype = NULL;
		switch (type) {
		case CONTACTS_EVENT_TYPE_BIRTH:
			caltype = "birthday";
			break;
		case CONTACTS_EVENT_TYPE_ANNIVERSARY:
			caltype = "anniversary";
			break;
		case CONTACTS_EVENT_TYPE_OTHER:
			caltype = "other";
			break;
		case CONTACTS_EVENT_TYPE_CUSTOM:
			ret = contacts_record_get_str_p(contact_event, _contacts_event.label, &caltype);
			if (CONTACTS_ERROR_NONE != ret) {
				ERR("contacts_record_get_str_p() Fail(%d)", ret);
				is_proper_type = false;
				break;
			}
			break;
		default:
			DBG("Invalid type(%d)", type);
			is_proper_type = false;
			break;
		}

		if (false == is_proper_type)
			continue;

		char *display = NULL;
		ret = contacts_record_get_str_p(contact, _contacts_contact.display_name, &display);
		BREAK_IF(CONTACTS_ERROR_NONE != ret, "contacts_record_get_str_p() Fail(%d)", ret);
		SEC_DBG("id(%d) display[%s] type(%d)", id, display, type);

		calendar_record_h out_event = NULL;
		_cal_server_contacts_set_new_event(id, display, date, caltype, account_id, &out_event);
		if (out_event)
			calendar_list_add(out_insert, out_event);
	}

	contacts_record_destroy(contact, true);
	return ret;
}

static void _cal_server_contacts_get_event_list(contacts_list_h contacts_list,
		calendar_list_h out_insert, int **out_delete, int *out_count)
{
	RET_IF(NULL == out_delete);
	RET_IF(NULL == out_count);

	int *array = NULL;
	int count = 0;
	contacts_list_first(contacts_list);
	do {
		contacts_record_h updated = NULL;
		contacts_list_get_current_record_p(contacts_list, &updated);

		int contact_id = 0;
		contacts_record_get_int(updated, _contacts_contact_updated_info.contact_id, &contact_id);

		int status;
		contacts_record_get_int(updated, _contacts_contact_updated_info.type, &status);

		int *delete_array = NULL;
		int delete_count = 0;

		switch (status) {
		case CONTACTS_CHANGE_INSERTED:
			cal_server_contacts_insert_event(contact_id, out_insert);
			break;
		case CONTACTS_CHANGE_UPDATED:
			cal_server_contacts_delete_event(contact_id, &delete_array, &delete_count);
			cal_server_contacts_insert_event(contact_id, out_insert);
			break;
		case CONTACTS_CHANGE_DELETED:
			cal_server_contacts_delete_event(contact_id, &delete_array, &delete_count);
			break;
		default:
			ERR("Invalid");
			break;
		}

		if (0 < delete_count) {
			DBG("delete_count");
			if (NULL == array)
				array = calloc(delete_count, sizeof(int));
			else
				array = realloc(array, (count +delete_count) *sizeof(int));

			if (NULL == array) {
				ERR("calloc() Fail");
				free(delete_array);
				break;
			}
			memcpy(array +count, delete_array, delete_count *sizeof(int));
			count += delete_count;
		}
		free(delete_array);
	} while (CONTACTS_ERROR_NONE == contacts_list_next(contacts_list));
	*out_delete = array;
	*out_count = count;
}

static int _cal_server_contacts_sync(void)
{
	CAL_START_TIMESTAMP

	int ret;

	int contacts_ver = -1;
	char query[CAL_DB_SQL_MAX_LEN] = {0};
	snprintf(query, sizeof(query), "SELECT contacts_ver FROM %s", CAL_TABLE_VERSION);
	ret = cal_db_util_query_get_first_int_result(query, NULL, &contacts_ver);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_query_get_first_int_result() Fail(%d)", ret);
		return ret;
	}
	DBG("contacts_ver(%d)", contacts_ver);

	contacts_list_h contacts_list = NULL;
	int latest_ver = -1;
	ret = contacts_db_get_changes_by_version(_contacts_contact_updated_info._uri,
			-1, contacts_ver, &contacts_list, &latest_ver);
	if (CONTACTS_ERROR_NONE != ret) {
		ERR("contacts_db_get_changes_by_version() Fail(%d)", ret);
		contacts_list_destroy(contacts_list, true);
		return ret;
	}

	if (NULL == contacts_list) {
		DBG("contacts_list is NULL");
		contacts_list_destroy(contacts_list, true);
		return CALENDAR_ERROR_NO_DATA;
	}
	DBG("get changes and get the latest contacts version(%d)\n", latest_ver);

	int count = 0;
	ret = contacts_list_get_count(contacts_list, &count);
	if (count == 0) {
		if (contacts_ver == latest_ver) {
			contacts_list_destroy(contacts_list, true);
			return CALENDAR_ERROR_NO_DATA;
		}
	}
	DBG("contacts count(%d)", count);

	/* make event list */
	calendar_list_h insert_list = NULL;
	calendar_list_create(&insert_list);
	int *delete_array = NULL;
	int delete_count = 0;
	_cal_server_contacts_get_event_list(contacts_list, insert_list, &delete_array, &delete_count);

	ret = cal_db_util_begin_trans();
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_begin_trans() Fail(%d)", ret);
		contacts_list_destroy(contacts_list, true);
		calendar_list_destroy(insert_list, true);
		free(delete_array);
		cal_db_util_end_trans(false);
		return ret;
	}

	ret = cal_db_delete_records(_calendar_event._uri, delete_array, delete_count);
	ret = cal_db_insert_records(insert_list, NULL, NULL);

	snprintf(query, sizeof(query), "UPDATE %s SET contacts_ver=%d", CAL_TABLE_VERSION, latest_ver);
	ret = cal_db_util_query_exec(query);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_query_exec() Fail(%d)", ret);
		contacts_list_destroy(contacts_list, true);
		calendar_list_destroy(insert_list, true);
		free(delete_array);
		cal_db_util_end_trans(false);
		return ret;
	}
	cal_db_util_notify(CAL_NOTI_TYPE_EVENT);
	contacts_list_destroy(contacts_list, true);
	calendar_list_destroy(insert_list, true);
	free(delete_array);
	cal_db_util_end_trans(true);

	CAL_PRINT_TIMESTAMP
	return CALENDAR_ERROR_NONE;
}

void cal_server_contacts_delete(int account_id)
{
	int ret;
	int event_id;
	int count;
	calendar_list_h list = NULL;
	calendar_record_h event = NULL;
	calendar_query_h query = NULL;
	calendar_filter_h filter = NULL;
	char buf[4];
	int *record_id_array = NULL;
	int i = 0;

	snprintf(buf, sizeof(buf), "%d", account_id);

	ret = calendar_query_create(_calendar_event._uri, &query);
	RETM_IF(CALENDAR_ERROR_NONE != ret, "calendar_query_create() Fail");
	ret = calendar_filter_create(_calendar_event._uri, &filter);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("calendar_filter_create() Fail");
		calendar_query_destroy(query);
		return ;
	}
	ret = calendar_filter_add_str(filter, _calendar_event.sync_data4,
			CALENDAR_MATCH_EXACTLY, buf);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("calendar_filter_add_str() Fail(%d)", ret);
		calendar_filter_destroy(filter);
		calendar_query_destroy(query);
		return ;
	}
	ret = calendar_filter_add_operator(filter, CALENDAR_FILTER_OPERATOR_AND);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("calendar_filter_add_operator() Fail(%d)", ret);
		calendar_filter_destroy(filter);
		calendar_query_destroy(query);
		return ;
	}
	ret = calendar_filter_add_int(filter, _calendar_event.calendar_book_id,
			CALENDAR_MATCH_EQUAL, DEFAULT_BIRTHDAY_CALENDAR_BOOK_ID);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("calendar_filter_add_int() Fail(%d)", ret);
		calendar_filter_destroy(filter);
		calendar_query_destroy(query);
		return ;
	}
	ret = calendar_query_set_filter(query, filter);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("calendar_query_set_filter() Fail");
		calendar_filter_destroy(filter);
		calendar_query_destroy(query);
		return ;
	}
	unsigned int projection = _calendar_event.id;
	ret = calendar_query_set_projection(query, &projection , 1);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("calendar_query_set_projection() Fail");
		calendar_filter_destroy(filter);
		calendar_query_destroy(query);
		return ;
	}

	ret = cal_db_get_records_with_query(query, 0, 0, &list);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_get_records_with_query() Fail");
		calendar_list_destroy(list, true);
		calendar_filter_destroy(filter);
		calendar_query_destroy(query);
		return ;
	}
	ret = calendar_list_get_count(list, &count);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("calendar_list_get_count() Fail");
		calendar_list_destroy(list, true);
		calendar_filter_destroy(filter);
		calendar_query_destroy(query);
		return ;
	}
	DBG("event count(%d)\n", count);

	if (0 < count) {
		record_id_array = (int *)calloc(count, sizeof(int));
		if (NULL == record_id_array) {
			ERR("calloc() Fail");
			calendar_list_destroy(list, true);
			calendar_filter_destroy(filter);
			calendar_query_destroy(query);
			return;
		}

		calendar_list_first(list);
		do {
			if (CALENDAR_ERROR_NONE == calendar_list_get_current_record_p(list, &event)) {
				if (NULL == event) {
					DBG("No event\n");
					break;
				}
				calendar_record_get_int(event, _calendar_event.id, &event_id);
				DBG("delete event_id(%d)\n", event_id);
				record_id_array[i] = event_id;
				i++;
			}
		} while (CALENDAR_ERROR_NO_DATA != calendar_list_next(list));

		/* delete */
		ret = cal_db_delete_records(_calendar_event._uri, record_id_array, i);
		if (CALENDAR_ERROR_NONE != ret)
			DBG("cal_db_delete_records() Fail(%d)", ret);
		free(record_id_array);
	}
	calendar_list_destroy(list, true);
	calendar_filter_destroy(filter);
	calendar_query_destroy(query);
}

static gpointer _cal_server_contacts_sync_main(gpointer user_data)
{
	int ret = CALENDAR_ERROR_NONE;

	while (1) {
		/*
		 * while syncing with contacts, calendar-service could be stopped by on-demand.
		 * so, on-demand timeout is stopped.
		 */
		cal_server_ondemand_hold();

		ret = cal_connect();
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("cal_connect() Fail(%d)", ret);
			cal_server_ondemand_start();
			break;
		}

		cal_access_control_set_client_info(NULL, "calendar-service");

		while (1) {
			if (CALENDAR_ERROR_NONE != _cal_server_contacts_sync()) {
				DBG("end");
				break;
			}
		}
		cal_access_control_unset_client_info();

		cal_disconnect();

		g_mutex_lock(&_cal_server_contacts_sync_mutex);
		DBG("wait");
		cal_server_ondemand_release();
		cal_server_ondemand_start();
		g_cond_wait(&_cal_server_contacts_sync_cond, &_cal_server_contacts_sync_mutex);
		g_mutex_unlock(&_cal_server_contacts_sync_mutex);
	}

	return NULL;
}

static void cal_server_contacts_sync_start(void)
{
	CAL_FN_CALL();

	if (NULL == _cal_server_contacts_sync_thread) {
		g_mutex_init(&_cal_server_contacts_sync_mutex);
		g_cond_init(&_cal_server_contacts_sync_cond);
		_cal_server_contacts_sync_thread = g_thread_new(CAL_SERVER_CONTACTS_SYNC_THREAD_NAME,
				_cal_server_contacts_sync_main, NULL);
	}

	/* don't use mutex. */
	g_cond_signal(&_cal_server_contacts_sync_cond);
}

static void _changed_cb(const char* view_uri, void *user_data)
{
	cal_server_contacts_sync_start();
}

int cal_server_contacts_init(void)
{
	int ret = 0;

	ret = contacts_connect();
	if (CONTACTS_ERROR_NONE != ret) {
		ERR("contacts_connect() Fail(%d)", ret);
		return ret;
	}

	ret = contacts_db_add_changed_cb(_contacts_event._uri, _changed_cb, NULL);
	if (CONTACTS_ERROR_NONE != ret)
		WARN("contacts_db_add_changed_cb() Fail(%d)", ret);

	ret = contacts_db_add_changed_cb(_contacts_name._uri, _changed_cb, NULL);
	if (CONTACTS_ERROR_NONE != ret)
		WARN("contacts_db_add_changed_cb() Fail(%d)", ret);

	cal_server_contacts_sync_start();

	return CALENDAR_ERROR_NONE;
}

void cal_server_contacts_deinit(void)
{
	contacts_db_remove_changed_cb(_contacts_event._uri, _changed_cb, NULL);
	contacts_db_remove_changed_cb(_contacts_name._uri, _changed_cb, NULL);

	contacts_disconnect();
}
