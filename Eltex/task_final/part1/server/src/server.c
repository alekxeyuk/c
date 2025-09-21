#define _GNU_SOURCE

#include "server.h"

#include <locale.h>
#include <search.h>
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

  uint16_t from_port = 0, table_size = DEFAULT_TABLE_SIZE;
  const char *if_name = DEFAULT_IF_NAME;

  parse_args(argc, argv, &from_port, &if_name, &table_size);
  if (from_port == 0) {
    fprintf(stderr, "Missing required arguments. Use -h for help\n");
    return EXIT_FAILURE;
  }

  ENTRY e;
  ENTRY *ep;
  if (hcreate(table_size) == 0) {
    error_exit("Failed to init hash table\n");
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
    goto cleanup;
  }
  sock_fd = session->sockfd;

  printf("Server running...\nPress Ctrl+C to stop.\n");
  while (!should_exit) {
    char response[BUFFER_SIZE];
    struct sockaddr_in addr;
    uint16_t src_port = 0;

    int result = plugin.receive(session, response, sizeof(response), &addr, &src_port);
    char src_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr.sin_addr, src_ip, INET_ADDRSTRLEN);
    if (result == CLIENT_SUCCESS) {
      printf("<: %s from %s:%d\n", response, src_ip, src_port);
    } else if (result == CLIENT_BUFFER_TOO_SMALL) {
      printf("Response too large for bufer\n");
      continue;
    } else if (result != CLIENT_NOT_OURS) {
      fprintf(stderr, "Response receive failed : %d\n", result);
      continue;
    } else {
      continue;
    }

    result = plugin.connect(session, src_ip, src_port);
    if (result != CLIENT_SUCCESS) {
      fprintf(stderr, "Connection failed: %d\n", result);
      continue;
    }

    char key[INET_ADDRSTRLEN + 6];
    snprintf(key, sizeof(key), "%d%s", src_port, src_ip);
    e.key = key;
    e.data = (void *)0;
    if ((ep = hsearch(e, ENTER)) != NULL) {
      ep->data = (void *)((uintptr_t)(ep->data) + 1);
    }

    if (strncmp(response, STOP_MESSAGE, sizeof(STOP_MESSAGE)) == 0) {
      printf("Stopping is requested...\n");
      ep->data = (void *)0;
    }

    char buf[RESPONSE_SIZE];
    snprintf(buf, sizeof(buf), "%s %ld", response, ep ? (uintptr_t)(ep->data) : 1);
    result = plugin.send_message(session, buf);
    if (result != CLIENT_SUCCESS) {
      fprintf(stderr, "Message send failed: %d\n", result);
    }
    session->is_connected = 0;
  }

cleanup:
  close(session->sockfd);
  free(session);
  unload_plugins(handle);
  hdestroy();
  exit(EXIT_SUCCESS);
}
