all: server
	gcc -Wall -g -levent -pthread -o bin/server bin/*.o

server: argument_parser main_events signal_handlers worker
	gcc -Wall -g -c server.c -o bin/server.o

argument_parser:
	gcc -Wall -g -c argument_parser.c -o bin/argument_parser.o

main_events:
	gcc -Wall -g -c main_events.c -o bin/main_events.o

signal_handlers:
	gcc -Wall -g -c signal_handlers.c -o bin/signal_handlers.o

worker:
	gcc -Wall -g -c worker.c -o bin/worker.o

clean:
	rm bin/*.o
