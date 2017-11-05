#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <event2/event.h>
#include <sys/sysinfo.h>
#include "worker.h"


struct thread_input {
	int read_end;
};


pthread_t *threads;
struct thread_input *worker_inputs;
struct event_base **worker_event_bases;
struct event **pipe_events;
int *pipes;


void *
worker_main(void *input)
{
	struct thread_input *thread_input = (struct thread_input *)input;

	printf("worker: %d\n", thread_input->read_end);
	return NULL;
}


int
set_up_workers(int **write_ends)
{
	int processor_count = get_nprocs_conf(), i;

	threads = calloc(processor_count, sizeof(pthread_t));
	worker_inputs = calloc(processor_count, sizeof(struct thread_input));
	worker_event_bases = calloc(processor_count, sizeof(struct event_base *));
	pipe_events = calloc(processor_count, sizeof(struct event *));
	pipes = calloc(2 * processor_count, sizeof(int));
	*write_ends = calloc(processor_count, sizeof(int));
	for (i = 0; i < processor_count; ++i) {
		pipe(pipes + 2 * i);
		worker_inputs[i].read_end = pipes[2 * i];
		(*write_ends)[i] = pipes[2 * i + 1];
		pthread_create(threads + i, NULL, &worker_main, worker_inputs + i);
	}
	return processor_count;
}


void
tear_down_workers(void)
{
	free(pipes);
	free(pipe_events);
	free(worker_event_bases);
	free(threads);
}
