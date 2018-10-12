#include <memmgr-config.h>

#if defined (WINDOWS)
//#include <windows.h>
#else
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#endif

#include <time.h>
#include <memmgr.h>
#include <xplutil.h>

typedef struct _XplAlarmHandle
{
	struct _XplAlarmHandle	*next;
	const char				*file;
	uint32					line;
} _XplAlarmHandle;

#if defined (WINDOWS)

typedef struct _XplAlarm
{
	XplAlarmPublic			client;
	struct _XplAlarm		*next;

	HANDLE					timerHandle;
	time_t					repeat;
	int						consumers;

	XplAlarmHandle			handle;
	const char				*file;
	uint32					line;
} _XplAlarm;

static struct
{
	int						consumers;
	int						initialized;

	HANDLE					queue;
	XplLock					lock;
	_XplAlarm				*head;
	_XplAlarmHandle			*handles;
} XplAlarms;

/*
	Remove an alarm from the global list, and destroy it.

	The consumer must already have the lock.
*/
static void removeAlarm(_XplAlarm *a)
{
	_XplAlarm			**ap;
	HANDLE				timerHandle = INVALID_HANDLE_VALUE;

	for(ap=&XplAlarms.head;*ap;ap=&(*ap)->next)
	{
		if( *ap == a )
		{
			*ap = a->next;
			timerHandle = a->timerHandle;
			MemRelease( &a );
			break;
		}
	}

	if( INVALID_HANDLE_VALUE != timerHandle )
	{
		DeleteTimerQueueTimer( NULL, timerHandle, NULL );
	}
}

XplAlarmHandle _XplAlarmInit(const char *file, uint32 line)
{
	_XplAlarmHandle	*handle;

	if (!XplAlarms.consumers++) {
		if (!XplAlarms.initialized++) {
			XplLockInit(&XplAlarms.lock);
		}

		XplLockAcquire(&XplAlarms.lock);
		XplAlarms.queue = CreateTimerQueue();
		XplLockRelease(&XplAlarms.lock);
	}

	handle			= MemCallocWait(1, sizeof(_XplAlarmHandle));
	handle->file	= file;
	handle->line	= line;

	XplLockAcquire(&XplAlarms.lock);

	handle->next = XplAlarms.handles;
	XplAlarms.handles = handle;

	XplLockRelease(&XplAlarms.lock);

	return((XplAlarmHandle) handle);
}

EXPORT void XplAlarmShutdown(XplAlarmHandle *handle)
{
	_XplAlarmHandle	**h;
	_XplAlarm		*a, *next;

	if (!handle || !*handle) {
		return;
	}

	XplLockAcquire(&XplAlarms.lock);

	/* Destroy any alarms that are still out there for this handle */
	for (a = XplAlarms.head; a; a = next) {
		next = a->next;

		if (a->handle && a->handle == *handle) {
			removeAlarm( a );
		}
	}

	for (h = &XplAlarms.handles; h && *h; h = &((*h)->next)) {
		if (*handle == *h) {
			*h = (*h)->next;

			MemRelease(handle);
			break;
		}
	}

	XplLockRelease(&XplAlarms.lock);


	if (!--XplAlarms.consumers) {
		XplLockAcquire(&XplAlarms.lock);

		while( XplAlarms.head )
		{
			removeAlarm( XplAlarms.head );
		}
		if (XplAlarms.queue) {
			DeleteTimerQueueEx(XplAlarms.queue, NULL);
			XplAlarms.queue = NULL;
		}
		XplLockRelease(&XplAlarms.lock);
	}
}

static int _XplAlarmWrapper( XplThread_ thread )
{
	_XplAlarm 			**ap, *a, *alarm;
	XplAlarmCallback	callback = NULL;
	void				*data;
	HANDLE				timerHandle = INVALID_HANDLE_VALUE;

	// find the alarm
	alarm = thread->context;
	XplLockAcquire(&XplAlarms.lock);
	for(ap=&XplAlarms.head;*ap;ap=&(*ap)->next)
	{
		if( *ap == alarm )
		{
			// copy out the callback and data in case it is non-repeatable
			callback = (*ap)->client.callback;
			data = (*ap)->client.data;
			if( !(*ap)->repeat )
			{
				// non-repeatable, destroy it now
				a = *ap;
				*ap = a->next;
				timerHandle = a->timerHandle;
				MemRelease( &a );
			}
			break;
		}
	}
	XplLockRelease(&XplAlarms.lock);

	if( INVALID_HANDLE_VALUE != timerHandle )
	{
		DeleteTimerQueueTimer( NULL, timerHandle, NULL );
	}
	if( callback )
	{
		callback( data );
	}
	return 0;
}

// executed on the timer thread, get done quick...
static void CALLBACK _XplAlarmCallback(PVOID alarm, unsigned char test)
{
	XplThreadStart( NULL, _XplAlarmWrapper, alarm, NULL );
}


EXPORT XplAlarmP _XplAlarmCreate(XplAlarmCallback callback, void *data, time_t delay, time_t repeat, XplAlarmHandle handle, const char *file, uint32 line)
{
	_XplAlarm *newAlarm;

	newAlarm = MemCallocWait(1, sizeof(_XplAlarm));

	newAlarm->client.callback	= callback;
	newAlarm->client.data		= data;
	newAlarm->repeat			= repeat;
	newAlarm->timerHandle		= NULL;

	newAlarm->handle			= handle;
	newAlarm->file				= file;
	newAlarm->line				= line;

	XplLockAcquire(&XplAlarms.lock);

	/* Insert it into the list of alarms for tracking purposes */
	newAlarm->next				= XplAlarms.head;
	XplAlarms.head				= newAlarm;

	XplLockRelease(&XplAlarms.lock);

	if( CreateTimerQueueTimer(&newAlarm->timerHandle, NULL,
		   _XplAlarmCallback, newAlarm,
		   (DWORD)(delay * 1000), (DWORD)(repeat * 1000),
			(repeat) ? WT_EXECUTEINTIMERTHREAD : WT_EXECUTEINTIMERTHREAD|WT_EXECUTEONLYONCE) )
	{
		return((XplAlarmP) newAlarm);
	}

	// set to invalid so remove doesn't try to call DeleteTimerQueueTimer
	XplLockAcquire(&XplAlarms.lock);
	newAlarm->timerHandle = INVALID_HANDLE_VALUE;
	removeAlarm( newAlarm );
	XplLockRelease(&XplAlarms.lock);
	return NULL;
}

EXPORT void XplAlarmDestroy(XplAlarmP alarm)
{
	_XplAlarm	*realAlarm = (_XplAlarm *)alarm;

	XplLockAcquire(&XplAlarms.lock);
	removeAlarm(realAlarm);
	XplLockRelease(&XplAlarms.lock);
}

#else

typedef struct _XplAlarm
{
	XplAlarmPublic		client;

	time_t				repeat;
	time_t				delay;
	time_t				lastIteration;

	struct _XplAlarm	*next;
	XplAlarmHandle		handle;

	const char			*file;
	uint32				line;
} _XplAlarm;

static struct
{
	int					consumers;

	XplLock				lock;
	struct sigaction	oldAction;

	_XplAlarm			*head;
	_XplAlarmHandle		*handles;
} XplAlarms;

static void _XplAlarmDestroy(_XplAlarm *alarm);

static void XplAlarmPrepareNextTick(time_t now)
{
	_XplAlarm	*a;
	time_t		nextTime;
	time_t		nextDelay;
	time_t		delay	= 0;

	for (a = XplAlarms.head; a; a = a->next) {
								/* base for next time alarm goes off */
		nextTime = a->lastIteration ? a->lastIteration : now;
		if (a->delay) {			/* use delay if specified */
			nextTime += a->delay;
		}
		else if (a->repeat) {	/* otherwise use repeat interval */
			nextTime += a->repeat;
		}

		if (nextTime > now) {	/* next time is in the future? */
								/* yes, what is that delta? */
			nextDelay = nextTime - now;
								/* update if currently unset or earlier */
			if (!delay || (nextDelay < delay)) {
				delay = nextDelay;
			}
		}
		else {					/* oops, passed the next run time */
			delay = 1;			/* so set for one second from now */
		}
	}

	if (delay) {
		alarm(delay);
	}
}

static int _XplAlarmThreadWrapper( XplThread_ thread )
{
	_XplAlarm	*a = (_XplAlarm *)thread->context;

	if (a->client.callback) {
		a->client.callback( a->client.data );
	}
	MemFree(a);
	return(0);
}

void XplAlarmSignal(int signum)
{
	time_t			now;
	_XplAlarm		*a, *chain, *newAlarm, *next;

	if (signum != SIGALRM) {
		return;
	}

	if (!XplAlarms.consumers) {
		return;
	}

	XplLockAcquire(&XplAlarms.lock);

	/*
		Build a list in memory of the events that have been triggered so that
		the lock can be released before calling the callbacks.
	*/
	now = time(NULL);
	chain = NULL;
	for (a = XplAlarms.head; a; a = next) {
		next = a->next;

		if ((a->lastIteration + a->delay) > (now + 1)) {
			continue;
		}

		newAlarm = MemCallocWait(1, sizeof(_XplAlarm));

		newAlarm->client.callback	= a->client.callback;
		newAlarm->client.data		= a->client.data;

		newAlarm->next				= chain;
		chain						= newAlarm;

		a->lastIteration			= now;

		if (!a->repeat) {
			_XplAlarmDestroy(a);
		} else {
			a->delay = a->repeat;
		}
	}

	XplAlarmPrepareNextTick(now);
	XplLockRelease(&XplAlarms.lock);

	while ((a = chain)) {
		chain = a->next;

		XplThreadStart( NULL, _XplAlarmThreadWrapper, a, NULL );
	}
}

XplAlarmHandle _XplAlarmInit(const char *file, uint32 line)
{
	_XplAlarmHandle	*handle;

	if (!XplAlarms.consumers++) {
		XplLockInit(&XplAlarms.lock);
	}

	handle			= MemCallocWait(1, sizeof(_XplAlarmHandle));
	handle->file	= file;
	handle->line	= line;

	XplLockAcquire(&XplAlarms.lock);

	handle->next = XplAlarms.handles;
	XplAlarms.handles = handle;

	XplLockRelease(&XplAlarms.lock);

	return((XplAlarmHandle) handle);
}

void XplAlarmShutdown(XplAlarmHandle *handle)
{
	_XplAlarmHandle	**h;
	_XplAlarm		*a, *next;

	if (!handle || !*handle) {
		return;
	}

	XplLockAcquire(&XplAlarms.lock);

	/* Destroy any alarms that are still out there for this handle */
	for (a = XplAlarms.head; a; a = next) {
		next = a->next;

		if (a->handle && a->handle == *handle) {
			_XplAlarmDestroy(a);
		}
	}

	for (h = &XplAlarms.handles; h && *h; h = &((*h)->next)) {
		if (*handle == *h) {
			*h = (*h)->next;

			MemRelease(handle);
			break;
		}
	}

	XplLockRelease(&XplAlarms.lock);


	if (!--XplAlarms.consumers) {
		XplLockAcquire(&XplAlarms.lock);
		while (XplAlarms.head) {
			_XplAlarmDestroy(XplAlarms.head);
		}
		alarm(0);

		XplLockRelease(&XplAlarms.lock);
	}
}

EXPORT XplAlarmP _XplAlarmCreate(XplAlarmCallback callback, void *data, time_t delay, time_t repeat, XplAlarmHandle handle, const char *file, uint32 line)
{
	_XplAlarm	*a;

	if ((a = MemCalloc(1, sizeof(_XplAlarm)))) {
		a->client.callback	= callback;
		a->client.data		= data;
		a->repeat			= repeat;
		a->delay			= delay;
		a->lastIteration	= time(NULL);

		a->handle			= handle;
		a->file				= file;
		a->line				= line;
	}

	XplLockAcquire(&XplAlarms.lock);

	a->next	= XplAlarms.head;
	XplAlarms.head	= a;

	XplLockRelease(&XplAlarms.lock);

	XplAlarmPrepareNextTick(time(NULL));
	return((XplAlarmP) a);
}

/* The lock must be held before calling this */
static void _XplAlarmDestroy(_XplAlarm *alarm)
{
	_XplAlarm	*prev;

	if (XplAlarms.head == alarm) {
		XplAlarms.head = alarm->next;
	} else {
		for (prev = XplAlarms.head; prev && prev->next != alarm; prev = prev->next);

		if (prev) {
			prev->next = alarm->next;
		} else {
			/* This alarm is not in the list... */
			return;
		}
	}

	MemFree(alarm);
}

void XplAlarmDestroy(XplAlarmP alarm)
{
	XplLockAcquire(&XplAlarms.lock);

	_XplAlarmDestroy((_XplAlarm *) alarm);
	XplAlarmPrepareNextTick(time(NULL));

	XplLockRelease(&XplAlarms.lock);
}

#endif
