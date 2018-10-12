#include <memmgr-config.h>
#include <xpl.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#if !defined (_MSC_VER)
EXPORT char *strlwr( char *s )
{
	char *p;

	for(p=s;*p;p++)
	{
		*p = tolower( *p );
	}
	return s;
}

EXPORT char *strupr( char *s )
{
	char *p;

	for(p=s;*p;p++)
	{
		*p = toupper( *p );
	}
	return s;
}
#endif

EXPORT char * strichr(const char *haystack, const char needle)
{
    unsigned char c = toupper((unsigned char) needle);

    if (haystack) {
        const unsigned char *ptr;

        for (ptr = haystack; *ptr != '\0'; ptr++) {
            if (c == toupper((unsigned char) *ptr)) {
                return((char *) ptr);
            }
        }
    }

    return(NULL);
}

EXPORT char * strstrn(const char *haystack, const char *needle, size_t len)
{
    if (haystack && needle && *haystack && *needle) {
        char   *ptr;

        for (ptr = strichr(haystack, *needle); ptr; ptr = strichr(ptr + 1, *needle)) {
            if (!strncmp(ptr, needle, len)) {
                return(ptr);
            }
        }
    }

    return(NULL);
}

EXPORT char * stristrn(const char *haystack, const char *needle, size_t len)
{
    if (haystack && needle && *haystack && *needle) {
        char   *ptr;

        for (ptr = strichr(haystack, *needle); ptr; ptr = strichr(ptr + 1, *needle)) {
            if (!strnicmp(ptr, needle, len)) {
                return(ptr);
            }
        }
    }

    return(NULL);
}

// returns pointer to needle in haystack if found in len first bytes of haystack.
EXPORT char *strnistr(const char *haystack, const char *needle, size_t len)
{
		int i;
		size_t needle_len;

		if (haystack && needle && *haystack && *needle) {
			if (0 == (needle_len = strlen(needle)))
					return (char *)haystack;

			for (i=0; i<=(int)(len-needle_len); i++)
			{
					if ((haystack[0] == needle[0]) &&
							(0 == strnicmp(haystack, needle, needle_len)))
							return (char *)haystack;
					haystack++;
			}
		}
		return NULL;
}

EXPORT char * strspace( const char *source )
{
	while( source && *source )
	{
		if( isspace( *source ) )
		{
			return((char *) source);
		}
		source++;
	}
	return NULL;
}

EXPORT char * strrspace( const char *source )
{
	ssize_t		len;

	if (!source || 0 == (len = strlen(source))) {
		return(NULL);
	}

	for (len--; len >= 0; len--) {
		if (isspace(source[len])) {
			return((char *) (source  + len));
		}
	}
	return(NULL);
}

EXPORT char * _skipspace( const char *source, const char *breakchars )
{
	if( source )
	{
		while( *source && isspace( *source ) )
		{
#if 0
			DebugAssert(*source != '\r');
			DebugAssert(*source != '\n');
#endif
			if (breakchars && strchr(breakchars, *source)) {
				break;
			}

			source++;
		}
	}
	return((char *) source);
}

EXPORT char *chopspace( char *value )
{
	char	*p;

	if( value )
	{
		for(p=value+strlen(value);p>value;p--)
		{
			if( !isspace( *(p-1) ) )
			{
				break;
			}
		}
		*p = '\0';
	}
	return( skipspace( value ) );
}

EXPORT int namevalue(char *line, char **name, char **value)
{
	if (!name || !value || !line) {
		return(-(errno = EINVAL));
	}

	*name	= line;
	*value	= strchrs(line, ":=");

	if (!*value) {
		return(-(errno = ENOENT));
	}

	*(*value)	= '\0';
	*name		= chopspace(*name);
	*value		= chopspace((*value) + 1);

	if (!*name || !*(*name)) {
		return(-(errno = ENOENT));
	}

	if (!*value || !*(*value)) {
		return(-(errno = ENOENT));
	}

	return((errno = 0));
}

EXPORT char *removespace( char *source, char *dest, int destsize )
{
	int i = 0;
	char *s = source, *d = dest;

	if( NULL == source || NULL == dest || destsize <= 0 ) {
		return NULL;
	}

	while( i<destsize && *s ) {
		if( !isspace(*s) ) {
			*d++ = *s;
			i++;
		}
		s++;
	}

	*d = '\0';

	return dest;
}

EXPORT char * nextfieldex(char *value, char **end, XplBool stripquotes, char escape)
{
	char		*s;
	char		quote	= '\0';

	if (end) {
		*end = NULL;
	}

	if (!value) {
		return(NULL);
	}

	/*
		If the first character is a quote character then force strip quotes mode
		regardless of what options where passed in.
	*/
	switch (*value) {
		case '"': case '\'':
			stripquotes = TRUE;
			break;

		default:
			break;
	}

	s = value = skipspace(value);
	while (s) {
		/* Handle escape characters */
		if (escape && escape == *s) {
			memmove(s, s + 1, strlen(s));
			switch (*s) {
				case 'a':	*s = '\a';	break;
				case 'b':	*s = '\b';	break;
				case 'f':	*s = '\f';	break;
				case 'n':	*s = '\n';	break;
				case 'r':	*s = '\r';	break;
				case 't':	*s = '\t';	break;
				case 'v':	*s = '\v';	break;

				case '0':
					/* Eat null characters */
					memmove(s, s + 1, strlen(s));
					break;

				default:
					break;
			}
			s++;
			continue;
		}

		switch (*s) {
			case '\0':
			case '\r':
			case '\n':
				*s = '\0';
				return(*value ? value : NULL);

			case '"':
			case '\'':
				if (quote == '\0' || quote == *s) {
					if (quote == *s) {
						quote = '\0';
					} else {
						quote = *s;
					}

					if (stripquotes) {
						memmove(s, s + 1, strlen(s));
					} else {
						s++;
					}
					break;
				} else {
					/* fallthrough */
				}

			default:
				if (isspace(*s) && !quote) {
					*s = '\0';
					if (end && s[1]) {
						*end = s + 1;
					}
					return(*value ? value : NULL);
				}
				s++;
				break;
		}
	}

	return(NULL);
}

/*
	Find the next argument but do NOT strip quotes

	This version will work in cases where quotes may be part of the argument
	without being escaped. This is the case in many of our cli tools.
*/
EXPORT char * nextfield(char *value, char **end)
{
	return(nextfieldex(value, end, FALSE, '\\'));
}

/* Find the next argument and DO strip quotes */
EXPORT char * nextargument(char *value, char **end)
{
#ifdef WIN32
	return(nextfieldex(value, end, TRUE, '\0'));
#else
	return(nextfieldex(value, end, TRUE, '\\'));
#endif
}

static int strpat_r(const char *str, const char *pattern, size_t len, int depth)
{
	const char	*next;
	const char	*match;
	const char	*end = (char *) pattern + len;

	if (!str || !pattern || depth > 15) {
		return(-1);
	}

	for (;;) {
		if (pattern >= end || !*pattern) {
			if (!*str) {
				return(0);
			} else {
				return(-1);
			}
		}

		switch (*pattern) {
			case '?':
				/* A ? must match a single character */
				if (*str) {
					str++;
					pattern++;
				} else {
					return(-1);
				}

                break;

			case '*':
				/*
					A * may match 0 or more characters.

					Find the next wild card in the pattern, and then search for
					the exact value between the two wild cards in the string.

					Example, if pattern points to "*abc*" then "abc" must be
					found in the string.
				*/
				pattern++;

				for (next = pattern; *next && next < end && *next != '*' && *next != '?'; next++);
				if (next > pattern) {
					match = str - 1;
					while ((match = strstrn(match + 1, pattern, next - pattern))) {
						if (!strpat_r(match + (next - pattern), next, end - next, depth + 1)) {
							return(0);
						}
					}

					return(-1);
				} else if (pattern >= end || !*pattern) {
					/* The last character of pattern is a *, so we match. */
					return(0);
				}

				break;

			default:
				if (isspace(*pattern)) {
					/* Any whitespace matches any whitespace */
					if (isspace(*str)) {
						for (str++; isspace(*str); str++);
						for (pattern++; *pattern && pattern < end && isspace(*pattern); pattern++);
					} else {
						return(-1);
					}
				} else if (*pattern == *str) {
					pattern++;
					str++;
				} else {
					return(-1);
				}

				break;
		}
	}
}

static int stripat_r(const char *str, const char *pattern, size_t len, int depth)
{
	const char	*next;
	const char	*match;
	const char	*end = (char *) pattern + len;

	if (!str || !pattern || depth > 15) {
		return(-1);
	}

	for (;;) {
		if (pattern >= end || !*pattern) {
			if (!*str) {
				return(0);
			} else {
				return(-1);
			}
		}

		switch (*pattern) {
			case '?':
				/* A ? must match a single character */
				if (*str) {
					str++;
					pattern++;
				} else {
					return(-1);
				}

                break;

			case '*':
				/*
					A * may match 0 or more characters.

					Find the next wild card in the pattern, and then search for
					the exact value between the two wild cards in the string.

					Example, if pattern points to "*abc*" then "abc" must be
					found in the string.
				*/
				pattern++;

				for (next = pattern; *next && next < end && *next != '*' && *next != '?'; next++);
				if (next > pattern) {
					match = str - 1;
					while ((match = stristrn(match + 1, pattern, next - pattern))) {
						if (!stripat_r(match + (next - pattern), next, end - next, depth + 1)) {
							return(0);
						}
					}

					return(-1);
				} else if (pattern >= end || !*pattern) {
					/* The last character of pattern is a *, so we match. */
					return(0);
				}

				break;

			default:
				if (isspace(*pattern)) {
					/* Any whitespace matches any whitespace */
					if (isspace(*str)) {
						for (str++; isspace(*str); str++);
						for (pattern++; *pattern && pattern < end && isspace(*pattern); pattern++);
					} else {
						return(-1);
					}
				} else if (toupper(*pattern) == toupper(*str)) {
					pattern++;
					str++;
				} else {
					return(-1);
				}

				break;
		}
	}
}

EXPORT int strpat(const char *str, const char *pattern)
{
	if (!pattern) return(-1);

	return(strpat_r(str, pattern, strlen(pattern), 1));
}

EXPORT int strpatn(const char *str, const char *pattern, size_t len)
{
	if (!pattern) return(-1);

	return(strpat_r(str, pattern, len, 1));
}

EXPORT int stripat(const char *str, const char *pattern)
{
	if (!pattern) return(-1);

	return(stripat_r(str, pattern, strlen(pattern), 1));
}

EXPORT int stripatn(const char *str, const char *pattern, size_t len)
{
	if (!pattern) return(-1);

	return(stripat_r(str, pattern, len, 1));
}

EXPORT int _XplDebugPrintf(char *format, ...) {
	va_list varargs;
	int res;
	va_start(varargs, format);
	res = vfprintf(stderr, format, varargs);
	va_end(varargs);
	return res;
}

EXPORT size_t vstrcatf( char *buffer, size_t bufferSize, size_t *sizeNeeded, const char *format, va_list args )
{
	int		needed;				/* vsnprintf is prototyped as returning int */
	char	*start = NULL;		/* where to write if non-NULL */
	size_t	used = 0;			/* space consumed in buffer */
	size_t	space = 0;			/* space free in buffer */
	size_t	ret = 0;

								/* have buffer with a NUL inside it? */
	if(buffer && bufferSize && (start = memchr(buffer, '\0', bufferSize))) {
		used = start - buffer;	/* yes, number of used bytes */
								/* available space in buffer */
		space = bufferSize - used;
	}
#ifdef _MSC_VER
	needed = _vsnprintf(start, space, format, args);
	if (-1==needed) {
		needed = _vscprintf(format, args);
	}
#else
	needed = vsnprintf(start, space, format, args);
#endif
	if(needed < 0) {
		DebugAssert(0);			// output error from vsnprintf
	}

	/* vsnprintf does not include the \0 terminator character in it's count */
	needed++;

	if (start) {				/* have place to write? */
		if (needed > space) {
			DebugAssert(sizeNeeded);// buffer overflow and size return not provided
			*start = '\0';		/* tie off buffer as if vsnprintf never happened */
		} else {
			/*
				The return value is the number of characters written NOT
				including the terminator character.

				This is required to be consistent with the printf family of
				functions.
			*/
			ret = needed - 1;	/* success, return bytes written */
		}
	} else if (buffer) {		/* no place to write, did we have a buffer? */
		DebugAssert(0);			// no terminating NUL found in buffer
		// This may be a bug, as it will delete a byte and will return a size
		// count that is short by 1.  However, we already overflowed the buffer
		// if we got here, so there isn't much hope of goodness anyway.  If it
		// happens we need to fix the underlying bug, not try to recover here.
		used = bufferSize - 1;	/* make space for one byte */
		buffer[used] = '\0';	/* put a NUL there */
	}

	if (sizeNeeded) {			/* need to return size? */
		*sizeNeeded = used + needed;
	}

	return ret;
}

EXPORT size_t vstrprintf( char *buffer, size_t bufferSize, size_t *sizeNeeded, const char *format, va_list args )
{
	if( buffer && bufferSize )
	{
		*buffer = '\0';
	}
	return vstrcatf( buffer, bufferSize, sizeNeeded, format, args );
}

EXPORT size_t strprintf( char *buffer, size_t bufferSize, size_t *sizeNeeded, const char *format, ... )
{
	size_t	needed;
	va_list	args;

	if( buffer && bufferSize )
	{
		*buffer = '\0';
	}
	va_start( args, format );
	needed = vstrcatf( buffer, bufferSize, sizeNeeded, format, args );
	va_end( args );
	return needed;
}

EXPORT size_t strcatf( char *buffer, size_t bufferSize, size_t *sizeNeeded, const char *format, ... )
{
	size_t	needed;
	va_list	args;

	va_start( args, format );
	needed = vstrcatf( buffer, bufferSize, sizeNeeded, format, args );
	va_end( args );
	return needed;
}

EXPORT time_t strtotime( char *str, char **end, int radix )
{
	char			*p;
	unsigned long	seconds;

	seconds = strtoul( str, &p, radix );
	switch( *p )
	{
		case 'w':	// weeks
		case 'W':
			seconds *= 7;
			// fall
		case 'd':	// days
		case 'D':
			seconds *= 24;
			// fall
		case 'h':	// hours
		case 'H':
			seconds *= 60;
			// fall
		case 'm':	// minutes
		case 'M':
			seconds *= 60;
			// fall
		case 's':	// seconds
		case 'S':
			p++;
			break;

		default:
			seconds = 0;
	}
	if( end )
	{
		*end = p;
	}
	return (time_t)seconds;
}

EXPORT uint64 strtobytes( char *str, char **end, int radix )
{
	char			*p;
	uint64			bytes;

	bytes = strtoull( str, &p, radix );
	switch( *p )
	{
		case 't':	// terabytes
		case 'T':
			bytes *= 1024;
			// fall
		case 'g':	// gigabytes
		case 'G':
			bytes *= 1024;
			// fall
		case 'm':	// megabytes
		case 'M':
			bytes *= 1024;
			// fall
		case 'k':	// kilobytes
		case 'K':
			bytes *= 1024;
			// fall
			p++;
		case 'b':	// bytes
		case 'B':
		case '\0':
			break;

		default:
			bytes = 0;
	}
	if( 'b' == *p || 'B' == *p )
	{
		p++;
	}

	if( end )
	{
		*end = p;
	}
	return bytes;
}

EXPORT char *strchrs( char *str, char *chr )
{
	char	*c, *p, *s;

	if( str && chr )
	{
		s = NULL;
		for(c=chr;*c;c++)
		{
			if( p = strchr( str, *c ) )
			{
				if( !s || ( p < s ) )
				{
					s = p;
				}
			}
		}
		return s;
	}
	return NULL;
}

EXPORT char *strrchrs( char *str, char *chr )
{
	char	*c, *p, *s;

	if( str && chr )
	{
		s = NULL;
		for(c=chr;*c;c++)
		{
			if( p = strrchr( str, *c ) )
			{
				if( !s || ( p > s ) )
				{
					s = p;
				}
			}
		}
		return s;
	}
	return NULL;
}

EXPORT char *_strtok_r_( char *s1, const char *s2, char **s3 )
{
	char	*p;

	if( !s1 )
	{
		s1 = *s3;
	}
	if( p = strchrs( s1, (char *)s2 ) )
	{
		*p = '\0';
		p += strlen( s2 );
	}
	*s3 = p;
	return s1;
}

