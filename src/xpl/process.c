#include <signal.h>
#include "processp.h"

#define BUFLEN 1024


#if !defined(WINDOWS)
extern const char *__progname;
#else
static char BinaryName[30] = "";
#endif

static XPLPROCESS *processes = NULL;
static XplLock *processlock = NULL;


/* Add agent to process list
 * Accepts: agent name
 *		absolute path of working directory
 *		argument vector (argv[0] must have absolute path of agent)
 *		initial flags
 * Returns: process if success, NULL if failure
 *
 * On failure, errno is 0 if the process already exists, or the syscall reason for failure
 */

static XPLPROCESS *addProcess(char *agent, char *path, char *const argv[], uint32 flags)
{
	XPLPROCESS *p;
	XPLPROCESS *process = NULL;
	XplLockAcquire(processlock);
	for(p = processes; p && XplStrCaseCmp(agent, XplProcessName(p)); p = p->next);
	if(p) {						/* fail if existing process */
		errno = 0;				/* set for consumer */
	}
								/* fail if can't create */
	else if(process = XplCreateProcess(agent, path, argv, flags)) {
								/* add new process to list */
		process->next = processes;
		processes = process;
	}
	XplLockRelease(processlock);
	return process;
}


/* Pull process from list
 * Accepts: process
 */

static void removeProcess(XPLPROCESS *process)
{
	XPLPROCESS **procp;
	XplLockAcquire(processlock);
	for(procp = &processes; *procp && (process != *procp); procp = &(*procp)->next);
	if(*procp) {			/* remove from list if found */
		*procp = process->next;
	}
	else {
		DebugAssert(0);		/* process not found in list! */
	}
	XplLockRelease(processlock);
}


/* Find agent
 * Accepts: agent name
 * Returns: process if non-NULL with processes locked, else NULL (and unlocked) if failed
 */

static XPLPROCESS *findAgent(char *agent)
{
	XPLPROCESS *process;
	for(process = XplListAgents(); process && XplStrCaseCmp(agent, XplProcessName(process)); process = XplListNextAgent(process));
	return process;
}




/* Initialize process list lock
 *
 * Only necessary if you intend to use XplProcess() and XplSignalAgent()
 */

void XplProcessInit(void)
{
	static XplLock plock;
	DebugAssert(!processlock);
	processlock = &plock;
	XplLockInit(processlock);
}


/* Invoke process
 * Accepts: agent name
 *		absolute path of working directory
 *		argument vector (argv[0] must have absolute path of agent)
 *		initial flags
 *		data passed to callbacks
 *		second data passed to callbacks
 *		output callback routine
 *		exit callback routine
 *		signal callback routine
 * Returns: TRUE if successful process run (and exit), FALSE otherwise
 */
XplBool XplProcess(char *agent, char *path, char *const argv[], void *data, void *second, uint32 flags, agentOutputCB ocb, agentExitCB ecb, agentSignalCB scb)
{
	XPLPROCESS *process;
	XplBool ret = FALSE;
	if(!processlock) {
		DebugAssert(0);			/* must call XplProcessInit at startup */
	}
	else if(process = addProcess(agent, path, argv, flags)) {
		long i, delay;
		int state, sig;
		uint64 pid;
		XplBool coredump;
		time_t start = XplProcessStartTime(process);
								/* copy agent name */
		agent = MemStrdupWait(XplProcessName(process));
		ret = TRUE;				/* a process was created */
								/* enter process event loop */
		XplReadProcess(process, ocb, data);
		removeProcess(process);	/* pipe error, pull process from list */
		pid = (uint64) XplProcessID(process);
		flags = XplGetProcessFlags(process);
		// The ipipe to the process is closed.  Normally this means that
		// the process is exiting.  Now we need to wait for the process
		// to be reaped and we have its exit status.  We expect this to
		// happen reasonably soon, so we are aggressive in killing.
		//
		// If the process still doesn't go away after being killed, it is
		// probably stuck.  In that case, this thread will sit, slow polling,
		// until the process eventually exits and it gets reaped.  It would
		// become a zombie otherwise.

#define FAST 100				/* delay intervals in ms */
#define MEDIUM 500
#define SLOW 1000
#define CRAWL 5000

#define STOPTIME (5 * 1000)		/* action times in ms */
#define KILLTIME (10 * 1000)
#define CRAWLTIME (15 * 1000)
								/* poll, starting with 100ms intervals */
		for(i = 0, delay = FAST; !XplPollProcess(&process, &state, &sig, &coredump); i += delay) {
			XplDelay(delay);	/* give it some time to die */
			switch(delay) {
			case FAST:			/* fast polling, if reached stop time */
				if(i >= STOPTIME) {
								/* slow to medium and send stop */
					delay = MEDIUM;
					XplSignalProcess(process, SIGNAL_STOP);
					i = STOPTIME;
				}
				break;
			case MEDIUM:		/* medium polling, if reached kill time */
				if(i >= KILLTIME) {
								/* slow to slow and send kill */
					delay = SLOW;
					XplSignalProcess(process, SIGNAL_KILL);
					i = KILLTIME;
				}
				break;
			case SLOW:			/* slow polling, if reached crawl time */
				if(i >= CRAWLTIME) {
								/* slow to a crawl */
					delay = CRAWL;
				}
				break;
			}
		}
		if(sig) {				/* terminated due to signal? */
			if(scb) {
				switch(sig) {	/* translate signals to XplSignalType */
				case SIGTERM:
					sig = SIGNAL_STOP;
					break;
				case SIGINT:
					sig = SIGNAL_HALT;
					break;
#ifdef SIGKILL
				case SIGKILL:
					sig = SIGNAL_KILL;
					break;
#endif
#ifdef SIGQUIT
				case SIGQUIT:
#endif
				case SIGABRT:
					sig = SIGNAL_ABORT;
					break;
				default:		/* other signals returned as negative */
					sig = -sig;
				}
				(scb) (agent, pid, data, second, sig, start, flags, coredump);
			}
		}
		else if(ecb) {			/* must have exited */
			(ecb) (agent, data, second, state, start, flags);
		}
		MemRelease(&agent);
	}
	return ret;
}


/* Get agent flags
 * Accepts: agent name
 * Returns: flags or 0xffffffff if agent not found
 */

uint32 XplGetAgentFlags(char *agent)
{
	XPLPROCESS *process;
	uint32 ret = 0xffffffff;
	if(process = findAgent(agent)) {
		ret = XplGetProcessFlags(process);
		XplLockRelease(processlock);
	}
	return ret;
}


/* Set agent flags
 * Accepts: agent name
 *		mask to set
 * Returns: TRUE on success, FALSE if agent not found
 */

XplBool XplSetAgentFlags(char *agent, uint32 flags)
{
	XPLPROCESS *process;
	XplBool ret = FALSE;
	if(process = findAgent(agent)) {
		XplSetProcessFlags(process, flags);
		XplLockRelease(processlock);
	}
	return ret;
}


/* Clear agent flags
 * Accepts: agent name
 *		mask to clear
 * Returns: TRUE on success, FALSE if agent not found
 */

XplBool XplClearAgentFlags(char *agent, uint32 flags)
{
	XPLPROCESS *process;
	XplBool ret = FALSE;
	if(process = findAgent(agent)) {
		XplClearProcessFlags(process, flags);
		XplLockRelease(processlock);
	}
	return ret;
}


/* Set all agent flags
 * Accepts: agent name
 *		mask to set
 * Returns: TRUE on success, FALSE if agent not found
 */

void XplSetAllAgentFlags(uint32 flags)
{
	XPLPROCESS *process;
	for(process = XplListAgents(); process; process = XplListNextAgent(process)) {
		XplSetProcessFlags(process, flags);
	}
}


/* Clear all agent flags
 * Accepts: agent name
 *		mask to clear
 * Returns: TRUE on success, FALSE if agent not found
 */

void XplClearAllAgentFlags(uint32 flags)
{
	XPLPROCESS *process;
	for(process = XplListAgents(); process; process = XplListNextAgent(process)) {
		XplClearProcessFlags(process, flags);
	}
}


/* Send signal to agent launched by XplProcess()
 * Accepts: agent name
 *		signal type
 *		flags to be set in process
 * Returns: 0 if success, else error code
 */

int XplSignalAgent(char *agent, XplSignalType type, uint32 flags)
{
	XPLPROCESS *process;
	XplBool ret = ENOENT;
	if(process = findAgent(agent)) {
		XplSetProcessFlags(process, flags);
		ret = XplSignalProcess(process, type);
		XplLockRelease(processlock);
	}
	return ret;
}


/* Send signal to all agents launched by XplProcess()
 * Accepts: signal type
 *		flags to be set in process
 */

void XplSignalAllAgents(XplSignalType type, uint32 flags)
{
	XPLPROCESS *process;
	for(process = XplListAgents(); process; process = XplListNextAgent(process)) {
		XplSetProcessFlags(process, flags);
		XplSignalProcess(process, type);
	}
}


/* Wait for agent to terminate
 * Accepts: agent name
 *		timeout before killing (zero means wait forever)
 * Returns: TRUE if agent terminated, FALSE if agent had to be killed
 */

XplBool XplWaitAgent(char *agent, int timeout)
{
	XplBool ret = TRUE;
	XPLPROCESS *process;
	do {						/* find agent */
		if(process = findAgent(agent)) {
								/* forcibly kill after timeout */
			if(timeout && !--timeout) {
				ret = FALSE;	/* note that a kill was needed */
				XplSignalProcess(process, SIGNAL_KILL);
			}
			XplLockRelease(processlock);
			XplDelay(1000);
		}
	} while(process);
	return ret;
}


/* List all agents
 * Returns: first agent, process lock is locked
 */

XPLPROCESS *XplListAgents(void)
{
	XPLPROCESS *process;
	XplLockAcquire(processlock); /* seize the process lock */
	if(!(process = processes)) {
		XplLockRelease(processlock);
	}
	return process;
}


/* List next agent
 * Accepts: current agent
 * Returns: next agent, or NULL with process lock released when done
 */

XPLPROCESS *XplListNextAgent(XPLPROCESS *process)
{
	if(!(process = process->next)) {
		XplLockRelease(processlock);
	}
	return process;
}


/* See if agent is running
 * Accepts: agent name
 * Returns: TRUE if agent is running, else FALSE
 */

XplBool XplAgentExists(char *agent)
{
	XplBool ret = FALSE;
	if(findAgent(agent)) {
		XplLockRelease(processlock);
		ret = TRUE;
	}
	return ret;
}


/* Process interface - OS-independent routines */

/* Return name of process
 * Accepts: process
 * Returns: name
 */

char *XplProcessName(XPLPROCESS *process)
{
	return process->agent;
}


/* Get process flags
 * Accepts: process
 * Returns: flags
 */

uint32 XplGetProcessFlags(XPLPROCESS *process)
{
	return process->flags;
}


/* Set process flags
 * Accepts: process
 *		mask to set
 */

void XplSetProcessFlags(XPLPROCESS *process, uint32 flags)
{
	process->flags |= flags;
}

/* Clear process flags
 * Accepts: process
 *		mask to clean
 */

void XplClearProcessFlags(XPLPROCESS *process, uint32 flags)
{
	process->flags &= ~flags;
}


/* Return start time of process
 * Accepts: process
 * Returns: start time
 */

time_t XplProcessStartTime(XPLPROCESS *process)
{
	return process->start;
}


/* Process interface - OS-dependent routines */

#if defined(WINDOWS)

EXPORT char * XplGetBinaryName( void ) {
    char path[ 1000 ];
	char *file;

	if( '\0' == *BinaryName ) {
	  GetModuleFileName( NULL, path, sizeof( path ) );
	  file = strrchr( path, '\\' );
	  if( file ) {
		file++;
	  } else {
		file = path;
	  }
	  strncpy( BinaryName, file, sizeof( BinaryName ) );
	  BinaryName[ sizeof( BinaryName ) - 1 ] = '\0';
	}

	return BinaryName;
}

static XplBool isAbsolute(const char *path)
{
	if (!path || !*path) {
		return(FALSE);
	}

	if ('\\' == path[0] && isalnum(path[1])) {
		return(TRUE);
	}

	if (isalpha(path[0]) && ':' == path[1] && ('\\' == path[2] || '/' == path[2] ) && path[3]) {
		/* Allow a path starting with a drive letter */
		return(TRUE);
	}

	return(FALSE);
}

static char * BuildCommandLine(char *const argv[])
{
	size_t		len;
	int			i;
	char		*cmd;

	for (len = 0, i = 0; argv[i]; i++) {
		len += strlen(argv[i]) + 3;
	}

	len++;
	if ((cmd = MemMallocWait(len))) {
		*cmd = '\0';

		for (i = 0; argv[i]; i++) {
// TODO	Escape quotes in arguments
DebugAssert(!strchr(argv[i], '"'));
			strcatf(cmd, len, NULL, "\"%s\" ", argv[i]);
		}

		chopspace(cmd);
printf("Built command line: %s\n", cmd);
	}

	return(cmd);
}

/* Create process
 * Accepts: agent name
 *		absolute path of working directory
 *		argument vector (argv[0] must have absolute path of agent)
 *		flags
 * Returns: XPLPROCESS if success, NULL if failure with errno set
 */

XPLPROCESS *XplCreateProcess(char *agent, char *path, char *const argv[], uint32 flags)
{
	XPLPROCESS *ret = NULL;
	SECURITY_ATTRIBUTES sa;
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	HANDLE ihdl = NULL;
	HANDLE ohdl = NULL;
	HANDLE ipipe = NULL;
	HANDLE opipe = NULL;
	char *cmdline;

								/* demand absolute paths and agent in argv */
	if (!isAbsolute(argv[0]) || !isAbsolute(path) || !agent) {
		errno = EACCES;
	} else {
		sa.nLength = sizeof (sa);	/* use default ACL */
		sa.lpSecurityDescriptor = NULL;
		sa.bInheritHandle = TRUE;	/* inherit handles */

		if (!CreatePipe (&ihdl, &ipipe, &sa, 0)) {
			errno = EMFILE;
		}
		else if (!CreatePipe (&opipe, &ohdl, &sa, 0)) {
			errno = EMFILE;
			CloseHandle (ihdl);
		}
		else {
			si.cb = sizeof (STARTUPINFO);
			si.lpReserved = NULL;
			si.lpTitle = NULL;
			si.lpDesktop = NULL;
			si.dwX = si.dwY = si.dwXSize = si.dwYSize = 0;
			si.wShowWindow = SW_SHOW;
			si.lpReserved2 = NULL;
			si.cbReserved2 = 0;
			si.dwFlags = STARTF_USESTDHANDLES;
			si.hStdInput  = opipe;
			si.hStdOutput = ipipe;

								/* Build the command line string */
			cmdline = BuildCommandLine(argv);

								/* errors go to same place as output */
			DuplicateHandle (GetCurrentProcess (), ipipe, GetCurrentProcess (), &si.hStdError, DUPLICATE_SAME_ACCESS, TRUE, 0);
			if (!CreateProcess (argv[0], cmdline, NULL, NULL, TRUE, CREATE_NEW_PROCESS_GROUP, NULL, NULL, &si, &pi)) {
				errno = ENOMEM;
			} else {
				ret = (XPLPROCESS *) memset(MemMallocWait(sizeof(XPLPROCESS)), 0, sizeof(XPLPROCESS));

				ret->flags = flags;
				ret->start = time(NULL);
				ret->process = pi.hProcess;
				ret->thread = pi.hThread;
				ret->pid = pi.dwProcessId;
				ret->tid = pi.dwThreadId;
				ret->ihandle = ihdl;
				ret->ohandle = ohdl;
				ret->agent = MemStrdupWait(agent);

				ihdl = ohdl = NULL;	/* handles have been given to process */
				sa.bInheritHandle = 0; /* no inheritance */
			}

			MemRelease(&cmdline);
		}
								/* clean up */
		if (ipipe) CloseHandle (ipipe);
		if (opipe) CloseHandle (opipe);
		if (ihdl) CloseHandle (ihdl);
		if (ohdl) CloseHandle (ohdl);
	}
	return ret;
}


/* Return OS-dependent process ID
 * Accepts: process
 * Returns: process ID, always
 */

int XplProcessID(XPLPROCESS *process)
{
	return process->pid;
}


/* Read from process and send to output callback
 * Accepts: process
 *		process output callback
 *		data passed to output callback
 */
void XplReadProcess(XPLPROCESS *process, agentOutputCB ocb, void *data)
{
	DWORD	left, avail, code, read;
	DWORD	used = 0;
	char	*s, *e, w;
	char	buffer[1024];

	left = 0;

	/*
		Read until there is no data and the process has exited.  There will not
		be an EOF, so doing a read after all data has been read and the child is
		gone will block forever.

		To avoid this verify that there is data before reading.  If there is no
		data, and the process is no longer active then exit.
	*/
	for (;;) {
		for (;;) {
			/* Is there any data waiting? */
			code = STILL_ACTIVE;
			avail = 0;

			PeekNamedPipe(process->ihandle, NULL, 0, NULL, &avail, NULL);
			if (avail) {
				left += avail;
			}

			if (left) {
				break;
			}

			/* Is the process still running? */
			if (GetExitCodeProcess(process->process, &code) &&
				code != STILL_ACTIVE
			) {
				break;
			}

			XplDelay(30);
		}

		read = 0;
		if (left) {
			if (ReadFile(process->ihandle, buffer + used, sizeof(buffer) - 1 - used, &read, NULL)) {
				left -= read;
				buffer[used + read] = '\0';

				/* The callback expects lines */
				s = buffer;
				while (*s && (e = strchr(s, '\n'))) {
					w = e[1];
					e[1] = '\0';
					if (ocb) {
						(ocb) (process, data, s);
					}
					e[1] = w;
					s = e + 1;
				}

				if (*s) {
					/* There is a partial line remaining */
					used = strlen(s);
					memmove(buffer, s, used + 1);
				}
			}
		} else if (code != STILL_ACTIVE) {
			if (used && ocb) {
				buffer[used] = '\0';
				(ocb) (process, data, buffer);

				used = 0;
			}
			break;
		}
	}
}


/* Send signal to process
 * Accepts: process
 *		signal type
 * Returns: 0 if success, else error code
 */
#ifdef SIGNAL_WINDOWS

static BOOL CALLBACK _XplSignalWindow(HWND window, LPARAM arg)
{
	int		signal;

	switch ((XplSignalType) arg) {
		case SIGNAL_STOP:	signal = SIGTERM;	break;
		case SIGNAL_HALT:	signal = SIGINT;	break;
		case SIGNAL_KILL:	signal = SIGKILL;	break;
		case SIGNAL_ABORT:	signal = SIGQUIT;	break;

		default:
			DebugAssert(0);
			return(FALSE);
	}

	/*
		The message handler that is installed by the XPL library will subtract
		WM_USER from the message, and pass that to the app's signal handler.
	*/
	return(SendMessage(window, WM_USER + signal, 0, 0));
}
#endif

int XplSignalProcess(XPLPROCESS *process, XplSignalType type)
{
	BOOL	ret = FALSE;

	process->start = 0;
	switch (type) {
		case SIGNAL_STOP: /* SIGTERM */
			ret = GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT,	process->pid);
			break;

		case SIGNAL_HALT: /* SIGINT */
			ret = GenerateConsoleCtrlEvent(CTRL_C_EVENT,		process->pid);
			break;

		case SIGNAL_ABORT: /* SIGQUIT */
			ret = GenerateConsoleCtrlEvent(CTRL_LOGOFF_EVENT,	process->pid);

		case SIGNAL_KILL:
			ret = TerminateProcess(process->process, 0);
			break;
	}

	if (ret) {
		return(0);
	} else {
		return(GetLastError());
	}
}


/* Create detached process
 * Accepts: argument vector (argv[0] must have absolute path of agent)
 *		absolute path of working directory
 * Returns: TRUE if success, FALSE otherwise
 */

XplBool XplCreateDetachedProcess(char *const argv[], char *path)
{
	XplBool ret = FALSE;
	DebugAssert(0);		//TODO
	return ret;
}


/* Poll process and clear if exited
 * Accepts: pointer to process
 *		pointer to returned exit status
 *		pointer to returned signal
 * Returns: TRUE if process terminated, else FALSE
 */

XplBool XplPollProcess(XPLPROCESS **process, int *state, int *signal, XplBool *coredump)
{
	XplBool ret = FALSE;
	DWORD code;

	*state = *signal = 0;		/* initialize return values */
	*coredump = FALSE;

	code = 0;
	if (GetExitCodeProcess((*process)->process, &code) && code != STILL_ACTIVE) {
		ret = TRUE;
		*state = code;
	}

	if(ret) {					/* process went away? */
		if((*process)->agent) {
			MemRelease(&(*process)->agent);
		}

		if ((*process)->ihandle) {
			CloseHandle((*process)->ihandle);
			(*process)->ihandle = NULL;
		}

		if ((*process)->ohandle) {
			CloseHandle((*process)->ohandle);
			(*process)->ohandle = NULL;
		}

		if ((*process)->thread) {
			CloseHandle((*process)->thread);
			(*process)->thread = NULL;
		}

		if ((*process)->process) {
			CloseHandle((*process)->process);
			(*process)->process = NULL;
		}

		MemRelease(process);
	}
	return ret;
}
#else	//not Windows

EXPORT char * XplGetBinaryName( void ) {

	return ( char *)__progname;
}


/* Create process
 * Accepts: agent name
 *		absolute path of working directory
 *		argument vector (argv[0] must have absolute path of agent)
 *		flags
 * Returns: XPLPROCESS if success, NULL if failure with errno set
 */

XPLPROCESS *XplCreateProcess(char *agent, char *path, char *const argv[], uint32 flags)
{
	XPLPROCESS *ret = NULL;
	pid_t pid;
	struct stat sb;
	int pi[2], po[2];
								/* demand absolute paths in argv */
	if((*argv[0] != '/') || (*path != '/') || !agent) {
		errno = EACCES;
	}
	else if(stat(argv[0], &sb));
	else if(!S_ISREG(sb.st_mode)) {
		errno = EACCES;
	}
	else if(!pipe(pi)) {
		if(pipe(po)) {			/* create output pipe */
			close(pi[0]);		/* output pipes failed, close input pipes */
			close(pi[1]);
		}
		else {					/* create process */
			switch(pid = fork()) {
			case -1:			/* error */
				close(pi[0]);
				close(pi[1]);
				close(po[0]);
				close(po[1]);
				break;
			case 0:				/* child */
				alarm(0);		/* abolish any parent alarms */
				if(!chdir(path)) {
					struct rlimit rlim;
					int fd, i;	/* don't alter parent's space */
					dup2(po[0], 0);	/* parent's pipe output is my stdin */
					dup2(pi[1], 1);	/* parent's pipe input is my stdout */
					dup2(pi[1], 2);	/* also stderr */
								/* close fds until likely end of in-use */
					for(fd = 3, i = 0; i < 40; ++fd) {
						if(close(fd)) {
							++i;	/* count another close of a non-ex fd */
						}
						else {
							i = 0;	/* closed an fd, reset count */
						}
					}
								/* enable core dumps */
					if(!getrlimit(RLIMIT_CORE, &rlim)) {
						rlim.rlim_cur = rlim.rlim_max;
						setrlimit(RLIMIT_CORE, &rlim);
					}
								/* invoke the agent */
					_exit(execv(argv[0], argv));
				}
				_exit(EXIT_NOCHDIR); /* chdir() failed */
				break;
			default:			/* parent */
				ret = (XPLPROCESS *) memset(MemMallocWait(sizeof(XPLPROCESS)), 0, sizeof(XPLPROCESS));
				ret->flags = flags;
				ret->start = time(NULL);
				ret->pid = pid;
				ret->ipipe = pi[0];
				ret->opipe = po[1];
				ret->agent = MemStrdupWait(agent);
				close(pi[1]);	/* close child's side of the pipes */
				close(po[0]);
				break;
			}
		}
	}
	return ret;
}


/* Return OS-dependent process ID
 * Accepts: process
 * Returns: process ID, always
 */

pid_t XplProcessID(XPLPROCESS *process)
{
	return process->pid;
}


/* Read from process and send to output callback
 * Accepts: process
 *		process output callback
 *		data passed to output callback
 */

void XplReadProcess(XPLPROCESS *process, agentOutputCB ocb, void *data)
{
	FILE *output;
	char buffer[1024];
	if(output = fdopen(process->ipipe, "r")) {
		process->ipipe = -1;	/* note pipe has been stolen */
								/* read from pipe until error */
		while(fgets(buffer, sizeof(buffer), output)) {
			if(ocb) {
				(ocb) (process, data, buffer);
			}
		}
		fclose(output);
	}
}


/* Send signal to process
 * Accepts: process
 *		signal type
 * Returns: 0 if success, else error code
 */

int XplSignalProcess(XPLPROCESS *process, XplSignalType type)
{
	int ret = EINVAL;
	process->start = 0;			/* prevent process restart */
	switch(type) {
	case SIGNAL_STOP:			/* request process to exit gracefully */
		ret = kill(process->pid, SIGTERM) ? errno : 0;
		break;
	case SIGNAL_HALT:			/* request process to exit gracefully */
		ret = kill(process->pid, SIGINT) ? errno : 0;
		break;
	case SIGNAL_KILL:			/* request process to die immediately */
		ret = kill(process->pid, SIGKILL) ? errno : 0;
		break;
	case SIGNAL_ABORT:			/* request process to core dump and die */
		ret = kill(process->pid, SIGQUIT) ? errno : 0;
		break;
	default:
		DebugAssert(0);			/* unknown XplSignalType */
		break;
	}
	return ret;
}


/* Create detached process
 * Accepts: argument vector (argv[0] must have absolute path of agent)
 *		absolute path of working directory
 * Returns: TRUE if success, FALSE otherwise
 */

XplBool XplCreateDetachedProcess(char *const argv[], char *path)
{
	XplBool ret = TRUE;
	int status;
	pid_t pid;
								/* demand absolute paths in argv */
	if((*argv[0] == '/') && (*path == '/')) {
		switch(pid = fork()) {
		case -1:				/* error */
			break;
		case 0:					/* parent */
			while(waitpid(pid, &status, 0) < 0) {
				switch (errno) {
				case EINTR:		/* interrupt, just try again  */
					break;
				default:
					DebugAssert(0);	/* unexpected waitpid failure */
					return FALSE;
				}
			}
								/* success if child terminated OK */
			if(WIFEXITED(status) && !WEXITSTATUS(status)) {
				ret = TRUE;
			}
			break;
		default:				/* child */
			alarm(0);			/* abolish any parent alarms */
			if(!chdir(path)) {
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
								/* invoke the agent in grandchild */
				_exit(execv(argv[0], argv));
			}
			_exit(EXIT_NOCHDIR); /* chdir() failed */
			break;
		}
	}
	return ret;
}


/* Poll process and clear if exited
 * Accepts: pointer to process
 *		pointer to returned exit status
 *		pointer to returned signal
 * Returns: TRUE if process terminated, else FALSE
 */

XplBool XplPollProcess(XPLPROCESS **process, int *state, int *sig, XplBool *coredump)
{
	pid_t pid;
	int status = 0;
	XplBool ret = FALSE;
	*state = *sig = 0;		/* initialize return values */
	*coredump = FALSE;
								/* reap child, ignoring signals */
	while(((pid = waitpid((*process)->pid, &status, WNOHANG)) < 0) && (errno != EINTR));
	if(pid) {					/* unless process still pending */
		if(pid > 0) {			/* reaped a process? */
			DebugAssert(pid == (*process)->pid);
								/* process exited? */
			if(WIFEXITED(status)) {
				*state = WEXITSTATUS(status);
			}
								/* died with signal? */
			else if(WIFSIGNALED(status)) {
				*sig = WTERMSIG(status);
				if(WCOREDUMP(status)) {
					*coredump = TRUE;
				}
			}
			else {
				DebugAssert(0);	/* terminated but not with exit or signal? */
			}
		}
								/* shouldn't happen, but treat ECHILD as normal exit */
		else if(errno != ECHILD) {
			DebugAssert(0);		/* waitpid() failed, note errno */
		}
		if((*process)->ipipe >= 0) {
			close((*process)->ipipe);
		}
		if((*process)->opipe >= 0) {
			close((*process)->opipe);
		}
		if((*process)->agent) {
			MemRelease(&(*process)->agent);
		}
		MemRelease(process);
		ret = TRUE;				/* success */
	}
	return ret;
}
#endif
