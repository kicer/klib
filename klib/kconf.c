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
	FIND_KEY_START,
	FIND_KEY_END,
	FIND_VAL_START,
	FIND_VAL_END,
	PARSE_CONFIG_END,
} CONFIG_STATE;

#define is_comment(c) ((c) == '#')
#define is_sep(c) ((c) == '=')
#define is_quote(c) (((c) == '"') || ((c) == '\''))

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
		char *pbuf = buf;
		CONFIG_STATE state = FIND_KEY_START;

		int key_start = 0;
		int key_len = -1;
		int val_start = 0;
		int val_len = -1;
		int quote_flag = -1;
		int space_flag = -1;

		while(*pbuf != 0) {
			switch(state) {
				case FIND_KEY_START:
					if(isspace(*pbuf)) {
					} else if(is_comment(*pbuf)) {
						/* comment line */
						state = PARSE_CONFIG_END;
					} else if(is_quote(*pbuf)) {
						quote_flag = *pbuf;
						char *ptmp = strchr(pbuf+1, quote_flag);
						if(ptmp != NULL) {
							key_len = ptmp - pbuf - 1;
							if(key_len > 0) {
								key_start = pbuf - buf + 1;
								state = FIND_VAL_START;
								pbuf = ptmp;
							} else {
								/* key is "" */
								key_len = -1;
								state = PARSE_CONFIG_END;
							}
						} else {
							/* single-quote key */
							state = PARSE_CONFIG_END;
						}
					} else {
						key_start = pbuf - buf;
						state = FIND_KEY_END;
					}
					break;
				case FIND_KEY_END:
					if(is_sep(*pbuf) || is_comment(*pbuf)) {
						if(space_flag == -1) {
							key_len = pbuf - buf - key_start;
						} else {
							key_len = space_flag - key_start;
						}
						if(key_len > 0) {
							state = FIND_VAL_START;
						} else {
							/* key is "" */
							key_len = -1;
							state = PARSE_CONFIG_END;
						}
					} else if(isspace(*pbuf)) {
						if(space_flag == -1)
							space_flag = pbuf - buf;
					} else {
						space_flag = -1;
					}
					break;
				case FIND_VAL_START:
					break;
				case FIND_VAL_END:
					break;
				default:
					break;
			}
			if(state == PARSE_CONFIG_END) break;
			pbuf++;
		}

		if(key_len > 0) {
			char tmpbuf[key_len+1];
			tmpbuf[key_len] = 0;
			memcpy(tmpbuf, buf+key_start, key_len);
			printf("%s\tKEY: %d,%d <%s>\n\n", buf, key_start, key_len, tmpbuf);
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

	printf("Value of <%s> is: %d\n", "ONE ONE", k_get_int("ONE ONE", argv[1]));

	return 0;
}
#endif
