#include <stdio.h>
#include <stdlib.h>

#include <klib.h>


int parse_config(const char *key, const char *val, void *ptr) {
	printf("(%s,%s)\n", key, val);
	return 0;
}

int main(int argc, char **argv) {
	if(argc != 2) {
		fprintf(stderr, "Usage: %s <config-file>\n", argv[0]);
		exit(1);
	}

	printf("Value of <%s> is: %ld\n", "ONE   ONE", k_conf_get_long(argv[1], "ONE   ONE", -1));
	k_conf_set_long(argv[1], "ONE   ONE", 1024);
	k_conf_set_string(argv[1], "#", "comment");
	k_conf_set_string(argv[1], "'", "quote1");
	k_conf_set_string(argv[1], "=", "sep");
	k_conf_set_string(argv[1], "\"", "quote2");
	k_conf_set_string(argv[1], " ", "space");
	k_conf_set_string(argv[1], "	", "tab");
	printf("Value of <%s> is: %ld\n", "ONE   ONE", k_conf_get_long(argv[1], "ONE   ONE", -1));

	k_conf_foreach(argv[1], parse_config, NULL);

	return 0;
}
