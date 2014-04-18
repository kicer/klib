#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#define _KLIB_H_INSIDE_
#include "sys/kmacros.h"
#include "sys/ktypes.h"
#include "sys/kconf.h"
#undef _KLIB_H_INSIDE_

#define MAX_LENGTH_OF_LINE	512
#define SEP_CHAR		'='
#define COMMENT_CHAR	'#'
#define is_comment(c) ((c) == COMMENT_CHAR)
#define is_sep(c) ((c) == SEP_CHAR)
#define is_quote(c) (((c) == '"') || ((c) == '\''))
#define is_CRLF(c) (((c) == '\x0d') || ((c) == '\x0a'))

typedef enum {
	FIND_WORD_START,
	FIND_WORD_END,
	PARSE_WORD_END,
} CONFIG_STATE;

typedef struct {
	int word_start;
	int word_len;
	int quote_flag;
	const char *pbuf;
} WORD_INFO;

static WORD_INFO chk_word(const char *buf) {
	const char * pbuf = buf;
	CONFIG_STATE state = FIND_WORD_START;

	int word_start = 0;
	int word_len = -1;
	int quote_flag = -1;
	int space_flag = -1;

	while(*pbuf != 0) {
		switch(state) {
			case FIND_WORD_START:
				if(isspace(*pbuf)) {
				} else if(is_comment(*pbuf)) {
					/* comment line */
					state = PARSE_WORD_END;
				} else if(is_quote(*pbuf)) {
					quote_flag = *pbuf;
					char *ptmp = strchr(pbuf+1, quote_flag);
					if(ptmp != NULL) {
						word_len = ptmp - pbuf - 1;
						if(word_len > 0) {
							word_start = pbuf - buf + 1;
							state = PARSE_WORD_END;
							pbuf = ptmp;
						} else {
							/* key is "" */
							word_len = -1;
							state = PARSE_WORD_END;
						}
					} else {
						/* single-quote key */
						state = PARSE_WORD_END;
					}
				} else {
					word_start = pbuf - buf;
					state = FIND_WORD_END;
				}
				break;

			case FIND_WORD_END:
				if(is_sep(*pbuf) || is_comment(*pbuf) || is_CRLF(*pbuf)) {
					if(space_flag == -1) {
						word_len = pbuf - buf - word_start;
					} else {
						word_len = space_flag - word_start;
					}
					if(word_len <= 0) {
						/* key is "" */
						word_len = -1;
					}
					state = PARSE_WORD_END;
				} else if(isspace(*pbuf)) {
					if(space_flag == -1)
						space_flag = pbuf - buf;
				} else {
					space_flag = -1;
				}
				break;
			default:
				break;
		}
		if(state == PARSE_WORD_END) break;
		pbuf++;
	}

	WORD_INFO t;
	t.word_start = word_start;
	t.word_len = word_len;
	t.quote_flag = quote_flag;
	t.pbuf = pbuf;

	return t;
}

static const char *parse_key(const char *buf, const char *key) {
	if(buf == NULL) return NULL;

	WORD_INFO t = chk_word(buf);

	if(t.word_len > 0) {
		if(t.word_len == strlen(key)) {
			if(strncmp(key, buf+t.word_start, t.word_len) == 0)
				return t.pbuf;
		}
	}

	return NULL;
}

static const char *parse_value(const char *buf, char **val) {
	if(buf == NULL) return NULL;

	WORD_INFO t = chk_word(buf);

	if(t.word_len > 0) {
		*val = malloc(t.word_len+1);
		*((*val)+t.word_len) = 0;
		memcpy(*val, buf+t.word_start, t.word_len);
		return t.pbuf;
	}

	return NULL;
}

char* k_get_string(const char *key, const char *file) {
	FILE *fp = NULL;
	char buf[MAX_LENGTH_OF_LINE]; /* max length of line */
	char *pval = NULL;

	fp = fopen(file, "r");
	if(fp == NULL) {
		fprintf(stderr, "Unable to open '%s' to parse\n", file);
		return NULL;
	}
	
	while(fgets(buf, sizeof(buf), fp) != NULL) {
		const char *pbuf = parse_key(buf, key);
		if(pbuf != NULL) {
			const char *pstart = strchr(pbuf, SEP_CHAR);
			if(pstart != NULL) {
				parse_value(pstart+1, &pval);
			}
			break;
		}
	}

	fclose(fp);

	return pval;
}

int k_foreach_config(const char *file, int (*cb)(const char *key, const char *val, void *ptr), void *ptr) {
	FILE *fp = NULL;
	char buf[MAX_LENGTH_OF_LINE]; /* max length of line */
	char *pval = NULL;
	char *pkey = NULL;
	int ret = -1;

	fp = fopen(file, "r");
	if(fp == NULL) {
		fprintf(stderr, "Unable to open '%s' to parse\n", file);
		return -1;
	}

	while(fgets(buf, sizeof(buf), fp) != NULL) {
		WORD_INFO t1 = chk_word(buf);
		if(t1.word_len > 0) {
			char *pstart = strchr(t1.pbuf, SEP_CHAR);
			if(pstart != NULL) {
				WORD_INFO t2 = chk_word(pstart+1);
				if(t2.word_len > 0) {
					pkey = buf + t1.word_start;
					*(pkey+t1.word_len) = 0;
					pval = pstart + 1 + t2.word_start;
					*(pval+t2.word_len) = 0;
					ret = (*cb)(pkey, pval, ptr);
					if(ret != 0) break;
				}
			}
		}
	}

	fclose(fp);
	return ret;
}

long int k_get_long(const char *key, const char *file, long int def) {
	long int val = def;

	char *p = k_get_string(key, file);
	if(p != NULL) {
		if(*p) {
			char *endptr = NULL;
			errno = 0;
			val = strtol(p, &endptr, 0);
			if((endptr == p) || (errno != 0)) val = def;
		}
		free(p);
	}

	return val;
}

int k_set_string(const char *key, const char *file, const char *val) {
	return 0;
}

int k_set_int(const char *key, const char *file, int val) {
	char buff[16];
	snprintf(buff, sizeof(buff), "%d", val);

	return k_set_string(key, file, buff);
}
