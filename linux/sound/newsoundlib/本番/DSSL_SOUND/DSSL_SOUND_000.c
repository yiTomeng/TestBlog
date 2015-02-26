/*
 *****************************************************************************
 * MODULE       : DSSL_DIA_000.c
 * FUNCTION     : 上位データ受信タスクのメイン
 * VERSION      : 1.00.00
 * REMARKS      :
 * HISTORY      :
 * ID-----------DATE--------NAME----------NOTE -------------------------------
 * [V01.00.00]  2011.3.11   KOCHI    初期作成
 *****************************************************************************
 */
#define     DBG_MODE        0                   /* YES=デバック中 NO=リリース */

#include <stdio.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "quemsg.h"
#include "system.h"
#define     MAIN_PROGRAM
#include "defmilIDC.h"
#include "defmil30.h"
#include "DSSL_SOUND_000.h"
#include "SocketCommon.h"
#include "shmop.h"
#include "printff.h"
#include "linter.h"

#define TEST
#ifdef TEST
#include "lib/sound_lib.h"
#endif

TCD_TBL     Mtcd_tbl[CLIENT_MAX];                               //自装置TCD振り分けテーブル
TCP_DAT     tcp_snd_dat;                                        //送信エリア
TCP_DAT     tcp_rcv_dat;                                        //受信エリア
TCP_DAT     tcp_log_dat;                                        //ログ送信エリア

char *pConn = NULL;

unsigned long   SeqNo;                                          //シーケンス番号

extern  TCD_DAT     Txxx_tcd[];
extern  int     tsk_init();                                     //初期化処理
extern  int     tsk_main();                                     //メイン処理
extern  int     tsk_end();                                      //終了処理
extern  int     GetPrivatefile(T_INIFILE *IniF,char *fp);       //INIファイル読込
extern  int     GetIniFile(char* FileName,char* ChkMoji,char* setp);
extern  void    SigCld();                                       //子プロセス終了処理
extern  void    SigTerm();                                      //ソフト終了処理
extern  void    SigUsr1();
extern  void    SigTimer();
extern  void    SigInt();


//void    set_sch_info(char *pConn);                                         //当日・翌日スケジュールの作成


extern  int delete_zenjitu_sch(char *pConn);
extern  int check_toujitu_sch(char *pConn);//当日スケジュールが存在するかどうかをチェックする
extern  int check_yokujitu_sch(char *pConn);//翌日スケジュールが存在するかどうかをチェックする
extern  int check_toujitu_stop(char *pConn);//当日停留所情報が存在するかどうかをチェックする
extern  int check_yokujitu_stop(char *pConn);//翌日停留所情報が存在するかどうかをチェックする
extern  int setExclusionFlag(int exclusionFlag, char *pConn);//スケジュールの排他フラグの設定
extern  int set_bin_data(int sch_type, char *pConn);//便データの作成
extern  int set_stop_data(int sch_type, char* binCod, char* gouNo, int mashi, char *pConn);//当日・翌日スケジュールの識別子
extern  int check_kaise_date(int sch_type, char *pConn);
extern  void set_genyo_data(char *pConn);
extern  void delete_genyo_data(char *pConn);


/*********************************************************************
 * NAME         : MAIN
 * FUNCTION     : メイン処理
 *              :
 * INPUT        :
 * RETURN       :
 * PROGRAMMED   : NSEC.shiba
 * DATE(ORG)    : 2002.10.30
 * CALL         : なし
 * FILE         : なし
 * REMARKS      : 備考
 ********************************************************************/
int main(int argc,char *argv[])
{
    int         rc;
    int         key;

    rc = NG;
    if ( argc > 0 ){                                            //起動パラメータチェック
        key = atoi(argv[1]);
        if ((key > 0) && (key < SOCKET_MAX)){                   //範囲チェック
            rc = OK;
        }
    }
    if (rc == NG){
        printX(TSK_SOUND,ERR_YES|PRT_YES,"***KEY NG(1-%d) Key = %d!!\n", SOCKET_MAX, key);
        return(1);
    }

    printX(TSK_SOUND,PRT_YES, "main start!!\n");

    rc = tsk_init(key);                                         //初期化
    if ( rc == NG){
        printX(TSK_SOUND,LOG_YES|ERR_YES|PRT_YES,"***INIT NG \n");
        return(1);
    }
    #ifdef TEST1
    int dsp = 0;
    play_stat play_status;
    play_status.stop_cmd = NOT_STOP_PLAY;
    play_status.status = IS_NOT_RUNNING;
	while(1)
	{
		if ( (dsp == 0) && (ERR_ZERO != open_voice_device(&dsp)))
		{
			continue;
		}
		start_play_wave(&dsp, TSK_SOUND, "/home/Sound/test.wav", 1, &play_status);			//wavファイルを再生する関数
		sleep(1);
	}
	close_voice_device(&dsp);
	#endif
	
	#ifdef TEST
	snd_pcm_t *handler = NULL;
	//open_device(&handler, TSK_SOUND);
	//snd_pcm_t *handler = NULL;
	//int left_vol = 80;
	//int right_vol = 80;
	st_vol vol = {.left_vol = 80, .right_vol = 80};
	int refresh_cnt = 2;
	
	open_device(&handler, TSK_SOUND);
	
	assert(handler != NULL);
	while(1)
	{
		play_file(handler, "/home/Sound/test3.wav", vol, refresh_cnt);
		sleep(3);
		play_file(handler, "/home/Sound/test.wav", vol, refresh_cnt);
		sleep(3);
	}
	close_device(handler);
	
	#endif
    if ( rc == OK ){
        rc = shm_sys_attach(&shm_sys,key);                      //共有メモリ管理テーブル取得
        if ( rc >= 0 ){
            rc = sch_init(pConn);                                    //スケジュール管理初期化処理
            if ( rc >= 0){
                SeqNo = 1;                                      //シーケンスＮＯを１で初期化
//                set_sch_info(pConn);                                 //当日・翌日スケジュールの作成
                tsk_main();                                     //メイン処理
            }
        }
        rc = shm_sys_detach(&shm_sys,key);                      //共有メモリ管理テーブル取得解放
    }
    sch_end();                                                  //スケジュール用共有メモリ解放
    printX(TSK_SOUND,PRT_YES, "main end!!\n");
    tsk_end(key);                                               //終了
}

/*********************************************************************
 * NAME         : tsk_init
 * FUNCTION     : 初期化
 *              :
 * INPUT        :
 * RETURN       :
 * PROGRAMMED   : NSEC.shiba
 * DATE(ORG)    : 2002.10.30
 * CALL         : なし
 * FILE         : なし
 * REMARKS      : 備考
 ********************************************************************/
int tsk_init(int key)
{
    int         rc,cnt;
    T_INIFILE   inifile;                                        //INIファイルエリア
    SYS_TBL     sys_tbl;                                        //システム状態テーブル
    SHM_SYS     sp;

    // プログラムNo設定
    setpno( key );

    if ( GetPrivatefile(&inifile,TSK_INIFILE) != 0 ){           //INIファイル読込
        printX(TSK_SOUND,LOG_YES|ERR_YES|PRT_YES,"***GetPrivatefile = NG\n");
        return(NG);
    }
    printX(TSK_SOUND,LOG_YES|PRT_YES,"GetPrivatefile = OK\n");

    // DBとの接続を行い
    pConn = OpenDB();
    if ( pConn == NULL)
    {
        printX( mypno(), PRT_YES, "[%s:%d:%s()] OpenDB error0\n", __FILE__, __LINE__, __FUNCTION__ );
        return( NG );
    }


    memset(&shm_sys,0x00,sizeof(shm_sys));
    memset(&sys_tbl,0x00,sizeof(sys_tbl));                      //システム状態テーブルクリア
    memcpy(&sys_tbl.mydat,&inifile,sizeof(sys_tbl.mydat));
    sys_tbl.tsk_max = CLIENT_MAX;                               //接続プロセス数MAX
    sys_tbl.pid = getpid();                                     //プロセスIDの取得
    sys_tbl.key = key;                                          //起動時のキー番号
                                                                //このキー番号を使用して共有メモリを作成します
    shm_end(key);                                               //作成済みの共有メモリの削除

    rc = shm_sys_init(key);                                     //システム用共有メモリ作成
    if ( rc < 0 ){
        return(NG);
    }
    rc = shm_sys_attach(&sp,key);                               //共有メモリ管理テーブル取得
    if ( rc < 0 ){
        return(NG);
    }
    memcpy(sp.sys_tbl,&sys_tbl,sizeof(sys_tbl));

    rc = shm_sys_detach(&sp,key);                               //共有メモリ管理テーブル取得解放
    if ( rc < 0 ){
        return(NG);
    }

    if ( signal(SIGTERM,(void *)SigTerm) == SIG_ERR ){          //ソフトウェア終了シグナルハンドラの設定
        return(NG);
    }

    if ( signal(SIGCLD,(void *)SigCld) == SIG_ERR ){            //子プロセス終了シグナルハンドラの設定
        return(NG);
    }

    if ( signal(SIGUSR1,(void *)SigUsr1) == SIG_ERR ){          //子プロセス終了シグナルハンドラの設定
        return(NG);
    }

    if ( signal(SIGALRM,(void *)SigTimer) ){
        return(NG);
    }

    if ( signal(SIGINT,(void *)SigInt) ){
        return(NG);
    }
    return(OK);
}

/*********************************************************************
 * NAME         : tsk_main
 * FUNCTION     : メイン処理
 *              :
 * INPUT        :
 * RETURN       :
 * PROGRAMMED   : NSEC.shiba
 * DATE(ORG)    : 2002.10.30
 * CALL         : なし
 * FILE         : なし
 * REMARKS      : 備考
 ********************************************************************/
int tsk_main()
{
    int     rc,pos,pid,ClientSock;

    for( ; ; ){
        if ( shm_sys.sys_tbl->end_Flg != 0 ){                   //終了イベントあり？
            break;
        }

        ClientSock = socket_Init(&shm_sys.sys_tbl->mydat);      //TCP初期化
        if ( ClientSock < 0 ){
            break;
        }
        shm_sys.sys_tbl->Cld_end_Flg = 0;                       //クライアントsocketプロセス終了イベントあり？

        rc = create_CL_proc(ClientSock);
        if ( rc < 0 ){
            close(ClientSock);
            continue;
        }

        tcd_set(ClientSock);                                    //ＴＣＤ登録送信
        //tcd_1110();
        for( ; ; ){
            if ((shm_sys.sys_tbl->end_Flg != 0) ||              //終了イベントあり？
                (shm_sys.sys_tbl->Cld_end_Flg != 0) ){          //クライアントsocketプロセス終了イベントあり？
                close(ClientSock);
                sleep(1);
                break;
            }
            sleep(5);
            //受信データチェック
            tcd_main_rcv();
        }
    }
    return(0);
}

/*********************************************************************
 * NAME         : tcd_set
 * FUNCTION     : ＴＣＤ登録送信
 *              :
 * INPUT        :
 * RETURN       :
 * PROGRAMMED   : NSEC.shiba
 * DATE(ORG)    : 2002.10.20
 * CALL         : なし
 * FILE         : なし
 * REMARKS      : 備考
 ********************************************************************/
int tcd_set(int ClientSock)
{
    int     rc,Port,Ip,cnt;                                             // 取得ポート
    struct  sockaddr        SockInfo;                               // ソケット情報
    struct  sockaddr_in     *pTbl;                                  // ソケット情報参照用テーブル

    rc = SUB_ChkUsedTcp(ClientSock,&SockInfo);                  // ソケット取得ポート
    if ( rc == 0 ){
        pTbl    = (struct sockaddr_in *)&SockInfo;              // 参照テーブルにキャスト
        Port    = pTbl->sin_port;                               // ポート番号設定
        Ip      = pTbl->sin_addr.s_addr;
    }

    memset(&Mtcd_tbl[0],0x00,sizeof(TCD_TBL)*CLIENT_MAX);       //自装置TCD振り分けテーブル
    Mtcd_tbl[0].Head.IpAdr = Ip;                                //IPアドレス
    Mtcd_tbl[0].Head.Port = ntohs(Port);                        //ポート番号
    for (cnt=0;cnt<TCD_MAX;cnt++){
        if ( Txxx_tcd[cnt].Tcd == 0xFFFFFFFF ){                 //終了チェック
            break;
        }
        memcpy(&Mtcd_tbl[0].TcdData[cnt],&Txxx_tcd[cnt],sizeof(TCD_DAT));
    }
    Mtcd_tbl[0].Head.Tcdcnt = cnt ;                         //登録TCD個数

    memset(&tcp_snd_dat,0x00,sizeof(tcp_snd_dat));

    memcpy(&tcp_snd_dat.Head.Id[0],"NS",2);
    tcp_snd_dat.Head.Ver        = 0x100;
    tcp_snd_dat.Head.Size   = sizeof(TCD_HEAD)+sizeof(TCD_DAT)*Mtcd_tbl[0].Head.Tcdcnt;
    tcp_snd_dat.Head.Tcd        = 0xF0900000;
    gt_time_now(&tcp_snd_dat.Head.SndTim[0]);                   //現在時刻を取得

    memcpy(&tcp_snd_dat.Data[0],&Mtcd_tbl[0],tcp_snd_dat.Head.Size);
    rc = QueMsgSet((QINDEX_*)&shm_sys.que_sys->index[0][SOCKET_SND],(unsigned char*)&tcp_snd_dat);
    if ( rc == QUE_FULL ){                                      //キュー書込失敗
        printX(TSK_SOUND,LOG_YES|ERR_YES|PRT_YES,"***TCD SND NG(QUE_FULL TCD=%08X)\n",tcp_snd_dat.Head.Tcd);
    } else {
        printX(TSK_SOUND,LOG_YES|PRT_YES,"TCD SND OK(TCD=%08X,Ip=%s,Port=%d)\n",tcp_snd_dat.Head.Tcd,inet_ntoa(Ip),ntohs(Port));
    }
    return(rc);
}

/*********************************************************************
 * NAME         : idc_end
 * FUNCTION     : ソフトウェア終了処理
 *              :
 * INPUT        :
 * RETURN       :
 * PROGRAMMED   : NSEC.shiba
 * DATE(ORG)    : 2002.10.20
 * CALL         : なし
 * FILE         : なし
 * REMARKS      : 備考
 ********************************************************************/
int tsk_end(int key)
{
    shm_end(key);                                                   //共有メモリの削除
    CloseDB(pConn);                                                 //DBとの接続を閉じる
    pConn = NULL;
    return(0);
}

/*********************************************************************
 * NAME         : shm_sys_init
 * FUNCTION     : システム用共有メモリの作成
 *              :
 * INPUT        :
 * RETURN       :
 * PROGRAMMED   : NSEC.shiba
 * DATE(ORG)    : 2002.10.20
 * CALL         : なし
 * FILE         : なし
 * REMARKS      : 備考
 ********************************************************************/
int shm_sys_init(int key)
{
    int     rc,cnt;
    int     offset_sys,offset_que;

    offset_sys = SHM_SYSTEM_MAX*key;                            //クライアント用システム管理データ番号
    offset_que = SHM_QUE_NO_MAX*key;                            //クライアント用キューエリア

    rc = shm_init(SHM_SYSTEM_NO+offset_sys,sizeof(SYS_TBL));    //システム管理エリアsys_tbl
    if ( rc < 0 ){
        return(-1);
    }
    rc = shm_init(SHM_SVPROC_NO+offset_sys,sizeof(PROC_SYS)*CLIENT_MAX);    //サーバプロセス管理エリアSVproc_tbl
    if ( rc < 0 ){
        return(-1);
    }
    rc = shm_init(SHM_CLPROC_NO+offset_sys,sizeof(PROC_SYS)*CLIENT_MAX);    //クライアントプロセス管理エリアCLproc_tbl
    if ( rc < 0 ){
        return(-1);
    }
    rc = shm_init(SHM_QUESYS_NO+offset_sys,sizeof(QUE_SYS)*CLIENT_MAX); //キューシステム管理エリアqindex
    if ( rc < 0 ){
        return(-1);
    }

    for(cnt=0;cnt<CLIENT_MAX;cnt++){                            //送受信用のキュー共有メモリ
        rc = shm_init(SHM_QUE_NO_CL+offset_que+(cnt*2),
                    TCP_BUF_MAX*QUE_MAX+sizeof(QUE_));          //共有メモリ作成と初期化
        if ( rc < 0 ){
            return(-1);
        }
        rc = shm_init(SHM_QUE_NO_CL+offset_que+(cnt*2)+1,
                    TCP_BUF_MAX*QUE_MAX+sizeof(QUE_));          //共有メモリ作成と初期化
        if ( rc < 0 ){
            return(-1);
        }
    }

    return(0);
}

/*********************************************************************
 * NAME         : shm_end
 * FUNCTION     : 共有メモリの削除
 *              :
 * INPUT        :
 * RETURN       :
 * PROGRAMMED   : NSEC.shiba
 * DATE(ORG)    : 2002.10.20
 * CALL         : なし
 * FILE         : なし
 * REMARKS      : 備考
 ********************************************************************/
int shm_end(int key)
{
    int     cnt;
    int     offset_sys,offset_que;

    offset_sys = SHM_SYSTEM_MAX*key;                            //クライアント用システム管理データ番号
    offset_que = SHM_QUE_NO_MAX*key;                            //クライアント用キューエリア

    shm_delete(SHM_SYSTEM_NO+offset_sys);                       //システム管理エリアsys_tbl
    shm_delete(SHM_SVPROC_NO+offset_sys);                       //サーバプロセス管理エリアSVproc_tbl
    shm_delete(SHM_CLPROC_NO+offset_sys);                       //クライアントプロセス管理エリアCLproc_tbl
    shm_delete(SHM_QUESYS_NO+offset_sys);                       //キューシステム管理エリアqindex
    for(cnt=0;cnt<CLIENT_MAX;cnt++){                            //送受信用のキュー共有メモリ
        shm_delete(SHM_QUE_NO_CL+offset_que+(cnt*2));
        shm_delete(SHM_QUE_NO_CL+offset_que+(cnt*2)+1);
    }

    return(0);
}

/*********************************************************************
 * NAME         : shm_sys_attach
 * FUNCTION     : システム用共有メモリのアドレス取得
 *              :
 * INPUT        :
 * RETURN       :
 * PROGRAMMED   : NSEC.shiba
 * DATE(ORG)    : 2002.10.20
 * CALL         : なし
 * FILE         : なし
 * REMARKS      : 備考
 ********************************************************************/
int shm_sys_attach(SHM_SYS *sp,int key)
{
    int     rc,cnt;
    char    *adr;
    int     offset_sys,offset_que;

    offset_sys = SHM_SYSTEM_MAX*key;                            //クライアント用システム管理データ番号
    offset_que = SHM_QUE_NO_MAX*key;                            //クライアント用キューエリア

    adr = shm_attach(SHM_SVPROC_NO+offset_sys,READ_WRITE);                      //サーバプロセス管理エリアSVproc_tbl
    if ( adr == 0 ){
        printX(TSK_SOUND,LOG_YES|ERR_YES|PRT_YES,"***shm_sys_attach(SHM_SVPROC_NO)\n");
        return(-1);
    }
    sp->SVproc_sys = (PROC_SYS*)adr;

    adr = shm_attach(SHM_CLPROC_NO+offset_sys,READ_WRITE);                      //クライアントプロセス管理エリアCLproc_tbl
    if ( adr == 0 ){
        printX(TSK_SOUND,LOG_YES|ERR_YES|PRT_YES,"***shm_sys_attach(SHM_CLPROC_NO)\n");
        return(-1);
    }
    sp->CLproc_sys = (PROC_SYS*)adr;

    adr = shm_attach(SHM_QUESYS_NO+offset_sys,READ_WRITE);                      //キューシステム管理エリアqindex
    if ( adr == 0 ){
        printX(TSK_SOUND,LOG_YES|ERR_YES|PRT_YES,"***shm_sys_attach(SHM_QUESYS_NO)\n");
        return(-1);
    }
    sp->que_sys = (QUE_SYS*)adr;

    for(cnt=0;cnt<CLIENT_MAX;cnt++){                                //送受信用のキュー共有メモリ
        rc = socket_que_init(sp->que_sys,SHM_QUE_NO_CL+offset_que+(cnt*2),cnt,SOCKET_RCV);
        if ( rc < 0 ){
            return(-1);
        }
        rc = socket_que_init(sp->que_sys,SHM_QUE_NO_CL+offset_que+(cnt*2)+1,cnt,SOCKET_SND);
        if ( rc < 0 ){
            return(-1);
        }
    }

    adr = shm_attach(SHM_SYSTEM_NO+offset_sys,READ_WRITE);                      //システム管理エリアsys_tbl
    if ( adr == 0 ){
        printX(TSK_SOUND,LOG_YES|ERR_YES|PRT_YES,"***shm_sys_attach(SHM_SYSTEM_NO)\n");
        return(-1);
    }
    sp->sys_tbl = (SYS_TBL*)adr;

    return(0);
}

/*********************************************************************
 * NAME         : shm_sys_detach
 * FUNCTION     : システム用共有メモリのアドレス取得
 *              :
 * INPUT        :
 * RETURN       :
 * PROGRAMMED   : NSEC.shiba
 * DATE(ORG)    : 2002.10.20
 * CALL         : なし
 * FILE         : なし
 * REMARKS      : 備考
 ********************************************************************/
int shm_sys_detach(SHM_SYS *sp,int key)
{
    int     rc,cnt;
    char    *adr;
    int     offset_sys,offset_que;
    SHM_SYS     w_Shm;

    offset_sys = SHM_SYSTEM_MAX*key;                                //クライアント用システム管理データ番号
    offset_que = SHM_QUE_NO_MAX*key;                                //クライアント用キューエリア

    memcpy(&w_Shm,sp,sizeof(SHM_SYS));
    memset(&shm_sys,0x00,sizeof(shm_sys));
    rc = shm_detach((char*)w_Shm.sys_tbl);                          //システム管理エリアsys_tbl
    if (rc < 0){
        printX(TSK_SOUND,LOG_YES|ERR_YES|PRT_YES,"***shm_sys_detach(sys_tbl) NG\n");
        return(-1);
    }

    rc = shm_detach((char*)w_Shm.SVproc_sys);                       //サーバプロセス管理エリアSVproc_tbl
    if (rc < 0){
        printX(TSK_SOUND,LOG_YES|ERR_YES|PRT_YES,"***shm_sys_detach(SVproc_sys) NG\n");
        return(-1);
    }

    rc = shm_detach((char*)w_Shm.CLproc_sys);                       //クライアントプロセス管理エリアCLproc_tbl
    if (rc < 0){
        printX(TSK_SOUND,LOG_YES|ERR_YES|PRT_YES,"***shm_sys_detach(CLproc_sys) NG\n");
        return(-1);
    }

    for(cnt=0;cnt<CLIENT_MAX;cnt++){                                //送受信用のキュー共有メモリ
        rc = shm_detach((char*)w_Shm.que_sys->index[cnt][SOCKET_RCV].addr);
        if ( rc < 0 ){
            return(-1);
        }
        rc = shm_detach((char*)w_Shm.que_sys->index[cnt][SOCKET_SND].addr);
        if ( rc < 0 ){
            return(-1);
        }
    }

    rc = shm_detach((char*)w_Shm.que_sys);                          //キューシステム管理エリアqindex
    if (rc < 0){
        printX(TSK_SOUND,LOG_YES|ERR_YES|PRT_YES,"***shm_sys_detach(que_sys) NG\n");
        return(-1);
    }

    return(0);
}

/*********************************************************************
 * NAME         : socket_que_init
 * FUNCTION     : プロセス初期化処理
 *              :
 * INPUT        :
 * RETURN       :
 * PROGRAMMED   : NSEC.shiba
 * DATE(ORG)    : 2002.10.20
 * CALL         : なし
 * FILE         : なし
 * REMARKS      : 備考
 ********************************************************************/
int socket_que_init(QUE_SYS *que_sys,int key,int pos,int mode)
{
    int     rc;
    char    *adr,*p;

    adr = shm_attach(key,READ_WRITE);
    if ( adr == 0 ){
        printX(TSK_SOUND,LOG_YES|PRT_YES,"***socket_que_init(que_sys)\n");
        return(-1);
    }

    que_sys->index[pos][mode].addr  = adr;                      //管理テーブルアドレス
    que_sys->index[pos][mode].blk   = QUE_MAX;                  //キューの個数
    que_sys->index[pos][mode].length    = TCP_BUF_MAX;          //１データサイズ
    p = adr + sizeof(QUE_);
    QueMsgInit(&que_sys->index[pos][mode],p,key);               //キューの初期化

    return(0);
}

/*********************************************************************
 * NAME         : SigTerm
 * FUNCTION     : ソフトウェア終了シグナル
 *              :
 * INPUT        :
 * RETURN       :
 * PROGRAMMED   : NSEC.shiba
 * DATE(ORG)    : 2002.10.20
 * CALL         : なし
 * FILE         : なし
 * REMARKS      : 備考
 ********************************************************************/
void SigTerm()
{
    printX(TSK_SOUND,PRT_YES,"SigTerm\n");
    shm_sys.sys_tbl->end_Flg = 1;                               //終了イベント設定
}

void SigInt()
{
    printX(TSK_SOUND,PRT_YES,"SigInt\n");
    shm_sys.sys_tbl->end_Flg = 1;                               //終了イベント設定
}

/*********************************************************************
 * NAME         : SigCld
 * FUNCTION     : 子プロセス終了シグナル
 *              :
 * INPUT        :
 * RETURN       :
 * PROGRAMMED   : NSEC.shiba
 * DATE(ORG)    : 2002.10.20
 * CALL         : なし
 * FILE         : なし
 * REMARKS      : 備考
 ********************************************************************/
void SigCld()
{
    int         rc,pid,st,cnt;
    PROC_TBL    *proc_p;

    pid = wait(&st);
    proc_p=&shm_sys.CLproc_sys->proc[0];

    for(cnt=0;cnt<shm_sys.sys_tbl->tsk_max;cnt++){
        if ( proc_p->sndpid == pid ){                           //未使用に設定する
            printX(TSK_SOUND,LOG_YES|ERR_YES|PRT_YES,"SigCld = (pos=%02d,adr=%s,port=%d,sndpid=%d) \n",
                    cnt,proc_p->ipadr,proc_p->port,proc_p->sndpid);
            proc_p->sndpid = 0;
            proc_p->use = FALSE;
            if ( (proc_p->sndpid == 0)&&(proc_p->rcvpid == 0) ){//未使用に設定する
                rc=close(proc_p->ac_sock);
                printX(TSK_SOUND,LOG_YES|ERR_YES|PRT_YES,"socket close (socket=%d,rc=%d)\n",proc_p->ac_sock,rc);
                memset(proc_p,0x00,sizeof(PROC_TBL));
                shm_sys.sys_tbl->tsk_count--;                   //接続プロセス数更新
                shm_sys.sys_tbl->Cld_end_Flg = 1;               //クライアントsocketプロセス終了イベントあり？
            }
            break;
        }
        if ( proc_p->rcvpid == pid ){                           //未使用に設定する
            proc_p->rcvpid = 0;
            proc_p->use = FALSE;
            if ( (proc_p->sndpid == 0)&&(proc_p->rcvpid == 0) ){                            //未使用に設定する
                rc=close(proc_p->ac_sock);
// DEL ELWSC 2007.05.02 START
//              printX(TSK_SOUND,LOG_YES|ERR_YES|PRT_YES,"socket close (socket=%d,rc=%d)\n",proc_p->ac_sock,rc);
// DEL ELWSC 2007.05.02 END
                memset(proc_p,0x00,sizeof(PROC_TBL));
                shm_sys.sys_tbl->tsk_count--;                   //接続プロセス数更新
                shm_sys.sys_tbl->Cld_end_Flg = 1;               //クライアントsocketプロセス終了イベントあり？
            }
            break;
        }
        proc_p++;
    }
}

/*********************************************************************
 * NAME         : GetPrivatefile
 * FUNCTION     : ファイルの読み込み
 *              :
 * INPUT        :
 * RETURN       :
 * PROGRAMMED   : NSEC.shiba
 * DATE(ORG)    : 2002.10.20
 * CALL         : なし
 * FILE         : なし
 * REMARKS      : 備考
 ********************************************************************/
int GetPrivatefile(T_INIFILE *IniF,char *fp)
{
    int     rc;
    char    bf[128];

    rc = 0;
    memset(IniF,0x00,sizeof(T_INIFILE));

    if (GetIniFile(fp,"IDCモード",bf) == 0){
        IniF->idc_mode = atoi(bf);
    }
    if (GetIniFile(fp,"公開ＩＰ",bf) == 0){
        strcpy(IniF->serverMyIP,bf);
    }
    if (GetIniFile(fp,"公開ポート",bf) == 0){
        IniF->serverMyPORT = atoi(bf);
    }
    if (GetIniFile(fp,"接続ＩＰ",bf) == 0){
        strcpy(IniF->clientYouIP,bf);
    }
    if (GetIniFile(fp,"接続ポート",bf) == 0){
        IniF->clientYouPORT = atoi(bf);
    }
    if (GetIniFile(fp,"接続周期T1",bf) == 0){
        IniF->t1_tim = atoi(bf);
    }
    if (GetIniFile(fp,"接続周期T2",bf) == 0){
        IniF->t2_tim = atoi(bf);
    }
    if (GetIniFile(fp,"RCNT",bf) == 0){
        IniF->retry = atoi(bf);
    }

    if ( IniF->idc_mode == 0 || IniF->idc_mode == 1 ){          //IDC用のINIファイルチェック
        if ( IniF->serverMyIP[0] == 0 && IniF->serverMyPORT == 0 ){
            rc = -1;
        }
    } else {                                                    //クライアント用のINIファイルチェック
        if ( IniF->clientYouIP[0] == 0 && IniF->clientYouPORT == 0 ){
            rc = -1;
        }
    }
    return(0);
}

void    SigUsr1()
{
}

void    SigTimer()
{
    alarm(1);                       // 一秒割り込み
}

/*********************************************************************
 * NAME         : set_sch_info
 * FUNCTION     : 当日・翌日スケジュールの作成
 *              :
 * INPUT        :
 * RETURN       :
 * PROGRAMMED   : KOCHI_TYOU
 * DATE(ORG)    : 2011.04.06
 * CALL         : なし
 * FILE         : なし
 * REMARKS      : 備考
 ********************************************************************/
 /*
void    set_sch_info(char *pConn)
{
    int checkBinFlag = 0;                       //当日便データフラグ
    int checkStopFlag = 0;                      //当日停留所データフラグ
    int checkBinFlagY = 0;                      //翌日便データフラグ
    int checkStopFlagY = 0;                     //翌日停留所データフラグ
    int exclusionFlag = 0;                      //スケジュールの排他設定メソッドの返却値
    int flag = 0;

    //当日便データが存在するかどうかをチェックする
    checkBinFlag = check_toujitu_sch(pConn);

    //翌日便データが存在するかどうかをチェックする
    checkBinFlagY = check_yokujitu_sch(pConn);

    //スケジュールの排他フラグをONに設定する
    exclusionFlag = setExclusionFlag(EXCLUSIONFLAG_ON, pConn);
    if(exclusionFlag == 1)
    {
        return;
    }

    //当日は改正日かどうかをチェックする
    flag = check_kaise_date(TOUJITU_SCH, pConn);

    //当日は改正日の場合
    if(flag)
    {
        //現用の基本データを削除する
        delete_genyo_data(pConn);
        //改正用データから現用の基本データを作る
        set_genyo_data(pConn);
    }

    //前日以前のスケジュールを消す
    delete_zenjitu_sch(pConn);

    //当日スケジュール存在するかどうかのチェック
    if(checkBinFlag != 0 )  //当日便データが存在しない場合
    {
        //当日便スケジュールを作成する
        set_bin_data(TOUJITU_SCH, pConn);                      //当日便データを作成する
    }


    if(checkBinFlagY != 0 ) //翌日便データが存在しない場合
    {
        //翌日便スケジュールを作成する
        set_bin_data(YOKUJITU_SCH, pConn);                     //翌日便データを作成する
    }

    //スケジュールの排他フラグをOFFに設定する
    exclusionFlag = setExclusionFlag(EXCLUSIONFLAG_OFF, pConn);
    if(exclusionFlag == 1)
    {
        return;
    }

}
*/
