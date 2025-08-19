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

  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) return -1;

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

  if ((fd = create_socket(SERVER_PORT)) < 0) error_exit("socket");

  if (listen(fd, 5) < 0) {
    close(fd);
    error_exit("listen");
  }

  int client_fd = accept(fd, NULL, NULL);
  if (client_fd < 0) {
    close(fd);
    error_exit("accept");
  }

  char msg[MSG_SIZE];
  ssize_t len = read(client_fd, msg, sizeof(msg) - 1);
  if (len < 0) {
    close(fd);
    error_exit("recvfrom");
  }
  msg[len] = '\0';
  printf("Server: Received message: %s\n", msg);

  if (write(client_fd, "Hi!", 3) < 0) {
    perror("sendto");
  }

  shutdown(client_fd, SHUT_RDWR);
  shutdown(fd, SHUT_RDWR);
  close(fd);
  close(client_fd);
  exit(EXIT_SUCCESS);
}
