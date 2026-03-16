.PHONY: run clean all

all: hello

hello: main.o message.o
	@gcc main.o message.o -o hello

main.o: main.c message.h
	@gcc -c main.c

message.o: message.c message.h
	@gcc -c message.c

clean:
	@rm -f *.o hello

run: hello
	@./hello
