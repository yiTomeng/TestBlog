#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	int *a = (int*)malloc(sizeof(int));

	*a = 3;

	void *p = a;					//OK

	//int *b = p;					//NG
	int *b = static_cast<int*>(p);  //OK

	printf("a:%d\nb:%d\n", *a, *b);

	return 0;
}
