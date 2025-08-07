#ifndef HELPER_H_
#define HELPER_H_

#include <stdbool.h>
#include <linux/limits.h>
#include <stddef.h>
#include "plugins/plugin.h"

#define MAX_OPERATIONS 64

typedef struct {
    void* handle;
    const operation_t* op;
} plugin_entry_t;

void flush_stdint(void);
bool read_ab(int*, int*);
void print_menu(plugin_entry_t plugins[], size_t n);
int select_operation(size_t n);

bool has_so_suffix(const char* name);
size_t load_plugins(const char* path, plugin_entry_t plugins[], size_t max_plugins);
void unload_plugins(plugin_entry_t plugins[], size_t n);


#endif  // HELPER_H_
