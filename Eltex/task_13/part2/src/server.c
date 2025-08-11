#include <locale.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "common.h"

static int mqid = -1;
static user users[MAX_USERS] = {0};
static int nusers = 0;
static size_t client_length = sizeof(clientmsg) - sizeof(long);
static size_t server_length = sizeof(servermsg) - sizeof(long);

static void cleanup(int signum) {
  (void)signum;
  if (mqid != -1 && msgctl(mqid, IPC_RMID, NULL) == -1) {
    perror("Server: msgctl failed.");
  } else {
    printf("Server: Message queue removed successfully.\n");
  }
  exit(EXIT_SUCCESS);
}

static void broadcast_to_all(msgtype type, const char *username, const char *msg) {
  servermsg m;
  m.type = type;

  if (type == MJOIN || type == MLEAVE || type == MLIST_USERS) {
    snprintf(m.data.userlist.username, MAX_USERNAME_SIZE, "%.*s", MAX_USERNAME_SIZE - 1, username);
  } else if (type == MMESSAGE) {
    snprintf(m.data.chat.username, MAX_USERNAME_SIZE, "%.*s", MAX_USERNAME_SIZE - 1, username);
    snprintf(m.data.chat.msgtext, MAX_MSG_SIZE, "%.*s", MAX_MSG_SIZE - 1, msg);
  }

  for (int i = 0; i < nusers; i++) {
    m.mtype = users[i].pid;
    if (msgsnd(mqid, &m, server_length, 0) == -1) {
      perror("Server: msgsnd failed.");
    }
  }
}

static void send_user_list(long pid) {
  servermsg m;
  m.type = MJOIN;
  m.mtype = pid;

  for (int i = 0; i < nusers; i++) {
    if (users[i].pid != pid) {
      snprintf(m.data.userlist.username, MAX_USERNAME_SIZE, "%.*s", MAX_USERNAME_SIZE - 1, users[i].username);
      if (msgsnd(mqid, &m, server_length, 0) == -1) {
        perror("Server: msgsnd failed.");
      }
    }
  }
}

static user *find_user_by_pid(long pid) {
  for (int i = 0; i < nusers; i++) {
    if (users[i].pid == pid) {
      return &users[i];
    }
  }
  return NULL;
}

int main(int argc, char *argv[]) {
  setlocale(LC_ALL, "C.utf8");

  key_t key;
  clientmsg message;
  user *userp = NULL;

  if (argc != 2) {
    fprintf(stderr, "Usage: %s <path_to_key_file>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  if ((key = ftok(argv[1], 'A')) == -1) {
    perror("Server: ftok failed.");
    exit(EXIT_FAILURE);
  }
  printf("Server: Generated key: %d.\n", (int)key);

  if ((mqid = msgget(key, IPC_CREAT | S_IRWXU)) == -1) {
    perror("Server: msgget failed.");
    exit(EXIT_FAILURE);
  }
  printf("Server: Message queue [%d] created successfully.\n", mqid);

  signal(SIGINT, cleanup);
  signal(SIGTERM, cleanup);

  while (1) {
    if (msgrcv(mqid, &message, client_length, SERVER_MTYPE, 0) == -1) {
      perror("Server: msgrcv failed.");
      msgctl(mqid, IPC_RMID, NULL);
      exit(EXIT_FAILURE);
    }
    printf("Server: Received [%d] from [%ld]: %s\n", message.type, message.pid, message.body);

    switch (message.type) {
      case MJOIN:
        printf("Server: User %ld joined.\n", message.pid);
        if (nusers < MAX_USERS) {
          users[nusers].pid = message.pid;
          snprintf(users[nusers].username, MAX_USERNAME_SIZE, "%.*s", MAX_USERNAME_SIZE - 1, message.body);
          nusers++;
          printf("Server: User %ld added with username: %s\n", message.pid, users[nusers - 1].username);
          broadcast_to_all(MJOIN, users[nusers - 1].username, NULL);
          send_user_list(message.pid);
        } else {
          fprintf(stderr, "Server: User limit reached. Cannot add more users.\n");
        }
        break;
      case MLEAVE:
        userp = find_user_by_pid(message.pid);
        if (userp) {
          broadcast_to_all(MLEAVE, userp->username, NULL);
          *userp = users[nusers - 1];
          nusers--;
          printf("Server: User %ld left.\n", message.pid);
        } else {
          fprintf(stderr, "Server: User %ld not found.\n", message.pid);
        }
        break;
      case MMESSAGE:
        userp = find_user_by_pid(message.pid);
        if (userp) {
          printf("Server: Message from %s: %s\n", userp->username, message.body);
          broadcast_to_all(MMESSAGE, userp->username, message.body);
        } else {
          fprintf(stderr, "Server: User %ld not found for message.\n", message.pid);
        }
        break;
      case MLIST_USERS:
        break;
      default:
        fprintf(stderr, "Server: Unknown message type received.\n");
        continue;
    }
  }

  msgctl(mqid, IPC_RMID, NULL);
  exit(EXIT_SUCCESS);
}
