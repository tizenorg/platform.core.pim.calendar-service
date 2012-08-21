/*
 * Calendar Service
 *
 * Copyright (c) 2000 - 2012 Samsung Electronics Co., Ltd. All rights reserved.
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
#ifndef _TIMETEST_H_
#define _TIMETEST_H_

int init_time(void);
double set_start_time(void);
double exec_time(double start);

int print_time(char *prn_args, double time);
int print_argument(char *prn_args);
int print_milestone(char *prn_args, int level);
int std_output(char *prn_args, double time);
int file_print_init(char *filename);


#endif //_TIMETEST_H_
