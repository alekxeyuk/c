#ifndef UTILS_H_
#define UTILS_H_

#include <stdbool.h>

bool satou(const char* s, int* out);
int add_to_epoll(int epoll_fd, int fd);

#endif  // UTILS_H_