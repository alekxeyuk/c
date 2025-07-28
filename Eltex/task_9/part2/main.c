#include <locale.h>
#include <ncurses.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "colors.h"
#include "panel.h"
#include "utils.h"

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
  curs_set(0);

  init_colors();
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
