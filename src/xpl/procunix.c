#include "procp.h"

extern char **environ;

XplBool MTInitialized = FALSE;
XplSemaphore MTSem;

EXPORT void XplProcThreadSafeInit( void )
{
	if( !MTInitialized ) {
		XplSemaInit( MTSem, 1 );
		MTInitialized = TRUE;
	}
}

/* Add our base paths to the environment for the child process */
static char ** XplProcBuildEnv(char **env)
{
	int		count, i;
	char	**e;
	char	*base	= "NETMAIL_BASE_DIR="		XPL_BASE_DIR;
	char	*data	= "NETMAIL_DATA_DIR="		XPL_DATA_DIR;
	char	*var	= "NETMAIL_DATAVAR_DIR="	XPL_DATA_VAR_DIR;
	char	tmp[XPL_MAX_PATH];

	for (count = 0; env[count]; count++) {
		;
	}

	if (!(e = calloc(count + 4, sizeof(char *)))) {
		return(env);
	}

	for (i = 0; env[i]; i++) {
		e[i] = env[i];
	}

	if (!XplExpandEnv(tmp, base, sizeof(tmp))) {
		base = strdup(tmp);
	}
	if (!XplExpandEnv(tmp, data, sizeof(tmp))) {
		data = strdup(tmp);
	}
	if (!XplExpandEnv(tmp, var, sizeof(tmp))) {
		var = strdup(tmp);
	}

	e[i++] = base;
	e[i++] = data;
	e[i++] = var;
	e[i++] = NULL;

	return(e);
}

static void closePipe(int fildes[2])
{
	if (fildes) {
		if (-1 != fildes[0]) {
			close(fildes[0]);
			fildes[0] = -1;
		}

		if (-1 != fildes[1]) {
			close(fildes[1]);
			fildes[1] = -1;
		}
	}
}

/* Create a child process
 * Accepts:
 *		- path of working directory
 *          - absolute or relative paths are acceptable
 *          - NULL will cause the child process to have the same working
 *            directory as the parent process.
 *		- argument vector
 *          - args[0] is the path to the executible
 *          - args[0] can be an absolute path or relevant to the current
 *            working directory set by the previous argument.
 *          - no attempt will be made to search for the executable in the
 *            directories specified by the shell's PATH environment variable.
 * Returns: XPLPROC if success, NULL if failure with errno set
 */
EXPORT XPLPROC *XplProcExec(char *workdir, const char **args )
{
	XPLPROC			*ret		= NULL;
	XplBool			usingSem;
	pid_t			pid;
	struct stat		sb;
	int				pi[2]		= { -1, -1 };
	int				po[2]		= { -1, -1 };
	int				pe[2]		= { -1, -1 };
	char			*ptr;

	if (stat(args[0], &sb)) {
		;
	} else if(!S_ISREG(sb.st_mode)) {
		errno = EACCES;
	} else if (pipe(pi) || pipe(po) || pipe(pe)) {		/* Create pipes */
		closePipe(pi);
		closePipe(po);
		closePipe(pe);
	} else {
		/* create process */
		if( MTInitialized ) {
			XplSemaWait( MTSem );
			usingSem = TRUE;
		} else {
			usingSem = FALSE;
		}

		switch(pid = fork()) {
			case -1:
				/* error */
				closePipe(pi);
				closePipe(po);
				closePipe(pe);

				if (usingSem) {
					XplSemaPost( MTSem );
				}
				break;

			case 0:
				/* child */

				/* abolish any parent alarms */
				alarm(0);

				/* Close half of the pipes that we don't need */
				close(pi[1]);
				close(po[0]);
				close(pe[0]);

				/* Close standard input, error and output */
				close(0);
				close(1);
				close(2);

				/* Link standard input, error and output to our pipes */
				dup2(pi[0], 0);
				dup2(po[1], 1);
				dup2(pe[1], 2);

				if (!workdir || !chdir(workdir)) {
					struct rlimit rlim;
					int fd, i;			/* don't alter parent's space */
										/* close fds until likely end of in-use */
					for (fd = 3, i = 0; i < 40; ++fd) {
						if(close(fd)) {
							++i;		/* count another close of a non-ex fd */
						} else {
							i = 0;		/* closed an fd, reset count */
						}
					}

					/* enable core dumps */
					if (!getrlimit(RLIMIT_CORE, &rlim)) {
						rlim.rlim_cur = rlim.rlim_max;
						setrlimit(RLIMIT_CORE, &rlim);
					}

					/* Set our environment variables */
					environ = XplProcBuildEnv(environ);

					/* invoke the executable */
					_exit( execv( args[0], (char **) args ) );
				} else {
					/* chdir() failed */
					_exit(EXIT_NOCHDIR);
				}
				break;

			default:
				/* parent */
				if ((ret = calloc(1, sizeof(XPLPROC)))) {
					ret->pid		= pid;

					/* Save the pipes so we can write to or read from the child */
					ret->ipipe		= pi[1];
					ret->opipe		= po[0];
					ret->epipe		= pe[0];

					/*
						Keep a reference to the other side of the pipes as well
						to prevent an error when trying to read if the child has
						already exited. These will be closed when the proc is
						free'd.
					*/
					ret->ipipeChild	= pi[0];
					ret->opipeChild	= po[1];
					ret->epipeChild	= pe[1];

					ptr = strrchr( args[0], '/' );
					strprintf(ret->name,sizeof(ret->name),NULL ,"%s", ptr ? ptr + 1: args[0]);

					if (usingSem) {
						XplSemaPost( MTSem );
					}
				}
				break;
		}
	}
	return ret;
}

/* Create detached process
 * Accepts:
 *		- path of working directory
 *          - absolute or relative paths are acceptable
 *          - NULL will cause the child process to have the same working
 *            directory as the parent process.
 *		- argument vector
 *          - args[0] is the path to the executible
 *          - args[0] can be an absolute path or relevant to the current
 *            working directory set by the previous argument.
 *          - no attempt will be made to search for the executable in the
 *            directories specified by the shell's PATH environment variable.
 * Returns: 0 on success, non-zero otherwise
 */

EXPORT int XplProcExecDetached(char *workdir, const char **args)
{
	int ret = 0;
	int wpret;
	int status;
	pid_t pid;
	XplBool usingSem;

	if( MTInitialized ) {
		XplSemaWait( MTSem );
		usingSem = TRUE;
	} else {
		usingSem = FALSE;
	}
	switch(pid = fork()) {
	case -1:				/* error */
		if( usingSem ) XplSemaPost( MTSem );
		break;
	case 0:					/* child */
		alarm(0);			/* abolish any parent alarms */
		if(!workdir || !chdir(workdir)) {
			int fd, i;		/* don't alter parent's space */
			/* close fds until likely end of in-use */
			for(fd = 3, i = 0; i < 40; ++fd) {
				if(close(fd)) {
					++i;	/* count another close of a non-ex fd */
				}
				else {
					i = 0;	/* closed an fd, reset count */
				}
			}
			if(fork()) {	/* make grandchild so it's inherited by init */
				_exit(EXIT_SUCCESS); /* child is now done */
			}
			setpgid(0, getpid()); /* put grandchild in its own process group */

			/* Set our environment variables */
			environ = XplProcBuildEnv(environ);

			/* invoke the agent in grandchild */
			_exit(execv(args[0], ( char **)args));
		}
		_exit(EXIT_NOCHDIR); /* chdir() failed */
		break;
	default:				/* original parent */
		if( usingSem ) XplSemaPost( MTSem );
		for(;;) {
			wpret = waitpid(pid, &status, 0);
			if( wpret > -1 ) {
				/* success if child terminated OK */
				if(WIFEXITED(status) && !WEXITSTATUS(status)) {
					ret = 0;
					break;
				}
				DebugAssert( 0 ); // unexpected status
				ret = -1;
				break;
			}

			if( errno != EINTR ) {
				ret = -1;
				break;
			}
			/* we were interrupted, keep going */
		}

		break;
	}
	return ret;
}

/* Sends a signal to a child process
 * Accepts:
 *		- XPLPROC pointer
 *		- the signal to be sent
 * Returns: 0 on success, non-zero otherwise
 */

EXPORT int XplProcSignal( XPLPROC *childProc, ProcSignal signal )
{
	int ret = -1;

	if( !childProc || childProc->reaped ) {
		errno = ENOENT;
		return -1;
	}

	switch( signal ) {
	case PROC_SIGNAL_TERM:
		ret = kill( childProc->pid, SIGTERM );
		break;
	case PROC_SIGNAL_INT:
		ret = kill( childProc->pid, SIGINT );
		break;
	case PROC_SIGNAL_KILL:
		ret = kill( childProc->pid, SIGKILL );
		break;
	case PROC_SIGNAL_USR1:
		ret = kill( childProc->pid, SIGUSR1 );
		break;
	case PROC_SIGNAL_USR2:
		ret = kill( childProc->pid, SIGUSR2 );
		break;
	case PROC_SIGNAL_CORE:
		ret = kill( childProc->pid, SIGABRT );
		break;
	}
	return ret;
}

EXPORT int XplProcSignalGroup( XPLPROC *childProc, ProcSignal signal )
{
	// TODO SignalGroup is not yet supported on linux
	return(XplProcSignal(childProc, signal));
}

/* Blocks until the child process terminates
 * Accepts:
 *		- XPLPROC pointer
 *		- optional pointer to an int
 *          - if a pointer is passed, status information about the child is
 *            stored in it.
 *            - when this function's return value is:
 *              0, the child has exited and status contains the child's return
 *                 value or the signal that terminated the child.
 *              1, the function timed out before the child exited status
 *                 will have the value ETIMEDOUT
 *             -1, the function fail and status contains errno
 * Returns: 0 on success, -1 on failure, 1 on timeout
 */

static pid_t WaitForPid( XplProcID pid, int *status, unsigned int timeout )
{
	int t;
	pid_t pidHandle;

	*status = 18;
	if( !timeout ) {
		pidHandle = waitpid( pid, status, 0 );
	} else {
		t = 0;
		while( 0 == ( pidHandle = waitpid( pid, status, WNOHANG ) ) && ( t < timeout ) ) {
			XplDelay( 100 );
			t += 100;
		}
	}

	return pidHandle;
}

EXPORT JoinRetVal XplProcJoinEx( XPLPROC *childProc, AutopsyReport *report, unsigned int timeout )
{
	pid_t pid;
	int wpstatus;
	JoinRetVal retVal;
	CauseOfDeath cause;
	char *description;
	int childRet;
	XplBool core;

	retVal = JOIN_ERROR;
	cause = PROC_UNKNOWN;
	description = "UNKNOWN";
	childRet = 0;
	core = FALSE;

	if( childProc && !childProc->reaped && ( -1 != ( pid = WaitForPid( childProc->pid, &wpstatus, timeout ) ) )) {
		if( !pid ) {
			retVal = JOIN_TIMEOUT;
		} else {
			DebugAssert( pid == childProc->pid ); // Does this ever happen? (rodney)
			childProc->reaped = 1;
			retVal = JOIN_SUCCESS;

			if( WIFEXITED( wpstatus ) ) {
				cause = PROC_EXITED;
				childRet = WEXITSTATUS( wpstatus );
			} else if( WIFSIGNALED( wpstatus ) ) {
				int sig = WTERMSIG( wpstatus );
				switch(sig) {
				case SIGTERM:
					description = "TERMINATED";
					cause = PROC_KILLED;
					break;
				case SIGINT:
					description = "INTERRUPTED";
					cause = PROC_KILLED;
					break;
				case SIGKILL:
					description = "KILLED";
					cause = PROC_KILLED;
					break;
				case SIGQUIT:
					description = "QUIT";
					cause = PROC_CRASHED;
					break;
				case SIGABRT:
					description = "ABORTED";
					cause = PROC_CRASHED;
					break;
				default:
					cause = PROC_CRASHED;
					core = WCOREDUMP( wpstatus );
					break;
				}
			}
		}
	}
	if( report ) {
		report->cause = cause;
		report->returnValue = childRet;
		report->core = core;
		report->description = description;
	}
	return retVal;
}

EXPORT JoinRetVal XplProcJoinGroupEx( XPLPROC *childProc, AutopsyReport *report, unsigned int timeout )
{
	// TODO JoinGroup is not yet supported on linux
	return(XplProcJoinEx(childProc, report, timeout));
}

/* XplProcReadable

   This function never blocks. It return TRUE is there is data to read,
   FALSE if not.

*/
EXPORT XplBool XplProcReadable( XPLPROC *childProc, ProcStreamType type )
{
	struct pollfd pfd;
	int ret;
	XplTimeout internalTimeout;
	int count;

	switch( type ) {

	case PROC_STDOUT:
		pfd.fd = (int)( childProc->opipe );
		break;
	case PROC_STDERR:
		pfd.fd = (int)( childProc->epipe );
		break;
	case PROC_STDIN:
		DebugAssert( 0 );  // can't read from child's stdin
		errno = EINVAL;
		return FALSE;
	}
	pfd.events = POLLIN | POLLPRI;

	XplTimeoutSet( &internalTimeout, 0 );

	count = 0;
	errno = 0;

	for( ; ; ) {
		if( ( ret = poll( &pfd, 1, internalTimeout ) ) > 0 ) {
			if( 0 == ( pfd.revents & ( POLLHUP | POLLNVAL | POLLERR ) ) ) {
				if( pfd.revents & ( POLLIN | POLLPRI ) ) {
					return TRUE;
				}
			}
			/* the other side of the pipe has been closed */
			return FALSE;
		}

		if( 0 == ret ) {
			return FALSE;
		}

		/* there is an error */
		if( errno != EINTR ) {
			return FALSE;
		}
		/* we were interrupted, try again */
	}
}

/*
  XplProcRead reads as much data as it can into the buffer. If there is space
  leftover, the buffer will be terminated.
*/
EXPORT int XplProcRead( XPLPROC *childProc, ProcStreamType type, char *buffer, size_t bufferLen )
{
	int fd;
	int len;

	switch( type ) {
		case PROC_STDOUT:
			fd = childProc->opipe;
			break;
		case PROC_STDERR:
			fd = childProc->epipe;
			break;
		case PROC_STDIN:
			DebugAssert( 0 );  // can't read from child's stdin
			errno = EINVAL;
			return -1;
		default:
			errno = EINVAL;
			return -1;
	}

	if( bufferLen > 0 ) {
		len = read( fd, buffer, bufferLen );
	} else {
		len = 0;
	}
	if( len < 0 ) {
		return len;
	}
	if( len < bufferLen ) {
		buffer[ len ] = '\0';
	}
	return len;
}

EXPORT int XplProcWrite( XPLPROC *childProc, char *data, size_t dataLen )
{
	return write( childProc->ipipe, data, dataLen );
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
			/* close the parent end of the pipe */
			if( child->ipipe > -1 ) {
				close( child->ipipe );
			}
			if( child->opipe > -1 ) {
				close( child->opipe );
			}
			if( child->epipe > -1 ) {
				close( child->epipe );
			}
			/* close the child end as well */
			if( child->ipipeChild > -1 ) {
				close( child->ipipeChild );
			}
			if( child->opipeChild > -1 ) {
				close( child->opipeChild );
			}
			if( child->epipeChild > -1 ) {
				close( child->epipeChild );
			}
			free( child );
			*childProc = NULL;
		}
	}
}

EXPORT void XplProcFlush( XPLPROC *childProc )
{
    if( childProc && ( childProc->ipipe )  ) {
        close( childProc->ipipe );
        childProc->ipipe = -1;
    }
}
