all: client
	gcc -Wall -g -o bin/client *.o

client: argument_parser
	gcc -Wall -g -c client.c

argument_parser:
	gcc -Wall -g -c argument_parser.c

clean:
	rm *.o
