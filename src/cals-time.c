#include <unicode/ucal.h>
#include <unicode/ustring.h>
#include <unicode/ustdio.h>
#include <unicode/udat.h>
#include <stdlib.h>
#include "cals-time.h"
#include "calendar-svc-provider.h"
#include "cals-internal.h"

#define ms2sec(ms) (long long int)(ms / 1000.0)
#define sec2ms(s) (s * 1000.0)

static UCalendar *_ucal_get_cal(const char *tzid)
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
		ucal_setDateTime(cal, t->year, t->month - 1, t->mday, 0, 0, 0, &status);
		if (U_FAILURE(status)) {
			ERR("ucal_setDate failed (%s)", u_errorName(status));
			return;
		}
	} else
		ERR("Invalid dtstart type. Current time is used in default");
}

long long int cals_time_diff(struct cals_time *st, struct cals_time *et)
{
	UErrorCode status = U_ZERO_ERROR;
	UCalendar *cal;
	UDate ud;
	int dr;
	if (st->type == CALS_TIME_UTIME)
		return et->utime - st->utime;

	cal = _ucal_get_cal(et->tzid);

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

long long int cals_time_diff_with_now(struct cals_time *t)
{
	long long int now;
	UErrorCode status = U_ZERO_ERROR;
	UCalendar *cal;
	UDate ud;

	if (t->type == CALS_TIME_UTIME) {
		DBG("alarm utime diff (%lld) - (%lld)", t->utime, ms2sec(ucal_getNow()));
		return t->utime - ms2sec(ucal_getNow());
	}

	cal = _ucal_get_cal(t->tzid);
	_ucal_set_time(cal, t);
	ud = ucal_getMillis(cal, &status);
	ucal_close(cal);
	now = ms2sec(ucal_getNow());
	DBG("alarm allday diff (%lld) - (%lld) ", ms2sec(ud), now);
	return ms2sec(ud) - now ;
}

char *cals_time_get_str_datetime(char *tzid, long long int t)
{
	UErrorCode status = U_ZERO_ERROR;
	UCalendar *cal;
	int y, mon, d, h, m, s;
	char buf[17] = {0};

	if (tzid == NULL) {
		tzid = CALS_TZID_0;
	}
	cal = _ucal_get_cal(tzid);

	y = ucal_get(cal, UCAL_YEAR, &status);
	mon = ucal_get(cal, UCAL_MONTH, &status) + 1;
	d = ucal_get(cal, UCAL_DATE, &status);
	h = ucal_get(cal, UCAL_HOUR, &status);
	m = ucal_get(cal, UCAL_MINUTE, &status);
	s = ucal_get(cal, UCAL_SECOND, &status);

	snprintf(buf, sizeof(buf),
			"%04d%02d%02dT%02d%02d%02dZ",
			y, mon, d, h, m, s);
	return strdup(buf);
}

long long int cals_time_convert_to_lli(struct cals_time *ct)
{
	long long int lli;
	UCalendar *cal;
	UErrorCode status = U_ZERO_ERROR;
	UChar *_tzid;

	_tzid = NULL;

	if (ct->tzid) {
		_tzid = (UChar*)malloc(sizeof(UChar) * (strlen(ct->tzid) +1));
		if (_tzid)
			u_uastrcpy(_tzid, ct->tzid);
		else
			ERR("malloc failed");
	}
DBG("tzid(%s)", ct->tzid);
	cal = ucal_open(_tzid, u_strlen(_tzid), "en_US", UCAL_TRADITIONAL, &status);
	if (_tzid)
		free(_tzid);

	if (U_FAILURE(status)) {
		ERR("ucal_open failed (%s)", u_errorName(status));
		return -1;
	}

	ucal_set(cal, UCAL_YEAR, ct->year);
	ucal_set(cal, UCAL_MONTH, ct->month - 1);
	ucal_set(cal, UCAL_DATE, ct->mday);
	ucal_set(cal, UCAL_HOUR, 0);
	ucal_set(cal, UCAL_MINUTE, 0);
	ucal_set(cal, UCAL_SECOND, 0);

	lli = (long long int)(ms2sec(ucal_getMillis(cal, &status)));

	ucal_close(cal);
	DBG("%04d/%02d/%02d %02d:%02d:%03d", ct->year, ct->month, ct->mday, 0, 0, 0);
	DBG("%lld", lli);
	return lli;
}


long long int cals_time_date_to_utime(const char *tzid,
		int year, int month, int mday, int hour, int minute, int second)
{
	UCalendar *cal;
	UErrorCode status = U_ZERO_ERROR;
	UChar *_tzid;
	long long int lli;

	cal = _ucal_get_cal(tzid);

	ucal_set(cal, UCAL_YEAR, year);
	ucal_set(cal, UCAL_MONTH, month - 1);
	ucal_set(cal, UCAL_DATE, mday);
	ucal_set(cal, UCAL_HOUR, hour);
	ucal_set(cal, UCAL_MINUTE, minute);
	ucal_set(cal, UCAL_SECOND, second);

	lli = ms2sec(ucal_getMillis(cal, &status));
	if (U_FAILURE(status)) {
		ERR("ucal_open failed (%s)", u_errorName(status));
		ucal_close(cal);
		return -1;
	}
	ucal_close(cal);
	return lli;
}

long long int cals_get_lli_now(void)
{
	return ms2sec(ucal_getNow());
}
