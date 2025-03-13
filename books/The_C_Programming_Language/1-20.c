#include <stdio.h>

#define and &&
#define or ||

#define COLLUMNS_MAX 8

// Replace tabulation with spaces
int main(void) {
	int c;
	size_t column = 0;

	while ((c = getchar()) != EOF) {
		if (c == '\t') {
			size_t spaces = COLLUMNS_MAX - (column % COLLUMNS_MAX);
			column += spaces;
			while (spaces--) putchar(' ');
		}
		else {
			putchar(c);
			column++;
		}
		if (c == '\n') column = 0;
	}
	return 0;
}
