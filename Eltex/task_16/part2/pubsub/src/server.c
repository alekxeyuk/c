#include "server.h"

#include <arpa/inet.h>
#include <errno.h>
#include <locale.h>
#include <mqueue.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

static volatile sig_atomic_t should_exit = 0;
static int sockfd = -1;
static mqd_t mq = (mqd_t)-1;

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

  if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) return -1;

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
  (void)arg;
  udp_msg_t msg;
  ssize_t n;
  char buffer[BUFFER_SIZE];

  while (!should_exit) {
    if ((n = mq_receive(mq, (char *)&msg, sizeof(msg), NULL)) < 0) break;
    msg.data[msg.data_len] = '\0';
    if (strcmp(msg.data, TIME_REQUEST) == 0) {
      time_t now = time(NULL);
      snprintf(buffer, sizeof(buffer), "%s", ctime(&now));
      sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&msg.addr,
             msg.addr_len);
    } else {
      sendto(sockfd, "Unknown request\n", 16, 0, (struct sockaddr *)&msg.addr,
             msg.addr_len);
    }
  }

  return NULL;
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

  struct mq_attr attr = {.mq_flags = 0,
                         .mq_maxmsg = QUEUE_MSIZE,
                         .mq_msgsize = sizeof(udp_msg_t),
                         .mq_curmsgs = 0};

  mq_unlink(QUEUE_NAME);
  if ((mq = mq_open(QUEUE_NAME, O_CREAT | O_RDWR, S_IRWXU, &attr)) ==
      (mqd_t)-1) {
    close(sockfd);
    error_exit("mq_open");
  }

  pthread_t workers[MAX_THREADS];
  for (int i = 0; i < MAX_THREADS; i++) {
    if (pthread_create(&workers[i], NULL, worker_thread, NULL) != 0)
      error_exit("pthread_create");
  }

  printf("Server started on %s:%hu\n", ip, port);

  while (!should_exit) {
    udp_msg_t msg;
    ssize_t n = recvfrom(sockfd, msg.data, sizeof(msg.data), 0,
                         (struct sockaddr *)&msg.addr, &msg.addr_len);
    if (n < 0) {
      perror("recvfrom");
      break;
    }
    msg.data_len = (size_t)n;

    if (mq_send(mq, (char *)&msg, sizeof(msg), 1) < 0) {
      if (errno == EAGAIN) {
        fprintf(stderr, "No more space in queue!\n");
      } else {
        perror("mq_send");
      }
    }
  }

  printf("\nStoping server\n");
  for (int i = 0; i < MAX_THREADS; i++) {
    pthread_cancel(workers[i]);
    pthread_join(workers[i], NULL);
  }
  mq_close(mq);
  mq_unlink(QUEUE_NAME);
  shutdown(sockfd, SHUT_RDWR);
  close(sockfd);
  exit(EXIT_SUCCESS);
}