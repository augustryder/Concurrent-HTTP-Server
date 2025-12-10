
all: server.o cqueue.o
	clang server.o cqueue.o -o server

objfiles: server.c cqueue.c
	clang -pthread server.c cqueue.c -o server.o cqueue.o
clean:
	rm -f server server.o cqueue.o
