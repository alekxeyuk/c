#ifndef PANEL_H_
#define PANEL_H_
#include <limits.h>
#include <ncurses.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

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

void init_panel(PANEL* p, bool active, int cols, int y, int x);
bool enter_folder(PANEL* a_pn, const char* path);
bool read_folder(PANEL* a_pn);
void draw_file_list(PANEL* a_pn);
void draw_top_bar(PANEL* a_pn);
void render_panels(PANEL panels[2]);
#endif  // PANEL_H_