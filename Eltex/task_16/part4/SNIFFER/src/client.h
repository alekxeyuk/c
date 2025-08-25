#ifndef CLIENT_H_
#define CLIENT_H_

#define error_exit(msg) \
  do {                  \
    perror(msg);        \
    exit(EXIT_FAILURE); \
  } while (0)

#define BUFFER_SIZE 128

#endif  // CLIENT_H_