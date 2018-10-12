/****************************************************************************
 *
 * Copyright (c) 1999-2002 Novell, Inc.
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

#ifndef _HULAUTIL_H
#define _HULAUTIL_H

#include <stddef.h>
#include <xpl.h>

#define	NMAP_HASH_SIZE 128
#define NMAP_CRED_STORE_SIZE 512
#define NMAP_CRED_DIGEST_SIZE 16
#define NMAP_CRED_CHUNK_SIZE ((NMAP_CRED_STORE_SIZE * NMAP_CRED_DIGEST_SIZE) / NMAP_HASH_SIZE)

#ifndef offsetof
#define offsetof(s, m) ((size_t) &((s *)0)->m)
#endif

enum CommandColors {
    RedCommand = 0,
    BlackCommand,

    MaxCommandColors
};

typedef int (* CommandHandler)(void *param);

typedef struct _ProtocolCommand {
    unsigned char *name;
    unsigned char *help;

    int length;

    CommandHandler handler;

    struct _ProtocolCommand *parent;
    struct _ProtocolCommand *left;
    struct _ProtocolCommand *right;

    enum CommandColors color;
} ProtocolCommand;

typedef struct _ProtocolCommandTree {
    ProtocolCommand *sentinel;
    ProtocolCommand *root;

    XplAtomic nodes;

    ProtocolCommand proxy;
} ProtocolCommandTree;

extern const char *Base64Chars;

/***** Begin HulaKeyword *****/
#define HULA_KEYWORD_VALUES_PER_BYTE 256
#define HULA_KEYWORD_BITS_PER_BYTE 8
#define HULA_KEYWORD_BITS_PER_WORD (sizeof(unsigned long) * HULA_KEYWORD_BITS_PER_BYTE)
#define HULA_KEYWORD_FULL_MASK (unsigned long)(-1)
#define HULA_KEYWORD_HIGH_BIT_MASK ((unsigned long)(1) << (HULA_KEYWORD_BITS_PER_WORD - 1))
/* HULA_KEYWORD_BBPW (bits required to store the number of bits in a word) */
/* could be calculated as */
/* (int BBPW) */
/* after calling frexp(BITS_PER_WORD, &BBPW) */
/* but it would end up getting calculated at run time */
#define HULA_KEYWORD_BBPW ((HULA_KEYWORD_BITS_PER_WORD < 33) ? 5 : (HULA_KEYWORD_BITS_PER_WORD < 65) ? 6 : (HULA_KEYWORD_BITS_PER_WORD < 129) ? 7 : (HULA_KEYWORD_BITS_PER_WORD < 257) ? 8 : 9)
/* HULA_KEYWORD_BTIM is the number of bits in the HULA_KEYWORD_TABLE_INDEX_MASK */
#define HULA_KEYWORD_BTIM ((HULA_KEYWORD_BITS_PER_WORD - 1 - HULA_KEYWORD_BBPW) >> 1)
#define HULA_KEYWORD_TABLE_INDEX_MASK (HULA_KEYWORD_FULL_MASK >> (HULA_KEYWORD_BITS_PER_WORD - HULA_KEYWORD_BTIM))
#define HULA_KEYWORD_STRING_INDEX_MASK  (~(HULA_KEYWORD_HIGH_BIT_MASK | HULA_KEYWORD_TABLE_INDEX_MASK))

typedef unsigned long HulaKeywordTable[HULA_KEYWORD_VALUES_PER_BYTE];

typedef struct {
    HulaKeywordTable *table;
    unsigned long *sortedList;
} HulaKeywordIndex;

#ifdef __cplusplus
extern "C" {
#endif

EXPORT HulaKeywordIndex *HulaKeywordIndexCreate(unsigned char **stringList, XplBool makeCaseInsensitive);
EXPORT void HulaKeywordIndexFree(HulaKeywordIndex *Index);
EXPORT long HulaKeywordFind(HulaKeywordIndex *keywordTable, unsigned char *searchString);
/***** End HulaKeyword *****/

EXPORT void LoadProtocolCommandTree(ProtocolCommandTree *tree, ProtocolCommand *commands);
EXPORT void AddToProtocolCommandTree(ProtocolCommandTree *tree, ProtocolCommand *commands);
EXPORT ProtocolCommand *ProtocolCommandTreeSearch(ProtocolCommandTree *tree, const unsigned char *command);

EXPORT XplBool QuickNCmp(unsigned char *str1, unsigned char *str2, int len);
EXPORT XplBool QuickCmp(unsigned char *str1, unsigned char *str2);

#define HulaStrNCpy(dest, src, len) {size_t copylen = strlen(src); if (copylen >= (len)) {copylen = (len)-1;} memcpy((dest), (src), copylen); (dest)[copylen] = '\0';}

EXPORT char * DecodeBase64(char *EncodedString);
EXPORT size_t DecodeBase64Ex(char *EncodedString, size_t length);

EXPORT char * UrlDecode(char *UrlEncodedString);

#define ENCODE_BASE64_FLAG_NO_CRLF				(1<<0) // do not add CRLF to buffer
#define ENCODE_BASE64_FLAG_CALC_ENCD_SIZE_ONLY	(1<<1) // return the needed buffer SIZE  to malloc EncodeBase64Ex ONLY

/*
EncodeBase64Ex - Base64 Encoding function returns TRUE on success and has the following
	parameters:
unencodedBuffer 			IN	- a pointer to the beginning of a block of data to be encoded.
byteCountToBeEncoded		IN	- the number of bytes to be encoded from the block identified.
outputBuffer				IN/OUT	- address of a char * which may be NULL or point to
								  a previously allocated buffer. If NULL then the function
								  will replace the NULL with the address of an allocated
								  buffer containing the encoded result.
outputBufferSize			IN		- size of the output buffer that was already allocated. If
								outputBuffer is intended to receive the address of an
								allocated buffer use zero.
optionalEncodedSize 		OUT		- receives the size of the encoded data, including the
								terminating '\0' character. It is NOT optional if the flags
								parameter includes the
								ENCODE_BASE64_FLAG_CALC_ENCD_SIZE_ONLY bit or if
								the outputBufferSize is less than the actual size necessary
								to encode the original buffer.
flags						IN		- used to control behavior. Flags are explained below.
	ENCODE_BASE64_FLAG_CALC_ENCD_SIZE_ONLY	- calculate the needed buffer size. Only
								the following parameters are used:
								byteCountToBeEncoded,  optionalEncodedSize, and flags.
	ENCODE_BASE64_FLAG_NO_CRLF	- omit addition of CRLF pairs every 76 bytes of
								output. This flag is also passed through from the
								EncodeBase64Simple() function.
*/
#define EncodeBase64Ex(_unencodedBuffer,_byteCountToBeEncoded,_outputBuffer,\
		_outputBufferSize,_optionalEncodedSize,_flags)\
		EncodeBase64Ex_fl(_unencodedBuffer,_byteCountToBeEncoded,_outputBuffer,\
		_outputBufferSize,_optionalEncodedSize,_flags,__FILE__,__LINE__)
EXPORT XplBool EncodeBase64Ex_fl(
	const unsigned char *unencodedBuffer,
	size_t byteCountToBeEncoded,
	unsigned char **outputBuffer,
	size_t outputBufferSize,
	size_t *optionalEncodedSize,
	unsigned int flags,
	const char *file,
	const int line);

#define EncodeBase64(s) \
	EncodeBase64Simple_fl((s), strlen((s)), NULL, 0, __FILE__, __LINE__)
#define EncodeBase64WithLen(_dataBuffer,_dataLen,_optionalEncodedLen,_flags) \
	EncodeBase64Simple_fl((_dataBuffer), (_dataLen), (_optionalEncodedLen),\
	(_flags),__FILE__, __LINE__)
EXPORT unsigned char *EncodeBase64Simple_fl(
	const unsigned char *UnencodedString,
	size_t length,
	size_t *optionalEncodedLength,
	unsigned int flags,
	const char *file,
	const int line);

EXPORT XplBool HashCredential(const unsigned char *DN, unsigned char *Credential, unsigned char *Hash);

#ifdef __cplusplus
}
#endif

/* Value Lists */
typedef struct {
	unsigned char					**Value;
	unsigned long					Used;
	unsigned long					Allocated;
} ListValueStruct;

#define LIST_VALUE_ALLOC_SIZE       1024

#ifdef __cplusplus
extern "C" {
#endif

EXPORT ListValueStruct * ListCreateValueStructEx(const char *file, unsigned long line);
#define ListCreateValueStruct()	ListCreateValueStructEx(__FILE__,__LINE__)
EXPORT XplBool ListDestroyValueStruct(ListValueStruct *LV);
EXPORT XplBool ListAddValue(const unsigned char *Value, ListValueStruct *LV);
EXPORT XplBool ListFreeValue(unsigned long Index, ListValueStruct *LV);
EXPORT XplBool ListFreeValues(ListValueStruct *LV);

#ifdef __cplusplus
}
#endif


// Lists
typedef struct List
{
	struct List	*next;
	void		*element;
}List;

#ifdef __cplusplus
extern "C" {
#endif

EXPORT int ListFIFOSort( void *s1, void *s2 );
EXPORT int ListLIFOSort( void *s1, void *s2 );

#define ListAdd( l, e, u )			ListAddEx( (l), (e), (u), NULL, __FILE__, __LINE__ )
EXPORT int ListAddEx( List **list, void *element, XplBool unique, int (*sortCB)( void *e1, void *e2 ), const char *file, unsigned long line );
EXPORT int ListAddEx_1( List **list, void *element, XplBool unique, int (*sortCB)( void *e1, void *e2 ), void (*freeCB)( void *e), const char *file, unsigned long line );

#define ListAddString( l, s, u, f )	ListAddStringEx( (l), (s), (u), TRUE, (f), __FILE__, __LINE__ )
#define ListAddIString( l, s, u, f )	ListAddStringEx( (l), (s), (u), FALSE, (f), __FILE__, __LINE__ )
EXPORT int ListAddStringEx( List **list, char *string, XplBool unique, XplBool caseSensative, XplBool canFree, const char *file, unsigned long line );

EXPORT List* ListRemove( List **list, void *element );
EXPORT void ListAppend( List **dest, List **source );
#define ListFree( l, f )	ListFreeEx( (l), (f), __FILE__, __LINE__ )
EXPORT void ListFreeEx( List **list, void (*freeElementCB)( void *element ), const char *file, unsigned long line );
EXPORT void *ListFind( List *list, void *element );
EXPORT List *ListNext( List *list );
EXPORT void *ListSearch( List *list, void *previous, int (*matchCB)( void *element, void *client ), void *client );
EXPORT int ListEnumerate( List *list, int (*enumCB)( List *, void * ), void *client );
EXPORT char *EncodeB64( unsigned char *data, size_t length );
EXPORT int EncodeB64Ex( unsigned char *data, size_t length, char *buffer, size_t size );
EXPORT char *EncodeFS64( unsigned char *data, size_t length );
EXPORT int EncodeFS64Ex( unsigned char *data, size_t length, char *buffer, size_t size );
EXPORT char *EncodeURI64( unsigned char *data, size_t length );
EXPORT int EncodeURI64Ex( unsigned char *data, size_t length, char *buffer, size_t size );
EXPORT int DecodeB64Ex( char *src, size_t srcLen, char **dest, size_t *length );
EXPORT int DecodeB64( char *src, char **dest, size_t *length );

EXPORT char ** LoadDelimitedList(char *list, char seperator, XplBool include, XplBool terminate);

#ifdef __cplusplus
}
#endif

// GUID
typedef struct
{
	XplLock		lock;
	struct
	{
		void	*data;
		size_t	length;
	}machine;
	char		*machineStamp;
	time_t		timeStamp;
	uint32		counter;
}GUIDCounter;

#ifdef __cplusplus
extern "C" {
#endif

EXPORT GUIDCounter *AllocateGUIDCounter( void *machineData, size_t length );
EXPORT void ReleaseGUIDCounter( GUIDCounter **g );
EXPORT int GenerateGUID( GUIDCounter *g, char *buffer, size_t length );
EXPORT void CleanAndCreatePath(char *path);
EXPORT void CreatePath(char *path);

#ifdef __cplusplus
}
#endif

#endif /* _HULAUTIL_H */

