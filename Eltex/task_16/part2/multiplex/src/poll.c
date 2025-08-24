#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <locale.h>
#include <netinet/in.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

static int fd_is_valid(int fd) { return fcntl(fd, F_GETFD) != -1 || errno != EBADF; }

int main(int argc, char *argv[]) {
  setlocale(LC_ALL, "C.utf8");

  char buffer[BUFFER_SIZE];
  struct sockaddr_in client_addr;
  socklen_t addr_len = sizeof(client_addr);
  struct pollfd read_set[MAX_CLIENTS + 2];

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

  for (int i = 0; i < MAX_CLIENTS + 2; i++) {
    read_set[i].fd = -1;
    read_set[i].events = POLLIN;
  }

  read_set[0].fd = udp_sockfd;
  read_set[1].fd = tcp_sockfd;

  printf("Server started on %s:%hu\n", ip, port);

  while (!should_exit) {
    int ready;
    if ((ready = poll(read_set, MAX_CLIENTS + 2, -1)) < 0) {
      if (errno == EINTR) continue;
      error_exit("select");
    }

    // Handle UDP request
    if (read_set[0].revents & POLLIN) {
      ssize_t bytes_received =
          recvfrom(udp_sockfd, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&client_addr, &addr_len);
      if (bytes_received < 0) {
        perror("recvfrom");
        continue;
      }
      buffer[bytes_received] = '\0';
      if (strcmp(buffer, TIME_REQUEST) == 0) {
        time_t now = time(NULL);
        snprintf(buffer, sizeof(buffer), "%s", ctime(&now));
        sendto(udp_sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&client_addr, addr_len);
      } else {
        sendto(udp_sockfd, "Unknown request\n", 16, 0, (struct sockaddr *)&client_addr, addr_len);
      }

      if (--ready <= 0) continue;
    }

    // Accept new TCP connection
    if (read_set[1].revents & POLLIN) {
      int client_fd = accept(tcp_sockfd, (struct sockaddr *)&client_addr, &addr_len);
      if (client_fd < 0) {
        perror("accept");
        continue;
      }

      int i;
      for (i = 0; i < MAX_CLIENTS; i++) {
        if (read_set[i].fd < 0 || !fd_is_valid(read_set[i].fd)) {
          read_set[i].fd = client_fd;
          break;
        }
      }

      if (i == MAX_CLIENTS) {
        fprintf(stderr, "Max clients reached. Connection refused.\n");
        close(client_fd);
      } else {
        printf("Client connected from [%s:%d]. Total clients: %d\n", inet_ntoa(client_addr.sin_addr),
               ntohs(client_addr.sin_port), i - 1);
      }

      if (--ready <= 0) continue;
    }

    // Process TCP clients
    for (int i = 0; i < MAX_CLIENTS; i++) {
      int client_fd = read_set[i].fd;
      if (client_fd < 0) continue;

      if (read_set[i].revents & POLLIN) {
        ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
        if (bytes_read <= 0) {
          if (bytes_read == 0)
            printf("Client disconnected.\n");
          else
            perror("read");

          close(client_fd);
          read_set[i].fd = -1;
        } else {
          buffer[bytes_read] = '\0';
          if (strcmp(buffer, TIME_REQUEST) == 0) {
            time_t now = time(NULL);
            snprintf(buffer, sizeof(buffer), "%s", ctime(&now));
            write(client_fd, buffer, strlen(buffer));
          } else {
            write(client_fd, "Unknown request\n", 16);
          }
        }

        if (--ready <= 0) break;
      } else if (read_set[i].revents & POLLHUP) {
        printf("Client disconnected.\n");
        close(client_fd);
        read_set[i].fd = -1;

        if (--ready <= 0) break;
      }
    }
  }

  shutdown(tcp_sockfd, SHUT_RDWR);
  close(tcp_sockfd);
  shutdown(udp_sockfd, SHUT_RDWR);
  close(udp_sockfd);
  exit(EXIT_SUCCESS);
}