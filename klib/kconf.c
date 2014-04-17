#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define _KLIB_H_INSIDE_
#include "kmacros.h"
#include "ktypes.h"
#include "kconf.h"
#undef _KLIB_H_INSIDE_

typedef enum {
	FIND_WORD_START,
	FIND_WORD_END,
	PARSE_WORD_END,
} CONFIG_STATE;

typedef struct {
	int key_start;
	int key_len;
	int quote_flag;
	const char *pbuf;
} WORD_INFO;

#define SEP_CHAR		'='
#define COMMENT_CHAR	'#'
#define is_comment(c) ((c) == COMMENT_CHAR)
#define is_sep(c) ((c) == SEP_CHAR)
#define is_quote(c) (((c) == '"') || ((c) == '\''))


static WORD_INFO chk_word(const char *buf) {
	const char * pbuf = buf;
	CONFIG_STATE state = FIND_WORD_START;

	int key_start = 0;
	int key_len = -1;
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
						key_len = ptmp - pbuf - 1;
						if(key_len > 0) {
							key_start = pbuf - buf + 1;
							state = PARSE_WORD_END;
							pbuf = ptmp;
						} else {
							/* key is "" */
							key_len = -1;
							state = PARSE_WORD_END;
						}
					} else {
						/* single-quote key */
						state = PARSE_WORD_END;
					}
				} else {
					key_start = pbuf - buf;
					state = FIND_WORD_END;
				}
				break;

			case FIND_WORD_END:
				if(is_sep(*pbuf) || is_comment(*pbuf)) {
					if(space_flag == -1) {
						key_len = pbuf - buf - key_start;
					} else {
						key_len = space_flag - key_start;
					}
					if(key_len <= 0) {
						/* key is "" */
						key_len = -1;
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
	t.key_start = key_start;
	t.key_len = key_len;
	t.quote_flag = quote_flag;
	t.pbuf = pbuf;

	return t;
}

static const char *parse_key(const char *buf, const char *key) {
	WORD_INFO t = chk_word(buf);

	if(t.key_len > 0) {
		if(t.key_len == strlen(key)) {
			if(strncmp(key, buf+t.key_start, t.key_len) == 0)
				return t.pbuf;
		}
	}

	return NULL;
}

static int parse_value(const char *buf, char **val) {
	WORD_INFO t = chk_word(buf);

	if(t.key_len > 0) {
		*val = malloc(t.key_len+1);
		*((*val)+t.key_len) = 0;
		memcpy(*val, buf+t.key_start, t.key_len);
	}

	return 0;
}

char* k_get_string(const char *key, const char *file) {
	FILE *fp = NULL;
	char buf[512]; /* max length of line */
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
int main(int argc, char **argv) {
	if(argc != 2) {
		fprintf(stderr, "Usage: %s <config-file>\n", argv[0]);
		exit(1);
	}

	printf("Value of <%s> is: %d\n", "ONE   ONE", k_get_int("ONE   ONE", argv[1]));

	return 0;
}
#endif
