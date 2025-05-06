#include <stdio.h>

enum State { TEXT, CHAR, STRING, LINE, MULTILINE };
union InputChar {
  int ch_i;
  char c;
};

/*
 * Write a program to remove all comments from a C program.
 * Don't forget to handle quoted strings and character constants properly. C
 * comments do not nest.
 */

int main(void) {
  union InputChar ch = {.ch_i = 0};
  union InputChar prev = {.ch_i = EOF};
  unsigned char escaping = 0;
  enum State state = TEXT;

  while ((ch.ch_i = getchar()) != EOF) {
    switch (state) {
      case TEXT:
        if (ch.c == '/' && prev.c == '/') {
          state = LINE;
          prev.c = EOF;
        } else if (ch.c == '*' && prev.c == '/') {
          state = MULTILINE;
          prev.c = EOF;
        } else {
          if (prev.c != EOF) putchar(prev.c);
          if (ch.c == '"') {
            state = STRING;
            putchar(ch.c);
          } else if (ch.c == '\'') {
            state = CHAR;
            putchar(ch.c);
          }
          prev.c = ch.c;
        }
        break;
      case CHAR:
        putchar(ch.c);
        if (!escaping && ch.c == '\\') {
          escaping = 1;
        } else {
          if (!escaping && ch.c == '\'') {
            state = TEXT;
            ch.c = EOF;
          }
          escaping = 0;
        }
        prev.c = ch.c;
        break;
      case STRING:
        putchar(ch.c);
        if (!escaping && ch.c == '\\') {
          escaping = 1;
        } else {
          if (!escaping && ch.c == '\"') {
            state = TEXT;
            ch.c = EOF;
          }
          escaping = 0;
        }
        prev.c = ch.c;
        break;
      case LINE:
        if (ch.c == '\n') {
          putchar(ch.c);
          state = TEXT;
          prev.c = EOF;
        }
        break;
      case MULTILINE:
        if (prev.c == '*' && ch.c == '/') {
          state = TEXT;
          prev.c = EOF;
        } else {
          prev.c = ch.c;
        }
        break;
    }
  }

  if (state == MULTILINE && prev.c != EOF) {
    putchar('\n');
  } else if (prev.c != EOF) {
    putchar(prev.c);
  }

  return 0;
}