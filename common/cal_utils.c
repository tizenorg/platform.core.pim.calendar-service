/*
 * Calendar Service
 *
 * Copyright (c) 2015 Samsung Electronics Co., Ltd. All rights reserved.
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
#include <glib.h>
#include <glib/gprintf.h>
#include "cal_internal.h"

void cal_free(void *ptr)
{
	RET_IF(NULL == ptr);
	free(ptr);
}

char* cal_strdup(const char *src)
{
	if (src)
		return strdup(src);
	return NULL;
}

static gint _sort_cb(gconstpointer a, gconstpointer b)
{
	return GPOINTER_TO_INT(a) < GPOINTER_TO_INT(b) ? -1 : 1;
}

char* cal_strdup_with_sort(const char *src)
{
	RETV_IF(NULL == src, NULL);

	char **t = NULL;
	t = g_strsplit_set(src, " ,", -1);
	if (NULL == t) {
		ERR("g_strsplit_set() Fail");
		return NULL;
	}
	int len_t = 0;
	len_t = g_strv_length(t);
	if (0 == len_t) {
		ERR("Empty src");
		return NULL;
	}

	int i;
	GList *l = NULL;
	for (i = 0; i < len_t; i++) {
		if (NULL == t[i] || '\0' == *t[i])
			continue;
		int num = atoi(t[i]);
		if (0 == num) {
			ERR("Invalid parameter[%s]", t[i]);
			continue;
		}
		if (g_list_find(l, GINT_TO_POINTER(num))) {
			ERR("Find same data[%s]", t[i]);
			continue;
		}
		l = g_list_append(l, GINT_TO_POINTER(num));
	}
	l = g_list_sort(l, _sort_cb);

	int len_src = strlen(src) +1;
	char *out_str = calloc(len_src, sizeof(char));
	if (NULL == out_str) {
		ERR("calloc() Fail");
		g_list_free(l);
		g_strfreev(t);
		return NULL;
	}
	int len = 0;
	GList *cursor = NULL;
	cursor = g_list_first(l);
	while (cursor) {
		len += snprintf(out_str +len, len_src -len, "%d,", GPOINTER_TO_INT(cursor->data));
		cursor = g_list_next(cursor);
	}
	if (out_str)
		out_str[len -1] = '\0';

	g_list_free(l);
	g_strfreev(t);

	return out_str;
}
