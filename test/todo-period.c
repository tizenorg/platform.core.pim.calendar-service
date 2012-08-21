
#include <stdio.h>
#include <time.h>
#include <calendar-svc-provider.h>
#include "utime.h"

struct _pair {
	int i;
	char *s;
};

struct _pair is[8] = {
	{CALS_TODO_PRIORITY_NONE, "CALS_TODO_PRIORITY_NONE"},
	{CALS_TODO_PRIORITY_HIGH, "CALS_TODO_PRIORITY_HIGH"},
	{CALS_TODO_PRIORITY_MID,"CALS_TODO_PRIORITY_MID"},
	{CALS_TODO_PRIORITY_LOW,"CALS_TODO_PRIORITY_LOW"},
	{CALS_TODO_PRIORITY_HIGH | CALS_TODO_PRIORITY_MID | CALS_TODO_PRIORITY_LOW, "CALS_TODO_PRIORITY_HIGH |MID | LOW"},
	{CALS_TODO_PRIORITY_HIGH | CALS_TODO_PRIORITY_MID, "CALS_TODO_PRIORITY_HIGH | MID"},
	{CALS_TODO_PRIORITY_HIGH | CALS_TODO_PRIORITY_LOW, "CALS_TODO_PRIORITY_HIGH | LOW"},
	{CALS_TODO_PRIORITY_MID | CALS_TODO_PRIORITY_LOW, "CALS_TODO_PRIORITY_MID | LOW"}
};


int main(int argc, char **argv)
{
	int i, count;
	int ret, is_exist;;
	char *sum;
	cal_iter *iter;
	cal_struct *cs;

	calendar_svc_connect();
printf("%d\n", CALS_TODO_STATUS_NEEDS_ACTION | CALS_TODO_STATUS_COMPLETED | CALS_TODO_STATUS_IN_PROCESS | CALS_TODO_STATUS_CANCELLED);

	for (i = 0; i < 8; i++) {
		printf("start (priority:%d)%s\n", is[i].i, is[i].s);

		calendar_svc_todo_get_count_by_period(1,
//				D20120601T000000, D20120931T100000, is[i].i, -1, &count);
				200, 400, is[i].i,
				CALS_TODO_STATUS_NEEDS_ACTION | CALS_TODO_STATUS_COMPLETED |
					CALS_TODO_STATUS_IN_PROCESS | CALS_TODO_STATUS_CANCELLED,
				&count);
		if (ret != CAL_SUCCESS) {
			printf("Failed to get count\n");
			return -1;
		}
		printf("count(%d)\n", count);



		ret = calendar_svc_todo_get_list_by_period(1,
//				D20120601T000000, D20120931T100000, is[i].i, 0, &iter);
				200, 400, is[i].i,
				CALS_TODO_STATUS_NEEDS_ACTION | CALS_TODO_STATUS_COMPLETED |
					CALS_TODO_STATUS_IN_PROCESS | CALS_TODO_STATUS_CANCELLED,
				&iter);
		if (ret != CAL_SUCCESS) {
			printf("Failed to get list\n");
			break;
		}
		is_exist = 0;
		while (calendar_svc_iter_next(iter) == CAL_SUCCESS) {
			is_exist = 1;
			cs = NULL;
			calendar_svc_iter_get_info(iter, &cs);
			sum = calendar_svc_struct_get_str(cs, CAL_VALUE_TXT_SUMMARY);
			printf("sum(%s)\n", sum);
		}
		if (is_exist == 0) {
			printf("No date\n");
		}
	}
	calendar_svc_close();
	return 0;
}





