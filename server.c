#include <memory.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT "8000"
#define BACKLOG 20

int main(int argc, char** argv)
{

  // struct addrinfo
  // {
  //   int ai_flags;             /* input flags */
  //   int ai_family;            /* protocol family for socket */
  //   int ai_socktype;          /* socket type */
  //   int ai_protocol;          /* protocol for socket */
  //   socklen_t ai_addrlen;     /* length of socket-address */
  //   struct sockaddr *ai_addr; /* socket-address for socket */
  //   char *ai_canonname;       /* canonical name for service location */
  //   struct addrinfo *ai_next; /* pointer to next in list */
  // };

  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_flags = AI_PASSIVE;     // use own IP
  hints.ai_family = AF_INET;       // IPv6
  hints.ai_socktype = SOCK_STREAM; // TCP socket

  struct addrinfo* addrs;
  if (getaddrinfo(NULL, PORT, &hints, &addrs) != 0)
  {
    fprintf(stderr, "Could not find a valid address\n");
    exit(1);
  }

  struct addrinfo* a;
  int socketfd;
  for (a = addrs; a != NULL; a = a->ai_next)
  {
    if ((socketfd = socket(a->ai_family, a->ai_socktype, a->ai_protocol)) == -1)
      continue;

    printf("Socket created!\n");
    if (bind(socketfd, a->ai_addr, a->ai_addrlen) != 0)
    {
      fprintf(stderr, "bind fail, continuing\n");
      close(socketfd);
      continue;
    }

    break;
  }

  if (a == NULL)
  {
    // bind failed
    fprintf(stderr, "Failed to bind\n");
    exit(1);
  }

  freeaddrinfo(addrs);

  if (listen(socketfd, BACKLOG) != 0)
  {
    fprintf(stderr, "Failed to listen\n");
    exit(1);
  }

  printf("Server listening on port %s...\n", PORT);

  while (1)
  {
    int newfd;
    struct sockaddr client_addr;
    socklen_t size;
    if ((newfd = accept(socketfd, &client_addr, &size)) == -1)
    {
      fprintf(stderr, "Failure to accept client.\n");
      continue;
    }

    char buf[100];
    int flags = 0;
    int bytes_read = recv(newfd, &buf, 100, flags);
    if (bytes_read == 0)
    {
      // client closed connection
      close(newfd);
      continue;
    }
    buf[bytes_read] = '\0';

    int bytes_sent = send(newfd, &buf, bytes_read, flags);
    if (bytes_sent != bytes_read)
      fprintf(stderr, "Could not send all data!\n");
    if (bytes_sent == 0)
      fprintf(stderr, "Failure to send.\n");
    close(newfd);
  }
  close(socketfd);
}
