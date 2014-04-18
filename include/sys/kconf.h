#if !defined(KLIB_ENABLE_SINGLE_INCLUDES) && !defined (_KLIB_H_INSIDE_)
#error "Only <klib.h> can be included directly."
#endif

#ifndef _KCONF_H_
#define _KCONF_H_

K_BEGIN_DECLS

/* NOTE! return value with malloc,
 * must free() by yourself!!!
 * */
char* k_get_string(const char *key, const char *file);

/* loop config file and do cb() function
 * cb() return 0 to continue parse, otherwise break parse.
 * 3rd parameter of prt will pass to cb() function
 */
int k_foreach_config(const char *file, int (*cb)(const char *key, const char *val, void *ptr), void *ptr);

int k_set_string(const char *key, const char *file, const char *val);

/* return key value of file,
 * if not exist or convert error will return def
 * use strtol() to convert, support '+'/'-'/"0x"/'0' prefix
 */
long int k_get_long(const char *key, const char *file, long int def);

int k_set_int(const char *key, const char *file, int val);

K_END_DECLS

#endif /* _KCONF_H_ */
