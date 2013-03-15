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
#include <bundle.h>

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
 * @addtogroup CAPI_SOCIAL_CALENDAR_SVC_DATABASE_MODULE
 * @{
 */

/**
 * @brief The callback function to get the result of batch operation.
 *
 * @param[in]   error     		Error code for batch operation
 * @param[in]   user_data		The user data passed from the batch operation
 *
 * @return  @c true to continue with the next iteration of the loop or @c false to break out of the loop.
 *
 * @pre calendar_db_update_records() will invoke this callback.
 *
 * @see calendar_db_update_records()
 */
typedef void (*calendar_db_result_cb)( int error, void *user_data);

/**
 * @brief The callback function to get the result of batch operation.
 *
 * @param[in]   error           Error code for batch operation
 * @param[in]   record_id_array The record IDs for batch operation
 * @param[in]   count           The number of record ID array
 * @param[in]   user_data       The user data passed from the batch operation
 *
 * @return  @c true to continue with the next iteration of the loop or @c false to break out of the loop.
 *
 * @pre calendar_db_insert_records() will invoke this callback.
 *
 * @see calendar_db_insert_records()
 */
typedef void (*calendar_db_insert_result_cb)( int error, int* record_id_array, int count, void *user_data);

/**
 * @brief       Called when designated view changes.
 *
 * @param[in]   view_uri	The view uri
 * @param[in]   user_data	The user data passed from the callback registration function
 *
 * @see calendar_db_add_changed_cb()
 */
typedef void (*calendar_db_changed_cb)(const char* view_uri, void* user_data);

/**
 * @brief       Called when alarm is alerted.
 *
 * @param[in]   b			bundle
 * @param[in]   user_data	The user data passed from the callback registration function
 *
 * @see calendar_db_add_changed_cb()
 */
typedef void (*calendar_reminder_cb)(bundle *b, void* user_data);

/**
 * @brief Definition for calendar connect flag
 */
#define CALENDAR_CONNECT_FLAG_NONE         0x00000000
#define CALENDAR_CONNECT_FLAG_RETRY        0x00000001

/**
 * @brief Definition for default event calendar book database ID
 */
#define DEFAULT_EVENT_CALENDAR_BOOK_ID           1

/**
 * @brief Definition for default to-do calendar book database ID
 */
#define DEFAULT_TODO_CALENDAR_BOOK_ID            2

/**
 * @brief Definition for default birthday calendar book database ID
 */
#define DEFAULT_BIRTHDAY_CALENDAR_BOOK_ID        3

/**
 * @brief Definition for no due date of a to-do
 */
#define CALENDAR_TODO_NO_DUE_DATE   INT64_MAX

/**
 * @brief Definition for no start date of a to-do
 */
#define CALENDAR_TODO_NO_START_DATE (-INT64_MAX)

/**
 * @brief Definition for no until of a record
 */
#define CALENDAR_RECORD_NO_UNTIL    INT64_MAX

/**
 * @brief Definition for no coordinate(latitude/longitude) of a record
 */
#define CALENDAR_RECORD_NO_COORDINATE 1000

/**
 * @}
 */

/**
 * @addtogroup CAPI_SOCIAL_CALENDAR_SVC_FILTER_MODULE
 * @{
 */

/**
 * @brief Definition for all calendar book
 */
#define CALENDAR_BOOK_FILTER_ALL                    -1

/**
 * @brief Enumerations of filter match type for string
 */
typedef enum
{
	CALENDAR_MATCH_EXACTLY,			/**< . */
	CALENDAR_MATCH_FULLSTRING,		/**< . */
	CALENDAR_MATCH_CONTAINS,		/**< . */
	CALENDAR_MATCH_STARTSWITH,		/**< . */
	CALENDAR_MATCH_ENDSWITH,		/**< . */
	CALENDAR_MATCH_EXISTS           /**< . */
} calendar_match_str_flag_e;

/**
 * @brief Enumerations of filter match type for integer
 */
typedef enum
{
	CALENDAR_MATCH_EQUAL,					/**< . */
	CALENDAR_MATCH_GREATER_THAN,			/**< . */
	CALENDAR_MATCH_GREATER_THAN_OR_EQUAL,	/**< . */
	CALENDAR_MATCH_LESS_THAN,				/**< . */
	CALENDAR_MATCH_LESS_THAN_OR_EQUAL,		/**< . */
	CALENDAR_MATCH_NOT_EQUAL,               /**< this flag can yield poor performance */
	CALENDAR_MATCH_NONE						/**< . */
} calendar_match_int_flag_e;

/**
 * @brief Enumerations of filter combine type
 */
typedef enum {
	CALENDAR_FILTER_OPERATOR_AND,	/**< . */
	CALENDAR_FILTER_OPERATOR_OR		/**< . */
} calendar_filter_operator_e;

/**
 * @}
 */

/**
 * @addtogroup CAPI_SOCIAL_CALENDAR_SVC_RECORD_MODULE
 * @{
 */

/**
 * @brief Enumerations for calendar book type. "OR"ing supported.
 */
typedef enum
{
    CALENDAR_BOOK_TYPE_NONE          = 0,       /**< Calendar book type default */
    CALENDAR_BOOK_TYPE_EVENT         = 1<<0,        /**< Event calendar book type */
    CALENDAR_BOOK_TYPE_TODO          = 1<<1     /**< To-do Calendar book type */
} calendar_book_type_e;

/**
 * @brief Enumerations for calendar sensitivity type.
 */
typedef enum
{
    CALENDAR_SENSITIVITY_PUBLIC          = 0,	/**< Public Sensitivity */
    CALENDAR_SENSITIVITY_PRIVATE,				/**< Private Sensitivity */
    CALENDAR_SENSITIVITY_CONFIDENTIAL			/**< Confidential Sensitivity */
} calendar_sensitivity_e;

/**
 * @brief Enumerations of attendee status.
 */
typedef enum
{
	CALENDAR_ATTENDEE_STATUS_PENDING	= 0,	/**< Pending status */
	CALENDAR_ATTENDEE_STATUS_ACCEPTED,			/**< Accepted status */
	CALENDAR_ATTENDEE_STATUS_DECLINED,			/**< Decliend status */
	CALENDAR_ATTENDEE_STATUS_TENTATIVE,			/**< Tentative status */
	CALENDAR_ATTENDEE_STATUS_DELEGATED,			/**< Delegated status */
	CALENDAR_ATTENDEE_STATUS_COMPLETED,			/**< Completed status */
	CALENDAR_ATTENDEE_STATUS_IN_PROCESS,			/**< In process status */
	CALENDAR_ATTENDEE_STATUS_MAX,
}calendar_attendee_status_e;

/**
 * @brief Enumerations of attendee role.
 */
typedef enum
{
	CALENDAR_ATTENDEE_ROLE_REQ_PARTICIPANT	= 0,	/**< Participation is required */
	CALENDAR_ATTENDEE_ROLE_OPT_PARTICIPANT,			/**< Accepted status */
	CALENDAR_ATTENDEE_ROLE_NON_PARTICIPANT,			/**< Non-Participant */
	CALENDAR_ATTENDEE_ROLE_CHAIR,					/**< Chairperson */
	CALENDAR_ATTENDEE_ROLE_MAX,
}calendar_attendee_role_e;

/**
 * @brief  Alarm time unit type of event such as minutes, hours, days, or etc.
 */
typedef enum
{
    CALENDAR_ALARM_NONE = -1,                  /**< No reminder set */
    CALENDAR_ALARM_TIME_UNIT_SPECIFIC = 1,     /**< specific in sec */
    CALENDAR_ALARM_TIME_UNIT_MINUTE = 60,      /**< Alarm time unit in minutes */
    CALENDAR_ALARM_TIME_UNIT_HOUR = 3600,      /**< Alarm time unit in hours */
    CALENDAR_ALARM_TIME_UNIT_DAY = 86400,      /**< Alarm time unit in days */
    CALENDAR_ALARM_TIME_UNIT_WEEK = 604800,    /**< Alarm time unit in weeks */
    CALENDAR_ALARM_TIME_UNIT_MONTH = 18144000, /**< Alarm time unit in months */
} calendar_alarm_time_unit_type_e;

/**
 * @brief Enumerations of the frequency of event recurrence.
 */
typedef enum
{
    CALENDAR_RECURRENCE_NONE,           /**< No recurrence event */
    CALENDAR_RECURRENCE_DAILY,          /**< A Event occurs every day */
    CALENDAR_RECURRENCE_WEEKLY,         /**< A Event occurs on the same day of every week \n According to week flag, the event will recurrence every days of week */
    CALENDAR_RECURRENCE_MONTHLY,        /**< A Event occurs on the same day of every month */
    CALENDAR_RECURRENCE_YEARLY         /**< A Event occurs on the same day of every year */
} calendar_recurrence_frequency_e;

/**
 * @brief Enumerations of status for event.
 */
typedef enum
{
    CALENDAR_EVENT_STATUS_NONE		= 0x01,		/**< None */
    CALENDAR_EVENT_STATUS_TENTATIVE	= 0x02,		/**< The event is tentative */
    CALENDAR_EVENT_STATUS_CONFIRMED	= 0x04,		/**< The event is confirmed */
    CALENDAR_EVENT_STATUS_CANCELLED	= 0x08		/**< The event is cancelled */
}calendar_event_status_e;

/**
 * @brief Enumerations of busy status for event.
 */
typedef enum
{
    CALENDAR_EVENT_BUSY_STATUS_FREE = 0,		/**< The free status */
    CALENDAR_EVENT_BUSY_STATUS_BUSY,			/**< The busy status */
    CALENDAR_EVENT_BUSY_STATUS_UNAVAILABLE,		/**< The unavailable status */
    CALENDAR_EVENT_BUSY_STATUS_TENTATIVE		/**< The tentative status */
}calendar_event_busy_status_e;
/**
 * @brief Calendar event item priority
 */
typedef enum
{
    CALENDAR_EVENT_PRIORITY_LOW          = 0,	/**< Low priority */
    CALENDAR_EVENT_PRIORITY_NORMAL,				/**< Normal priority */
    CALENDAR_EVENT_PRIORITY_HIGH				/**< High priority */
} calendar_event_priority_e;

/**
 * @brief Calendar to-do item priority
 */
typedef enum
{
	CALENDAR_TODO_PRIORITY_NONE			= 0x01, /**< Priority none */
    CALENDAR_TODO_PRIORITY_LOW          = 0x08,	/**< Low priority */
    CALENDAR_TODO_PRIORITY_NORMAL		= 0x04,	/**< Normal priority */
    CALENDAR_TODO_PRIORITY_HIGH			= 0x02,	/**< High priority */
} calendar_todo_priority_e;

/**
 * @brief Enumerations of status for to-do.
 */
typedef enum
{
    CALENDAR_TODO_STATUS_NONE			= 0x0100,	/**< None */
    CALENDAR_TODO_STATUS_NEEDS_ACTION	= 0x0200,	/**< Needs action status */
    CALENDAR_TODO_STATUS_COMPLETED		= 0x0400,	/**< Completed status */
    CALENDAR_TODO_STATUS_IN_PROCESS		= 0x0800,	/**< Work in process status */
    CALENDAR_TODO_STATUS_CANCELED		= 0x1000	/**< Canceled status */
} calendar_todo_status_e;

typedef enum
{
	CALENDAR_TIME_UTIME = 0,			/**< . */
	CALENDAR_TIME_LOCALTIME,			/**< . */
} calendar_time_type_e;

typedef enum
{
	CALENDAR_RANGE_UNTIL,		/**< . */
	CALENDAR_RANGE_COUNT,		/**< . */
	CALENDAR_RANGE_NONE,		/**< . */
} calendar_range_type_e;

typedef enum
{
    CALENDAR_SYSTEM_NONE,					/**< . */
    CALENDAR_SYSTEM_GREGORIAN,				/**< . */
    CALENDAR_SYSTEM_EAST_ASIAN_LUNISOLAR,	/**< . */
} calendar_system_type_e;

typedef enum
{
    CALENDAR_MEETING_STATUS_NOTMEETING = 0,     /**< . */
    CALENDAR_MEETING_STATUS_MEETING,            /**< . */
    CALENDAR_MEETING_STATUS_RECEIVED,           /**< . */
    CALENDAR_MEETING_STATUS_CANCELED,           /**< . */
} calendar_meeting_status_e;

/**
 * @brief Enumerations of weekday of month(Same value as UCalendarDaysOfWeek in ICU).
 */
typedef enum
{
	CALENDAR_SUNDAY = 1,
	CALENDAR_MONDAY,
	CALENDAR_TUESDAY,
	CALENDAR_WEDNESDAY,
	CALENDAR_THURSDAY,
	CALENDAR_FRIDAY,
	CALENDAR_SATURDAY,
}calendar_days_of_week_e;

// deprecated
#define CALENDAR_EVENT_MODIFIED_STATUS_INSERTED 0
#define CALENDAR_EVENT_MODIFIED_STATUS_UPDATED  1
#define CALENDAR_EVENT_MODIFIED_STATUS_DELETED  2
/**
 * @brief Enumerations of modified status for record.
 */
typedef enum
{
    CALENDAR_RECORD_MODIFIED_STATUS_INSERTED = 0,    /**< The record is inserted */
    CALENDAR_RECORD_MODIFIED_STATUS_UPDATED,   		/**< The record is updated */
    CALENDAR_RECORD_MODIFIED_STATUS_DELETED          /**< The record is deleted */
}calendar_record_modified_status_e;

/**
 * @brief The structure of time
 */
typedef struct
{
    calendar_time_type_e type;
    union {
        long long int utime;
        struct {
            int year;
            int month;
            int mday;
        }date;
    }time;
}calendar_time_s;

/**
 * @brief Enumerations of type for record.
 */
typedef enum
{
    CALENDAR_RECORD_TYPE_NONE = 0,          /**< . */
    CALENDAR_RECORD_TYPE_CALENDAR_BOOK,     /**< . */
    CALENDAR_RECORD_TYPE_EVENT,             /**< . */
    CALENDAR_RECORD_TYPE_TODO,              /**< . */
}calendar_record_type_e;

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif //__TIZEN_SOCIAL_CALENDAR_TYPES_H__
