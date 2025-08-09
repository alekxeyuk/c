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
  const char* msg = "Hello!";
  size_t length = sizeof(msgbuf) - sizeof(long);

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

  // Wait for a message from the server
  if (msgrcv(mqid, &message, length, CLIENT_MTYPE, 0) == -1) {
    perror("Client: msgrcv failed.");
    msgctl(mqid, IPC_RMID, NULL);
    exit(EXIT_FAILURE);
  }
  printf("Client: Received message from server: %s\n", message.text);

  // Send a message back to the server
  message.mtype = SERVER_MTYPE;
  snprintf(message.text, MAX_MSG_SIZE, "%s", msg);
  if (msgsnd(mqid, &message, length, 0) == -1) {
    perror("Client: msgsnd failed.");
    msgctl(mqid, IPC_RMID, NULL);
    exit(EXIT_FAILURE);
  }
  printf("Client: Sent message to server: %s\n", msg);

  msgctl(mqid, IPC_RMID, NULL);
  exit(EXIT_SUCCESS);
}