#include <errno.h>
#include <error.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <unistd.h>
#include <getopt.h>

#include <sys/sysinfo.h>

#include <sys/socket.h>
#include <netdb.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <signal.h>

#include <event2/event.h>

int listening_sok;
