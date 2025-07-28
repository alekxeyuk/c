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

enum COLOR_SCHEME {
  CS_DIR = 1,
  CS_FILE,
  CS_SELECTED,
  CS_STATUS_ACT,
  CS_STATUS_NACT
};

static void cleanup(void) { endwin(); }
static bool prefix(const char* pre, const char* str) {
  return strncmp(pre, str, strlen(pre)) == 0;
}
// Sorts FENTRY's into two categories: files and directories, with latter on top
static int sort_by_type(const void* a, const void* b) {
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
  p->cols = cols;
  p->files = NULL;
}
bool enter_folder(PANEL* a_pn, const char* path) {
  int st = chdir(path);
  if (st == -1) {
    perror("Failed to enter folder");
    return false;
  }
  getcwd(a_pn->cwd, sizeof(a_pn->cwd));
  return true;
}
// Scan files on path and populate panel FENTRY array, sorted
bool read_folder(PANEL* a_pn) {
  struct stat sb;
  struct dirent** namelist;

  int list_size = scandir(".", &namelist, NULL, alphasort);
  if (list_size == -1) {
    return false;
  }

  a_pn->nfiles = (size_t)list_size;
  if (a_pn->files == NULL) {
    a_pn->files = malloc(sizeof(FENTRY[list_size]));
  } else {
    a_pn->files = realloc(a_pn->files, sizeof(FENTRY[list_size]));
  }
  if (a_pn->files == NULL) {
    return false;
  }

  int j = 0;
  for (int i = 0; i < list_size; i++) {
    if (strcmp(namelist[i]->d_name, ".") == 0) {
      free(namelist[i]);
      continue;
    }

    stat(namelist[i]->d_name, &sb);
    strncpy(a_pn->files[j].name, namelist[i]->d_name,
            sizeof(a_pn->files[j].name) - 1);
    a_pn->files[j].name[sizeof(a_pn->files[j].name) - 1] = '\0';
    strftime(a_pn->files[j].mtime_str, sizeof(a_pn->files[j].mtime_str),
             "%b %e %R", localtime(&sb.st_mtim.tv_sec));
    a_pn->files[j].is_dir = S_ISDIR(sb.st_mode);
    a_pn->files[j].size = (intmax_t)sb.st_size;

    j++;
    free(namelist[i]);
  }
  free(namelist);

  a_pn->nfiles = (size_t)j;

  // Two category: file and directory
  qsort(a_pn->files, a_pn->nfiles, sizeof(FENTRY), sort_by_type);
  a_pn->selected = 0;
  return true;
}

void draw_file_list(PANEL* a_pn) {
  WINDOW* win = a_pn->win;
  const int max_len = a_pn->cols - (MTIMMX + SMX + 4);

  int color_scheme = CS_FILE;
  for (size_t i = 0; i < a_pn->nfiles; i++) {
    const FENTRY* file = &a_pn->files[i];
    const int name_len = (int)strlen(file->name);
    const int l = max_len / 2;
    const int r = max_len / 2 + max_len % 2;

    if (file->is_dir) {
      color_scheme = CS_DIR;
    } else {
      color_scheme = CS_FILE;
    }
    if (i == a_pn->selected && a_pn->active) {
      color_scheme = CS_SELECTED;
    }
    wattron(win, COLOR_PAIR(color_scheme));
    if (max_len < 3) {
      NULL;
    } else if (name_len > max_len + 1) {
      mvwprintw(win, (int)i + 1, 1, "%.*s~%s", l, file->name,
                file->name + name_len - r);
    } else {
      mvwprintw(a_pn->win, (int)i + 1, 1, "%-*s",
                a_pn->cols - (MTIMMX + SMX + 3), a_pn->files[i].name);
    }
    mvwprintw(win, (int)i + 1, a_pn->cols - MTIMMX - SMX - 1, "%7jd",
              file->size);
    mvwprintw(win, (int)i + 1, a_pn->cols - MTIMMX, "%s", file->mtime_str);
    wattroff(win, COLOR_PAIR(color_scheme));
  }
  mvwvline(win, 1, a_pn->cols - MTIMMX - SMX - 1 - 1, ACS_VLINE, LINES - 2);
  mvwvline(win, 1, a_pn->cols - MTIMMX - 1, ACS_VLINE, LINES - 2);
}
// Static Getter/Setter for curent user's PWD
const char* stored_user_pwd(void* ptr) {
  static const char* pwd;
  if (ptr != NULL) {
    pwd = (const char*)ptr;
  }
  return pwd;
}
// Displays current path on top of the files list, replaces it with ~ if we are
// at user's home dir
void draw_top_bar(PANEL* a_pn) {
  const char* user_pwd = stored_user_pwd(NULL);
  const bool is_under_home = prefix(user_pwd, a_pn->cwd);
  const int max_len = a_pn->cols - 8;
  const int pwd_len = (int)strlen(user_pwd);
  const int cwd_len = (int)strlen(a_pn->cwd);

  int resulting_len = cwd_len;
  if (is_under_home) {
    resulting_len -= pwd_len;
  }

  const int color_scheme = a_pn->active ? CS_STATUS_ACT : CS_STATUS_NACT;
  wattron(a_pn->win, COLOR_PAIR(color_scheme));
  if (resulting_len > max_len) {
    mvwprintw(a_pn->win, 0, 3, " ...%s ", a_pn->cwd + cwd_len - (max_len - 2));
  } else if (is_under_home) {
    mvwprintw(a_pn->win, 0, 3, " ~%s ", a_pn->cwd + pwd_len);
  } else {
    mvwprintw(a_pn->win, 0, 3, " %s ", a_pn->cwd);
  }
  wattroff(a_pn->win, COLOR_PAIR(color_scheme));
}

void render_panels(PANEL panels[2]) {
  PANEL* a_pn;
  for (int p_i = 0; p_i < 2; p_i++) {
    a_pn = &panels[p_i];

    a_pn->cols = (COLS + p_i) / 2 - 1;
    box(a_pn->win, ACS_VLINE, ACS_HLINE);
    draw_file_list(a_pn);
    draw_top_bar(a_pn);
    wnoutrefresh(a_pn->win);
  }
}

void sel_dec(size_t* sel) {
  if (*sel > 0) (*sel)--;
}
void sel_inc(size_t* sel, size_t max) {
  if (*sel < max) (*sel)++;
}

int main(void) {
  setlocale(LC_ALL, "C.utf8");

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
  stored_user_pwd(user_pwd->pw_dir);

  int selected = 0;
  int left_w = COLS / 2;
  int right_w = COLS - left_w;

  PANEL panels[2];
  init_panel(&panels[0], true, left_w, 0, 0);
  init_panel(&panels[1], false, right_w, 0, left_w);
  bool read_st = read_folder(&panels[0]);
  read_st &= read_folder(&panels[1]);
  if (!read_st) {
    perror("Was not able to initialize panels with CWD of current user");
    exit(EXIT_FAILURE);
  }

  int ch = 0;
  do {
    if (ch == KEY_RESIZE) {
      left_w = COLS / 2;
      right_w = COLS - left_w;
      werase(stdscr);
      werase(panels[0].win);
      werase(panels[1].win);

      wresize(panels[0].win, LINES, left_w);
      wresize(panels[1].win, LINES, right_w);
      mvwin(panels[1].win, 0, left_w);
      wnoutrefresh(stdscr);
    } else if (ch == KEY_UP) {
      sel_dec(&panels[selected].selected);
    } else if (ch == KEY_DOWN) {
      sel_inc(&panels[selected].selected, panels[selected].nfiles - 1);
    } else if (ch == '\n') {
      size_t sel_f_id = panels[selected].selected;
      FENTRY* sel_file = &panels[selected].files[sel_f_id];
      if (sel_file->is_dir) {
        read_st &= enter_folder(&panels[selected], sel_file->name);
        if (!read_st) {
          perror("Was not able to change CWD to a new folder");
          exit(EXIT_FAILURE);
        }
        read_st &= read_folder(&panels[selected]);
        if (!read_st) {
          perror("Was not able to initialize panels with CWD of current user");
          exit(EXIT_FAILURE);
        }
        werase(panels[selected].win);
      }
    } else if (ch == '\t') {
      panels[selected].active = false;
      selected = selected ? 0 : 1;
      panels[selected].active = true;
      read_st &= enter_folder(&panels[selected], panels[selected].cwd);
      if (!read_st) {
        perror("Was not able to change CWD to a new folder");
        exit(EXIT_FAILURE);
      }
    }
    render_panels(panels);
    doupdate();
  } while ((ch = wgetch(stdscr)) != 'q');

  free(panels[0].files);
  free(panels[1].files);
  delwin(panels[0].win);
  delwin(panels[1].win);
  return EXIT_SUCCESS;
}
