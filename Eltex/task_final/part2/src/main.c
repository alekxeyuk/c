#define _GNU_SOURCE
#include "main.h"

#include <err.h>
#include <errno.h>
#include <locale.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/signalfd.h>
#include <sys/timerfd.h>
#include <unistd.h>

#include "utils.h"

int main(int argc, char* argv[]) {
  (void)argc;
  (void)argv;

  setlocale(LC_ALL, "C.utf8");

  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGINT);
  sigaddset(&mask, SIGQUIT);

  if (sigprocmask(SIG_BLOCK, &mask, NULL) < 0) err(EXIT_FAILURE, "sigprocmask");
  int sfd = signalfd(-1, &mask, 0);
  if (sfd < 0) err(EXIT_FAILURE, "signalfd");

  int tfd = timerfd_create(CLOCK_REALTIME, 0);
  if (tfd < 0) err(EXIT_FAILURE, "timerfd_create");

  struct itimerspec spec = {
      .it_value.tv_sec = 100,
      .it_interval.tv_sec = 100,
  };
  if (timerfd_settime(tfd, 0, &spec, NULL) < 0)
    err(EXIT_FAILURE, "timer_settime");

  char buffer[200];
  struct epoll_event events[EVENTS_SIZE];
  int epfd = epoll_create1(0);

  struct epoll_event event = {
      .events = EPOLLIN,
      .data.fd = STDIN_FILENO,
  };

  if (epoll_ctl(epfd, EPOLL_CTL_ADD, STDIN_FILENO, &event) != 0) {
    perror("epoll_ctl");
    exit(EXIT_FAILURE);
  }

  event.data.fd = sfd;
  if (epoll_ctl(epfd, EPOLL_CTL_ADD, sfd, &event) != 0) {
    perror("epoll_ctl");
    exit(EXIT_FAILURE);
  }

  event.data.fd = tfd;
  if (epoll_ctl(epfd, EPOLL_CTL_ADD, tfd, &event) != 0) {
    perror("epoll_ctl");
    exit(EXIT_FAILURE);
  }

  int working = 1;
  while (working) {
    int ready = 0;
    if ((ready = epoll_wait(epfd, events, EVENTS_SIZE, -1)) < 0) {
      if (errno == EINTR) continue;
      perror("epoll_wait");
      exit(EXIT_FAILURE);
    }

    for (int i = 0; i < ready; i++) {
      if (events[i].data.fd == STDIN_FILENO) {
        if (fgets(buffer, 200, stdin) == NULL) goto STOP;
        buffer[strcspn(buffer, "\n")] = '\0';
        printf("{read from stdin} [%s]\n", buffer);
        handle_input(buffer, cmd_table, CMD_TABLE_SIZE);
      }
      if (events[i].data.fd == sfd) {
        struct signalfd_siginfo fdsi;
        ssize_t s = read(sfd, &fdsi, sizeof(fdsi));
        if (s != sizeof(fdsi)) err(EXIT_FAILURE, "read");

        switch (fdsi.ssi_signo) {
          case SIGINT:
            printf("GOT SIGINT\n");
            break;
          case SIGQUIT:
            printf("GOT SIGQUIT\n");
            goto STOP;
            break;
          default:
            printf("Unexpected signal\n");
            break;
        }
      }
      if (events[i].data.fd == tfd) {
        uint64_t numExp;
        ssize_t s = read(tfd, &numExp, sizeof(uint64_t));
        if (s != sizeof(uint64_t)) err(EXIT_FAILURE, "read");
        printf("[Timer shot] %lu\n", numExp);
      }
    }
  }

STOP:
  close(epfd);
  close(sfd);
  close(tfd);
  return EXIT_SUCCESS;
}