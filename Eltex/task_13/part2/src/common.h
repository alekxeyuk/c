#ifndef COMMON_H_
#define COMMON_H_

#define MAX_MESSAGES 100
#define MAX_USERNAME_SIZE 16
#define MAX_MSG_SIZE 64
#define MAX_USERS 16

#define SERVER_MTYPE 1

typedef enum {
  MNOOP = -1,
  MJOIN = 1,
  MLEAVE,
  MMESSAGE,
  MLIST_USERS,
  MSTOP,
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
  msgtype type;
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
} user_t;

#endif  // COMMON_H_