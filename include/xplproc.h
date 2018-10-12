#ifndef _XPLPROC_H

typedef struct _XPLPROC {
	void *doNotUse;
} XPLPROC;


#define _XPLPROC_H
#endif

#define EXIT_NOCHDIR 0200
#define EXIT_RESTART 0201		/* agent requests launcher to restart */

typedef enum {
	PROC_SIGNAL_TERM,
	PROC_SIGNAL_INT,
	PROC_SIGNAL_KILL,
	PROC_SIGNAL_USR1,
	PROC_SIGNAL_USR2,
	PROC_SIGNAL_CORE
} ProcSignal;

typedef enum {
	PROC_STDIN,
	PROC_STDOUT,
	PROC_STDERR
} ProcStreamType;

typedef enum {
	PROC_UNKNOWN,
	PROC_CRASHED,
	PROC_KILLED,
	PROC_EXITED
} CauseOfDeath;

typedef enum {
	JOIN_SUCCESS,
	JOIN_ERROR,
	JOIN_TIMEOUT
} JoinRetVal;

typedef struct {
	CauseOfDeath cause;
	char *description;
	int returnValue;
	XplBool core;
} AutopsyReport;

#define XplProcJoin( c, r )       XplProcJoinEx( ( c ), ( r ), 0 )

EXPORT void XplProcThreadSafeInit( void );
EXPORT void XplProcFree( XPLPROC **childProc );
/* Paths that are base on CMAKE paths may have environment variables in them on Windows.  
   XplProcExec() and XplProcExecDetached() know how to interpret these paths when they are
   used for the work directory or the executable paths.  Paths that are passed as arguments
   to the executable must be expanded before they are passed to these functions.
*/

EXPORT XPLPROC *XplProcExec(char *workdir, const char **args );
EXPORT int XplProcExecDetached(char *workdir, const char **args );
EXPORT JoinRetVal XplProcJoinEx( XPLPROC *childProc, AutopsyReport *report, unsigned int timeout );
EXPORT JoinRetVal XplProcJoinGroupEx( XPLPROC *childProc, AutopsyReport *report, unsigned int timeout );
EXPORT int XplProcSignal( XPLPROC *childProc, ProcSignal signal );
EXPORT int XplProcSignalGroup( XPLPROC *childProc, ProcSignal signal );
EXPORT void XplProcFlush( XPLPROC *childProc );
EXPORT XplPid XplProcGetID(XPLPROC *process);
EXPORT XplBool XplProcReadable( XPLPROC *childProc, ProcStreamType type );
EXPORT XplBool XplProcWaitForReadable( XPLPROC *childProc, ProcStreamType type, int timeout );
EXPORT int XplProcRead( XPLPROC *childProc, ProcStreamType type, char *buffer, size_t bufferLen );
EXPORT int XplProcWrite( XPLPROC *childProc, char *data, size_t dataLen );
EXPORT char * XplProcName( XPLPROC *proc );
