all: server argument_parser main_events worker headers start_line
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

start_line:
	gcc -Wall -g -c start_line.c -o bin/start_line.o

clean:
	rm bin/*.o
