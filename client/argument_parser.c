#include "argument_parser.h"


void
populate_address(struct arguments *to_populate, const char *val)
{
	to_populate->address = malloc(strlen(val));
	strcpy(to_populate->address, val);
}


void
populate_port(struct arguments *to_populate, const char *val)
{
	to_populate->port = atoi(val);
	if (to_populate->port < 0)
		to_populate->port = 0;
}


int
populate_arguments(int argc, char **argv, struct arguments *to_populate)
{
	int is_long_opt = 0, long_option_idx;
	int short_option;
	struct option long_options[] = {
		{"address", required_argument, &is_long_opt, 1},
		{"port", required_argument, &is_long_opt, 1},
		{0, 0, 0, 0}
	};
	void (*process_long_opt[])(struct arguments *, const char *) = {
		&populate_address,
		&populate_port
	};

	while ((short_option = getopt_long(argc, argv, ":", long_options, &long_option_idx)) != -1) {
		switch (short_option) {
		case 0:
			if (is_long_opt)
				(*process_long_opt[long_option_idx])(to_populate, optarg);
			break;
		case '?':
			printf("Unrecognized argument.\n");
			break;
		case ':':
			printf("Required argument value not provided.\n");
			break;
		default:
			break;
		}
		is_long_opt = 0;
	}
	if (to_populate->address == NULL || to_populate->port == 0)
		return 1;

	return 0;
}
