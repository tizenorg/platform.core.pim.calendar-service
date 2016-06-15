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

#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_view.h"
#include "cal_record.h"

#include "cal_db_util.h"
#include "cal_db.h"
#include "cal_db_query.h"

#define CAL_DB_CALTIME_FIELD_MAX 3
#define CAL_DB_ESCAPE_CHAR	'\\'

static const char* _cal_db_utime_field_name[] = {
	"dtstart_utime",
	"dtend_utime",
	"until_utime"
};
static const char* _cal_db_datetime_field_name[] = {
	"dtstart_datetime",
	"dtend_datetime",
	"until_datetime"
};
static const char* _cal_db_timetype_field_name[] = {
	"dtstart_type",
	"dtend_type",
	"until_type"
};

static int _cal_db_query_create_composite_condition(cal_composite_filter_s *com_filter,
		char **condition, GSList **bind_text);
static int _cal_db_query_create_attribute_condition(cal_composite_filter_s *com_filter,
		cal_attribute_filter_s *filter, char **condition, GSList **bind_text);
static int _cal_db_query_create_str_condition(cal_composite_filter_s *com_filter,
		cal_attribute_filter_s *filter, char **condition, GSList **bind_text);
static int _cal_db_query_create_int_condition(cal_composite_filter_s *com_filter,
		cal_attribute_filter_s *filter, char **condition);
static int _cal_db_query_create_double_condition(cal_composite_filter_s *com_filter,
		cal_attribute_filter_s *filter, char **condition);
static int _cal_db_query_create_lli_condition(cal_composite_filter_s *com_filter,
		cal_attribute_filter_s *filter, char **condition);
static int _cal_db_query_create_caltime_condition(cal_composite_filter_s *com_filter,
		cal_attribute_filter_s *filter, char **condition);
static const char * _cal_db_query_get_property_field_name(const cal_property_info_s *properties,
		int count, unsigned int property_id);
static const char * _cal_db_query_get_utime_field_name(const char* src);
static const char * _cal_db_query_get_datetime_field_name(const char* src);
static const char * _cal_db_query_get_timetype_field_name(const char* src);

int cal_db_query_create_condition(calendar_query_h query, char **condition, GSList **bind_text)
{
	cal_query_s *que = NULL;
	int ret = CALENDAR_ERROR_NONE;

	RETV_IF(NULL == query, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == condition, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == bind_text, CALENDAR_ERROR_INVALID_PARAMETER);

	que = (cal_query_s *)query;

	ret = _cal_db_query_create_composite_condition(que->filter, condition, bind_text);

	return ret;
}

int cal_db_query_create_projection(calendar_query_h query, char **projection)
{
	int i = 0;
	const char *field_name;
	char *out_projection = NULL;
	cal_query_s *query_s = NULL;
	cal_property_info_s *properties = NULL;

	RETV_IF(NULL == query, CALENDAR_ERROR_INVALID_PARAMETER);
	query_s = (cal_query_s *)query;

	properties = query_s->properties;

	if (NULL == query_s->projection || '\0' == *(query_s->projection)) {
		*projection = g_strdup("* ");
		return CALENDAR_ERROR_NONE;
	}

	field_name = _cal_db_query_get_property_field_name(properties, query_s->property_count, query_s->projection[0]);
	if (field_name)
		cal_db_append_string(&out_projection, (char*)field_name);

	for (i = 1; i < query_s->projection_count; i++) {
		field_name = _cal_db_query_get_property_field_name(properties, query_s->property_count, query_s->projection[i]);
		if (field_name) {
			cal_db_append_string(&out_projection, ",");
			cal_db_append_string(&out_projection, (char*)field_name);
		}
	}

	*projection = out_projection;

	return CALENDAR_ERROR_NONE;
}

int cal_db_query_create_order(calendar_query_h query, char *condition, char **order)
{
	const char *field_name = NULL;
	char out_order[CAL_DB_SQL_MAX_LEN] = {0};
	cal_query_s *query_s = NULL;
	cal_property_info_s *properties = NULL;

	RETV_IF(NULL == query, CALENDAR_ERROR_INVALID_PARAMETER);
	query_s = (cal_query_s *)query;
	properties = query_s->properties;

	if (query_s->sort_property_id <= 0)
		return CALENDAR_ERROR_NO_DATA;

	field_name = _cal_db_query_get_property_field_name(properties, query_s->property_count, query_s->sort_property_id);
	if (CAL_PROPERTY_CHECK_DATA_TYPE(query_s->sort_property_id, CAL_PROPERTY_DATA_TYPE_CALTIME) == true && field_name) {
		const char *p_utime = NULL;
		const char *p_datetime = NULL;

		p_utime = _cal_db_query_get_utime_field_name(field_name);
		p_datetime = _cal_db_query_get_datetime_field_name(field_name);

		if (!p_utime && !p_datetime) {
			if (condition) {
				p_utime = _cal_db_query_get_utime_field_name(condition);
				p_datetime = _cal_db_query_get_datetime_field_name(condition);
			} else {
				DBG("No condition");
			}
		}

		if (p_utime && p_datetime) {
			cal_record_type_e type = cal_view_get_type(query_s->view_uri);
			switch (type) {
			case CAL_RECORD_TYPE_INSTANCE_NORMAL_EXTENDED:
			case CAL_RECORD_TYPE_INSTANCE_NORMAL:
				snprintf(out_order, sizeof(out_order), "ORDER BY %s %s ", p_utime, query_s->asc == false ? "DESC" : "ASC");
				break;
			case CAL_RECORD_TYPE_INSTANCE_ALLDAY_EXTENDED:
			case CAL_RECORD_TYPE_INSTANCE_ALLDAY:
				snprintf(out_order, sizeof(out_order), "ORDER BY %s %s ", p_datetime, query_s->asc == false ? "DESC" : "ASC");
				break;
			default:
				snprintf(out_order, sizeof(out_order), "ORDER BY %s %s, %s %s ", p_utime, query_s->asc == false ? "DESC" : "ASC", p_datetime, query_s->asc == false ? "DESC" : "ASC");
				break;
			}
		} else if (p_utime && !p_datetime) {
			snprintf(out_order, sizeof(out_order), "ORDER BY %s %s ", p_utime, query_s->asc == false ? "DESC" : "ASC");
		} else if (!p_utime && p_datetime) {
			snprintf(out_order, sizeof(out_order), "ORDER BY %s %s ", p_datetime, query_s->asc == false ? "DESC" : "ASC");
		} else {
			DBG("No utime, datetime");
		}
	} else {
		snprintf(out_order, sizeof(out_order), "ORDER BY %s %s ", field_name, query_s->asc == false ? "DESC" : "ASC");
	}
	*order = strdup(out_order);

	return CALENDAR_ERROR_NONE;
}

bool cal_db_query_find_projection_property(calendar_query_h query, unsigned int property)
{
	int i = 0;
	cal_query_s *query_s = NULL;

	RETV_IF(NULL == query, CALENDAR_ERROR_INVALID_PARAMETER);
	query_s = (cal_query_s *)query;

	if (NULL == query_s->projection || '\0' == *query_s->projection)
		return true;

	for (i = 0; i < query_s->projection_count; i++)
		if (query_s->projection[i] == property)
			return true;

	return false;
}

int cal_db_query_create_projection_update_set(calendar_record_h record, char **set, GSList **bind_text)
{
	cal_record_s *_record = NULL;
	int i = 0;
	const cal_property_info_s* property_info = NULL;
	int property_info_count = 0;
	char out_set[CAL_DB_SQL_MAX_LEN] = {0};
	int len = 0;
	const char *field_name;
	int ret = CALENDAR_ERROR_NONE;

	RETV_IF(NULL == record, -1);

	_record = (cal_record_s *)record;

	RETV_IF(0 == _record->properties_max_count, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == _record->properties_flags, CALENDAR_ERROR_INVALID_PARAMETER);

	/* get propety_info_s from uri */
	property_info = cal_view_get_property_info(_record->view_uri, &property_info_count);

	for (i = 0; i < property_info_count; i++) {
		if (true == cal_record_check_property_flag(record, property_info[i].property_id , CAL_PROPERTY_FLAG_DIRTY)) {
			field_name = property_info[i].fields;

			if (NULL == field_name)
				continue;

			if (CAL_PROPERTY_CHECK_DATA_TYPE(property_info[i].property_id, CAL_PROPERTY_DATA_TYPE_INT) == true) {
				int tmp = 0;
				ret = calendar_record_get_int(record, property_info[i].property_id, &tmp);
				if (CALENDAR_ERROR_NONE != ret)
					continue;
				if (strlen(out_set) != 0)
					len += snprintf(out_set+len, sizeof(out_set)-len, ", ");

				len += snprintf(out_set+len, sizeof(out_set)-len, "%s=%d", field_name, tmp);
			} else if (CAL_PROPERTY_CHECK_DATA_TYPE(property_info[i].property_id, CAL_PROPERTY_DATA_TYPE_STR) == true) {
				char *tmp = NULL;
				ret = calendar_record_get_str(record, property_info[i].property_id, &tmp);
				if (CALENDAR_ERROR_NONE != ret)
					continue;
				if (strlen(out_set) != 0)
					len += snprintf(out_set+len, sizeof(out_set)-len, ", ");

				len += snprintf(out_set+len, sizeof(out_set)-len, "%s=?", field_name);
				*bind_text = g_slist_append(*bind_text, tmp);
			} else if (CAL_PROPERTY_CHECK_DATA_TYPE(property_info[i].property_id, CAL_PROPERTY_DATA_TYPE_DOUBLE) == true) {
				double tmp = 0;
				ret = calendar_record_get_double(record, property_info[i].property_id, &tmp);
				if (CALENDAR_ERROR_NONE != ret)
					continue;
				if (strlen(out_set) != 0)
					len += snprintf(out_set+len, sizeof(out_set)-len, ", ");

				len += snprintf(out_set+len, sizeof(out_set)-len, "%s=%lf", field_name, tmp);
			} else if (CAL_PROPERTY_CHECK_DATA_TYPE(property_info[i].property_id, CAL_PROPERTY_DATA_TYPE_LLI) == true) {
				long long int tmp = 0;
				ret = calendar_record_get_lli(record, property_info[i].property_id, &tmp);
				if (CALENDAR_ERROR_NONE != ret)
					continue;
				if (strlen(out_set) != 0)
					len += snprintf(out_set+len, sizeof(out_set)-len, ", ");

				len += snprintf(out_set+len, sizeof(out_set)-len, "%s=%lld", field_name, tmp);
			} else if (CAL_PROPERTY_CHECK_DATA_TYPE(property_info[i].property_id, CAL_PROPERTY_DATA_TYPE_CALTIME) == true) {
				/*
				 * field_name
				 * dtstart_type, dtstart_utime, dtstart_datetime
				 *      -> dtstart_type=%d, dtstart_utime=%lli, dtstart_datetime=?
				 * dtend_type, dtend_utime, dtend_datetime
				 *      -> dtend_type=%d, dtend_utime=%lli, dtend_datetime=?
				 * until_type, until_utime, until_datetime
				 *      -> until_type=%d, until_utime=%lli, until_datetime=?
				 */
				calendar_time_s tmp = {0,};
				const char *timetype_field = NULL;
				const char *utime_field = NULL;
				const char *datetime_field = NULL;
				timetype_field = _cal_db_query_get_timetype_field_name(field_name);
				utime_field = _cal_db_query_get_utime_field_name(field_name);
				datetime_field = _cal_db_query_get_datetime_field_name(field_name);
				ret = calendar_record_get_caltime(record, property_info[i].property_id, &tmp);
				if (CALENDAR_ERROR_NONE != ret)
					continue;
				if (tmp.type == CALENDAR_TIME_UTIME) {
					if (strlen(out_set) != 0)
						len += snprintf(out_set+len, sizeof(out_set)-len, ", ");

					len += snprintf(out_set+len, sizeof(out_set)-len, "%s=%d, %s=%lld",
							timetype_field, CALENDAR_TIME_UTIME, utime_field, tmp.time.utime);
				} else {
					char *bind_tmp = NULL;
					char bind_datetime[CAL_STR_SHORT_LEN32] = {0};
					snprintf(bind_datetime, sizeof(bind_datetime), CAL_FORMAT_LOCAL_DATETIME,
							tmp.time.date.year,
							tmp.time.date.month,
							tmp.time.date.mday,
							tmp.time.date.hour,
							tmp.time.date.minute,
							tmp.time.date.second);

					if (strlen(out_set) != 0)
						len += snprintf(out_set+len, sizeof(out_set)-len, ", ");

					len += snprintf(out_set+len, sizeof(out_set)-len, "%s=%d, %s=?",
							timetype_field, CALENDAR_TIME_LOCALTIME, datetime_field);

					bind_tmp = strdup(bind_datetime);
					*bind_text = g_slist_append(*bind_text, bind_tmp);
				}
			}
		}
	}

	*set = strdup(out_set);
	DBG("set=%s", *set);

	return CALENDAR_ERROR_NONE;
}

int cal_db_query_create_projection_update_set_with_property(
		calendar_record_h record, unsigned int *properties, int properties_count,
		char **set, GSList **bind_text)
{
	cal_record_s *_record = NULL;
	int i = 0;
	const cal_property_info_s* property_info = NULL;
	int property_info_count = 0;
	char out_set[CAL_DB_SQL_MAX_LEN] = {0};
	int len = 0;
	const char *field_name;
	int ret = CALENDAR_ERROR_NONE;

	RETV_IF(NULL == record, -1);

	_record = (cal_record_s *)record;

	if (_record->properties_max_count == 0 || NULL == _record->properties_flags)
		return CALENDAR_ERROR_NONE;

	/* get propety_info_s from uri */
	property_info = cal_view_get_property_info(_record->view_uri, &property_info_count);

	for (i = 0; i < property_info_count; i++) {
		int j = 0;
		bool flag = false;
		for (j = 0; j < properties_count; j++) {
			if (property_info[i].property_id == properties[j]) {
				flag = true;
				break;
			}
		}
		if (true == flag &&
				(true == cal_record_check_property_flag(record, property_info[i].property_id , CAL_PROPERTY_FLAG_DIRTY))) {
			field_name = property_info[i].fields;

			if (NULL == field_name)
				continue;

			if (CAL_PROPERTY_CHECK_DATA_TYPE(property_info[i].property_id, CAL_PROPERTY_DATA_TYPE_INT) == true) {
				int tmp = 0;
				ret = calendar_record_get_int(record, property_info[i].property_id, &tmp);
				if (CALENDAR_ERROR_NONE != ret)
					continue;
				if (strlen(out_set) != 0)
					len += snprintf(out_set+len, sizeof(out_set)-len, ", ");

				len += snprintf(out_set+len, sizeof(out_set)-len, "%s=%d", field_name, tmp);
			} else if (CAL_PROPERTY_CHECK_DATA_TYPE(property_info[i].property_id, CAL_PROPERTY_DATA_TYPE_STR) == true) {
				char *tmp = NULL;
				ret = calendar_record_get_str(record, property_info[i].property_id, &tmp);
				if (CALENDAR_ERROR_NONE != ret)
					continue;
				if (strlen(out_set) != 0)
					len += snprintf(out_set+len, sizeof(out_set)-len, ", ");

				len += snprintf(out_set+len, sizeof(out_set)-len, "%s=?", field_name);
				*bind_text = g_slist_append(*bind_text, tmp);
			} else if (CAL_PROPERTY_CHECK_DATA_TYPE(property_info[i].property_id, CAL_PROPERTY_DATA_TYPE_DOUBLE) == true) {
				double tmp = 0;
				ret = calendar_record_get_double(record, property_info[i].property_id, &tmp);
				if (CALENDAR_ERROR_NONE != ret)
					continue;
				if (strlen(out_set) != 0)
					len += snprintf(out_set+len, sizeof(out_set)-len, ", ");

				len += snprintf(out_set+len, sizeof(out_set)-len, "%s=%lf", field_name, tmp);
			} else if (CAL_PROPERTY_CHECK_DATA_TYPE(property_info[i].property_id, CAL_PROPERTY_DATA_TYPE_LLI) == true) {
				long long int tmp = 0;
				ret = calendar_record_get_lli(record, property_info[i].property_id, &tmp);
				if (CALENDAR_ERROR_NONE != ret)
					continue;
				if (strlen(out_set) != 0)
					len += snprintf(out_set+len, sizeof(out_set)-len, ", ");

				len += snprintf(out_set+len, sizeof(out_set)-len, "%s=%lld", field_name, tmp);
			} else if (CAL_PROPERTY_CHECK_DATA_TYPE(property_info[i].property_id, CAL_PROPERTY_DATA_TYPE_CALTIME) == true) {
				/*
				 * field_name
				 * dtstart_type, dtstart_utime, dtstart_datetime
				 *      -> dtstart_type=%d, dtstart_utime=%lli, dtstart_datetime=?
				 * dtend_type, dtend_utime, dtend_datetime
				 *      -> dtend_type=%d, dtend_utime=%lli, dtend_datetime=?
				 * until_type, until_utime, until_datetime
				 *      -> until_type=%d, until_utime=%lli, until_datetime=?
				 */
				calendar_time_s tmp = {0,};
				const char *timetype_field = NULL;
				const char *utime_field = NULL;
				const char *datetime_field = NULL;
				timetype_field = _cal_db_query_get_timetype_field_name(field_name);
				utime_field = _cal_db_query_get_utime_field_name(field_name);
				datetime_field = _cal_db_query_get_datetime_field_name(field_name);
				ret = calendar_record_get_caltime(record, property_info[i].property_id, &tmp);
				if (CALENDAR_ERROR_NONE != ret)
					continue;
				if (tmp.type == CALENDAR_TIME_UTIME) {
					if (strlen(out_set) != 0)
						len += snprintf(out_set+len, sizeof(out_set)-len, ", ");

					len += snprintf(out_set+len, sizeof(out_set)-len, "%s=%d, %s=%lld",
							timetype_field, CALENDAR_TIME_UTIME, utime_field, tmp.time.utime);
				} else {
					char *bind_tmp = NULL;
					char bind_datetime[CAL_STR_SHORT_LEN32] = {0};
					snprintf(bind_datetime, sizeof(bind_datetime), CAL_FORMAT_LOCAL_DATETIME,
							tmp.time.date.year,
							tmp.time.date.month,
							tmp.time.date.mday,
							tmp.time.date.hour,
							tmp.time.date.minute,
							tmp.time.date.second);

					if (strlen(out_set) != 0)
						len += snprintf(out_set+len, sizeof(out_set)-len, ", ");

					len += snprintf(out_set+len, sizeof(out_set)-len, "%s=%d, %s=?",
							timetype_field, CALENDAR_TIME_LOCALTIME, datetime_field);

					bind_tmp = strdup(bind_datetime);
					*bind_text = g_slist_append(*bind_text, bind_tmp);
				}
			}
		}
	}

	*set = strdup(out_set);

	return CALENDAR_ERROR_NONE;
}

static int _cal_db_query_create_composite_condition(cal_composite_filter_s *com_filter, char **condition, GSList **bind_text)
{
	GSList *cursor_filter = NULL;
	GSList *cursor_ops = NULL;
	calendar_filter_operator_e op;
	char *cond = NULL;
	char *out_cond = NULL;
	GSList *binds = NULL, *binds2 = NULL;
	cal_filter_s *filter;
	int ret = CALENDAR_ERROR_NONE;

	RETV_IF(NULL == com_filter, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == com_filter->filters, CALENDAR_ERROR_INVALID_PARAMETER);

	filter = (cal_filter_s *)com_filter->filters->data;
	if (filter->filter_type == CAL_FILTER_COMPOSITE)
		ret = _cal_db_query_create_composite_condition((cal_composite_filter_s*)filter, &cond, &binds);
	else
		ret = _cal_db_query_create_attribute_condition(com_filter, (cal_attribute_filter_s*)filter, &cond, &binds);

	if (CALENDAR_ERROR_NONE != ret) {
		/* LCOV_EXCL_START */
		ERR("_cal_db_query_create_attribute_condition() Fail(%d)", ret);
		return CALENDAR_ERROR_INVALID_PARAMETER;
		/* LCOV_EXCL_STOP */
	}

	cursor_filter = com_filter->filters->next;

	cal_db_append_string(&out_cond, "(");
	cal_db_append_string(&out_cond, cond);
	cal_db_append_string(&out_cond, ")");

	CAL_FREE(cond);

	for (cursor_ops = com_filter->filter_ops; cursor_ops && cursor_filter;
			cursor_filter = cursor_filter->next, cursor_ops = cursor_ops->next) {
		filter = (cal_filter_s *)cursor_filter->data;
		if (filter->filter_type == CAL_FILTER_COMPOSITE)
			ret = _cal_db_query_create_composite_condition((cal_composite_filter_s*)filter, &cond, &binds2);
		else
			ret = _cal_db_query_create_attribute_condition(com_filter, (cal_attribute_filter_s*)filter, &cond, &binds2);

		if (CALENDAR_ERROR_NONE != ret) {
			/* LCOV_EXCL_START */
			ERR("_cal_db_query_create_attribute_condition() Fail(%d)", ret);
			CAL_FREE(out_cond);
			if (binds) {
				g_slist_free_full(binds, free);
				binds = NULL;
			}
			return CALENDAR_ERROR_INVALID_PARAMETER;
			/* LCOV_EXCL_STOP */
		}

		op = (calendar_filter_operator_e)cursor_ops->data;
		if (op == CALENDAR_FILTER_OPERATOR_AND) {
			cal_db_append_string(&out_cond, "AND (");
			cal_db_append_string(&out_cond, cond);
			cal_db_append_string(&out_cond, ")");
		} else {
			cal_db_append_string(&out_cond, "OR (");
			cal_db_append_string(&out_cond, cond);
			cal_db_append_string(&out_cond, ")");
		}

		if (binds2)
			binds = g_slist_concat(binds, binds2);
		binds2 = NULL;

		CAL_FREE(cond);
	}

	*condition = out_cond;
	*bind_text = binds;

	return CALENDAR_ERROR_NONE;
}

static int _cal_db_query_create_attribute_condition(cal_composite_filter_s *com_filter,
		cal_attribute_filter_s *filter, char **condition, GSList **bind_text)
{
	int ret;
	char *cond = NULL;

	RETV_IF(NULL == filter, CALENDAR_ERROR_INVALID_PARAMETER);

	switch (filter->filter_type) {
	case CAL_FILTER_INT:
		ret = _cal_db_query_create_int_condition(com_filter, filter, &cond);
		break;
	case CAL_FILTER_STR:
		ret = _cal_db_query_create_str_condition(com_filter, filter, &cond, bind_text);
		break;
	case CAL_FILTER_DOUBLE:
		ret = _cal_db_query_create_double_condition(com_filter, filter, &cond);
		break;
	case CAL_FILTER_LLI:
		ret = _cal_db_query_create_lli_condition(com_filter, filter, &cond);
		break;
	case CAL_FILTER_CALTIME:
		ret = _cal_db_query_create_caltime_condition(com_filter, filter, &cond);
		break;
	default:
		/* LCOV_EXCL_START */
		ERR("The filter type is not supported (%d)", filter->filter_type);
		return CALENDAR_ERROR_INVALID_PARAMETER;
		/* LCOV_EXCL_STOP */
	}

	if (CALENDAR_ERROR_NONE == ret)
		*condition = (cond);

	return ret;
}

static int _cal_db_query_create_int_condition(cal_composite_filter_s *com_filter,
		cal_attribute_filter_s *filter, char **condition)
{
	const char *field_name;
	char out_cond[CAL_DB_SQL_MAX_LEN] = {0};

	field_name = _cal_db_query_get_property_field_name(com_filter->properties,
			com_filter->property_count, filter->property_id);
	RETVM_IF(NULL == field_name, CALENDAR_ERROR_INVALID_PARAMETER,
			"Invalid parameter : property id(%d)", filter->property_id);

	switch (filter->match) {
	case CALENDAR_MATCH_EQUAL:
		snprintf(out_cond, sizeof(out_cond), "%s = %d", field_name, filter->value.i);
		break;
	case CALENDAR_MATCH_GREATER_THAN:
		snprintf(out_cond, sizeof(out_cond), "%s > %d", field_name, filter->value.i);
		break;
	case CALENDAR_MATCH_GREATER_THAN_OR_EQUAL:
		snprintf(out_cond, sizeof(out_cond), "%s >= %d", field_name, filter->value.i);
		break;
	case CALENDAR_MATCH_LESS_THAN:
		snprintf(out_cond, sizeof(out_cond), "%s < %d", field_name, filter->value.i);
		break;
	case CALENDAR_MATCH_LESS_THAN_OR_EQUAL:
		snprintf(out_cond, sizeof(out_cond), "%s <= %d", field_name, filter->value.i);
		break;
	case CALENDAR_MATCH_NONE:
		snprintf(out_cond, sizeof(out_cond), "%s IS NULL", field_name);
		break;
	case CALENDAR_MATCH_NOT_EQUAL:
		snprintf(out_cond, sizeof(out_cond), "%s <> %d", field_name, filter->value.i);
		break;
	default:
		/* LCOV_EXCL_START */
		ERR("Invalid parameter : int match rule(%d) is not supported", filter->match);
		return CALENDAR_ERROR_INVALID_PARAMETER;
		/* LCOV_EXCL_STOP */
	}

	*condition = strdup(out_cond);
	return CALENDAR_ERROR_NONE;
}

static int _cal_db_query_create_double_condition(cal_composite_filter_s *com_filter,
		cal_attribute_filter_s *filter, char **condition)
{
	const char *field_name;
	char out_cond[CAL_DB_SQL_MAX_LEN] = {0};

	field_name = _cal_db_query_get_property_field_name(com_filter->properties,
			com_filter->property_count, filter->property_id);
	RETVM_IF(NULL == field_name, CALENDAR_ERROR_INVALID_PARAMETER,
			"Invalid parameter : property id(%d)", filter->property_id);

	switch (filter->match) {
	case CALENDAR_MATCH_EQUAL:
		snprintf(out_cond, sizeof(out_cond), "%s = %lf", field_name, filter->value.d);
		break;
	case CALENDAR_MATCH_GREATER_THAN:
		snprintf(out_cond, sizeof(out_cond), "%s > %lf", field_name, filter->value.d);
		break;
	case CALENDAR_MATCH_GREATER_THAN_OR_EQUAL:
		snprintf(out_cond, sizeof(out_cond), "%s >= %lf", field_name, filter->value.d);
		break;
	case CALENDAR_MATCH_LESS_THAN:
		snprintf(out_cond, sizeof(out_cond), "%s < %lf", field_name, filter->value.d);
		break;
	case CALENDAR_MATCH_LESS_THAN_OR_EQUAL:
		snprintf(out_cond, sizeof(out_cond), "%s <= %lf", field_name, filter->value.d);
		break;
	case CALENDAR_MATCH_NONE:
		snprintf(out_cond, sizeof(out_cond), "%s IS NULL", field_name);
		break;
	case CALENDAR_MATCH_NOT_EQUAL:
		snprintf(out_cond, sizeof(out_cond), "%s <> %lf", field_name, filter->value.d);
		break;
	default:
		/* LCOV_EXCL_START */
		ERR("Invalid parameter : int match rule(%d) is not supported", filter->match);
		return CALENDAR_ERROR_INVALID_PARAMETER;
		/* LCOV_EXCL_STOP */
	}

	*condition = strdup(out_cond);
	return CALENDAR_ERROR_NONE;
}

static int _cal_db_query_create_lli_condition(cal_composite_filter_s *com_filter,
		cal_attribute_filter_s *filter, char **condition)
{
	const char *field_name;
	char out_cond[CAL_DB_SQL_MAX_LEN] = {0};

	field_name = _cal_db_query_get_property_field_name(com_filter->properties,
			com_filter->property_count, filter->property_id);
	RETVM_IF(NULL == field_name, CALENDAR_ERROR_INVALID_PARAMETER,
			"Invalid parameter : property id(%d)", filter->property_id);

	switch (filter->match) {
	case CALENDAR_MATCH_EQUAL:
		snprintf(out_cond, sizeof(out_cond), "%s = %lld", field_name, filter->value.lli);
		break;
	case CALENDAR_MATCH_GREATER_THAN:
		snprintf(out_cond, sizeof(out_cond), "%s > %lld", field_name, filter->value.lli);
		break;
	case CALENDAR_MATCH_GREATER_THAN_OR_EQUAL:
		snprintf(out_cond, sizeof(out_cond), "%s >= %lld", field_name, filter->value.lli);
		break;
	case CALENDAR_MATCH_LESS_THAN:
		snprintf(out_cond, sizeof(out_cond), "%s < %lld", field_name, filter->value.lli);
		break;
	case CALENDAR_MATCH_LESS_THAN_OR_EQUAL:
		snprintf(out_cond, sizeof(out_cond), "%s <= %lld", field_name, filter->value.lli);
		break;
	case CALENDAR_MATCH_NONE:
		snprintf(out_cond, sizeof(out_cond), "%s IS NULL", field_name);
		break;
	case CALENDAR_MATCH_NOT_EQUAL:
		snprintf(out_cond, sizeof(out_cond), "%s <> %lld", field_name, filter->value.lli);
		break;
	default:
		/* LCOV_EXCL_START */
		ERR("Invalid parameter : int match rule(%d) is not supported", filter->match);
		return CALENDAR_ERROR_INVALID_PARAMETER;
		/* LCOV_EXCL_STOP */
	}

	*condition = strdup(out_cond);
	return CALENDAR_ERROR_NONE;
}

static int _cal_db_query_create_caltime_condition(cal_composite_filter_s *com_filter,
		cal_attribute_filter_s *filter, char **condition)
{
	const char *field_name;
	char out_cond[CAL_DB_SQL_MAX_LEN] = {0};
	const char *tmp = NULL;

	field_name = _cal_db_query_get_property_field_name(com_filter->properties,
			com_filter->property_count, filter->property_id);
	RETVM_IF(NULL == field_name, CALENDAR_ERROR_INVALID_PARAMETER,
			"Invalid parameter : property id(%d)", filter->property_id);

	if (filter->value.caltime.type == CALENDAR_TIME_UTIME) {
		tmp = _cal_db_query_get_utime_field_name(field_name);
		if (NULL == tmp)
			tmp = field_name;

		switch (filter->match) {
		case CALENDAR_MATCH_EQUAL:
			snprintf(out_cond, sizeof(out_cond), "%s = %lld", tmp, filter->value.caltime.time.utime);
			break;
		case CALENDAR_MATCH_GREATER_THAN:
			snprintf(out_cond, sizeof(out_cond), "%s > %lld", tmp, filter->value.caltime.time.utime);
			break;
		case CALENDAR_MATCH_GREATER_THAN_OR_EQUAL:
			snprintf(out_cond, sizeof(out_cond), "%s >= %lld", tmp, filter->value.caltime.time.utime);
			break;
		case CALENDAR_MATCH_LESS_THAN:
			snprintf(out_cond, sizeof(out_cond), "%s < %lld", tmp, filter->value.caltime.time.utime);
			break;
		case CALENDAR_MATCH_LESS_THAN_OR_EQUAL:
			snprintf(out_cond, sizeof(out_cond), "%s <= %lld", tmp, filter->value.caltime.time.utime);
			break;
		case CALENDAR_MATCH_NONE:
			snprintf(out_cond, sizeof(out_cond), "%s IS NULL", tmp);
			break;
		case CALENDAR_MATCH_NOT_EQUAL:
			snprintf(out_cond, sizeof(out_cond), "%s <> %lld", tmp, filter->value.caltime.time.utime);
			break;
		default:
			/* LCOV_EXCL_START */
			ERR("Invalid parameter : int match rule(%d) is not supported", filter->match);
			return CALENDAR_ERROR_INVALID_PARAMETER;
			/* LCOV_EXCL_STOP */
		}
	} else if (filter->value.caltime.type == CALENDAR_TIME_LOCALTIME) {
		char sdate[CAL_STR_SHORT_LEN32] = {0};
		snprintf(sdate, sizeof(sdate), CAL_FORMAT_LOCAL_DATETIME,
				filter->value.caltime.time.date.year, filter->value.caltime.time.date.month, filter->value.caltime.time.date.mday,
				filter->value.caltime.time.date.hour, filter->value.caltime.time.date.minute, filter->value.caltime.time.date.second);
		tmp = _cal_db_query_get_datetime_field_name(field_name);
		if (NULL == tmp)
			tmp = field_name;

		switch (filter->match) {
		case CALENDAR_MATCH_EQUAL:
			snprintf(out_cond, sizeof(out_cond), "%s = '%s'", tmp, sdate);
			break;
		case CALENDAR_MATCH_GREATER_THAN:
			snprintf(out_cond, sizeof(out_cond), "%s > '%s'", tmp, sdate);
			break;
		case CALENDAR_MATCH_GREATER_THAN_OR_EQUAL:
			snprintf(out_cond, sizeof(out_cond), "%s >= '%s'", tmp, sdate);
			break;
		case CALENDAR_MATCH_LESS_THAN:
			snprintf(out_cond, sizeof(out_cond), "%s < '%s'", tmp, sdate);
			break;
		case CALENDAR_MATCH_LESS_THAN_OR_EQUAL:
			snprintf(out_cond, sizeof(out_cond), "%s <= '%s'", tmp, sdate);
			break;
		case CALENDAR_MATCH_NOT_EQUAL:
			snprintf(out_cond, sizeof(out_cond), "%s <> '%s'", tmp, sdate);
			break;
		case CALENDAR_MATCH_NONE:
		default:
			/* LCOV_EXCL_START */
			ERR("Invalid parameter : int match rule(%d) is not supported", filter->match);
			return CALENDAR_ERROR_INVALID_PARAMETER;
			/* LCOV_EXCL_STOP */
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("Invalid parameter : property id(%d)", filter->property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
		/* LCOV_EXCL_STOP */
	}

	cal_record_type_e record_type = cal_view_get_type(com_filter->view_uri);
	if (record_type != CAL_RECORD_TYPE_INSTANCE_NORMAL &&
			record_type != CAL_RECORD_TYPE_INSTANCE_ALLDAY &&
			record_type != CAL_RECORD_TYPE_INSTANCE_NORMAL_EXTENDED &&
			record_type != CAL_RECORD_TYPE_INSTANCE_ALLDAY_EXTENDED) {
		int len = strlen(out_cond);
		const char *type_field = _cal_db_query_get_timetype_field_name(field_name);
		snprintf(out_cond + len, sizeof(out_cond) - len -1, " AND %s = %d ", type_field, filter->value.caltime.type);
	}

	*condition = strdup(out_cond);
	return CALENDAR_ERROR_NONE;
}

static char * _cal_db_get_str_with_escape(char *str, int len, bool with_escape)
{
	int i, j = 0;
	char temp_str[len*2+1];

	if (false == with_escape)
		return strdup(str);

	for (i = 0; i < len; i++) {
		if (str[i] == '\'' || str[i] == '_' || str[i] == '%' || str[i] == '\\')
			temp_str[j++] = CAL_DB_ESCAPE_CHAR;

		temp_str[j++] = str[i];
	}
	temp_str[j] = '\0';

	return strdup(temp_str);
}

static int _cal_db_query_create_str_condition(cal_composite_filter_s *com_filter,
		cal_attribute_filter_s *filter, char **condition, GSList **bind_text)
{
	const char *field_name;
	char out_cond[CAL_DB_SQL_MAX_LEN] = {0};
	bool with_escape = true;

	field_name = _cal_db_query_get_property_field_name(com_filter->properties,
			com_filter->property_count, filter->property_id);
	RETVM_IF(NULL == field_name, CALENDAR_ERROR_INVALID_PARAMETER,
			"Invalid parameter : property id(%d)", filter->property_id);

	switch (filter->match) {
	case CALENDAR_MATCH_EXACTLY:
		snprintf(out_cond, sizeof(out_cond), "%s = ?", field_name);
		with_escape = false;
		break;
	case CALENDAR_MATCH_FULLSTRING:
		snprintf(out_cond, sizeof(out_cond), "%s LIKE ? ESCAPE '%c'", field_name, CAL_DB_ESCAPE_CHAR);
		break;
	case CALENDAR_MATCH_CONTAINS:
		snprintf(out_cond, sizeof(out_cond), "%s LIKE ('%%' || ? || '%%') ESCAPE '%c'", field_name, CAL_DB_ESCAPE_CHAR);
		break;
	case CALENDAR_MATCH_STARTSWITH:
		snprintf(out_cond, sizeof(out_cond), "%s LIKE (? || '%%') ESCAPE '%c'", field_name, CAL_DB_ESCAPE_CHAR);
		break;
	case CALENDAR_MATCH_ENDSWITH:
		snprintf(out_cond, sizeof(out_cond), "%s LIKE ('%%' || ?) ESCAPE '%c'", field_name, CAL_DB_ESCAPE_CHAR);
		break;
	case CALENDAR_MATCH_EXISTS:
		snprintf(out_cond, sizeof(out_cond), "%s IS NOT NULL", field_name);
		break;
	default:
		/* LCOV_EXCL_START */
		ERR("Invalid paramter : int match rule (%d) is not supported", filter->match);
		return CALENDAR_ERROR_INVALID_PARAMETER;
		/* LCOV_EXCL_STOP */
	}

	if (filter->value.s) {
		*bind_text = g_slist_append(*bind_text,
				_cal_db_get_str_with_escape(filter->value.s, strlen(filter->value.s), with_escape));
	}
	*condition = strdup(out_cond);
	return CALENDAR_ERROR_NONE;
}

static const char * _cal_db_query_get_property_field_name(const cal_property_info_s *properties,
		int count, unsigned int property_id)
{
	int i;
	for (i = 0; i < count; i++) {
		cal_property_info_s *p = (cal_property_info_s*)&(properties[i]);
		if (property_id == p->property_id) {
			if (p->fields)
				return p->fields;
			else
				return NULL;
		}
	}
	return NULL;
}

static const char * _cal_db_query_get_utime_field_name(const char* src)
{
	char *tmp1 = NULL;
	int i = 0;

	for (i = 0; i < CAL_DB_CALTIME_FIELD_MAX; i++) {
		tmp1 = strstr(src, _cal_db_utime_field_name[i]);
		if (tmp1)
			return _cal_db_utime_field_name[i];
	}

	return NULL;
}

static const char * _cal_db_query_get_datetime_field_name(const char* src)
{
	char *tmp1 = NULL;
	int i = 0;

	for (i = 0; i < CAL_DB_CALTIME_FIELD_MAX; i++) {
		tmp1 = strstr(src, _cal_db_datetime_field_name[i]);
		if (tmp1)
			return _cal_db_datetime_field_name[i];
	}

	return NULL;
}

static const char * _cal_db_query_get_timetype_field_name(const char* src)
{
	char *tmp1 = NULL;
	int i = 0;

	for (i = 0; i < CAL_DB_CALTIME_FIELD_MAX; i++) {
		tmp1 = strstr(src, _cal_db_timetype_field_name[i]);
		if (tmp1)
			return _cal_db_timetype_field_name[i];
	}

	return NULL;
}

