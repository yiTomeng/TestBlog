#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	int *a = (int*)malloc(sizeof(int));

	*a = 3;

	void *p = a;				//OK

	int *b = p;					//OK

	printf("a:%d\nb:%d\n", *a, *b); //a:3 b:3

	return 0;
}
