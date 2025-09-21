#ifndef HELPER_H_
#define HELPER_H_

#include <dirent.h>
#include <dlfcn.h>
#include <stdio.h>
#include <unistd.h>

#include "../shared/src/client_lib.h"

void *load_plugin(const char *name, operations_t *op_handle);
void unload_plugins(void *handle);
void parse_args(int argc, char *argv[], uint16_t *from_port, const char **if_name, uint16_t *table_size);
size_t read_line(char *buffer, int size);

#endif  // HELPER_H_