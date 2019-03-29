#ifndef XPL_WRAP_H
#define XPL_WRAP_H

#ifdef WINDOWS
# define XPL_WRAP_MODE_TYPE		int
#else
# define XPL_WRAP_MODE_TYPE		mode_t
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
	Expand system environment variables in a path

	(Currently only implemented on windows)
*/
EXPORT int XplExpandEnv(char *dest, const char *src, size_t size);

/* Wrap various system calls when needed to modify the behavior */
EXPORT FILE *XplWrapFOpen(const char *path, const char *mode);
EXPORT FILE *XplWrapFOpen64(const char *path, const char *mode);
EXPORT FILE *XplWrapFReopen(const char *path, const char *mode, FILE *stream);
EXPORT FILE *XplWrapFReopen64(const char *path, const char *mode, FILE *stream);
EXPORT int XplWrapCreat(const char *path, XPL_WRAP_MODE_TYPE mode);
EXPORT int XplWrapOpen(const char *path, int flags, ...);

EXPORT int XplWrapAccess(const char *path, int mode);
EXPORT int XplWrapStat(const char *path, struct stat *buf);
EXPORT int XplWrapSetMTime(const char *path, uint32 mtime);
EXPORT int XplWrapChdir(const char *path);
EXPORT int XplWrapChmod(const char *path, int pmode);
EXPORT int XplWrapUnlink(const char *path);
EXPORT int XplWrapRename(const char *oldpath, const char *newpath);
EXPORT int XplWrapRmDir(const char *path);

#ifdef WINDOWS
EXPORT int XplWrapMkDir(const char *path);
EXPORT XplBool XplSetCurrentDirectoryA(const char *path);
#else
EXPORT int XplWrapMkDir(const char *path, XPL_WRAP_MODE_TYPE mode);
#endif

#ifdef __cplusplus
}
#endif

/* A consumer may define XPL_NO_WRAP to avoid using the wrapped functions */
#ifndef XPL_NO_WRAP
# define fopen(p, m)				XplWrapFOpen((p), (m))
# define fopen64(p, m)				XplWrapFOpen64((p), (m))
# define freopen(p, m, s)			XplWrapFReopen((p), (m), (s))
# define freopen64(p, m, s)			XplWrapFReopen64((p), (m), (s))
# define creat(p, m)				XplWrapCreat((p), (m))
# define access(p, m)				XplWrapAccess((p), (m))
# define XplStat(p, b)				XplWrapStat((p), (b))
# define stat(p, b)					XplWrapStat((p), (b))
# define chdir(p)					XplWrapChdir((p))
# define chmod(p, m)				XplWrapChmod((p), (m))
# define unlink(p)					XplWrapUnlink((p))
# define rename(o, n)				XplWrapRename((o), (n))
# ifdef WINDOWS
#  define mkdir(p)					XplWrapMkDir((p))
#  define SetCurrentDirectoryA(p)	XplSetCurrentDirectoryA((p))
# else
#  define mkdir(p, m)				XplWrapMkDir((p), (m))
# endif
#define rmdir(p)					XplWrapRmDir((p))

// Disabled for now because it is causing issues with fields in structs named 'open'
// # define open(p, f, ...)	XplWrapOpen((p), (f), ##__VA_ARGS__)
#else
// # undef open
# define XplStat(p, b)				stat((p), (b))
# undef fopen
# undef freopen
# undef creat
# undef access
# undef chdir
# undef stat
# undef unlink
# undef rename
# undef mkdir
# undef rmdir
#endif


/* Wrappers for 64 bit file access */
#if defined( LINUX )
#if defined( UBUNTU )

#define _FILE_OFFSET_BITS 64

# define XplFOpen					fopen
# define XplFReopen					freopen
# define XplFOffset					off_t
# define XplFSeek					fseek
# define XplFTell					ftell

#else
# define XplFOpen					fopen64
# define XplFReopen					freopen64
# define XplFOffset					off_t
# define XplFSeek					fseeko
# define XplFTell					ftello
#endif
#else
# define XplFOpen					fopen
# define XplFReopen					freopen
# define XplFOffset					__int64
# define XplFSeek					_fseeki64
# define XplFTell					_ftelli64
#endif

#define XplFRewind(s)				XplFSeek((s), 0, SEEK_SET)


#endif // XPL_WRAP_H

