#include <stdio.h>

#define and &&
#define or ||

#define MAXLINE 10

int getline(char line[], int maxline);
void copy(char from[], char to[]);

// Finds the longest line length
int main(void) {
	int len, max = 0;
	char line[MAXLINE + 1];
	char long_line[MAXLINE + 1];

	while ((len = getline(line, MAXLINE)) > 0)
		if (len > max) {
			max = len;
			copy(line, long_line);
		}

	if (max > 0)
		printf("%d  - %s", max, long_line);
	return 0;
}

int getline(char s[], int lim) {
	int c, i;

	for (i = 0; (c = getchar()) != EOF and c != '\n'; i++)
		if (i < lim)
			s[i] = (char)c;

	if ((lim)-i > 1 and c == '\n')
		s[i++] = (char)c;

	s[i > lim ? lim : i] = '\0';
	return i;
}

void copy(char from[], char to[]) {
	for (size_t i = 0; (to[i] = from[i]) != '\0'; i++);
}
