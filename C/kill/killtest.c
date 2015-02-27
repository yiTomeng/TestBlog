#include <stdio.h>
#include <signal.h>

void SigTerm(void);
void SigKill(void);

int main(int argc, char *argv[])
{
	
	if ( signal(SIGTERM,(void *)SigTerm) == SIG_ERR ){          //ソフトウェア終了シグナルハンドラの設定
		printf("sigterm\n");
        return(-1);
    }

    #if 0				//コメントアウトしないと、ifに入る
    if ( signal(SIGKILL,(void *)SigKill) == SIG_ERR ){          //ソフトウェア終了シグナルハンドラの設定
    	printf("sigkill\n");
        return(-2);
    }
    #endif
    int a = 0;
    while(1)
    {
    	a++;
    	sleep(1);
    }
	return 0;
}

void SigTerm(void)
{
	printf("SigTerm\n");
	return;
}

void SigKill(void)
{
	printf("SigKill\n");
	return;
}
