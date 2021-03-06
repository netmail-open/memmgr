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

#ifndef MEMMGR_H
#define MEMMGR_H

#include "xpl.h"

#if (!defined(MEMMGR_TYPE_NONE) || defined(MEMMGR_TYPE_GUARD) || defined(MEMMGR_TYPE_SLABBER))
# define MEMMGR_TYPE_SLABBER	1
#endif

#ifdef MEMMGR_TYPE_SLABBER

#define MEMMGR_SIG_REDZ			0x7a446552
#define MEMMGR_SIG_ONE			0x2e456e4f
#define MEMMGR_SIG_FREE			0x65457246
#define MEMMGR_SIG_ZONE			0x654e6f5a
#define MEMMGR_SIG_WATCH		0x63546157
#define MEMMGR_SIG_ING  		0x674e6948


// Advice for "type of memory" being allocated

typedef enum {
	MEMMGR_ADVICE_SMALL,
	MEMMGR_ADVICE_NORMAL,
	MEMMGR_ADVICE_LARGE
} MemMgrAdvice;

// either size or nodes need to be zero
// the one that is zero will be calculated from pages and the other one
typedef struct
{
	char			*name;	// name of pool
	int				pages;	// physical pages per slab
	int				size;	// node size (0 means calculate from number of nodes)
	int				nodes;	// number of nodes (0 means calculate from node size)
	int				depth;	// number of slabs before the leak detection kicks in
	MemMgrAdvice	advice;	// system advice, tells the OS what kind of page backing might be needed
}MemPoolInfo;

typedef struct
{
	MemPoolInfo	info32;
	MemPoolInfo info64;
	MemPoolInfo info128;
	MemPoolInfo info256;
	MemPoolInfo info512;
	MemPoolInfo info1k;
	MemPoolInfo info2k;
	MemPoolInfo info4k;
	MemPoolInfo info8k;
	MemPoolInfo info16k;
	MemPoolInfo info32k;
	MemPoolInfo info64k;
	MemPoolInfo info128k;
	MemPoolInfo info256k;
	MemPoolInfo info512k;
	MemPoolInfo info1m;
}MemPoolConfig;

#ifndef MEM_SIG_SIZE
#define MEM_SIG_SIZE	1
#endif

/* #ifdef DEBUG_ASSERT */
typedef struct
{
	union
	{
		struct
		{
			uint32		redz;
			uint32		one;
		};
		char			c[8];
	} data[MEM_SIG_SIZE];
}_MemSig;
/* #endif */

typedef struct
{
	const char			*file;
	uint32				line;
}_MemInfo;

// must mirror _MemNode in slab.h
typedef struct _StackNode
{
	union
	{
		// stack nodes will have this set to NULL
		void			*slab;
	};
	union
	{
		// stack nodes will have this set to NULL
		void			*pool;
	};
	size_t				size;
/*
  FIXME:
  having these extras only exist for debug causes build errors on the
  Asserts that check them when DEBUG/DEBUG_ASSERT are not defined.
  for now, keeping this extra structure makes the build work and lets
  the consumer not have to define DEBUG.
 */
/* #if defined(DEBUG)||defined(DEBUG_ASSERT) */
	_MemInfo			allocInfo;
	uint32				key;
/*
#endif
#ifdef DEBUG_ASSERT
*/
	_MemInfo			freeInfo;
	_MemSig				sig;
/* #endif */
	char				data[];
}_StackNode;


#if defined(DEBUG)||defined(DEBUG_ASSERT)
#define _SetAlloc( _node_, _file_, _line_ )     (_node_)->allocInfo.file = (_file_);			\
												(_node_)->allocInfo.line = (_line_);
#else
#define _SetAlloc( _node_, _file_, _line_ )
#endif

#ifdef DEBUG_ASSERT
#define _SetSig( _node_ )																		\
{																								\
	int sigindex;																				\
	for (sigindex = 0; sigindex < MEM_SIG_SIZE; sigindex++) {									\
		(_node_)->sig.data[sigindex].redz = MEMMGR_SIG_REDZ;									\
		(_node_)->sig.data[sigindex].one  = MEMMGR_SIG_ONE;										\
	}																							\
}

#define _SetSigF( _node_ )																		\
{																								\
	int sigindex;																				\
	for (sigindex = 0; sigindex < MEM_SIG_SIZE; sigindex++) {									\
		(_node_)->sig.data[sigindex].redz = MEMMGR_SIG_FREE;									\
		(_node_)->sig.data[sigindex].one  = MEMMGR_SIG_ZONE;									\
	}																							\
}


#define _SetTrail( _node_ )																		\
{																								\
	int sigindex;																				\
	for (sigindex = 0; sigindex < MEM_SIG_SIZE; sigindex++) {									\
		((_MemSig *)((_node_)->data + (_node_)->size))->data[sigindex].redz	= MEMMGR_SIG_REDZ;	\
		((_MemSig *)((_node_)->data + (_node_)->size))->data[sigindex].one	= MEMMGR_SIG_ONE;	\
	}																							\
}

#define _SetTrailF( _node_ )																	\
{																								\
	int sigindex;																				\
	for (sigindex = 0; sigindex < MEM_SIG_SIZE; sigindex++) {									\
		((_MemSig *)((_node_)->data + (_node_)->size))->data[sigindex].redz	= MEMMGR_SIG_FREE;	\
		((_MemSig *)((_node_)->data + (_node_)->size))->data[sigindex].one	= MEMMGR_SIG_ZONE;	\
	}																							\
}

#define _ClearTrail( _node_ )																	\
{																								\
	int sigindex;																				\
	for (sigindex = 0; sigindex < MEM_SIG_SIZE; sigindex++) {									\
		((_MemSig *)((_node_)->data + (_node_)->size))->data[sigindex].redz= 0;					\
		((_MemSig *)((_node_)->data + (_node_)->size))->data[sigindex].one	= 0;				\
	}																							\
}

#define _SetFree( _node_, _file_, _line_ )      (_node_)->freeInfo.lastAlloc.file = (_node_)->allocInfo.file;	\
												(_node_)->freeInfo.lastAlloc.line = (_node_)->allocInfo.line;	\
												(_node_)->allocInfo.file = NULL;	\
												(_node_)->allocInfo.line = 0;	\
												(_node_)->freeInfo.info.file = (_file_);		\
												(_node_)->freeInfo.info.line = (_line_);
#else
#define _SetSig( _node_ )
#define _SetSigF( _node_ )
#define _SetTrail( _node_ )
#define _SetTrailF( _node_ )
#define _ClearTrail( _node_ )
#define _SetFree( _node_, _file_, _line_ )
#define _AssertSig( _node_, _isfree_ )
#define _AssertTrail( _node_ )
#endif

#define StackBuffer( _type_, _name_, _size_ )	_StackNode _name_##_node;						\
												char _name_##_buffer[(_size_)+sizeof(_MemSig)];	\
												_type_ *(_name_)

#define StackAlloc( _type_, _name_, _size_ )	_name_##_node.slab=NULL;						\
												_name_##_node.size=(_size_);					\
												_name_=(_type_ *)&_name_##_node.data;			\
												_SetAlloc(&_name_##_node, __FILE__, __LINE__);	\
												_SetSig(&_name_##_node);						\
												_SetTrail(&_name_##_node);						\
												_MarkEmpty( &_name_##_node)

#define StackFree( _name_ )																		\
												_AssertSig(&_name_##_node,TRUE);				\
												_AssertTrail(&_name_##_node);					\
												_SetFree(&_name_##_node, __FILE__, __LINE__);	\
												_SetSigF(&_name_##_node);						\
												_SetTrailF(&_name_##_node);						\
												_MarkFree(&_name_##_node)

typedef struct _MemStatistics {
    struct {
		unsigned long min;
		unsigned long max;
        unsigned long count;
        unsigned long size;
    } totalAlloc;

    unsigned long pitches;
    unsigned long hits;
    unsigned long strikes;

    struct {
        unsigned long size;

        unsigned long minimum;
        unsigned long maximum;

        unsigned long allocated;
    } entry;

    unsigned char *name;

    struct _MemStatistics *next;
    struct _MemStatistics *previous;
} MemStatistics;

typedef XplBool (*PoolEntryCB)( void *ptr, void *client );
typedef XplBool (*PoolEnumerateCB)(void *buffer, size_t size, const char *file, const int line, void *clientData);
typedef void *MemPool;
typedef void *MemChunk;

#ifdef __cplusplus
extern "C" {
#endif

EXPORT int ttyprintf( FILE *fp, const char *format, ... );
EXPORT XplBool MMOpenEx( const char *consumer, MemPoolConfig *config, const char *file, unsigned long line );
#define MMOpen( c, f, l )	MMOpenEx( (c), NULL, (f), (l) )
EXPORT XplBool MMClose( const char *consumer, XplBool xplMain, const char *file, unsigned long line );
EXPORT void MMUpdateOwner( void *ptr, const char *file, unsigned long line );
EXPORT void MMLock( void *ptr, uint32 key );
EXPORT void MMUnlock( void *ptr, uint32 key );
EXPORT void MMCopyOwner( void *dst, void *src );
EXPORT void MMGetOwner( void *ptr, const char **file, unsigned long *line );
EXPORT void MMAssert( void *ptr );
EXPORT void *MMAllocEx( void *ptr, size_t size, size_t *actual, XplBool wait, XplBool zero, const char *file, unsigned long line );
EXPORT void *MMFree( void *ptr, const char *file, unsigned long line );
EXPORT char *MMDupEx( const char *src, size_t size, XplBool wait, const char *file, unsigned long line );
EXPORT void MMFreeRelease( void *ptr, void *unused, const char *file, unsigned long line );
EXPORT char *MMStrdup( const char *string, size_t *actual, XplBool wait, const char *file, unsigned long line );
EXPORT char *MMStrndup( const char *string, size_t *actual, size_t size, XplBool wait, const char *file, unsigned long line );
EXPORT char *MMSprintf( const char *file, unsigned long line, const char *format, ... ) XplFormatString(3, 4);
EXPORT int MMAsprintf( char **ptr, const char *file, unsigned long line, const char *format, ... ) XplFormatString(4, 5);
EXPORT size_t MMSize( void *ptr );
EXPORT int MMStrcatv( char **ptr, const char *file, unsigned long line, const char *format, va_list args );
EXPORT int MMStrcatf( char **ptr, const char *file, unsigned long line, const char *format, ... )  XplFormatString(4, 5);

EXPORT void MMGenerateReports( void );
EXPORT void MMDumpPools( FILE *fp );
EXPORT const char *MMConsumer( void );

EXPORT XplBool ZeroMemoryCB( void *ptr, void *client );
EXPORT MemPool *MMPoolAlloc( const char *identity, size_t size, size_t nodesPerSlab, PoolEntryCB prepareCB, PoolEntryCB freeCB, PoolEntryCB destroyCB, void *clientData, const char *file, unsigned long line );
EXPORT void MMPoolAbandon( MemPool *handle );
EXPORT void *MMPoolGet( MemPool *handle, const char *file, unsigned long line );
EXPORT MemChunk *MMPoolGetChunk( MemPool *handle, size_t nodes, const char *file, unsigned long line );
EXPORT void *MMChunkGet( MemChunk *chunk, const char *file, unsigned long line );
EXPORT int MMPoolValidate( MemPool *handle );
EXPORT void MMTrackMemory( int on );
EXPORT void MMWatch( void *ptr );
EXPORT void MMUnwatch( void *ptr );

// depricated interfaces
EXPORT void MMDiscard_is_depricated( void *ptr, const char *file, unsigned long line );
EXPORT void MMPoolStats( MemPool *handle, MemStatistics *stats );
EXPORT MemStatistics *MMAllocStatistics( const char *file, unsigned long line );
EXPORT void MMFreeStatistics( MemStatistics *stats, const char *file, unsigned long line );
EXPORT void MMPoolEnumerate( MemPool *handle, PoolEntryCB cb, void *clientData );
EXPORT void MMPoolEnumerateID( const char *identity, PoolEnumerateCB cb, void *clientData );
// end of depricated interfaces

#ifdef __cplusplus
}
#endif

#ifdef DEBUG_ASSERT
#define MemAssert(p)							MMAssert(p)
#else
#define MemAssert(p)
#endif

#define MemoryManagerOpen( c )					MMOpen( (c), __FILE__, __LINE__ )
#define MemoryManagerOpenEx( c, cfg )			MMOpenEx( (c), cfg, __FILE__, __LINE__ )
#define MemoryManagerClose( c )					MMClose( (c), FALSE, __FILE__, __LINE__ )
#define MemUpdateOwner( p, f, l )				MMUpdateOwner( (p), (f), (l) )
#define MemLock( p, v )							MMLock( (p), (v) )
#define MemUnlock( p, v )						MMUnlock( (p), (v) )
#define MemCopyOwner( d, s )					MMCopyOwner( (d), (s) )
#define MemGetOwner( p, f, l )					MMGetOwner( (p), (f), (l) )
#define MemMallocEx( p, s, a, w, z )			MMAllocEx( (p), (s), (a), (w), (z), __FILE__, __LINE__ )
#define MemMalloc( s )							MMAllocEx( NULL, (s), NULL, 0, 0, __FILE__, __LINE__ )
#define MemMallocWait( s )						MMAllocEx( NULL, (s), NULL, 1, 0, __FILE__, __LINE__ )
#define MemFree( p )							MMFree( (p), __FILE__, __LINE__ )
#define MemFreeEx( p, f, l )					MMFree( (p), (f), (l) )
#define MemRelease( p )							(*((void **)(p))) = MMFree( ((void *) *(p)), __FILE__, __LINE__ )
#define MemReleaseEx( p, f, l )					(*((void **)(p))) = MMFree( ((void *) *(p)), (f), (l) )
#define MemRealloc( p, s )						MMAllocEx( (p), (s), NULL, 0, 0, __FILE__, __LINE__ )
#define MemReallocWait( p, s )					MMAllocEx( (p), (s), NULL, 1, 0, __FILE__, __LINE__ )
#define MemDup( p, s )							MMDupEx( (p), (s), FALSE, __FILE__, __LINE__ );
#define MemDupWait( p, s )						MMDupEx( (p), (s), TRUE, __FILE__, __LINE__ );
#define MemStrdup( s )							MMStrdup( (s), NULL, 0, __FILE__, __LINE__ )
#define MemStrdupWait( s )						MMStrdup( (s), NULL, 1, __FILE__, __LINE__ )
#define MemStrndup( s, m )						MMStrndup( (s), NULL, (m), 0, __FILE__, __LINE__ )
#define MemStrndupWait( s, m )					MMStrndup( (s), NULL, (m), 1, __FILE__, __LINE__ )
#define MemSprintf( f, ... )					MMSprintf( __FILE__, __LINE__, (f), ##__VA_ARGS__ )
#define MemAsprintf( p, f, ... )				MMAsprintf( (p), __FILE__, __LINE__, (f), ##__VA_ARGS__ )
#define MemCalloc( c, s )						MMAllocEx( NULL, (c)*(s), NULL, 0, 1, __FILE__, __LINE__ )
#define MemCallocWait( c, s )					MMAllocEx( NULL, (c)*(s), NULL, 1, 1, __FILE__, __LINE__ )
#define MemSize( p )							MMSize( (p) )
#define MemStrcatv( p, f, a )					MMStrcatv( (p), __FILE__, __LINE__, (f), (a) )
#define MemStrcatf( p, f, ... )					MMStrcatf( (p), __FILE__, __LINE__, (f), ##__VA_ARGS__ )

#define MemGenerateReports()					MMGenerateReports()
#define MemDumpPools( f )						MMDumpPools( (f) )
#define MemConsumer()							MMConsumer()

#define MemPoolAlloc( i, s, n, p, f, d, c )		MMPoolAlloc( (i), (s), (n), (p), (f), (d), (c), __FILE__, __LINE__ )
#define MemPoolGet( h )							MMPoolGet( (h), __FILE__, __LINE__ )
#define MemPoolGetChunk( h, n )					MMPoolGetChunk( (h), (n), __FILE__, __LINE__ )
#define MemChunkGet( c )						MMChunkGet( (c), __FILE__, __LINE__ )
#define MemPoolValidate( h )					MMPoolValidate( (h) )
#define MemTrackMemory( o )						MMTrackMemory( (o) )
#define MemWatch( p )							MMWatch( (p) )
#define MemUnwatch( p )							MMUnwatch( (p))

// depricated interfaces

#define MemPrivatePoolFree( h )					MMFree( (h), __FILE__, __LINE__ )
#define MemPrivatePoolFreeEx( h, f, l )			MMFree( (h), (f), (l) )
#define MemPrivatePoolGetEntry( h )             MMPoolGet( (h), __FILE__, __LINE__ )
#define MemPrivatePoolReturnEntry( p )  		MMFree( (p), __FILE__, __LINE__ )
#define MemPrivatePoolReturnEntryEx( p, f, l )  MMFree( (p), (f), (l) )
#define MemPrivatePoolDiscardEntry( p )         MMDiscard_is_depricated( (p), __FILE__, __LINE__ )

#define MemPrivatePoolStatistics( h, s )		MMPoolStats( (h), (s) )
#define MemAllocStatistics()					MMAllocStatistics( __FILE__, __LINE__ )
#define MemFreeStatistics( s )					MMFreeStatistics( s, __FILE__, __LINE__ )
#define MemPrivatePoolEnumerate( h, c, d )		MMPoolEnumerate( h, c, d )
#define MemPoolEnumerate( i, c, d )				MMPoolEnumerateID( i, c, d )
// end of depricated interfaces

#elif defined(MEMMGR_TYPE_NONE) || defined(MEMMGR_TYPE_GUARD)
/*
	Use the no-memmgr.h header instead of memmgr.h so that all Memory Manager
	calls will be replaced with system calls.
*/
#include "no-memmgr.h"
#else
/* Library Specific */
#define MEMMGR_API_VERSION          1
#define MEMMGR_API_COMPATIBILITY    1

typedef struct _MemStatistics {
    struct {
		unsigned long min;
		unsigned long max;
        unsigned long count;
        unsigned long size;
    } totalAlloc;

    unsigned long pitches;
    unsigned long hits;
    unsigned long strikes;

    struct {
        unsigned long size;

        unsigned long minimum;
        unsigned long maximum;

        unsigned long allocated;
    } entry;

    unsigned char *name;

    struct _MemStatistics *next;
    struct _MemStatistics *previous;
} MemStatistics;

typedef XplBool (*PoolEntryCB)(void *Buffer, void *ClientData);
typedef XplBool (*PoolEnumerateCB)(void *Buffer, size_t Size, const char *File, const int Line, void *ClientData);

#define MemoryManagerOpen( a )	_MemoryManagerOpen( (a), __FILE__, __LINE__ )
#define MemoryManagerOpenEx( a, cfg )	_MemoryManagerOpen( (a), __FILE__, __LINE__ )
EXPORT XplBool _MemoryManagerOpen( const char *agent, const char *file, unsigned long line );
#define MemoryManagerClose( a )	_MemoryManagerClose( (a), __FILE__, __LINE__ )
EXPORT XplBool _MemoryManagerClose( const char *agent, const char *file, unsigned long line );

#if !defined(DEBUG)
#define MemUpdateOwner(s, f, l)
#define MemCopyOwner(d, s)
#else
EXPORT void MemUpdateOwner(void *Source, const char *File, const unsigned int Line);
EXPORT void MemCopyOwner(void *Dest, void *Source);
#endif
#define MemLock(p, v)
#define MemUnlock(p, v)
#define MemGetOwner( p, f, l )	if((f)) *(f)="unknown";if((l)) *(l)=0;

/*    Memory Allocation API's    */
EXPORT void *MemCallocDirect(size_t Number, size_t Size, const char *File, unsigned int Line);
EXPORT void *MemCallocDebugDirect(size_t Number, size_t Size, const char *File, unsigned int Line);

EXPORT void *MemMallocDirect(size_t Size, const char *File, unsigned int Line);
EXPORT void *MemMallocDebugDirect(size_t Size, const char *File, unsigned int Line);

EXPORT void *MemReallocDirect(void *Source, size_t Size, const char *File, unsigned int Line);
EXPORT void *MemReallocDebugDirect(void *Source, size_t Size, const char *File, unsigned int Line);

#if !defined(DEBUG)
#define MemCalloc(n, s)     MemCallocDirect((n), (s), __FILE__, __LINE__)
#else
#define MemCalloc(n, s)     MemCallocDebugDirect((n), (s), __FILE__, __LINE__)
#endif

#define MemCallocWait(n,s)  MemCallocDirect( (n), (s), __FILE__, __LINE__)

#if !defined(DEBUG)
#define MemMalloc(s)        MemMallocDirect((s), __FILE__, __LINE__)
#else
#define MemMalloc(s)        MemMallocDebugDirect((s), __FILE__, __LINE__)
#endif

#if !defined(DEBUG)
#define MemRealloc(m, s)    MemReallocDirect((m), (s), __FILE__, __LINE__)
#else
#define MemRealloc(m, s)    MemReallocDebugDirect((m), (s), __FILE__, __LINE__)
#endif

/* Check a pointer in debug mode */
#ifdef DEBUG
EXPORT void MemAssert(void *ptr);
#else
#define MemAssert(ptr)
#endif

/*    Memory De-Allocation API's    */
EXPORT void MemFreeDirect(void *Source);
EXPORT void MemFreeDebugDirect(void *Source, const char *File, unsigned int Line);

#if !defined(DEBUG)
#define MemFree(m)          MemFreeDirect((m))
#else
#define MemFree(m)          MemFreeDebugDirect((m), __FILE__, __LINE__)
#endif

/*    String Allocation API's    */
EXPORT char *MemStrdupDirect(const char *StrSource, const char *File, unsigned int Line);
EXPORT char *MemStrndupDirect(const char *StrSource, size_t length, const char *File, unsigned int Line);
EXPORT int MemAsprintfDirect (char **StrP, char *File, unsigned int Line, const char *Format, ...) XplFormatString(4, 5);

EXPORT char *MemStrdupDebugDirect(const char *StrSource, const char *File, unsigned int Line);
EXPORT char *MemStrndupDebugDirect(const char *StrSource, size_t length, const char *File, unsigned int Line);
EXPORT int MemAsprintfDebugDirect (char **StrP, char *File, unsigned int Line, const char *Format, ...) XplFormatString(4, 5);

#if !defined(DEBUG)
#define MemStrdup(s)          MemStrdupDirect((s), __FILE__, __LINE__)
#define MemStrndup(s,l)       MemStrndupDirect((s), (l), __FILE__, __LINE__)
#define MemAsprintf(p,f,...)  MemAsprintfDirect((p), __FILE__, __LINE__, (f), ##__VA_ARGS__)
#else
#define MemStrdup(s)          MemStrdupDebugDirect((s), __FILE__, __LINE__)
#define MemStrndup(s,l)       MemStrndupDebugDirect((s), (l), __FILE__, __LINE__)
#define MemAsprintf(p,f,...)  MemAsprintfDebugDirect((p), __FILE__, __LINE__, (f), ##__VA_ARGS__)
#endif

/*    Management Statistics    */
EXPORT void MemPoolEnumerate(unsigned char *Name, PoolEnumerateCB EnumerationCB, void *Data);
EXPORT MemStatistics *MemAllocStatistics(void);
EXPORT void MemFreeStatistics(MemStatistics *Statistics);

/*    Private Pool API's    */
EXPORT void MemPrivatePoolFree(void *PoolHandle);
EXPORT void MemPrivatePoolEnumerate(void *PoolHandle, PoolEntryCB EnumerationCB, void *Data);
EXPORT void MemPrivatePoolStatistics(void *PoolHandle, MemStatistics *PoolStats);

EXPORT void *MemPrivatePoolGetEntryDirect(void *PoolHandle);
EXPORT void *MemPrivatePoolGetEntryDebugDirect(void *PoolHandle, const char *File, unsigned int Line);

#if !defined(DEBUG)
#define MemPrivatePoolGetEntry(h)          MemPrivatePoolGetEntryDirect((h))
#else
#define MemPrivatePoolGetEntry(h)          MemPrivatePoolGetEntryDebugDirect((h), __FILE__, __LINE__)
#endif



EXPORT void MemPrivatePoolReturnEntry(void *PoolEntry);
EXPORT void MemPrivatePoolReturnEntryDebug(void *Source, const char *File, unsigned int Line);
EXPORT void MemPrivatePoolDiscardEntry(void *Source);

/* New APIs with guaranteed success returns and pointer zeroing */

#if !defined(DEBUG)
#define MemMallocWait(s)        MemMallocWaitDirect((s), __FILE__, __LINE__)
#define MemReallocWait(m, s)    MemReallocWaitDirect((m), (s), __FILE__, __LINE__)
#define MemRelease(m)			MemReleaseDirect((m))
#define MemStrdupWait(s)		MemStrdupWaitDirect((s), __FILE__, __LINE__)
#else
#define MemMallocWait(s)        MemMallocDebugWaitDirect((s), __FILE__, __LINE__)
#define MemReallocWait(m, s)    MemReallocDebugWaitDirect((m), (s), __FILE__, __LINE__)
#define MemRelease(m)			MemReleaseDebugDirect((m), __FILE__, __LINE__)
#define MemStrdupWait(s)		MemStrdupDebugWaitDirect((s), __FILE__, __LINE__)
#endif


EXPORT void *MemMallocWaitDirect (size_t Size, const char *File, unsigned int Line);
EXPORT void *MemMallocDebugWaitDirect (size_t Size, const char *File, unsigned int Line);
EXPORT void MemReallocWaitDirect (void **Source, size_t Size, const char *File, unsigned int Line);
EXPORT void MemReallocDebugWaitDirect (void **Source, size_t Size, const char *File, unsigned int Line);
EXPORT void MemReleaseDirect(void **Source);
EXPORT void MemReleaseDebugDirect(void **Source, const char *File, unsigned int Line);
EXPORT char *MemStrdupWaitDirect(const char *StrSource, const char *File, unsigned int Line);
EXPORT char *MemStrdupDebugWaitDirect(const char *StrSource, const char *File, unsigned int Line);

#endif // MEMMGR_TYPE
#endif // MEMMGR_H

