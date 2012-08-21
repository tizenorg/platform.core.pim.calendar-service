
#include <stdio.h>
#include <stdlib.h>
#include <calendar-svc-provider.h>

int main(int argc, char **argv)
{
	int r;
	int calendar_id;
	char *path;

	calendar_id = atoi(argv[1]);
	printf("calendar id(%d)\n", calendar_id);

	path = argv[2];
	printf("path(%s)\n", path);

	calendar_svc_connect();

	r = calendar_svc_calendar_export(calendar_id, path);
	if (r != CAL_SUCCESS) {
		printf("Failed to export schedules\n");
		return -1;
	}

	calendar_svc_close();
	return 0;
}
