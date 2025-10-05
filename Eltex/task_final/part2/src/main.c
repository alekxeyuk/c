#define _GNU_SOURCE
#include "main.h"

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <locale.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/signalfd.h>
#include <sys/timerfd.h>
#include <unistd.h>

#include "command.h"
#include "utils.h"

static int create_signalfd(void) {
  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGINT);
  sigaddset(&mask, SIGQUIT);

  if (sigprocmask(SIG_BLOCK, &mask, NULL) < 0) err(EXIT_FAILURE, "sigprocmask");
  int sfd = signalfd(-1, &mask, 0);
  if (sfd < 0) err(EXIT_FAILURE, "signalfd");

  return sfd;
}

static int create_pipe(void) {
  int drivers_pipe[2];  // drivers -> parent
  if (pipe2(drivers_pipe, O_NONBLOCK) == -1) {
    err(EXIT_FAILURE, "pipe2");
  }
  register_pipe(drivers_pipe[1]);
  return drivers_pipe[0];
}

int main(int argc, char* argv[]) {
  (void)argc;
  (void)argv;

  setlocale(LC_ALL, "C.utf8");

  int sfd = create_signalfd();
  int read_pipe = create_pipe();

  char buffer[BUF_SIZE];
  struct epoll_event events[EVENTS_SIZE];
  int epfd = epoll_create1(0);

  if (add_to_epoll(epfd, STDIN_FILENO) != 0) err(EXIT_FAILURE, "epoll_ctl");
  if (add_to_epoll(epfd, sfd) != 0) err(EXIT_FAILURE, "epoll_ctl");
  if (add_to_epoll(epfd, read_pipe) != 0) err(EXIT_FAILURE, "epoll_ctl");

  while (true) {
    int ready = 0;
    if ((ready = epoll_wait(epfd, events, EVENTS_SIZE, -1)) < 0) {
      if (errno == EINTR) continue;
      perror("epoll_wait");
      exit(EXIT_FAILURE);
    }

    for (int i = 0; i < ready; i++) {
      if (events[i].data.fd == STDIN_FILENO) {
        if (fgets(buffer, BUF_SIZE, stdin) == NULL) goto STOP;
        buffer[strcspn(buffer, "\n")] = '\0';
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
      if (events[i].data.fd == read_pipe) {
        ssize_t bytes_read =
            read(events[i].data.fd, buffer, sizeof(buffer) - 1);
        if (bytes_read > 0) {
          buffer[bytes_read] = '\0';
          if (strncmp(buffer, MSG_AVAILABLE, strlen(MSG_AVAILABLE)) == 0) {
            pid_t pid;
            satou(buffer + strlen(MSG_AVAILABLE), &pid);
            update_driver_state(pid, DRIVER_AVAILABLE, 0);
          } else {
            printf("%s\n", buffer);
          }
        }
      }
    }
  }

STOP:
  close(epfd);
  close(sfd);
  close(read_pipe);
  kill_drivers();
  return EXIT_SUCCESS;
}