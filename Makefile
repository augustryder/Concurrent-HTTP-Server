
all: server.c
	clang -pthread server.c -o server

clean:
	rm -f server
