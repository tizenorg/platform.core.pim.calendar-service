#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <calendar-svc-provider.h>
#include "utime.h"

int main(int argc, char **argv)
{
	int ret;
	int idx;
	int intv;
	long long int get_lli;
	cal_struct *cs = NULL;

	if (argc < 2)
		return -1;

	idx = atoi(argv[1]);

	calendar_svc_connect();

	calendar_svc_get(CAL_STRUCT_SCHEDULE, idx, NULL, &cs);

	get_lli = calendar_svc_struct_get_lli(cs, CALS_VALUE_LLI_DTSTART_UTIME);
	printf("%lld\n", get_lli);
	get_lli = calendar_svc_struct_get_lli(cs, CALS_VALUE_LLI_DTEND_UTIME);
	printf("%lld\n", get_lli);


	intv = calendar_svc_struct_get_int(cs, CALS_VALUE_INT_RRULE_ID);
	printf("id:%d\n", intv);
	intv = calendar_svc_struct_get_int(cs, CALS_VALUE_INT_RRULE_FREQ);
	printf("freq:%d\n", intv);

	calendar_svc_close();

	calendar_svc_struct_free(&cs);
	return ret;
}
