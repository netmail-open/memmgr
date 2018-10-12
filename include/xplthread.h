/****************************************************************************
 |
 | Copyright (c) 1999-2002 Novell, Inc.
 | All Rights Reserved.
 |
 | This program is free software; you can redistribute it and/or
 | modify it under the terms of version 2.1 of the GNU Lesser General
 | Public License as published by the Free Software Foundation.
 |
 | This program is distributed in the hope that it will be useful,
 | but WITHOUT ANY WARRANTY; without even the implied warranty of
 | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 | GNU Lesser General Public License for more details.
 |
 | You should have received a copy of the GNU Lesser General Public License
 | along with this program; if not, contact Novell, Inc.
 |
 | To contact Novell about this file by physical or electronic mail,
 | you may find current contact information at www.novell.com
 |
 |***************************************************************************/

#ifndef XPLTHREAD_H
#define XPLTHREAD_H
#include <memmgr-config.h>
//#include <xplutil.h>

#ifdef HAVE_SEMAPHORE_H
# include <semaphore.h>
#endif

#include <string.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAVE_DELAY
# define XplDelay(msec) delay(msec)
#elif defined (HAVE_SLEEP)
# define XplDelay(msec) Sleep(msec)
#else
# define XplDelay(msec) { struct timeval timeout; timeout.tv_usec=((msec) % 1000) * 1000; timeout.tv_sec=(msec)/1000; select(0, NULL, NULL, NULL, &timeout); }
#endif

#if defined(LINUX)
# ifndef __USE_GNU
#  define __USE_GNU
#  include <pthread.h>
#  undef __USE_GNU
# else
#  include <pthread.h>
# endif
#endif

/*******************************************************************************
XplMutex
*******************************************************************************/

#if defined(LINUX)
#include <signal.h>

typedef pthread_mutex_t	_XplMutex;
#elif defined(WIN32)
# include <winsock2.h>
# include <ws2tcpip.h>
# include <mswsock.h>
# include <windows.h>

typedef HANDLE			_XplMutex;
#else
# error "Mutexes not implemented on this platform"
#endif

#ifdef DEBUG
typedef enum
{
	XPL_MUTEX_NOT_READY		= 0,
	XPL_MUTEX_READY			= 0x12345
} XplMutexState;

typedef struct
{
	_XplMutex		mutex;
	int				depth;
	XplMutexState	state;

	struct {
		const char	*file;
		int			line;
	} created;

	struct {
		const char	*file;
		int			line;
	} locked;

	struct {
		const char	*file;
		int			line;
	} released;

	struct {
		const char	*file;
		int			line;
	} destroyed;
} XplMutex;

#define XplMutexInit(m) {					\
	(m).created.file = __FILE__;			\
	(m).created.line = __LINE__;			\
	(m).depth = 0;							\
	(m).state = XPL_MUTEX_READY;			\
	_XplMutexInit( &(m).mutex, TRUE );		\
}

#define XplMutexDestroy(m) {				\
	(m).destroyed.file = __FILE__;			\
	(m).destroyed.line = __LINE__;			\
	_XplMutexDestroy( &(m).mutex );			\
}

#define XplMutexLock(m) {					\
	CriticalAssert(XPL_MUTEX_READY == (m).state);\
	_XplMutexLock( &(m).mutex );			\
	if (0 == (m).depth) {					\
		(m).locked.file = __FILE__;			\
		(m).locked.line = __LINE__;			\
	}										\
	(m).depth++;							\
}

#define XplMutexUnlock(m) {					\
	CriticalAssert(XPL_MUTEX_READY == (m).state);\
	(m).depth--;							\
	if (0 == (m).depth) {					\
		(m).released.file = __FILE__;		\
		(m).released.line = __LINE__;		\
	}										\
	_XplMutexUnlock( &(m).mutex );			\
}

/* End DEBUG */
#else
/* NOT DEBUG */

typedef struct
{
	_XplMutex		mutex;
	int				depth;
} XplMutex;

#define XplMutexInit(m) {					\
	(m).depth = 0;							\
	_XplMutexInit( &(m).mutex, TRUE );		\
}

#define XplMutexDestroy(m) {				\
	_XplMutexDestroy( &(m).mutex );			\
}

#define XplMutexLock(m) {					\
	_XplMutexLock( &(m).mutex );			\
	(m).depth++;							\
}

#define XplMutexUnlock(m) {					\
	(m).depth--;							\
	_XplMutexUnlock( &(m).mutex );			\
}

#endif /* End NOT DEBUG */

EXPORT int _XplMutexInit( _XplMutex *mutex, XplBool recursive );
EXPORT int _XplMutexDestroy( _XplMutex *mutex );
EXPORT int _XplMutexLock( _XplMutex *mutex );
EXPORT int _XplMutexUnlock( _XplMutex *mutex );

/*******************************************************************************
End XplMutex
*******************************************************************************/



/*******************************************************************************
XplLock

The XplLock is a hybrid spinlock/mutex that acts as a code serialization lock to
protect data, like spinlocks or mutexes the code should never call any API that
will explicitly generate a thread context switch.  On Linux and Windows these
locks DO cause an explicit context switch on a miss because we are running in
user mode.

It is safe to recursively lock an XplMutex but doing this with an XplLock is not
permitted.

XplLock also avoids modifying errno, but XplMutex will set errno.
*******************************************************************************/
#if defined(LINUX)
typedef XplMutex							XplLock;

# define XplLockInit(m) {					\
	int xpllock_prev_errno = errno;			\
	XplMutexInit(*(m));						\
	errno = xpllock_prev_errno;				\
}
# define XplLockDestroy(m) {				\
	int xpllock_prev_errno = errno;			\
	XplMutexDestroy(*(m))					\
	errno = xpllock_prev_errno;				\
}

# define XplLockAcquire(m) {				\
	int xpllock_prev_errno = errno;			\
	XplMutexLock( *(m) );					\
	DebugAssert(1 == (*(m)).depth);			\
	errno = xpllock_prev_errno;				\
}

# define XplLockRelease(m) {				\
	int xpllock_prev_errno = errno;			\
	XplMutexUnlock( *(m) );					\
	errno = xpllock_prev_errno;				\
}

# define XplLockUpdateOwner(m, f, i)
# define XplLockValue(m)					(*(m)).depth
#elif defined(WIN32)
typedef LONG								XplLock;

# define XplLockInit(l)						*(l) = (LONG) 0
# define XplLockAcquire(l)					while (InterlockedExchange((l), (LONG)1)) { XplDelay(1); }
# define XplLockValue(l)					*(l)
# define XplLockUpdateOwner(l, f, i)
# define XplLockRelease(l)					InterlockedExchange((l), (LONG) 0)
#endif


/*******************************************************************************
XplAtomic

The XplSafe functions implement atomics, the atomics are 32 bit unsigned values
*******************************************************************************/
#define XPL_ATOMIC_FLAG_IGNORE_HIGHBIT		0x00000001

#if defined(LINUX)
# define ATOMIC_SIG							0x1234fedc
typedef struct
{
	XplLock									lock;
	unsigned long							value;

#  ifdef DEBUG
	unsigned long							flags;
	unsigned long							signature;
	char									*id;
#  endif
} XplAtomic;

#endif // END LINUX

#if defined(LINUX)
# ifdef DEBUG
void _XplSafeInit			( XplAtomic *variable, unsigned long value, char *id, const char *file, const int line );
void _XplSafeFlag			( XplAtomic *variable, unsigned long flags, const char *file, const int line);

unsigned long _XplSafeRead	( XplAtomic *variable, const char *file, const int line );
unsigned long _XplSafeWrite	( XplAtomic *variable, unsigned long value, const char *file, const int line );
unsigned long _XplSafeAdd	( XplAtomic *variable, unsigned long value, const char *file, const int line );
unsigned long _XplSafeAnd	( XplAtomic *variable, unsigned long value, const char *file, const int line );
unsigned long _XplSafeOr	( XplAtomic *variable, unsigned long value, const char *file, const int line );

#  define XplSafeInitTrace( V, v, id )			_XplSafeInit( &(V),(v), (id), __FILE__, __LINE__ )
#  define XplSafeInit( V, v )					_XplSafeInit( &(V),(v), NULL, __FILE__, __LINE__ )
#  define XplSafeFlag(V, f)						_XplSafeFlag(&(V), (f), __FILE__, __LINE__)

#  define XplSafeRead( V )						_XplSafeRead( &(V), __FILE__, __LINE__ )
#  define XplSafeWrite( V, v )					_XplSafeWrite( &(V), (v), __FILE__, __LINE__ )
#  define XplSafeIncrement( V )					_XplSafeAdd( &(V),  1, __FILE__, __LINE__ )
#  define XplSafeDecrement( V )					_XplSafeAdd( &(V), -1, __FILE__, __LINE__ )
#  define XplSafeAdd( V, v )					_XplSafeAdd( &(V), (v), __FILE__, __LINE__ )
#  define XplSafeSub( V, v )					_XplSafeAdd( &(V), (-1 * (v)), __FILE__, __LINE__ )
#  define XplSafeAnd( V, v )					_XplSafeAnd( &(V), (v), __FILE__, __LINE__ )
#  define XplSafeOr( V, v )						_XplSafeOr( &(V), (v), __FILE__, __LINE__ )

# else // DEBUG
void _XplSafeInit			( XplAtomic *variable, unsigned long value );
unsigned long _XplSafeRead	( XplAtomic *variable );
unsigned long _XplSafeWrite	( XplAtomic *variable, unsigned long value );
unsigned long _XplSafeAdd	( XplAtomic *variable, unsigned long value );
unsigned long _XplSafeAnd	( XplAtomic *variable, unsigned long value );
unsigned long _XplSafeOr	( XplAtomic *variable, unsigned long value );

#  define XplSafeInitTrace( V, v, id )			_XplSafeInit( &(V),(v), NULL )
#  define XplSafeInit( V, v )					_XplSafeInit( &(V),(v) )
#  define XplSafeFlag(V, f)

#  define XplSafeRead( V )						_XplSafeRead( &(V) )
#  define XplSafeWrite( V, v )					_XplSafeWrite( &(V), (v) )
#  define XplSafeIncrement( V )					_XplSafeAdd( &(V),  1 )
#  define XplSafeDecrement( V )					_XplSafeAdd( &(V), -1 )
#  define XplSafeAdd( V, v )					_XplSafeAdd( &(V), (v) )
#  define XplSafeSub( V, v )					_XplSafeAdd( &(V), (-1 * (v)) )
#  define XplSafeAnd( V, v )					_XplSafeAnd( &(V), (v) )
#  define XplSafeOr( V, v )						_XplSafeOr( &(V), (v) )

# endif // !DEBUG

#elif defined(WIN32)


# include <winsock2.h>
# include <ws2tcpip.h>
# include <mswsock.h>
# include <windows.h>


typedef LONG									XplAtomic;
# define XplSafeInitTrace(Variable, Value, id)	InterlockedExchange(&(Variable), (Value))
# define XplSafeInit(Variable, Value)			InterlockedExchange(&(Variable), (Value))
# define XplSafeFlag(Variabe, Value)
# define XplSafeRead(Variable)					(Variable)
# define XplSafeWrite(Variable, Value)			InterlockedExchange(&(Variable), (Value))
# define XplSafeIncrement(Variable)				InterlockedIncrement(&(Variable))
# define XplSafeDecrement(Variable)				InterlockedDecrement(&(Variable))
# define XplSafeAdd(Variable, Value)			InterlockedExchangeAdd(&(Variable), (Value))
# define XplSafeSub(Variable, Value)			InterlockedExchangeAdd(&(Variable), -1*(Value))
# ifdef INTERLOCKED_AND_OR
#  define XplSafeAnd(Variable, Value)			InterlockedAnd(&(Variable), (Value))
#  define XplSafeOr(Variable, Value)			InterlockedOr(&(Variable), (Value))
# else
#  define XplSafeAnd(Variable, Value)			InterlockedExchange(&(Variable), (Variable & Value))
#  define XplSafeOr(Variable, Value)			InterlockedExchange(&(Variable), (Variable | Value))
# endif

#else
# error "Safe variable operations not implemented on this platform"
#endif

/*******************************************************************************
End XplSafe
*******************************************************************************/




// gets multiplied by 2 in the old code
#define DEFAULT_STACK_SIZE	1024 * 128

#ifdef __WATCOMC__
EXPORT char *asctime_r( const struct tm *timeptr, char *buf );
EXPORT char *ctime_r( const time_t *timer, char *buf );
EXPORT struct tm *gmtime_r( const time_t *timer, struct tm *tmbuf );
EXPORT struct tm *localtime_r( const time_t *timer, struct tm *tmbuf );
#endif

#ifdef _MSC_VER
EXPORT char *asctime_r( const struct tm *timeptr, char *buf );
EXPORT char *ctime_r( const time_t *timer, char *buf );
EXPORT struct tm *gmtime_r( const time_t *timer, struct tm *tmbuf );
EXPORT struct tm *localtime_r( const time_t *timer, struct tm *tmbuf );
#endif

#define THREAD_STACK_SIZE	256 * 1024

typedef void (*XplThreadFunc)( void * );

typedef struct
{
	XplBool				running;
	char				*identity;
	int					id;
} *XplThreadGroup, XplThreadGroupStruct;

typedef struct XplThreadStruct
{
	XplBool				running;
	XplThreadGroup		group;
	void				*context;
	int					exitCode;
	int					id;
	void				(*destroyCB)(struct XplThreadStruct *);
} *XplThread, *XplThread_, XplThreadStruct;

typedef struct XplCancelPoint
{
	struct XplCancelPoint	*next;	// internal use only
	XplThread				thread;
	XplBool					cancelled;
	void					*context;
}XplCancelPoint;

typedef struct
{
	XplLock			lock;
	XplCancelPoint	*list;
}XplCancelList;

typedef int	XplThreadGroupID;
typedef int	XplThreadID;

#define XplGetThreadID_	XplGetThreadID
#define XplGetThreadGroupID_	XplGetThreadGroupID
#define XplSetThreadGroupID( g )	XplGetThreadID()
#define XplRenameThread( t, n )

EXPORT XplThreadGroup _XplThreadGroupCreate( const char *identity, const char *file, unsigned long line );
//	XplThreadGroup XplThreadGroupCreate( const char *identity );
//		Returns non-NULL on success, NULL on error.  Sets errno
// 		identity:	required
#define XplThreadGroupCreate( i )	_XplThreadGroupCreate( (i), __FILE__, __LINE__ )
EXPORT int _XplThreadGroupDestroy( XplThreadGroup *group, const char *file, unsigned long line );
//	int XplThreadGroupDestroy( XplThreadGroup *group );
// 		Signals each thread in the thread group with SIGTERM and blocks until
// 		all of the threads have exited.
//		Returns 0 on success, non-zero on error.  Sets errno
// 		group:	required
#define XplThreadGroupDestroy( g )	_XplThreadGroupDestroy( (g), __FILE__, __LINE__ )
EXPORT int _XplThreadGroupStackSize( XplThreadGroup group, size_t sizeInK, const char *file, unsigned long line );
// int XplThreadGroupStackSize( XplThreadGroup group, size_t sizeInK );
// 		Sets the thread stack size for new threads created using the specified
// 		thread group, does not effect already running threads.
//		Returns 0 on success, non-zero on error.  Sets errno
// 		group:			required
//		sizeInK:		required	A value of zero will set it back to default.
#define XplThreadGroupStackSize( g, s )	_XplThreadGroupStackSize( (g), (s), __FILE__, __LINE__ );
EXPORT int _XplThreadStart( XplThreadGroup group, int (*userFunction)( XplThread ), void *context, XplThread *threadP, const char *file, unsigned long line );
//	int XplThreadStart( XplThreadGroup group, int (*userFunction)( XplThread ), void *context, XplThread *thread );
//		Returns 0 on success, non-zero on error.  Sets errno
// 		group:			optional
// 		userFunction:	required
// 		context:		optional	Reference with thread->context inside of userFunction().
// 		thread:			optional	Returns the new thread handle, if you use this you will
// 			need to free the thread with XplThreadFree() when you are done with it.
#define XplThreadStart( g, u, c, t )	_XplThreadStart( (g), (u), (c), (t), __FILE__, __LINE__ )
EXPORT int _XplThreadWrap( XplThread *threadP, const char *file, unsigned long line );
//	int XplThreadWrap( XplThread *thread );
//		Special API to create a thread context on a thread that was not created
//		with this API.
//		Returns 0 on success, non-zero on error.  Sets errno
// 		thread:			required	Returns the new thread handle, XplThreadWrapEnd() needs to
// 			be called with it.
#define XplThreadWrap( t )			_XplThreadWrap( (t), __FILE__, __LINE__ )
EXPORT int _XplThreadWrapEnd( XplThread *threadP, const char *file, unsigned long line );
//	int XplThreadWrapEnd( XplThread *thread );
// 		Special API to destroy the thread context on a thread that was not
//		created with this API but was wrapped with XplThreadWrap().
//		Returns 0 on success, non-zero on error.  Sets errno
// 		thread:			required	Call only with the handle returned from XplThreadWrap()
#define XplThreadWrapEnd( t )		_XplThreadWrapEnd( (t), __FILE__, __LINE__ )
EXPORT int _XplThreadFree( XplThread *thread, const char *file, unsigned long line );
//	int XplThreadFree( XplThread *thread );
//		Returns 0 on success, non-zero on error.  Sets errno
// 		thread:		required
#define XplThreadFree( t )			_XplThreadFree( (t), __FILE__, __LINE__ )
//	XplThread XplGetThread( void );
//		Returns the current thread handle on success, NULL on error
EXPORT XplThread XplGetThread( void );
//	XplThreadGroup XplGetThreadGroup( XplThread thread );
//		Returns the current thread group handle on success, NULL on error
//		thread:		optional	NULL selects the current thread
EXPORT XplThreadGroup XplGetThreadGroup( XplThread thread );
//	XplThreadID XplGetThreadID( void );
//		Returns the current thread id on success, -1 on error
EXPORT XplThreadID XplGetThreadID( void );
//	XplThreadGroupID XplGetThreadGroupID( XplThread thread );
//		Returns the current thread group id on success, -1 on error
EXPORT XplThreadGroupID XplGetThreadGroupID( XplThread thread );
//	int XplThreadSignal( XplThread thread, int sig, void *context );
//		Returns 0 on success, non-zero on error.  Sets errno
//		thread:		optional	NULL selects the current thread
EXPORT int XplThreadSignal( XplThread thread, int sig, void *context );
//	int XplThreadGroupSignal( XplThreadGroup group, int sig, void *context );
//		Returns 0 on success, non-zero on error.  Sets errno
//		thread:		optional	NULL selects the current thread
// 		sig:		required	0 shouldn't be used, use SIGTERM, SIGINT or user defined.
// 		context:	optional	context stored along side of signal.
EXPORT int XplThreadGroupSignal( XplThreadGroup group, int sig, void *context );
//	int XplThreadJoin( XplThread thread, int milliseconds );
// 		Blocks until thread exits or the timeout expires (ETIMEDOUT).
//		Returns 0 on success, non-zero on error.  Sets errno
//		thread:			required
// 		milliseconds:	required	Join will timeout after this many milliseconds,
// 			0 means return immediately, -1 means wait indefinitely.
EXPORT int XplThreadJoin( XplThread thread, int milliseconds );
//	int XplThreadCatch( XplThread thread, void **context, int64 milliseconds );
//		Returns 0 on success, non-zero on error.  Sets errno
//		thread:			optional	NULL selects current thread
// 		context:		optional	optional pointer passed into XplThreadSignal().
// 		milliseconds:	required	Join will timeout after this many milliseconds,
// 			0 means return immediately, -1 means wait indefinitely.
EXPORT int XplThreadCatch( XplThread thread, void **context, int64 milliseconds );
//	int XplThreadStats( uint32 *total, uint32 *idle, uint32 *active, uint32 *peak );
//		Returns 0 on success, non-zero on error.  Sets errno
//		total:		optional
// 		idle:		optional
// 		active:		optional
// 		peak:		optional
EXPORT int XplThreadStats( uint32 *total, uint32 *idle, uint32 *active, uint32 *peak );
//	int XplThreadCount( XplThreadGroup group );
// 		Returns number of threads on success, -1 on error.  Sets errno
//		group:		optional	NULL selects the current thread group
EXPORT int XplThreadCount( XplThreadGroup group );
//	XplBool XplThreadTerminated( XplThread thread, void *context, int64 milliseconds );
// 		Returns TRUE if thread has been signalled with SIGTERM, FALSE if not
//		thread:			optional	NULL selects the current thread
// 		context:		optional	optional pointer passed into XplThreadSignal().
// 		milliseconds:	required	Join will timeout after this many milliseconds,
// 			0 means return immediately, -1 means wait indefinitely.
EXPORT XplBool XplThreadTerminated( XplThread thread, void *context, int64 milliseconds );
//	int XplThreadRegisterData( char *name );
//		Returns 0 on success, non-zero on error.  Sets errno
//		name:	required
EXPORT int XplThreadRegisterData( char *name );
//	int XplThreadSetData( XplThread thread, int slot, void *data );
//		Returns 0 on success, non-zero on error.  Sets errno
//		thread:		optional	NULL selects the current thread
// 		slot:		required	slot from XplThreadRegisterData()
// 		data:		required	set whatever you want
EXPORT int XplThreadSetData( XplThread thread, int slot, void *data );
//	int XplThreadGetData( XplThread thread, int slot, void **data );
//		Returns 0 on success, non-zero on error.  Sets errno
//		thread:		optional	NULL selects the current thread
// 		slot:		required	slot from XplThreadRegisterData()
// 		data:		required
EXPORT int XplThreadGetData( XplThread thread, int slot, void **data );
//	int XplThreadGroupSetData( XplThreadGroup group, int slot, void *data );
//		Returns 0 on success, non-zero on error.  Sets errno
//		group:		optional	NULL selects the current thread group
// 		slot:		required	slot from XplThreadRegisterData()
// 		data:		required	set whatever you want
EXPORT int XplThreadGroupSetData( XplThreadGroup group, int slot, void *data );
//	int XplThreadGroupGetData( XplThreadGroup group, int slot, void **data );
//		Returns 0 on success, non-zero on error.  Sets errno
//		group:		optional	NULL selects the current thread group
// 		slot:		required	slot from XplThreadRegisterData()
// 		data:		required
EXPORT int XplThreadGroupGetData( XplThreadGroup group, int slot, void **data );
//	void _XplCancelInit( XplCancelList *list, XplCancelPoint *point );
//		list:		required
//		point:		required	Should be the address of a stack structure
EXPORT void _XplCancelInit( XplCancelList *list, XplCancelPoint *point );
//	void XplCancelInit( XplCancelPoint *point );
//		point:		required	Should be the address of a stack structure
EXPORT void XplCancelInit( XplCancelPoint *point );
//	void XplCancelListInit( XplCancelList *list );
//		list:		required
EXPORT void XplCancelListInit( XplCancelList *list );
//	int _XplCancelDestroy( XplCancelList *list, XplCancelPoint *point );
//		list:		required
//		point:		required	Should be the address of a stack structure
EXPORT int _XplCancelDestroy( XplCancelList *list, XplCancelPoint *point );
//	void XplCancelDestroy( XplCancelPoint *point );
//		point:		required	Should be the address of a stack structure
EXPORT void XplCancelDestroy( XplCancelPoint *point );
//	int _XplCancelThread( XplCancelList *list, XplThread thread, void *context );
//		Returns 0 on success, non-zero on error.  Sets errno
//		list:		required
// 		thread:		optional	NULL selects all threads
// 		context:	optional	stored in cp.context
EXPORT int _XplCancelThread( XplCancelList *list, XplThread thread, void *context );
//	int XplCancelThread( XplThread thread, void *context );
//		Returns 0 on success, non-zero on error.  Sets errno
// 		thread:		optional	NULL selects all threads
// 		context:	optional	stored in cp.context
EXPORT int XplCancelThread( XplThread thread, void *context );

//#endif	// OLD_THREADS

#if defined(HAVE_SEMAPHORE_H) && !defined(MACOSX)
#elif defined(WIN32)
#else
# error "Semaphores not implemented on this platform"
#endif

/*******************************************************************************
Semaphores
*******************************************************************************/
#if defined(LINUX) || defined(S390RH) || defined(MACOSX)
typedef sem_t			_XplSema;
#elif defined(WIN32)
typedef HANDLE			_XplSema;
#else
# error "Semaphores not implemented on this platform"
#endif

EXPORT int _XplSemaInit( _XplSema *sema, int initialCount );
EXPORT int _XplSemaDestroy( _XplSema *sema );
EXPORT int _XplSemaWait( _XplSema *sema );
EXPORT int _XplSemaTimedWait( _XplSema *sema, int64 milliseconds );
EXPORT int _XplSemaPost( _XplSema *sema );
EXPORT int _XplSemaValue( _XplSema *sema, int *value );

typedef struct
{
	_XplMutex	mutex;
	_XplSema	sema;
}XplCondVariable;

EXPORT int XplCondInit( XplCondVariable *var );
EXPORT int XplCondDestroy( XplCondVariable *var );
EXPORT int XplCondWait( XplCondVariable *var, int64 milliseconds );
EXPORT int XplCondSignal( XplCondVariable *var );
EXPORT int XplCondLock( XplCondVariable *var );
EXPORT int XplCondUnlock( XplCondVariable *var );

#ifdef DEBUG
typedef struct
{
	_XplSema	s;
	_XplSema	*p;
} XplSemaphore;

EXPORT int __XplSemaInit( XplSemaphore *sema, int initialCount );
EXPORT int __XplSemaDestroy( XplSemaphore *sema );
EXPORT int __XplSemaWait( XplSemaphore *sema );
EXPORT int __XplSemaTimedWait( XplSemaphore *sema, int64 milliseconds );
EXPORT int __XplSemaPost( XplSemaphore *sema );
EXPORT int __XplSemaValue( XplSemaphore *sema, int *value );
#define XplSemaInit( sem, value )		__XplSemaInit( &(sem), (value) )
#define XplSemaDestroy( sem )			__XplSemaDestroy( &(sem) )
#define XplSemaWait( sem )				__XplSemaWait( &(sem) )
#define XplSemaTimedWait( sem, milli )	__XplSemaTimedWait( &(sem), milli )
#define XplSemaPost( sem )				__XplSemaPost( &(sem) )
#define XplSemaValue( sem, value )		__XplSemaValue( &(sem), (value) )

#else	// DEBUG
typedef _XplSema	XplSemaphore;

#define XplSemaInit( sem, value )		_XplSemaInit( &(sem), (value) )
#define XplSemaDestroy( sem )			_XplSemaDestroy( &(sem) )
#define XplSemaWait( sem )				_XplSemaWait( &(sem) )
#define XplSemaTimedWait( sem, milli )	_XplSemaTimedWait( &(sem), milli )
#define XplSemaPost( sem )				_XplSemaPost( &(sem) )
#define XplSemaValue( sem, value )		_XplSemaValue( &(sem), (value) )
#endif	// DEBUG

# define XplDelayOnLocalSemaphore(sem, timeout, result)			\
{																\
	if( !XplSemaTimedWait( (sem), (timeout) ) ) (result) = 0;	\
	else if( ETIMEDOUT == errno )    (result) = 1;				\
	else    (result) = -1;										\
}

#ifdef __cplusplus
}
#endif

#endif

