#ifndef COMMON_H_
#define COMMON_H_

#include <pthread.h>

#define error_exit(msg) \
  do {                  \
    perror(msg);        \
    exit(EXIT_FAILURE); \
  } while (0)

#define BUFFER_SIZE 128
#define TIME_REQUEST "GET_TIME"
#define R_Q_SIZE 5
#define MAX_THREADS 10

typedef struct worker {
  pthread_t thread;
  int id;
  int client_fd; /* -1 = no client */
  int busy;      /* 0 free, 1 busy */
  pthread_mutex_t mutex;
  pthread_cond_t cond;
} worker_t;

#endif  // COMMON_H_
