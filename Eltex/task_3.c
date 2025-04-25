#include <locale.h>
#include <stdio.h>

#ifdef _WIN32
#define MY_SCANF(format, ...) scanf_s(format, __VA_ARGS__)
#else
#define MY_SCANF(format, ...) scanf(format, __VA_ARGS__)
#endif

#define BI_IN_BY 8
#define N 10
#define BUFF_SIZE 1024

typedef enum { THIRD = 1, SUBTASK, ARRAY, SUBSTR } state;

static int array_task(void);
static void bin_print(const int* const number);
static int change_third_byte(void);
static int error_print(const char* const msg);
static void flush_stdint(void);
static void menu_print(void);
static int str_c_find(char* string, char c);
static int str_len(char* string);
static char* substr(char* string, char* sub);
static int substr_task(void);
static int subtask_2(void);

int main(void) {
  setlocale(LC_ALL, "Russian");

  state user_input = 0;
  menu_print();

  if (MY_SCANF("%d", &user_input) == 0) {
    return error_print("Your input is not a number");
  }

  flush_stdint();

  switch (user_input) {
    case THIRD:
      return change_third_byte();
    case SUBTASK:
      return subtask_2();
    case ARRAY:
      return array_task();
    case SUBSTR:
      return substr_task();
    default:
      return error_print("No such option");
  }
}

static void menu_print(void) {
  printf(
      "\
Please choose mode of operation: \n\
1 - Change third byte\n\
2 - SubTask - result 12.0\n\
3 - Array print using pointers\n\
4 - SubString search\n\
^D/^Z - Exit\n\
");
}

static int error_print(const char* const msg) {
  printf("%s", msg);
  return 1;
}

static void bin_print(const int* const number) {
  unsigned char* ptr_to_number = (unsigned char*)number;

  for (int byte = sizeof(*number) - 1; byte >= 0; --byte) {
    for (int bit = BI_IN_BY - 1; bit >= 0; --bit) {
      putchar(((ptr_to_number[byte] >> bit) & 1) + '0');
    }
    putchar('|');
  }
}

static void flush_stdint(void) {
  int c;
  while ((c = getchar()) != '\n' && c != EOF);
}

static int change_third_byte(void) {
  int orig_num = 0;
  int byte_val = 0;
  if (MY_SCANF("%d", &orig_num) == 0 || orig_num < 0) {
    return error_print("Wrong first number");
  }
  if (MY_SCANF("%d", &byte_val) == 0 || byte_val > 255 || byte_val < 0) {
    return error_print(
        "Wrong second number. Enter an unsigned byte value (0-255)");
  }

  bin_print(&orig_num);
  putchar('\n');

  unsigned char* ptr = (unsigned char*)&orig_num;
  ptr[2] = (unsigned char)byte_val;

  bin_print(&orig_num);
  putchar('\n');

  return 0;
}

static int subtask_2(void) {
  float x = 5.0;
  printf("x = %f, ", x);
  float y = 6.0;
  printf("y = %f\n", y);
  // TODO: отредактируйте эту строку, и только правую часть уравнения
  float* xp = &y;
  float* yp = &y;
  printf("Результат: %f\n", *xp + *yp);
  return 0;
}

static int array_task(void) {
  int array[N] = {0};

  for (int i = 0; i < N; i++) {
    *(array + i) = i + 1;
  }

  for (size_t i = 0; i < N; i++) {
    printf("%d ", *(array + i));
  }
  return 0;
}

static char* substr(char* string, char* sub) {
  while (*string) {
    if (*string == *sub) {
      char* p = string;
      char* s = sub;
      while (*s && *p == *s) {
        s++;
        p++;
      }
      if (*s == '\0') {
        return string;
      }
    }
    string++;
  }
  return NULL;
}

static int str_c_find(char* string, char c) {
  int index;
  for (index = 0; *(string) && *(string) != c; string++) {
    index++;
  }
  return index;
}

static int str_len(char* string) { return str_c_find(string, '\0'); }

static int substr_task(void) {
  char string_buff[BUFF_SIZE] = {0};
  char sub_buff[BUFF_SIZE] = {0};

  if (fgets(string_buff, sizeof(string_buff), stdin) == NULL) {
    return error_print("Wrong first input");
  }
  if (fgets(sub_buff, sizeof(sub_buff), stdin) == NULL) {
    return error_print("Wrong second input");
  }

  string_buff[str_c_find(string_buff, '\n')] = '\0';
  sub_buff[str_c_find(sub_buff, '\n')] = '\0';

  char* sub_ptr = substr(string_buff, sub_buff);

  if (sub_ptr == NULL) {
    return error_print("No match found");
  }

  printf("Match: %s\n", string_buff);
  printf("%*c", (int)(sub_ptr - string_buff) + 7, ' ');
  for (int i = 0; i < str_len(sub_buff); i++) {
    putchar('^');
  }
  putchar('\n');
  return 0;
}
