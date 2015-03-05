#include <stdio.h>

int main(void)
{
	const int a = 5;

	//a = 7;					//NG

	int *pa = (int *)&a;

	*pa = 6;

	int ta = a;

	printf("a:%d  pa:%d   ta:%d\n", a, *pa, ta);

	const int b;

	(void)0;
	
	int *pb = (int *)&b;

	*pb = 8;

	int tb = b;

	printf("b:%d  pb:%d   tb:%d\n", b, *pb, tb);

	return 0;

}
