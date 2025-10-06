#ifndef HELPER_H_
#define HELPER_H_

#include <dirent.h>
#include <dlfcn.h>
#include <err.h>
#include <signal.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <sys/signalfd.h>
#include <unistd.h>

#include "../shared/src/client_lib.h"

void* load_plugin(const char* name, operations_t* op_handle);
void unload_plugins(void* handle);
void parse_args(int argc, char* argv[], char** dest_ip, uint16_t* dest_port,
                uint16_t* from_port, const char** if_name);
size_t read_line(char* buffer, int size);
int add_to_epoll(int epoll_fd, int fd);
int create_signalfd(void);

#endif  // HELPER_H_