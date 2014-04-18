#if !defined(KLIB_ENABLE_SINGLE_INCLUDES) && !defined (_KLIB_H_INSIDE_)
#error "Only <klib.h> can be included directly."
#endif

#ifndef _KMACROS_H_
#define _KMACROS_H_


/* GLIB2: /usr/include/glib-2.0/glib/gmacros.h */
/* Guard C code in headers, while including them from C++ */
#ifdef  __cplusplus
# define K_BEGIN_DECLS  extern "C" {
# define K_END_DECLS    }
#else
# define K_BEGIN_DECLS
# define K_END_DECLS
#endif

#ifndef	FALSE
#define	FALSE	(0)
#endif

#ifndef	TRUE
#define	TRUE	(!FALSE)
#endif

#undef	MAX
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))

#undef	MIN
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))

#undef	ABS
#define ABS(a)	   (((a) < 0) ? -(a) : (a))

#undef	CLAMP
#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))


#endif /* _KMACROS_H_ */
