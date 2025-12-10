#pragma once

#include "pthread.h"

typedef struct _node_t
{
  int value;
  struct _node_t* next;
} node_t;

typedef struct _cqueue_t
{
  node_t* head;
  node_t* tail;
  pthread_mutex_t head_lock;
  pthread_mutex_t tail_lock;
  pthread_cond_t not_empty;
  int shutdown;
} cqueue_t;

void cqueue_init(cqueue_t* cq);
void cqueue_enqueue(cqueue_t* cq, int value);
int cqueue_dequeue(cqueue_t* cq, int* ret_value);
void cqueue_shutdown(cqueue_t* cq);
void cqueue_free(cqueue_t* cq);
