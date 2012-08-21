
#include <stdio.h>
#include <stdlib.h>
#include <calendar-svc-provider.h>

/*
 * argv[1]: path
 * argv[2]: calendar_id
 */
int main(int argc, char **argv)
{
	int ret, calendar_id;
	char *path;

	if (argc < 3) {
		printf("argument needs 3\n");
		return -1;
	}

	if (argv[1] == NULL || argv[2] == NULL) {
		printf("Invalid argument\n");
		return -1;
	}

	path = argv[1];
	calendar_id = atoi(argv[2]);

	calendar_svc_connect();

	ret = calendar_svc_calendar_import(path, calendar_id);
	if (ret != CAL_SUCCESS) {
		printf("Failed to import path(%s) to calendar id(%d)\n", path, calendar_id);
		return -1;
	}

	calendar_svc_close();
	return 0;
}
