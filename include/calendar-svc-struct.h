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
#ifndef __CALENDAR_SVC_STRUCT_H__
#define __CALENDAR_SVC_STRUCT_H__


/**
 * @defgroup CALENDAR_SVC Calendar Service
 * @section intro 2.0 calendar engine
 * - this document maded for calendar service
 * - this document contain api signature & usage
 *
 * @section CREATEINFO author infomation
 * - author      :   heungjae jeong(hj98.jeong@samsung.com)
 * - date      	   :   2010/09/06
 *
 * @section DEPENDENCY DEPENDENCY
 * - GLIB
 *
 * @section MODIFYINFO Revision history
 * - maximus/2010-09-06   	  : make api signature
 * - maximus/2010-09-16       : add calendar type & exception date, change api name(remove _event)
 * - maximus/2010-09-16       : add utility function(for vcalendar)
 *
 *
 */


/**
 *cal_struct is an opaque type, it must be used via accessor functions.
 *@ingroup common
 *@see also:
 * calendar_svc_struct_new(), calendar_svc_struct_free()
 * calendar_svc_struct_get_value(), calendar_svc_struct_get_list(),
 * calendar_svc_struct_store_value(), calendar_svc_struct_store_list()
 */
typedef struct _cal_struct cal_struct;


/**
 * cal_value is an opaque type, it must be
 * used via accessor functions.
 * @ingroup common
 * @see calendar_svc_struct_new(), calendar_svc_struct_free()
 */
typedef struct _cal_value cal_value;



/**
 * cal_struct is an opaque type, it must be used via accessor functions.
 * @ingroup common
 * @see calendar_svc_find_event_list(), calendar_svc_event_get_changes()
 *      calendar_svc_get_event_by_period(), calendar_svc_iter_get_info(),
 *      calendar_svc_iter_next(), calendar_svc_iter_remove()
 */
typedef struct _cal_iter cal_iter;



/**
 * @defgroup example example
 * @ingroup CALENDAR_SVC
 * @li
 *      insert example
 * @code
   void insert_test(void)
   {
      cal_struct *event=NULL;
      cal_value *attendee1=NULL,*attendee2=NULL;
      GList *attendee_list=NULL;

      calendar_svc_connect();
      event = calendar_svc_struct_new(CAL_STRUCT_SCHEDULE);

      calendar_svc_struct_set_str(event, CAL_VALUE_TXT_SUMMARY, "weekly meeting");
      calendar_svc_struct_set_str(event, CAL_VALUE_TXT_DESCRIPTION, "review : project status");
      calendar_svc_struct_set_str(event, CAL_VALUE_TXT_LOCATION, "meeting room #1");

      attendee1 = calendar_svc_value_new(CAL_VALUE_LST_ATTENDEE_LIST);
      if(attendee1) {
         calendar_svc_value_set_str(attendee1, CAL_VALUE_TXT_ATTENDEE_DETAIL_NAME, "heungjae jeong");
         calendar_svc_value_set_str(attendee1, CAL_VALUE_TXT_ATTENDEE_DETAIL_EMAIL, "id@domain.com");
         calendar_svc_value_set_int(attendee1, CAL_VALUE_INT_ATTENDEE_DETAIL_STATUS, 1);
      }
      attendee_list = g_list_append(attendee_list, attendee1);

      attendee2 = calendar_svc_value_new(CAL_VALUE_LST_ATTENDEE_LIST);
      if(attendee2) {
         calendar_svc_value_set_str(attendee2, CAL_VALUE_TXT_ATTENDEE_DETAIL_NAME, "boncheol gu");
         calendar_svc_value_set_str(attendee2, CAL_VALUE_TXT_ATTENDEE_DETAIL_EMAIL, "id@domain.com");
         calendar_svc_value_set_str(attendee2, CAL_VALUE_INT_ATTENDEE_DETAIL_STATUS, 0);
      }
      attendee_list = g_list_append(attendee_list, attendee2);

      calendar_svc_struct_store_list(event, CAL_VALUE_LST_ATTENDEE_LIST, attendee_list);

      calendar_svc_insert(event);
      calendar_svc_struct_free(&event);

      calendar_svc_close();
   }
 * @endcode
 */

/**
 * @addtogroup example
 * @li
 *      update example
 * @code
   void update_test(void)
   {
      cal_struct *event=NULL;
      cal_value *attendee1,*attendee2;
      GList *attendee_list=NULL;
      char * attendee_name = NULL;

      calendar_svc_connect();

      calendar_svc_get(CAL_STRUCT_SCHEDULE,1,&event)

      calendar_svc_struct_set_str(event, CAL_VALUE_TXT_SUMMARY, "weekly meeting2");

      attendee_list = calendar_svc_struct_get_list(event,CAL_VALUE_LST_ATTENDEE_LIST);

      while(attendee_list)
      {

         attendee1 = attendee_list->data;

         attendee_name = calendar_svc_value_get_str(attendee2, CAL_VALUE_TXT_ATTENDEE_DETAIL_NAME);

         if(strcmp(attendee_name,"boncheol gu")==0)
         {
            calendar_svc_value_set_str(attendee1, CAL_VALUE_TXT_ATTENDEE_DETAIL_NAME,"boncheol gu2");
         }
         else if(strcmp(attendee_name,"heungjae jeong")==0)
         {
            calendar_svc_value_set_int(attendee1, CAL_VALUE_INT_DETAIL_DELETE,true); // set delete flag
         }
      }

      attendee2 = calendar_svc_value_new(CAL_VALUE_LST_ATTENDEE_LIST);
      if(attendee2) {
         calendar_svc_value_set_str(attendee2, CAL_VALUE_TXT_ATTENDEE_DETAIL_NAME, "jihwa park");
         calendar_svc_value_set_str(attendee2, CAL_VALUE_TXT_ATTENDEE_DETAIL_EMAIL, "id@domain.com");
      }
      attendee_list = g_list_append(attendee_list, attendee2);

      calendar_svc_struct_store_list(event, CAL_VALUE_LST_ATTENDEE_LIST, attendee_list);

      calendar_svc_update(event);
      calendar_svc_struct_free(&event);

      calendar_svc_close();
   }
 * @endcode
 */


/**
 * @addtogroup example
 * @li
 *      get list/delete example
 * @code
   void delete_test(void)
   {
      cal_struct *event=NULL;
      cal_value *attendee1=NULL,*attendee2=NULL;
      GList *attendee_list=NULL;
      cal_iter *iter=NULL;
      int ret = 0;
      int index = 0;
	  int version = 0;

      calendar_svc_connect();

      calendar_svc_event_get_changes(0, version, &iter);
      ret = calendar_svc_iter_next(iter);
      while(CAL_SUCCESS == ret)
      {
         calendar_svc_iter_get_info(iter, &event);
         index = calendar_svc_struct_get_int(event,CAL_VALUE_INT_INDEX);

         calendar_svc_delete(CAL_STRUCT_SCHEDULE,index);

         calendar_svc_struct_free(&event);
         ret = calendar_svc_iter_next(iter);
      }

      calendar_svc_iter_remove(&iter);
      calendar_svc_close();
   }
 * @endcode
 */



/**
 * @addtogroup example
 * @li
 *      get event example
 * @code

   void get_test(void)
   {
      int index, ret=-1;
      cal_struct *event = NULL;
      cal_value *name;
      GList *get_list, *cursor;
      index = 1;
      char *summary;
      ui_event_t appEvent={0,};

      ret = calendar_svc_get(CAL_STRUCT_SCHEDULE,index,CAL_VALUE_MAIN_FILED, &event);

      summary = calendar_svc_struct_get_str(event, CAL_VALUE_TXT_SUMMARY);
      strncpy(appEvent.summay,summary,sizeof(appEvent.summay)-1);


      get_list = NULL;
      calendar_svc_struct_get_list(event, CAL_VALUE_LST_ATTENDEE_LIST, &get_list);

      cursor = get_list;
      for(;cursor;cursor=g_slist_next(cursor))
      {
         DBG("number Type = %d",
            calendar_svc_value_get_int((cal_value*)cursor->data, CAL_VALUE_INT_ATTENDEE_DETAIL_STATUS));

         DBG("Number = %s\n",
            calendar_svc_value_get_str((cal_value*)cursor->data, CAL_VALUE_TXT_ATTENDEE_DETAIL_NAME));
      }

      calendar_svc_struct_free(&event);

   }
* @endcode
*/


#endif /* __CALENDAR_SVC_STRUCT_H__ */

