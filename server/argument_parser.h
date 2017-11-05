#ifndef _WEBSOCKET_ARGUMENT_PARSER_H
#define _WEBSOCKET_ARGUMENT_PARSER_H

struct arguments {
	int port;
};


int parse_arguments(int argc, char **argv, struct arguments *arg_store);

#endif
