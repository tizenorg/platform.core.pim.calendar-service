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

#include <stdlib.h>
#include <sys/time.h> // check time
#include <contacts.h>
#include <unistd.h>	// usleep

#include "calendar.h"
#include "cal_internal.h" // DBG
#include "cal_typedef.h"
#include "cal_db.h"
#include "cal_db_util.h"
#include "cal_time.h"
#include "cal_server_contacts.h"
#include "cal_access_control.h"

#define CAL_SERVER_CONTACTS_SYNC_THREAD_NAME "cal_server_contacts_sync"

static struct timeval stv; // check time
static struct timeval etv; // check time

GThread *__cal_server_contacts_sync_thread = NULL;
GCond __cal_server_contacts_sync_cond;
GMutex __cal_server_contacts_sync_mutex;

static int __cal_server_contacts_get_contacts_db_version(int *version)
{
	int ver = 0;
	int ret;
	const char *query = "SELECT contacts_ver FROM "CAL_TABLE_VERSION;

	DBG("query[%s]", query);
	ret = _cal_db_util_query_get_first_int_result(query, NULL, &ver);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("_cal_db_util_query_get_first_int_result() failed");
		return ret;
	}

	if (version) *version = ver;
	return CALENDAR_ERROR_NONE;
}

static int __cal_server_contacts_set_contacts_db_version(int version)
{
	int ret = CALENDAR_ERROR_NONE;
	cal_db_util_error_e dbret = CAL_DB_OK;
	char query[CAL_DB_SQL_MAX_LEN] = {0};

	ret = _cal_db_util_begin_trans();
	if (CALENDAR_ERROR_NONE != ret)	{
		ERR("_cal_db_util_begin_trans() failed");
		return CALENDAR_ERROR_DB_FAILED;
	}

	snprintf(query, sizeof(query), "UPDATE %s SET contacts_ver = %d",
			CAL_TABLE_VERSION, version);
	dbret = _cal_db_util_query_exec(query);
	if (CAL_DB_OK != dbret) {
		ERR("_cal_db_util_query_exec() failed (%d)", dbret);
		switch (dbret) {
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

static int __cal_server_contacts_set_new_event(int id, char *label, int date, char *type, int account_id, calendar_record_h *out_event)
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
	_cal_time_get_next_date(&st, &et);

	snprintf(buf, sizeof(buf), "%d", st.time.date.mday);

	ret = calendar_record_create(_calendar_event._uri, &event);
	retvm_if(CALENDAR_ERROR_NONE != ret, ret, "calendar_record_create() failed");
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

	if (account_id > 0) {
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

static int __cal_server_contacts_find_delete_event(int id, int **delete_ids, int *delete_count)
{
	int ret;
	int event_id;
	int count;
	calendar_list_h list = NULL;
	calendar_record_h event = NULL;
	calendar_query_h query = NULL;
	calendar_filter_h filter = NULL;

	ret = calendar_query_create(_calendar_event._uri, &query);
	retvm_if(CALENDAR_ERROR_NONE != ret, ret, "calendar_query_create() failed");
	ret = calendar_filter_create(_calendar_event._uri, &filter);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("calendar_filter_create() failed");
		calendar_query_destroy(query);
		return ret;
	}
	ret = calendar_filter_add_int(filter, _calendar_event.person_id,
			CALENDAR_MATCH_EQUAL, id);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("Failed to add _calendar_event.person_id");
		calendar_filter_destroy(filter);
		calendar_query_destroy(query);
		return ret;
	}
	ret = calendar_query_set_filter(query, filter);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("calendar_query_set_filter() failed");
		calendar_filter_destroy(filter);
		calendar_query_destroy(query);
		return ret;
	}
	ret = calendar_db_get_records_with_query(query, 0, 0, &list);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("calendar_db_get_records_with_query() failed");
		calendar_list_destroy(list, true);
		calendar_filter_destroy(filter);
		calendar_query_destroy(query);
		return ret;
	}
	ret = calendar_list_get_count(list, &count);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("calendar_list_get_count() failed");
		calendar_list_destroy(list, true);
		calendar_filter_destroy(filter);
		calendar_query_destroy(query);
		return ret;
	}
	DBG("event count(%d)\n", count);

	calendar_list_first(list);
	*delete_count = 0;
	*delete_ids = (int*)calloc(count, sizeof(int));
	do {
		if (calendar_list_get_current_record_p(list, &event) == CALENDAR_ERROR_NONE) {
			if (event == NULL) {
				DBG("No event\n");
				break;
			}
			calendar_record_get_int(event, _calendar_event.id, &event_id);
			(*delete_ids)[*delete_count] = event_id;
			(*delete_count)++;
			DBG("delete event_id(%d)\n", event_id);
		}
	} while (calendar_list_next(list) != CALENDAR_ERROR_NO_DATA);

	calendar_list_destroy(list, true);
	calendar_filter_destroy(filter);
	calendar_query_destroy(query);

	return CALENDAR_ERROR_NONE;
}

static int __cal_server_contacts_make_insert_event(int id, calendar_list_h *out_list)
{
	int ret;
	int index;
	int type;
	int date;
	int address_book_id = 0;
	int account_id = 0;
	contacts_record_h ctevent = NULL;
	contacts_record_h address_book = NULL;
	contacts_record_h contact = NULL;
	calendar_record_h out_event = NULL;

	contacts_db_get_record(_contacts_contact._uri, id, &contact);

	ret = contacts_record_get_int(contact, _contacts_contact.address_book_id, &address_book_id);
	if (ret != CONTACTS_ERROR_NONE) {
		DBG("get fail");
	}
	else if (address_book_id > 0) { // default phone addressbook is 0
		if (contacts_db_get_record(_contacts_address_book._uri, address_book_id, &address_book) != CONTACTS_ERROR_NONE) {
			DBG("contacts_db_get_record(%d)", address_book_id);
		}
		else {
			contacts_record_get_int(address_book, _contacts_address_book.account_id, &account_id);
			DBG("account_id[%d]",account_id);
			contacts_record_destroy(address_book, true);
		}
	}

	index = 0;
	while (CONTACTS_ERROR_NONE == contacts_record_get_child_record_at_p(contact,
				_contacts_contact.event, index++, &ctevent)) {
		ret = contacts_record_get_int(ctevent, _contacts_event.type, &type);
		if (CONTACTS_ERROR_NONE != ret) {
			ERR("Failed to get _contacts_event.type");
			break;
		}
		ret = contacts_record_get_int(ctevent, _contacts_event.date, &date);
		if (CONTACTS_ERROR_NONE != ret) {
			ERR("Failed to get _contacts_event.date");
			break;
		}

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
			ret = contacts_record_get_str_p(ctevent, _contacts_event.label, &caltype);
			if (CALENDAR_ERROR_NONE != ret) {
				ERR("Failed to get _contacts_event.label");
				break;
			}
			break;

		default:
			DBG("Couldn't find type(%d)", type);
			is_proper_type = false;
			break;
		}

		if (false == is_proper_type)
			continue;

		char *display = NULL;
		ret = contacts_record_get_str_p(contact, _contacts_contact.display_name, &display);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("Failed to get _contacts_contact.display_name");
			break;
		}
		SEC_DBG("id(%d) display[%s] type(%d)", id, display, type);

		__cal_server_contacts_set_new_event(id, display, date, caltype, account_id, &out_event);
		calendar_list_add(*out_list, out_event);
	}

	contacts_record_destroy(contact, true);
	contact = NULL;
	return CALENDAR_ERROR_NONE;
}

static void __contacts_changed_cb(const char* view_uri, void *user_data)
{
	_cal_server_contacts_sync_start();
}

static void __cal_server_contacts_append_delete_ids(int **delete_ids, int *size, int *delete_count, int *del_ids, int dcount)
{
	if (*size < *delete_count + dcount) {
		do {
			(*size) = (*size) * 2;
		} while(*size < *delete_count + dcount);
		*delete_ids = realloc(*delete_ids, *size * sizeof(int));
	}

	int i=0;
	for(;i<dcount;i++) {
		(*delete_ids)[*delete_count] = del_ids[i];
		(*delete_count)++;
	}
}

#define BULK_MAX_COUNT		100
#define SYNC_USLEEP		500

static int __cal_server_contacts_sync()
{
	int ret;
	int db_ver = -1;
	int latest_ver = -1;
	int status;
	int id;
	int count = 0;
	contacts_list_h list = NULL;
	contacts_record_h updated = NULL;

	calendar_list_h insert_list = NULL;

	gettimeofday(&stv, NULL); // check time

	ret = __cal_server_contacts_get_contacts_db_version(&db_ver);
	if (ret != CALENDAR_ERROR_NONE)
		return false;

	DBG("contacts db version(%d)", db_ver);

	ret = contacts_db_get_changes_by_version(_contacts_contact_updated_info._uri,
			-1, db_ver, &list, &latest_ver);
	if (CONTACTS_ERROR_NONE != ret) {
		ERR("contacts_db_get_changes_by_version() failed");
		contacts_list_destroy(list, true);
		return false;
	}

	if (list == NULL) {
		DBG("list is NULL");
		return false;
	}
	DBG("get changes and get the latest contacts version(%d)\n", latest_ver);

	ret = contacts_list_get_count(list, &count);
	DBG("contacts count(%d)", count);
	if (count == 0) {
		if (db_ver != latest_ver) {
			__cal_server_contacts_set_contacts_db_version(latest_ver);
			DBG("set latest ver(%d)", latest_ver);
		}
		return false;
	}

	int size = 100;
	int *delete_ids = calloc(size, sizeof(int));
	int delete_count = 0;

	calendar_list_create(&insert_list);

	while (CONTACTS_ERROR_NONE == ret) {
		int *del_ids = NULL;
		int dcount = 0;
		contacts_list_get_current_record_p(list, &updated);
		contacts_record_get_int(updated, _contacts_contact_updated_info.contact_id, &id);
		contacts_record_get_int(updated, _contacts_contact_updated_info.type, &status);

		switch (status) {
		case CONTACTS_CHANGE_INSERTED:
			__cal_server_contacts_make_insert_event(id, &insert_list);
			break;

		case CONTACTS_CHANGE_UPDATED:
			__cal_server_contacts_find_delete_event(id, &del_ids, &dcount);
			__cal_server_contacts_append_delete_ids(&delete_ids, &size, &delete_count, del_ids, dcount);
			free(del_ids);
			__cal_server_contacts_make_insert_event(id, &insert_list);
			break;

		case CONTACTS_CHANGE_DELETED:
			__cal_server_contacts_find_delete_event(id, &del_ids, &dcount);
			__cal_server_contacts_append_delete_ids(&delete_ids, &size, &delete_count, del_ids, dcount);
			free(del_ids);
			break;

		default:
			ERR("Not valid");
			break;
		}
		ret = contacts_list_next(list);
	}

	// delete events
	int index = 0;
	int remain_count = delete_count;
	while (remain_count > 0) {
		int ids[BULK_MAX_COUNT] = {0};
		int i;
		for (i=0;i<BULK_MAX_COUNT && index<delete_count;i++) {
			ids[i] = delete_ids[index];
			remain_count--;
			index++;
		}
		DBG("delete record : count(%d)", index);
		calendar_db_delete_records(_calendar_event._uri, ids, i);
		usleep(SYNC_USLEEP);
	}
	free(delete_ids);

	// insert events
	int insert_count = 0;
	calendar_list_get_count(insert_list, &insert_count);
	while (insert_count > BULK_MAX_COUNT) {
		calendar_list_h temp = NULL;
		calendar_list_create(&temp);
		int i;
		for (i=0;i<BULK_MAX_COUNT;i++) {
			calendar_record_h temp_record = NULL;
			calendar_list_first(insert_list);
			calendar_list_get_current_record_p(insert_list, &temp_record);
			calendar_list_remove(insert_list, temp_record);
			calendar_list_add(temp, temp_record);
			insert_count--;
		}
		calendar_db_insert_records(temp, NULL, NULL);
		calendar_list_destroy(temp, true);
		calendar_list_get_count(insert_list, &insert_count);
		usleep(SYNC_USLEEP);
	}
	if (insert_count > 0) {
		DBG("insert record : count(%d)", insert_count);
		calendar_db_insert_records(insert_list, NULL, NULL);
		usleep(SYNC_USLEEP);
	}
	calendar_list_destroy(insert_list, true);

	contacts_list_destroy(list, true);
	__cal_server_contacts_set_contacts_db_version(latest_ver);
	DBG("set latest ver(%d)", latest_ver);

	int diff;
	gettimeofday(&etv, NULL);
	diff = ((int)etv.tv_sec *1000 + (int)etv.tv_usec/1000)
		-((int)stv.tv_sec *1000 + (int)stv.tv_usec/1000);
	DBG(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"); // time check
	DBG("diff %ld(%d.%d)",diff, diff/1000, diff%1000); // time check

	return true;
}

int _cal_server_contacts(void)
{
	int ret;

	ret = contacts_db_add_changed_cb(_contacts_event._uri, __contacts_changed_cb, NULL);
	retvm_if(CONTACTS_ERROR_NONE != ret, ret, "contacts_db_add_changed_cb() failed");

	return CALENDAR_ERROR_NONE;
}

void _cal_server_contacts_delete(int account_id)
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
	retm_if(CALENDAR_ERROR_NONE != ret, "calendar_query_create() failed");
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

	if (count > 0) {
		record_id_array = (int *)malloc(count*sizeof(int));
		calendar_list_first(list);
		do {
			if (calendar_list_get_current_record_p(list, &event) == CALENDAR_ERROR_NONE) {
				if (event == NULL) {
					DBG("No event\n");
					break;
				}
				calendar_record_get_int(event, _calendar_event.id, &event_id);
				DBG("delete event_id(%d)\n", event_id);
				record_id_array[i] = event_id;
				i++;
			}
		} while (calendar_list_next(list) != CALENDAR_ERROR_NO_DATA);

		// delete
		ret = calendar_db_delete_records(_calendar_event._uri, record_id_array, i);
		if (ret != CALENDAR_ERROR_NONE)
			DBG("calendar_db_delete_records fail");
		free(record_id_array);
	}
	calendar_list_destroy(list, true);
	calendar_filter_destroy(filter);
	calendar_query_destroy(query);

	return;
}

static gpointer  __cal_server_contacts_sync_main(gpointer user_data)
{
	int ret = CALENDAR_ERROR_NONE;
	CAL_FN_CALL;

	while(1) {
		ret = calendar_connect();
		if (CALENDAR_ERROR_NONE != ret)
			break;
		_cal_access_control_set_client_info("calendar-service", NULL);

		while(1) {
			if (__cal_server_contacts_sync() == false) {
				CAL_DBG("end");
				break;
			}
		}
		_cal_access_control_unset_client_info();

		calendar_disconnect();

		g_mutex_lock(&__cal_server_contacts_sync_mutex);
		CAL_DBG("wait");
		g_cond_wait(&__cal_server_contacts_sync_cond, &__cal_server_contacts_sync_mutex);
		g_mutex_unlock(&__cal_server_contacts_sync_mutex);
	}

	return NULL;
}

void _cal_server_contacts_sync_start(void)
{
	CAL_FN_CALL;

	if (__cal_server_contacts_sync_thread == NULL) {
		g_mutex_init(&__cal_server_contacts_sync_mutex);
		g_cond_init(&__cal_server_contacts_sync_cond);
		__cal_server_contacts_sync_thread = g_thread_new(CAL_SERVER_CONTACTS_SYNC_THREAD_NAME, __cal_server_contacts_sync_main,NULL);
	}

	// don't use mutex.
	g_cond_signal(&__cal_server_contacts_sync_cond);

	return;
}

