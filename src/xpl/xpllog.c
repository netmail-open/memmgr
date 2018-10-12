#include <memmgr-config.h>
#include <xpl.h>
#include <stdio.h>
#include <memmgr.h>

/* Windows only for now*/
#if defined(WINDOWS)

#define DEBUG_LOG_PATH "C:\\mplus.log"

#define MAXLEN_LOGLINE 1024


XplRWLock _XPLLogLock_;

/* Just for debug puposes
 * there is no locking of any kind
 * so entries can be mixed
 * and opennig and closing the file
 * every time must not be very efficent */
EXPORT int XplConsoleToFile(const char *format, ...)
{
	FILE *log;
	char buffer[MAXLEN_LOGLINE];
	SYSTEMTIME st;
	va_list args;

	XplRWWriteLockAcquire(&(_XPLLogLock_));
	if (!(log = fopen(DEBUG_LOG_PATH, "a"))) {
		printf("Could not open log file " DEBUG_LOG_PATH " \n");
		return -1;
	}

	 GetLocalTime(&st);
     fprintf(log, "[%02d/%02d/%04d - %02d:%02d:%02d:%03d] ",
                   st.wMonth, st.wDay, st.wYear, st.wHour,
                   st.wMinute, st.wSecond, st.wMilliseconds);

    va_start(args, format);
    vstrprintf(buffer, MAXLEN_LOGLINE, NULL, format, args);
    va_end(args);

	fputs(buffer, log);

	printf( "%s",buffer);

	fclose(log);
	 XplRWWriteLockRelease(&(_XPLLogLock_));
	return 0;
}


#endif

EXPORT ErrorLogFile *ErrorLogOpen( const char *name )
{
	ErrorLogFile			*log;
	XplThreadGroupID		id;
	char					buffer[4096];

	if( log = MemMallocEx( NULL, sizeof( ErrorLogFile ), NULL, FALSE, TRUE ) )
	{
		id = XplGetThreadGroupID_( NULL );
		strprintf( buffer, sizeof( buffer ), NULL, "%s/%s.%ld.log", SYSCONF_DIR, name, (long)id );
		if( log->file = fopen( buffer, "wt" ) )
		{
			XplMutexInit( log->mutex );
			return log;
		}
		MemRelease( &log );
	}
	return NULL;
}

EXPORT int ErrorLogClose( ErrorLogFile **log )
{
	if( log && *log )
	{
		if( (*log)->file )
		{
			fclose( (*log)->file );
			(*log)->file = NULL;
		}
		XplMutexDestroy( (*log)->mutex );
		MemRelease( log );
		return 0;
	}
	return -1;
}

EXPORT int ErrorLog( ErrorLogFile *log, const char *format, ... )
{
	time_t		now;
	size_t		needed, pre;
	va_list		args;
	struct tm	tm;
	char		*buffer;
	char		_buffer[1024];

	if( log )
	{
		XplMutexLock( log->mutex );
		if( log->file )
		{
			time( &now );
			localtime_r( &now, &tm );
			strftime( _buffer, sizeof(_buffer), "%F %T ", &tm );
			_buffer[sizeof(_buffer)-1] = '\0';
			pre = strlen( _buffer ) + 1;

			va_start( args, format );
			buffer = _buffer;
			vstrcatf( _buffer, sizeof( _buffer ), &needed, format, args );
			needed += pre;

			if( sizeof( _buffer ) <= needed )
			{
				va_end( args );
				if( !( buffer = MemMalloc( needed ) ) )
				{
					return -1;
				}
				strftime( buffer, needed, "%F %T ", &tm );
				va_start( args, format );
				vstrcatf( buffer, needed, &needed, format, args );
			}
			fprintf( log->file, "%s\n", buffer );
			va_end( args );
			fflush( log->file );
			if( buffer != _buffer )
			{
				MemRelease( &buffer );
			}
		}
		XplMutexUnlock( log->mutex );
		return 0;
	}
	return -1;
}

