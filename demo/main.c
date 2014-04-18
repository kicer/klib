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

	printf("Value of <%s> is: %ld\n", "ONE   ONE", k_get_long("ONE   ONE", argv[1], -1));

	k_foreach_config(argv[1], parse_config, NULL);

	return 0;
}
