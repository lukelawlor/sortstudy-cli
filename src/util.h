// Returns the literal characters passed to it as a string
#define	STR_LIT(x)	#x

// Returns the value of a macro as a string
#define	STR(x)		STR_LIT(x)

// Returnd the maximum of two ints
#define	MAX(x, y)	(x > y ? x : y)

// Returns the number of digits in a positive base 10 int
int get_digits(int x);
