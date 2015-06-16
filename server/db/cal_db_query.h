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
#ifndef __CAL_DB_QUERY_H__
#define __CAL_DB_QUERY_H__

/*
 * bind_text->date string is only pointer copy.
 if (bind_text) {
	 g_slist_free(bind_text);
 }
 CAL_FREE(condition);
 CAL_FREE(projection);
 */
int cal_db_query_create_condition(calendar_query_h query, char **condition, GSList **bind_text);
int cal_db_query_create_projection(calendar_query_h query, char **projection);
int cal_db_query_create_order(calendar_query_h query, char *condition, char **order);
bool cal_db_query_find_projection_property(calendar_query_h query, unsigned int property);
int cal_db_query_create_projection_update_set(calendar_record_h record, char **set, GSList **bind_text);
int cal_db_query_create_projection_update_set_with_property(
		calendar_record_h record, unsigned int *properties, int properties_count,
		char **set, GSList **bind_text);

#endif /* __CAL_DB_QUERY_H__ */
