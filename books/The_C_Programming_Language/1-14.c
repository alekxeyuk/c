#include <stdio.h>
#include <stdbool.h>

#define IN true
#define OUT false
#define and &&
#define or ||

#define MAX_CHARS 255
#define LOWEST_PRINTABLE 33
#define HIGHEST_PRINTABLE 126
#define MAX_ROW_LEN 40


// Print the histogram of frequencies of characters of the input.
int main(void) {
	size_t char_occurrence[MAX_CHARS] = { 0 };
	int c = 0;

	while ((c = getchar()) != EOF) {
		++char_occurrence[c];
	}

	size_t lowest = 0, highest = HIGHEST_PRINTABLE;
	size_t max = 0;
	for (size_t i = LOWEST_PRINTABLE; i <= HIGHEST_PRINTABLE; i++) {
		size_t occurrence = char_occurrence[i];

		if (occurrence > max)
			max = occurrence;

		if (occurrence != 0) {
			if (lowest == 0)
				lowest = i;
			highest = i;
		}
	}

	if (max == 0) {
		printf("No data\n");
		return 1;
	}

	float normalization_coefficient = ((float)MAX_ROW_LEN) / max;

	for (size_t i = lowest; i <= highest; i++) {
		size_t occurrence = char_occurrence[i];
		printf("%2zu (%c) |", occurrence, (int)i);
		for (size_t hi = 0; hi < MAX_ROW_LEN; hi++) {
			if (hi < occurrence * normalization_coefficient) putchar('=');
			else putchar(' ');
		}
		putchar('|');
		putchar('\n');
	}
	return 0;
}
