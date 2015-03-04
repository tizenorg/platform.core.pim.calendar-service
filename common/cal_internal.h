/*
 * Calendar Service
 *
 * Copyright (c) 2012 - 2013 Samsung Electronics Co., Ltd. All rights reserved.
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

#define COLOR_GREEN "\033[0;32m"
#define COLOR_END		"\033[0;m"

#define ENTER() SLOGD(COLOR_GREEN"BEGIN >>>>"COLOR_END)
#define LEAVE() SLOGD(COLOR_GREEN"END <<<<"COLOR_END)

#define DLOG(prio, fmt, arg...) \
	do { SLOG(prio, LOG_TAG, fmt, ##arg); } while(0);
#define INFO(fmt, arg...) SLOGI(fmt, ##arg)
#define WARN(fmt, arg...) SLOGW(fmt, ##arg)
#define ERR(fmt, arg...) SLOGE(fmt, ##arg)
#define DBG(fmt, arg...) SLOGD(fmt, ##arg)
#define SEC_INFO(fmt, arg...) SECURE_LOGI(fmt, ##arg)
#define SEC_ERR(fmt, arg...) SECURE_LOGE(fmt, ##arg)
#define SEC_DBG(fmt, arg...) SECURE_LOGD(fmt, ##arg)

//#define CAL_DEBUGGING
#ifdef CAL_DEBUGGING
#if defined(CAL_IPC_SERVER)
#define CAL_FN_CALL DBG("SERVER:>>>>>>>>%s called", __FUNCTION__)
#define CAL_FN_END DBG("SERVER:<<<<<<<<%s ended", __FUNCTION__)
#elif defined(CAL_IPC_CLIENT)
#define CAL_FN_CALL DBG("CLIENT:>>>>>>>>%s called", __FUNCTION__)
#define CAL_FN_END DBG("CLIENT:<<<<<<<<%s ended", __FUNCTION__)
#else
#define CAL_FN_CALL DBG(">>>>>>>>%s called", __FUNCTION__)
#define CAL_FN_END DBG("<<<<<<<<%s ended", __FUNCTION__)
#endif

#if defined(CAL_IPC_SERVER)
#define CAL_DBG(fmt, arg...) DBG("SERVER:%d " fmt, __LINE__, ##arg)
#elif defined(CAL_IPC_CLIENT)
#define CAL_DBG(fmt, arg...) DBG("CLIENT:%d " fmt, __LINE__, ##arg)
#else
#define CAL_DBG(fmt, arg...) DBG("%d " fmt, __LINE__, ##arg)
#endif
#else /* CAL_DEBUGGING */
#define CAL_FN_CALL
#define CAL_FN_END
#define CAL_DBG(fmt, arg...)
#endif /* CAL_DEBUGGING */

#define warn_if(expr, fmt, arg...) do { \
	if (expr) { \
		WARN(fmt, ##arg); \
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
		if (ptr) \
		free(ptr); \
		ptr = NULL; \
	} while(0)


// Thread-local storage
#if defined(CAL_IPC_SERVER)
#define TLS __thread
#elif defined(CAL_IPC_CLIENT)
#define TLS __thread
#else   //CAL_NATIVE
#define TLS
#endif

#endif /* __CALENDAR_SVC_INTERNAL_H__ */

