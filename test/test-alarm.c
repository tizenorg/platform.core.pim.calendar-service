#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <glib.h>
#define _GNU_SOURCE
#include <time.h>

#include <calendar-svc-provider.h>

#define ICALENAR_BUFFER_MAX 1024*1024

static const char* calendar_svc_get_stream_from_ics(const char *path)
{
	FILE *file;
	int buf_size, len;
	char *stream;
	char buf[1024];

	file = fopen(path, "r");

	len = 0;
	buf_size = ICALENAR_BUFFER_MAX;
	stream = malloc(ICALENAR_BUFFER_MAX);

	while (fgets(buf, sizeof(buf), file)) {
		if (len + sizeof(buf) < buf_size) {
			len += snprintf(stream + len, strlen(buf) +1, "%s", buf);

		} else {
			char *new_stream;
			buf_size *= 2;
			new_stream = realloc(stream, buf_size);
			if (new_stream) {
				stream = new_stream;
			} else {
				free(stream);
				fclose(file);
				printf("out of memory\n");
				return NULL;
			}
			len += snprintf(stream + len, strlen(buf) +1, "%s", buf);
		}
	}
	fclose(file);
	return stream;
}



int main(int argc, char **argv)
{
fprintf(stdout, "%s\n", __func__);
	const char *stream;
	GList *list_sch = NULL, *l;
	int i = 0;

	if (argc < 2) return -1;

	fprintf(stdout, "file path:%s\n", argv[1]);
	if (access(argv[1], F_OK) != 0) {
		fprintf(stdout, "Failed to access (%s)\n", argv[1]);
		return -1;
	}

	stream = calendar_svc_get_stream_from_ics(argv[1]);
	calendar_svc_ics_to_sch(stream, &list_sch);

	if (list_sch == NULL) {
		fprintf(stdout, "No list\n");
		return 0;
	}


	l = list_sch;
	while (l) {
		cal_struct *event;
		char *str;
		printf("%d\n", i++);
/*
		calendar_svc_get(CAL_STRUCT_SCHDULE, 1, NULL, &event);
		str = calendar_svc_struct_get_str(l->data, CAL_VALUE_TXT_SUMMARY);
		if (str) {
			printf("summary(%s)\n", str);
		}
*/
		l = g_list_next(l);
	}

//	calendar_svc_struct_free(&event);

	l = list_sch;
	while (l = g_list_next(l)) {
		if (l) {
			fprintf(stdout, "exist\n");
		}
	}


	if (stream) {
		free(stream);
	}
	return 0;
}
