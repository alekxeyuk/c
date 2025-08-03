#include <fcntl.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define PIPE_NAME "/tmp/testFIFO"
#define MSG_SIZE 128

int main(void) {
  setlocale(LC_ALL, "C.utf8");

  int fd;
  char msg[MSG_SIZE];

  if ((fd = open(PIPE_NAME, O_RDONLY)) < 0) {
    perror("open failed");
    exit(EXIT_FAILURE);
  }

  if (read(fd, msg, sizeof(msg)) < 0) {
    perror("read failed");
    exit(EXIT_FAILURE);
  }

  printf("received: [%.*s]\n", MSG_SIZE, msg);

  close(fd);

  if (remove(PIPE_NAME) < 0) {
    perror("remove failed");
    exit(EXIT_FAILURE);
  }

  exit(EXIT_SUCCESS);
}
