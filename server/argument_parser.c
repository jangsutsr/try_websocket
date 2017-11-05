#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include "argument_parser.h"


int
populate_port(struct arguments *arg_store, const char *optarg)
{
	arg_store->port = atoi(optarg);
	if (arg_store->port < 0)
		arg_store->port = 0;
	return 0;
}


int
parse_arguments(int argc, char **argv, struct arguments *arg_store)
{
	int short_opt, is_long_opt = 0;
	int option_index;
	struct option longopts[] = {
		{"port", required_argument, &is_long_opt, 1},
		{0, 0, 0, 0}
	};
	int (*arg_handlers[])(struct arguments *, const char *) = {
		&populate_port
	};

	while ((short_opt = getopt_long(argc, argv, ":", longopts, &option_index)) != -1) {
		if (!short_opt && is_long_opt) {
			(*arg_handlers[option_index])(arg_store, optarg);
		} else if (short_opt == '?') {
			printf("non-exist\n");
		} else if (short_opt == ':') {
			printf("option missing\n");
		}
		is_long_opt = 0;
	}
	return 0;
}
