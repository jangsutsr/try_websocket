#ifndef _WEBSOCKET_WORKER_H
#define _WEBSOCKET_WORKER_H

#include <pthread.h>
#include <event2/event.h>

struct worker_pipe_callback_input {
	struct event_base *worker_event_base;
	struct event **pipe_event;
};

struct worker_thread_input {
	int read_end;
	int thread_index;
	struct event_base *event_base;
	struct event *pipe_event;
	struct worker_pipe_callback_input pipe_callback;
};

struct worker {
	pthread_t thread;
	int write_end;
	struct worker_thread_input thread_input;
};

int worker_set_up(struct worker **workers);
void worker_tear_down(struct worker *workers, int worker_count);

#endif
