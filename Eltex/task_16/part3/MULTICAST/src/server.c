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

  return fd;
}

int main(void) {
  setlocale(LC_ALL, "C.utf8");

  int fd;
  struct sockaddr_in client, multicast;
  socklen_t client_len = sizeof(client);
  socklen_t multicast_len = sizeof(multicast);

  if ((fd = create_socket()) < 0) error_exit("socket");

  memset(&multicast, 0, multicast_len);
  multicast.sin_family = AF_INET;
  multicast.sin_port = htons(PORT);
  if (inet_pton(AF_INET, MULTICAST_ADDR, &multicast.sin_addr) <= 0) {
    return -1;
  }

  __u_char ttl = 32;
  setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));

  while (1) {
    if (sendto(fd, "Hello from server!", 18, 0, (struct sockaddr *)&multicast, multicast_len) < 0) {
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
