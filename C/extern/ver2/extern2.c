#include "extern2.h"

int m = 5;

int add_two_times(int a, int b)
{
	int c = add(a, b);
	
	return (c + c);
}