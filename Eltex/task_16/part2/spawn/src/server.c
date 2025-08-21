#include "server.h"

#include <arpa/inet.h>
#include <errno.h>
#include <locale.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

static volatile sig_atomic_t should_exit = 0;
static pthread_t threads[MAX_THREADS];
static int thread_count = 0;
static int sockfd;

static void signal_handler(int signum) {
  (void)signum;
  should_exit = 1;
  shutdown(sockfd, SHUT_RDWR);
  close(sockfd);
  printf("\nStoping...\n");
}

static int create_socket(const char *ip, unsigned short port) {
  struct sockaddr_in addr;
  int fd;

  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) return -1;

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0) {
    close(fd);
    return -1;
  }

  if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    close(fd);
    return -1;
  }

  return fd;
}

static void *handle_client(void *arg) {
  int client_fd = *(int *)arg;
  free(arg);
  char buffer[BUFFER_SIZE];
  ssize_t bytes_read;

  while (!should_exit && (bytes_read = read(client_fd, buffer, sizeof(buffer) - 1)) > 0) {
    buffer[bytes_read] = '\0';
    if (strcmp(buffer, TIME_REQUEST) == 0) {
      time_t now = time(NULL);
      snprintf(buffer, sizeof(buffer), "%s", ctime(&now));
      write(client_fd, buffer, strlen(buffer));
    } else {
      write(client_fd, "Unknown request\n", 16);
    }
  }

  if (bytes_read < 0) {
    perror("read");
  } else if (bytes_read == 0) {
    printf("Client disconnected.\n");
  }
  close(client_fd);
  thread_count--;
  pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
  setlocale(LC_ALL, "C.utf8");

  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

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

  if ((sockfd = create_socket(ip, port)) < 0) error_exit("socket_connect");

  if (listen(sockfd, R_Q_SIZE) < 0) {
    close(sockfd);
    error_exit("listen");
  }

  while (!should_exit) {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client_fd = accept(sockfd, (struct sockaddr *)&client_addr, &addr_len);
    if (should_exit) {
      break;
    }
    if (client_fd < 0) {
      close(sockfd);
      error_exit("accept");
    }

    if (thread_count >= MAX_THREADS) {
      close(client_fd);
      fprintf(stderr, "No more place for threads.\n");
      continue;
    }

    int *arg = malloc(sizeof(int));
    if (arg == NULL) {
      close(client_fd);
      fprintf(stderr, "malloc failed.\n");
      continue;
    }

    *arg = client_fd;
    if (pthread_create(&threads[thread_count], &attr, handle_client, arg) != 0) {
      free(arg);
      close(client_fd);
      fprintf(stderr, "pthread_create failed.\n");
      continue;
    }

    thread_count++;
    printf("Client connected from [%s:%d]. Total threads: %d\n", inet_ntoa(client_addr.sin_addr),
           ntohs(client_addr.sin_port), thread_count);
  }

  shutdown(sockfd, SHUT_RDWR);
  close(sockfd);
  pthread_attr_destroy(&attr);
  exit(EXIT_SUCCESS);
}