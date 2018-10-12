########## CMake Policies ##############
#if (COMMAND cmake_policy)
#	cmake_policy(SET CMP0003 NEW)
#	cmake_policy(SET CMP0002 NEW) #unique names
#
#	if ("${CMAKE_VERSION}" STRLESS "2.8.5")
#		# There is no STRGREATEROREQUAL so we have to have an else...
#	else ()
#		cmake_policy(SET CMP0015 NEW)
#	endif()
#endif()

######### OS Detection ##############
if (CMAKE_SYSTEM_NAME MATCHES Linux)
	set (LINUX						1)
	set (UNIX						1)
endif (CMAKE_SYSTEM_NAME MATCHES Linux)

if (CMAKE_SYSTEM_NAME MATCHES Windows)
	set (WIN32						1)
	set (WINDOWS					1)
	set (_WIN32						)
	set (_CRT_SECURE_NO_DEPRECATE	1)
endif (CMAKE_SYSTEM_NAME MATCHES Windows)


######### Debug Options ##############
if (CMAKE_BUILD_TYPE MATCHES debug)
	add_definitions(-DDEBUG)
	set(DEBUG 1)

	set(OPTIMIZATION_LEVEL	"0"
		CACHE STRING		"Optimization Level")

	set (ASSERT_TYPE		"crash"
		CACHE STRING		"Assert type, options are: crash break thread print none")
	string(TOUPPER "${ASSERT_TYPE}" ASSERT_TYPE)

	if (LINUX)
		OPTION(ENABLE_GPROF	"Enable profiling with gprof" OFF)
		if (ENABLE_GPROF)
			add_definitions(-DGPROF)
			set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg")
		endif()
	endif()
else()
	set(OPTIMIZATION_LEVEL	"0"
		CACHE STRING		"Optimization Level")

	set (ASSERT_TYPE		"none"
		CACHE STRING		"Assert type, options are: crash break thread print none")
	string(TOUPPER "${ASSERT_TYPE}" ASSERT_TYPE)
endif()


# Paths
################################################################################
#
# The RUNTIME_BASE_DIR and RUNTIME_DATA_DIR paths should match
# CMAKE_INSTALL_PREFIX and DATA_DIR in most cases. They may be set differently
# to deal with installers that will move the files after the fact though.

# TODO	Remove DATA_DIR. It is an obsolete option.
#set(DATA_DIR				"${CMAKE_INSTALL_PREFIX}")
#
#set(
#	RUNTIME_BASE_DIR		""
#	CACHE STRING			"Runtime base directory. If not set then the configured CMAKE_INSTALL_PREFIX value will be used."
#)
#
#if (NOT RUNTIME_BASE_DIR)
#	set(
#		RUNTIME_BASE_DIR	"${CMAKE_INSTALL_PREFIX}"
#	)
#endif()
#
#set(
#	RUNTIME_DATA_DIR		""
#	CACHE STRING			"Runtime data directory. If not set then the configured DATA_DIR value will be used."
#)
#
#if (NOT RUNTIME_DATA_DIR)
#	set(
#		RUNTIME_DATA_DIR	"${DATA_DIR}"
#	)
#endif()



######### CPU Detection ##############
function(setbit_nope bits)
	set(BIT ${bits} PARENT_SCOPE)

	# Sanitize C_FLAGS
	string(REPLACE "-m32"				"" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
	string(REPLACE "-m64"				"" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
	string(REPLACE "-DAMD64"			"" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
	string(REPLACE "-DPOINTER_SIZE_64"	"" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
	string(REPLACE "-DPOINTER_SIZE_32"	"" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

	if (LINUX)
		set(CMAKE_C_FLAGS "-m${bits} ${CMAKE_C_FLAGS}")
	endif()

	if (bits EQUAL 64)
		set(AMD64 1 PARENT_SCOPE)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DPOINTER_SIZE_64 -DAMD64")
	else()
		set(AMD64 0 PARENT_SCOPE)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DPOINTER_SIZE_32")
	endif()

	if (LINUX)
		string(REGEX REPLACE "32$"			"" LIB_DEST_DIR  "${LIB_DEST_DIR}")
		string(REGEX REPLACE "64$"			"" LIB_DEST_DIR  "${LIB_DEST_DIR}")

		set(LIB_DEST_DIR ${LIB_DEST_DIR}${bits} PARENT_SCOPE)
	endif()

	# Set this in the parent scope once to ensure that all the changes are
	# passed through.
	set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} PARENT_SCOPE)
endfunction()

# Add a directory for a library that may be built multiple times if multiple
# architectures are needed for the current build.
#
# The name of the directory should be passed as the first argument, which the
# library can access as TARGET_NAME.
#
# The library can install libraries or modules using the installlib() and
# installmodule() functions.
function(addlib_nope name)
	set(LIB NAME "${name}")

	function(installlib TARGET_NAME)
		string(REGEX REPLACE "${BIT}$" "" TARGET_BASE_NAME "${TARGET_NAME}")

		set_target_properties(${TARGET_NAME}
				PROPERTIES
					VERSION				0.0.0
					SOVERSION			0
					INSTALL_NAME_DIR	"${LIB_DEST_DIR}"
					OUTPUT_NAME			"${TARGET_BASE_NAME}"
		)

		install(TARGETS ${TARGET_NAME}
			RUNTIME DESTINATION ${LIB_DEST_DIR}
			LIBRARY DESTINATION ${LIB_DEST_DIR}
		)

		exportlib(${TARGET_NAME})
	endfunction()

	# Varient of installlib for static libs
	function(exportlib TARGET_NAME)
		get_property(EXPORT_TARGETS GLOBAL PROPERTY NETMAIL_EXPORT_TARGETS)
		list(APPEND EXPORT_TARGETS ${TARGET_NAME})
		set_property(GLOBAL PROPERTY NETMAIL_EXPORT_TARGETS ${EXPORT_TARGETS})
	endfunction()

	function(installmodule TARGET_NAME)
		string(REGEX REPLACE "${BIT}$" "" TARGET_BASE_NAME "${TARGET_NAME}")
		if (ARGV1)
			set(LIB_DEST_DIR "${ARGV1}")
		endif()

		set_target_properties(${TARGET_NAME}
				PROPERTIES
					VERSION				0.0.0
					SOVERSION			0
					PREFIX				""
					INSTALL_NAME_DIR	"${LIB_DEST_DIR}"
					OUTPUT_NAME			"${TARGET_BASE_NAME}"
		)

		install(TARGETS ${TARGET_NAME}
			RUNTIME DESTINATION ${LIB_DEST_DIR}
			LIBRARY DESTINATION ${LIB_DEST_DIR}
		)
	endfunction()

	if (BUILDBOTH AND NOT ARGV1)
		unset(MAIN_PLATFORM)
		setbit(32)
		set(TARGET_NAME "${name}${BIT}")
		add_subdirectory(${name} ${TARGET_NAME})

		setbit(64)
	endif()

	set(MAIN_PLATFORM 1)
	set(TARGET_NAME "${name}${BIT}")
	add_subdirectory(${name})
endfunction()


function(addlib name)
	set(TARGET_NAME "${name}")
	add_subdirectory(${name})
endfunction()
function(installlib TARGET_NAME)
	set_target_properties(${TARGET_NAME}
		PROPERTIES
		VERSION				0.0.0
		SOVERSION			0
		INSTALL_NAME_DIR	"${LIB_DEST_DIR}"
		OUTPUT_NAME			"${TARGET_BASE_NAME}"
	)
	install(TARGETS ${TARGET_NAME}
		RUNTIME DESTINATION ${LIB_DEST_DIR}
		LIBRARY DESTINATION ${LIB_DEST_DIR}
	)
	exportlib(${TARGET_NAME})
endfunction()
function(exportlib TARGET_NAME)
	get_property(EXPORT_TARGETS GLOBAL PROPERTY NETMAIL_EXPORT_TARGETS)
	list(APPEND EXPORT_TARGETS ${TARGET_NAME})
	set_property(GLOBAL PROPERTY NETMAIL_EXPORT_TARGETS ${EXPORT_TARGETS})
endfunction()



## Set compiler specific options
#if (WATCOM)
#	include(${CMAKE_CURRENT_LIST_DIR}/watcom.cmake)
#endif()
#
#if (CMAKE_COMPILER_IS_GNUCC)
#	include(${CMAKE_CURRENT_LIST_DIR}/gcc.cmake)
#endif()
#
#if (MSVC)
#	include(${CMAKE_CURRENT_LIST_DIR}/msvc.cmake)
#endif()
#
## OS specific options
#if (LINUX)
#	include(${CMAKE_CURRENT_LIST_DIR}/linux.cmake)
#endif()
#
#if (WINDOWS)
#	include(${CMAKE_CURRENT_LIST_DIR}/windows.cmake)
#endif()

#if (CMAKE_SIZEOF_VOID_P MATCHES "8")
#	setbit(64)
#
#	if (LINUX)
#		if (NETMAIL_BUILD_ARCH)
#			# Use the value from the parent project
#			set(BUILD_ARCH "${NETMAIL_BUILD_ARCH}")
#		else()
#			set(
#				BUILD_ARCH		"both"
#				CACHE STRING	"Architectures to build. Options are 32, 64 or both. Choosing both will build 2 copies of each library and most executables as 64 bit."
#			)
#		endif()
#
#		if ("${BUILD_ARCH}" MATCHES "32")
#			setbit(32)
#		elseif ("${BUILD_ARCH}" MATCHES "64")
#			setbit(64)
#		else()
#			set(BUILDBOTH 1)
#		endif()
#	endif()
#else()
#	setbit(32)
#endif()


#include(${CMAKE_CURRENT_LIST_DIR}/rpath.cmake)




######### Save build options ##############
#INSTALL(
#	FILES		${CMAKE_CURRENT_BINARY_DIR}/CMakeCache.txt
#	DESTINATION	etc/build/${PROJECT_NAME}/
#)
