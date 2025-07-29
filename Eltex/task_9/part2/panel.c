#include "panel.h"

#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "colors.h"
#include "utils.h"

static int sort_by_type(const void* a, const void* b) {
  const FENTRY* entryA = (const FENTRY*)a;
  const FENTRY* entryB = (const FENTRY*)b;
  if (entryA->is_dir && !entryB->is_dir) return -1;
  if (!entryA->is_dir && entryB->is_dir) return 1;
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

bool read_folder(PANEL* a_pn) {
  struct stat sb;
  struct dirent** namelist;
  int list_size = scandir(".", &namelist, NULL, alphasort);
  if (list_size == -1) return false;
  a_pn->nfiles = (size_t)list_size;
  if (a_pn->files == NULL) {
    a_pn->files = malloc(sizeof(FENTRY[list_size]));
  } else {
    a_pn->files = realloc(a_pn->files, sizeof(FENTRY[list_size]));
  }
  if (a_pn->files == NULL) return false;
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
  qsort(a_pn->files, a_pn->nfiles, sizeof(FENTRY), sort_by_type);
  a_pn->selected = 0;
  a_pn->topline = 0;
  return true;
}

void draw_file_list(PANEL* a_pn) {
  WINDOW* win = a_pn->win;
  const int max_len = a_pn->cols - (MTIMMX + SMX + 4);

  int color_scheme = CS_FILE;
  int j = 1;
  for (size_t i = a_pn->topline; i < a_pn->nfiles; i++) {
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
      mvwprintw(win, j, 1, "%.*s~%s", l, file->name, file->name + name_len - r);
    } else {
      mvwprintw(a_pn->win, j, 1, "%-*s", a_pn->cols - (MTIMMX + SMX + 3),
                a_pn->files[i].name);
    }
    mvwprintw(win, j, a_pn->cols - MTIMMX - SMX - 1, "%7jd", file->size);
    mvwprintw(win, j, a_pn->cols - MTIMMX, "%s", file->mtime_str);
    wattroff(win, COLOR_PAIR(color_scheme));
    if (j >= LINES - 2) break;
    j++;
  }
  mvwvline(win, 1, a_pn->cols - MTIMMX - SMX - 1 - 1, ACS_VLINE, LINES - 2);
  mvwvline(win, 1, a_pn->cols - MTIMMX - 1, ACS_VLINE, LINES - 2);
}

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