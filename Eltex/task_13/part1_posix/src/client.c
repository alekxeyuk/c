#include <fcntl.h>
#include <locale.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "common.h"

int main(void) {
  setlocale(LC_ALL, "C.utf8");

  mqd_t s2c_mq, c2s_mq;
  char buffer[MAX_MSG_SIZE];
  const char* msg = "Hello!";
  ssize_t msg_size;

  // Open the server-to-client message queue
  if ((s2c_mq = mq_open(S2C_MQ_NAME, O_RDONLY)) == (mqd_t)-1) {
    perror("Client: mq_open S2C_MQ failed");
    exit(EXIT_FAILURE);
  }
  printf("Client: Connected to message queue %s [%d].\n", S2C_MQ_NAME,
         (int)s2c_mq);

  // Open the client-to-server message queue
  if ((c2s_mq = mq_open(C2S_MQ_NAME, O_WRONLY)) == (mqd_t)-1) {
    perror("Client: mq_open C2S_MQ failed");
    mq_close(s2c_mq);
    exit(EXIT_FAILURE);
  }
  printf("Client: Connected to message queue %s [%d].\n", C2S_MQ_NAME,
         (int)c2s_mq);

  // Wait for a message from the server
  if ((msg_size = mq_receive(s2c_mq, buffer, MAX_MSG_SIZE, NULL)) == -1) {
    perror("Client: mq_receive failed");
    goto cleanup;
  }
  buffer[msg_size] = '\0';
  printf("Client: Received message from server: %s\n", buffer);

  // Send a message back to the server
  if (mq_send(c2s_mq, msg, MAX_MSG_SIZE, 0) == -1) {
    perror("Client: mq_send failed");
    goto cleanup;
  }
  printf("Client: Sent message to server: %s\n", msg);

cleanup:
  mq_close(s2c_mq);
  mq_close(c2s_mq);
  exit(EXIT_SUCCESS);
}