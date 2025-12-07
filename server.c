#include <memory.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT "8000"
#define BACKLOG 20
#define BUF_SIZE 4096

struct client_data
{
  int fd;
};

void* handle_client(void* arg);

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

  // Collect address info for creating a IPv4/IPv6 TCP socket
  // for listening on PORT on all interfaces (0.0.0.0:PORT)
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_flags = AI_PASSIVE;     // all interfaces (loopback, WiFi, etc.)
  hints.ai_family = AF_UNSPEC;     // IPv4 or IPv6
  hints.ai_socktype = SOCK_STREAM; // TCP socket

  // NULL hostname for local machine
  struct addrinfo* addrs;
  if (getaddrinfo(NULL, PORT, &hints, &addrs) != 0)
  {
    fprintf(stderr, "No valid address found.\n");
    exit(1);
  }

  struct addrinfo* a;
  int socketfd;
  for (a = addrs; a != NULL; a = a->ai_next)
  {
    if ((socketfd = socket(a->ai_family, a->ai_socktype, a->ai_protocol)) == -1)
      continue;

    if (bind(socketfd, a->ai_addr, a->ai_addrlen) != 0)
    {
      fprintf(stderr, "Bind failed, trying again...\n");
      close(socketfd);
      continue;
    }

    break;
  }

  if (a == NULL)
  {
    fprintf(stderr, "Failed to bind.\n");
    exit(1);
  }

  freeaddrinfo(addrs);

  if (listen(socketfd, BACKLOG) != 0)
  {
    fprintf(stderr, "Failed to listen\n");
    exit(1);
  }

  printf("Server listening on %s...\n", PORT);

  while (1)
  {
    int clientfd;
    struct sockaddr client_addr;
    socklen_t size;
    if ((clientfd = accept(socketfd, &client_addr, &size)) == -1)
    {
      fprintf(stderr, "Failure to accept client.\n");
      continue;
    }

    struct client_data* data = malloc(sizeof(struct client_data));
    data->fd = clientfd;

    pthread_t thread;
    if ((pthread_create(&thread, NULL, handle_client, (void*)data)) != 0)
    {
      fprintf(stderr, "Failure to spawn thread.\n");
      close(clientfd);
      free(data);
      continue;
    }
    pthread_detach(thread);
  }
  close(socketfd);
}

void* handle_client(void* arg)
{
  struct client_data* data = (struct client_data*)arg;
  int fd = data->fd;
  char buf[BUF_SIZE];
  int flags = 0;
  int bytes_read = recv(fd, &buf, BUF_SIZE, flags);
  if (bytes_read == 0)
  {
    // Client closed connection
    close(fd);
    return NULL;
  }
  buf[bytes_read] = '\0';

  printf("Data received: %s\n", buf);

  int bytes_sent = send(fd, &buf, bytes_read, flags);
  if (bytes_sent != bytes_read)
    fprintf(stderr, "Could not send all data!\n");
  if (bytes_sent == 0)
    fprintf(stderr, "Failure to send.\n");

  printf("Data sent\n");

  close(fd);
  free(data);
  return NULL;
}
