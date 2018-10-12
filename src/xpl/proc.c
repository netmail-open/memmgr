#include "procp.h"

EXPORT XplPid XplProcGetID(XPLPROC *process)
{
	return process->pid;
}

/*
  XplProcWaitForReadable() will return TRUE when there is data to read and
  FALSE when the timeout has expired. The timeout is in milliseconds
*/

EXPORT XplBool XplProcWaitForReadable( XPLPROC *childProc, ProcStreamType type, int timeout )
{
	int internalTimeout;

	if( XplProcReadable( childProc, type ) ) {
		return TRUE;
	}

	internalTimeout = timeout;
	do {
		XplDelay( 100 );
		if( XplProcReadable( childProc, type ) ) {
			return TRUE;
		}
		internalTimeout -= 100;
	} while( internalTimeout > 0 );
	return FALSE;
}

EXPORT char * XplProcName( XPLPROC *proc )
{
	return proc->name;
}
