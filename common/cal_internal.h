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
#ifndef __CALENDAR_SVC_INTERNAL_H__
#define __CALENDAR_SVC_INTERNAL_H__

#include <stdio.h>
#include <string.h>

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#define SAFE_STRDUP(src) (src)?strdup((char *)src):NULL

#define CAL_DEBUGGING

#define LOG_TAG "CALENDAR_SVC"
#include <dlog.h>

#define COLOR_RED    "\033[0;31m"
#define COLOR_GREEN  "\033[0;32m"
#define COLOR_BROWN  "\033[0;33m"
#define COLOR_BLUE   "\033[0;34m"
#define COLOR_PURPLE "\033[0;35m"
#define COLOR_CYAN   "\033[0;36m"
#define COLOR_END    "\033[0;m"

#if defined(CAL_IPC_SERVER)
#define IPC_ROLE COLOR_BLUE"[SERVER]"COLOR_END
#elif defined(CAL_IPC_CLIENT)
#define IPC_ROLE COLOR_BROWN"[CLIENT]"COLOR_END
#else
#define IPC_ROLE COLOR_GREEN"[LIB]"COLOR_END
#endif

#ifdef CAL_DEBUGGING
 #define INFO(fmt, arg...) SLOGI(IPC_ROLE" "fmt, ##arg)
 #define ERR(fmt, arg...) SLOGE(IPC_ROLE" "fmt, ##arg)
 #define DBG(fmt, arg...) SLOGD(IPC_ROLE" "fmt, ##arg)
 #define WARN(fmt, arg...) SLOGD(IPC_ROLE" "fmt, ##arg)
 #define VERBOSE(fmt, arg...) SLOGV(IPC_ROLE" "fmt, ##arg)
#else /* CAL_DEBUGGING */
 #define INFO(fmt, arg...)
 #define ERR(fmt, arg...)
 #define DBG(fmt, arg...)
 #define WARN(fmt, arg...)
 #define VERBOSE(fmt, arg...)
#endif /* CAL_DEBUGGING */

#define SEC_INFO(fmt, arg...) SECURE_LOGI(fmt, ##arg)
#define SEC_ERR(fmt, arg...) SECURE_LOGE(fmt, ##arg)
#define SEC_DBG(fmt, arg...) SECURE_LOGD(fmt, ##arg)
#define SECURE(fmt, arg...) SECURE_LOGD(fmt, ##arg)

#define CAL_FN_CALL() DBG(">>>>>>>> called")
#define CAL_FN_END() DBG("<<<<<<<< ended")
#define CAL_DBG(fmt, arg...) DBG(fmt, ##arg)
#define CAL_WARN(fmt, arg...) WARN(fmt, ##arg)
#define CAL_ERR(fmt, arg...) ERR(fmt, ##arg)
#define CAL_INFO(fmt, arg...) INFO(fmt, ##arg)
#define CAL_VERBOSE(fmt, arg...) VERBOSE(fmt, ##arg)

#define WARN_IF(expr, fmt, arg...) do { \
	if (expr) { \
		WARN(fmt, ##arg); \
	} \
} while (0)
#define RET_IF(expr) do { \
	if (expr) { \
		ERR("(%s)", #expr); \
		return; \
	} \
} while (0)
#define RETV_IF(expr, val) do { \
	if (expr) { \
		ERR("(%s)", #expr); \
		return (val); \
	} \
} while (0)
#define RETM_IF(expr, fmt, arg...) do { \
	if (expr) { \
		ERR(fmt, ##arg); \
		return; \
	} \
} while (0)
#define RETVM_IF(expr, val, fmt, arg...) do { \
	if (expr) { \
		ERR(fmt, ##arg); \
		return (val); \
	} \
} while (0)
#define BREAK_IF(expr, fmt, arg...) do { \
	if (expr) { \
		ERR(fmt, ##arg); \
		break; \
	} \
} while (0)

#define CAL_START_TIMESTAMP struct timeval timeval_s = {0}; \
	struct timeval timeval_e = {0}; \
	struct timeval timeval_d = {0}; \
	DBG(COLOR_PURPLE">>>>>"COLOR_END); \
	gettimeofday(&timeval_s, NULL);

#define CAL_PRINT_TIMESTAMP gettimeofday(&timeval_e, NULL); \
	timersub(&timeval_e, &timeval_s, &timeval_d); \
	timeval_s = timeval_e; \
	DBG(COLOR_PURPLE"<<<<< (%03d.%03dsec)"COLOR_END, timeval_d.tv_sec % 1000, timeval_d.tv_usec/1000);

#define CAL_PROFILE
#ifdef CAL_PROFILE
#define CAL_PROFILE_GET_TIME() (clock() / (CLOCKS_PER_SEC / 1000));
#define CAL_PROFILE_PRINT(starttime,endtime) ERR("%ld ~ %ld : %ld(%d sec) msec",starttime,endtime,endtime-starttime, (endtime-starttime)/1000);
#else
#define CAL_PROFILE_GET_TIME(input_time) 0
#define CAL_PROFILE_PRINT(starttime,endtime)
#endif

#define CAL_FREE(ptr) \
	do { \
		if (ptr) \
		free(ptr); \
		ptr = NULL; \
	} while (0)


// Thread-local storage
#if defined(CAL_IPC_SERVER)
#define TLS __thread
#elif defined(CAL_IPC_CLIENT)
#define TLS __thread
#else   //CAL_NATIVE
#define TLS
#endif

#endif /* __CALENDAR_SVC_INTERNAL_H__ */

