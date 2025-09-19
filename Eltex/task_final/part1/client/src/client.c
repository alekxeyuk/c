#define _GNU_SOURCE

#include "client.h"

#include <locale.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "../shared/src/client_lib.h"
#include "../shared/src/common.h"
#include "helper.h"

static volatile sig_atomic_t should_exit = 0;
int sock_fd = -1;

static void signal_handler(int signum) {
  (void)signum;
  should_exit = 1;
  close(sock_fd);
  printf("\nStoping... Press Enter\n");
}

int main(int argc, char *argv[]) {
  setlocale(LC_ALL, "C.utf8");

  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);

  char *dest_ip;
  uint16_t dest_port = 0, from_port = 0;
  const char *if_name = "eth0";

  parse_args(argc, argv, &dest_ip, &dest_port, &from_port, &if_name);
  if (!dest_ip || dest_port == 0 || from_port == 0) {
    fprintf(stderr, "Missing required arguments. Use -h for help\n");
    return EXIT_FAILURE;
  }

  operations_t plugin;
  void *handle = load_plugin(PLUGIN_NAME, &plugin);
  if (!handle) {
    fprintf(stderr, "Failed to load plugin %s\n", PLUGIN_NAME);
    return EXIT_FAILURE;
  }

  ClientSession *session = plugin.init(from_port, if_name);
  if (!session) {
    fprintf(stderr, "Failed to initialize client session\n");
    return CLIENT_ERROR;
  }
  sock_fd = session->sockfd;

  int result = plugin.connect(session, dest_ip, dest_port);
  if (result != CLIENT_SUCCESS) {
    fprintf(stderr, "Connection failed: %d\n", result);
    goto cleanup;
  }

  printf("Client running...\nEnter %s or press Ctrl+C to stop.\n", STOP_MESSAGE);
  while (!should_exit) {
    char buf[BUFFER_SIZE];
    read_line(buf, sizeof(buf));
    if (strncmp(buf, STOP_MESSAGE, sizeof(STOP_MESSAGE)) == 0) {
      printf("Stopping as requested...\n");
      should_exit = 1;
    }

    result = plugin.send_message(session, buf);
    if (result != CLIENT_SUCCESS) {
      fprintf(stderr, "Message send failed: %d\n", result);
      goto cleanup;
    }

    char response[BUFFER_SIZE];
    result = plugin.receive_response(session, response, sizeof(response));
    if (result == CLIENT_SUCCESS) {
      printf("<: %s\n", response);
    } else if (result == CLIENT_BUFFER_TOO_SMALL) {
      printf("Response too large for bufer\n");
    } else {
      fprintf(stderr, "Response receive failed : %d\n", result);
    }
  }

cleanup:
  plugin.disconnect(session);
  unload_plugins(handle);
  exit(EXIT_SUCCESS);
}
