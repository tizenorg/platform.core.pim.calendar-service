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
#include <stdlib.h>

int main(int argc, char **argv)
{
	FILE *fp;
	int c;

	fp = fopen(argv[1], "r");
	if (fp == NULL)
		exit(EXIT_FAILURE);

	printf("static const char *schema_query = \"\\\n");

	do{
		c = fgetc(fp);
		switch (c)
		{
		case '\n':
			printf("\\\n");
			break;
		case '-':
			if ('-' == (c = fgetc(fp))) {
				while ('\n' != c && EOF != c)
					c = fgetc(fp);
				printf("\\\n");
			}
			else printf("-%c",c);
			break;
		case EOF:
			break;
		default:
			printf("%c",c);
			break;
		}
	}while(EOF != c);
	printf("\";\n");

	exit(EXIT_SUCCESS);
}

