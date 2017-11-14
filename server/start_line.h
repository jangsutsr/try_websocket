#ifndef _WEBSOCKET_START_LINE_H
#define _WEBSOCKET_START_LINE_H

#include <sys/types.h>

#define FREE_IF_NOT_NULL(x) if(x!=NULL){free(x);x=NULL;}

struct request_start_line {
	char *method;
	char *request_target;
	char *http_version;
};

int
req_start_line_init(
	char *buffer, ssize_t start, ssize_t end, struct request_start_line *req_start
);
void req_start_line_destroy(struct request_start_line *req_start);

#endif
