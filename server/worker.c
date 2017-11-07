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


struct thread_input {
	int read_end;
	int thread_index;
};


struct pipe_callback_input {
	struct event_base *worker_event_base;
	struct event **pipe_event;
};


int *write_ends, worker_count;
pthread_t *threads;
struct thread_input *worker_inputs;
struct event_base **worker_event_bases;
struct event **pipe_events;
int *pipes;


int
set_up_ws_connection(int sok)
{
	char *buffer = calloc(1024, sizeof(char));
	ssize_t recv_bytes;

	while ((recv_bytes = recv(sok, buffer, 1023, MSG_DONTWAIT)) > 0) {
		printf("%s", buffer);
		memset(buffer, '\0', recv_bytes);
	}
	send(sok, "hehe\r\n\r\n", 8 * sizeof(char), 0);
	free(buffer);
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
