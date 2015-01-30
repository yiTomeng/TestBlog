#include <stdio.h>
#include <fcntl.h> 

int main(int argc, char *argv[])
{
	int i;

	for(i = 0; i < 101; i = i + 10)
	{
		fflush(stdout);
		printf("%3d%%played", i);
	}

	return 0;
}