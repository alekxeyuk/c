#include <stdio.h>

#define and &&
#define or ||

#define MAXLINE 100

int my_getline(char line[], int maxline);
static int is_blank(int c);
int trail_cleanup(char s[], int lim);

// Removes trailing blanks and tabs
int main(void) {
	int len;
	char line[MAXLINE + 1];

	putchar('>');
	while ((len = my_getline(line, MAXLINE)) > 0) {
		len = trail_cleanup(line, len > MAXLINE ? MAXLINE : len);
		printf("%d  - |%s|\n>", len, line);
	}
	return 0;
}

int my_getline(char s[], int lim) {
	int c, i;

	for (i = 0; (c = getchar()) != EOF and c != '\n'; i++)
		if (i < lim)
			s[i] = (char)c;

	if ((lim)-i > 1 and c == '\n')
		s[i++] = (char)c;

	s[i > lim ? lim : i] = '\0';
	return i;
}

int trail_cleanup(char s[], int lim) {
	lim--;
	while (lim >= 0 && is_blank(s[lim])) {
		lim--;
	}
	s[++lim] = '\0';
	return lim;
}

static int is_blank(int c) {
	return c == ' ' or c == '\t' or c == '\n';
}
