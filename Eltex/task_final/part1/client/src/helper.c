#define _GNU_SOURCE
#include "helper.h"

void* load_plugin(const char* name, operations_t* op_handle) {
  void* handle = dlopen(name, RTLD_LAZY);
  if (!handle) {
    fprintf(stderr, "dlopen(%s) failed: %s\n", name, dlerror());
    return NULL;
  }

  const operations_t* ops = (operations_t*)dlsym(handle, "export");
  if (!ops) {
    fprintf(stderr, "dlsym(operations) failed: %s\n", dlerror());
    dlclose(handle);
    return NULL;
  }

  memcpy(op_handle, ops, sizeof(operations_t));

  return handle;
}

void unload_plugins(void* handle) {
  printf("Unloading plugin...\n");
  dlclose(handle);
}

void parse_args(int argc, char* argv[], char** dest_ip, uint16_t* dest_port,
                uint16_t* from_port, const char** if_name) {
  int opt;

  while ((opt = getopt(argc, argv, "d:p:f:i:h")) != -1) {
    switch (opt) {
      case 'd':
        if (!optarg) error_exit("destination_ip missing.\n");
        *dest_ip = optarg;
        break;
      case 'p':
        if (!optarg) error_exit("destination_port missing.\n");
        *dest_port = (uint16_t)atoi(optarg);
        if (*dest_port == 0) error_exit("destination_port is invalid.\n");
        break;
      case 'f':
        if (!optarg) error_exit("from_port missing.\n");
        *from_port = (uint16_t)atoi(optarg);
        if (*from_port == 0) error_exit("from_port is invalid.\n");
        break;
      case 'i':
        if (!optarg) error_exit("interface missing.\n");
        *if_name = optarg;
        break;
      case 'h':
        fprintf(stderr,
                "Usage: %s [-d destination_ip] [-p destination_port] [-f "
                "from_port] [-i interface]\n",
                argv[0]);
        exit(EXIT_SUCCESS);
    }
  }
}

size_t read_line(char* buffer, int size) {
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

int add_to_epoll(int epoll_fd, int fd) {
  struct epoll_event event;
  event.events = EPOLLIN;
  event.data.fd = fd;
  return epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
}

int create_signalfd(void) {
  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGINT);
  sigaddset(&mask, SIGQUIT);
  sigaddset(&mask, SIGTERM);

  if (sigprocmask(SIG_BLOCK, &mask, NULL) < 0) err(EXIT_FAILURE, "sigprocmask");
  int sfd = signalfd(-1, &mask, 0);
  if (sfd < 0) err(EXIT_FAILURE, "signalfd");

  return sfd;
}