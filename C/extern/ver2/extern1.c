#include <stdio.h>

#include "extern2.h"

int main(int argc, char* argv)
{
	int a = 10;
	int b = 12;
	
	printf("%d\n", m + add_two_times(a, b));
	
	return 0;
}

int add(int a, int b)
{
	return (a+b);
}