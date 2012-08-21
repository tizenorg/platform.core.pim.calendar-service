
#include <stdio.h>
#include <stdlib.h>
#include <calendar-svc-provider.h>

int main(int argc, char **argv)
{
	int ret;
	char *stream;
	GList *schedules = NULL;
	cal_struct *cs = NULL;

	cs = calendar_svc_struct_new(CAL_STRUCT_CALENDAR);
	if (cs == NULL) {
		printf("Failed to calloc\n");
		return-1;
	}
	calendar_svc_struct_set_str(cs, CAL_VALUE_TXT_SUMMARY, "title");
	/* set data in cs... */

	schedules = g_list_append(schedules, cs);

	ret = calendar_svc_write_schedules(schedules, &stream);
	if (ret != CAL_SUCCESS) {
		printf("Failed to read schedules(errno:%d)\n", ret);
		return -1;
	}

	if (stream == NULL) {
		printf("stream is NULL\n");
		return -1;
	}

	return 0;
}
