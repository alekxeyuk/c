#include <locale.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

static void action(int sig) {
  printf("(%ld) Received signal = [%d]\n", time(NULL), sig);
}

int main(void) {
  setlocale(LC_ALL, "C.utf8");

  struct sigaction sa;
  sa.sa_handler = action;
  sigemptyset(&sa.sa_mask);
  sigaddset(&sa.sa_mask, SIGUSR1);
  sa.sa_flags = SA_RESTART;

  if (sigaction(SIGUSR1, &sa, NULL) < 0) {
    perror("sigaction failed");
    exit(EXIT_FAILURE);
  }

  while (true) {
    usleep(1000);
  }

  exit(EXIT_SUCCESS);
}
