INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/native)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/common)
LINK_DIRECTORIES(${CMAKE_BINARY_DIR})

SET(INC_DIR ${CMAKE_SOURCE_DIR}/include)

SET(CALSVCNATIVE calendar-service-native)

SET(SRCS
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
	${CMAKE_SOURCE_DIR}/common/cal_inotify.c
	${CMAKE_SOURCE_DIR}/common/cal_list.c
	${CMAKE_SOURCE_DIR}/common/cal_time.cpp
	${CMAKE_SOURCE_DIR}/common/cal_vcalendar.c
	${CMAKE_SOURCE_DIR}/common/cal_vcalendar_make.c
	${CMAKE_SOURCE_DIR}/common/cal_vcalendar_parse.c
	${CMAKE_SOURCE_DIR}/common/cal_mutex.c
	${CMAKE_SOURCE_DIR}/native/cal_db.c
	${CMAKE_SOURCE_DIR}/native/cal_calendar.c
	${CMAKE_SOURCE_DIR}/native/cal_db_util.c
	${CMAKE_SOURCE_DIR}/native/cal_db_alarm.c
	${CMAKE_SOURCE_DIR}/native/cal_db_attendee.c
	${CMAKE_SOURCE_DIR}/native/cal_db_instance.c
	${CMAKE_SOURCE_DIR}/native/cal_db_rrule.c
	${CMAKE_SOURCE_DIR}/native/cal_db_query.c
	${CMAKE_SOURCE_DIR}/native/cal_db_plugin_calendar.c
	${CMAKE_SOURCE_DIR}/native/cal_db_plugin_event.c
	${CMAKE_SOURCE_DIR}/native/cal_db_event.c
	${CMAKE_SOURCE_DIR}/native/cal_db_plugin_attendee.c
	${CMAKE_SOURCE_DIR}/native/cal_db_plugin_alarm.c
	${CMAKE_SOURCE_DIR}/native/cal_db_plugin_search.c
	${CMAKE_SOURCE_DIR}/native/cal_db_plugin_timezone.c
	${CMAKE_SOURCE_DIR}/native/cal_db_plugin_instance_normal.c
	${CMAKE_SOURCE_DIR}/native/cal_db_plugin_instance_normal_extended.c
	${CMAKE_SOURCE_DIR}/native/cal_db_plugin_instance_allday.c
        ${CMAKE_SOURCE_DIR}/native/cal_db_plugin_instance_allday_extended.c
	${CMAKE_SOURCE_DIR}/native/cal_db_plugin_todo.c
	${CMAKE_SOURCE_DIR}/native/cal_db_plugin_extended.c
	${CMAKE_SOURCE_DIR}/native/cal_db_extended.c
	${CMAKE_SOURCE_DIR}/native/cal_reminder.c
	${CMAKE_SOURCE_DIR}/native/cal_db_instance_helper.c
)

INCLUDE(FindPkgConfig)
pkg_check_modules(servicenative_pkgs REQUIRED db-util alarm-service capi-base-common)

FOREACH(flag ${servicenative_pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} ")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_CFLAGS}")

SET(servicenative_pkgs_LDFLAGS "${pkgs_LDFLAGS} ${servicenative_pkgs_LDFLAGS}")

ADD_DEFINITIONS("-DCAL_NATIVE")
ADD_DEFINITIONS("-DPREFIX=\"${PREFIX}\"")

#SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")

#ADD_LIBRARY(${CALSVCNATIVE} SHARED ${SRCS})
#SET_TARGET_PROPERTIES(${CALSVCNATIVE} PROPERTIES SOVERSION ${VERSION_MAJOR})
#SET_TARGET_PROPERTIES(${CALSVCNATIVE} PROPERTIES VERSION ${VERSION})
#TARGET_LINK_LIBRARIES(${CALSVCNATIVE} ${servicenative_pkgs_LDFLAGS})

INSTALL(TARGETS ${CALSVCNATIVE} DESTINATION ${LIB_INSTALL_DIR})

#pc
CONFIGURE_FILE(${CALSVCNATIVE}.pc.in ${CALSVCNATIVE}.pc @ONLY)
INSTALL(FILES ${CALSVCNATIVE}.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig)
#INSTALL(FILES calendar-service2.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig)
