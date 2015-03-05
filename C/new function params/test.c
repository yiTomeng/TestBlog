#include <stdio.h>

int add(a,b)
	int a; int b;
	{
		int sum = a + b;
		return sum;
	}


int main(void)
{
	int a = 10;
	int b = 20;

	printf("%d\n", add(a, b));
	return 0;
}
