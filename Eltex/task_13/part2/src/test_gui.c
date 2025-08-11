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

#include "common.h"
#include "ui.h"

char input_buffer[MAX_MSG_SIZE];
static int input_pos = 0;

// Threads
volatile int running = 1;
// pthread_mutex_t ui_mut = PTHREAD_MUTEX_INITIALIZER;
// pthread_cond_t ui_update = PTHREAD_COND_INITIALIZER;
// update_type_t update_type = NOOP;
// static pthread_mutex_t ui_resize_mut = PTHREAD_MUTEX_INITIALIZER;
static pthread_t ui_thread;
static pthread_t sig_thread;

static void sig_win(int arg) { update_ui(RESIZE); }

static void stop(int signum) {
  running = 0;
  printf("Stopping UI thread...\n");
  update_ui(STOP);
  pthread_join(ui_thread, NULL);
  printf("UI thread stopped.\n");
  exit(EXIT_SUCCESS);
}

void process_input(int ch) {
  switch (ch) {
    case '\n':  // Enter key
                /*
                  if (input_pos > 0) {
                      // Add message to list
                      if (msg_count < MAX_MESSAGES) {
                          snprintf(messages[msg_count], MAX_MSG_LEN, "You: %s", input_buffer);
                          msg_count++;
                      }
          
                      // Clear input
                      input_buffer[0] = '\0';
                      input_pos = 0;
          
                      draw_messages();
                      draw_input();
                  }
                  */
      update_ui(INPUT);
      break;
    case KEY_BACKSPACE:
    case 127:  // Delete key
      if (input_pos > 0) {
        input_buffer[--input_pos] = '\0';
        update_ui(INPUT);
      }
      break;
    default:
      // Add printable characters
      if (isprint(ch) && input_pos < MAX_MSG_SIZE - 1) {
        input_buffer[input_pos++] = ch;
        input_buffer[input_pos] = '\0';
        update_ui(INPUT);
      }
  }
}

int main(void) {
  setlocale(LC_ALL, "C.utf8");

  start_ui_thread(&ui_thread);
  signal(SIGWINCH, sig_win);
  signal(SIGINT, stop);
  signal(SIGTERM, stop);

  while (running) {
    int ch = getchar();
    process_input(ch);
  }

  stop(1);
  exit(EXIT_SUCCESS);
}