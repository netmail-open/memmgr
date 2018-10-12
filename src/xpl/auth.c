#include <memmgr-config.h>
#include <xpl.h>
#include <memmgr.h>


#if defined(LINUX)
#include <sys/syscall.h>

EXPORT char *XplPasswordPolicyTest( char *user, char *pass )
{
	/* The Linux VMs do not have a password policy */
	return NULL;
}

EXPORT char *XplPasswordSet( char *user, char *pass )
{
	char *cmdLine;
	int err;
	char *errMsg = NULL;

	MemAsprintf( &cmdLine, "echo \"%s\" | passwd --stdin %s", pass, user );
	if( !cmdLine ) {
		MemAsprintf( &errMsg, "Failed to set password. Out of Memory!  errno %d", errno );
	} else {
		err = system( cmdLine );
		if( err ) {
			MemAsprintf( &errMsg, "Failed to set password.  errno %d", errno );
		}
		MemFree( cmdLine );
	}
	return errMsg;
}

#elif defined(WIN32)
# include <lm.h>
#pragma comment(lib, "Netapi32.lib")

wchar_t *utf8_dupe_to_utf16( char *str, int *error ) {
	wchar_t *wzStr = NULL;
	int err;
	uint32 bufferLen;

	if( 0 == ( bufferLen = MultiByteToWideChar( CP_UTF8, MB_ERR_INVALID_CHARS, str, -1, wzStr, 0 ) ) ) {	
		err = GetLastError();
	} else if( !( wzStr = ( wchar_t * )MemMalloc( bufferLen * 2 ) ) ) {
		err = GetLastError();
	} else {
		if( !MultiByteToWideChar( CP_UTF8, MB_ERR_INVALID_CHARS, str, -1, wzStr, bufferLen ) ) {
			err = GetLastError();
			MemFree( wzStr );
			wzStr = NULL;
		} else {
			err = 0;
		}
	}

	if( error ) {
		*error = err;
	}

	return wzStr;
}

static char *errorText( DWORD dwLastError )
{
	HMODULE hModule = NULL;
	LPSTR MessageBuffer;
	char *error = NULL;
	DWORD dwBufferLength;
	DWORD dwFormatFlags;

	if( dwLastError == NERR_PasswordTooShort ) {
		error = MemStrdupWait( "The password does not meet the password policy requirements." );
	} else {
		dwFormatFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM;
		if( dwLastError >= NERR_BASE && dwLastError <= MAX_NERR ) {
			hModule = LoadLibraryEx( TEXT( "netmsg.dll" ), NULL, LOAD_LIBRARY_AS_DATAFILE );
			if( hModule != NULL ) {
				dwFormatFlags |= FORMAT_MESSAGE_FROM_HMODULE;
			}
			if( ( dwBufferLength = FormatMessageA( dwFormatFlags, hModule, dwLastError, 0, (LPSTR)&MessageBuffer, 0, NULL ) ) ) {
				error = MemStrdupWait( MessageBuffer );
				LocalFree( MessageBuffer );
			}
			if( hModule ) {
				FreeLibrary( hModule );
			}
		}
	}
	return error;
}

EXPORT char *XplPasswordPolicyTest( char *user, char *pass )
{
	NET_API_STATUS status;
	NET_VALIDATE_PASSWORD_CHANGE_INPUT_ARG InputArg = {0};
	NET_VALIDATE_OUTPUT_ARG *pOutputArg = NULL;
	wchar_t *wzPwd;
	wchar_t *wzUser;
	int errNum;
	char *errMsg = NULL;
	LPCWSTR domainController = NULL;

	NetGetDCName(NULL, NULL, (LPBYTE *) &domainController );
	if( !(wzPwd = utf8_dupe_to_utf16( pass, &errNum ) ) ) {
		MemAsprintf( &errMsg, "Failed to convert the password to unicode. Error: %d", errNum );
	} else {
		if( !(wzUser = utf8_dupe_to_utf16( user, &errNum ) ) ) {
			MemAsprintf( &errMsg, "Failed to convert the user name to unicode. Error: %d", errNum );
		} else {
			InputArg.ClearPassword = wzPwd;
			InputArg.UserAccountName = wzUser;
			InputArg.PasswordMatch = TRUE;

			status = NetValidatePasswordPolicy(domainController, NULL, NetValidatePasswordChange, &InputArg, (void**)&pOutputArg);
			switch( status ) {
			case ERROR_INVALID_PARAMETER:
				errMsg = MemStrdupWait( "Failed to validate password. Invalid arguments." );
				break;
			case ERROR_NOT_ENOUGH_MEMORY:
				errMsg = MemStrdupWait( "Failed to validate password. Out of Memory." );
				break;
			case NERR_Success:
				if( pOutputArg->ValidationStatus ) {
					errMsg = errorText( pOutputArg->ValidationStatus );
				}
				NetValidatePasswordPolicyFree((void **)&pOutputArg);
				break;
			}
			MemFree( wzUser );
		}
		MemFree( wzPwd );
	}
	if( domainController ) {
		NetApiBufferFree( ( LPVOID ) domainController );
	}
	return errMsg;
}

EXPORT char *XplPasswordSet( char *user, char *pass )
{
	NET_API_STATUS nas;
	USER_INFO_1003 userInfo;
	LPCWSTR domainController = NULL;
	wchar_t *wzPwd;
	wchar_t *wzUser;
	int errNum;
	char *errMsg = NULL;

	NetGetDCName(NULL, NULL, (LPBYTE *) &domainController );
	if( !(wzPwd = utf8_dupe_to_utf16( pass, &errNum ) ) ) {
		MemAsprintf( &errMsg, "Failed to convert the password to unicode. Error: %d", errNum );
	} else {
		if( !(wzUser = utf8_dupe_to_utf16( user, &errNum ) ) ) {
			MemAsprintf( &errMsg, "Failed to convert the user name to unicode. Error: %d", errNum );
		} else {
			userInfo.usri1003_password = wzPwd;
            /* NM-6047 Try with local computer only */
			if( NERR_Success != ( nas = NetUserSetInfo( NULL /*domainController*/, wzUser, 1003, (LPBYTE ) &userInfo, NULL ) ) ) {
                errMsg = errorText( nas );
			}
			MemFree( wzUser );
		}
		MemFree( wzPwd );
	}
	if( domainController ) {
		NetApiBufferFree( ( LPVOID ) domainController );
	}
	return errMsg;
}

#elif defined(MACOSX)

EXPORT char *XplPasswordPolicyTest( char *user, char *pass )
{
	/* No password policy implemented on this platform */
	return MemStrdup( "No password policy implemented on this platform" );
}

EXPORT char *XplPasswordSet( char *user, char *pass )
{
	/* not implemented on this platform */
	return MemStrdup( "XplPasswordSet() not implemented on this platform" );
}

#else

EXPORT char *XplPasswordPolicyTest( char *user, char *pass )
{
	/* No password policy implemented on this platform */
	return MemStrdup( "No password policy implemented on this platform" );
}

EXPORT char *XplPasswordSet( char *user, char *pass )
{
	/* not implemented on this platform */
	return MemStrdup( "XplPasswordSet() not implemented on this platform" );
}

#endif
