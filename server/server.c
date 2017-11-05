#include <errno.h>
#include <error.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>
#include <event2/event.h>
#include <pthread.h>

#include "argument_parser.h"
#include "main_events.h"
#include "worker.h"
#include "server.h"


int listening_sok;


int
create_listen_socket(const int port_num)
{
	int custom_errno = 0;
	int sok;
	char *port_num_str;
	struct addrinfo *addr_hints, *possible_addr_head, *possible_addr;

	port_num_str = malloc(6 * sizeof(char));
	sprintf(port_num_str, "%d", port_num ? port_num : 2333);
	addr_hints = calloc(1, sizeof(struct addrinfo));
	addr_hints->ai_family = AF_UNSPEC;
	addr_hints->ai_socktype = SOCK_STREAM;
	addr_hints->ai_flags = AI_PASSIVE;
	custom_errno = getaddrinfo(NULL, port_num_str, addr_hints, &possible_addr_head);
	if (custom_errno != 0)
		error(1, custom_errno, gai_strerror(custom_errno));
	free(addr_hints);
	for (possible_addr = possible_addr_head;
		 possible_addr != NULL;
		 possible_addr = possible_addr->ai_next) {
		sok = socket(possible_addr->ai_family, SOCK_STREAM, 0);
		if (sok < 0)
			continue;
		if (bind(sok, possible_addr->ai_addr, possible_addr->ai_addrlen) == -1) {
			close(sok);
			continue;
		}
		if (listen(sok, 0) == -1) {
			close(sok);
			continue;
		}
		break;
	}
	if (possible_addr == NULL)
		error(1, errno, "No matching addresses.\n");
	freeaddrinfo(possible_addr_head);
	free(port_num_str);
	return sok;
}



void
handle_control_c(int signal)
{
	tear_down_workers();
	tear_down_main_events();	
	if (close(listening_sok) == -1)
		error(1, errno, "Error closing listening socket");
	printf("Exiting...\n");
	exit(EXIT_SUCCESS);
}

int
main(int argc, char **argv)
{
	struct arguments inputs = {
		.port = 0,
	};

	signal(SIGINT, &handle_control_c);
	parse_arguments(argc, argv, &inputs);
	listening_sok = create_listen_socket(inputs.port);
	set_up_main_events(listening_sok);
	set_up_workers();
	printf("Worker count: %d\n", worker_count);
	run_main_events();
	handle_control_c(SIGINT);
	return EXIT_SUCCESS;
}
