/*
 *****************************************************************************
 * MODULE       : DSSL_DIA_100.c
 * FUNCTION     : rcvdatのメイン
 * VERSION      : 1.00.00
 * REMARKS      :
 * HISTORY      :
 * ID-----------DATE--------NAME----------NOTE -------------------------------
 * [V01.00.00]	2011.3.11	KOCHI    初期作成
 *****************************************************************************
 */
#define		DBG_MODE		0					/* YES=デバック中 NO=リリース */

#include <stdio.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/time.h>
#include "defmilIDC.h"
#include "defmilF0.h"
#include "defmilF1.h"
#include "defmilF2.h"
#include "quemsg.h"
#include "system.h"
#include "DSSL_SOUND_000.h"
#include "SocketCommon.h"
#include "shmop.h"
#include "printff.h"
#include "defmil12.h"
#include "defmil30.h"
#include "defmil31.h"
#include "defmilHK70.h"

extern	TCP_DAT		tcp_snd_dat;								//送信エリア
extern	TCP_DAT		tcp_rcv_dat;								//受信エリア


static int		flg_NotSend = 0;

int snd_f290(unsigned long TaskKey);
static int tcd_f291(int pos, TCP_DAT * tcp);


/*********************************************************************
 * NAME         : XXX_main
 * FUNCTION     : 受信キュー読込処理
 *              :
 * INPUT        :
 * RETURN       :
 * PROGRAMMED   : 
 * DATE(ORG)    : 2002.10.20
 * CALL         : なし
 * FILE         : なし
 * REMARKS      : 備考
 ********************************************************************/
int	tcd_main_rcv()
{
	int			rc,pos;
	PROC_TBL	*proc_p;

	proc_p=&shm_sys.CLproc_sys->proc[0];
	for(pos=0;pos<shm_sys.sys_tbl->tsk_max;pos++){
		if ( proc_p->use == TRUE){								//当該インデックス使用状態 TRUE=使用中
			for( ; ; ){
				rc = QueMsgGet((QINDEX_*)&shm_sys.que_sys->index[pos][SOCKET_RCV],(unsigned char*)&tcp_rcv_dat);
				if ( rc == QUE_EMPTY ){							//キュー登録データ無し
					break;
				}

				tcd_rcv_chk(pos,&tcp_rcv_dat);
			}
		}
		proc_p++;
	}
	return(0);
}


/*********************************************************************
 * NAME         : tcd_rcv_chk
 * FUNCTION     : 受信解析処理
 *              :
 * INPUT        :
 * RETURN       :
 * PROGRAMMED   : 
 * DATE(ORG)    : 2002.10.20
 * MODIFIED     : ELWSC.TYOU
 * DATE         : 2007.07.05
 * CALL         : なし
 * FILE         : なし
 * REMARKS      : 備考
 ********************************************************************/
int	tcd_rcv_chk(int pos,TCP_DAT *tcp)
{
	int rc = 0;
	
	
	if (flg_NotSend == 0) {

	switch(tcp->Head.Tcd&(TCDmsk_Mcod|TCDmsk_Scod)){			// TCD番号
	case	TCDF091:											// TCD登録アンサー
		tcd_f091(pos,tcp);
		break;

	case	TCDF110:											// プロセス制御通知
		tcd_f110(pos,tcp);
		break;

	case	TCDF102:											// システム時刻通知
		tcd_f102(pos,tcp);
		break;

	case TCDF291:												// 動作開始許可
		rc = tcd_f291(pos, tcp);
		break;

	default:
		break;
	}
	
	} else {
		switch (tcp->Head.Tcd&(TCDmsk_Mcod|TCDmsk_Scod)) {			//TCD番号
		  case TCDF091:									// TCD登録アンサー
			tcd_f091(pos,tcp);
			break;
		  case TCDF110:									// プロセス制御通知
			rc = tcd_f110(pos, tcp);
			break;
		  case TCDF291:									// 動作開始許可
			rc = tcd_f291(pos, tcp);
			break;
		  default:
			break;
		}
	}
	
}

/*********************************************************************
 * NAME         : tcd_f091
 * FUNCTION     : TCD登録アンサー処理
 *              :
 * INPUT        :
 * RETURN       :
 * PROGRAMMED   : 
 * DATE(ORG)    : 2002.11.20
 * MODIFIED     : ELWSC.TYOU
 * DATE         : 2007.07.05
 * CALL         : なし
 * FILE         : なし
 * REMARKS      : 備考
 ********************************************************************/
int	tcd_f091(int pos,TCP_DAT *tcp)
{
	FmtTCDF091	*Ansp;

	Ansp = (FmtTCDF091*)tcp;									//プロセスID保存
	shm_sys.sys_tbl->ProcID = Ansp->Tcd_Ans.ProcID;
	printX(TSK_SOUND,LOG_YES|PRT_YES,"TCD RCV F091(ProcID=%08X)\n",
							shm_sys.sys_tbl->ProcID);
	
	snd_f290(TSK_SOUND);
	
}

// 2007.01.16 add start 北総では、中央からの時刻を受ける必要がある？為、追加
/*********************************************************************
 * NAME         : tcd_f102
 * FUNCTION     : システム時刻通知
 *              :
 * INPUT        :
 * RETURN       :
 * PROGRAMMED   : ELWSC
 * DATE(ORG)    : 2007.01.12
 * CALL         : なし
 * FILE         : なし
 * REMARKS      : 備考
 ********************************************************************/
int	tcd_f102(int pos,TCP_DAT *tcp)
{
	FmtTCDHKF102			*Rcv_p;


	Rcv_p = (FmtTCDHKF102 *)tcp;						//受信エリアポインタ設定
	printX(TSK_SOUND,LOG_NO|PRT_YES, "TCD RCV SYSTEM TIME NOTICE[TCD=%08X]\n", Rcv_p->Head.Tcd);

	
	if (Rcv_p->PcReset)//PC 時間再設定フラグ
	{
		FmtDayTim Now;
		GetNowOver24H(&Now);

		//システム時刻の何月日が変更された場合
		if( Now.Year != Tshm->Now_Day.Year || Now.Month != Tshm->Now_Day.Month || Now.Day != Tshm->Now_Day.Day ){
			printX(TSK_SOUND, LOG_YES, "(tcd_f102)駅側年月日が変更された場合、スケジュール時間再読み込む[Now:%04d/%02d/%02d Old:%04d/%02d/%02d]\n",
											Now.Year, Now.Month,  Now.Day, 
											Tshm->Now_Day.Year, Tshm->Now_Day.Month, Tshm->Now_Day.Day);
			GetNowOver24H( &Tshm->Now_Day );
			
		}
	}


	return 0;
}




/*********************************************************************
 * NAME         : tcd_f291
 * FUNCTION     : 動作開始許可受信処理
 *              :
 * INPUT        :
 * RETURN       :
 * PROGRAMMED   : ELWSC.TYOU
 * DATE(ORG)    : 2007.07.05
 * CALL         : なし
 * FILE         : なし
 * REMARKS      : 備考
 ********************************************************************/
int	tcd_f291(int pos, TCP_DAT *tcp)
{
	int				rc;
	FmtTCDF291	  * Rcv_p;
	unsigned long	TaskKey;

	rc = 0;
	Rcv_p = (FmtTCDF291*)tcp;
	TaskKey = Rcv_p->TaskKey;
	if (TaskKey != TSK_SOUND) {
		return rc;
	}
	flg_NotSend = 0;
	printX(TSK_SOUND, LOG_YES|PRT_YES, "TCD RCV F291 TaskKey=%d\n", TaskKey);
	return rc;
}

/*********************************************************************
 * NAME         :snd_f290
 * FUNCTION     :タスク動作開始許可
 *              :
 * INPUT        :
 * RETURN       :
 * PROGRAMMED   : ELWSC.TYOU
 * DATE(ORG)    : 2007.07.05
 * CALL         : なし
 * FILE         : なし
 * REMARKS      : 備考
 ********************************************************************/
int snd_f290(unsigned long TaskKey)
{
	int				rc;
	FmtTCDF290	  * Snd_p;
	unsigned long	Tcd;
	int				size;
	int				ret;

	Snd_p = (FmtTCDF290*)&tcp_snd_dat;							//送信エリアポインタ設定

	memset(&tcp_snd_dat, 0x00, sizeof(tcp_snd_dat));			//送信エリアクリア
	Tcd = TCDF290;												//TCD
	size = sizeof(FmtTCDF290)-sizeof(FmtIDCHead);
	Tcp_Hed_datmk(Tcd, size, &tcp_snd_dat);						//TCPヘッダ作成

	Snd_p->TaskKey = TaskKey;

	ret = QueMsgSet((QINDEX_*)&shm_sys.que_sys->index[0][SOCKET_SND], (unsigned char*)&tcp_snd_dat);
	if (ret == QUE_FULL) {										//キュー書込失敗
		rc = 1;
		printX(TSK_SOUND,LOG_YES|ERR_YES|PRT_YES,"***TCD SND NG(QUE_FULL TCD=%08X)\n",Tcd);
	} else {
		rc = 0;
		printX(TSK_SOUND,LOG_YES|ERR_YES|PRT_YES,"TCD SND F290 OK(TCD=%08X) TaskKey=%d\n",Tcd,TaskKey);
	}
	return rc;
}

/*********************************************************************
 * NAME         : tcdf111_snd
 * FUNCTION     : プロセス状態通知
 *              :
 * INPUT        :
 * RETURN       :
 * PROGRAMMED   : NSEC.shiba
 * DATE(ORG)    : 2002.12.12
 * CALL         : なし
 * FILE         : なし
 * REMARKS      : 備考
 ********************************************************************/
int	tcdf111_snd(unsigned char proc_id)
{
	int				rc,size;
	unsigned long	Tcd;
	FmtTCDF111		*Snd_p;

	Snd_p = (FmtTCDF111*)&tcp_snd_dat;							//送信エリアポインタ設定

	memset(&tcp_snd_dat,0x00,sizeof(tcp_snd_dat));				//送信エリアクリア
	Tcd		= TCDF111 | TCDF111_SelProc | proc_id;				//TCD
	size	= sizeof(FmtTCDF111)-sizeof(FmtIDCHead);
	Tcp_Hed_datmk(Tcd,size,&tcp_snd_dat);						//TCPヘッダ作成
	Snd_p->STS.Com = TCDF111_ProcSts;							//コマンド

	rc = QueMsgSet((QINDEX_*)&shm_sys.que_sys->index[0][SOCKET_SND],(unsigned char*)&tcp_snd_dat);
	if ( rc == QUE_FULL ){										//キュー書込失敗
		rc = 1;
		printX(TSK_SOUND,LOG_YES|ERR_YES|PRT_YES,"***TCD SND NG(QUE_FULL TCD=%08X)\n",Tcd);
	} else {
		rc = 0;
		printX(TSK_SOUND,LOG_YES|PRT_YES,"TCD SND F111 OK(TCD=%08X)\n",Tcd);
	}
	return(rc);
}

/*********************************************************************
 * NAME         :tcd_f110
 * FUNCTION     :プロセス制御通知
 *              :
 * INPUT        :
 * RETURN       :
 * PROGRAMMED   :
 * DATE(ORG)    : 2002.11.20
 * CALL         : なし
 * FILE         : なし
 * REMARKS      : 備考
 ********************************************************************/
int	tcd_f110(int pos,TCP_DAT *tcp)
{
	int				rc,size,cnt;
	int				CtlGrp;										//表示管理データ送信系統番号
	int				Dia_no;
	int				Ban_no;
	FmtTCDF110		*Rcv_p;
	FmtTCDF111		*Snd_p;
	unsigned long	Tcd,Acode,Bcode,Com;

	Snd_p = (FmtTCDF111*)&tcp_snd_dat;							//送信エリアポインタ設定
	Rcv_p = (FmtTCDF110*)tcp;									//受信エリアポインタ設定
	Acode = Rcv_p->Head.Tcd & TCDmsk_Acod;						/* ＴＣＤユーザＡコード */
	Bcode = Rcv_p->Head.Tcd & TCDmsk_Bcod;						/* ＴＣＤユーザＢコード */
	Com   = Rcv_p->Com;
	//printX(TSK_SOUND,LOG_YES|PRT_YES,"TCD RCV F110(Acode=%X,Bcode=%X,Com=%X)\n",Acode,Bcode,Com,shm_sys.sys_tbl->key);

	rc = -1;
	switch(Acode){												//ＴＣＤユーザＡコード
	case	TCDF110_SelProc:									/* 系統選択　：ユーザコードＢ：系統番号 */
		if ( Bcode == shm_sys.sys_tbl->key ){					//起動時のキー番号
			rc = 0;
		}
		break;
	case	TCDF110_SelAll:
		rc = 0;
		break;
	default:
		printX(TSK_SOUND,LOG_YES|ERR_YES|PRT_YES,"***TCD RCV F110 A-CODE NG(Acode=%d) \n",Acode);
		break;
	}

	if ( rc == 0 ){												//指定パラメータＯＫ
		if ( Com == TCDF110_ProcSts ){
			printX(TSK_SOUND,LOG_YES|PRT_YES,"TCD RCV F110(Acode=%X,Bcode=%X,Com=%X)\n",Acode,Bcode,Com,shm_sys.sys_tbl->key);
			tcdf111_snd(shm_sys.sys_tbl->key);					//プロセス状態通知
		}
		if ( Com == TCDF110_ProcEnd ){
			printX(TSK_SOUND,LOG_YES|PRT_YES,"***TCD RCV PROG END***\n");
			sleep(1);
			shm_sys.sys_tbl->end_Flg = 1;						//終了イベント設定
		}
	}
	return(rc);
}










