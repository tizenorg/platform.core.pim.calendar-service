#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <calendar-svc-provider.h>
#include "utime.h"

#define D19700101 62167219200
char cal_month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

long long int _mkutime(int y, int mon, int d, int h, int min, int s)
{
	int i;
	long long int t;

	t = y * 365;
	t += ((int)y/4 - (int)y/100 + (int)y/400);
	for (i = 0; i < mon-1; i++) {
		t += cal_month[i];
	}
	if (i > 2 && (y % 4 == 0)) {
		t += 1;
		if ((y % 100 == 0) && (y % 400 != 0)) {
			t -= 1;
		}
	}
	t += d;
	t *= (24 * 60 * 60);
	t += (((h * 60) + min ) * 60 + s);
	t -= D19700101;
	return t;
}

char *_mkdatetime(long long int lli)
{
	int y, mon, d, h, min, s;
	long long int t;
	static char buf[17];

	t = lli + D19700101;
	s = t % 60;
	t /= 60;
	min = t % 60;
	t /= 60;
	h = t % 24;
	t /= 24;
	y = (t * 400) / (365 * 400 + 100 - 4 + 1);
	t = t - (y * 365) - (y/4) + (y/100) - (y/400);
	mon = 0;
	while (t > cal_month[mon]) {
		if (t == 29 && mon == 1 && y % 4 == 0) {
			if (y % 100 == 0 && y % 400 != 0) {
				break;
			}
		} else {
			t -= cal_month[mon];
			mon++;
		}
	}
	mon++;
	d = t;
	snprintf(buf, sizeof(buf), "%04d/%02d/%02d %02d:%02d:%02dZ", y, mon, d, h, min, s);
	return strdup(buf);
}

int main(int argc, char **argv)
{
	int ret;
	int index = 0;
	cal_struct *cs = NULL;

	if (argc < 2) {
		printf("needs 2 arguments\n");
		return -1;
	}

	index = atoi(argv[1]);
	if (index < 0) {
		printf("Invalid index (%d)\n", index);
		return -1;
	}

	cs = calendar_svc_struct_new(CAL_STRUCT_SCHEDULE);
	if (cs == NULL) {
		printf("Failed to new calendar\n");
		return -1;
	}

	calendar_svc_struct_set_int(cs, CAL_VALUE_INT_ACCOUNT_ID, -1);
	calendar_svc_struct_set_int(cs, CAL_VALUE_INT_CALENDAR_ID, 1);

	calendar_svc_struct_set_str(cs, CAL_VALUE_TXT_SUMMARY, "event summary");
	calendar_svc_struct_set_int(cs, CALS_VALUE_INT_DTSTART_TYPE, CALS_TIME_UTIME);
	calendar_svc_struct_set_lli(cs, CALS_VALUE_LLI_DTSTART_UTIME, D20120701T000000);
	calendar_svc_struct_set_str(cs, CALS_VALUE_TXT_DTSTART_TZID, "Europe/London");
	calendar_svc_struct_set_int(cs, CALS_VALUE_INT_DTEND_TYPE, CALS_TIME_UTIME);
	calendar_svc_struct_set_lli(cs, CALS_VALUE_LLI_DTEND_UTIME, D20120701T060000);
	calendar_svc_struct_set_str(cs, CALS_VALUE_TXT_DTEND_TZID, "Europe/London");


	/* freq */

	switch (index) {

/* yearly */
	case 0: // bad
		calendar_svc_struct_set_int(cs, CALS_VALUE_INT_RRULE_FREQ, CALS_FREQ_YEARLY);
		break;

	case 1: // bad
		calendar_svc_struct_set_int(cs, CALS_VALUE_INT_RRULE_FREQ, CALS_FREQ_YEARLY);
		calendar_svc_struct_set_str(cs, CALS_VALUE_TXT_RRULE_BYMONTH, "7");
		break;

	case 2: // bad
		calendar_svc_struct_set_int(cs, CALS_VALUE_INT_RRULE_FREQ, CALS_FREQ_YEARLY);
		calendar_svc_struct_set_str(cs, CALS_VALUE_TXT_RRULE_BYWEEKNO, "5, 6");
		break;

	case 3: // bad
		calendar_svc_struct_set_int(cs, CALS_VALUE_INT_RRULE_FREQ, CALS_FREQ_YEARLY);
		calendar_svc_struct_set_str(cs, CALS_VALUE_TXT_RRULE_BYYEARDAY, "5");
		break;

	case 4:
		calendar_svc_struct_set_int(cs, CALS_VALUE_INT_RRULE_FREQ, CALS_FREQ_YEARLY);
		calendar_svc_struct_set_str(cs, CALS_VALUE_TXT_RRULE_BYMONTHDAY, "5");
		break;

/* monthly */
	case 10: // bad
		calendar_svc_struct_set_int(cs, CALS_VALUE_INT_RRULE_FREQ, CALS_FREQ_MONTHLY);
		break;

	case 11: // bad: not working bymonth
		calendar_svc_struct_set_int(cs, CALS_VALUE_INT_RRULE_FREQ, CALS_FREQ_MONTHLY);
		calendar_svc_struct_set_str(cs, CALS_VALUE_TXT_RRULE_BYMONTH, "3, 7");
		break;

	case 12: // good
		printf("every 11st, 25th in a month\n");
		calendar_svc_struct_set_int(cs, CALS_VALUE_INT_RRULE_FREQ, CALS_FREQ_MONTHLY);
		calendar_svc_struct_set_str(cs, CALS_VALUE_TXT_RRULE_BYMONTHDAY, "11, 25");
		break;

	case 13: // good
		printf("eveny 2nd Monday, 3rd Friday in a month\n");
		calendar_svc_struct_set_int(cs, CALS_VALUE_INT_RRULE_FREQ, CALS_FREQ_MONTHLY);
		calendar_svc_struct_set_str(cs, CALS_VALUE_TXT_RRULE_BYDAY, "2MO,3FR");
		break;

	case 14: // bad
		calendar_svc_struct_set_int(cs, CALS_VALUE_INT_RRULE_FREQ, CALS_FREQ_MONTHLY);
		calendar_svc_struct_set_str(cs, CALS_VALUE_TXT_RRULE_BYDAY, "2MO,3FR");
		calendar_svc_struct_set_str(cs, CALS_VALUE_TXT_RRULE_BYSETPOS, "2");
		break;

/* weekly */
	case 20: // bad
		calendar_svc_struct_set_int(cs, CALS_VALUE_INT_RRULE_FREQ, CALS_FREQ_WEEKLY);
		break;

	case 21: // bad
		calendar_svc_struct_set_int(cs, CALS_VALUE_INT_RRULE_FREQ, CALS_FREQ_WEEKLY);
		calendar_svc_struct_set_str(cs, CALS_VALUE_TXT_RRULE_BYMONTH, "3, 7");
		break;

	case 22: // good
		printf("every Saturday, Tuesday in a week\n");
		calendar_svc_struct_set_int(cs, CALS_VALUE_INT_RRULE_FREQ, CALS_FREQ_WEEKLY);
		calendar_svc_struct_set_str(cs, CALS_VALUE_TXT_RRULE_BYDAY, "SA,TU");
		break;

	case 23: // bad
		calendar_svc_struct_set_int(cs, CALS_VALUE_INT_RRULE_FREQ, CALS_FREQ_WEEKLY);
		calendar_svc_struct_set_str(cs, CALS_VALUE_TXT_RRULE_BYDAY, "SA,TU");
		calendar_svc_struct_set_str(cs, CALS_VALUE_TXT_RRULE_BYSETPOS, "2");
		break;

/* daily */
	case 30: //good
		printf("every day\n");
		calendar_svc_struct_set_int(cs, CALS_VALUE_INT_RRULE_FREQ, CALS_FREQ_DAILY);
		break;

	case 31: // bad
		calendar_svc_struct_set_int(cs, CALS_VALUE_INT_RRULE_FREQ, CALS_FREQ_DAILY);
		calendar_svc_struct_set_str(cs, CALS_VALUE_TXT_RRULE_BYMONTH, "3, 9");
		break;

	case 32: // bad
		calendar_svc_struct_set_int(cs, CALS_VALUE_INT_RRULE_FREQ, CALS_FREQ_DAILY);
		calendar_svc_struct_set_str(cs, CALS_VALUE_TXT_RRULE_BYMONTHDAY, "10, 15");
		break;

	case 33: // bad
		calendar_svc_struct_set_int(cs, CALS_VALUE_INT_RRULE_FREQ, CALS_FREQ_DAILY);
		calendar_svc_struct_set_str(cs, CALS_VALUE_TXT_RRULE_BYDAY, "SA");
		break;

	default:
		break;

	}
	calendar_svc_struct_set_int(cs, CALS_VALUE_INT_RRULE_INTERVAL, 1);

	/* until */
	calendar_svc_struct_set_int(cs, CALS_VALUE_INT_RRULE_RANGE_TYPE, CALS_RANGE_COUNT);
	calendar_svc_struct_set_int(cs, CALS_VALUE_INT_RRULE_COUNT, 4);
//	calendar_svc_struct_set_int(cs, CALS_VALUE_INT_RRULE_UNTIL_TYPE, CALS_TIME_UTIME);
//	calendar_svc_struct_set_lli(cs, CALS_VALUE_LLI_RRULE_UNTIL_UTIME, D20150701T000000);

	calendar_svc_connect();
	ret = calendar_svc_insert(cs);
	calendar_svc_struct_free(&cs);


	int cnt = 0;
	long long int dtstart_utime;
	long long int from, to;
	char *str;
	cal_iter *iter = NULL;

	from = _mkutime(2012, 1, 1, 0, 0, 0);
	to = _mkutime(2012, 12, 31, 0, 0, 0);
	calendar_svc_event_get_normal_list_by_period(1,
			CALS_LIST_PERIOD_NORMAL_ONOFF, from, to, &iter);

	while (calendar_svc_iter_next(iter) == CAL_SUCCESS) {
		cs = NULL;
		calendar_svc_iter_get_info(iter, &cs);
		dtstart_utime = calendar_svc_struct_get_lli(cs,
				CALS_LIST_PERIOD_NORMAL_ONOFF_LLI_DTSTART_UTIME);
		str = _mkdatetime(dtstart_utime);
		printf("%02d dtstart_utime(%s)\n", ++cnt, str);
		if (str) free(str);
		calendar_svc_struct_free(&cs);
	}

//	calendar_svc_delete(CAL_STRUCT_SCHEDULE, ret);
	calendar_svc_close();

	return 0;
}
