#include "server.h" 


struct arguments {
	int port;
};


int parse_arguments(int argc, char **argv, struct arguments *arg_store);
