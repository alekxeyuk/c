#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#ifndef MAX_STACK_SIZE
#define MAX_STACK_SIZE 100
#endif

// Comments/Strings parsing
enum State { TEXT, CHAR, STRING, LINE, MULTILINE };
typedef union {
  int ch_i;
  char c;
} InputChar;

// Stack stuff
typedef struct {
  InputChar symbol;
  unsigned int line_number;
} StackElement;

typedef struct {
  StackElement data[MAX_STACK_SIZE];
  int top;
} Stack;

void initStack(Stack* s);
bool isEmpty(Stack* s);
bool isFull(Stack* s);
int push(Stack* s, char symbol, int line_number);
int pop(Stack* s, StackElement* element);
int peek(Stack* s, StackElement* element);

/*
 * Write a program to check a C program for rudimentary syntax
 * errors like unbalanced parentheses, brackets and braces. Don't forget about
 * quotes, both single and double, escape sequences, and comments.
 */

int main(void) {
  Stack stack;
  initStack(&stack);

  InputChar ch = {.ch_i = 0};
  InputChar prev = {.ch_i = EOF};
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

void initStack(Stack* s) {
  memset(s->data, 0, sizeof(s->data));
  s->top = -1;
}

bool isEmpty(Stack* s) { return s->top == -1; }

bool isFull(Stack* s) { return s->top == MAX_STACK_SIZE - 1; }

int push(Stack* s, char symbol, int line_number) {
  if (isFull(s)) {
    return -1;  // Stack is full
  }

  ++s->top;
  s->data[s->top].symbol.c = symbol;
  s->data[s->top].line_number = line_number;

  return 0;
}

int pop(Stack* s, StackElement* element) {
  if (isEmpty(s)) {
    return -1;  // Empty Stack
  }

  *element = s->data[s->top];
  --s->top;
  
  return 0;
}

int peek(Stack* s, StackElement* element) {
  if (isEmpty(s)) {
    return -1;  // Empty Stack
  }

  *element = s->data[s->top];

  return 0;
}
