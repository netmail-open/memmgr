#ifndef __XPLDEFS_H__
#define __XPLDEFS_H__

/*
    File System
*/

/*
    These definitions are for the second argument to the access interface.
    These may be OR'd together as needed.
*/
#if !defined(F_OK)
#define F_OK    0   /* File exists */
#endif

#if !defined(X_OK)
#define X_OK    1   /* File can be executed */
#endif

#if !defined(W_OK)
#define W_OK    2   /* File can be written to */
#endif

#if !defined(R_OK)
#define R_OK    4   /* File can be read from */
#endif

#if defined(LINUX) || defined(MACOSX)

#ifndef min
#define min(a, b)   (((a) < (b))? (a): (b))
#endif

#ifndef max
#define max(a, b)   (((a) > (b))? (a): (b))
#endif

#endif /* LINUX || MACOSX */

#endif /* __XPLDEFS_H__ */
