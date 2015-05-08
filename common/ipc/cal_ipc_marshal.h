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
#ifndef __CAL_IPC_MARSHAL__
#define __CAL_IPC_MARSHAL__

#include <pims-ipc-data.h>
#include "cal_typedef.h"
#include "cal_record.h"

typedef int (*cal_ipc_unmarshal_record_cb)(const pims_ipc_data_h ipc_data, calendar_record_h record);
typedef int (*cal_ipc_marshal_record_cb)(const calendar_record_h record, pims_ipc_data_h ipc_data);
typedef int (*cal_ipc_marshal_record_get_primary_id_cb)(const calendar_record_h record, unsigned int *property_id, int *id);

typedef struct {
	cal_ipc_unmarshal_record_cb unmarshal_record;
	cal_ipc_marshal_record_cb marshal_record;
	cal_ipc_marshal_record_get_primary_id_cb get_primary_id;
} cal_ipc_marshal_record_plugin_cb_s;

int cal_ipc_unmarshal_record(const pims_ipc_data_h ipc_data, calendar_record_h* precord);
int cal_ipc_marshal_record(const calendar_record_h record, pims_ipc_data_h ipc_data);
int cal_ipc_marshal_record_get_primary_id(const calendar_record_h record, unsigned int *property_id, int *id);

int cal_ipc_unmarshal_char(const pims_ipc_data_h ipc_data, char** ppbufchar);
int cal_ipc_unmarshal_int(const pims_ipc_data_h data, int *pout);
int cal_ipc_unmarshal_uint(const pims_ipc_data_h data, unsigned int *pout);
int cal_ipc_unmarshal_lli(const pims_ipc_data_h data, long long int *pout);
int cal_ipc_unmarshal_long(const pims_ipc_data_h data, long *pout);
int cal_ipc_unmarshal_double(const pims_ipc_data_h data, double *pout);
int cal_ipc_unmarshal_caltime(const pims_ipc_data_h data, calendar_time_s *pout);
int cal_ipc_unmarshal_record_common(const pims_ipc_data_h ipc_data, cal_record_s* common);

int cal_ipc_marshal_char(const char* bufchar, pims_ipc_data_h ipc_data);
int cal_ipc_marshal_int(const int in, pims_ipc_data_h ipc_data);
int cal_ipc_marshal_uint(const unsigned int in, pims_ipc_data_h ipc_data);
int cal_ipc_marshal_lli(const long long int in, pims_ipc_data_h ipc_data);
int cal_ipc_marshal_long(const long in, pims_ipc_data_h ipc_data);
int cal_ipc_marshal_double(const double in, pims_ipc_data_h ipc_data);
int cal_ipc_marshal_caltime(const calendar_time_s in, pims_ipc_data_h ipc_data);
int cal_ipc_marshal_record_common(const cal_record_s* common, pims_ipc_data_h ipc_data);

int cal_ipc_unmarshal_query(const pims_ipc_data_h ipc_data, calendar_query_h *query);
int cal_ipc_marshal_query(const calendar_query_h query, pims_ipc_data_h ipc_data);
int cal_ipc_unmarshal_list(const pims_ipc_data_h ipc_data, calendar_list_h *list);
int cal_ipc_marshal_list(const calendar_list_h list, pims_ipc_data_h ipc_data);

int cal_ipc_unmarshal_child_list(const pims_ipc_data_h ipc_data, calendar_list_h* list);

/*
 * for property_id
 */
#define CAL_IPC_CHECK_PROPERTIES_FLAG(src) (CAL_IPC_CHECK_PROJECTION(src) || CAL_IPC_CHECK_DIRTY(src))
#define CAL_IPC_CHECK_PROJECTION(src) ((src) & (unsigned char)CAL_PROPERTY_FLAG_PROJECTION)
#define CAL_IPC_CHECK_DIRTY(src) ((src) & (unsigned char)CAL_PROPERTY_FLAG_DIRTY)

#endif /* __CAL_IPC_MARSHAL__ */
