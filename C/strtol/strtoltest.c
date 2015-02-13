#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
	char *test_str1 = "123,334";						//Œ‹‰Ê334
	//char *test_str1 = "123,0x334";					//Œ‹‰Ê820
	//char *test_str1 = "123,0334";						//Œ‹‰Ê220
	
	char *test_str3 = (char *)malloc(BUFSIZ);
	
	strncpy(test_str3, test_str1, strlen(test_str1) + 1);
	char *test_str2 = strchr(test_str3, ',');
		
	printf("test_str2:%s\n", test_str2);
	
	*test_str2 = 0;
	
	test_str2++;
	
	printf("test_str2:%s\n", test_str2);
	
	long m = strtol(test_str2, &test_str2, 0);
	
	printf("m: %ld\n", m);
	
	return 0;
}
