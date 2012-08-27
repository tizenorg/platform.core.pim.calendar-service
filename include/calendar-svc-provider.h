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
#ifndef __CALENDAR_SVC_H__
#define __CALENDAR_SVC_H__

#ifndef DEPRECATED
#define DEPRECATED __attribute__ ((deprecated))
#endif


/**
 * @defgroup CALENDAR_SVC  Calendar Service
 */

/**
 * @defgroup common	common
 * @ingroup CALENDAR_SVC
 * @brief
 *		common struct for calendar service
 */


/**
 * cal_struct is an opaque type, it must be
 * used via accessor functions.
 * @addtogroup common
 * @see calendar_svc_struct_new(), calendar_svc_struct_free()
 * @see calendar_svc_struct_get_value(), calendar_svc_struct_get_list(),
 * @see calendar_svc_struct_store_value(), calendar_svc_struct_store_list()
 */

#include <glib.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <calendar-svc-errors.h>
#include <calendar-svc-struct.h>
#include <alarm.h>

/**
 * @addtogroup common
 * @{
 */

/**
 * This enumeration date type, sun or lunar.
 */
typedef enum
{
	CAL_DATE_SUN = 0,	   /**< date is sun type*/
	CAL_DATE_LUNAR,		/**< date is lunar type */
} cal_date_type_t;

/**
 * This enumeration defines Remind Tick Unit for schedule.
 * Ex. remindTick = 1, remindTickUnit = CAL_SCH_TIME_UNIT_MIN, Organizer alarms
 * 1 minute before schedule starting time.
 */
typedef enum /* use with *60 */
{
	CAL_SCH_TIME_UNIT_OFF = -1, /**< off */
	CAL_SCH_TIME_UNIT_MIN = 1, /**< Minute */
	CAL_SCH_TIME_UNIT_HOUR = 60, /**< Hour 60 * 60 */
	CAL_SCH_TIME_UNIT_DAY = 1440, /**< Day 60 * 60 *24 */
	CAL_SCH_TIME_UNIT_WEEK = 10080, /**< Week DAY * 7 */
	CAL_SCH_TIME_UNIT_MONTH,	/**< Month - will be removed*/
	CAL_SCH_TIME_UNIT_SPECIFIC  /**< using alarm time */
} cal_sch_remind_tick_unit_t;

/**
 * This enumeration defines Repeat term.
 */
typedef enum
{
	CAL_REPEAT_NONE = 0,	         /**< never Repeat */
	CAL_REPEAT_EVERY_DAY,	      /**< Repeats every day */
	CAL_REPEAT_EVERY_WEEK,	      /**< Repeats every week */
	CAL_REPEAT_EVERY_MONTH,	      /**< Repeats every month */
	CAL_REPEAT_EVERY_YEAR,	      /**< Repeats every year */
	CAL_REPEAT_EVERY_WEEKDAYS,    /**< Repeats every weekdays *//* same with CAL_REPEAT_EVERY_WEEK, but week_flag="0111110", day_date=1, sun_moon=0, week_start=0 */
	CAL_REPEAT_EVERY_MONTH_DAY,   /**< Repeats every month's week days *//* same with CAL_REPEAT_EVERY_MONTH, but week_flag="1000000"~"0000001", day_date=0, sun_moon=0, week_start=0 */
	CAL_REPEAT_EVERY_YEAR_DAY,    /**< Repeats every year's month week days *//* same with CAL_REPEAT_EVERY_YEAR, but week_flag="1000000"~"0000001", day_date=0, sun_moon=0, week_start=0 */
} cal_repeat_term_t;

/**
 * This enumeration defines Repeat term.
 */
typedef enum
{
	CALS_REPEAT_UNTIL_TYPE_NONE = 0,  /**< Repeat endlessly */
	CALS_REPEAT_UNTIL_TYPE_COUNT,     /**< Repeat number of times, which the CAL_VALUE_INT_REPEAT_OCCURRENCES indicates */
	CALS_REPEAT_UNTIL_TYPE_DATETIME,  /**< Repeat until the date-time which the CAL_VALUE_GMT_REPEAT_END_DATE indicates */
} cal_repeat_until_type_t;

/**
 * This enumeration defines sync status.
 */
typedef enum
{
	CAL_SYNC_STATUS_NEW = 0,	/**< newly added. */
	CAL_SYNC_STATUS_UPDATED,	/**< updated. */
	CAL_SYNC_STATUS_DELETED,	/**< deleted. */
	CAL_SYNC_STATUS_SYNCED,		/**< synced */
} cal_sync_status_t;

/**
 * This enumeration defines Expiration for schedule data.
 */
typedef enum
{
	CAL_SCH_EXPIRATION_NONE = 0,			  /**< never expirate */
	CAL_SCH_EXPIRATION_AFTER_1_MONTH,		/**< After 1 month */
	CAL_SCH_EXPIRATION_AFTER_1_YEAR,		  /**< After 1 year */
	CAL_SCH_EXPIRATION_AFTER_2_YEARS,		/**< After 2 years */
	CAL_SCH_EXPIRATION_AFTER_3_YEARS		  /**< After 3 years */
} cal_expiration_t;

/**
 * This enumeration defines lart type.
 */
typedef enum
{
	CAL_ALERT_MELODY = 0,				  /**< alarm type is melody */
	CAL_ALERT_MUTE,						  /**< alarm type is mute */
	CAL_ALERT_INCREASING_MELODY,		  /**< alarm type is increasing melody */
	CAL_ALERT_VIBRATION,				   /**< alarm type is vibrate */
	CAL_ALERT_VIBRATION_THEN_MELODY,	  /**< alarm type is vibrate then melody */
	CAL_ALERT_VIBMELODY,				   /**< alarm type is melody with vibrate */
	CAL_ALERT_VIB_INCREASING_MELODY		/**< alarm type is increasing melody */
} cal_alert_type_t;

/**
 * This enumeration defines alarm volume .
 */
typedef enum
{
	CAL_SNOOZE_OFF = 0,	/**< snoooze is off */
	CAL_SNOOZE_1MIN,	  /**< snoooze time is 1 min */
	CAL_SNOOZE_3MINS,	  /**< snoooze time is 3 mins */
	CAL_SNOOZE_5MINS,	  /**< snoooze time is 5 mins */
	CAL_SNOOZE_10MINS, 	/**< snoooze time is 10 mins */
	CAL_SNOOZE_15MINS	  /**< snoooze time is 15 mins */

} cal_snooze_type_t;

/**
 * This enumeration defines alarm snooze count .
 */
typedef enum
{
	CAL_SNOOZE_0TIME = 0,		/**< snoooze count is 0 time */
	CAL_SNOOZE_1TIME = 1,	  /**< snoooze count is 1 time */
	CAL_SNOOZE_2TIMES = 2,	  /**< snoooze count is 2 times */
	CAL_SNOOZE_5TIMES = 5,	  /**< snoooze count is 5 times */
	CAL_SNOOZE_10TIMES = 10	  /**< snoooze count is 10 times */
} cal_snooze_count_t;

/**
 * This enumeration defines attendee's status .
 */
typedef enum
{
	CAL_TZ_FLAG_GMT = 0,    /**< gmt time */
	CAL_TZ_FLAG_LOCAL = 1,   /**< calendar local time */
} cal_timezone_flag;

/**
 * This enumeration defines calendar's visibility .
 */
typedef enum
{
	PUBLIC_VISIBILITY = 0,
	PRIVATE_VISIBILITY,
	CONFIDENTIAL_VISIBILITY
} cal_visibility_type_t;

/**
 * This enumeration defines event attendee's availability .
 */
typedef enum
{
	EVENT_BUSY_FB=0,
	EVENT_BUSY_UNAVAILABLE_FB,
	EVENT_FREE_FB,
	EVENT_BUSY_TENTATIVE_FB,
} cal_event_availability_type_t;


/**
 * This enumeration defines event attendee's role .
 */
typedef enum
{
	EVENT_ATTENDEE_REQ_PARTICIPANT_ROLE=0,
	EVENT_ATTENDEE_OPT_PARTICIPANT_ROLE,
	EVENT_ATTENDEE_NON_PARTICIPANT_ROLE,
	EVENT_ATTENDEE_CHAIR_ROLE,
} cal_event_attendee_role_type_t;

/**
 * This enumeration defines event attendee's status.
 */
typedef enum
{
	EVENT_ATTENDEE_NEEDS_ACTION_AT_STATUS=0,
	EVENT_ATTENDEE_ACCEPTED_AT_STATUS,
	EVENT_ATTENDEE_DECLINED_AT_STATUS,
	EVENT_ATTENDEE_TENTATIVE_AT_STATUS,
	EVENT_ATTENDEE_DELEGATED_AT_STATUS,
	EVENT_ATTENDEE_COMPLETED_AT_STATUS,
	EVENT_ATTENDEE_IN_PROCESS_AT_STATUS
} cal_event_attendee_status_type_t;


/**
 * Deprecated.
 */
#define EVENT_ATTENDEE_PENDING_AT_STATUS EVENT_ATTENDEE_NEEDS_ACTION_AT_STATUS

/**
 * This enumeration defines event attendee's type .
 */
typedef enum
{
	EVENT_ATTENDEE_INDIVIDUAL_TYPE=0,
	EVENT_ATTENDEE_GROUP_TYPE,
	EVENT_ATTENDEE_RESOURCE_TYPE,
	EVENT_ATTENDEE_ROOM_TYPE,
	EVENT_ATTENDEE_UNKNOWN_TYPE
} cal_event_attendee_type_t;


/**
 * This enumeration defines CAL_VALUE_INT_PRIORITY .
 */
typedef enum
{
	EVENT_PRIORITY_LOW = 0,
	EVENT_PRIORITY_NORMAL,
	EVENT_PRIORITY_HIGH,
} cal_priority_type_t;

enum {
	CALS_TODO_PRIORITY_NONE = 0x01,
	CALS_TODO_PRIORITY_HIGH = 0x02,
	CALS_TODO_PRIORITY_MID = 0x04,
	CALS_TODO_PRIORITY_LOW = 0x08,
};

/**
 * This enumeration defines status.
 * (related with CAL_VALUE_INT_TASK_STATUS)
 */

#define CALS_STATUS_NONE CALS_EVENT_STATUS_NONE
typedef enum
{
	CALS_EVENT_STATUS_NONE = 0x0001,
	CALS_EVENT_STATUS_TENTATIVE = 0x0002,
	CALS_EVENT_STATUS_CONFIRMED = 0x0004,
	CALS_EVENT_STATUS_CANCELLED = 0x0008,
	CALS_TODO_STATUS_NONE = 0x0100,
	CALS_TODO_STATUS_NEEDS_ACTION = 0x0200,
	CALS_TODO_STATUS_IN_PROCESS = 0x0400,
	CALS_TODO_STATUS_CANCELLED = 0x0800,
	CALS_TODO_STATUS_COMPLETED = 0x1000,
} cals_status_t;

/**
 * This enumeration defines calendar type.
 * (related with CAL_TABLE_INT_STORE_TYPE)
 */
typedef enum
{
	CALS_CALENDAR_TYPE_NONE = 0,
	CALS_CALENDAR_TYPE_EVENT = 1<<0,
	CALS_CALENDAR_TYPE_TODO = 1<<1,
} cals_calendar_store_type;

/**
 * This enumeration defines todo list ordering type.
 */
typedef enum
{
	CALS_TODO_LIST_ORDER_END_DATE = 0,
	CALS_TODO_LIST_ORDER_PRIORITY,
	CALS_TODO_LIST_ORDER_STATUS,
} cals_todo_list_order_t;

/**
 * This enumeration defines calendar sensitivity.
 * (related with CAL_VALUE_INT_SENSITIVITY)
 */
typedef enum
{
	CALS_SENSITIVITY_PUBLIC = 0x0,
	CALS_SENSITIVITY_PRIVATE,
	CALS_SENSITIVITY_CONFIDENTIAL,
} cals_sensitivity_t;

/**
 * This enumeration defines updated type
 * (related with CALS_STRUCT_UPDATED_INT_TYPE)
 */
enum cals_updated_type {
	CALS_UPDATED_TYPE_INSERTED = 0x0,
	CALS_UPDATED_TYPE_MODIFIED,
	CALS_UPDATED_TYPE_DELETED,
};

/**
 * Flags to specify what fields will be searched by search API
 */
enum SEARCHFIELD {
	CALS_SEARCH_FIELD_NONE = 0,
	CALS_SEARCH_FIELD_SUMMARY = 1<<0,
	CALS_SEARCH_FIELD_DESCRIPTION = 1<<2,
	CALS_SEARCH_FIELD_LOCATION = 1<<3,
	CALS_SEARCH_FIELD_ATTENDEE = 1<<4,
};

/**
 * @}
 */

/**
 * @addtogroup common
 * @{
 * brief
 * 		calendar_svc_struct_new's argument
 */
#define CAL_STRUCT_TYPE					  /**< CAL_STRUCT_TYPE */
#define CAL_STRUCT_CALENDAR "calendar"		/**< CAL_STRUCT_CALENDAR */
#define CAL_STRUCT_SCHEDULE "schedule"		/**< CAL_STRUCT_SCHEDULE */
#define CAL_STRUCT_TODO		"todo"			/**< CAL_STRUCT_TASK */
#define CAL_STRUCT_TIMEZONE	"timezone"		/**< CAL_STRUCT_TIMEZONE */
#define CAL_STRUCT_UPDATED "updated"     /**< CAL_STRUCT_UPDATED */
#define CALS_STRUCT_PERIOD_NORMAL_ONOFF "period_normal_onoff"
#define CALS_STRUCT_PERIOD_ALLDAY_ONOFF "period_allday_onoff"
#define CALS_STRUCT_PERIOD_NORMAL_BASIC "period_normal_basic"
#define CALS_STRUCT_PERIOD_ALLDAY_BASIC "period_allday_basic"
#define CALS_STRUCT_PERIOD_NORMAL_OSP "period_normal_osp"
#define CALS_STRUCT_PERIOD_ALLDAY_OSP "period_allday_osp"
#define CALS_STRUCT_PERIOD_NORMAL_LOCATION "period_normal_location"
#define CALS_STRUCT_PERIOD_ALLDAY_LOCATION "period_allday_location"
#define CALS_STRUCT_PERIOD_NORMAL_ALARM "period_normal_alarm"

// id for all data read
#define ALL_ACCOUNT_ID 0
#define ALL_CALENDAR_ID 0

// id for all data without visibility false
#define ALL_VISIBILITY_ACCOUNT -2

// id for local data read
#define LOCAL_ACCOUNT_ID -1
#define LOCAL_ALL_CALENDAR -1

/* start deprecated */
#define DEFAULT_CALENDAR_ID 1
/* end deprecated, replace DEFAULT_EVENT_CALENDAR_ID */

#define DEFAULT_EVENT_CALENDAR_ID 1
#define DEFAULT_TODO_CALENDAR_ID 2

// added val 2012.07.30
#define CALS_TODO_NO_DUE_DATE INT64_MAX

/**
 * @}
 */

/**
 * @addtogroup common
 * @{
 * brief
 *		calendar_svc_struct_xxx()'s argument
 */
#define CAL_TABLE_INT_INDEX "index"
#define CAL_TABLE_TXT_CALENDAR_ID "calendar_id"
#define CAL_TABLE_TXT_UID "uid"
#define CAL_TABLE_TXT_LINK "link"
#define CAL_TABLE_INT_UPDATED "updated"
#define CAL_TABLE_TXT_NAME "name"
#define CAL_TABLE_TXT_DESCRIPTION "description"
#define CAL_TABLE_TXT_AUTHOR "author"
#define CAL_TABLE_TXT_COLOR "color"
#define CAL_TABLE_INT_HIDDEN "hidden"
#define CAL_TABLE_INT_SELECTED "selected"
#define CAL_TABLE_TXT_LOCATION "location"
#define CAL_TABLE_INT_LOCALE "locale"
#define CAL_TABLE_INT_COUNTRY "country"
#define CAL_TABLE_INT_TIME_ZONE "time_zone"
#define CAL_TABLE_TXT_TIME_ZONE_LABEL "timezone_label"
#define CAL_TABLE_INT_DISPLAY_ALL_TIMEZONES "display_all_timezones"
#define CAL_TABLE_INT_DATE_FIELD_ORDER "date_field_order"
#define CAL_TABLE_INT_FROMAT_24HOUR_TIME "format_24hour_time"
#define CAL_TABLE_INT_WEEK_START "week_start"
#define CAL_TABLE_INT_DEFAULT_CAL_MODE "default_cal_mode"
#define CAL_TABLE_INT_CUSTOM_CAL_MODE "custom_cal_mode"
#define CAL_TABLE_TXT_USER_LOCATION "user_location"
#define CAL_TABLE_TXT_WEATHER "weather"
#define CAL_TABLE_INT_SHOW_DECLINED_EVENTS "show_declined_events"
#define CAL_TABLE_INT_HIDE_INVITATIONS "hide_invitations"
#define CAL_TABLE_INT_ALTERNATE_CALENDAR "alternate_calendar"
#define CAL_TABLE_INT_VISIBILITY "visibility"
#define CAL_TABLE_INT_PROJECTION "projection"
#define CAL_TABLE_INT_SEQUENCE "sequence"
#define CAL_TABLE_INT_SUPRESS_REPLY_NOTIFICATIONS "suppress_reply_notifications"
#define CAL_TABLE_INT_SYNC_EVENT "sync_event"
#define CAL_TABLE_INT_TIMES_CLEANED "times_cleaned"
#define CAL_TABLE_INT_GUESTS_CAN_MODIFY "guests_can_modify"
#define CAL_TABLE_INT_GUESTS_CAN_INVITE_OTHERS "guests_can_invite_others"
#define CAL_TABLE_INT_GUESTS_CAN_SEE_GUESTS "guests_can_see_guests"
#define CAL_TABLE_INT_ACCESS_LEVEL "access_level"
#define CAL_TABLE_INT_SYNC_STATUS "sync_status"
#define CAL_TABLE_INT_IS_DELETED "is_deleted"
#define CAL_TABLE_INT_ACCOUNT_ID "account_id"
#define CAL_TABLE_INT_SENSITIVITY "sensitivity"
#define CAL_TABLE_INT_STORE_TYPE "store_type" /**< #cals_calendar_store_type */

/**
 * @}
 */

/**
 * @addtogroup common
 * @{
 * brief
 * 		calendar_svc_struct_xxx()'s argument
 */
#define CALS_STRUCT_UPDATED_INT_VERSION "version"                /**< Version of schedule */
#define CALS_STRUCT_UPDATED_INT_TYPE "updated_type"      /**< Type of schedule update #cals_updated_type */
#define CALS_STRUCT_UPDATED_INT_ID "updated_id"      /**< id of updated schedule */
#define CALS_STRUCT_UPDATED_INT_CALENDAR_ID "updated_calendar_id"      /**< id of updated schedule */
/**
 * @}
 */



/**
 * @addtogroup common
 * @{
 * brief
 * 		calendar_svc_struct_xxx()'s argument
 */
#define CAL_VALUE_INT_INDEX				  "id"					/**< Record index */
#define CAL_VALUE_INT_ACCOUNT_ID			  "account_id"			/**< account id */
#define CAL_VALUE_INT_TYPE				  "type"				/**< Calendar component type */
#define CAL_VALUE_TXT_CATEGORIES "categories" /**< Category of schedule */
#define CAL_VALUE_TXT_EXDATE "exdate" /**< Exdate */
#define CAL_VALUE_TXT_SUMMARY				  "summary"				/**< Summary, appointment, task: subject, birthday:Name */
#define CAL_VALUE_TXT_DESCRIPTION			  "description"			/**< Description,appointment, task: description, anniversary,holiday:occasion*/
#define CAL_VALUE_TXT_LOCATION				  "location"				/**< Location */
#define CAL_VALUE_INT_MISSED				  "missed"				  /**< Miss alarm flag */
#define CAL_VALUE_INT_TASK_STATUS			  "task_status"			/**< current task status #cals_status_t */
#define CAL_VALUE_INT_PRIORITY				  "priority"				/**< Priority */
#define CAL_VALUE_INT_TIMEZONE				  "timezone"				/**< deprecated - timezone of task */
#define CAL_VALUE_INT_FILE_ID				  "file_id"				/**< file id for attach or alarm tone*/
#define CAL_VALUE_INT_CONTACT_ID			  "contact_id"			/**< contact id for birthday in contact list */
#define CAL_VALUE_INT_BUSY_STATUS			  "busy_status"			/**< ACS, G : Flag of busy or not */
#define CAL_VALUE_INT_SENSITIVITY			  "sensitivity"			/**< iCal:CLASS #cals_sensitivity_t */
#define CAL_VALUE_TXT_UID					  "uid"					  /**< ACS, G : Unique ID of the meeting item */
#define CAL_VALUE_INT_CALENDAR_TYPE			  "calendar_type"		/**< ACS, G : Type(all,phone,google) of calendar */
#define CAL_VALUE_TXT_ORGANIZER_NAME		  "organizer_name"		/**< ACS, G : Name of organizer(author) */
#define CAL_VALUE_TXT_ORGANIZER_EMAIL		  "organizer_email"		/**< ACS, G : Email of organizer */
#define CAL_VALUE_INT_MEETING_STATUS		  "meeting_status"		/**< ACS, G : The status of the meeting. */
#define CAL_VALUE_TXT_GCAL_ID				  "gcal_id"				/**< G : Server id of calendar */
#define CAL_VALUE_INT_DELETED				  "deleted"				/**< G : Flag for deleted */
#define CAL_VALUE_TXT_UPDATED				  "updated"				/**< G : Updated time stamp */
#define CAL_VALUE_INT_LOCATION_TYPE			  "location_type"		/**< G : Location type */
#define CAL_VALUE_TXT_LOCATION_SUMMARY		  "location_summary"	/**< G : A simple string value that can be used as a representation of this location */
#define CAL_VALUE_TXT_ETAG					  "etag"					/**< G : ETAG of this event */
#define CAL_VALUE_INT_CALENDAR_ID			  "calendar_id"			/**< G : id to map from calendar table */
#define CAL_VALUE_INT_SYNC_STATUS			  "sync_status"			/**< G : Indication for event entry whether added/ modified/ deleted */
#define CAL_VALUE_TXT_EDIT_URL				  "edit_uri"	/**< G : EditUri for google calendar */
#define CAL_VALUE_TXT_GEDERID				  "gevent_id"				/**< G : Server id of an event */
#define CAL_VALUE_INT_DST					  "dst"					  /**< dst of event */
#define CAL_VALUE_INT_ORIGINAL_EVENT_ID		  "original_event_id" /**< original event id for recurrency exception */
#define CAL_VALUE_INT_CALENDAR_INDEX      "calendar_index"   /**< specific calendar id - will be remove */
#define CAL_VALUE_DBL_LATITUDE         "latitude"      /**< latitude */
#define CAL_VALUE_DBL_LONGITUDE        "longitude"     /**< longitude */
#define CAL_VALUE_INT_EMAIL_ID				  "email_id"			/**< email id */
#define CAL_VALUE_INT_AVAILABILITY			  "availability"
#define CAL_VALUE_LLI_CREATED_TIME "created_time"
#define CAL_VALUE_LLI_COMPLETED_TIME "completed_time"
#define CAL_VALUE_INT_PROGRESS "progress"
#define CAL_VALUE_INT_IS_DELETED "is_deleted"/**< In deleting action, this is set 1 and will be deleted after sync */

#define CAL_VALUE_INT_CAL_TYPE				  "cal_type" /**< deprecated */

/**
 * @}
 */


/**
 * @addtogroup common
 * @{
 * brief
 * 		attendee cal_value's detail field
 */
#define CAL_VALUE_LST_ATTENDEE_LIST         "attendee_list"     /**< attendee's detail information set */
#define CAL_VALUE_TXT_ATTENDEE_DETAIL_NAME			"attendee_name"			/**< attendee_name */
#define CAL_VALUE_TXT_ATTENDEE_DETAIL_EMAIL		  "attendee_email"			/**< attendee_email */
#define CAL_VALUE_TXT_ATTENDEE_DETAIL_NUMBER		"attendee_number"			/**< attendee_email */
#define CAL_VALUE_INT_ATTENDEE_DETAIL_STATUS		"attendee_status"			/**< #cal_event_attendee_status_type_t */
#define CAL_VALUE_INT_ATTENDEE_DETAIL_TYPE			"attendee_type"			/**< #cal_event_attendee_type_t */
#define CAL_VALUE_INT_ATTENDEE_DETAIL_CT_INDEX		"attendee_ct_index"		/**< contact db index for reference */
#define CAL_VALUE_INT_ATTENDEE_ROLE					"attendee_role" /**< #cal_event_attendee_role_type_t */
#define CAL_VALUE_INT_ATTENDEE_RSVP					"attendee_rsvp"
#define CAL_VALUE_TXT_ATTENDEE_GROUP				"attendee_group"
#define CAL_VALUE_TXT_ATTENDEE_DELEGATOR_URI		"attendee_delegator_uri"
#define CAL_VALUE_TXT_ATTENDEE_DELEGATE_URI			"attendee_delegate_uri"
#define CAL_VALUE_TXT_ATTENDEE_UID					"attendee_uid"

/**
 * @}
 */

#define CAL_VALUE_LST_ALARM			        "alarm"    /**< exception's detail information set */
#define CAL_VALUE_LLI_ALARMS_TIME					"alarm_time"			/**< alarm time */
#define CAL_VALUE_INT_ALARMS_TICK					"remind_tick"			/**< Alarms before remindTick */
#define CAL_VALUE_INT_ALARMS_TICK_UNIT				"remind_tick_unit"	/**< Remind tick unit */
#define CAL_VALUE_TXT_ALARMS_TONE					"alarm_tone"			/**< Alert Sound File Name */
#define CAL_VALUE_TXT_ALARMS_DESCRIPTION "alarm_description"			/**< Alert description */
#define CAL_VALUE_INT_ALARMS_TYPE					"alarm_type"			/**< Alert type(see 'cal_alert_type_t') */
#define CAL_VALUE_INT_ALARMS_ID						"alarm_id"				/**< Alarm id */

/**
 * @}
 */



/**
 * @addtogroup common
 * @{
 * brief
 * 		delete flag in detail list
 */

#define CAL_VALUE_INT_DETAIL_DELETE     "is_deleted" /**< delete setting in detail list*/

/**
 * @}
 */



/**
 * @addtogroup common
 * @{
 * brief
 *		calendar_svc_value_xxx()'s argument for timezone
 */
/* type for timezone information save */

#define CAL_TZ_VALUE_INT_INDEX							"index"
#define CAL_TZ_VALUE_INT_TZ_OFFSET						"tz_offset_from_gmt"

#define CAL_TZ_VALUE_TXT_STD_NAME						"standard_name"
#define CAL_TZ_VALUE_INT_STD_START_MONTH				"std_start_month"
#define CAL_TZ_VALUE_INT_STD_START_POSITION_OF_WEEK		"std_start_position_of_week"
#define CAL_TZ_VALUE_INT_STD_START_DAY					"std_start_day"
#define CAL_TZ_VALUE_INT_STD_START_HOUR					"std_start_hour"
#define CAL_TZ_VALUE_INT_STD_BIAS						"standard_bias"

#define CAL_TZ_VALUE_TXT_DST_NAME						"day_light_name"
#define CAL_TZ_VALUE_INT_DST_START_MONTH				"day_light_start_month"
#define CAL_TZ_VALUE_INT_DST_START_POSITION_OF_WEEK		"day_light_start_position_of_week"
#define CAL_TZ_VALUE_INT_DST_START_DAY					"day_light_start_day"
#define CAL_TZ_VALUE_INT_DST_START_HOUR					"day_light_start_hour"
#define CAL_TZ_VALUE_INT_DST_BIAS						"day_light_bias"

/**
 * @}
 */



/**
 * @ingroup CALENDAR_SVC
 * @defgroup service_management service_management
 * @brief
 *		calendar service module management
 */

/**
 * @fn int calendar_svc_connect(void);
 *   This function opens database,it is must be called before other data operaion.
 *
 * @ingroup service_management
 * @return This function returns CAL_SUCCESS or error code on failure.
 * @exception None.
 * @remarks None.
 * @pre none
 * @post calendar_svc_close() should be called when leave.
 * @code
    #include <calendar-svc-provider.h>
    void sample_code()
    {
    	//connect to database
    	calendar_svc_connect();

    	//..do some operation to database

    	//close database
    	calendar_svc_close();
    }
 * @endcode
 * @see calendar_svc_close().
 */
int calendar_svc_connect(void);

/**
 * @fn int calendar_svc_close(void);
 *   This function closes database,it is must be called when leave.
 *
 * @ingroup service_management
 * @return This function returns CAL_SUCCESS or error code on failure.
 * @exception None.
 * @remarks None.
 * @pre the database connected
 * @post none
 * @code
   #include <calendar-svc-provider.h>
   void sample_code()
   {
   		//connect to database
    	calendar_svc_connect();

    	//..do some operation to database

    	//close database
    	calendar_svc_close();
   }
 * @endcode
 * @see calendar_svc_connect().
 */
int calendar_svc_close(void);

/**
 * @fn int calendar_svc_begin_trans(void);
 * This function start db transaction,it is coninient for user do many operaion once.
 *
 * @ingroup service_management
 * @return This function returns CAL_SUCCESS or error code on failure.
 * @exception None.
 * @remarks None.
 * @pre database connected
 * @post calendar_svc_end_trans() should be called when leave
 * @code
   #include <calendar-svc-provider.h>
   void sample_code()
   {
   		//connect to database
    	calendar_svc_connect();

		// begin transaction
		calendar_svc_begin_trans();

    	//..do some operation to database

    	// end transaction
		calendar_svc_end_trans();

    	//close database
    	calendar_svc_close();

   }
 * @endcode
 * @see calendar_svc_end_trans().
 */
int calendar_svc_begin_trans(void);

/**
 * @fn int calendar_svc_end_trans(bool is_success);
 * This function finishes database transaction of calendar service.
 * If it returns error, the transaction has been rollbacked.
 * When transction is success, it returns the last contacts version.
 *
 * @ingroup service_management
 * @param[in] is_success Commit changes if #true. Otherwise, no changes will be made on the database.
 * @return CAL_SUCCESS or the last calendar version(when success) on success,
 *         Negative value(#cal_error) on error
 * @exception None.
 * @remarks None.
 * @pre database connected and calendar_svc_begin_trans() is called.
 * @post none
 * @code
   #include <calendar-svc-provider.h>
   void sample_code()
   {
   		//connect to database
    	calendar_svc_connect();

		// begin transaction
		calendar_svc_begin_trans();

    	//..do some operation to database

    	// end transaction
		calendar_svc_end_trans(true);

    	//close database
    	calendar_svc_close();

   }
 * @endcode
 * @see calendar_svc_begin_trans().
 */
int calendar_svc_end_trans(bool is_success);


/**
 * @fn int calendar_svc_subscribe_db_change (const char *data_type,void(*cb)(void *), void *user_data);
 * This function registers callback function in receiver,it is convenient for user receive noti from database.
 *
 * @ingroup service_management
 * @param[in]	datatype for subscribe detail db change
 * @param[in]	cb Fuction pointer of calendar notification callback
 * @param[in]	user_data when cb function is called, user_data will be passed.
 * @return   This function returns CAL_SUCCESS or error code on failure.
 * @exception None.
 * @remarks None.
 * @pre database connected
 * @post none
 * @code
   #include <calendar-svc-provider.h>
   #include <glib.h>

   int received_cb (void *user_data)
   {
   	  if(NULL != user_data)
   	  {
   	  	printf("enter received_cb:%s\n",(char*)user_data);
   	  }
   }

   void sample_code()
   {
      //connect database
      calendar_svc_connect();

      char* user_data = "Get a noti!\n";
   	calendar_svc_subscribe_db_change (CAL_STRUCT_SCHEDULE,received_cb, user_data);

	  GMainLoop* loop = g_main_loop_new(NULL,TRUE);

   	  //close database
   	  calendar_svc_close();
   }
 * @endcode
 * @see calendar_svc_unsubscribe_db_change().
 */
int calendar_svc_subscribe_db_change (const char *data_type,void(*cb)(void *), void *user_data);


int calendar_svc_subscribe_change (void(*cb)(void *), void *user_data);


/**
 * @fn int calendar_svc_unsubscribe_db_change (const char *data_type,void(*cb)(void *));
 * This function deregisters callback function in receiver,it is convenient for user unscribe some receive noti from database.
 *
 * @ingroup service_management
 * @param[in]	datatype for subscribe detail db change
 * @param[in]	cb Fuction pointer of calendar notification callback
 * @return   This function returns CAL_SUCCESS or error code on failure.
 * @exception None.
 * @remarks None.
 * @pre calendar_svc_subscribe_change called
 * @post none
 * @code
   #include <calendar-svc-provider.h>

   int received_cb (void *user_data)
   {
   	  if(NULL != user_data)
   	  {
   	  	printf("enter received_cb:%s\n",(char*)user_data);
   	  }
   }

   void sample_code()
   {
       //connect database
	   calendar_svc_connect();

	   char* user_data = "Get a noti!\n";
	   calendar_svc_unsubscribe_db_change (CAL_STRUCT_SCHEDULE,received_cb, user_data);

	   cal_struct * event = calendar_svc_struct_new(CAL_STRUCT_SCHEDULE);
	   calendar_svc_insert(event);

	   GMainLoop* loop = g_main_loop_new(NULL,TRUE);
	   g_main_loop_run(loop);

	   //...

	   //close database
	   calendar_svc_close();
	   calendar_svc_unsubscribe_db_change (CAL_STRUCT_SCHEDULE,received_cb);
   }
 * @endcode
 * @see calendar_svc_subscribe_change().
 */
int calendar_svc_unsubscribe_db_change (const char *data_type,void(*cb)(void *));


int calendar_svc_unsubscribe_change (void(*cb)(void *));



/**
 * @defgroup event_management event_management
 * @ingroup CALENDAR_SVC
 * @brief
 *		major data access api
 */


/**
 * @fn int calendar_svc_insert(cal_struct *record);
 * This function insert records to database,user can save event through calling it.
 *
 * @ingroup event_management
 * @param[in] record calendar data for add
 * @return This function returns CAL_SUCCESS or error code on failure.
 * @exception None.
 * @remarks event should .
 * @pre database connected
 * @post none
 * @code
   #include <calendar-svc-provider.h>
   void sample_code()
   {
   	  //connect to database
   	  calendar_svc_connect();

   	  //create the variable
   	  cal_struct* event = calendar_svc_struct_new("schedule");

	  //insert the event
   	  calendar_svc_insert(event);

   	  //close database
   	  calendar_svc_close();
   }
 * @endcode
 * @see detail_management module
 */
int calendar_svc_insert(cal_struct *record);



/**
 * @fn int calendar_svc_update(cal_struct *record);
 * This function updates record to database,it is convenient for user to update some record.
 *
 * @ingroup event_management
 * @return This function returns CAL_SUCCESS or error code on failure.
 * @param[in] record calendar data for update
 * @return This function returns CAL_SUCCESS or error code on failure.
 * @exception None.
 * @remarks None.
 * @pre database connected
 * @post none
 * @code
   #include <calendar-svc-provider.h>
   void sample_code()
   {
   	  cal_struct* event = NULL;
   	  int index = 1;

   	  //connect to database
   	  calendar_svc_connect();

	  //get the record whose index is 1
   	  calendar_svc_get("schedule",index,NULL,&event);

	  //modify the summary
	  calendar_svc_struct_set_str(event,"summary","weekend");

   	  //update
   	  calendar_svc_update(event);

	  //free
   	  calendar_svc_free(&evnet);

	  //close database
   	  calendar_svc_close();
   }
 * @endcode
 * @see detail_management module
 */
int calendar_svc_update(cal_struct *record);

/**
 * @fn int calendar_svc_delete(const char *data_type,int index);
 * This function delete records from database,it is convenient for user to delete some record.
 *
 * @ingroup event_management
 * @return This function returns CAL_SUCCESS or error code on failure.
 * @param[in] data_type sepecific record type
 * @param[in] index event db index
 * @return This function returns CAL_SUCCESS or error code on failure.
 * @exception None.
 * @remarks None.
 * @pre database connected
 * @post none
 * @code
   #include <calendar-svc-provider.h>
   void sample_code()
   {
   	  int index = 1;

   	  //connect to database
   	  calendar_svc_connect();

	  //delete the record whose index is 1
   	  calendar_svc_delete("schedule",index);

	  //free
   	  calendar_svc_free(&evnet);

	  //close database
   	  calendar_svc_close();
   }
 * @endcode
 * @see detail_management module
 */
int calendar_svc_delete(const char *data_type,int index);

/**
 * @fn int calendar_svc_delete_all(int account_id,const char *data_type);
 * This function delete all records from database,it is convenient for user to delete all of records.
 * local account deletes data immediately but the others set is_deleted parameter 1.
 *
 * @ingroup event_management
 * @return This function returns CAL_SUCCESS or error code on failure.
 * @param[in] account_id account db index
 * @param[in] data_type detail data type(eg. CAL_STRUCT_CALENDAR,CAL_STRUCT_SCHEDULE, NULL), if null delete all data by account_id
 * @return This function returns CAL_SUCCESS or error code on failure.
 * @exception None.
 * @remarks None.
 * @pre database connected
 * @post none
 * @code
   #include <calendar-svc-provider.h>
   void sample_code()
   {
	  //connect to database
   	  calendar_svc_connect();

	  //delete the all records of schudule_table
   	  calendar_svc_delete_all(0,"schedule");

	  //close database
   	  calendar_svc_close();
   }
 * @endcode
 * @see detail_management module
 */
int calendar_svc_delete_all(int account_id,const char *data_type);

/**
 * @fn int calendar_svc_delete_account(int account_id);
 * This function delete all records from database,it is convenient for user to delete all of records according to account.
 * local account deletes data immediately but the others set is_deleted parameter 1.
 *
 * @ingroup event_management
 * @return This function returns CAL_SUCCESS or error code on failure.
 * @param[in] account_id account db index
 * @return This function returns CAL_SUCCESS or error code on failure.
 * @exception None.
 * @remarks None.
 * @pre database connected
 * @post none
 * @code
   #include <calendar-svc-provider.h>
   void sample_code()
   {
	  //connect to database
   	  calendar_svc_connect();

	  //delete the all records of schudule_table
   	  calendar_svc_delete_account(0);

	  //close database
   	  calendar_svc_close();
   }
 * @endcode
 * @see detail_management module
 */

int calendar_svc_delete_account(int account_id);


/**
 * @fn int calendar_svc_clean_after_sync(int calendar_id);
 * This function clean deleted(marked) all records from database,which is used to remove data from database after sync operation.
 *
 * @ingroup event_management
 * @return This function returns CAL_SUCCESS or error code on failure.
 * @param[in] account_id calendar id
 * @return This function returns CAL_SUCCESS or error code on failure.
 * @exception None.
 * @remarks None.
 * @pre database connected
 * @post none
 * @code
   #include <calendar-svc-provider.h>
   void sample_code()
   {
	  //connect to database
   	  calendar_svc_connect();

	  //delete the all records from schudule_table
   	  calendar_svc_clean_after_sync(0);

	  //close database
   	  calendar_svc_close();
   }
 * @endcode
 * @see detail_management module
 */

int calendar_svc_clean_after_sync(int calendar_id);

/**
 * @fn int calendar_svc_get(const char *data_type,int index,const char *field_list, cal_struct **record);
 * This function get records from database,user can get event from database through calling it.
 *
 * @ingroup event_management
 * @return This function returns inserted contact id or error code on failure.
 * @param[in] data_type sepecific record type
 * @param[in] index db index
 * @param[in] field_list specific field list(eg. "summary,description"), if NULL, all field is returned.
 * @param[out] record calendar data , it should be free by calendar_svc_struct_free
 * @return This function returns CAL_SUCCESS or error code on failure.
 * @exception None.
 * @remarks event should .
 * @pre database connected
 * @post none
 * @code
   #include <calendar-svc-provider.h>
   void sample_code()
   {
   	  int index = 1;
   	  cal_struct* event = NULL;

   	  //connect to database
   	  calendar_svc_connect();

   	  //get the record whose index is 1
   	  calendar_svc_get("schedule",index,NULL, &event);

	  //free the space
	  calendar_svc_struct_free(&event);

   	  //close database
   	  calendar_svc_close();
   }
 * @endcode
 * @see detail_management module
 */
int calendar_svc_get(const char *data_type,int index,const char *field_list, cal_struct **record);

/**
 * @fn int calendar_svc_get_count(int account_id,int calendar_id,const char *data_type);
 * This function get count of records from database,user can get the count through calling it.
 *
 * @ingroup event_management
 * @return Integer value, or 0 if no value is obtained
 * @param[in] account_id account db index
 * @param[in] calendar_id calendar id(will be support phase 2)
 * @param[in] data_type data_type(CAL_STRUCT_CALENDAR or CAL_STRUCT_SCHEDULE)
 * @return This function returns CAL_SUCCESS or error code on failure.
 * @exception None.
 * @remarks event should .
 * @pre database connected
 * @post none
 * @code
   #include <calendar-svc-provider.h>
   void sample_code()
   {
   	  int count = -1;

   	  //connect to database
   	  calendar_svc_connect();

   	  //get the count of all
   	  count = calendar_svc_get_count(0,0,"shchedule");

	  //close database
   	  calendar_svc_close();
   }
 * @endcode
 * @see detail_management module
 */
int calendar_svc_get_count(int account_id,int calendar_id,const char *data_type);

int calendar_svc_calendar_get_count(int account_id);
int calendar_svc_event_get_count(int calendar_id);
int calendar_svc_todo_get_count(int calendar_id);

/**
 * @fn int calendar_svc_get_all(int account_id,int calendar_id,const char *data_type, cal_iter **iter);
 * This function get all records from database,it is convenient for user to get all of the reocrds once.
 *
 * @ingroup event_management
 * @return This function returns CAL_SUCCESS or error code on failure.
 * @param[in] account_id account db index
 * @param[in] calendar_id calendar id. If account_id is set, the account_id will be ignore.
 * @param[in] data_type data_type(CAL_STRUCT_CALENDAR or CAL_STRUCT_SCHEDULE)
 * @param[out] iter calendar data
 * @return This function returns CAL_SUCCESS or error code on failure.
 * @exception None.
 * @remarks event should .
 * @pre database connected
 * @post call calendar_svc_iter_remove() when leave
 * @code
   #include <calendar-svc-provider.h>
   void sample_code()
   {
   	  cal_iter *iter = NULL;

   	  //connect to database
   	  calendar_svc_connect();

   	  //get all records
   	  calendar_svc_get_all(0,0,"schedule", &iter);

	  //free
   	  calendar_svc_iter_remove(&iter);

	  //close database
   	  calendar_svc_close();
   }
 * @endcode
 * @see detail_management module
 */
int calendar_svc_get_all(int account_id,int calendar_id,const char *data_type, cal_iter **iter);

/**
 * @fn int calendar_svc_event_get_changes(int calendar_id, int version, cal_iter **iter);
 * This function provides the iterator to get all changes later than the version.
 *
 * @ingroup event_management
 * @return This function returns CAL_SUCCESS or error code on failure.
 * @param[in] calendar_id calendar ID
 * @param[in] version version number
 * @param[out] iter interation struct for list travel
 * @return This function returns CAL_SUCCESS or error code on failure.
 * @exception None.
 * @remarks None.
 * @pre database connected
 * @post none
 * @code
   #include <calendar-svc-provider.h>
   #include <stdio.h>
   void sample_code()
   {
      int ret;
      cal_struct *cs;
      cal_iter *it;
      int id, type, ver;

      calendar_svc_event_get_changes(1, 0, &it);

      while (calendar_svc_iter_next(it) == CAL_SUCCESS) {
         cs = NULL;
         ret = calendar_svc_iter_get_info(it, &cs);
         if (ret != CAL_SUCCESS) {
            printf("calendar_svc_iter_get_info failed (%d)\n", ret);
            return -1;
         }
         id = calendar_svc_struct_get_int(cs, CALS_STRUCT_UPDATED_INT_ID);
         type = calendar_svc_struct_get_int(cs, CALS_STRUCT_UPDATED_INT_TYPE);
         ver = calendar_svc_struct_get_int(cs, CALS_STRUCT_UPDATED_INT_VERSION);
         printf("type = %d id = %d ver = %d\n", id, type, ver);
         calendar_svc_struct_free(&cs);
      }
      calendar_svc_iter_remove(&it);
   }
 * @endcode
 * @see detail_management module
 */
int calendar_svc_event_get_changes(int calendar_id, int version, cal_iter **iter);

/**
 * @fn int calendar_svc_todo_get_changes(int calendar_id, int version, cal_iter **iter);
 * This function provides the iterator to get all changes later than the version.
 *
 * @ingroup event_management
 * @return This function returns CAL_SUCCESS or error code on failure.
 * @param[in] calendar_id calendar ID
 * @param[in] version version number
 * @param[out] iter interation struct for list travel
 * @return This function returns CAL_SUCCESS or error code on failure.
 * @exception None.
 * @remarks None.
 * @pre database connected
 * @post none
 * @code
   #include <calendar-svc-provider.h>
   #include <stdio.h>
   void sample_code()
   {
      int ret;
      cal_struct *cs;
      cal_iter *it;
      int id, type, ver;

      calendar_svc_todo_get_changes(1, 0, &it);

      while (calendar_svc_iter_next(it) == CAL_SUCCESS) {
         cs = NULL;
         ret = calendar_svc_iter_get_info(it, &cs);
         if (ret != CAL_SUCCESS) {
            printf("calendar_svc_iter_get_info failed (%d)\n", ret);
            return -1;
         }
         id = calendar_svc_struct_get_int(cs, CALS_STRUCT_UPDATED_INT_ID);
         type = calendar_svc_struct_get_int(cs, CALS_STRUCT_UPDATED_INT_TYPE);
         ver = calendar_svc_struct_get_int(cs, CALS_STRUCT_UPDATED_INT_VERSION);
         printf("type = %d id = %d ver = %d\n", id, type, ver);
         calendar_svc_struct_free(&cs);
      }
      calendar_svc_iter_remove(&it);
   }
 * @endcode
 * @see detail_management module
 */
int calendar_svc_todo_get_changes(int calendar_id, int version, cal_iter **iter);

int calendar_svc_convert_id_to_uid(const char *data_type,int index,char **uid);

/**
 * @fn int calendar_svc_iter_get_info(cal_iter *iter, cal_struct **row_record);
 * This function get cal_value by cal_iter,it is convenient for user to get event from iter.
 *
 * @ingroup event_management
 * @return This function returns CAL_SUCCESS or error code on failure.
 * @param[in] iter interation struct for list travel
 * @param[out] row_record detail information
 * @exception None.
 * @remarks row_record should be free with calendar_svc_struct_free
 * @pre database connected
 * @post none
 * @code
   #include <calendar-svc-provider.h>
   void sample_code()
   {
   	  cal_iter *iter = NULL;
   	  cal_struct* evnet = NULL;

	  //connect to database
   	  calendar_svc_connect();

	  //get all records
	  calendar_svc_get_all(0,0,"schedule", &iter);

	  //get events
   	  calendar_svc_iter_get_info(iter, &event);

   	  //free
   	  calendar_svc_iter_remove(&iter);

	  //close database
   	  calendar_svc_close();
   }
 * @endcode
 * @see detail_management module
 */
int calendar_svc_iter_get_info(cal_iter *iter, cal_struct **row_record);


/**
 * @fn int calendar_svc_iter_next(cal_iter *iter);
 * This function get cal_value by cal_iter,it is convenient for user to get record one by one.
 *
 * @ingroup event_management
 * @return This function returns CAL_SUCCESS or error code on failure.
 * @param[in] iter interation struct for list travel
 * @exception None.
 * @remarks None.
 * @pre database connected
 * @post none
 * @code
   #include <calendar-svc-provider.h>
   void sample_code()
   {
   	  cal_iter *iter = NULL;
   	  cal_struct* evnet = NULL;

	  //connect to database
   	  calendar_svc_connect();

	  //get all records
	  calendar_svc_get_all(0,0,"schedule", &iter);

	  //get events
   	  calendar_svc_iter_get_info(iter, &event);

	  //get next event
   	  calendar_svc_iter_next(iter);
   	  calendar_svc_iter_get_info(iter, &event);

   	  //free
   	  calendar_svc_iter_remove(&iter);

	  //close database
   	  calendar_svc_close();
   }
 * @endcode
 * @see detail_management module
 */
int calendar_svc_iter_next(cal_iter *iter);


/**
 * @fn int calendar_svc_iter_remove(cal_iter **iter);
 * This function remove db iteration struct,it is convenient for user to avoid memory leak.
 *
 * @ingroup event_management
 * @return This function returns CAL_SUCCESS or error code on failure.
 * @param[in] iter interation struct for list travel
 * @exception None.
 * @remarks None.
 * @pre database connected
 * @post none
 * @code
   #include <calendar-svc-provider.h>
   void sample_code()
   {
   	  cal_iter *iter = NULL;

	  //connect to database
   	  calendar_svc_connect();

	  //get all records
	  calendar_svc_get_all(0,0,"schedule", &iter);

   	  //free
   	  calendar_svc_iter_remove(&iter);

	  //close database
   	  calendar_svc_close();
   }
 * @endcode
 * @see detail_management module
 */
int calendar_svc_iter_remove(cal_iter **iter);

/**
 * @defgroup detail_management detail_management
 * @ingroup CALENDAR_SVC
 * @brief
 *		deatil field access api
 */


/**
 * @fn cal_struct * calendar_svc_struct_new(const char *data_type);
 * This function alloc calendar struct,it is convenient for user to create an event.
 *
 * @ingroup detail_management
 * @return This function returns allocated event struct
 * @param[in] data_type (eg.CAL_EVENT_TYPE_SCHEDULE,CAL_EVENT_TYPE_CALENDAR..)
 * @exception None.
 * @remarks it should be free with calendar_svc_struct_free
 * @pre cal_struct variable is defined.
 * @post None.
 * @remarks None.
 * @code
   #include <calendar_svc_provider.h>
   void sample_code()
   {
	//connect to database
   	calendar_svc_connect();

   	//create variable
    cal_struct * event = calendar_svc_struct_new("schedule");

    //free the space
    calendar_svc_struct_free(&event);

	//close the database
   	calendar_svc_close();
   }
 * @endcode
 * @see calendar_svc_struct_free().
 */
cal_struct * calendar_svc_struct_new(const char *data_type);

/**
 * @fn int calendar_svc_struct_free(cal_struct **record);
 * This function is used to free space malloced to cal_struct variable,it is convenient for user to avoid memory leak when using cal_struct.
 *
 * @ingroup detail_management
 * @param[in] record Point to alloced address
 * @return This function returns CAL_SUCCESS or error code on failure.
 * @pre cal_struct variable is defined.
 * @post None.
 * @remarks None.
 * @code
   #include <calendar_svc_provider.h>
   void sample_code()
   {
	//connect to database
   	calendar_svc_connect();

   	//create variable
    cal_struct * event = calendar_svc_struct_new("schedule");

    //free the space
    calendar_svc_struct_free(&event);

	//close the database
   	calendar_svc_close();
   }
 * @endcode
 * @see common, CAL_STRUCT_TYPE , calendar_svc_struct_new.
 */
int calendar_svc_struct_free(cal_struct **record);


/**
 * @fn int calendar_svc_struct_get_str(cal_struct* record, const char* field);
 * This function gets the point of string value of the calendar service value,it is convenient for user get the value needed without knowing the detail of the struct.
 *
 * @ingroup detail_management
 * @param[in] record Point to The calendar struct
 * @param[in] field The index of the string value in calendar service value.
 * @return string value(should not be freed), or NULL if no value is obtained
 * @remarks if parent cal_struct is destroyed, return string is not valid.
 * @pre cal_struct varibale is defined.
 * @post none
 * @code
 	#include <calendar-svc-provider.h>
 	void sample_code()
 	{
		char* summary = NULL;
		index = 1;
		cal_struct* event = NULL;

 		//connect to database
 		calendar_svc_connect();

 		//get the record
 		calendar_svc_get("schedule",index,NULL,&event);

		//get the str value
 		summary = calendar_svc_struct_get_str(event,CAL_VALUE_TXT_SUMMARY);

 		//free space
		calendar_svc_free(&event);

		//close database
 		calendar_svc_close();
 	}
 * @endcode
 * @see calendar_svc_struct_set_str().
 */
char *calendar_svc_struct_get_str(cal_struct* record, const char *field);

/**
 * @fn int calendar_svc_struct_get_int(cal_struct* record, const char* field);
 * This function gets Integer value of the calendar service value,it is convenient for user get the value needed without knowing the detail of the struct.
 *
 * @ingroup detail_management
 * @param[in] record Point to The calendar struct
 * @param[in] field The index of the integer value in calendar service value.
 * @return Integer value, or 0 if no value is obtained
 * @remarks none
 * @pre cal_struct varibale is defined.
 * @post none
 * @code
 	#include <calendar-svc-provider.h>
 	void sample_code()
 	{
		int account_id = 0;
		index = 1;
		cal_struct* event = NULL;

 		//connect to database
 		calendar_svc_connect();

 		//get the record
 		calendar_svc_get("schedule",index,NULL,&event);

		//get the int value
 		account_id = calendar_svc_struct_get_int(event,CAL_VALUE_INT_ACCOUNT_ID);

 		//free space
		calendar_svc_free(&event);

		//close database
 		calendar_svc_close();
 	}
 * @endcode
 * @see calendar_svc_struct_set_int().
 */
int calendar_svc_struct_get_int(cal_struct* record, const char *field);


/**
 * @fn double calendar_svc_struct_get_double(cal_struct* record, const char* field);
 * This function gets double value of the calendar service value,it is convenient for user get the value needed without knowing the detail of the struct.
 *
 * @ingroup detail_management
 * @param[in] record Point to The calendar struct
 * @param[in] field The index of the integer value in calendar service value.
 * @return double value, or 0.0 if no value is obtained
 * @remarks none
 * @pre cal_struct varibale is defined.
 * @post none
 * @code
 	#include <calendar-svc-provider.h>
 	void sample_code()
 	{
		double latitude = 0.0;
		index = 1;
		cal_struct* event = NULL;

 		//connect to database
 		calendar_svc_connect();

 		//get the record
 		calendar_svc_get("schedule",index,NULL,&event);

		//get the double value
 		latitude = calendar_svc_struct_get_double(event,CAL_VALUE_DBL_LATITUDE);

		//free space
		calendar_svc_free(&event);

		//close database
 		calendar_svc_close();
 	}
 * @endcode
 * @see calendar_svc_struct_set_double().
 */
double calendar_svc_struct_get_double(cal_struct* record, const char *field);


/**
 * @fn int calendar_svc_struct_set_double(cal_struct* record, const char* field,double value);
 * This function sets double value of the calendar service value,it is convenient for user set the value without knowing the detail of the struct.
 *
 * @ingroup detail_management
 * @param[in] record Point to The calendar struct
 * @param[in] field The index of the integer value in calendar service value.
 * @param[in] value The dobule value to be set.
 * @return Integer value, or 0 if no value is obtained
 * @remarks none.
 * @pre cal_struct variable is defined.
 * @post the corresponding value of cal_struct is set.
 * @code
 	#include <calendar-svc-provider.h>
 	void sample_code()
 	{
		double latitude = 3.14;
		index = 0;
		cal_struct* event = NULL;

 		//connect to database
 		calendar_svc_connect();

		//create a cal_struct variable
		event = calendar_svc_struct_new("schedule");

		//set the double value
 		calendar_svc_set_double(event,CAL_VALUE_DBL_LATITUDE,latitude);

 		//insert the record
 		index = calendar_svc_insert(event);

		//free the space
 		calendar_svc_struct_free(&event);

		//close database
 		calendar_svc_close();
 	}
 * @endcode
 * @see calendar_svc_struct_get_double().
 */
int calendar_svc_struct_set_double(cal_struct* record, const char *field,double value);


/**
 * @fn int calendar_svc_struct_set_int(cal_struct* record, const char* field, int intval);
 * This function sets integer value to the calendar service value,it is convenient for user set the value without knowing the detail of the struct.
 *
 * @ingroup detail_management
 * @param[in] record Point to The calendar struct
 * @param[in] field The index of the integer value in calendar service value.
 * @param[in] intval The integer value to be set.
 * @return	This function returns CAL_SUCCESS or error code on failure.
 * @remarks none.
 * @pre cal_struct variable is defined.
 * @post the corresponding value of cal_struct is set.
 * @code
 	#include <calendar-svc-provider.h>
 	void sample_code()
 	{
		int account_id = 1;
		index = 0;
		cal_struct* event = NULL;

 		//connect to database
 		calendar_svc_connect();

		//create a cal_struct variable
		event = calendar_svc_struct_new("schedule");

		//set the int value
 		calendar_svc_set_int(event,CAL_VALUE_INT_ACCOUNT_ID,account_id);

 		//insert the record
 		index = calendar_svc_insert(event);

		//free the space
 		calendar_svc_struct_free(&event);

		//close database
 		calendar_svc_close();
 	}
 * @endcode
 * @see calendar_svc_struct_get_int().
 */
int calendar_svc_struct_set_int(cal_struct* record, const char *field, int intval);

/**
 * @fn int calendar_svc_struct_set_str(cal_struct* record, const char* field, const char *strval);
 * This function sets string value to the calendar service value,it is convenient for user set the value without knowing the detail of the struct.
 * If it is in struct, free old string and copy strval to struct.
 *
 * @ingroup detail_management
 * @param[in] record Point to The calendar struct
 * @param[in] field The index of the string value in calendar service value.
 * @param[in] strval The string value to be set.
 * @return	This function returns CAL_SUCCESS or error code on failure.
 * @remarks none.
 * @pre cal_struct variable is defined.
 * @post the corresponding value of cal_struct is set.
 * @code
 	#include <calendar-svc-provider.h>
 	void sample_code()
 	{
		char* summary = "party";
		index = 0;
		cal_struct* event = NULL;

 		//connect to database
 		calendar_svc_connect();

		//create a cal_struct variable
		event = calendar_svc_struct_new("schedule");

		//set the string value
 		calendar_svc_set_str(event,CAL_VALUE_TXT_SUMMARY,summary);

 		//insert the record
 		index = calendar_svc_insert(event);

		//free the space
 		calendar_svc_struct_free(&event);

		//close database
 		calendar_svc_close();
 	}
 * @endcode
 * @see calendar_svc_struct_get_str().
 */
int calendar_svc_struct_set_str(cal_struct* record, const char *field, const char *strval);


/**
 * @fn int calendar_svc_struct_get_list(cal_struct* record, const char* field, GList** retlist);
 * This function gets the point of glib double-linked list in the calendar service struct,it is convenient for user get the value without knowing the detail of the struct.
 *
 * @ingroup detail_management
 * @param[in] record structure A calendar service struct
 * @param[in] field The index of the glib singly-linked list in calendar service struct.
 * @param[out] retlist the glib singly-linked list requested with field(should not be freed or removed)
 * @return	This function returns CAL_SUCCESS or error code on failure.
 * @remarks if parent cal_struct is destroyed, retlist is not valid.
 * @pre cal_struct variable is defined.
 * @post none.
 * @code
 	#include <calendar-svc-provider.h>
 	void sample_code()
 	{
		GList* list = NULL;
		index = 1;
		cal_struct* event = NULL;

 		//connect to database
 		calendar_svc_connect();

		//get the record
 		calendar_svc_get("schedule",index,NULL,&event);

		//get the list
		calendar_svc_struct_get_list(event,"attendee_list",&list);

		//free the space
 		calendar_svc_struct_free(&event);

		//close database
 		calendar_svc_close();
 	}
 * @endcode
 * @see calendar_svc_struct_store_list().
 */
int calendar_svc_struct_get_list(cal_struct* record,const char *field, GList** retlist);

/**
 * @fn int calendar_svc_struct_store_list(cal_struct* record, const char* field, GList* list)
 * This function sets the glib double-linked list to the calendar service struct,it is convenient for user set the value.
 * \n Values(cal_value) of the list are moved to the calendar service struct. But the list is copied.
 *
 * @ingroup detail_management
 * @param[in] record structure A calendar service struct
 * @param[in] field The index of the glib singly-linked list in calendar service struct.
 * @param[in] list the glib singly-linked list to be set
 * @return	This function returns CAL_SUCCESS or error code on failure.
 * @remarks if parent cal_struct is destroyed, GSList is not valid.
 * @pre cal_struct variable is defined.
 * @post none.
 * @code
 	#include <calendar-svc-provider.h>
 	void sample_code()
 	{
		GList* list = NULL;
		index = 1;
		cal_struct* event = NULL;

 		//connect to database
 		calendar_svc_connect();

		//create the event
 		event = calendar_svc_struct_new("schedule");

		//set the list
		calendar_svc_struct_store_list(event,"attendee_list",list);

		//free the space
 		calendar_svc_struct_free(&event);

		//close database
 		calendar_svc_close();
 	}
 * @endcode
 * @see calendar_svc_struct_get_list().
 */
int calendar_svc_struct_store_list(cal_struct* record,const char *field, GList* list);

/**
 * @fn int calendar_svc_value_new(const char* val_type);
 * Allocate, initialize and return a new calendar service value,it is convenient for user to create a calendar service value.
 *
 * @ingroup detail_management
 * @param[in] val_type The type of calendar service value
 * @return The pointer of New calendar service value, NULL on error
 * @remarks none.
 * @pre none.
 * @post none.
 * @code
 	#include <calendar-svc-provider.h>
 	void sample_code()
 	{
		GList* list = NULL;
		index = 1;
		cal_value* event = NULL;

 		//connect to database
 		calendar_svc_connect();

		//create the event
 		event = calendar_svc_value_new("attendee_list");

		//set the list
		list = g_list_append(list,event);
		calendar_svc_struct_store_list(event,"attendee_list",list);

		//free the space
 		calendar_svc_value_free(&event);

		//close database
 		calendar_svc_close();
 	}
 * @endcode
 * @see calendar_svc_value_free().
 */
cal_value* calendar_svc_value_new(const char *val_type);

/**
 * @fn int calendar_svc_value_free(cal_value** value);
 * A destructor for calendar service value,,it is convenient for user to free the space allocated.
 * If it is in struct, return CAL_ERR_ARG_INVALID.
 *
 * @ingroup detail_management
 * @param[in] value A calendar service value
 * @return	This function returns CAL_SUCCESS or error code on failure.
 * @remarks none.
 * @pre cal_value variable is defined.
 * @post none.
 * @code
 	#include <calendar-svc-provider.h>
 	void sample_code()
 	{
		GList* list = NULL;
		index = 1;
		cal_value* event = NULL;

 		//connect to database
 		calendar_svc_connect();

		//create the event
 		event = calendar_svc_value_new("attendee_list");

		//set the list
		list = g_list_append(list,event);
		calendar_svc_struct_store_list(event,"attendee_list",list);

		//free the space
 		calendar_svc_value_free(&event);

		//close database
 		calendar_svc_close();
 	}
 * @endcode
 * @see calendar_svc_value_new().
 */
int calendar_svc_value_free(cal_value** value);

/**
 * @fn int calendar_svc_value_set_int(cal_value* value, const char* field, int intval);
 * This function sets integer value to the calendar service value,it is convenient for user set value of cal_value varible.
 *
 * @ingroup detail_management
 * @param[in] value The calendar service value
 * @param[in] field The index of the integer value in calendar service value.
 * @param[in] intval The integer value to be set.
 * @return	This function returns CAL_SUCCESS or error code on failure.
 * @remarks none.
 * @pre cal_value variable is defined.
 * @post none.
 * @code
 	#include <calendar-svc-provider.h>
 	void sample_code()
 	{
		GList* list = NULL;
		index = 1;
		cal_value* event = NULL;

 		//connect to database
 		calendar_svc_connect();

		//create the event
 		event = calendar_svc_value_new("attendee_list");
 		calendar_svc_value_set_int(event,"attendee_status",1);

		//set the list
		list = g_list_append(list,event);
		calendar_svc_struct_store_list(event,"attendee_list",list);

		//free the space
 		calendar_svc_value_free(&event);

		//close database
 		calendar_svc_close();
 	}
 * @endcode
 * @see calendar_svc_value_get_int().
 */
int calendar_svc_value_set_int(cal_value* value, const char *field, int intval);

int calendar_svc_value_set_lli (cal_value *value, const char *field, long long int llival);
long long int calendar_svc_value_get_lli (cal_value *value, const char *field);
/**
 * @fn int calendar_svc_value_set_str(cal_value* value, const char* field, const char *strval);
 * This function sets string value to the calendar service value,it is convenient for user set value of cal_value varible.
 * If it is in struct, free old string and copy strval to struct.
 *
 * @ingroup detail_management
 * @param[in] value The calendar service value
 * @param[in] field The index of the string value in calendar service value.
 * @param[in] strval The string value to be set.
 * @return	This function returns CAL_SUCCESS or error code on failure.
 * @remarks none.
 * @pre cal_value variable is defined.
 * @post none.
 * @code
 	#include <calendar-svc-provider.h>
 	void sample_code()
 	{
		GList* list = NULL;
		index = 1;
		cal_value* event = NULL;

 		//connect to database
 		calendar_svc_connect();

		//create the event
 		event = calendar_svc_value_new("attendee_list");
 		calendar_svc_value_set_str(event,"attendee_name","Max");

		//set the list
		list = g_list_append(list,event);
		calendar_svc_struct_store_list(event,"attendee_list",list);

		//free the space
 		calendar_svc_value_free(&event);

		//close database
 		calendar_svc_close();
 	}
 * @endcode
 * @see calendar_svc_value_get_str().
 */
int calendar_svc_value_set_str(cal_value* value, const char *field, const char *strval);


/**
 * @fn int calendar_svc_value_get_int(cal_value* value, const char* field);
 * This function gets Integer value of the calendar service value,it is convenient for user get value of cal_value varible.
 *
 * @ingroup detail_management
 * @param[in] value The calendar service value
 * @param[in] field The index of the integer value in calendar service value.
 * @return Integer value, or 0 if no value is obtained
 * @remarks none.
 * @pre cal_value variable is defined.
 * @post none.
 * @code
 	#include <calendar-svc-provider.h>
 	void sample_code()
 	{
 		int type = 0;
		cal_value* event = NULL;

 		//connect to database
 		calendar_svc_connect();

		//create the event
 		event = calendar_svc_value_new(CAL_VALUE_LST_ATTENDEE_LIST);

		//get the type value
 		type = calendar_svc_value_get_int(event,
			CAL_VALUE_INT_ATTENDEE_DETAIL_TYPE);

		//free the space
 		calendar_svc_value_free(&event);

		//close database
 		calendar_svc_close();
 	}
 * @endcode
 * @see calendar_svc_value_set_int().
 */
int calendar_svc_value_get_int(cal_value* value, const char *field);

/**
 * @fn char* calendar_svc_value_get_str(cal_value* value, const char* field);
 * This function gets the point of string value of the calendar service value,it is convenient for user get value of cal_value varible.
 *
 * @ingroup detail_management
 * @param[in] value The calendar service value
 * @param[in] field The index of the string value in calendar service value.
 * @return string value(should not be freed), or NULL if no value is obtained
 * @remarks none.
 * @pre cal_value variable is defined.
 * @post none.
 * @code
 	#include <calendar-svc-provider.h>
 	void sample_code()
 	{
 		char* name = "money";
		cal_value* event = NULL;

 		//connect to database
 		calendar_svc_connect();

		//create the event
 		event = calendar_svc_value_new(CAL_VALUE_LST_ATTENDEE_LIST);

		//get the event_id value
 		name = calendar_svc_value_get_str(event,
			CAL_VALUE_TXT_ATTENDEE_DETAIL_NAME);

		//free the space
 		calendar_svc_value_free(&event);

		//close database
 		calendar_svc_close();
 	}
 * @endcode
 * @see calendar_svc_value_set_str().
 */
char *calendar_svc_value_get_str(cal_value* value, const char *field);

/**
 * @fn int calendar_svc_util_convert_event_to_vcs (cal_struct *record,char **raw_data,int *data_size);
 * This function converts data (cal_struct(event) to raw_data(vcal format)),it is convenient for user to convert.
 *
 * @ingroup utilities
 * @param[in] record      original record type
 * @param[out]	raw_data	    vcalendar event raw data
 * @param[out]	data_size	   raw_data buf size
 * @return	 This function returns CAL_SUCCESS or error code on failure.
 * @remarks none
 * @pre none.
 * @post none
 * @code
	#include <calendar_svc_provider.h>
	void sample_code()
	{
		char raw_data = NULL;
		int data_size = 0;

		//connect to database
		calendar_svc_connect();

		cal_struct* event = NULL;

		//get the record
		calendar_svc_get("schedule",1,NULL,&event);

		//convert
		calendar_svc_util_convert_event_to_vcs (event,&raw_data,&data_size);

		calendar_svc_struct_free(&event);

		//close to database
		calendar_svc_close();
	}
 * @endcode
 */
DEPRECATED int calendar_svc_util_convert_event_to_vcs (cal_struct *record,char **raw_data,int *data_size);


/**
 * @fn int calendar_svc_find_event_list(int account_id,const char* search_type,const void* search_value, cal_iter **iter);
 * This function get records from database by search param,it is convenient for user to get records according to some condition.
 *
 * @ingroup event_management
 * @return This function returns CAL_SUCCESS or error code on failure.
 * @param[in] account_id account db index
 * @param[in] search_type event search type(eg. CAL_VALUE_SUMMARY or CAL_VALUE_DESCRIPTION,..)
 * @param[in] search_value event search value(eg. "weekly report", etc.. ), it can be integer value(eg. 1 or 2.. etc)
 * @param[out] iter interation struct for list travel
 * @return This function returns CAL_SUCCESS or error code on failure.
 * @exception None.
 * @remarks None.
 * @pre database connected
 * @post none
 * @code
  #include <calendar-svc-provider.h>
  void sample_code()
  {
  	 cal_iter *iter = NULL;

	 //connect to database
  	 calendar_svc_connect();

	 //find event whose summary including string like "party"
  	 calendar_svc_find_event_list(0,"summary","party", &iter);

  	 //free
  	 calendar_svc_iter_remove(&iter);

	 //close database
  	 calendar_svc_close();
  }
 * @endcode
 * @see detail_management module
 * @deprecated it will replacement calendar_svc_find_list
 */
int calendar_svc_find_event_list(int account_id,const char *search_type,const void* search_value, cal_iter **iter);

/**
 * @fn int calendar_svc_event_search(int field, const char *keyword, cal_iter **iter);
 * #calendar_svc_event_search searches events including the keyword in given fields.
 *
 * @ingroup event_management
 * @return This function returns CAL_SUCCESS or error code on failure.
 * @param[in] field fields where the keyword is searched.  #SEARCHFIELD
 * @param[in] keyword keyword to be searched
 * @param[out] iter interation struct for list travel
 * @return This function returns CAL_SUCCESS or error code on failure.
 * @exception None.
 * @remarks None.
 * @pre database connected
 * @post none
 * @code
  #include <calendar-svc-provider.h>
  void sample_code()
  {
	int ret;
	cal_struct *cs;
	cal_iter *it;
	char *summary;
	char *desc;
	int id;

	calendar_svc_connect();
	int search_field;

	search_field = CALS_SEARCH_FIELD_NONE;
	search_field |= CALS_SEARCH_FIELD_SUMMARY;
	search_field |= CALS_SEARCH_FIELD_DESCRIPTION;
	search_field |= CALS_SEARCH_FIELD_LOCATION;
	search_field |= CALS_SEARCH_FIELD_ATTENDEE;

	ret = calendar_svc_event_search(search_field, "Hello", &it);
	if (ret < 0)
		return -1;

	while (calendar_svc_iter_next(it) == CAL_SUCCESS) {
		cs = NULL;
		ret = calendar_svc_iter_get_info(it, &cs);
		if (ret != CAL_SUCCESS) {
			printf("calendar_svc_iter_get_info failed (%d)\n", ret);
			return -1;
		}

		id = calendar_svc_struct_get_int(cs, CAL_VALUE_INT_INDEX);
		summary = calendar_svc_struct_get_str(cs, CAL_VALUE_TXT_SUMMARY);
		desc = calendar_svc_struct_get_str(cs, CAL_VALUE_TXT_DESCRIPTION);
		printf("type = %d id = %s desc = %s\n", id, summary, desc);
		calendar_svc_struct_free(&cs);
	}

	calendar_svc_iter_remove(&it);

	calendar_svc_close();

	return 0;
  }
 * @endcode
 * @see detail_management module
 */
int calendar_svc_event_search(int field, const char *keyword, cal_iter **iter);

/**
 * @fn int calendar_svc_smartsearch_excl(const char *keyword, int offset, int limit, cal_iter **iter)
 * Search events by keyword with database offset and limit option.
 * This function is provided for Smartsearch application exclusively.
 *
 * @ingroup event_management
 * @return CAL_SUCCESS or negative error code on failure.
 * @param[in] field fields where the keyword is searched, #SEARCHFIELD
 * @param[in] keyword keyword to be searched
 * @param[in] offset offset to omit some searching results
 * @param[in] limit limit of the number of results. If negative, no limit is applied.
 * @param[out] iter interation struct for list travel
 * @return This function returns CAL_SUCCESS or error code on failure.
 * @exception None.
 * @remarks None.
 * @pre database connected
 * @post none
 * @see detail_management module
 */
int calendar_svc_smartsearch_excl(const char *keyword, int offset, int limit, cal_iter **iter);

/**
 * @fn int calendar_svc_todo_search(int field, const char *keyword, cal_iter **iter);
 * #calendar_svc_event_search searches TO-DOs including the keyword in given fields.
 *
 * @ingroup event_management
 * @return This function returns CAL_SUCCESS or error code on failure.
 * @param[in] field fields where the keyword is searched, #SEARCHFIELD
 * @param[in] keyword keyword to be searched
 * @param[out] iter interation struct for list travel
 * @return This function returns CAL_SUCCESS or error code on failure.
 * @exception None.
 * @remarks None.
 * @pre database connected
 * @post none
 * @code
  #include <calendar-svc-provider.h>
  void sample_code()
  {
	int ret;
	cal_struct *cs;
	cal_iter *it;
	char *summary;
	char *desc;
	int id;

	calendar_svc_connect();
	int search_field;

	search_field = CALS_SEARCH_FIELD_NONE;
	search_field |= CALS_SEARCH_FIELD_SUMMARY;
	search_field |= CALS_SEARCH_FIELD_DESCRIPTION;
	search_field |= CALS_SEARCH_FIELD_LOCATION;
	search_field |= CALS_SEARCH_FIELD_ATTENDEE;

	ret = calendar_svc_todo_search(search_field, "Hello", &it);
	if (ret < 0)
		return -1;

	while (calendar_svc_iter_next(it) == CAL_SUCCESS) {
		cs = NULL;
		ret = calendar_svc_iter_get_info(it, &cs);
		if (ret != CAL_SUCCESS) {
			printf("calendar_svc_iter_get_info failed (%d)\n", ret);
			return -1;
		}

		id = calendar_svc_struct_get_int(cs, CAL_VALUE_INT_INDEX);
		summary = calendar_svc_struct_get_str(cs, CAL_VALUE_TXT_SUMMARY);
		desc = calendar_svc_struct_get_str(cs, CAL_VALUE_TXT_DESCRIPTION);
		printf("type = %d id = %s desc = %s\n", id, summary, desc);
		calendar_svc_struct_free(&cs);
	}

	calendar_svc_iter_remove(&it);

	calendar_svc_close();

	return 0;
  }
 * @endcode
 * @see detail_management module
 */
int calendar_svc_todo_search(int field, const char *keyword, cal_iter **iter);

/**
 * @fn int calendar_svc_read_schedules(const char *stream, GList **schedules);
 * This function reads schedules and provides schedule list.
 *
 * @ingroup event management
 * @param[in] stream vcalendar(ver1.0) icalendar(ver2.0) stream
 * @param[out] schedules schedule list which data is cal_struct
 * @return This function returns CAL_SUCCESS or error code on failure.
 * @remarks none
 * @pre none
 * @post none
 * @code
	#include <stdio.h>
	#include <stdlib.h>
	#include <calendar-svc-provider.h>

	void sample_code()
	{
		int ret, len = 0;
		char *vals;
		char *stream = NULL;
		char buf[1024];
		time_t tt;
		cal_struct *cs = NULL;
		GList *l, *schedules = NULL;
		buf_size = 1024;
		FILE *file;

		file = fopen(path, "r");
		if (file == NULL) {
			printf("failed to open\n");
			return -1;
		}
		stream = malloc(1024);
		while (fgets(buf, sizeof(buf), file)) {
			if (len + sizeof(buf) < buf_size) {
				len += snprintf(stream + len, strlen(buf) +1, "%s", buf);

			} else {
				char *new_stream;
				buf_size *= 2;
				new_stream = realloc(stream, buf_size);
				if (new_stream) {
					stream = new_stream;
				} else {
					free(stream);
					fclose(file);
					printf("out of memory\n");
					return NULL;
				}
				len += snprintf(stream + len, strlen(buf) +1, "%s", buf);
			}
		}
		fclose(file);

		ret = calendar_svc_read_schedules(stream, &schedules);
		if (ret < 0) {
			printf("Failed to read schedules(errno:%d)\n", ret);
			return -1;
		}

		if (schedules == NULL) {
			printf("No schedules\n");
			return -1;
		}

		l = schedules;
		while (l) {
			cs = l->data;
			if (cs == NULL) {
				l = g_list_next(l);
				continue;
			}
			vals = NULL;
			vals = calendar_svc_struct_get_str(cs, CAL_VALUE_TXT_SUMMARY);
			printf("summary:%s\n", vals);

			l = g_list_next(l);
		}
		if (stream) free(stream);
		return 0;
	}
 * @endcode
 * @see calendar_svc_write_schedules().
 */
int calendar_svc_read_schedules(const char *stream, GList **schedules);

/**
 * @fn int calendar_svc_calendar_import(const char *path, int calendar_id);
 * This function import vcalendar(ver 1.0), icalendar(ver 2.0) to calendar DB.
 *
 * @ingroup event management
 * @param[in] path file path
 * @param[out] calendar_id calendar id
 * @return This function returns CAL_SUCCESS or error code on failure.
 * @remarks none
 * @pre none
 * @post none
 * @code
	#include <stdio.h>
	#include <stdlib.h>
	#include <calendar-svc-provider.h>

	int sample_code())
	{
		int r;
		int calendar_id = 1;
		char *path = "/opt/media/test.ics";

    	calendar_svc_connect();
		ret = calendar_svc_calendar_import(path, calendar_id);
		if (ret != CAL_SUCCESS) {
			printf("Failed to import path(%s) to id(%d)\n", path, calendar_id);
			return -1;
		}
    	calendar_svc_close();
		return 0;
	}
 * @endcode
 * @see calendar_svc_calendar_export().
 */
int calendar_svc_calendar_import(const char *path, int calendar_id);

/**
 * @fn int calendar_svc_write_schedules(GList *schedules, char **stream);
 * This function writes schedules to stream.
 *
 * @ingroup event management
 * @param[in] schedules schedule list which data is cal_struct
 * @param[out] stream vcalendar(ver1.0) icalendar(ver2.0) stream
 * @return This function returns CAL_SUCCESS or error code on failure.
 * @remarks none
 * @pre none
 * @post none
 * @code
	#include <stdio.h>
	#include <stdlib.h>
	#include <calendar-svc-provider.h>

	int main(int argc, char **argv)
	{
		int ret;
		char *stream;
		GList *schedules = NULL;
		cal_struct *cs = NULL;

		cs = calendar_svc_struct_new(CAL_STRUCT_CALENDAR);
		if (cs == NULL) {
			printf("Failed to calloc\n");
			return-1;
		}
		calendar_svc_struct_set_str(cs, CAL_VALUE_TXT_SUMMARY, "title");
		// set data in cs...

		schedules = g_list_append(schedules, cs);

		ret = calendar_svc_write_schedules(schedules, &stream);
		if (ret != CAL_SUCCESS) {
			printf("Failed to read schedules(errno:%d)\n", ret);
			return -1;
		}

		if (stream == NULL) {
			printf("stream is NULL\n");
			return -1;
		}

		if (stream) free(stream);
		return 0;
	}
 * @endcode
 * @see calendar_svc_read_schedules().
 */
int calendar_svc_write_schedules(GList *schedules, char **stream);

/**
 * @fn int calendar_svc_calendar_export(int calendar_id, const char *path);
 * This function export calendar DB to file.
 *
 * @ingroup event management
 * @param[in] calendar_id calendar id
 * @param[out] path file path
 * @return This function returns CAL_SUCCESS or error code on failure.
 * @remarks none
 * @pre none
 * @post none
 * @code
	#include <stdio.h>
	#include <stdlib.h>
	#include <calendar-svc-provider.h>

	int sample_code())
	{
		int r;
		int calendar_id = 1;
		char *path = "/opt/media/test.ics";

    	calendar_svc_connect();
		r = calendar_svc_calendar_export(calendar_id, path);
		if (r != CAL_SUCCESS) {
			printf("Failed to export schedules\n");
			return -1;
		}

    	calendar_svc_close();
		return 0;
	}
 * @endcode
 * @see calendar_svc_calendar_import().
 */
int calendar_svc_calendar_export(int calendar_id, const char *path);

#define CALS_VALUE_INT_DTSTART_TYPE "dtstart_type"
#define CALS_VALUE_LLI_DTSTART_UTIME "dtstart_utime"
#define CALS_VALUE_INT_DTSTART_YEAR "dtstart_year"
#define CALS_VALUE_INT_DTSTART_MONTH "dtstart_month"
#define CALS_VALUE_INT_DTSTART_MDAY "dtstart_mday"
#define CALS_VALUE_TXT_DTSTART_TZID "dtstart_tzid"
#define CALS_VALUE_INT_DTEND_TYPE "dtend_type"
#define CALS_VALUE_LLI_DTEND_UTIME "dtend_utime"
#define CALS_VALUE_INT_DTEND_YEAR "dtend_year"
#define CALS_VALUE_INT_DTEND_MONTH "dtend_month"
#define CALS_VALUE_INT_DTEND_MDAY "dtend_mday"
#define CALS_VALUE_TXT_DTEND_TZID "dtend_tzid"
#define CALS_VALUE_LLI_LASTMOD "last_mod"

#define CALS_VALUE_INT_RRULE_FREQ "freq"
#define CALS_VALUE_INT_RRULE_ID "rrule_id"
#define CALS_VALUE_INT_RRULE_RANGE_TYPE "range_type"	//endless, until, count
#define CALS_VALUE_INT_RRULE_UNTIL_TYPE "until_type"	//until by utc, until by local date
#define CALS_VALUE_LLI_RRULE_UNTIL_UTIME "until_utime"  //unix time
#define CALS_VALUE_INT_RRULE_UNTIL_YEAR "until_year"
#define CALS_VALUE_INT_RRULE_UNTIL_MONTH "until_month"
#define CALS_VALUE_INT_RRULE_UNTIL_MDAY "until_mday"
#define CALS_VALUE_INT_RRULE_COUNT "count"
#define CALS_VALUE_INT_RRULE_INTERVAL "interval"
#define CALS_VALUE_TXT_RRULE_BYSECOND "bysecond"
#define CALS_VALUE_TXT_RRULE_BYMINUTE "byminute"
#define CALS_VALUE_TXT_RRULE_BYHOUR "byhour"
#define CALS_VALUE_TXT_RRULE_BYDAY "byday"
#define CALS_VALUE_TXT_RRULE_BYMONTHDAY "bymonthday"
#define CALS_VALUE_TXT_RRULE_BYYEARDAY "byyearday"
#define CALS_VALUE_TXT_RRULE_BYWEEKNO "byweekno"
#define CALS_VALUE_TXT_RRULE_BYMONTH "bymonth"
#define CALS_VALUE_TXT_RRULE_BYSETPOS "bysetpos"
#define CALS_VALUE_INT_RRULE_WKST "wkst"

/* 0x00 ~0x06 used cal_struct_type */
enum {
	CALS_LIST_PERIOD_NORMAL_ONOFF = 0x07,
	CALS_LIST_PERIOD_ALLDAY_ONOFF,
	CALS_LIST_PERIOD_NORMAL_BASIC,
	CALS_LIST_PERIOD_ALLDAY_BASIC,
	CALS_LIST_PERIOD_NORMAL_OSP = 0x100,
	CALS_LIST_PERIOD_ALLDAY_OSP,
	CALS_LIST_PERIOD_NORMAL_LOCATION,
	CALS_LIST_PERIOD_ALLDAY_LOCATION,
	CALS_LIST_PERIOD_NORMAL_ALARM,
	CALS_LIST_PERIOD_ALLDAY_ALARM,
};

#define CALS_LIST_PERIOD_NORMAL_ONOFF_INT_EVENTID CAL_VALUE_INT_INDEX
#define CALS_LIST_PERIOD_NORMAL_ONOFF_INT_DTSTART_TYPE CALS_VALUE_INT_DTSTART_TYPE
#define CALS_LIST_PERIOD_NORMAL_ONOFF_LLI_DTSTART_UTIME CALS_VALUE_LLI_DTSTART_UTIME
#define CALS_LIST_PERIOD_NORMAL_ONOFF_INT_DTEND_TYPE CALS_VALUE_INT_DTEND_TYPE
#define CALS_LIST_PERIOD_NORMAL_ONOFF_LLI_DTEND_UTIME CALS_VALUE_LLI_DTEND_UTIME

#define CALS_LIST_PERIOD_ALLDAY_ONOFF_INT_EVENTID CAL_VALUE_INT_INDEX
#define CALS_LIST_PERIOD_ALLDAY_ONOFF_INT_DTSTART_TYPE CALS_VALUE_INT_DTSTART_TYPE
#define CALS_LIST_PERIOD_ALLDAY_ONOFF_INT_DTSTART_YEAR CALS_VALUE_INT_DTSTART_YEAR
#define CALS_LIST_PERIOD_ALLDAY_ONOFF_INT_DTSTART_MONTH CALS_VALUE_INT_DTSTART_MONTH
#define CALS_LIST_PERIOD_ALLDAY_ONOFF_INT_DTSTART_MDAY CALS_VALUE_INT_DTSTART_MDAY
#define CALS_LIST_PERIOD_ALLDAY_ONOFF_INT_DTEND_TYPE CALS_VALUE_INT_DTEND_TYPE
#define CALS_LIST_PERIOD_ALLDAY_ONOFF_INT_DTEND_YEAR CALS_VALUE_INT_DTEND_YEAR
#define CALS_LIST_PERIOD_ALLDAY_ONOFF_INT_DTEND_MONTH CALS_VALUE_INT_DTEND_MONTH
#define CALS_LIST_PERIOD_ALLDAY_ONOFF_INT_DTEND_MDAY CALS_VALUE_INT_DTEND_MDAY

#define CALS_LIST_PERIOD_NORMAL_BASIC_INT_EVENTID CAL_VALUE_INT_INDEX
#define CALS_LIST_PERIOD_NORMAL_BASIC_INT_DTSTART_TYPE CALS_VALUE_INT_DTSTART_TYPE
#define CALS_LIST_PERIOD_NORMAL_BASIC_LLI_DTSTART_UTIME CALS_VALUE_LLI_DTSTART_UTIME
#define CALS_LIST_PERIOD_NORMAL_BASIC_INT_DTEND_TYPE CALS_VALUE_INT_DTEND_TYPE
#define CALS_LIST_PERIOD_NORMAL_BASIC_LLI_DTEND_UTIME CALS_VALUE_LLI_DTEND_UTIME
#define CALS_LIST_PERIOD_NORMAL_BASIC_TXT_SUMMARY CAL_VALUE_TXT_SUMMARY
#define CALS_LIST_PERIOD_NORMAL_BASIC_TXT_LOCATION CAL_VALUE_TXT_LOCATION

#define CALS_LIST_PERIOD_ALLDAY_BASIC_INT_EVENTID CAL_VALUE_INT_INDEX
#define CALS_LIST_PERIOD_ALLDAY_BASIC_INT_DTSTART_TYPE CALS_VALUE_INT_DTSTART_TYPE
#define CALS_LIST_PERIOD_ALLDAY_BASIC_INT_DTSTART_YEAR CALS_VALUE_INT_DTSTART_YEAR
#define CALS_LIST_PERIOD_ALLDAY_BASIC_INT_DTSTART_MONTH CALS_VALUE_INT_DTSTART_MONTH
#define CALS_LIST_PERIOD_ALLDAY_BASIC_INT_DTSTART_MDAY CALS_VALUE_INT_DTSTART_MDAY
#define CALS_LIST_PERIOD_ALLDAY_BASIC_INT_DTEND_TYPE CALS_VALUE_INT_DTEND_TYPE
#define CALS_LIST_PERIOD_ALLDAY_BASIC_INT_DTEND_YEAR CALS_VALUE_INT_DTEND_YEAR
#define CALS_LIST_PERIOD_ALLDAY_BASIC_INT_DTEND_MONTH CALS_VALUE_INT_DTEND_MONTH
#define CALS_LIST_PERIOD_ALLDAY_BASIC_INT_DTEND_MDAY CALS_VALUE_INT_DTEND_MDAY
#define CALS_LIST_PERIOD_ALLDAY_BASIC_TXT_SUMMARY CAL_VALUE_TXT_SUMMARY
#define CALS_LIST_PERIOD_ALLDAY_BASIC_TXT_LOCATION CAL_VALUE_TXT_LOCATION

#define CALS_LIST_PERIOD_NORMAL_OSP_INT_EVENTID CAL_VALUE_INT_INDEX
#define CALS_LIST_PERIOD_NORMAL_OSP_INT_DTSTART_TYPE CALS_VALUE_INT_DTSTART_TYPE
#define CALS_LIST_PERIOD_NORMAL_OSP_LLI_DTSTART_UTIME CALS_VALUE_LLI_DTSTART_UTIME
#define CALS_LIST_PERIOD_NORMAL_OSP_INT_DTEND_TYPE CALS_VALUE_INT_DTEND_TYPE
#define CALS_LIST_PERIOD_NORMAL_OSP_LLI_DTEND_UTIME CALS_VALUE_LLI_DTEND_UTIME
#define CALS_LIST_PERIOD_NORMAL_OSP_TXT_SUMMARY CAL_VALUE_TXT_SUMMARY
#define CALS_LIST_PERIOD_NORMAL_OSP_TXT_LOCATION CAL_VALUE_TXT_LOCATION
#define CALS_LIST_PERIOD_NORMAL_OSP_INT_CALENDAR_ID CAL_VALUE_INT_CALENDAR_ID
#define CALS_LIST_PERIOD_NORMAL_OSP_TXT_DESCRIPTION CAL_VALUE_TXT_DESCRIPTION
#define CALS_LIST_PERIOD_NORMAL_OSP_INT_BUSY_STATUS CAL_VALUE_INT_BUSY_STATUS
#define CALS_LIST_PERIOD_NORMAL_OSP_INT_STATUS CAL_VALUE_INT_MEETING_STATUS
#define CALS_LIST_PERIOD_NORMAL_OSP_INT_PRIORITY CAL_VALUE_INT_PRIORITY
#define CALS_LIST_PERIOD_NORMAL_OSP_INT_VISIBILITY CAL_VALUE_INT_SENSITIVITY
#define CALS_LIST_PERIOD_NORMAL_OSP_INT_IS_RECURRING CALS_VALUE_INT_RRULE_ID

#define CALS_LIST_PERIOD_ALLDAY_OSP_INT_EVENTID CAL_VALUE_INT_INDEX
#define CALS_LIST_PERIOD_ALLDAY_OSP_INT_DTSTART_TYPE CALS_VALUE_INT_DTSTART_TYPE
#define CALS_LIST_PERIOD_ALLDAY_OSP_INT_DTSTART_YEAR CALS_VALUE_INT_DTSTART_YEAR
#define CALS_LIST_PERIOD_ALLDAY_OSP_INT_DTSTART_MONTH CALS_VALUE_INT_DTSTART_MONTH
#define CALS_LIST_PERIOD_ALLDAY_OSP_INT_DTSTART_MDAY CALS_VALUE_INT_DTSTART_MDAY
#define CALS_LIST_PERIOD_ALLDAY_OSP_INT_DTEND_TYPE CALS_VALUE_INT_DTEND_TYPE
#define CALS_LIST_PERIOD_ALLDAY_OSP_INT_DTEND_YEAR CALS_VALUE_INT_DTEND_YEAR
#define CALS_LIST_PERIOD_ALLDAY_OSP_INT_DTEND_MONTH CALS_VALUE_INT_DTEND_MONTH
#define CALS_LIST_PERIOD_ALLDAY_OSP_INT_DTEND_MDAY CALS_VALUE_INT_DTEND_MDAY
#define CALS_LIST_PERIOD_ALLDAY_OSP_TXT_SUMMARY CAL_VALUE_TXT_SUMMARY
#define CALS_LIST_PERIOD_ALLDAY_OSP_TXT_LOCATION CAL_VALUE_TXT_LOCATION
#define CALS_LIST_PERIOD_ALLDAY_OSP_INT_CALENDAR_ID CAL_VALUE_INT_CALENDAR_ID
#define CALS_LIST_PERIOD_ALLDAY_OSP_TXT_DESCRIPTION CAL_VALUE_TXT_DESCRIPTION
#define CALS_LIST_PERIOD_ALLDAY_OSP_INT_BUSY_STATUS CAL_VALUE_INT_BUSY_STATUS
#define CALS_LIST_PERIOD_ALLDAY_OSP_INT_STATUS CAL_VALUE_INT_MEETING_STATUS
#define CALS_LIST_PERIOD_ALLDAY_OSP_INT_PRIORITY CAL_VALUE_INT_PRIORITY
#define CALS_LIST_PERIOD_ALLDAY_OSP_INT_VISIBILITY CAL_VALUE_INT_SENSITIVITY
#define CALS_LIST_PERIOD_ALLDAY_OSP_INT_IS_RECURRING CALS_VALUE_INT_RRULE_ID

#define CALS_LIST_PERIOD_NORMAL_LOCATION_INT_EVENTID CAL_VALUE_INT_INDEX
#define CALS_LIST_PERIOD_NORMAL_LOCATION_INT_DTSTART_TYPE CALS_VALUE_INT_DTSTART_TYPE
#define CALS_LIST_PERIOD_NORMAL_LOCATION_LLI_DTSTART_UTIME CALS_VALUE_LLI_DTSTART_UTIME
#define CALS_LIST_PERIOD_NORMAL_LOCATION_INT_DTEND_TYPE CALS_VALUE_INT_DTEND_TYPE
#define CALS_LIST_PERIOD_NORMAL_LOCATION_LLI_DTEND_UTIME CALS_VALUE_LLI_DTEND_UTIME
#define CALS_LIST_PERIOD_NORMAL_LOCATION_TXT_SUMMARY CAL_VALUE_TXT_SUMMARY
#define CALS_LIST_PERIOD_NORMAL_LOCATION_TXT_LOCATION CAL_VALUE_TXT_LOCATION
#define CALS_LIST_PERIOD_NORMAL_LOCATION_INT_CALENDAR_ID CAL_VALUE_INT_CALENDAR_ID
#define CALS_LIST_PERIOD_NORMAL_LOCATION_TXT_DESCRIPTION CAL_VALUE_TXT_DESCRIPTION
#define CALS_LIST_PERIOD_NORMAL_LOCATION_INT_BUSY_STATUS CAL_VALUE_INT_BUSY_STATUS
#define CALS_LIST_PERIOD_NORMAL_LOCATION_INT_STATUS CAL_VALUE_INT_MEETING_STATUS
#define CALS_LIST_PERIOD_NORMAL_LOCATION_INT_PRIORITY CAL_VALUE_INT_PRIORITY
#define CALS_LIST_PERIOD_NORMAL_LOCATION_INT_VISIBILITY CAL_VALUE_INT_SENSITIVITY
#define CALS_LIST_PERIOD_NORMAL_LOCATION_INT_IS_RECURRING CALS_VALUE_INT_RRULE_ID
#define CALS_LIST_PERIOD_NORMAL_LOCATION_DBL_LATITUDE CAL_VALUE_DBL_LATITUDE
#define CALS_LIST_PERIOD_NORMAL_LOCATION_DBL_LONGITUDE CAL_VALUE_DBL_LONGITUDE

#define CALS_LIST_PERIOD_ALLDAY_LOCATION_INT_EVENTID CAL_VALUE_INT_INDEX
#define CALS_LIST_PERIOD_ALLDAY_LOCATION_INT_DTSTART_TYPE CALS_VALUE_INT_DTSTART_TYPE
#define CALS_LIST_PERIOD_ALLDAY_LOCATION_INT_DTSTART_YEAR CALS_VALUE_INT_DTSTART_YEAR
#define CALS_LIST_PERIOD_ALLDAY_LOCATION_INT_DTSTART_MONTH CALS_VALUE_INT_DTSTART_MONTH
#define CALS_LIST_PERIOD_ALLDAY_LOCATION_INT_DTSTART_MDAY CALS_VALUE_INT_DTSTART_MDAY
#define CALS_LIST_PERIOD_ALLDAY_LOCATION_INT_DTEND_TYPE CALS_VALUE_INT_DTEND_TYPE
#define CALS_LIST_PERIOD_ALLDAY_LOCATION_INT_DTEND_YEAR CALS_VALUE_INT_DTEND_YEAR
#define CALS_LIST_PERIOD_ALLDAY_LOCATION_INT_DTEND_MONTH CALS_VALUE_INT_DTEND_MONTH
#define CALS_LIST_PERIOD_ALLDAY_LOCATION_INT_DTEND_MDAY CALS_VALUE_INT_DTEND_MDAY
#define CALS_LIST_PERIOD_ALLDAY_LOCATION_TXT_SUMMARY CAL_VALUE_TXT_SUMMARY
#define CALS_LIST_PERIOD_ALLDAY_LOCATION_TXT_LOCATION CAL_VALUE_TXT_LOCATION
#define CALS_LIST_PERIOD_ALLDAY_LOCATION_INT_CALENDAR_ID CAL_VALUE_INT_CALENDAR_ID
#define CALS_LIST_PERIOD_ALLDAY_LOCATION_TXT_DESCRIPTION CAL_VALUE_TXT_DESCRIPTION
#define CALS_LIST_PERIOD_ALLDAY_LOCATION_INT_BUSY_STATUS CAL_VALUE_INT_BUSY_STATUS
#define CALS_LIST_PERIOD_ALLDAY_LOCATION_INT_STATUS CAL_VALUE_INT_MEETING_STATUS
#define CALS_LIST_PERIOD_ALLDAY_LOCATION_INT_PRIORITY CAL_VALUE_INT_PRIORITY
#define CALS_LIST_PERIOD_ALLDAY_LOCATION_INT_VISIBILITY CAL_VALUE_INT_SENSITIVITY
#define CALS_LIST_PERIOD_ALLDAY_LOCATION_INT_IS_RECURRING CALS_VALUE_INT_RRULE_ID
#define CALS_LIST_PERIOD_ALLDAY_LOCATION_DBL_LATITUDE CAL_VALUE_DBL_LATITUDE
#define CALS_LIST_PERIOD_ALLDAY_LOCATION_DBL_LONGITUDE CAL_VALUE_DBL_LONGITUDE

#define CALS_LIST_PERIOD_NORMAL_ALARM_INT_EVENTID CAL_VALUE_INT_INDEX
#define CALS_LIST_PERIOD_NORMAL_ALARM_INT_CALENDAR_ID CAL_VALUE_INT_CALENDAR_ID
#define CALS_LIST_PERIOD_NORMAL_ALARM_INT_DTSTART_TYPE CALS_VALUE_INT_DTSTART_TYPE
#define CALS_LIST_PERIOD_NORMAL_ALARM_LLI_DTSTART_UTIME CALS_VALUE_LLI_DTSTART_UTIME
#define CALS_LIST_PERIOD_NORMAL_ALARM_INT_DTEND_TYPE CALS_VALUE_INT_DTEND_TYPE
#define CALS_LIST_PERIOD_NORMAL_ALARM_LLI_DTEND_UTIME CALS_VALUE_LLI_DTEND_UTIME
#define CALS_LIST_PERIOD_NORMAL_ALARM_LLI_ALARM_UTIME CAL_VALUE_LLI_ALARMS_TIME
#define CALS_LIST_PERIOD_NORMAL_ALARM_INT_ALARM_ID CAL_VALUE_INT_ALARMS_ID

enum cals_day {
	CALS_SUNDAY,
	CALS_MONDAY,
	CALS_TUESDAY,
	CALS_WEDNESDAY,
	CALS_THURSDAY,
	CALS_FRIDAY,
	CALS_SATURDAY,
	CALS_NODAY,
};

enum cals_freq {
	CALS_FREQ_ONCE = 0x0,
	CALS_FREQ_YEARLY,
	CALS_FREQ_MONTHLY,
	CALS_FREQ_WEEKLY,
	CALS_FREQ_DAILY,
	CALS_FREQ_HOURLY,
	CALS_FREQ_MINUTELY,
	CALS_FREQ_SECONDLY,
};

enum cals_time_type {
	CALS_TIME_UTIME,
	CALS_TIME_LOCALTIME,
};

enum cals_range {
	CALS_RANGE_UNTIL,
	CALS_RANGE_COUNT,
	CALS_RANGE_NONE,
};

int calendar_svc_event_get_normal_list_by_period(int calendar_id, int op_code,
		long long int start, long long int end, cal_iter **iter);

int calendar_svc_event_get_allday_list_by_period(int calendar_id, int op_code,
		int dtstart_year, int dtstart_mon, int dtstart_day,
		int dtend_year, int dtend_mon, int dtend_day, cal_iter **iter);

int calendar_svc_struct_set_lli(cal_struct *record, const char *field, long long int llival);

long long int calendar_svc_struct_get_lli(cal_struct *record, const char *field);

int calendar_svc_todo_get_list_by_period(int calendar_id,
		long long int due_from, long long int dueto, int priority, int status, cal_iter **iter);
int calendar_svc_todo_get_count_by_period(int calendar_id,
		long long int due_from, long long int dueto, int priority, int status, int *count);
int calendar_svc_event_delete_normal_instance(int event_id, long long int dtstart_utime);
int calendar_svc_event_delete_allday_instance(int event_id, int dtstart_year, int dtstart_month, int dtstart_mday);
#ifdef __cplusplus
}
#endif

/**
 * deprecated
 */
typedef enum
{
	CAL_STATUS_FREE = 0,		   /**< deprecated */
	CAL_STATUS_TENTATIVE,		  /**< deprecated */
	CAL_STATUS_BUSY,		     /**< deprecated */
	CAL_STATUS_OUROFOFFICE,		  /**< deprecated */
	CAL_STATUS_CONFIRM,		  /**< deprecated */
	CAL_STATUS_DENIED,		  /**< deprecated */
} cal_status_type_t;

#endif /* __CALENDAR_SVC_H__ */

