#include "queue.h"

#include <stdio.h>

#include "ui.h"

void init_queue(queue_t* queue) { queue->front = queue->rear = queue->size = 0; }

int is_queue_empty(queue_t* queue) { return queue->front == queue->rear; }

void enqueue(queue_t* queue, update_type_t request) {
  if ((queue->rear + 1) % QUEUE_SIZE == queue->front) {
    return;
  }
  queue->requests[queue->rear] = request;
  queue->rear = (queue->rear + 1) % QUEUE_SIZE;
  queue->size++;
}

update_type_t dequeue(queue_t* queue) {
  if (is_queue_empty(queue)) {
    return UNOOP;
  }
  update_type_t request = queue->requests[queue->front];
  queue->front = (queue->front + 1) % QUEUE_SIZE;
  queue->size--;
  return request;
}
