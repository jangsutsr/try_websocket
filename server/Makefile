all: server argument_parser main_events worker headers
	gcc -Wall -g -levent -pthread -o bin/server bin/*.o

server:
	gcc -Wall -g -c server.c -o bin/server.o

argument_parser:
	gcc -Wall -g -c argument_parser.c -o bin/argument_parser.o

main_events:
	gcc -Wall -g -c main_events.c -o bin/main_events.o

worker:
	gcc -Wall -g -c worker.c -o bin/worker.o

headers:
	gcc -Wall -g -c headers.c -o bin/headers.o

clean:
	rm bin/*.o
