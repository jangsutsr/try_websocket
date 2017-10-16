#include "client.h"
#include "argument_parser.h"


int
get_conn_sok(const char *address, const int port)
{
	struct addrinfo *possible_addrs, *possible_addr, hints = {
		.ai_flags = 0,
		.ai_family = AF_INET,
		.ai_socktype = SOCK_STREAM,
		.ai_protocol = 0,
		.ai_addrlen = 0,
		.ai_addr = NULL,
		.ai_canonname = NULL,
		.ai_next = NULL
	};
	char *port_str;
	int local_errno = 0;
	int return_sock = -1;

	port_str = malloc(6 * sizeof(char));
	sprintf(port_str, "%d", port ? port : 2333);
	local_errno = getaddrinfo(address, port_str, &hints, &possible_addrs);
	if (local_errno != 0)
		error(1, local_errno, gai_strerror(local_errno));
	free(port_str);
	possible_addr = possible_addrs;
	while (possible_addr != NULL) {
		return_sock = socket(
			possible_addr->ai_family,
			possible_addr->ai_socktype,
			possible_addr->ai_protocol
		);
		if (return_sock == -1) {
			possible_addr = possible_addr->ai_next;
			continue;
		}
		if (connect(return_sock,
					possible_addr->ai_addr,
					possible_addr->ai_addrlen) == -1) {
			close(return_sock);
			possible_addr = possible_addr->ai_next;
			continue;
		}
		break;
	}
	if (possible_addr == NULL)
		error(1, errno, "Cannot establish connection");

	return return_sock;
}


int
main(int argc, char **argv)
{
	struct arguments inputs = {
		.address = NULL,
		.port = 0
	};
	int conn_sok;

	if (populate_arguments(argc, argv, &inputs))
		return 1;

	conn_sok = get_conn_sok(inputs.address, inputs.port);
	printf("Connection established\n");

	if (close(conn_sok) != 0)
		error(1, errno, "Error closing connection socket");
	return 0;
}
