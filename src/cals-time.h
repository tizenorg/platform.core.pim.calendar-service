#ifndef __CALENDAR_SVC_TIME_H__
#define __CALENDAR_SVC_TIME_H__

struct cals_time {
	int type;
	long long int utime;
	int year;
	int month;
	int mday;
	char tzid[15];
};

long long int cals_time_diff(struct cals_time *st, struct cals_time *et);
long long int cals_time_diff_with_now(struct cals_time *t);
char *cals_time_get_str_datetime(char *tzid, long long int t);
long long int cals_get_lli_now(void);
long long int cals_time_date_to_utime(const char *tzid,
		int year, int month, int mday, int hour, int minute, int second);

#ifdef CALS_IPC_CLIENT
long long int _date_to_utime(int y, int mon, int d, int h, int min, int s);
long long int _datetime_to_utime(char *datetime);
#endif

#define CALS_TZID_0 "Etc/Unknown"

#endif
