INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src)
LINK_DIRECTORIES(${CMAKE_BINARY_DIR})

SET(TARGET calendar-svc-initdb)

EXECUTE_PROCESS(COMMAND ./generator.sh WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

FILE(GLOB SRCS *.c)

pkg_check_modules(initdb_pkgs REQUIRED db-util dlog)

UNSET(EXTRA_CFLAGS)
FOREACH(flag ${initdb_pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

#SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")

ADD_EXECUTABLE(${TARGET} ${SRCS})
SET_TARGET_PROPERTIES(${TARGET} PROPERTIES COMPILE_FLAGS ${EXTRA_CFLAGS})
TARGET_LINK_LIBRARIES(${TARGET} ${initdb_pkgs_LDFLAGS})

INSTALL(TARGETS ${TARGET} DESTINATION bin)
