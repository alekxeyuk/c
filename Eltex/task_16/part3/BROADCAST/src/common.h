#ifndef COMMON_H_
#define COMMON_H_

#define error_exit(msg) \
  do {                  \
    perror(msg);        \
    exit(EXIT_FAILURE); \
  } while (0)

#define MSG_SIZE 32
#define BROADCAST_PORT 5000
#define RESPONSE_PORT 5001

#endif  // COMMON_H_