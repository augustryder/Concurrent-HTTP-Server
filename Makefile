
all: server.c
	clang server.c -o server

clean:
	rm -f server
