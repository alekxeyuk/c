#include "helper.h"

void *load_plugin(const char *name, operations_t *op_handle) {
  void *handle = dlopen(name, RTLD_LAZY);
  if (!handle) {
    fprintf(stderr, "dlopen(%s) failed: %s\n", name, dlerror());
    return NULL;
  }

  const operations_t *ops = (operations_t *)dlsym(handle, "export");
  if (!ops) {
    fprintf(stderr, "dlsym(operations) failed: %s\n", dlerror());
    dlclose(handle);
    return NULL;
  }

  memcpy(op_handle, ops, sizeof(operations_t));

  return handle;
}

void unload_plugins(void *handle) {
  printf("Unloading plugin...\n");
  dlclose(handle);
}

void parse_args(int argc, char *argv[], uint16_t *from_port, const char **if_name, uint16_t *table_size) {
  int opt;

  while ((opt = getopt(argc, argv, "f:i:s:h")) != -1) {
    switch (opt) {
      case 'f':
        if (!optarg) error_exit("from_port missing.\n");
        *from_port = (uint16_t)atoi(optarg);
        if (*from_port == 0) error_exit("from_port is invalid.\n");
        break;
      case 'i':
        if (!optarg) error_exit("interface missing.\n");
        *if_name = optarg;
        break;
      case 's':
        if (!optarg) error_exit("table_size missing.\n");
        *table_size = (uint16_t)atoi(optarg);
        if (*table_size == 0) error_exit("table_size is invalid.\n");
        break;
      case 'h':
        fprintf(stderr, "Usage: %s [-f from_port] [-i interface] [-s hash_table_size]\n", argv[0]);
        exit(EXIT_SUCCESS);
    }
  }
}

size_t read_line(char *buffer, int size) {
  printf(":> ");
  if (fgets(buffer, size, stdin) == NULL) {
    return 0;
  }
  size_t len = strlen(buffer);
  if (len > 0 && buffer[len - 1] == '\n') {
    buffer[len - 1] = '\0';
    --len;
  }
  return len;
}