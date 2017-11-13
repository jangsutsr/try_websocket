#ifndef _WEBSOCKET_HEADERS_H
#define _WEBSOCKET_HEADERS_H


#define _GNU_SOURCE
#include <sys/types.h>
#include <search.h>


struct request_headers {
	struct hsearch_data *table;
	ENTRY *store;
	int capacity;
	int count;
};

void init_headers(struct request_headers **headers, int capacity);
void destroy_headers(struct request_headers *headers);
char *get_header(struct request_headers *headers, char *key);
int insert_header(struct request_headers *headers, char *key, char *val);
void display_headers(struct request_headers *headers);
int process_header_line(char *buffer, ssize_t start, ssize_t end, struct request_headers *headers);


#endif
