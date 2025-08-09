#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "common.h"

int main(void) {
  setlocale(LC_ALL, "C.utf8");

  key_t key;
  int mqid;
  msgbuf message;
  const char* msg = "Hi!";
  size_t length = sizeof(msgbuf) - sizeof(long);

  // Generate a unique key for the message queue
  if ((key = ftok(".", 'A')) == -1) {
    perror("Server: ftok failed.");
    exit(EXIT_FAILURE);
  }
  printf("Server: Generated key: %d.\n", (int)key);

  // Create the message queue
  if ((mqid = msgget(key, IPC_CREAT | S_IRWXU)) == -1) {
    perror("Server: msgget failed.");
    exit(EXIT_FAILURE);
  }
  printf("Server: Message queue [%d] created successfully.\n", mqid);

  // Send a message to the client
  message.mtype = CLIENT_MTYPE;
  snprintf(message.text, MAX_MSG_SIZE, "%s", msg);
  if (msgsnd(mqid, &message, length, 0) == -1) {
    perror("Server: msgsnd failed.");
    msgctl(mqid, IPC_RMID, NULL);
    exit(EXIT_FAILURE);
  }
  printf("Server: Sent message to client: %s\n", msg);

  // Wait for a message from the client
  if (msgrcv(mqid, &message, length, SERVER_MTYPE, 0) == -1) {
    perror("Server: msgrcv failed.");
    msgctl(mqid, IPC_RMID, NULL);
    exit(EXIT_FAILURE);
  }
  printf("Server: Received message from client: %s\n", message.text);

  msgctl(mqid, IPC_RMID, NULL);
  exit(EXIT_SUCCESS);
}
