INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/server)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/server/db)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/common)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/common/ipc)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/schema)
LINK_DIRECTORIES(${CMAKE_BINARY_DIR})
LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/server)

SET(DAEMON calendar-serviced)
SET(LIBNAME calendar-service-server)

SET(LIBSRCS
	${CMAKE_SOURCE_DIR}/common/ipc/cal_ipc_marshal.c
	${CMAKE_SOURCE_DIR}/common/ipc/cal_ipc_marshal_alarm.c
	${CMAKE_SOURCE_DIR}/common/ipc/cal_ipc_marshal_attendee.c
	${CMAKE_SOURCE_DIR}/common/ipc/cal_ipc_marshal_calendar.c
	${CMAKE_SOURCE_DIR}/common/ipc/cal_ipc_marshal_event.c
	${CMAKE_SOURCE_DIR}/common/ipc/cal_ipc_marshal_instance_allday.c
	${CMAKE_SOURCE_DIR}/common/ipc/cal_ipc_marshal_instance_allday_extended.c
	${CMAKE_SOURCE_DIR}/common/ipc/cal_ipc_marshal_instance_normal.c
	${CMAKE_SOURCE_DIR}/common/ipc/cal_ipc_marshal_instance_normal_extended.c
	${CMAKE_SOURCE_DIR}/common/ipc/cal_ipc_marshal_search.c
	${CMAKE_SOURCE_DIR}/common/ipc/cal_ipc_marshal_timezone.c
	${CMAKE_SOURCE_DIR}/common/ipc/cal_ipc_marshal_todo.c
	${CMAKE_SOURCE_DIR}/common/ipc/cal_ipc_marshal_updated_info.c
	${CMAKE_SOURCE_DIR}/common/ipc/cal_ipc_marshal_extended.c
	${CMAKE_SOURCE_DIR}/common/cal_record.c
	${CMAKE_SOURCE_DIR}/common/cal_record_calendar.c
	${CMAKE_SOURCE_DIR}/common/cal_record_event.c
	${CMAKE_SOURCE_DIR}/common/cal_record_todo.c
	${CMAKE_SOURCE_DIR}/common/cal_record_attendee.c
	${CMAKE_SOURCE_DIR}/common/cal_record_alarm.c
	${CMAKE_SOURCE_DIR}/common/cal_record_search.c
	${CMAKE_SOURCE_DIR}/common/cal_record_timezone.c
	${CMAKE_SOURCE_DIR}/common/cal_record_updated_info.c
	${CMAKE_SOURCE_DIR}/common/cal_record_instance_normal.c
	${CMAKE_SOURCE_DIR}/common/cal_record_instance_allday.c
	${CMAKE_SOURCE_DIR}/common/cal_record_instance_normal_extended.c
	${CMAKE_SOURCE_DIR}/common/cal_record_instance_allday_extended.c
	${CMAKE_SOURCE_DIR}/common/cal_record_extended.c
	${CMAKE_SOURCE_DIR}/common/cal_view.c
	${CMAKE_SOURCE_DIR}/common/cal_filter.c
	${CMAKE_SOURCE_DIR}/common/cal_query.c
	${CMAKE_SOURCE_DIR}/common/cal_list.c
	${CMAKE_SOURCE_DIR}/common/cal_time.cpp
	${CMAKE_SOURCE_DIR}/common/cal_inotify.c
    ${CMAKE_SOURCE_DIR}/common/cal_vcalendar.c
    ${CMAKE_SOURCE_DIR}/common/cal_vcalendar_make.c
    ${CMAKE_SOURCE_DIR}/common/cal_vcalendar_parse.c
	${CMAKE_SOURCE_DIR}/common/cal_mutex.c
	${CMAKE_SOURCE_DIR}/common/cal_utils.c
	cal_access_control.c
	cal_server_service.c
	cal_server_ipc.c
	db/cal_db.c
	db/cal_db_util.c
	db/cal_db_rrule.c
	db/cal_db_query.c
	db/cal_db_plugin_todo.c
	db/cal_db_plugin_search.c
	db/cal_db_plugin_calendar.c
	db/cal_db_plugin_calendar_helper.c
	db/cal_db_plugin_event.c
	db/cal_db_plugin_event_helper.c
	db/cal_db_plugin_alarm.c
	db/cal_db_plugin_alarm_helper.c
	db/cal_db_plugin_attendee.c
	db/cal_db_plugin_attendee_helper.c
	db/cal_db_plugin_timezone.c
	db/cal_db_plugin_timezone_helper.c
	db/cal_db_plugin_instance_normal.c
	db/cal_db_plugin_instance_normal_extended.c
	db/cal_db_plugin_instance_allday.c
	db/cal_db_plugin_instance_allday_extended.c
	db/cal_db_plugin_extended.c
	db/cal_db_plugin_extended_helper.c
	db/cal_db_instance.c
	db/cal_db_instance_helper.c
)

INCLUDE(FindPkgConfig)
pkg_check_modules(calserver_pkgs REQUIRED
	dlog
	pims-ipc
	db-util
	alarm-service
	capi-base-common
	icu-i18n
	accounts-svc
	capi-appfw-application
	libtzplatform-config
	libsmack
)

FOREACH(flag ${calserver_pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -fPIE")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_CFLAGS} -fPIE")
SET(calserver_pkgs_LDFLAGS "${pkgs_LDFLAGS} ${calserver_pkgs_LDFLAGS}")

ADD_DEFINITIONS("-DCAL_IPC_SERVER")
ADD_DEFINITIONS("-DPREFIX=\"${PREFIX}\"")

ADD_LIBRARY(${LIBNAME} STATIC ${LIBSRCS})
TARGET_LINK_LIBRARIES(${LIBNAME} ${calserver_pkgs_LDFLAGS})

#cmake_policy(SET CMP0002 OLD)
ADD_EXECUTABLE(${DAEMON}
	cal_server.c
	cal_server_contacts.c
	cal_server_alarm.c
	cal_server_calendar_delete.c
	cal_server_reminder.c
	cal_server_schema.c
	cal_server_update.c
	)
SET_TARGET_PROPERTIES(${DAEMON} PROPERTIES COMPILE_FLAGS ${EXTRA_CFLAGS})
TARGET_LINK_LIBRARIES(${DAEMON} ${calserver_pkgs_LDFLAGS} ${LIBNAME} -pie)

INSTALL(TARGETS ${DAEMON} DESTINATION bin)
