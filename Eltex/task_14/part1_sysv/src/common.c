#include "common.h"

void sem_do(int sem_id, unsigned short sem_num, short op, short sem_flg) {
  struct sembuf sem_op;
  sem_op.sem_num = sem_num;
  sem_op.sem_op = op;
  sem_op.sem_flg = sem_flg;

  if (semop(sem_id, &sem_op, 1) == -1) error_exit("semop failed");
}

void sem_wait(int sem_id, unsigned short sem_num) { sem_do(sem_id, sem_num, -1, 0); }

void sem_post(int sem_id, unsigned short sem_num) { sem_do(sem_id, sem_num, 1, 0); }