#include <ctype.h>
#include <errno.h>
#include <locale.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static bool correct_pid_t(long v) {
  pid_t p = (pid_t)v;
  return (long)p == v;
}

static bool parse_pid(const char* s, pid_t* out) {
  errno = 0;

  while (isspace((unsigned char)*s)) s++;
  if (*s == '\0') return false;

  char* end = NULL;
  long val = strtol(s, &end, 10);

  if (errno == ERANGE || end == s || val < 0) return false;
  if (!correct_pid_t(val)) return false;

  *out = (pid_t)val;
  return true;
}

int main(void) {
  setlocale(LC_ALL, "C.utf8");

  char buf[128];

  printf("Please provide PID: ");

  if (!fgets(buf, sizeof(buf), stdin)) {
    perror("fgets failed");
    exit(EXIT_FAILURE);
  }

  buf[strcspn(buf, "\n")] = '\0';

  pid_t p;
  if (!parse_pid(buf, &p)) {
    fprintf(stderr, "invalid PID\n");
    exit(EXIT_FAILURE);
  }

  printf("I will send SIGUSR1 signals to the PID you provided\n");
  while (true) {
    if (kill(p, SIGUSR1) < 0) {
      perror("kill failed");
      exit(EXIT_FAILURE);
    }
    usleep(10000);
  }

  exit(EXIT_SUCCESS);
}
