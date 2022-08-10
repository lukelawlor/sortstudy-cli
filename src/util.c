#include "util.h"

// Returns the number of digits in a positive base 10 int
int get_digits(int x)
{
	int digits = 1;
	while ((x /= 10) > 0)
		digits++;
	return digits;
}
