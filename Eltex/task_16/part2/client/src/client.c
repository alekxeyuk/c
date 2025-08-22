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
static int sockfd;

static void signal_handler(int signum) {
  (void)signum;
  should_exit = 1;
  close(sockfd);
  printf("\nStoping...\n");
}

static int socket_connect(enum __socket_type type, const char *ip,
                          unsigned short port) {
  struct sockaddr_in addr;
  int fd;

  if ((fd = socket(AF_INET, type, 0)) < 0) return -1;

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

  char buf[BUFFER_SIZE];
  ssize_t rec_size;

  if (argc != 4) {
    fprintf(stderr, "Usage: %s <udp|tcp> <ip> <port>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  enum __socket_type type = SOCK_STREAM;
  const char *proto = argv[1];
  if (strcmp(proto, "udp") == 0) {
    type = SOCK_DGRAM;
  } else if (strcmp(proto, "tcp") != 0) {
    fprintf(stderr, "Invalid protocol: %s\n", proto);
    exit(EXIT_FAILURE);
  }

  const char *ip = argv[2];
  unsigned short port = (unsigned short)atoi(argv[3]);
  if (port == 0) {
    fprintf(stderr, "Invalid port number: %s\n", argv[3]);
    exit(EXIT_FAILURE);
  }

  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);

  if ((sockfd = socket_connect(type, ip, port)) < 0)
    error_exit("socket_connect");

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
    if (buf[rec_size - 1] == '\n') {
      buf[rec_size - 1] = '\0';
    }
    printf("[Received: %s]\n", buf);

    sleep(1);
  }

  close(sockfd);
  exit(EXIT_SUCCESS);
}
