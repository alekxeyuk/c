#ifndef COMMON_H_
#define COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>

#define error_exit(msg) \
  do {                  \
    perror(msg);        \
    exit(EXIT_FAILURE); \
  } while (0)

#define SHM_PATH "/tmp"
#define MSG_SIZE 32
#define SEM_SIZE 2
#define SRV_SEM_N 0
#define CLT_SEM_N 1

union semun {
  int val;                // Value for SETVAL
  struct semid_ds *buf;   // Buffer for IPC_STAT, IPC_SET
  unsigned short *array;  // Array for GETALL, SETALL
  struct seminfo *__buf;  // Buffer for IPC_INFO (Linux-specific)
};

// op can be -1 (wait) or 1 (signal)
void sem_do(int sem_id, unsigned short sem_num, short op, short sem_flg);
void sem_wait(int sem_id, unsigned short sem_num);
void sem_post(int sem_id, unsigned short sem_num);

#endif  // COMMON_H_