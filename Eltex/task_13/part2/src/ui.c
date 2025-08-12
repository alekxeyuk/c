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
#include "log.h"

static WINDOW *log_win, *users_win, *input_win;
extern char input_buffer[MAX_MSG_SIZE];
extern chatlog_t chatlog;
extern char users[MAX_USERS][MAX_USERNAME_SIZE];
// extern int msg_count;
extern int user_count;
extern int running;
static pthread_mutex_t ui_mut = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t ui_update = PTHREAD_COND_INITIALIZER;
static pthread_cond_t op_complete = PTHREAD_COND_INITIALIZER;
static update_type_t update_type = UNOOP;

static pthread_mutex_t *log_mut = NULL;
static pthread_mutex_t *users_mut = NULL;

static void setup_windows(void) {
  int max_y, max_x;
  getmaxyx(stdscr, max_y, max_x);

  int input_height = 3;
  int users_width = 16;
  int log_height = max_y - input_height;
  int log_width = max_x - users_width;

  log_win = newwin(log_height, log_width, 0, 0);
  users_win = newwin(log_height, users_width, 0, log_width);
  input_win = newwin(input_height, max_x, log_height, 0);

  scrollok(log_win, TRUE);
}

// Draw message list
static void draw_messages(void) {
  werase(log_win);
  box(log_win, 0, 0);

  int max_y, max_x;
  getmaxyx(log_win, max_y, max_x);

  // Show last visible messages
  int start = chatlog.count - (max_y - 2);
  if (start < 0) start = 0;

  for (int i = start; i < chatlog.count; i++) {
    const chatmsg *m = get_message(&chatlog, i);
    if (m->type != MMESSAGE) {
      const char *action = (m->type == MJOIN) ? "joined" : (m->type == MLEAVE) ? "left" : "user list updated";
      mvwprintw(log_win, i - start + 1, 1, "%s has %s", m->username, action);
    } else {
      mvwprintw(log_win, i - start + 1, 1, "%s: %.*s", m->username, max_x - 3 - (int)strlen(m->username), m->msgtext);
    }
  }

  wnoutrefresh(log_win);
}

// Draw user list
static void draw_users(void) {
  werase(users_win);
  box(users_win, 0, 0);
  mvwprintw(users_win, 0, 1, " Users ");

  for (int i = 0; i < user_count; i++) {
    mvwprintw(users_win, i + 1, 1, "%s", users[i]);
  }

  wnoutrefresh(users_win);
}

// Draw input area
static void draw_input(void) {
  werase(input_win);
  box(input_win, 0, 0);
  mvwprintw(input_win, 0, 1, " Ctrl+Enter or TAB to send ");
  mvwprintw(input_win, 1, 1, "> %s", input_buffer);
  wnoutrefresh(input_win);
}

// Handle terminal resize
static void handle_resize(void) {
  int max_y, max_x;
  getmaxyx(stdscr, max_y, max_x);

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
}

// UI thread main function
static void *ui_thread_func(void *arg) {
  (void)arg;
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
    while (update_type == UNOOP) {
      pthread_cond_wait(&ui_update, &ui_mut);
    }
    switch (update_type) {
      case URESIZE:
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
        resizeterm(ws.ws_row, ws.ws_col);
        handle_resize();
        break;
      case ULOG:
        draw_messages();
        break;
      case UUSER:
        draw_users();
        break;
      case UINPUT:
        draw_input();
        break;
      case USTOP:
        break;
      case UNOOP:
        break;
    }
    update_type = UNOOP;
    doupdate();
    pthread_cond_signal(&op_complete);
    pthread_mutex_unlock(&ui_mut);
  }

  delwin(log_win);
  delwin(users_win);
  delwin(input_win);
  endwin();

  pthread_mutex_destroy(&ui_mut);
  pthread_cond_destroy(&ui_update);
  pthread_cond_destroy(&op_complete);

  return NULL;
}

int start_ui_thread(pthread_t *ui_thread, pthread_mutex_t *l_mut, pthread_mutex_t *u_mut) {
  log_mut = l_mut;
  users_mut = u_mut;
  return pthread_create(ui_thread, NULL, ui_thread_func, NULL);
}

void update_ui(update_type_t type, int block) {
  pthread_mutex_lock(&ui_mut);
  update_type = type;
  pthread_cond_signal(&ui_update);
  if (block) {
    pthread_cond_wait(&op_complete, &ui_mut);
  } else {
    pthread_cond_signal(&op_complete);
  }
  pthread_mutex_unlock(&ui_mut);
}
