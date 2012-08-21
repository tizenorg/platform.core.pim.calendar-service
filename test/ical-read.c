#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <glib.h>
#include <calendar-svc-provider.h>

static char* _get_stream(const char *path)
{
	FILE *file;
	int buf_size, len;
	char *stream;
	char buf[1024];

	file = fopen(path, "r");

	len = 0;
	buf_size = 1024;
	stream = malloc(1024);

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

static int _do_import_with_newapi(char *stream)
{
	int ret;
	char *vals;
	char buf[256];
	time_t tt;
	cal_struct *cs = NULL;
	cal_value *cv = NULL;
	GList *ls, *schedules = NULL;
	GList *lc, *categories = NULL;

	if (stream == NULL) {
		printf("Invalid argument: stream is NULL\n");
		return -1;
	}

	ret = calendar_svc_read_schedules(stream, &schedules);
	if (ret != CAL_SUCCESS) {
		printf("Failed to read schedules(errno:%d)\n", ret);
		return -1;
	}

	if (schedules == NULL) {
		printf("No schedules\n");
		return -1;
	}

	ls = schedules;
	while (ls) {
		cs = ls->data;
		if (cs == NULL) {
			ls = g_list_next(ls);
			continue;
		}
		vals = NULL;
		vals = calendar_svc_struct_get_str(cs, CAL_VALUE_TXT_SUMMARY);
		printf("summary:%s\n", vals);

		vals = calendar_svc_struct_get_str(cs, CAL_VALUE_TXT_CATEGORIES);
		printf("categories:%s\n", vals);

		tt = calendar_svc_struct_get_time(cs,
				CAL_VALUE_GMT_START_DATE_TIME, CAL_TZ_FLAG_LOCAL);
		ctime_r(&tt, buf);
		printf("stime:%s", buf);

		tt = calendar_svc_struct_get_time(cs,
				CAL_VALUE_GMT_END_DATE_TIME, CAL_TZ_FLAG_LOCAL);
		ctime_r(&tt, buf);
		printf("etime:%s", buf);

		vals = calendar_svc_struct_get_str(cs, CAL_VALUE_TXT_DESCRIPTION);
		if (vals) {
			printf("desc(%s)\n", vals);
		} else {
			printf("No description\n");
		}

		ls = g_list_next(ls);
	}
	return 0;
}

/*
 * argv[1]: file path
 */
int main(int argc, char **argv)
{
	int sel;
	char *stream = NULL;
	stream = _get_stream(argv[1]);

	_do_import_with_newapi(stream);

printf("%d\n", __LINE__);
//	if (stream) free(stream);

	return 0;
}
