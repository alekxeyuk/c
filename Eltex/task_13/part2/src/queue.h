#ifndef QUEUE_H_
#define QUEUE_H_

#include "ui.h"

#define QUEUE_SIZE 64

typedef struct {
  update_type_t requests[QUEUE_SIZE];
  int front, rear;
  int size;
} queue_t;

void init_queue(queue_t *queue);
int is_queue_empty(queue_t *queue);
void enqueue(queue_t *queue, update_type_t request);
update_type_t dequeue(queue_t *queue);

#endif  // QUEUE_H_