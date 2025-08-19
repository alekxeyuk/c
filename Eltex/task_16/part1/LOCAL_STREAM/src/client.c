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

  if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    close(fd);
    return -1;
  }

  return fd;
}

int main(void) {
  setlocale(LC_ALL, "C.utf8");

  int fd;

  if ((fd = create_named_socket(SERVER_SOCKET_NAME)) < 0) error_exit("socket");

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
