/****************************************************************************
 *
 * Copyright (c) 1997-2004 Novell, Inc.
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

#ifndef _HULA_XPLERR_H
#define _HULA_XPLERR_H
#include <memmgr-config.h>
//#if !defined(_MSC_VER)
#include <errno.h>
//#endif
//#include <xplutil.h>

/*! \fn         int TranslateError(int error)
    \brief      Translate WinSock error codes into their closest errno match.

                The WinSock error codes range from 10000 to 11999.  The
                TranslateError interface attempts to map the specific WinSock
                error code to the closest ISO C99 Standard error definitions.

                The WinSock error codes are defined in winsock.h and winsock2.h.
                The following definitions and explanations of the WinSock
                error codes are from the Microsoft Windows SDK.

    \param[in]  error
    \return     An signed integer denoting the error that occurred.
*/

#ifdef __cplusplus
extern "C" {
#endif

EXPORT char * XplStrError(int errnum);
EXPORT char * XplStrErrorDefine(int errnum); // returns the corresponding #define name or NULL

EXPORT void XplPErrorEx(char *label, char *file, int line);
#define XplPError(label) XplPErrorEx((label), __FILE__, __LINE__)

#if defined(DEBUG)
	#define XplDebugPError(label) XplPErrorEx((label), __FILE__, __LINE__)
#else
	#define XplDebugPError(label)
#endif

EXPORT int XplTranslateError(int error);


#ifdef __cplusplus
}
#endif

#endif /* _HULA_XPLERR_H */
