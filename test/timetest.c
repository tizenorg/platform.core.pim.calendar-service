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
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include "timetest.h"
#include "test-log.h"

FILE *outfp;
double correction;

double set_start_time(void)
{
	//	TEST_FN_START;

	struct timeval tv;
	double curtime;

	gettimeofday(&tv, NULL);
	curtime = tv.tv_sec * 1000 + (double)tv.tv_usec/1000;
	return curtime;
}

double exec_time(double start)
{
	//	TEST_FN_START;

	double end = set_start_time();
	return (end - start - correction);
}

int init_time(void)
{
	//	TEST_FN_START;

	double temp_t;
	temp_t = set_start_time();
	correction = exec_time(temp_t);

	return 0;
}

int print_time(char *prn_args, double time)
{
	TEST_FN_START;

#ifdef USE_STD_OUT
	printf("%200s =\t", prn_args);
	printf("%f \n", time);
#else
	fprintf(outfp, "%.50s\t", prn_args);
	fprintf(outfp, "%f \n", time);
#endif

	return 0;
}


int print_argument(char *prn_args)
{
	TEST_FN_START;

#ifdef USE_STD_OUT
	printf("%s", prn_args);
#else
	fprintf(outfp, "%s\n", prn_args);
#endif

	return 0;
}



int print_milestone(char *prn_args, int level)
{
	TEST_FN_START;
	int i;

	if (level > 1) {
		for (i=0;i<level;i++)
			printf("\n##################################################################\n");
		printf("\n%s =\n", prn_args);
		for (i=0;i<level;i++)
			printf("\n##################################################################\n");
	}

#ifdef USE_STD_OUT
	if (1 == level)
		printf("\n##################################################################\n");
	printf("\n%s =\n", prn_args);
	printf("\n##################################################################\n");
#else
	for (i=0;i<level;i++)
		fprintf(outfp, "\n##################################################################\n");
	fprintf(outfp, "%s \n", prn_args);
	for (i=0;i<level;i++)
		fprintf(outfp, "\n##################################################################\n");
#endif


	return 0;
}

int std_output(char *prn_args, double time)
{
	TEST_FN_START;

	printf("%.50s =\t", prn_args);
	printf("%f \n", time);

	return 0;
}


int file_print_init(char *filename)
{
	TEST_FN_START;

	outfp = fopen(filename, "w"); //"aw"
	if (NULL == outfp) {
		ERR("(%s) Open Error", filename);
		return -1;

	}
	return 0;
}

