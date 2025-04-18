#include <stdio.h>

#ifdef _WIN32
#define MY_SCANF(format, ...) scanf_s(format, __VA_ARGS__)
#else
#define MY_SCANF(format, ...) scanf(format, __VA_ARGS__)
#endif

#define N 5

typedef enum { SQUARE = 1, REVERSE, TRIANGLE, MATRIX } state;

static void menu_print(void) {
  printf(
      "\
Please choose mode of operation: \n\
1 - Square matrix\n\
2 - Reverse array\n\
3 - Triangle 0 and 1\n\
4 - Spiral matrix\n\
^D/^Z - Exit\n\
");
}

static int error_print(const char* const msg) {
  printf("%s", msg);
  return 1;
}

static int count_digits(int n) {
  int count = 0;
  while (n != 0) {
    n /= 10;
    count++;
  }
  return count;
}

static void array_print(int arr[], int size, int matrix_sup) {
  for (int i = 0; i < size; i++) {
    printf("%*d ", matrix_sup, arr[i]);
  }
  putchar('\n');
}

static void matrix_print(int arr[], int size) {
  int max_digits = count_digits(size * size);
  for (int i = 0; i < size; i++) {
    array_print(arr + i * size, size, max_digits);
  }
}

static int square(void) {
  int arr[N][N] = {{0}};
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      arr[i][j] = (i * N) + j + 1;
    }
  }
  matrix_print((int*)arr, N);
  return 0;
}

static int reverse(void) {
  int arr[N] = {0};

  for (int i = 0; i < N; i++) {
    arr[i] = i + 1;
  }
  array_print(arr, N, 0);

  for (int i = 0; i < N / 2; i++) {
    arr[i] ^= arr[N - 1 - i];
    arr[N - 1 - i] ^= arr[i];
    arr[i] ^= arr[N - 1 - i];
  }
  array_print(arr, N, 0);
  return 0;
}

static int triangle(void) {
  int arr[N][N] = {{0}};
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      arr[i][j] = i + j >= N - 1 ? 1 : 0;
    }
  }
  matrix_print((int*)arr, N);
  return 0;
}

static int matrix(void) {
  int arr[N][N] = {{0}};
  int cnt = 1;
  int t = 0;      // top row
  int b = N - 1;  // bottom row
  int l = 0;      // left column
  int r = N - 1;  // right column
  int d = 0;      // 0 - right, 1 - down, 2 - left, 3 - up

  while (t <= b && l <= r) {
    switch (d) {
      case 0:  // → from left to right on top row
        for (int i = l; i <= r; i++) {
          arr[t][i] = cnt++;
        }
        t++;
        break;
      case 1:  // ↓ from top to bottom on right column
        for (int i = t; i <= b; i++) {
          arr[i][r] = cnt++;
        }
        r--;
        break;
      case 2:  // ← from right to left bottom row
        for (int i = r; i >= l; i--) {
          arr[b][i] = cnt++;
        }
        b--;
        break;
      case 3:  // ↑ from bottom to the top on left column
        for (int i = b; i >= t; i--) {
          arr[i][l] = cnt++;
        }
        l++;
        break;
    }
    d = (d + 1) % 4;
  }

  matrix_print((int*)arr, N);
  return 0;
}

int main(void) {
  int user_input = 0;
  menu_print();

  if (MY_SCANF("%d", &user_input) == 0) {
    return error_print("Your input is not a number");
  }

  switch (user_input) {
    case SQUARE:
      return square();
    case REVERSE:
      return reverse();
    case TRIANGLE:
      return triangle();
    case MATRIX:
      return matrix();
    default:
      return error_print("No such option");
  }
}
