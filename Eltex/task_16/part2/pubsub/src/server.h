#ifndef COMMON_H_
#define COMMON_H_

#include <netinet/in.h>
#include <pthread.h>

#define error_exit(msg) \
  do {                  \
    perror(msg);        \
    exit(EXIT_FAILURE); \
  } while (0)

#define QUEUE_NAME "/pubsub"
#define QUEUE_MSIZE 10
#define BUFFER_SIZE 128
#define TIME_REQUEST "GET_TIME"
#define MAX_THREADS 10

typedef struct worker {
  struct sockaddr_in addr;
  socklen_t addr_len;
  size_t data_len;
  char data[BUFFER_SIZE];
} udp_msg_t;

#endif  // COMMON_H_