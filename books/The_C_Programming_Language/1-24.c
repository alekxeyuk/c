#include <stdbool.h>
#include <stdio.h>
#include <string.h>

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

static void syntax_error(char c_c, unsigned line_c, char c_p, unsigned line_p);

/*
 * Write a program to check a C program for rudimentary syntax
 * errors like unbalanced parentheses, brackets and braces. Don't forget about
 * quotes, both single and double, escape sequences, and comments.
 */

int main(void) {
  Stack stack;
  StackElement element = {'\0', 0};
  initStack(&stack);

  InputChar ch = {.ch_i = 0};
  InputChar prev = {.ch_i = EOF};
  unsigned char escaping = 0;
  enum State state = TEXT;

  unsigned int line_count = 0;

  while ((ch.ch_i = getchar()) != EOF) {
    if (ch.c == '\n') line_count++;
    switch (state) {
      case TEXT:
        if (ch.c == '/' && prev.c == '/') {
          state = LINE;
          prev.c = EOF;
        } else if (ch.c == '*' && prev.c == '/') {
          state = MULTILINE;
          prev.c = EOF;
        } else {
          if (ch.c == '"') {
            state = STRING;
          } else if (ch.c == '\'') {
            state = CHAR;
          } else {
            switch (ch.c) {
              case '(':
              case '[':
              case '{':
                if (push(&stack, ch.c, line_count) == -1) {
                  printf("Stack overflowed on <%c> at <%u>\n", ch.c,
                         line_count);
                }
                break;
              case ')':
              case ']':
              case '}':
                if (pop(&stack, &element) == -1) {
                  printf("Unexpected closing token: <%c> at line number <%u>\n",
                         ch.c, line_count);
                  break;
                }
                if (ch.c == ')' && element.symbol.c != '(') {
                  syntax_error(ch.c, line_count, element.symbol.c,
                               element.line_number);
                } else if (ch.c == ']' && element.symbol.c != '[') {
                  syntax_error(ch.c, line_count, element.symbol.c,
                               element.line_number);
                } else if (ch.c == '}' && element.symbol.c != '{') {
                  syntax_error(ch.c, line_count, element.symbol.c,
                               element.line_number);
                }
                break;
              default:
                break;
            }
          }
          prev.c = ch.c;
        }
        break;
      case CHAR:
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

while (pop(&stack, &element) != -1) {
    printf("Tokens left in stack: <%c> at line number <%u>\n", element.symbol.c,
           element.line_number);
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

static void syntax_error(char c_c, unsigned line_c, char c_p, unsigned line_p) {
  printf("Unbalanced closing token: <%c> at <%u> | <%c> at <%u>\n", c_c, line_c,
         c_p, line_p);
}
