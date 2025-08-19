#include <locale.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "common.h"

static int create_socket(unsigned short port) {
  struct sockaddr_in addr;
  int fd;

  if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) return -1;

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

  if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    close(fd);
    return -1;
  }

  return fd;
}

int main(void) {
  setlocale(LC_ALL, "C.utf8");

  int fd;
  struct sockaddr_in client;
  socklen_t client_len = sizeof(client);

  if ((fd = create_socket(SERVER_PORT)) < 0) error_exit("socket");

  char msg[MSG_SIZE];
  ssize_t len = recvfrom(fd, msg, sizeof(msg) - 1, 0, (struct sockaddr *)&client, &client_len);
  if (len < 0) {
    close(fd);
    error_exit("recvfrom");
  }

  msg[len] = '\0';
  printf("Server: Received message: %s\n", msg);

  if (sendto(fd, "Hi!", 3, 0, (struct sockaddr *)&client, client_len) < 0) {
    perror("sendto");
  }

  close(fd);
  exit(EXIT_SUCCESS);
}
