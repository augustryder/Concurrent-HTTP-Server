#include "cqueue.h"
#include "pthread.h"
#include "stdio.h"
#include "stdlib.h"

void cqueue_init(cqueue_t* cq)
{
  // create a dummy sentinel node in order to
  // separate enqueue and dequeue operations;
  // head and tail never point to the same node
  // when the queue is non-empty to prevent race
  // conditions when enqueuing and dequeuing simultaneously
  node_t* sentinel = (node_t*)malloc(sizeof(node_t));
  if (sentinel == NULL)
  {
    fprintf(stderr, "Failed to initialize cqueue.\n");
    exit(1);
  }
  sentinel->value = 0;
  sentinel->next = NULL;

  cq->head = sentinel;
  cq->tail = sentinel;

  pthread_mutex_init(&cq->head_lock, NULL);
  pthread_mutex_init(&cq->tail_lock, NULL);
  pthread_cond_init(&cq->not_empty, NULL);
  cq->shutdown = 0;
}

void cqueue_enqueue(cqueue_t* cq, int value)
{
  node_t* new_node = (node_t*)malloc(sizeof(node_t));
  if (new_node == NULL)
  {
    fprintf(stderr, "Out of memory (cqueue).\n");
    exit(1);
  }
  new_node->value = value;
  new_node->next = NULL;

  pthread_mutex_lock(&cq->tail_lock);
  cq->tail->next = new_node;
  cq->tail = new_node;
  pthread_mutex_unlock(&cq->tail_lock);
  pthread_cond_signal(&cq->not_empty);
}

int cqueue_dequeue(cqueue_t* cq, int* ret_value)
{
  pthread_mutex_lock(&cq->head_lock);
  while (!cq->shutdown && cq->head->next == NULL)
    pthread_cond_wait(&cq->not_empty, &cq->head_lock);

  if (cq->shutdown && cq->head->next == NULL)
  {
    pthread_mutex_unlock(&cq->head_lock);
    return -1;
  }

  node_t* sentinel = cq->head;
  *ret_value = sentinel->next->value;
  cq->head = sentinel->next;

  free(sentinel);
  pthread_mutex_unlock(&cq->head_lock);
  return 0;
}

void cqueue_shutdown(cqueue_t* cq)
{
  pthread_mutex_lock(&cq->head_lock);
  cq->shutdown = 1;
  pthread_cond_broadcast(&cq->not_empty); // wake all threads
  pthread_mutex_unlock(&cq->head_lock);
}

void cqueue_free(cqueue_t* cq)
{
  int dummy;
  while (cqueue_dequeue(cq, &dummy) != -1)
    ;
  free(cq->head);
  pthread_mutex_destroy(&cq->head_lock);
  pthread_mutex_destroy(&cq->tail_lock);
  pthread_cond_destroy(&cq->not_empty);
}
