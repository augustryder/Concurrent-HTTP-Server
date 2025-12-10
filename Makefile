
all: server.o cqueue.o threadpool.o
	clang server.o cqueue.o threadpool.o -o server

objfiles: server.c cqueue.c threadpool.c
	clang -pthread server.c cqueue.c threadpool.c -o server.o cqueue.o threadpool.o

clean:
	rm -f server server.o cqueue.o threadpool.o
