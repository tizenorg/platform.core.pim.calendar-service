#include <stdlib.h>
#include <glib.h>
#include <unicode/ucal.h>
#include <unicode/ustring.h>
#include <unicode/ustdio.h>
#include <unicode/udat.h>
#include <sys/types.h>
#include "cals-typedef.h"
#include "cals-internal.h"
#include "cals-instance.h"
#include "cals-sqlite.h"
#include "cals-db-info.h"
#include "cals-utils.h"

#define ms2sec(ms) (long long int)(ms / 1000.0)
#define sec2ms(s) (s * 1000.0)

/* input order
   UCAL_MONTH + UCAL_DAY_OF_MONTH
   UCAL_MONTH + UCAL_WEEK_OF_MONTH + UCAL_DAY_OF_WEEK
   UCAL_MONTH + UCAL_DAY_OF_WEEK_IN_MONTH + UCAL_DAY_OF_WEEK
   UCAL_DAY_OF_YEAR
   UCAL_DAY_OF_WEEK + UCAL_WEEK_OF_YEAR
*/

int _print_cal(UCalendar *cal)
{
	int y, m, d;
	UErrorCode status = U_ZERO_ERROR;

	y = ucal_get(cal, UCAL_YEAR, &status);
	m = ucal_get(cal, UCAL_MONTH, &status) + 1;
	d = ucal_get(cal, UCAL_DATE, &status);
	DBG("PRINT CAL %04d/%02d/%02d", y, m, d);
	return 0;
}

static int _get_max_count(cal_sch_full_t *);
static int instance_insert_yearly(int event_id,
		struct cals_time *st, int dr, cal_sch_full_t *sch);
static int instance_insert_monthly(int event_id,
		struct cals_time *st, int dr, cal_sch_full_t *sch);
static int instance_insert_weekly(int event_id,
		struct cals_time *st, int dr, cal_sch_full_t *sch);
static int instance_insert_daily(int event_id,
		struct cals_time *st, int dr, cal_sch_full_t *sch);
static int instance_insert_once(int event_id,
		struct cals_time *st, int dr, cal_sch_full_t *sch);

struct inst_info {
	int (*insert)(int, struct cals_time *, int duration, cal_sch_full_t *);
	UCalendarDateFields f;
	int max;
};

static struct inst_info inst_info[] = {
	[CALS_FREQ_YEARLY] = {instance_insert_yearly, UCAL_YEAR, 100,},
	[CALS_FREQ_MONTHLY] = {instance_insert_monthly, UCAL_MONTH, 120,},
	[CALS_FREQ_WEEKLY] = {instance_insert_weekly, UCAL_WEEK_OF_YEAR, 520,},
	[CALS_FREQ_DAILY] = {instance_insert_daily, UCAL_DATE, 3650,},
	[CALS_FREQ_HOURLY] = {instance_insert_once, 0, 1,},
	[CALS_FREQ_MINUTELY] = {instance_insert_once, 0, 1,},
	[CALS_FREQ_SECONDLY] = {instance_insert_once, 0, 1,},
	[CALS_FREQ_ONCE] = {instance_insert_once, 0, 1,},
};

struct day {
	int uday;
	const char *str;
};

enum cals_month {

	CALS_JANUARY = 1,
	CALS_FEBRUARY,
	CALS_MARCH,
	CALS_APRIL,
	CALS_MAY,
	CALS_JUNE,
	CALS_JULY,
	CALS_AUGUST,
	CALS_SEPTEMBER,
	CALS_OCTOBER,
	CALS_NOVEMBER,
	CALS_DECEMBER,
	CALS_NOMONTH = 0x100,
};

static int months[] = {
	[CALS_JANUARY] = UCAL_JANUARY,
	[CALS_FEBRUARY] = UCAL_FEBRUARY,
	[CALS_MARCH] = UCAL_MARCH,
	[CALS_APRIL] = UCAL_APRIL,
	[CALS_MAY] = UCAL_MAY,
	[CALS_JUNE] = UCAL_JUNE,
	[CALS_JULY] = UCAL_JULY,
	[CALS_AUGUST] = UCAL_AUGUST,
	[CALS_SEPTEMBER] = UCAL_SEPTEMBER,
	[CALS_OCTOBER] = UCAL_OCTOBER,
	[CALS_NOVEMBER] = UCAL_NOVEMBER,
	[CALS_DECEMBER] = UCAL_DECEMBER,
};

static struct day wdays[] = {
	[CALS_SUNDAY] = {UCAL_SUNDAY, "SU"},
	[CALS_MONDAY] = {UCAL_MONDAY, "MO"},
	[CALS_TUESDAY] = {UCAL_TUESDAY, "TU"},
	[CALS_WEDNESDAY] = {UCAL_WEDNESDAY, "WE"},
	[CALS_THURSDAY] = {UCAL_THURSDAY, "TH"},
	[CALS_FRIDAY] = {UCAL_FRIDAY, "FR"},
	[CALS_SATURDAY] = {UCAL_SATURDAY, "SA"},
};

static UCalendar *_ucal_get_cal(const char *tzid, int wkst)
{
	UCalendar *cal;
	UErrorCode status = U_ZERO_ERROR;
	UChar *_tzid;

	_tzid = NULL;

	if (tzid) {
		_tzid = (UChar*)malloc(sizeof(UChar) * (strlen(tzid) +1));
		if (_tzid)
			u_uastrcpy(_tzid, tzid);
		else
			ERR("malloc failed");
	}

	cal = ucal_open(_tzid, u_strlen(_tzid), "en_US", UCAL_TRADITIONAL, &status);
	if (_tzid)
		free(_tzid);

	if (U_FAILURE(status)) {
		ERR("ucal_open failed (%s)", u_errorName(status));
		return NULL;
	}

	if (wkst >= CALS_SUNDAY && wkst <= CALS_SATURDAY)
		ucal_setAttribute(cal, UCAL_FIRST_DAY_OF_WEEK, wdays[wkst].uday);

	return cal;
}

static void _ucal_set_time(UCalendar *cal, struct cals_time *t)
{
	UErrorCode status = U_ZERO_ERROR;

	if (t->type == CALS_TIME_UTIME) {
		ucal_setMillis(cal, sec2ms(t->utime), &status);
		if (U_FAILURE(status)) {
			ERR("ucal_setMillis failed (%s)", u_errorName(status));
			return;
		}
	} else if (t->type == CALS_TIME_LOCALTIME) {
		ucal_setDate(cal, t->year, months[t->month], t->mday, &status);
		if (U_FAILURE(status)) {
			ERR("ucal_setDate failed (%s)", u_errorName(status));
			return;
		}
	} else
		ERR("Invalid dtstart type. Current time is used in default");
}

static inline void _ucal_set_mday(UCalendar *cal, int mday)
{
	ucal_set(cal, UCAL_DATE, mday);
}

static inline void _ucal_set_month(UCalendar *cal, int month)
{
	if (month == CALS_NOMONTH)
		return;
	ucal_set(cal, UCAL_MONTH, months[month]);
}


static inline void _ucal_set_wday(UCalendar *cal, int wday)
{
	if (wday == CALS_NODAY) {
		DBG("this is mday repeat, so pass this shift");
		return;
	}
	ucal_set(cal, UCAL_DAY_OF_WEEK, wdays[wday].uday);
}

static inline void _ucal_set_week(UCalendar *cal, int week, cal_sch_full_t *sch)
{
	if (!week)
		return;

	switch (inst_info[sch->freq].f) {
	case UCAL_WEEK_OF_YEAR:
		return;
	default:
		ucal_set(cal, UCAL_DAY_OF_WEEK_IN_MONTH, week);
		break;
	}
}

static void _ucal_get_instance(UCalendar *cal,
		struct cals_time *st, struct cals_time *result)
{
	UErrorCode status = U_ZERO_ERROR;

	if (st->type == CALS_TIME_UTIME) {
		result->type = CALS_TIME_UTIME;
		result->utime = (long long int)(ms2sec(ucal_getMillis(cal, &status)));
		return;
	}

	result->type = CALS_TIME_LOCALTIME;
	result->year = ucal_get(cal, UCAL_YEAR, &status);
	result->month = ucal_get(cal, UCAL_MONTH, &status) + 1;
	result->mday = ucal_get(cal, UCAL_DATE, &status);
	DBG("get instance %04d/%02d/%02d", result->year, result->month, result->mday);

	return;
}

static inline int _is_after(struct cals_time *t1, struct cals_time *t2)
{
	if (t1->type == CALS_TIME_UTIME) {
		if (t1->utime > t2->utime)
			return 1;
		else
			return 0;
	}

	DBG("%d %d %d /%d %d %d", t1->year, t1->month, t1->mday,
			t2->year, t2->month, t2->mday);
	if (t1->year > t2->year) {
		DBG("exit year");
		return 1;
	} else if (t1->year < t2->year) {
		return 0;
	} else {
		if (t1->month > t2->month) {
			return 1;
		} else if (t1->month < t2->month) {
			return 0;
		} else {
			if (t1->mday > t2->mday) {
				return 1;
			} else {
				return 0;
			}
		}
	}
}

static inline void _set_until(struct cals_time *until, cal_sch_full_t *sch)
{
	until->type = sch->until_type;

	if (sch->range_type == CALS_RANGE_UNTIL) {
		if (sch->until_type == CALS_TIME_UTIME) {
			DBG("until utime(%lld)", sch->until_utime);
			until->utime = sch->until_utime;
		} else {
			DBG("until datetime(%04d/%02d/%02d",
					sch->until_year, sch->until_month, sch->until_mday);
			until->year = sch->until_year;
			until->month = sch->until_month;
			until->mday = sch->until_mday;
		}
		return;
	}

	until->utime = 253402300799;
	until->year = 9999;
	until->month = 12;
	until->mday = 31;
}

int _ucal_del_inundant(int event_id, struct cals_time *st, cal_sch_full_t *sch)
{
	int r;
	int cnt;
	char query[CALS_SQL_MIN_LEN];

	if (sch->range_type != CALS_RANGE_COUNT) {
		return 0;
	}

	cnt = _get_max_count(sch);

	if (st->type == CALS_TIME_UTIME) {
		snprintf(query, sizeof(query), "DELETE FROM %s "
				"WHERE event_id = %d "
				"AND dtstart_utime > (SELECT dtstart_utime FROM %s "
				"WHERE event_id = %d ORDER BY dtstart_utime LIMIT %d, 1) ",
				CALS_TABLE_NORMAL_INSTANCE,
				event_id,
				CALS_TABLE_NORMAL_INSTANCE,
				event_id, cnt -1);

	} else if (st->type == CALS_TIME_LOCALTIME) {
		snprintf(query, sizeof(query), "DELETE FROM %s "
				"WHERE event_id = %d "
				"AND dtstart_datetime > (SELECT dtstart_datetime FROM %s "
				"WHERE event_id = %d ORDER BY dtstart_datetime LIMIT %d, 1) ",
				CALS_TABLE_ALLDAY_INSTANCE,
				event_id,
				CALS_TABLE_ALLDAY_INSTANCE,
				event_id, cnt -1);
	}

	r = cals_query_exec(query);
	if (r) {
		ERR("cals_query_prepare failed (%d)", r);
		return -1;
		}
	DBG("query(%s)", query);
	return 0;
}

int _shift_to_valid_mday(UCalendar *cal, int mday, int wday, cal_sch_full_t *sch)
{
	CALS_FN_CALL;
	int y, m, d, month;
	UErrorCode status = U_ZERO_ERROR;

	if (wday != CALS_NODAY) {
		DBG("this is wday repeat, so pass this shift");
		return 0;
	}

	DBG("mday(%d)wday%d)", mday, wday);
	switch (inst_info[sch->freq].f) {
	case UCAL_YEAR:
		month = ucal_get(cal, UCAL_MONTH, &status);
		d = ucal_get(cal, UCAL_DATE, &status);
		while (d != mday && y < 9999 ) {
			ucal_add(cal, inst_info[sch->freq].f, sch->interval, &status);
			ucal_set(cal, UCAL_MONTH, month);
			ucal_set(cal, UCAL_DATE, mday);
			y = ucal_get(cal, UCAL_YEAR, &status);
			m = ucal_get(cal, UCAL_MONTH, &status);
			d = ucal_get(cal, UCAL_DATE, &status);
		}
		break;

	case UCAL_MONTH:
		d = ucal_get(cal, UCAL_DATE, &status);
		while (d != mday && y < 9999 ) {
			ucal_set(cal, UCAL_DATE, mday);
			y = ucal_get(cal, UCAL_YEAR, &status);
			m = ucal_get(cal, UCAL_MONTH, &status);
			d = ucal_get(cal, UCAL_DATE, &status);
		}
		break;
	default:
		break;
	}
	DBG("shift to %04d/%02d/%02d and should be same %02d date", y, m, d, mday);
	return 0;
}

int _shift_to_valid_wday(UCalendar *cal, int year, int month, int mday, int wday, int week, cal_sch_full_t *sch)
{
	CALS_FN_CALL;
	int y, m, d; /* date */
	UErrorCode status = U_ZERO_ERROR;

	if (!week)
		return 0;

	y = ucal_get(cal, UCAL_YEAR, &status);
	m = ucal_get(cal, UCAL_MONTH, &status) + 1;
	d = ucal_get(cal, UCAL_DATE, &status);
	DBG(" got next mday(%d) origin mday(%d)", d, mday);

	switch (inst_info[sch->freq].f) {
	case UCAL_YEAR:
		if (y <= year && m <= month && d < mday) {
			DBG("less than start date");
			ucal_add(cal, UCAL_YEAR, 1, &status);
		} else if (m < month) {
			DBG("mismatched month");
			ucal_add(cal, UCAL_MONTH, 1, &status);
		}
		_ucal_set_wday(cal, wday);
		_ucal_set_week(cal, week, sch);
		break;

	case UCAL_MONTH:
		if ((m <= month && d < mday) || (m < month)) {
			DBG(" so, added 1 week");
			ucal_add(cal, UCAL_MONTH, 1, &status);
			_ucal_set_mday(cal, 15);
			_ucal_set_wday(cal, wday);
			_ucal_set_week(cal, week, sch);
		}
		break;

	case UCAL_WEEK_OF_YEAR:
		if (y <= year && m <= month && d < mday) {
//			while ((m <= month && d < mday) || (m < month)) {
//				ucal_add(cal, UCAL_WEEK_OF_YEAR, 1, &status);
				ucal_add(cal, inst_info[sch->freq].f, sch->interval, &status);
				m = ucal_get(cal, UCAL_MONTH, &status) + 1;
				d = ucal_get(cal, UCAL_DATE, &status);
//			}
		}
		break;
	default:
		break;
	}

	_print_cal(cal);
	return 0;
}


static int _insert_instance(UCalendar *cal, int event_id,
		struct cals_time *st, int dr, int wday, int week, cal_sch_full_t *sch)
{
	CALS_FN_CALL;
	int r;
	int i;
	int cnt;
	int e_year;
	int e_month;
	int e_mday;
	int year;
	int month;
	int mday;

	UCalendar *e_cal;
	UErrorCode status = U_ZERO_ERROR;
	struct cals_time in;
	struct cals_time until;
	char query[CALS_SQL_MIN_LEN];

	r = CAL_SUCCESS;

	memset(&until, 0, sizeof(struct cals_time));
	memset(&in, 0, sizeof(struct cals_time));

	year = ucal_get(cal, UCAL_YEAR, &status);
	month = ucal_get(cal, UCAL_MONTH, &status) + 1;
	mday = ucal_get(cal, UCAL_DATE, &status);
	cnt = _get_max_count(sch);
	_set_until(&until, sch);

	for (i = 0; i < cnt; i++) {
		if (wday != CALS_NODAY) {
			DBG("set 15th for wday");
			switch (inst_info[sch->freq].f) {
			case UCAL_YEAR:
			case UCAL_MONTH:
				_ucal_set_mday(cal, 15);
				break;
			default:
				break;
			}
		}
		_ucal_set_wday(cal, wday);
		_ucal_set_week(cal, week, sch);
		_shift_to_valid_wday(cal, year, month, mday, wday, week, sch);

		_ucal_get_instance(cal, st, &in);
		if (sch->freq != CALS_FREQ_ONCE && _is_after(&in, &until)) {
			DBG("exit in is_after");
			break;
		}

		if (st->type == CALS_TIME_UTIME) {
			snprintf(query, sizeof(query), "INSERT INTO %s "
					"VALUES (%d, %lld, %lld)",
					CALS_TABLE_NORMAL_INSTANCE,
					event_id, in.utime, in.utime + dr);

		} else if (st->type == CALS_TIME_LOCALTIME) {
			if (dr > 0) {
				e_cal = ucal_clone(cal, &status);
				ucal_add(e_cal, UCAL_DATE, dr, &status);
				e_year = ucal_get(e_cal, UCAL_YEAR, &status);
				e_month = ucal_get(e_cal, UCAL_MONTH, &status) + 1;
				e_mday = ucal_get(e_cal, UCAL_DATE, &status);
				ucal_close(e_cal);
			}
			else {
				e_year = in.year;
				e_month = in.month;
				e_mday = in.mday;
			}

			snprintf(query, sizeof(query), "INSERT INTO %s "
					"VALUES (%d, %04d%02d%02d, %04d%02d%02d)",
					CALS_TABLE_ALLDAY_INSTANCE, event_id,
					in.year, in.month, in.mday,
					e_year, e_month, e_mday);
		} else {
			ERR("Invalid dtstart time type");
			return CAL_ERR_ARG_INVALID;
		}

		DBG("query(%s)", query);
		r = cals_query_exec(query);
		if (r) {
			ERR("cals_query_prepare failed (%d)", r);
			break;
		}

		ucal_add(cal, inst_info[sch->freq].f, sch->interval, &status);
		_print_cal(cal);
		_shift_to_valid_mday(cal, mday, wday, sch);
	}
	ucal_close(cal);
	return r;
}

static int _get_duration(struct cals_time *st, struct cals_time *et)
{
	UErrorCode status = U_ZERO_ERROR;
	UCalendar *cal;
	UDate ud;
	int dr;
	if (st->type == CALS_TIME_UTIME)
		return et->utime - st->utime;

	cal = _ucal_get_cal(et->tzid, -1);

	_ucal_set_time(cal, et);

	ud = ucal_getMillis(cal, &status);

	_ucal_set_time(cal, st);

	dr = ucal_getFieldDifference(cal, ud, UCAL_DATE, &status);
	if (U_FAILURE(status)) {
		ERR("ucal_getFieldDifference failed (%s)", u_errorName(status));
		return 0;
	}

	ucal_close(cal);

	return dr;
}

static inline int _get_month(const char *str)
{
	int month;

	DBG("[%s]", str);
	if (!str || !*str)
		return -1;

	month = atoi(str);
	DBG("(%d)", month);
	if (month < CALS_JANUARY || month > CALS_DECEMBER)
		return -1;

	return month;
}

static inline int _get_mday(const char *str, int *mday)
{
	int d;

	if (!str || !*str)
		return -1;

	d = atoi(str);
	if (d < 1 || d > 31)
		return -1;

	*mday = d;
	return 0;
}

int _adjust_valid_first_mday(UCalendar *cal, int year, int month, int mday, cal_sch_full_t *sch)
{
	CALS_FN_CALL;
	int caly, calm, cald;
	UErrorCode status = U_ZERO_ERROR;

	caly = ucal_get(cal, UCAL_YEAR, &status);
	calm = ucal_get(cal, UCAL_MONTH, &status) + 1;
	cald = ucal_get(cal, UCAL_DATE, &status);

	if (caly <= year && calm <= month, cald< mday) {
		switch (inst_info[sch->freq].f) {
		case UCAL_YEAR:
			ucal_add(cal, UCAL_YEAR, 1, &status);
			break;
		case UCAL_MONTH:
			ucal_add(cal, UCAL_MONTH, 1, &status);
			break;
		case UCAL_WEEK_OF_YEAR:
			ucal_add(cal, UCAL_DATE, 7, &status);
			break;
		default:
			break;
		}
	}
	return 0;
}

int _set_valid_first_mday(UCalendar *cal, struct cals_time *st, cal_sch_full_t *sch)
{
	return 0;
}

static int insert_bymday(int event_id,
		struct cals_time *st, int dr, int month, cal_sch_full_t *sch)
{
	UCalendar *cal;
	int r;
	int i;
	int y, m, d;
	int mday;
	char **t;
	const char *dl = ",";
	UErrorCode status = U_ZERO_ERROR;

	t = g_strsplit(sch->bymonthday, dl, -1);

	if (!t) {
		ERR("g_strsplit failed");
		return CAL_ERR_OUT_OF_MEMORY;
	}

	for (i = 0; t[i]; ++i) {
		r = _get_mday(t[i], &mday);
		if (r < 0) {
			ERR("Failed to get mday");
			g_strfreev(t);
			return CAL_ERR_ARG_INVALID;
		}

		cal = _ucal_get_cal(st->tzid, sch->wkst);
		if (!cal)
			return CAL_ERR_FAIL;
		_ucal_set_time(cal, st);
		y = ucal_get(cal, UCAL_YEAR, &status);
		m = ucal_get(cal, UCAL_MONTH, &status) + 1;
		d = ucal_get(cal, UCAL_DATE, &status);
		_ucal_set_month(cal, month);
		_ucal_set_mday(cal, mday);
		_adjust_valid_first_mday(cal, y, m, d, sch);

		r = _insert_instance(cal, event_id, st, dr, CALS_NODAY, 0, sch);
		if (r) {
			ERR("_insert_bymday failed (%d)", r);
			g_strfreev(t);
			return r;
		}
	}
	_ucal_del_inundant(event_id, st, sch);

	return CAL_SUCCESS;
}

static int insert_no_by(int event_id,
		struct cals_time *st, int dr, cal_sch_full_t *sch)
{
	UCalendar *cal;

	cal = _ucal_get_cal(st->tzid, sch->wkst);
	if (!cal)
		return CAL_ERR_FAIL;

	_ucal_set_time(cal, st);

	return _insert_instance(cal, event_id, st, dr, CALS_NODAY, 0, sch);
}

static inline int _get_wday(const char *str, int *week, int *wday)
{
	int i;
	int d;
	char buf[3];

	if (!str || !*str)
		return -1;

	if (!sscanf(str, "%d", &d)) {
		DBG("no digit in front of char, so set 1 as default");
		if (sscanf(str, "%s", buf) != 1) {
			ERR("Failed to get wday[%s]", str);
			return -1;
		}
		d = 1;
	} else {
		if (sscanf(str, "%d%s", &d, buf) != 2) {
			ERR("Failed to get wday[%s]", str);
			return -1;
		}
	}

	*week = d > 4 ? -1 : d;

	buf[2] = '\0';
	DBG("Sets week(%d) wday[%s]", d, buf);

	for (i = 0; i < sizeof(wdays)/sizeof(struct day); i++) {
		if (!strncmp(wdays[i].str, buf, 2)) {
			DBG("inserted week(%d) wday[%s]", *week, wdays[i].str);
			*wday = i;
			break;
		}
	}
	return 0;
}

static int insert_byday(int event_id,
		struct cals_time *st, int dr, int month, cal_sch_full_t *sch)
{
	CALS_FN_CALL;
	UCalendar *cal;
	int r;
	int i;
	int wday;
	int week;
	char **t;
	const char *d = ",";

	t = g_strsplit(sch->byday, d, -1);

	if (!t) {
		ERR("g_strsplit failed");
		return CAL_ERR_OUT_OF_MEMORY;
	}

	wday = 0;
	week = 0;

	for (i = 0; t[i]; ++i) {
		r = _get_wday(t[i], &week, &wday);
		if (r < 0) {
			ERR("Failed to get wday");
			g_strfreev(t);
			return CAL_ERR_ARG_INVALID;
		}

		cal = _ucal_get_cal(st->tzid, sch->wkst);
		if (!cal)
			return CAL_ERR_FAIL;
		_ucal_set_time(cal, st);
		_ucal_set_month(cal, month);

		r = _insert_instance(cal, event_id, st, dr, wday, week, sch);
		if (r) {
			ERR("_insert_bymday failed (%d)", r);
			g_strfreev(t);
			return r;
		}
	}

	_ucal_del_inundant(event_id, st, sch);
	g_strfreev(t);

	return CAL_SUCCESS;
}

static int instance_insert_yearly(int event_id,
		struct cals_time *st, int dr, cal_sch_full_t *sch)
{
	CALS_FN_CALL;
	int month;

	month = _get_month(sch->bymonth);
	if (month < 0) {
		ERR("_get_month failed");
		return CAL_ERR_ARG_INVALID;
	}

	if (sch->bymonthday)
		return insert_bymday(event_id, st, dr, month, sch);
	else
		return insert_byday(event_id, st, dr, month, sch);

	return CAL_ERR_ARG_INVALID;
}

static int instance_insert_monthly(int event_id,
		struct cals_time *st, int dr, cal_sch_full_t *sch)
{
	CALS_FN_CALL;
	if (sch->bymonthday)
		return insert_bymday(event_id, st, dr, CALS_NOMONTH, sch);
	else if (sch->byday)
		return insert_byday(event_id, st, dr, CALS_NOMONTH, sch);
	return CAL_ERR_ARG_INVALID;
}

static int instance_insert_daily(int event_id,
		struct cals_time *st, int dr, cal_sch_full_t *sch)
{
	CALS_FN_CALL;
	return insert_no_by(event_id, st, dr, sch);
}

static int instance_insert_weekly(int event_id,
		struct cals_time *st, int dr, cal_sch_full_t *sch)
{
	CALS_FN_CALL;
	if (sch->byday)
		return insert_byday(event_id, st, dr, CALS_NOMONTH, sch);
	return CAL_ERR_ARG_INVALID;
}

static int instance_insert_once(int event_id,
		struct cals_time *st, int dr, cal_sch_full_t *sch)
{
	CALS_FN_CALL;
	return insert_no_by(event_id, st, dr, sch);
}

static int _get_max_count(cal_sch_full_t *sch)
{
	int cnt;

	cnt = inst_info[sch->freq].max;

	if (sch->range_type == CALS_RANGE_COUNT) {
		if (sch->count < cnt)
			cnt = sch->count;
	}
	return cnt;
}

int cals_instance_insert(int event_id, struct cals_time *st,
		struct cals_time *et, cal_sch_full_t *sch)
{
	if (sch->cal_type != CALS_SCH_TYPE_EVENT) {
		DBG("Check schedule type, you're handling with type(%d)", sch->cal_type);
		return -1;
	}
	int dr = _get_duration(st, et);
	return inst_info[sch->freq].insert(event_id, st, dr, sch);
}

int cals_instance_delete(int event_id, struct cals_time *st)
{
	int r, ret;
	int type;
	sqlite3_stmt *stmt;
	char query[CALS_SQL_MIN_LEN];

	/* get type for noty */
	snprintf(query, sizeof(query), "SELECT type FROM %s WHERE id = %d ",
			CALS_TABLE_SCHEDULE, event_id);
	stmt = cals_query_prepare(query);
	if (!stmt) {
		ERR("cals_query_prepare failed");
		return CAL_ERR_DB_FAILED;
	}

	r = cals_stmt_step(stmt);
	if (r != CAL_SUCCESS) {
		sqlite3_finalize(stmt);
		ERR("cals_stmt_step failed (%d)", r);
		return r;
	}

	type = sqlite3_column_int(stmt, 0);
	sqlite3_finalize(stmt);

	/* send noti */
	ret = cals_notify(type == CALS_SCH_TYPE_EVENT ? CALS_NOTI_TYPE_EVENT : CALS_NOTI_TYPE_TODO);
	if (ret < 0) {
		WARN("cals_notify failed (%d)", ret);
	}

	/* delete instance from table */
	if (st->type == CALS_TIME_UTIME) {
		snprintf(query, sizeof(query), "DELETE FROM %s "
			"WHERE event_id = %d AND dtstart_utime = %lld",
			CALS_TABLE_NORMAL_INSTANCE, event_id, st->utime);

	} else if (st->type == CALS_TIME_LOCALTIME) {
		snprintf(query, sizeof(query), "DELETE FROM %s "
				"WHERE event_id = %d AND %04d%02d%02d",
				CALS_TABLE_ALLDAY_INSTANCE, event_id, st->year, st->month, st->mday);

	} else {
		ERR("Invalid start time type");
		return CAL_ERR_ARG_INVALID;
	}

	r = cals_query_exec(query);
	if (r)
		ERR("cals_query_exec failed");

	return r;
}

