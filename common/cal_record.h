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
#ifndef __CALENDAR_SVC_RECORD_H__
#define __CALENDAR_SVC_RECORD_H__

#include "calendar_record.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	CAL_RECORD_TYPE_INVALID = 0,
	CAL_RECORD_TYPE_CALENDAR,
	CAL_RECORD_TYPE_EVENT ,
	CAL_RECORD_TYPE_TODO,
	CAL_RECORD_TYPE_TIMEZONE,
	CAL_RECORD_TYPE_ATTENDEE,
	CAL_RECORD_TYPE_ALARM,
	CAL_RECORD_TYPE_INSTANCE_NORMAL,
	CAL_RECORD_TYPE_INSTANCE_ALLDAY,
	CAL_RECORD_TYPE_INSTANCE_NORMAL_EXTENDED,
	CAL_RECORD_TYPE_INSTANCE_ALLDAY_EXTENDED,
	CAL_RECORD_TYPE_UPDATED_INFO,
	CAL_RECORD_TYPE_SEARCH,
	CAL_RECORD_TYPE_EXTENDED,
} cal_record_type_e;

typedef int (*cal_record_create_cb)(calendar_record_h* out_record);
typedef int (*cal_record_destroy_cb)(calendar_record_h record, bool delete_child);
typedef int (*cal_record_clone_cb)(calendar_record_h record, calendar_record_h* out_record);
typedef int (*cal_record_get_str_cb)(calendar_record_h record, unsigned int property_id, char** out_str);
typedef int (*cal_record_get_str_p_cb)(calendar_record_h record, unsigned int property_id, char** out_str);
typedef int (*cal_record_get_int_cb)(calendar_record_h record, unsigned int property_id, int* out_value);
typedef int (*cal_record_get_double_cb)(calendar_record_h record, unsigned int property_id, double* out_value);
typedef int (*cal_record_get_lli_cb)(calendar_record_h record, unsigned int property_id, long long int* out_value);
typedef int (*cal_record_get_caltime_cb)(calendar_record_h record, unsigned int property_id, calendar_time_s* out_value);
typedef int (*cal_record_set_str_cb)(calendar_record_h record, unsigned int property_id, const char* value);
typedef int (*cal_record_set_int_cb)(calendar_record_h record, unsigned int property_id, int value);
typedef int (*cal_record_set_double_cb)(calendar_record_h record, unsigned int property_id, double value);
typedef int (*cal_record_set_lli_cb)(calendar_record_h record, unsigned int property_id, long long int value);
typedef int (*cal_record_set_caltime_cb)(calendar_record_h record, unsigned int property_id, calendar_time_s value);
typedef int (*cal_record_add_child_record_cb)(calendar_record_h record, unsigned int property_id, calendar_record_h child_record);
typedef int (*cal_record_remove_child_record_cb)(calendar_record_h record, unsigned int property_id, calendar_record_h child_record);
typedef int (*cal_record_get_child_record_count_cb)(calendar_record_h record, unsigned int property_id, unsigned int* count);
typedef int (*cal_record_get_child_record_at_p_cb)(calendar_record_h record, unsigned int property_id, int index, calendar_record_h* child_record);
typedef int (*cal_record_clone_child_record_list_cb)(calendar_record_h record, unsigned int property_id, calendar_list_h* out_list);

typedef enum {
	CAL_PROPERTY_FLAG_PROJECTION = 0x00000001,
	CAL_PROPERTY_FLAG_DIRTY = 0x00000002,
} cal_properties_flag_e;

typedef struct {
	cal_record_create_cb create;
	cal_record_destroy_cb destroy;
	cal_record_clone_cb clone;
	cal_record_get_str_cb get_str;
	cal_record_get_str_p_cb get_str_p;
	cal_record_get_int_cb get_int;
	cal_record_get_double_cb get_double;
	cal_record_get_lli_cb get_lli;
	cal_record_get_caltime_cb get_caltime;
	cal_record_set_str_cb set_str;
	cal_record_set_int_cb set_int;
	cal_record_set_double_cb set_double;
	cal_record_set_lli_cb set_lli;
	cal_record_set_caltime_cb set_caltime;
	cal_record_add_child_record_cb add_child_record;
	cal_record_remove_child_record_cb remove_child_record;
	cal_record_get_child_record_count_cb get_child_record_count;
	cal_record_get_child_record_at_p_cb get_child_record_at_p;
	cal_record_clone_child_record_list_cb clone_child_record_list;
} cal_record_plugin_cb_s;

typedef struct {
	cal_record_type_e type;
	cal_record_plugin_cb_s *plugin_cb;
	const char* view_uri;
	unsigned int properties_max_count;
	unsigned char *properties_flags;
	unsigned char property_flag;
} cal_record_s;

#define CAL_RECORD_INIT_COMMON(common, intype, cb, uri) do {\
	(common)->type = (intype);\
	(common)->plugin_cb = (cb);\
	(common)->view_uri = (uri);\
	(common)->properties_max_count = 0;\
	(common)->properties_flags = NULL;\
	(common)->property_flag = 0;\
} while (0)

#define CAL_RECORD_COPY_COMMON(dst, src) do {\
	(dst)->type = (src)->type;\
	(dst)->plugin_cb = (src)->plugin_cb;\
	(dst)->view_uri = (src)->view_uri;\
	(dst)->properties_max_count = (src)->properties_max_count;\
	if ((src)->properties_flags) {\
		(dst)->properties_flags  = calloc((dst)->properties_max_count, sizeof(char));\
		if ((dst)->properties_flags)\
		memcpy((dst)->properties_flags, (src)->properties_flags, sizeof(char)*(dst)->properties_max_count);\
	} \
	(dst)->property_flag = (src)->property_flag;\
} while (0)

#define CAL_RECORD_RESET_COMMON(src) do {\
	if ((src)->properties_flags) {\
		free((src)->properties_flags); \
		(src)->properties_max_count = 0;\
		(src)->properties_flags = NULL;\
		(src)->property_flag = 0;\
	} \
} while (0)

cal_record_plugin_cb_s* cal_record_get_plugin_cb(cal_record_type_e type);

bool cal_record_check_property_flag(calendar_record_h record, unsigned int property_id, cal_properties_flag_e flag);
int cal_record_set_projection(calendar_record_h record, const unsigned int *projection, const int projection_count, int properties_max_count);

int cal_record_set_str(calendar_record_h record, unsigned int property_id, const char* value);
int cal_record_set_int(calendar_record_h record, unsigned int property_id, int value);
int cal_record_set_double(calendar_record_h record, unsigned int property_id, double value);
int cal_record_set_lli(calendar_record_h record, unsigned int property_id, long long int value);
int cal_record_set_caltime(calendar_record_h record, unsigned int property_id, calendar_time_s value);

#ifdef __cplusplus
}
#endif

#endif /* __CALENDAR_SVC_RECORD_H__ */

