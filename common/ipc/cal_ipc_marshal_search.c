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

#include <stdlib.h> //calloc
#include "cal_internal.h"
#include "cal_ipc_marshal.h"
#include "cal_view.h"

static int _cal_ipc_unmarshal_search(pims_ipc_data_h ipc_data, calendar_record_h record);
static int _cal_ipc_marshal_search(const calendar_record_h record, pims_ipc_data_h ipc_data);

cal_ipc_marshal_record_plugin_cb_s cal_ipc_record_search_plugin_cb = {
	.unmarshal_record = _cal_ipc_unmarshal_search,
	.marshal_record = _cal_ipc_marshal_search,
	.get_primary_id = NULL
};

static int _cal_ipc_unmarshal_search_value(pims_ipc_data_h ipc_data, cal_search_value_s* pvalue);
static int _cal_ipc_marshal_search_value(const cal_search_value_s* pvalue, pims_ipc_data_h ipc_data);

static int _cal_ipc_unmarshal_search(pims_ipc_data_h ipc_data, calendar_record_h record)
{
	cal_search_s* psearch = NULL;
	RETV_IF(ipc_data==NULL,CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(record==NULL,CALENDAR_ERROR_INVALID_PARAMETER);

	psearch = (cal_search_s*) record;

	int count = 0,i = 0;
	if (cal_ipc_unmarshal_int(ipc_data,&count) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	for(i=0;i<count;i++) {
		cal_search_value_s* value_data = NULL;
		value_data = calloc(1, sizeof(cal_search_value_s));
		if (NULL == value_data) {
			ERR("calloc() Fail");
			return CALENDAR_ERROR_OUT_OF_MEMORY;
		}

		if (_cal_ipc_unmarshal_search_value(ipc_data, value_data) != CALENDAR_ERROR_NONE) {
			CAL_FREE(value_data);
			ERR("cal_ipc_unmarshal fail");
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
		psearch->values = g_slist_append(psearch->values, value_data);
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_ipc_marshal_search(const calendar_record_h record, pims_ipc_data_h ipc_data)
{
	cal_search_s* psearch = (cal_search_s*) record;
	RETV_IF(ipc_data==NULL,CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(psearch==NULL,CALENDAR_ERROR_INVALID_PARAMETER);

	if (psearch->values) {
		int count = g_slist_length(psearch->values);
		GSList *cursor = psearch->values;
		cal_search_value_s* value_data;

		if (cal_ipc_marshal_int(count,ipc_data) != CALENDAR_ERROR_NONE) {
			ERR("cal_ipc_marshal fail");
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}

		while (cursor) {
			value_data = (cal_search_value_s *)cursor->data;
			if (NULL == value_data) {
				cursor = g_slist_next(cursor);
				continue;
			}
			if (_cal_ipc_marshal_search_value((const cal_search_value_s*)value_data, ipc_data) != CALENDAR_ERROR_NONE) {
				ERR("cal_ipc_marshal fail");
				return CALENDAR_ERROR_INVALID_PARAMETER;
			}
			cursor = g_slist_next(cursor);
		}

	}
	else {
		if (cal_ipc_marshal_int(0,ipc_data) != CALENDAR_ERROR_NONE) {
			ERR("cal_ipc_marshal fail");
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
	}

	return CALENDAR_ERROR_NONE;
}

static int _cal_ipc_unmarshal_search_value(pims_ipc_data_h ipc_data, cal_search_value_s* pvalue)
{
	if (cal_ipc_unmarshal_int(ipc_data,&pvalue->property_id) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_unmarshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	if (CAL_PROPERTY_CHECK_DATA_TYPE(pvalue->property_id, CAL_PROPERTY_DATA_TYPE_STR) == true) {
		if (cal_ipc_unmarshal_char(ipc_data,&pvalue->value.s) != CALENDAR_ERROR_NONE) {
			ERR("cal_ipc_marshal fail");
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
	}
	else if (CAL_PROPERTY_CHECK_DATA_TYPE(pvalue->property_id, CAL_PROPERTY_DATA_TYPE_INT) == true) {
		if (cal_ipc_unmarshal_int(ipc_data,&pvalue->value.i) != CALENDAR_ERROR_NONE) {
			ERR("cal_ipc_marshal fail");
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
	}
	else if (CAL_PROPERTY_CHECK_DATA_TYPE(pvalue->property_id, CAL_PROPERTY_DATA_TYPE_DOUBLE) == true) {
		if (cal_ipc_unmarshal_double(ipc_data,&pvalue->value.d) != CALENDAR_ERROR_NONE) {
			ERR("cal_ipc_marshal fail");
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
	}
	else if (CAL_PROPERTY_CHECK_DATA_TYPE(pvalue->property_id, CAL_PROPERTY_DATA_TYPE_LLI) == true) {
		if (cal_ipc_unmarshal_lli(ipc_data,&pvalue->value.lli) != CALENDAR_ERROR_NONE) {
			ERR("cal_ipc_marshal fail");
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
	}
	else if (CAL_PROPERTY_CHECK_DATA_TYPE(pvalue->property_id, CAL_PROPERTY_DATA_TYPE_CALTIME) == true) {
		if (cal_ipc_unmarshal_caltime(ipc_data,&pvalue->value.caltime) != CALENDAR_ERROR_NONE) {
			ERR("cal_ipc_marshal fail");
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
	}
	else {
		ERR("invalid parameter (property:%d)",pvalue->property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	return CALENDAR_ERROR_NONE;
}

static int _cal_ipc_marshal_search_value(const cal_search_value_s* pvalue, pims_ipc_data_h ipc_data)
{
	if (cal_ipc_marshal_int(pvalue->property_id,ipc_data) != CALENDAR_ERROR_NONE) {
		ERR("cal_ipc_marshal fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	if (CAL_PROPERTY_CHECK_DATA_TYPE(pvalue->property_id, CAL_PROPERTY_DATA_TYPE_STR) == true) {
		if (cal_ipc_marshal_char(pvalue->value.s,ipc_data) != CALENDAR_ERROR_NONE) {
			ERR("cal_ipc_marshal fail");
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
	}
	else if (CAL_PROPERTY_CHECK_DATA_TYPE(pvalue->property_id, CAL_PROPERTY_DATA_TYPE_INT) == true) {
		if (cal_ipc_marshal_int(pvalue->value.i,ipc_data) != CALENDAR_ERROR_NONE) {
			ERR("cal_ipc_marshal fail");
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
	}
	else if (CAL_PROPERTY_CHECK_DATA_TYPE(pvalue->property_id, CAL_PROPERTY_DATA_TYPE_DOUBLE) == true) {
		if (cal_ipc_marshal_double(pvalue->value.d,ipc_data) != CALENDAR_ERROR_NONE) {
			ERR("cal_ipc_marshal fail");
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
	}
	else if (CAL_PROPERTY_CHECK_DATA_TYPE(pvalue->property_id, CAL_PROPERTY_DATA_TYPE_LLI) == true) {
		if (cal_ipc_marshal_lli(pvalue->value.lli,ipc_data) != CALENDAR_ERROR_NONE) {
			ERR("cal_ipc_marshal fail");
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
	}
	else if (CAL_PROPERTY_CHECK_DATA_TYPE(pvalue->property_id, CAL_PROPERTY_DATA_TYPE_CALTIME) == true) {
		if (cal_ipc_marshal_caltime(pvalue->value.caltime,ipc_data) != CALENDAR_ERROR_NONE) {
			ERR("cal_ipc_marshal fail");
			return CALENDAR_ERROR_INVALID_PARAMETER;
		}
	}
	else {
		ERR("invalid parameter (property:%d)",pvalue->property_id);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	return CALENDAR_ERROR_NONE;
}
