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
#include <unistd.h>	// usleep

#include "calendar.h"
#include "cal_typedef.h"
#include "cal_internal.h"
#include "cal_db.h"
#include "cal_db_util.h"
#include "cal_time.h"
#include "cal_server_contacts.h"
#include "cal_access_control.h"

#define CAL_SERVER_CONTACTS_SYNC_THREAD_NAME "cal_server_contacts_sync"

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
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "calendar_record_create() failed");
	ret = calendar_record_set_str(event, _calendar_event.summary, label);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("calendar_record_set_str() failed:summary");
		calendar_record_destroy(event, true);
		return ret;
	}

	ret = calendar_record_set_int(event, _calendar_event.calendar_book_id, DEFAULT_BIRTHDAY_CALENDAR_BOOK_ID);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("calendar_record_set_int() failed:calendar_book_id");
		calendar_record_destroy(event, true);
		return ret;
	}
	ret = calendar_record_set_caltime(event, _calendar_event.start_time, st);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("calendar_record_set_caltime() failed:start_time");
		calendar_record_destroy(event, true);
		return ret;
	}
	ret = calendar_record_set_caltime(event, _calendar_event.end_time, et);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("calendar_record_set_caltime() failed:end_time");
		calendar_record_destroy(event, true);
		return ret;
	}
	ret = calendar_record_set_int(event, _calendar_event.person_id, id);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("calendar_record_set_int() failed:person_id");
		calendar_record_destroy(event, true);
		return ret;
	}
	ret = calendar_record_set_str(event, _calendar_event.sync_data1, type);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("calendar_record_set_str() failed:sync_data1");
		calendar_record_destroy(event, true);
		return ret;
	}
	ret = calendar_record_set_int(event, _calendar_event.freq, CALENDAR_RECURRENCE_YEARLY);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("calendar_record_set_int() failed:freq");
		calendar_record_destroy(event, true);
		return ret;
	}
	ret = calendar_record_set_int(event, _calendar_event.interval, 1);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("calendar_record_set_int() failed:interval");
		calendar_record_destroy(event, true);
		return ret;
	}
	ret = calendar_record_set_str(event, _calendar_event.bymonthday, buf);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("calendar_record_set_str() failed:bymonthday");
		calendar_record_destroy(event, true);
		return ret;
	}
	ret = calendar_record_set_int(event, _calendar_event.range_type, CALENDAR_RANGE_NONE);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("calendar_record_set_int() failed:interval");
		calendar_record_destroy(event, true);
		return ret;
	}

	if (0 < account_id) {
		snprintf(buf, sizeof(buf), "%d", account_id);
		ret = calendar_record_set_str(event, _calendar_event.sync_data4, buf);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("calendar_record_set_str() failed:interval");
			calendar_record_destroy(event, true);
			return ret;
		}
	}

	*out_event = event;

	return CALENDAR_ERROR_NONE;
}

static int cal_server_contacts_delete_event(int contact_id)
{
	int ret = 0;;

	char query[CAL_DB_SQL_MAX_LEN] = {0};
	snprintf(query, sizeof(query), "SELECT id FROM %s WHERE contact_id=%d", CAL_TABLE_SCHEDULE, contact_id);
	int event_id = 0;
	ret = cal_db_util_query_get_first_int_result(query, NULL, &event_id);
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_db_util_query_get_first_int_result() Fail(%d)", ret);

	cal_db_event_delete_record(event_id);
	return CALENDAR_ERROR_NONE;
}

static int cal_server_contacts_insert_event(int id)
{
	int ret = 0;
	int account_id = 0;

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
		DBG("account_id[%d]",account_id);
	}

	int index = 0;
	contacts_record_h contact_event = NULL;
	while (CONTACTS_ERROR_NONE == contacts_record_get_child_record_at_p(contact, _contacts_contact.event, index++, &contact_event)) {
		int type = 0;
		ret = contacts_record_get_int(contact_event, _contacts_event.type, &type);
		BREAK_IF(CONTACTS_ERROR_NONE != ret, "Failed to get _contacts_event.type");

		int date = 0;
		ret = contacts_record_get_int(contact_event, _contacts_event.date, &date);
		BREAK_IF(CONTACTS_ERROR_NONE != ret, "Failed to get _contacts_event.date");

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
				ERR("Failed to get _contacts_event.label");
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
		BREAK_IF(CONTACTS_ERROR_NONE != ret, "Failed to get _contacts_contact.display_name");
		SEC_DBG("id(%d) display[%s] type(%d)", id, display, type);

		calendar_record_h out_event = NULL;
		_cal_server_contacts_set_new_event(id, display, date, caltype, account_id, &out_event);
		cal_db_event_insert_record(out_event, -1, NULL);
	}

	contacts_record_destroy(contact, true);
	return ret;
}

static void __contacts_changed_cb(const char* view_uri, void *user_data)
{
	cal_server_contacts_sync_start();
}

#define BULK_MAX_COUNT		100
#define SYNC_USLEEP		500

static int _cal_server_contacts_sync(void)
{
	CAL_START_TIMESTAMP

	int ret;
	int contacts_ver = -1;
	int status;
	contacts_record_h updated = NULL;

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

	ret = cal_db_util_begin_trans();
	RETVM_IF(CALENDAR_ERROR_NONE != ret, ret, "cal_db_util_begin_trans() Fail(%d)", ret);

	snprintf(query, sizeof(query), "UPDATE %s SET contacts_ver=%d", CAL_TABLE_VERSION, latest_ver);
	ret = cal_db_util_query_exec(query);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("cal_db_util_query_exec() Fail(%d)", ret);
		contacts_list_destroy(contacts_list, true);
		cal_db_util_end_trans(false);
		return ret;
	}

	contacts_list_first(contacts_list);
	do {
		int contact_id = 0;
		contacts_list_get_current_record_p(contacts_list, &updated);
		contacts_record_get_int(updated, _contacts_contact_updated_info.contact_id, &contact_id);
		contacts_record_get_int(updated, _contacts_contact_updated_info.type, &status);

		switch (status) {
		case CONTACTS_CHANGE_INSERTED:
			cal_server_contacts_insert_event(contact_id);
			break;
		case CONTACTS_CHANGE_UPDATED:
			cal_server_contacts_delete_event(contact_id);
			cal_server_contacts_insert_event(contact_id);
			break;
		case CONTACTS_CHANGE_DELETED:
			cal_server_contacts_delete_event(contact_id);
			break;
		default:
			ERR("Not valid");
			break;
		}
	} while (CONTACTS_ERROR_NONE == contacts_list_next(contacts_list));

	contacts_list_destroy(contacts_list, true);
	cal_db_util_end_trans(true);

	CAL_PRINT_TIMESTAMP
		return CALENDAR_ERROR_NONE;
}

int cal_server_contacts(void)
{
	int ret;

	ret = contacts_db_add_changed_cb(_contacts_event._uri, __contacts_changed_cb, NULL);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "contacts_db_add_changed_cb() failed");

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
	RETM_IF(CALENDAR_ERROR_NONE != ret, "calendar_query_create() failed");
	ret = calendar_filter_create(_calendar_event._uri, &filter);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("calendar_filter_create() failed");
		calendar_query_destroy(query);
		return ;
	}
	ret = calendar_filter_add_str(filter, _calendar_event.sync_data4,
			CALENDAR_MATCH_EXACTLY, buf);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("Failed to add _calendar_event.person_id");
		calendar_filter_destroy(filter);
		calendar_query_destroy(query);
		return ;
	}
	ret = calendar_filter_add_operator(filter, CALENDAR_FILTER_OPERATOR_AND);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("Failed to add _calendar_event.person_id");
		calendar_filter_destroy(filter);
		calendar_query_destroy(query);
		return ;
	}
	ret = calendar_filter_add_int(filter, _calendar_event.calendar_book_id,
			CALENDAR_MATCH_EQUAL,DEFAULT_BIRTHDAY_CALENDAR_BOOK_ID);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("Failed to add _calendar_event.person_id");
		calendar_filter_destroy(filter);
		calendar_query_destroy(query);
		return ;
	}
	ret = calendar_query_set_filter(query, filter);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("calendar_query_set_filter() failed");
		calendar_filter_destroy(filter);
		calendar_query_destroy(query);
		return ;
	}
	unsigned int projection = _calendar_event.id;
	ret = calendar_query_set_projection(query, &projection ,1);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("calendar_query_set_projection() failed");
		calendar_filter_destroy(filter);
		calendar_query_destroy(query);
		return ;
	}

	ret = calendar_db_get_records_with_query(query, 0, 0, &list);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("calendar_db_get_records_with_query() failed");
		calendar_list_destroy(list, true);
		calendar_filter_destroy(filter);
		calendar_query_destroy(query);
		return ;
	}
	ret = calendar_list_get_count(list, &count);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("calendar_list_get_count() failed");
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
		ret = calendar_db_delete_records(_calendar_event._uri, record_id_array, i);
		if (CALENDAR_ERROR_NONE != ret)
			DBG("calendar_db_delete_records() Fail(%d)", ret);
		free(record_id_array);
	}
	calendar_list_destroy(list, true);
	calendar_filter_destroy(filter);
	calendar_query_destroy(query);
}

static gpointer  _cal_server_contacts_sync_main(gpointer user_data)
{
	int ret = CALENDAR_ERROR_NONE;
	CAL_FN_CALL();

	while(1) {
		ret = calendar_connect();
		if (CALENDAR_ERROR_NONE != ret)
			break;
		cal_access_control_set_client_info(NULL, NULL);

		while(1) {
			if (CALENDAR_ERROR_NONE != _cal_server_contacts_sync()) {
				DBG("end");
				break;
			}
		}
		cal_access_control_unset_client_info();

		calendar_disconnect();

		g_mutex_lock(&_cal_server_contacts_sync_mutex);
		DBG("wait");
		g_cond_wait(&_cal_server_contacts_sync_cond, &_cal_server_contacts_sync_mutex);
		g_mutex_unlock(&_cal_server_contacts_sync_mutex);
	}

	return NULL;
}

void cal_server_contacts_sync_start(void)
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

