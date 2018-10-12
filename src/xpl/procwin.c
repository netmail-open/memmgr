#include "procp.h"

EXPORT HANDLE XplProcHandle( XPLPROC *childProc, ProcStreamType type );
EXPORT int XplProcReadTD( HANDLE *handle, char *buffer, size_t length );

typedef struct {
        char *buffer;
        int count;
        int longest;
        char **str;
} StrList;


static XPLPROC *procAlloc( void )
{
	XPLPROC *proc ;

	if( ( proc = ( XPLPROC * )MemMalloc( sizeof( XPLPROC ) ) ) ) {
		memset( proc, 0, sizeof( XPLPROC ) );
	}
	return proc;
}

static void procFree( XPLPROC **proc )
{
	MemRelease( proc );
}

static StrList *getExtList( void )
{
	char *extVar;
	char *localExt;
	StrList *list;
	int index;
	char *ptr;

	extVar = getenv( "PATHEXT" );
	if( !extVar ) {
		/* default to something reasonable */
		extVar = ".EXE; .BAT";
	}

	localExt = MemStrdup( extVar );
	if( localExt ) {
		list = MemMalloc( sizeof( StrList ) ) ;
		if( list ) {
			memset( list, 0, sizeof( StrList ) );
			list->buffer = localExt;
			if( *( list->buffer ) == '\0' ) {
				return list;
			}
			ptr = list->buffer;
			for( ; ; ) {
				list->count++;
				ptr = strchr( ptr, ';' );
				if( !ptr ) {
						break;
				}
				ptr++;
			}
			list->str = MemMalloc( list->count * sizeof( char * ) );
			if( list->str ) {
				ptr = list->buffer;
				index = 0;
				for( ; ; ) {
					list->str[ index ] = ptr;
					ptr = strchr( ptr, ';' );
					if( !ptr ) {
						list->longest = max( list->longest, strlen( list->str[ index ] ) );
						break;
					}
					list->longest = max( list->longest, ( ptr - list->str[ index ] ) );
					*ptr = '\0';
					ptr++;
					index++;
				}
				return list;
			}
			MemFree( list );
		}
		MemFree( localExt );
	}
	return NULL;
}

static void freeExtList( StrList **list )
{
	if( list && *list ) {
		if( (*list)->buffer ) {
			MemFree( (*list)->buffer );
		}
		if( (*list)->str ) {
			MemFree( (*list)->str );
		}
		MemFree( *list );
		*list = NULL;
	}
}

static const char *strTrim( const char *str, int strLen, int *trimmedLen )
{
	const char *ptr;
	const char *start;

	ptr = str;
	while( ptr < ( str + strLen ) ) {
		if( ( *ptr != ' ' ) && ( *ptr != '\t' ) ) {
			break;
		}
		ptr++;
	}
	start = ptr;
	ptr = str + strLen - 1;
	while( ptr >= str ) {
		if( ( *ptr != ' ' ) && ( *ptr != '\t' ) ) {
			ptr++;
			break;
		}
		ptr--;
	}
	if( ptr > start ) {
		*trimmedLen = ptr - start;
	} else {
		*trimmedLen = 0;
	}
	return start;
}

static char *findExecutable( const char *path )
{
	int len;
	int i;
	StrList *list;
	char *foundPath;
	const char *start;

	foundPath = NULL;
	len = strlen( path );

	if( len ) {
		if( !access( path, 0 ) ) {
			foundPath = MemStrdup( path );
		} else if( ( path[ len - 1 ] != '\\' ) && ( path[ len - 1 ] != '/' ) ) {
			if( ( list = getExtList() ) ) {
				if( ( foundPath = MemMalloc( len + list->longest + 1 ) ) ) {
					/* trim trailing and leading whitespace */
					start = strTrim( path, len, &len );
					memcpy( foundPath, start, len );
					for( i = 0; i < list->count; i++ ) {
						strcpy( foundPath + len, list->str[ i ] );
						if( !access( foundPath, 0 ) ) {
							break;
						}
					}
					if( i == list->count ) {
						MemFree( foundPath );
						foundPath = NULL;
					}
				}
				freeExtList( &list );
			}
		}
	}
	return foundPath;
}

static XplBool isAbsolute(const char *path)
{
	if (!path || !*path) {
		return(FALSE);
	}

	if ('\\' == path[0] && isalnum(path[1])) {
		return(TRUE);
	}

	if (isalpha(path[0]) && ':' == path[1] && ( '\\' == path[2] ) || ( '/' == path[2] ) && path[3]) {
		/* Allow a path starting with a drive letter */
		return(TRUE);
	}

	return(FALSE);
}

static void appendStr( char **where, char *what, int len )
{
	memcpy( *where, what, len );
	*where = *where + len;
	**where = '\0';
}

static void quoteAndAppendStr( char **where, char *what )
{
	char *ptr;

	**where = '\"';
	*where = *where + 1;
	ptr = what;
	while( *ptr ) {
		if( *ptr == '"' ) {
			**where = '\\';
			*where = *where + 1;
		}
		**where = *ptr;
		*where = *where + 1;
		ptr++;
	}
	**where = '\"';
	*where = *where + 1;

}

static XplBool appendSafeArgument( char **where, const char *What )
{
	XplBool ret;
	const char *ptr;
	char *what;
	int len;

	ptr = strTrim( What, strlen( What ), &len );
	if( !len ) {
		ret = TRUE;
	} else if( !( what = MemMalloc( len + 1 ) ) ) {
		ret = FALSE;
	} else {
		ret = TRUE;
		memcpy( what, ptr, len );
		what[ len ] = '\0';

		if (!strchrs(what, " \t:/\\=") || '/' == *what) {
			/* no need to put this in quotes */
			appendStr( where, what, len );
		} else if( ( *what == '"' ) && ( *( what + len - 1 ) == '"' ) ) {
			/* this is already in quotes */
			appendStr( where, what, len );
		} else {
			/* this needs quoted */
			quoteAndAppendStr( where, what );
		}
		MemFree( what );
	}
	return ret;
}

static char *buildCommandLine( char *executablePath, const char **args, char **appName, char **fileName )
{
	char *cmdLine;
	char *ptr;
	int maxCmdLineLen;
	int i;
	/*
	  Reserve room for escaped chars in quoted string ( x 2 )
	  as well as the quotes ( + 2)
	  and the space delimiter or the terminating '\0'
	*/
	maxCmdLineLen = ( strlen( executablePath ) * 2 ) + 2 + 1;
	i = 1;
	while( args[ i ] ) {
		maxCmdLineLen += ( strlen( args[ i ] ) * 2 ) + 2 + 1;
		i++;
	}
	if( ( cmdLine = ( char * )MemMalloc( maxCmdLineLen ) ) ) {
		if( ( ptr = strrchr( executablePath, '\\' ) ) || ( ptr = strrchr( executablePath, '/' ) ) ) {
			*fileName = ptr + 1;
		} else {
			*fileName = executablePath;
		}
		ptr = strrchr( executablePath, '.' );
		if( ptr && !stricmp( ".bat", ptr ) ) {
			*appName = NULL;
		} else {
			*appName = executablePath;
		}
		ptr = cmdLine;
		appendSafeArgument( &ptr, executablePath );
		i = 1;
		while( args[ i ] ) {
			*ptr = ' ';
			ptr++;
			appendSafeArgument( &ptr, args[ i ] );
			i++;
		}
		*ptr = '\0';
		return cmdLine;
	}

	return NULL;
}

static void rbInit( ProcReadableStruct *rb, XPLPROC *proc, HANDLE *rhandle )
{
	rb->rhandle = rhandle;
	rb->proc = proc;
	rb->readPtr = rb->writePtr = rb->buffer;
}

EXPORT void XplProcThreadSafeInit( void )
{
}

static XplBool SetUpJob( XPLPROC *proc )
{
	proc->hJob = CreateJobObject( NULL, NULL );
	if( proc->hJob ) {
		if( AssignProcessToJobObject( proc->hJob, proc->hProcess ) ) {
			if( proc->iocp = CreateIoCompletionPort( INVALID_HANDLE_VALUE, NULL, 0, 1 ) ) {
				proc->port.CompletionKey = proc->hJob;
				proc->port.CompletionPort = proc->iocp;
				if( SetInformationJobObject( proc->hJob, JobObjectAssociateCompletionPortInformation, &proc->port, sizeof( proc->port ) ) ) {
					return TRUE;
				}
				CloseHandle( proc->iocp );
				proc->iocp = NULL;
			}
		}
		CloseHandle( proc->hJob );
		proc->hJob = NULL;
	}
	return FALSE;
}


static int _CreateIOHandle( HANDLE *readP, HANDLE *writeP, XplBool output )
{
	SECURITY_ATTRIBUTES saAttr;

	// Set the bInheritHandle flag so pipe handles are inherited.
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;
	// Create a pipe for the child process's STDOUT.
	if( CreatePipe( readP, writeP, &saAttr, 0 ) )
	{
		// Ensure the read handle to the pipe for STDOUT is not inherited.
		if( SetHandleInformation( (output) ? *readP : *writeP, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT ) )
		{
			return 0;
		}
		CloseHandle( *readP );
		CloseHandle( *writeP );
	}
	return -1;
}

static char *_ExpandPath( const char *path )
{
	char	*expanded;

	errno = ENOMEM;
	if( expanded = MemMalloc( XPL_MAX_PATH + 1 ) )
	{
		if( !XplExpandEnv( expanded, path, XPL_MAX_PATH ) )
		{
			return expanded;
		}
		MemRelease( &expanded );
	}
	return NULL;
}

/* Create a child process
 * Accepts:
 *              - absolute path of working directory or NULL
 *          - NULL will cause the child process to have the same working
 *            directory as the parent process.
 *              - argument vector
 *          - argv[0] must have absolute path of app
 * Returns: XPLPROC if success, NULL if failure with errno set
 */

EXPORT XPLPROC *XplProcExec( char *workdir, const char **args )
{
	CRITICAL_SECTION	cs;
	XPLPROC				*proc = NULL;
	PROCESS_INFORMATION	pi;
	STARTUPINFO			sui;
	BOOL				success = FALSE;
	HANDLE				stdInRd = NULL;
	HANDLE				stdInWr = NULL;
	HANDLE				stdOutRd = NULL;
	HANDLE				stdOutWr = NULL;
	HANDLE				stdErrRd = NULL;
	HANDLE				stdErrWr = NULL;
	char				*executablePath;
	char				*commandLine;
	char				*appName;
	char				*fileName;
	char				*exWorkDir;
	char				*exExecPath;
	int					err;

	errno = 0;
	err = 0;
	exWorkDir = NULL;
	exExecPath = NULL;

	if( !args[ 0 ] || !args[ 0 ][ 0 ] )
	{
		errno = EINVAL;
		return NULL;
	}
	err = ENOMEM;
	InitializeCriticalSection( &cs );
	EnterCriticalSection( &cs );
	if( exExecPath = _ExpandPath( args[0] ) )
	{
		err = ENOENT;
		if( executablePath = findExecutable( exExecPath ) )
		{
			err = ENOMEM;
			if( ( commandLine = buildCommandLine( executablePath, args, &appName, &fileName ) ) )
			{
				/* if workdir is given, it must be absolute */
				if( !workdir || ( exWorkDir = _ExpandPath( workdir ) ) )
				{
					err = EINVAL;
					if( !exWorkDir || isAbsolute( exWorkDir ) )
					{
						if( ( proc = procAlloc() ) )
						{
							strprintf( proc->name, sizeof(proc->name), NULL ,"%s", fileName );
							if( !_CreateIOHandle( &stdInRd, &stdInWr, FALSE ) )
							{
								if( !_CreateIOHandle( &stdOutRd, &stdOutWr, TRUE ) )
								{
									if( !_CreateIOHandle( &stdErrRd, &stdErrWr, TRUE ) )
									{
										// Set up members of the PROCESS_INFORMATION structure.
										memset( &pi, 0, sizeof( PROCESS_INFORMATION ) );
										// Set up members of the STARTUPINFO structure.
										memset( &sui, 0, sizeof(STARTUPINFO) );
										sui.cb = sizeof(STARTUPINFO);
										// Specify the STDIN, STDOUT and STDERR handles for redirection.
										sui.hStdError = stdErrWr;
										sui.hStdOutput = stdOutWr;
										sui.hStdInput = stdInRd;
										sui.dwFlags |= STARTF_USESTDHANDLES;

										// Create the child process.
										success = CreateProcess(appName,			// usually same as the first arg of the command line
																commandLine,		// command line
																NULL,				// process security attributes
																NULL,				// primary thread security attributes
																TRUE,				// handles are inherited
																CREATE_SUSPENDED | CREATE_NEW_PROCESS_GROUP,
																NULL,				// use parent's environment
																exWorkDir,			// use parent's current directory
																&sui,				// STARTUPINFO pointer
																&pi);				// receives PROCESS_INFORMATION
										if( success ) {
											/*
											   now that the child has these three
											   handles, the parent needs to close
											   its references to them, otherwise
											   the pipe can not properly close
											   when the child exits.
											*/
											CloseHandle( stdErrWr );
											CloseHandle( stdOutWr );
											CloseHandle( stdInRd );

											// collect handles that belong to the parent side of the pipes in proc struct
											proc->ihandle = stdInWr;
											proc->ohandle = stdOutRd;
											proc->ehandle = stdErrRd;
											// keep the child's process handle for the parent to reference
											proc->pid = pi.dwProcessId;
											proc->hProcess = pi.hProcess;
											XplLockInit( &( proc->lock ) );
											rbInit( &proc->rbOut, proc, &( proc->ohandle ) );
											rbInit( &proc->rbErr, proc, &( proc->ehandle ) );
											if( SetUpJob( proc ) ) {
												ResumeThread( pi.hThread );
												CloseHandle( pi.hThread );
												MemFree( commandLine );
												MemFree( executablePath );
												MemRelease( &exWorkDir );
												MemRelease( &exExecPath );
												LeaveCriticalSection( &cs );
												errno = 0;
												return proc;
											}
											TerminateProcess( pi.hProcess, 0xc00c0005 );
											CloseHandle( pi.hThread );
										}
									}
								}
							}
							err = XplTranslateError( GetLastError() );
							procFree( &proc );
						}
					}
					MemRelease( &exWorkDir );
				}
				MemRelease( &commandLine );
			}
			MemRelease( &executablePath );
		}
		MemRelease( &exExecPath );
	}
	LeaveCriticalSection( &cs );
	errno = err;
	return NULL;
}

/* Create detached process
 * Accepts:
 *              - absolute path of working directory or NULL
 *          - NULL will cause the new process to have the same working
 *            directory as the launching process.
 *              - argument vector
 * Returns: 0 on success, non-zero otherwise
 */

EXPORT int XplProcExecDetached(char *workdir, const char **args)
{
	int err = 0;
	int ret = -1;
	char *executablePath;
	char *parentWorkDir;
	char *exWorkDir;
	char *exExecPath;

	ret = -1;
	parentWorkDir = NULL;
	exWorkDir = NULL;
	exExecPath = NULL;

	if( workdir && ( !( exWorkDir = MemMalloc( XPL_MAX_PATH + 1 ) ) ||
		XplExpandEnv( exWorkDir, workdir, XPL_MAX_PATH ) ) ||
		( !( parentWorkDir = MemMalloc( XPL_MAX_PATH + 1 ) ) ) ) {
			/* get the expanded version of the work dir */
			err = ENOMEM;
	} else if( exWorkDir && !isAbsolute( exWorkDir ) ) {
		/* if workdir is given, it must be absolute */
		err = EINVAL;
	} else if( !args[ 0 ] || !args[ 0 ][ 0 ] ) {
		err = EINVAL;
	} else if( !( exExecPath = MemMalloc( XPL_MAX_PATH + 1 ) ) ||
		XplExpandEnv( exExecPath, args[ 0 ], XPL_MAX_PATH ) ) {
		err = ENOMEM;
	} else if( NULL == ( executablePath = findExecutable( exExecPath ) ) ) {
		err = ENOENT;
	} else {
		if( exWorkDir ) {
			GetCurrentDirectory( XPL_MAX_PATH, parentWorkDir );
			SetCurrentDirectory( exWorkDir );
			/* FIXME - Is there a better way to do this?  If another thread in the parent
			is using the current directory, this could mess them up */
		}

		if( !spawnvp(P_NOWAIT, executablePath, args ) ) {
			err = ENOMEM;
		} else {
			ret = 0;
		}
		if( exWorkDir ) {
			SetCurrentDirectory( parentWorkDir );
		}

		MemFree( executablePath );
	}
	if( err ) errno = err;

	MemRelease( &exWorkDir );
	MemRelease( &exExecPath );
	MemRelease( &parentWorkDir );
	return ret;
}

static int winSignal(DWORD pid, int message)
{
	int			count	= 0;
	DWORD		wpid	= 0;
	HWND		w;

	/* Search all top level windows */
	for (w = NULL; w = FindWindowEx(NULL, w, NULL, NULL); ) {
		GetWindowThreadProcessId(w, &wpid);

		if (wpid == pid) {
			PostMessage(w, message, 0, 0);
			count++;
		}
	}

	/* Search all message only windows */
	for (w = NULL; w = FindWindowEx(HWND_MESSAGE, w, NULL, NULL); ) {
		GetWindowThreadProcessId(w, &wpid);

		if (wpid == pid) {
			PostMessage(w, message, 0, 0);
			count++;
		}
	}

	return(count);
}

/* Sends a signal to a child process
 * Accepts:
 *              - XPLPROC pointer
 *              - the signal to be sent
 * Returns: 0 on success, non-zero otherwise
 */
EXPORT int XplProcSignal( XPLPROC *childProc, ProcSignal signal )
{
	int count	= 0;

	if( !childProc || childProc->reaped ) {
		errno = ENOENT;
		return -1;
	}
	switch( signal ) {
		case PROC_SIGNAL_INT:
			count = winSignal(childProc->pid, WM_DESTROY);
			if (!count) {
				/* Generate a console control event if there was no window */
				count = GenerateConsoleCtrlEvent(CTRL_C_EVENT, childProc->pid);
			}
			break;
		case PROC_SIGNAL_USR1:
			count = winSignal(childProc->pid, WM_USER + 1);
			break;
		case PROC_SIGNAL_USR2:
			count = winSignal(childProc->pid, WM_USER + 2);
			break;
		case PROC_SIGNAL_TERM:
			count = winSignal(childProc->pid, WM_CLOSE);

			if (!count) {
				/* Generate a console control event if there was no window */
				count = GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, childProc->pid);
			}

			if (count > 0) {
				break;
			}
			/*
				Fallthrough. A count of 0 means that the process had no windows
				that could receive a message. So, fallback to terminating the
				process.
			*/
		case PROC_SIGNAL_KILL:
			count = TerminateProcess(childProc->hProcess, 0xc00c0005);
			break;
	}

	if (count) {
		return(0);
	} else {
		//return(GetLastError());
		return(-1);
	}
}

/* Sends a signal to a child process and it's children
 * Accepts:
 *              - XPLPROC pointer
 *              - the signal to be sent
 * Returns: 0 on success, non-zero otherwise
 */
EXPORT int XplProcSignalGroup( XPLPROC *childProc, ProcSignal signal )
{
	int count = 0, c;
	int i;
	PJOBOBJECT_BASIC_PROCESS_ID_LIST pList;
	size_t size = 4 * 1024;

	if( !childProc || childProc->reaped ) {
		errno = ENOENT;
		return -1;
	}

	switch (signal) {
		case PROC_SIGNAL_TERM:
			/*
				A console control event can only be generated if we are sure
				that the process has it's own console. If attempted on a process
				that doesn't then we will receive the event instead.

				Generate the event here and also attempt to message each child.
			*/
			count = GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, childProc->pid);

			/* Fall through */

		case PROC_SIGNAL_INT:
		case PROC_SIGNAL_USR1:
		case PROC_SIGNAL_USR2:
			pList = MemMallocWait(size);

			if (!QueryInformationJobObject(childProc->hJob, JobObjectBasicProcessIdList, pList, size, NULL)) {
				MemRelease(&pList);

				errno = EINVAL;
				return -1;
			}

			for (i = 0; i < pList->NumberOfProcessIdsInList; i++) {
				switch (signal) {
					case PROC_SIGNAL_TERM:
						c = winSignal(pList->ProcessIdList[i], WM_CLOSE);
						break;

					case PROC_SIGNAL_INT:
						c = winSignal(pList->ProcessIdList[i], WM_DESTROY);
						break;

					case PROC_SIGNAL_USR1:
						c = winSignal(pList->ProcessIdList[i], WM_USER + 1);
						break;

					case PROC_SIGNAL_USR2:
						c = winSignal(pList->ProcessIdList[i], WM_USER + 2);
						break;

					default:
						c = 0;
						break;
				}

				count += c;
			}
			MemRelease(&pList);
			break;

		case PROC_SIGNAL_KILL:
			/*
				Terminating the job is effectively the same as calling
				TerminateProcess on each child process.
			*/
			count = TerminateJobObject(childProc->hJob, 0xc00c0006);
			break;
	}

	if (count) {
		return(0);
	} else {
		//return(GetLastError());
		return(-1);
	}
}


/* Blocks until the child process terminates
 * Accepts:
 *              - XPLPROC pointer
 *              - optional pointer to an int
 */
EXPORT JoinRetVal XplProcJoinEx( XPLPROC *childProc, AutopsyReport *report, unsigned int timeout )
{
	DWORD waitRet;

	JoinRetVal joinRet;
	CauseOfDeath cause;
	char *description;
	DWORD childRet;
	XplBool core;

	cause = PROC_UNKNOWN;
	description = "UNKNOWN";
	childRet = 0;
	core = FALSE;

	SetLastError(0);
	if( !childProc || childProc->reaped ) {
		joinRet = JOIN_ERROR;
	} else while( TRUE ) {
		if( !childProc->done ) {
			if( WAIT_TIMEOUT == ( waitRet = WaitForSingleObject( childProc->hProcess, timeout ? timeout: INFINITE ) ) ) {
				joinRet = JOIN_TIMEOUT;
				break;
			}
			if( GetExitCodeProcess( childProc->hProcess, &childRet ) ) {
				if( 0xc0000005 == childRet ) {
					/*
					  Windows appears to set this value when a process crashes.
					  I did not find offical documentation on this, but
					  discovered it emperically.
					*/
					description = "CRASHED";
					cause = PROC_CRASHED;
					childRet = 0;
				} else if( 0xc00c0005 == childRet ) {
					/*
					  When we call TerminateProcess() we send it 0xc00c0005
					  There is nothing official about this number.  I made it
					  up.  We have no control over what number is sent by other
					  sources.  Task Manager, for example, sends 1.  Microsoft
					  does not provide a way for us to tell if the application
					  was kill by task manager or simply exited with a value of
					  1.
					 */
					description = "KILLED";
					cause = PROC_KILLED;
					childRet = 0;
				} else {
					description = "EXITED";
					cause = PROC_EXITED;
				}
			} else {
				cause = PROC_KILLED;
			}
			childProc->done = TRUE;
		}
		joinRet = JOIN_SUCCESS;
		break;
	}
	if( report ) {
		report->cause = cause;
		report->returnValue = (int)childRet;
		report->core = core;
		report->description = description;
	}
	return joinRet;
}

/* Blocks until the child process and it's children terminate
 * Accepts:
 *              - XPLPROC pointer
 *              - optional pointer to an int
 */
EXPORT JoinRetVal XplProcJoinGroupEx( XPLPROC *childProc, AutopsyReport *report, unsigned int timeout )
{
	DWORD completionCode;
	JoinRetVal joinRet;
	ULONG_PTR completionKey;
	LPOVERLAPPED overlapped;

	joinRet = XplProcJoinEx( childProc, report, timeout);

	if (JOIN_SUCCESS != joinRet) {
		return(joinRet);
	}

	if ( childProc && childProc->done && !childProc->reaped ) {
		while( TRUE ) {
			if( !GetQueuedCompletionStatus( childProc->iocp, &completionCode, &completionKey, &overlapped, timeout ? timeout: INFINITE ) ) {
				if( NULL == overlapped ) {
					joinRet = JOIN_TIMEOUT;
					break;
				}
			} else if( JOB_OBJECT_MSG_ACTIVE_PROCESS_ZERO == completionCode ) {
				joinRet = JOIN_SUCCESS;
				childProc->reaped = 1;
				break;
			}
		}
	}

	return(joinRet);
}

#if 0
static XplBool rbPointersOK( ProcReadableStruct *rb )
{
	char *endPtr;
	if( rb->writePtr < rb->readPtr ) {
		DebugAssert( 0 );
		return FALSE;
	}
	endPtr = rb->buffer + PROC_READ_BUFF_SIZE;
	if( ( rb->writePtr < rb->buffer ) ) {
		DebugAssert( 0 );
		return FALSE;
	}
	if( rb->writePtr > endPtr ) {
		DebugAssert( 0 );
		return FALSE;
	}
	if( ( rb->readPtr < rb->buffer ) ) {
		DebugAssert( 0 );
		return FALSE;
	}
	if( rb->readPtr > endPtr ) {
		DebugAssert( 0 );
		return FALSE;
	}

	return TRUE;
}

static DWORD rbThread( void *arg )
{
	DWORD readCount;
	ProcReadableStruct *rb;
	int len;
	int ret;
	int err;
	XplBool itWorked;

	rb = ( ProcReadableStruct * )arg;
	err = 0;

	XplLockAcquire( &( rb->proc->lock ) );
	DebugAssert( rbPointersOK( rb ) );
	DebugAssert( rb->state == PROC_READING );
	while( 0 == err ) {
		if( rb->readPtr > rb->buffer ) {
			len = rb->writePtr - rb->readPtr;
			if( len ) {
				memmove( rb->buffer, rb->readPtr, len );
			}
			rb->readPtr = rb->buffer;
			rb->writePtr = rb->buffer + len;
			DebugAssert( rbPointersOK( rb ) );
		}
		len = ( rb->buffer + PROC_READ_BUFF_SIZE ) - rb->writePtr;
		if( len > 0 ) {
			rb->firstReadStarted = TRUE;
			XplLockRelease( &rb->proc->lock );
			itWorked = ReadFile( *( rb->rhandle ), rb->writePtr, len, &readCount, NULL );
			XplLockAcquire( &rb->proc->lock );
			rb->firstReadCompleted = TRUE;
			if( itWorked ) {
				ret = readCount;
				if( ret > 0 ) {
					rb->writePtr += ret;
					DebugAssert( rbPointersOK( rb ) );
				} else {
					/* the read thinks it hit the end of stream */
					break;
				}
			} else {
				ret = -1;
				errno = XplTranslateError( GetLastError() );
				err = errno;
				if( !err ) {
					err = EIO;
					DebugAssert( 0 );
				}
			}
		} else {
			/* the buffer is full - spin */
			XplLockRelease( &rb->proc->lock );
			XplDelay( 500 );
			XplLockAcquire( &rb->proc->lock );
		}
	}
	DebugAssert( rb->state == PROC_READING );
	rb->state = PROC_POST;
	XplLockRelease( &rb->proc->lock );
	return 0;
}

static ProcReadableStruct *rbGet( XPLPROC *childProc, ProcStreamType type )
{
	ProcReadableStruct *rb;
	XplBool needThread;
	HANDLE thandle;

	errno = 0;
	switch( type ) {
	case PROC_STDOUT:
		rb = &childProc->rbOut;
		break;
	case PROC_STDERR:
		rb = &childProc->rbErr;
		break;
	case PROC_STDIN:
		DebugAssert( 0 );  // can't read from child's stdin
		errno = EINVAL;
		return NULL;
	}

	XplLockAcquire( &( childProc->lock ) );
	if( rb->state == PROC_PRE ) {
		rb->state = PROC_READING;
		needThread = TRUE;
	} else {
		needThread = FALSE;
	}
	XplLockRelease( &( childProc->lock ) );
	if( needThread ) {
		thandle = CreateThread( NULL, PROC_READ_STACK_SIZE, (LPTHREAD_START_ROUTINE)rbThread, rb, 0, NULL );
		XplLockAcquire( &( childProc->lock ) );
		if( thandle ) {
			if( rb->state != PROC_POST ) {
				rb->thandle = thandle;
			}
		} else {
			DebugAssert( 0 ); // a read thread did not launch
			rb->state = PROC_PRE;
			rb = NULL;
		}
		XplLockRelease( &( childProc->lock ) );
	}
	return rb;
}

/* XplProcReadable

   This function never blocks. It return TRUE is there is data to read,
   FALSE if not.
*/
EXPORT XplBool XplProcReadable( XPLPROC *childProc, ProcStreamType type )
{
	ProcReadableStruct *rb;
	int len;

	rb = rbGet( childProc, type );
	if( !rb ) {
		return FALSE;
	}
	while( !rb->firstReadStarted ) {
		/* give the rbthread time to spin up and call read */
		XplDelay( 100 );
	}
	XplLockAcquire( &rb->proc->lock );
	DebugAssert( rbPointersOK( rb ) );
	len = rb->writePtr - rb->readPtr;
	XplLockRelease( &rb->proc->lock );
	if( len > 0 ) {
		return TRUE;
	}
	return FALSE;
}
#endif

EXPORT XplBool XplProcReadable( XPLPROC *childProc, ProcStreamType type )
{
	HANDLE	handle;
	DWORD	avail;

	if( handle = XplProcHandle( childProc, type ) )
	{
		avail = 0;
		if( PeekNamedPipe( handle, NULL, 0, NULL, &avail, NULL ) && avail > 0 )
		{
			return TRUE;
		}
//		error = GetLastError();
	}
	return FALSE;
}

/*
  XplProcRead reads as much data as it can into the buffer. If there is space
  leftover, the buffer will be terminated.
*/
#if 0
EXPORT int XplProcRead( XPLPROC *childProc, ProcStreamType type, char *buffer, size_t bufferLen )
{
	ProcReadableStruct *rb;
	int readCount;

	rb = rbGet( childProc, type );
	if( !rb ) {
		return -1;
	}
	while( !rb->firstReadCompleted ) {
		/*
		  give the rbthread time to return from read
		  the first time. Read should block if there
		  is nothing to read.
		*/
		XplDelay( 100 );
	}

	XplLockAcquire( &rb->proc->lock );
	DebugAssert( rbPointersOK( rb ) );
	readCount = rb->writePtr - rb->readPtr;
	if( readCount > 0 ) {
		if( readCount > bufferLen ) {
			readCount = bufferLen;
		}
		memcpy( buffer, rb->readPtr, readCount );
		rb->readPtr += readCount;
		DebugAssert( rbPointersOK( rb ) );
	}
	XplLockRelease( &rb->proc->lock );

	if( readCount < bufferLen ) {
		buffer[ readCount ] = '\0';
	}

	return readCount;
}
#endif

EXPORT HANDLE XplProcHandle( XPLPROC *childProc, ProcStreamType type )
{
	switch( type )
	{
		case PROC_STDOUT:
			return childProc->ohandle;

		case PROC_STDERR:
			return childProc->ehandle;
			break;

		default:
		case PROC_STDIN:
			DebugAssert( 0 );  // can't read from child's stdin
	}
	errno = EINVAL;
	return NULL;
}

EXPORT int XplProcReadTD( HANDLE *handle, char *buffer, size_t length )
{
	DWORD	bytes, avail, error;

	if( *handle )
	{
		avail = 0;
		if( PeekNamedPipe( *handle, NULL, 0, NULL, &avail, NULL ) && avail > 0 )
		{
			if (avail > length) {
				avail = length;
			}

			if( ReadFile( *handle, buffer, avail, &bytes, NULL ) )
			{
				return (int)bytes;
			}
		}
		error = GetLastError();
		if( 109 == error )
		{
			// broken pipe
			*handle = NULL;
		}
	}
	return 0;
}

EXPORT int XplProcRead( XPLPROC *childProc, ProcStreamType type, char *buffer, size_t bufferLen )
{
	HANDLE	handle;

	if( handle = XplProcHandle( childProc, type ) )
	{
		return XplProcReadTD( &handle, buffer, bufferLen );
	}
	return -1;
}

#if 0
EXPORT int XplProcReadTD( XPLPROC *childProc, ProcStreamType type, char *buffer, size_t length )
{
	HANDLE	handle;
	DWORD	readBytes, error;

	if( !childProc )
	{
		errno = EINVAL;
		return -1;
	}
	switch( type )
	{
		case PROC_STDOUT:
			handle = childProc->ohandle;
			break;

		case PROC_STDERR:
			handle = childProc->ehandle;
			break;

		default:
		case PROC_STDIN:
			DebugAssert( 0 );  // can't read from child's stdin
			errno = EINVAL;
			return -1;
	}
	if( handle )
	{
		if( PeekNamedPipe( handle, buffer, length, &readBytes, NULL, NULL ) )
		{
			DWORD	bytes;
			if( ReadFile( handle, buffer, readBytes, &bytes, NULL ) )
			{
				return (int)bytes;
			}
		}
		error = GetLastError();
	}
	return 0;
}
#endif

EXPORT int XplProcWrite( XPLPROC *childProc, char *data, size_t dataLen )
{
	DWORD dwWritten;

	if( !WriteFile( childProc->ihandle, data, dataLen, &dwWritten, NULL ) ) {
		dwWritten = 0;
	}
	return dwWritten;
}

static void procShutdown( XPLPROC *proc )
{
	XPLPROC *procP;
	if( proc && proc->hProcess ) {
		procP = proc->hProcess;
		proc->hProcess = NULL;
		if( proc->rbOut.thandle ) {
#ifndef WATCOM
			CancelSynchronousIo( proc->rbOut.thandle );
#endif
			CloseHandle( proc->rbOut.thandle );
			proc->rbOut.thandle = NULL;
		}
		if( proc->rbErr.thandle ) {
#ifndef WATCOM
			CancelSynchronousIo( proc->rbErr.thandle );
#endif
			CloseHandle( proc->rbErr.thandle );
			proc->rbErr.thandle = NULL;
		}
		if( proc->ihandle ) {
			CloseHandle( proc->ihandle );
			proc->ihandle = NULL;
		}
		if( proc->ohandle ) {
			CloseHandle( proc->ohandle );
			proc->ohandle = NULL;
		}
		if( proc->ehandle ) {
			CloseHandle( proc->ehandle );
			proc->ehandle = NULL;
		}
		if( procP ) {
			CloseHandle( procP );
		}
		while( proc->rbOut.state == PROC_READING ) {
			XplDelay( 50 );
		}
		while( proc->rbErr.state == PROC_READING ) {
			XplDelay( 50 );
		}

		if (proc->hJob) {
			CloseHandle(proc->hJob);
			proc->hJob = NULL;
		}

		if (proc->iocp) {
			CloseHandle(proc->iocp);
			proc->iocp = NULL;
		}
	}
}

/* Release an XPLPROC object and null out the pointer.
 * Blocks if the child has not terminated.
 * Accepts: a pointer to an XPLPROC pointer
 * Returns: void
 */
EXPORT void XplProcFree( XPLPROC **childProc )
{
	XPLPROC *child;

	if( childProc ) {
		child = *childProc;
		if( child ) {
			XplProcJoin( child, NULL );
			procShutdown( child );
			procFree( childProc );
		}
	}
}

EXPORT void XplProcFlush( XPLPROC *childProc )
{
    if( childProc && ( childProc->ihandle )  ) {
		CloseHandle( childProc->ihandle );
        childProc->ihandle = NULL;
    }
}


static int _ProcIOCPThread( XplThread_ thread )
{
//  DWORD length;
  XplBool resultOk;
  WSAOVERLAPPED *ovl;
  ProcEvent *pe;
  DWORD completionCode;
  ULONG_PTR completionKey;
//  int err;
  XPLPROC *proc;
  ProcEventEngine *engine = (ProcEventEngine *)thread->context;

  for(;;){
	ovl = NULL;
	pe = NULL;
	resultOk = GetQueuedCompletionStatus( engine->iocp, &completionCode, (PULONG_PTR)&completionKey, &ovl, INFINITE );
	if( !ovl )
	{
		// should only happen when shutting down engine
		break;
	}
	pe = XplParentOf( ovl, ProcEvent, ovl );
	switch( pe->type ) {
	case PROC_EVENT_TYPE_OUT_READABLE:
		proc = XplParentOf( pe, XPLPROC, peOutReadable );
		XplLockAcquire( &proc->lock );
		proc->outReadable = TRUE;
		XplLockRelease( &proc->lock );
		break;
	case PROC_EVENT_TYPE_ERR_READABLE:
		proc = XplParentOf( pe, XPLPROC, peErrReadable );
		XplLockAcquire( &proc->lock );
		proc->errReadable = TRUE;
		XplLockRelease( &proc->lock );
		break;
	case PROC_EVENT_TYPE_IN_WRITABLE:
		proc = XplParentOf( pe, XPLPROC, peInWritable );
		XplLockAcquire( &proc->lock );
		proc->inWritable = TRUE;
		XplLockRelease( &proc->lock );
		break;
	case PROC_EVENT_TYPE_JOB_DONE:
		proc = XplParentOf( pe, XPLPROC, peJobDone );
		XplLockAcquire( &proc->lock );
		proc->jobDone = TRUE;
		XplLockRelease( &proc->lock );
		break;
	}
  }

  if( engine ) {
	  if( engine->iocp ) {
		  CloseHandle( engine->iocp );
	  }
	  MemFree( engine );
  }
  return 0;
}

ProcEventEngine *ProcEngineAlloc( void )
{
	ProcEventEngine *engine;

	engine = MemMalloc( sizeof( ProcEventEngine ) );
	if( engine ) {
		engine->iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
		if( engine->iocp ) {
			if( !XplThreadStart( NULL, _ProcIOCPThread, engine, NULL ) ) {
				return engine;
			}
			CloseHandle( engine->iocp );
		}
		MemFree( engine );
	}
	return NULL;
}

void ProcEngineFree( ProcEventEngine *engine )
{
	if( engine && engine->iocp ) {
		PostQueuedCompletionStatus( engine->iocp, (DWORD)0, (ULONG_PTR)NULL, (LPOVERLAPPED)NULL );
	}
}
