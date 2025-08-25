#ifndef COMMON_H_
#define COMMON_H_

#define error_exit(msg) \
  do {                  \
    perror(msg);        \
    exit(EXIT_FAILURE); \
  } while (0)

#define MSG_SIZE 32
#define PORT 5000
#define MULTICAST_ADDR "224.1.1.1"

#endif  // COMMON_H_