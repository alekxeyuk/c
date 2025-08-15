#include <fcntl.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>

#include "common.h"

int main(void) {
  setlocale(LC_ALL, "C.utf8");

  key_t key;
  int sem_id;
  int shm_fd;
  char *shm_addr;

  if ((key = ftok(SHM_PATH, 'A')) == -1) error_exit("ftok failed");

  if ((sem_id = semget(key, 0, 0)) == -1) error_exit("semget failed");

  if ((shm_fd = shmget(key, 0, 0)) == -1) error_exit("shmget failed");
  if ((shm_addr = shmat(shm_fd, NULL, 0)) == (char *)-1) error_exit("shmat failed");

  sem_post(sem_id, CLT_SEM_N);
  sem_wait(sem_id, SRV_SEM_N);
  printf("Server response: %s\n", shm_addr);

  strncpy(shm_addr, "Hello!", MSG_SIZE);
  sem_post(sem_id, CLT_SEM_N);

  if (shmdt(shm_addr) == -1) error_exit("shmdt failed");

  exit(EXIT_SUCCESS);
}