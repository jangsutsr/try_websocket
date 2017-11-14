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
#include "start_line.h"
#include "headers.h"


#define FREE_IF_NOT_NULL(x) if(x!=NULL){free(x);x=NULL;}

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
				if (line_count == 0 && req_start_line_init(buffer, left, right, req_start)) {
					req_start_line_destroy(req_start);
					FREE_IF_NOT_NULL(buffer)
					return 1;
				} else if (line_count > 0 && process_header_line(buffer, left, right, headers)) {
					destroy_headers(headers);
					req_start_line_destroy(req_start);
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
	req_start_line_destroy(req_start);
	FREE_IF_NOT_NULL(buffer)
	return 0;
}


void
worker_pipe_callback(evutil_socket_t read_end, short what, void *args)
{
	struct worker_pipe_callback_input *pipe_callback_input = args;
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
worker_tear_down_event(struct event_base *worker_event_base, struct event *pipe_event)
{
	if (event_base_loopexit(worker_event_base, NULL) == -1)
		error(1, errno, "Error exiting listening event loop");
	event_free(pipe_event);
	event_base_free(worker_event_base);
}


void *
worker_main(void *input)
{
	struct worker_thread_input *thread_input = (struct worker_thread_input *)input;
	
	thread_input->event_base = event_base_new();
	thread_input->pipe_event = event_new(
		thread_input->event_base, thread_input->read_end, EV_PERSIST | EV_READ,
		&worker_pipe_callback, &(thread_input->pipe_callback)
	);
	(thread_input->pipe_callback).worker_event_base = thread_input->event_base;
	(thread_input->pipe_callback).pipe_event = &(thread_input->pipe_event);
	event_add(thread_input->pipe_event, NULL);
	event_base_loop(thread_input->event_base, 0);
	worker_tear_down_event(thread_input->event_base, thread_input->pipe_event);
	return NULL;
}


int
worker_set_up(struct worker **workers)
{
	int i, worker_count, p[2];

	worker_count = get_nprocs_conf();
	*workers = calloc(worker_count, sizeof(struct worker));
	for (i = 0; i < worker_count; ++i) {
		(*workers)[i].thread_input.thread_index = i;
		pipe(p);
		(*workers)[i].thread_input.read_end = p[0];
		(*workers)[i].write_end = p[1];
		pthread_create(
			&((*workers)[i].thread), NULL, &worker_main, &((*workers)[i].thread_input)
		);
	}
	return worker_count;
}


void
worker_tear_down(struct worker *workers, int worker_count)
{
	int i;
	char flag = 'e'; 

	for (i = 0; i < worker_count; ++i) {
		write(workers[i].write_end, &flag, sizeof(char));
		pthread_join(workers[i].thread, NULL);
	}
	free(workers);
}
