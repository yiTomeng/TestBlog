#include <stdio.h>


int main(int argc, char *argv[])
{
	int err = 0;

/*	test 1
	if ( (err = 1) != 0)
	{
		printf("err:%d\n", err);
	}
	
	err = 2;
	if (err != 0)
	{
		printf("err:%d\n", err);
	}
*/

/* test 2
	if ( (err = 1) != 4)
	{
		printf("err:%d\n", err);
	}
	
	err = 2;
	if (err != 4)
	{
		printf("err:%d\n", err);
	}
*/
/* test3
	if ( (err = 4) != 4)
	{
		printf("err:%d\n", err);
	}
	
	err = 4;
	if (err != 4)
	{
		printf("err:%d\n", err);
	}
*/

/*	test 4
	if ( (err = 2) != 0)
	{
		printf("err:%d\n", err);
	}
	
	err = 2;
	if (err != 0)
	{
		printf("err:%d\n", err);
	}
*/

/*	test 5
	if ( (err = 0) != 0)
	{
		printf("err:%d\n", err);
	}
	
	err = 0;
	if (err != 0)
	{
		printf("err:%d\n", err);
	}
*/

/*test 6
if ( (err = 0) == 0)
	{
		printf("err:%d\n", err);
	}
	
	err = 0;
	if (err == 0)
	{
		printf("err:%d\n", err);
	}
*/
	
	/*test 7
	if ( (err = 1) < 0)
	{
		printf("err:%d\n", err);
	}
	
	err = 2;
	if (err < 0)
	{
		printf("err:%d\n", err);
	}
	*/
	
	/*test 8
	if ( (err = 1) >= 0)
	{
		printf("err:%d\n", err);
	}
	
	err = 2;
	if (err >= 0)
	{
		printf("err:%d\n", err);
	}
	*/
	
	return 0;
}