#include "client.h"

#include <arpa/inet.h>
#include <locale.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

static volatile sig_atomic_t should_exit = 0;

static void signal_handler(int signum) {
  (void)signum;
  should_exit = 1;
  printf("\nStoping...\n");
}

static int socket_connect(const char *ip, unsigned short port) {
  struct sockaddr_in addr;
  int fd;

  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) return -1;

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  if (inet_pton(AF_INET, ip, &addr.sin_addr) != 1) {
    fprintf(stderr, "Invalid IP address: %s\n", ip);
    close(fd);
    return -1;
  }

  if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    close(fd);
    return -1;
  }

  return fd;
}

int main(int argc, char *argv[]) {
  setlocale(LC_ALL, "C.utf8");

  int sockfd;
  char buf[BUFFER_SIZE];
  ssize_t rec_size;

  if (argc != 3) {
    fprintf(stderr, "Usage: %s <ip> <port>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  const char *ip = argv[1];
  unsigned short port = (unsigned short)atoi(argv[2]);
  if (port == 0) {
    fprintf(stderr, "Invalid port number: %s\n", argv[2]);
    exit(EXIT_FAILURE);
  }

  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);

  if ((sockfd = socket_connect(ip, port)) < 0) error_exit("socket_connect");

  while (!should_exit) {
    if (write(sockfd, TIME_REQUEST, strlen(TIME_REQUEST)) < 0) {
      perror("write");
      break;
    }

    rec_size = read(sockfd, buf, sizeof(buf) - 1);
    if (rec_size < 0) {
      perror("read");
      break;
    } else if (rec_size == 0) {
      fprintf(stderr, "Server closed the connection\n");
      should_exit = 1;
      continue;
    }

    buf[rec_size] = '\0';
    printf("Received: %s\n", buf);

    sleep(1);
  }

  close(sockfd);
  exit(EXIT_SUCCESS);
}
