#define _DEFAULT_SOURCE
#include "helper.h"
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <dirent.h>

void flush_stdint(void) {
  int c;
  while ((c = getchar()) != '\n' && c != EOF);
}

bool read_ab(int* a, int* b) {
  printf("a = ");
  if (scanf("%d", a) == 0) return false;
  printf("b = ");
  if (scanf("%d", b) == 0) return false;
  return true;
}

void print_menu(plugin_entry_t plugins[], size_t n) {
  for (size_t i = 0; i < n; i++) {
    printf("(%ld) %s - %s\n", i + 1, plugins[i].op->name, plugins[i].op->description);
  }
  printf("(0) Exit\n");
}

int select_operation(size_t n) {
  printf("> ");
  fflush(stdout);

  int choice = -1;
  if (scanf("%d", &choice) != 1 || choice < 0 || (size_t)choice > n) {
    flush_stdint();
    return -2; // Error: Invalid choice
  }
  flush_stdint();
  if (choice == 0) {
    return -1; // Exit choice
  }
  return choice - 1;
}

bool has_so_suffix(const char* name) {
  const char* dot = strrchr(name, '.');
  return dot && strncmp(dot, ".so", 3) == 0;
}

size_t load_plugins(const char *path, plugin_entry_t plugins[], size_t max_plugins) {
  DIR *dir = opendir(path);
  if (!dir) {
    perror("opendir failed");
    return 0;
  }

  struct dirent *entry;
  size_t count = 0;

  while ((entry = readdir(dir)) != NULL) {
    if (entry->d_type != DT_REG || !has_so_suffix(entry->d_name))
      continue;

    if (count >= max_plugins) {
      fprintf(stderr, "PLugins limit reached (%zu)\n", max_plugins);
      break;
    }

    char full_path[PATH_MAX];
    snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

    void *handle = dlopen(full_path, RTLD_LAZY);
    if (!handle) {
      fprintf(stderr, "dlopen(%s) failed: %s\n", full_path, dlerror());
      continue;
    }

    const operation_t *op = (const operation_t*)dlsym(handle, "operation");
    if (!op) {
      fprintf(stderr, "dlsym(%s) failed: %s\n", full_path, dlerror());
      dlclose(handle);
      continue;
    }

    plugins[count].handle = handle;
    plugins[count].op = op;
    count++;
  }

  closedir(dir);
  return count;
}

void unload_plugins(plugin_entry_t plugins[], size_t n) {
  for (size_t i = 0; i < n; i++) {
    dlclose(plugins[i].handle);
  }
}
