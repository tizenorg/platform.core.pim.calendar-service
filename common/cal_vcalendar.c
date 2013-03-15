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

API int calendar_vcalendar_make_from_records(calendar_list_h list, char **vcalendar_stream)
{
	int ret;
	cal_make_s *b;
	char *ical;

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
		return CALENDAR_ERROR_NO_DATA;
	}

	*vcalendar_stream = ical;

	return CALENDAR_ERROR_NONE;
}

/*
 * parse from here
 */

API int calendar_vcalendar_parse_to_calendar(const char* vcalendar_stream, calendar_list_h *out_list)
{
	char *prop, *cont;
	char *stream = NULL;
	char *cursor = NULL;
	calendar_list_h list = NULL;

	retvm_if(vcalendar_stream == NULL, CALENDAR_ERROR_INVALID_PARAMETER,
			"Invalid argument: vcalendar_stream is NULL");
	retvm_if(out_list == NULL, CALENDAR_ERROR_INVALID_PARAMETER,
			"Invalid argument: calendar_list_h * is NULL");

	stream  = strdup(vcalendar_stream);
	cursor = stream;

	cursor = _cal_vcalendar_parse_remove_space(cursor);
	if (cursor == NULL) {
		ERR("_cal_vcalendar_parse_remove_space() failed");
		CAL_FREE(stream);
		return CALENDAR_ERROR_OUT_OF_MEMORY;
	}
	_cal_vcalendar_parse_unfolding(cursor);

	cursor = _cal_vcalendar_parse_read_line(cursor, &prop, &cont);
	if (cursor == NULL) {
		ERR("_cal_vcalendar_parse_read_line() failed");
		CAL_FREE(prop);
		CAL_FREE(cont);
		CAL_FREE(stream);
		return CALENDAR_ERROR_OUT_OF_MEMORY;
	}

	if (strncmp(prop, "BEGIN", strlen("BEGIN")) ||
			strncmp(cont + 1, "VCALENDAR", strlen("VCALENDAR"))) {
		ERR("Failed to find BEGIN:VCALDENDAR [%s][%s]", prop, cont);
		CAL_FREE(prop);
		CAL_FREE(cont);
		return -1;
	}
	CAL_FREE(prop);
	CAL_FREE(cont);

	_cal_vcalendar_parse_vcalendar(&list, cursor);
	if (list == NULL) {
		ERR("No schedules");
		CAL_FREE(stream);
		return CALENDAR_ERROR_NO_DATA;
	}

	calendar_list_first(list);
	*out_list = list;

	CAL_FREE(stream);
	return CALENDAR_ERROR_NONE;
}

API int calendar_vcalendar_parse_to_calendar_foreach(const char *vcalendar_file_path, calendar_vcalendar_parse_cb callback, void *user_data)
{
    FILE *file;
	int ret = CALENDAR_ERROR_NONE;
    int buf_size, len;
    char *stream;
    char buf[1024];

    retvm_if(vcalendar_file_path == NULL, CALENDAR_ERROR_INVALID_PARAMETER,
            "Invalid argument: vcalendar_file_path is NULL");
    retvm_if(callback == NULL, CALENDAR_ERROR_INVALID_PARAMETER,
            "Invalid argument: callback is NULL");

    file = fopen(vcalendar_file_path, "r");

    retvm_if(file == NULL, CALENDAR_ERROR_INVALID_PARAMETER,
            "Invalid argument: no file");

    len = 0;
    buf_size = ICALENAR_BUFFER_MAX;
    stream = malloc(ICALENAR_BUFFER_MAX);

    while (fgets(buf, sizeof(buf), file))
    {
        if (len + sizeof(buf) < buf_size)
        {
            len += snprintf(stream + len, strlen(buf) +1, "%s", buf);
        }
        else
        {
            char *new_stream;
            buf_size *= 2;
            new_stream = realloc(stream, buf_size);
            if (new_stream)
            {
                stream = new_stream;
            } else
            {
                if (stream) free(stream);
                fclose(file);
                ERR("out of memory");
                return CALENDAR_ERROR_OUT_OF_MEMORY;
            }
            len += snprintf(stream + len, strlen(buf) +1, "%s", buf);
        }

        if (!strncmp(buf, "END:VCALENDAR", strlen("END:VCALENDAR")))
        {
            DBG("end vcalendar");
            calendar_list_h list = NULL;
            int count = 0, i = 0;

            if (calendar_vcalendar_parse_to_calendar(stream, &list) != CALENDAR_ERROR_NONE)
            {
                ERR("calendar_vcalendar_parse_to_calendar fail");
                if (stream) free(stream);
                fclose(file);
                return CALENDAR_ERROR_INVALID_PARAMETER;
            }

            ret = calendar_list_get_count(list, &count);
			if (ret != CALENDAR_ERROR_NONE || count < 1)
			{
				ERR("calendar_list_get_count() failed");
				calendar_list_destroy(list, true);
				if (stream) free(stream);
				fclose(file);
				return ret;
			}

			DBG("vcalendar has count(%d)", count);
            calendar_list_first(list);
            for(i = 0; i < count; i++)
            {
                calendar_record_h record = NULL;
                if (calendar_list_get_current_record_p(list,&record) != CALENDAR_ERROR_NONE)
                {
                    ERR("calendar_list_get_count fail");
                    calendar_list_destroy(list, true);
                    if (stream) free(stream);
                    fclose(file);
                    return CALENDAR_ERROR_INVALID_PARAMETER;
                }
                if (!callback(record, user_data))
                {
                    ERR("callback is false");
                    calendar_list_destroy(list, true);
                    if (stream) free(stream);
                    fclose(file);
                    return CALENDAR_ERROR_INVALID_PARAMETER;
                }
				calendar_list_next(list);
            }

            calendar_list_destroy(list, true);
            len = 0;
        }
    }
    if (stream) free(stream);
    fclose(file);

    return CALENDAR_ERROR_NONE;
}
