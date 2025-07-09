#include <stdio.h>
#include "utils.h"

int strncmp(const char* ptr1, const char* ptr2, size_t n) {
  char* f = (char*)ptr1;
  char* s = (char*)ptr2;
  for (size_t i = 0; i < n; i++) {
    if (f[i] == '\0' && s[i] == '\0') return 0;
    if (f[i] > s[i]) return 1;
    if (f[i] < s[i]) return -1;
  }
  return 0;
}

void flush_stdint(void) {
  int c;
  while ((c = getchar()) != '\n' && c != EOF);
}