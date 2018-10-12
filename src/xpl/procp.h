#include <memmgr-config.h>
#include <stdio.h>


# include <signal.h>
#if defined(WINDOWS)
# include <stdlib.h>
# include <stdarg.h>
# include <time.h>
# include <stdarg.h>
# include <io.h>
#else
# include <unistd.h>
# include <sys/types.h>
# include <wait.h>
#endif

#include <xpl.h>

#define _XPLPROC_H 1
#if defined(WINDOWS)
#define PROC_READ_BUFF_SIZE 1024
#define PROC_READ_STACK_SIZE 1024 * 16






#define ENGINE_STACK_SIZE			( 1024 * 32 )

typedef enum {
	PROC_EVENT_STATE_ACTIVE,
	PROC_EVENT_STATE_IDLE
} ProcEventState;

typedef enum {
	PROC_EVENT_TYPE_OUT_READABLE,
	PROC_EVENT_TYPE_ERR_READABLE,
	PROC_EVENT_TYPE_IN_WRITABLE,
	PROC_EVENT_TYPE_JOB_DONE
} ProcEventType;

typedef struct {
	HANDLE iocp;
} ProcEventEngine;

typedef struct {
	XplLock lock;
	ProcEventType type;
	ProcEventState state;
	WSAOVERLAPPED ovl;
} ProcEvent;






typedef enum {
	PROC_PRE = 0,
	PROC_READING,
	PROC_POST
} ProcReadableState;

typedef	struct {
	ProcReadableState state;
	XplBool firstReadStarted;
	XplBool firstReadCompleted;
	char buffer[ PROC_READ_BUFF_SIZE ];
	char *readPtr;
	char *writePtr;
	HANDLE thandle;  // this needs closed
	HANDLE *rhandle; // this is just a pointer to the real handle
	int err;
	struct XPLPROC *proc;
} ProcReadableStruct;
#endif

typedef struct XPLPROC {
	int ipipe;                                      /* input pipe */
	int opipe;                                      /* output pipe */
	int epipe;                                      /* error pipe */
	int reaped;
	XplPid pid;                                     /* pid */
#if defined(LINUX)
	int ipipeChild;                                 /* child end of the input pipe */
	int opipeChild;                                 /* child end of the output pipe */
	int epipeChild;                                 /* child end of the error pipe */
#elif defined(WINDOWS)
	XplBool done;
	HANDLE hProcess;
	HANDLE hJob;
	HANDLE iocp;
	JOBOBJECT_ASSOCIATE_COMPLETION_PORT port;
	HANDLE ihandle;
	HANDLE ohandle;
	HANDLE ehandle;
	XplLock lock;
	ProcReadableStruct rbOut;
	ProcReadableStruct rbErr;

	ProcEvent peOutReadable;
	ProcEvent peErrReadable;
	ProcEvent peInWritable;
	ProcEvent peJobDone;
	XplBool outReadable;
	XplBool errReadable;
	XplBool inWritable;
	XplBool jobDone;
#endif
	unsigned char	name[255];
} XPLPROC;

#include <xplproc.h>
