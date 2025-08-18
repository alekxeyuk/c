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

  shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, S_IRWXU);
  if (shm_fd == -1) {
    perror("shm_open failed");
    return EXIT_FAILURE;
  }

  if (ftruncate(shm_fd, MSG_SIZE) == -1) {
    perror("ftruncate failed");
    return EXIT_FAILURE;
  }

  shm_ptr = mmap(0, MSG_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (shm_ptr == MAP_FAILED) {
    perror("mmap failed");
    return EXIT_FAILURE;
  }

  sem_t *client_sem = sem_open(CLIENT_SEM_NAME, O_CREAT, S_IRWXU, 0);
  sem_t *server_sem = sem_open(SERVER_SEM_NAME, O_CREAT, S_IRWXU, 0);
  if (client_sem == SEM_FAILED || server_sem == SEM_FAILED) {
    perror("sem_open failed");
    return EXIT_FAILURE;
  }

  sem_wait(client_sem);  // Waiting for the client initialization
  strcpy(shm_ptr, "Hi!");
  sem_post(server_sem);  // Signaling the client that he can read our response

  sem_wait(client_sem);  // Waiting for response from client
  printf("Client response: %s\n", (char *)shm_ptr);

  sem_close(client_sem);
  sem_close(server_sem);
  sem_unlink(CLIENT_SEM_NAME);
  sem_unlink(SERVER_SEM_NAME);
  munmap(shm_ptr, MSG_SIZE);
  close(shm_fd);
  shm_unlink(SHM_NAME);

  exit(EXIT_SUCCESS);
}
