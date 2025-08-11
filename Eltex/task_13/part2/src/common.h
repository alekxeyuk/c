#ifndef COMMON_H_
#define COMMON_H_

#define MAX_USERNAME_SIZE 16
#define MAX_MSG_SIZE 64
#define MAX_USERS 16

#define SERVER_MTYPE 1

typedef enum {
  JOIN = 1,
  LEAVE,
  MESSAGE,
  LIST_USERS,
} msgtype;

typedef struct {
  long mtype;
  msgtype type;
  long pid;
  char body[MAX_MSG_SIZE];
} clientmsg;

typedef struct {
  char username[MAX_USERNAME_SIZE];
  char msgtext[MAX_MSG_SIZE];
} chatmsg;

typedef struct {
  char username[MAX_USERNAME_SIZE];
} userlistmsg;

typedef struct {
  long mtype;
  msgtype type;
  union {
    chatmsg chat;
    userlistmsg userlist;
  } data;
} servermsg;

typedef struct {
  long pid;
  char username[MAX_USERNAME_SIZE];
} user;

#endif  // COMMON_H_