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
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/inotify.h>

#include "cals-internal.h"
#include "cals-typedef.h"

typedef struct
{
	int wd;
	void (*cb)(void *);
	void *cb_data;
}noti_info;

static int inoti_fd = -1;
static guint inoti_handler;
static GSList *noti_list;

static inline void _handle_callback(GSList *noti_list, int wd, uint32_t mask)
{
	noti_info *noti;
	GSList *it = NULL;

	for (it = noti_list;it;it=it->next)
	{
		noti = (noti_info *)it->data;
		if (noti->wd == wd) {
			if ((mask & IN_CLOSE_WRITE) && noti->cb)
				noti->cb(noti->cb_data);
		}
	}
}

static gboolean _inotify_gio_cb(GIOChannel *src, GIOCondition cond, gpointer data)
{
	int fd, ret;
	struct inotify_event ie;
	char name[FILENAME_MAX];

	fd = g_io_channel_unix_get_fd(src);

	while (0 < (ret = read(fd, &ie, sizeof(ie)))) {
		if (sizeof(ie) == ret) {
			if (noti_list)
				_handle_callback(noti_list, ie.wd, ie.mask);

			while (0 < ie.len) {
				ret = read(fd, name, (ie.len<sizeof(name))?ie.len:sizeof(name));
				if (-1 == ret) {
					if (EINTR == errno)
						continue;
					else
						break;
				}
				ie.len -= ret;
			}
		}else {
			while (ret < sizeof(ie)) {
				int read_size;
				read_size = read(fd, name, sizeof(ie)-ret);
				if (-1 == read_size) {
					if (EINTR == errno)
						continue;
					else
						break;
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

	retvm_if(fd < 0, CAL_ERR_ARG_INVALID, "fd is invalid");

	channel = g_io_channel_unix_new(fd);
	retvm_if(NULL == channel, CAL_ERR_INOTIFY_FAILED, "g_io_channel_unix_new() Failed");

	g_io_channel_set_flags(channel, G_IO_FLAG_NONBLOCK, NULL);

	ret = g_io_add_watch(channel, G_IO_IN, _inotify_gio_cb, NULL);
	g_io_channel_unref(channel);

	return ret;
}

int cals_inotify_init(void)
{
	inoti_fd = inotify_init();
	retvm_if(-1 == inoti_fd, CAL_ERR_INOTIFY_FAILED,
			"inotify_init() Failed(%d)", errno);

	fcntl(inoti_fd, F_SETFD, FD_CLOEXEC);
	fcntl(inoti_fd, F_SETFL, O_NONBLOCK);

	inoti_handler = _inotify_attach_handler(inoti_fd);
	if (inoti_handler <= 0) {
		ERR("_inotify_attach_handler() Failed");
		close(inoti_fd);
		inoti_fd = -1;
		inoti_handler = 0;
		return CAL_ERR_INOTIFY_FAILED;
	}

	return CAL_SUCCESS;
}

static inline int _inotify_get_wd(int fd, const char *notipath)
{
	return inotify_add_watch(fd, notipath, IN_ACCESS);
}

static inline int _inotify_watch(int fd, const char *notipath)
{
	int ret;

	ret = inotify_add_watch(fd, notipath, IN_CLOSE_WRITE);
	retvm_if(-1 == ret, CAL_ERR_INOTIFY_FAILED,
			"inotify_add_watch() Failed(%d)", errno);

	return CAL_SUCCESS;
}

int cals_inotify_subscribe(const char *path, void (*cb)(void *), void *data)
{
	int ret, wd;
	noti_info *noti, *same_noti = NULL;
	GSList *it;

	retv_if(NULL==path, CAL_ERR_ARG_NULL);
	retv_if(NULL==cb, CAL_ERR_ARG_NULL);
	retvm_if(inoti_fd < 0, CAL_ERR_ENV_INVALID,
			"inoti_fd(%d) is invalid", inoti_fd);

	wd = _inotify_get_wd(inoti_fd, path);
	retvm_if(-1 == wd, CAL_ERR_INOTIFY_FAILED,
			"_inotify_get_wd() Failed(%d)", errno);

	for (it=noti_list;it;it=it->next)
	{
		if (it->data)
		{
			same_noti = it->data;
			if (same_noti->wd == wd && same_noti->cb == cb && same_noti->cb_data == data) {
				break;
			}
			else {
				same_noti = NULL;
			}
		}
	}

	if (same_noti) {
		_inotify_watch(inoti_fd, path);
		ERR("The same callback(%s) is already exist", path);
		return CAL_ERR_ALREADY_EXIST;
	}

	ret = _inotify_watch(inoti_fd, path);
	retvm_if(CAL_SUCCESS != ret, ret, "_inotify_watch() Failed");

	noti = calloc(1, sizeof(noti_info));
	retvm_if(NULL == noti, CAL_ERR_OUT_OF_MEMORY, "calloc() Failed");

	noti->wd = wd;
	noti->cb_data = data;
	noti->cb = cb;
	noti_list = g_slist_append(noti_list, noti);

	return CAL_SUCCESS;
}

static inline int _del_noti_with_data(GSList **noti_list, int wd,
		void (*cb)(void *), void *user_data)
{
	int del_cnt, remain_cnt;
	GSList *it, *result;

	del_cnt = 0;
	remain_cnt = 0;

	it = result = *noti_list;
	while (it)
	{
		noti_info *noti = it->data;
		if (noti && wd == noti->wd)
		{
			if (cb == noti->cb && user_data == noti->cb_data) {
				it = it->next;
				result = g_slist_remove(result , noti);
				free(noti);
				del_cnt++;
				continue;
			}
			else {
				remain_cnt++;
			}
		}
		it = it->next;
	}
	retvm_if(del_cnt == 0, CAL_ERR_NO_DATA, "nothing deleted");

	*noti_list = result;

	return remain_cnt;
}

static inline int _del_noti(GSList **noti_list, int wd, void (*cb)(void *))
{
	int del_cnt, remain_cnt;
	GSList *it, *result;

	del_cnt = 0;
	remain_cnt = 0;

	it = result = *noti_list;
	while (it)
	{
		noti_info *noti = it->data;
		if (noti && wd == noti->wd)
		{
			if (NULL == cb || noti->cb == cb) {
				it = it->next;
				result = g_slist_remove(result, noti);
				free(noti);
				del_cnt++;
				continue;
			}
			else {
				remain_cnt++;
			}
		}
		it = it->next;
	}
	retvm_if(del_cnt == 0, CAL_ERR_NO_DATA, "nothing deleted");

	*noti_list = result;

	return remain_cnt;
}

int cals_inotify_unsubscribe(const char *path, void (*cb)(void *))
{
	int ret, wd;

	retv_if(NULL == path, CAL_ERR_ARG_NULL);
	retvm_if(inoti_fd < 0, CAL_ERR_ENV_INVALID,
			"inoti_fd(%d) is invalid", inoti_fd);

	wd = _inotify_get_wd(inoti_fd, path);
	retvm_if(-1 == wd, CAL_ERR_INOTIFY_FAILED,
			"_inotify_get_wd() Failed(%d)", errno);

	ret = _del_noti(&noti_list, wd, cb);
	warn_if(ret < CAL_SUCCESS, "_del_noti() Failed(%d)", ret);

	if (0 == ret)
		return inotify_rm_watch(inoti_fd, wd);

	return _inotify_watch(inoti_fd, path);
}

int cals_inotify_unsubscribe_with_data(const char *path,
		void (*cb)(void *), void *user_data)
{
	int ret, wd;

	retv_if(NULL==path, CAL_ERR_ARG_NULL);
	retv_if(NULL==cb, CAL_ERR_ARG_NULL);
	retvm_if(inoti_fd < 0, CAL_ERR_ENV_INVALID,
			"inoti_fd(%d) is invalid", inoti_fd);

	wd = _inotify_get_wd(inoti_fd, path);
	retvm_if(-1 == wd, CAL_ERR_INOTIFY_FAILED,
			"_inotify_get_wd() Failed(%d)", errno);

	ret = _del_noti_with_data(&noti_list, wd, cb, user_data);
	warn_if(ret < CAL_SUCCESS, "_del_noti_with_data() Failed(%d)", ret);

	if (0 == ret)
		return inotify_rm_watch(inoti_fd, wd);

	return _inotify_watch(inoti_fd, path);
}

static void _clear_nslot_list(gpointer data, gpointer user_data)
{
	free(data);
}

static inline gboolean _inotify_detach_handler(guint id)
{
	return g_source_remove(id);
}

void cals_inotify_close(void)
{
	if (inoti_handler) {
		_inotify_detach_handler(inoti_handler);
		inoti_handler = 0;
	}

	if (noti_list) {
		g_slist_foreach(noti_list, _clear_nslot_list, NULL);
		g_slist_free(noti_list);
		noti_list = NULL;
	}

	if (0 <= inoti_fd) {
		close(inoti_fd);
		inoti_fd = -1;
	}
}
