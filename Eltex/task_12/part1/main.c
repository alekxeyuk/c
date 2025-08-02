#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void) {
  setlocale(LC_ALL, "C.utf8");

  char buf;
  pid_t cpid;
  int pipefd[2];
  int wstatus;

  if (pipe(pipefd) == -1) {
    perror("pipe failed");
    exit(EXIT_FAILURE);
  }

  cpid = fork();
  switch (cpid) {
    case -1:
      // Error
      perror("fork failed");
      exit(EXIT_FAILURE);
    case 0:
      // Child
      close(pipefd[1]);  // In
      while (read(pipefd[0], &buf, 1) > 0) {
        putchar((int)buf);
      }
      putchar('\n');
      close(pipefd[0]);  // Out
      exit(EXIT_SUCCESS);
    default:
      // Parent
      close(pipefd[0]);  // Out
      write(pipefd[1], "Hi!", 4);
      close(pipefd[1]);  // In
      if (waitpid(cpid, &wstatus, 0) == -1) {
        perror("waitpid failed");
        exit(EXIT_FAILURE);
      }
  }
  exit(EXIT_SUCCESS);
}
