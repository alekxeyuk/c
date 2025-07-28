#ifndef COLORS_H_
#define COLORS_H_

enum COLOR_SCHEME {
  CS_DIR = 1,
  CS_FILE,
  CS_SELECTED,
  CS_STATUS_ACT,
  CS_STATUS_NACT
};

void init_colors(void);
#endif  // COLORS_H_