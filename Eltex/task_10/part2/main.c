#include <locale.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>


int main(void) {
  setlocale(LC_ALL, "C.utf8");

  int wstatus;
  pid_t cpid1, cpid2, w;

  cpid1 = fork();
  switch(cpid1) {
    case -1:
      // Error
      perror("fork failed");
      exit(EXIT_FAILURE);
      break;
    case 0:
      // Child
      printf("I am a child with pid = %jd and ppid = %jd\n", (intmax_t) getpid(), (intmax_t) getppid());
      exit(EXIT_SUCCESS);
      break;
    default:
      // Parent
      cpid2 = fork();
      printf("I am a parent with pid = %jd and ppid = %jd\n", (intmax_t) getpid(), (intmax_t) getppid());
      w = waitpid(cpid1, &wstatus, 0);
      if (w == -1) {
        perror("waitpid failed");
        exit(EXIT_FAILURE);
      }
      printf("Child process ended with status = %d\n", WEXITSTATUS(wstatus));
      break;
  }

  return EXIT_SUCCESS;
}
