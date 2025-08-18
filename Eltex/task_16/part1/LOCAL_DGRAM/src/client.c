#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "common.h"

static int create_named_socket(const char *client_name, const char *server_name) {
  struct sockaddr_un server_addr, client_addr;

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sun_family = AF_LOCAL;
  strncpy(server_addr.sun_path, server_name, sizeof(server_addr.sun_path) - 1);

  memset(&client_addr, 0, sizeof(client_addr));
  client_addr.sun_family = AF_LOCAL;
  strncpy(client_addr.sun_path, client_name, sizeof(client_addr.sun_path) - 1);

  int fd = socket(AF_LOCAL, SOCK_DGRAM, 0);
  if (fd < 0) return -1;

  unlink(client_name);

  if (bind(fd, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0) {
    close(fd);
    return -1;
  }

  if (connect(fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    close(fd);
    return -1;
  }

  return fd;
}

int main(void) {
  setlocale(LC_ALL, "C.utf8");

  int fd;

  if ((fd = create_named_socket(CLIENT_SOCKET_NAME, SERVER_SOCKET_NAME)) < 0) error_exit("socket");

  char msg[MSG_SIZE];
  ssize_t len = write(fd, "Hello!", 7);
  if (len < 0) {
    close(fd);
    unlink(CLIENT_SOCKET_NAME);
    error_exit("write");
  }

  len = read(fd, msg, sizeof(msg) - 1);
  if (len < 0) {
    close(fd);
    unlink(CLIENT_SOCKET_NAME);
    error_exit("read");
  }
  msg[len] = '\0';
  printf("Client: Received message: %s\n", msg);

  close(fd);
  unlink(CLIENT_SOCKET_NAME);
  exit(EXIT_SUCCESS);
}
