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
 * This enumeration defines schedule category.
 */
typedef enum
{
	CAL_SCH_NONE=0,				/**< None type */
	CAL_SCH_APPOINTMENT,		   /**< appointment category */
	CAL_SCH_IMPORTANT,			/**< important category */
	CAL_SCH_SPECIAL_OCCASION,	/**< anniversary category */
	CAL_SCH_BIRTHDAY,			   /**< birthday category */
	CAL_SCH_HOLIDAY,			   /**< holiday category */
	CAL_SCH_PRIVATE,			/**< private category */
	CAL_SCH_BUSSINESS,			/**< bussiness category */
} cal_sch_category_t;

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
typedef enum
{
	CAL_SCH_TIME_UNIT_OFF = -1, /**< off */
	CAL_SCH_TIME_UNIT_MIN = 0,	/**< Minute */
	CAL_SCH_TIME_UNIT_HOUR,		/**< Hour */
	CAL_SCH_TIME_UNIT_DAY,		/**< Day */
	CAL_SCH_TIME_UNIT_WEEK,		/**< Week */
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
	EVENT_PRIORITY_LOW =0,
	EVENT_PRIORITY_NORMAL,
	EVENT_PRIORITY_HIGH,
} cal_priority_type_t;

/**
 * This enumeration defines status.
 * (related with CAL_VALUE_INT_TASK_STATUS)
 */
typedef enum
{
	CALS_STATUS_NONE =0,
	CALS_EVENT_STATUS_TENTATIVE,
	CALS_EVENT_STATUS_CONFIRMED,
	CALS_EVENT_STATUS_CANCELLED,
	CALS_TODO_STATUS_NEEDS_ACTION,
	CALS_TODO_STATUS_COMPLETED,
	CALS_TODO_STATUS_IN_PROCESS,
	CALS_TODO_STATUS_CANCELLED,
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
 * @}
 */

/**
 * @addtogroup common
 * @{
 * brief
 * 		calendar_svc_struct_new's argument
 */
#define CAL_STRUCT_TYPE					  /**< CAL_STRUCT_TYPE */
#define CAL_STRUCT_CALENDAR "calendar" 		/**< CAL_STRUCT_CALENDAR */
#define CAL_STRUCT_SCHEDULE "schedule" 		/**< CAL_STRUCT_SCHEDULE */
#define CAL_STRUCT_TODO		"todo" 		  	/**< CAL_STRUCT_TASK */
#define CAL_STRUCT_TIMEZONE	"timezone"		/**< CAL_STRUCT_TIMEZONE */

// id for all data read
#define ALL_ACCOUNT_ID 0
#define ALL_CALENDAR_ID 0

// id for all data without visibility false
#define ALL_VISIBILITY_ACCOUNT -2

// id for local data read
#define LOCAL_ACCOUNT_ID -1
#define LOCAL_ALL_CALENDAR -1

#define DEFAULT_CALENDAR_ID 1

/**
 * @}
 */

/**
 * @addtogroup common
 * @{
 * brief
 *		calendar_svc_value_xxx()'s argument
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
 * 		calendar_svc_value_xxx()'s argument
 */
#define CAL_VALUE_INT_INDEX 				  "id"					/**< Record index */
#define CAL_VALUE_INT_ACCOUNT_ID			  "account_id"			/**< account id */
#define CAL_VALUE_INT_TYPE 				  "type"				/**< Calendar component type */
#define CAL_VALUE_INT_CATEGORY 			  "category"			/**< Category of schedule #cal_sch_category_t */
#define CAL_VALUE_TXT_SUMMARY 				  "summary"				/**< Summary, appointment, task: subject, birthday:Name */
#define CAL_VALUE_TXT_DESCRIPTION	 		  "description"			/**< Description,appointment, task: description, anniversary,holiday:occasion*/
#define CAL_VALUE_TXT_LOCATION 			 	  "location"				/**< Location */
#define CAL_VALUE_INT_ALL_DAY_EVENT		 	  "all_day_event"		/**< All day event flag */
#define CAL_VALUE_GMT_START_DATE_TIME	 	  "start_date_time"		/**< schedule:start time, anniv,holiday,birthday,memo,todo: date */
#define CAL_VALUE_GMT_END_DATE_TIME		 	  "end_date_time"		/**< end time */
#define CAL_VALUE_INT_REPEAT_TERM		 	  "repeat_item"			/**< Repeat term */
#define CAL_VALUE_INT_REPEAT_INTERVAL	 	  "repeat_interval"		/**< Interval of repeat term */
#define CAL_VALUE_INT_REPEAT_OCCURRENCES 	  "repeat_occurrences"	  /**< occurrences of repeat */
#define CAL_VALUE_GMT_REPEAT_END_DATE	 	  "repeat_end_date"		/**< End date for repeat */
#define CAL_VALUE_INT_SUN_MOON			 	  "sun_moon"				/**< Using sun or lunar calendar */
#define CAL_VALUE_INT_WEEK_START		 	  "week_start"			/**< Start day of a week */
#define CAL_VALUE_TXT_WEEK_FLAG			 	  "week_flag" 			/**< 1001000(sun,wed) Indicate which day is select in a week */
#define CAL_VALUE_INT_DAY_DATE			 	  "day_date"				/**< 0- for weekday(sun,mon,etc.), 1- for specific day(1,2.. Etc) */
#define CAL_VALUE_GMT_LAST_MODIFIED_TIME 	  "last_modified_time"	/**< for PC Sync */
#define CAL_VALUE_INT_MISSED			 	  "missed"				  /**< Miss alarm flag */
#define CAL_VALUE_INT_TASK_STATUS		 	  "task_status"			/**< current task status #cals_status_t */
#define CAL_VALUE_INT_PRIORITY			 	  "priority"				/**< Priority */
#define CAL_VALUE_INT_TIMEZONE			 	  "timezone"				/**< deprecated - timezone of task */
#define CAL_VALUE_INT_FILE_ID 			 	  "file_id"				/**< file id for attach or alarm tone*/
#define CAL_VALUE_INT_CONTACT_ID		 	  "contact_id"			/**< contact id for birthday in contact list */
#define CAL_VALUE_INT_BUSY_STATUS		 	  "busy_status"			/**< ACS, G : Flag of busy or not */
#define CAL_VALUE_INT_SENSITIVITY		 	  "sensitivity"			/**< ACS, G : The sensitivity of the task item (normal, presonal, private, confidential). */
#define CAL_VALUE_TXT_UID				 	  "uid"					  /**< ACS, G : Unique ID of the meeting item */
#define CAL_VALUE_INT_CALENDAR_TYPE		 	  "calendar_type"		/**< ACS, G : Type(all,phone,google) of calendar */
#define CAL_VALUE_TXT_ORGANIZER_NAME	 	  "organizer_name"		/**< ACS, G : Name of organizer(author) */
#define CAL_VALUE_TXT_ORGANIZER_EMAIL	 	  "organizer_email"		/**< ACS, G : Email of organizer */
#define CAL_VALUE_INT_MEETING_STATUS	 	  "meeting_status"		/**< ACS, G : The status of the meeting. */
#define CAL_VALUE_TXT_GCAL_ID			 	  "gcal_id"				/**< G : Server id of calendar */
#define CAL_VALUE_INT_DELETED			 	  "deleted"				/**< G : Flag for deleted */
#define CAL_VALUE_TXT_UPDATED			 	  "updated"				/**< G : Updated time stamp */
#define CAL_VALUE_INT_LOCATION_TYPE		 	  "location_type"		/**< G : Location type */
#define CAL_VALUE_TXT_LOCATION_SUMMARY	 	  "location_summary"	/**< G : A simple string value that can be used as a representation of this location */
#define CAL_VALUE_TXT_ETAG				 	  "etag"					/**< G : ETAG of this event */
#define CAL_VALUE_INT_CALENDAR_ID 		 	  "calendar_id"			/**< G : id to map from calendar table */
#define CAL_VALUE_INT_SYNC_STATUS		 	  "sync_status"			/**< G : Indication for event entry whether added/ modified/ deleted */
#define CAL_VALUE_TXT_EDIT_URL			 	  "edit_uri"     	/**< G : EditUri for google calendar */
#define CAL_VALUE_TXT_GEDERID		 	 	  "gevent_id"				/**< G : Server id of an event */
#define CAL_VALUE_INT_DST 				 	  "dst"					  /**< dst of event */
#define CAL_VALUE_INT_ORIGINAL_EVENT_ID	 	  "original_event_id" /**< original event id for recurrency exception */
#define CAL_VALUE_INT_CALENDAR_INDEX      "calendar_index"   /**< specific calendar id - will be remove */
#define CAL_VALUE_DBL_LATITUDE         "latitude"      /**< latitude */
#define CAL_VALUE_DBL_LONGITUDE        "longitude"     /**< longitude */
#define CAL_VALUE_INT_IS_DELETED        "is_deleted"     /**< readonly */
#define CAL_VALUE_TXT_TZ_NAME	        "tz_name"      /**< tz file name */
#define CAL_VALUE_TXT_TZ_CITY_NAME	      "tz_city_name"    /**< tz city name */
#define CAL_VALUE_INT_EMAIL_ID				  "email_id"			/**< email id */
#define CAL_VALUE_INT_AVAILABILITY			  "availability"
#define CAL_VALUE_GMT_CREATED_DATE_TIME "created_date_time"
#define CAL_VALUE_GMT_COMPLETED_DATE_TIME "completed_date_time"
#define CAL_VALUE_INT_PROGRESS "progress"

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
#define CAL_VALUE_TXT_ATTENDEE_DETAIL_NAME		 	"attendee_name"			/**< attendee_name */
#define CAL_VALUE_TXT_ATTENDEE_DETAIL_EMAIL		  "attendee_email"			/**< attendee_email */
#define CAL_VALUE_TXT_ATTENDEE_DETAIL_NUMBER		"attendee_number"			/**< attendee_email */
#define CAL_VALUE_INT_ATTENDEE_DETAIL_STATUS	 	"attendee_status"			/**< #cal_event_attendee_status_type_t */
#define CAL_VALUE_INT_ATTENDEE_DETAIL_TYPE		 	"attendee_type"			/**< #cal_event_attendee_type_t */
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

/**
 * @addtogroup common
 * @{
 * brief
 * 		meeting category cal_value's detail field
 */
#define CAL_VALUE_LST_MEETING_CATEGORY        "meeting_category"  /**< attendee's detail information set */
#define CAL_VALUE_INT_MEETING_CATEGORY_DETAIL_ID   "event_id"			     /**< attendee_name */
#define CAL_VALUE_TXT_MEETING_CATEGORY_DETAIL_NAME  "category_name"			  /**< attendee_email */

/**
 * @}
 */


/**
 * @addtogroup common
 * @{
 * brief
 * 		exception event date
 */
#define CAL_VALUE_LST_EXCEPTION_DATE         "exception_date"    /**< exception's detail information set */
#define CAL_VALUE_GMT_EXCEPTION_DATE_TIME       "exception_date_time"	/**< exception event's start date */
#define CAL_VALUE_INT_EXCEPTION_DATE_ID        "exception_event_id"	/**< if occasion update case, it has valid id(not -1) */

/**
 * @}
 */



/**
 * @addtogroup common
 * @{
 * brief
 * 		exception event date
 */
#define CAL_VALUE_LST_ALARM			        "alarm"    /**< exception's detail information set */
#define CAL_VALUE_GMT_ALARMS_TIME		 	  		"alarm_time"			/**< alarm time */
#define CAL_VALUE_INT_ALARMS_TICK		 	  		"remind_tick"			/**< Alarms before remindTick */
#define CAL_VALUE_INT_ALARMS_TICK_UNIT	 	  		"remind_tick_unit"	/**< Remind tick unit */
#define CAL_VALUE_TXT_ALARMS_TONE		 	  		"alarm_tone"			/**< Alert Sound File Name */
#define CAL_VALUE_TXT_ALARMS_DESCRIPTION "alarm_description"			/**< Alert description */
#define CAL_VALUE_INT_ALARMS_TYPE		 	  		"alarm_type"			/**< Alert type(see 'cal_alert_type_t') */
#define CAL_VALUE_INT_ALARMS_ID			 	  		"alarm_id"				/**< Alarm id */

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
 *   api param
 */
#define CAL_VALUE_CUSTOM                     /**< custom field value(MIME Type will be Support) */
#define CAL_VALUE_ALL_FIELD         "all_field_list"   /**< event's all data field return */
#define CAL_VALUE_MAIN_FILED         "main_field_list"  /**< event's major data field return(summay,description,status,etc..) */
#define CAL_VALUE_LIST_FILED         "list_field_list"  /**< event's sub data field for list view(summary,start/end date/all day,repeat) */
#define CAL_VALUE_MONTH_FILED         "month_field_list" /**< event's sub data field for month view check */
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
 * @fn int calendar_svc_end_trans(void);
 * This function finish db transaction,it is coninient for user do many operaion once.
 *
 * @ingroup service_management
 * @return This function returns CAL_SUCCESS or error code on failure.
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
		calendar_svc_end_trans();

    	//close database
    	calendar_svc_close();

   }
 * @endcode
 * @see calendar_svc_begin_trans().
 */
int calendar_svc_end_trans(void);


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
 * @fn int calendar_svc_event_delete_by_period(int account_id,time_t start_time,time_t end_time);
 * This function delete records from database,it is convenient for user to delete records set once.
 *
 * @ingroup event_management
 * @return This function returns CAL_SUCCESS or error code on failure.
 * @param[in] account_id account db index
 * @param[in] start_time timestamp
 * @param[in] end_time  timestamp
 * @return This function returns CAL_SUCCESS or error code on failure.
 * @exception None.
 * @remarks None.
 * @pre database connected
 * @post none
 * @code
   #include <calendar-svc-provider.h>
   void sample_code()
   {
	  time_t start_time = time(NULL);
	  time_t end_time = start_time + 10000;

   	  //connect to database
   	  calendar_svc_connect();

	  //delete the records whose lase modified time is between start_time and end_time
   	  calendar_svc_event_delete_by_period(0,start_time,end_time);

	  //close database
   	  calendar_svc_close();
   }
 * @endcode
 * @see detail_management module
 * @see
 */
int calendar_svc_event_delete_by_period(int account_id,time_t start_time,time_t end_time);

/**
 * @fn int calendar_svc_delete_all(int account_id,const char *data_type);
 * This function delete all records from database,it is convenient for user to delete all of records.
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
 * @fn int calendar_svc_clean_after_sync(int account_id);
 * This function clean deleted(marked) all records from database,which is used to remove data from database after sync operation.
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

	  //delete the all records from schudule_table
   	  calendar_svc_clean_after_sync(0);

	  //close database
   	  calendar_svc_close();
   }
 * @endcode
 * @see detail_management module
 */

int calendar_svc_clean_after_sync(int account_id);

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
 * @fn int calendar_svc_get_list(int account_id,int calendar_id,const char *data_type,const char *field_type,int offset,int count, cal_iter **iter)
 * This function get all records from database,but, this api support data filter for performance.
 *
 * @ingroup event_management
 * @return This function returns CAL_SUCCESS or error code on failure.
 * @param[in] account_id account db index
 * @param[in] calendar_id calendar id(will be support phase 2)
 * @param[in] data_type data_type(CAL_STRUCT_CALENDAR or CAL_STRUCT_SCHEDULE)
 * @param[in] sub field type(CAL_VALUE_ALL_FIELD or CAL_VALUE_MAIN_FILED,CAL_VALUE_LIST_FILED,CAL_VALUE_MONTH_FILED)
 * @param[in] offset start item list index
 * @param[in] count return data count(limit count)
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
   	  calendar_svc_get_list(ALL_VISIBILITY_ACCOUNT,ALL_CALENDAR_ID,CAL_STRUCT_SCHEDULE,CAL_VALUE_LIST_FILED,0,10, &iter);

	  //free
   	  calendar_svc_iter_remove(&iter);

	  //close database
   	  calendar_svc_close();
   }
 * @endcode
 * @see detail_management module
 */
int calendar_svc_get_list(int account_id,int calendar_id,const char *data_type,const char *field_type,int offset,int count, cal_iter **iter);


/**
 * @fn int calendar_svc_get_updated_event_list(int account_id,time_t timestamp, cal_iter **iter);
 * This function get update records from database by time statmp,it is convenient for user to decide which records need to sync.
 *
 * @ingroup event_management
 * @return This function returns CAL_SUCCESS or error code on failure.
 * @param[in] account_id account db index
 * @param[in] timestamp updated timestamp
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
   	  time timestamp = time(NULL) - 10000;

	  //connect to database
   	  calendar_svc_connect();

	  //get events updated after timestamp
   	  calendar_svc_get_updated_event_list(0,timestamp, &iter);

   	  //free
   	  calendar_svc_iter_remove(&iter);

	  //close database
   	  calendar_svc_close();
   }
 * @endcode
 * @see detail_management module
 */
int calendar_svc_get_updated_event_list(int account_id,time_t timestamp, cal_iter **iter);


/**
 * @fn int calendar_svc_get_event_list_by_period(int account_id,time_t start_time,time_t end_time,cal_iter **iter);
 * This function get update records from database by time statmp,it is convenient for user to get records according to time.
 *
 * @ingroup event_management
 * @return This function returns CAL_SUCCESS or error code on failure.
 * @param[in] account_id account db index(0 for all event
 * @param[in] start_time timestamp
 * @param[in] end_time  timestamp
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
   	  time start_time = time(NULL);
   	  time end_time = start_time + 10000;

	  //connect to database
   	  calendar_svc_connect();

	  //get events
   	  calendar_svc_get_event_list_by_period(0,start_time,end_time,&iter);

   	  //free
   	  calendar_svc_iter_remove(&iter);

	  //close database
   	  calendar_svc_close();
   }
 * @endcode
 * @see detail_management module
 */
int calendar_svc_get_event_list_by_period(int account_id,
										time_t start_time,
										time_t end_time,
										cal_iter **iter);


int calendar_svc_get_event_list_by_tm_period (int account_id,
                          int calendar_id,
                        struct tm* startdate,
                        struct tm* enddate,
                        cal_iter **iter);


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
 * @fn int calendar_svc_util_next_valid_event_tm ( cal_struct * event,struct tm* start_tm,struct tm* end_tm,struct tm* next_valid_start_tm,struct tm* next_valid_end_tm );
 * This function gets next valid event(it should be recurrence event) by period,user can get next valid event't time through calling it.
 *
 * @ingroup event_management
 * @param[in] event point of event struct
 * @param[in] start_time start point of valid time period
 * @param[in] end_time end point of valid time period
 * @param[out] next_valid_start_time next valid start time in period
 * @param[out] next_valid_end_time next valid end time in period
 * @return This function returns CAL_SUCCESS or error code on failure.
 * @remarks none
 * @pre the event must be recurrence event
 * @post none
 * @code
   #include <calendar_svc_provider.h>
   #include <time.h>
   void sample_code()
   {
   		cal_struct* event = NULL;
   		index = 1;
   		time_t start_time = time(NULL);
   		time_t end_time = start_time + 1000000;
   		time_t next_valid_start_time = 0;
   		time_t next_valid_end_time = 0;
   		struct tm stm,etm;
   		struct tm estm,eetm;

		localtime_r(&start_time,&stm);
		localtime_r(&end_time,&etm);

   		//connect to database
   		calendar_svc_connect();

		//get the record
		calendar_svc_get("schedule",NULL,index,&event);

		//get the next valid event time
		memset(&estm,0x00,sizeof(struct tm));
		memset(&eetm,0x00,sizeof(struct tm));
		while(calendar_svc_util_next_valid_event_tm(event,&stm,&etm,&estm,&eetm)==CAL_SUCCESS)
		{
			//using estm,eetm
		}

		//free the space
		calendar_svc_struct_free(&event);

   		//close database
   		calendar_svc_close();

   }
 * @endcode
 * @see none.
 */
int calendar_svc_util_next_valid_event_tm ( cal_struct * event,
 struct tm* start_tm,
 struct tm* end_tm,
 struct tm* next_valid_start_tm,
 struct tm* next_valid_end_tm );


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
 * @fn struct tm* calendar_svc_struct_get_tm(cal_struct* record, const char* field, int timezone_flag);
 * This function gets time value of the calendar service value,it is convenient for user get the value needed without knowing the detail of the struct.
 *
 * @ingroup detail_management
 * @param[in] record Point to The calendar struct
 * @param[in] field The index of the integer value in calendar service value.
 * @param[in] timezone_flag #cal_timezone_flag time flag means 'time' value is local or gmt time(CAL_TZ_FLAG_GMT or CAL_TZ_FLAG_LOCAL)
 * @return Integer value, or 0 if no value is obtained
 * @remarks none
 * @pre cal_struct varibale is defined.
 * @post none
 * @code
 	#include <calendar-svc-provider.h>
 	#include <time.h>
 	void sample_code()
 	{
		time_t last_modified_time = 0;
		index = 1;
		cal_struct* event = NULL;

 		//connect to database
 		calendar_svc_connect();

 		//get the record
 		calendar_svc_get("schedule",index,NULL,&event);

		//get the time value
 		last_modified_time = calendar_svc_struct_get_time(event,CAL_VALUE_GMT_LAST_MODIFIED_TIME,CAL_TZ_FLAG_GMT);

 		//free space
		calendar_svc_free(&event);

		//close database
 		calendar_svc_close();
 	}
 * @endcode
 * @see calendar_svc_struct_set_time().
 */
struct tm* calendar_svc_struct_get_tm(cal_struct* record, const char *field, int timezone_flag);



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
 * @fn int calendar_svc_struct_set_tm(cal_struct* record, const char* field,int timezone_flag, struct tm* time);
 * This function sets time value of the calendar service value,it is convenient for user set the value without knowing the detail of the struct.
 *
 * @ingroup detail_management
 * @param[in] record Point to The calendar struct
 * @param[in] field The index of the integer value in calendar service value.
 * @param[in] timezone_flag #cal_timezone_flag time flag means 'time' value is local or gmt time(CAL_TZ_FLAG_GMT or CAL_TZ_FLAG_LOCAL)
 * @param[in] time time value in calendar service value.
 * @return Integer value, or 0 if no value is obtained
 * @remarks none.
 * @pre cal_struct variable is defined.
 * @post the corresponding value of cal_struct is set.
 * @code
 	#include <calendar-svc-provider.h>
 	#include <time.h>
 	void sample_code()
 	{
		time_t last_modified_time = time(NULL);
		index = 0;
		cal_struct* event = NULL;

 		//connect to database
 		calendar_svc_connect();

		//create a cal_struct variable
		event = calendar_svc_struct_new("schedule");

		//set the time value
 		calendar_svc_set_time(event,CAL_VALUE_GMT_LAST_MODIFIED_TIME,CAL_TZ_FLAG_GMT,last_modified_time);

 		//insert the record
 		index = calendar_svc_insert(event);

		//free the space
 		calendar_svc_struct_free(&event);

		//close database
 		calendar_svc_close();
 	}
 * @endcode
 * @see calendar_svc_struct_get_time().
 */
int calendar_svc_struct_set_tm(cal_struct* record, const char *field, int timezone_flag,struct tm* time);

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
 * @fn int calendar_svc_value_set_time(cal_value* value, const char* field,int timezone_flag, time_t time);
 * This function sets time value of the calendar service value,it is convenient for user set value of cal_value varible.
 *
 * @ingroup detail_management
 * @param[in] value The calendar service value
 * @param[in] field The index of the integer value in calendar service value.
 * @param[in] timezone_flag #cal_timezone_flag time flag means 'time' value is local or gmt time(CAL_TZ_FLAG_GMT or CAL_TZ_FLAG_LOCAL)
 * @param[in] time timestamp value in calendar service value.
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
 		event = calendar_svc_value_new(CAL_VALUE_LST_EXCEPTION_DATE);
 		calendar_svc_value_set_time(event,CAL_VALUE_GMT_EXCEPTION_DATE_TIME,CAL_TZ_FLAG_GMT,time(NULL));

		//set the list
		list = g_list_append(list,event);
		calendar_svc_struct_store_list(event,CAL_VALUE_LST_EXCEPTION_DATE,list);

		//free the space
 		calendar_svc_value_free(&event);

		//close database
 		calendar_svc_close();
 	}
 * @endcode
 * @see calendar_svc_value_get_time().
 */
int calendar_svc_value_set_tm (cal_value *value, const char *field,int timezone_flag, struct tm* time);

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
 		int event_id = 0;
		cal_value* event = NULL;

 		//connect to database
 		calendar_svc_connect();

		//create the event
 		event = calendar_svc_value_new("meeting_category");

		//get the event_id value
 		event_id = calendar_svc_value_get_int(event,"event_id");

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
 		char* category_name = "money";
		cal_value* event = NULL;

 		//connect to database
 		calendar_svc_connect();

		//create the event
 		event = calendar_svc_value_new("meeting_category");

		//get the event_id value
 		category_name = calendar_svc_value_get_str(event,"category_name");

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
 * @fn time_t calendar_svc_value_get_time(cal_value* value, const char* field,int timezone_flag);
 * This function gets time value of the calendar service value,it is convenient for user get value of cal_value varible.
 *
 * @ingroup detail_management
 * @param[in] value The calendar service value
 * @param[in] field The index of the integer value in calendar service value.
 * @param[in] timezone_flag #cal_timezone_flag time flag means 'time' value is local or gmt time(CAL_TZ_FLAG_GMT or CAL_TZ_FLAG_LOCAL)
 * @return time time value in calendar service value.
 * @remarks none.
 * @pre cal_value variable is defined.
 * @post none.
 * @code
 	#include <calendar-svc-provider.h>
 	#include <time.t>
 	void sample_code()
 	{
 		time_t exception_time = 0;
		index = 1;
		cal_value* event = NULL;

 		//connect to database
 		calendar_svc_connect();

		//create the event
 		event = calendar_svc_value_new(CAL_VALUE_LST_EXCEPTION_DATE);
 		exception_time = calendar_svc_value_get_time(event,CAL_VALUE_GMT_EXCEPTION_DATE_TIME,CAL_TZ_FLAG_GMT);

		//free the space
 		calendar_svc_value_free(&event);

		//close database
 		calendar_svc_close();
 	}
 * @endcode
 * @see calendar_svc_value_set_time().
 */
struct tm* calendar_svc_value_get_tm (cal_value *value, const char *field,int timezone_flag);



/**
 * @defgroup utilities utilities
 * @ingroup CALENDAR_SVC
 * @brief
 *		date/time, timzone utilities
 */

/**
 * @fn int calendar_svc_util_convert_db_time (struct tm* fromTime,char* fromTz, struct tm *toTime, char *toTz);
 * This function gets local time by gmt0 time,it is convenient for user to convert time.
 */
int calendar_svc_util_convert_db_time (struct tm* fromTime,char *fromTz, struct tm *toTime, char *toTz);


/**
 * @fn int calendar_svc_util_gmt_to_local(time_t fromTime,time_t *toTime);
 * This function gets local time by gmt0 time,it is convenient for user to convert time.
 *
 * @ingroup utilities
 * @param[in] fromTime gmt0 time
 * @param[out] toTime local time
 * @return This function returns CAL_SUCCESS or error code on failure.
 * @remarks none
 * @pre none
 * @post none
 * @code
   #include <calendar_svc_provider.h>
   #include <time.h>
   void sample_code()
   {
   		time_t fromTime = time(NULL);
   		time_t toTime = 0;

   		//connect to database
   		calendar_svc_connect();

    	//convert
    	calendar_svc_util_gmt_to_local(fromTime,&toTime);

   		//close to database
   		calendar_svc_close();
   }
 * @endcode
 * @see calendar_svc_util_local_to_gmt().
 */
int calendar_svc_util_gmt_to_local(time_t fromTime,time_t *toTime);

/**
 * @fn int calendar_svc_util_local_to_gmt(time_t fromTime,time_t *toTime);
 * This function gets gmt0 time by local time,it is convenient for user to convert time.
 *
 * @ingroup utilities
 * @param[in] fromTime local time
 * @param[out] toTime gmt0 time
 * @return This function returns CAL_SUCCESS or error code on failure.
 * @remarks none
 * @pre none
 * @post none
 * @code
   #include <calendar_svc_provider.h>
   #include <time.h>
   void sample_code()
   {
   		time_t fromTime = time(NULL);
   		time_t toTime = 0;

   		//connect to database
   		calendar_svc_connect();

    	//convert
    	calendar_svc_util_local_to_gmt(fromTime,&toTime);

   		//close to database
   		calendar_svc_close();
   }
 * @endcode
 * @see calendar_svc_util_gmt_to_local().
 */
int calendar_svc_util_local_to_gmt(time_t fromTime,time_t *toTime);


/**
 * @fn int calendar_svc_util_save_vcs_by_index(const int index,char* full_file_path);
 * This function makes vcal file by record index,it is convenient for user to get vcal from the calendar record.
 *
 * @ingroup utilities
 * @param[in]	index	    event's index for vcal converting
 * @param[out]	full_file_path	Points the file path.
 * @return	 This function returns CAL_SUCCESS or error code on failure.
 * @remarks none
 * @pre none
 * @post none
 * @code
	#include <calendar_svc_provider.h>
	#include <time.h>
	void sample_code()
	{
			char* full_file_path = "/opt/dbspace";
			int index = 1;

			//connect to database
			calendar_svc_connect();

			//convert
			calendar_svc_util_save_vcs_by_index(index,full_file_path);

			//close to database
			calendar_svc_close();
	}
 * @endcode
 * @see calendar_svc_util_register_vcs_file(),calendar_svc_util_convert_vcs_to_event().
 */
int calendar_svc_util_save_vcs_by_index(const int index,char *full_file_path);

/**
 * @fn int calendar_svc_util_register_vcs_file(const char * file_name);
 * This function registers vcal to calendar db,user can save vcal to database through calling it.
 *
 * @ingroup utilities
 * @param[in]	file_name	    vcalendar's file name
 * @return	 This function returns cid or error code on failure.
 * @remarks none
 * @pre the vcalendar exists
 * @post none
 * @code
	#include <calendar_svc_provider.h>
	void sample_code()
	{
			char* full_name = "/opt/dbspace/vcalendar_test.vcs";
			int cal_id = 0;

			//connect to database
			calendar_svc_connect();

			//convert
			cal_id = calendar_svc_util_register_vcs_file(full_name);

			//close to database
			calendar_svc_close();
	}
 * @endcode
 * @see calendar_svc_util_save_vcs_by_index(),calendar_svc_util_convert_vcs_to_event().
 */
int calendar_svc_util_register_vcs_file(const char * file_name);

/**
 * @fn int calendar_svc_util_convert_vcs_to_event (const char *raw_data,int data_size,cal_struct **record);
 * This function converts data (raw_data(vcal format) to cal_struct(event)),it is convenient for user to convert.
 *
 * @ingroup utilities
 * @param[in]	raw_data	    vcalendar event raw data
 * @param[in]	data_size	   raw_data buf size
 * @param[out] 	record    assigned record
 * @return	 This function returns CAL_SUCCESS or error code on failure.
 * @remarks none
 * @pre the vcalendar data is valid.
 * @post none
 * @code
	#include <calendar_svc_provider.h>
	void sample_code()
	{
			char raw_data[] = "";//raw data

			//connect to database
			calendar_svc_connect();

			cal_struct* event = NULL;

			//convert
			calendar_svc_util_convert_vcs_to_event (raw_data,strlen(raw_data),&event);

			calendar_svc_struct_free(&event);

			//close to database
			calendar_svc_close();
	}
 * @endcode
 * @see calendar_svc_util_convert_event_to_vcs().
 */
int calendar_svc_util_convert_vcs_to_event (const char *raw_data,int data_size,cal_struct **record);

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
 * @see calendar_svc_util_convert_vcs_to_event().
 */
int calendar_svc_util_convert_event_to_vcs (cal_struct *record,char **raw_data,int *data_size);


/**
 * @fn int calendar_svc_util_next_valid_event(cal_struct* event,time_t start_time,time_t end_time,time_t *next_valid_start_time,time_t *next_valid_end_time);
 * This function gets next valid event(it should be recurrence event) by period,user can get next valid event't time through calling it.
 *
 * @ingroup utilities
 * @param[in] event point of event struct
 * @param[in] start_time start point of valid time period
 * @param[in] end_time end point of valid time period
 * @param[out] next_valid_start_time next valid start time in period
 * @param[out] next_valid_end_time next valid end time in period
 * @return This function returns CAL_SUCCESS or error code on failure.
 * @remarks none
 * @pre the event must be recurrence event
 * @deprecated this api will be deprecated.
 * @post none
 * @code
  #include <calendar_svc_provider.h>
  #include <time.h>
  void sample_code()
  {
  		cal_struct* event = NULL;
  		index = 1;
  		time_t start_time = time(NULL);
  		time_t end_time = start_time + 1000000;
  		time_t next_valid_start_time = 0;
  		time_t next_valid_end_time = 0;

  		//connect to database
  		calendar_svc_connect();

		//get the record
		calendar_svc_get("schedule",NULL,index,&event);

		//get the next valid event time
		calendar_svc_util_next_valid_event(event,start_time,end_time,&next_valid_start_time,&next_valid_end_time);

		//free the space
		calendar_svc_struct_free(&event);

  		//close database
  		calendar_svc_close();

  }
 * @endcode
 * @see none.
 */
int calendar_svc_util_next_valid_event(cal_struct* event,time_t start_time,time_t end_time,
                               time_t *next_valid_start_time,time_t *next_valid_end_time);

/**
 * @fn time_t calendar_svc_value_get_time(cal_value* value, const char* field,int timezone_flag);
 * This function gets time value of the calendar service value,it is convenient for user get value of cal_value varible.
 *
 * @ingroup detail_management
 * @param[in] value The calendar service value
 * @param[in] field The index of the integer value in calendar service value.
 * @param[in] timezone_flag #cal_timezone_flag time flag means 'time' value is local or gmt time(CAL_TZ_FLAG_GMT or CAL_TZ_FLAG_LOCAL)
 * @return time time value in calendar service value.
 * @remarks none.
 * @pre cal_value variable is defined.
 * @deprecated this api will be deprecated.
 * @post none.
 * @code
 	#include <calendar-svc-provider.h>
 	#include <time.t>
 	void sample_code()
 	{
 		time_t exception_time = 0;
		index = 1;
		cal_value* event = NULL;

 		//connect to database
 		calendar_svc_connect();

		//create the event
 		event = calendar_svc_value_new(CAL_VALUE_LST_EXCEPTION_DATE);
 		exception_time = calendar_svc_value_get_time(event,CAL_VALUE_GMT_EXCEPTION_DATE_TIME,CAL_TZ_FLAG_GMT);

		//free the space
 		calendar_svc_value_free(&event);

		//close database
 		calendar_svc_close();
 	}
 * @endcode
 * @see calendar_svc_value_set_time().
 */
time_t calendar_svc_value_get_time(cal_value* value, const char *field,int timezone_flag);

/**
 * @fn int calendar_svc_value_set_time(cal_value* value, const char* field,int timezone_flag, time_t time);
 * This function sets time value of the calendar service value,it is convenient for user set value of cal_value varible.
 *
 * @ingroup detail_management
 * @param[in] value The calendar service value
 * @param[in] field The index of the integer value in calendar service value.
 * @param[in] timezone_flag #cal_timezone_flag time flag means 'time' value is local or gmt time(CAL_TZ_FLAG_GMT or CAL_TZ_FLAG_LOCAL)
 * @param[in] time timestamp value in calendar service value.
 * @return	This function returns CAL_SUCCESS or error code on failure.
 * @remarks none.
 * @pre cal_value variable is defined.
 * @post none.
 * @deprecated this api will be deprecated.
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
 		event = calendar_svc_value_new(CAL_VALUE_LST_EXCEPTION_DATE);
 		calendar_svc_value_set_time(event,CAL_VALUE_GMT_EXCEPTION_DATE_TIME,CAL_TZ_FLAG_GMT,time(NULL));

		//set the list
		list = g_list_append(list,event);
		calendar_svc_struct_store_list(event,CAL_VALUE_LST_EXCEPTION_DATE,list);

		//free the space
 		calendar_svc_value_free(&event);

		//close database
 		calendar_svc_close();
 	}
 * @endcode
 * @see calendar_svc_value_get_time().
 */
int calendar_svc_value_set_time(cal_value* value, const char *field,int timezone_flag, time_t time);

/**
 * @fn int calendar_svc_struct_set_time(cal_struct* record, const char* field,int timezone_flag, time_t time);
 * This function sets time value of the calendar service value,it is convenient for user set the value without knowing the detail of the struct.
 *
 * @ingroup detail_management
 * @param[in] record Point to The calendar struct
 * @param[in] field The index of the integer value in calendar service value.
 * @param[in] timezone_flag #cal_timezone_flag time flag means 'time' value is local or gmt time(CAL_TZ_FLAG_GMT or CAL_TZ_FLAG_LOCAL)
 * @param[in] time time value in calendar service value.
 * @return Integer value, or 0 if no value is obtained
 * @remarks none.
 * @pre cal_struct variable is defined.
 * @post the corresponding value of cal_struct is set.
 * @deprecated this api will be deprecated.
 * @code
 	#include <calendar-svc-provider.h>
 	#include <time.h>
 	void sample_code()
 	{
		time_t last_modified_time = time(NULL);
		index = 0;
		cal_struct* event = NULL;

 		//connect to database
 		calendar_svc_connect();

		//create a cal_struct variable
		event = calendar_svc_struct_new("schedule");

		//set the time value
 		calendar_svc_set_time(event,CAL_VALUE_GMT_LAST_MODIFIED_TIME, CAL_TZ_FLAG_GMT, last_modified_time);

 		//insert the record
 		index = calendar_svc_insert(event);

		//free the space
 		calendar_svc_struct_free(&event);

		//close database
 		calendar_svc_close();
 	}
 * @endcode
 * @see calendar_svc_struct_set_tm().
 */
int calendar_svc_struct_set_time(cal_struct* record, const char *field,int timezone_flag, time_t time);

/**
 * @fn time_t calendar_svc_struct_get_time(cal_struct* record, const char* field, int timezone_flag);
 * This function gets time value of the calendar service value,it is convenient for user get the value needed without knowing the detail of the struct.
 *
 * @ingroup detail_management
 * @param[in] record Point to The calendar struct
 * @param[in] field The index of the integer value in calendar service value.
 * @param[in] timezone_flag #cal_timezone_flag time flag means 'time' value is local or gmt time(CAL_TZ_FLAG_GMT or CAL_TZ_FLAG_LOCAL)
 * @return Integer value, or 0 if no value is obtained
 * @remarks none
 * @pre cal_struct varibale is defined.
 * @post none
 * @deprecated this api will be deprecated.
 * @code
 	#include <calendar-svc-provider.h>
 	#include <time.h>
 	void sample_code()
 	{
		time_t last_modified_time = 0;
		index = 1;
		cal_struct* event = NULL;

 		//connect to database
 		calendar_svc_connect();

 		//get the record
 		calendar_svc_get("schedule",index,NULL,&event);

		//get the time value
 		last_modified_time = calendar_svc_struct_get_time(event,CAL_VALUE_GMT_LAST_MODIFIED_TIME);

 		//free space
		calendar_svc_free(&event);

		//close database
 		calendar_svc_close();
 	}
 * @endcode
 * @see calendar_svc_struct_get_tm().
 */
time_t calendar_svc_struct_get_time(cal_struct* record, const char *field, int timezone_flag);

/**
 * @fn void calendar_svc_util_get_local_tz_info(char **lock_city_name,char **lock_tz_path,char** lock_tz_offset,char **local_city_name,char **local_tz_path,char **local_tz_offset);
	get timezone information by setting value
 * @deprecated it will be deprecated.
 */
void calendar_svc_util_get_local_tz_info(char **lock_city_name,char **lock_tz_path,char** lock_tz_offset,
											char **local_city_name,char **local_tz_path,char **local_tz_offset);

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

int calendar_svc_find_recurring_event_list (int account_id, cal_iter **iter);

int calendar_svc_find_event_list_by_filter(int account_id, int filter_count, const char *search_type[], const void *search_value[], cal_iter **iter);

/**
 * @fn int calendar_svc_find(int account_id,int calendar_id,const char *data_type,const char *search_type,const void *search_value, cal_iter **iter);
 * This function get records from database by search param,it is convenient for user to get records according to some condition.
 *
 * @ingroup event_management
 * @return This function returns CAL_SUCCESS or error code on failure.
 * @param[in] account_id account db index
 * @param[in] calendar_id calendar db index
 * @param[in] data_type struct type(CAL_STRUCT_SCHEDULE,CAL_STRUCT_CALENDAR,CAL_STRUCT_TIMEZONE,..)
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
	calendar_svc_find(LOCAL_ACCOUNT_ID,DEFAULT_CALENDAR_ID,CAL_STRUCT_SCHEDULE,"summary","party", &iter);

	//free
	calendar_svc_iter_remove(&iter);

	//close database
	calendar_svc_close();
  }
 * @endcode
 * @see detail_management module
 * @deprecated it will replacement calendar_svc_find_list
 */
int calendar_svc_find(int account_id,int calendar_id,const char *data_type,const char *search_type,const void *search_value, cal_iter **iter);


/**
 * @fn int calendar_svc_search_list(int account_id,int calendar_id,const char *data_type,const char *search_type,const void *search_value,int offset,int count, cal_iter **iter);
 * This function get records from database by search param,it is convenient for user to get records according to some condition.

 * @deprecated it will replacement calendar_svc_find_list
 **/
int calendar_svc_search_list(int account_id,int calendar_id,const char *data_type,const char *search_type,const void *search_value,
								 int offset,int count, cal_iter **iter);


#ifdef __cplusplus
}
#endif

typedef enum
{
        CAL_STATUS_FREE = 0,               /**< deprecated */
        CAL_STATUS_TENTATIVE,             /**< deprecated */
        CAL_STATUS_BUSY,                     /**< deprecated */
        CAL_STATUS_OUROFOFFICE,           /**< deprecated */
        CAL_STATUS_CONFIRM,               /**< deprecated */
        CAL_STATUS_DENIED,                /**< deprecated */
} cal_status_type_t;

#endif /* __CALENDAR_SVC_H__ */

