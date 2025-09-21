// client_lib.h
#ifndef CLIENT_LIB_H
#define CLIENT_LIB_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "common.h"

typedef int socket_t;

typedef struct {
  socket_t sockfd;
  struct sockaddr_in server_addr;
  int is_connected;
  bool correct;

  uint16_t src_port;
  uint16_t dest_port;
  const char* if_name;
} ClientSession;

typedef ClientSession* (*client_init_t)(uint16_t src_port, const char* if_name);
typedef int (*client_connect_t)(ClientSession* session, const char* server_addr, uint16_t port);
typedef int (*client_send_message_t)(ClientSession* session, const char* message);
typedef int (*client_receive_t)(ClientSession* session, char* buf, size_t buf_size, struct sockaddr_in* src_addr,
                                uint16_t* src_port);

typedef struct {
  client_init_t init;
  client_connect_t connect;
  client_send_message_t send_message;
  client_receive_t receive;
} operations_t;

extern const operations_t export;

#define CLIENT_SUCCESS 0
#define CLIENT_ERROR -1
#define CLIENT_BUFFER_TOO_SMALL -2
#define CLIENT_NOT_OURS -3
#endif  // CLIENT_LIB_H