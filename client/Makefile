all: client
	gcc -Wall -g -o bin/client bin/*.o

client: argument_parser
	gcc -Wall -g -c client.c -o bin/client.o

argument_parser:
	gcc -Wall -g -c argument_parser.c -o bin/argument_parser.o

clean:
	rm bin/*.o
