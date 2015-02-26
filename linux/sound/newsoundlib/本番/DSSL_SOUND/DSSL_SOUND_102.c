/*
 *****************************************************************************
 * MODULE       : DSSL_DIA_102.c
 * FUNCTION     : スケジュール管理初期化処理
 * VERSION      : 1.00.00
 * REMARKS      :
 * HISTORY      :
 * ID-----------DATE--------NAME----------NOTE -------------------------------
 * [V01.00.00]  2011.3.11   KOCHI    初期作成
 *****************************************************************************
 */
#define     DBG_MODE        0                   /* YES=デバック中 NO=リリース */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/shm.h>
#include "defmilIDC.h"
#include "quemsg.h"
#include "system.h"
#include "SocketCommon.h"
#include "defmil30.h"
#include "shmop.h"
#include "DSSL_SOUND_000.h"
#include "printff.h"
#include "linter.h"

int                     TshmID;                 /* 共有メモリＩＤ */
FmtTshm                 *Tshm;                  /* 共有メモリアドレス */
int                     SchCheck_Stop;          /* プロセス終了イベント */

extern  void    SigTermSchCheck();
extern  int     sch_param_read();
/*********************************************************************
 * NAME         : sch_init
 * FUNCTION     : スケジュール管理初期化
 *              :
 * INPUT        :
 * RETURN       :
 * PROGRAMMED   : NSEC.shiba
 * DATE(ORG)    : 2004.05.20
 * CALL         : なし
 * FILE         : なし
 * REMARKS      : 備考
 ********************************************************************/
int     sch_init(char * pConn)
{
    int         rc;

    SchCheck_Stop = 0;

    rc = Sch_MemGet();              //スケジュールデータ共有メモリの取得
    if ( rc != 0 ){
        return(-1);
    }

    rc = sch_param_read(pConn);
    if ( rc == NG){
        printX(TSK_SOUND,LOG_YES|ERR_YES|PRT_YES,"***パラメータ読込み異常\n");
        return(-1);
    }

    rc = sch_proc_start_sch();      //スケジュール管理チェックプロセス起動
    if ( rc != 0 ){
        return(-1);
    }
    return(0);
}

/*********************************************************************
 * NAME         : sch_check_proc_sch
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
int sch_check_proc_sch()
{
    char *pConn_child;

    // DBとの接続を行い
    pConn_child = OpenDB();
    if ( pConn_child == NULL)
    {
        printX( mypno(), PRT_YES, "[%s:%d:%s()] OpenDB error0\n", __FILE__, __LINE__, __FUNCTION__ );
        return( -1);
    }

    for( ; ; ){
        if ( (shm_sys.sys_tbl->end_Flg != 0) ||                         //終了イベントあり？
            ( SchCheck_Stop != 0 ) ){                                   //終了イベントあり？
            printX(TSK_SOUND,LOG_YES|PRT_YES,"***sch_check_proc_sch END !! \n");
            break;
        }

        sleep(5);
        sch_daychg_check(pConn_child);                      //日替わりチェック
    }

    CloseDB(pConn_child);

    return(0);
}

/*********************************************************************
 * NAME         : sch_proc_start_sch
 * FUNCTION     : スケジュール管理チェックプロセス起動
 *              :
 * INPUT        :
 * RETURN       :
 * PROGRAMMED   : NSEC.shiba
 * DATE(ORG)    : 2002.10.20
 * CALL         : なし
 * FILE         : なし
 * REMARKS      : 備考
 ********************************************************************/
int     sch_proc_start_sch()
{
    int         pid;

    pid = fork();                                               //子プロセス生成
    if ( pid < 0 ){                                             //子プロセス失敗
        printX(TSK_SOUND,LOG_YES|ERR_YES|PRT_YES,"***sch_proc_start_sch Create fork NG\n");
        return(-1);
    }
    if ( pid == 0 ){                                                    //子プロセス生成
        if ( signal(SIGTERM,(void *)SigTermSchCheck) == SIG_ERR ){      //ソフトウェア終了シグナルハンドラの設定
            exit(1);
        }
        printX(TSK_SOUND,LOG_YES|PRT_YES,"sch_proc_start_sch Create fork OK\n");
        sch_check_proc_sch();                                           //スケジュール管理チェック処理
        printX(TSK_SOUND,LOG_YES|PRT_YES,"sch_proc_start_sch Create fork END\n");
        exit(1);                                                        //プロセス終了
    }
    return(0);

}

/*********************************************************************
 * NAME         : Sch_MemGet
 * FUNCTION     : スケジュール管理で使用する共有メモリを取得
 *              :
 * INPUT        :
 * RETURN       :
 * PROGRAMMED   : NSEC.shiba
 * DATE(ORG)    : 2002.10.20
 * CALL         : なし
 * FILE         : なし
 * REMARKS      : 備考
 ********************************************************************/
int     Sch_MemGet()
{
    int         rc, Size;

    rc = 0;

    Size = sizeof(FmtTshm);
    TshmID = shmget(IPC_PRIVATE, Size, IPC_CREAT|0666);
    if (TshmID == -1){
        rc = 1;
        printX(TSK_SOUND,LOG_YES|ERR_YES|PRT_YES,"***Sch_MemGet NG\n");
    }

    Tshm = (FmtTshm*)shmat(TshmID, 0, 0);
    if (Tshm == (FmtTshm*)-1){
        rc = 1;
        printX(TSK_SOUND,LOG_YES|ERR_YES|PRT_YES,"***Sch_MemGet NG\n");
    }
    memset(Tshm, 0x00, Size);
    printX(TSK_SOUND,LOG_YES|PRT_YES,"Sch_MemGet OK\n");

    return rc;
}

void SigTermSchCheck()
{
    SchCheck_Stop   = 1;                                            /* プロセス終了イベント */
}

/*********************************************************************
 * NAME         : sch_end
 * FUNCTION     : スケジュール管理で使用するデータ終了
 *              :
 * INPUT        :
 * RETURN       :
 * PROGRAMMED   : NSEC.shiba
 * DATE(ORG)    : 2004.05.20
 * CALL         : なし
 * FILE         : なし
 * REMARKS      : 備考
 ********************************************************************/
int     sch_end()
{
    int         rc;

    rc = sch_MemDel();                                          //スケジュール管理で使用するデータを開放
    if ( rc != 0 ){
        return(-1);
    }

    return(0);
}

/*********************************************************************
 * NAME         : sch_MemDel
 * FUNCTION     : スケジュール管理で使用するデータ削除
 *              :
 * INPUT        :
 * RETURN       :
 * PROGRAMMED   : NSEC.shiba
 * DATE(ORG)    : 2004.05.20
 * CALL         : なし
 * FILE         : なし
 * REMARKS      : 備考
 ********************************************************************/
int     sch_MemDel()
{
    int     rc;

    rc = 0;
    if (Tshm != 0){
        shm_detach((char *)Tshm);
    }
    if (TshmID != 0){
        if(shmctl(TshmID, IPC_RMID, 0) < 0){                        //共有メモリの解放
            rc = 1;
            printX(TSK_SOUND,ERR_YES|PRT_YES,"*** Sch_MemDel NG\n");
        }
    }
    printX(TSK_SOUND,PRT_YES,"Sch_MemDel OK\n");

    return(rc);
}
