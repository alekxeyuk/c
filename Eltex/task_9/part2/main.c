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

#define MTIMMX 12
#define SMX 7

typedef struct FENTRY {
  char name[256];
  intmax_t size;
  char mtime_str[13];
  bool is_dir;
} FENTRY;

typedef struct PANEL {
  WINDOW* win;
  char cwd[PATH_MAX];
  FENTRY* files;
  size_t topline, selected, nfiles;
  bool active;
  int cols;
} PANEL;

static void cleanup(void) { endwin(); }
static bool prefix(const char* pre, const char* str) {
  return strncmp(pre, str, strlen(pre)) == 0;
}
static int compare_entries(const void* a, const void* b) {
  const FENTRY* entryA = (const FENTRY*)a;
  const FENTRY* entryB = (const FENTRY*)b;

  if (entryA->is_dir && !entryB->is_dir) {
    return -1;
  }
  if (!entryA->is_dir && entryB->is_dir) {
    return 1;
  }
  return 0;
}
void init_panel(PANEL* p, bool active, int cols, int y, int x) {
  getcwd(p->cwd, sizeof(p->cwd));
  p->topline = 0;
  p->selected = 0;
  p->nfiles = 0;
  p->active = active;
  p->win = newwin(LINES, cols, y, x);
  p->cols = cols - 1;
}

enum COLOR_SCHEME {
  CS_DIR = 1,
  CS_FILE,
  CS_SELECTED,
  CS_STATUS_ACT,
  CS_STATUS_NACT
};

int main(void) {
  setlocale(LC_ALL, "C.utf8");

  // WINDOW* left;
  // WINDOW* right;
  // WINDOW* subwnd;
  // signal(SIGWINCH, update_size);

  if (!initscr()) {
    perror("initscr failed!");
    return EXIT_FAILURE;
  }
  if (atexit(cleanup) != 0) {
    endwin();
    return EXIT_FAILURE;
  }
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  start_color();
  curs_set(0);

  init_pair(CS_DIR, COLOR_YELLOW, COLOR_BLACK);         // directory
  init_pair(CS_FILE, COLOR_GREEN, COLOR_BLACK);         // file
  init_pair(CS_SELECTED, COLOR_BLACK, COLOR_CYAN);      // selection
  init_pair(CS_STATUS_ACT, COLOR_BLACK, COLOR_WHITE);   // status bar cwd
  init_pair(CS_STATUS_NACT, COLOR_WHITE, COLOR_BLACK);  // status bar cwd
  wnoutrefresh(stdscr);

  uid_t user_id = getuid();
  struct passwd* user_pwd = getpwuid(user_id);

  int selected = 0;
  int left_w = COLS / 2;
  int right_w = COLS - left_w;

  PANEL panels[2];
  init_panel(&panels[0], true, left_w, 0, 0);
  init_panel(&panels[1], false, right_w, 0, left_w);

  PANEL* a_pn;

  for (int p_i = 0; p_i < 2; p_i++) {
    a_pn = &panels[p_i];

    box(a_pn->win, ACS_VLINE, ACS_HLINE);

    struct stat sb;
    struct dirent** namelist;
    int list_size = scandir(".", &namelist, NULL, alphasort);
    if (list_size == -1) {
      perror("scandir");
      exit(EXIT_FAILURE);
    }

    a_pn->nfiles = (size_t)list_size;
    a_pn->files = malloc(sizeof(FENTRY[list_size]));

    for (int i = 0; i < list_size; i++) {
      stat(namelist[i]->d_name, &sb);

      strncpy(a_pn->files[i].name, namelist[i]->d_name,
              sizeof(a_pn->files[i].name) - 1);
      a_pn->files[i].name[sizeof(a_pn->files[i].name) - 1] = '\0';  // Save name
      strftime(a_pn->files[i].mtime_str, sizeof(a_pn->files[i].mtime_str),
               "%b %e %R", localtime(&sb.st_mtim.tv_sec));  // Save mtime
      a_pn->files[i].is_dir = S_ISDIR(sb.st_mode);          // Save file type
      a_pn->files[i].size = (intmax_t)sb.st_size;           // Save file size

      free(namelist[i]);
    }
    free(namelist);
    qsort(a_pn->files, a_pn->nfiles, sizeof(FENTRY), compare_entries);

    int color_scheme = CS_FILE;
    for (size_t i = 0; i < a_pn->nfiles; i++) {
      if (a_pn->files[i].is_dir) {
        color_scheme = CS_DIR;
      } else {
        color_scheme = CS_FILE;
      }
      if (i == a_pn->selected && a_pn->active) {
        color_scheme = CS_SELECTED;
      }
      wattron(a_pn->win, COLOR_PAIR(color_scheme));
      mvwprintw(a_pn->win, (int)i + 1, 1, "%-*s",
                a_pn->cols - (MTIMMX + SMX + 3), a_pn->files[i].name);
      mvwprintw(a_pn->win, (int)i + 1, a_pn->cols - MTIMMX - SMX - 1, "%7jd",
                a_pn->files[i].size);
      mvwprintw(a_pn->win, (int)i + 1, a_pn->cols - MTIMMX, "%s",
                a_pn->files[i].mtime_str);
      wattroff(a_pn->win, COLOR_PAIR(color_scheme));
    }
    mvwvline(a_pn->win, 1, a_pn->cols - MTIMMX - SMX - 1 - 1, ACS_VLINE,
             LINES - 2);
    mvwvline(a_pn->win, 1, a_pn->cols - MTIMMX - 1, ACS_VLINE, LINES - 2);

    // TOP BAR CURRENT WORKING DIRECTORY
    // getcwd(a_pn->cwd, sizeof(a_pn->cwd));  // TODO: can fail

    color_scheme = a_pn->active ? CS_STATUS_ACT : CS_STATUS_NACT;
    wattron(a_pn->win, COLOR_PAIR(color_scheme));
    if (prefix(user_pwd->pw_dir, a_pn->cwd)) {
      mvwprintw(a_pn->win, 0, 3, " ~%s ", a_pn->cwd + strlen(user_pwd->pw_dir));
    } else {
      mvwprintw(a_pn->win, 0, 3, " %s ", a_pn->cwd);
    }
    wattroff(a_pn->win, COLOR_PAIR(color_scheme));
    // TOP BAR CURRENT WORKING DIRECTORY

    // mvwaddstr(left, 2, 2, "Hello world");

    wnoutrefresh(a_pn->win);
  }

  // wnoutrefresh(panels[1].win);
  doupdate();

  for (;;) {
    int ch = wgetch(stdscr);
    if (ch == KEY_RESIZE) {
      left_w = COLS / 2;
      right_w = COLS - left_w;
      werase(stdscr);
      werase(panels[0].win);
      werase(panels[1].win);

      wresize(panels[0].win, LINES, left_w);
      wresize(panels[1].win, LINES, right_w);
      mvwin(panels[1].win, 0, left_w);

      box(panels[0].win, ACS_VLINE, ACS_HLINE);
      box(panels[1].win, ACS_VLINE, ACS_HLINE);

      mvwprintw(panels[0].win, 1, 1, "New size: %d rows × %d cols", LINES,
                COLS);
      mvwprintw(panels[0].win, 3, 1, "Press 'q' to quit.");
    } else if (ch == 'q' || ch == 'Q') {
      break;
    } else if (ch == KEY_UP) {
      // selected++;
    } else if (ch == KEY_DOWN) {
      // selected = selected == 0 ? 0 : selected-1;
    } else if (ch == KEY_STAB) {
      selected = selected ? 0 : 1;
    }
    wnoutrefresh(stdscr);
    wnoutrefresh(panels[0].win);
    wnoutrefresh(panels[1].win);
    doupdate();
  }

  /*init_pair(1, COLOR_RED, COLOR_BLACK);
  init_pair(2, COLOR_WHITE, COLOR_BLACK);
  refresh();
  wnd = newwin(32, 32, 2, 4);
  box(wnd, '|', '-');
  subwnd = derwin(wnd, 30, 30, 1, 1);
  wattron(subwnd, A_BOLD | COLOR_PAIR(1));
  wprintw(subwnd, "Hello brave new world!\n");
  wattroff(subwnd, A_BOLD | COLOR_PAIR(1));
  wrefresh(wnd);
  delwin(subwnd);
  delwin(wnd);

  attron(A_BLINK | COLOR_PAIR(2));
  move(9, 0);
  printw("Press any button...");
  refresh();
  getch();*/
  free(panels[0].files);
  free(panels[1].files);
  delwin(panels[0].win);
  delwin(panels[1].win);
  return EXIT_SUCCESS;
}
