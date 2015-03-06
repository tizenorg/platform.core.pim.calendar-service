/*
 * Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __TIZEN_SOCIAL_CALENDAR_TYPES_H__
#define __TIZEN_SOCIAL_CALENDAR_TYPES_H__

#include <stdint.h>
#include <tizen.h>
#include <calendar_errors.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#define _CALENDAR_HANDLE(A) typedef struct __##A* A;

#define _CALENDAR_BEGIN_VIEW() \
		typedef struct{ \
			const char* _uri;
#define _CALENDAR_PROPERTY_INT(property_id_name)        unsigned int property_id_name;
#define _CALENDAR_PROPERTY_STR(property_id_name)        unsigned int property_id_name;
#define _CALENDAR_PROPERTY_DOUBLE(property_id_name)     unsigned int property_id_name;
#define _CALENDAR_PROPERTY_LLI(property_id_name)        unsigned int property_id_name;
#define _CALENDAR_PROPERTY_CALTIME(property_id_name)    unsigned int property_id_name;
#define _CALENDAR_PROPERTY_CHILD_MULTIPLE(property_id_name) unsigned int property_id_name;
#define _CALENDAR_END_VIEW(name) } name##_property_ids; \
    extern API const name##_property_ids name;

#define _CALENDAR_BEGIN_READ_ONLY_VIEW() \
        typedef struct{ \
            const char* _uri;
#define _CALENDAR_PROPERTY_PROJECTION_INT(property_id_name)        unsigned int property_id_name;
#define _CALENDAR_PROPERTY_PROJECTION_STR(property_id_name)        unsigned int property_id_name;
#define _CALENDAR_PROPERTY_PROJECTION_DOUBLE(property_id_name)     unsigned int property_id_name;
#define _CALENDAR_PROPERTY_PROJECTION_LLI(property_id_name)        unsigned int property_id_name;
#define _CALENDAR_PROPERTY_PROJECTION_CALTIME(property_id_name)    unsigned int property_id_name;
#define _CALENDAR_PROPERTY_FILTER_INT(property_id_name)        unsigned int property_id_name;
#define _CALENDAR_PROPERTY_FILTER_STR(property_id_name)        unsigned int property_id_name;
#define _CALENDAR_PROPERTY_FILTER_DOUBLE(property_id_name)     unsigned int property_id_name;
#define _CALENDAR_PROPERTY_FILTER_LLI(property_id_name)        unsigned int property_id_name;
#define _CALENDAR_PROPERTY_FILTER_CALTIME(property_id_name)    unsigned int property_id_name;
#define _CALENDAR_END_READ_ONLY_VIEW(name) } name##_property_ids; \
    extern API const name##_property_ids name;

_CALENDAR_HANDLE( calendar_record_h )
_CALENDAR_HANDLE( calendar_filter_h )
_CALENDAR_HANDLE( calendar_list_h )
_CALENDAR_HANDLE( calendar_query_h )


/**
 * @file calendar_types.h
 */

/**
 * @addtogroup CAPI_SOCIAL_CALENDAR_SVC_DATABASE_MODULE
 * @{
 */
/**
 * @brief Definition for a calendar connect flag.
 *
 * @since_tizen 2.3
 */
#define CALENDAR_CONNECT_FLAG_NONE         0x00000000
/**
 * @brief Definition for a calendar connect flag.
 *
 * @since_tizen 2.3
 */
#define CALENDAR_CONNECT_FLAG_RETRY        0x00000001

/**
 * @brief Definition for a default event calendar book database ID.
 *
 * @since_tizen 2.3
 */
#define DEFAULT_EVENT_CALENDAR_BOOK_ID           1

/**
 * @brief Definition for a default to-do calendar book database ID.
 *
 * @since_tizen 2.3
 */
#define DEFAULT_TODO_CALENDAR_BOOK_ID            2

/**
 * @brief Definition for a default birthday calendar book database ID.
 *
 * @since_tizen 2.3
 */
#define DEFAULT_BIRTHDAY_CALENDAR_BOOK_ID        3

/**
 * @brief Definition for no due date of a to-do.
 *
 * @since_tizen 2.3
 */
#define CALENDAR_TODO_NO_DUE_DATE   INT64_MAX

/**
 * @brief Definition for no start date of a to-do.
 *
 * @since_tizen 2.3
 */
#define CALENDAR_TODO_NO_START_DATE (-INT64_MAX)

/**
 * @brief Definition for no until of a record.
 *
 * @since_tizen 2.3
 */
#define CALENDAR_RECORD_NO_UNTIL    INT64_MAX

/**
 * @brief Definition for no coordinate(latitude/longitude) of a record.
 *
 * @since_tizen 2.3
 */
#define CALENDAR_RECORD_NO_COORDINATE 1000.0

/**
 * @}
 */

/**
 * @addtogroup CAPI_SOCIAL_CALENDAR_SVC_FILTER_MODULE
 * @{
 */

/**
 * @brief Definition for all calendar books.
 *
 * @since_tizen 2.3
 */
#define CALENDAR_BOOK_FILTER_ALL                    -1

/**
 * @brief Enumeration for the filter match type of a string.
 *
 * @since_tizen 2.3
 */
typedef enum
{
	CALENDAR_MATCH_EXACTLY,			/**< Full string, case-sensitive */
	CALENDAR_MATCH_FULLSTRING,		/**< Full string, case-insensitive */
	CALENDAR_MATCH_CONTAINS,		/**< Sub string, case-insensitive */
	CALENDAR_MATCH_STARTSWITH,		/**< Start with, case-insensitive */
	CALENDAR_MATCH_ENDSWITH,		/**< End with, case-insensitive */
	CALENDAR_MATCH_EXISTS,			/**< IS NOT NULL */
	CALENDAR_MATCH_STR_MAX        /**< Calendar match string flag max enum count */
} calendar_match_str_flag_e;

/**
 * @brief Enumeration for the filter match type of an integer.
 *
 * @since_tizen 2.3
 */
typedef enum
{
	CALENDAR_MATCH_EQUAL,					/**< '=' */
	CALENDAR_MATCH_GREATER_THAN,			/**< '>' */
	CALENDAR_MATCH_GREATER_THAN_OR_EQUAL,	/**< '>=' */
	CALENDAR_MATCH_LESS_THAN,				/**< '<' */
	CALENDAR_MATCH_LESS_THAN_OR_EQUAL,		/**< '<=' */
	CALENDAR_MATCH_NOT_EQUAL,               /**< '<>', this flag can yield poor performance */
	CALENDAR_MATCH_NONE,						/**< IS NULL */
	CALENDAR_MATCH_INT_MAX					/**< Calendar match integer flag max enum count */
} calendar_match_int_flag_e;

/**
 * @brief Enumeration for a filter operator.
 *
 * @since_tizen 2.3
 */
typedef enum {
	CALENDAR_FILTER_OPERATOR_AND,	/**< AND */
	CALENDAR_FILTER_OPERATOR_OR,		/**< OR */
	CALENDAR_FILTER_OPERATOR_MAX	/**< Calendar filter operator max enum count */
} calendar_filter_operator_e;

/**
 * @}
 */

/**
 * @addtogroup CAPI_SOCIAL_CALENDAR_SVC_RECORD_MODULE
 * @{
 */

/**
 * @brief Enumeration for the calendar book type.
 *
 * @details "OR"ing is supported.
 *
 * @since_tizen 2.3
 */
typedef enum
{
	CALENDAR_BOOK_TYPE_NONE          = 0,       /**< Default calendar book type */
	CALENDAR_BOOK_TYPE_EVENT         = 1<<0,        /**< Event calendar book type */
	CALENDAR_BOOK_TYPE_TODO          = 1<<1     /**< To-do calendar book type */
} calendar_book_type_e;

/**
 * @brief Enumeration for the calendar sensitivity type.
 *
 * @since_tizen 2.3
 */
typedef enum
{
	CALENDAR_SENSITIVITY_PUBLIC          = 0,	/**< Public Sensitivity */
	CALENDAR_SENSITIVITY_PRIVATE,				/**< Private Sensitivity */
	CALENDAR_SENSITIVITY_CONFIDENTIAL			/**< Confidential Sensitivity */
} calendar_sensitivity_e;

/**
 * @brief Enumeration for the attendee status.
 *
 * @since_tizen 2.3
 */
typedef enum
{
	CALENDAR_ATTENDEE_STATUS_PENDING	= 0,	/**< Pending status */
	CALENDAR_ATTENDEE_STATUS_ACCEPTED,			/**< Accepted status */
	CALENDAR_ATTENDEE_STATUS_DECLINED,			/**< Declined status */
	CALENDAR_ATTENDEE_STATUS_TENTATIVE,			/**< Tentative status */
	CALENDAR_ATTENDEE_STATUS_DELEGATED,			/**< Delegated status */
	CALENDAR_ATTENDEE_STATUS_COMPLETED,			/**< Completed status */
	CALENDAR_ATTENDEE_STATUS_IN_PROCESS,			/**< In process status */
	CALENDAR_ATTENDEE_STATUS_MAX,    /**< Calendar attendee status max enum count */
}calendar_attendee_status_e;

/**
 * @brief Enumeration for the attendee role.
 *
 * @since_tizen 2.3
 */
typedef enum
{
	CALENDAR_ATTENDEE_ROLE_REQ_PARTICIPANT	= 0,	/**< Participation is required */
	CALENDAR_ATTENDEE_ROLE_OPT_PARTICIPANT,			/**< Accepted status */
	CALENDAR_ATTENDEE_ROLE_NON_PARTICIPANT,			/**< Non-Participant */
	CALENDAR_ATTENDEE_ROLE_CHAIR,					/**< Chairperson */
	CALENDAR_ATTENDEE_ROLE_MAX,    /**< Calendar attendee role max enum count */
}calendar_attendee_role_e;

/**
 * @brief Enumeration for the attendee cutype.
 *
 * @since_tizen 2.3
 */
typedef enum
{
	CALENDAR_ATTENDEE_CUTYPE_INDIVIDUAL = 0,	/**< Individual cutype */
	CALENDAR_ATTENDEE_CUTYPE_GROUP,             /**< Group cutype */
	CALENDAR_ATTENDEE_CUTYPE_RESOURCE,          /**< Resource cutype */
	CALENDAR_ATTENDEE_CUTYPE_ROOM,              /**< Room cutype */
	CALENDAR_ATTENDEE_CUTYPE_UNKNOWN,           /**< Unknown cutype */
	CALENDAR_ATTENDEE_CUTYPE_MAX,    /**< Calendar attendee cutype max enum count */
}calendar_attendee_cutyep_e;

/**
 * @brief Enumeration for the alarm time unit type of an event, such as minutes, hours, days, and so on.
 *
 * @since_tizen 2.3
 */
typedef enum
{
	CALENDAR_ALARM_NONE = -1,                  /**< No reminder set */
	CALENDAR_ALARM_TIME_UNIT_SPECIFIC = 1,     /**< Specific in seconds */
	CALENDAR_ALARM_TIME_UNIT_MINUTE = 60,      /**< Alarm time unit in minutes */
	CALENDAR_ALARM_TIME_UNIT_HOUR = 3600,      /**< Alarm time unit in hours */
	CALENDAR_ALARM_TIME_UNIT_DAY = 86400,      /**< Alarm time unit in days */
	CALENDAR_ALARM_TIME_UNIT_WEEK = 604800,    /**< Alarm time unit in weeks */
} calendar_alarm_time_unit_type_e;

/**
 * @brief Enumeration for the alarm action.
 *
 * @since_tizen 2.3
 */
typedef enum
{
	CALENDAR_ALARM_ACTION_AUDIO = 0, /**< Audio action */
	CALENDAR_ALARM_ACTION_DISPLAY,   /**< Display action */
	CALENDAR_ALARM_ACTION_EMAIL,     /**< Email action */
	CALENDAR_ALARM_ACTION_MAX,    /**< Calenar alarm action max enum count */
}calendar_alarm_action_e;

/**
 * @brief Enumeration for the frequency of an event's recurrence.
 *
 * @since_tizen 2.3
 */
typedef enum
{
	CALENDAR_RECURRENCE_NONE,           /**< No recurrence event */
	CALENDAR_RECURRENCE_DAILY,          /**< An event occurs every day */
	CALENDAR_RECURRENCE_WEEKLY,         /**< An event occurs on the same day of every week \n According to the week flag, the event will recur every day of the week */
	CALENDAR_RECURRENCE_MONTHLY,        /**< An event occurs on the same day of every month */
	CALENDAR_RECURRENCE_YEARLY         /**< An event occurs on the same day of every year */
} calendar_recurrence_frequency_e;

/**
 * @brief Enumeration for the event status.
 *
 * @since_tizen 2.3
 */
typedef enum
{
	CALENDAR_EVENT_STATUS_NONE		= 0x01,		/**< No status */
	CALENDAR_EVENT_STATUS_TENTATIVE	= 0x02,		/**< The event is tentative */
	CALENDAR_EVENT_STATUS_CONFIRMED	= 0x04,		/**< The event is confirmed */
	CALENDAR_EVENT_STATUS_CANCELLED	= 0x08		/**< The event is canceled */
}calendar_event_status_e;

/**
 * @brief Enumeration for the busy status of an event.
 *
 * @since_tizen 2.3
 */
typedef enum
{
	CALENDAR_EVENT_BUSY_STATUS_FREE = 0,		/**< The free status */
	CALENDAR_EVENT_BUSY_STATUS_BUSY,			/**< The busy status */
	CALENDAR_EVENT_BUSY_STATUS_UNAVAILABLE,		/**< The unavailable status */
	CALENDAR_EVENT_BUSY_STATUS_TENTATIVE		/**< The tentative status */
}calendar_event_busy_status_e;
/**
 * @brief Enumeration for the calendar event item's priority.
 *
 * @since_tizen 2.3
 */
typedef enum
{
	CALENDAR_EVENT_PRIORITY_NONE		= 0x01, /**< No priority */
	CALENDAR_EVENT_PRIORITY_LOW         = 0x08,	/**< Low priority */
	CALENDAR_EVENT_PRIORITY_NORMAL		= 0x04,	/**< Normal priority */
	CALENDAR_EVENT_PRIORITY_HIGH		= 0x02,	/**< High priority */
} calendar_event_priority_e;

/**
 * @brief Enumeration for the calendar to-do item's priority.
 *
 * @since_tizen 2.3
 */
typedef enum
{
	CALENDAR_TODO_PRIORITY_NONE			= 0x01, /**< No priority */
	CALENDAR_TODO_PRIORITY_LOW          = 0x08,	/**< Low priority */
	CALENDAR_TODO_PRIORITY_NORMAL		= 0x04,	/**< Normal priority */
	CALENDAR_TODO_PRIORITY_HIGH			= 0x02,	/**< High priority */
} calendar_todo_priority_e;

/**
 * @brief Enumeration for the status of a to-do.
 *
 * @since_tizen 2.3
 */
typedef enum
{
	CALENDAR_TODO_STATUS_NONE			= 0x0100,	/**< No status */
	CALENDAR_TODO_STATUS_NEEDS_ACTION	= 0x0200,	/**< Needs action status */
	CALENDAR_TODO_STATUS_COMPLETED		= 0x0400,	/**< Completed status */
	CALENDAR_TODO_STATUS_IN_PROCESS		= 0x0800,	/**< Work in process status */
	CALENDAR_TODO_STATUS_CANCELED		= 0x1000	/**< Canceled status */
} calendar_todo_status_e;

/**
 * @brief Enumeration for the time type.
 *
 * @since_tizen 2.3
 */
typedef enum
{
	CALENDAR_TIME_UTIME = 0,			/**< Unix time */
	CALENDAR_TIME_LOCALTIME,			/**< Local time */
} calendar_time_type_e;

/**
 * @brief Enumeration for the range type.
 *
 * @since_tizen 2.3
 */
typedef enum
{
	CALENDAR_RANGE_UNTIL,		/**< Range until */
	CALENDAR_RANGE_COUNT,		/**< Range count */
	CALENDAR_RANGE_NONE,		/**< No range */
} calendar_range_type_e;

/**
 * @brief Enumeration for the system type.
 *
 * @since_tizen 2.3
 */
typedef enum
{
	CALENDAR_SYSTEM_NONE,				   /**< Locale's default calendar */
	CALENDAR_SYSTEM_GREGORIAN,			   /**< Locale's default calendar */
	CALENDAR_SYSTEM_EAST_ASIAN_LUNISOLAR, /**< East asian lunisolar calendar */
} calendar_system_type_e;

/**
 * @brief Enumeration for the meeting status.
 *
 * @since_tizen 2.3
 */
typedef enum
{
	CALENDAR_MEETING_STATUS_NOTMEETING = 0,	  /**< No meeting */
	CALENDAR_MEETING_STATUS_MEETING,				  /**< Meeting exists */
	CALENDAR_MEETING_STATUS_RECEIVED,			  /**< Meeting received */
	CALENDAR_MEETING_STATUS_CANCELED,			  /**< Meeting canceled */
} calendar_meeting_status_e;

/**
 * @brief Enumeration for weekdays.
 * @details Same value as UCalendarDaysOfWeek in ICU.
 *
 * @since_tizen 2.3
 */
typedef enum
{
	CALENDAR_SUNDAY = 1,			/**< Sunday */
	CALENDAR_MONDAY,				/**< Monday */
	CALENDAR_TUESDAY,				/**< Tuesday */
	CALENDAR_WEDNESDAY,				/**< Wednesday */
	CALENDAR_THURSDAY,				/**< Thursday */
	CALENDAR_FRIDAY,				/**< Friday */
	CALENDAR_SATURDAY,				/**< Saturday */
}calendar_days_of_week_e;

/**
 * @brief Enumeration for the modified status of a record.
 *
 * @since_tizen 2.3
 */
typedef enum
{
	CALENDAR_RECORD_MODIFIED_STATUS_INSERTED = 0,		/**< The record is inserted */
	CALENDAR_RECORD_MODIFIED_STATUS_UPDATED,				/**< The record is updated */
	CALENDAR_RECORD_MODIFIED_STATUS_DELETED				/**< The record is deleted */
}calendar_record_modified_status_e;

/**
 * @brief The structure of time.
 *
 * @since_tizen 2.3
 */
typedef struct
{
	calendar_time_type_e type;	/**< type */
	union {
		long long int utime;	/**< utime */
		struct {
			int year;			/**< year */
			int month;			/**< month */
			int mday;			/**< mday */
			int hour;			/**< hour */
			int minute;			/**< minute */
			int second;			/**< second */
			bool is_leap_month; /**< Deprecated since 2.4:leap month */
		}date;
	}time;
}calendar_time_s;

/**
 * @brief Enumeration for the type of a record.
 *
 * @since_tizen 2.3
 */
typedef enum
{
	CALENDAR_RECORD_TYPE_NONE = 0,			 /**< No record type */
	CALENDAR_RECORD_TYPE_CALENDAR_BOOK,		 /**< Book type */
	CALENDAR_RECORD_TYPE_EVENT,				 /**< Event type */
	CALENDAR_RECORD_TYPE_TODO,					 /**< Todo type */
}calendar_record_type_e;

/**
 * @brief Enumeration for the book mode.
 *
 * @since_tizen 2.3
 */
typedef enum {
	CALENDAR_BOOK_MODE_NONE = 0, /**< All modules can read and write records of this calendar_book */
	CALENDAR_BOOK_MODE_RECORD_READONLY, /**< All modules can only read records of this calendar book */
} calendar_book_mode_e;

/**
 * @brief Enumeration for the sync event type.
 *
 * @since_tizen 2.3
 */
typedef enum {
	CALENDAR_BOOK_SYNC_EVENT_FOR_ME = 0,            /**< This book would not be synced to others except me */
	CALENDAR_BOOK_SYNC_EVENT_FOR_EVERY_AND_DELETE,  /**< This book would be sync to everyone and deleted events would be disappeared on time */
	CALENDAR_BOOK_SYNC_EVENT_FOR_EVERY_AND_REMAIN,	/**< This book would be sync to everyone but deleted events would be remained. deleted events is removed by calendar_db_clean_after_sync() API*/
} calendar_book_sync_event_type_e;

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif //__TIZEN_SOCIAL_CALENDAR_TYPES_H__
