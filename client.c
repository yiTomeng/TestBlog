
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


int main(int argc, char* argv[])
{
	int socket_flg;
	int client_sock;
	int rpid, spid;
    int len = 0;
	char send_buf[512], recv_buf[512];

	memset(send_buf, 0x00, 512);
	memset(recv_buf, 0x00, 512);
	struct  sockaddr_in     sock_name;                               //サーバーソケット名
	struct  linger          lin;
	char    ip[32] = "127.0.0.1";
	int 	port = 6524;
	fd_set  readfds,writefds,exceptfds;
	struct  timeval     tim;

	socket_flg = socket(AF_INET, SOCK_STREAM, 0);               //通信端点の作成

	int opt = 1;

	int status = setsockopt(socket_flg, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt));

	lin.l_onoff = 1;
	lin.l_linger = 0;

	status = setsockopt(socket_flg, SOL_SOCKET, SO_LINGER, (char*)&lin, sizeof(struct linger));

/*
	memset(&sock_name,0x00,sizeof(SockName));
    sock_name.sin_family = AF_INET;
    sock_name.sin_port   = htons(ip);                       	 //INIファイル公開ポートを使用
    sock_name.sin_addr.s_addr = inet_addr(port);                 //INIファイル公開IP


    status = bind(socket_flg, (struct sockaddr*)&sock_name, sizeof(sock_name));

    listen(socket_flg, 1);
*/

    memset(&sock_name,0x00,sizeof(sock_name));
    sock_name.sin_family = AF_INET;
    sock_name.sin_port   = htons(port);            //INIファイル接続ポートを使用
    sock_name.sin_addr.s_addr = inet_addr(ip);     //INIファイル接続IP

  
    status = connect(socket_flg, (struct sockaddr*)&sock_name, sizeof(sock_name));

    if (status < 0)
    {
        printf("connect error\n");
        return 0;
    }

    spid = fork();
    if (spid < 0)
    {
        printf("pid error\n");
    }

    else if (0 == spid)
    {
        while(1)
        {
            strcpy(send_buf, "クライアント：サーバーに送信");
            send(socket_flg, send_buf, strlen(send_buf) + 1, 0);
            printf("%s\n", send_buf);
            sleep(1);
        }
    }
#if 0
    tim.tv_sec  = 1;                                            //wait時間 単位＝秒
    tim.tv_usec = 0;                                            //wait時間 単位＝マイクロ秒
    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    FD_ZERO(&exceptfds);
    FD_SET(socket_flg,&readfds);

    if ( select(FD_SETSIZE,&readfds,&writefds,&exceptfds,&tim) )
    {
        if ( FD_ISSET(socket_flg,&readfds) )
        {               //接続受付
            len = sizeof(sock_name);
           
            rpid = fork();

            if (rpid < 0)
            {
            	printf("pid error\n");
            }

            else if (0 == rpid)
            {
            	while(1)
            	{
            		recv(socket_flg, recv_buf, 512, 0);
                    sleep(5);
            		printf("%s\n", recv_buf);
            	}
				            	
            }

/*
            spid = fork();
            if (spid < 0)
            {
            	printf("pid error\n");
            }

            else if (0 == spid)
            {
            	while(1)
            	{
            		strcpy(send_buf, "クライアント：サーバーに送信");
            		send(socket_flg, send_buf, strlen(send_buf) + 1, 0);
            		sleep(1);
            	}
            }
            */
        }

    }
#endif
    return 0;
}