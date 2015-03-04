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

#include "calendar_vcalendar.h"

#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_record.h"
#include "cal_view.h"
#include "cal_time.h"
#include "cal_list.h"

#include "cal_vcalendar.h"
#include "cal_vcalendar_make.h"
#include "cal_vcalendar_parse.h"

#define ICALENAR_BUFFER_MAX (1024*1024)

/*
 * vcalendar should not have multi version: ver1.0 or 2.0 only.
 * could have multi timezone events: MULTI BEGIN:VCALENDAR.
 */
API int calendar_vcalendar_make_from_records(calendar_list_h list, char **vcalendar_stream)
{
	int ret;
	cal_make_s *b;
	char *ical = NULL;

	retvm_if(list == NULL, CALENDAR_ERROR_INVALID_PARAMETER,
			"Invalid argument: calendar_list_h is NULL");
	retvm_if(vcalendar_stream == NULL, CALENDAR_ERROR_INVALID_PARAMETER,
			"Invalid argument: vcalendar_stream is NULL");

	b = _cal_vcalendar_make_new();
	retvm_if(!b, CALENDAR_ERROR_OUT_OF_MEMORY,
			"_cal_vcalendar_make_new() Failed");

	ret = _cal_vcalendar_make_vcalendar(b, list);

	if (ret < 0) {
		_cal_vcalendar_make_free(&b);
		return ret;
	}

	ical = _cal_vcalendar_make_get_data(b);
	_cal_vcalendar_make_free(&b);

	if (!ical) {
		ERR("Failed to get ical data");
		return CALENDAR_ERROR_OUT_OF_MEMORY;
	}

	if (!*ical) {
		ERR("No ical data");
		free(ical);
		return CALENDAR_ERROR_NO_DATA;
	}

	*vcalendar_stream = ical;

	return CALENDAR_ERROR_NONE;
}

static const char* __calendar_vcalendar_get_vcalendar_object(const char *original, char **pvcalendar_object)
{
	int len = 0;
	const char *vcal_start = original;
	const char *vcal_cursor = NULL;
	bool new_line = false;
	char *vcalendar_object = NULL;

	retv_if(NULL == pvcalendar_object, original);
	*pvcalendar_object = NULL;

	while ('\n' == *vcal_start || '\r' == *vcal_start)
		vcal_start++;

	if (strncmp(vcal_start, "BEGIN:VCALENDAR", strlen("BEGIN:VCALENDAR")))
		return vcal_start;

	vcal_start += strlen("BEGIN:VCALENDAR");
	while ('\n' == *vcal_start || '\r' == *vcal_start)
		vcal_start++;
	vcal_cursor = vcal_start;

	while (*vcal_cursor) {
		if (new_line) {
			if (0 == strncmp(vcal_cursor, "END:VCALENDAR", strlen("END:VCALENDAR"))) {
				vcal_cursor += strlen("END:VCALENDAR");
				while ('\r' == *vcal_cursor || '\n' == *vcal_cursor) {
					new_line = true;
					vcal_cursor++;
				}

				len = (int)vcal_cursor - (int)vcal_start;
				vcalendar_object = calloc(len + 1, sizeof(char));
				memcpy(vcalendar_object, vcal_start, len);
				*pvcalendar_object = vcalendar_object;

				return vcal_cursor;
			}
			new_line = false;
		}
		vcal_cursor++;
		while ('\r' == *vcal_cursor || '\n' == *vcal_cursor) {
			new_line = true;
			vcal_cursor++;
		}
	}
	return vcal_cursor;
}

/*
 * parse from here
 */
API int calendar_vcalendar_parse_to_calendar(const char* vcalendar_stream, calendar_list_h *out_list)
{
	int count = 0;
	const char *cursor = NULL;
	char *vcalendar_object = NULL;
	calendar_error_e err;
	calendar_list_h list = NULL;

	retvm_if(vcalendar_stream == NULL, CALENDAR_ERROR_INVALID_PARAMETER,
			"Invalid argument: vcalendar_stream is NULL");
	retvm_if(out_list == NULL, CALENDAR_ERROR_INVALID_PARAMETER,
			"Invalid argument: calendar_list_h * is NULL");

	// get vcalendar object
	cursor = vcalendar_stream;

	int ret = 0;
	ret = calendar_list_create(&list);
	retvm_if (CALENDAR_ERROR_NONE != ret, ret, "Failed to calendar_list_create()");

	_cal_time_init();

	while (NULL != (cursor = __calendar_vcalendar_get_vcalendar_object(cursor, &vcalendar_object))) {
		if (NULL == vcalendar_object)
			break;

		err = _cal_vcalendar_parse_vcalendar_object(vcalendar_object, list, NULL);
		if (CALENDAR_ERROR_NONE != err) {
			ERR("_cal_vcalendar_parse_vcalendar_object() failed(%d)", err);
			calendar_list_destroy(list, true);
			free(vcalendar_object);
			_cal_time_fini();
			return err;
		}
		free(vcalendar_object);
	}
	calendar_list_get_count(list, &count);
	if (count <= 0) {
		calendar_list_destroy(list, true);
		_cal_time_fini();
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	calendar_list_first(list);
	*out_list = list;
	_cal_time_fini();
	return CALENDAR_ERROR_NONE;
}

API int calendar_vcalendar_parse_to_calendar_foreach(const char *vcalendar_file_path, calendar_vcalendar_parse_cb callback, void *user_data)
{
	FILE *file;
	int buf_size, len;
	char *stream;
	char buf[1024];
	vcalendar_foreach_s *foreach_data = NULL;

	retvm_if(vcalendar_file_path == NULL, CALENDAR_ERROR_INVALID_PARAMETER,
			"Invalid argument: vcalendar_file_path is NULL");
	retvm_if(callback == NULL, CALENDAR_ERROR_INVALID_PARAMETER,
			"Invalid argument: callback is NULL");

	int ret = 0;
	calendar_list_h list = NULL;
	ret = calendar_list_create(&list);
	retvm_if (CALENDAR_ERROR_NONE != ret, ret, "Failed to calendar_list_create()");

	file = fopen(vcalendar_file_path, "r");
	if (file == NULL) {
		ERR("Invalid argument: no file");
		calendar_list_destroy(list, true);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	len = 0;
	buf_size = ICALENAR_BUFFER_MAX;
	stream = malloc(ICALENAR_BUFFER_MAX);

	foreach_data = calloc(1, sizeof(vcalendar_foreach_s));
	foreach_data->callback = callback;
	foreach_data->user_data = user_data;
	foreach_data->ret = true;

	while (fgets(buf, sizeof(buf), file)) {
		if (len + sizeof(buf) < buf_size) {
			len += snprintf(stream + len, strlen(buf) +1, "%s", buf);
		}
		else {
			char *new_stream;
			buf_size *= 2;
			new_stream = realloc(stream, buf_size);
			if (new_stream) {
				stream = new_stream;
			}
			else {
				free(stream);
				fclose(file);
				free(foreach_data);
				calendar_list_destroy(list, true);
				ERR("out of memory");
				return CALENDAR_ERROR_OUT_OF_MEMORY;
			}
			len += snprintf(stream + len, strlen(buf) +1, "%s", buf);
		}

		if (0 == strncmp(buf, "END:VCALENDAR", strlen("END:VCALENDAR"))) {
			DBG("end vcalendar");
			int err;
			char *vcalendar_object = NULL;
			__calendar_vcalendar_get_vcalendar_object(stream, &vcalendar_object);
			err = _cal_vcalendar_parse_vcalendar_object(vcalendar_object, list, foreach_data);
			if (CALENDAR_ERROR_NONE != err || false == foreach_data->ret) {
				ERR("_cal_vcalendar_parse_vcalendar_object() failed(%d)", err);
				calendar_list_destroy(list, true);
				free(vcalendar_object);
				free(stream);
				free(foreach_data);
				fclose(file);
				return err;
			}
			free(vcalendar_object);
			len = 0;
		}
	}

	calendar_list_destroy(list, true);
	free(stream);
	free(foreach_data);
	fclose(file);

	return CALENDAR_ERROR_NONE;
}
