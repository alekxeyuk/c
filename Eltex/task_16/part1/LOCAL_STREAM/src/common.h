#ifndef COMMON_H_
#define COMMON_H_

#define error_exit(msg) \
  do {                  \
    perror(msg);        \
    exit(EXIT_FAILURE); \
  } while (0)

#define MSG_SIZE 16
#define SERVER_SOCKET_NAME "/tmp/app_server.socket"

#endif  // COMMON_H_