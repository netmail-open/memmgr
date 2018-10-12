#ifndef __XPLTYPES_H__
#define __XPLTYPES_H__

#include <limits.h>

/* Basic Definitions */

typedef int XplBool;

#ifndef WIN32
typedef unsigned long LONG;
#endif

/* Windows does not have uint in types.h */
#ifdef WIN32
typedef unsigned int uint;
#endif

/* VS2005 does not have ssize_t */
#ifdef _MSC_VER
#include <BaseTsd.h>
#if !defined(_SSIZE_T_) && !defined(_SSIZE_T_DEFINED)
typedef SSIZE_T ssize_t;
#endif
#endif

typedef	unsigned char BYTE;
typedef	unsigned short WORD;

#ifndef _UNICODE_TYPE_DEFINED
#define _UNICODE_TYPE_DEFINED
typedef unsigned short unicode;
#endif

#ifndef FALSE
# define FALSE (0)
#endif

#ifndef TRUE
# define TRUE (!FALSE)
#endif

#ifdef WIN32
// these two shoud be one, we cant get pid by HANDLE oe HANLDE by pid
#define XplPid	DWORD
#define XplProcID HANDLE
#else
#define XplProcID pid_t
#define	XplPid	pid_t
#endif

#ifndef LLONG_MAX
#define LLONG_MAX 9223372036854775807LL
#endif

#ifndef ULLONG_MAX
#define ULLONG_MAX 18446744073709551615ULL
#endif

#ifndef _UNSIGNED_INT_64
#define _UNSIGNED_INT_64
#ifdef __WATCOMC__
typedef unsigned __int64 uint64;
#elif ULLONG_MAX == 18446744073709551615ULL
typedef unsigned long long uint64;
#elif ULONG_MAX == 18446744073709551615ULL
typedef unsigned long uint64;
#elif UINT_MAX == 18446744073709551615ULL
typedef unsigned int uint64;
#elif USHRT_MAX == 18446744073709551615ULL
typedef unsigned short uint64;
#else
#error "Can't determine the size of uint64"
#endif
#endif

#ifndef _SIGNED_INT_64
#define _SIGNED_INT_64
#ifdef __WATCOMC__
typedef signed __int64 int64;
#elif LLONG_MAX == 9223372036854775807LL
typedef signed long long int64;
#elif LONG_MAX == 9223372036854775807LL
typedef signed long int64;
#elif INT_MAX == 9223372036854775807LL
typedef signed int int64;
#elif SHRT_MAX == 9223372036854775807LL
typedef signed short int64;
#else
#error "Can't determine the size of int64"
#endif
#endif

#ifndef _UNSIGNED_INT_32
#define _UNSIGNED_INT_32
#if ULONG_MAX == 4294967295UL
typedef unsigned long uint32;
#elif UINT_MAX == 4294967295UL
typedef unsigned int uint32;
#elif USHRT_MAX == 4294967295UL
typedef unsigned short uint32;
#else
#error "Can't determine the size of uint32"
#endif
#endif

#ifndef _SIGNED_INT_32
#define _SIGNED_INT_32
#if LONG_MAX == 2147483647L
typedef signed long int32;
#elif INT_MAX == 2147483647L
typedef signed int int32;
#elif SHRT_MAX == 2147483647L
typedef signed short int32;
#else
#error "Can't determine the size of int32"
#endif
#endif

#ifndef _UNSIGNED_INT_16
#define _UNSIGNED_INT_16
#if USHRT_MAX == 65535U
typedef unsigned short uint16;
#elif UCHAR_MAX == 65535U
typedef signed char uint16;
#else
#error "Can't determine the size of uint16"
#endif
#endif

#ifndef _SIGNED_INT_16
#define _SIGNED_INT_16
#if SHRT_MAX == 32767
typedef signed short int16;
#elif SCHAR_MAX == 32767
typedef unsigned char uint16;
#else
#error "Can't determine the size of int16"
#endif
#endif

#ifndef _UNSIGNED_INT_8
#define _UNSIGNED_INT_8
#if UCHAR_MAX == 255U
typedef unsigned char uint8;
#else
#error "Can't determine the size of uint8"
#endif
#endif

#ifndef _SIGNED_INT_8
#define _SIGNED_INT_8
#if SCHAR_MAX == 127
typedef signed char int8;
#else
#error "Can't determine the size of int8"
#endif
#endif

#if defined(LINUX) || defined(MACOSX)

#define s_addr_1  s_addr>>24 & 0xff
#define s_addr_2  s_addr>>16 & 0xff
#define s_addr_3  s_addr>>8  & 0xff
#define s_addr_4  s_addr     & 0xff

#if XPL_LITTLE_ENDIAN
#define s_net     s_addr_4
#define s_host    s_addr_3
#define s_lh      s_addr_2
#define s_impno   s_addr_1
#else
#define s_net     s_addr_1
#define s_host    s_addr_2
#define s_lh      s_addr_3
#define s_impno   s_addr_4
#endif

#endif

#endif /* __XPLTYPES_H__ */
