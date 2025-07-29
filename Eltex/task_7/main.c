#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "libcalc.h"

typedef enum { ADD = 1, SUB, MUL, DIV, EXIT } state;

static void menu_print(void);
static void flush_stdint(void);
bool read_ab(int*, int*);

static void addition(void);
static void subtraction(void);
static void multiplication(void);
static void division(void);

int main(void) {
  setlocale(LC_ALL, "C.utf8");
  state user_input = 0;

  do {
    menu_print();
	int r = scanf("%d", (int*)&user_input);
    if (r == 0) {
      flush_stdint();
      printf("Некорректный ввод\n");
      continue;
    }
    flush_stdint();

    switch (user_input) {
      case ADD:
        addition();
        break;
      case SUB:
        subtraction();
        break;
      case MUL:
        multiplication();
        break;
      case DIV:
        division();
        break;
      case EXIT:
        exit(EXIT_SUCCESS);
      default:
        printf("Некорректный ввод\n");
        break;
    }
  } while (1);
  
  exit(EXIT_SUCCESS);
}

static void menu_print(void) {
  printf(
      "\
1) Сложение\n\
2) Вычитание\n\
3) Умножение\n\
4) Деление\n\
5) Выход\n\
");
}

static void flush_stdint(void) {
  int c;
  while ((c = getchar()) != '\n' && c != EOF);
}

bool read_ab(int* a, int* b) {
	printf("a = ");
	if (scanf("%d", a) == 0) return false;
	printf("b = ");
	if (scanf("%d", b) == 0) return false;
	return true;
}

static void addition(void) {
	int a, b = 0;
	if (!read_ab(&a, &b)) {
		printf("Failed to understand your input\n");
		flush_stdint();
		return;
	}
	printf("%d + %d = %d\n", a, b, add(a, b));
}

static void subtraction(void) {
	int a, b = 0;
	if (!read_ab(&a, &b)) {
		printf("Failed to understand your input\n");
		flush_stdint();
		return;
	}
	printf("%d - %d = %d\n", a, b, sub(a, b));
}

static void multiplication(void) {
	int a, b = 0;
	if (!read_ab(&a, &b)) {
		printf("Failed to understand your input\n");
		flush_stdint();
		return;
	}
	printf("%d * %d = %d\n", a, b, mul(a, b));
}

static void division(void) {
	int a, b = 0;
	if (!read_ab(&a, &b)) {
		printf("Failed to understand your input\n");
		flush_stdint();
		return;
	}
	printf("%d / %d = %d\n", a, b, divide(a, b));
}