#include <errno.h>
#include <error.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <event2/event.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "worker.h"
#include "headers.h"


#define FREE_IF_NOT_NULL(x) if(x!=NULL){free(x);x=NULL;}


struct thread_input {
	int read_end;
	int thread_index;
};


struct pipe_callback_input {
	struct event_base *worker_event_base;
	struct event **pipe_event;
};

struct request_start_line {
	char *method;
	char *request_target;
	char *http_version;
};


int *write_ends, worker_count;
pthread_t *threads;
struct thread_input *worker_inputs;
struct event_base **worker_event_bases;
struct event **pipe_events;
int *pipes;


int
init_start_line(char *buffer, ssize_t start, ssize_t end, struct request_start_line *req_start)
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
destroy_start_line(struct request_start_line *req_start)
{
	FREE_IF_NOT_NULL(req_start->method)
	FREE_IF_NOT_NULL(req_start->request_target)
	FREE_IF_NOT_NULL(req_start->http_version)
	FREE_IF_NOT_NULL(req_start)
}


int
set_up_ws_connection(int sok)
{
	char *buffer = calloc(1024, sizeof(char));
	ssize_t recv_bytes, left = 0, right = 0;
	int line_count = 0;
	struct request_start_line *req_start = calloc(1, sizeof(struct request_start_line));
	struct request_headers *headers;

	init_headers(&headers, 100);
	while ((recv_bytes = recv(sok, buffer + right, 1023 - right, MSG_DONTWAIT)) > 0) {
		left = 0;
		while (1) {
			while (right < recv_bytes && buffer[right] != '\n')
				++right;
			if (right < recv_bytes) {
				++right;
				if (line_count == 0 && init_start_line(buffer, left, right, req_start)) {
					destroy_start_line(req_start);
					FREE_IF_NOT_NULL(buffer)
					return 1;
				} else if (line_count > 0 && process_header_line(buffer, left, right, headers)) {
					destroy_headers(headers);
					destroy_start_line(req_start);
					FREE_IF_NOT_NULL(buffer)
					return 1;
				}
				left = right;
				++line_count;
			} else {
				memmove(buffer, buffer + left, right - left);
				memset(buffer + right - left, '\0', 1024 - (right - left));
				right -= left;
				break;
			}
		}
	}
	send(sok, "hehe\r\n\r\n", 8 * sizeof(char), 0);
	display_headers(headers);
	destroy_headers(headers);
	destroy_start_line(req_start);
	FREE_IF_NOT_NULL(buffer)
	return 0;
}


void
pipe_callback(evutil_socket_t read_end, short what, void *args)
{
	struct pipe_callback_input *pipe_callback_input = args;
	char flag;
	int sok;
	
	read(read_end, &flag, sizeof(char));
	switch (flag) {
	case 'c':
		read(read_end, &sok, sizeof(int));
		if (set_up_ws_connection(sok) != 0)
			printf("hehe\n");
		close(sok);
		break;
	case 'e':
	default:
		event_del(*(pipe_callback_input->pipe_event));
		break;
	}
}


void
tear_down_worker_event(struct event_base *worker_event_base, struct event *pipe_event)
{
	if (event_base_loopexit(worker_event_base, NULL) == -1)
		error(1, errno, "Error exiting listening event loop");
	event_free(pipe_event);
	event_base_free(worker_event_base);
}


void *
worker_main(void *input)
{
	struct thread_input *thread_input = (struct thread_input *)input;
	struct pipe_callback_input pipe_callback_input;
	
	pipe_callback_input.pipe_event = pipe_events + thread_input->thread_index;
	worker_event_bases[thread_input->thread_index] = event_base_new();
	pipe_callback_input.worker_event_base = worker_event_bases[thread_input->thread_index];
	pipe_events[thread_input->thread_index] = event_new(
		worker_event_bases[thread_input->thread_index], thread_input->read_end,
		EV_PERSIST | EV_READ, &pipe_callback, &pipe_callback_input
	);
	event_add(pipe_events[thread_input->thread_index], NULL);
	event_base_loop(worker_event_bases[thread_input->thread_index], 0);
	tear_down_worker_event(worker_event_bases[thread_input->thread_index],
						   pipe_events[thread_input->thread_index]);
	return NULL;
}


void
set_up_workers(void)
{
	int i;

	worker_count = get_nprocs_conf();
	threads = calloc(worker_count, sizeof(pthread_t));
	worker_inputs = calloc(worker_count, sizeof(struct thread_input));
	worker_event_bases = calloc(worker_count, sizeof(struct event_base *));
	pipe_events = calloc(worker_count, sizeof(struct event *));
	pipes = calloc(2 * worker_count, sizeof(int));
	write_ends = calloc(worker_count, sizeof(int));
	for (i = 0; i < worker_count; ++i) {
		worker_inputs[i].thread_index = i;
		pipe(pipes + 2 * i);
		worker_inputs[i].read_end = pipes[2 * i];
		write_ends[i] = pipes[2 * i + 1];
		pthread_create(threads + i, NULL, &worker_main, worker_inputs + i);
	}
}


void
tear_down_workers(void)
{
	int i;
	char flag = 'e'; 

	for (i = 0; i < worker_count; ++i) {
		write(write_ends[i], &flag, sizeof(char));
		pthread_join(threads[i], NULL);
	}
	free(write_ends);
	free(threads);
	free(worker_inputs);
	free(pipes);
	free(pipe_events);
	free(worker_event_bases);
}
