#include <arpa/inet.h>
#include <locale.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "common.h"

static int create_socket(void) {
  struct sockaddr_in addr;
  struct ip_mreqn mreq;
  int fd;

  if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) return -1;

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(PORT);
  addr.sin_addr.s_addr = INADDR_ANY;
  if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    close(fd);
    return -1;
  }

  memset(&mreq, 0, sizeof(mreq));
  if (inet_pton(AF_INET, MULTICAST_ADDR, &mreq.imr_multiaddr) <= 0) {
    close(fd);
    return -1;
  }
  mreq.imr_address.s_addr = INADDR_ANY;
  setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));

  return fd;
}

int main(void) {
  setlocale(LC_ALL, "C.utf8");

  int fd;
  struct sockaddr_in addr;
  socklen_t addr_len = sizeof(addr);

  if ((fd = create_socket()) < 0) error_exit("socket");

  while (1) {
    char msg[MSG_SIZE];
    ssize_t len = recvfrom(fd, msg, sizeof(msg) - 1, 0, (struct sockaddr *)&addr, &addr_len);
    if (len < 0) {
      close(fd);
      error_exit("recvfrom");
    }
    msg[len] = '\0';
    printf("Client: Received message: %s [%s:%d]\n", msg, inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

    len = sendto(fd, "Hello from client!", 18, 0, (struct sockaddr *)&addr, addr_len);
    if (len < 0) {
      close(fd);
      error_exit("sendto");
    }
  }

  close(fd);
  exit(EXIT_SUCCESS);
}
