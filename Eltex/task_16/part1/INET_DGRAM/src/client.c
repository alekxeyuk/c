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

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

  if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) return -1;

  if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    close(fd);
    return -1;
  }

  return fd;
}

int main(void) {
  setlocale(LC_ALL, "C.utf8");

  int fd;

  if ((fd = create_socket(SERVER_PORT)) < 0) error_exit("socket");

  char msg[MSG_SIZE];
  ssize_t len = write(fd, "Hello!", 6);
  if (len < 0) {
    close(fd);
    error_exit("write");
  }

  len = read(fd, msg, sizeof(msg) - 1);
  if (len < 0) {
    close(fd);
    error_exit("read");
  }
  msg[len] = '\0';
  printf("Client: Received message: %s\n", msg);

  close(fd);
  exit(EXIT_SUCCESS);
}
