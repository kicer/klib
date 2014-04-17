#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define _KLIB_H_INSIDE_
#include "kmacros.h"
#include "ktypes.h"
#include "kconf.h"
#undef _KLIB_H_INSIDE_

#define MAX_LENGTH_OF_LINE	512
#define SEP_CHAR		'='
#define COMMENT_CHAR	'#'
#define is_comment(c) ((c) == COMMENT_CHAR)
#define is_sep(c) ((c) == SEP_CHAR)
#define is_quote(c) (((c) == '"') || ((c) == '\''))

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
				if(is_sep(*pbuf) || is_comment(*pbuf)) {
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
		const char *pbuf = parse_value(buf, &pkey);
		if(pkey != NULL) {
			const char *pstart = strchr(pbuf, SEP_CHAR);
			if(pstart != NULL) {
				parse_value(pstart+1, &pval);
				if(pval != NULL) {
					ret = (*cb)(pkey, pval, ptr);
				}
			}
		}

		if(pkey != NULL) {
			free(pkey);
			pkey = NULL;
		}
		if(pval != NULL) {
			free(pval);
			pval = NULL;
		}

		if(ret != 0) break;
	}

	fclose(fp);
	return ret;
}

int k_get_int(const char *key, const char *file) {
	int val = 0;

	char *p = k_get_string(key, file);
	if(p != NULL) {
		if(*p) {
			val = atoi(p);
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

#if 1
int parse_config(const char *key, const char *val, void *ptr) {
	printf("(%s,%s)\n", key, val);
	return 0;
}
int main(int argc, char **argv) {
	if(argc != 2) {
		fprintf(stderr, "Usage: %s <config-file>\n", argv[0]);
		exit(1);
	}

	printf("Value of <%s> is: %d\n", "ONE   ONE", k_get_int("ONE   ONE", argv[1]));

	k_foreach_config(argv[1], parse_config, NULL);

	return 0;
}
#endif
