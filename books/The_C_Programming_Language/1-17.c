#include <stdio.h>

#define and &&
#define or ||

#define MAXLINE 100
#define TRESHOLD 80

int getline(char line[], int maxline);

// Prints lines that are longer than 80 chars
int main(void) {
	int len;
	char line[MAXLINE + 1];

	while ((len = getline(line, MAXLINE)) > 0)
		if (len > TRESHOLD)
			printf("%d  - %s", len, line);

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
