#ifndef XPLPROCESS
typedef void * XPLPROCESS;
#endif

#define EXIT_NOCHDIR 0200
#define EXIT_RESTART 0201		/* agent requests launcher to restart */

typedef enum {
	SIGNAL_STOP,
	SIGNAL_HALT,
	SIGNAL_KILL,
	SIGNAL_ABORT
} XplSignalType;


/* Agent callbacks */

typedef void (*agentOutputCB) (XPLPROCESS *process, void *data, char *buffer);
typedef void (*agentExitCB) (char *agent, void *data, void *second, int state, time_t start, uint32 flags);
typedef void (*agentSignalCB) (char *agent, uint64 pid, void *data, void *second, int sig, time_t start, uint32 flags, XplBool coredump);


/* Agent interface */

EXPORT void XplProcessInit(void);

EXPORT XplBool XplProcess(char *agent, char *path, char *const argv[], void *data, void *second, uint32 flags, agentOutputCB ocb, agentExitCB ecb, agentSignalCB scb);

EXPORT uint32 XplGetAgentFlags(char *agent);
EXPORT XplBool XplSetAgentFlags(char *agent, uint32 flags);
EXPORT XplBool XplClearAgentFlags(char *agent, uint32 flags);
EXPORT void XplSetAllAgentFlags(uint32 flags);
EXPORT void XplClearAllAgentFlags(uint32 flags);

EXPORT int XplSignalAgent(char *agent, XplSignalType type, uint32 flags);
EXPORT void XplSignalAllAgents(XplSignalType type, uint32 flags);

EXPORT XplBool XplWaitAgent(char *agent, int timeout);

EXPORT XPLPROCESS *XplListAgents(void);
EXPORT XPLPROCESS *XplListNextAgent(XPLPROCESS *process);
EXPORT XplBool XplAgentExists(char *agent);

/* OS-independent process interface */

EXPORT char *XplProcessName(XPLPROCESS *process);
EXPORT uint32 XplGetProcessFlags(XPLPROCESS *process);
EXPORT void XplSetProcessFlags(XPLPROCESS *process, uint32 flags);
EXPORT void XplClearProcessFlags(XPLPROCESS *process, uint32 flags);
EXPORT time_t XplProcessStartTime(XPLPROCESS *process);


/* OS-dependent process interface */
EXPORT char * XplGetBinaryName( void );
EXPORT XPLPROCESS *XplCreateProcess(char *agent, char *path, char *const argv[], uint32 flags);
#if defined(WINDOWS)
EXPORT int XplProcessID(XPLPROCESS *process);
#else
EXPORT pid_t XplProcessID(XPLPROCESS *process);
#endif
EXPORT void XplReadProcess(XPLPROCESS *process, agentOutputCB ocb, void *data);
EXPORT int XplSignalProcess(XPLPROCESS *process, XplSignalType type);
EXPORT XplBool XplCreateDetachedProcess(char *const argv[], char *path);
EXPORT XplBool XplPollProcess(XPLPROCESS **process, int *state, int *signal, XplBool *coredump);

