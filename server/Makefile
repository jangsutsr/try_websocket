all: server
	gcc -Wall -g -levent -o bin/server *.o

server: argument_parser main_events signal_handlers
	gcc -Wall -g -c server.c

argument_parser:
	gcc -Wall -g -c argument_parser.c

main_events:
	gcc -Wall -g -c main_events.c

signal_handlers:
	gcc -Wall -g -c signal_handlers.c

clean:
	rm *.o
