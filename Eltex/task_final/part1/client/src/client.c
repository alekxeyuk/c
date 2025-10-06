#define _GNU_SOURCE

#include "client.h"

#include <err.h>
#include <errno.h>
#include <locale.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/signalfd.h>
#include <time.h>
#include <unistd.h>

#include "../shared/src/client_lib.h"
#include "../shared/src/common.h"
#include "helper.h"

static volatile sig_atomic_t should_exit = 0;
int sock_fd = -1;

int main(int argc, char* argv[]) {
  setlocale(LC_ALL, "C.utf8");

  int sfd = create_signalfd();

  int epoll_fd = epoll_create1(0);
  if (epoll_fd == -1) err(EXIT_FAILURE, "epoll_create1");

  if (add_to_epoll(epoll_fd, STDIN_FILENO) != 0) err(EXIT_FAILURE, "epoll_ctl");
  if (add_to_epoll(epoll_fd, sfd) != 0) err(EXIT_FAILURE, "epoll_ctl");

  char* dest_ip;
  uint16_t dest_port = 0, from_port = 0;
  const char* if_name = "eth0";

  parse_args(argc, argv, &dest_ip, &dest_port, &from_port, &if_name);
  if (!dest_ip || dest_port == 0 || from_port == 0) {
    fprintf(stderr, "Missing required arguments. Use -h for help\n");
    return EXIT_FAILURE;
  }

  operations_t plugin;
  void* handle = load_plugin(PLUGIN_NAME, &plugin);
  if (!handle) {
    fprintf(stderr, "Failed to load plugin %s\n", PLUGIN_NAME);
    return EXIT_FAILURE;
  }

  ClientSession* session = plugin.init(from_port, if_name);
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

  printf("Client running...\nEnter %s or press Ctrl+C to stop.\n",
         STOP_MESSAGE);

  struct epoll_event events[2];
  while (!should_exit) {
    int nfds = epoll_wait(epoll_fd, events, 2, -1);
    if (nfds == -1) {
      if (errno == EINTR) continue;
      perror("epoll_wait failed");
      break;
    }

    for (int i = 0; i < nfds; i++) {
      if (events[i].data.fd == STDIN_FILENO) {
        char buf[BUFFER_SIZE];
        ssize_t n = read(STDIN_FILENO, buf, sizeof(buf) - 1);
        if (n <= 0) {
          if (n == 0 || (n < 0 && errno != EAGAIN && errno != EINTR)) {
            should_exit = 1;
            break;
          }
          continue;
        }

        buf[n] = '\0';

        if (n > 0 && buf[n - 1] == '\n') {
          buf[n - 1] = '\0';
        }

        if (strncmp(buf, STOP_MESSAGE, sizeof(STOP_MESSAGE)) == 0) {
          printf("Stopping as requested...\n");
          plugin.send_message(session, STOP_MESSAGE);
          should_exit = 1;
          break;
        }

        result = plugin.send_message(session, buf);
        if (result != CLIENT_SUCCESS) {
          fprintf(stderr, "Message send failed: %d\n", result);
          should_exit = 1;
          break;
        }

        char response[RESPONSE_SIZE];
        for (; !should_exit;) {
          result =
              plugin.receive(session, response, sizeof(response), NULL, NULL);
          if (result == CLIENT_SUCCESS) {
            printf("<: %s\n", response);
            break;
          } else if (result == CLIENT_BUFFER_TOO_SMALL) {
            printf("Response too large for buffer\n");
            break;
          } else if (result == CLIENT_NOT_OURS) {
            continue;
          } else {
            fprintf(stderr, "Response receive failed: %d\n", result);
            should_exit = 1;
            break;
          }
        }
      } else if (events[i].data.fd == sfd) {
        struct signalfd_siginfo siginfo;
        ssize_t s = read(sfd, &siginfo, sizeof(siginfo));
        if (s == sizeof(siginfo)) {
          if (siginfo.ssi_signo == SIGINT || siginfo.ssi_signo == SIGTERM ||
              siginfo.ssi_signo == SIGQUIT) {
            printf("\nReceived signal %d, stopping...\n", siginfo.ssi_signo);
            should_exit = 1;
            break;
          }
        }
      }
    }
  }

cleanup:
  if (session) {
    close(session->sockfd);
    free(session);
  }
  if (handle) {
    unload_plugins(handle);
  }
  if (epoll_fd != -1) {
    close(epoll_fd);
  }
  if (sfd != -1) {
    close(sfd);
  }
  exit(EXIT_SUCCESS);
}
