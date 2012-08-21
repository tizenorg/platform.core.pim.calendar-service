
#include <stdio.h>
#include <time.h>
#include <calendar-svc-provider.h>

int main(int argc, char **argv)
{
	int ret;
	int index;

	if (argc < 2) {
		printf("needs 2 arguments\n");
		return -1;
	}

	index = atoi(argv[1]);
	if (index < 0) {
		printf("Invalid index (%d)\n", index);
		return -1;
	}

	calendar_svc_connect();
	printf("id(%d)\n", index);
	ret = calendar_svc_delete(CAL_STRUCT_SCHEDULE, index);
	if (ret != CAL_SUCCESS) {
		printf("Failed to delete\n");
		return -1;
	}
	calendar_svc_close();
	return 0;
}
