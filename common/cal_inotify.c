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
#include "cal_client_ipc.h"
#include "cal_mutex.h"
#endif

typedef struct {
	int wd;
	calendar_db_changed_cb callback;
	void *cb_data;
	cal_noti_type_e noti_type;
	bool blocked;
} noti_info;

typedef struct {
	int wd;
	int subscribe_count;
	void (*cb)(void *);
	void *cb_data;
} socket_init_noti_info_s;

static GHashTable *_cal_socket_init_noti_table = NULL;

static int inoti_fd = -1;
static guint inoti_handler;
static GSList *noti_list;

#ifdef CAL_IPC_CLIENT

static int calendar_inoti_count = 0;

void cal_inotify_call_pending_callback(void)
{
	noti_info *noti;
	GSList *cursor = NULL;

	cursor = noti_list;
	while (cursor) {
		noti = (noti_info *)cursor->data;
		if (noti->callback && noti->blocked) {
			noti->blocked = false;
			switch (noti->noti_type) {
			case CAL_NOTI_TYPE_CALENDAR:
				noti->callback(CALENDAR_VIEW_CALENDAR, noti->cb_data);
				break;
			case CAL_NOTI_TYPE_EVENT:
				noti->callback(CALENDAR_VIEW_EVENT, noti->cb_data);
				break;
			case CAL_NOTI_TYPE_TODO:
				noti->callback(CALENDAR_VIEW_TODO, noti->cb_data);
				break;
			default:
				break;
			}
		}
		cursor = cursor->next;
	}
}
#endif

static inline void _handle_callback(GSList *noti_list, int wd, uint32_t mask)
{
	noti_info *noti;
	GSList *cursor;

	cursor = noti_list;
	while (cursor)
	{
		noti = (noti_info *)cursor->data;
		if (noti->wd == wd) {
#ifdef CAL_IPC_CLIENT
			if (cal_client_ipc_is_call_inprogress()) {
				noti->blocked = true;
				continue;
			}
#endif

			if ((mask & IN_CLOSE_WRITE) && noti->callback) {
				switch(noti->noti_type) {
				case CAL_NOTI_TYPE_CALENDAR:
					noti->callback(CALENDAR_VIEW_CALENDAR, noti->cb_data);
					break;
				case CAL_NOTI_TYPE_EVENT:
					noti->callback(CALENDAR_VIEW_EVENT, noti->cb_data);
					break;
				case CAL_NOTI_TYPE_TODO:
					noti->callback(CALENDAR_VIEW_TODO, noti->cb_data);
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
			if (noti_list)
				_handle_callback(noti_list, ie.wd, ie.mask);

			while (0 != ie.len) {
				ret = read(fd, name, (ie.len<sizeof(name))?ie.len:sizeof(name));
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
		}
		else {
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

int cal_inotify_initialize(void)
{
	int ret;

#ifdef CAL_IPC_CLIENT
	cal_mutex_lock(CAL_MUTEX_INOTIFY);
	calendar_inoti_count++;

	if (1 < calendar_inoti_count) {
		DBG("inotify count =%d",calendar_inoti_count);
		cal_mutex_unlock(CAL_MUTEX_INOTIFY);
		return CALENDAR_ERROR_NONE;
	}
	DBG("inotify count =%d",calendar_inoti_count);
	cal_mutex_unlock(CAL_MUTEX_INOTIFY);
#endif
	inoti_fd = inotify_init();
	if (inoti_fd == -1) {
		ERR("inotify_init() Fail(%d)", errno);
#ifdef CAL_IPC_CLIENT
		cal_mutex_lock(CAL_MUTEX_INOTIFY);
		calendar_inoti_count = 0;
		cal_mutex_unlock(CAL_MUTEX_INOTIFY);
#endif
		return -1; /* CALENDAR_ERROR_FAILED_INOTIFY */
	}
	DBG("-----------------------------");

	ret = fcntl(inoti_fd, F_SETFD, FD_CLOEXEC);
	WARN_IF(ret < 0, "fcntl failed(%d)", ret);
	ret = fcntl(inoti_fd, F_SETFL, O_NONBLOCK);
	WARN_IF(ret < 0, "fcntl failed(%d)", ret);

	inoti_handler = _inotify_attach_handler(inoti_fd);
	if (inoti_handler <= 0) {
		ERR("_inotify_attach_handler() Fail");
		close(inoti_fd);
		inoti_fd = -1;
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
	if (ret < 0)
	{
		ERR("Failed to add watch(ret:%d)", ret);
		return -1; // CALENDAR_ERROR_FAILED_INOTIFY
	}

	return CALENDAR_ERROR_NONE;
}

int cal_inotify_subscribe_ipc_ready(calendar_h handle, void (*cb)(void *), void *user_data)
{
	int ret = 0;
	socket_init_noti_info_s *sock_info = NULL;

	if (NULL == _cal_socket_init_noti_table)
		_cal_socket_init_noti_table = g_hash_table_new_full(g_str_hash, g_str_equal, free, free);
	else
		sock_info = g_hash_table_lookup(_cal_socket_init_noti_table, CAL_NOTI_IPC_READY);

	if (NULL == sock_info) {
		int wd = _cal_inotify_get_wd(inoti_fd, CAL_NOTI_IPC_READY);
		if (-1 == wd) {
			ERR("_cal_inotify_get_wd() Fail(%d):path[%s]", errno, CAL_NOTI_IPC_READY);
			if (EACCES == errno)
				return CALENDAR_ERROR_PERMISSION_DENIED;
			return CALENDAR_ERROR_NONE;
		}
		ret = _cal_inotify_add_watch(inoti_fd, CAL_NOTI_IPC_READY);
		if (CALENDAR_ERROR_NONE != ret) {
			ERR("_cal_inotify_add_watch() Fail(%d)", ret);
			return ret;
		}
		sock_info = calloc(1, sizeof(socket_init_noti_info_s));
		if (NULL == sock_info) {
			ERR("calloc() Fail");
			return CALENDAR_ERROR_OUT_OF_MEMORY;
		}

		sock_info->wd = wd;
		sock_info->cb = cb;
		sock_info->cb_data = user_data;
		g_hash_table_insert(_cal_socket_init_noti_table, g_strdup(CAL_NOTI_IPC_READY), sock_info);
	}
	sock_info->subscribe_count++;
	return CALENDAR_ERROR_NONE;
}

int cal_inotify_unsubscribe_ipc_ready(calendar_h handle)
{
	RETV_IF(NULL == _cal_socket_init_noti_table, CALENDAR_ERROR_INVALID_PARAMETER);

	socket_init_noti_info_s *sock_info = NULL;

	sock_info = g_hash_table_lookup(_cal_socket_init_noti_table, CAL_NOTI_IPC_READY);
	if (NULL == sock_info) {
		ERR("g_hash_table_lookup() Fail");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	if (1 == sock_info->subscribe_count) {
		int wd = sock_info->wd;
		inotify_rm_watch(inoti_fd, wd);
		g_hash_table_remove(_cal_socket_init_noti_table, CAL_NOTI_IPC_READY);
	}
	else {
		sock_info->subscribe_count--;
	}
	return CALENDAR_ERROR_NONE;
}

int cal_inotify_subscribe(cal_noti_type_e type, const char *path, calendar_db_changed_cb callback, void *data)
{
	int ret, wd;
	noti_info *noti, *same_noti = NULL;
	GSList *cursor;

	RETV_IF(NULL == path, CALENDAR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == callback, CALENDAR_ERROR_INVALID_PARAMETER);
	RETVM_IF(inoti_fd < 0, CALENDAR_ERROR_INVALID_PARAMETER, "inoti_fd(%d) is invalid", inoti_fd);

	wd = _cal_inotify_get_wd(inoti_fd, path);
	if (wd == -1) {
		ERR("Failed to get wd(err:%d)", errno);
		if (errno == EACCES)
			return CALENDAR_ERROR_PERMISSION_DENIED;
		return CALENDAR_ERROR_SYSTEM;
	}

	cursor = noti_list;
	while (cursor) {
		if (cursor->data == NULL) {
			DBG("No data exist");
			cursor = cursor->next;
			continue;
		}

		same_noti = cursor->data;
		if (same_noti->wd == wd && same_noti->callback == callback && same_noti->cb_data == data) {
			break;

		}
		else {
			same_noti = NULL;
		}

		cursor = cursor->next;
	}

	if (same_noti) {
		_cal_inotify_add_watch(inoti_fd, path);
		ERR("The same callback(%s) is already exist", path);
		return CALENDAR_ERROR_SYSTEM;
	}

	ret = _cal_inotify_add_watch(inoti_fd, path);
	if (CALENDAR_ERROR_NONE != ret) {
		ERR("Failed to add watch");
		return CALENDAR_ERROR_SYSTEM;
	}

	noti = calloc(1, sizeof(noti_info));
	if (noti == NULL) {
		ERR("Failed to alloc");
		return CALENDAR_ERROR_OUT_OF_MEMORY;
	}

	noti->wd = wd;
	noti->cb_data = data;
	noti->callback = callback;
	noti->noti_type = type;
	noti->blocked = false;
	noti_list = g_slist_append(noti_list, noti);

	return CALENDAR_ERROR_NONE;
}

static inline int _cal_inotify_delete_noti_with_data(GSList **noti_list, int wd,
		calendar_db_changed_cb callback, void *user_data)
{
	int del_cnt, remain_cnt;
	GSList *cursor, *result;

	del_cnt = 0;
	remain_cnt = 0;

	cursor = result = *noti_list;
	while (cursor)
	{
		noti_info *noti = cursor->data;
		if (noti && wd == noti->wd)
		{
			if (callback == noti->callback && user_data == noti->cb_data) {
				cursor = cursor->next;
				result = g_slist_remove(result , noti);
				free(noti);
				del_cnt++;
				continue;
			}
			else
			{
				remain_cnt++;
			}
		}
		cursor = cursor->next;
	}

	if (del_cnt == 0)
	{
		ERR("Nothing to delete");
		return CALENDAR_ERROR_NO_DATA;
	}

	*noti_list = result;

	return remain_cnt;
}

static inline int _cal_notify_delete_noti(GSList **noti_list, int wd, calendar_db_changed_cb callback)
{
	int del_cnt, remain_cnt;
	GSList *cursor, *result;

	del_cnt = 0;
	remain_cnt = 0;

	cursor = result = *noti_list;
	while (cursor)
	{
		noti_info *noti = cursor->data;
		if (noti && wd == noti->wd)
		{
			if (NULL == callback || noti->callback == callback)
			{
				cursor = cursor->next;
				result = g_slist_remove(result, noti);
				free(noti);
				del_cnt++;
				continue;
			}
			else
			{
				remain_cnt++;
			}
		}
		cursor = cursor->next;
	}

	if (del_cnt == 0)
	{
		ERR("Nothing to delete");
		return CALENDAR_ERROR_NO_DATA;
	}

	*noti_list = result;

	return remain_cnt;
}

int cal_inotify_unsubscribe_with_data(const char *path, calendar_db_changed_cb callback, void *user_data)
{
	int wd;
	int ret;


	if (path == NULL)
	{
		ERR("Invalid argument: path is NULL");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}
	if (callback == NULL)
	{
		ERR("Invalid argument: callback is NULL");
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	if (inoti_fd < 0)
	{
		ERR("Invalid argument: inoti_fd(%d) is invalid", inoti_fd);
		return CALENDAR_ERROR_INVALID_PARAMETER;
	}

	wd = _cal_inotify_get_wd(inoti_fd, path);
	if (wd == -1)
	{
		ERR("Failed to get wd(err:%d)", errno);
		if (errno == EACCES)
			return CALENDAR_ERROR_PERMISSION_DENIED;
		return CALENDAR_ERROR_SYSTEM;
	}

	ret = _cal_inotify_delete_noti_with_data(&noti_list, wd, callback, user_data);
	if (CALENDAR_ERROR_NONE != ret)
	{
		WARN("Failed to delete noti(err:%d)", ret);
		return _cal_inotify_add_watch(inoti_fd, path);
	}

	ret = inotify_rm_watch(inoti_fd, wd);
	return (ret == 0) ? CALENDAR_ERROR_NONE : CALENDAR_ERROR_SYSTEM;
}

static inline gboolean _cal_inotify_detach_handler(guint id)
{
	return g_source_remove(id);
}

static void __clear_nslot_list(gpointer data, gpointer user_data)
{
	free(data);
}

void cal_inotify_finalize(void)
{
#ifdef CAL_IPC_CLIENT
	cal_mutex_lock(CAL_MUTEX_INOTIFY);
	calendar_inoti_count--;

	if (0 < calendar_inoti_count) {
		DBG("inotify count =%d",calendar_inoti_count);
		cal_mutex_unlock(CAL_MUTEX_INOTIFY);
		return ;
	}
	DBG("inotify count =%d",calendar_inoti_count);
	cal_mutex_unlock(CAL_MUTEX_INOTIFY);
#endif
	if (inoti_handler) {
		_cal_inotify_detach_handler(inoti_handler);
		inoti_handler = 0;
	}

	if (noti_list)
	{
		g_slist_foreach(noti_list, __clear_nslot_list, NULL);
		g_slist_free(noti_list);
		noti_list = NULL;
	}

	if (0 <= inoti_fd) {
		close(inoti_fd);
		inoti_fd = -1;
	}
}
