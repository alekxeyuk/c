#include <curses.h>
#include <dirent.h>
#include <locale.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
// #include <fcntl.h>

// #define STDIN_FILENO fileno(stdin)

// void update_size(__attribute__((unused)) int signo) {
//   struct winsize size;
//   ioctl(STDIN_FILENO, TIOCGWINSZ, &size);
//   resize_term(size.ws_row, size.ws_col);
// }

static void cleanup(void) { endwin(); }
static bool prefix(const char* pre, const char* str) {
  return strncmp(pre, str, strlen(pre)) == 0;
}

typedef struct FENTRY {
  char name[256];
  intmax_t size;
  char mtime_str[13];
  bool is_dir;
} FENTRY;

typedef struct PANEL {
  WINDOW* win;
  char cwd[PATH_MAX];
  FENTRY** files;
  int topline, selected, nfiles;
  bool active;
} PANEL;

enum COLOR_SCHEME { CS_DIR = 1, CS_FILE, CS_STATUS_CWD, CS_SELECTED };

int main(void) {
  setlocale(LC_ALL, "C.utf8");

  WINDOW* left;
  WINDOW* right;

  int selected = 0;
  // WINDOW* subwnd;

  if (!initscr()) {
    perror("initscr failed!");
    return EXIT_FAILURE;
  }
  if (atexit(cleanup) != 0) {
    endwin();
    return EXIT_FAILURE;
  }
  // signal(SIGWINCH, update_size);
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  start_color();
  curs_set(0);
  // box(stdscr, ACS_VLINE, ACS_HLINE);

  init_pair(CS_DIR, COLOR_YELLOW, COLOR_BLACK);        // directory
  init_pair(CS_FILE, COLOR_GREEN, COLOR_BLACK);        // file
  init_pair(CS_STATUS_CWD, COLOR_BLACK, COLOR_WHITE);  // status bar cwd
  init_pair(CS_SELECTED, COLOR_BLACK, COLOR_CYAN);     // selection
  refresh();

  int left_w = COLS / 2;
  int right_w = COLS - left_w;
  left = newwin(LINES, left_w, 0, 0);
  right = newwin(LINES, right_w, 0, left_w);
  box(left, ACS_VLINE, ACS_HLINE);
  box(right, ACS_VLINE, ACS_HLINE);

  struct stat sb;
  struct dirent** namelist;
  char time_str[13];
  int list_size = scandir(".", &namelist, NULL, alphasort);
  if (list_size == -1) {
    perror("scandir");
    exit(EXIT_FAILURE);
  }
  for (int i = 0; i < list_size; i++) {
    stat(namelist[i]->d_name, &sb);
    strftime(time_str, sizeof(time_str), "%b %e %R",
             localtime(&sb.st_mtim.tv_sec));

    int color_scheme = CS_FILE;
    if (S_ISDIR(sb.st_mode)) {
      color_scheme = CS_DIR;
    }
    if (i == selected) {
      color_scheme = CS_SELECTED;
    }
    wattron(left, COLOR_PAIR(color_scheme));
    mvwprintw(left, i + 1, 1, "%s", namelist[i]->d_name);
    mvwprintw(left, i + 1, left_w - 13 - 8, "%7jd", (intmax_t)sb.st_size);
    mvwprintw(left, i + 1, left_w - 13, "%s", time_str);
    wattroff(left, COLOR_PAIR(color_scheme));
    free(namelist[i]);
  }
  mvwvline(left, 1, left_w - 14 - 8, ACS_VLINE, LINES - 2);
  mvwvline(left, 1, left_w - 14, ACS_VLINE, LINES - 2);
  free(namelist);

  // TOP BAR CURRENT WORKING DIRECTORY
  uid_t user_id = getuid();
  struct passwd* user_pwd = getpwuid(user_id);
  char cwd[256];
  getcwd(cwd, sizeof(cwd));  // TODO: can fail

  wattron(left, COLOR_PAIR(CS_STATUS_CWD));
  if (prefix(user_pwd->pw_dir, cwd)) {
    mvwprintw(left, 0, 3, " ~%s ", cwd + strlen(user_pwd->pw_dir));
  } else {
    mvwprintw(left, 0, 3, " %s ", cwd);
  }
  wattroff(left, COLOR_PAIR(CS_STATUS_CWD));
  // TOP BAR CURRENT WORKING DIRECTORY

  refresh();
  wrefresh(left);
  wrefresh(right);

  for (;;) {
    int ch = wgetch(stdscr);
    if (ch == KEY_RESIZE) {
      left_w = COLS / 2;
      right_w = COLS - left_w;
      wclear(stdscr);
      wclear(left);
      wclear(right);
      delwin(right);

      wresize(left, LINES, left_w);
      right = newwin(LINES, right_w, 0, left_w);

      box(left, ACS_VLINE, ACS_HLINE);
      box(right, ACS_VLINE, ACS_HLINE);

      mvwprintw(left, 1, 1, "New size: %d rows × %d cols", LINES, COLS);
      mvwprintw(left, 3, 1, "Press 'q' to quit.");

      wrefresh(stdscr);
      wrefresh(left);
      wrefresh(right);
    } else if (ch == 'q' || ch == 'Q') {
      break;
    } else if (ch == KEY_UP) {
      selected++;
    } else if (ch == KEY_DOWN) {
      selected = selected == 0 ? 0 : selected-1;
    }
  }

  return EXIT_SUCCESS;
}
