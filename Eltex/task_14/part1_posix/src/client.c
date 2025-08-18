#include <fcntl.h>
#include <locale.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "common.h"

int main(void) {
  setlocale(LC_ALL, "C.utf8");
  
  int shm_fd;
  void *shm_ptr;

  shm_fd = shm_open(SHM_NAME, O_RDWR, S_IRWXU);
  if (shm_fd == -1) {
    perror("shm_open failed");
    return EXIT_FAILURE;
  }

  shm_ptr = mmap(0, MSG_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (shm_ptr == MAP_FAILED) {
    perror("mmap failed");
    return EXIT_FAILURE;
  }

  sem_t *client_sem = sem_open(CLIENT_SEM_NAME, 0);
  sem_t *server_sem = sem_open(SERVER_SEM_NAME, 0);
  if (client_sem == SEM_FAILED || server_sem == SEM_FAILED) {
    perror("sem_open failed");
    return EXIT_FAILURE;
  }

  sem_post(client_sem);  // We are ready
  sem_wait(server_sem);  // Waiting for the server to respond
  printf("Server message: %s\n", (char *)shm_ptr);

  strcpy(shm_ptr, "Hello!");
  sem_post(client_sem);  // Signaling the server that we are done

  sem_close(client_sem);
  sem_close(server_sem);
  munmap(shm_ptr, MSG_SIZE);
  close(shm_fd);

  exit(EXIT_SUCCESS);
}