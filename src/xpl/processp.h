#include <memmgr-config.h>
#include <stdio.h>

#if defined(WINDOWS)
# include <stdlib.h>
# include <stdarg.h>
# include <time.h>
# include <stdarg.h>
# include <io.h>
#else
# include <signal.h>
# include <unistd.h>
# include <sys/types.h>
# include <wait.h>
#endif

#include <memmgr.h>

#define XPLPROCESS struct xpl_process

XPLPROCESS {
	char *agent;				/* agent name */
	uint32 flags;				/* flags */
	time_t start;				/* start time */
#if defined(WINDOWS)
	HANDLE process;				/* process handle */
	HANDLE thread;				/* process thread */
	DWORD pid;					/* process id */
	DWORD tid;					/* thread id */
	HANDLE ihandle;				/* input from proces */
	HANDLE ohandle;				/* output from process */
#else
	pid_t pid;					/* process id */
	int ipipe;					/* input pipe */
	int opipe;					/* output pipe */
#endif
	XPLPROCESS *next;
};

#include <xplprocess.h>
