#include <stdio.h>
#include <stdlib.h>

#include <klib.h>


int main(int argc, char **argv) {
	if(argc < 3) {
		fprintf(stderr, "Usage: %s <config-file> <key> [default]\n", argv[0]);
		exit(1);
	}

	char *p = k_conf_get_string(argv[1], argv[2]);
	if(p != NULL) {
		printf("%s\n", p);
		free(p);
	} else {
		if(argc >= 4) printf("%s\n", argv[3]);
		else return -1;
	}

	return 0;
}
