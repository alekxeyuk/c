#include <arpa/inet.h>
#include <locale.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "common.h"

static int create_socket(void) {
  int fd;

  if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) return -1;

  int on = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on)) < 0) {
    close(fd);
    return -1;
  }

  return fd;
}

int main(void) {
  setlocale(LC_ALL, "C.utf8");

  int fd;
  struct sockaddr_in client, broadcast;
  socklen_t client_len = sizeof(client);

  if ((fd = create_socket()) < 0) error_exit("socket");

  memset(&broadcast, 0, sizeof(broadcast));
  broadcast.sin_family = AF_INET;
  broadcast.sin_port = htons(BROADCAST_PORT);
  broadcast.sin_addr.s_addr = INADDR_BROADCAST;

  while (1) {
    if (sendto(fd, "Hello from server!", 18, 0, (struct sockaddr *)&broadcast, sizeof(broadcast)) < 0) {
      perror("sendto");
    }

    char msg[MSG_SIZE];
    ssize_t len = recvfrom(fd, msg, sizeof(msg) - 1, MSG_DONTWAIT, (struct sockaddr *)&client, &client_len);
    if (len > 0) {
      msg[len] = '\0';
      printf("Server: Received message: %s [%s:%d]\n", msg, inet_ntoa(client.sin_addr), ntohs(client.sin_port));
    }

    sleep(1);
  }

  close(fd);
  exit(EXIT_SUCCESS);
}
