
#include <stdio.h>
#include <time.h>
#include <calendar-svc-provider.h>

int main(int argc, char **argv)
{
	int ret;
	time_t tt;
	cal_struct *cs = NULL;

	cs = calendar_svc_struct_new(CAL_STRUCT_SCHEDULE);
	if (cs == NULL) {
		printf("Failed to new calendar\n");
		return -1;
	}

	tt = time(NULL);

	calendar_svc_struct_set_int(cs, CAL_VALUE_INT_ACCOUNT_ID, 1);
	calendar_svc_struct_set_int(cs, CAL_VALUE_INT_TASK_STATUS, 0);
	calendar_svc_struct_set_int(cs, CAL_VALUE_INT_PRIORITY, 0);
	calendar_svc_struct_set_int(cs, CAL_VALUE_INT_FILE_ID, 0);
	calendar_svc_struct_set_int(cs, CAL_VALUE_INT_CONTACT_ID, 0);
	calendar_svc_struct_set_int(cs, CAL_VALUE_INT_BUSY_STATUS, 0);
	calendar_svc_struct_set_int(cs, CAL_VALUE_INT_SENSITIVITY, 0);
	calendar_svc_struct_set_int(cs, CAL_VALUE_INT_CALENDAR_TYPE, 0);
	calendar_svc_struct_set_int(cs, CAL_VALUE_INT_MEETING_STATUS, 0);
	calendar_svc_struct_set_int(cs, CAL_VALUE_INT_LOCATION_TYPE, 0);
	calendar_svc_struct_set_int(cs, CAL_VALUE_INT_CALENDAR_ID, 1);
	calendar_svc_struct_set_int(cs, CAL_VALUE_INT_SYNC_STATUS, 1);
	calendar_svc_struct_set_int(cs, CAL_VALUE_INT_DST, 0);
	calendar_svc_struct_set_int(cs, CAL_VALUE_INT_ORIGINAL_EVENT_ID, 0);
	calendar_svc_struct_set_int(cs, CAL_VALUE_INT_EMAIL_ID, 0);
	calendar_svc_struct_set_int(cs, CAL_VALUE_INT_AVAILABILITY, 0);
	calendar_svc_struct_set_int(cs, CAL_VALUE_INT_PROGRESS, 0);

	calendar_svc_struct_set_str(cs, CAL_VALUE_TXT_CATEGORIES, "event categories");
	calendar_svc_struct_set_str(cs, CAL_VALUE_TXT_SUMMARY, "event summary");
	calendar_svc_struct_set_str(cs, CAL_VALUE_TXT_DESCRIPTION, "event description");
	calendar_svc_struct_set_str(cs, CAL_VALUE_TXT_LOCATION, "event location");
	calendar_svc_struct_set_str(cs, CAL_VALUE_TXT_UID, "event uid");
	calendar_svc_struct_set_str(cs, CAL_VALUE_TXT_ORGANIZER_NAME, "event org name");
	calendar_svc_struct_set_str(cs, CAL_VALUE_TXT_ORGANIZER_EMAIL, "event org email");
	calendar_svc_struct_set_str(cs, CAL_VALUE_TXT_GCAL_ID, "event gcal id");
	calendar_svc_struct_set_str(cs, CAL_VALUE_TXT_UPDATED, "event udpated");
	calendar_svc_struct_set_str(cs, CAL_VALUE_TXT_LOCATION_SUMMARY, "event loc summary");
	calendar_svc_struct_set_str(cs, CAL_VALUE_TXT_ETAG, "event etag");
	calendar_svc_struct_set_str(cs, CAL_VALUE_TXT_EDIT_URL, "event url");
	calendar_svc_struct_set_str(cs, CAL_VALUE_TXT_GEDERID, "event gederid");

	calendar_svc_struct_set_double(cs, CAL_VALUE_DBL_LATITUDE, 0.1);
	calendar_svc_struct_set_double(cs, CAL_VALUE_DBL_LONGITUDE, 0.2);

	calendar_svc_struct_set_int(cs, CALS_VALUE_INT_DTSTART_TYPE, CALS_TIME_LOCALTIME);
	calendar_svc_struct_set_int(cs, CALS_VALUE_INT_DTSTART_YEAR, 2012);
	calendar_svc_struct_set_int(cs, CALS_VALUE_INT_DTSTART_MONTH, 7);
	calendar_svc_struct_set_int(cs, CALS_VALUE_INT_DTSTART_MDAY, 1);
	calendar_svc_struct_set_lli(cs, CALS_VALUE_LLI_DTSTART_UTIME, 1339833600);
	calendar_svc_struct_set_str(cs, CALS_VALUE_TXT_DTSTART_TZID, "Europe/London");
	calendar_svc_struct_set_int(cs, CALS_VALUE_INT_DTEND_TYPE, CALS_TIME_UTIME);
	calendar_svc_struct_set_int(cs, CALS_VALUE_INT_DTEND_YEAR, 2012);
	calendar_svc_struct_set_int(cs, CALS_VALUE_INT_DTEND_MONTH, 7);
	calendar_svc_struct_set_int(cs, CALS_VALUE_INT_DTEND_MDAY, 1);
	calendar_svc_struct_set_lli(cs, CALS_VALUE_LLI_DTEND_UTIME, 1339837200);
	calendar_svc_struct_set_str(cs, CALS_VALUE_TXT_DTEND_TZID, "Europe/London");

	calendar_svc_struct_set_int(cs, CALS_VALUE_INT_RRULE_FREQ, CALS_FREQ_MONTHLY);
	calendar_svc_struct_set_int(cs, CALS_VALUE_INT_RRULE_RANGE_TYPE, CALS_RANGE_UNTIL);
	calendar_svc_struct_set_int(cs, CALS_VALUE_INT_RRULE_UNTIL_TYPE, CALS_TIME_LOCALTIME);
	calendar_svc_struct_set_int(cs, CALS_VALUE_INT_RRULE_UNTIL_YEAR, 2013);
	calendar_svc_struct_set_int(cs, CALS_VALUE_INT_RRULE_UNTIL_MONTH, 5);
	calendar_svc_struct_set_int(cs, CALS_VALUE_INT_RRULE_UNTIL_MDAY, 1);
	calendar_svc_struct_set_lli(cs, CALS_VALUE_LLI_RRULE_UNTIL_UTIME, 1452483199);
//	calendar_svc_struct_set_int(cs, CALS_VALUE_INT_RRULE_COUNT, 6);
	calendar_svc_struct_set_int(cs, CALS_VALUE_INT_RRULE_INTERVAL, 2);
//	calendar_svc_struct_set_str(cs, CALS_VALUE_TXT_RRULE_BYSECOND, "bysecond");
//	calendar_svc_struct_set_str(cs, CALS_VALUE_TXT_RRULE_BYMINUTE, "byminute");
//	calendar_svc_struct_set_str(cs, CALS_VALUE_TXT_RRULE_BYHOUR, "byhour");
	calendar_svc_struct_set_str(cs, CALS_VALUE_TXT_RRULE_BYDAY, "3SA,3TU");
//	calendar_svc_struct_set_str(cs, CALS_VALUE_TXT_RRULE_BYDAY, "SA,TU");
//	calendar_svc_struct_set_str(cs, CALS_VALUE_TXT_RRULE_BYMONTHDAY, "4,27");
//	calendar_svc_struct_set_str(cs, CALS_VALUE_TXT_RRULE_BYYEARDAY, "byyearday");
//	calendar_svc_struct_set_str(cs, CALS_VALUE_TXT_RRULE_BYWEEKNO, "byweekno");
//	calendar_svc_struct_set_str(cs, CALS_VALUE_TXT_RRULE_BYMONTH, "7");
//	calendar_svc_struct_set_str(cs, CALS_VALUE_TXT_RRULE_BYSETPOS, "bysetpos");

	calendar_svc_connect();
	ret = calendar_svc_insert(cs);
	calendar_svc_close();

	calendar_svc_struct_free(&cs);
	return ret;
}
