INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/client)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/common)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/common/ipc)
LINK_DIRECTORIES(${CMAKE_BINARY_DIR})

SET(INC_DIR ${CMAKE_SOURCE_DIR}/include)

SET(CALSVC2 calendar-service2)

SET(SRCS
	cal_client_ipc.c
	cal_client_db.c
    cal_client_reminder.c
	${CMAKE_SOURCE_DIR}/common/ipc/cal_ipc_marshal.c
	${CMAKE_SOURCE_DIR}/common/ipc/cal_ipc_marshal_alarm.c
	${CMAKE_SOURCE_DIR}/common/ipc/cal_ipc_marshal_attendee.c
	${CMAKE_SOURCE_DIR}/common/ipc/cal_ipc_marshal_calendar.c
	${CMAKE_SOURCE_DIR}/common/ipc/cal_ipc_marshal_event.c
	${CMAKE_SOURCE_DIR}/common/ipc/cal_ipc_marshal_instance_allday.c
	${CMAKE_SOURCE_DIR}/common/ipc/cal_ipc_marshal_instance_normal.c
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
    ${CMAKE_SOURCE_DIR}/common/cal_record_extended.c
    ${CMAKE_SOURCE_DIR}/common/cal_view.c
    ${CMAKE_SOURCE_DIR}/common/cal_filter.c
    ${CMAKE_SOURCE_DIR}/common/cal_query.c
    ${CMAKE_SOURCE_DIR}/common/cal_inotify.c
    ${CMAKE_SOURCE_DIR}/common/cal_list.c
    ${CMAKE_SOURCE_DIR}/common/cal_time.cpp
    ${CMAKE_SOURCE_DIR}/common/cal_vcalendar.c
    ${CMAKE_SOURCE_DIR}/common/cal_vcalendar_make.c
    ${CMAKE_SOURCE_DIR}/common/cal_vcalendar_parse.c
    ${CMAKE_SOURCE_DIR}/common/cal_mutex.c
)

INCLUDE(FindPkgConfig)
pkg_check_modules(service2_pkgs REQUIRED pims-ipc dlog capi-base-common)

FOREACH(flag ${service2_pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} ")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_CFLAGS}")

SET(service2_pkgs_LDFLAGS "${pkgs_LDFLAGS} ${service2_pkgs_LDFLAGS}")

ADD_DEFINITIONS("-DCAL_IPC_CLIENT")
ADD_DEFINITIONS("-DPREFIX=\"${PREFIX}\"")

#SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")

ADD_LIBRARY(${CALSVC2} SHARED ${SRCS})
SET_TARGET_PROPERTIES(${CALSVC2} PROPERTIES SOVERSION ${VERSION_MAJOR})
SET_TARGET_PROPERTIES(${CALSVC2} PROPERTIES VERSION ${VERSION})
TARGET_LINK_LIBRARIES(${CALSVC2} ${service2_pkgs_LDFLAGS})

INSTALL(TARGETS ${CALSVC2} DESTINATION ${LIB_INSTALL_DIR})
INSTALL(FILES ${CALSVC2}.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig)

#header
FILE(GLOB HEADER_FILES ${INC_DIR}/calendar*.h)
INSTALL(FILES ${HEADER_FILES} DESTINATION include/calendar-service2)
