/*
 *****************************************************************************
 * MODULE       : DSSL_DIA_003.c
 * FUNCTION     : スケジュール管理用パラメータ読込み
 * VERSION      : 1.00.00
 * REMARKS      :
 * HISTORY      :
 * ID-----------DATE--------NAME----------NOTE -------------------------------
 * [V01.00.00]  2011.3.11   KOCHI    初期作成
 *****************************************************************************
 */
#define     DBG_MODE        1

#include <stdio.h>
#include "quemsg.h"
#include "system.h"
#include "printff.h"
#include "shmop.h"
#include "defmilIDC.h"
#include "SocketCommon.h"
#include "printff.h"
#include "linter.h"
#include "defmilHK70.h"
#include "defmilF2.h"
#include "DSSL_SOUND_000.h"

extern  TCP_DAT     tcp_snd_dat;                                //送信エリア
extern  TCP_DAT     tcp_rcv_dat;                                //受信エリア


/*********************************************************************
 * NAME         :Tcp_Hed_datmk
 * FUNCTION     :TCDのヘッダを作成
 *              :
 * INPUT        :
 * RETURN       :
 * PROGRAMMED   :
 * DATE(ORG)    : 2002.11.20
 * CALL         : なし
 * FILE         : なし
 * REMARKS      : 備考
 ********************************************************************/
void    Tcp_Hed_datmk(unsigned long tcd,unsigned long size,TCP_DAT *tcp)
{
    memcpy(&tcp->Head.Id[0],"NS",2);                            //識別子("NS")
    tcp->Head.Ver = 0x100;                                      //バージョン番号=0x0100
    tcp->Head.Size = size;                                      //データ部サイズ
    tcp->Head.Tcd = tcd;                                        //コマンド番号
    tcp->Head.ProcID = shm_sys.sys_tbl->ProcID;                 //プロセスＩＤ
    gt_time_now(&tcp->Head.SndTim[0]);                          //現在時刻を取得
    tcp->Head.SndNo = shm_sys.sys_tbl->SndNo;                   //送信プロセスメッセージ通番号
}
/*********************************************************************
* NAME         : sch_chgtim_load
* FUNCTION     : 日替わり時刻読込み
*              :
* INPUT        :
* RETURN       :
* PROGRAMMED   :
* DATE(ORG)    : 2008.01.19
* CALL         : なし
* FILE         : なし
* REMARKS      : 備考
********************************************************************/
void sch_chgtim_load(char *pConn){
    ULONG Rowcnt;
    SlctResult strResult[MAX_RESULTNUM];
    char Query[MAX_SQL_LEN];

    sprintf(Query,"select PAR9, PAR15 from TRST003 where PAR_KBN = \'SY\';\0\0");
    Rowcnt=GetData(pConn, (unsigned char*)Query, strResult );

    if( Rowcnt <= 0 ) {
        printX(TSK_SOUND,LOG_YES|ERR_YES|PRT_YES,"***Not Match Record (sql = %s)\n",Query);
        Tshm->DayChgDat.ChangeTime      = 4;
        Tshm->DayChgDat.DayChgProcTime  = 2750;
    }else{
        Tshm->DayChgDat.ChangeTime      = atoi( strResult[0].Column[0]);            //日替わり時刻
        Tshm->DayChgDat.DayChgProcTime  = atoi( strResult[0].Column[1]);            //日替わり処理実行時刻
    }

    printX(TSK_SOUND, LOG_YES, "[%d]日替わり時刻 = %d\n",
        __FUNCTION__, Tshm->DayChgDat.ChangeTime);

    return;
}

/*********************************************************************
 * NAME         :sch_param_read
 * FUNCTION     :スケジュール管理用パラメータ読込み
 *              :
 * INPUT        :
 * RETURN       :
 * PROGRAMMED   :
 * DATE(ORG)    : 2004.10.06
 * CALL         : なし
 * FILE         : なし
 * REMARKS      : 備考
 ********************************************************************/
int sch_param_read(char *pConn){
    int rc;
    sch_chgtim_load(pConn);                 //日替わり時刻読込み
    return OK;
}

