/****************************************************************************
 |
 | Copyright (c) 1999-2002 Novell, Inc.
 | All Rights Reserved.
 |
 | This program is free software; you can redistribute it and/or
 | modify it under the terms of version 2.1 of the GNU Lesser General
 | Public License as published by the Free Software Foundation.
 |
 | This program is distributed in the hope that it will be useful,
 | but WITHOUT ANY WARRANTY; without even the implied warranty of
 | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 | GNU Lesser General Public License for more details.
 |
 | You should have received a copy of the GNU Lesser General Public License
 | along with this program; if not, contact Novell, Inc.
 |
 | To contact Novell about this file by physical or electronic mail,
 | you may find current contact information at www.novell.com
 |
 |***************************************************************************/
#ifndef XPLSERVICE_H
#define XPLSERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

EXPORT int XplStart( int argc, char **argv, char *consumer );
EXPORT void XplStop( char *consumer );
EXPORT void *XplMemoryConfig( void );
int _XplMain( int argc, char **argv );

typedef void (*XplSignalFunc)( int );
EXPORT void XplSignalHandler( XplSignalFunc );

EXPORT int XplRandom( char *data, size_t length );
EXPORT XplBool XplMainFinished( void );

/*
	Register a callback to be called after XplMain/XplServiceMain have exited.

	Multiple callbacks may be registered. Each will be called in the same
	reverse order that they were registered.

	A callback can be removed with XplOnShutdownCancel(). If baton is NULL then
	any previously registered matching callback will be removed regardless of
	the baton value.
*/
typedef void (*XplShutdownFunc)( void *baton );
EXPORT void XplOnShutdown( XplShutdownFunc cb, void *baton );
EXPORT int XplOnShutdownCancel( XplShutdownFunc cb, void *baton );

#undef XplServiceCode
#undef XplServiceExit
#define XplServiceCode(s)
#define XplServiceExit(r)	return (r)
#define XplCheckExeName()

#define XplServiceMain	XplMain

#ifdef WIN32

EXPORT HWND XplGetMainWindowHandle(void);

# if defined _MSC_VER
#include <signal.h>
#  define SIGHUP		1 // todo find real windows equvalence
#  define SIGUSR1		10
#  define SIGUSR2		12
#  define _SIGMAX		32
#  if 0
#  define SIGHUP		1
#  define SIGINT		4
#  define SIGTERM		6
#  define SIGUSR1		10
#  define SIGUSR2		12
#  define _SIGMAX		32
#  define SIGABRT		??
# endif
# endif

#define XplSignalBlock(s)
#define XplSignalUnblock(s)
#define XplUnloadApp(id)		TerminateProcess( GetCurrentProcess(), 0 )

#define XPL_CLASS_NAME "NetmailClass"
EXPORT int _WinMain_( int (*_main_)( int, char **), int argc, char **argv );

// XplMain macro
// embeds a main() that calls _WinMain_ in memmgr/xplmain.c
// renames XplMain() in the consumer to _XplMain()
#define XplMain	main( int argc, char **argv )		\
	{return _WinMain_( _XplMain, argc, argv );}		\
int _XplMain

EXPORT int _ReturnZero( void );
#define XplGetUnprivilegedUser()	  _ReturnZero()
#define XplSetEffectiveUser(username) _ReturnZero()
#define XplSetEffectiveUserId(uid)	  _ReturnZero()
#define XplSetEffectiveGroupId(gid)   _ReturnZero()
#define XplSetRealUser(username)	  _ReturnZero()
#define XplSetRealUserId(uid)		  _ReturnZero()
#define XplSetRealGroupId(gid)		  _ReturnZero()
#define XplGetEffectiveUserId()	 	  _ReturnZero()

#endif	// WIN32

#if defined(LINUX) || defined(S390RH) || defined(MACOSX)

EXPORT void XplSignalBlock( void );
EXPORT void XplSignalUnblock( void );
#define XplUnloadApp(id)	kill( (id), SIGKILL )
int _LinuxMain_( int (*_main_)( int, char **), int argc, char **argv );

// XplMain macro
// embeds a main() that calls _LinuxMain_ in memmgr/xplmain.c
// renames XplMain() in the consumer to _XplMain()
#define XplMain	main( int argc, char **argv )				\
	{return _LinuxMain_( _XplMain, argc, argv );}			\
int _XplMain

char *XplGetUnprivilegedUser( void );
int XplSetEffectiveUser(const char *username);
int XplSetEffectiveUserId(uid_t uid);
int XplSetEffectiveGroupId(gid_t gid);

int XplSetRealUser(const char *username);
int XplSetRealUserId(uid_t uid);
int XplSetRealGroupId(gid_t gid);

#define XplGetEffectiveUserId()  geteuid()

#endif	// LINUX

#ifdef __cplusplus
}
#endif

#endif
