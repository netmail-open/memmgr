/****************************************************************************
 *
 * Copyright (c) 1999-2002 Novell, Inc.
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2.1 of the GNU Lesser General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, contact Novell, Inc.
 *
 * To contact Novell about this file by physical or electronic mail,
 * you may find current contact information at www.novell.com
 *
 ****************************************************************************/

#ifndef XPLUTIL_H
#define XPLUTIL_H
#include <memmgr-config.h>
#include <xpltypes.h>
#include <xplthread.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#if !defined(WINDOWS)
#include <signal.h>
#endif

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#define ECONNCLOSEDBYPEER		777		/* Connection closed gracefully by peer */
#define ECONNIOBUG				778		/* Connio messed up */
#define ECONNIOMODE				779		/* Conn is not in a mode that is compatible with request */
#define ECONNIOINIT				780		/* ConnIO is not initialized to support this feature */
#define ECONNIOCBSHORT			781		/* ConnIOCB returned 'done' before count was satisfied */
#define ECONNIOCBLONG			782		/* ConnIOCB says it took more data than it was given */
#define ECONNSSLSTART			783		/* SSL or TLS failed to start  */
#define ECONNALREADYSECURED		784		/* Connection is already secured */
#define ECONNNOTSECURED			785		/* Connection is not secured */
#define ECONNMOREDATA			786		/* Event requires more data */
#define ECONNAMBIG				787		/* WSA error did not uniquely translate to errno */
#define ECONNHANDLESHUTDOWN		788		/* The conn handle is shutting down */

#define ESERVICENEW				801		/* Service has not been started */
#define ESERVICESTOPPED			802		/* Service has been stopped */

#define ECOMPLETE				901		/* Task complete */

#define EIDENTIFYFAILED			902		/* User identification failed */


#if defined(LINUX) || defined(MACOSX)

#include <dirent.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

#elif defined(WIN32)

#include <windows.h>
#if !defined (_MSC_VER)
#include <winerror.h>
#endif

#include <time.h>

//#include <WinSock2.h>
#include <process.h>

#ifdef __WATCOMC__

#define EWOULDBLOCK				EAGAIN	/* Operation would block */
#define ELOOP					41		/* Too many symbolic links encountered */
#define EREMOTE					66		/* Object is remote */
#define EPROTO					71		/* Protocol error */
#define EUSERS					87		/* Too many users */
#define ENOTSOCK				88		/* Socket operation on non-socket */
#define EDESTADDRREQ			89		/* Destination address required */
#define EMSGSIZE				90		/* Message too long */
#define EPROTOTYPE				91		/* Protocol wrong type for socket */
#define ENOPROTOOPT				92		/* Protocol not available */
#define EPROTONOSUPPORT			93		/* Protocol not supported */
#define ESOCKTNOSUPPORT			94		/* Socket type not supported */
#define EOPNOTSUPP				95		/* Operation not supported on transport endpoint */
#define EPFNOSUPPORT			96		/* Protocol family not supported */
#define EAFNOSUPPORT			97		/* Address family not supported by protocol */
#define EADDRINUSE				98		/* Address already in use */
#define EADDRNOTAVAIL			99		/* Cannot assign requested address */
#define ENETDOWN			   100		/* Network is down */
#define ENETUNREACH			   101		/* Network is unreachable */
#define ENETRESET			   102		/* Network dropped connection because of reset */
#define ECONNABORTED		   103		/* Software caused connection abort */
#define ECONNRESET			   104		/* Connection reset by peer */
#define ENOBUFS				   105		/* No buffer space available */
#define EISCONN				   106		/* Transport endpoint is already connected */
#define ENOTCONN			   107		/* Transport endpoint is not connected */
#define ESHUTDOWN			   108		/* Cannot send after transport endpoint shutdown */
#define ETOOMANYREFS		   109		/* Too many references: cannot splice */
#define ETIMEDOUT			   110		/* Connection timed out */
#define ECONNREFUSED		   111		/* Connection refused */
#define EHOSTDOWN			   112		/* Host is down */
#define EHOSTUNREACH		   113		/* No route to host */
#define EALREADY			   114		/* Operation already in progress */
#define EINPROGRESS			   115		/* Operation now in progress */
#define ESTALE				   116		/* Stale NFS file handle */
#define EDQUOT				   122		/* Quota exceeded */
#define ECANCELED			   125		/* Operation Canceled */
#define EIDRM				126
#define ECHRNG				127
#define EL2NSYNC			128
#define EL3HLT				129
#define EL3RST				130
#define ELNRNG				131
#define EUNATCH				132
#define ENOCSI				133
#define EL2HLT				134
#define EBADE				135
#define EBADR				136
#define EXFULL				137
#define ENOANO				138
#define EBADRQC				139
#define EBADSLT				140
#define EBFONT				141
#define ENOSTR				142
#define ENODATA				143
#define ETIME				144
#define ENOSR				145
#define ENONET				146
#define ENOPKG				147
#define ENOLINK				148
#define EADV				149
#define ESRMNT				150
#define ECOMM				151
#define EMULTIHOP			152
#define EDOTDOT				153
#define EBADMSG				154
#define EOVERFLOW			155
#define ENOTUNIQ			156
#define EBADFD				157
#define EREMCHG				158
#define ELIBACC				159
#define ELIBSCN				160
#define ELIBMAX				161
#define ELIBEXEC			162
#define ERESTART			163
#define ESTRPIPE			164
#define EUCLEAN				165
#define ENOTNAM				166
#define ENAVAIL				167
#define EISNAM				168
#define EREMOTEIO			169
#define ENOMEDIUM			170
#define EMEDIUMTYPE			171
#define ENOKEY				172
#define EKEYEXPIRED			173
#define EKEYREVOKED			174
#define EKEYREJECTED		175
#define EOWNERDEAD			176
#define ENOTRECOVERABLE		177
#define ENOTSUP				178
#define ENOMSG				998		/* No message of desired type */

#if  ( __WATCOMC__ < 1280 )



#define WSAID_ACCEPTEX \
		{0xb5367df1,0xcbac,0x11cf,{0x95,0xca,0x00,0x80,0x5f,0x48,0xa1,0x92}}
#define WSAID_CONNECTEX \
		{0x25a207b9,0xddf3,0x4660,{0x8e,0xe9,0x76,0xe5,0x8c,0x74,0x06,0x3e}}
#define WSAID_DISCONNECTEX \
		{0x7fda2e11,0x8630,0x436f,{0xa0, 0x31, 0xf5, 0x36, 0xa6, 0xee, 0xc1, 0x57}}
#define WSAID_GETACCEPTEXSOCKADDRS \
		{0xb5367df2,0xcbac,0x11cf,{0x95,0xca,0x00,0x80,0x5f,0x48,0xa1,0x92}}
#define WSAID_TRANSMITFILE \
		{0xb5367df0,0xcbac,0x11cf,{0x95,0xca,0x00,0x80,0x5f,0x48,0xa1,0x92}}
#define WSAID_TRANSMITPACKETS \
		{0xd9689da0,0x1f90,0x11d3,{0x99,0x71,0x00,0xc0,0x4f,0x68,0xc8,0x76}}
#define WSAID_WSARECVMSG \
		{0xf689d7c8,0x6f1f,0x436b,{0x8a,0x53,0xe5,0x4f,0xe3,0x51,0xc3,0x22}}


typedef TRANSMIT_PACKETS_ELEMENT *PTRANSMIT_PACKETS_ELEMENT;
typedef TRANSMIT_PACKETS_ELEMENT *LPTRANSMIT_PACKETS_ELEMENT;

#else
#include <mswsock.h>
#endif //  ( __WATCOMC__ < 1280 )

typedef XplBool (WINAPI * LPFN_ACCEPTEX)(SOCKET, SOCKET, PVOID, DWORD, DWORD, DWORD, LPDWORD, LPOVERLAPPED);
typedef XplBool (WINAPI * LPFN_CONNECTEX)(SOCKET, const struct sockaddr *, int, PVOID, DWORD, LPDWORD, LPOVERLAPPED);
typedef XplBool (WINAPI * LPFN_DISCONNECTEX)(SOCKET, LPOVERLAPPED, DWORD, DWORD);
typedef VOID (WINAPI * LPFN_GETACCEPTEXSOCKADDRS)(PVOID, DWORD, DWORD, DWORD, struct sockaddr **, LPINT, struct sockaddr **, LPINT);
typedef XplBool (WINAPI * LPFN_TRANSMITFILE)(SOCKET, HANDLE, DWORD, DWORD, LPOVERLAPPED, LPTRANSMIT_FILE_BUFFERS, DWORD);
typedef XplBool (WINAPI * LPFN_TRANSMITPACKETS)(SOCKET, LPTRANSMIT_PACKETS_ELEMENT, DWORD, DWORD, LPOVERLAPPED, DWORD);
typedef INT  (WINAPI * LPFN_WSARECVMSG)(SOCKET, LPWSAMSG, LPDWORD, LPWSAOVERLAPPED, LPWSAOVERLAPPED_COMPLETION_ROUTINE);

#else

/*
	The following ISO C99 Standard error definitions match what is defined in
	the Microsoft Windows SDK header errno.h.

	EPERM				 1		Operation not permitted
	ENOENT				 2		No such file or directory
	ESRCH				 3		No such process
	EINTR				 4		Interrupted system call
	EIO					 5		I/O error
	ENXIO				 6		No such device or address
	E2BIG				 7		Argument list too long
	ENOEXEC				 8		Exec format error
	EBADF				 9		Bad file number
	ECHILD				10		No child processes
	EAGAIN				11		Try again
	ENOMEM				12		Out of memory
	EACCES				13		Permission denied
	EFAULT				14		Bad address
	EBUSY				16		Device or resource busy
	EEXIST				17		File exists
	EXDEV				18		Cross-device link
	ENODEV				19		No such device
	ENOTDIR				20		Not a directory
	EISDIR				21		Is a directory
	EINVAL				22		Invalid argument
	ENFILE				23		File table overflow
	EMFILE				24		Too many open files
	ENOTTY				25		Not a typewriter
	EFBIG				27		File too large
	ENOSPC				28		No space left on device
	ESPIPE				29		Illegal seek
	EROFS				30		Read-only file system
	EMLINK				31		Too many links
	EPIPE				32		Broken pipe
	EDOM				33		Math argument out of domain of func
	ERANGE				34		Math result not representable
*/

/*
	The following ISO C99 Standard error definitions are not defined in the
	Microsoft Windows SDK header errno.h.
*/
//#define ENOTBLK					15		/* Block device required */
//#define ETXTBSY					26		/* Text file busy */
// TODO: FIXME!  hans check if this if OK
#if defined (_MSC_VER)

//#define ELOOP			455  // completely fake hans TODO: should this be 40, 41?
//#define ENOMSG			456 // completely fake hans TODO: find proper value
#else
#define ELOOP					40		/* Too many symbolic links encountered */
#endif


//ESOCKTNOSUPPORT         WSAESOCKTNOSUPPORT
#if defined (_MSC_VER)

// Hans NEW:
#define EINVAL					22
// end new
//#define EWOULDBLOCK				EAGAIN	/* Operation would block */
//#define EIDRM					43		/* Identifier removed */
#define ECHRNG					44		/* Channel number out of range */
#define EL2NSYNC				45		/* Level 2 not synchronized */
#define EL3HLT					46		/* Level 3 halted */
#define EL3RST					47		/* Level 3 reset */
#define ELNRNG					48		/* Link number out of range */
#define EUNATCH					49		/* Protocol driver not attached */
#define ENOCSI					50		/* No CSI structure available */
#define EL2HLT					51		/* Level 2 halted */
#define EBADE					52		/* Invalid exchange */
#define EBADR					53		/* Invalid request descriptor */
#define EXFULL					54		/* Exchange full */
#define ENOANO					55		/* No anode */
#define EBADRQC					56		/* Invalid request code */
#define EBADSLT					57		/* Invalid slot */
#define EDEADLOCK				EDEADLK /* Resource deadlock would occur */
#define EBFONT					59		/* Bad font file format */
//#define ENOSTR					60		/* Device not a stream */
//#define ENODATA					61		/* No data available */
//#define ETIME					62		/* Timer expired */
//#define ENOSR					63		/* Out of streams resources */
#define ENONET					64		/* Machine is not on the network */
#define ENOPKG					65		/* Package not installed */
#define EREMOTE					66		/* Object is remote */
//#define ENOLINK					67		/* Link has been severed */
#define EADV					68		/* Advertise error */
#define ESRMNT					69		/* Srmount error */
#define ECOMM					70		/* Communication error on send */
//#define EPROTO					71		/* Protocol error */
#define EMULTIHOP				72		/* Multihop attempted */
#define EDOTDOT					73		/* RFS specific error */
//#define EBADMSG					74		/* Not a data message */
//#define EOVERFLOW				75		/* Value too large for defined data type */
#define ENOTUNIQ				76		/* Name not unique on network */
#define EBADFD					77		/* File descriptor in bad state */
#define EREMCHG					78		/* Remote address changed */
#define ELIBACC					79		/* Can not access a needed shared library */
#define ELIBBAD				    STRUNCATE	 /* Accessing a corrupted shared library */
#define ELIBSCN					81		/* .lib section in a.out corrupted */
#define ELIBMAX					82		/* Attempting to link in too many shared libraries */
#define ELIBEXEC				83		/* Cannot exec a shared library directly */
#define ERESTART				85		/* Interrupted system call should be restarted */
#define ESTRPIPE				86		/* Streams pipe error */
#define EUSERS					87		/* Too many users */
//#define ENOTSOCK				88		/* Socket operation on non-socket */
//#define EDESTADDRREQ			89		/* Destination address required */
//#define EMSGSIZE				90		/* Message too long */
//#define EPROTOTYPE				91		/* Protocol wrong type for socket */
//#define ENOPROTOOPT				92		/* Protocol not available */
//#define EPROTONOSUPPORT			93		/* Protocol not supported */
#define ESOCKTNOSUPPORT			94		/* Socket type not supported */
//#define EOPNOTSUPP				95		/* Operation not supported on transport endpoint */
#define EPFNOSUPPORT			96		/* Protocol family not supported */
//#define EAFNOSUPPORT			97		/* Address family not supported by protocol */
//#define EADDRINUSE				98		/* Address already in use */
//#define EADDRNOTAVAIL			99		/* Cannot assign requested address */
//#define ENETDOWN			   100		/* Network is down */
//#define ENETUNREACH			   101		/* Network is unreachable */
//#define ENETRESET			   102		/* Network dropped connection because of reset */
//#define ECONNABORTED		   103		/* Software caused connection abort */
//#define ECONNRESET			   104		/* Connection reset by peer */
//#define ENOBUFS				   105		/* No buffer space available */
//#define EISCONN				   106		/* Transport endpoint is already connected */
//#define ENOTCONN			107		/* Transport endpoint is not connected */


// MS errno uses the 100- range

#define ESHUTDOWN			   1108		/* Cannot send after transport endpoint shutdown */
#define ETOOMANYREFS		   1109		/* Too many references: cannot splice */
//#define ETIMEDOUT			   110		/* Connection timed out */
//#define ECONNREFUSED		   111		/* Connection refused */
#define EHOSTDOWN			   1112		/* Host is down */
//#define EHOSTUNREACH		   113		/* No route to host */
//#define EALREADY			   114		/* Operation already in progress */
//#define EINPROGRESS			   115		/* Operation now in progress */
#define ESTALE				   1116		/* Stale NFS file handle */
#define EUCLEAN				   1117		/* Structure needs cleaning */
#define ENOTNAM				   1118		/* Not a XENIX named type file */
#define ENAVAIL				   1119		/* No XENIX semaphores available */
#define EISNAM				   1120		/* Is a named type file */
#define EREMOTEIO			   1121		/* Remote I/O error */
#define EDQUOT				   1122		/* Quota exceeded */
#define ENOMEDIUM			   1123		/* No medium found */
#define EMEDIUMTYPE			   1124		/* Wrong medium type */
//#define ECANCELED			   125		/* Operation Canceled */
#define ENOKEY				   1126		/* Required key not available */
#define EKEYEXPIRED			   1127		/* Key has expired */
#define EKEYREVOKED			   1128		/* Key has been revoked */
#define EKEYREJECTED		   1129		/* Key was rejected by service */
//#define EOWNERDEAD		   130		/* Owner died */
//#define ENOTRECOVERABLE		   131		/* State not recoverable */
#define ERESTARTSYS			   512		/* */
#define ERESTARTNOINTR		   513		/* */
#define ERESTARTNOHAND		   514		/* restart if no handler.. */
#define ENOIOCTLCMD			   515		/* No ioctl command */
#define ERESTART_RESTARTBLOCK  516		/* restart by calling sys_restart_syscall */
#define EBADHANDLE			   521		/* Illegal NFS file handle */
#define ENOTSYNC			   522		/* Update synchronization mismatch */
#define EBADCOOKIE			   523		/* Cookie is stale */
//#define ENOTSUP				   524		/* Operation is not supported */
#define ETOOSMALL			   525		/* Buffer or request is too small */
#define ESERVERFAULT		   526		/* An untranslatable error occurred */
#define EBADTYPE			   527		/* Type not supported by server */
#define EJUKEBOX			   528		/* Request initiated, but will not complete before timeout */
#define EIOCBQUEUED			   529		/* iocb queued, will get completion event */
#define EIOCBRETRY			   530		/* iocb queued, will trigger a retry */
#endif
/*
	The following ISO C99 Standard error definitions conflict with what is
	defined in the Microsoft Windows SDK header errno.h.

	Label			ISO C99		Win32 SDK		Description
	--------------	-------		---------	-------------------------------------
	EDEADLK			  35			   36	Resource deadlock would occur
	ENAMETOOLONG	  36			   38	File name too long
	ENOLCK			  37			   39	No record locks available
	ENOSYS			  38			   40	Function not implemented
	ENOTEMPTY		  39			   41	Directory not empty
	ENOMSG			  42		   EILSEQ	No message of desired type
	ELIBBAD			  80		STRUNCATE	Accessing a corrupted shared library
	EILSEQ			  84			   42	Illegal byte sequence
*/
//#define ENOMSG				   998		/* No message of desired type */

#endif
#endif


/* Limits */
#if defined (PATH_MAX)
# define XPL_MAX_PATH PATH_MAX
#elif defined (MAX_PATH)
# define XPL_MAX_PATH MAX_PATH
#elif defined (_PC_PATH_MAX)
# define XPL_MAX_PATH sysconf(_PC_PATH_MAX)
#elif defined (_MAX_PATH)
# define XPL_MAX_PATH _MAX_PATH
#else
# error "XPL_MAX_PATH is not implemented on this platform"
#endif

#if defined (NAME_MAX)
# define XPL_NAME_MAX FILENAME_MAX
#elif defined (FILENAME_MAX)
# define XPL_NAME_MAX FILENAME_MAX
#else
# error "XPL_NAME_MAX is not implemented on this platform"
#endif



#define MAXEMAILNAMESIZE	256

/* Packing/Byte order */
#if defined(LINUX)
# define Xpl8BitPackedStructure		__attribute__ ((aligned(1)))
# define Xpl64BitPackedStructure	__attribute__ ((aligned(8)))
#elif defined(MACOSX)
/* OS X appears to ignore packed */
# define Xpl8BitPackedStructure		__attribute__ ((aligned(1)))
# define Xpl64BitPackedStructure	__attribute__ ((aligned(8)))
#elif defined(WIN32)
# define Xpl8BitPackedStructure
# define Xpl64BitPackedStructure
#elif defined(NETWARE) || defined(LIBC)
# define Xpl8BitPackedStructure
# define Xpl64BitPackedStructure
#else
# error "Packing not defined on this platform"
#endif

#if XPL_BIG_ENDIAN
# define XplHostToLittle(LongWord) (((LongWord & 0xFF000000) >>24) | ((LongWord & 0x00FF0000) >> 8) | ((LongWord & 0x0000FF00) << 8) | ((LongWord & 0x000000FF) << 24))
# define XplHostToLittleLong(long) (((long & 0xFF000000) >>24) | ((long & 0x00FF0000) >> 8) | ((long & 0x0000FF00) << 8) | ((long & 0x000000FF) << 24))
# define XplHostToBigLong(n) (n)
# define XplHostToLittleShort(short) (((short & 0xFF00) >> 8) | ((short & 0x00FF) << 8))
# define XplHostToBigShort(n) (n)
#else
# define XplHostToLittle(n) (n)
# define XplHostToLittleLong(n) (n)
# define XplHostToBigLong(long) (((long & 0xFF000000) >>24) | ((long & 0x00FF0000) >> 8) | ((long & 0x0000FF00) << 8) | ((long & 0x000000FF) << 24))
# define XplHostToLittleShort(n) (n)
# define XplHostToBigShort(short) (((short & 0xFF00) >> 8) | ((short & 0x00FF) << 8))
#endif



/* C Library functions */
#ifndef  HAVE_STRTOLL
#ifdef	HAVE__STRTOI64
#define  strtoll _strtoi64
#else
#error "strtoll and  _strtoi64 are not implemented on this platform"
#endif
#endif

#ifndef HAVE_ATOLL
#ifdef HAVE__ATOI64
#define atoll _atoi64
#endif
#endif

#ifndef  HAVE_STRTOULL
#ifdef	HAVE__STRTOUI64
#define  strtoull _strtoui64
#else
#error "strtoull and _strtoui64 are not implemented on this platform"
#endif
#endif


#ifdef HAVE_STRCASECMP
# define XplStrCaseCmp(a,b) strcasecmp(a,b)
# define stricmp(a,b) strcasecmp(a,b)
#elif defined(HAVE_STRICMP)
# define XplStrCaseCmp(a,b) stricmp(a,b)
# define strcasecmp(a,b) stricmp(a,b)
#else
# error "XplStrCaseCmp is not implemented on this platform"
#endif

#ifdef HAVE_STRNCASECMP
# define XplStrNCaseCmp(a,b,c) strncasecmp(a,b,c)
# define strnicmp(a,b,c) strncasecmp(a,b,c)
#elif defined(HAVE_STRNICMP)
# define XplStrNCaseCmp(a,b,c) strnicmp(a,b,c)
# define strncasecmp(a,b,c) strnicmp(a,b,c)
#else
# error "XplStrNCaseCmp is not implemented on this platform"
#endif

#ifdef HAVE_VSNPRINTF
# define XplVsnprintf vsnprintf
#elif defined (HAVE__VSNPRINTF)
# define XplVsnprintf _vsnprintf
#endif

#ifdef HAVE_GETTIMEOFDAY
# define XplGetHighResolutionTimer() 0
# define XplGetHighResolutionTime(counter)	 {	struct timeval tOfDaYs;												 \
												gettimeofday(&tOfDaYs,NULL);										 \
												(counter) = (time_t)((tOfDaYs.tv_sec * 1000000) + tOfDaYs.tv_usec);  \
											 }
#elif defined (WIN32)


# define XplGetHighResolutionTimer() 0
# define XplGetHighResolutionTime(counter) {   LARGE_INTEGER   c;							\
													  if (QueryPerformanceCounter(&c) != 0) {	\
														 (counter) = c.LowPart;					 \
													  }											 \
											 }
#else
# error "XplGetHighResolutionTime is not implemented on this platform"
#endif


#ifndef XPL_TIMEOUT
#define XPL_TIMEOUT
#if defined(LINUX) || defined(S390RH) || defined(SOLARIS) || defined(MACOSX)
typedef int XplTimeout;
#elif defined(WIN32)
typedef struct timeval XplTimeout;
#endif
EXPORT int XplTimeoutSet( XplTimeout *timeout, int milliseconds );
EXPORT int XplTimeoutGet( XplTimeout *timeout );
#endif


#if defined(LINUX) || defined(MACOSX)

typedef struct _CrossPlatformTimer {
	unsigned long usec;
	unsigned long sec;
} XplTimer;

typedef enum {
	XPL_TIME_SEC,
	XPL_TIME_MSEC,
	XPL_TIME_USEC,
	XPL_TIME_NSEC,
} XplTimeUnit;

#define XplCPUTimer		struct timespec


#ifdef __cplusplus
extern "C" {
#endif

__inline static void
XplCPUTimerSubtract( XplCPUTimer *end, XplCPUTimer *begin, XplCPUTimer *result )
{
	if( end->tv_sec >= begin->tv_sec ) {
		if( end->tv_nsec >= begin->tv_nsec ) {
			result->tv_nsec = end->tv_nsec - begin->tv_nsec;
			result->tv_sec = end->tv_sec - begin->tv_sec;
			return;
		}

		if( end->tv_sec > begin->tv_sec ) {
			result->tv_nsec = 1000000000 + end->tv_nsec - begin->tv_nsec;
			result->tv_sec = end->tv_sec - begin->tv_sec - 1;
			return;
		}
	}
	fprintf(stderr, "XplCPUTimerSubtruct: End Time is less then Begin time\n" );

	result->tv_sec = 0;
	result->tv_nsec = 0;
	return;
}

__inline static void
XplCPUTimerAdd( XplCPUTimer *time1, XplCPUTimer *time2, XplCPUTimer *result )
{
	result->tv_sec = time1->tv_sec + time2->tv_sec;
	result->tv_nsec = time1->tv_nsec + time2->tv_nsec;

	if( result->tv_nsec > 1000000000 ) {
		result->tv_nsec -= 1000000000;
		result->tv_sec++;
	}
}


#elif defined(WIN32) || defined(WINDOWS)
typedef struct _CrossPlatformTimer {
	unsigned long usec;
	unsigned long sec;

	LARGE_INTEGER stop;
	LARGE_INTEGER start;

	XplBool usable;
} XplTimer;

typedef enum {
	XPL_TIME_SEC,
	XPL_TIME_MSEC,
	XPL_TIME_USEC,
	XPL_TIME_NSEC,
} XplTimeUnit;

#define XplCPUTimer		int

#ifdef __cplusplus
extern "C" {
#endif


__inline static void
XplCPUTimerSubtract( XplCPUTimer *end, XplCPUTimer *begin, XplCPUTimer *result )
{
	if( end >= begin ) {
		*result = *end - *begin;
		return;
	}
	*result = 0;
	return;
}

__inline static void
XplCPUTimerAdd( XplCPUTimer *time1, XplCPUTimer *time2, XplCPUTimer *result )
{
	*result = *time1 + *time2;
}


#elif defined(NETWARE) || defined(LIBC)
typedef struct _CrossPlatformTimer {
	unsigned long usec;
	unsigned long sec;
} XplTimer;

#define XplCPUTimer		int
#else
#error No cross platform timer implemented on this platform.
#endif

__inline static void
XplTimerSubtract( XplTimer *end, XplTimer *begin, XplTimer *result )
{
	if( end->sec >= begin->sec ) {
		if( end->usec >= begin->usec ) {
			result->usec = end->usec - begin->usec;
			result->sec = end->sec - begin->sec;
			return;
		}

		if( end->sec > begin->sec ) {
			result->usec = 1000000 + end->usec - begin->usec;
			result->sec = end->sec - begin->sec - 1;
			return;
		}
	}
	fprintf(stderr, "XplTimerSubtract: End Time is less then Begin time\n" );

	result->sec = 0;
	result->usec = 0;
	return;
}

__inline static void
XplTimerAdd( XplTimer *time1, XplTimer *time2, XplTimer *result )
{
	result->sec = time1->sec + time2->sec;
	result->usec = time1->usec + time2->usec;

	if( result->usec > 1000000 ) {
		result->usec -= 1000000;
		result->sec++;
	}
}

typedef struct {
	XplTimer total;
	XplTimer max;
	XplTimer min;
	unsigned long count;
} XplTimerAccumulator;

EXPORT int XplCPUTimerCmp( XplCPUTimer *left, XplCPUTimer *right );
EXPORT void XplCPUTimerDivide( XplCPUTimer *dividend, unsigned int divisor, XplCPUTimer *quotient );
EXPORT unsigned long XplCPUTimerInt( XplCPUTimer *timer, XplTimeUnit units );
EXPORT void XplCPUThreadTimerStart(XplCPUTimer *timer);
EXPORT void XplCPUThreadTimerStop(XplCPUTimer *timer);
EXPORT unsigned long XplTimerInt( XplTimer *timer, XplTimeUnit units );
EXPORT int XplTimerCmp( XplTimer *left, XplTimer *right );
EXPORT void XplTimerDivide( XplTimer *dividend, unsigned int divisor, XplTimer *quotient );
EXPORT void XplTimerStart(XplTimer *timer);
EXPORT void XplTimerStop(XplTimer *timer);
EXPORT XplTimer *XplTimerSplit(XplTimer *startTime, XplTimer *splitTime );
EXPORT XplTimer *XplTimerLap(XplTimer *lastTime, XplTimer *lapTime );
EXPORT XplTimer *XplTimerSplitAndLap( XplTimer *startTime, XplTimer *splitTime, XplTimer *lastTime, XplTimer *lapTime );
EXPORT void XplTimerAccumulate( XplTimerAccumulator *accum, XplTimer *timer );
EXPORT void XplTimerAverage( XplTimerAccumulator *accum, XplTimer *out );
EXPORT void XplTimerAccumulatorPrint( XplTimerAccumulator *accum, char *label );
EXPORT void XplTimedCheckStart(XplTimer *timer);
EXPORT void XplTimedCheckStop(XplTimer *timer, const char* checkName, long checkRate, XplAtomic *abortTrigger);

#ifdef __cplusplus
}
#endif

/* UI Definitions - need to reconsider these*/

#ifdef HAVE_RINGTHEBELL
# define XplBell() XplBell()
#else
# define XplBell()
#endif

#if defined __GNUC__
#define XplFormatString(formatStringIndex, firstToCheck)	__attribute__ ((format (printf, formatStringIndex, firstToCheck)))
#else
#define XplFormatString(formatStringIndex, firstToCheck)
#endif

#if defined WIN32 && (defined DEBUG || defined _DEBUG)

// #define XPL_CONSOLE_TO_FILE

#ifdef __cplusplus
extern "C" {
#endif

EXPORT void XplDebugOut(const char *Format, ...) XplFormatString(1, 2);
EXPORT void _DebugPrintErr(const char *format, ...) XplFormatString(1, 2);

#ifdef __cplusplus
}
#endif

//# define	XplConsolePrintf				  XplDebugOut

#if defined(XPL_CONSOLE_TO_FILE)
# define XplConsolePrintf					XplConsoleToFile
# define XplConsoleDebugPrintf				consoleprintf
#else
# define  XplConsolePrintf					printf
# define  XplConsoleDebugPrintf				consoleprintf
#endif

#elif defined HAVE_CONSOLEPRINTF
# define XplConsolePrintf consoleprintf
# define XplConsoleDebugPrintf consoleprintf
#else
# define XplConsolePrintf printf
# define XplConsoleDebugPrintf _XplDebugPrintf
#endif

#if defined(DEBUG)
# if defined(WINDOWS) /* to get logs for debbuging */
#  define DebugPrintf XplConsolePrintf
# else
#  define DebugPrintf XplConsoleDebugPrintf
# endif
#else
# define DebugPrintf if(0) printf
#endif


#if defined(DEBUG)
# define DebugPrintErr(fmt, ...) fprintf(stderr, (fmt), ##__VA_ARGS__)
#else
# define DebugPrintErr(fmt, ...) if (0) fprintf(stderr, (fmt), ##__VA_ARGS__)
#endif

#if defined (WINDOWS)
# define XplClearScreen() system("cls")
#else
# define XplClearScreen() system("clear")
#endif

#ifdef HAVE_CREATESCREEN
# define XplCreateScreen(a,b) CreateScreen(a,b)
#else
# define XplCreateScreen(a,b) 1
# define DONT_CHECK_CTRL_CHARS 0
# define AUTO_DESTROY_SCREEN   0
#endif

#ifdef HAVE_DISPLAYSCREEN
# define XplDisplayScreen(number) DisplayScreen(number)
#else
# define XplDisplayScreen(number)
#endif

#ifdef HAVE_DESTROYSCREEN
# define XplDestroyScreen(number) DestroyScreen(number)
#else
# define XplDestroyScreen(number)
#endif

#ifdef HAVE_HIDEINPUTCURSOR
# define XplHideInputCursor() HideInputCursor
#else
# define XplHideInputCursor()
#endif

#ifdef HAVE_GOTOXY
# define XplGotoXY(x,y) gotoxy(x,y)
#else
# define XplGotoXY(x,y)
#endif



#ifdef HAVE_SETCURRENTSCREEN
# define XplSetCurrentScreen(number) SetCurrentScreen(number)
#else
# define XplSetCurrentScreen(number)
#endif

#ifdef HAVE_UNGETCHARACTER
# define XplUngetCh(a) ungetcharacter((a))
#else
# define XplUngetCh(a)
#endif


/* File and Directory Functions */

#if defined(LINUX) || defined(MACOSX)

typedef struct _XplDir {
   unsigned long  d_attr;
   unsigned long  d_size;
   unsigned char *d_name;
   unsigned long  d_cdatetime;
   DIR *dirp;
   struct dirent *direntp;
   unsigned char Path[XPL_MAX_PATH];
} XplDir;

#ifdef __cplusplus
extern "C"{
#endif

int XplMakeDir(const char *path);

#ifdef __cplusplus
}
#endif

#elif defined(WIN32)

#include <direct.h>
#include <io.h>

typedef struct _XplDir {
   unsigned long		 d_attr;
   unsigned long		 d_size;
   unsigned char		 *d_name;
   unsigned long		 d_cdatetime;
   long					 dirp;
   struct _finddata_t	FindData;
   unsigned char		 Path[_MAX_PATH+1];
} XplDir;

#define   XplMakeDir(path) mkdir(path)

#endif

typedef struct _XplDirMatch {
   struct _XplDirMatch *next;
   struct _XplDirMatch *base;

   unsigned long d_attr;
   unsigned long d_size;
   unsigned long d_cdatetime;

   unsigned char *d_name;

	struct {
		void *data;
		void *lock;
	} client;
} XplDirMatch;

#ifdef __cplusplus
extern "C" {
#endif

EXPORT XplDir *XplOpenDir(const char  *dirname);
EXPORT XplDir *XplReadDir(XplDir *dirp);
EXPORT int XplCloseDir(XplDir *dirp);
EXPORT int XplIsSubDir(XplDir *dirp);

EXPORT XplDirMatch *XplOpenDirMatch(const char *pattern);
EXPORT XplDirMatch *XplReadDirMatch(XplDirMatch *dirp);
EXPORT XplDirMatch *XplResetDirMatch(XplDirMatch *dirp);
EXPORT int XplCloseDirMatch(XplDirMatch *dirp);
EXPORT void XplMakePath(const char *path);
EXPORT int XplStat( const char *path, struct stat *st );


#if defined(WIN32)
	#define XplGetCurrentDir _getcwd
#else
	#define XplGetCurrentDir getcwd
#endif

typedef struct
{
	struct stat		st;
	char			d_name[256];
}XDirEnt;

typedef struct
{
	void	*dirp;
	void	(*XFree)(void *);
	XDirEnt	entry;
	char	*pattern;
	char	name[];
}XDir;

EXPORT XDir *XOpenDir( const char *path, void *(*XAlloc)(size_t), void (*XFree)(void *) );
EXPORT XDirEnt *XReadDir( XDir *dir );
EXPORT int XCloseDir( XDir *dir );

#ifdef HAVE_FSYNC
# define XplFlushWrites(fileptr) fsync(fileno(fileptr))
#elif defined (HAVE_FEFLUSHWRITE)
# define XplFlushWrites(fileptr) FEFlushWrite(fileno(fileptr))
#elif defined (HAVE__COMMIT)
# define XplFlushWrites(fileptr) _commit(fileno(fileptr))
#elif defined (HAVE_FFLUSH)
# define XplFlushWrites(fileptr) fflush(fileptr)
#else
# error "XplFlushWrites not defined for this platform"
#endif

#if 0
GetFileInformationByHandle
	lstat!
#endif

#if !defined (S_IFMT)
#define S_IFMT	_S_IFMT
#endif

#if !defined(S_IFBLK)
#if defined(_S_IFBLK)
#define S_IFBLK	_S_IFBLK
#else
#define S_IFBLK	0
#endif
#endif

#if !defined(S_IFCHR)
#define S_IFCHR	_S_IFCHR
#endif

#if !defined(S_IFIFO)
#define S_IFIFO	_S_IFIFO
#endif

#if !defined(S_IFREG)
#define S_IFREG	_S_IFREG
#endif

#if !defined(S_IFDIR)
#define S_IFDIR	_S_IFDIR
#endif

#if !defined(S_IFLNK)
#if defined(_S_IFLNK)
#define S_IFLNK	_S_IFLNK
#else
#define S_IFLNK	0
#endif
#endif

#if !defined(S_IFSOCK)
#define S_IFSOCK	_S_IFSOCK
#endif

#if !defined(S_IREAD)
#define S_IREAD	_S_IREAD
#endif

#if !defined(S_IWRITE)
#define S_IWRITE	_S_IWRITE
#endif

#if !defined(S_IEXEC)
#define S_IEXEC	_S_IEXEC
#endif

#if !defined(S_ISDIR)
#if defined(S_IFMT) && defined(S_IFDIR)
#define S_ISDIR(x)	(S_IFDIR == ((x) & S_IFMT))
#else
#error "S_ISDIR() is not defined on this platform"
#endif
#endif

#if !defined(S_ISREG)
#if defined(S_IFMT) && defined(S_IFREG)
#define S_ISREG(x)	(S_IFREG == ((x) & S_IFMT))
#else
#error "S_ISREG() is not defined on this platform"
#endif
#endif

#if !defined(S_ISLNK)
#if defined(S_IFMT) && defined(S_IFLNK)
#define S_ISLNK(x)	(S_IFLNK == ((x) & S_IFMT))
#else
#error "S_ISLNK() is not defined on this platform"
#endif
#endif

/* Utility Functions */
#if !defined (_MSC_VER)
EXPORT char *strlwr( char *s );
EXPORT char *strupr( char *s );
#endif
#define strlower( s )	strlwr( (s) )
#define strupper( s )	strupr( (s) )
#define XplStrLower(s) strlwr((s))

EXPORT char * strstrn(const char *haystack, const char *needle, size_t len);
EXPORT char * strnistr(const char *haystack, const char *needle, size_t len);
EXPORT char * strichr(const char *haystack, const char needle);
EXPORT char * stristrn(const char *haystack, const char *needle, size_t len);
#define stristr(h, n) stristrn((h), (n), strlen((n)))
EXPORT int strpat(const char *str, const char *pattern);
EXPORT int strpatn(const char *str, const char *pattern, size_t len);
EXPORT int stripat(const char *str, const char *pattern);
EXPORT int stripatn(const char *str, const char *pattern, size_t len);
EXPORT char *strspace( const char *source );
EXPORT char *strrspace( const char *source );
EXPORT char *_skipspace( const char *source, const char *breakchars );
#define skipspace(s) _skipspace((s), "\r\n")
EXPORT char *chopspace( char *value );
EXPORT char *removespace( char *source, char *dest, int destsize );

/*
	Split the provided line into a name value pair, assuming a seperator
	character of : or =.

	Leading and trailing whitespace will be removed from both the name and the
	value.

	NOTE: This function is destructive. The contents of line will be modified.
*/
EXPORT int namevalue(char *line, char **name, char **value);

/*
	Return the next field in the string 'value', and set 'end' to the first
	character after that field.

	NOTE: This function will modify the string that is specified by value.

	Example:
		If "line" starts as: 'foo "smeg head" bar' then this code:

			while ((field = nextfield(line, &line))) {
				printf("\"%s\" ", field);
			}
			printf("\n");

		will print:
			"foo" "smeg head" "bar"

	The nextfield() version will leave quotes within the argument, unless the
	argument starts with a quote character or the quote is escaped. The
	nextargument() version will always strip unescaped quotes.
*/
EXPORT char * nextfield(char *value, char **end);
EXPORT char * nextargument(char *value, char **end);
EXPORT char * nextfieldex(char *value, char **end, XplBool stripquotes, char escape);

/*
	strxcmp and strxicmp behave exactly like strncmp and strnicmp, except that
	the length is automatically determined by the length of the second argument.

	If the second argument is NULL then -1 is returned.
*/
#define strxcmp(s1, s2)		(s2 ? strncmp (s1, s2, strlen(s2)) : -1)
#define strxicmp(s1, s2)	(s2 ? strnicmp(s1, s2, strlen(s2)) : -1)

/*
	str safe cmp

	strscmp and strsicmp behave exactly like strcmp and stricmp, except that the
	arguments may be NULL. A NULL string is considered a match to another NULL
	string.
*/
#define strscmp(s1, s2)		((!s1 || !s2) ?									\
								((!s1 && !s2) ? 0 : -1) :					\
								strcmp(s1, s2))

#define strsicmp(s1, s2)	((!s1 || !s2) ?									\
								((!s1 && !s2) ? 0 : -1) :					\
								stricmp(s1, s2))

EXPORT size_t vstrcatf( char *buffer, size_t bufferSize, size_t *sizeNeeded, const char *format, va_list args );
EXPORT size_t vstrprintf( char *buffer, size_t bufferSize, size_t *sizeNeeded, const char *format, va_list args );
EXPORT size_t strprintf( char *buffer, size_t bufferSize, size_t *sizeNeeded, const char *format, ... ) XplFormatString(4, 5);
EXPORT size_t strcatf( char *buffer, size_t bufferSize, size_t *sizeNeeded, const char *format, ... ) XplFormatString(4, 5);
EXPORT time_t strtotime( char *str, char **end, int radix );
EXPORT uint64 strtobytes( char *str, char **end, int radix );
EXPORT char *strchrs( char *str, char *chr );
EXPORT char *strrchrs( char *str, char *chr );

EXPORT char *_strtok_r_( char *s1, const char *s2, char **s3 );
#ifndef strtok_r
#define strtok_r _strtok_r_
#endif

/* Statistics Functions */
EXPORT uint64			XplGetMemAvail (void);
EXPORT uint64			XplGetMemFree(uint64 *total);
EXPORT unsigned int		XplGetCPUUsage(void);
EXPORT int				XplGetProcessCpuUsage(XplPid pid, long sampleMs);
EXPORT unsigned long	XplGetProcessMemoryUsage(XplPid pid);
EXPORT long				XplGetServerUtilization(void);
EXPORT uint64			XplGetDiskspaceUsed(unsigned char *Path);
EXPORT uint64			XplGetDiskspaceFree(unsigned char *Path);
EXPORT unsigned long	XplGetDiskBlocksize(unsigned char *Path);

/* Shared Library functions */
/* rough cut */

#if defined(WINDOWS) || defined(WIN32)
# define XPL_DLL_EXTENSION ".dll"
# define XPL_DLL_PREFIX ""
#elif defined (NETWARE)
# define XPL_DLL_EXTENSION ".nlm"
# define XPL_DLL_PREFIX ""
#elif defined (MACOSX)
/* This should be .dylib, but the modules get built as .so on os x */
# define XPL_DLL_EXTENSION ".so"
# define XPL_DLL_PREFIX "lib"
#else
# define XPL_DLL_EXTENSION ".so"
# define XPL_DLL_PREFIX "lib"
#endif

#ifdef HAVE_DLFCN_H
#include <dlfcn.h>

typedef void *XplPluginHandle;
XplPluginHandle XplLoadDLL(const char *path);
XplPluginHandle XplLoadDLLLocal(const char *path);
# define XplGetDLLFunction(Function, Handle)			dlsym((Handle), (Function))

/* Unloading a module with valgrind prevent's it's symbols from showing up in the error reports */
# ifdef HAVE_VALGRIND_H
#  include <valgrind/valgrind.h>
#  define XplUnloadDLL(Handle)							if (!RUNNING_ON_VALGRIND && Handle) dlclose((Handle))
# else
#  define XplUnloadDLL(Handle)							if (Handle) dlclose((Handle))
# endif

# define FARPROC										void *
#elif defined (WINDOWS)

typedef HINSTANCE XplPluginHandle;

EXPORT XplPluginHandle XplLoadDLL(const char *path);		/* Defined in libxpl */
# define XplLoadDLLLocal(path)							XplLoadDLL(path)

EXPORT void *Win32GetFunction(XplPluginHandle handle, const char *function);
#define XplGetDLLFunction(Function, Handle)				Win32GetFunction((Handle), (Function))
#define XplUnloadDLL(Handle)							FreeLibrary(Handle);

#endif

/* Internationalization */
#ifdef WIN32
int XplGetCurrentOSLangaugeID(void);
#else
# define XplGetCurrentOSLanguageID() 4
#endif

EXPORT int XplReturnLanguageName(int lang, unsigned char *buffer);

/* Time */
#ifdef HAVE__CONVERTDOSTIMETOCALENDAR
# define XplCalendarTime(time) _ConvertDOSTimeToCalendar(time)
#elif defined (HAVE_DOS2CALENDAR)
# define XplCalendarTime(time) dos2calendar(time)
#else
# define XplCalendarTime(time) (time)
#endif

/* structure math */
#ifndef __WATCOMC__
#ifndef offsetof
#define offsetof( type, member )				__builtin_offsetof( type, member )
#endif
#endif
#define XplOffsetOf( type, member )				offsetof( type, member)
#define XplParentOf( ptr, type, member )		(type *)((char *)ptr - offsetof( type, member ))


EXPORT XplBool XplSetCoreBehavior( void );
/* Signal handling */


#if 0
#ifdef WIN32
#define XplUnloadApp(id)   TerminateProcess(GetCurrentProcess(),0);
//#define XplUnloadApp(id)		 ExitProcess(0);
#else
#define XplUnloadApp(ID)   kill((ID), SIGKILL)
#endif
#endif

/* Read/Write locks */
typedef struct {
   long				RWState;
   unsigned long	RWMode;
   unsigned long	RWReaders;
   XplSemaphore		RWLock;
   XplSemaphore		RWMutexRead;
   XplSemaphore		RWMutexReadWrite;
} XplRWLock;

EXPORT XplBool   XplRWLockInit(XplRWLock *RWLock);
EXPORT XplBool   XplRWLockDestroy(XplRWLock *RWLock);
EXPORT XplBool   XplRWReadLockAcquire(XplRWLock *RWLock);
EXPORT XplBool   XplRWWriteLockAcquire(XplRWLock *RWLock);
EXPORT XplBool   XplRWReadLockRelease(XplRWLock *RWLock);
EXPORT XplBool   XplRWWriteLockRelease(XplRWLock *RWLock);

/* Windows specifics */

#if defined(HAVE__SNPRINTF) && !defined(HAVE_SNPRINTF)
#define snprintf _snprintf
#endif


/* Strok Function */
#if defined(LINUX) || defined(MACOSX)
#define XplStrTokR(string, delimiters, ptr)  strtok_r(string, delimiters, ptr)
#elif defined(WIN32)
#define XplStrTokR(string, delimiters, ptr)  strtok(string, delimiters)
#else
#error string token parser not defined!
#endif

#ifdef __WATCOMC__
#include<signal.h>
#define SIGHUP	SIGBREAK
#if 0
_WCRTLINK extern int  raise( int __sig );
_WCRTLINK extern void (*signal( int __sig, void (*__func)(int) ) )(int);

#define SIGHUP		1	/* hangup */
#define SIGINT		2	/* interrupt */
#define SIGQUIT		3	/* quit */
#define SIGILL		4	/* illegal instruction (not reset when caught) */
#define SIGTRAP		5	/* trace trap (not reset when caught) */
#define SIGIOT		6	/* IOT instruction */
#define SIGABRT		6	/* used by abort */
#define SIGBUS		7	/* bus error */
#define SIGFPE		8	/* floating point exception */
#define SIGKILL		9	/* kill (cannot be caught or ignored) */
#define SIGUSR1		10	/* user defined signal 1 */
#define SIGSEGV		11	/* segmentation violation */
#define SIGUSR2		12	/* user defined signal 2 */
#define SIGPIPE		13	/* write on pipe with no reader */
#define SIGALRM		14	/* real-time alarm clock */
#define SIGTERM		15	/* software termination signal from kill */
#define SIGCHLD		17	/* death of child */
#define SIGCONT		18	/* continue a stopped process */
#define SIGSTOP		19	/* sendable stop signal not from tty */
#define SIGTSTP		20	/* stop signal from tty */
#define SIGTTIN		21	/* attempted background tty read */
#define SIGTTOU		22	/* attempted background tty write */
#define SIGURG		23	/* urgent condition on I/O channel */
#define SIGVTALRM	26	/* virtual alarm clock */
#define SIGPROF		27	/* profiler */
#define SIGWINCH	28	/* window change */
#define SIGIO		29	/* Asynchronus I/O */
#define SIGPOLL		29	/* System V name for SIGIO */
#define SIGPWR		30	/* power-fail restart */
#define SIGSYS		31	/* bad argument to system call */
#define _SIGMAX		32
#endif

#endif


/* config file */
#ifndef CFG_INTERNAL
typedef void CFGFile;

EXPORT CFGFile *XplCFGOpen( char *name );
EXPORT CFGFile *XplCFGCreate( char *name );
EXPORT int XplCFGFlush( CFGFile *config );
EXPORT int XplCFGClose( CFGFile *config );
EXPORT char *XplCFGGetValue( CFGFile *config, char *sectionName, char *name );
EXPORT int XplCFGSetValue( CFGFile *config, char *sectionName, char *name, char *value );
#endif

/*
	Macro Helpers

	XPL_MACRO_VALUE appears to do the same thing as XPL_MACRO_NAME but since the
	preprocessor has already evaluated s before calling XPL_MACRO_NAME(), s will
	contain the value of the macro by the time XPL_MACRO_NAME is called.
*/
#define XPL_MACRO_VALUE(s)	XPL_MACRO_NAME(s)
#define XPL_MACRO_NAME(s)	#s


/* Get the ASSERT_<type> define */
#include <xplutil-assert.h>

#if defined(ASSERT_TYPE_CRASH) || defined(ASSERT_TYPE_THREAD) || defined(ASSERT_TYPE_PRINT) || defined(ASSERT_TYPE_BREAK)
# define DEBUG_ASSERT				1
#endif

#if defined(LINUX)
void SigTrapIgnoreHandler(int signum);
void SigTrapAttachHandler(int signum);
#endif

/*
	DebugAssert(condition)

	If the provided condition is not true, then perform a debug action, which
	will vary depending on the build option specified:

		none		Do nothing

		crash		Stop execution and cause a crash in order to cause a
					debugger to stop or to generate a core file for later
					debugging.

		thread		Fork a child thread, and cause it to crash, allowing the
					main program to continue while the child thread is debugged.

		print		Display a warning on stdout.

		break		Break if a debugger is attached, otherwise print a warning.
*/
#if defined(DEBUG_ASSERT)
# if defined(ASSERT_TYPE_BREAK)
#  if defined(LINUX)
#   define DebugAssert(x) if (!(x)) {										\
		int DebugAssertErrno = errno;										\
		fprintf(stderr, "WARNING: Assert failed at %s:%d (errno: %d)\n",	\
			__FILE__, __LINE__, DebugAssertErrno);							\
		signal(SIGTRAP, SigTrapIgnoreHandler);								\
		__asm__("int $3\n" : : );											\
	}
#  elif defined(WINDOWS)
#   define DebugAssert(x) if (!(x)) {										\
		int DebugAssertErrno = errno;										\
		fprintf(stderr, "WARNING: Assert failed at %s:%d (errno: %d)\n",	\
			__FILE__, __LINE__, DebugAssertErrno);							\
		if (IsDebuggerPresent()) {											\
			__debugbreak();													\
		}																	\
	}
#  endif
# elif defined(ASSERT_TYPE_THREAD)
#  define DebugAssert(x) if (!(x)) {										\
		int DebugAssertErrno = errno;										\
		if (!fork()) {														\
			fprintf(stderr, "WARNING: Assert failed at %s:%d (errno: %d)\n",\
				__FILE__, __LINE__, DebugAssertErrno);						\
			*((char *)NULL) = (char) DebugAssertErrno;						\
		}																	\
	}
# elif defined(ASSERT_TYPE_PRINT)
#  define DebugAssert(x) if (!(x)) {										\
		int DebugAssertErrno = errno;										\
		fprintf(stderr, "WARNING: Assert failed at %s:%d (errno: %d)\n",	\
			__FILE__, __LINE__, DebugAssertErrno);							\
	}
# else
#  define DebugAssert(x) if (!(x)) {										\
		int DebugAssertErrno = errno;										\
		fprintf(stderr, "WARNING: Assert failed at %s:%d (errno: %d)\n",	\
			__FILE__, __LINE__, DebugAssertErrno);							\
		*((char *)NULL) = (char) DebugAssertErrno;							\
	}
# endif
#else
# define DebugAssert( x )
#endif

#define CriticalAssert(x) if (!(x)) {										\
		int DebugAssertErrno = errno;										\
		fprintf(stderr, "WARNING: Assert failed at %s:%d (errno: %d)\n",	\
			__FILE__, __LINE__, DebugAssertErrno);							\
		*((char *)NULL) = (char) DebugAssertErrno;							\
}

/*
	DebugBreak()

	Break if a debugger is attached, otherwise do nothing.
*/
#if defined(LINUX)
# define DebugBreak()														\
	signal(SIGTRAP, SigTrapIgnoreHandler);									\
	__asm__("int $3\n" : : );

#elif defined(WINDOWS)
# define DebugBreak()														\
	if (IsDebuggerPresent()) {												\
		__debugbreak();														\
	}
#else
# define DebugBreak()
#endif

__inline static char *PrintableHexStringAlloc( char *charString, unsigned int charStringLen )
{
	unsigned int idx;
	char *buffer;
	unsigned int bufferSize;
	char *ptr;

	bufferSize = ( 3 * charStringLen ) + 1; // ' xx' (for each) + '\0'

	buffer = (char *)malloc( bufferSize );
	if( buffer ) {
		ptr = buffer;

		for( idx = 0; idx < charStringLen; idx++ ) {
			strprintf( ptr, bufferSize - ( ptr - buffer ), NULL, " %02X", (unsigned char)charString[ idx ] );
			ptr += 3;
		}
		*ptr = '\0';
	}
	return buffer;
}

__inline static void PrintableHexStringFree( char *printableString )
{
	free( printableString );
}

/* Alarm API */
typedef void (* XplAlarmCallback)(void *XplAlarmCallbackData);
typedef struct {
	XplAlarmCallback	callback;
	void				*data;
} XplAlarmPublic;

typedef XplAlarmPublic * XplAlarmP;
typedef XplAlarmPublic XplAlarm;
typedef void * XplAlarmHandle;

EXPORT XplAlarmHandle _XplAlarmInit(const char *file, uint32 line);
#define XplAlarmInit() _XplAlarmInit(__FILE__, __LINE__)
EXPORT void XplAlarmShutdown(XplAlarmHandle *handle);

#ifdef LINUX
EXPORT void XplAlarmSignal(int signal);
#endif

EXPORT XplAlarmP _XplAlarmCreate(XplAlarmCallback callback, void *data, time_t delay, time_t repeat, XplAlarmHandle handle, const char *file, uint32 line);
#define XplAlarmCreate(c, d, D, R, h) _XplAlarmCreate((c), (d), (D), (R), (h), __FILE__, __LINE__)
EXPORT void XplAlarmDestroy(XplAlarmP alarm);

/* Logging facilities */
EXPORT int XplConsoleToFile(const char *format, ...) XplFormatString(1, 2);

EXPORT int _XplDebugPrintf(char *format, ...);

typedef struct
{
	XplMutex			mutex;
	FILE				*file;
}ErrorLogFile;

EXPORT ErrorLogFile *ErrorLogOpen( const char *name );
EXPORT int ErrorLogClose( ErrorLogFile **log );
EXPORT int ErrorLog( ErrorLogFile *log, const char *format, ... );

#ifdef __cplusplus
}
#endif

#endif
