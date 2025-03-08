#include <stdio.h>
#include <stdbool.h>

#define IN true
#define OUT false
#define and &&
#define or ||

#define MAX_WORD_LEN 45
#define MAX_ROW_LEN 40

static int is_word(int c) {
	if (c >= '0' and c <= '9'
		or c >= 'a' and c <= 'z'
		or c >= 'A' and c <= 'Z') {
		return true;
	}
	return false;
}

// Print the histogram of lengths of words of the input.
int main(void) {
	size_t len_occurrence[MAX_WORD_LEN + 1] = { 0 };
	size_t len_current = 0;
	int c = 0;
	bool state = OUT;

	while ((c = getchar()) != EOF) {
		if (!is_word(c)) {
			if (state == IN) {
				state = OUT;
				++len_occurrence[len_current];
				len_current = 0;
			}
		}
		else {
			state = IN;
			len_current++;
		}
	}

	size_t lowest = 0, highest = MAX_WORD_LEN;
	size_t max = 0;
	for (size_t i = 1; i < MAX_WORD_LEN; i++) {
		size_t occurrence = len_occurrence[i];

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
		size_t occurrence = len_occurrence[i];
		printf("%2zu (%2zu w.) |", i, occurrence);
		for (size_t hi = 0; hi < MAX_ROW_LEN; hi++) {
			if (hi < occurrence * normalization_coefficient) putchar('=');
			else putchar(' ');
		}
		putchar('|');
		putchar('\n');
	}
	return 0;
}
