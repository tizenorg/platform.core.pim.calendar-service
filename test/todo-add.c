
#include <stdio.h>
#include <time.h>
#include <calendar-svc-provider.h>
#include "utime.h"

int main(int argc, char **argv)
{
	int i;
	int ret;
	char buf[32] = {0};
	cal_struct *cs = NULL;

	calendar_svc_connect();

	for (i = 0; i < 10; i++) {
		printf("start insert priority:%d\n", i);
		cs = calendar_svc_struct_new(CAL_STRUCT_TODO);
		if (cs == NULL) {
			printf("Failed to new calendar\n");
			return -1;
		}

		calendar_svc_struct_set_int(cs, CAL_VALUE_INT_ACCOUNT_ID, 1);
		calendar_svc_struct_set_int(cs, CAL_VALUE_INT_TASK_STATUS, 1);
		calendar_svc_struct_set_int(cs, CAL_VALUE_INT_CALENDAR_TYPE, 2);
		calendar_svc_struct_set_int(cs, CAL_VALUE_INT_CALENDAR_ID, 1);
		calendar_svc_struct_set_int(cs, CAL_VALUE_INT_PRIORITY, i);
		snprintf(buf, sizeof(buf), "status:1 /priority:%d", i);
		calendar_svc_struct_set_str(cs, CAL_VALUE_TXT_SUMMARY, buf);

		calendar_svc_struct_set_int(cs, CALS_VALUE_INT_DTSTART_TYPE, CALS_TIME_UTIME);
		calendar_svc_struct_set_lli(cs, CALS_VALUE_LLI_DTSTART_UTIME, D20120701T000000);
		calendar_svc_struct_set_str(cs, CALS_VALUE_TXT_DTSTART_TZID, "Europe/London");
		calendar_svc_struct_set_int(cs, CALS_VALUE_INT_DTEND_TYPE, CALS_TIME_UTIME);
		calendar_svc_struct_set_lli(cs, CALS_VALUE_LLI_DTEND_UTIME, D20120901T000000);
		calendar_svc_struct_set_str(cs, CALS_VALUE_TXT_DTEND_TZID, "Europe/London");

		ret = calendar_svc_insert(cs);
		calendar_svc_struct_free(&cs);
	}
	calendar_svc_close();

	return 0;
}



