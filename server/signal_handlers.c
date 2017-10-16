#include "signal_handlers.h" 
#include "main_events.h"


void
handle_control_c(int signal)
{
	tear_down_main_events();	
	if (close(listening_sok) == -1)
		error(1, errno, "Error closing listening socket");
	printf("Exiting...\n");
	exit(0);
}
