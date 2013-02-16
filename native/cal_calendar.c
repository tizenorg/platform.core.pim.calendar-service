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

#include <unistd.h>
#include <glib.h>
#include <glib-object.h>
#include <db-util.h>

#include "cal_internal.h"
#include "cal_typedef.h"
#include "cal_db.h"
#include "cal_view.h"

#ifdef CAL_NATIVE
#include "cal_inotify.h"
#endif

static TLS int db_ref_cnt = 0;

API int calendar_connect(void)
{
	CAL_FN_CALL;
	int ret = 0;

	DBG("pthread_self=%x, db_ref_cnt=%p", pthread_self(),&db_ref_cnt );

	g_type_init();	// added for alarmmgr

	if(db_ref_cnt <= 0)
	{
		ret = _cal_db_open();
		retvm_if(ret, ret, "cal_db_open() Failed(%d)", ret);

		db_ref_cnt = 0;
#ifdef CAL_NATIVE
		_cal_inotify_initialize();
#endif
	}
	db_ref_cnt++;

	_cal_view_initialize();

    DBG("db_ref_cnt(%d)", db_ref_cnt);

	return CALENDAR_ERROR_NONE;
}

API int calendar_disconnect(void)
{
	CAL_FN_CALL;

    DBG("db_ref_cnt(%d)", db_ref_cnt);
    DBG("pthread_self=%x, db_ref_cnt=%p", pthread_self(),&db_ref_cnt );

	retvm_if(0 == db_ref_cnt, CALENDAR_ERROR_INVALID_PARAMETER,
			"Calendar service was not connected");

	if (db_ref_cnt==1) {
		_cal_db_close();

		db_ref_cnt = 0;

#ifdef CAL_NATIVE
		_cal_inotify_finalize();
#endif

		_cal_view_finalize();
		return CALENDAR_ERROR_NONE;
	}
	db_ref_cnt--;

	return CALENDAR_ERROR_NONE;
}

#ifdef CAL_NATIVE
API int calendar_db_add_changed_cb(const char* view_uri, calendar_db_changed_cb callback, void* user_data )
{
    CAL_FN_CALL;
    int ret;
    cal_record_type_e type = CAL_RECORD_TYPE_INVALID;

    retv_if(NULL == view_uri || NULL == callback , CALENDAR_ERROR_INVALID_PARAMETER);

    type = _cal_view_get_type(view_uri);

    switch(type)
    {
    case CAL_RECORD_TYPE_CALENDAR:
        ret = _cal_inotify_subscribe(CAL_NOTI_TYPE_CALENDAR, CAL_NOTI_CALENDAR_CHANGED, callback, user_data);
        break;
    case CAL_RECORD_TYPE_EVENT:
        ret = _cal_inotify_subscribe(CAL_NOTI_TYPE_EVENT, CAL_NOTI_EVENT_CHANGED, callback, user_data);
        break;
    case CAL_RECORD_TYPE_TODO:
        ret = _cal_inotify_subscribe(CAL_NOTI_TYPE_TODO, CAL_NOTI_TODO_CHANGED, callback, user_data);
        break;
    default:
        ERR("Invalid view_uri(%s)", view_uri);
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }
    retvm_if(CALENDAR_ERROR_NONE != ret, ret, "_cal_inotify_subscribe() Failed(%d)", ret);

    return CALENDAR_ERROR_NONE;
}

API int calendar_db_remove_changed_cb( const char* view_uri, calendar_db_changed_cb callback, void* user_data )
{
    CAL_FN_CALL;
    int ret;
    cal_record_type_e type = CAL_RECORD_TYPE_INVALID;

    retv_if(NULL == view_uri || NULL == callback , CALENDAR_ERROR_INVALID_PARAMETER);

    type = _cal_view_get_type(view_uri);

    switch(type)
    {
    case CAL_RECORD_TYPE_CALENDAR:
        ret = _cal_inotify_unsubscribe_with_data(CAL_NOTI_CALENDAR_CHANGED,
				callback, user_data);
        break;
    case CAL_RECORD_TYPE_EVENT:
        ret = _cal_inotify_unsubscribe_with_data(CAL_NOTI_EVENT_CHANGED,
				callback, user_data);
        break;
    case CAL_RECORD_TYPE_TODO:
        ret = _cal_inotify_unsubscribe_with_data(CAL_NOTI_TODO_CHANGED,
				callback, user_data);
        break;
    default:
        ERR("Invalid view_uri(%s)", view_uri);
        return CALENDAR_ERROR_INVALID_PARAMETER;
    }
    retvm_if(CALENDAR_ERROR_NONE != ret, ret, "_cal_inotify_unsubscribe_with_data() Failed(%d)", ret);

    return CALENDAR_ERROR_NONE;
}

API int calendar_connect_on_thread(void)
{
    CAL_FN_CALL;
    int ret = 0;

    DBG("pthread_self=%x, db_ref_cnt=%p", pthread_self(),&db_ref_cnt );

    g_type_init();  // added for alarmmgr

    if(db_ref_cnt <= 0)
    {
        ret = _cal_db_open();
        retvm_if(ret, ret, "cal_db_open() Failed(%d)", ret);

        db_ref_cnt = 0;
        _cal_inotify_initialize();

    }
    db_ref_cnt++;

    _cal_view_initialize();

    DBG("db_ref_cnt(%d)", db_ref_cnt);

    return CALENDAR_ERROR_NONE;
}

API int calendar_disconnect_on_thread(void)
{
    CAL_FN_CALL;

    DBG("db_ref_cnt(%d)", db_ref_cnt);
    DBG("pthread_self=%x, db_ref_cnt=%p", pthread_self(),&db_ref_cnt );

    retvm_if(0 == db_ref_cnt, CALENDAR_ERROR_INVALID_PARAMETER,
            "Calendar service was not connected");

    if (db_ref_cnt==1) {
        _cal_db_close();

        db_ref_cnt = 0;

        _cal_inotify_finalize();

        _cal_view_finalize();
        return CALENDAR_ERROR_NONE;
    }
    db_ref_cnt--;

    return CALENDAR_ERROR_NONE;
}

API int calendar_connect_with_flags(unsigned int flags)
{
    int ret = CALENDAR_ERROR_NONE;

    ret = calendar_connect();
    if (ret != CALENDAR_ERROR_NONE)
    {
        if (flags & CALENDAR_CONNECT_FLAG_RETRY)
        {
            int retry_time = 500;
            int i = 0;
            for(i=0;i<6;i++)
            {
                usleep(retry_time*1000);
                ret = calendar_connect();
                DBG("retry cnt=%d, ret=%x",(i+1), ret);
                if (ret == CALENDAR_ERROR_NONE)
                    break;
                retry_time *= 2;
            }

        }
    }

    return ret;
}
#endif
