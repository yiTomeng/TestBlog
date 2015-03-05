#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/shm.h>

int main(int argc, char *argv[])
{
	//char *m = (char *)calloc(1, BUFSIZ);			//自分持つ
	char *m;
	int shm_key = 0;
	int shm_id = 0;

	shm_id = shmget(shm_key, BUFSIZ, IPC_CREAT | 0666);

	m = (char *)shmat(shm_id, 0, 0);

	pid_t pid ;
//memset(m, 0x00, BUFSIZ);
	strcpy(m, "father");

	pid = fork();

	if (pid < 0)
	{
		printf("fork is unsuccessful\n");
		return -1;
	}
	else if (pid == 0)
	{
		while(1)
		{
			printf("%s in child\n", m);
			memset(m, 0x00, BUFSIZ);
			strcpy(m, "child");
			printf("%s in child\n", m);
			sleep(3);	
		}
		
	}

	while(1)
	{
		memset(m, 0x00, BUFSIZ);
		strcpy(m, "father");
		while(1)
		{
			printf("%s in father\n", m);
			sleep(3);	
		}
		
	}

	return 0;
}
