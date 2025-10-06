#define _GNU_SOURCE
#include "driver.h"

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/signalfd.h>
#include <sys/timerfd.h>
#include <unistd.h>

#include "command.h"
#include "utils.h"

static driver_t drivers[MAX_DRIVERS];
static int driver_count = 0;
static int parent_socket = -1;

bool register_socket(int socket) {
  if (parent_socket != -1) return false;
  parent_socket = socket;
  return true;
}

static void add_driver(pid_t pid, int pipe_to) {
  drivers[driver_count].pid = pid;
  drivers[driver_count].state = DRIVER_AVAILABLE;
  drivers[driver_count].timer = 0;
  drivers[driver_count].pipe_to = pipe_to;
  driver_count++;
}

static driver_t* find_driver(pid_t pid) {
  for (int i = 0; i < driver_count; i++) {
    if (drivers[i].pid == pid) {
      return &drivers[i];
    }
  }
  return NULL;
}

static void driver_loop(int pipe_read, int pipe_write) {
  char buffer[PIPE_BUF_SIZE];
  ssize_t bytes_read;
  state_update_t state = {
      .pid = getpid(),
      .state = DRIVER_AVAILABLE,
      .timer = 0,
      .update = false,
  };

  struct epoll_event events[DEVENTS_SIZE];
  int epfd = epoll_create1(0);

  int tfd = timerfd_create(CLOCK_REALTIME, 0);
  if (tfd < 0) exit(EXIT_FAILURE);
  struct itimerspec spec = {
      .it_value.tv_sec = 0,
      .it_interval.tv_sec = 0,
  };

  if (add_to_epoll(epfd, pipe_read) != 0) exit(EXIT_FAILURE);
  if (add_to_epoll(epfd, tfd) != 0) exit(EXIT_FAILURE);

  while (true) {
    int ready = 0;
    if ((ready = epoll_wait(epfd, events, DEVENTS_SIZE, -1)) < 0) {
      if (errno == EINTR) continue;
      perror("epoll_wait");
      exit(EXIT_FAILURE);
    }

    for (int i = 0; i < ready; i++) {
      if (events[i].data.fd == tfd) {
        uint64_t numExp;
        read(tfd, &numExp, sizeof(uint64_t));
        state.state = DRIVER_AVAILABLE;
        state.timer = 0;
        state.update = true;
        write(pipe_write, &state, sizeof(state));
      }
      if (events[i].data.fd == pipe_read) {
        bytes_read = read(pipe_read, buffer, sizeof(buffer) - 1);
        if (bytes_read > 0) {
          buffer[bytes_read] = '\0';

          if (strcmp(buffer, MSG_STATUS) == 0) {
            state.update = false;
            write(pipe_write, &state, sizeof(state));
          } else if (strncmp(buffer, MSG_TASK, strlen(MSG_TASK)) == 0) {
            if (state.state != DRIVER_AVAILABLE) {
              state.update = false;
              write(pipe_write, &state, sizeof(state));
              continue;
            }

            satou(buffer + 5, &state.timer);
            state.state = DRIVER_BUSY;
            state.update = true;
            spec.it_value.tv_sec = state.timer;
            timerfd_settime(tfd, 0, &spec, NULL);
            write(pipe_write, &state, sizeof(state));
          }
        }
      }
    }
  }
}

bool handle_create_driver(int argc, int argv[]) {
  (void)argc, (void)argv;

  if (driver_count >= MAX_DRIVERS) {
    printf("%s\n", ERR_TOO_MANY_DRIVERS);
    return false;
  }

  int pipe_to[2];  // parent -> driver

  if (pipe2(pipe_to, O_NONBLOCK) == -1) {
    perror(ERR_PIPE_CREATE "\n");
    return false;
  }

  pid_t pid = fork();
  if (pid == -1) {
    printf(ERR_FORK_FAILED "\n");
    close(pipe_to[0]);
    close(pipe_to[1]);
    return false;
  }

  if (pid == 0) {
    // driver
    close(pipe_to[1]);
    driver_loop(pipe_to[0], parent_socket);
  } else {
    // parent
    close(pipe_to[0]);
    add_driver(pid, pipe_to[1]);
    printf("Created driver with PID %d\n", pid);
  }

  return true;
}

bool handle_send_task(int argc, int argv[]) {
  if (argc != 2) {
    return false;
  }
  int pid = argv[0];
  int timer = argv[1];

  driver_t* driver = find_driver(pid);
  if (!driver) {
    printf("%s: %d\n", ERR_DRIVER_NOT_FOUND, pid);
    return false;
  }

  char command[PIPE_BUF_SIZE];
  snprintf(command, sizeof(command), "%s %d", MSG_TASK, timer);
  if (write(driver->pipe_to, command, strlen(command)) == -1) {
    perror("Failed to send task to driver");
    return false;
  }

  return true;
}

bool handle_get_status(int argc, int argv[]) {
  if (argc != 1) {
    return false;
  }
  int pid = argv[0];

  driver_t* driver = find_driver(pid);
  if (!driver) {
    printf("%s: %d\n", ERR_DRIVER_NOT_FOUND, pid);
    return false;
  }

  if (write(driver->pipe_to, MSG_STATUS, strlen(MSG_STATUS)) == -1) {
    perror("Failed to send status request");
    return false;
  }
  return true;
}

bool handle_get_drivers(int argc, int argv[]) {
  (void)argc, (void)argv;
  if (driver_count == 0) {
    printf("No drivers running\n");
    return false;
  }

  printf("Active drivers:\n");
  for (int i = 0; i < driver_count; i++) {
    printf("\tPID %d:\t", drivers[i].pid);
    if (drivers[i].state == DRIVER_BUSY) {
      printf("%s %d\n", MSG_BUSY, drivers[i].timer);
    } else {
      printf("%s\n", MSG_AVAILABLE);
    }
  }
  return true;
}

void kill_drivers(void) {
  for (int i = 0; i < driver_count; i++) {
    if (drivers[i].pid > 0) {
      kill(drivers[i].pid, SIGTERM);
    }
  }
  driver_count = 0;
}

void update_driver_state(pid_t pid, driver_state_t state, int timer) {
  driver_t* driver = find_driver(pid);
  if (driver) {
    driver->state = state;
    driver->timer = timer;
  } else {
    printf(ERR_DRIVER_NOT_FOUND "\n");
  }
}