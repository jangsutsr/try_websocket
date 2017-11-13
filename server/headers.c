#define _GNU_SOURCE
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <search.h>
#include "headers.h"


void
init_headers(struct request_headers **headers, int capacity)
{
	*headers = calloc(1, sizeof(struct request_headers));
	(*headers)->table = calloc(1, sizeof(struct hsearch_data));
	hcreate_r(capacity, (*headers)->table);
	(*headers)->capacity = capacity;
	(*headers)->count = 0;
	(*headers)->store = calloc(capacity / 2, sizeof(ENTRY));
}


void
destroy_headers(struct request_headers *headers)
{
	int i;

	for (i = 0; i < headers->count; ++i) {
		free((headers->store)[i].key);
		free((headers->store)[i].data);
	}
	free(headers->store);
	hdestroy_r(headers->table);
	free(headers->table);
	free(headers);
}


char *
get_header(struct request_headers *headers, char *key)
{
	ENTRY search = {
		.key = key,
		.data = NULL
	}, *ret;

	if (!hsearch_r(search, FIND, &ret, headers->table))
		return NULL;
	return (char *)(ret->data);
}


int
insert_header(struct request_headers *headers, char *key, char *val)
{
	ENTRY search = {
		.key = key,
		.data = val
	}, *ret;

	if (hsearch_r(search, FIND, &ret, headers->table))
		return 1;
	if (headers->count >= headers->capacity / 2)
		return 1;
	if (!hsearch_r(search, ENTER, &ret, headers->table))
		return 1;
	headers->store[headers->count].key = key;
	headers->store[headers->count].data = val;
	++(headers->count);
	return 0;
}


void
display_headers(struct request_headers *headers)
{
	int i;

	for (i = 0; i < headers->count; ++i)
		printf("header: %s\nvalue: %s\n", headers->store[i].key, (char *)headers->store[i].data);
}


int
process_header_line(char *buffer, ssize_t start, ssize_t end, struct request_headers *headers)
{
	ssize_t left = start, right = start;
	char *key, *val;

	while (right < end && buffer[right] != ':')
		++right;
	key = calloc(right - left + 1, sizeof(char));
	memcpy(key, buffer + left, right - left);
	++right;
	while (right < end && buffer[right] == ' ')
		++right;
	left = right;
	while (right < end && buffer[right] != '\r' && buffer[right] != '\n')
		++right;
	val = calloc(right - left + 1, sizeof(char));
	memcpy(val, buffer + left, right - left);
	if (strlen(key) && strlen(val)) {
		insert_header(headers, key, val);
	} else {
		free(key);
		free(val);
	}
	return 0;
}

