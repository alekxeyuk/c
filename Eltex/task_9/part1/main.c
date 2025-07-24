#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

int main(void) {
  int fd;
  mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
  const char *pathname = "output.txt";

  // Write
  fd = open(pathname, O_WRONLY | O_CREAT | O_TRUNC, mode);
  if (fd == -1) {
    perror("Was not able to open a file");
    return EXIT_FAILURE;
  }
  char buf[] = "String from file\n";
  ssize_t wrote = write(fd, buf, sizeof(buf));
  if (wrote == -1) {
    perror("Was not able to write to a file");
    return EXIT_FAILURE;
  }
  close(fd);

  // Read backwards
  fd = open(pathname, O_RDONLY, mode);
  if (fd == -1) {
    perror("Was not able to open a file");
    return EXIT_FAILURE;
  }
  char r_buff[1] = {0};
  for (off_t pos = lseek(fd, 0, SEEK_END); pos >= 0; --pos) {
    off_t cur_pos = lseek(fd, pos, SEEK_SET);
    if (cur_pos == -1) {
      perror("Was not able to seek");
      return EXIT_FAILURE;
    }
    ssize_t r_status = read(fd, r_buff, 1);
    if (r_status == -1) {
      perror("Was not able to read");
      return EXIT_FAILURE;
    }

    printf("%c", r_buff[0]);
  }
  printf("\n");
  close(fd);
  return EXIT_SUCCESS;
}