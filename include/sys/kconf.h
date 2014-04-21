#if !defined(KLIB_ENABLE_SINGLE_INCLUDES) && !defined (_KLIB_H_INSIDE_)
#error "Only <klib.h> can be included directly."
#endif

#ifndef _KCONF_H_
#define _KCONF_H_

K_BEGIN_DECLS

/* NOTE! return NULL if not found,
 * else return value with malloc, must free() by yourself!!!
 * */
char* k_conf_get_string(const char *file, const char *key);

/* loop config file and do cb() function
 * cb() return 0 to continue parse, otherwise break parse.
 * 3rd parameter of prt will pass to cb() function
 */
int k_conf_foreach(const char *file, int (*cb)(const char *key, const char *val, void *ptr), void *ptr);

int k_conf_set_string(const char *file, const char *key, const char *val);

/* return key value of file,
 * if not exist or convert error will return def
 * use strtol() to convert, support '+'/'-'/"0x"/'0' prefix
 */
long int k_conf_get_long(const char *file, const char *key, long int def);

int k_conf_set_long(const char *file, const char *key, long int val);

K_END_DECLS

#endif /* _KCONF_H_ */
