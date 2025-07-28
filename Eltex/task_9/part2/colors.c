#include "colors.h"

#include <ncurses.h>

void init_colors(void) {
  start_color();
  init_pair(CS_DIR, COLOR_YELLOW, COLOR_BLACK);         // directory
  init_pair(CS_FILE, COLOR_GREEN, COLOR_BLACK);         // file
  init_pair(CS_SELECTED, COLOR_BLACK, COLOR_CYAN);      // selection
  init_pair(CS_STATUS_ACT, COLOR_BLACK, COLOR_WHITE);   // status bar active
  init_pair(CS_STATUS_NACT, COLOR_WHITE, COLOR_BLACK);  // status bar nactive
}