#ifndef COMMON_H_
#define COMMON_H_

#define error_exit(msg) \
  do {                  \
    perror(msg);        \
    exit(-1);           \
  } while (0)

#define BUFFER_SIZE 128
#define STOP_MESSAGE "STOP"

#endif  // COMMON_H_