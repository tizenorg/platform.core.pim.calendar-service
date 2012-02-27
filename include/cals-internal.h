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
#ifndef __CALENDAR_SVC_INTERNAL_H__
#define __CALENDAR_SVC_INTERNAL_H__

#include <stdio.h>
#include <string.h>
#include <alarm.h>

#define EVENT_UPDATE "calendar_event_update"
#define CALENDARBOOK_UPDATE "calendar_book_update"
#define TASK_UPDATE "task_update"

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#define SAFE_STRDUP(src) (src)?strdup((char *)src):NULL

#define CALS_DEBUGGING

#define LOG_TAG "CALENDAR_SVC"
#include <dlog.h>
#define DLOG(prio, fmt, arg...) \
  do { SLOG(prio, LOG_TAG, fmt, ##arg); } while(0);
#define INFO(fmt, arg...) SLOGI(fmt, ##arg)
#define ERR(fmt, arg...) SLOGE("%s:%d " fmt, __FUNCTION__, __LINE__, ##arg)
#define DBG(fmt, arg...) SLOGD("%s:" fmt, __FUNCTION__, ##arg)

//#define CALS_DEBUGGING
#ifdef CALS_DEBUGGING
#define CALS_FN_CALL DBG(">>>>>>>>%s called", __FUNCTION__)
#define CALS_FN_END DBG("<<<<<<<<%s ended", __FUNCTION__)
#define CALS_DBG(fmt, arg...) DBG("%d " fmt, __LINE__, ##arg)
#else /* CALS_DEBUGGING */
#define CALS_FN_CALL
#define CALS_FN_END
#define CALS_DBG(fmt, arg...)
#endif /* CALS_DEBUGGING */


#define TMDUMP(_X_)\
{\
	ERR("tm(%d/%d/%d %d:%d)",(_X_).tm_year+1900,(_X_).tm_mon+1,(_X_).tm_mday,(_X_).tm_hour,(_X_).tm_min);\
}

#define warn_if(expr, fmt, arg...) do { \
	if (expr) { \
		ERR(fmt, ##arg); \
	} \
} while (0)
#define ret_if(expr) do { \
	if (expr) { \
		ERR("(%s)", #expr); \
		return; \
	} \
} while (0)
#define retv_if(expr, val) do { \
	if (expr) { \
		ERR("(%s)", #expr); \
		return (val); \
	} \
} while (0)
#define retm_if(expr, fmt, arg...) do { \
	if (expr) { \
		ERR(fmt, ##arg); \
		return; \
	} \
} while (0)
#define retvm_if(expr, val, fmt, arg...) do { \
	if (expr) { \
		ERR(fmt, ##arg); \
		return (val); \
	} \
} while (0)

#define retex_if(expr, val, fmt, arg...) do { \
	  if(expr) { \
		 ERR(fmt, ##arg); \
		 val; \
		 goto CATCH; \
	  } \
	} while (0);

#define CAL_PROFILE
#ifdef CAL_PROFILE
#define CAL_PROFILE_GET_TIME() (clock() / (CLOCKS_PER_SEC / 1000));
#define CAL_PROFILE_PRINT(starttime,endtime) ERR("%ld ~ %ld : %ld(%d sec) msec",starttime,endtime,endtime-starttime,(endtime-starttime)/1000);
#else
#define CAL_PROFILE_GET_TIME(input_time) 0
#define CAL_PROFILE_PRINT(starttime,endtime)
#endif

#define CAL_FREE(ptr) \
 do { \
  free(ptr); \
  ptr = NULL; \
 }while(0);

#endif /* __CALENDAR_SVC_INTERNAL_H__ */

