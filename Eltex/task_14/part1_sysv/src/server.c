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
  union semun sem_union;
  unsigned short sem_array[SEM_SIZE] = {0, 0};

  if ((key = ftok(SHM_PATH, 'A')) == -1) error_exit("ftok failed");

  if ((sem_id = semget(key, SEM_SIZE, IPC_CREAT | S_IRWXU)) == -1) error_exit("semget failed");

  if ((shm_fd = shmget(key, MSG_SIZE, IPC_CREAT | S_IRWXU)) == -1) error_exit("shmget failed");
  if ((shm_addr = shmat(shm_fd, NULL, 0)) == (char *)-1) error_exit("shmat failed");

  sem_union.array = sem_array;
  if (semctl(sem_id, 0, SETALL, sem_union) == -1) error_exit("semctl failed");

  sem_wait(sem_id, CLT_SEM_N);
  strncpy(shm_addr, "Hi!", MSG_SIZE);
  sem_post(sem_id, SRV_SEM_N);

  sem_wait(sem_id, CLT_SEM_N);
  printf("Client response: %s\n", shm_addr);

  if (shmdt(shm_addr) == -1) error_exit("shmdt failed");
  if (shmctl(shm_fd, IPC_RMID, NULL) == -1) error_exit("shmctl failed");
  if (semctl(sem_id, 0, IPC_RMID) == -1) error_exit("semctl failed");

  exit(EXIT_SUCCESS);
}
