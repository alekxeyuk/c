#include <locale.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "helper.h"

int main(void) {
  setlocale(LC_ALL, "C.utf8");

  int wstatus;
  pid_t cpid, w;
  char input[MAX_CMD_LEN];
  char* argv[MAX_ARGS];
  char* subs[MAX_ARGS];

  while (true) {
    printf(">> ");
    fflush(stdout);

    char* st = fgets(input, sizeof(input), stdin);
    if (st == NULL) {
      perror("fgets failed");
      exit(EXIT_FAILURE);
    }
    input[strcspn(input, "\n")] = '\0';

    int subs_n = split_str(input, subs, "|");

    process_command(subs);

    int prev_fd = STDIN_FILENO;
    int p[2];

    for (int i = 0; i < subs_n; i++) {
      if (i < subs_n - 1) {
        pipe(p);
      }

      cpid = fork();
      switch (cpid) {
        case -1:
          // Error
          perror("fork failed");
          exit(EXIT_FAILURE);
        case 0:
          // Child
          if (prev_fd != STDIN_FILENO) {
            // Reading from pipe
            dup2(prev_fd, STDIN_FILENO);
            close(prev_fd);
          }
          if (i < subs_n - 1) {
            // Writing in pipe
            dup2(p[1], STDOUT_FILENO);
            close(p[0]);
            close(p[1]);
          }
          split_str(subs[i], argv, " \t");
          execvp(argv[0], argv);
          perror("execvp failed");
          exit(EXIT_FAILURE);
        default:
          // Parent
          if (prev_fd != STDIN_FILENO) {
            close(prev_fd);
          }
          if (i < subs_n - 1) {
            close(p[1]);
            prev_fd = p[0];
          }
      }
    }

    w = waitpid(cpid, &wstatus, 0);
    if (w == -1) {
      perror("waitpid failed");
      exit(EXIT_FAILURE);
    }
    printf("command exited with = %d\n", WEXITSTATUS(wstatus));
  }

  exit(EXIT_SUCCESS);
}
