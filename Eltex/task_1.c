#include <stdio.h>

#ifdef _WIN32
#define MY_SCANF(format, ...) scanf_s(format, __VA_ARGS__)
#else
#define MY_SCANF(format, ...) scanf(format, __VA_ARGS__)
#endif

#define BI_IN_BY 8
#define THIRD_BYTE_SHIFT (BI_IN_BY * 2)
#define THIRD_BYTE_MASK 0xFF00FFFF

typedef enum { BINARY = 1, COUNT, CHANGE } state;

static void menu_print(void) {
  printf(
      "\
Please choose mode of operation: \n\
1 - Binary representation of a number\n\
2 - Count one's inside number's binary representation\n\
3 - Modify third byte in a number\n\
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

//  Представление числа в двоичной системе
static int dec_to_bin(void) {
  int number = 0;
  if (MY_SCANF("%d", &number) == 0) {
    return error_print("Wrong number");
  }

  bin_print(&number);
  return 0;
}

// Подсчёт количества единиц в двоичном представлении
static int count_bits(void) {
  int number = 0;
  if (MY_SCANF("%d", &number) == 0) {
    return error_print("Wrong number");
  }

  unsigned int count = 0;

  unsigned int mask = 1;
  for (int bit = 0; bit < (int)sizeof(number) * BI_IN_BY; bit++) {
    if (number & mask) {
      count++;
    }
    mask <<= 1;
  }
  printf("%u", count);
  return 0;
}

// Изменение третьего байта в числе
static int change_byte(void) {
  int orig_num = 0;
  int byte_val = 0;
  if (MY_SCANF("%d", &orig_num) == 0 || orig_num < 0) {
    return error_print("Wrong first number");
  }
  if (MY_SCANF("%d", &byte_val) == 0 || byte_val > 255 || byte_val < 0) {
    return error_print(
        "Wrong second number. Enter an unsigned byte value (0-255)");
  }

  printf("Original before modifying:       ");
  bin_print(&orig_num);
  putchar('\n');

  printf("Byte to insert before modifying: ");
  bin_print(&byte_val);
  putchar('\n');

  byte_val <<= THIRD_BYTE_SHIFT;
  orig_num &= THIRD_BYTE_MASK;
  orig_num |= byte_val;

  printf("Byte to insert after modifying:  ");
  bin_print(&byte_val);
  putchar('\n');

  printf("Original after modifying:        ");
  bin_print(&orig_num);
  putchar('\n');
  return 0;
}

int main(void) {
  int user_input = 0;
  menu_print();

  if (MY_SCANF("%d", &user_input) == 0) {
    return error_print("Your input is not a number");
  }

  switch (user_input) {
    case BINARY:
      return dec_to_bin();
    case COUNT:
      return count_bits();
    case CHANGE:
      return change_byte();
    default:
      return error_print("No such option");
  }
}
