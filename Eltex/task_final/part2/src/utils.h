#ifndef UTILS_H_
#define UTILS_H_

#include <stdbool.h>

#include "driver.h"

typedef struct {
  pid_t pid;
  driver_state_t state;
  int timer;
  bool update;
} state_update_t;

bool satou(const char* s, int* out);
int add_to_epoll(int epoll_fd, int fd);

#endif  // UTILS_H_