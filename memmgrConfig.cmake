# - Try to find memmgr lib
# Once done this will define
#
#  MEMMGR_FOUND
#  MEMMGR_INCLUDE_DIR
#  MEMMGR_LIBRARIES
if(WIN32)
elseif()
	find_path(MEMMGR_INCLUDE_DIR memmgr/memmgr.h
		/usr/local/include
		/usr/include
	)

	find_library(MEMMGR_LIBRARY memmgr)

	if (MEMMGR_LIBRARY AND MEMMGR_INCLUDE_DIR)
		set(MEMMGR_LIBRARIES		${MEMMGR_LIBRARY})
		set(MEMMGR_LIBS   ${MEMMGR_LIBRARY} ncurses)

		set(MEMMGR_INCLUDE_DIRS	${MEMMGR_INCLUDE_DIR})
		set(MEMMGR_FOUND			1)
	else()
		message("***  memmgr library not found")
	endif()
endif()

mark_as_advanced(
	MEMMGR_LIBRARY
	MEMMGR_INCLUDE_DIR
)
