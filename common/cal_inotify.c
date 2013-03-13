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

#ifdef CAL_IPC_CLIENT
#include "cal_client_ipc.h"
#include "cal_mutex.h"
#endif

typedef struct
{
	int wd;
	calendar_db_changed_cb callback;
	void *cb_data;
	cal_noti_type_e noti_type;
	bool blocked;
}noti_info;

static int inoti_fd = -1;
static guint inoti_handler;
static GSList *noti_list;

#ifdef CAL_IPC_CLIENT

static int calendar_inoti_count = 0;

void _cal_inotify_call_pending_callback(void)
{
    noti_info *noti;
    GSList *cursor = NULL;

    cursor = noti_list;
    while (cursor)
    {
		noti = (noti_info *)cursor->data;
		if (noti->callback && noti->blocked)
		{
			noti->blocked = false;
			switch (noti->noti_type)
			{
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
        if (noti->wd == wd)
		{
#ifdef CAL_IPC_CLIENT
			if (_cal_client_ipc_is_call_inprogress())
			{
				noti->blocked = true;
				continue;
			}
#endif

            if ((mask & IN_CLOSE_WRITE) && noti->callback)
            {
                switch(noti->noti_type)
                {
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

    while (0 < (ret = read(fd, &ie, sizeof(ie))))
    {
        if (sizeof(ie) == ret)
        {
            if (noti_list)
                _handle_callback(noti_list, ie.wd, ie.mask);

            while (0 != ie.len)
            {
                ret = read(fd, name, (ie.len<sizeof(name))?ie.len:sizeof(name));
                if (-1 == ret)
                {
                    if (EINTR == errno)
                        continue;
                    else
                        return TRUE;
                }
                if (ret > ie.len)
                {
                    ie.len = 0;
                }
                else
                {
                    ie.len -= ret;
                }
            }
        }
        else
        {
            while (ret < sizeof(ie))
            {
                int read_size;
                read_size = read(fd, name, sizeof(ie)-ret);
                if (-1 == read_size)
                {
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

    if (fd < 0)
    {
        ERR("Invalid argument: fd is NULL");
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    channel = g_io_channel_unix_new(fd);
    if (channel == NULL)
    {
        ERR("Failed to new channel");
        return -1; // CALENDAR_ERROR_FAILED_INOTIFY
    }

    g_io_channel_set_flags(channel, G_IO_FLAG_NONBLOCK, NULL);

    ret = g_io_add_watch(channel, G_IO_IN, _inotify_gio_cb, NULL);
    g_io_channel_unref(channel);

    return ret;
}

int _cal_inotify_initialize(void)
{
    int ret;

#ifdef CAL_IPC_CLIENT
    _cal_mutex_lock(CAL_MUTEX_INOTIFY);
    calendar_inoti_count++;

    if (calendar_inoti_count > 1)
    {
        CAL_DBG("inotify count =%d",calendar_inoti_count);
        _cal_mutex_unlock(CAL_MUTEX_INOTIFY);
        return CALENDAR_ERROR_NONE;
    }
    CAL_DBG("inotify count =%d",calendar_inoti_count);
    _cal_mutex_unlock(CAL_MUTEX_INOTIFY);
#endif
    inoti_fd = inotify_init();
    if (inoti_fd == -1)
    {
        ERR("Failed to init inotify(err:%d)", errno);
#ifdef CAL_IPC_CLIENT
        _cal_mutex_lock(CAL_MUTEX_INOTIFY);
        calendar_inoti_count = 0;
        _cal_mutex_unlock(CAL_MUTEX_INOTIFY);
#endif
        return -1; // CALENDAR_ERROR_FAILED_INOTIFY;
    }

    ret = fcntl(inoti_fd, F_SETFD, FD_CLOEXEC);
    warn_if(ret < 0, "fcntl failed(%d)", ret);
    ret = fcntl(inoti_fd, F_SETFL, O_NONBLOCK);
    warn_if(ret < 0, "fcntl failed(%d)", ret);

    inoti_handler = _inotify_attach_handler(inoti_fd);
    if (inoti_handler <= 0)
    {
        ERR("_inotify_attach_handler() Failed");
        close(inoti_fd);
        inoti_fd = -1;
        inoti_handler = 0;
#ifdef CAL_IPC_CLIENT
        _cal_mutex_lock(CAL_MUTEX_INOTIFY);
        calendar_inoti_count = 0;
        _cal_mutex_unlock(CAL_MUTEX_INOTIFY);
#endif
        return -1; // CALENDAR_ERROR_FAILED_INOTIFY
    }

    return CALENDAR_ERROR_NONE;
}

static inline int __cal_inotify_get_wd(int fd, const char *notipath)
{
    return inotify_add_watch(fd, notipath, IN_ACCESS);
}

static inline int __cal_inotify_add_watch(int fd, const char *notipath)
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

int _cal_inotify_subscribe(cal_noti_type_e type, const char *path, calendar_db_changed_cb callback, void *data)
{
    int ret, wd;
    noti_info *noti, *same_noti = NULL;
    GSList *cursor;

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
        ERR("Invalid argument: iinoti_fd(%d) is invalid", inoti_fd);
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    wd = __cal_inotify_get_wd(inoti_fd, path);
    if (wd == -1)
    {
        ERR("Failed to get wd(err:%d)", errno);
        return -1; // CALENDAR_ERROR_FAILED_INOTIFY
    }

    cursor = noti_list;
    while (cursor)
    {
        if (cursor->data == NULL)
        {
            DBG("No data exist");
            cursor = cursor->next;
            continue;
        }

        same_noti = cursor->data;
        if (same_noti->wd == wd && same_noti->callback == callback && same_noti->cb_data == data) {
            break;

        }
        else
        {
            same_noti = NULL;
        }

        cursor = cursor->next;
    }

    if (same_noti)
    {
        __cal_inotify_add_watch(inoti_fd, path);
        ERR("The same callback(%s) is already exist", path);
        return -1; //CAL_ERR_ALREADY_EXIST;
    }

    ret = __cal_inotify_add_watch(inoti_fd, path);
    if (ret != CALENDAR_ERROR_NONE)
    {
        ERR("Failed to add watch");
        return -1; // CALENDAR_ERROR_FAILED_INOTIFY
    }

    noti = calloc(1, sizeof(noti_info));
    if (noti == NULL)
    {
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

static inline int __cal_inotify_delete_noti_with_data(GSList **noti_list, int wd,
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

static inline int __cal_notify_delete_noti(GSList **noti_list, int wd, calendar_db_changed_cb callback)
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

int _cal_inotify_unsubscribe_with_data(const char *path, calendar_db_changed_cb callback, void *user_data)
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
        ERR("Invalid argument: iinoti_fd(%d) is invalid", inoti_fd);
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }

    wd = __cal_inotify_get_wd(inoti_fd, path);
    if (wd == -1)
    {
        ERR("Failed to get wd(err:%d)", errno);
        return -1; // CALENDAR_ERROR_FAILED_INOTIFY
    }

    ret = __cal_inotify_delete_noti_with_data(&noti_list, wd, callback, user_data);
    if (ret != CALENDAR_ERROR_NONE)
    {
        WARN("Failed to delete noti(err:%d)", ret);
        return __cal_inotify_add_watch(inoti_fd, path);
    }

    return inotify_rm_watch(inoti_fd, wd);
}

static inline gboolean __cal_inotify_detach_handler(guint id)
{
    return g_source_remove(id);
}

static void __clear_nslot_list(gpointer data, gpointer user_data)
{
    free(data);
}

void _cal_inotify_finalize(void)
{
#ifdef CAL_IPC_CLIENT
    _cal_mutex_lock(CAL_MUTEX_INOTIFY);
    calendar_inoti_count--;

    if (calendar_inoti_count > 0)
    {
        CAL_DBG("inotify count =%d",calendar_inoti_count);
        _cal_mutex_unlock(CAL_MUTEX_INOTIFY);
        return ;
    }
    CAL_DBG("inotify count =%d",calendar_inoti_count);
    _cal_mutex_unlock(CAL_MUTEX_INOTIFY);
#endif
    if (inoti_handler)
    {
        __cal_inotify_detach_handler(inoti_handler);
        inoti_handler = 0;
    }

    if (noti_list)
    {
        g_slist_foreach(noti_list, __clear_nslot_list, NULL);
        g_slist_free(noti_list);
        noti_list = NULL;
    }

    if (inoti_fd >= 0)
    {
        close(inoti_fd);
        inoti_fd = -1;
    }
}
