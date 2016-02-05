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
#include <string.h>
#include <glib.h>

#include "calendar.h"
#include "cal_view.h"
#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_handle.h"
#include "cal_utils.h"

#define CAL_DBUS_SET_STRING(x) (x) ? x : ""
#define CAL_DBUS_GET_STRING(x) do { \
	x = (NULL != x && '\0' != *x) ? strdup(x) : NULL; \
} while (0)

GVariant* cal_dbus_utils_null_to_gvariant(void)
{
	GVariant *value = NULL;
	value = g_variant_new("(s)", "");
	return value;
}

GVariant *cal_dbus_utils_char_to_gvariant(unsigned char *ch, int count)
{
	GVariantBuilder builder;
	g_variant_builder_init(&builder, G_VARIANT_TYPE("ay"));

	int i;
	for (i = 0; i < count; i++)
		g_variant_builder_add(&builder, "y", ch[i]);

	return g_variant_builder_end(&builder);
}

int cal_dbus_utils_gvariant_to_char(GVariant *arg_char, int count,
		unsigned char **out_flag)
{
	if (0 == count)
		return  CALENDAR_ERROR_NONE;

	GVariantIter *iter_char = NULL;
	g_variant_get(arg_char, "ay", &iter_char);

	unsigned char *flag = calloc(count, sizeof(unsigned char));
	if (NULL == flag) {
		ERR("calloc() Fail");
		return CALENDAR_ERROR_OUT_OF_MEMORY;
	}
	int i = 0;
	for (i = 0; i < count; i++)
		g_variant_iter_loop(iter_char, "y", &flag[i]);

	*out_flag= flag;
	return CALENDAR_ERROR_NONE;
}

GVariant* cal_dbus_utils_common_to_gvariant(cal_record_s *rec)
{
	GVariant *arg_flags = cal_dbus_utils_char_to_gvariant(rec->properties_flags,
			rec->properties_max_count);
	GVariant *value = NULL;
	value = g_variant_new("(isuvy)",
			rec->type,
			CAL_DBUS_SET_STRING(rec->view_uri),
			rec->properties_max_count,
			arg_flags,
			rec->property_flag);
	return value;
}

int cal_dbus_utils_gvariant_to_common(GVariant *arg_common, calendar_record_h *record)
{
	cal_record_s rec = {0};
	GVariant *arg_flags = NULL;
	g_variant_get(arg_common, "(i&suvy)",
			&rec.type,
			rec.view_uri,
			&rec.properties_max_count,
			&arg_flags,
			&rec.property_flag);
	CAL_DBUS_GET_STRING(rec.view_uri);
	cal_dbus_utils_gvariant_to_char(arg_flags, rec.properties_max_count,
			&rec.properties_flags);

	cal_record_s *p = ((cal_record_s *)(*record));

	p->properties_max_count = rec.properties_max_count;
	p->properties_flags = rec.properties_flags;

	return CALENDAR_ERROR_NONE;
}

GVariant* cal_dbus_utils_handle_to_gvariant(calendar_h handle)
{
	GVariant *value = NULL;
	cal_s *p = (cal_s *)handle;
	value = g_variant_new("(ii)",
			p->version, p->connection_count);
	return value;
}

static GVariant* _caltime_to_gvariant(calendar_time_s *ct)
{
	GVariant *value = NULL;
	switch (ct->type) {
	case CALENDAR_TIME_UTIME:
		value = g_variant_new("(x)", ct->time.utime);
		break;
	case CALENDAR_TIME_LOCALTIME:
		value = g_variant_new("(iiiiii)",
				ct->time.date.year, ct->time.date.month, ct->time.date.mday,
				ct->time.date.hour, ct->time.date.minute, ct->time.date.second);
		break;
	}
	return value;
}

static int _gvariant_to_caltime(int type, GVariant *arg_caltime, calendar_time_s *ct)
{
	switch (type) {
	case CALENDAR_TIME_UTIME:
		g_variant_get(arg_caltime, "(x)", &ct->time.utime);
		break;
	case CALENDAR_TIME_LOCALTIME:
		g_variant_get(arg_caltime, "(iiiiii)",
				&ct->time.date.year, &ct->time.date.month, &ct->time.date.mday,
				&ct->time.date.hour, &ct->time.date.minute, &ct->time.date.second);
		break;
	}
	return CALENDAR_ERROR_NONE;
}

GVariant *cal_dbus_utils_ids_to_gvariant(int *ids, int count)
{
	GVariantBuilder builder;
	g_variant_builder_init(&builder, G_VARIANT_TYPE("ai"));

	int i;
	for (i = 0; i < count; i++)
		g_variant_builder_add(&builder, "i", ids[i]);

	return g_variant_builder_end(&builder);
}

static GVariant *_attr_value_to_gvariant(cal_attribute_filter_s *p)
{
	GVariant *value = NULL;

	switch (p->filter_type) {
	case CAL_FILTER_STR:
		value = g_variant_new("s", CAL_DBUS_SET_STRING(p->value.s));
		break;
	case CAL_FILTER_INT:
		value = g_variant_new("i", p->value.i);
		break;
	case CAL_FILTER_DOUBLE:
		value = g_variant_new("d", p->value.d);
		break;
	case CAL_FILTER_LLI:
		value = g_variant_new("x", p->value.lli);
		break;
	case CAL_FILTER_CALTIME:
		value = g_variant_new("(iv)", p->value.caltime.type,
				_caltime_to_gvariant(&p->value.caltime));
		break;
	default:
		ERR("Invalid parameter(0x%x)", p->property_id);
		break;
	}
	return value;
}

static GVariant *_composite_to_gvariant(cal_composite_filter_s *filter)
{
	GVariantBuilder builder;
	g_variant_builder_init(&builder, G_VARIANT_TYPE("a(isiv)"));

	int has_composite = 0;
	cal_composite_filter_s *f = filter;
	do {
		has_composite = 0;
		int count_composite = 0;
		GVariant *arg_composite = NULL;
		if (f->filters) {
			count_composite = g_slist_length(f->filters);
			GSList *cursor = f->filters;

			int is_error = 0;
			GVariantBuilder builder_attribute;
			g_variant_builder_init(&builder_attribute, G_VARIANT_TYPE("a(iiiv)"));
			while (cursor) {
				cal_filter_s *child = cursor->data;
				if (NULL == child) {
					ERR("child is NULL");
					is_error = 1;
					break;
				}

				if (CAL_FILTER_COMPOSITE == child->filter_type) {
					has_composite = 1;
					f = (cal_composite_filter_s *)child;
					break;
				}

				cal_attribute_filter_s *attr = (cal_attribute_filter_s *)child;
				GVariant *arg_attr_value = _attr_value_to_gvariant(attr);
				g_variant_builder_add(&builder_attribute, "(iiiv)",
						attr->filter_type, attr->property_id, attr->match, arg_attr_value);

				cursor = g_slist_next(cursor);
			}

			if (1 == is_error) {
				count_composite = 0;
				arg_composite = cal_dbus_utils_null_to_gvariant();
			} else {
				arg_composite = g_variant_builder_end(&builder_attribute);
			}
		} else {
			arg_composite = cal_dbus_utils_null_to_gvariant();
		}

		g_variant_builder_add(&builder, "(isiv)",
				f->filter_type, f->view_uri, count_composite, arg_composite);
	} while (1 == has_composite);

	return g_variant_builder_end(&builder);
}

static GVariant *_operate_to_gvariant(cal_composite_filter_s *f, int *out_count)
{
	int count_operate = 0;
	GVariant *arg_operate = NULL;
	if (f->filter_ops) {
		GVariantBuilder builder;
		g_variant_builder_init(&builder, G_VARIANT_TYPE("ai"));

		count_operate = g_slist_length(f->filter_ops);
		GSList *cursor = f->filter_ops;
		while (cursor) {
			calendar_filter_operator_e operate = (calendar_filter_operator_e)cursor->data;
			g_variant_builder_add(&builder, "i", operate);
			cursor = g_slist_next(cursor);
		}
		arg_operate = g_variant_builder_end(&builder);
	} else {
		arg_operate = cal_dbus_utils_null_to_gvariant();
	}
	*out_count = count_operate;

	return arg_operate;
}

static GVariant *_filter_to_gvariant(cal_composite_filter_s *f)
{
	GVariant *arg_composite_pack = NULL;
	if (f->filters)
		arg_composite_pack = _composite_to_gvariant(f);
	else
		arg_composite_pack = cal_dbus_utils_null_to_gvariant();

	int count_operate = 0;
	GVariant *arg_operate = NULL;
	if (f->filter_ops)
		arg_operate = _operate_to_gvariant(f, &count_operate);
	else
		arg_operate = cal_dbus_utils_null_to_gvariant();

	GVariant *value = NULL;
	value = g_variant_new("(viv)",
			arg_composite_pack, count_operate, arg_operate);
	return value;
}

static GVariant *_projection_to_gvariant(cal_query_s *p)
{
	int i;
	GVariantBuilder builder;
	g_variant_builder_init(&builder, G_VARIANT_TYPE("au"));
	for (i = 0; i < p->projection_count; i++)
		g_variant_builder_add(&builder, "u", p->projection[i]);

	return g_variant_builder_end(&builder);
}

GVariant *cal_dbus_utils_query_to_gvariant(calendar_query_h query)
{
	RETV_IF(NULL == query, NULL);

	cal_query_s *q = (cal_query_s *)query;
	int has_filter = 0;
	GVariant *arg_filter = NULL;
	if (q->filter) {
		has_filter = 1;
		arg_filter = _filter_to_gvariant(q->filter);
	} else {
		has_filter = 0;
		arg_filter = cal_dbus_utils_null_to_gvariant();
	}

	GVariant *arg_projection = NULL;
	if (q->projection_count)
		arg_projection = _projection_to_gvariant(q);
	else
		arg_projection = cal_dbus_utils_null_to_gvariant();

	GVariant *value = NULL;
	value = g_variant_new("(siviviii)",
			CAL_DBUS_SET_STRING(q->view_uri), has_filter, arg_filter,
			q->projection_count, arg_projection,
			q->sort_property_id, (int)q->asc, (int)q->distinct);

	return value;
}


static GVariant *_book_to_gvariant(calendar_record_h record)
{
	RETV_IF(NULL == record, NULL);

	GVariant *value = NULL;
	cal_book_s *p = (cal_book_s *)record;
	value = g_variant_new("(iisissssiiiissssi)",
			p->index, p->store_type,
			CAL_DBUS_SET_STRING(p->uid),
			p->updated,
			CAL_DBUS_SET_STRING(p->name),
			CAL_DBUS_SET_STRING(p->description),
			CAL_DBUS_SET_STRING(p->color),
			CAL_DBUS_SET_STRING(p->location),
			p->visibility, p->sync_event, p->is_deleted, p->account_id,
			CAL_DBUS_SET_STRING(p->sync_data1),
			CAL_DBUS_SET_STRING(p->sync_data2),
			CAL_DBUS_SET_STRING(p->sync_data3),
			CAL_DBUS_SET_STRING(p->sync_data4),
			p->mode);
	return value;
}

static GVariant *_only_event_to_gvariant(calendar_record_h record)
{
	GVariant *value = NULL;
	cal_event_s *p = (cal_event_s *)record;
	RETV_IF(NULL == record, NULL);

	GVariant *arg_until = _caltime_to_gvariant(&p->until);
	GVariant *arg_start = _caltime_to_gvariant(&p->start);
	GVariant *arg_end = _caltime_to_gvariant(&p->end);

	value = g_variant_new("(iisssssiiiiiiisssiddixixiiiviisssssssssissiiiissssivsivsi)",
			p->index, p->calendar_id,
			CAL_DBUS_SET_STRING(p->summary),
			CAL_DBUS_SET_STRING(p->description),
			CAL_DBUS_SET_STRING(p->location),
			CAL_DBUS_SET_STRING(p->categories),
			CAL_DBUS_SET_STRING(p->exdate),
			p->event_status, p->priority, p->timezone, p->contact_id, p->busy_status,
			p->sensitivity, p->meeting_status,
			CAL_DBUS_SET_STRING(p->uid),
			CAL_DBUS_SET_STRING(p->organizer_name),
			CAL_DBUS_SET_STRING(p->organizer_email),
			p->original_event_id, p->latitude, p->longitude, p->email_id,
			p->created_time, p->is_deleted, p->last_mod, p->freq, p->range_type,
			p->until.type, arg_until, p->count, p->interval,
			CAL_DBUS_SET_STRING(p->bysecond),
			CAL_DBUS_SET_STRING(p->byminute),
			CAL_DBUS_SET_STRING(p->byhour),
			CAL_DBUS_SET_STRING(p->byday),
			CAL_DBUS_SET_STRING(p->bymonthday),
			CAL_DBUS_SET_STRING(p->byyearday),
			CAL_DBUS_SET_STRING(p->byweekno),
			CAL_DBUS_SET_STRING(p->bymonth),
			CAL_DBUS_SET_STRING(p->bysetpos),
			p->wkst,
			CAL_DBUS_SET_STRING(p->recurrence_id),
			CAL_DBUS_SET_STRING(p->rdate),
			p->has_attendee, p->has_alarm, p->system_type, p->updated,
			CAL_DBUS_SET_STRING(p->sync_data1),
			CAL_DBUS_SET_STRING(p->sync_data2),
			CAL_DBUS_SET_STRING(p->sync_data3),
			CAL_DBUS_SET_STRING(p->sync_data4),
			p->start.type, arg_start,
			CAL_DBUS_SET_STRING(p->start_tzid),
			p->end.type, arg_end,
			CAL_DBUS_SET_STRING(p->end_tzid),
			p->is_allday);
	return value;
}

static GVariant *_alarm_to_gvariant(calendar_record_h record)
{
	GVariant *value = NULL;
	cal_alarm_s *p = (cal_alarm_s *)record;
	GVariant *arg_alarm = _caltime_to_gvariant(&p->alarm);
	value = g_variant_new("(iiiiissisiv)",
			p->id, p->parent_id, p->is_deleted, p->remind_tick, p->remind_tick_unit,
			CAL_DBUS_SET_STRING(p->alarm_description),
			CAL_DBUS_SET_STRING(p->alarm_summary),
			p->alarm_action,
			CAL_DBUS_SET_STRING(p->alarm_attach),
			p->alarm.type, arg_alarm);
	return value;
}

static GVariant *_attendee_to_gvariant(calendar_record_h record)
{
	GVariant *value = NULL;
	cal_attendee_s *p = (cal_attendee_s *)record;
	value = g_variant_new("(iisiisssiiissss)",
			p->id, p->parent_id,
			CAL_DBUS_SET_STRING(p->attendee_number),
			p->attendee_cutype, p->attendee_ct_index,
			CAL_DBUS_SET_STRING(p->attendee_uid),
			CAL_DBUS_SET_STRING(p->attendee_group),
			CAL_DBUS_SET_STRING(p->attendee_email),
			p->attendee_role, p->attendee_status, p->attendee_rsvp,
			CAL_DBUS_SET_STRING(p->attendee_delegatee_uri),
			CAL_DBUS_SET_STRING(p->attendee_delegator_uri),
			CAL_DBUS_SET_STRING(p->attendee_name),
			CAL_DBUS_SET_STRING(p->attendee_member));
	return value;
}

static GVariant *_timezone_to_gvariant(calendar_record_h record)
{
	GVariant *value = NULL;
	cal_timezone_s *p = (cal_timezone_s *)record;
	value = g_variant_new("(iisiiiiisiiiiii)",
			p->index, p->tz_offset_from_gmt,
			CAL_DBUS_SET_STRING(p->standard_name),
			p->std_start_month, p->std_start_position_of_week, p->std_start_day,
			p->std_start_hour, p->standard_bias,
			CAL_DBUS_SET_STRING(p->day_light_name),
			p->day_light_start_month, p->day_light_start_position_of_week,
			p->day_light_start_day, p->day_light_start_hour, p->day_light_bias,
			p->calendar_id);
	return value;
}

static GVariant *_extended_to_gvariant(calendar_record_h record)
{
	GVariant *value = NULL;
	cal_extended_s *p = (cal_extended_s *)record;
	value = g_variant_new("(iiiss)",
			p->id, p->record_id, p->record_type,
			CAL_DBUS_SET_STRING(p->key),
			CAL_DBUS_SET_STRING(p->value));
	return value;
}

static GVariant *_event_to_gvariant(calendar_record_h record)
{
	int i;
	RETV_IF(NULL == record, NULL);

	GVariant *arg_only_event = _only_event_to_gvariant(record);
	if (NULL == arg_only_event) {
		ERR("_only_event_to_gvariant() Fail");
		return NULL;
	}

	unsigned int count_alarm = 0;
	GVariantBuilder builder_alarm;
	g_variant_builder_init(&builder_alarm, G_VARIANT_TYPE("av"));
	calendar_record_get_child_record_count(record, _calendar_event.calendar_alarm, &count_alarm);
	if (0 < count_alarm) {
		for (i = 0 ; i < count_alarm; i++) {
			calendar_record_h alarm = NULL;
			calendar_record_get_child_record_at_p(record, _calendar_event.calendar_alarm, i, &alarm);
			g_variant_builder_add(&builder_alarm, "v", _alarm_to_gvariant(alarm));
		}
	} else {
		DBG("No alarm");
		g_variant_builder_add(&builder_alarm, "v", cal_dbus_utils_null_to_gvariant());
	}

	unsigned int count_attendee = 0;
	GVariantBuilder builder_attendee;
	g_variant_builder_init(&builder_attendee, G_VARIANT_TYPE("av"));
	calendar_record_get_child_record_count(record, _calendar_event.calendar_attendee, &count_attendee);
	if (0 < count_attendee) {
		for (i = 0 ; i < count_attendee; i++) {
			calendar_record_h attendee = NULL;
			calendar_record_get_child_record_at_p(record, _calendar_event.calendar_attendee, i, &attendee);
			g_variant_builder_add(&builder_attendee, "v", _attendee_to_gvariant(attendee));
		}
	} else {
		DBG("No attendee");
		g_variant_builder_add(&builder_attendee, "v", cal_dbus_utils_null_to_gvariant());
	}

	unsigned int count_exception = 0;
	GVariantBuilder builder_exception;
	g_variant_builder_init(&builder_exception, G_VARIANT_TYPE("av"));
	calendar_record_get_child_record_count(record, _calendar_event.exception, &count_exception);
	if (0 < count_exception) {
		for (i = 0 ; i < count_exception; i++) {
			calendar_record_h exception = NULL;
			calendar_record_get_child_record_at_p(record, _calendar_event.exception, i, &exception);
			g_variant_builder_add(&builder_exception, "v", _only_event_to_gvariant(exception));
		}
	} else {
		DBG("No exception");
		g_variant_builder_add(&builder_exception, "v", cal_dbus_utils_null_to_gvariant());
	}

	unsigned int count_extended = 0;
	GVariantBuilder builder_extended;
	g_variant_builder_init(&builder_extended, G_VARIANT_TYPE("av"));
	calendar_record_get_child_record_count(record, _calendar_event.extended, &count_extended);
	if (0 < count_extended) {
		for (i = 0 ; i < count_extended; i++) {
			calendar_record_h extended = NULL;
			calendar_record_get_child_record_at_p(record, _calendar_event.extended, i, &extended);
			g_variant_builder_add(&builder_extended, "v", _extended_to_gvariant(extended));
		}
	} else {
		DBG("No extended");
		g_variant_builder_add(&builder_extended, "v", cal_dbus_utils_null_to_gvariant());
	}

	GVariant *value = NULL;
	value = g_variant_new("(viaviaviaviav)", arg_only_event,
			count_alarm, &builder_alarm, count_attendee, &builder_attendee,
			count_exception, &builder_exception, count_extended, &builder_extended);

	return value;
}

static GVariant *_only_todo_to_gvariant(calendar_record_h record)
{
	GVariant *value = NULL;
	RETV_IF(NULL == record, NULL);

	cal_todo_s *p = (cal_todo_s *)record;
	GVariant *arg_until = _caltime_to_gvariant(&p->until);
	GVariant *arg_start = _caltime_to_gvariant(&p->start);
	GVariant *arg_due = _caltime_to_gvariant(&p->due);

	value = g_variant_new("(iissssiiisddxxiixiiiviisssssssssiiiissssivsivsssii)",
			p->index, p->calendar_id,
			CAL_DBUS_SET_STRING(p->summary),
			CAL_DBUS_SET_STRING(p->description),
			CAL_DBUS_SET_STRING(p->location),
			CAL_DBUS_SET_STRING(p->categories),
			p->todo_status, p->priority, p->sensitivity,
			CAL_DBUS_SET_STRING(p->uid),
			p->latitude, p->longitude, p->created_time, p->completed_time, p->progress,
			p->is_deleted, p->last_mod, p->freq, p->range_type, p->until.type,
			arg_until, p->count, p->interval,
			CAL_DBUS_SET_STRING(p->bysecond),
			CAL_DBUS_SET_STRING(p->byminute),
			CAL_DBUS_SET_STRING(p->byhour),
			CAL_DBUS_SET_STRING(p->byday),
			CAL_DBUS_SET_STRING(p->bymonthday),
			CAL_DBUS_SET_STRING(p->byyearday),
			CAL_DBUS_SET_STRING(p->byweekno),
			CAL_DBUS_SET_STRING(p->bymonth),
			CAL_DBUS_SET_STRING(p->bysetpos),
			p->wkst, p->has_alarm, p->system_type, p->updated,
			CAL_DBUS_SET_STRING(p->sync_data1),
			CAL_DBUS_SET_STRING(p->sync_data2),
			CAL_DBUS_SET_STRING(p->sync_data3),
			CAL_DBUS_SET_STRING(p->sync_data4),
			p->start.type, arg_start,
			CAL_DBUS_SET_STRING(p->start_tzid),
			p->due.type, arg_due,
			CAL_DBUS_SET_STRING(p->due_tzid),
			CAL_DBUS_SET_STRING(p->organizer_name),
			CAL_DBUS_SET_STRING(p->organizer_email),
			p->has_attendee, p->is_allday);

	return value;
}

static GVariant *_todo_to_gvariant(calendar_record_h record)
{
	int i;
	RETV_IF(NULL == record, NULL);

	GVariant *arg_only_todo = _only_todo_to_gvariant(record);
	if (NULL == arg_only_todo) {
		ERR("_only_todo_to_gvariant() Fail");
		return NULL;
	}

	unsigned int count_alarm = 0;
	GVariantBuilder builder_alarm;
	g_variant_builder_init(&builder_alarm, G_VARIANT_TYPE("av"));
	calendar_record_get_child_record_count(record, _calendar_todo.calendar_alarm, &count_alarm);
	if (0 < count_alarm) {
		for (i = 0 ; i < count_alarm; i++) {
			calendar_record_h alarm = NULL;
			calendar_record_get_child_record_at_p(record, _calendar_todo.calendar_alarm, i, &alarm);
			g_variant_builder_add(&builder_alarm, "v", _alarm_to_gvariant(alarm));
		}
	} else {
		DBG("No alarm");
		g_variant_builder_add(&builder_alarm, "v", cal_dbus_utils_null_to_gvariant());
	}

	unsigned int count_attendee = 0;
	GVariantBuilder builder_attendee;
	g_variant_builder_init(&builder_attendee, G_VARIANT_TYPE("av"));
	calendar_record_get_child_record_count(record, _calendar_todo.calendar_attendee, &count_attendee);
	if (0 < count_attendee) {
		for (i = 0 ; i < count_attendee; i++) {
			calendar_record_h attendee = NULL;
			calendar_record_get_child_record_at_p(record, _calendar_todo.calendar_attendee, i, &attendee);
			g_variant_builder_add(&builder_attendee, "v", _attendee_to_gvariant(attendee));
		}
	} else {
		DBG("No attendee");
		g_variant_builder_add(&builder_attendee, "v", cal_dbus_utils_null_to_gvariant());
	}

	unsigned int count_extended = 0;
	GVariantBuilder builder_extended;
	g_variant_builder_init(&builder_extended, G_VARIANT_TYPE("av"));
	calendar_record_get_child_record_count(record, _calendar_todo.extended, &count_extended);
	if (0 < count_extended) {
		for (i = 0 ; i < count_extended; i++) {
			calendar_record_h extended = NULL;
			calendar_record_get_child_record_at_p(record, _calendar_todo.extended, i, &extended);
			g_variant_builder_add(&builder_extended, "v", _extended_to_gvariant(extended));
		}
	} else {
		DBG("No extended");
		g_variant_builder_add(&builder_extended, "v", cal_dbus_utils_null_to_gvariant());
	}


	GVariant *value = NULL;
	value = g_variant_new("(viaviaviav)", arg_only_todo,
			count_alarm, &builder_alarm, count_attendee, &builder_attendee,
			count_extended, &builder_extended);

	return value;
}

static GVariant *_instance_normal_to_gvariant(calendar_record_h record)
{
	GVariant *value = NULL;
	cal_instance_normal_s *p = (cal_instance_normal_s *)record;

	GVariant *arg_start = _caltime_to_gvariant(&p->start);
	GVariant *arg_end = _caltime_to_gvariant(&p->end);

	value = g_variant_new("(iiivivsssiiiiiddiixs)",
			p->event_id, p->calendar_id, p->start.type, arg_start, p->end.type, arg_end,
			CAL_DBUS_SET_STRING(p->summary),
			CAL_DBUS_SET_STRING(p->description),
			CAL_DBUS_SET_STRING(p->location),
			p->busy_status, p->event_status, p->priority, p->sensitivity, p->has_rrule,
			p->latitude, p->longitude, p->has_alarm, p->original_event_id, p->last_mod,
			CAL_DBUS_SET_STRING(p->sync_data1));
	return value;
}

static GVariant *_instance_allday_to_gvariant(calendar_record_h record)
{
	GVariant *value = NULL;
	cal_instance_allday_s *p = (cal_instance_allday_s *)record;

	GVariant *arg_start = _caltime_to_gvariant(&p->start);
	GVariant *arg_end = _caltime_to_gvariant(&p->end);

	value = g_variant_new("(iiivivsssiiiiiddiixsi)",
			p->event_id, p->calendar_id, p->start.type, arg_start, p->end.type, arg_end,
			CAL_DBUS_SET_STRING(p->summary),
			CAL_DBUS_SET_STRING(p->description),
			CAL_DBUS_SET_STRING(p->location),
			p->busy_status, p->event_status, p->priority, p->sensitivity, p->has_rrule,
			p->latitude, p->longitude, p->has_alarm, p->original_event_id, p->last_mod,
			CAL_DBUS_SET_STRING(p->sync_data1),
			p->is_allday);
	return value;
}

static GVariant *_instance_normal_extended_to_gvariant(calendar_record_h record)
{
	GVariant *value = NULL;
	cal_instance_normal_extended_s *p = (cal_instance_normal_extended_s *)record;

	GVariant *arg_start = _caltime_to_gvariant(&p->start);
	GVariant *arg_end = _caltime_to_gvariant(&p->end);

	value = g_variant_new("(iiivivsssiiiiiddiixssissss)",
			p->event_id, p->calendar_id, p->start.type, arg_start, p->end.type, arg_end,
			CAL_DBUS_SET_STRING(p->summary),
			CAL_DBUS_SET_STRING(p->description),
			CAL_DBUS_SET_STRING(p->location),
			p->busy_status, p->event_status, p->priority, p->sensitivity, p->has_rrule,
			p->latitude, p->longitude, p->has_alarm, p->original_event_id, p->last_mod,
			CAL_DBUS_SET_STRING(p->organizer_name),
			CAL_DBUS_SET_STRING(p->categories),
			p->has_attendee,
			CAL_DBUS_SET_STRING(p->sync_data1),
			CAL_DBUS_SET_STRING(p->sync_data2),
			CAL_DBUS_SET_STRING(p->sync_data3),
			CAL_DBUS_SET_STRING(p->sync_data4));
	return value;
}

static GVariant *_instance_allday_extended_to_gvariant(calendar_record_h record)
{
	GVariant *value = NULL;
	cal_instance_allday_extended_s *p = (cal_instance_allday_extended_s *)record;

	GVariant *arg_start = _caltime_to_gvariant(&p->start);
	GVariant *arg_end = _caltime_to_gvariant(&p->end);

	value = g_variant_new("(iiivivsssiiiiiddiixssissss)",
			p->event_id, p->calendar_id, p->start.type, arg_start, p->end.type, arg_end,
			CAL_DBUS_SET_STRING(p->summary),
			CAL_DBUS_SET_STRING(p->description),
			CAL_DBUS_SET_STRING(p->location),
			p->busy_status, p->event_status,
			p->priority, p->sensitivity, p->has_rrule, p->latitude, p->longitude,
			p->has_alarm, p->original_event_id, p->last_mod,
			CAL_DBUS_SET_STRING(p->organizer_name),
			CAL_DBUS_SET_STRING(p->categories),
			p->has_attendee,
			CAL_DBUS_SET_STRING(p->sync_data1),
			CAL_DBUS_SET_STRING(p->sync_data2),
			CAL_DBUS_SET_STRING(p->sync_data3),
			CAL_DBUS_SET_STRING(p->sync_data4));
	return value;
}

static GVariant *_updated_info_to_gvariant(calendar_record_h record)
{
	GVariant *value = NULL;
	cal_updated_info_s *p = (cal_updated_info_s *)record;
	value = g_variant_new("(iiii)", p->type, p->id, p->calendar_id, p->version);
	return value;
}

static GVariant *_value_to_gvariant(cal_search_value_s *p)
{
	GVariant *value = NULL;

	switch (p->property_id) {
	case CAL_PROPERTY_DATA_TYPE_STR:
		value = g_variant_new("s", CAL_DBUS_SET_STRING(p->value.s));
		break;
	case CAL_PROPERTY_DATA_TYPE_INT:
		value = g_variant_new("i", p->value.i);
		break;
	case CAL_PROPERTY_DATA_TYPE_DOUBLE:
		value = g_variant_new("d", p->value.d);
		break;
	case CAL_PROPERTY_DATA_TYPE_LLI:
		value = g_variant_new("x", p->value.lli);
		break;
	case CAL_PROPERTY_DATA_TYPE_CALTIME:
		value = _caltime_to_gvariant(&p->value.caltime);
		break;
	default:
		ERR("Invalid parameter(0x%x)", p->property_id);
		break;
	}
	return value;
}

static GVariant *_search_to_gvariant(calendar_record_h record)
{
	GVariantBuilder builder;

	cal_search_s *p = (cal_search_s *)record;
	if (p->values) {
		g_variant_builder_init(&builder, (G_VARIANT_TYPE("ia(iv)")));

		int count = g_slist_length(p->values);
		g_variant_builder_add(&builder, "i", count);

		GSList *cursor = p->values;
		while (cursor) {
			cal_search_value_s * d = (cal_search_value_s *)(cursor->data);
			GVariant *arg_value = _value_to_gvariant(d);
			g_variant_builder_add(&builder, "iv", d->property_id, arg_value);
			cursor = g_slist_next(cursor);
		}
	} else {
		g_variant_builder_init(&builder, (G_VARIANT_TYPE("i")));
		g_variant_builder_add(&builder, "i", 0);
	}

	return g_variant_builder_end(&builder);
}

GVariant *cal_dbus_utils_record_to_gvariant(calendar_record_h record)
{
	int type = 0;
	GVariant *value = NULL;
	GVariant *arg_common = NULL;
	GVariant *arg_record = NULL;

	if (NULL == record) {
		ERR("record is NULL");
		arg_common = cal_dbus_utils_null_to_gvariant();
		type = -1;
	} else {
		cal_record_s *rec = (cal_record_s *)record;
		arg_common = cal_dbus_utils_common_to_gvariant(rec);
		type = rec->type;
	}

	switch (type) {
	case CAL_RECORD_TYPE_CALENDAR:
		arg_record = _book_to_gvariant(record);
		break;
	case CAL_RECORD_TYPE_EVENT:
		arg_record = _event_to_gvariant(record);
		break;
	case CAL_RECORD_TYPE_TODO:
		arg_record = _todo_to_gvariant(record);
		break;
	case CAL_RECORD_TYPE_ALARM:
		arg_record = _alarm_to_gvariant(record);
		break;
	case CAL_RECORD_TYPE_ATTENDEE:
		arg_record = _attendee_to_gvariant(record);
		break;
	case CAL_RECORD_TYPE_TIMEZONE:
		arg_record = _timezone_to_gvariant(record);
		break;
	case CAL_RECORD_TYPE_INSTANCE_NORMAL:
		arg_record = _instance_normal_to_gvariant(record);
		break;
	case CAL_RECORD_TYPE_INSTANCE_ALLDAY:
		arg_record = _instance_allday_to_gvariant(record);
		break;
	case CAL_RECORD_TYPE_INSTANCE_NORMAL_EXTENDED:
		arg_record = _instance_normal_extended_to_gvariant(record);
		break;
	case CAL_RECORD_TYPE_INSTANCE_ALLDAY_EXTENDED:
		arg_record = _instance_allday_extended_to_gvariant(record);
		break;
	case CAL_RECORD_TYPE_UPDATED_INFO:
		arg_record = _updated_info_to_gvariant(record);
		break;
	case CAL_RECORD_TYPE_SEARCH:
		arg_record = _search_to_gvariant(record);
		break;
	case CAL_RECORD_TYPE_EXTENDED:
		arg_record = _extended_to_gvariant(record);
		break;
	default:
		ERR("Invalid type(%d)", type);
		arg_record = cal_dbus_utils_null_to_gvariant();
		break;
	}
	value = g_variant_new("(ivv)", type, arg_common, arg_record);
	return value;
}

GVariant *cal_dbus_utils_list_to_gvariant(calendar_list_h list)
{
	int has_list = 0;
	GVariant *value = NULL;
	GVariant *arg_list = NULL;

	if (NULL == list) {
		ERR("list is NULL");
		has_list = 0;
		arg_list = cal_dbus_utils_null_to_gvariant();
		return g_variant_new("(iv)", has_list, arg_list);
	}

	has_list = 1;
	GVariantBuilder builder;
	g_variant_builder_init(&builder, G_VARIANT_TYPE("av"));

	int i;
	int count = 0;
	calendar_list_get_count(list, &count);

	calendar_list_first(list);
	for (i = 0; i < count; i++) {
		calendar_record_h record = NULL;
		calendar_list_get_current_record_p(list, &record);
		GVariant *arg_record = cal_dbus_utils_record_to_gvariant(record);
		g_variant_builder_add(&builder, "v", arg_record);
		calendar_list_next(list);
	}
	arg_list = g_variant_builder_end(&builder);

	value = g_variant_new("(iv)", has_list, arg_list);
	return value;
}

int cal_dbus_utils_gvariant_to_handle(GVariant *arg_handle, calendar_h *out_handle)
{
	RETV_IF(NULL == arg_handle, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_handle, CALENDAR_ERROR_INVALID_PARAMETER);

	calendar_h handle = NULL;
	cal_handle_create(&handle);
	if (NULL == handle) {
		ERR("handle is NULL");
		return CALENDAR_ERROR_OUT_OF_MEMORY;
	}
	cal_s *p = (cal_s *)handle;

	g_variant_get(arg_handle, "(ii)",
			&p->version, &p->connection_count);
	*out_handle = handle;
	return CALENDAR_ERROR_NONE;
}

static int _gvariant_to_book(GVariant *arg_record, calendar_record_h *out_record)
{
	RETV_IF(NULL == arg_record, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_record, CALENDAR_ERROR_INVALID_PARAMETER);

	calendar_record_h record = NULL;
	calendar_record_create(_calendar_book._uri, &record);
	cal_book_s *p = (cal_book_s *)record;

	g_variant_get(arg_record, "(ii&si&s&s&s&siiii&s&s&s&si)",
			&p->index, &p->store_type, &p->uid, &p->updated, &p->name, &p->description,
			&p->color, &p->location, &p->visibility, &p->sync_event, &p->is_deleted,
			&p->account_id, &p->sync_data1, &p->sync_data2, &p->sync_data3,
			&p->sync_data4, &p->mode);
	CAL_DBUS_GET_STRING(p->uid);
	CAL_DBUS_GET_STRING(p->name);
	CAL_DBUS_GET_STRING(p->description);
	CAL_DBUS_GET_STRING(p->color);
	CAL_DBUS_GET_STRING(p->location);
	CAL_DBUS_GET_STRING(p->sync_data1);
	CAL_DBUS_GET_STRING(p->sync_data2);
	CAL_DBUS_GET_STRING(p->sync_data3);
	CAL_DBUS_GET_STRING(p->sync_data4);

	*out_record = record;
	return CALENDAR_ERROR_NONE;
}

static int _gvariant_to_alarm(GVariant * arg_record, calendar_record_h *out_record)
{
	RETV_IF(NULL == arg_record, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_record, CALENDAR_ERROR_INVALID_PARAMETER);

	calendar_record_h record = NULL;
	calendar_record_create(_calendar_alarm._uri, &record);
	GVariant *arg_alarm = NULL;
	cal_alarm_s *p = (cal_alarm_s *)record;
	g_variant_get(arg_record, "(iiiii&s&si&siv)",
			&p->id, &p->parent_id, &p->is_deleted, &p->remind_tick, &p->remind_tick_unit,
			&p->alarm_description, &p->alarm_summary, &p->alarm_action, &p->alarm_attach,
			&p->alarm.type, &arg_alarm);
	_gvariant_to_caltime(p->alarm.type, arg_alarm, &p->alarm);
	CAL_DBUS_GET_STRING(p->alarm_description);
	CAL_DBUS_GET_STRING(p->alarm_summary);
	CAL_DBUS_GET_STRING(p->alarm_attach);

	*out_record = record;
	return CALENDAR_ERROR_NONE;
}

static int _gvariant_to_attendee(GVariant * arg_record, calendar_record_h *out_record)
{
	RETV_IF(NULL == arg_record, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_record, CALENDAR_ERROR_INVALID_PARAMETER);

	calendar_record_h record = NULL;
	calendar_record_create(_calendar_attendee._uri, &record);
	cal_attendee_s *p = (cal_attendee_s *)record;
	g_variant_get(arg_record, "(ii&sii&s&s&siii&s&s&s&s)",
			&p->id, &p->parent_id, &p->attendee_number, &p->attendee_cutype,
			&p->attendee_ct_index, &p->attendee_uid, &p->attendee_group,
			&p->attendee_email, &p->attendee_role, &p->attendee_status,
			&p->attendee_rsvp, &p->attendee_delegatee_uri, &p->attendee_delegator_uri,
			&p->attendee_name, &p->attendee_member);
	CAL_DBUS_GET_STRING(p->attendee_number);
	CAL_DBUS_GET_STRING(p->attendee_uid);
	CAL_DBUS_GET_STRING(p->attendee_group);
	CAL_DBUS_GET_STRING(p->attendee_email);
	CAL_DBUS_GET_STRING(p->attendee_delegatee_uri);
	CAL_DBUS_GET_STRING(p->attendee_delegator_uri);
	CAL_DBUS_GET_STRING(p->attendee_name);
	CAL_DBUS_GET_STRING(p->attendee_member);

	*out_record = record;
	return CALENDAR_ERROR_NONE;
}

static int _gvariant_to_timezone(GVariant * arg_record, calendar_record_h *out_record)
{
	RETV_IF(NULL == arg_record, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_record, CALENDAR_ERROR_INVALID_PARAMETER);

	calendar_record_h record = NULL;
	calendar_record_create(_calendar_timezone._uri, &record);
	cal_timezone_s *p = (cal_timezone_s *)record;
	g_variant_get(arg_record, "(ii&siiiii&siiiiii)",
			&p->index, &p->tz_offset_from_gmt, &p->standard_name, &p->std_start_month,
			&p->std_start_position_of_week, &p->std_start_day, &p->std_start_hour,
			&p->standard_bias, &p->day_light_name, &p->day_light_start_month,
			&p->day_light_start_position_of_week, &p->day_light_start_day,
			&p->day_light_start_hour, &p->day_light_bias, &p->calendar_id);
	CAL_DBUS_GET_STRING(p->standard_name);
	CAL_DBUS_GET_STRING(p->day_light_name);

	*out_record = record;
	return CALENDAR_ERROR_NONE;
}

static int _gvariant_to_only_event(GVariant *arg_record, calendar_record_h *out_record)
{
	calendar_record_h record = NULL;
	calendar_record_create(_calendar_event._uri, &record);
	cal_event_s *p = (cal_event_s *)record;

	GVariant *arg_until = NULL;
	GVariant *arg_start = NULL;
	GVariant *arg_end = NULL;

	g_variant_get(arg_record, "(ii&s&s&s&s&siiiiiii&s&s&siddixixiiivii"
			"&s&s&s&s&s&s&s&s&si&s&siiii&s&s&s&siv&siv&si)",
			&p->index, &p->calendar_id, &p->summary, &p->description, &p->location,
			&p->categories, &p->exdate, &p->event_status, &p->priority, &p->timezone,
			&p->contact_id, &p->busy_status, &p->sensitivity, &p->meeting_status,
			&p->uid, &p->organizer_name, &p->organizer_email, &p->original_event_id,
			&p->latitude, &p->longitude, &p->email_id, &p->created_time, &p->is_deleted,
			&p->last_mod, &p->freq, &p->range_type, &p->until.type, &arg_until,
			&p->count, &p->interval, &p->bysecond, &p->byminute, &p->byhour, &p->byday,
			&p->bymonthday, &p->byyearday, &p->byweekno, &p->bymonth, &p->bysetpos,
			&p->wkst, &p->recurrence_id, &p->rdate, &p->has_attendee, &p->has_alarm,
			&p->system_type, &p->updated, &p->sync_data1, &p->sync_data2, &p->sync_data3,
			&p->sync_data4, &p->start.type, &arg_start, &p->start_tzid, &p->end.type,
			&arg_end, &p->end_tzid, &p->is_allday);
	_gvariant_to_caltime(p->until.type, arg_until, &p->until);
	_gvariant_to_caltime(p->start.type, arg_start, &p->start);
	_gvariant_to_caltime(p->end.type, arg_end, &p->end);
	CAL_DBUS_GET_STRING(p->summary);
	CAL_DBUS_GET_STRING(p->description);
	CAL_DBUS_GET_STRING(p->location);
	CAL_DBUS_GET_STRING(p->categories);
	CAL_DBUS_GET_STRING(p->exdate);
	CAL_DBUS_GET_STRING(p->uid);
	CAL_DBUS_GET_STRING(p->organizer_name);
	CAL_DBUS_GET_STRING(p->organizer_email);
	CAL_DBUS_GET_STRING(p->bysecond);
	CAL_DBUS_GET_STRING(p->byminute);
	CAL_DBUS_GET_STRING(p->byhour);
	CAL_DBUS_GET_STRING(p->byday);
	CAL_DBUS_GET_STRING(p->bymonthday);
	CAL_DBUS_GET_STRING(p->byyearday);
	CAL_DBUS_GET_STRING(p->byweekno);
	CAL_DBUS_GET_STRING(p->bymonth);
	CAL_DBUS_GET_STRING(p->bysetpos);
	CAL_DBUS_GET_STRING(p->recurrence_id);
	CAL_DBUS_GET_STRING(p->rdate);
	CAL_DBUS_GET_STRING(p->sync_data1);
	CAL_DBUS_GET_STRING(p->sync_data2);
	CAL_DBUS_GET_STRING(p->sync_data3);
	CAL_DBUS_GET_STRING(p->sync_data4);
	CAL_DBUS_GET_STRING(p->start_tzid);
	CAL_DBUS_GET_STRING(p->end_tzid);

	*out_record = record;
	return CALENDAR_ERROR_NONE;
}

static int _gvariant_to_value(GVariant *arg_value, cal_search_value_s *p)
{
	switch (p->property_id) {
	case CAL_PROPERTY_DATA_TYPE_STR:
		g_variant_get(arg_value, "&s", &p->value.s);
		break;
	case CAL_PROPERTY_DATA_TYPE_INT:
		g_variant_get(arg_value, "i", &p->value.i);
		break;
	case CAL_PROPERTY_DATA_TYPE_DOUBLE:
		g_variant_get(arg_value, "d", &p->value.d);
		break;
	case CAL_PROPERTY_DATA_TYPE_LLI:
		g_variant_get(arg_value, "x", &p->value.lli);
		break;
	case CAL_PROPERTY_DATA_TYPE_CALTIME:
		g_variant_get(arg_value, "(ixiiiiiii)",
			&p->value.caltime.type, &p->value.caltime.time.utime,
			&p->value.caltime.time.date.year, &p->value.caltime.time.date.month,
			&p->value.caltime.time.date.mday, &p->value.caltime.time.date.hour,
			&p->value.caltime.time.date.minute, &p->value.caltime.time.date.second);
		break;
	default:
		ERR("Invalid parameter(0x%x)", p->property_id);
		break;
	}
	return CALENDAR_ERROR_NONE;
}

static int _gvariant_to_search(GVariant *arg_record, calendar_record_h *out_record)
{
	RETV_IF(NULL == arg_record, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_record, CALENDAR_ERROR_INVALID_PARAMETER);

	int count = 0;
	GVariantIter *iter_value = NULL;
	g_variant_get(arg_record, "ia(iv)", &count, &iter_value);
	int value_count = g_variant_iter_n_children(iter_value);
	if (0 != value_count) {
		gboolean is_continue = FALSE;
		do {
			cal_search_value_s p = {0};
			GVariant *arg_value = NULL;
			is_continue = g_variant_iter_loop(iter_value, "(iv)",
					&p.property_id, arg_value);
			_gvariant_to_value(arg_value, &p);
		} while (TRUE == is_continue);
		g_variant_iter_free(iter_value);
	}
	return CALENDAR_ERROR_NONE;
}

static int _gvariant_to_extended(GVariant * arg_record, calendar_record_h *out_record)
{
	RETV_IF(NULL == arg_record, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_record, CALENDAR_ERROR_INVALID_PARAMETER);

	calendar_record_h record = NULL;
	calendar_record_create(_calendar_extended_property._uri, &record);
	cal_extended_s *p = (cal_extended_s *)record;
	g_variant_get(arg_record, "(iii&s&s)",
			&p->id, &p->record_id, &p->record_type, &p->key, &p->value);
	CAL_DBUS_GET_STRING(p->key);
	CAL_DBUS_GET_STRING(p->value);

	*out_record = record;
	return CALENDAR_ERROR_NONE;
}

static int _gvariant_to_attr_value(int type, GVariant *arg_value, cal_attribute_filter_s *p)
{
	GVariant *arg_caltime = NULL;
	switch (type) {
	case CAL_FILTER_STR:
		g_variant_get(arg_value, "&s", &p->value.s);
		break;
	case CAL_FILTER_INT:
		g_variant_get(arg_value, "i", &p->value.i);
		break;
	case CAL_FILTER_DOUBLE:
		g_variant_get(arg_value, "d", &p->value.d);
		break;
	case CAL_FILTER_LLI:
		g_variant_get(arg_value, "x", &p->value.lli);
		break;
	case CAL_FILTER_CALTIME:
		g_variant_get(arg_value, "(iv)", &p->value.caltime.type, &arg_caltime);
		_gvariant_to_caltime(p->value.caltime.type, arg_caltime, &p->value.caltime);
		break;
	default:
		break;
	}
	return CALENDAR_ERROR_NONE;
}

static int _gvariant_to_composite(GVariant * arg_composite_pack, cal_composite_filter_s **out_composite)
{
	GVariantIter *iter_composite_pack = NULL;
	g_variant_get(arg_composite_pack, "a(isiv)", &iter_composite_pack);

	int filter_type = 0;
	char *view_uri = NULL;
	int count_composite = 0;
	GVariant *arg_composite = NULL;

	int composite_filter_type = 0;
	int property_id = 0;
	int match = 0;
	GVariant *arg_attr_value = NULL;
	GVariantIter *iter_composite = NULL;

	cal_composite_filter_s *composite = NULL;
	while (g_variant_iter_loop(iter_composite_pack, "(i&siv)",
				&filter_type, &view_uri, &count_composite, &arg_composite)) {

		cal_composite_filter_s *cf = calloc(1, sizeof(cal_composite_filter_s));
		if (NULL == cf) {
			ERR("calloc() Fail");
			break;
		}
		cf->filter_type = CAL_FILTER_COMPOSITE;
		cf->view_uri = cal_strdup(view_uri);
		cf->properties = (cal_property_info_s *)cal_view_get_property_info(view_uri,
				&cf->property_count);

		int is_exit = 0;
		if (0 == count_composite) {
			DBG("composite count is 0");
			is_exit = 1;
		}
		g_variant_get(arg_composite, "a(iiiv)", &iter_composite);
		while (g_variant_iter_loop(iter_composite, "(iiiv)",
					&composite_filter_type, &property_id, &match, &arg_attr_value)) {
			cal_attribute_filter_s *filter = calloc(1, sizeof(cal_attribute_filter_s));
			if (NULL == filter) {
				ERR("calloc() Fail");
				break;
			}
			filter->filter_type = composite_filter_type;
			filter->property_id = property_id;
			filter->match = match;
			_gvariant_to_attr_value(composite_filter_type, arg_attr_value, filter);
			cf->filters = g_slist_append(cf->filters, filter);
		}

		if (1 == is_exit)
			break;

		if (NULL == composite)
			composite = cf;
		else
			composite->filters = g_slist_append(composite->filters, cf);

	}
	*out_composite = composite;
	return CALENDAR_ERROR_NONE;
}

static int _gvariant_to_operate(GVariant * arg_operate, cal_composite_filter_s *f)
{
	GVariantIter *iter_operate = NULL;
	g_variant_get(arg_operate, "ai", &iter_operate);

	int operate = 0;
	while (g_variant_iter_loop(iter_operate, "i", &operate))
		f->filter_ops = g_slist_append(f->filter_ops, GINT_TO_POINTER(operate));

	return CALENDAR_ERROR_NONE;
}

static int _gvariant_to_filter(GVariant *arg_filter, cal_query_s *q)
{
	GVariant *arg_composite_pack = NULL;
	int count_operate = 0;
	GVariant *arg_operate = NULL;
	g_variant_get(arg_filter, "(viv)", &arg_composite_pack, &count_operate, &arg_operate);

	_gvariant_to_composite(arg_composite_pack, &q->filter);

	if (count_operate)
		_gvariant_to_operate(arg_operate, q->filter);

	return CALENDAR_ERROR_NONE;
}

static int _gvariant_to_projection(int count_projection, GVariant *arg_projection, cal_query_s *q)
{
	GVariantIter *iter_projection = NULL;
	g_variant_get(arg_projection, "au", &iter_projection);

	int i = 0;
	q->projection = calloc(count_projection, sizeof(unsigned int));
	if (NULL == q->projection) {
		ERR("calloc() Fail");
		return CALENDAR_ERROR_OUT_OF_MEMORY;
	}
	while (g_variant_iter_loop(iter_projection, "u", &q->projection[i]))
		i++;

	q->projection_count = count_projection;

	return CALENDAR_ERROR_NONE;
}

int cal_dbus_utils_gvariant_to_query(GVariant *arg_query, calendar_query_h *out_query)
{
	RETV_IF(NULL == arg_query, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_query, CALENDAR_ERROR_INVALID_PARAMETER);

	char *view_uri = NULL;
	int has_filter = 0;
	GVariant *arg_filter = NULL;
	int count_projection = 0;
	GVariant *arg_projection = NULL;
	int property_id = 0;
	int has_asc = 0;
	int has_distinct = 0;

	g_variant_get(arg_query, "(&siviviii)",
			&view_uri, &has_filter, &arg_filter, &count_projection, &arg_projection,
			&property_id, &has_asc, &has_distinct);

	calendar_query_h query = NULL;
	calendar_query_create(view_uri, &query);

	cal_query_s *q = (cal_query_s *)query;

	if (has_filter)
		_gvariant_to_filter(arg_filter, q);

	if (count_projection)
		_gvariant_to_projection(count_projection, arg_projection, q);

	q->sort_property_id = property_id;
	q->asc = has_asc;
	q->distinct = has_distinct;

	*out_query = query;

	return CALENDAR_ERROR_NONE;
}

int cal_dbus_utils_gvariant_to_ids(GVariant *arg_ids, int count, int **out_ids)
{
	GVariantIter *iter_ids = NULL;
	g_variant_get(arg_ids, "ai", &iter_ids);

	int *ids = calloc(count, sizeof(int));
	if (NULL == ids) {
		ERR("calloc() Fail");
		return CALENDAR_ERROR_OUT_OF_MEMORY;
	}
	int i = 0;
	for (i = 0; i < count; i++)
		g_variant_iter_loop(iter_ids, "i", &ids[i]);

	*out_ids = ids;
	return CALENDAR_ERROR_NONE;
}

static int _gvariant_to_event(GVariant * arg_record, calendar_record_h *out_record)
{
	RETV_IF(NULL == arg_record, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_record, CALENDAR_ERROR_INVALID_PARAMETER);

	GVariant *arg_only_event = NULL;
	int count_alarm = 0;
	GVariantIter *iter_alarm = NULL;
	int count_attendee = 0;
	GVariantIter *iter_attendee = NULL;
	int count_exception = 0;
	GVariantIter *iter_exception = NULL;
	int count_extended = 0;
	GVariantIter *iter_extended = NULL;
	g_variant_get(arg_record, "(viaviaviaviav)", &arg_only_event,
			&count_alarm, &iter_alarm, &count_attendee, &iter_attendee,
			&count_exception, &iter_exception, &count_extended, &iter_extended);

	calendar_record_h record = NULL;
	_gvariant_to_only_event(arg_only_event, &record);

	if (0 < count_alarm) {
		GVariant *arg_alarm = NULL;
		while (g_variant_iter_loop(iter_alarm, "v", &arg_alarm)) {
			calendar_record_h alarm = NULL;
			_gvariant_to_alarm(arg_alarm, &alarm);
			calendar_record_add_child_record(record, _calendar_event.calendar_alarm, alarm);
		}
	} else {
		DBG("No alarm");
	}

	if (0 < count_attendee) {
		GVariant *arg_attendee = NULL;
		while (g_variant_iter_loop(iter_attendee, "v", &arg_attendee)) {
			calendar_record_h attendee = NULL;
			_gvariant_to_attendee(arg_attendee, &attendee);
			calendar_record_add_child_record(record, _calendar_event.calendar_attendee, attendee);
		}
	} else {
		DBG("No attendee");
	}

	if (0 < count_exception) {
		GVariant *arg_exception = NULL;
		while (g_variant_iter_loop(iter_exception, "v", &arg_exception)) {
			calendar_record_h exception = NULL;
			_gvariant_to_only_event(arg_exception, &exception);
			calendar_record_add_child_record(record, _calendar_event.exception, exception);
		}
	} else {
		DBG("No exception");
	}

	if (0 < count_extended) {
		GVariant *arg_extended = NULL;
		while (g_variant_iter_loop(iter_extended, "v", &arg_extended)) {
			calendar_record_h extended = NULL;
			_gvariant_to_extended(arg_extended, &extended);
			calendar_record_add_child_record(record, _calendar_event.extended, extended);
		}
	} else {
		DBG("No extended");
	}

	*out_record = record;
	return CALENDAR_ERROR_NONE;
}

static int _gvariant_to_only_todo(GVariant *arg_record, calendar_record_h *out_record)
{
	calendar_record_h record = NULL;
	calendar_record_create(_calendar_todo._uri, &record);
	cal_todo_s *p = (cal_todo_s *)record;

	GVariant *arg_until = NULL;
	GVariant *arg_start = NULL;
	GVariant *arg_due = NULL;

	g_variant_get(arg_record, "(ii&s&s&s&siii&sddxxiixiiivii&s&s&s&s&s&s&s&s&si"
			"iii&s&s&s&siv&siv&s&s&sii)",
			&p->index, &p->calendar_id,
			&p->summary, &p->description, &p->location, &p->categories, &p->todo_status,
			&p->priority, &p->sensitivity, &p->uid, &p->latitude, &p->longitude,
			&p->created_time, &p->completed_time, &p->progress, &p->is_deleted,
			&p->last_mod, &p->freq, &p->range_type, &p->until.type, &arg_until,
			&p->count, &p->interval, &p->bysecond, &p->byminute, &p->byhour, &p->byday,
			&p->bymonthday, &p->byyearday, &p->byweekno, &p->bymonth, &p->bysetpos,
			&p->wkst, &p->has_alarm, &p->system_type, &p->updated, &p->sync_data1,
			&p->sync_data2, &p->sync_data3, &p->sync_data4, &p->start.type, &arg_start,
			&p->start_tzid, &p->due.type, &arg_due, &p->due_tzid, &p->organizer_name,
			&p->organizer_email, &p->has_attendee, &p->is_allday);
	_gvariant_to_caltime(p->until.type, arg_until, &p->until);
	_gvariant_to_caltime(p->start.type, arg_start, &p->start);
	_gvariant_to_caltime(p->due.type, arg_due, &p->due);
	CAL_DBUS_GET_STRING(p->summary);
	CAL_DBUS_GET_STRING(p->description);
	CAL_DBUS_GET_STRING(p->location);
	CAL_DBUS_GET_STRING(p->categories);
	CAL_DBUS_GET_STRING(p->uid);
	CAL_DBUS_GET_STRING(p->bysecond);
	CAL_DBUS_GET_STRING(p->byminute);
	CAL_DBUS_GET_STRING(p->byhour);
	CAL_DBUS_GET_STRING(p->byday);
	CAL_DBUS_GET_STRING(p->bymonthday);
	CAL_DBUS_GET_STRING(p->byyearday);
	CAL_DBUS_GET_STRING(p->byweekno);
	CAL_DBUS_GET_STRING(p->bymonth);
	CAL_DBUS_GET_STRING(p->bysetpos);
	CAL_DBUS_GET_STRING(p->sync_data1);
	CAL_DBUS_GET_STRING(p->sync_data2);
	CAL_DBUS_GET_STRING(p->sync_data3);
	CAL_DBUS_GET_STRING(p->sync_data4);
	CAL_DBUS_GET_STRING(p->start_tzid);
	CAL_DBUS_GET_STRING(p->due_tzid);
	CAL_DBUS_GET_STRING(p->organizer_name);
	CAL_DBUS_GET_STRING(p->organizer_email);

	*out_record = record;
	return CALENDAR_ERROR_NONE;
}

static int _gvariant_to_todo(GVariant * arg_record, calendar_record_h *out_record)
{
	RETV_IF(NULL == arg_record, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_record, CALENDAR_ERROR_INVALID_PARAMETER);

	GVariant *arg_only_todo = NULL;
	int count_alarm = 0;
	GVariantIter *iter_alarm = NULL;
	int count_attendee = 0;
	GVariantIter *iter_attendee = NULL;
	int count_extended = 0;
	GVariantIter *iter_extended = NULL;
	g_variant_get(arg_record, "(viaviaviav)", &arg_only_todo,
			&count_alarm, &iter_alarm, &count_attendee, &iter_attendee,
			&count_extended, &iter_extended);

	calendar_record_h record = NULL;
	_gvariant_to_only_todo(arg_only_todo, &record);

	if (0 < count_alarm) {
		GVariant *arg_alarm = NULL;
		while (g_variant_iter_loop(iter_alarm, "v", &arg_alarm)) {
			calendar_record_h alarm = NULL;
			_gvariant_to_alarm(arg_alarm, &alarm);
			calendar_record_add_child_record(record, _calendar_todo.calendar_alarm, alarm);
		}
	} else {
		DBG("No alarm");
	}

	if (0 < count_attendee) {
		GVariant *arg_attendee = NULL;
		while (g_variant_iter_loop(iter_attendee, "v", &arg_attendee)) {
			calendar_record_h attendee = NULL;
			_gvariant_to_attendee(arg_attendee, &attendee);
			calendar_record_add_child_record(record, _calendar_todo.calendar_attendee, attendee);
		}
	} else {
		DBG("No attendee");
	}

	if (0 < count_extended) {
		GVariant *arg_extended = NULL;
		while (g_variant_iter_loop(iter_extended, "v", &arg_extended)) {
			calendar_record_h extended = NULL;
			_gvariant_to_extended(arg_extended, &extended);
			calendar_record_add_child_record(record, _calendar_todo.extended, extended);
		}
	} else {
		DBG("No extended");
	}

	*out_record = record;
	return CALENDAR_ERROR_NONE;

	return CALENDAR_ERROR_NONE;
}


static int _gvariant_to_instance_normal(GVariant * arg_record, calendar_record_h *out_record)
{
	RETV_IF(NULL == arg_record, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_record, CALENDAR_ERROR_INVALID_PARAMETER);

	calendar_record_h record = NULL;
	calendar_record_create(_calendar_instance_utime_calendar_book._uri, &record);
	GVariant *arg_start = NULL;
	GVariant *arg_end = NULL;
	cal_instance_normal_s *p = (cal_instance_normal_s *)record;
	g_variant_get(arg_record, "(iiiviv&s&s&siiiiiddiix&s)",
			&p->event_id, &p->calendar_id, &p->start.type, &arg_start, &p->end.type,
			&arg_end, &p->summary, &p->description, &p->location, &p->busy_status,
			&p->event_status, &p->priority, &p->sensitivity, &p->has_rrule, &p->latitude,
			&p->longitude, &p->has_alarm, &p->original_event_id, &p->last_mod,
			&p->sync_data1);
	_gvariant_to_caltime(p->start.type, arg_start, &p->start);
	_gvariant_to_caltime(p->end.type, arg_end, &p->end);
	CAL_DBUS_GET_STRING(p->summary);
	CAL_DBUS_GET_STRING(p->description);
	CAL_DBUS_GET_STRING(p->location);
	CAL_DBUS_GET_STRING(p->sync_data1);

	*out_record = record;
	return CALENDAR_ERROR_NONE;
}

static int _gvariant_to_instance_allday(GVariant * arg_record, calendar_record_h *out_record)
{
	RETV_IF(NULL == arg_record, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_record, CALENDAR_ERROR_INVALID_PARAMETER);

	calendar_record_h record = NULL;
	calendar_record_create(_calendar_instance_localtime_calendar_book._uri, &record);
	GVariant *arg_start = NULL;
	GVariant *arg_end = NULL;
	cal_instance_allday_s *p = (cal_instance_allday_s *)record;
	g_variant_get(arg_record, "(iiiviv&s&s&siiiiiddiix&si)",
			&p->event_id, &p->calendar_id, &p->start.type, &arg_start, &p->end.type,
			&arg_end, &p->summary, &p->description, &p->location, &p->busy_status,
			&p->event_status, &p->priority, &p->sensitivity, &p->has_rrule, &p->latitude,
			&p->longitude, &p->has_alarm, &p->original_event_id, &p->last_mod,
			&p->sync_data1, &p->is_allday);
	_gvariant_to_caltime(p->start.type, arg_start, &p->start);
	_gvariant_to_caltime(p->end.type, arg_end, &p->end);
	CAL_DBUS_GET_STRING(p->summary);
	CAL_DBUS_GET_STRING(p->description);
	CAL_DBUS_GET_STRING(p->location);
	CAL_DBUS_GET_STRING(p->sync_data1);

	*out_record = record;
	return CALENDAR_ERROR_NONE;
}

static int _gvariant_to_instance_normal_extended(GVariant * arg_record, calendar_record_h *out_record)
{
	RETV_IF(NULL == arg_record, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_record, CALENDAR_ERROR_INVALID_PARAMETER);

	calendar_record_h record = NULL;
	calendar_record_create(_calendar_instance_utime_calendar_book_extended._uri, &record);
	GVariant *arg_start = NULL;
	GVariant *arg_end = NULL;
	cal_instance_normal_extended_s *p = (cal_instance_normal_extended_s *)record;
	g_variant_get(arg_record, "(iiiviv&s&s&siiiiiddiix&s&si&s&s&s&s)",
			&p->event_id, &p->calendar_id, &p->start.type, &arg_start, &p->end.type,
			&arg_end, &p->summary, &p->description, &p->location, &p->busy_status,
			&p->event_status, &p->priority, &p->sensitivity, &p->has_rrule, &p->latitude,
			&p->longitude, &p->has_alarm, &p->original_event_id, &p->last_mod,
			&p->organizer_name, &p->categories, &p->has_attendee, &p->sync_data1,
			&p->sync_data2, &p->sync_data3, &p->sync_data4);
	_gvariant_to_caltime(p->start.type, arg_start, &p->start);
	_gvariant_to_caltime(p->end.type, arg_end, &p->end);
	CAL_DBUS_GET_STRING(p->summary);
	CAL_DBUS_GET_STRING(p->description);
	CAL_DBUS_GET_STRING(p->location);
	CAL_DBUS_GET_STRING(p->organizer_name);
	CAL_DBUS_GET_STRING(p->categories);
	CAL_DBUS_GET_STRING(p->sync_data1);
	CAL_DBUS_GET_STRING(p->sync_data2);
	CAL_DBUS_GET_STRING(p->sync_data3);
	CAL_DBUS_GET_STRING(p->sync_data4);

	*out_record = record;
	return CALENDAR_ERROR_NONE;
}

static int _gvariant_to_instance_allday_extended(GVariant * arg_record, calendar_record_h *out_record)
{
	RETV_IF(NULL == arg_record, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_record, CALENDAR_ERROR_INVALID_PARAMETER);

	calendar_record_h record = NULL;
	calendar_record_create(_calendar_instance_localtime_calendar_book_extended._uri, &record);
	GVariant *arg_start = NULL;
	GVariant *arg_end = NULL;
	cal_instance_allday_extended_s *p = (cal_instance_allday_extended_s *)record;
	g_variant_get(arg_record, "(iiiviv&s&s&siiiiiddiix&s&si&s&s&s&s)",
			&p->event_id, &p->calendar_id, &p->start.type, &arg_start, &p->end.type,
			&arg_end, &p->summary, &p->description, &p->location, &p->busy_status,
			&p->event_status, &p->priority, &p->sensitivity, &p->has_rrule, &p->latitude,
			&p->longitude, &p->has_alarm, &p->original_event_id, &p->last_mod,
			&p->organizer_name, &p->categories, &p->has_attendee, &p->sync_data1,
			&p->sync_data2, &p->sync_data3, &p->sync_data4);
	_gvariant_to_caltime(p->start.type, arg_start, &p->start);
	_gvariant_to_caltime(p->end.type, arg_end, &p->end);
	CAL_DBUS_GET_STRING(p->summary);
	CAL_DBUS_GET_STRING(p->description);
	CAL_DBUS_GET_STRING(p->location);
	CAL_DBUS_GET_STRING(p->organizer_name);
	CAL_DBUS_GET_STRING(p->categories);
	CAL_DBUS_GET_STRING(p->sync_data1);
	CAL_DBUS_GET_STRING(p->sync_data2);
	CAL_DBUS_GET_STRING(p->sync_data3);
	CAL_DBUS_GET_STRING(p->sync_data4);

	*out_record = record;
	return CALENDAR_ERROR_NONE;
}

static int _gvariant_to_updated_info(GVariant * arg_record, calendar_record_h *out_record)
{
	RETV_IF(NULL == arg_record, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_record, CALENDAR_ERROR_INVALID_PARAMETER);

	calendar_record_h record = NULL;
	calendar_record_create(_calendar_updated_info._uri, &record);
	cal_updated_info_s *p = (cal_updated_info_s *)record;
	g_variant_get(arg_record, "(iiii)", &p->type, &p->id, &p->calendar_id, &p->version);
	*out_record = record;
	return CALENDAR_ERROR_NONE;
}

static int _gvariant_to_record(int type, GVariant *arg_common, GVariant *arg_record, calendar_record_h *out_record)
{
	RETV_IF(NULL == arg_record, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_record, CALENDAR_ERROR_INVALID_PARAMETER);

	calendar_record_h record = NULL;
	switch (type) {
	case CAL_RECORD_TYPE_CALENDAR:
		_gvariant_to_book(arg_record, &record);
		break;
	case CAL_RECORD_TYPE_EVENT:
		_gvariant_to_event(arg_record, &record);
		break;
	case CAL_RECORD_TYPE_TODO:
		_gvariant_to_todo(arg_record, &record);
		break;
	case CAL_RECORD_TYPE_ALARM:
		_gvariant_to_alarm(arg_record, &record);
		break;
	case CAL_RECORD_TYPE_ATTENDEE:
		_gvariant_to_attendee(arg_record, &record);
		break;
	case CAL_RECORD_TYPE_TIMEZONE:
		_gvariant_to_timezone(arg_record, &record);
		break;
	case CAL_RECORD_TYPE_INSTANCE_NORMAL:
		_gvariant_to_instance_normal(arg_record, &record);
		break;
	case CAL_RECORD_TYPE_INSTANCE_ALLDAY:
		_gvariant_to_instance_allday(arg_record, &record);
		break;
	case CAL_RECORD_TYPE_INSTANCE_NORMAL_EXTENDED:
		_gvariant_to_instance_normal_extended(arg_record, &record);
		break;
	case CAL_RECORD_TYPE_INSTANCE_ALLDAY_EXTENDED:
		_gvariant_to_instance_allday_extended(arg_record, &record);
		break;
	case CAL_RECORD_TYPE_UPDATED_INFO:
		_gvariant_to_updated_info(arg_record, &record);
		break;
	case CAL_RECORD_TYPE_SEARCH:
		_gvariant_to_search(arg_record, &record);
		break;
	case CAL_RECORD_TYPE_EXTENDED:
		_gvariant_to_extended(arg_record, &record);
		break;
	default:
		ERR("Invalid type(%d)", type);
		break;
	}
	if (-1 != type)
		cal_dbus_utils_gvariant_to_common(arg_common, &record);
	*out_record = record;
	return CALENDAR_ERROR_NONE;
}

int cal_dbus_utils_gvariant_to_record(GVariant *arg_record_pack, calendar_record_h *out_record)
{
	int type = 0;
	GVariant *arg_common = NULL;
	GVariant *arg_record = NULL;
	g_variant_get(arg_record_pack, "(ivv)", &type, &arg_common, &arg_record);
	_gvariant_to_record(type, arg_common, arg_record, out_record);
	return CALENDAR_ERROR_NONE;
}

int cal_dbus_utils_gvariant_to_list(GVariant *arg_list_pack, calendar_list_h *out_list)
{
	int has_list = 0;
	GVariant *arg_list = NULL;
	g_variant_get(arg_list_pack, "(iv)", &has_list, &arg_list);

	calendar_list_h l = NULL;
	calendar_list_create(&l);

	if (1 == has_list) {
		GVariantIter *iter_value = NULL;
		g_variant_get(arg_list, "av", &iter_value);

		GVariant *arg_record = NULL;
		while (g_variant_iter_loop(iter_value, "v", &arg_record)) {
			calendar_record_h record = NULL;
			cal_dbus_utils_gvariant_to_record(arg_record, &record);
			calendar_list_add(l, record);
		}
	}
	*out_list = l;

	return CALENDAR_ERROR_NONE;
}

GVariant *cal_dbus_utils_stream_to_gvariant(int stream_size, char *stream)
{
	return g_variant_new("(is)", stream_size, CAL_DBUS_SET_STRING(stream));
}

int cal_dbus_utils_gvariant_to_stream(GVariant *arg_stream, int *out_size, char **out_stream)
{
	int size = 0;
	char *stream = NULL;

	RETV_IF(NULL == out_size, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_stream, CALENDAR_ERROR_INVALID_PARAMETER);

	g_variant_get(arg_stream, "(i&s)", &size, &stream);
	CAL_DBUS_GET_STRING(stream);
	*out_size = size;
	*out_stream = cal_strdup(stream);
	return CALENDAR_ERROR_NONE;
}
