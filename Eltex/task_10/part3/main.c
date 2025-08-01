#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_CMD_LEN 1024
#define MAX_ARGS 64

static bool process_command(char* argv[]) {
  if (strncmp("exit", argv[0], 4) == 0) {
    exit(EXIT_SUCCESS);
  }
  return false;
}

int main(void) {
  setlocale(LC_ALL, "C.utf8");

  int wstatus;
  pid_t cpid, w;
  char input[MAX_CMD_LEN];
  char* argv[MAX_ARGS];

  while (true) {
    printf(">> ");
    fflush(stdout);

    char* st = fgets(input, sizeof(input), stdin);
    if (st == NULL) {
      perror("fgets failed");
      exit(EXIT_FAILURE);
    }
    input[strcspn(input, "\n")] = '\0';

    int i = 0;
    char* token = strtok(input, " \t");
    while (token != NULL && i < MAX_ARGS - 1) {
      argv[i] = token;
      token = strtok(NULL, " \t");
      i++;
    }
    argv[i] = NULL;

    if (argv[0] == NULL) {
      continue;
    }

    process_command(argv);

    cpid = fork();
    switch (cpid) {
      case -1:
        // Error
        perror("fork failed");
        exit(EXIT_FAILURE);
      case 0:
        // Child
        execvp(argv[0], argv);
        perror("execvp failed");
        exit(EXIT_FAILURE);
      default:
        // Parent
        w = waitpid(cpid, &wstatus, 0);
        if (w == -1) {
          perror("waitpid failed");
          exit(EXIT_FAILURE);
        }
        printf("command exited with = %d\n", WEXITSTATUS(wstatus));
    }
  }

  exit(EXIT_SUCCESS);
}
