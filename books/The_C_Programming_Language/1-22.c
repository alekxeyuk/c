#include <stdio.h>
#include <ctype.h>

#define MAX_LINE (4 - 1)
#define MAX_BUFF (MAX_LINE + 2)

#define INWORD 1
#define OUTWORD 0

/*
* A program to "fold" long input lines into two or more shorter lines
* after the last non-blank character that occurs before the n-th column of input.
*/

static void dump_buff(char* buff, int n) {
	for (int i = 0; i <= n; i++) {
		putchar(buff[i]);
	}
}
static int l_shift_buff(char* buff, int from) {
	int count;
	int ops = (MAX_BUFF - from);
	for (count = 0; count < ops; count++) {
		buff[count] = buff[from++];
	}
	return count;
}

int main(void) {
	int ch;
	int idx = 0;
	int last_word_end = -1;
	char state = OUTWORD;
	char buff[MAX_BUFF] = { '\0' };


	while ((ch = getchar()) != EOF) {
		if (ch == '\n' && idx > 0) {
			buff[idx] = '\0';
			idx = 0;
			last_word_end = -1;
			state = OUTWORD;
			printf("%s\n", buff);
			continue;
		}

		buff[idx] = (char)ch;

		if (isspace(ch)) {
		    if (state == INWORD) {
		        state = OUTWORD;
		        last_word_end = idx - 1;
		    }
		}
		else if (state == OUTWORD) {
		    state = INWORD;
		}

		if (idx <= MAX_LINE) {
			idx++;
		}
		else {
			last_word_end = last_word_end >= 0 ? last_word_end : MAX_LINE;
			dump_buff(buff, last_word_end);
			idx = l_shift_buff(buff, last_word_end + 1);

			last_word_end = -1;
			putchar('\n');
		}
	}

	return 0;
}
