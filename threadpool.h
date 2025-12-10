#pragma once

#include "cqueue.h"
#include "pthread.h"

#define THREADPOOL_SIZE 32

typedef struct _threadpool_t
{
  pthread_t threads[THREADPOOL_SIZE];
  cqueue_t cq;
  void* (*handler)(int);
} threadpool_t;

void threadpool_init(threadpool_t* tp, void* (*handler)(int));
void threadpool_enqueue_request(threadpool_t* tp, int value);
void threadpool_shutdown(threadpool_t* tp);
