#include <stdio.h>

int main(int argc, char *argv)
{
	int a = 2;
	typeof(a) b = (a == 2) ? : 4;
	int c = (a != 2) ? : 5;
	
	
	printf(" b: %d\n c: %d\n", b, c);

	return 0;
}
