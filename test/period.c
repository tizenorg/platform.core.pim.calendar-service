
#include <stdio.h>
#include <time.h>
#include <calendar-svc-provider.h>

#define D20120601T000000 1338508800
#define D20120931T100000 1349085600

int _get_iter_period_normal_onoff(int calendar_id, long long int llis, long long int llie)
{
	int index;
	int is_exist = 0;
	long long int s;
	cal_iter *iter;
	cal_struct *cs;

	calendar_svc_event_get_normal_list_by_period(calendar_id, CALS_LIST_PERIOD_NORMAL_ONOFF,
			llis, llie, &iter);
	while (calendar_svc_iter_next(iter) == CAL_SUCCESS) {
		is_exist = 1;
		cs = NULL;
		calendar_svc_iter_get_info(iter, &cs);
		index = calendar_svc_struct_get_int(cs, CALS_LIST_PERIOD_NORMAL_ONOFF_INT_EVENTID);
		s = calendar_svc_struct_get_lli(cs, CALS_LIST_PERIOD_NORMAL_ONOFF_LLI_DTSTART_UTIME);
		printf("index(%d)dtstart utime:%lld\n", index, s);
	}
	return is_exist;
}

int _get_iter_period_allday_onoff(int calendar_id,
		int sy, int sm, int sd, int ey, int em, int ed)
{
	int index;
	int dtstart_year, dtstart_month, dtstart_mday;
	int is_exist = 0;
	cal_iter *iter;
	cal_struct *cs;

	calendar_svc_event_get_allday_list_by_period(calendar_id, CALS_LIST_PERIOD_ALLDAY_ONOFF,
			sy, sm, sd, ey, em, ed, &iter);
	while (calendar_svc_iter_next(iter) == CAL_SUCCESS) {
		is_exist = 1;
		cs = NULL;
		calendar_svc_iter_get_info(iter, &cs);
		index = calendar_svc_struct_get_int(cs, CALS_LIST_PERIOD_ALLDAY_ONOFF_INT_EVENTID);
		dtstart_year = calendar_svc_struct_get_int(cs, CALS_LIST_PERIOD_ALLDAY_ONOFF_INT_DTSTART_YEAR);
		dtstart_month = calendar_svc_struct_get_int(cs, CALS_LIST_PERIOD_ALLDAY_ONOFF_INT_DTSTART_MONTH);
		dtstart_mday = calendar_svc_struct_get_int(cs, CALS_LIST_PERIOD_ALLDAY_ONOFF_INT_DTSTART_MDAY);

		printf("index(%d)dtstart datetime(%04d/%02d/%02d)\n", index, dtstart_year, dtstart_month, dtstart_mday);
	}
	return is_exist;
}

int _get_iter_period_normal_basic(int calendar_id, long long int llis, long long int llie)
{
	int index;
	int is_exist = 0;
	char *sum, *loc;
	long long int s, e;
	cal_iter *iter;
	cal_struct *cs;

	calendar_svc_event_get_normal_list_by_period(calendar_id, CALS_LIST_PERIOD_NORMAL_BASIC,
			llis, llie, &iter);
	while (calendar_svc_iter_next(iter) == CAL_SUCCESS) {
		is_exist = 1;
		cs = NULL;
		calendar_svc_iter_get_info(iter, &cs);
		index = calendar_svc_struct_get_int(cs, CALS_LIST_PERIOD_NORMAL_BASIC_INT_EVENTID);
		s = calendar_svc_struct_get_lli(cs, CALS_LIST_PERIOD_NORMAL_BASIC_LLI_DTSTART_UTIME);
		e = calendar_svc_struct_get_lli(cs, CALS_LIST_PERIOD_NORMAL_BASIC_LLI_DTEND_UTIME);
		sum = calendar_svc_struct_get_str(cs, CALS_LIST_PERIOD_NORMAL_BASIC_TXT_SUMMARY);
		loc = calendar_svc_struct_get_str(cs, CALS_LIST_PERIOD_NORMAL_BASIC_TXT_LOCATION);
		printf("index(%d) s(%lld) e(%lld), sum(%s) loc(%s)\n", index, s, e, sum, loc);
	}
	return is_exist;
}

int _get_iter_period_allday_basic(int calendar_id,
		int sy, int sm, int sd, int ey, int em, int ed)
{
	int index;
	int dtstart_year, dtstart_month, dtstart_mday;
	int dtend_year, dtend_month, dtend_mday;
	int is_exist = 0;
	char *sum, *loc;
	cal_iter *iter;
	cal_struct *cs;

	calendar_svc_event_get_allday_list_by_period(calendar_id, CALS_LIST_PERIOD_ALLDAY_BASIC,
			sy, sm, sd, ey, em, ed, &iter);
	while (calendar_svc_iter_next(iter) == CAL_SUCCESS) {
		is_exist = 1;
		cs = NULL;
		calendar_svc_iter_get_info(iter, &cs);
		index = calendar_svc_struct_get_int(cs, CALS_LIST_PERIOD_ALLDAY_BASIC_INT_EVENTID);
		dtstart_year = calendar_svc_struct_get_int(cs, CALS_LIST_PERIOD_ALLDAY_BASIC_INT_DTSTART_YEAR);
		dtstart_month = calendar_svc_struct_get_int(cs, CALS_LIST_PERIOD_ALLDAY_BASIC_INT_DTSTART_MONTH);
		dtstart_mday = calendar_svc_struct_get_int(cs, CALS_LIST_PERIOD_ALLDAY_BASIC_INT_DTSTART_MDAY);
		dtend_year = calendar_svc_struct_get_int(cs, CALS_LIST_PERIOD_ALLDAY_BASIC_INT_DTEND_YEAR);
		dtend_month = calendar_svc_struct_get_int(cs, CALS_LIST_PERIOD_ALLDAY_BASIC_INT_DTEND_MONTH);
		dtend_mday = calendar_svc_struct_get_int(cs, CALS_LIST_PERIOD_ALLDAY_BASIC_INT_DTEND_MDAY);
		sum = calendar_svc_struct_get_str(cs, CALS_LIST_PERIOD_ALLDAY_BASIC_TXT_SUMMARY);
		loc = calendar_svc_struct_get_str(cs, CALS_LIST_PERIOD_ALLDAY_BASIC_TXT_LOCATION);

		printf("index(%d) (%04d/%02d/%02d) - (%04d/%02d/%02d)sum(%s) loc(%s)\n",
				index,
				dtstart_year, dtstart_month, dtstart_mday,
				dtend_year, dtend_month, dtend_mday,
				sum, loc);
	}
	return is_exist;
}



int _get_iter_period_normal_osp(int calendar_id, long long int llis, long long int llie)
{
	int index, cal_id, busy, meeting, prio, sensi;
	int is_exist = 0;
	char *sum, *loc, *dsc;
	long long int s, e;
	cal_iter *iter;
	cal_struct *cs;

	calendar_svc_event_get_normal_list_by_period(calendar_id, CALS_LIST_PERIOD_NORMAL_OSP,
			llis, llie, &iter);
	while (calendar_svc_iter_next(iter) == CAL_SUCCESS) {
		is_exist = 1;
		cs = NULL;
		calendar_svc_iter_get_info(iter, &cs);
		index = calendar_svc_struct_get_int(cs, CALS_LIST_PERIOD_NORMAL_OSP_INT_EVENTID);
		cal_id = calendar_svc_struct_get_int(cs, CALS_LIST_PERIOD_NORMAL_OSP_INT_CALENDAR_ID);
		busy = calendar_svc_struct_get_int(cs, CALS_LIST_PERIOD_NORMAL_OSP_INT_BUSY_STATUS);
		meeting = calendar_svc_struct_get_int(cs, CALS_LIST_PERIOD_NORMAL_OSP_INT_STATUS);
		prio = calendar_svc_struct_get_int(cs, CALS_LIST_PERIOD_NORMAL_OSP_INT_PRIORITY);
		sensi = calendar_svc_struct_get_int(cs, CALS_LIST_PERIOD_NORMAL_OSP_INT_VISIBILITY);
		s = calendar_svc_struct_get_lli(cs, CALS_LIST_PERIOD_NORMAL_OSP_LLI_DTSTART_UTIME);
		e = calendar_svc_struct_get_lli(cs, CALS_LIST_PERIOD_NORMAL_OSP_LLI_DTEND_UTIME);
		sum = calendar_svc_struct_get_str(cs, CALS_LIST_PERIOD_NORMAL_OSP_TXT_SUMMARY);
		loc = calendar_svc_struct_get_str(cs, CALS_LIST_PERIOD_NORMAL_OSP_TXT_LOCATION);
		dsc = calendar_svc_struct_get_str(cs, CALS_LIST_PERIOD_NORMAL_OSP_TXT_DESCRIPTION);
		printf("index(%d) s(%lld) e(%lld), sum(%s) loc(%s) "
				"dsc(%s) cal_id(%d) busy(%d) meeting(%d) prio(%d) sensi(%d)\n",
				index, s, e, sum, loc, dsc, cal_id, busy, meeting, prio, sensi);
	}
	return is_exist;
}

int _get_iter_period_allday_osp(int calendar_id,
		int sy, int sm, int sd, int ey, int em, int ed)
{
	int index, cal_id, busy, meeting, prio, sensi;
	int dtstart_year, dtstart_month, dtstart_mday;
	int dtend_year, dtend_month, dtend_mday;
	int is_exist = 0;
	char *sum, *loc, *dsc;
	cal_iter *iter;
	cal_struct *cs;

	calendar_svc_event_get_allday_list_by_period(calendar_id, CALS_LIST_PERIOD_ALLDAY_OSP,
			sy, sm, sd, ey, em, ed, &iter);
	while (calendar_svc_iter_next(iter) == CAL_SUCCESS) {
		is_exist = 1;
		cs = NULL;
		calendar_svc_iter_get_info(iter, &cs);
		index = calendar_svc_struct_get_int(cs, CALS_LIST_PERIOD_ALLDAY_OSP_INT_EVENTID);
		dtstart_year = calendar_svc_struct_get_int(cs, CALS_LIST_PERIOD_ALLDAY_OSP_INT_DTSTART_YEAR);
		dtstart_month = calendar_svc_struct_get_int(cs, CALS_LIST_PERIOD_ALLDAY_OSP_INT_DTSTART_MONTH);
		dtstart_mday = calendar_svc_struct_get_int(cs, CALS_LIST_PERIOD_ALLDAY_OSP_INT_DTSTART_MDAY);
		dtend_year = calendar_svc_struct_get_int(cs, CALS_LIST_PERIOD_ALLDAY_OSP_INT_DTEND_YEAR);
		dtend_month = calendar_svc_struct_get_int(cs, CALS_LIST_PERIOD_ALLDAY_OSP_INT_DTEND_MONTH);
		dtend_mday = calendar_svc_struct_get_int(cs, CALS_LIST_PERIOD_ALLDAY_OSP_INT_DTEND_MDAY);
		sum = calendar_svc_struct_get_str(cs, CALS_LIST_PERIOD_ALLDAY_OSP_TXT_SUMMARY);
		loc = calendar_svc_struct_get_str(cs, CALS_LIST_PERIOD_ALLDAY_OSP_TXT_LOCATION);
		cal_id = calendar_svc_struct_get_int(cs, CALS_LIST_PERIOD_ALLDAY_OSP_INT_CALENDAR_ID);
		busy = calendar_svc_struct_get_int(cs, CALS_LIST_PERIOD_ALLDAY_OSP_INT_BUSY_STATUS);
		meeting = calendar_svc_struct_get_int(cs, CALS_LIST_PERIOD_ALLDAY_OSP_INT_STATUS);
		prio = calendar_svc_struct_get_int(cs, CALS_LIST_PERIOD_ALLDAY_OSP_INT_PRIORITY);
		sensi = calendar_svc_struct_get_int(cs, CALS_LIST_PERIOD_ALLDAY_OSP_INT_VISIBILITY);
		dsc = calendar_svc_struct_get_str(cs, CALS_LIST_PERIOD_ALLDAY_OSP_TXT_DESCRIPTION);

		printf("index(%d) (%04d/%02d/%02d) - (%04d/%02d/%02d)sum(%s) loc(%s) "
				"dsc(%s) cal_id(%d) busy(%d) meeting(%d) prio(%d) sensi(%d)\n",
				index,
				dtstart_year, dtstart_month, dtstart_mday,
				dtend_year, dtend_month, dtend_mday,
				sum, loc,
				dsc, cal_id, busy, meeting, prio, sensi);
	}
	return is_exist;
}

int main(int argc, char **argv)
{
	int index;
	int is_exist;;
	long long int lli;
	cal_iter *iter;
	cal_struct *cs;

	calendar_svc_connect();

	printf("======= CALS_LIST_PERIOD_NORMAL_ONOFF ======\n");
	printf("\nall calendar(-1)\n");
	is_exist = _get_iter_period_normal_onoff(-1, D20120601T000000, D20120931T100000);
	if (is_exist == 0) {
		printf("no data exist\n");
	}

	printf("\ncalendar(1)\n");
	is_exist = _get_iter_period_normal_onoff(1, D20120601T000000, D20120931T100000);
	if (is_exist == 0) {
		printf("no data exist\n");
	}

	printf("\ncalendar(2)\n");
	is_exist = _get_iter_period_normal_onoff(2, D20120601T000000, D20120931T100000);
	if (is_exist == 0) {
		printf("no data exist\n");
	}

	printf("======= CALS_LIST_PERIOD_ALLDAY_ONOFF ======\n");
	printf("\nall calendar(-1)\n");
	is_exist = _get_iter_period_allday_onoff(-1, 2012, 6, 1, 2012, 9, 31);
	if (is_exist == 0) {
		printf("no data exist\n");
	}

	printf("\ncalendar(1)\n");
	is_exist = _get_iter_period_allday_onoff(1, 2012, 6, 1, 2012, 9, 31);
	if (is_exist == 0) {
		printf("no data exist\n");
	}

	printf("\ncalendar(2)\n");
	is_exist = _get_iter_period_allday_onoff(2, 2012, 6, 1, 2012, 9, 31);
	if (is_exist == 0) {
		printf("no data exist\n");
	}

	printf("======= CALS_LIST_PERIOD_NORMAL_BASIC ======\n");
	printf("\nall calendar(-1)\n");
	is_exist = _get_iter_period_normal_basic(-1, D20120601T000000, D20120931T100000);
	if (is_exist == 0) {
		printf("no data exist\n");
	}

	printf("\ncalendar(1)\n");
	is_exist = _get_iter_period_normal_basic(1, D20120601T000000, D20120931T100000);
	if (is_exist == 0) {
		printf("no data exist\n");
	}

	printf("\ncalendar(2)\n");
	is_exist = _get_iter_period_normal_basic(2, D20120601T000000, D20120931T100000);
	if (is_exist == 0) {
		printf("no data exist\n");
	}

	printf("======= CALS_LIST_PERIOD_ALLDAY_BASIC ======\n");
	printf("\nall calendar(-1)\n");
	is_exist = _get_iter_period_allday_basic(-1, 2012, 6, 1, 2012, 9, 31);
	if (is_exist == 0) {
		printf("no data exist\n");
	}

	printf("\ncalendar(1)\n");
	is_exist = _get_iter_period_allday_basic(1, 2012, 6, 1, 2012, 9, 31);
	if (is_exist == 0) {
		printf("no data exist\n");
	}

	printf("\ncalendar(2)\n");
	is_exist = _get_iter_period_allday_basic(2, 2012, 6, 1, 2012, 9, 31);
	if (is_exist == 0) {
		printf("no data exist\n");
	}

	printf("======= CALS_LIST_PERIOD_NORMAL_OSP ======\n");
	printf("\nall calendar(-1)\n");
	is_exist = _get_iter_period_normal_osp(-1, D20120601T000000, D20120931T100000);
	if (is_exist == 0) {
		printf("no data exist\n");
	}

	printf("\ncalendar(1)\n");
	is_exist = _get_iter_period_normal_osp(1, D20120601T000000, D20120931T100000);
	if (is_exist == 0) {
		printf("no data exist\n");
	}

	printf("\ncalendar(2)\n");
	is_exist = _get_iter_period_normal_osp(2, D20120601T000000, D20120931T100000);
	if (is_exist == 0) {
		printf("no data exist\n");
	}

	printf("======= CALS_LIST_PERIOD_ALLDAY_OSP ======\n");
	printf("\nall calendar(-1)\n");
	is_exist = _get_iter_period_allday_osp(-1, 2012, 6, 1, 2012, 9, 31);
	if (is_exist == 0) {
		printf("no data exist\n");
	}

	printf("\ncalendar(1)\n");
	is_exist = _get_iter_period_allday_osp(1, 2012, 6, 1, 2012, 9, 31);
	if (is_exist == 0) {
		printf("no data exist\n");
	}

	printf("\ncalendar(2)\n");
	is_exist = _get_iter_period_allday_osp(2, 2012, 6, 1, 2012, 9, 31);
	if (is_exist == 0) {
		printf("no data exist\n");
	}



	long long int ts;
	int i, y, m, d;
	char mon[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	long long int th = 62167219200;

	y = 1970;
	m = 1;
	d = 1;

	ts = y*365;
	printf("%lld\n", ts);
	ts = ts + (int)y/4 - (int)y/100 + (int)y/400;
	printf("%lld\n", ts);
	for (i = 0; i < m-1; i++) {
		ts += mon[i];
	}
	ts = ts + d;
	printf("%lld\n", ts);
	ts = ts * 24;
	printf("%lld\n", ts);
	ts = ts * 60;
	printf("%lld\n", ts);
	ts = ts * 60;
	printf("bb:%lld\n", ts);
	th = ts;


	y = 2012;
	m = 6;
	d = 1;

	ts = y*365;
	printf("%lld\n", ts);
	ts = ts + (int)y/4 - (int)y/100 + (int)y/400;
	printf("%lld\n", ts);
	for (i = 0; i < m-1; i++) {
		ts += mon[i];
	}
	ts = ts + d;
	printf("%lld\n", ts);
	ts = ts * 24;
	printf("%lld\n", ts);
	ts = ts * 60;
	printf("%lld\n", ts);
	ts = ts * 60;
	printf("%lld\n", ts);
	ts = ts - 62167219200;
	printf("aa%lld\n", ts);

	ts = 1338508800;
	ts = ts / 60;
	printf("%lld\n", ts);
	ts = ts / 60;
	printf("%lld\n", ts);
	ts = ts / 24;
	printf("%lld\n", ts);

	printf("%lld\n", ts / 365);
	printf("%lld\n", ts % 365);

	ts = ts - mon[m-1] - d;
	printf("%lld\n", ts);
	ts = ts - y/4 + y/100 - y/400;
	printf("%lld\n", ts);

	calendar_svc_close();
	return 0;
}
