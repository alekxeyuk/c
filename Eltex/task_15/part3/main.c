#include <locale.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

static void action(int sig) {
  static unsigned int counter = 0;
  printf("#%u (%ld) Received signal = [%d]\n", counter++, time(NULL), sig);
}

int main(void) {
  setlocale(LC_ALL, "C.utf8");

  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set, SIGUSR1);

  if (sigprocmask(SIG_BLOCK, &set, NULL) < 0) {
    perror("sigprocmask failed");
    exit(EXIT_FAILURE);
  }

  int sig;
  while (true) {
    sigwait(&set, &sig);
    action(sig);
  }

  exit(EXIT_SUCCESS);
}
