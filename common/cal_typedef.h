/*
 * Calendar Service
 *
 * Copyright (c) 2012 - 2015 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef __CALENDAR_SVC_TYPEDEF_H__
#define __CALENDAR_SVC_TYPEDEF_H__

#include <glib.h>
#include <complex.h>
#include <stdbool.h>
#include <stdint.h>
#include "cal_record.h"
#include <tzplatform_config.h>

#define CAL_STR_SHORT_LEN32 32
#define CAL_STR_SHORT_LEN64 64
#define CAL_STR_MIDDLE_LEN 1024
#define CAL_STR_LONG_LEN 2048
#define CAL_STRING_EQUAL 0

#define CAL_TZID_GMT "Etc/GMT"
#define CAL_SOCK_PATH "/run/user/%d"
#define CAL_NOTI_EVENT_CHANGED tzplatform_mkpath(TZ_USER_DATA,"calendar-svc/.CALENDAR_SVC_EVENT_CHANGED")
#define CAL_NOTI_TODO_CHANGED tzplatform_mkpath(TZ_USER_DATA,"calendar-svc/.CALENDAR_SVC_TODO_CHANGED")
#define CAL_NOTI_CALENDAR_CHANGED tzplatform_mkpath(TZ_USER_DATA,"calendar-svc/.CALENDAR_SVC_CALENDAR_CHANGED")
#define CAL_NOTI_IPC_READY tzplatform_mkpath(TZ_USER_DATA,"calendar-svc/.CALENDAR_SVC_IPC_READY")
#define CAL_NOTI_REMINDER_CAHNGED "reminder"
#define CAL_FORMAT_LOCAL_DATETIME "%04d-%02d-%02dT%02d:%02d:%02d"
#define CAL_DATETIME_FORMAT_YYYYMMDD "%04d%02d%02d"
#define CAL_DATETIME_FORMAT_YYYYMMDDTHHMMSS "%04d%02d%02dT%02d%02d%02d"
#define CAL_DATETIME_FORMAT_YYYYMMDDTHHMMSSZ "%04d%02d%02dT%02d%02d%02dZ"

/**
 * @enum cal_priority_e
 * This enumeration defines priority for todo data.
 */
typedef enum
{
	CAL_PRIORITY_LOW,	/**< priority low */
	CAL_PRIORITY_MID,	/**< priority middle */
	CAL_PRIORITY_HIGH	/**< priority high */
}cal_priority_e;

typedef enum
{
	CAL_PERMISSION_NONE = 0x00,
	CAL_PERMISSION_READ = 0x01,
	CAL_PERMISSION_WRITE = 0x02,
} cal_permission_e;

#define LOCAL_ACCOUNT_ID -1

#define CAL_INVALID_ID				(-1)

typedef enum
{
	CAL_SCH_TYPE_NONE=0,           /**< None type */
	CAL_SCH_TYPE_EVENT,    /**< schedule event type */
	CAL_SCH_TYPE_TODO,     /**< task event type */
	CAL_SCH_TYPE_MAX,      /**< max type */
} cal_sch_type_e;

typedef struct {
	int count;
	GList *record;
	GList *cursor;
} cal_list_s;

/**
 * This structure defines schedule information.
 */
typedef struct
{
	cal_record_s common;
	int index;				/**< Record index */
	int calendar_id;

	char *summary;			/**< Summary, appointment, task: subject, birthday:Name */
	char *description;		/**< Description,appointment, task: description, anniversary,holiday:occasion*/
	char *location;			/**< Location */
	char *categories;
	char *exdate;

	calendar_event_status_e event_status;		/**< current task status */
	cal_priority_e priority;		/**< Priority */
	int timezone;			/**< timezone of task */

	int contact_id;			/**< contact id for birthday in contact list */

	int busy_status;		/**< ACS, G : Flag of busy or not */
	int sensitivity;		/**< ACS, G : The sensitivity (public, private, confidential). #cal_visibility_type_t*/
	int meeting_status;		/**< ACS, G : The status of the meeting. */
	char *uid;				/**< ACS, G : Unique ID of the meeting item */
	char *organizer_name;		/**< ACS, G : Name of organizer(author) */
	char *organizer_email;	/**< ACS, G : Email of organizer */

	int original_event_id;        /**< original event id for recurrency exception */
	double latitude;
	double longitude;
	int email_id;
	long long int created_time;
	int is_deleted; /**< for sync */
	long long int last_mod;
	int freq;
	int range_type;
	calendar_time_s until;
	int count;
	int interval;
	char *bysecond;
	char *byminute;
	char *byhour;
	char *byday;
	char *bymonthday;
	char *byyearday;
	char *byweekno;
	char *bymonth;
	char *bysetpos;
	int wkst;
	char *recurrence_id;
	char *rdate;
	int has_attendee;
	int has_alarm;
	int system_type;
	long updated;
	char *sync_data1;
	char *sync_data2;
	char *sync_data3;
	char *sync_data4;

	calendar_time_s start;
	char* start_tzid;
	calendar_time_s end;
	char* end_tzid;
	int is_allday;
	cal_list_s *alarm_list;
	cal_list_s *attendee_list;
	cal_list_s *exception_list;
	cal_list_s *extended_list;
}cal_event_s;

typedef struct
{
	cal_record_s common;
	int index;
	int calendar_id;
	char *summary;
	char *description;
	char *location;
	char *categories;
	calendar_todo_status_e todo_status;
	cal_priority_e priority;
	int sensitivity;
	char *uid;
	double latitude;
	double longitude;
	long long int created_time;
	long long int completed_time;
	int progress;
	int is_deleted; /**< for sync */
	long long int last_mod;
	int freq;
	int range_type;
	calendar_time_s until;
	int count;
	int interval;
	char *bysecond;
	char *byminute;
	char *byhour;
	char *byday;
	char *bymonthday;
	char *byyearday;
	char *byweekno;
	char *bymonth;
	char *bysetpos;
	int wkst;
	int has_alarm;
	int system_type;
	long updated;
	char *sync_data1;
	char *sync_data2;
	char *sync_data3;
	char *sync_data4;
	calendar_time_s start;
	char* start_tzid;
	calendar_time_s due;
	char* due_tzid;
	char *organizer_name;
	char *organizer_email;
	int has_attendee;
	int is_allday;

	cal_list_s *alarm_list;
	cal_list_s *attendee_list;
	cal_list_s *extended_list;
}cal_todo_s;

typedef struct
{
	int freq;
	int range_type;
	calendar_time_s until;
	int count;
	int interval;
	char *bysecond;
	char *byminute;
	char *byhour;
	char *byday;
	char *bymonthday;
	char *byyearday;
	char *byweekno;
	char *bymonth;
	char *bysetpos;
	int wkst;
}cal_rrule_s;

/**
 * This structure defines participant information of a meetting.
 * ical: cutype, member, role, partstat, rsvp, delto, delfrom, sentby, cn,  dir, language
 */
typedef struct
{
	cal_record_s common;
	int id; /* Internal property. Do not add to view_uri property */
	int parent_id;
	char *attendee_number;
	int attendee_cutype;
	int attendee_ct_index;
	char *attendee_uid;

	/* ical spec from here */
	char *attendee_group;	/* cutype */
	char *attendee_email;	/* member */
	int attendee_role;		/* role */
	int attendee_status;	/* partstat: ACCEPTED, DECLINED.. */
	int attendee_rsvp;		/* rsvp */
	char *attendee_delegatee_uri;	/* delfrom */
	char *attendee_delegator_uri;	/* delto */
	/* sentby */
	char *attendee_name;	/* cn */
	char *attendee_member;	/* member */

}cal_attendee_s;

/**
 * This structure defines exception information of alarm.
 */
typedef struct
{
	cal_record_s common;
	int id; /* Internal property. Do not add to view_uri property */
	int parent_id;
	int is_deleted;

	int remind_tick;
	int remind_tick_unit;

	char *alarm_description;			/**< Alert description */
	char *alarm_summary;
	int alarm_action;
	char *alarm_attach;
	calendar_time_s alarm;
}cal_alarm_s;

/* This is the calendar schema */
typedef struct
{
	cal_record_s common;
	int index;
	int store_type;
	char *uid;
	long updated;
	char *name;
	char *description;
	char *color;
	char *location;
	int visibility;
	int sync_event;
	int is_deleted;
	int account_id;
	char *sync_data1;
	char *sync_data2;
	char *sync_data3;
	char *sync_data4;
	int mode;
} cal_book_s;


/* type for timezone information save */
typedef struct
{
	cal_record_s common;
	int index;
	int tz_offset_from_gmt;

	char *standard_name;
	int std_start_month;
	int std_start_position_of_week;
	int std_start_day;
	int std_start_hour;
	int standard_bias;

	char *day_light_name;
	int day_light_start_month;
	int day_light_start_position_of_week;
	int day_light_start_day;
	int day_light_start_hour;
	int day_light_bias;
	int calendar_id;
} cal_timezone_s;

typedef struct
{
	cal_record_s common;
	int event_id;
	int calendar_id;
	calendar_time_s start;
	calendar_time_s end;
	char *summary;
	char *description;
	char *location;
	int busy_status;
	int event_status;
	int priority;
	int sensitivity;
	int has_rrule;  /* rrule_id */
	double latitude;
	double longitude;
	int has_alarm;
	int original_event_id;
	long long int last_mod;
	char *sync_data1;
} cal_instance_normal_s;

typedef struct
{
	cal_record_s common;
	int event_id;
	int calendar_id;
	calendar_time_s start;
	calendar_time_s end;
	char *summary;
	char *description;
	char *location;
	int busy_status;
	int event_status;
	int priority;
	int sensitivity;
	int has_rrule;  /* rrule_id */
	double latitude;
	double longitude;
	int has_alarm;
	int original_event_id;
	long long int last_mod;
	char *sync_data1;
	int is_allday;
} cal_instance_allday_s;

typedef struct
{
	cal_record_s common;
	int event_id;
	int calendar_id;
	calendar_time_s start;
	calendar_time_s end;
	char *summary;
	char *description;
	char *location;
	int busy_status;
	int event_status;
	int priority;
	int sensitivity;
	int has_rrule; /*rrule_id */
	double latitude;
	double longitude;
	int has_alarm;
	int original_event_id;
	long long int last_mod;
	char *organizer_name;
	char *categories;
	int has_attendee;
	char *sync_data1;
	char *sync_data2;
	char *sync_data3;
	char *sync_data4;
} cal_instance_normal_extended_s;

typedef struct
{
	cal_record_s common;
	int event_id;
	int calendar_id;
	calendar_time_s start;
	calendar_time_s end;
	char *summary;
	char *description;
	char *location;
	int busy_status;
	int event_status;
	int priority;
	int sensitivity;
	int has_rrule; /* rrule_id */
	double latitude;
	double longitude;
	int has_alarm;
	int original_event_id;
	long long int last_mod;
	char *sync_data1;
	char *organizer_name;
	char *categories;
	int has_attendee;
	char *sync_data2;
	char *sync_data3;
	char *sync_data4;
	int is_allday;
} cal_instance_allday_extended_s;

typedef struct
{
	cal_record_s common;
	int type;
	int id;
	int calendar_id;
	int version;
}cal_updated_info_s;

typedef enum
{
	CAL_NOTI_TYPE_EVENT = 0x0,
	CAL_NOTI_TYPE_TODO,
	CAL_NOTI_TYPE_CALENDAR,
}cal_noti_type_e;

typedef struct{
	unsigned int property_id;
	const char* fields; /* DB field */
}cal_property_info_s;

typedef enum {
	CAL_FILTER_STR,
	CAL_FILTER_INT,
	CAL_FILTER_DOUBLE,
	CAL_FILTER_LLI,
	CAL_FILTER_CALTIME,
	CAL_FILTER_COMPOSITE,
}cal_filter_type_e;

typedef struct  {
	int filter_type; /* composite */
}cal_filter_s;

typedef struct {
	int filter_type;
	char *view_uri;
	GSList *filter_ops; /* calendar_filter_operator_e op */
	GSList *filters; /* calendar_filter_h l_filter */
	cal_property_info_s *properties;
	int property_count;
}cal_composite_filter_s;

typedef struct  {
	int filter_type; /* cal_filter_type_e */
	int property_id;
	int match; /* calendar_match_str_flag_e or calendar_match_int_flag_e */
	union {
		int i;
		char *s;
		double d;
		long long int lli;
		calendar_time_s caltime;
	}value;
}cal_attribute_filter_s;

typedef struct  {
	char* view_uri;
	cal_composite_filter_s* filter;
	int projection_count;
	unsigned int *projection;
	int sort_property_id;
	bool asc;
	cal_property_info_s *properties;
	int property_count;
	bool distinct;
}cal_query_s;

#define CAL_CALTIME_SET_UTIME(dest, src_utime) do {\
	(dest).type = CALENDAR_TIME_UTIME; \
	(dest).time.utime = src_utime; \
} while (0)

#define CAL_CALTIME_SET_DATE(dest, src_year, src_month, src_mday) do {\
	(dest).type = CALENDAR_TIME_LOCALTIME; \
	(dest).time.date.year = src_year; \
	(dest).time.date.month = src_month; \
	(dest).time.date.mday = src_mday; \
} while (0)

typedef struct {
	int property_id;
	union {
		int i;
		char *s;
		double d;
		long long int lli;
		calendar_time_s caltime;
	}value;
}cal_search_value_s;

typedef struct {
	cal_record_s common;
	GSList *values;
}cal_search_s;

typedef struct {
	cal_record_s common;
	int id;
	int record_id;
	int record_type;
	char* key;
	char* value;
}cal_extended_s;

/**
 * @}
 */

#endif /* __CALENDAR_SVC_TYPEDEF_H__ */
