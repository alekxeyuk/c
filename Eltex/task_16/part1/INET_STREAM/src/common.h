#ifndef COMMON_H_
#define COMMON_H_

#define error_exit(msg) \
  do {                  \
    perror(msg);        \
    exit(EXIT_FAILURE); \
  } while (0)

#define MSG_SIZE 16
#define SERVER_PORT 2025

#endif  // COMMON_H_