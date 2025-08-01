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
  pid_t cpid3, cpid4, cpid5;

  cpid1 = fork();
  switch(cpid1) {
    case -1:
      // Error
      perror("fork failed");
      exit(EXIT_FAILURE);
    case 0:
      // Child 1
      cpid3 = fork();
      switch(cpid3) {
        case -1:
          perror("fork failed");
          exit(EXIT_FAILURE);
        case 0:
          // Child 3
          printf("I am a child-3 with pid = %jd and ppid = %jd\n", (intmax_t) getpid(), (intmax_t) getppid());
          exit(EXIT_SUCCESS);
        default:
          // Parent
          cpid4 = fork();
          switch(cpid4) {
            case -1:
              perror("fork failed");
              exit(EXIT_FAILURE);
            case 0:
              // Child 4
              printf("I am a child-4 with pid = %jd and ppid = %jd\n", (intmax_t) getpid(), (intmax_t) getppid());
              exit(EXIT_SUCCESS);
            default:
              printf("I am a child-1 with pid = %jd and ppid = %jd\n", (intmax_t) getpid(), (intmax_t) getppid());
              w = waitpid(cpid3, &wstatus, 0);
              if (w == -1) {
                perror("waitpid failed");
                exit(EXIT_FAILURE);
              }
              printf("Child-3 process ended with status = %d\n", WEXITSTATUS(wstatus));
              w = waitpid(cpid4, &wstatus, 0);
              if (w == -1) {
                perror("waitpid failed");
                exit(EXIT_FAILURE);
              }
              printf("Child-4 process ended with status = %d\n", WEXITSTATUS(wstatus));
              exit(EXIT_SUCCESS);
          }
      }
    default:
      // Parent
      cpid2 = fork();
      switch(cpid2) {
        case -1:
          // Error
          perror("fork failed");
          exit(EXIT_FAILURE);
        case 0:
          // Child 2
          cpid5 = fork();
          switch(cpid5) {
            case -1:
              // Error
              perror("fork failed");
              exit(EXIT_FAILURE);
            case 0:
              // Child 5
              printf("I am a child-5 with pid = %jd and ppid = %jd\n", (intmax_t) getpid(), (intmax_t) getppid());
              exit(EXIT_SUCCESS);
            default:
              printf("I am a child-2 with pid = %jd and ppid = %jd\n", (intmax_t) getpid(), (intmax_t) getppid());
              w = waitpid(cpid5, &wstatus, 0);
              if (w == -1) {
                perror("waitpid failed");
                exit(EXIT_FAILURE);
              }
              printf("Child-5 process ended with status = %d\n", WEXITSTATUS(wstatus));
              exit(EXIT_SUCCESS);
          }
        default:
          printf("I am a parent with pid = %jd and ppid = %jd\n", (intmax_t) getpid(), (intmax_t) getppid());
          w = waitpid(cpid1, &wstatus, 0);
          if (w == -1) {
            perror("waitpid failed");
            exit(EXIT_FAILURE);
          }
          printf("Child-1 process ended with status = %d\n", WEXITSTATUS(wstatus));
          w = waitpid(cpid2, &wstatus, 0);
          if (w == -1) {
            perror("waitpid failed");
            exit(EXIT_FAILURE);
          }
          printf("Child-2 process ended with status = %d\n", WEXITSTATUS(wstatus));
     }
  }

  return EXIT_SUCCESS;
}
