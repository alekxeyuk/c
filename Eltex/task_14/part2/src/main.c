#include <asm/termbits.h>
#include <ctype.h>
#include <locale.h>
#include <ncurses.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>

#include "client.h"
#include "common.h"
#include "flags.h"
#include "log.h"
#include "ui.h"

char* name;
char input_buffer[MAX_MSG_SIZE];
chatlog_t chatlog = {0};
char users[MAX_USERS][MAX_USERNAME_SIZE];
flags_t flags = 0;
int user_count = 0;
static int input_pos = 0;

volatile int running = 1;
pthread_mutex_t buffer_mut = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t log_mut = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t usr_mut = PTHREAD_MUTEX_INITIALIZER;
static pthread_t ui_thread;
static pthread_t con_thread;
static pthread_t pub_thread;

static void sig_win(int arg) {
  (void)arg;
  update_ui(URESIZE);
}

static void stop(int signum) {
  (void)signum;
  running = 0;
  printf("Stopping UI thread...\n");
  update_ui(USTOP);
  pthread_join(ui_thread, NULL);
  printf("UI thread stopped.\n");

  printf("Stopping publisher thread...\n");
  send_message(MSTOP);
  pthread_join(pub_thread, NULL);
  printf("Publisher thread stopped.\n");

  printf("Stopping consumer thread...\n");
  pthread_cancel(con_thread);
  pthread_join(con_thread, NULL);
  printf("Consumer thread stopped.\n");
  close_message_queue();
  printf("Message queue closed.\n");
  exit(EXIT_SUCCESS);
}

static void process_input(int ch) {
  switch (ch) {
    case '\t':
    case '\n':
      if (input_pos > 0) {
        send_message(MMESSAGE);
        pthread_mutex_lock(&buffer_mut);
        input_buffer[0] = '\0';
        input_pos = 0;
        pthread_mutex_unlock(&buffer_mut);
      }
      update_ui(UINPUT);
      break;
    case 127:
      if (input_pos > 0) {
        input_buffer[--input_pos] = '\0';
        update_ui(UINPUT);
      }
      break;
    default:
      if (isprint(ch) && input_pos < MAX_MSG_SIZE - 1) {
        input_buffer[input_pos++] = (char)ch;
        input_buffer[input_pos] = '\0';
        update_ui(UINPUT);
      }
  }
}

int main(int argc, char* argv[]) {
  setlocale(LC_ALL, "C.utf8");

  if (argc != 2) {
    fprintf(stderr, "Usage: %s <username>\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  name = argv[1];

  if (init_message_queue() == -1) {
    fprintf(stderr, "Failed to initialize message queue.\n");
    exit(EXIT_FAILURE);
  }

  init_chatlog(&chatlog);

  start_ui_thread(&ui_thread, NULL, NULL);
  start_publisher_thread(&pub_thread);
  start_consumer_thread(&con_thread, &log_mut, &usr_mut);
  signal(SIGWINCH, sig_win);
  signal(SIGINT, stop);
  signal(SIGTERM, stop);

  send_message(MJOIN);

  while (running) {
    int ch = getchar();
    process_input(ch);
  }

  stop(1);
  exit(EXIT_SUCCESS);
}