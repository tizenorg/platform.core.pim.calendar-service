CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(calendar-service C CXX)

SET(DEST_INCLUDE_DIR "include/calendar-service")
SET(SRC_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")

SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(EXEC_PREFIX "\${prefix}")
SET(LIBDIR ${LIB_INSTALL_DIR})
SET(INCLUDEDIR "\${prefix}/${DEST_INCLUDE_DIR}")

EXECUTE_PROCESS(COMMAND build-util/generator.sh)

#INCLUDE_DIRECTORIES(${SRC_INCLUDE_DIR})
SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} -I${CMAKE_SOURCE_DIR}/include")

INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs REQUIRED
	glib-2.0
	db-util
	sqlite3
	vconf
	alarm-service
	icu-i18n
	contacts-service2
	capi-appfw-package-manager
	libtzplatform-config
)

FOREACH(flag ${pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} -fvisibility=hidden -fdata-sections -ffunction-sections -Wl,--gc-sections")

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_CFLAGS}")
#SET(CMAKE_C_FLAGS_DEBUG "-O0 -g")

ADD_DEFINITIONS("-DPREFIX=\"${PREFIX}\"")

# Install header file
FILE(GLOB HEADER_FILES ${SRC_INCLUDE_DIR}/calendar*.h)
INSTALL(FILES ${HEADER_FILES} DESTINATION ${INCLUDEDIR}2)

#ADD_SUBDIRECTORY(schema)
ADD_SUBDIRECTORY(client)
ADD_SUBDIRECTORY(server)
