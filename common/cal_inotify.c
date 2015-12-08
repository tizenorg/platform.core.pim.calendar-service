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

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/inotify.h>

#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_view.h"
#include "cal_inotify.h"
#include "cal_utils.h"

#ifdef CAL_IPC_CLIENT
#include "cal_mutex.h"
#endif

typedef struct {
	int wd;
	calendar_db_changed_cb cb;
	void *cb_data;
	cal_noti_type_e noti_type;
	bool blocked;
} noti_info_s;

static int _inoti_fd = -1;
static guint inoti_handler;
static GSList *_noti_list;

#ifdef CAL_IPC_CLIENT
static int calendar_inoti_count = 0;
#endif

static inline void _handle_callback(GSList *_noti_list, int wd, uint32_t mask)
{
	GSList *cursor;

	cursor = _noti_list;
	while (cursor) {
		noti_info_s *noti = NULL;
		noti = (noti_info_s *)cursor->data;
		if (noti->wd == wd) {
			if ((mask & IN_CLOSE_WRITE) && noti->cb) {
				switch (noti->noti_type) {
				case CAL_NOTI_TYPE_CALENDAR:
					noti->cb(CALENDAR_VIEW_CALENDAR, noti->cb_data);
					break;
				case CAL_NOTI_TYPE_EVENT:
					noti->cb(CALENDAR_VIEW_EVENT, noti->cb_data);
					break;
				case CAL_NOTI_TYPE_TODO:
					noti->cb(CALENDAR_VIEW_TODO, noti->cb_data);
					break;
				default:
					break;
				}
			}
		}
		cursor = cursor->next;
	}
}

static gboolean _inotify_gio_cb(GIOChannel *src, GIOCondition cond, gpointer data)
{
	int fd, ret;
	struct inotify_event ie;
	char name[FILENAME_MAX] = {0};

	fd = g_io_channel_unix_get_fd(src);

	while (0 < (ret = read(fd, &ie, sizeof(ie)))) {
		if (sizeof(ie) == ret) {
			if (_noti_list)
				_handle_callback(_noti_list, ie.wd, ie.mask);

			while (0 != ie.len) {
				ret = read(fd, name, (ie.len < sizeof(name)) ? ie.len : sizeof(name));
				if (-1 == ret) {
					if (EINTR == errno)
						continue;
					else
						return TRUE;
				}
				if (ie.len < ret)
					ie.len = 0;
				else
					ie.len -= ret;
			}
		} else {
			while (ret < sizeof(ie)) {
				int read_size;
				read_size = read(fd, name, sizeof(ie)-ret);
				if (-1 == read_size) {
					if (EINTR == errno)
						continue;
					else
						return TRUE;
				}
				ret += read_size;
			}
		}
	}
	return TRUE;
}

static inline int _inotify_attach_handler(int fd)
{
	guint ret;
	GIOChannel *channel;

	if (fd < 0) {
		ERR("Invalid argument: fd is NULL");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	channel = g_io_channel_unix_new(fd);
	if (NULL == channel) {
		ERR("g_io_channel_unix_new() Fail");
		return -1; /* CALENDAR_ERROR_FAILED_INOTIFY */
	}

	g_io_channel_set_flags(channel, G_IO_FLAG_NONBLOCK, NULL);

	ret = g_io_add_watch(channel, G_IO_IN, _inotify_gio_cb, NULL);
	g_io_channel_unref(channel);

	return ret;
}

int cal_inotify_init(void)
{
	int ret;

#ifdef CAL_IPC_CLIENT
	cal_mutex_lock(CAL_MUTEX_INOTIFY);
	calendar_inoti_count++;

	if (1 < calendar_inoti_count) {
		DBG("inotify count =%d", calendar_inoti_count);
		cal_mutex_unlock(CAL_MUTEX_INOTIFY);
		return CALENDAR_ERROR_NONE;
	}
	DBG("inotify count =%d", calendar_inoti_count);
	cal_mutex_unlock(CAL_MUTEX_INOTIFY);
#endif
	_inoti_fd = inotify_init();
	if (_inoti_fd == -1) {
		ERR("inotify_init() Fail(%d)", errno);
#ifdef CAL_IPC_CLIENT
		cal_mutex_lock(CAL_MUTEX_INOTIFY);
		calendar_inoti_count = 0;
		cal_mutex_unlock(CAL_MUTEX_INOTIFY);
#endif
		return -1; /* CALENDAR_ERROR_FAILED_INOTIFY */
	}

	ret = fcntl(_inoti_fd, F_SETFD, FD_CLOEXEC);
	WARN_IF(ret < 0, "fcntl failed(%d)", ret);
	ret = fcntl(_inoti_fd, F_SETFL, O_NONBLOCK);
	WARN_IF(ret < 0, "fcntl failed(%d)", ret);

	inoti_handler = _inotify_attach_handler(_inoti_fd);
	if (inoti_handler <= 0) {
		ERR("_inotify_attach_handler() Fail");
		close(_inoti_fd);
		_inoti_fd = -1;
		inoti_handler = 0;
#ifdef CAL_IPC_CLIENT
		cal_mutex_lock(CAL_MUTEX_INOTIFY);
		calendar_inoti_count = 0;
		cal_mutex_unlock(CAL_MUTEX_INOTIFY);
#endif
		return -1; /* CALENDAR_ERROR_FAILED_INOTIFY */
	}

	return CALENDAR_ERROR_NONE;
}

static inline int _cal_inotify_get_wd(int fd, const char *notipath)
{
	return inotify_add_watch(fd, notipath, IN_ACCESS);
}

static inline int _cal_inotify_add_watch(int fd, const char *notipath)
{
	int ret;

	ret = inotify_add_watch(fd, notipath, IN_CLOSE_WRITE);
	if (ret < 0) {
		ERR("Failed to add watch(ret:%d)", ret);
		return -1; /* CALENDAR_ERROR_FAILED_INOTIFY */
	}

	return CALENDAR_ERROR_NONE;
}

static bool _has_noti(int wd, void *cb, void *cb_data)
{
	bool has_noti = false;
	GSList *cursor = NULL;
	cursor = _noti_list;
	while (cursor) {
		noti_info_s *info = (noti_info_s *)cursor->data;
		if (NULL == info) {
			ERR("No info");
			cursor = g_slist_next(cursor);
			continue;
		}

		if (info->wd == wd && info->cb == cb && info->cb_data == cb_data)
			has_noti = true;

		cursor = g_slist_next(cursor);
	}
	return has_noti;
}

static int _append_noti(int wd, int type, void *cb, void *cb_data)
{
	noti_info_s *info = NULL;
	info = calloc(1, sizeof(noti_info_s));
	if (NULL == info) {
		ERR("calloc() Fail");
		return CALENDAR_ERROR_OUT_OF_MEMORY;
	}

	info->wd = wd;
	info->noti_type = type;
	info->cb_data = cb_data;
	info->cb = cb;
	info->blocked = false;

	_noti_list = g_slist_append(_noti_list, info);

	return CALENDAR_ERROR_NONE;
}

int cal_inotify_subscribe(cal_noti_type_e type, const char *path, void *cb, void *cb_data)
{
	int ret, wd;

	RETV_IF(NULL == path, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(_inoti_fd < 0, CALENDAR_ERROR_INVALID_PARAMETER, "_inoti_fd(%d) is invalid", _inoti_fd);

	wd = _cal_inotify_get_wd(_inoti_fd, path);
	if (wd == -1) {
		ERR("_cal_inotify_get_wd() Fail(%d)", errno);
		if (errno == EACCES)
			return CALENDAR_ERROR_PERMISSION_DENIED;
		return CALENDAR_ERROR_SYSTEM;
	}

	if (true == _has_noti(wd, cb, cb_data)) {
		ERR("noti is already registered: path[%s]", path);
		_cal_inotify_add_watch(_inoti_fd, path);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	ret = _cal_inotify_add_watch(_inoti_fd, path);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("_cal_inotify_add_watch() Fail(%d)", ret);
		return ret;
	}
	_append_noti(wd, type, cb, cb_data);

	return CALENDAR_ERROR_NONE;
}

static int _cal_del_noti(GSList **_noti_list, int wd, void *cb, void *cb_data)
{
	int del_cnt, remain_cnt;
	GSList *cursor, *result;

	del_cnt = 0;
	remain_cnt = 0;

	cursor = result = *_noti_list;
	while (cursor) {
		noti_info_s *noti = cursor->data;
		if (noti && wd == noti->wd) {
			if (cb == noti->cb && cb_data == noti->cb_data) {
				cursor = g_slist_next(cursor);
				result = g_slist_remove(result , noti);
				free(noti);
				del_cnt++;
				continue;
			} else {
				remain_cnt++;
			}
		}
		cursor = g_slist_next(cursor);
	}

	if (del_cnt == 0) {
		ERR("Nothing to delete");
		return CALENDAR_ERROR_NO_DATA;
	}
	*_noti_list = result;

	return remain_cnt;
}

int cal_inotify_unsubscribe(const char *path, void *cb, void *cb_data)
{
	int ret, wd;

	RETV_IF(NULL == path, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(_inoti_fd < 0, CALENDAR_ERROR_SYSTEM, "System : _inoti_fd(%d) is invalid", _inoti_fd);

	wd = _cal_inotify_get_wd(_inoti_fd, path);
	if (wd == -1) {
		ERR("_cal_inotify_get_wd() Fail(%d)", errno);
		if (errno == EACCES)
			return CALENDAR_ERROR_PERMISSION_DENIED;
		return CALENDAR_ERROR_SYSTEM;
	}

	ret = _cal_del_noti(&_noti_list, wd, cb, cb_data);
	WARN_IF(ret < CALENDAR_ERROR_NONE, "_cal_del_noti() Fail(%d)", ret);

	if (CALENDAR_ERROR_NONE != ret) {
		ret = _cal_inotify_add_watch(_inoti_fd, path);
		return ret;
	}

	return inotify_rm_watch(_inoti_fd, wd);
}

static inline gboolean _cal_inotify_detach_handler(guint id)
{
	return g_source_remove(id);
}

static void __clear_nslot_list(gpointer data, gpointer user_data)
{
	noti_info_s *noti = (noti_info_s *)data;
	free(noti);
}

void cal_inotify_deinit(void)
{
#ifdef CAL_IPC_CLIENT
	cal_mutex_lock(CAL_MUTEX_INOTIFY);
	calendar_inoti_count--;

	if (0 < calendar_inoti_count) {
		DBG("inotify count =%d", calendar_inoti_count);
		cal_mutex_unlock(CAL_MUTEX_INOTIFY);
		return ;
	}
	DBG("inotify count =%d", calendar_inoti_count);
	cal_mutex_unlock(CAL_MUTEX_INOTIFY);
#endif
	if (inoti_handler) {
		_cal_inotify_detach_handler(inoti_handler);
		inoti_handler = 0;
	}

	if (_noti_list)	{
		g_slist_foreach(_noti_list, __clear_nslot_list, NULL);
		g_slist_free(_noti_list);
		_noti_list = NULL;
	}

	if (0 <= _inoti_fd) {
		close(_inoti_fd);
		_inoti_fd = -1;
	}
}
