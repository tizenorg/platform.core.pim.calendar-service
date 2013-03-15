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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <calendar2.h>

#include "dft_util.h"

#define MAX_COLS 32768

static void __dft_calendar_help(void)
{
	printf("usage: dft-calendar path(string) count(int)\n"
			"eg. dft-calendar \"./calendar.txt\" 10\n");
}

// get type depending on file name.
static int __dft_get_file_type(char *path)
{
	char *p, *q = NULL;
	char dl = '/';

	q = p = path;
	while (*p)
	{
		if (*p == dl)
		{
			q = p;
		}
		p++;
	}
	if (strstr(q, "event") || strstr(q, "schedule"))
	{
		return CALENDAR_BOOK_TYPE_EVENT;
	}
	else if (strstr(q, "todo") || strstr(q, "task"))
	{
		return CALENDAR_BOOK_TYPE_TODO;
	}
	else
	{
		printf("set default type\n");
		return CALENDAR_BOOK_TYPE_EVENT;
	}
	return -1;
}

static int  __dft_event_set_1(calendar_record_h event, int book_id, char *subject, char *location)
{
	int ret;

	ret = calendar_record_set_int(event, _calendar_event.calendar_book_id, book_id);
	if(CALENDAR_ERROR_NONE != ret)
	{
		printf("failed to set _calendar_event.calendar_book_id\n");
		return -1;
	}
	ret = calendar_record_set_str(event, _calendar_event.summary, subject);
	if(CALENDAR_ERROR_NONE != ret)
	{
		printf("failed to set _calendar_event.summary\n");
		return -1;
	}
	ret = calendar_record_set_str(event, _calendar_event.location, location);
	if(CALENDAR_ERROR_NONE != ret)
	{
		printf("failed to set _calendar_event.location\n");
		return -1;
	}
	return 0;
}

static int __dft_event_set_2(calendar_record_h event, char *allday, char *sdate, char *stime, char *edate, char *etime)
{
	int ret;
	int y, m, d;
	calendar_time_s st = {0}, et = {0};

	if (!strncmp(allday, "Off", sizeof("Off")))
	{
		st.type = CALENDAR_TIME_UTIME;
		st.time.utime = _convert_to_utime(sdate, stime);
		ret = calendar_record_set_str(event, _calendar_event.start_tzid, "Asia/Seoul");
		if(CALENDAR_ERROR_NONE != ret)
		{
			printf("failed to set _calendar_event.start_tzid\n");
			return -1;
		}
		ret = calendar_record_set_caltime(event, _calendar_event.start_time, st);
		if(CALENDAR_ERROR_NONE != ret)
		{
			printf("failed to set _calendar_event.start_time\n");
			return -1;
		}

		et.type = CALENDAR_TIME_UTIME;
		et.time.utime = _convert_to_utime(edate, etime);
		ret = calendar_record_set_str(event, _calendar_event.end_tzid, "Asia/Seoul");
		if(CALENDAR_ERROR_NONE != ret)
		{
			printf("failed to set _calendar_event.end_tzid\n");
			return -1;
		}
		ret = calendar_record_set_caltime(event, _calendar_event.end_time, et);
		if(CALENDAR_ERROR_NONE != ret)
		{
			printf("failed to set _calendar_event.end_time\n");
			return -1;
		}
	} else {
		_convert_to_datetime(sdate, &y, &m, &d);
		st.type = CALENDAR_TIME_LOCALTIME;
		st.time.date.year = y;
		st.time.date.month = m;
		st.time.date.mday = d;
		ret = calendar_record_set_str(event, _calendar_event.start_tzid, "Asia/Seoul");
		if(CALENDAR_ERROR_NONE != ret)
		{
			printf("failed to set _calendar_event.start_tzid\n");
			return -1;
		}
		ret = calendar_record_set_caltime(event, _calendar_event.start_time, st);
		if(CALENDAR_ERROR_NONE != ret)
		{
			printf("failed to set _calendar_event.start_time\n");
			return -1;
		}

		_convert_to_datetime(edate, &y, &m, &d);
		et.type = CALENDAR_TIME_LOCALTIME;
		et.time.date.year = y;
		et.time.date.month = m;
		et.time.date.mday = d;
		ret = calendar_record_set_str(event, _calendar_event.end_tzid, "Asia/Seoul");
		if(CALENDAR_ERROR_NONE != ret)
		{
			printf("failed to set _calendar_event.end_tzid\n");
			return -1;
		}
		ret = calendar_record_set_caltime(event, _calendar_event.end_time, et);
		if(CALENDAR_ERROR_NONE != ret)
		{
			printf("failed to set _calendar_event.end_time\n");
			return -1;
		}
	}
	return 0;
}

static int __dft_event_set_3(calendar_record_h event, int occurence, int status, int sensitivity)
{
	int ret;

	switch (occurence)
	{
	case CALENDAR_RECURRENCE_NONE:
	case CALENDAR_RECURRENCE_DAILY:
	case CALENDAR_RECURRENCE_WEEKLY:
	case CALENDAR_RECURRENCE_MONTHLY:
	case CALENDAR_RECURRENCE_YEARLY:
	    break;
	default:
		printf("Out of range, so set CALENDAR_RECURRENCE_NONE\n");
		occurence = CALENDAR_RECURRENCE_NONE;
		break;
	}
	ret = calendar_record_set_int(event, _calendar_event.count, occurence);
	if(CALENDAR_ERROR_NONE != ret)
	{
		printf("failed to set _calendar_event.count\n");
		return -1;
	}
	switch (status)
	{
	case CALENDAR_EVENT_BUSY_STATUS_FREE:
	case CALENDAR_EVENT_BUSY_STATUS_BUSY:
	case CALENDAR_EVENT_BUSY_STATUS_UNAVAILABLE:
	case CALENDAR_EVENT_BUSY_STATUS_TENTATIVE:
		break;
	default:
		printf("Out of range, so set CALENDAR_EVENT_BUSY_STATUS_FREE\n");
		status = CALENDAR_EVENT_BUSY_STATUS_FREE;
		break;
	}
	ret = calendar_record_set_int(event, _calendar_event.busy_status, status);
	if(CALENDAR_ERROR_NONE != ret)
	{
		printf("failed to set _calendar_event.busy_status\n");
		return -1;
	}

	switch (sensitivity)
	{
	case CALENDAR_SENSITIVITY_PUBLIC:
	case CALENDAR_SENSITIVITY_PRIVATE:
	case CALENDAR_SENSITIVITY_CONFIDENTIAL:
		break;
	default:
		printf("Out of range, so set CALENDAR_SENSITIVITY_PUBLIC\n");
		sensitivity = CALENDAR_SENSITIVITY_PUBLIC;
		break;
	}
	ret = calendar_record_set_int(event, _calendar_event.sensitivity, sensitivity);
	if(CALENDAR_ERROR_NONE != ret)
	{
		printf("failed to set _calendar_event.sensitivity\n");
		return -1;
	}
	return 0;
}

static int  __dft_todo_set_1(calendar_record_h todo, int book_id, char *subject, char *location)
{
	int ret;

	ret = calendar_record_set_int(todo, _calendar_todo.calendar_book_id, book_id);
	if(CALENDAR_ERROR_NONE != ret)
	{
		printf("failed to set _calendar_todo.calendar_book_id\n");
		return -1;
	}
	ret = calendar_record_set_str(todo, _calendar_todo.summary, subject);
	if(CALENDAR_ERROR_NONE != ret)
	{
		printf("failed to set _calendar_todo.summary\n");
		return -1;
	}
	ret = calendar_record_set_str(todo, _calendar_todo.location, location);
	if(CALENDAR_ERROR_NONE != ret)
	{
		printf("failed to set _calendar_todo.location\n");
		return -1;
	}
	return 0;
}

static int __dft_todo_set_2(calendar_record_h todo, char *allday, char *sdate, char *stime, char *edate, char *etime)
{
	int ret;
	int y, m, d;
	calendar_time_s st = {0}, et = {0};

	if (!strncmp(allday, "Off", sizeof("Off")))
	{
		st.type = CALENDAR_TIME_UTIME;
		st.time.utime = _convert_to_utime(sdate, stime);
		ret = calendar_record_set_str(todo, _calendar_todo.start_tzid, "Asia/Seoul");
		if(CALENDAR_ERROR_NONE != ret)
		{
			printf("failed to set _calendar_todo.start_tzid\n");
			return -1;
		}
		ret = calendar_record_set_caltime(todo, _calendar_todo.start_time, st);
		if(CALENDAR_ERROR_NONE != ret)
		{
			printf("failed to set _calendar_todo.start_time\n");
			return -1;
		}

		et.type = CALENDAR_TIME_UTIME;
		et.time.utime = _convert_to_utime(edate, etime);
		ret = calendar_record_set_str(todo, _calendar_todo.due_tzid, "Asia/Seoul");
		if(CALENDAR_ERROR_NONE != ret)
		{
			printf("failed to set _calendar_todo.due_tzid\n");
			return -1;
		}
		ret = calendar_record_set_caltime(todo, _calendar_todo.due_time, et);
		if(CALENDAR_ERROR_NONE != ret)
		{
			printf("failed to set _calendar_todo.due_time\n");
			return -1;
		}
	} else {
		_convert_to_datetime(sdate, &y, &m, &d);
		st.type = CALENDAR_TIME_LOCALTIME;
		st.time.date.year = y;
		st.time.date.month = m;
		st.time.date.mday = d;
		ret = calendar_record_set_str(todo, _calendar_todo.start_tzid, "Asia/Seoul");
		if(CALENDAR_ERROR_NONE != ret)
		{
			printf("failed to set _calendar_todo.start_tzid\n");
			return -1;
		}
		ret = calendar_record_set_caltime(todo, _calendar_todo.start_time, st);
		if(CALENDAR_ERROR_NONE != ret)
		{
			printf("failed to set _calendar_todo.start_time\n");
			return -1;
		}

		_convert_to_datetime(edate, &y, &m, &d);
		et.type = CALENDAR_TIME_LOCALTIME;
		et.time.date.year = y;
		et.time.date.month = m;
		et.time.date.mday = d;
		ret = calendar_record_set_str(todo, _calendar_todo.due_tzid, "Asia/Seoul");
		if(CALENDAR_ERROR_NONE != ret)
		{
			printf("failed to set _calendar_todo.due_tzid\n");
			return -1;
		}
		ret = calendar_record_set_caltime(todo, _calendar_todo.due_time, et);
		if(CALENDAR_ERROR_NONE != ret)
		{
			printf("failed to set _calendar_todo.due_time\n");
			return -1;
		}
	}
	return 0;
}

static int __dft_todo_set_3(calendar_record_h todo, int occurence, int status, int sensitivity)
{
	int ret;

	switch (occurence)
	{
	case CALENDAR_RECURRENCE_NONE:
	case CALENDAR_RECURRENCE_DAILY:
	case CALENDAR_RECURRENCE_WEEKLY:
	case CALENDAR_RECURRENCE_MONTHLY:
	case CALENDAR_RECURRENCE_YEARLY:
	    break;
	default:
		printf("Out of range, so set CALENDAR_RECURRENCE_NONE\n");
		occurence = CALENDAR_RECURRENCE_NONE;
		break;
	}
	ret = calendar_record_set_int(todo, _calendar_todo.count, occurence);
	if(CALENDAR_ERROR_NONE != ret)
	{
		printf("failed to set _calendar_todo.count\n");
		return -1;
	}

	switch (status)
	{
	case CALENDAR_TODO_STATUS_NONE:
	case CALENDAR_TODO_STATUS_NEEDS_ACTION:
	case CALENDAR_TODO_STATUS_COMPLETED:
	case CALENDAR_TODO_STATUS_IN_PROCESS:
	case CALENDAR_TODO_STATUS_CANCELED:
		break;
	default:
		printf("Out of range, so set ALENDAR_TODO_STATUS_NEEDS_ACTION\n");
		status = CALENDAR_TODO_STATUS_NEEDS_ACTION;
		break;
	}
	ret = calendar_record_set_int(todo, _calendar_todo.todo_status, status);
	if(CALENDAR_ERROR_NONE != ret)
	{
		printf("failed to set _calendar_todo.todo_status\n");
		return -1;
	}

	switch (sensitivity)
	{
	case CALENDAR_SENSITIVITY_PUBLIC:
	case CALENDAR_SENSITIVITY_PRIVATE:
	case CALENDAR_SENSITIVITY_CONFIDENTIAL:
		break;
	default:
		printf("Out of range, so set CALENDAR_SENSITIVITY_PUBLIC\n");
		sensitivity = CALENDAR_SENSITIVITY_PUBLIC;
		break;
	}
	ret = calendar_record_set_int(todo, _calendar_todo.sensitivity, sensitivity);
	if(CALENDAR_ERROR_NONE != ret)
	{
		printf("failed to set _calendar_todo.sensitivity\n");
		return -1;
	}
	return 0;
}

int main(int argc, char **argv)
{
	FILE *file;
	int ret;
	int cnt;
	int type;
	char path[256] = {0};
	char _cnt[16] = {0};

	char subject[MAX_COLS];
	char location[MAX_COLS];
	char sdate[16];
	char stime[16];
	char edate[16];
	char etime[16];
	char allday[16];
	int occurence;
	char reminder[16];
	int reminder_value;
	int category;
	int status;
	int sensitivity;
	char description[MAX_COLS];
	calendar_record_h record = NULL;

	printf("argc(%d)\n", argc);

	if (argc != 3) {
		printf("Check parameters.\n");
		__dft_calendar_help();
		return -1;
	}

	snprintf(path, sizeof(path), "%s", argv[1]);
	snprintf(_cnt, sizeof(_cnt), "%s", argv[2]);
	cnt = atoi(_cnt);

	type = __dft_get_file_type(argv[1]);
	if (type != CALENDAR_BOOK_TYPE_EVENT && type != CALENDAR_BOOK_TYPE_TODO)
	{
		printf("Invalid book type");
		return -1;
	}

	if (cnt <= 0) {
		printf("Invalid count(%d)\n", cnt);
		return -1;
	}

	file = fopen(path, "r");
	if (file == NULL) {
		printf("Failed to open file(%s)\n", path);
		return -1;
	}

	ret = calendar_connect();
	if (CALENDAR_ERROR_NONE != ret)
	{
		printf("calendar_connect() failed\n");
		fclose(file);
		return -1;
	}

	while ( !feof(file) && fscanf(file,
				"%32767[^\t]\t %32767[^\t]\t "
				"%s %s %s %s"
				"%s %d "
				"%s %d "
				"%d %d %d "
				"%32767[^\0] ",
				subject, location,
				sdate, stime, edate, etime,
				allday, &occurence,
				reminder, &reminder_value,
				&category, &status, &sensitivity,
				description) > 1) {

		switch (type)
		{
		case CALENDAR_BOOK_TYPE_EVENT:
			ret = CALENDAR_ERROR_NONE;

			ret |= calendar_record_create(_calendar_event._uri, &record);
			ret |= __dft_event_set_1(record, 1, subject, location);
			ret |= __dft_event_set_2(record, allday, sdate, stime, edate, etime);
			ret |= __dft_event_set_3(record, occurence, status, sensitivity);
			ret |= calendar_db_insert_record(record, NULL);
			ret |= calendar_record_destroy(record, true);

			if(CALENDAR_ERROR_NONE != ret)
			{
				printf("__dft_work_event() failed\n");
				return -1;
			}
			break;

		case CALENDAR_BOOK_TYPE_TODO:
			ret = CALENDAR_ERROR_NONE;

			ret |= calendar_record_create(_calendar_todo._uri, &record);
			ret |= __dft_todo_set_1(record, 1, subject, location);
			ret |= __dft_todo_set_2(record, allday, sdate, stime, edate, etime);
			ret |= __dft_todo_set_3(record, occurence, status, sensitivity);
			ret |= calendar_db_insert_record(record, NULL);
			ret |= calendar_record_destroy(record, true);

			if(CALENDAR_ERROR_NONE != ret)
			{
				printf("__dft_work_todo() failed\n");
				return -1;
			}
			break;
		}

		cnt--;
		if (cnt <= 0) {
			break;
		}
	}

	fclose(file);
	ret = calendar_disconnect();
	if(CALENDAR_ERROR_NONE != ret)
	{
		printf("calendar_disconnect() failed \n");
		return -1;
	}
	return 0;
}
