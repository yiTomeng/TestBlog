/*
 *****************************************************************************
 * MODULE       : DSSL_DIA_101.c
 * FUNCTION     : TCP送受信処理
 * VERSION      : 1.00.00
 * REMARKS      :
 * HISTORY      :
 * ID-----------DATE--------NAME----------NOTE -------------------------------
 * [V01.00.00]	2011.3.11	KOCHI    初期作成
 *****************************************************************************
 */
#define		DBG_MODE		0					/* 1=デバック中 0=リリース */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/time.h>
#include "quemsg.h"
#include "system.h"
#include "defmilIDC.h"
#include "SocketCommon.h"
#include "shmop.h"
#include "printff.h"

extern	void SigTerm();
/*********************************************************************
 * NAME         : create_proc
 * FUNCTION     : プロセス作成
 *              :
 * INPUT        :
 * RETURN       :
 * PROGRAMMED   : NSEC.shiba
 * DATE(ORG)    : 2002.10.20
 * CALL         : なし
 * FILE         : なし
 * REMARKS      : 備考
 ********************************************************************/
int	create_CL_proc(int new_Sock)
{
	int			rc,pos,key;
	int			spid,rpid;
	char		ipadr[32];
	char		*radr,*sadr;

	pos = CLproc_Noget();
	if ( pos < 0 ){
		printX(TSK_SOUND,LOG_YES|ERR_YES|PRT_YES,"***CLproc_Noget NG max=%d\n",shm_sys.sys_tbl->tsk_max);
		return(0);
	}
	CLproc_tbl_setA(pos,new_Sock);

	rpid = fork();												//子プロセス生成
	if ( rpid < 0 ){											//子プロセス失敗
		printX(TSK_SOUND,LOG_YES|ERR_YES|PRT_YES,"***RCV Create fork NG\n");
		return(-1);
	}
	if ( rpid == 0 ){											//受信用子プロセス生成
		if ( signal(SIGTERM,(void *)SigTerm) == SIG_ERR ){		//ソフトウェア終了シグナルハンドラの設定
			exit(1);
		}
		printX(TSK_SOUND,LOG_YES|PRT_YES,"RCV Create fork OK(socket=%d,pos=%d)\n",new_Sock,pos);
		socket_Rcv_proc(new_Sock,pos);							//受信処理
		rc = close(new_Sock);
		printX(TSK_SOUND,LOG_YES|PRT_YES,"RCV Create fork END(socket=%d,pos=%d,close=%d)\n",new_Sock,pos,rc);
		exit(1);												//プロセス終了
	}
	spid = fork();												//子プロセス生成
	if ( spid < 0 ){											//子プロセス失敗
		printX(TSK_SOUND,LOG_YES|ERR_YES|PRT_YES,"***SND Create fork NG\n");
		return(-1);
	}
	if ( spid == 0 ){											//送信用子プロセス生成
		if ( signal(SIGTERM,(void *)SigTerm) == SIG_ERR ){		//ソフトウェア終了シグナルハンドラの設定
			exit(1);
		}
		printX(TSK_SOUND,LOG_YES|PRT_YES,"SND Create fork OK(socket=%d,pos=%d)\n",new_Sock,pos);
		socket_Snd_proc(new_Sock,pos);							//送信処理
		rc = close(new_Sock);
		printX(TSK_SOUND,LOG_YES|PRT_YES,"SND Create fork END(socket=%d,pos=%d,close=%d)\n",new_Sock,pos,rc);
		exit(1);												//プロセス終了
	}
	CLproc_tbl_setB(pos,rpid,spid);
	return(0);
}

/*********************************************************************
 * NAME         : CLproc_tbl_set
 * FUNCTION     : プロセス管理テーブルの更新
 *              :
 * INPUT        :
 * RETURN       :
 * PROGRAMMED   : NSEC.shiba
 * DATE(ORG)    : 2002.10.20
 * CALL         : なし
 * FILE         : なし
 * REMARKS      : 備考
 ********************************************************************/
int	CLproc_tbl_setA(int pos,int new_Sock)
{
	int			rc;
	PROC_TBL	*proc_p;
	char		ipadr[32];

	proc_p=&shm_sys.CLproc_sys->proc[pos];
	proc_p->use		= TRUE;										//当該インデックス使用状態 TRUE=使用中
	proc_p->ac_sock	= new_Sock;									//ccept済みのSocket
	shm_sys.sys_tbl->tsk_count++;								//接続プロセス数更新
	return(0);
}

int	CLproc_tbl_setB(int pos,int rpid,int spid)
{
	PROC_TBL	*proc_p;

	proc_p=&shm_sys.CLproc_sys->proc[pos];
	proc_p->rcvpid		= rpid;									//受信プロセスID
	proc_p->sndpid		= spid;									//送信プロセスID
	return(0);
}

int	CLproc_Noget()
{
	int			rc,cnt;
	PROC_TBL	*proc_p;

	rc = -1;
	proc_p=&shm_sys.CLproc_sys->proc[0];
	for(cnt=0;cnt<shm_sys.sys_tbl->tsk_max;cnt++){
		if ( proc_p->use == FALSE ){
			rc = cnt;
			break;
		}
		proc_p++;
	}
	return(rc);
}
