#ifndef COMMON_H_
#define COMMON_H_

#define error_exit(msg) \
  do {                  \
    perror(msg);        \
    exit(EXIT_FAILURE); \
  } while (0)

#define BUFFER_SIZE 128
#define TIME_REQUEST "GET_TIME"
#define R_Q_SIZE 5
#define MAX_CLIENTS 10

#endif  // COMMON_H_