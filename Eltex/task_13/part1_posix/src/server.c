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
  struct mq_attr attr;
  char buffer[MAX_MSG_SIZE];
  const char *msg = "Hi!";
  ssize_t msg_size;

  attr.mq_flags = 0;
  attr.mq_maxmsg = MAX_MSG_COUNT;
  attr.mq_msgsize = MAX_MSG_SIZE;
  attr.mq_curmsgs = 0;

  // Create the server-to-client message queue
  if ((s2c_mq = mq_open(S2C_MQ_NAME, O_WRONLY | O_CREAT, S_IRWXU, &attr)) ==
      (mqd_t)-1) {
    perror("Server: mq_open S2C_MQ failed");
    exit(EXIT_FAILURE);
  }
  printf("Server: Message queue %s created successfully [%d].\n", S2C_MQ_NAME,
         (int)s2c_mq);

  // Create the client-to-server message queue
  if ((c2s_mq = mq_open(C2S_MQ_NAME, O_RDONLY | O_CREAT, S_IRWXU, &attr)) ==
      (mqd_t)-1) {
    perror("Server: mq_open C2S_MQ failed");
    mq_close(s2c_mq);
    mq_unlink(S2C_MQ_NAME);
    exit(EXIT_FAILURE);
  }
  printf("Server: Message queue %s created successfully [%d].\n", C2S_MQ_NAME,
         (int)c2s_mq);

  // Send a message to the client
  if (mq_send(s2c_mq, msg, MAX_MSG_SIZE, 0) == -1) {
    perror("Server: mq_send failed");
    goto cleanup;
  }
  printf("Server: Sent message to client: %s\n", msg);

  // Wait for a message from the client
  if ((msg_size = mq_receive(c2s_mq, buffer, MAX_MSG_SIZE, NULL)) == -1) {
    perror("Server: mq_receive failed");
    goto cleanup;
  }
  buffer[msg_size] = '\0';
  printf("Server: Received message from client: %s\n", buffer);

cleanup:
  mq_close(s2c_mq);
  mq_close(c2s_mq);
  mq_unlink(S2C_MQ_NAME);
  mq_unlink(C2S_MQ_NAME);
  exit(EXIT_SUCCESS);
}
