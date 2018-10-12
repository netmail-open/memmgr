#include <memmgr-config.h>
#include "xpl.h"
#include "memmgr.h"
#ifdef _MSC_VER
#include <crtdbg.h>
#endif

XplThread_		_xpl_main_thread_			= NULL;
XplThread_		_xpl_signal_thread_			= NULL;
XplSignalFunc	ApplicationSignalHandler	= NULL;

XplBool			_xpl_main_started_			= FALSE;
XplBool			_xpl_main_finished_			= FALSE;

typedef struct XplShutdownHandler
{
	struct XplShutdownHandler		*next;

	XplShutdownFunc					cb;
	void							*baton;
} XplShutdownHandler;

XplShutdownHandler					*_xpl_shutdown_handlers	= NULL;
XplMutex							_xpl_shutdown_lock;

EXPORT void XplOnShutdown( XplShutdownFunc cb, void *baton )
{
	XplShutdownHandler				*handler;

	if (!_xpl_main_started_) {
		/* This can not be used without using XplServiceMain */
		errno = EINVAL;
		return;
	}

	handler			= MemCallocWait(1, sizeof(XplShutdownHandler));

	handler->cb		= cb;
	handler->baton	= baton;

	XplMutexLock( _xpl_shutdown_lock );

	handler->next	= _xpl_shutdown_handlers;
	_xpl_shutdown_handlers = handler;

	XplMutexUnlock( _xpl_shutdown_lock );
	errno = 0;
}

static void XplHandleShutdown( void )
{
	XplShutdownHandler				*handler;

	XplMutexLock( _xpl_shutdown_lock );

	while ((handler = _xpl_shutdown_handlers)) {
		/*
			Unlink before calling the callback in case it calls
			XplOnShutdownCancel.
		*/
		_xpl_shutdown_handlers = handler->next;

		if (handler && handler->cb) {
			handler->cb(handler->baton);
		}

		MemRelease(&handler);
	}

	XplMutexUnlock( _xpl_shutdown_lock );
}

EXPORT int XplOnShutdownCancel( XplShutdownFunc cb, void *baton )
{
	XplShutdownHandler				**handler;
	XplShutdownHandler				*h;
	int								count	= 0;

	if (!_xpl_main_started_) {
		/* This can not be used without using XplServiceMain */
		errno = EINVAL;
		return(0);
	}

	XplMutexLock( _xpl_shutdown_lock );

	handler = &_xpl_shutdown_handlers;
	for (; handler && (h = *handler); handler = &((*handler)->next)) {
		if (cb && cb != h->cb) {
			continue;
		}

		if (baton && baton != h->baton) {
			continue;
		}

		*handler = h->next;
		MemRelease(&h);
		count++;
	}

	XplMutexUnlock( _xpl_shutdown_lock );

	return(count);
}

EXPORT XplBool XplMainFinished( void )
{
	return _xpl_main_finished_;
}

#ifdef WINDOWS
HWND			mainWindowHandle			= NULL;
HCRYPTPROV		_cryptoProvider;
LPCSTR			_netmailKeyContainer;
#else
FILE			*_devRandomFP;
#endif
XplMutex		_randomMutex;

static int _ParseConsumer( char *argv0, char *buffer, size_t length )
{
	char	*p, *last;

	last = argv0;
	for(p=last;*p;p++)
	{
		if( ( '/' == *p ) || ( '\\' == *p ) )
		{
			if( *(p+1) )
			{
				last = p + 1;
			}
		}
	}
	strprintf( buffer, length, NULL, "%s", last );
	if( p = strchr( buffer, '.' ) )
	{
		*p = '\0';
	}
	strlower( buffer );
	return 0;
}

static int _signal_thread_( XplThread_ thread )
{
	int		signo;
	void	*done	= NULL;

	for (;;) {
		signo = XplThreadCatch(thread, &done, -1);

		if (ApplicationSignalHandler) {
			ApplicationSignalHandler(signo);
		}

		switch (signo) {
			case SIGTERM:
				if (done) {
					/*
						When the main thread is done it will signal this thread
						with a non-NULL context.
					*/
					return(0);
				}
				break;

#ifdef LINUX
			case SIGALRM:
				/* Trigger XplAlarm */
				XplAlarmSignal(signo);
				break;
#endif

			default:
				break;
		}
	}
}

#ifdef WIN32
static BOOL WINAPI _xpl_ctrl_handler_( DWORD type )
{
	int		sig	= 0;

	switch( type )
	{
		case CTRL_C_EVENT:
			sig = SIGINT;
			break;

		case CTRL_BREAK_EVENT:
		case CTRL_CLOSE_EVENT:
		case CTRL_LOGOFF_EVENT:
		case CTRL_SHUTDOWN_EVENT:
			sig = SIGTERM;
			break;

		default:
			return FALSE;
	}

	XplThreadSignal( _xpl_signal_thread_, sig, NULL);
	XplThreadSignal( _xpl_main_thread_, sig, NULL );

	return TRUE;
}

static LRESULT CALLBACK _XplMsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	if( ( msg > WM_USER ) && ( msg < WM_USER + _SIGMAX ) )
	{
		XplThreadSignal( _xpl_signal_thread_, msg - WM_USER, NULL);
		return 0;
	}
	return DefWindowProc( hWnd, msg, wParam, lParam );
}

#define MAX_ARGV	20

EXPORT void XplSignalHandler( XplSignalFunc func )
{
	ApplicationSignalHandler = func;
}

typedef struct
{
	HWND	windowHandle;
	int		(*_main_)( int argc, char **argv );
	int		argc;
	char	*argv[MAX_ARGV+1];
}_MainArgs;

static int _main_thread_( XplThread_ thread )
{
	int			ret;
	_MainArgs	*arg = (_MainArgs *)thread->context;
	// DWORD		mainThread	= GetWindowThreadProcessId(arg->windowHandle, NULL);

	SetConsoleCtrlHandler( _xpl_ctrl_handler_, TRUE );
	ret = arg->_main_( arg->argc, arg->argv );
	SetConsoleCtrlHandler( _xpl_ctrl_handler_, FALSE );

	PostMessage( arg->windowHandle, WM_DESTROY, ret, 0 );
	// PostThreadMessage( mainThread, WM_DESTROY, ret, 0 );
	return ret;
}

#ifdef _MSC_VER
static void _invalid_param_handler(
	const wchar_t * expression,
	const wchar_t * function,
	const wchar_t * file,
	unsigned int line, uintptr_t reserved)
{
#if DEBUG
	wprintf(L"Invalid parameter detected in function \"%s\". %s:%d\n",
		function, file, line);
#endif
}
#endif

static int SetEnvPaths(void)
{
	long			ret				= -1;
	DWORD			dwType			= REG_SZ;
	HKEY			hkeyDXVer;
	DWORD			size;
	char			*path			= "SOFTWARE\\Wow6432Node\\MessagingArchitects\\Netmail Platform\\";
	char			buffer[XPL_MAX_PATH + 1];

	if (ERROR_SUCCESS != (ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, path, 0, KEY_READ, &hkeyDXVer))) {
		fprintf(stderr, "Could not read registry key: %s (%ld) \n", path, ret);

		return((int) ret);
	}

	size = sizeof(buffer);
	if (ERROR_SUCCESS != (ret = RegQueryValueEx(hkeyDXVer,"InstallPath", NULL, &dwType, (BYTE *)buffer, &size))) {
		fprintf(stderr, "Could not read install path: %s (%ld) \n", path, ret);
		return((int) ret);
	}

	SetEnvironmentVariable("NETMAIL_PLATFORM_INSTALL_PATH",	buffer);
	SetEnvironmentVariable("NETMAIL_BASE_DIR",				buffer);

	return(0);
}

static int expandenv(char *dest, const char *src, size_t size)
{
#ifdef WINDOWS
	int		r;

	if ((r = ExpandEnvironmentStrings(src, dest, size)) > 0) {
		if (r <= size) {
			return(0);
		} else {
			/* The output buffer wasn't large enough */
			return(-1);
		}
	}

	return(-2);
#else
	return(-1);
#endif
}

/* Add our base paths to the environment for the child process */
static void XplProcAddEnvPaths(void)
{
	char	*basedir	= XPL_BASE_DIR;
	char	*datadir	= XPL_DATA_DIR;
	char	*vardir		= XPL_DATA_VAR_DIR;
	char	*tmp, *value;
	size_t	len			= 32768; /* Max size of an environment variable */
	char	path[XPL_MAX_PATH + 1];

	if ((tmp = MemMallocWait(len))) {
		*tmp = '\0';

		/*
			Add our sbin path to the PATH variable. This acts as a lib path on
			windows so or DLLs can be found.
		*/
		value = XPL_DEFAULT_BIN_DIR;
		if (!expandenv(path, value, sizeof(path))) {
			value = path;
		}
		strcatf(tmp, len, NULL, "%s;", value);

		/* Include whatever value had previously been set for PATH */
		GetEnvironmentVariable("PATH", tmp + strlen(tmp), len - strlen(tmp));

		/* Ensure that system32 is in the path as well */
		value = "%WINDIR%\\system32";
		if (!expandenv(path, value, sizeof(path))) {
			value = path;
		}
		strcatf(tmp, len, NULL, ";%s", value);

		/* Store the new value */
		SetEnvironmentVariable("PATH", tmp);

		if (!(expandenv(tmp, basedir, len))) {
			basedir = tmp;
		}
		SetEnvironmentVariable("NETMAIL_BASE_DIR", basedir);

		if (!(expandenv(tmp, datadir, len))) {
			datadir = tmp;
		}
		SetEnvironmentVariable("NETMAIL_DATA_DIR", datadir);

		if (!(expandenv(tmp, vardir, len))) {
			vardir = tmp;
		}
		SetEnvironmentVariable("NETMAIL_DATAVAR_DIR", vardir);

		MemRelease(&tmp);
	}
}

EXPORT int _WinMain_( int (*_RealMain_)( int, char **), int argc, char **argv )
{
	int				i;
	_MainArgs		args;
	int				ret;
	char			consumer[1024];
	MSG				msg;
	int				(*_start_)( int, char **, char * ) = NULL;
	void			(*_stop_)( char * ) = NULL;
	void			*(*_mem_config_)( void ) = NULL;
	HINSTANCE	hInst		= GetModuleHandle(0);
	WNDCLASS	xplWndClass	= {
		CS_HREDRAW | CS_VREDRAW,	// style
		_XplMsgProc,				// lpfnWndProc
		0,							// cbClsExtra
		0,							// cbWndExtra
		NULL,						// hInstance
		NULL,						// hIcon
		NULL,						// hCursor
		NULL,						// hbrBackground
		NULL,						// lpszMenuName
		XPL_CLASS_NAME,				// lpszClassName
	};
	xplWndClass.hInstance = hInst;

#ifdef _MSC_VER
	/*
		Some windows functions will cause the program to terminate if called
		with invalid parameters (such as strftime). This is not desirable
		behavior for a server.

		By registering our handler we can deal with the return code of the
		function where it is called.
	*/
	_set_invalid_parameter_handler(_invalid_param_handler);

	/* prevent the pop up window when a CRT assert happens
	 	ctr lib will assert on ispsace and friends when char is out 
		of standard ASCII */
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);


#endif

	args._main_ = _RealMain_;
	args.argc = argc;
	for (i = 0; i < argc && i < MAX_ARGV; i++) {
		args.argv[i] = argv[i];
	}

	_ParseConsumer( args.argv[0], consumer, sizeof( consumer ) );

#ifdef __WATCOMC__
	_start_ = (int (*)(int, char **, char *))GetProcAddress( hInst, "_XplStart" );
	_stop_ = (void (*)(char *))GetProcAddress( hInst, "_XplStop" );
	_mem_config_ = (void *(*)(void))GetProcAddress( hInst, "_XplMemoryConfig" );
#else
	_start_ = (int (*)(int, char **, char *))GetProcAddress( hInst, "XplStart" );
	_stop_ = (void (*)(char *))GetProcAddress( hInst, "XplStop" );
	_mem_config_ = (void *(*)(void))GetProcAddress( hInst, "XplMemoryConfig" );
#endif

	MMOpenEx( consumer, (_mem_config_) ? (MemPoolConfig *)_mem_config_() : NULL, __FILE__, __LINE__ );
	if( _start_ )
	{
		if( ret = _start_( argc, argv, consumer ) )
		{
			MMClose( consumer, TRUE, __FILE__, __LINE__ );
			return ret;
		}
	}

	// Ensure that our environment variables are set
	GetEnvironmentVariable("NETMAIL_PLATFORM_INSTALL_PATH", NULL, 0);
	if (ERROR_ENVVAR_NOT_FOUND == GetLastError()) {
		SetEnvPaths();
	}
	XplProcAddEnvPaths();

	// Ensure that we have a console so that GenerateConsoleCtrlEvent can be used
	AllocConsole();


	// try and initialize a crypto provider for XplRandom()
	if( !CryptAcquireContext( &_cryptoProvider, _netmailKeyContainer, NULL, PROV_RSA_FULL, 0 ) )
	{
		if( NTE_BAD_KEYSET == GetLastError() )
		{
			if( !CryptAcquireContext( &_cryptoProvider, _netmailKeyContainer, NULL, PROV_RSA_FULL, CRYPT_NEWKEYSET ) )
			{
				_cryptoProvider = (HCRYPTPROV)NULL;
			}
		}
	}
	XplMutexInit( _randomMutex );
	XplMutexInit( _xpl_shutdown_lock );

	RegisterClass( &xplWndClass );
	if( args.windowHandle = CreateWindow( XPL_CLASS_NAME, consumer, WS_OVERLAPPED, 0,0,0,0, HWND_MESSAGE, NULL, hInst, NULL ) )
	{
		mainWindowHandle = args.windowHandle;

		XplThreadStart( NULL, _signal_thread_, NULL, &_xpl_signal_thread_ );
		XplThreadStart( NULL, _main_thread_, &args, &_xpl_main_thread_ );

		while (0 < GetMessage(&msg, NULL, 0U, 0U) && WM_QUIT != msg.message) {
			switch (msg.message) {
				case WM_CLOSE:
					XplThreadSignal( _xpl_signal_thread_, SIGTERM, NULL);
					XplThreadSignal( _xpl_main_thread_,   SIGTERM, NULL);
					break;

				case WM_USER + 1:
					XplThreadSignal( _xpl_signal_thread_, SIGUSR1, NULL);
					XplThreadSignal( _xpl_main_thread_,   SIGUSR1, NULL);
					break;

				case WM_USER + 2:
					XplThreadSignal( _xpl_signal_thread_, SIGUSR2, NULL);
					XplThreadSignal( _xpl_main_thread_,   SIGUSR2, NULL);
					break;

				case WM_DESTROY:
					PostQuitMessage((int) msg.wParam);
					break;

				default:
					TranslateMessage( &msg );
					DispatchMessage( &msg );
					break;
			}
		}

		mainWindowHandle = NULL;
		DestroyWindow( args.windowHandle );
	}

	UnregisterClass( XPL_CLASS_NAME, hInst );

	_xpl_main_started_ = TRUE;
	ret = XplThreadJoin( _xpl_main_thread_, -1 );
	XplThreadFree( &_xpl_main_thread_ );
	_xpl_main_finished_ = TRUE;

	XplHandleShutdown();

	if (_xpl_signal_thread_) {
		XplThreadSignal( _xpl_signal_thread_, SIGTERM, (void *) TRUE );

		XplThreadJoin( _xpl_signal_thread_, -1 );
		XplThreadFree( &_xpl_signal_thread_ );
	}

	XplMutexDestroy( _randomMutex );
	XplMutexDestroy( _xpl_shutdown_lock );
	if( _cryptoProvider )
	{
		CryptReleaseContext( _cryptoProvider, 0 );
		_cryptoProvider = (HCRYPTPROV)NULL;
	}
	if( _stop_ )
	{
		_stop_( consumer );
	}
	MMClose( consumer, TRUE, __FILE__, __LINE__ );
	return ret;
}

EXPORT HWND XplGetMainWindowHandle(void)
{
	return(mainWindowHandle);
}

EXPORT int XplRandom( char *data, size_t length )
{
	DWORD	size = (DWORD)length;

	if( _cryptoProvider )
	{
		XplMutexLock( _randomMutex );
		if( CryptGenRandom( _cryptoProvider, size, data ) )
		{
			XplMutexUnlock( _randomMutex );
			return 0;
		}
		XplMutexUnlock( _randomMutex );
		errno = ENOSYS;
	}
	errno = ENOENT;
	return -1;
}

#endif	// WIN32

#if defined(LINUX) || defined(S390RH) || defined(MACOSX)

int _LinuxMain_( int (*_main_)( int, char **), int argc, char **argv )
{
	int				ret;
	void			*handle;
	int				(*_start_)( int, char **, char * ) = NULL;
	void			(*_stop_)( char * ) = NULL;
	void			*(*_mem_config_)( void ) = NULL;
	char			consumer[1024];

	_ParseConsumer( argv[0], consumer, sizeof( consumer ) );
	if( handle = dlopen( NULL, RTLD_NOW ) )
	{
		_start_ = (int (*)(int, char **, char *))dlsym( handle, "XplStart" );
		_stop_ = (void (*)(char *))dlsym( handle, "XplStop" );
		_mem_config_ = (void *(*)(void))dlsym( handle, "XplMemoryConfig" );
		dlclose( handle );
	}

	MMOpenEx( consumer, (_mem_config_) ? (MemPoolConfig *)_mem_config_() : NULL, __FILE__, __LINE__ );
	if( _start_ )
	{
		if( ret = _start_( argc, argv, consumer ) )
		{
			MMClose( consumer, TRUE, __FILE__, __LINE__ );
			return ret;
		}
	}

	_devRandomFP = fopen( "/dev/urandom", "r" );

	XplMutexInit( _randomMutex );
	XplMutexInit( _xpl_shutdown_lock );

	XplThreadStart( NULL, _signal_thread_, NULL, &_xpl_signal_thread_ );
	// build a thread context on this thread
	XplThreadWrap( &_xpl_main_thread_ );
	// call main()

	_xpl_main_started_ = TRUE;
	ret = _main_( argc, argv );
	_xpl_main_finished_ = TRUE;
	XplHandleShutdown();

	// The signal handler can not fire once the main thread has exited.
	XplSignalHandler( NULL );
	// remove context from this thread
	XplThreadWrapEnd( &_xpl_main_thread_ );

	if (_xpl_signal_thread_) {
		XplThreadSignal( _xpl_signal_thread_, SIGTERM, (void *) TRUE );

		XplThreadJoin( _xpl_signal_thread_, -1 );
		XplThreadFree( &_xpl_signal_thread_ );
	}
	if( _devRandomFP )
	{
		fclose( _devRandomFP );
	}
	if( _stop_ )
	{
		_stop_( consumer );
	}
	XplMutexDestroy( _randomMutex );
	XplMutexDestroy( _xpl_shutdown_lock );
	MMClose( consumer, TRUE, __FILE__, __LINE__ );
	return ret;
}

EXPORT int XplRandom( char *data, size_t length )
{
	if( _devRandomFP )
	{
		XplMutexLock( _randomMutex );
		if( length == fread( data, 1, length, _devRandomFP ) )
		{
			XplMutexUnlock( _randomMutex );
			return 0;
		}
		XplMutexUnlock( _randomMutex );
	}
	errno = ENOENT;
	return -1;
}

static void XPLSignalProcessor(int signo, siginfo_t *info, void *context)
{
	if (_xpl_signal_thread_) {
		XplThreadSignal( _xpl_signal_thread_, signo, NULL );
	}

	switch( signo )
	{
		case SIGINT:
		case SIGTERM:
			XplThreadSignal( _xpl_main_thread_, SIGTERM, NULL );
			break;

		default:
			break;
	}

	return;
}

unsigned int _XplSignalList[] = {
    SIGTERM, SIGINT, SIGUSR1, SIGUSR2, SIGHUP, SIGALRM,

    /* debugging - catch these signals too for now */
    SIGVTALRM, SIGPIPE,

#ifdef SIGPOLL
    SIGPOLL,
#endif
#ifdef SIGSTKFLT
    SIGSTKFLT,
#endif
#ifdef SIGPWR
    SIGPWR,
#endif
#ifdef SIGUNUSED
    SIGUNUSED,
#endif

    0  /* zero used as end-of-array marker, do not remove ;) */
};

void XplSignalBlock(void)
{
    sigset_t		set;
    int				i;

    sigemptyset(&set);
    for (i = 0; _XplSignalList[i]; i++) {
        sigaddset(&set, _XplSignalList[i]);
    }

    pthread_sigmask(SIG_BLOCK, &set, NULL);
    return;
}

void XplSignalUnblock(void)
{
    sigset_t		set;
    int				i;

    sigemptyset(&set);
    for (i = 0; _XplSignalList[i]; i++) {
        sigaddset(&set, _XplSignalList[i]);
    }

    pthread_sigmask(SIG_UNBLOCK, &set, NULL);
    return;
}

void XplSignalHandler( XplSignalFunc signalFunc )
{
    struct sigaction	act;
    int					i;

	memset(&act, 0, sizeof(struct sigaction));
    XplSignalBlock();

    ApplicationSignalHandler = signalFunc;

    act.sa_sigaction	= XPLSignalProcessor;
	act.sa_flags		= SA_SIGINFO;

    for (i = 0; _XplSignalList[i]; i++) {
		if (sigaction(_XplSignalList[i], &act, NULL)) {
			perror("XplSignalHandler()");
		}
    }

	XplSignalUnblock();

    return;
}

#if defined(LINUX)
/*
	Handler for SIGTRAP that will ignore the signal. This is to be used with
	DebugAssert() when set to break. When not in the debugger this handler will
	be called.
*/
void SigTrapIgnoreHandler(int signum)
{
    signal(SIGTRAP, SIG_DFL);
}

void SigTrapAttachHandler(int signum)
{
	int		pid;

	signal(SIGTRAP, SIG_DFL);

	printf("\n\nDEBUG: Would you like to attach the debugger to this process? (%d)\n", getpid());
	printf("DEBUG: y/n: ");
	fflush(stdout);

	switch (getchar()) {
		default:
			return;

		case 'y':
		case 'Y':
			break;
	}

	switch ((pid = fork())) {
		case -1:
			fprintf(stderr, "failed to fork\n");
			fflush(stderr);
			break;

		case 0: {
			/* child */
			char cmd[256];
			char *argv[4] = {"sh", "-c", cmd, 0};

			sprintf(cmd, "gdb %s %d", argv[0], getppid());
			execve("/bin/sh", argv, NULL);
			exit(127);
		}

		default:
			sleep(5);
	}
}
#endif	// LINUX
#endif	// defined(LINUX) || defined(S390RH) || defined(MACOSX)

