
#include <stdio.h>
#include <time.h>
#include <calendar-svc-provider.h>


int main(int argc, char **argv)
{
	int index;
	int ret;
	int cal_id, acc_id;
	char *sum;
	cal_iter *iter;
	cal_struct *cs;

	calendar_svc_connect();

	ret = calendar_svc_get_list(0, 0, CAL_STRUCT_SCHEDULE, NULL, 0, 10, &iter);
	if (ret != CAL_SUCCESS) {
		printf("Failed to get list\n");
		return -1;
	}
	while (calendar_svc_iter_next(iter) == CAL_SUCCESS) {
		cs = NULL;
		ret = calendar_svc_iter_get_info(iter, &cs);
		if (ret != CAL_SUCCESS) {
			printf("Failed to get info\n");
			break;
		}
		sum = calendar_svc_struct_get_str(cs, CAL_VALUE_TXT_SUMMARY);
		printf("summary(%s)\n", sum);
	}
	calendar_svc_iter_remove(&iter);
	calendar_svc_close();

	return 0;
}
