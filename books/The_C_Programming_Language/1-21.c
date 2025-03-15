#include <stdio.h>
#define TAB_WIDTH 8

// Flush the accumulated spaces by printing either tabs or spaces.
static void flush_spaces(int* column, int space_count) {
	while (space_count > 0) {
		int spaces_to_tab = TAB_WIDTH - (*column % TAB_WIDTH);
		if (space_count >= spaces_to_tab) {
			putchar('\t');
			*column += spaces_to_tab;
			space_count -= spaces_to_tab;
		}
		else {
			while (space_count-- > 0) {
				putchar(' ');
				(*column)++;
			}
			break;
		}
	}
}

// Replace spaces with tabulation
int main(void) {
	int ch;
	int space_count = 0;
	int column = 0;

	while ((ch = getchar()) != EOF) {
		if (ch == ' ') space_count++;
		else {
			if (space_count) {
				flush_spaces(&column, space_count);
				space_count = 0;
			}
			if (ch == '\t') {
				putchar(ch);
				column = (column / TAB_WIDTH + 1) * TAB_WIDTH;
			}
			else {
				putchar(ch);
				column++;
				if (ch == '\n') column = 0;
			}
		}
	}

	if (space_count) flush_spaces(&column, space_count);
	return 0;
}
