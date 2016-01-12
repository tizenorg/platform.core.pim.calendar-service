CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(calendar-service C CXX)

INCLUDE(FindPkgConfig)

SET(EXTRA_CFLAGS "-Wall -Werror-implicit-function-declaration -fvisibility=hidden")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_CFLAGS}")

SET(CLIENT calendar-service2)
SET(DAEMON calendar-serviced)

IF(NOT DEFINED DBUS_INTERFACE)
	MESSAGE("No DBUS_INTERFACE. Check build system")
	SET(DBUS_INTERFACE "org.tizen.CalendarService.dbus")
ENDIF(NOT DEFINED DBUS_INTERFACE)

EXECUTE_PROCESS(COMMAND build-util/generator.sh)

ADD_SUBDIRECTORY(common)
ADD_SUBDIRECTORY(client)
ADD_SUBDIRECTORY(server)
