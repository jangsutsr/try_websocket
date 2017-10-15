#include "client.h"


struct arguments {
	char *address;
	int port;
};


int
populate_arguments(int argc, char **argv, struct arguments *to_populate);
