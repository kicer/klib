#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>

#define _KLIB_H_INSIDE_
#include "sys/kmacros.h"
#include "sys/ktypes.h"
#include "sys/kconf.h"
#undef _KLIB_H_INSIDE_

#define MAX_LENGTH_OF_LINE	512
#define SEP_CHAR		'='
#define COMMENT_CHAR		'#'
#define CR_CHAR			'\x0d' /* \r */
#define LF_CHAR			'\x0a' /* \n */
#define QUOTE1_CHAR		'\''
#define QUOTE2_CHAR		'"'
#define is_comment(c) ((c) == COMMENT_CHAR)
#define is_sep(c) ((c) == SEP_CHAR)
#define is_quote(c) (((c) == QUOTE1_CHAR) || ((c) == QUOTE2_CHAR))
#define is_CRLF(c) (((c) == CR_CHAR) || ((c) == LF_CHAR))

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
					/* invalid, comment line */
					state = PARSE_WORD_END;
				} else if(is_quote(*pbuf)) {
					quote_flag = *pbuf;
					char *ptmp = strchr(pbuf+1, quote_flag);
					if(ptmp != NULL) {
						word_len = ptmp - pbuf - 1;
						word_start = pbuf - buf + 1;
						state = PARSE_WORD_END;
						pbuf = ptmp;
					} else {
						/* invalid, only single-quote key */
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

	if(state == FIND_WORD_END) { /* no CR/LF in last line */
		if(space_flag == -1) {
			word_len = pbuf - buf - word_start;
		} else {
			word_len = space_flag - word_start;
		}
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

char* k_conf_get_string(const char *file, const char *key) {
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

int k_conf_foreach(const char *file, int (*cb)(const char *key, const char *val, void *ptr), void *ptr) {
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

long int k_conf_get_long(const char *file, const char *key, long int def) {
	long int val = def;

	char *p = k_conf_get_string(file, key);
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

static char chk_word_quote(const char *word) {
	if((word == NULL) || (*word == 0)) return '\0';

	const char *p = word;
	int special_symbol_flag = 0;
	int quote1_flag = 0;
	int quote2_flag = 0;
	while(*p) {
		switch(*p) {
			case ' ':
			case '\t':
			case COMMENT_CHAR:
			case SEP_CHAR:
				special_symbol_flag = 1; break;
			case QUOTE1_CHAR:
				quote1_flag = 1; special_symbol_flag = 1; break;
			case QUOTE2_CHAR:
				quote2_flag = 1; special_symbol_flag = 1; break;
			case LF_CHAR:
			case CR_CHAR:
				return '\0'; break;
		}
		p++;
	}

	if(special_symbol_flag == 1) {
		if(quote2_flag == 0) return QUOTE2_CHAR;
		else if(quote1_flag == 0) return QUOTE1_CHAR;
		else return '\0';
	}

	return ' ';
}

/*
 *                   /-<- f position
 *    |<-line_len ->|
 * ___|_____________|__|_______
 *    |<-  str_len   ->|
 */
static int relloc_line(FILE *f, char *str, int str_len, int line_len) {
	char buf[str_len];

	int read_len = fread(buf, 1, str_len, f);
	if(fseek(f, 0-read_len-line_len, SEEK_CUR) == 0) {
		fwrite(str, str_len, 1, f);
		if(read_len<str_len) {
			fwrite(buf, read_len, 1, f);
			return 0;
		} else {
			if(fseek(f, 0+line_len, SEEK_CUR) == 0) {
				return relloc_line(f, buf, str_len, line_len);
			}
		}
	}

	return -1;
}

int k_conf_set_string(const char *file, const char *key, const char *val) {
	char key_quote, val_quote;
	char buf[MAX_LENGTH_OF_LINE];
	FILE *f = NULL;
	int ret = -1;
	int pair_len = strlen(key)+strlen(val)+4+1+1;

	if((key_quote = chk_word_quote(key)) == '\0') return -1;
	if((val_quote = chk_word_quote(val)) == '\0') return -1;
	if(pair_len >= MAX_LENGTH_OF_LINE) return -1;

	/* check file if not exist */
	if(access(file, F_OK) != 0) {
		if((f = fopen(file, "w")) != NULL) {
			fprintf(f, "%c%s%c=%c%s%c\n",
					key_quote, key, key_quote,
					val_quote, val, val_quote);
			fclose(f);
			ret = 0;
		}

		return ret;
	}

	f = fopen(file, "r+");
	if(f != NULL) {
		int find_key_flag = 0;
		int i;
		while(fgets(buf, sizeof(buf), f) != NULL) {
			const char *pbuf = parse_key(buf, key);
			if(pbuf != NULL) {
				find_key_flag = 1;
				int line_len = strlen(buf);

				if(line_len >= pair_len) {
					if(fseek(f, 0-line_len, SEEK_CUR) == 0) {
						fprintf(f, "%c%s%c=%c%s%c",
								key_quote, key, key_quote,
								val_quote, val, val_quote);
						for(i = pair_len; i<line_len; i++) {
							fputc(' ', f);
						}
						fputc('\n', f);
						ret = 0;
					}
				} else { /* insert a line */
					char line_buf[pair_len+1];
					snprintf(line_buf, sizeof(line_buf), "%c%s%c=%c%s%c\n",
							key_quote, key, key_quote,
							val_quote, val, val_quote);
					ret = relloc_line(f, line_buf, pair_len, line_len);
				}
				break;
			}
		}

		if(find_key_flag == 0) {
			/* not found key, append in file */
			if(fseek(f, 0, SEEK_END) == 0) {
				fprintf(f, "%c%s%c=%c%s%c\n",
						key_quote, key, key_quote,
						val_quote, val, val_quote);
				ret = 0;
			}
		}
	}

	if(f != NULL) fclose(f);

	return ret;
}

int k_conf_set_long(const char *file, const char *key, long int val) {
	char buff[16];
	snprintf(buff, sizeof(buff), "%ld", val);

	return k_conf_set_string(file, key, buff);
}
