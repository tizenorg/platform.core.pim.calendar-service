/*
 * Calendar Service
 *
 * Copyright (c) 2000 - 2012 Samsung Electronics Co., Ltd. All rights reserved.
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
#ifndef _CALENDAR_SVC_TYPEDEF_H_
#define _CALENDAR_SVC_TYPEDEF_H_

#include <time.h>
#include <sqlite3.h>

#include "calendar-svc-provider.h"

/* Definition */
#define CAL_INVALID_INDEX				(-1)

#define CAL_SCH_SUMMARY_LEN_MAX			1024//256			/**< Summary string length of Schedule */
#define CAL_SCH_DESCRIPTION_LEN_MAX		4096			    /**< Description string length of Schedule */

#define CAL_RECORD_CNT_MAX				2000			/**< max record count */

#define CAL_YEAR_MIN				1900		/**< Start value of year's range */
#define CAL_YEAR_MAX				2037		/**< End value of year's range */
#define CAL_MONTH_CNT_MAX			12			/**< max month count */
#define CAL_MONTH_CNT_MIN			1			/**< min month count */
#define CAL_DAY_CNT_MAX				31			/**< max day count */
#define BENCHMARK_YEAR				1900L		/**< tm_year's benchmark */
#define ONE_DAY_SECONDS				86400L		/**< seconds in a day */
#define ONE_WEEK_SECONDS			604800L		/**< seconds in a week */
#define ONE_MONTH_SECONDS			2678400L	/**< seconds in a month */
#define ONE_YEAR_SECONDS			31536000L	/**< seconds in a year */
#define DAY_OF_A_WEEK				7			/**< days of a week */
#define MAX_REPEAT_OF_A_WEEK		9			/**< max repeat event count in week */

#define MONTH_MAX 11
#define MONTH_DAY_MIN 1
#define MONTH_DAY_MAX 31
#define MONTH_MIN 0

#define TM_YEAR_MIN 0
#define TM_YEAR_MAX 138

#define BASE_TIME_YEAR 70

#define CAL_EVENT_UID_MAX_COUNT					256					/**< Max count of calendar UID*/

typedef enum
{
	CAL_STRUCT_TYPE_SCHEDULE=0,	/**< schedule type */
	CAL_STRUCT_TYPE_CALENDAR,	/**< calendar type */
	CAL_STRUCT_TYPE_TODO,    /**< task type   */
	CAL_STRUCT_TYPE_TIMEZONE,
	CAL_STRUCT_TYPE_SCHEDULE_LIST,
	CAL_STRUCT_TYPE_TODO_LIST,
}cal_struct_type;

typedef enum
{
	CAL_EVENT_PATICIPANT = 0,	/**< CAL_STRUCT_PARTICIPANT */
	CAL_EVENT_CATEGORY,		/**< CAL_STRUCT_CATEFORY */
	CAL_EVENT_RECURRENCY,	/**< CAL_STRUCT_RECURRENCY */
	CAL_EVENT_DELETE,		/**< CAL_STRUCT_DELETE */
	CAL_EVENT_SYNC_STATUS,	/**< CAL_STRUCT_SYNC_STATUS */
	CAL_EVENT_CALENDAR,	/**< CAL_STRUCT_CALENDAR */
	CAL_EVENT_ALARM,
	CAL_EVENT_MAX,

} cal_data_type_t;

struct _cal_struct{
	cal_struct_type event_type;
	void* user_data;
};

struct _cal_value{
	cal_data_type_t v_type;
	void* user_data;
};

struct _cal_iter{
	int i_type;
	sqlite3_stmt *stmt;
	int is_patched;
};

typedef struct
{
	int localtime_offset;
	int local_dst_offset;
	char local_tz_name[50];
	struct tm start_local_dst_date_time;
	struct tm start_local_std_date_time;

	int temptime_offset;
	int temp_dst_offset;
	char temp_tz_name[50];
	struct tm start_temp_dst_date_time;
	struct tm start_temp_std_date_time;

	int is_initialize;
	int is_used_calendar_tz;
} cal_svc_tm_info_t;

typedef enum
{
	CALS_NOTI_TYPE_EVENT,
	CALS_NOTI_TYPE_TODO,
	CALS_NOTI_TYPE_CALENDAR,
}cals_noti_type;

/**
 * @enum cal_record_sort_t
 * This enumeration defines record sort type.
 */
typedef enum
{
	CAL_SORT_BY_ID = 0,			/**< get reocrds sort by id */
	CAL_SORT_BY_START_DATE,		/**< get reocrds sort by start time */
	CAL_SORT_BY_END_DATE,		/**< get reocrds sort by end time */
	CAL_SORT_BY_MODIFIED_DATE,	/**< get reocrds sort by modified time */
	CAL_SORT_BY_STATUS,			/**< get reocrds sort by status */
	CAL_SORT_BY_PRIORITY_TIME_ALPHABET,			/**< get reocrds sort by priority->time->alphabet */
	CAL_SORT_BY_PRIORITY,			/**< get reocrds sort by priority */

} cal_record_sort_t;

/**
 * @enum cal_record_asc_desc_t
 * This enumeration defines event sort type.
 */
typedef enum
{
	CAL_SORT_ASC =0,						/**< asc sort type */
	CAL_SORT_DESC							/**< desc sort type */
}cal_record_asc_desc_t;

/**
 * @enum calendar_type_t
 * This enumeration defines calendar type.
 */
typedef enum
{
	CAL_ALL_CALENDAR =0,					/**< all calendar type */
	CAL_PHONE_CALENDAR,						/**< phone calendar type */
	CAL_ACTIVESYNC_CALENDAR,				/**< activesync calendar type*/
	CAL_GOOGLE_CALENDAR						/**< google calendar type*/
}calendar_type_t;

/**
 * @enum cal_type_t
 * This enumeration defines calendar event type.
 */
typedef enum
{
	CAL_EVENT_NONE=0,			/**< None type */
	CAL_EVENT_SCHEDULE_TYPE,	/**< schedule event type */
	CAL_EVENT_TODO_TYPE,		/**< task event type */
	CAL_EVENT_MEMO_TYPE,		/**< memo event type */
	CAL_EVENT_MAX_TYPE,		/**< max type */
} cal_event_type_t;


/**
 * @enum cal_vCal_ver_t
 * This enumeration defines vCalendar version.
 */
typedef enum
{
	CAL_VCAL_VER_1_0 = 0,	/**< vCalendar ver 1.0 */
	CAL_VCAL_VER_2_0,		/**< vCalendar ver 2.0 */
	CAL_VCAL_VER_UNKNOWN	/**< vCalendar ver unknown */
} cal_vCal_ver_t;

/**
 * @enum cal_filter_t
 * This enumeration defines filter type for todo data.
 */
typedef enum
{
	CAL_TODO_FILTER_UNDONE = 1,						/**< todo condition is undone */
	CAL_TODO_FILTER_DONE = 2,							/**< todo condition is done */
	CAL_TODO_FILTER_OVER_DUE = 4,						/**< todo condition is duration */
	CAL_TODO_FILTER_UNDONE_AND_DONE =3,				/**< todo condition is undone and done */
	CAL_TODO_FILTER_UNDONE_AND_OVER_DUE = 5,			/**< todo condition is undone and overdue */
	CAL_TODO_FILTER_DONE_AND_OVER_DUE = 6,				/**< todo condition is done and overdue */
	CAL_TODO_FILTER_ALL = 7,								/**< todo condition is all */

	CAL_TODO_FILTER_MAX = CAL_TODO_FILTER_ALL
}cal_filter_t;

/**
 * @enum cal_priority_t
 * This enumeration defines priority for todo data.
 */
typedef enum
{
	CAL_PRIORITY_LOW,	/**< priority low */
	CAL_PRIORITY_MID,	/**< priority middle */
	CAL_PRIORITY_HIGH	/**< priority high */
}cal_priority_t;


/**
 * @enum cal_starting_day_type_t
 * This enumeration defines starting day.
 */
typedef enum
{
	CAL_STARTING_DAY_SUNDAY=0,	/**< starting day is sunday */
	CAL_STARTING_DAY_MONDAY		/**< starting day is monday */
} cal_starting_day_type_t;


typedef enum
{
	CAL_TIME_ZONE_GMT_L11,
	CAL_TIME_ZONE_GMT_L10,
	CAL_TIME_ZONE_GMT_L9,
	CAL_TIME_ZONE_GMT_L8,
	CAL_TIME_ZONE_GMT_L7,
	CAL_TIME_ZONE_GMT_L6,
	CAL_TIME_ZONE_GMT_L5,
	CAL_TIME_ZONE_GMT_L45,
	CAL_TIME_ZONE_GMT_L4,
	CAL_TIME_ZONE_GMT_L35,
	CAL_TIME_ZONE_GMT_L3,
	CAL_TIME_ZONE_GMT_L2,
	CAL_TIME_ZONE_GMT_L1,
	CAL_TIME_ZONE_GMT_0,
	CAL_TIME_ZONE_GMT_1,
	CAL_TIME_ZONE_GMT_2,
	CAL_TIME_ZONE_GMT_3,
	CAL_TIME_ZONE_GMT_35,
	CAL_TIME_ZONE_GMT_4,
	CAL_TIME_ZONE_GMT_45,
	CAL_TIME_ZONE_GMT_5,
	CAL_TIME_ZONE_GMT_55,
	CAL_TIME_ZONE_GMT_575,
	CAL_TIME_ZONE_GMT_6,
	CAL_TIME_ZONE_GMT_65,
	CAL_TIME_ZONE_GMT_7,
	CAL_TIME_ZONE_GMT_8,
	CAL_TIME_ZONE_GMT_9,
	CAL_TIME_ZONE_GMT_95,
	CAL_TIME_ZONE_GMT_10,
	CAL_TIME_ZONE_GMT_11,
	CAL_TIME_ZONE_GMT_12,
	CAL_TIME_ZONE_GMT_13,

	CAL_TIME_ZONE_COUNT_MAX
}cal_time_zone_t;

/**
 * This structure stores the information whether there is an event on specific month
 * event_info description.
 * 0 : none.
 * 1 : event start or one day's event.
 * 2 : event not start and not end date , if 3 more days event than, 12223
 * 3 : event end
 * 4 : repeated event's start & end date is duplicated.
 */
typedef struct
{
	int index;							/**< index of event */
	cal_sch_category_t cal_type; 							/**< category type */
	int all_day_event;					/***< this event is all day event or not */
	struct tm start_date_time[MAX_REPEAT_OF_A_WEEK];
	struct tm end_date_time[MAX_REPEAT_OF_A_WEEK];
	int repeat_event_count;
	//int			event_info[ DAY_OF_A_WEEK ]; 	/**< event info in a month 0-none */
}cal_weekly_event_info_t;

/**
 * This structure stores the information whether there is an event on specific month.
 */
typedef struct
{
	int year;						/**< Specifies a year */
	int month;						/**< Specifies a month */
	bool exist[CAL_DAY_CNT_MAX];	/**< There can be an event or not on specific date */
}cal_event_info_t;					/* Event data info for calendar */

/**
 * This structure stores the information whether there is an event on specific month.
 */
typedef struct
{
	int year;						/**< Specifies a year */
	int month;						/**< Specifies a month */
	int index;						/**< index of event */
	int event_info[CAL_DAY_CNT_MAX]; /**< event info in a month 0-none, 1-has */
}cal_monthly_event_info_t;

/**
 * This structure defines schedule information.
 */
typedef struct
{
	int index;				/**< Record index */
	int account_id;			/**< Account_id */
	cal_event_type_t cal_type;			/**< Calendar event type */
	cal_sch_category_t sch_category;		/**< Category of schedule */

	char *summary;			/**< Summary, appointment, task: subject, birthday:Name */
	char *description;		/**< Description,appointment, task: description, anniversary,holiday:occasion*/
	char *location;			/**< Location */
	GList *meeting_category;	/**< collection of categories */
	bool all_day_event;		/**< All day event flag */
	struct tm start_date_time;	/**< schedule:start time, anniv,holiday,birthday,memo,todo: date */
	struct tm end_date_time;		/**< end time */
	GList *alarm_list;
	cal_repeat_term_t repeat_term;		/**< Repeat term */
	int repeat_interval;	/**< Interval of repeat term */
	int repeat_until_type;	/**< repeat until type */
	int repeat_occurrences;	/**< occurrences of repeat */
	struct tm repeat_end_date;	/**< End date for repeat */
	cal_date_type_t sun_moon;			/**< Using sun or lunar calendar */
	cal_starting_day_type_t week_start;			/**< Start day of a week */
	char *week_flag;//[DAY_OF_A_WEEK + 1]; /**< Indicate which day is select in a week */
	int day_date;			/**< 0- for weekday(sun,mon,etc.), 1- for specific day(1,2.. Etc) */
	struct tm last_modified_time;	/**< for PC Sync */
	bool missed;				/**< Miss alarm flag */
	cals_status_t task_status;		/**< current task status */
	cal_priority_t priority;		/**< Priority */
	int timezone;			/**< timezone of task */
	int file_id;			/**< file id for attach or alarm tone*/
	int contact_id;			/**< contact id for birthday in contact list */
	GList *attendee_list;	/**< collection of attendee */
	int busy_status;		/**< ACS, G : Flag of busy or not */
	int sensitivity;		/**< ACS, G : The sensitivity (public, private, confidential). #cal_visibility_type_t*/
	int meeting_status;		/**< ACS, G : The status of the meeting. */
	char *uid;				/**< ACS, G : Unique ID of the meeting item */
	char *organizer_name;		/**< ACS, G : Name of organizer(author) */
	char *organizer_email;	/**< ACS, G : Email of organizer */
	calendar_type_t calendar_type;		/**< ACS, G : Type(all,phone,google) of calendar */
	char *gcal_id;			/**< G : Server id of calendar */
	bool deleted;			/**< G : Flag for deleted */
	char *updated;			/**< G : Updated time stamp */
	int location_type;		/**< G : Location type */
	char *location_summary;	/**< G : A simple string value that can be used as a representation of this location */
	char *etag;				/**< G : ETAG of this event */
	int calendar_id;			/**< G : id to map from calendar table */
	cal_sync_status_t sync_status;		/**< G : Indication for event entry whether added/ modified/ deleted */
	char *edit_uri;      /**< G : EditUri for google calendar */
	char *gevent_id;			/**< G : Server id of an event */
	int dst;					/**< dst of event */
	GList *exception_date_list;		       /**< exception dates */
	int original_event_id;        /**< original event id for recurrency exception */
	double latitude;
	double longitude;
	char *tz_name;
	char *tz_city_name;
	int is_deleted;
	int email_id;
	int availability;
	struct tm created_date_time; /**< created time */
	struct tm completed_date_time; /**< completed time */
	int progress;
}cal_sch_full_t;


/**
 * This structure defines schedule information.
 */
typedef struct
{
	int index;				/**< Record index */
	cal_event_type_t cal_type;			/**< Calendar event type */
	cal_sch_category_t sch_category;		/**< Category of schedule */

	char *summary; /**< Summary, appointment, task: subject, birthday:Name */
	char *description; /**< Description,appointment, task: description, anniversary,holiday:occasion*/
	char *location; /**< Location */

	bool all_day_event;		/**< All day event flag */
	struct tm start_date_time;	/**< schedule:start time, anniv,holiday,birthday,memo,todo: date */
	struct tm end_date_time;		/**< end time */
	struct tm alarm_time;			/**< alarm time */
	int remind_tick;		/**< Alarms before remindTick */
	cal_sch_remind_tick_unit_t	remind_tick_unit;	/**< Remind tick unit */
	int alarm_id;			/**< Alarm id */

	cal_repeat_term_t repeat_term;		/**< Repeat term */
	int repeat_interval;	/**< Interval of repeat term */
	int repeat_until_type;	/**< Repeat until type */
	struct tm repeat_end_date;	/**< End date for repeat */
	cal_date_type_t sun_moon;			/**< Using sun or lunar calendar */
	cal_starting_day_type_t week_start;			/**< Start day of a week */
	char *week_flag; /**< Indicate which day is select in a week */
	int day_date;			/**< 0- for weekday(sun,mon,etc.), 1- for specific day(1,2.. Etc) */
	bool missed;				/**< Miss alarm flag */

	int participant_id;		/**<contact id for participant*/
	calendar_type_t calendar_type;		/**<one type of calendar */
	cal_time_zone_t timezone;			/**< timezone of task */
	int dst;					/**< dst of event */
	int is_deleted;
}cal_sch_t;

/**
 * This structure defines participant information of a meetting.
 * ical: cutype, member, role, partstat, rsvp, delto, delfrom, sentby, cn,  dir, language
 */
typedef struct
{
	int event_id;
	char *attendee_number;
	int attendee_type;
	int attendee_ct_index;
	char *attendee_uid;
	int is_deleted;

	/* ical spec from here */
	char *attendee_group;	/* cutype */
	char *attendee_email;	/* member */
	int attendee_role;		/* role */
	int attendee_status;	/* partstat: ACCEPTED, DECLINED.. */
	int attendee_rsvp;		/* rsvp */
	char *attendee_delegate_uri;	/* delfrom */
	char *attendee_delegator_uri;	/* delto */
	/* sentby */
	char *attendee_name;	/* cn */
	/* dir */
	/* language */

}cal_participant_info_t;

/**
 * This structure defines category information of a meetting.
 */
typedef struct
{
	int event_id;
	char *category_name;
}cal_category_info_t;

/**
 * This structure defines exception information of recurrence event.
 */
typedef struct
{
	cal_sch_full_t *exception_record;
	struct tm exception_start_time;
	int is_modified;
	int event_id;
}cal_exception_info_t;

/**
 * This structure defines exception information of alarm.
 */
typedef struct
{
	int alarm_id;			/**< Alarm id */
	int event_id;
	cal_alert_type_t alarm_type;			/**< Alert type(see 'cal_alert_type_t') */
	int is_deleted;

	/* audio */
	/* -- trigger */
	struct tm alarm_time;
	int remind_tick;
	cal_sch_remind_tick_unit_t	remind_tick_unit;
	/* --attach */
	char *alarm_tone;			/**< Alert Sound File Name */

	/* display */
	char *alarm_description;			/**< Alert description */


	/* email */

}cal_alarm_info_t;


/**
 * This structure stores record and its repeat type.
 * repeat_type  / include original data / include repeated data / repeated schedule or not
 *     1              o                x                 o
 *     2              o                o                 o
 *     3              x                o                 o
 *     4              o                x                 x (just one day event )
 *     5              o                x                 x ( more than two days )
 */
typedef struct
{
	cal_sch_t record;						/**< event record */
	cal_event_type_t repeat_type;				/**< repeat type as above, 1-5 */
}cal_period_record_t;

/**
 * @enum cal_search_repeat_term_t
 * This enumeration defines Repeat term.
 */
typedef enum
{
	CAL_SEARCH_REPEAT_INVALID = -1,	/**< Invalid Repeat */
	CAL_SEARCH_REPEAT_NONE = 0,	/**< never Repeat */
	CAL_SEARCH_REPEAT_EVERY_DAY,	/**< Repeats every day */
	CAL_SEARCH_REPEAT_EVERY_WEEK,	/**< Repeats every week */
	CAL_SEARCH_REPEAT_EVERY_MONTH,	/**< Repeats every month */
	CAL_SEARCH_REPEAT_EVERY_YEAR,	/**< Repeats every year */
	CAL_SEARCH_REPEAT_EVERY_WEEKDAYS, /**< Repeats every weekdays *//* same with CAL_REPEAT_EVERY_WEEK, but week_flag="0111110", day_date=1, sun_moon=0, week_start=0 */
	CAL_SEARCH_REPEAT_EVERY_MONTH_DAY, /**< Repeats every month's week days *//* same with CAL_REPEAT_EVERY_MONTH, but week_flag="1000000"~"0000001", day_date=0, sun_moon=0, week_start=0 */
	CAL_SEARCH_REPEAT_EVERY_YEAR_DAY, /**< Repeats every year's month week days *//* same with CAL_REPEAT_EVERY_YEAR, but week_flag="1000000"~"0000001", day_date=0, sun_moon=0, week_start=0 */
} cal_search_repeat_term_t;


typedef struct
{
	int index;  														/**< Record index */
	char summary[CAL_SCH_SUMMARY_LEN_MAX + 1];		/**< Summary, appointment, task: subject, birthday:Name */
	char description[CAL_SCH_DESCRIPTION_LEN_MAX + 1];				/**< Description,appointment, task: description, anniversary,holiday:occasion*/
	struct tm start_date_time; 											/**< schedule:start time, anniv,holiday,birthday,memo,todo: date */
	struct tm end_date_time; 											/**< end time */
	int alarmed; 													/**< Indicates whether an alarm is to be activated for the event */
	struct tm alarm_time; 												/**< alarm time */
	cal_search_repeat_term_t eventRecurrence;  							/**< Recurrent interval of this event. */
}cal_record_search_field;


//This is the calendar schema
typedef struct
{
	int index;
	int store_type;
	char *calendar_id;
	char *uid;
	char *link;
	long updated;
	char *name;
	char *description;
	char *author;
	char *color;
	int hidden;
	int selected;
	char *location;
	int locale;
	int country;
	long time_zone;
	char *timezone_label;
	int display_all_timezones;
	int date_field_order;
	int format_24hour_time;
	int week_start;
	int default_cal_mode;
	int custom_cal_mode;
	char *user_location;
	char *weather;
	int show_declined_events;
	int hide_invitations;
	int alternate_calendar;
	int visibility;
	int projection;
	int sequence;
	int suppress_reply_notifications;
	int sync_event;
	int times_cleaned;
	int guests_can_modify;
	int guests_can_invite_others;
	int guests_can_see_guests;
	int access_level;
	int sync_status;
	int is_deleted;
	int account_id;
	int sensitivity;
}calendar_t;

typedef struct
{
	//int gcal_dev_db_id;
	char *uid;
	char *name;
	calendar_type_t calendar_type;		/**<one type of calendar */
}calendar_main_info_t;


/* type for timezone information save */
typedef struct
{
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
} cal_timezone_t;

/**
 * @}
 */
#endif // _CALENDAR_SVC_TYPEDEF_H_
