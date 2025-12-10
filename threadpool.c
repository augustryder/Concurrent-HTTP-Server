#include "threadpool.h"
#include "cqueue.h"
#include "stdio.h"
#include "stdlib.h"

static void* worker_thread(void* arg)
{
  threadpool_t* tp = (threadpool_t*)arg;
  while (1)
  {
    int val;
    if (cqueue_dequeue(&tp->cq, &val) != 0)
      break; // shutdown signaled
    tp->handler(val);
  }
  return NULL;
}

void threadpool_init(threadpool_t* tp, void* (*handler)(int))
{
  tp->handler = handler;
  cqueue_init(&tp->cq);
  for (int i = 0; i < THREADPOOL_SIZE; i++)
  {
    int rc = pthread_create(&(tp->threads[i]), NULL, worker_thread, (void*)tp);
    if (rc != 0)
    {
      fprintf(stderr, "Failed to create thread.\n");
      exit(1);
    }
  }
}

void threadpool_enqueue_request(threadpool_t* tp, int value)
{
  cqueue_enqueue(&tp->cq, value);
}

void threadpool_shutdown(threadpool_t* tp)
{
  cqueue_shutdown(&tp->cq);

  for (int i = 0; i < THREADPOOL_SIZE; i++)
    pthread_join(tp->threads[i], NULL);

  cqueue_free(&tp->cq);
}
