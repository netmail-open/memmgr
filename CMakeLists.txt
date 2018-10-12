project(memmgr)
cmake_minimum_required(VERSION 2.6)
include(CheckIncludeFiles)
include(CheckSymbolExists)

option(STATIC_LIB "Compile as static libraries" OFF)

if(STATIC_LIB)
  set(BUILD_SHARED_LIBS OFF)
  add_definitions(-DCOMPILE_AS_STATIC)
else(STATIC_LIB)
  set(BUILD_SHARED_LIBS ON)
endif(STATIC_LIB)

#add_definitions(-DDEBUG) #fixme  
#add_definitions(-DDEBUG_ASSERT) #fixme  

include(cmake/platform.cmake)
include(include/CMakeChecks.txt)

option(DISTINGUISH_INTEGERS "Distinguish between integer and non-integer numbers" OFF)
if(DISTINGUISH_INTEGERS)
  add_definitions(-DWJE_DISTINGUISH_INTEGER_TYPE)
endif()

if("${CMAKE_SYSTEM}" MATCHES "Linux")
  add_definitions(-D_GNU_SOURCE)
endif()

if(NOT CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX /usr/local)
endif(NOT CMAKE_INSTALL_PREFIX)
set(INC_DEST_DIR ${CMAKE_INSTALL_PREFIX}/include)
if(NOT LIB_DEST_DIR)
  set(LIB_DEST_DIR ${CMAKE_INSTALL_PREFIX}/lib)
endif(NOT LIB_DEST_DIR)
set(BIN_DEST_DIR ${CMAKE_INSTALL_PREFIX}/bin)

if(CMAKE_BUILD_TYPE MATCHES Debug)
  add_definitions(-DDEBUG)
  option(SHOWNOTES "Show preprocessor notes" OFF)

  if(CMAKE_COMPILER_IS_GNUCC)
	# GCC specific debug options
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g3 -ggdb3 -gdwarf-2")
	set(AVOID_VERSION -avoid-version)
  endif(CMAKE_COMPILER_IS_GNUCC)
endif(CMAKE_BUILD_TYPE MATCHES Debug)

add_definitions(-DHAVE_CONFIG_H)

check_include_files(string.h HAVE_STRING_H)
check_symbol_exists(strcasecmp "strings.h" HAVE_STRCASECMP)

# use, i.e. don't skip the full RPATH for the build tree
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)
# when building, don't use the install RPATH already
# (but later on when installing)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
# the RPATH to be used when installing
SET(CMAKE_INSTALL_RPATH ${LIB_DEST_DIR})
# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

ENABLE_TESTING(1)

INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/documentation/
	DESTINATION doc/memmgr/
)

set(MEMMGR_TYPE "SLABBER")


############ memmgr-config.h ###########
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/memmgr-config.h.in ${CMAKE_CURRENT_BINARY_DIR}/include/memmgr-config.h)
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/include/memmgr-config.h DESTINATION include/)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/xplutil-assert.h.in ${CMAKE_CURRENT_BINARY_DIR}/include/xplutil-assert.h)
add_definitions(-DHAVE_CONFIG_H)
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/include/xplutil-assert.h DESTINATION include/)


include_directories(
	include
	${CMAKE_CURRENT_BINARY_DIR}/include     # To find config.h
)
add_subdirectory(src)
add_subdirectory(include)

# pkg-config .pc
SET(PKG_CONFIG_REQUIRES glib-2.0)
SET(PKG_CONFIG_LIBDIR
    ${LIB_DEST_DIR}
)
SET(PKG_CONFIG_INCLUDEDIR
	${INC_DEST_DIR}
)
SET(PKG_CONFIG_LIBS
    "-L\${libdir} -lmemmgr"
)
SET(PKG_CONFIG_CFLAGS
    "-I\${includedir}"
)
CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/pkg-config.pc.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/memmgr.pc"
)
INSTALL(FILES "${CMAKE_BINARY_DIR}/memmgr.pc"
        DESTINATION ${LIB_DEST_DIR}/pkgconfig)

#INSTALL(FILES "${CMAKE_BINARY_DIR}/memmgrConfig.cmake"
#        DESTINATION ${LIB_DEST_DIR}/cmake/memmgr)
#install(targets memmgr export memmgr)
#export(targets memmgr file ${CMAKE_BINARY_DIR}/memmgrConfig.cmake)
#install(TARGETS memmgr EXPORT memmgr)

# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
