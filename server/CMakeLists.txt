INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/common)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/common/ipc)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/server)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/server/db)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/schema)

FILE(GLOB DAEMON_SRCS
	${CMAKE_SOURCE_DIR}/common/ipc/*.c
	${CMAKE_SOURCE_DIR}/common/*.c*
	db/*.c
	*.c)

pkg_check_modules(daemon_pkgs REQUIRED
	glib-2.0
	db-util
	sqlite3
	vconf
	alarm-service
	icu-i18n
	contacts-service2
	libtzplatform-config
	capi-appfw-package-manager
	pims-ipc
	dlog
	capi-base-common
	accounts-svc
	capi-appfw-application
	libsmack)
INCLUDE_DIRECTORIES(${daemon_pkgs_INCLUDE_DIRS})
LINK_DIRECTORIES(${daemon_pkgs_LIBRARY_DIRS})

ADD_DEFINITIONS("-DCAL_IPC_SERVER")

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIE")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIE")
SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--hash-style=both -pie")

ADD_EXECUTABLE(${DAEMON} ${DAEMON_SRCS})
SET_TARGET_PROPERTIES(${DAEMON} PROPERTIES COMPILE_FLAGS ${EXTRA_CFLAGS})
TARGET_LINK_LIBRARIES(${DAEMON} ${daemon_pkgs_LIBRARIES})

INSTALL(TARGETS ${DAEMON} DESTINATION ${BIN_INSTALL_DIR})
