#include "client.h"

#include <ctype.h>
#include <locale.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "common.h"
#include "ui.h"

extern char *name;
extern char input_buffer[MAX_MSG_SIZE];
extern char messages[MAX_MESSAGES][MAX_MSG_SIZE];
extern char users[MAX_USERS][MAX_USERNAME_SIZE];
extern int msg_count;
extern int user_count;
extern pthread_mutex_t buffer_mut;
extern int running;
static pthread_mutex_t sender_mut = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t sender_cond = PTHREAD_COND_INITIALIZER;
static msgtype message_type = MNOOP;

static pthread_mutex_t *log_mut = NULL;
static pthread_mutex_t *users_mut = NULL;

static int mqid = -1;
static pid_t pid = -1;
static size_t client_length = sizeof(clientmsg) - sizeof(long);
static size_t server_length = sizeof(servermsg) - sizeof(long);

int init_message_queue(const char *path, char key_char) {
  key_t key;
  pid = getpid();

  if ((key = ftok(path, key_char)) == -1) {
    perror("Client: ftok failed.");
    return -1;
  }

  if ((mqid = msgget(key, S_IRWXU)) == -1) {
    perror("Client: msgget failed.");
    return -1;
  }
  return 1;
}

static void *con_thread_func(void *arg) {
  (void)arg;
  servermsg m;

  while (running) {
    if (msgrcv(mqid, &m, server_length, (long)pid, 0) == -1) {
      perror("Client: msgrcv failed.");
      pthread_exit(NULL);
    }
    if (m.type == MJOIN || m.type == MLEAVE || m.type == MLIST_USERS) {
      const char *action = (m.type == MJOIN) ? "joined" : (m.type == MLEAVE) ? "left" : "user list updated";
      if (msg_count < MAX_MESSAGES) {
        snprintf(messages[msg_count], MAX_MSG_SIZE, "%s has %s", m.data.userlist.username, action);
        msg_count++;
      }
      if (m.type == MJOIN) {
        if (user_count < MAX_USERS) {
          snprintf(users[user_count], MAX_USERNAME_SIZE, "%s", m.data.userlist.username);
          user_count++;
        }
      } else if (m.type == MLEAVE) {
        for (int i = 0; i < user_count; i++) {
          if (strcmp(users[i], m.data.userlist.username) == 0) {
            for (int j = i; j < user_count - 1; j++) {
              strcpy(users[j], users[j + 1]);
            }
            user_count--;
            break;
          }
        }
      }
      update_ui(UUSER);
    } else if (m.type == MMESSAGE) {
      if (msg_count < MAX_MESSAGES) {
        int can_fit = 64 - (int)strlen(m.data.chat.username) - 2;
        snprintf(messages[msg_count], MAX_MSG_SIZE, "%s: %.*s", m.data.chat.username, can_fit, m.data.chat.msgtext);
        msg_count++;
      }
    }
    update_ui(ULOG);
  }
  return NULL;
}

int start_consumer_thread(pthread_t *ui_thread, pthread_mutex_t *l_mut, pthread_mutex_t *u_mut) {
  if (mqid == -1) {
    fprintf(stderr, "Message queue not initialized.\n");
    return -1;
  }
  log_mut = l_mut;
  users_mut = u_mut;
  return pthread_create(ui_thread, NULL, con_thread_func, NULL);
}

static void *pub_thread_func(void *arg) {
  (void)arg;
  clientmsg message;
  message.mtype = SERVER_MTYPE;
  message.pid = (long)pid;

  while (running) {
    pthread_mutex_lock(&sender_mut);
    while (message_type == MNOOP) {
      pthread_cond_wait(&sender_cond, &sender_mut);
    }
    message.type = message_type;
    switch (message_type) {
      case MSTOP:
        message.type = MLEAVE;
        /* FALLTHROUGH */
      case MJOIN:
      case MLEAVE:
        snprintf(message.body, MAX_MSG_SIZE, "%.*s", MAX_USERNAME_SIZE, name);
        break;
      case MMESSAGE:
        snprintf(message.body, MAX_MSG_SIZE, "%.*s", MAX_MSG_SIZE - 1, input_buffer);
        pthread_mutex_unlock(&buffer_mut);
        break;
      case MLIST_USERS:
      case MNOOP:
        break;
    }
    msgsnd(mqid, &message, client_length, 0);
    message_type = MNOOP;
    pthread_mutex_unlock(&sender_mut);
  }

  return NULL;
}

int start_publisher_thread(pthread_t *ui_thread) {
  if (mqid == -1) {
    fprintf(stderr, "Message queue not initialized.\n");
    return -1;
  }
  return pthread_create(ui_thread, NULL, pub_thread_func, NULL);
}

void send_message(msgtype type) {
  pthread_mutex_lock(&sender_mut);
  message_type = type;
  if (type == MMESSAGE) {
    pthread_mutex_lock(&buffer_mut);
  }
  pthread_cond_signal(&sender_cond);
  pthread_mutex_unlock(&sender_mut);
}

void close_message_queue(void) {
  if (mqid != -1 && msgctl(mqid, IPC_RMID, NULL) == -1) {
    perror("Client: msgctl failed.");
  } else {
    mqid = -1;
  }
}
