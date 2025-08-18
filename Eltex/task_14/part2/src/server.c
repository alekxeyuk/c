#include <locale.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "common.h"
#include "log.h"
#include "msgq.h"

static shmq_handle_t mqid;
static user_t users[MAX_USERS] = {0};
static int nusers = 0;
static chatlog_t chatlog = {0};
static size_t client_length = sizeof(clientmsg_t) - sizeof(long);
static size_t server_length = sizeof(servermsg_t) - sizeof(long);

static void cleanup(int signum) {
  (void)signum;
  if (msgctl(&mqid, IPC_RMID) == -1) {
    perror("Server: msgctl failed.");
  } else {
    printf("Server: Message queue removed successfully.\n");
  }
  exit(EXIT_SUCCESS);
}

static void broadcast_to_all(msgtype type, const char *username, const char *msg) {
  servermsg_t m;
  m.type = type;

  if (type == MJOIN || type == MLEAVE || type == MLIST_USERS) {
    snprintf(m.data.userlist.username, MAX_USERNAME_SIZE, "%.*s", MAX_USERNAME_SIZE - 1, username);
  } else if (type == MMESSAGE) {
    snprintf(m.data.chat.username, MAX_USERNAME_SIZE, "%.*s", MAX_USERNAME_SIZE - 1, username);
    snprintf(m.data.chat.msgtext, MAX_MSG_SIZE, "%.*s", MAX_MSG_SIZE - 1, msg);
  }

  for (int i = 0; i < nusers; i++) {
    m.mtype = users[i].pid;
    if (msgsnd(&mqid, &m, server_length, m.mtype) == -1) {
      perror("Server: msgsnd failed.");
    }
  }
}

static void send_user_list(long pid) {
  servermsg_t m;
  m.type = MJOIN;
  m.mtype = pid;

  for (int i = 0; i < nusers; i++) {
    if (users[i].pid != pid) {
      snprintf(m.data.userlist.username, MAX_USERNAME_SIZE, "%.*s", MAX_USERNAME_SIZE - 1, users[i].username);
      if (msgsnd(&mqid, &m, server_length, pid) == -1) {
        perror("Server: msgsnd failed.");
      }
    }
  }
}

static void send_chat_log(long pid) {
  servermsg_t m;
  m.type = MMESSAGE;
  m.mtype = pid;

  for (int i = 0; i < chatlog.count; i++) {
    const chatmsg *msg = get_message(&chatlog, i);
    snprintf(m.data.chat.username, MAX_USERNAME_SIZE, "%.*s", MAX_USERNAME_SIZE - 1, msg->username);
    snprintf(m.data.chat.msgtext, MAX_MSG_SIZE, "%.*s", MAX_MSG_SIZE - 1, msg->msgtext);
    if (msgsnd(&mqid, &m, server_length, pid) == -1) {
      perror("Server: msgsnd failed.");
    }
  }
}

static user_t *find_user_by_pid(long pid) {
  for (int i = 0; i < nusers; i++) {
    if (users[i].pid == pid) {
      return &users[i];
    }
  }
  return NULL;
}

int main(void) {
  setlocale(LC_ALL, "C.utf8");

  clientmsg_t message = {0};
  user_t *userp = NULL;

  if ((shmq_create(SERVER_SHM_NAME, &mqid, SERVER_MTYPE)) == -1) {
    perror("Server: msgget failed.");
    exit(EXIT_FAILURE);
  }
  printf("Server: Message queue [%d] created successfully.\n", mqid.fd);

  signal(SIGINT, cleanup);
  signal(SIGTERM, cleanup);

  init_chatlog(&chatlog);

  while (1) {
    if (msgrcv(&mqid, &message, client_length, SERVER_MTYPE) == -1) {
      perror("Server: msgrcv failed.");
      msgctl(&mqid, IPC_RMID);
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
          send_chat_log(message.pid);
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
          add_message(&chatlog, userp->username, message.body, MMESSAGE);
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

  msgctl(&mqid, IPC_RMID);
  exit(EXIT_SUCCESS);
}
