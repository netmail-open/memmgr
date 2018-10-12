if(WINDOWS)
	set(PROC_C procwin.c)
else()
	set(PROC_C procunix.c)
endif()

add_definitions( -D_FILE_OFFSET_BITS=64 -DLARGEFILE_SOURCE -D_LARGEFILE64_SOURCE )
add_library(${TARGET_NAME} SHARED
	wrap.c
	dir.c
	fsys.c
	mem.c
	rwlock.c
	cfgfile.c
	err.c
	elf.c
	auth.c
	process.c
	proc.c
	${PROC_C}
	xplalarm.c
	xpllog.c
)

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ALL_CFLAGS} ${PTHREAD_CFLAGS}")

if(WINDOWS)
	set(PDH_LIBS pdh)
	set (PSAPI_LIBS psapi)	#Kernel32.lib on Windows 7 and Windows Server 2008 R2;
							#Psapi.lib (if PSAPI_VERSION=1) on Windows 7 and Windows Server 2008 R2;
							#Psapi.lib on Windows Server 2008, Windows Vista, Windows Server 2003, and Windows XP
endif(WINDOWS)

target_link_libraries(${TARGET_NAME}
#	memmgr${BIT}
	${PSAPI_LIBS}
	${PDH_LIBS}
	${DL_LIBS}
	${RT_LIBS}
	${RESOLV_LIBS}
	${OPENSSL_LIBS}
)

SET_TARGET_PROPERTIES(
	${TARGET_NAME}
	PROPERTIES
	VERSION 1.0.0
	SOVERSION 1
	INSTALL_NAME_DIR "${LIB_DEST_DIR}"
)

install(TARGETS ${TARGET_NAME}
	EXPORT memmgr
	LIBRARY DESTINATION ${LIB_DEST_DIR}
	ARCHIVE DESTINATION ${LIB_DEST_DIR}
)

if (MAIN_PLATFORM)
	# Unit Tests for XplAlarm
	add_executable(xplalarmtest xplalarmtest.c)
	target_link_libraries(xplalarmtest
		${AVOID_VERSION}
#		memmgr${BIT}
#		xpl${BIT}
		memmgr
		xpl
		${PTHREAD_LIBS}
		${ALL_LIBS}
	)

	add_test(XplAlarm:RunOnce		${EXECUTABLE_OUTPUT_PATH}/xplalarmtest runonce		)
	add_test(XplAlarm:Repeat		${EXECUTABLE_OUTPUT_PATH}/xplalarmtest repeat		)
	add_test(XplAlarm:Cancel		${EXECUTABLE_OUTPUT_PATH}/xplalarmtest cancel		)
	add_test(XplAlarm:Multiple		${EXECUTABLE_OUTPUT_PATH}/xplalarmtest multiple		)
endif()

