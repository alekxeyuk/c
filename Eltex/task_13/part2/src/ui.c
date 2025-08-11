#include "ui.h"

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

static WINDOW *log_win, *users_win, *input_win;
extern char input_buffer[MAX_MSG_SIZE];
extern int running;
static pthread_mutex_t ui_mut = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t ui_update = PTHREAD_COND_INITIALIZER;
static update_type_t update_type = NOOP;

static void setup_windows() {
  int max_y, max_x;
  getmaxyx(stdscr, max_y, max_x);

  // Calculate dimensions
  int input_height = 3;
  int users_width = 16;
  int log_height = max_y - input_height;
  int log_width = max_x - users_width;

  // Create windows
  log_win = newwin(log_height, log_width, 0, 0);
  users_win = newwin(log_height, users_width, 0, log_width);
  input_win = newwin(input_height, max_x, log_height, 0);

  // Enable scrolling for message window
  scrollok(log_win, TRUE);
}

// Draw message list
static void draw_messages() {
  werase(log_win);
  box(log_win, 0, 0);

  /*
  int max_y, max_x;
  getmaxyx(log_win, max_y, max_x);

  // Show last visible messages
  int start = msg_count - (max_y - 2);
  if (start < 0) start = 0;

  for (int i = start; i < msg_count; i++) {
      mvwprintw(log_win, i - start + 1, 1, "%s", messages[i]);
  }
  */
  wnoutrefresh(log_win);
}

// Draw user list
static void draw_users() {
  werase(users_win);
  box(users_win, 0, 0);
  mvwprintw(users_win, 0, 1, " Users");
  /*
  for (int i = 0; i < user_count; i++) {
      mvwprintw(users_win, i + 1, 1, "%s", users[i]);
  }
  */
  wnoutrefresh(users_win);
}

// Draw input area
static void draw_input() {
  werase(input_win);
  box(input_win, 0, 0);
  mvwprintw(input_win, 1, 1, "> %s", input_buffer);
  wnoutrefresh(input_win);
}

// Handle terminal resize
static void handle_resize() {
  // pthread_mutex_lock(&ui_resize_mut);
  int max_y, max_x;
  getmaxyx(stdscr, max_y, max_x);

  // Calculate dimensions
  int input_height = 3;
  int users_width = 16;
  int log_height = max_y - input_height;
  int log_width = max_x - users_width;

  werase(stdscr);

  wresize(log_win, log_height, log_width);
  wresize(users_win, log_height, users_width);
  wresize(input_win, input_height, max_x);
  mvwin(users_win, 0, log_width);
  mvwin(input_win, log_height, 0);
  wnoutrefresh(stdscr);

  draw_messages();
  draw_users();
  draw_input();
  //  pthread_mutex_unlock(&ui_resize_mut);
}

// UI thread main function
static void *ui_thread_func(void *arg) {
  if (!initscr()) {
    perror("initscr failed!");
    return NULL;
  }
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  curs_set(0);

  wnoutrefresh(stdscr);

  setup_windows();
  draw_messages();
  draw_users();
  draw_input();
  doupdate();

  struct winsize ws;
  while (running) {
    pthread_mutex_lock(&ui_mut);
    while (update_type == NOOP) {
      pthread_cond_wait(&ui_update, &ui_mut);
    }
    switch (update_type) {
      case RESIZE:
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
        resizeterm(ws.ws_row, ws.ws_col);
        handle_resize();
        break;
      case LOG:
        break;
      case USER:
        break;
      case INPUT:
        draw_input();
        break;
      case STOP:
        break;
    }
    update_type = NOOP;
    doupdate();
    pthread_mutex_unlock(&ui_mut);
  }

  // Cleanup
  delwin(log_win);
  delwin(users_win);
  delwin(input_win);
  endwin();

  pthread_mutex_destroy(&ui_mut);
  pthread_cond_destroy(&ui_update);

  return NULL;
}

int start_ui_thread(pthread_t *ui_thread) { return pthread_create(ui_thread, NULL, ui_thread_func, NULL); }

/*
 * Update the UI with a specific type of update.
 * This function locks the mutex, sets the update type, signals the condition variable,
 * and then unlocks the mutex.
 */
void update_ui(update_type_t type) {
  pthread_mutex_lock(&ui_mut);
  update_type = type;
  pthread_cond_signal(&ui_update);
  pthread_mutex_unlock(&ui_mut);
}
