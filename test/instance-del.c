
#include <stdio.h>
#include <time.h>
#include <calendar-svc-provider.h>
#include "utime.h"

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
//	calendar_svc_event_delete_normal_instance(index, 1445299200);
	calendar_svc_event_delete_allday_instance(index, 2013, 9, 22);
	calendar_svc_close();
	return 0;
}
