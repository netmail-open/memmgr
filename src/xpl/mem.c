/****************************************************************************
 *
 * Copyright (c) 1998-2002 Novell, Inc.
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

#include <memmgr-config.h>
#include <xpl.h>

#if defined(SOLARIS) || defined(LINUX) || defined(S390RH) || defined(MACOSX)

#include <sys/resource.h>
#include <signal.h>

#endif

#if defined(LINUX)
#include <sys/sysinfo.h>
#include <sys/time.h>
#ifdef HAVE_SYS_STATVFS_H
#include <sys/statvfs.h>
#endif
#ifdef HAVE_SYS_VFS_H
#include <sys/vfs.h>
#endif

#if defined(LINUX) || defined(S390RH) || defined(MACOSX)
#ifndef __USE_GNU
#define __USE_GNU
#endif
#include <pthread.h>
#undef __USE_GNU
#endif

#if 1
#define	PROC_SUPER_MAGIC 0x9fa0
#define	PROCFS "/proc"

uint64
XplGetMemFree(uint64 *total)
{
    uint64			memfree		= 0;
    uint64			memcached	= 0;
	struct statfs	sfb;
	FILE			*info;
	char			buffer[1024];
	char			*value;

	if (statfs(PROCFS, &sfb) || PROC_SUPER_MAGIC != sfb.f_type) {
		XplConsolePrintf("XPL: PROC file system is not mounted\r\n");
		return(0);
	}

	if (!(info = fopen(PROCFS "/meminfo", "rb"))) {
		XplConsolePrintf("XPL: PROC file system could not be read.\r\n");
		return(0);
	}

	if (total) {
		*total = 0;
	}

	while (!feof(info) && !ferror(info)) {
		if (!fgets(buffer, sizeof(buffer), info)) {
			continue;
		}

		if (!(value = strchr(buffer, ':'))) {
			continue;
		}
		*value = '\0';
		for (value++; isspace(*value); value++);

		if (total && !stricmp(buffer, "MemTotal")) {
			*total = strtoull(value, NULL, 10) * 1024;
		} else if (!stricmp(buffer, "MemFree")) {
			memfree = strtoull(value, NULL, 10) * 1024;
		} else if (!stricmp(buffer, "Cached")) {
			memcached = strtoull(value, NULL, 10) * 1024;
		}
	}
	fclose(info);
	return(memfree + memcached);
}

#else

EXPORT uint64 XplGetMemFree(uint64 *total)
{
    int ccode;
    struct sysinfo si;

    ccode = sysinfo(&si);
    if (!ccode) {
        if (total) {
            *total = si.totalram;
        }

        return(si.freeram);
    }

    XplConsolePrintf("Xpl: Unable to read system information: error %d\r\n", ccode);

    return(0);
}
#endif

#elif defined(WIN32)

EXPORT uint64 XplGetMemFree(uint64 *total)
{
	MEMORYSTATUS status;

	GlobalMemoryStatus( &status );
	if( total )
	{
		*total = status.dwTotalPhys;
	}
	return status.dwAvailPhys;
}

#elif defined(MACOSX)

uint64
XplGetMemFree(uint64 *total)
{
    if (total) {
        *total = (1024 * 1024 * 2);
    }

	return(1024 * 1024 * 2);
}

#else

/* Not implemented here - lie for now */
uint64
XPlGetMemFree(uint64 *total)
{
    if (total) {
        *total = 256 * 1024 * 1024;
    }

    return(256 * 1024 * 1024);
}

#endif

#if defined(LINUX) || defined(S390RH) || defined(MACOSX)

XplBool XplSetCoreBehavior( void )
{
	FILE *f;
	XplBool ret;

	ret = TRUE;
	if(f = fopen("/proc/sys/kernel/core_pattern", "w")) {
		if(fputs("core-%e-%p\n", f) == EOF) {
			ret = FALSE;
		} else {
			ret = TRUE;
		}
		fclose(f);
	}
	else {
		ret = FALSE;
	}
	return ret;
}

uint64 XplGetMemAvail(void)
{
	struct rlimit	rlp;

#if defined(RLIMIT_AS)
	getrlimit(RLIMIT_AS, &rlp);
#elif defined(RLIMIT_DATA)
	getrlimit(RLIMIT_DATA, &rlp);
#elif defined(RLIMIT_VMEM)
	getrlimit(RLIMIT_VMEM, &rlp);
#else
#error XplGetMemAvail not ported.
#endif
	return(rlp.rlim_cur);
}

EXPORT XplPluginHandle XplLoadDLL(const char *path)
{
    void		*handle;
	char		expath[XPL_MAX_PATH];

	if (!XplExpandEnv(expath, path, sizeof(expath))) {
		path = (const char *) expath;
	}

	/* NM-13976 Do not allow loading an .rpmsave file */
	if (!stripat(path, "*.rpmsave") || !stripat(path, "*.rpmnew")) {
		return(NULL);
	}

	if (!(handle = dlopen(path, RTLD_NOW | RTLD_GLOBAL))) {
        struct stat sb;

        /*
			Print an error if there is really an error loading, but attempt to
			filter out the more common ones that are caused by loading all files
			in a directory and trying to load files that don't exist.
        */
        if (path[strlen(path) - 3] == '.' && toupper(path[strlen(path) - 2]) == 'S' && toupper(path[strlen(path) - 1]) == 'O' && !stat(path, &sb)) {
#if defined(LINUX)
            unsigned char *error = dlerror();

            if (error) {
                XplConsolePrintf("Failed to open %s: %s\n", path, error);
            }
#endif
        }
    }

    return(handle);
}

EXPORT XplPluginHandle XplLoadDLLLocal(const char *path)
{
    void * handle = (void *) dlopen(path, RTLD_NOW | RTLD_LOCAL);

    if (!handle) {
        struct stat sb;

        if (path[strlen(path) - 3] == '.' && toupper(path[strlen(path) - 2]) == 'S' && toupper(path[strlen(path) - 1]) == 'O' && !stat(path, &sb)) {
#if defined(LINUX)
            unsigned char *error = dlerror();

            if (error) {
                XplConsolePrintf("Failed to open %s: %s\n", path, error);
            }
#endif
        }
    }

    return(handle);
}

#endif

const unsigned char CAN[] = { 'F', 'r', 'a', 'n', 0xC3, 0xA7, 'a', 'i', 's', ' ', 'C', 'A', 'N', 0 };
const unsigned char CHS[] = { 0xE7, 0xAE, 0x80, 0xE4, 0xBD, 0x93, 0xE4,
										0xB8, 0xAD, 0xE6, 0x96, 0x87, 0x20, ' ', 'C', 'H', 'S', 0 };

const unsigned char FRA[] = { 'F', 'r', 'a', 'n', 0xC3, 0xA7, 'a', 'i', 's', ' ', 'F', 'R', 'A', 0 };
const unsigned char JPN[] = { 0xE6, 0x97, 0xA5, 0xE6, 0x9C, 0xAC, 0xE8, 0xAA, 0x9E, ' ', 'J', 'P', 'N', 0};
const unsigned char KOR[] = { 0xED, 0x95, 0x9C, 0xEA, 0xB5, 0xAD, 0xEC, 0x96, 0xB4, ' ', 'K', 'O', 'R', 0};
const unsigned char BRA[] = { 0x50, 0x6F, 0x72, 0x74, 0x75,  0x67, 0x75, 0xC3, 0xAA, 0x73, 0x20,
										0x64, 0x6F,  0x20, 0x42, 0x72, 0x61, 0x73, 0x69, 0x6C, ' ', 'P', 'T', 'B', 0};
const unsigned char RUS[] = { 0xD1, 0x80, 0xD1, 0x83, 0xD1,  0x81, 0xD1, 0x81, 0xD0, 0xBA, 0xD0,
										0xB8, 0xD0, 0xB9, ' ', 'R', 'U', 'S', 0};
const unsigned char SA[] =  { 0x45, 0x73, 0x70, 0x61, 0xC3,  0xB1, 0x6F, 0x6C, ' ', 'L', 'A', 'T', 0};
const unsigned char CHT[] = { 0xE7, 0xB9, 0x81, 0xE9, 0xAB,  0x94, 0xE4, 0xB8, 0xAD, 0xE6, 0x96, 0x87, ' ', 'C', 'H', 'T', 0};
const unsigned char POR[] = { 0x50, 0x6F, 0x72, 0x74, 0x75,  0x67, 0x75, 0xC3, 0xAA, 0x73, ' ', 'P', 'O', 'R', 0};
const unsigned char SPA[] = { 0x45, 0x73, 0x70, 0x61, 0xC3,  0xB1, 0x6F, 0x6C, ' ', 'S', 'P', 'A', 0};
const unsigned char CZE[] = { 0xC4, 0x8D, 0x65, 0x73, 0x6B, 0x79, ' ', 'C', 'Z', 'E', 0};
const unsigned char THA[] = { 0xE0, 0xB9, 0x82, 0xE0, 0xB8,  0x94, 0xE0, 0xB8, 0xA2, ' ', 'T', 'H', 'A', 0};
const unsigned char TUR[] = { 0x54, 0xC3, 0xBC, 0x72, 0x6B,  0xC3, 0xA7, 0x65, ' ', 'T', 'U', 'R', 0};
const unsigned char HEB[] = { 0xD7, 0xA2, 0xD7, 0x91, 0xD7,  0xA8, 0xD7, 0x99, 0xD7, 0xAA, ' ', 'H', 'E', 'B', 0};
const unsigned char ARA[] = { 0xEF, 0xBB, 0x8B, 0xEF, 0xBA, 0xAD, 0xEF, 0xBA,  0x91, 0xEF, 0xBB, 0xB2, ' ', 'A', 'R', 'A', 0};

const unsigned char GRC[] = { 'G', 'R', 'E', 'E', 'K', ' ', 'G', 'R', 'C', 0};

int
XplReturnLanguageName(int lang, unsigned char *Buffer)
{
	switch(lang) {
		case 0: memcpy(Buffer, CAN, strlen(CAN)+1); break;
		case 1: memcpy(Buffer, CHS, strlen(CHS)+1); break;
		case 2: strcpy(Buffer, "Dansk DAN"); break;
		case 3: strcpy(Buffer, "Nederlands NDL"); break;
		case 4: strcpy(Buffer, "English"); break;
		case 5: strcpy(Buffer, "Suomi FIN"); break;
		case 6: memcpy(Buffer, FRA, strlen(FRA)+1); break;
		case 7: strcpy(Buffer, "Deutsch DEU"); break;
		case 8: strcpy(Buffer, "Italiano ITA"); break;
		case 9: memcpy(Buffer, JPN, strlen(JPN)+1); break;
		case 10: memcpy(Buffer, KOR, strlen(KOR)+1); break;
		case 11: strcpy(Buffer, "Norsk NOR"); break;
		case 12: memcpy(Buffer, BRA, strlen(BRA)+1); break;
		case 13: memcpy(Buffer, RUS, strlen(RUS)+1); break;
		case 14: memcpy(Buffer, SA,  strlen(SA)+1);  break;
		case 15: strcpy(Buffer, "Svenska SVE"); break;
		case 16: memcpy(Buffer, CHT,  strlen(CHT)+1);  break;
		case 17: strcpy(Buffer, "Polski POL"); break;
		case 18: memcpy(Buffer, POR,  strlen(POR)+1);  break;
		case 19: memcpy(Buffer, SPA,  strlen(SPA)+1);  break;
		case 20: strcpy(Buffer, "Magyar HUN"); break;
		case 21: memcpy(Buffer, CZE,  strlen(CZE)+1);  break;
		case 22: memcpy(Buffer, THA,  strlen(THA)+1);  break;
		case 26: memcpy(Buffer, GRC,  strlen(GRC)+1);  break;
		case 41: memcpy(Buffer, TUR,  strlen(TUR)+1);  break;
		case 60:	memcpy(Buffer, HEB,  strlen(HEB)+1);  break;
		case 61: memcpy(Buffer, ARA,  strlen(ARA)+1);  break;
		default:
			sprintf(Buffer, "Unknown (ID: %d)", lang);
			break;
	}
	return(0);
}


#if defined(WIN32)

XplBool
XPLGetNullDACL(SECURITY_ATTRIBUTES *sa)
{
	PSECURITY_DESCRIPTOR		pHandleSD = NULL;

	memset ((void *)sa, 0, sizeof(SECURITY_ATTRIBUTES));

	pHandleSD = (PSECURITY_DESCRIPTOR)(malloc(SECURITY_DESCRIPTOR_MIN_LENGTH));
	if (!pHandleSD) {
		return(FALSE);
	}

	if (!InitializeSecurityDescriptor(pHandleSD, SECURITY_DESCRIPTOR_REVISION)) {
		return(FALSE);
	}

	// set NULL DACL on the SD
	if (!SetSecurityDescriptorDacl(pHandleSD, TRUE, (PACL) NULL, FALSE)) {
		return(FALSE);
	}

	// now set up the security attributes
	sa->nLength = sizeof(SECURITY_ATTRIBUTES);
	sa->bInheritHandle = TRUE;
	sa->lpSecurityDescriptor = pHandleSD;

	return(TRUE);
}

EXPORT XplPluginHandle XplLoadDLL(const char *path)
{
	char		*ptr;
	char		expath[XPL_MAX_PATH];

	if (!XplExpandEnv(expath, path, sizeof(expath))) {
		path = (const char *) expath;
	} else {
		strncpy(expath, path, sizeof(expath));
		path = (const char *) expath;
	}

	for (ptr = expath; *ptr; ptr++) {
		if (*ptr == '/') {
			*ptr = '\\';
		}
	}

	return(LoadLibrary(path));
}


EXPORT int XplGetCurrentOSLanguageID(void)
{
	/* Should build something around GetLocaleInfo() */
	return(4);
}

EXPORT XplBool XplSetCoreBehavior( void )
{
	return TRUE;
}

EXPORT uint64 XplGetMemAvail(void)
{
    MEMORYSTATUS	MemStat;

	GlobalMemoryStatus(&MemStat);
	return(MemStat.dwAvailPhys);
}

EXPORT void XplDebugOut(const char *Format, ...)
{
	char	DebugBuffer[10240];
	va_list	argptr;

	va_start(argptr, Format);
	vsprintf(DebugBuffer, Format, argptr);
	va_end(argptr);

	OutputDebugString(DebugBuffer);
}

#endif /* WIN32 */

#if defined(LIBC)
uint64
XplGetMemAvail(void)
{
	struct memory_info	info;

	if (netware_mem_info(&info) == 0) {
		return(info.CacheBufferMemory);
	}

	return(0);
}
#endif	/*	defined(LIBC)	*/

#if defined(SOLARIS) || defined(LINUX) || defined(S390RH) || defined(MACOSX)


EXPORT int XplTimeoutSet( XplTimeout *timeout, int milliseconds )
{
	int	previous = *timeout;

	errno= 0;

	*timeout = milliseconds;

	return( previous );
}

EXPORT int XplTimeoutGet( XplTimeout *timeout )
{
	errno = 0;
    return( *timeout );
}

int
XplCPUTimerCmp( XplCPUTimer *left, XplCPUTimer *right )
{
	if( left->tv_sec > right->tv_sec ) {
		return 1;
	}

	if( left->tv_sec < right->tv_sec ) {
		return -1;
	}

	/* seconds are equal */

	if( left->tv_nsec > right->tv_nsec ) {
		return 1;
	}

	if( left->tv_nsec < right->tv_nsec ) {
		return -1;
	}

	return 0;
}


void
XplCPUTimerDivide( XplCPUTimer *dividend, unsigned int divisor, XplCPUTimer *quotient )
{
	uint64 dividendNS;
	uint64 quotientNS;
	uint64 quotientS;

	dividendNS = ( ( uint64 )( dividend->tv_sec) * 1000000000 ) + ( uint64 )dividend->tv_nsec;
	quotientNS =  dividendNS / divisor ;
	quotientS = quotientNS / 1000000000;
	quotientNS = quotientNS % 1000000000;

	quotient->tv_sec = ( time_t )quotientS;
	quotient->tv_nsec = ( long )quotientNS;
}

unsigned long
XplCPUTimerInt( XplCPUTimer *timer, XplTimeUnit units )
{
	switch( units ) {
		case XPL_TIME_SEC:
			return( timer->tv_sec );
		case XPL_TIME_MSEC:
			return( timer->tv_nsec / 1000000 );
		case XPL_TIME_USEC:
			return( timer->tv_nsec / 1000 );
		case XPL_TIME_NSEC:
			return( timer->tv_nsec );
	}
	return( 0 );
}

void
XplCPUThreadTimerStart(XplCPUTimer *timer)
{
	if( clock_gettime( CLOCK_THREAD_CPUTIME_ID, timer ) ) {
		timer->tv_sec = 0;
		timer->tv_nsec = 0;
	}
}

void
XplCPUThreadTimerStop( XplCPUTimer *timer )
{
	struct timespec end;

	if( clock_gettime( CLOCK_THREAD_CPUTIME_ID, &end ) == 0 ) {
		XplCPUTimerSubtract( &end, timer, timer );
		return;
	}
	timer->tv_sec = 0;
	timer->tv_nsec = 0;
	return;
}

void
XplTimerStart(XplTimer *timer)
{
    struct timeval start;

    if (gettimeofday(&start, NULL) == 0) {
        timer->sec = start.tv_sec;
        timer->usec = start.tv_usec;
    } else {
        timer->sec = 0;
        timer->usec = 0;
    }

    return;
}

void
XplTimerStop(XplTimer *timer)
{
    struct timeval stop;

    if (gettimeofday(&stop, NULL) == 0) {
        if (stop.tv_sec == timer->sec) {
            timer->sec = 0;
            timer->usec = stop.tv_usec - timer->usec;
        } else {
            timer->sec = stop.tv_sec - timer->sec;

            if (stop.tv_usec <= timer->usec) {
                stop.tv_usec += 1000000;
				timer->sec--;
            }

            timer->usec = stop.tv_usec - timer->usec;
        }
    } else {
        timer->sec = 0;
        timer->usec = 0;
    }

    return;
}

#elif defined(WIN32)

EXPORT int XplTimeoutSet( XplTimeout *timeout, int milliseconds )
{
    int	previous;

	errno = 0;
    if( milliseconds < 4294968 ) {
        previous = ( timeout->tv_sec * 1000 ) + ( timeout->tv_usec / 1000 );

		timeout->tv_sec = milliseconds / 1000;
		timeout->tv_usec = (milliseconds % 1000) * 1000;

#if defined(DEBUG_SOCKWIN)
		XplConsolePrintf("Socket Timeout set to %d ms -> %lu sec %lu usec\n",milliseconds,timeout->tv_sec,timeout->tv_usec);
#endif
        return( previous );
    }

    errno = ERANGE;
    return SOCKET_ERROR;
}

EXPORT int XplTimeoutGet( XplTimeout *timeout )
{
	errno = 0;
    return( ( timeout->tv_sec * 1000 ) + ( timeout->tv_usec / 1000 ) );
}


LARGE_INTEGER XplFrequencyPerSecond = { 0 };
LARGE_INTEGER XplFrequencyPerMilliSecond = { 0 };

EXPORT int XplCPUTimerCmp( XplCPUTimer *left, XplCPUTimer *right )
{
	if( *left > *right ) {
		return 1;
	}

	if( *left < *right ) {
		return -1;
	}

	return 0;
}

EXPORT void XplCPUTimerDivide( XplCPUTimer *dividend, unsigned int divisor, XplCPUTimer *quotient )
{
	/* FIXME - needs implemented for win32 */
//	*result = 0;
}

EXPORT unsigned long XplCPUTimerInt( XplCPUTimer *timer, XplTimeUnit units )
{
	/* FIXME - needs implemented for win32 */
	return( 0 );
}

EXPORT void XplCPUThreadTimerStart(XplCPUTimer *timer)
{
	/* FIXME - needs implemented for win32 */
	*timer = 0;
}

EXPORT void XplCPUThreadTimerStop(XplCPUTimer *timer)
{
	/* FIXME - needs implemented for win32 */
	*timer = 0;
}

EXPORT void XplTimerStart(XplTimer *timer)
{
    LARGE_INTEGER start;
	/*
	   the 'start' variable may seem redundant, but QueryPerformanceCounter() requires
	   a byte aligned structure and we can't assume the one in the timer is aligned.
	   To be safe, we are calling it with a stack variable and then copying what we need
	   into the timer argument
	*/
    timer->sec = 0;
    timer->usec = 0;

    do {
        if (XplFrequencyPerSecond.QuadPart) {
		  if (QueryPerformanceCounter( &start ) ) {
			    timer->usable = TRUE;
			    timer->start.QuadPart = start.QuadPart;
				timer->sec = start.QuadPart / XplFrequencyPerSecond.QuadPart;
				start.QuadPart -=  ( (timer->sec) * XplFrequencyPerSecond.QuadPart );
				timer->usec = start.QuadPart / XplFrequencyPerMilliSecond.QuadPart;
				return;
			} else {
			  int err = GetLastError();
			  DebugPrintf( "XplTimerStart() failed because QueryPerformanceCounter failed with error %s\n", err );
			  DebugAssert( 0 ); // QueryPerformanceCounter() is not working.  It might be a platorm thing ( rodney )
			}
            break;
        }

        if (QueryPerformanceFrequency((LARGE_INTEGER *)&XplFrequencyPerSecond)) {
            if (XplFrequencyPerSecond.QuadPart) {
                if (XplFrequencyPerSecond.QuadPart >= 1000) {
                    XplFrequencyPerMilliSecond.QuadPart = XplFrequencyPerSecond.QuadPart / 1000;
                } else {
                    XplFrequencyPerMilliSecond.QuadPart = 10;
                }

                continue;
            }
        }

        break;
    } while (TRUE);

    timer->usable = FALSE;
    return;
}

EXPORT void XplTimerStop(XplTimer *timer)
{
    LARGE_INTEGER stop;

    if (QueryPerformanceCounter(&stop)) {
        if (timer->usable) {
            stop.QuadPart -= timer->start.QuadPart;

            timer->sec = stop.QuadPart / XplFrequencyPerSecond.QuadPart;
            stop.QuadPart -= (timer->sec) * XplFrequencyPerSecond.QuadPart;

            timer->usec = stop.QuadPart / XplFrequencyPerMilliSecond.QuadPart;
        }

        return;
    }

    return;
}

#endif


EXPORT int XplTimerCmp( XplTimer *left, XplTimer *right )
{
	if( left->sec > right->sec ) {
		return 1;
	}

	if( left->sec < right->sec ) {
		return -1;
	}

	/* seconds are equal */

	if( left->usec > right->usec ) {
		return 1;
	}

	if( left->usec < right->usec ) {
		return -1;
	}

	return 0;
}

EXPORT void XplTimerDivide( XplTimer *dividend, unsigned int divisor, XplTimer *quotient )
{
	uint64 dividendUS;
	uint64 quotientUS;
	uint64 quotientS;

	dividendUS = ( ( uint64 )( dividend->sec) * 1000000 ) + ( uint64 )dividend->usec;
	quotientUS =  dividendUS / divisor ;
	quotientS = quotientUS / 1000000;
	quotientUS = quotientUS % 1000000;

	quotient->sec = ( time_t )quotientS;
	quotient->usec = ( long )quotientUS;
}

EXPORT unsigned long XplTimerInt( XplTimer *timer, XplTimeUnit units )
{
	switch( units ) {
		case XPL_TIME_SEC:
			return( timer->sec );
		case XPL_TIME_MSEC:
			return( timer->usec / 1000 );
		case XPL_TIME_USEC:
			return( timer->usec );
		case XPL_TIME_NSEC:
			return( timer->usec * 1000 );
	}
	return( 0 );
}

EXPORT XplTimer *XplTimerSplit( XplTimer *startTime, XplTimer *splitTime )
{
	XplTimer stopTime;

	XplTimerStart( &stopTime );

	splitTime->sec = stopTime.sec - startTime->sec;
	if( stopTime.usec < startTime->usec ) {
		(splitTime->sec)--;
		splitTime->usec = 1000000 - ( startTime->usec - stopTime.usec );
	} else {
		splitTime->usec = stopTime.usec - startTime->usec;
	}
    return( splitTime );
}

EXPORT XplTimer *XplTimerLap( XplTimer *lastTime, XplTimer *lapTime )
{
	XplTimer stopTime;

	XplTimerStart( &stopTime );

	lapTime->sec = stopTime.sec - lastTime->sec;
	if( stopTime.usec < lastTime->usec ) {
		(lapTime->sec)--;
		lapTime->usec = 1000000 - ( lastTime->usec - stopTime.usec );
	} else {
		lapTime->usec = stopTime.usec - lastTime->usec;
	}
	lastTime->sec = stopTime.sec;
	lastTime->usec = stopTime.usec;
    return( lapTime );
}

EXPORT XplTimer *XplTimerSplitAndLap( XplTimer *startTime, XplTimer *splitTime, XplTimer *lastTime, XplTimer *lapTime )
{
	XplTimer stopTime;

	XplTimerStart( &stopTime );

	splitTime->sec = stopTime.sec - startTime->sec;
	if( stopTime.usec < startTime->usec ) {
		(splitTime->sec)--;
		splitTime->usec = 1000000 - ( startTime->usec - stopTime.usec );
	} else {
		splitTime->usec = stopTime.usec - startTime->usec;
	}

	lapTime->sec = stopTime.sec - lastTime->sec;
	if( stopTime.usec < lastTime->usec ) {
		(lapTime->sec)--;
		lapTime->usec = 1000000 - ( lastTime->usec - stopTime.usec );
	} else {
		lapTime->usec = stopTime.usec - lastTime->usec;
	}
	lastTime->sec = stopTime.sec;
	lastTime->usec = stopTime.usec;
    return( lapTime );
}

EXPORT void XplTimerAccumulate( XplTimerAccumulator *accum, XplTimer *timer )
{
    if( accum->count ) {
        if( ( timer->sec > accum->max.sec ) || ( ( timer->sec == accum->max.sec ) && ( timer->usec > accum->max.usec ) ) ) {
            accum->max.sec = timer->sec;
            accum->max.usec = timer->usec;
        }

        if( ( timer->sec < accum->min.sec ) || ( ( timer->sec == accum->min.sec ) && ( timer->usec < accum->min.usec ) ) ) {
            accum->min.sec = timer->sec;
            accum->min.usec = timer->usec;
        }

        accum->total.sec += timer->sec;
        accum->total.usec += timer->usec;
        if( accum->total.usec > 1000000 ) {
            accum->total.sec++;
            accum->total.usec -= 1000000;
        }
    } else {
            accum->max.sec = timer->sec;
            accum->max.usec = timer->usec;
            accum->min.sec = timer->sec;
            accum->min.usec = timer->usec;
    }
    accum->count++;
}

EXPORT void XplTimerAverage( XplTimerAccumulator *accum, XplTimer *out )
{
    unsigned long uSec;

    if( accum->count ) {
        out->sec = accum->total.sec / accum->count;
        /* get the remainder in seconds */
        uSec = accum->total.sec % accum->count;
        /* convert that to micoseconds */
        uSec *= 1000000;
        /* add usec total that is already in microseconds */
        uSec += accum->total.usec;
        /* take the average */
        out->usec = uSec / accum->count;
        /* there is a chance that the sec remainder + the usec value will add up to be more than 1 second */
        if( out->usec > 1000000 ) {
            out->sec++;
            out->usec -= 1000000;
        }
        return;
    }
    out->sec = 0;
    out->usec = 0;
    return;
}

EXPORT void XplTimerAccumulatorPrint( XplTimerAccumulator *accum, char *label )
{
    XplTimer t;

    XplTimerAverage( accum, &t );

    XplConsolePrintf( "Ave: %1lu.%06lu Max: %1lu.%06lu Min: %1lu.%06lu %s\n", t.sec, t.usec, accum->max.sec, accum->max.usec, accum->min.sec, accum->min.usec, label );
}

EXPORT void XplTimedCheckStart(XplTimer *timer){
	XplTimerStart(timer);
}

EXPORT void XplTimedCheckStop(XplTimer *timer, const char* checkName, long checkRate , XplAtomic *abortTrigger){
	long delay;
	int i;

	XplTimerStop(timer);
	delay = (checkRate * 1000) - (XplTimerInt( timer , XPL_TIME_SEC ) * 1000  + XplTimerInt(timer, XPL_TIME_MSEC));

	if (delay < 0){
		printf("WARNING: timed check %s took %ldms more than check rate %lds\n", checkName, labs(delay),checkRate );
	} else if (delay) {
		if (abortTrigger){
			for (i=0; i <= (delay/250) ; i++ ){
				if ( XplSafeRead(*abortTrigger) == 1 ){
					break;
				}
				XplDelay(250);
			}
		} else {
			XplDelay(delay);
		}
	}
}
