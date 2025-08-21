#include "server.h"

#include <arpa/inet.h>
#include <errno.h>
#include <locale.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

static volatile sig_atomic_t should_exit = 0;
static int sockfd;

static worker_t workers[MAX_THREADS];
static int pool_size = MAX_THREADS;
static pthread_mutex_t pool_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t pool_cond = PTHREAD_COND_INITIALIZER;

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

static void *worker_thread(void *arg) {
  worker_t *w = (worker_t *)arg;
  char buffer[BUFFER_SIZE];

  while (!should_exit) {
    pthread_mutex_lock(&w->mutex);
    while (w->client_fd == -1 && !should_exit) {
      pthread_cond_wait(&w->cond, &w->mutex);
    }
    if (should_exit) {
      pthread_mutex_unlock(&w->mutex);
      break;
    }

    int client_fd = w->client_fd;
    w->client_fd = -1;
    pthread_mutex_unlock(&w->mutex);

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

    pthread_mutex_lock(&pool_mutex);
    pthread_mutex_lock(&w->mutex);
    w->busy = 0;
    pthread_mutex_unlock(&w->mutex);
    pthread_cond_signal(&pool_cond);
    pthread_mutex_unlock(&pool_mutex);
  }

  return NULL;
}

static int find_free_worker(void) {
  int assigned = -1;
  while (!should_exit) {
    for (int i = 0; i < pool_size; ++i) {
      pthread_mutex_lock(&workers[i].mutex);
      bool free = (workers[i].client_fd == -1 && workers[i].busy == 0);
      pthread_mutex_unlock(&workers[i].mutex);
      if (free) return i;
    }
    pthread_cond_wait(&pool_cond, &pool_mutex);
  }
  return assigned;
}

static void cleanup(void) {
  should_exit = 1;
  for (int i = 0; i < pool_size; ++i) {
    pthread_mutex_lock(&workers[i].mutex);
    pthread_cond_signal(&workers[i].cond);
    pthread_mutex_unlock(&workers[i].mutex);
  }

  for (int i = 0; i < pool_size; ++i) {
    pthread_join(workers[i].thread, NULL);
    pthread_mutex_destroy(&workers[i].mutex);
    pthread_cond_destroy(&workers[i].cond);
  }

  pthread_mutex_destroy(&pool_mutex);
  pthread_cond_destroy(&pool_cond);
}

int main(int argc, char *argv[]) {
  setlocale(LC_ALL, "C.utf8");

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

  for (int i = 0; i < pool_size; i++) {
    workers[i].id = i;
    workers[i].client_fd = -1;
    workers[i].busy = 0;
    pthread_mutex_init(&workers[i].mutex, NULL);
    pthread_cond_init(&workers[i].cond, NULL);
    if (pthread_create(&workers[i].thread, NULL, worker_thread, &workers[i]) != 0) error_exit("pthread_create");
  }

  printf("Server started on %s:%hu\n", ip, port);

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

    pthread_mutex_lock(&pool_mutex);
    int assigned = find_free_worker();

    if (should_exit) {
      pthread_mutex_unlock(&pool_mutex);
      close(client_fd);
      break;
    }

    pthread_mutex_lock(&workers[assigned].mutex);
    workers[assigned].client_fd = client_fd;
    workers[assigned].busy = 1;
    pthread_cond_signal(&workers[assigned].cond);
    pthread_mutex_unlock(&workers[assigned].mutex);
    pthread_mutex_unlock(&pool_mutex);

    printf("Client connected from [%s:%d]. Assigned worker: %d\n", inet_ntoa(client_addr.sin_addr),
           ntohs(client_addr.sin_port), assigned);
  }

  shutdown(sockfd, SHUT_RDWR);
  close(sockfd);
  cleanup();
  exit(EXIT_SUCCESS);
}