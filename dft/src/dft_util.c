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
#include <string.h>
#include <glib.h>
#include <unicode/ucal.h>
#include <unicode/ustring.h>
#include <unicode/ustdio.h>
#include <unicode/udat.h>
#include <sys/types.h>

#define ms2sec(ms) (long long int)(ms / 1000.0)
#define sec2ms(s) (s * 1000.0)

long long int _convert_to_utime(char *date, char *time)
{
	int y, mon, d, h, min;
	char buf[8];
	UErrorCode status = U_ZERO_ERROR;
	UCalendar *cal;
	UDate ud;

	if (date == NULL) {
		printf("Invalid argument: data is NULL\n");
		return -1;
	}

	if (time == NULL) {
		printf("Invalid argument: time is NULL\n");
		return -1;
	}

	cal = ucal_open(NULL, -1, NULL, UCAL_TRADITIONAL, &status);
	if (U_FAILURE(status)) {
		printf("ucal_open failed (%s)", u_errorName(status));
		return -1;
	}

	snprintf(buf, 5, "%s", date + 4);
	y = atoi(buf);
	snprintf(buf, 3, "%s", date + 2);
	mon = atoi(buf);
	snprintf(buf, 3, "%s", date);
	d = atoi(buf);

	snprintf(buf, 3, "%s", time);
	h = atoi(buf);
	snprintf(buf, 3, "%s", time + 2);
	min = atoi(buf);

	ucal_setDateTime(cal, y, mon, d, h, min, 0, &status);
	if (U_FAILURE(status)) {
		printf("ucal_setDate failed (%s)", u_errorName(status));
		return -1;
	}

	ud = ucal_getMillis(cal, &status);
	if (U_FAILURE(status)) {
		printf("ucal_getMillis failed (%s)", u_errorName(status));
		return -1;
	}
	ucal_close(cal);
	return ms2sec(ud);
}

int _convert_to_datetime(char *date, int *y, int *m, int *d)
{
	char buf[8];

	if (date == NULL) {
		printf("Invalid argument: data is NULL\n");
		return -1;
	}

	snprintf(buf, 5, "%s", date + 4);
	*y = atoi(buf);
	snprintf(buf, 3, "%s", date + 2);
	*m = atoi(buf);
	snprintf(buf, 3, "%s", date);
	*d = atoi(buf);
	return 0;
}
