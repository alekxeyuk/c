#include <locale.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "common.h"

static int mqid = -1;
static pid_t pid = -1;
static size_t client_length = sizeof(clientmsg) - sizeof(long);
static size_t server_length = sizeof(servermsg) - sizeof(long);
static pthread_t consumer_t;

static void logout(int signum) {
  (void)signum;
  clientmsg m;
  m.mtype = SERVER_MTYPE;
  m.type = LEAVE;
  m.pid = (long)pid;
  if (mqid != -1 && msgsnd(mqid, &m, client_length, 0) == -1) {
    perror("Client: msgsnd failed.");
  } else {
    printf("Client: Sent LEAVE msg.\n");
  }
  if (pthread_cancel(consumer_t) != 0) {
    perror("Client: pthread_cancel failed");
  }
  exit(EXIT_SUCCESS);
}

static void* consumer(void* arg) {
  (void)arg;
  servermsg m;

  while (1) {
    if (msgrcv(mqid, &m, server_length, (long)pid, 0) == -1) {
      perror("Client: msgrcv failed.");
      pthread_exit(NULL);
    }
    if (m.type == JOIN || m.type == LEAVE || m.type == LIST_USERS) {
      printf("Client: Received user list update from server: %s\n", m.data.userlist.username);
    } else if (m.type == MESSAGE) {
      printf("Client: Received chat message from server: %s: %s\n", m.data.chat.username, m.data.chat.msgtext);
    } else {
      printf("Client: Unknown message type received from server.\n");
    }
  }
  pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
  setlocale(LC_ALL, "C.utf8");

  if (argc != 2) {
    fprintf(stderr, "Usage: %s <username>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  key_t key;
  clientmsg message;
  // servermsg server_message;
  char msg[MAX_MSG_SIZE] = "Hello from client!";
  char* name = argv[1];
  pid = getpid();
  // Generate a unique key for the message queue
  if ((key = ftok(".", 'A')) == -1) {
    perror("Client: ftok failed.");
    exit(EXIT_FAILURE);
  }
  printf("Client: Generated key: %d.\n", (int)key);

  // Open the message queue
  if ((mqid = msgget(key, S_IRWXU)) == -1) {
    perror("Client: msgget failed.");
    exit(EXIT_FAILURE);
  }
  printf("Client: Message queue [%d] opened successfully.\n", mqid);

  signal(SIGINT, logout);
  signal(SIGTERM, logout);

  // Send a message back to the server
  message.mtype = SERVER_MTYPE;
  message.type = JOIN;
  message.pid = (long)pid;
  snprintf(message.body, MAX_MSG_SIZE, "%.*s", MAX_USERNAME_SIZE, name);
  if (msgsnd(mqid, &message, client_length, 0) == -1) {
    perror("Client: msgsnd failed.");
    msgctl(mqid, IPC_RMID, NULL);
    exit(EXIT_FAILURE);
  }
  printf("Client: Sent JOIN request.\n");

  if (pthread_create(&consumer_t, NULL, consumer, NULL) != 0) {
    perror("pthread_create failed");
    exit(EXIT_FAILURE);
  }

  while (1) {
    if (fgets(msg, MAX_MSG_SIZE, stdin) == NULL) {
      perror("Client: fgets failed.");
      continue;
    }
    msg[strcspn(msg, "\n")] = 0;

    // Send a message to the server
    message.type = MESSAGE;
    snprintf(message.body, MAX_MSG_SIZE, "%.*s", MAX_MSG_SIZE - 1, msg);
    if (msgsnd(mqid, &message, client_length, 0) == -1) {
      perror("Client: msgsnd failed.");
      msgctl(mqid, IPC_RMID, NULL);
      exit(EXIT_FAILURE);
    }
    printf("Client: Sent message to server: %s\n", message.body);
  }

  msgctl(mqid, IPC_RMID, NULL);
  exit(EXIT_SUCCESS);
}