#include <stdio.h>
#include <string.h>

void reverse(char *str)
{
	if (str == NULL)
	{
		return;
	}
	
	int len = strlen(str);
	
	if (len > 1)
	{
		char c = str[0];
		
		str[0] = str[len-1];
		
		str[len - 1] = '\0';
		
		reverse(str+1);
		
		str[len - 1] = c;
	}
}

int main(int argc, char* argv[])
{
	char str[BUFSIZ];
	
	memset(str, 0x00, BUFSIZ);
	
	printf("“ü—Í‚µ‚Ä‚­‚¾‚¢F\n");
	int len = -1; 
	while(scanf("%s", str))
	{
		if (strcmp(str, "exit") == 0)
		{
			break;
		}
		printf("str=%s 		", str);
		
		reverse(str);
		
		printf("reverse str=%s\n", str);
	}
	return 0;
}
