#include <stdio.h>
#include <stdlib.h>

#include <klib.h>


int main(int argc, char **argv) {
	if(argc != 4) {
		fprintf(stderr, "Usage: %s <config-file> <key> <value>\n", argv[0]);
		exit(1);
	}

	return k_conf_set_string(argv[1], argv[2], argv[3]);
}
