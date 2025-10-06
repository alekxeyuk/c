#include "utils.h"

#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <stddef.h>
#include <sys/epoll.h>

bool satou(const char* s, int* out) {
  errno = 0;
  char* end = NULL;

  while (isspace((unsigned char)*s)) s++;
  if (*s == '\0' || *s == '-') return false;

  uintmax_t v = strtoumax(s, &end, 10);
  if (errno == ERANGE || end == s || *end != '\0') return false;

  *out = (int)v;
  return true;
}

int add_to_epoll(int epoll_fd, int fd) {
  struct epoll_event event;
  event.events = EPOLLIN;
  event.data.fd = fd;
  return epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
}