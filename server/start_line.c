#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "start_line.h"


int
req_start_line_init(char *buffer, ssize_t start, ssize_t end, struct request_start_line *req_start)
{
	ssize_t left = start, right = start;

	req_start->method = NULL;
	req_start->request_target = NULL;
	req_start->http_version = NULL;
	while (left < end && buffer[left] != ' ') {
		while (right < end && buffer[right] != ' ')
			++right;
		req_start->method = calloc(right - left + 1, sizeof(char));
		memcpy(req_start->method, buffer + left, sizeof(char) * (right - left));
		left = right;
	}
	if (left >= end)
		return 1;
	++left;
	++right;
	while (left < end && buffer[left] != ' ') {
		while (right < end && buffer[right] != ' ')
			++right;
		req_start->request_target = calloc(right - left + 1, sizeof(char));
		memcpy(req_start->request_target, buffer + left, sizeof(char) * (right - left));
		left = right;
	}
	if (left >= end)
		return 1;
	++left;
	++right;
	while (left < end && buffer[left] != '\r' && buffer[left] != '\n') {
		while (right < end && buffer[right] != '\r' && buffer[right] != '\n')
			++right;
		req_start->http_version = calloc(right - left + 1, sizeof(char));
		memcpy(req_start->http_version, buffer + left, sizeof(char) * (right - left));
		left = right;
	}
	return 0;
}


void
req_start_line_destroy(struct request_start_line *req_start)
{
	FREE_IF_NOT_NULL(req_start->method)
	FREE_IF_NOT_NULL(req_start->request_target)
	FREE_IF_NOT_NULL(req_start->http_version)
	FREE_IF_NOT_NULL(req_start)
}
