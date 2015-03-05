#include <stdio.h>

int main(int argc, char *argv[])
{
	int a = 0;

	float b = 5.3;
	float c = 5.6;
	float d = -5.3;
	float e = -5.6;

	scanf("%f", &a);

	printf("%x\n", a);

	printf("(int)b:%d\n", (int)b);
	printf("(int)c:%d\n", (int)c);
	printf("(int)d:%d\n", (int)d);
	printf("(int)e:%d\n", (int)e);

	printf("b:%d\n", b);
	printf("c:%d\n", c);
	printf("d:%d\n", d);
	printf("e:%d\n", e);

	return 0;
}
