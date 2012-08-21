#include <calendar-svc-provider.h>
#include <stdio.h>

int main(void)
{
	int ret;
	cal_struct *cs;
	cal_iter *it;
	int id;
	int type;
	int ver;

	calendar_svc_connect();

	ret = calendar_svc_event_get_changes(1, 0, &it);
	if (ret < 0)
		return -1;

	while (calendar_svc_iter_next(it) == CAL_SUCCESS) {
		cs = NULL;
		ret = calendar_svc_iter_get_info(it, &cs);
		if (ret != CAL_SUCCESS) {
			printf("calendar_svc_iter_get_info failed (%d)\n", ret);
			return -1;
		}

		type = calendar_svc_struct_get_int(cs, CALS_STRUCT_UPDATED_INT_TYPE);
		id = calendar_svc_struct_get_int(cs, CALS_STRUCT_UPDATED_INT_ID);
		ver = calendar_svc_struct_get_int(cs, CALS_STRUCT_UPDATED_INT_VERSION);
		printf("type = %d id = %d ver = %d\n", type, id, ver);
		calendar_svc_struct_free(&cs);
	}

	calendar_svc_iter_remove(&it);

	calendar_svc_close();

	return 0;
}


