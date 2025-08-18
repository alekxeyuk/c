#ifndef MSGQ_H_
#define MSGQ_H_

#include <semaphore.h>
#include <stdbool.h>

#include "common.h"

#define SERVER_SHM_NAME "/chat_shared_memory"
#define SHM_SIZE (sizeof(shmq_t))
#define MAX_QUEUE_SIZE 128

typedef struct {
  long mtype;
  bool used;
  union {
    servermsg_t server_msg;
    clientmsg_t client_msg;
  } data;
} shared_message_t;

typedef struct {
  sem_t mutex;
  sem_t spaces;
  sem_t waiters[MAX_USERS];
  long pids[MAX_USERS];
  size_t w_count;

  int count;
  shared_message_t messages[MAX_QUEUE_SIZE];

  u_int8_t magic;
} shmq_t;

typedef struct {
  int fd;
  shmq_t *queue;
  sem_t *waiting_for;
} shmq_handle_t;

static const u_int8_t SHMQ_MAGIC = 0xAF;

int shmq_create(const char *name, shmq_handle_t *handle, long pid);
int shmq_open(const char *name, shmq_handle_t *handle, long pid);
int msgctl(shmq_handle_t *handle, int cmd);
int msgsnd(shmq_handle_t *handle, const void *msg, size_t msgsz, long mtype);
int msgrcv(shmq_handle_t *handle, const void *msg, size_t msgsz, long mtype);

#endif  // MSGQ_H_