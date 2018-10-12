#ifndef XPL_H
#define XPL_H

/* #include <config.h> */

#ifdef WINDOWS
# define		WIN_CDECL		__cdecl
# define		WIN_STDCALL		__stdcall
# define		EXPORT			__declspec(dllexport)
# define		IMPORT			__declspec(dllimport)
# define		INLINE			__inline
#else
# define		WIN_CDECL
# define		WIN_STDCALL
# define		EXPORT
# define		IMPORT
# define		INLINE			__inline
#endif

#if HAVE_INTTYPES_H
#include<inttypes.h>
#else
#define PRIx32  "lx"
#define PRIx64  "llx"

#define PRIu8   "hhu"
#define PRIu16  "hu"
#define PRIu32  "lu"
#define PRIu64  "llu"
#endif

#include "xpldefs.h"
#include "xpltypes.h"
#include "xplutil.h"
#include "xplerr.h"
#include "xplthread.h"
/* #include "xplip.h" */
#include "xplsockaddr.h"
#include "xplservice.h"
#include "xplold.h"
#include "xplauth.h"

#if HAVE_STDINT_H
#include <stdint.h>
#else
typedef int64 int64_t;
typedef uint64 uint64_t;
#endif

#ifndef WINDOWS
/* xplwrap is currently only needed on windows */
# define XPL_NO_WRAP	1
#endif

#include "xplwrap.h"

/*#include "memmgr.h"*/

#ifdef _MSC_VER
#ifndef va_copy
#define va_copy(dest, src) (dest = src)
#endif
#endif


#endif
