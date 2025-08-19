#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "common.h"

static int create_named_socket(const char *name) {
  struct sockaddr_un addr;
  int fd;

  if ((fd = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0) return -1;

  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_LOCAL;
  strncpy(addr.sun_path, name, sizeof(addr.sun_path) - 1);

  unlink(name);
  if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    close(fd);
    return -1;
  }

  return fd;
}

int main(void) {
  setlocale(LC_ALL, "C.utf8");

  int fd;

  if ((fd = create_named_socket(SERVER_SOCKET_NAME)) < 0) error_exit("socket");

  if (listen(fd, 5) < 0) {
    close(fd);
    unlink(SERVER_SOCKET_NAME);
    error_exit("listen");
  }

  int client_fd = accept(fd, NULL, NULL);
  if (client_fd < 0) {
    close(fd);
    unlink(SERVER_SOCKET_NAME);
    error_exit("accept");
  }

  char msg[MSG_SIZE];
  ssize_t len = read(client_fd, msg, sizeof(msg) - 1);
  if (len < 0) {
    close(fd);
    unlink(SERVER_SOCKET_NAME);
    error_exit("recvfrom");
  }
  msg[len] = '\0';
  printf("Server: Received message: %s\n", msg);

  if (write(client_fd, "Hi!", 3) < 0) {
    perror("sendto");
  }

  close(fd);
  close(client_fd);
  unlink(SERVER_SOCKET_NAME);
  exit(EXIT_SUCCESS);
}
