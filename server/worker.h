#ifndef _WEBSOCKET_WORKER_H
#define _WEBSOCKET_WORKER_H


int *write_ends, worker_count;


void set_up_workers(void);
void tear_down_workers(void);


#endif
