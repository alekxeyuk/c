#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <locale.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include "common.h"

static volatile sig_atomic_t should_exit = 0;
static int tcp_sockfd;
static int udp_sockfd;

static void signal_handler(int signum) {
  (void)signum;
  should_exit = 1;
  shutdown(tcp_sockfd, SHUT_RDWR);
  shutdown(udp_sockfd, SHUT_RDWR);
  close(tcp_sockfd);
  close(udp_sockfd);
  printf("\nStoping...\n");
}

static int create_socket(enum __socket_type type, const char *ip, unsigned short port) {
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

  if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    close(fd);
    return -1;
  }

  return fd;
}

static int add_to_epoll(int epoll_fd, int fd) {
  struct epoll_event event;
  event.events = EPOLLIN;
  event.data.fd = fd;
  return epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
}

static int process_fd(int fd) {
  char buffer[BUFFER_SIZE];
  struct sockaddr_in client_addr;
  socklen_t addr_len = sizeof(client_addr);

  ssize_t bytes_received = recvfrom(fd, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&client_addr, &addr_len);
  if (bytes_received <= 0) {
    perror("recvfrom");
    return -1;
  }
  buffer[bytes_received] = '\0';
  if (strcmp(buffer, TIME_REQUEST) == 0) {
    time_t now = time(NULL);
    snprintf(buffer, sizeof(buffer), "%s", ctime(&now));
    sendto(fd, buffer, strlen(buffer), 0, (struct sockaddr *)&client_addr, addr_len);
  } else {
    sendto(fd, "Unknown request\n", 16, 0, (struct sockaddr *)&client_addr, addr_len);
  }
  return 0;
}

int main(int argc, char *argv[]) {
  setlocale(LC_ALL, "C.utf8");

  struct sockaddr_in client_addr;
  socklen_t addr_len = sizeof(client_addr);
  int epoll_fd;
  struct epoll_event events[MAX_CLIENTS + 2];

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

  if ((tcp_sockfd = create_socket(SOCK_STREAM, ip, port)) < 0) error_exit("create_socket");
  if ((udp_sockfd = create_socket(SOCK_DGRAM, ip, port)) < 0) error_exit("create_socket");

  if (listen(tcp_sockfd, R_Q_SIZE) < 0) {
    close(tcp_sockfd);
    close(udp_sockfd);
    error_exit("listen");
  }

  if ((epoll_fd = epoll_create1(0)) < 0) {
    close(tcp_sockfd);
    close(udp_sockfd);
    close(epoll_fd);
    error_exit("epoll_create1");
  }

  if (add_to_epoll(epoll_fd, udp_sockfd) < 0 || add_to_epoll(epoll_fd, tcp_sockfd) < 0) {
    close(tcp_sockfd);
    close(udp_sockfd);
    close(epoll_fd);
    error_exit("add_to_epoll");
  }

  printf("Server started on %s:%hu\n", ip, port);

  while (!should_exit) {
    int ready;
    if ((ready = epoll_wait(epoll_fd, events, MAX_CLIENTS + 2, -1)) < 0) {
      if (errno == EINTR) continue;
      error_exit("select");
    }

    for (int i = 0; i < ready; i++) {
      if (events[i].data.fd == udp_sockfd) {
        // Handle UDP request
        process_fd(udp_sockfd);
      } else if (events[i].data.fd == tcp_sockfd) {
        // Accept new TCP connection
        int client_fd = accept(tcp_sockfd, (struct sockaddr *)&client_addr, &addr_len);
        if (client_fd < 0) {
          perror("accept");
          continue;
        }

        if (add_to_epoll(epoll_fd, client_fd) < 0) {
          fprintf(stderr, "Failed to add client to epoll. Connection refused.\n");
          close(client_fd);
        } else {
          printf("Client connected from [%s:%d].\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        }
      } else {
        // Process TCP clients
        int client_fd = events[i].data.fd;
        if (events[i].events & EPOLLHUP || process_fd(client_fd) < 0) {
          printf("Client disconnected.\n");
          close(client_fd);
          epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
          continue;
        }
      }
    }
  }

  shutdown(tcp_sockfd, SHUT_RDWR);
  close(tcp_sockfd);
  shutdown(udp_sockfd, SHUT_RDWR);
  close(udp_sockfd);
  close(epoll_fd);
  exit(EXIT_SUCCESS);
}