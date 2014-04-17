#if !defined(KLIB_ENABLE_SINGLE_INCLUDES) && !defined (_KLIB_H_INSIDE_)
#error "Only <klib.h> can be included directly."
#endif

#ifndef _KCONF_H_
#define _KCONF_H_

K_BEGIN_DECLS

/* NOTE! return value with malloc, must free() by yourself!!! */
char* k_get_string(const char *key, const char *file);
int k_get_int(const char *key, const char *file);
int k_foreach_config(const char *file, int (*cb)(const char *key, const char *val, void *ptr), void *ptr);
int k_set_string(const char *key, const char *file, const char *val);
int k_set_int(const char *key, const char *file, int val);

K_END_DECLS

#endif /* _KCONF_H_ */
