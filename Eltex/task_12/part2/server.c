#include <errno.h>
#include <fcntl.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define PIPE_NAME "/tmp/testFIFO"

int main(void) {
  setlocale(LC_ALL, "C.utf8");

  if (mkfifo(PIPE_NAME, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) < 0) {
    if (errno != EEXIST) {
      perror("mkfifo failed");
      exit(EXIT_FAILURE);
    }
  }

  int fd;
  char msg[] = "Hi!";

  if ((fd = open(PIPE_NAME, O_WRONLY)) < 0) {
    perror("open failed");
    exit(EXIT_FAILURE);
  }

  if (write(fd, msg, strnlen(msg, 4)) < 0) {
    perror("write failed");
    exit(EXIT_FAILURE);
  }

  close(fd);

  exit(EXIT_SUCCESS);
}
