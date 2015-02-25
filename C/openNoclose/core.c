#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void readn()
{
	FILE *fp = NULL;
	int len = 0;
	char buf[BUFSIZ];
	memset(buf, 0x00, BUFSIZ);
	if ( 0 > (fp = fopen("read.txt", "r")) )
	{
		printf("file open error\n");
		exit(1);
	}
	printf("abc\n");

	while(!feof(fp))
	{
		memset(buf, 0x00, BUFSIZ);

		len = fread(buf, 1, 3, fp);
		
		printf("buf:%s\n", buf);
	}
	fclose(fp);					//なければNG

}
int main(int argc, char *argv[])
{

	/*core*/
	#if 0
	char *p = "Hello Core";

	//p[0] = 'a';				//NG segment fault

	printf("%s\n", p);
	#endif

	/*fread asm*/
	#if 0  //問題なし   1
	FILE *fp = NULL;
	int len = 0;
	char buf[BUFSIZ];
	memset(buf, 0x00, BUFSIZ);
	
	if ( 0 > (fp = fopen("read.txt", "r")) )
	{
		printf("file open error\n");
		exit(1);
	}

	while(!feof(fp))
	{
		memset(buf, 0x00, BUFSIZ);

		len = fread(buf, 1, 3, fp);
		
		printf("buf:%s\n", buf);
	}

	fclose(fp);
	#endif
	
	#if 0		//2
	while(1)
	{
		readn();
	}
	#endif
	
	
	//3 問題ない(cygwinで、asianuxで一回やったところ失敗した、何かミスが有るかも)
	FILE *fp = NULL;
	int len = 0;
	char buf[BUFSIZ];
	memset(buf, 0x00, BUFSIZ);
	while(1)
	{
	if ( 0 > (fp = fopen("read.txt", "r")) )
	{
		printf("file open error\n");
		exit(1);
	}
	printf("abc\n");

	while(!feof(fp))
	{
		memset(buf, 0x00, BUFSIZ);

		len = fread(buf, 1, 3, fp);
		
		printf("buf:%s\n", buf);
	}
	fclose(fp);
	}
	
	
	
	return 0;
}
