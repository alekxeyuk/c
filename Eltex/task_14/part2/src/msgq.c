#include "msgq.h"

#include <errno.h>
#include <fcntl.h>
#include <locale.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

static int init_waiter(shmq_t *q, int index) {
  if (index < 0 || index >= MAX_USERS) return -1;
  if (sem_init(&q->waiters[index], 1, 1) != 0) return -1;
  return 0;
}

static sem_t *get_waiter(shmq_t *q, long pid) {
  for (size_t i = 0; i < MAX_USERS; i++) {
    if (q->pids[i] == pid) {
      return &q->waiters[i];
    }
  }
  return NULL;
}

static sem_t *wait_for(shmq_t *q, long pid) {
  for (int i = 0; i < MAX_USERS; i++) {
    if (q->pids[i] == 0) {
      q->pids[i] = pid;
      if (init_waiter(q, i) != 0) return NULL;
      return &q->waiters[i];
    }
  }
  return NULL;
}

static int init_shared(shmq_t *q) {
  if (q->magic == SHMQ_MAGIC) return 0;

  if (sem_init(&q->mutex, 1, 1) != 0) return -1;
  if (sem_init(&q->spaces, 1, MAX_QUEUE_SIZE) != 0) return -1;

  q->w_count = 0;
  q->count = 0;
  memset(q->messages, 0, sizeof(q->messages));
  memset(q->waiters, 0, sizeof(q->waiters));
  memset(q->pids, 0, sizeof(q->pids));

  q->magic = SHMQ_MAGIC;
  return 0;
}

static int cleanup_shared(shmq_t *q) {
  if (q->magic != SHMQ_MAGIC) return -1;

  sem_destroy(&q->mutex);
  sem_destroy(&q->spaces);
  for (size_t i = 0; i < MAX_USERS; i++) {
    sem_destroy(&q->waiters[i]);
    q->pids[i] = 0;
  }
  q->w_count = 0;

  q->count = 0;
  q->magic = 0;

  munmap(q, SHM_SIZE);
  return 0;
}

int shmq_create(const char *name, shmq_handle_t *handle, long pid) {
  int fd = shm_open(name, O_CREAT | O_RDWR, S_IRWXU);
  if (fd < 0) return errno;

  if (ftruncate(fd, SHM_SIZE) != 0) {
    int e = errno;
    close(fd);
    shm_unlink(name);
    return e;
  }

  void *addr = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (addr == MAP_FAILED) {
    int e = errno;
    close(fd);
    shm_unlink(name);
    return e;
  }

  shmq_t *queue = (shmq_t *)addr;
  memset(queue, 0, SHM_SIZE);

  if (init_shared(queue) != 0) {
    int e = errno ? errno : EFAULT;
    munmap(queue, SHM_SIZE);
    close(fd);
    shm_unlink(name);
    return e;
  }

  handle->fd = fd;
  handle->queue = queue;
  handle->waiting_for = wait_for(queue, pid);

  return 0;
}

int shmq_open(const char *name, shmq_handle_t *handle, long pid) {
  int fd = shm_open(name, O_RDWR, S_IRWXU);
  if (fd < 0) return errno;

  void *addr = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (addr == MAP_FAILED) {
    int e = errno;
    close(fd);
    return e;
  }

  shmq_t *queue = (shmq_t *)addr;

  if (queue->magic != SHMQ_MAGIC) {
    munmap(queue, SHM_SIZE);
    close(fd);
    return -1;
  }

  handle->fd = fd;
  handle->queue = queue;
  handle->waiting_for = wait_for(queue, pid);

  return 0;
}

int msgctl(shmq_handle_t *handle, int cmd) {
  if (cmd == IPC_RMID) {
    if (handle->queue) {
      cleanup_shared(handle->queue);
      close(handle->fd);
      shm_unlink(SERVER_SHM_NAME);
      handle->fd = -1;
      handle->queue = NULL;
    }
    return 0;
  }
  return -1;
}

int msgsnd(shmq_handle_t *handle, const void *msg, size_t msgsz, long mtype) {
  if (!handle || !handle->queue || !msg) return -1;

  shmq_t *queue = handle->queue;

  sem_wait(&queue->spaces);
  sem_wait(&queue->mutex);

  if (queue->count >= MAX_QUEUE_SIZE) {
    sem_post(&queue->mutex);
    sem_post(&queue->spaces);
    return -1;
  }

  int idx = queue->count;
  shared_message_t *message = &queue->messages[idx];

  message->mtype = mtype;
  message->used = true;
  memcpy(&message->data.server_msg, msg, msgsz);
  queue->count++;

  sem_t *waiter = get_waiter(queue, mtype);
  if (waiter) {
    sem_post(waiter);
  }

  sem_post(&queue->mutex);

  return 0;
}

int msgrcv(shmq_handle_t *handle, const void *msg, size_t msgsz, long mtype) {
  if (!handle || !handle->queue || !msg) return -1;

  shmq_t *queue = handle->queue;

  while (true) {
    sem_wait(handle->waiting_for);
    sem_wait(&queue->mutex);
    for (int i = 0; i < queue->count; i++) {
      if (queue->messages[i].used && queue->messages[i].mtype == mtype) {
        memcpy((void *)msg, &queue->messages[i].data, msgsz);
        queue->messages[i].used = false;
        queue->count--;
        for (int j = i; j < queue->count; j++) {
          queue->messages[j] = queue->messages[j + 1];
        }
        sem_post(&queue->mutex);
        sem_post(&queue->spaces);
        return 0;
      }
    }
    sem_post(&queue->mutex);
  }

  return -1;
}

int clean_client(shmq_handle_t *handle, long pid) {
  if (!handle || !handle->queue) return -1;

  shmq_t *queue = handle->queue;

  sem_wait(&queue->mutex);
  for (size_t i = 0; i < MAX_USERS; i++) {
    if (queue->pids[i] == pid) {
      sem_destroy(&queue->waiters[i]);
      queue->pids[i] = 0;
      break;
    }
  }
  sem_post(&queue->mutex);

  munmap(queue, SHM_SIZE);
  close(handle->fd);
  
  return 0;
}
