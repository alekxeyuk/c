#include <locale.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

int main(void) {
  setlocale(LC_ALL, "C.utf8");

  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set, SIGINT);
  sigaddset(&set, SIGTERM);

  if (sigprocmask(SIG_BLOCK, &set, NULL) < 0) {
    perror("sigprocmask failed");
    exit(EXIT_FAILURE);
  }

  pid_t pid = getpid();

  while (true) {
    printf("(%ld) nothing ever happens at (%d)\n", time(NULL), pid);
    usleep(1000000);
  }

  exit(EXIT_SUCCESS);
}
