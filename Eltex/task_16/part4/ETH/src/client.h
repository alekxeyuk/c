#ifndef CLIENT_H_
#define CLIENT_H_

#include <arpa/inet.h>

#define error_exit(msg) \
  do {                  \
    perror(msg);        \
    exit(EXIT_FAILURE); \
  } while (0)

#define BUFFER_SIZE 128
#define MESSAGE "GET_TIME"

typedef struct {
  in_addr_t ip;
  unsigned char mac[6];
  int if_index;
} source_info;

#endif  // CLIENT_H_