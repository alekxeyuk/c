#include <stdio.h>
#include <stdbool.h>

#define IN true
#define OUT false
#define and &&
#define or ||

// Write a program that prints its input one word per line.
int main(void) {
	int c = 0;
	bool state = OUT;

	while ((c = getchar()) != EOF) {
		if (c == ' ' or c == '\t' or c == '\n') {
			if (state == IN) {
				state = OUT;
				putchar('\n');
			}
		}
		else {
			state = IN;
			putchar(c);
		}
	}
	return 0;
}
