#include <stdio.h>

float f_to_c(float F);
void print_table(int lower, int higher, int step);

// Convert Fahrenheit to Celsius
int main(void) {
	int lower, higher, step;
	scanf_s("%d %d %d", &lower, &higher, &step);
	print_table(lower, higher, step);
	return 0;
}

float f_to_c(float F) {
	return (float)((5.0 / 9.0) * (F - 32.0));
}

void print_table(int lower, int higher, int step) {
	float f = (float)lower;
	float c = 0.0;
	while (f <= higher) {
		c = f_to_c(f);
		printf("%3.0f %3.1f\n", f, c);
		f += step;
	}
}
