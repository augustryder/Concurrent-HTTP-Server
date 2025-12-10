#include "threadpool.h"
#include <fcntl.h>
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
#define BUFFER_SIZE 4096
#define MAX_PATH_LEN 256

#define THREADPOOL 0
#define NO_CONCURRENCY 0
#define THREAD_PER_REQUEST 1

struct client_data
{
  int fd;
};

#if THREAD_PER_REQUEST
void* handle_client(void* arg);
#endif
#if THREADPOOL || NO_CONCURRENCY
void* handle_client(int fd);
#endif
int parse_header(const char* header, char* route);
int send_response(int fd, char* response_header, FILE* body);

static pthread_mutex_t print_lock;

int main(int argc, char** argv)
{
  pthread_mutex_init(&print_lock, NULL);
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
    // Try to initialize socket
    if ((socketfd = socket(a->ai_family, a->ai_socktype, a->ai_protocol)) == -1)
      continue;

    // Try to bind socket to address
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

  // Set socket as listening
  if (listen(socketfd, BACKLOG) != 0)
  {
    fprintf(stderr, "Failed to listen\n");
    exit(1);
  }

  printf("Server listening on %s...\n", PORT);

#if THREADPOOL
  threadpool_t threadpool;
  threadpool_init(&threadpool, handle_client);
#endif

  // Handle requests indefinitely
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

#if THREADPOOL
    threadpool_enqueue_request(&threadpool, clientfd);
#endif

#if THREAD_PER_REQUEST
    struct client_data* data = malloc(sizeof(struct client_data));
    if (data == NULL)
    {
      fprintf(stderr, "Failed to allocate memory\n");
      close(clientfd);
      continue;
    }
    data->fd = clientfd;

    // Spawn new thread for each request
    pthread_t thread;
    if ((pthread_create(&thread, NULL, handle_client, (void*)data)) != 0)
    {
      fprintf(stderr, "Failed to spawn thread.\n");
      close(clientfd);
      free(data);
      continue;
    }
    pthread_detach(thread);
#endif

#if NO_CONCURRENCY
    handle_client(clientfd);
#endif
  }
  pthread_mutex_destroy(&print_lock);
  close(socketfd);
}

#if THREAD_PER_REQUEST
void* handle_client(void* arg)
#endif
#if THREADPOOL || NO_CONCURRENCY
    void* handle_client(int fd)
#endif
{
#if THREAD_PER_REQUEST
  struct client_data* data = (struct client_data*)arg;
  int fd = data->fd;
#endif
  // Receive request
  char req[BUFFER_SIZE];
  int bytes_read = recv(fd, &req, BUFFER_SIZE - 1, 0);
  if (bytes_read < 1)
  {
    // free(data);
    close(fd);
    return NULL;
  }
  req[bytes_read] = '\0';

  // Parse path
  char route[MAX_PATH_LEN];
  if ((parse_header(req, route)) != 0)
  {
    fprintf(stderr, "Error in parsing header: %s\n", req);
    // free(data);
    close(fd);
    return NULL;
  }

  if (strcmp(route, "/") == 0)
  {
    // Open HTML file and get size
    FILE* f = fopen("www/index.html", "rb");
    if (f == NULL)
    {
      fprintf(stderr, "Could not open index.html.\n");
      // free(data);
      close(fd);
      return NULL;
    }
    long fsize;
    if (fseek(f, 0, SEEK_END) != 0 || (fsize = ftell(f)) == -1 ||
        fseek(f, 0, SEEK_SET) != 0)
    {
      fprintf(stderr, "Failed to get file size.\n");
      // free(data);
      close(fd);
      fclose(f);
      return NULL;
    }

    // Simulate I/0 operation (for testing multi-threaded performance)
    usleep(100000);

    // Format response header
    char response_header[BUFFER_SIZE];
    snprintf(response_header, BUFFER_SIZE,
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: text/html; charset=UTF-8\r\n"
             "Content-Length: %ld\r\n"
             "\r\n",
             fsize);
    response_header[BUFFER_SIZE - 1] = '\0';

    int bytes_sent;
    if ((bytes_sent = send_response(fd, response_header, f)) == -1)
    {
      fprintf(stderr, "Failed to send response.\n");
      // free(data);
      close(fd);
      fclose(f);
      return NULL;
    }

    if (bytes_sent < strlen(response_header) + fsize)
      fprintf(stderr, "Failed to send all data.\n");

    pthread_mutex_lock(&print_lock);
    printf("Served: %s (%d bytes)\n", route, bytes_sent);
    pthread_mutex_unlock(&print_lock);

    fclose(f);
  }
  else if (strcmp(route, "/image") == 0)
  {
    // Open image file and get size
    FILE* f = fopen("www/blah.jpeg", "rb");
    if (f == NULL)
    {
      fprintf(stderr, "Could not open blah.jpeg.\n");
      // free(data);
      close(fd);
      return NULL;
    }
    long fsize;
    if (fseek(f, 0, SEEK_END) != 0 || (fsize = ftell(f)) == -1 ||
        fseek(f, 0, SEEK_SET) != 0)
    {
      fprintf(stderr, "Failed to get file size.\n");
      // free(data);
      close(fd);
      fclose(f);
      return NULL;
    }

    // Format response header
    char response_header[BUFFER_SIZE];
    snprintf(response_header, BUFFER_SIZE,
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: image/jpeg\r\n"
             "Content-Length: %ld\r\n"
             "\r\n",
             fsize);
    response_header[BUFFER_SIZE - 1] = '\0';

    int bytes_sent;
    if ((bytes_sent = send_response(fd, response_header, f)) == -1)
    {
      fprintf(stderr, "Failed to send response.\n");
      // free(data);
      close(fd);
      fclose(f);
      return NULL;
    }

    if (bytes_sent != strlen(response_header) + fsize)
      fprintf(stderr, "Failed to send all data.\n");

    pthread_mutex_lock(&print_lock);
    printf("Served: %s (%d bytes)\n", route, bytes_sent);
    pthread_mutex_unlock(&print_lock);

    fclose(f);
  }
  else
  {
    // 404 message
    char response_header[BUFFER_SIZE];
    snprintf(response_header, BUFFER_SIZE,
             "HTTP/1.1 404 Not Found\r\n"
             "Content-Type: text/html; charset=UTF-8\r\n"
             "Content-Length: 0\r\n"
             "\r\n");
    response_header[BUFFER_SIZE - 1] = '\0';

    if (send(fd, response_header, strlen(response_header), 0) == -1)
    {
      fprintf(stderr, "Failed to send response header.\n");
      // free(data);
      close(fd);
      return NULL;
    }

    pthread_mutex_lock(&print_lock);
    printf("404 Not Found: %s\n", route);
    pthread_mutex_unlock(&print_lock);
  }

  close(fd);
  // free(data);
  return NULL;
}

int parse_header(const char* header, char* route)
{
  char method[4], protocol[5];
  sscanf(header, "%3s %255s %4s", method, route, protocol);
  method[3] = '\0';
  route[MAX_PATH_LEN - 1] = '\0';
  protocol[4] = '\0';

  if (strcmp(method, "GET") != 0)
    return -1;
  if (strcmp(protocol, "HTTP") != 0)
    return -1;
  if (route[0] != '/')
    return -1;

  return 0;
}

int send_response(int fd, char* response_header, FILE* body)
{
  int bytes_sent;
  int header_size = strlen(response_header);
  if ((bytes_sent = send(fd, response_header, header_size, 0)) == -1)
    return -1;

  char buf[BUFFER_SIZE];
  int n;
  while ((n = fread(buf, 1, BUFFER_SIZE, body)) > 0)
  {
    int total_sent = 0;
    while (total_sent < n)
    {
      int sent = send(fd, buf + total_sent, n - total_sent, 0);
      if (sent == -1)
        return -1;
      total_sent += sent;
    }
    bytes_sent += total_sent;
  }
  if (ferror(body))
    return -1;
  return bytes_sent;
}
