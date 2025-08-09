#ifndef COMMON_H_
#define COMMON_H_

#define MAX_MSG_SIZE 64

#define SERVER_MTYPE 1
#define CLIENT_MTYPE 2

typedef struct {
  long mtype;
  char text[MAX_MSG_SIZE];
} msgbuf;

#endif  // COMMON_H_