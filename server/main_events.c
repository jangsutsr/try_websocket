#include "main_events.h"


struct event_base *listening_event_base;


struct event *new_connection_event;


void
new_connection_callback(evutil_socket_t listening_sok,
						short what, void *args)
{
	int accepted_sok;

	accepted_sok = accept(listening_sok, NULL, NULL);
	if (accepted_sok != -1) {
		printf("Incoming request.\n");
		close(accepted_sok);
	}
}


void
set_up_main_events(int listening_sok)
{
	listening_event_base = event_base_new();
	new_connection_event = event_new(
		listening_event_base,
		listening_sok,
		EV_PERSIST | EV_READ,
		&new_connection_callback,
		NULL
	);
	event_add(new_connection_event, NULL);
}


void
run_main_events(void)
{
	event_base_loop(listening_event_base, 0);
}


void
tear_down_main_events(void)
{
	if (event_base_loopexit(listening_event_base, NULL) == -1)
		error(1, errno, "Error exiting listening event loop.\n");
	event_free(new_connection_event);
	event_base_free(listening_event_base);
}
