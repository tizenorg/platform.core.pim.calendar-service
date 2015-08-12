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

#ifndef __CAL_DB_INSTANCE_H__
#define __CAL_DB_INSTANCE_H__

int cal_db_instance_publish_record(calendar_record_h record);
int cal_db_instance_discard_record(int index);
int cal_db_instance_get_now(long long int *current);
int cal_db_instance_update_exdate_del(int id, char *exdate);

#endif /* __CAL_DB_INSTANCE_H__ */