/****************************************************************************
 *
 * Copyright (c) 2001-2002 Novell, Inc.
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2.1 of the GNU Lesser General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, contact Novell, Inc.
 *
 * To contact Novell about this file by physical or electronic mail,
 * you may find current contact information at www.novell.com
 *
 ****************************************************************************/

#include <memmgr-config.h>
#include <xpl.h>
#include <stdio.h>
#include <ctype.h>
#include <xplerr.h>

#ifdef DEBUG
#define TEST_ERROR_NUM_BASE		930
#define TEST_ERROR_DEF_ONLY		(TEST_ERROR_NUM_BASE+0)
#define TEST_ERROR_STR_ONLY		(TEST_ERROR_NUM_BASE+1)
#define TEST_ERROR_UNDEFINED	(TEST_ERROR_NUM_BASE+2) // neither
#endif

EXPORT char * XplStrErrorDefine(int errnum)
{
	if (errnum < 0) errnum = -errnum;

	switch (errnum) {
#if defined(EPERM)
			case EPERM:
				return "EPERM";
#endif
#if defined(ENOENT)
			case ENOENT:
				return "ENOENT";
#endif
#if defined(ESRCH)
			case ESRCH:
				return "ESRCH";
#endif
#if defined(EINTR)
			case EINTR:
				return "EINTR";
#endif
#if defined(EIO)
			case EIO:
				return "EIO";
#endif
#if defined(ENXIO)
			case ENXIO:
				return "ENXIO";
#endif
#if defined(E2BIG)
			case E2BIG:
				return "E2BIG";
#endif

#if defined(ENOEXEC)
			case ENOEXEC:
				return "ENOEXEC";
#endif
#if defined(EBADF)
			case EBADF:
				return "EBADF";
#endif
#if defined(ECHILD)
			case ECHILD:
				return "ECHILD";
#endif
		/*==================*/
		/* EAGAIN or EWOULDBLOCK */
		/*==================*/
#if defined(EAGAIN)
#if defined(EWOULDBLOCK) && (EAGAIN != EWOULDBLOCK)
				case EAGAIN:
					return "EAGAIN";
#elif !defined(EWOULDBLOCK)
				case EAGAIN:
					return "EAGAIN";
#endif
#endif
#if defined(EWOULDBLOCK)
#if defined(EAGAIN)
				 #if (EAGAIN != EWOULDBLOCK)
				 case EWOULDBLOCK:
					return "EWOULDBLOCK";
				 #else
				 case EWOULDBLOCK:
					return "EWOULDBLOCK or EAGAIN";
				 #endif
#else
			case EWOULDBLOCK:
				return "EWOULDBLOCK";
#endif
#endif
#if defined(ENOMEM)
			case ENOMEM:
				return "ENOMEM";
#endif
#if defined(EACCES)
			case EACCES:
				return "EACCES";
#endif
#if defined(EFAULT)
			case EFAULT:
				return "EFAULT";
#endif
#if defined(ENOTBLK)
			case ENOTBLK:
				return "ENOTBLK";
#endif
#if defined(EBUSY)
			case EBUSY:
				return "EBUSY";
#endif
#if defined(EEXIST)
			case EEXIST:
				return "EEXIST";
#endif
#if defined(EXDEV)
			case EXDEV:
				return "EXDEV";
#endif
#if defined(ENODEV)
			case ENODEV:
				return "ENODEV";
#endif
#if defined(ENOTDIR)
			case ENOTDIR:
				return "ENOTDIR";
#endif
#if defined(EISDIR)
			case EISDIR:
				return "EISDIR";
#endif
#if defined(EINVAL)
			case EINVAL:
				return "EINVAL";
#endif
#if defined(ENFILE)
			case ENFILE:
				return "ENFILE";
#endif
#if defined(EMFILE)
			case EMFILE:
				return "EMFILE";
#endif
#if defined(ENOTTY)
			case ENOTTY:
				return "ENOTTY";
#endif
#if defined(ETXTBSY)
			case ETXTBSY:
				return "ETXTBSY";
#endif
#if defined(EFBIG)
			case EFBIG:
				return "EFBIG";
#endif
#if defined(ENOSPC)
			case ENOSPC:
				return "ENOSPC";
#endif
#if defined(ESPIPE)
			case ESPIPE:
				return "ESPIPE";
#endif
#if defined(EROFS)
			case EROFS:
				return "EROFS";
#endif
#if defined(EMLINK)
			case EMLINK:
				return "EMLINK";
#endif
#if defined(EPIPE)
			case EPIPE:
				return "EPIPE";
#endif
#if defined(EDOM)
			case EDOM:
				return "EDOM";
#endif
#if defined(ERANGE)
			case ERANGE:
				return "ERANGE";
#endif
		/*==================*/
		/* EDEADLK or EDEADLOCK */
		/*==================*/
#if defined(EDEADLK)
#if defined(EDEADLOCK) && (EDEADLK != EDEADLOCK)
				case EDEADLK:
					return "EDEADLK";
#elif !defined(EDEADLOCK)
				case EDEADLK:
					return "EDEADLK";
#endif
#endif
#if defined(EDEADLOCK)
#if defined(EDEADLK)
#if (EDEADLK != EDEADLOCK)
					case EDEADLOCK:
						return "EDEADLOCK";
#else
					case EDEADLOCK:
					return "EDEADLOCK or EDEADLK";
#endif
#else
				case EDEADLOCK:
					return "EDEADLOCK";
#endif
#endif
#if defined(ENAMETOOLONG)
			case ENAMETOOLONG:
				return "ENAMETOOLONG";
#endif
#if defined(ENOLCK)
			case ENOLCK:
				return "ENOLCK";
#endif
#if defined(ENOSYS)
			case ENOSYS:
				return "ENOSYS";
#endif
#if defined(ENOTEMPTY)
			case ENOTEMPTY:
				return "ENOTEMPTY";
#endif
#if defined(ELOOP)
			case ELOOP:
				return "ELOOP";
#endif
#if defined(ENOMSG)
			case ENOMSG:
				return "ENOMSG";
#endif
#if defined(EIDRM)
			case EIDRM:
				return "EIDRM";
#endif
#if defined(ECHRNG)
			case ECHRNG:
				return "ECHRNG";
#endif
#if defined(EL2NSYNC)
			case EL2NSYNC:
				return "EL2NSYNC";
#endif
#if defined(EL3HLT)
			case EL3HLT:
				return "EL3HLT";
#endif
#if defined(EL3RST)
			case EL3RST:
				return "EL3RST";
#endif
#if defined(ELNRNG)
			case ELNRNG:
				return "ELNRNG";
#endif
#if defined(EUNATCH)
			case EUNATCH:
				return "EUNATCH";
#endif
#if defined(ENOCSI)
			case ENOCSI:
				return "ENOCSI";
#endif
#if defined(EL2HLT)
			case EL2HLT:
				return "EL2HLT";
#endif
#if defined(EBADE)
			case EBADE:
				return "EBADE";
#endif
#if defined(EBADR)
			case EBADR:
				return "EBADR";
#endif
#if defined(EXFULL)
			case EXFULL:
				return "EXFULL";
#endif
#if defined(ENOANO)
			case ENOANO:
				return "ENOANO";
#endif
#if defined(EBADRQC)
			case EBADRQC:
				return "EBADRQC";
#endif
#if defined(EBADSLT)
			case EBADSLT:
				return "EBADSLT";
#endif
#if defined(EBFONT)
			case EBFONT:
				return "EBFONT";
#endif
#if defined(ENOSTR)
			case ENOSTR:
				return "ENOSTR";
#endif
#if defined(ENODATA)
			case ENODATA:
				return "ENODATA";
#endif
#if defined(ETIME)
			case ETIME:
				return "ETIME";
#endif
#if defined(ENOSR)
			case ENOSR:
				return "ENOSR";
#endif
#if defined(ENONET)
			case ENONET:
				return "ENONET";
#endif
#if defined(ENOPKG)
			case ENOPKG:
				return "ENOPKG";
#endif
#if defined(EREMOTE)
			case EREMOTE:
				return "EREMOTE";
#endif
#if defined(ENOLINK)
			case ENOLINK:
				return "ENOLINK";
#endif
#if defined(EADV)
			case EADV:
				return "EADV";
#endif
#if defined(ESRMNT)
			case ESRMNT:
				return "ESRMNT";
#endif
#if defined(ECOMM)
			case ECOMM:
				return "ECOMM";
#endif
#if defined(EPROTO)
			case EPROTO:
				return "EPROTO";
#endif
#if defined(EMULTIHOP)
			case EMULTIHOP:
				return "EMULTIHOP";
#endif
#if defined(EDOTDOT)
			case EDOTDOT:
				return "EDOTDOT";
#endif
#if defined(EBADMSG)
			case EBADMSG:
				return "EBADMSG";
#endif
#if defined(EOVERFLOW)
			case EOVERFLOW:
				return "EOVERFLOW";
#endif
#if defined(ENOTUNIQ)
			case ENOTUNIQ:
				return "ENOTUNIQ";
#endif
#if defined(EBADFD)
			case EBADFD:
				return "EBADFD";
#endif
#if defined(EREMCHG)
			case EREMCHG:
				return "EREMCHG";
#endif
#if defined(ELIBACC)
			case ELIBACC:
				return "ELIBACC";
#endif
#if defined(ELIBBAD)
			case ELIBBAD:
				return "ELIBBAD";
#endif
#if defined(ELIBSCN)
			case ELIBSCN:
				return "ELIBSCN";
#endif
#if defined(ELIBMAX)
			case ELIBMAX:
				return "ELIBMAX";
#endif
#if defined(ELIBEXEC)
			case ELIBEXEC:
				return "ELIBEXEC";
#endif
#if defined(EILSEQ)
			case EILSEQ:
				return "EILSEQ";
#endif
#if defined(ERESTART)
			case ERESTART:
				return "ERESTART";
#endif
#if defined(ESTRPIPE)
			case ESTRPIPE:
				return "ESTRPIPE";
#endif
#if defined(EUSERS)
			case EUSERS:
				return "EUSERS";
#endif
#if defined(ENOTSOCK)
			case ENOTSOCK:
				return "ENOTSOCK";
#endif
#if defined(EDESTADDRREQ)
			case EDESTADDRREQ:
				return "EDESTADDRREQ";
#endif
#if defined(EMSGSIZE)
			case EMSGSIZE:
				return "EMSGSIZE";
#endif
#if defined(EPROTOTYPE)
			case EPROTOTYPE:
				return "EPROTOTYPE";
#endif
#if defined(ENOPROTOOPT)
			case ENOPROTOOPT:
				return "ENOPROTOOPT";
#endif
#if defined(EPROTONOSUPPORT)
			case EPROTONOSUPPORT:
				return "EPROTONOSUPPORT";
#endif
#if defined(ESOCKTNOSUPPORT)
			case ESOCKTNOSUPPORT:
				return "ESOCKTNOSUPPORT";
#endif
#if defined(EOPNOTSUPP)
			case EOPNOTSUPP:
				return "EOPNOTSUPP";
#endif
#if defined(EPFNOSUPPORT)
			case EPFNOSUPPORT:
				return "EPFNOSUPPORT";
#endif
#if defined(EAFNOSUPPORT)
			case EAFNOSUPPORT:
				return "EAFNOSUPPORT";
#endif
#if defined(EADDRINUSE)
			case EADDRINUSE:
				return "EADDRINUSE";
#endif
#if defined(EADDRNOTAVAIL)
			case EADDRNOTAVAIL:
				return "EADDRNOTAVAIL";
#endif
#if defined(ENETDOWN)
			case ENETDOWN:
				return "ENETDOWN";
#endif
#if defined(ENETUNREACH)
			case ENETUNREACH:
				return "ENETUNREACH";
#endif
#if defined(ENETRESET)
			case ENETRESET:
				return "ENETRESET";
#endif
#if defined(ECONNABORTED)
			case ECONNABORTED:
				return "ECONNABORTED";
#endif
#if defined(ECONNRESET)
			case ECONNRESET:
				return "ECONNRESET";
#endif
#if defined(ENOBUFS)
			case ENOBUFS:
				return "ENOBUFS";
#endif
#if defined(EISCONN)
			case EISCONN:
				return "EISCONN";
#endif
#if defined(ENOTCONN)
			case ENOTCONN:
				return "ENOTCONN";
#endif
#if defined(ESHUTDOWN)
			case ESHUTDOWN:
				return "ESHUTDOWN";
#endif
#if defined(ETOOMANYREFS)
			case ETOOMANYREFS:
				return "ETOOMANYREFS";
#endif
#if defined(ETIMEDOUT)
			case ETIMEDOUT:
				return "ETIMEDOUT";
#endif
#if defined(ECONNREFUSED)
			case ECONNREFUSED:
				return "ECONNREFUSED";
#endif
#if defined(EHOSTDOWN)
			case EHOSTDOWN:
				return "EHOSTDOWN";
#endif
#if defined(EHOSTUNREACH)
			case EHOSTUNREACH:
				return "EHOSTUNREACH";
#endif
#if defined(EALREADY)
			case EALREADY:
				return "EALREADY";
#endif
#if defined(EINPROGRESS)
			case EINPROGRESS:
				return "EINPROGRESS";
#endif
#if defined(ESTALE)
			case ESTALE:
				return "ESTALE";
#endif
#if defined(EUCLEAN)
			case EUCLEAN:
				return "EUCLEAN";
#endif
#if defined(ENOTNAM)
			case ENOTNAM:
				return "ENOTNAM";
#endif
#if defined(ENAVAIL)
			case ENAVAIL:
				return "ENAVAIL";
#endif
#if defined(EISNAM)
			case EISNAM:
				return "EISNAM";
#endif
#if defined(EREMOTEIO)
			case EREMOTEIO:
				return "EREMOTEIO";
#endif
#if defined(EDQUOT)
			case EDQUOT:
				return "EDQUOT";
#endif
#if defined(ENOMEDIUM)
			case ENOMEDIUM:
				return "ENOMEDIUM";
#endif
#if defined(EMEDIUMTYPE)
			case EMEDIUMTYPE:
				return "EMEDIUMTYPE";
#endif
#if defined(ECANCELED)
			case ECANCELED:
				return "ECANCELED";
#endif
#if defined(ENOKEY)
			case ENOKEY:
				return "ENOKEY";
#endif
#if defined(EKEYEXPIRED)
			case EKEYEXPIRED:
				return "EKEYEXPIRED";
#endif
#if defined(EKEYREVOKED)
			case EKEYREVOKED:
				return "EKEYREVOKED";
#endif
#if defined(EKEYREJECTED)
			case EKEYREJECTED:
				return "EKEYREJECTED";
#endif
#if defined(EOWNERDEAD)
			case EOWNERDEAD:
				return "EOWNERDEAD";
#endif
#if defined(ENOTRECOVERABLE)
			case ENOTRECOVERABLE:
				return "ENOTRECOVERABLE";
#endif

#if defined(ERESTARTSYS)
			case ERESTARTSYS:
				return "ERESTARTSYS";
#endif
#if defined(ERESTARTNOINTR)
			case ERESTARTNOINTR:
				return "ERESTARTNOINTR";
#endif
#if defined(ERESTARTNOHAND)
			case ERESTARTNOHAND:
				return "ERESTARTNOHAND";
#endif
#if defined(ENOIOCTLCMD)
			case ENOIOCTLCMD:
				return "ENOIOCTLCMD";
#endif
#if defined(ERESTART_RESTARTBLOCK)
			case ERESTART_RESTARTBLOCK:
				return "ERESTART_RESTARTBLOCK";
#endif
#if defined(EBADHANDLE)
			case EBADHANDLE:
				return "EBADHANDLE";
#endif
#if defined(ENOTSYNC)
			case ENOTSYNC:
				return "ENOTSYNC";
#endif
#if defined(EBADCOOKIE)
			case EBADCOOKIE:
				return "EBADCOOKIE";
#endif
#if defined(ENOTSUPP)
			case ENOTSUPP:
				return "ENOTSUPP";
#endif
#if defined(ETOOSMALL)
			case ETOOSMALL:
				return "ETOOSMALL";
#endif
#if defined(ESERVERFAULT)
			case ESERVERFAULT:
				return "ESERVERFAULT";
#endif
#if defined(EBADTYPE)
			case EBADTYPE:
				return "EBADTYPE";
#endif
#if defined(EJUKEBOX)
			case EJUKEBOX:
				return "EJUKEBOX";
#endif
#if defined(EIOCBQUEUED)
			case EIOCBQUEUED:
				return "EIOCBQUEUED";
#endif
#if defined(EIOCBRETRY)
			case EIOCBRETRY:
				return "EIOCBRETRY";
#endif

#if defined(ECONNCLOSEDBYPEER)
			case ECONNCLOSEDBYPEER:
				return "ECONNCLOSEDBYPEER";
#endif
#if defined(ECONNIOBUG)
			case ECONNIOBUG:
				return "ECONNIOBUG";
#endif
#if defined(ECONNIOMODE)
			case ECONNIOMODE:
				return "ECONNIOMODE";
#endif
#if defined(ECONNIOINIT)
			case ECONNIOINIT:
				return "ECONNIOINIT";
#endif

#ifdef DEBUG
		// These three labels are arbitrarily assigned for the purposes of doing
		// unit tests of XplPErrorEx() below.
			case TEST_ERROR_DEF_ONLY:
				return "TEST_ERROR_DEF_ONLY";
			case TEST_ERROR_STR_ONLY:
			case TEST_ERROR_UNDEFINED:
		// fall through
#endif
			default:
				return NULL;														break;
	}
}

EXPORT char * XplStrError(int errnum)
{
	char	*error;
	int		preverrno = errno;

	if (errnum < 0) errnum = -errnum;

	errno = 0;
	if (!(error = strerror(errnum) ) || errno == EINVAL || !strnicmp(error, "unknown error", 13)) {
		switch (errnum) {
#if defined(ENOTBLK)
			case ENOTBLK:				error = "Block device required";									break;
#endif
#if defined(ETXTBSY)
			case ETXTBSY:				error = "Text file busy";											break;
#endif
#if defined(ELOOP)
			case ELOOP:					error = "Too many symbolic links encountered";						break;
#endif
#if defined(EIDRM)
			case EIDRM:					error = "Identifier removed";										break;
#endif
#if defined(ECHRNG)
			case ECHRNG:				error = "Channel number out of range";								break;
#endif
#if defined(EL2NSYNC)
			case EL2NSYNC:				error = "Level 2 not synchronized";									break;
#endif
#if defined(EL3HLT)
			case EL3HLT:				error = "Level 3 halted";											break;
#endif
#if defined(EL3RST)
			case EL3RST:				error = "Level 3 reset";											break;
#endif
#if defined(ELNRNG)
			case ELNRNG:				error = "Link number out of range";									break;
#endif
#if defined(EUNATCH)
			case EUNATCH:				error = "Protocol driver not attached";								break;
#endif
#if defined(ENOCSI)
			case ENOCSI:				error = "No CSI structure available";								break;
#endif
#if defined(EL2HLT)
			case EL2HLT:				error = "Level 2 halted";											break;
#endif
#if defined(EBADE)
			case EBADE:					error = "Invalid exchange";											break;
#endif
#if defined(EBADR)
			case EBADR:					error = "Invalid request descriptor";								break;
#endif
#if defined(EXFULL)
			case EXFULL:				error = "Exchange full";											break;
#endif
#if defined(ENOANO)
			case ENOANO:				error = "No anode";													break;
#endif
#if defined(EBADRQC)
			case EBADRQC:				error = "Invalid request code";										break;
#endif
#if defined(EBADSLT)
			case EBADSLT:				error = "Invalid slot";												break;
#endif
#if defined(EBFONT)
			case EBFONT:				error = "Bad font file format";										break;
#endif
#if defined(ENOSTR)
			case ENOSTR:				error = "Device not a stream";										break;
#endif
#if defined(ENODATA)
			case ENODATA:				error = "No data available";										break;
#endif
#if defined(ETIME)
			case ETIME:					error = "Timer expired";											break;
#endif
#if defined(ENOSR)
			case ENOSR:					error = "Out of streams resources";									break;
#endif
#if defined(ENONET)
			case ENONET:				error = "Machine is not on the network";							break;
#endif
#if defined(ENOPKG)
			case ENOPKG:				error = "Package not installed";									break;
#endif
#if defined(EREMOTE)
			case EREMOTE:				error = "Object is remote";											break;
#endif
#if defined(ENOLINK)
			case ENOLINK:				error = "Link has been severed";									break;
#endif
#if defined(EADV)
			case EADV:					error = "Advertise error";											break;
#endif
#if defined(ESRMNT)
			case ESRMNT:				error = "Srmount error";											break;
#endif
#if defined(ECOMM)
			case ECOMM:					error = "Communication error on send";								break;
#endif
#if defined(EPROTO)
			case EPROTO:				error = "Protocol error";											break;
#endif
#if defined(EMULTIHOP)
			case EMULTIHOP:				error = "Multihop attempted";										break;
#endif
#if defined(EDOTDOT)
			case EDOTDOT:				error = "RFS specific error";										break;
#endif
#if defined(EBADMSG)
			case EBADMSG:				error = "Not a data message";										break;
#endif
#if defined(EOVERFLOW)
			case EOVERFLOW:				error = "Value too large for defined data type";					break;
#endif
#if defined(ENOTUNIQ)
			case ENOTUNIQ:				error = "Name not unique on network";								break;
#endif
#if defined(EBADFD)
			case EBADFD:				error = "File descriptor in bad state";								break;
#endif
#if defined(EREMCHG)
			case EREMCHG:				error = "Remote address changed";									break;
#endif
#if defined(ELIBACC)
			case ELIBACC:				error = "Can not access a needed shared library";					break;
#endif
#if defined(ELIBSCN)
			case ELIBSCN:				error = ".lib section in a.out corrupted";							break;
#endif
#if defined(ELIBMAX)
			case ELIBMAX:				error = "Attempting to link in too many shared libraries";			break;
#endif
#if defined(ELIBEXEC)
			case ELIBEXEC:				error = "Cannot exec a shared library directly";					break;
#endif
#if defined(ERESTART)
			case ERESTART:				error = "Interrupted system call should be restarted";				break;
#endif
#if defined(ESTRPIPE)
			case ESTRPIPE:				error = "Streams pipe error";										break;
#endif
#if defined(EUSERS)
			case EUSERS:				error = "Too many users";											break;
#endif
#if defined(ENOTSOCK)
			case ENOTSOCK:				error = "Socket operation on non-socket";							break;
#endif
#if defined(EDESTADDRREQ)
			case EDESTADDRREQ:			error = "Destination address required";								break;
#endif
#if defined(EMSGSIZE)
			case EMSGSIZE:				error = "Message too long";											break;
#endif
#if defined(EPROTOTYPE)
			case EPROTOTYPE:			error = "Protocol wrong type for socket";							break;
#endif
#if defined(ENOPROTOOPT)
			case ENOPROTOOPT:			error = "Protocol not available";									break;
#endif
#if defined(EPROTONOSUPPORT)
			case EPROTONOSUPPORT:		error = "Protocol not supported";									break;
#endif
#if defined(ESOCKTNOSUPPORT)
			case ESOCKTNOSUPPORT:		error = "Socket type not supported";								break;
#endif
#if defined(EOPNOTSUPP)
			case EOPNOTSUPP:			error = "Operation not supported on transport endpoint";			break;
#endif
#if defined(EPFNOSUPPORT)
			case EPFNOSUPPORT:			error = "Protocol family not supported";							break;
#endif
#if defined(EAFNOSUPPORT)
			case EAFNOSUPPORT:			error = "Address family not supported by protocol";					break;
#endif
#if defined(EADDRINUSE)
			case EADDRINUSE:			error = "Address already in use";									break;
#endif
#if defined(EADDRNOTAVAIL)
			case EADDRNOTAVAIL:			error = "Cannot assign requested address";							break;
#endif
#if defined(ENETDOWN)
			case ENETDOWN:				error = "Network is down";											break;
#endif
#if defined(ENETUNREACH)
			case ENETUNREACH:			error = "Network is unreachable";									break;
#endif
#if defined(ENETRESET)
			case ENETRESET:				error = "Network dropped connection because of reset";				break;
#endif
#if defined(ECONNABORTED)
			case ECONNABORTED:			error = "Software caused connection abort";							break;
#endif
#if defined(ECONNRESET)
			case ECONNRESET:			error = "Connection reset by peer";									break;
#endif
#if defined(ENOBUFS)
			case ENOBUFS:				error = "No buffer space available";								break;
#endif
#if defined(EISCONN)
			case EISCONN:				error = "Transport endpoint is already connected";					break;
#endif
#if defined(ENOTCONN)
			case ENOTCONN:				error = "Transport endpoint is not connected";						break;
#endif
#if defined(ESHUTDOWN)
			case ESHUTDOWN:				error = "Cannot send after transport endpoint shutdown";			break;
#endif
#if defined(ETOOMANYREFS)
			case ETOOMANYREFS:			error = "Too many references: cannot splice";						break;
#endif
#if defined(ETIMEDOUT)
			case ETIMEDOUT:				error = "Connection timed out";										break;
#endif
#if defined(ECONNREFUSED)
			case ECONNREFUSED:			error = "Connection refused";										break;
#endif
#if defined(EHOSTDOWN)
			case EHOSTDOWN:				error = "Host is down";												break;
#endif
#if defined(EHOSTUNREACH)
			case EHOSTUNREACH:			error = "No route to host";											break;
#endif
#if defined(EALREADY)
			case EALREADY:				error = "Operation already in progress";							break;
#endif
#if defined(EINPROGRESS)
			case EINPROGRESS:			error = "Operation now in progress";								break;
#endif
#if defined(ESTALE)
			case ESTALE:				error = "Stale NFS file handle";									break;
#endif
#if defined(EUCLEAN)
			case EUCLEAN:				error = "Structure needs cleaning";									break;
#endif
#if defined(ENOTNAM)
			case ENOTNAM:				error = "Not a XENIX named type file";								break;
#endif
#if defined(ENAVAIL)
			case ENAVAIL:				error = "No XENIX semaphores available";							break;
#endif
#if defined(EISNAM)
			case EISNAM:				error = "Is a named type file";										break;
#endif
#if defined(EREMOTEIO)
			case EREMOTEIO:				error = "Remote I/O error";											break;
#endif
#if defined(EDQUOT)
			case EDQUOT:				error = "Quota exceeded";											break;
#endif
#if defined(ENOMEDIUM)
			case ENOMEDIUM:				error = "No medium found";											break;
#endif
#if defined(EMEDIUMTYPE)
			case EMEDIUMTYPE:			error = "Wrong medium type";										break;
#endif
#if defined(ECANCELED)
			case ECANCELED:				error = "Operation Canceled";										break;
#endif
#if defined(ENOKEY)
			case ENOKEY:				error = "Required key not available";								break;
#endif
#if defined(EKEYEXPIRED)
			case EKEYEXPIRED:			error = "Key has expired";											break;
#endif
#if defined(EKEYREVOKED)
			case EKEYREVOKED:			error = "Key has been revoked";										break;
#endif
#if defined(EKEYREJECTED)
			case EKEYREJECTED:			error = "Key was rejected by service";								break;
#endif
#if defined(EOWNERDEAD)
			case EOWNERDEAD:			error = "Owner died";												break;
#endif
#if defined(ENOTRECOVERABLE)
			case ENOTRECOVERABLE:		error = "State not recoverable";									break;
#endif
#if defined(ERESTARTSYS)
			case ERESTARTSYS:			error = "ERESTARTSYS";												break;
#endif
#if defined(ERESTARTNOINTR)
			case ERESTARTNOINTR:		error = "ERESTARTNOINTR";											break;
#endif
#if defined(ERESTARTNOHAND)
			case ERESTARTNOHAND:		error = "restart if no handler..";									break;
#endif
#if defined(ENOIOCTLCMD)
			case ENOIOCTLCMD:			error = "No ioctl command";											break;
#endif
#if defined(ERESTART_RESTARTBLOCK)
			case ERESTART_RESTARTBLOCK:	error = "restart by calling sys_restart_syscall";					break;
#endif
#if defined(EBADHANDLE)
			case EBADHANDLE:			error = "Illegal NFS file handle";									break;
#endif
#if defined(ENOTSYNC)
			case ENOTSYNC:				error = "Update synchronization mismatch";							break;
#endif
#if defined(EBADCOOKIE)
			case EBADCOOKIE:			error = "Cookie is stale";											break;
#endif
#if defined(ENOTSUPP)
			case ENOTSUPP:				error = "Operation is not supported";								break;
#endif
#if defined(ETOOSMALL)
			case ETOOSMALL:				error = "Buffer or request is too small";							break;
#endif
#if defined(ESERVERFAULT)
			case ESERVERFAULT:			error = "An untranslatable error occurred";							break;
#endif
#if defined(EBADTYPE)
			case EBADTYPE:				error = "Type not supported by server";								break;
#endif
#if defined(EJUKEBOX)
			case EJUKEBOX:				error = "Request initiated, but will not complete before timeout";	break;
#endif
#if defined(EIOCBQUEUED)
			case EIOCBQUEUED:			error = "iocb queued, will get completion event";					break;
#endif
#if defined(EIOCBRETRY)
			case EIOCBRETRY:			error = "iocb queued, will trigger a retry";						break;
#endif
#if defined(ECONNCLOSEDBYPEER)
			case ECONNCLOSEDBYPEER:		error = "Connection closed gracefully by peer";						break;
#endif
#if defined(ECONNIOBUG)
			case ECONNIOBUG:			error = "Connio messed up";											break;
#endif
#if defined(ECONNIOMODE)
			case ECONNIOMODE:			error = "Conn is not in a mode that is compatible with request";	break;
#endif
#if defined(ECONNIOINIT)
			case ECONNIOINIT:			error = "ConnIO is not initialized to support this feature";		break;
#endif
#if defined(ECONNIOCBSHORT)
			case ECONNIOCBSHORT:		error = "ConnIOCB returned DONE prematurely";						break;
#endif
#if defined(ECONNIOCBLONG)
			case ECONNIOCBLONG:			error = "ConnIOCB says it took more data than it was given";		break;
#endif
#if defined(ECONNSSLSTART)
			case ECONNSSLSTART:			error = "ConnIO SSL or TLS connection failed to start";				break;
#endif
#if defined(ECONNALREADYSECURED)
			case ECONNALREADYSECURED:	error = "Connection is already secured";							break;
#endif
#if defined(ECONNNOTSECURED)
			case ECONNNOTSECURED:		error = "Connection is NOT secured";								break;
#endif
#if defined(ECONNMOREDATA)
			case ECONNMOREDATA:			error = "ConnIO event needs more data";								break;
#endif
#if defined(EAGAIN)
			case EAGAIN:				error = "Resource unavailable, try again";								break;
#endif
#ifdef DEBUG
			// These three labels are arbitrarily assigned for the purposes of doing
			// unit tests of XplPErrorEx() below.
			case TEST_ERROR_STR_ONLY:
				error = "test error string";	break;
			case TEST_ERROR_DEF_ONLY:
			case TEST_ERROR_UNDEFINED:
			// fall through
#endif
			default:					error = NULL;														break;
		}
	}

	errno = preverrno;
	return(error);
}

EXPORT void XplPErrorEx(char *label, char *file, int line)
{
	if (errno != 0) {
		char	*error			= XplStrError(errno);
		char 	*poundDefine	= XplStrErrorDefine(errno);

		if (poundDefine) {
			if (error) {
				if (label) {
					fprintf(stderr, "%s: (%s) %s\n", label, poundDefine, error);
				} else {
					fprintf(stderr, "%s:%d: (%s) %s\n", file, line, poundDefine, error);
				}
			} else {
				if (label) {
					fprintf(stderr, "%s: (%s) %d - no detail\n", label, poundDefine, errno);
				} else {
					fprintf(stderr, "%s:%d: (%s) %d - no detail\n", file, line, poundDefine, errno);
				}
			}
		} else {
			if (error) {
				if (label) {
					fprintf(stderr, "%s: %s\n", label, error);
				} else {
					fprintf(stderr, "%s:%d: %s\n", file, line, error);
				}
			} else {
				if (label) {
					fprintf(stderr, "%s: %d - unknown error\n", label, errno);
				} else {
					fprintf(stderr, "%s:%d: %d - unknown error\n", file, line, errno);
				}
			}
		}
	}
}

/*! \fn         int XplTranslateError(int error)
    \brief      Map platform specific error codes to the closest ISO C99 error.

                The XplTranslateError interface will map platform specific
                error codes to the best appropriate match within the ISO C99
                error definitions.
    \param[in]  error
    \return     An signed integer denoting the error that occurred.
*/

#if defined(LINUX) || defined(S390RH) || defined(SOLARIS) || defined(MACOSX)
EXPORT int XplTranslateError(int error)
{
	if (error < 0) error = -error;

    errno = error;
    return( error );
}


#elif defined(WIN32)
#include <config.h>

#include <xplutil.h>

/*
    The WinSock error codes range from 10000 to 11999.  The XplTranslateError
    interface attempts to map the specific WinSock error code to the closest
    ISO C99 Standard error definitions.

    The WinSock error codes are defined in winerror.h.  The
    following definitions and explanations of the WinSock error codes are from
    the Microsoft Windows SDK.
*/
EXPORT int XplTranslateError(int error)
{
    int map;

    switch (error) {
        case WSAEINTR: {
            /*! \def    WSAEINTR
                        A blocking operation was interrupted by a call to
                        WSACancelBlockingCall.
            */
            map = EINTR;
            break;
        }

        case WSAEBADF: {
            /*! \def    WSAEBADF
                        The file handle supplied is not valid.
            */
            map = EBADF;
            break;
        }

        case WSAEACCES: {
            /*! \def    WSAEACCES
                        An attempt was made to access a socket in a way
                        forbidden by its access permissions.
            */
            map = EACCES;
            break;
        }

        case WSAEFAULT: {
            /*! \def    WSAEFAULT
                        The system detected an invalid pointer address in
                        attempting to use a pointer argument in a call.
            */
            map = EFAULT;
            break;
        }

        case WSAEINVAL: {
            /*! \def    WSAEINVAL
                        An invalid argument was supplied.
            */
            map = EINVAL;
            break;
        }

        case WSAEMFILE: {
            /*! \def    WSAEMFILE
                        Too many open sockets.
            */
            map = EMFILE;
            break;
        }

        case WSAEWOULDBLOCK: {
            /*! \def    WSAEWOULDBLOCK
                        A non-blocking socket operation could not be completed
                        immediately.
            */
            map = EWOULDBLOCK;
            break;
        }

        case WSAEINPROGRESS: {
            /*! \def    WSAEINPROGRESS
                        A blocking operation is currently executing.
            */
            map = EINPROGRESS;
            break;
        }

        case WSAEALREADY: {
            /*! \def    WSAEALREADY
                        An operation was attempted on a non-blocking socket
                        that already had an operation in progress.
            */
            map = EALREADY;
            break;
        }

        case WSAENOTSOCK: {
            /*! \def    WSAENOTSOCK
                        An operation was attempted on something that is not a
                        socket.
            */
            map = ENOTSOCK;
            break;
        }

        case WSAEDESTADDRREQ: {
            /*! \def    WSAEDESTADDRREQ
                        A required address was omitted from an operation on a
                        socket.
            */
            map = EDESTADDRREQ;
            break;
        }

        case WSAEMSGSIZE: {
            /*! \def    WSAEMSGSIZE
                        A message sent on a datagram socket was larger than
                        the internal message buffer or some other network
                        limit, or the buffer used to receive a datagram into
                        was smaller than the datagram itself.
            */
            map = EMSGSIZE;
            break;
        }

        case WSAEPROTOTYPE: {
            /*! \def    WSAEPROTOTYPE
                        A protocol was specified in the socket function call
                        that does not support the semantics of the socket type
                        requested.
            */
            map = EPROTOTYPE;
            break;
        }

        case WSAENOPROTOOPT: {
            /*! \def    WSAENOPROTOOPT
                        An unknown, invalid, or unsupported option or level was
                        specified in a getsockopt or setsockopt call.
            */
            map = ENOPROTOOPT;
            break;
        }

        case WSAEPROTONOSUPPORT: {
            /*! \def    WSAEPROTONOSUPPORT
                        The requested protocol has not been configured into the
                        system, or no implementation for it exists.
            */
            map = EPROTONOSUPPORT;
            break;
        }

        case WSAESOCKTNOSUPPORT: {
            /*! \def    WSAESOCKTNOSUPPORT
                        The support for the specified socket type does not exist
                        in this address family.
            */
            map = ESOCKTNOSUPPORT;
            break;
        }

        case WSAEOPNOTSUPP: {
            /*! \def    WSAEOPNOTSUPP
                        The attempted operation is not supported for the type
                        of object referenced.
            */
            map = EOPNOTSUPP;
            break;
        }

        case WSAEPFNOSUPPORT: {
            /*! \def    WSAEPFNOSUPPORT
                        The protocol family has not been configured into the
                        system or no implementation for it exists.
            */
            map = EPFNOSUPPORT;
            break;
        }

        case WSAEAFNOSUPPORT: {
            /*! \def    WSAEAFNOSUPPORT
                        An address incompatible with the requested protocol
                        was used.
            */
            map = EAFNOSUPPORT;
            break;
        }

        case WSAEADDRINUSE: {
            /*! \def    WSAEADDRINUSE
                        Only one usage of each socket address
                        (protocol/network address/port) is normally permitted.
            */
            map = EADDRINUSE;
            break;
        }

        case WSAEADDRNOTAVAIL: {
            /*! \def    WSAEADDRNOTAVAIL
                        The requested address is not valid in its context.
            */
            map = EADDRNOTAVAIL;
            break;
        }

        case WSAENETDOWN: {
            /*! \def    WSAENETDOWN
                        A socket operation encountered a dead network.
            */
            map = ENETDOWN;
            break;
        }

        case WSAENETUNREACH: {
            /*! \def    WSAENETUNREACH
                        A socket operation was attempted to an unreachable
                        network.
            */
            map = ENETUNREACH;
            break;
        }

        case WSAENETRESET: {
            /*! \def    WSAENETRESET
                        The connection has been broken due to keep-alive
                        activity detecting a failure while the operation was in
                        progress.
            */
            map = ENETRESET;
            break;
        }

		case ERROR_NETNAME_DELETED:
		case WSAECONNABORTED: {
            /*! \def    WSAECONNABORTED
                        An established connection was aborted by the software
                        in your host machine.
            */
            map = ECONNABORTED;
            break;
        }

        case WSAECONNRESET: {
            /*! \def    WSAECONNRESET
                        An existing connection was forcibly closed by the
                        remote host.
            */
            map = ECONNRESET;
            break;
        }

        case WSA_OPERATION_ABORTED: {
            /*! \def    WSA_OPERATION_ABORTED
                        An overlapped operation was canceled due to the closure
						of the socket, or the execution of the SIO_FLUSH command
						in WSAIoctl.
            */
            map = ECANCELED;
            break;
		}

        case WSAENOBUFS: {
            /*! \def    WSAENOBUFS
                        An operation on a socket could not be performed
                        because the system lacked sufficient buffer space
                        or because a queue was full.
            */
            map = ENOBUFS;
            break;
        }

        case WSAEISCONN: {
            /*! \def    WSAEISCONN
                        A connect request was made on an already connected
                        socket.
            */
            map = EISCONN;
            break;
        }

        case WSAENOTCONN: {
            /*! \def    WSAENOTCONN
                        A request to send or receive data was disallowed
                        because the socket is not connected and (when sending
                        on a datagram socket using a sendto call) no address
                        was supplied.
            */
            map = ENOTCONN;
            break;
        }

        case WSAESHUTDOWN: {
            /*! \def    WSAESHUTDOWN
                        A request to send or receive data was disallowed
                        because the socket had already been shut down in that
                        direction with a previous shutdown call.
            */
            map = ESHUTDOWN;
            break;
        }

        case WSAETOOMANYREFS: {
            /*! \def    WSAETOOMANYREFS
                        Too many references to some kernel object.
            */
            map = ETOOMANYREFS;
            break;
        }

        case WSAETIMEDOUT: {
            /*! \def    WSAETIMEDOUT
                        A connection attempt failed because the connected
                        party did not properly respond after a period of time,
                        or established connection failed because connected
                        host has failed to respond.
            */
            map = ETIMEDOUT;
            break;
        }

        case WSAECONNREFUSED: {
            /*! \def    WSAECONNREFUSED
                        No connection could be made because the target machine
                        actively refused it.
            */
            map = ECONNREFUSED;
            break;
        }

        case WSAELOOP: {
            /*! \def    WSAELOOP
                        Cannot translate name.
            */
            map = ELOOP;
            break;
        }

        case WSAENAMETOOLONG: {
            /*! \def    WSAENAMETOOLONG
                        Name component or name was too long.
            */
            map = ENAMETOOLONG;
            break;
        }

        case WSAEHOSTDOWN: {
            /*! \def    WSAEHOSTDOWN
                        A socket operation failed because the destination host
                        was down.
            */
            map = EHOSTDOWN;
            break;
        }

        case WSAEHOSTUNREACH: {
            /*! \def    WSAEHOSTUNREACH
                        A socket operation was attempted to an unreachable host.
            */
            map = EHOSTUNREACH;
            break;
        }

        case WSAENOTEMPTY: {
            /*! \def    WSAENOTEMPTY
                        Cannot remove a directory that is not empty.
            */
            map = ENOTEMPTY;
            break;
        }

        case WSAEPROCLIM: {
            /*! \def    WSAEPROCLIM
                        A Windows Sockets implementation may have a limit on
                        the number of applications that may use it simultaneously.
            */
            map = ENFILE;
            break;
        }

        case WSAEUSERS: {
            /*! \def    WSAEUSERS
                        Ran out of quota.
            */
            map = EUSERS;
            break;
        }

        case WSAEDQUOT: {
            /*! \def    WSAEDQUOT
                        Ran out of disk quota.
            */
            map = EDQUOT;
            break;
        }

        case WSAESTALE: {
            /*! \def    WSAESTALE
                        File handle reference is no longer available.
            */
            map = ESTALE;
            break;
        }

        case WSAEREMOTE: {
            /*! \def    WSAEREMOTE
                        Item is not available locally.
            */
            map = EREMOTE;
            break;
        }

        case WSAEDISCON: {
            /*! \def    WSAEDISCON
                        Returned by WSARecv or WSARecvFrom to indicate the
                        remote party has initiated a graceful shutdown sequence.
            */
            map = ECONNCLOSEDBYPEER;
            break;
        }

        case WSAENOMORE: {
            /*! \def    WSAENOMORE
                        No more results can be returned by WSALookupServiceNext.
            */
            map = ENOMSG;
            break;
        }

        case WSAECANCELLED: {
            /*! \def    WSAECANCELLED
                        A call to WSALookupServiceEnd was made while this call
                        was still processing. The call has been canceled.
            */
            map = ECANCELED;
            break;
        }

        case 0: {
            map = 0;
            break;
        }

        default: {
            map = ECONNAMBIG;
            break;
        }
    }

    return(map);
}

#elif defined(NETWARE) || defined(LIBC)
int XplTranslateError(int error)
{
	if (error < 0) {
		return(-error);
	} else {
		return(error);
	}
}
#else
# error There is no ISO C99 error mapping interface defined for this platform.
#endif
