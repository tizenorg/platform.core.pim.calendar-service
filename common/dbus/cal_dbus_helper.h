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

#ifndef __CALENDAR_SVC_DBUS_UTILS_H__
#define __CALENDAR_SVC_DBUS_UTILS_H__

/*
 * DBUS_INTERFACE name validation.
 * Each element must only contain the ASCII characters "[A-Z][a-z][0-9]_" and must not begin with a digit.
 */
#ifndef CAL_DBUS_INTERFACE
#define CAL_DBUS_INTERFACE "org.tizen.calendar_service.dbus"
#warning "CAL_DBUS_INTERFACE is redefined"
#endif

#define CAL_DBUS_OBJPATH "/org/tizen/calendar_service/dbus"

GVariant *cal_dbus_utils_common_to_gvariant(cal_record_s *rec);
GVariant *cal_dbus_utils_handle_to_gvariant(calendar_h handle);
GVariant *cal_dbus_utils_record_to_gvariant(calendar_record_h record);
GVariant *cal_dbus_utils_list_to_gvariant(calendar_list_h list);
GVariant *cal_dbus_utils_query_to_gvariant(calendar_query_h query);
GVariant *cal_dbus_utils_ids_to_gvariant(int *ids, int count);
GVariant *cal_dbus_utils_stream_to_gvariant(int stream_size, char *stream);

int cal_dbus_utils_gvariant_to_common(GVariant *arg_common, cal_record_s **rec);
int cal_dbus_utils_gvariant_to_handle(GVariant *arg_handle, calendar_h *handle);
int cal_dbus_utils_gvariant_to_record(GVariant *arg_record, calendar_record_h *record);
int cal_dbus_utils_gvariant_to_list(GVariant *arg_list, calendar_list_h *list);
int cal_dbus_utils_gvariant_to_query(GVariant *arg_query, calendar_query_h *query);
int cal_dbus_utils_gvariant_to_ids(GVariant *arg_ids, int count, int **ids);
int cal_dbus_utils_gvariant_to_stream(GVariant *arg_stream, int *out_size, char **out_stream);

#endif /*__CALENDAR_SVC_DBUS_UTILS_H__ */
