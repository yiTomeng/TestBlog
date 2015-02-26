/*
 *****************************************************************************
 * MODULE       : DSSL_DIA_006.c
 * FUNCTION     : 静止画案内表示スケジュールチェック
 * VERSION      : 1.00.00
 * REMARKS      :
 * HISTORY      :
 * ID-----------DATE--------NAME----------NOTE -------------------------------
 * [V01.00.00]  2011.3.11   KOCHI    初期作成
 *****************************************************************************
 */
#define     DBG_MODE        1                   /* YES=デバック中 NO=リリース */
#include <stdio.h>
#include <time.h>
#include "quemsg.h"
#include "system.h"
#include "defmilIDC.h"
#include "defmilF2.h"
#include "defmil30.h"
#include "defmil31.h"
#include "defmil40.h"
#include "defmilD0.h"
#include "SocketCommon.h"
#include "shmop.h"
#include "printff.h"
#include "linter.h"
#include "DSSL_SOUND_000.h"

//現用スケジュールの取得（基本データとマスタデータから取得する）
#define SQLSELECT_GET_SCH   "SELECT " \
                                "  A.COD" \
                                ", A.GOU_NO" \
                                ", A.MASHI" \
                                ", B.NAM" \
                                ", B.NAM_E" \
                                ", B.NAM_C" \
                                ", B.NAM_K" \
                                ", A.HOUMEN" \
                                ", A.NORIBA" \
                                ", A.PASSENGER_DOOR" \
                                ", A.DESTINATION" \
                                ", A.UNKO_NO" \
                                ", A.START_TIME" \
                                ", C.UNKO_STATE" \
                                ", D.NAM" \
                                ", D.NAM_E" \
                                ", D.NAM_C" \
                                ", D.NAM_K" \
                                ", E.NAM" \
                                ", A.RSN_CD" \
                                " FROM" \
                                " (BUST03 A" \
                                " INNER JOIN BUST01 B " \
                                " ON (A.COD = B.COD) AND (A.GOU_NO = B.GOU_NO) AND (A.MASHI = B.MASHI) AND (A.GEN_KAI = B.GEN_KAI))" \
                                " INNER JOIN (SELECT * FROM BUST05 WHERE UNKO_DAY = '%s') C" \
                                " ON (A.COD = C.COD) AND (A.GOU_NO = C.GOU_NO) AND (A.MASHI = C.MASHI) AND (A.GEN_KAI = C.GEN_KAI) " \
                                " INNER JOIN BUST10 D" \
                                " ON (A.DESTINATION = D.COD) AND (A.GEN_KAI = D.GEN_KAI) " \
                                " INNER JOIN BUST09 E" \
                                " ON (A.HOUMEN = E.COD) AND (A.GEN_KAI = E.GEN_KAI) " \
                                " WHERE A.GEN_KAI = '%d' AND C.UNKO_DAY = '%s'" \
                                " ORDER BY CAST(A.COD as INTEGER),CAST(A.GOU_NO as INTEGER);"

//スケジュールの作成（当日・翌日）
#define SQLINSERT_SET_SCH   "INSERT INTO BUST06(" \
                                "  COD" \
                                ", DAYBYDAY" \
                                ", GOU_NO" \
                                ", MASHI" \
                                ", START_TIME" \
                                ", HOUMEN" \
                                ", NORIBA" \
                                ", PASSENGER_DOOR" \
                                ", DESTINATION" \
                                ", UNKO_NO" \
                                ", UNKO_STATE" \
                                ", B_NAM" \
                                ", B_NAM_E" \
                                ", B_NAM_C" \
                                ", B_NAM_K" \
                                ", D_NAM" \
                                ", D_NAM_E" \
                                ", D_NAM_C" \
                                ", D_NAM_K" \
                                ", H_NAM" \
                                ", RSN_CD" \
                                ", SAI_KSN_PGID" \
                                ", SAI_KSN_ID" \
                                ", SAI_KSN_YMD" \
                                ")VALUES(" \
                                "  '%s', '%s', '%s', '%d', '%s', " \
                                "  '%s', '%s', '%s', '%s', '%s', " \
                                "  '%d', '%s', '%s', '%s', '%s', " \
                                "  '%s', '%s', '%s', '%s', '%s', " \
                                "  '%s', '', '', current_timestamp);"

//停留所情報の取得（基本データとマスタデータから取得する）
#define SQLSELECT_GET_STOP  "SELECT " \
                                "  A.COD" \
                                ", A.GOU_NO" \
                                ", A.MASHI" \
                                ", A.BUSSTOP_NO" \
                                ", A.BUSSTOP_COD" \
                                ", B.NAM" \
                                ", B.NAM_E" \
                                ", B.NAM_C" \
                                ", B.NAM_K" \
                                ", A.SHOW" \
                                " FROM BUST04 A" \
                                " INNER JOIN BUST02 B" \
                                "   ON (A.BUSSTOP_COD = B.COD) AND (A.GEN_KAI = B.GEN_KAI)" \
                                " INNER JOIN BUST05 C" \
                                "   ON (A.COD = C.COD) AND (A.GOU_NO = C.GOU_NO) AND (A.MASHI = C.MASHI) AND (A.GEN_KAI = C.GEN_KAI) " \
                                " WHERE A.GEN_KAI = '%d' AND C.UNKO_DAY = '%s' AND A.COD = '%s' AND A.GOU_NO = '%s' AND A.MASHI = '%d' " \
                                " ORDER BY CAST(A.COD as INTEGER),CAST(A.GOU_NO as INTEGER);"

//停留所データの作成
#define SQLINSERT_SET_STOP  "INSERT INTO BUST07(" \
                            "  COD" \
                            ", DAYBYDAY" \
                            ", GOU_NO" \
                            ", MASHI" \
                            ", BUSSTOP_NO" \
                            ", BUSSTOP_COD" \
                            ", NAM" \
                            ", NAM_E" \
                            ", NAM_C" \
                            ", NAM_K" \
                            ", SHOW" \
                            ", SAI_KSN_PGID" \
                            ", SAI_KSN_ID" \
                            ", SAI_KSN_YMD" \
                            ")VALUES(" \
                            "  '%s', '%s', '%s', '%d', '%s', " \
                            "  '%s', '%s', '%s', '%s', '%s', " \
                            "  '%d', '', ''," \
                            "  current_timestamp);"

//排他フラグの設定
#define UPDATE_EXCLUSION_FLAG   "UPDATE BUST00 SET PAR01 = '%d' WHERE KBN = 'X0008';"

//改正日の取得
#define SELECT_KAYSE_DATE   "SELECT PAR01 FROM BUST00 WHERE KBN = 'X0009';"

//当日スケジュール作成時エラーデータ
#define INSERT_ERROR_DATA   "INSERT INTO BUST17(" \
                            "  COD" \
                            ", GOU_NO" \
                            ", MASHI" \
                            ", ERROR_COD" \
                            ", SAI_KSN_YMD" \
                            ")VALUES(" \
                            "  '%s', '%s', '%d', '%d', current_timestamp);"

//基本スケジュールデータ取得
#define SELECT_BIN_MST_DATA "SELECT " \
                                "  COD" \
                                ", GOU_NO" \
                                ", MASHI" \
                                ", HOUMEN" \
                                ", DESTINATION" \
                                ", GEN_KAI" \
                                " FROM BUST03 " \
                                " ORDER BY CAST(COD as INTEGER),CAST(GOU_NO as INTEGER);"

#define GEN_YOU                 0
#define KAYISEYI_YOU            1
#define ERROR_TYPE_1            0
#define ERROR_TYPE_2            1
#define ERROR_TYPE_3            2
#define G_BIN_MAX               1500
#define G_STOP_MAX              35

SlctResult  strResult[G_BIN_MAX];                   //データ格納構造体(MAX_RESULTNUM == 128)
SlctResult  strStopResult[G_STOP_MAX];              //データ格納構造体(MAX_RESULTNUM == 128)
SlctResult  strResult2[MAX_RESULTNUM];              // 検索結果(付帯案内文章で使用)

extern  TCP_DAT     tcp_snd_dat;                    //送信エリア
extern  TCP_DAT     tcp_rcv_dat;                    //受信エリア
static  unsigned char starup  = 1;

time_t nowtime;                                         //システム時間取得用変数
struct tm *timeinfo;                                    //システム時間取得用ポイント

void sch_common_timeSYS(time_t * t);
int GetNowOver24H(FmtDayTim * Now);
int snd_3130(unsigned char Kind);
void sch_daychg_check(char *pConn);
int delete_zenjitu_sch(char *pConn);
int check_toujitu_sch(char *pConn);
int check_yokujitu_sch(char *pConn);
int check_toujitu_stop(char *pConn);
int check_yokujitu_stop(char *pConn);
int set_bin_data(int sch_type, char *pConn);
int set_stop_data(int sch_type, char* binCod, char* gouNo, int mashi, char *pConn);
int DelFutaiData( int sch_type, char *pConn );
int set_Hutai_data(int sch_type, int KaiseiFlg,int binCod, int gouNo, int mashi, int RsnCod , char *pConn);
int snd_d202(void);
int setExclusionFlag(int exclusionFlag, char *pConn);
int check_kaise_date(int sch_type, char *pConn);
int check_data(SlctResult *strResult,int cnt);
int check_bin_data(SlctResult *strResult,int cnt, int type, char *pConn);
int check_bin_mst_data(int type, char *pConn);
void error_data_clear(int type, char *pConn);
void delete_genyo_data(char *pConn);
void set_genyo_data(char *pConn);
/*********************************************************************
 * NAME         : sch_common_timeSYS
 * FUNCTION     : システム時刻対応のtime()関数
 *              :
 * INPUT        : t : 現在のシステム時刻を受け取るtime_t型変数のアドレス
 * RETURN       : (なし)
 * PROGRAMMED   : ELWSC
 * DATE(ORG)    : 2007.05.03
 * CALL         : なし
 * FILE         : なし
 * REMARKS      : 備考
 ********************************************************************/
void sch_common_timeSYS(time_t * t)
{
    time_t  machine_time;

    time(&machine_time);    // マシン固有時刻を得る

    *t = (time_t)(((long)machine_time) );

    return;
}

/*********************************************************************
 * NAME         : GetNowOver24H
 * FUNCTION     : 現在時刻を得る
 *              :
 * INPUT        : 時刻を受け取るFmtDayTim型変数のアドレス
 * RETURN       : 0 補正なし
 * RETURN       : 1 補正あり
 * PROGRAMMED   : ELWSC
 * DATE(ORG)    : 2007.05.05
 * CALL         : なし
 * FILE         : なし
 * REMARKS      : 備考
 ********************************************************************/
int GetNowOver24H(FmtDayTim * Now)
{
    int         rc;
    time_t      t;
    struct tm * p;
    char        wkbuf1[32];


    rc = 0;

    sch_common_timeSYS(&t); // システム時刻
    p = localtime(&t);

    sprintf(wkbuf1, "%04d/%02d/%02d %02d:%02d:%02d"
        , (p->tm_year + 1900)
        , (p->tm_mon + 1)
        , p->tm_mday
        , p->tm_hour
        , p->tm_min
        , p->tm_sec
        );

    Now->Min  = p->tm_min;
    Now->Hour = p->tm_hour;


    if (Now->Hour < Tshm->DayChgDat.ChangeTime) {

        rc = 1;
        Now->Hour += 24;
        t -= (24*60*60);
        p = localtime(&t);
    }
    Now->Day    = p->tm_mday;
    Now->Month  = p->tm_mon + 1;
    Now->Year   = p->tm_year + 1900;


    return rc;
}

/*********************************************************************
 * NAME         : sch_kanri_higawari_snd
 * FUNCTION     : 日替わり通知送信
 *              :
 * INPUT        :
 * RETURN       :
 * PROGRAMMED   : HCC
 * DATE(ORG)    : 2004.11.12
 * CALL         : なし
 * FILE         : なし
 * REMARKS      : 備考
 ********************************************************************/
int snd_3130(unsigned char Kind)
{

    int                 rc, size;
    FmtTCD3130      *Snd_p;
    unsigned long       Tcd;


    memset(&tcp_snd_dat,0x00,sizeof(tcp_snd_dat));              //送信エリアクリア
    Snd_p       = (FmtTCD3130*)&tcp_snd_dat;                    //送信エリアポインタ設定
    Tcd         = TCD3130;                                      //TCD
    Snd_p->Kind = Kind;                                         //日替わり種別 1:初回起動 2:日替わり 3:データ再読むこみ


    size = sizeof(FmtTCD3130) - sizeof(FmtIDCHead);
    Tcp_Hed_datmk(Tcd,size,&tcp_snd_dat);                       //TCPヘッダ作成
    rc = QueMsgSet((QINDEX_*)&shm_sys.que_sys->index[0][SOCKET_SND],(unsigned char*)&tcp_snd_dat);
    if ( rc == QUE_FULL ){                                      //キュー書込失敗
        rc = 1;
        printX(TSK_SOUND,LOG_YES|ERR_YES|PRT_YES,"***TCD SND 3130 NG(QUE_FULL TCD=%08X)\n",Tcd);

    } else {
        rc = 0;
        printX(TSK_SOUND,LOG_YES|PRT_YES,"[%s]TCD SND 3130 OK(日替わり種別=%d)\n",
            __FUNCTION__, Kind);
    }
    return (rc);
}

/*********************************************************************
 * NAME         : sch_daychg_check
 * FUNCTION     : 日替わりチェック
 *              :
 * INPUT        :
 * RETURN       :
 * PROGRAMMED   : KOCHI_TYOU
 * DATE(ORG)    : 2011.04.05
 * CALL         : なし
 * FILE         : なし
 * REMARKS      : 備考
 ********************************************************************/
void sch_daychg_check(char *pConn)
{
    FmtDayTim       Now;
    unsigned short  Nowhhmm;
    char    DayChgProcFlg = 0;          //日替わり処理実行フラグ 0:日替わり処理未 1:日替わり処理済
    int checkBinFlag = 0;                       //当日便データフラグ
    int checkStopFlag = 0;                      //当日停留所データフラグ
    int checkBinFlagY = 0;                      //翌日便データフラグ
    int checkStopFlagY = 0;                     //翌日停留所データフラグ
    int exclusionFlag = 0;                      //スケジュールの排他設定メソッドの返却値
    int flag = 0;
    GetNowOver24H(&Now);
    Nowhhmm = ((short)Now.Hour * 100) + (short)Now.Min;
    //システム日付の再取得
    GetNowOver24H( &Tshm->Now_Day );

    //現在時刻が日替わり処理時刻と同じになった
    if( Tshm->DayChgDat.DayChgProcTime == Nowhhmm )
    {
        //スケジュールの排他フラグをONに設定する
        exclusionFlag = setExclusionFlag(EXCLUSIONFLAG_ON, pConn);
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

        //当日便データが存在するかどうかをチェックする
        checkBinFlag = check_toujitu_sch(pConn);

        //当日停留所データが存在するかどうかをチェックする
        checkStopFlag = check_toujitu_stop(pConn);

        //翌日便データが存在するかどうかをチェックする
        checkBinFlagY = check_yokujitu_sch(pConn);

        //翌日停留所データが存在するかどうかをチェックする
        checkStopFlagY = check_yokujitu_stop(pConn);

        if(checkBinFlag != 0)   //当日便データが存在しない場合
        {
            //当日便スケジュールを作成する
            set_bin_data(TOUJITU_SCH, pConn);                       //当日便データを作成する
        }

        if(checkBinFlagY != 0)  //翌日便データが存在しない場合

        {
            //翌日便スケジュールを作成する
            set_bin_data(YOKUJITU_SCH, pConn);                      //翌日便データを作成する
        }

        //スケジュールの排他フラグをOFFに設定する
        exclusionFlag = setExclusionFlag(EXCLUSIONFLAG_OFF, pConn);

        if(exclusionFlag == 1)
        {
            return;
        }

        //データ管理タスクへの送信（全乗り場対象）
        snd_d202();

        sleep(60);
    }

}

/*********************************************************************
 * NAME         : delete_zenjitu_sch
 * FUNCTION     : 前日のスケジュールを消す
 *              :
 * INPUT        :
 * RETURN       :
 * PROGRAMMED   : KOCHI_TYOU
 * DATE(ORG)    : 2011.04.05
 * CALL         : なし
 * FILE         : なし
 * REMARKS      : 備考
 ********************************************************************/
int delete_zenjitu_sch(char *pConn)
{

    //システム時間を取得する
    time( &nowtime );
    timeinfo = localtime( &nowtime );
    char sys_date[9];/* 最後にヌルキャラクタが付加されるため、[10]ではメモリーが破壊される */
    memset(&sys_date[0], 0x00, sizeof(sys_date));

    //前日の日付を取得する
    sprintf(sys_date,"%04d%02d%02d",timeinfo->tm_year+1900, timeinfo->tm_mon+1, timeinfo->tm_mday-1);

    int binRowcnt = 0,stopRowcnt = 0;
    char binQuery[MAX_SQL_LEN];
    memset(&binQuery[0], 0x00, sizeof(binQuery));

    int Rowcnt;
    char stopQuery[MAX_SQL_LEN];
    memset(&stopQuery[0], 0x00, sizeof(stopQuery));

    //前日の便データの削除
    sprintf(binQuery,"delete from BUST06 where DAYBYDAY <= '%s';", sys_date);

    binRowcnt = Execute(pConn, (unsigned char*)binQuery);//削除メソッド
    if (binRowcnt < 0)
    {
        printX(TSK_SOUND,LOG_YES|PRT_YES,"[%s] データ削除NG(sql=%s)\n", __FUNCTION__, binQuery);
    }

    //前日の停留所データの削除
    sprintf(stopQuery,"delete from BUST07 where DAYBYDAY <= '%s';", sys_date);

    stopRowcnt = Execute(pConn, (unsigned char*)stopQuery);//削除メソッド
    if (stopRowcnt < 0)
    {
        printX(TSK_SOUND,LOG_YES|PRT_YES,"[%s] データ削除NG(sql=%s)\n", __FUNCTION__, stopQuery);
    }

    return 0;

}

/*********************************************************************
 * NAME         : check_toujitu_sch
 * FUNCTION     : 当日スケジュールが存在するかどうかをチェックする
 *              :
 * INPUT        :
 * RETURN       : 0：当日スケジュールが存在する
                  1：当日スケジュールが存在しない
 * PROGRAMMED   : KOCHI_TYOU
 * DATE(ORG)    : 2011.03.11
 * CALL         : なし
 * FILE         : なし
 * REMARKS      : 備考
 ********************************************************************/
int check_toujitu_sch(char *pConn)
{
    //システム時間を取得する
    time( &nowtime );
    timeinfo = localtime( &nowtime );
    char sys_date[9];/* 最後にヌルキャラクタが付加されるため、[10]ではメモリーが破壊される */
    memset(&sys_date[0], 0x00, sizeof(sys_date));

    //当日の日付を取得する
    sprintf(sys_date,"%04d%02d%02d",timeinfo->tm_year+1900, timeinfo->tm_mon+1, timeinfo->tm_mday);

    int Rowcnt = 0;
    memset(&strResult[0], 0x00, sizeof(strResult));
    char Query[MAX_SQL_LEN];
    memset(&Query[0], 0x00, sizeof(Query));

    sprintf(Query,"select count(*) from BUST06 where DAYBYDAY = '%s';", sys_date);
    Rowcnt=GetData(pConn, (unsigned char*) Query, strResult );
    if( atoi(strResult[0].Column[0]) == 0 ) {
        return 1;
    }

    return 0;

}

/*********************************************************************
 * NAME         : check_yokujitu_sch
 * FUNCTION     : 翌日スケジュールが存在するかどうかをチェックする
 *              :
 * INPUT        :
 * RETURN       : 0：翌日スケジュールが存在する
                  1：翌日スケジュールが存在しない
 * PROGRAMMED   : KOCHI_TYOU
 * DATE(ORG)    : 2011.04.06
 * CALL         : なし
 * FILE         : なし
 * REMARKS      : 備考
 ********************************************************************/
int check_yokujitu_sch(char *pConn)
{
    //システム時間を取得する
    time( &nowtime );
    timeinfo = localtime( &nowtime );
    char sys_date[9];/* 最後にヌルキャラクタが付加されるため、[10]ではメモリーが破壊される */
    memset(&sys_date[0], 0x00, sizeof(sys_date));

    //翌日の日付を取得する
    sprintf(sys_date,"%04d%02d%02d",timeinfo->tm_year+1900, timeinfo->tm_mon+1, timeinfo->tm_mday + 1);

    int Rowcnt = 0;
    memset(&strResult[0], 0x00, sizeof(strResult));
    char Query[MAX_SQL_LEN];
    memset(&Query[0], 0x00, sizeof(Query));

    sprintf(Query,"select count(*) from BUST06 where DAYBYDAY = '%s';", sys_date);
    Rowcnt=GetData(pConn, (unsigned char*) Query, strResult );
    if( atoi(strResult[0].Column[0]) == 0 ) {
        return 1;
    }

    return 0;

}

/*********************************************************************
 * NAME         : check_toujitu_stop
 * FUNCTION     : 当日停留所情報が存在するかどうかをチェックする
 *              :
 * INPUT        :
 * RETURN       : 0：当日停留所情報が存在する
                  1：当日停留所情報が存在しない
 * PROGRAMMED   : KOCHI_TYOU
 * DATE(ORG)    : 2011.03.11
 * CALL         : なし
 * FILE         : なし
 * REMARKS      : 備考
 ********************************************************************/
int check_toujitu_stop(char *pConn)
{
    //システム時間を取得する
    time( &nowtime );
    timeinfo = localtime( &nowtime );
    char sys_date[9];/* 最後にヌルキャラクタが付加されるため、[10]ではメモリーが破壊される */
    memset(&sys_date[0], 0x00, sizeof(sys_date));

    //当日の日付を取得する
    sprintf(sys_date,"%04d%02d%02d",timeinfo->tm_year+1900, timeinfo->tm_mon+1, timeinfo->tm_mday);

    int Rowcnt = 0;
    memset(&strResult[0], 0x00, sizeof(strResult));
    char Query[MAX_SQL_LEN];
    memset(&Query[0], 0x00, sizeof(Query));

    sprintf(Query,"select count(*) from BUST07 where DAYBYDAY = '%s';", sys_date);
    Rowcnt=GetData(pConn, (unsigned char*) Query, strResult );
    if( atoi(strResult[0].Column[0]) == 0 ) {
        return 1;
    }

    return 0;

}

/*********************************************************************
 * NAME         : check_yokujitu_stop
 * FUNCTION     : 翌日停留所情報が存在するかどうかをチェックする
 *              :
 * INPUT        :
 * RETURN       : 0：翌日停留所情報が存在する
                  1：翌日停留所情報が存在しない
 * PROGRAMMED   : KOCHI_TYOU
 * DATE(ORG)    : 2011.04.06
 * CALL         : なし
 * FILE         : なし
 * REMARKS      : 備考
 ********************************************************************/
int check_yokujitu_stop(char *pConn)
{
    //システム時間を取得する
    time( &nowtime );
    timeinfo = localtime( &nowtime );
    char sys_date[9];/* 最後にヌルキャラクタが付加されるため、[10]ではメモリーが破壊される */
    memset(&sys_date[0], 0x00, sizeof(sys_date));

    //当日の日付を取得する
    sprintf(sys_date,"%04d%02d%02d",timeinfo->tm_year+1900, timeinfo->tm_mon+1, timeinfo->tm_mday + 1);

    int Rowcnt = 0;
    memset(&strResult[0], 0x00, sizeof(strResult));
    char Query[MAX_SQL_LEN];
    memset(&Query[0], 0x00, sizeof(Query));

    sprintf(Query,"select count(*) from BUST07 where DAYBYDAY = '%s';", sys_date);
    Rowcnt=GetData(pConn, (unsigned char*) Query, strResult );
    if( atoi(strResult[0].Column[0]) == 0 ) {
        return 1;
    }

    return 0;

}

/*********************************************************************
 * NAME         : set_bin_data
 * FUNCTION     : 便データの作成
 *              :
 * INPUT        : 当日・翌日スケジュールの識別子
 * RETURN       : 0：成功
 *                1：失敗
 * PROGRAMMED   : KOCHI_TYOU
 * DATE(ORG)    : 2011.03.11
 * CALL         : なし
 * FILE         : なし
 * REMARKS      : 備考
 ********************************************************************/
int set_bin_data(int sch_type, char *pConn)
{
    int Rowcnt = 0;                                         //検索メソッドの返却値
    int m = 0;                                              //ループ用変数
    int ret = 0;                                            //スケジュール追加用メソッドの返却値
    int rc = 0;                                             //メソッドの返却値
    memset(&strResult[0], 0x00, sizeof(strResult));
    char sys_date[9];                                       //当日/翌日の日付
    memset(&sys_date[0], 0x00, sizeof(sys_date));
    char    SelectQuery[MAX_SQL_LEN];                       //検索用SQL文
    memset(&SelectQuery[0], 0x00, sizeof(SelectQuery));
    char    InsertQuery[MAX_SQL_LEN];                       //追加用SQL文
    int flag = 0;
    int FuatiDel;                                           // 付帯削除 返り値

    //システム時間を取得する
    time( &nowtime );
    timeinfo = localtime( &nowtime );

    //識別子は当日スケジュールの場合
    if(sch_type == TOUJITU_SCH)
    {
        //当日の日付を取得する
        sprintf(sys_date,"%04d%02d%02d",timeinfo->tm_year+1900, timeinfo->tm_mon+1, timeinfo->tm_mday);
    }
    //識別子は翌日スケジュールの場合
    else if(sch_type == YOKUJITU_SCH)
    {
        //翌日の日付を取得する
        sprintf(sys_date,"%04d%02d%02d",timeinfo->tm_year+1900, timeinfo->tm_mon+1, timeinfo->tm_mday+1);

    }

    flag = check_kaise_date(YOKUJITU_SCH, pConn);
    //翌日は改正日の場合
    if(sch_type == YOKUJITU_SCH && flag)
    {
        //翌日のスケジュールの取得（改正用）
        sprintf(SelectQuery, SQLSELECT_GET_SCH,
                sys_date, KAYISEYI_YOU, sys_date);                              /* 翌日の日付           :   UNKO_DAY        */
    }
    else
    {
        //当日または翌日の現用スケジュールの取得（渡された識別子次第）
        sprintf(SelectQuery, SQLSELECT_GET_SCH,
                sys_date, GEN_YOU, sys_date);                           /* 当日/翌日の日付      :   UNKO_DAY        */
    }

    //エラーテーブル中のデータをクリアする
    error_data_clear(sch_type, pConn);

    //エラー検知処理(エラー種別２：マスタデータにデータがない)
    check_bin_mst_data(sch_type, pConn);

    //検索メソッド
    Rowcnt=GetData(pConn, (unsigned char*) SelectQuery, strResult );

    if( Rowcnt <= 0 ) {
        printX(TSK_SOUND,LOG_YES|PRT_YES,"[%s] 便データがなし(sql=%s)\n", __FUNCTION__, SelectQuery);
        rc = 1;
    }
    else
    {
        // 該当日の付帯文章データを削除する
        FuatiDel = DelFutaiData( sch_type, pConn );

        //基本データから取得した結果を元に、当日/翌日のスケジュールを作成する
        for(m = 0; m < Rowcnt; m++)
        {

            //当日/翌日の便データ追加用SQLを初期化する
            memset(&InsertQuery[0], 0x00, sizeof(InsertQuery));
            //エラー検知処理(エラー種別１:同一運行日かつ発車時刻に、同一乗り場が設定されてる、３：乗り場と乗車口が未設定)
            if(check_bin_data(strResult,m,sch_type, pConn))
            {
                continue;
            }

            sprintf(InsertQuery, SQLINSERT_SET_SCH,
                    strResult[m].Column[0],                         /* 便コード             :   COD             */
                    sys_date,                                       /* 当日／翌日           :   DAYBYDAY        */
                    strResult[m].Column[1],                         /* 号車                 :   GOU_NO          */
                    atoi(strResult[m].Column[2]),                   /* 増表示               :   MASHI           */
                    strResult[m].Column[12],                        /* 発時刻               :   START_TIME      */
                    strResult[m].Column[7],                         /* 方面コード           :   HOUMEN          */
                    strResult[m].Column[8],                         /* のりばコード         :   NORIBA          */
                    strResult[m].Column[9],                         /* 乗降口コード         :   PASSENGER_DOOR  */
                    strResult[m].Column[10],                        /* 行き先コード         :   DESTINATION     */
                    strResult[m].Column[11],                        /* 運行会社コード       :   UNKO_NO         */
                    atoi(strResult[m].Column[13]),                  /* 運行状態             :   UNKO_STATE      */
                    strResult[m].Column[3],                         /* 便名称               :   B_NAM           */
                    strResult[m].Column[4],                         /* 便名称（英語）       :   B_NAM_E         */
                    strResult[m].Column[5],                         /* 便名称（中国語）     :   B_NAM_C         */
                    strResult[m].Column[6],                         /* 便名称（韓国語）     :   B_NAM_K         */
                    strResult[m].Column[14],                        /* 行先名称             :   D_NAM"          */
                    strResult[m].Column[15],                        /* 行先名称（英語）     :   D_NAM_E         */
                    strResult[m].Column[16],                        /* 行先名称（中国語）   :   D_NAM_C         */
                    strResult[m].Column[17],                        /* 行先名称（韓国語）   :   D_NAM_K         */
                    strResult[m].Column[18],                        /* 方面名称             :   H_NAM           */
                    strResult[m].Column[19]                         /* 路線コード           :   RSN_CD          */
                    );

            //当日/翌日の便データの追加
            ret = Execute(pConn, (unsigned char*)InsertQuery);

            if (ret < 0)
            {
                //当日便データの追加失敗
                if(sch_type == TOUJITU_SCH)
                {
                    printX(TSK_SOUND,LOG_YES|PRT_YES,"[%s] 当日便データの追加NG(sql=%s)\n", __FUNCTION__, InsertQuery);
                }
                //翌日便データの追加失敗
                else if(sch_type == YOKUJITU_SCH)
                {
                    printX(TSK_SOUND,LOG_YES|PRT_YES,"[%s] 翌日便データの追加NG(sql=%s)\n", __FUNCTION__, InsertQuery);
                }
            }

            //停留所のデータ取得
            set_stop_data(sch_type, strResult[m].Column[0], strResult[m].Column[1], atoi(strResult[m].Column[2]), pConn);

            // 付帯削除 正常終了
            if(FuatiDel == 0)
            {
                //付帯文章 データ作成

                ret = set_Hutai_data(sch_type, flag, atoi(strResult[m].Column[0]), atoi(strResult[m].Column[1]), atoi(strResult[m].Column[2]), atoi(strResult[m].Column[19]), pConn );
                if(ret < 0 )
                {
                    printX(TSK_SOUND,LOG_YES|PRT_YES,"付帯文章作成失敗 ret = %d\n",ret);
                }
            }
        }
    }

    return rc;
}

/*********************************************************************
 * NAME         : set_stop_data
 * FUNCTION     : 停留所データの作成
 *              :
 * INPUT        : 当日・翌日スケジュールの識別子
 * RETURN       : 0：成功
 *                1：失敗
 * PROGRAMMED   : KOCHI_TYOU
 * DATE(ORG)    : 2011.03.11
 * CALL         : なし
 * FILE         : なし
 * REMARKS      : 備考
 ********************************************************************/
int set_stop_data(int sch_type, char* binCod, char* gouNo, int mashi, char *pConn)
{
    int retStop = 0;                                            //停留所データ追加メッソドの返却値
    char    InsertStopQuery[MAX_SQL_LEN];                       //追加用SQL文
    int StopRowcnt = 0;                                         //検索メソッドの返却値
    int n = 0;                                                  //ループ用変数
    int rc = 0;                                                 //メソッドの返却値
    memset(&strStopResult[0], 0x00, sizeof(strStopResult));
    char sys_date[9];                                           //当日/翌日の日付
    memset(&sys_date[0], 0x00, sizeof(sys_date));
    char    SelectStopQuery[MAX_SQL_LEN];                       //検索用SQL文
    memset(&SelectStopQuery[0], 0x00, sizeof(SelectStopQuery));
    int flag = 0;

    //システム時間を取得する
    time( &nowtime );
    timeinfo = localtime( &nowtime );

    //識別子は当日スケジュールの場合
    if(sch_type == TOUJITU_SCH)
    {
        //当日の日付を取得する
        sprintf(sys_date,"%04d%02d%02d",timeinfo->tm_year+1900, timeinfo->tm_mon+1, timeinfo->tm_mday);
    }
    //識別子は翌日スケジュールの場合
    else if(sch_type == YOKUJITU_SCH)
    {
        //翌日の日付を取得する
        sprintf(sys_date,"%04d%02d%02d",timeinfo->tm_year+1900, timeinfo->tm_mon+1, timeinfo->tm_mday+1);
    }


    flag =  check_kaise_date(YOKUJITU_SCH, pConn);

    //翌日は改正日の場合
    if(flag && sch_type == YOKUJITU_SCH)
    {
        //翌日の停留所データの取得（改正用）
        sprintf(SelectStopQuery, SQLSELECT_GET_STOP,
                KAYISEYI_YOU,                       /* 改正用               :   GEN_KAI         */
                sys_date,                           /* 翌日の日付           :   UNKO_DAY        */
                binCod,                             /* 便コード             :   COD             */
                gouNo,                              /* 号車                 :   GOU_NO          */
                mashi                               /* 増表示               :   MASHI           */
                );
    }
    else
    {
        //当日または翌日の現用停留所データの取得（渡された識別子次第）
        sprintf(SelectStopQuery, SQLSELECT_GET_STOP,
                GEN_YOU,                            /* 現用                 :   GEN_KAI         */
                sys_date,                           /* 翌日の日付           :   UNKO_DAY        */
                binCod,                             /* 便コード             :   COD             */
                gouNo,                              /* 号車                 :   GOU_NO          */
                mashi                               /* 増表示               :   MASHI           */
                );
    }

    //検索メソッド
    StopRowcnt=GetData(pConn, (unsigned char*) SelectStopQuery, strStopResult );

    if( StopRowcnt <= 0 ) {
        printX(TSK_SOUND,LOG_YES|PRT_YES,"[%s] 停留所データがなし(sql=%s)\n", __FUNCTION__, SelectStopQuery);
        rc = 1;
    }
    else
    {
        //当日/翌日の停留所データの追加
        for(n = 0; n < StopRowcnt; n++)
        {
            //当日/翌日の停留所データ追加用SQLを初期化する
            memset(&InsertStopQuery[0], 0x00, sizeof(InsertStopQuery));
            //取得したデータが「NULL」かどうかのチェック
            check_data(strStopResult,n);
            sprintf(InsertStopQuery, SQLINSERT_SET_STOP,
                    strStopResult[n].Column[0],                 /* 便コード             :   COD             */
                    sys_date,                                   /* 当日／翌日           :   DAYBYDAY        */
                    strStopResult[n].Column[1],                 /* 号車                 :   GOU_NO          */
                    atoi(strStopResult[n].Column[2]),           /* 増表示               :   MASHI           */
                    strStopResult[n].Column[3],                 /* 停留所順             :   BUSSTOP_NO      */
                    strStopResult[n].Column[4],                 /* 停留所コード         :   BUSSTOP_COD     */
                    strStopResult[n].Column[5],                 /* 停留所名称           :   NAM             */
                    strStopResult[n].Column[6],                 /* 停留所名称（英語）   :   NAM_E           */
                    strStopResult[n].Column[7],                 /* 停留所名称（中国語） :   NAM_C           */
                    strStopResult[n].Column[8],                 /* 停留所名称（韓国語） :   NAM_K           */
                    atoi(strStopResult[n].Column[9])            /* 表示                 :   SHOW            */
                    );

            //当日/翌日の停留所データの追加
            retStop = Execute(pConn, (unsigned char*)InsertStopQuery);
            if (retStop < 0)
            {
                //当日停留所データの追加失敗
                if(sch_type == TOUJITU_SCH)
                {
                    printX(TSK_SOUND,LOG_YES|PRT_YES,"[%s] 当日停留所データの追加NG(sql=%s)\n", __FUNCTION__, InsertStopQuery);
                }
                //翌日停留所データの追加失敗
                else if(sch_type == YOKUJITU_SCH)
                {
                    printX(TSK_SOUND,LOG_YES|PRT_YES,"[%s] 翌日停留所データの追加NG(sql=%s)\n", __FUNCTION__, InsertStopQuery);
                }
            }
        }
    }

    return rc;
}
/*********************************************************************
 * NAME         : DelFutaiData
 * FUNCTION     : 基本ダイヤより当日ダイヤに付帯文章を割り当て
 *              :
 * INPUT        : 当日・翌日スケジュールの識別子
 * RETURN       : 0：成功
 *                1：失敗
 * PROGRAMMED   : KOCHI_TYOU
 * DATE(ORG)    : 2011.03.11
 * CALL         : なし
 * FILE         : なし
 * REMARKS      : 備考
 ********************************************************************/
int DelFutaiData( int sch_type, char *pConn )
{
    char                Query[MAX_SQL_LEN];
    int                 ret;

    unsigned long       DateSec;
    struct tm           Date;


    // 初期化
    memset( Query, 0x00, sizeof(Query));

    // ダイヤ作成該当日を取得
    time( &DateSec );
    // 翌日ダイヤなら1日分 秒加算
    if( sch_type == YOKUJITU_SCH )
    {
        DateSec = DateSec + ONE_DAY_SEC;
    }
    else if( sch_type != TOUJITU_SCH )
    {
        printX(TSK_SOUND,LOG_YES|PRT_YES,"[%s]スケジュール種別が不正 sch_type =%d \n",__FUNCTION__,sch_type );
        return(-1);
    }

    // 日付け取得
    localtime_r( &DateSec, &Date );

    // SQL作成
    sprintf( Query, "DELETE FROM BUST13 WHERE DAYBYDAY = '%04d%02d%02d';",
                ( Date.tm_year + 1900 ),( Date.tm_mon + 1 ), Date.tm_mday );
    ret = Execute(pConn, (unsigned char*) Query );
    if( ret < 0 )
    {
        printX(TSK_SOUND, LOG_YES|PRT_YES,"付帯文章データ削除失敗 ret = %d SQL:%s\n",ret,Query);
        return(-1);
    }
    return 0;
}
/*********************************************************************
 * NAME         : set_stop_data
 * FUNCTION     : 基本ダイヤより当日ダイヤに付帯文章を割り当て
 *              :
 * INPUT        : 当日・翌日スケジュールの識別子
 * RETURN       : 0：成功
 *                1：失敗
 * PROGRAMMED   : KOCHI_TYOU
 * DATE(ORG)    : 2011.03.11
 * CALL         : なし
 * FILE         : なし
 * REMARKS      : 備考
 ********************************************************************/
int set_Hutai_data(int sch_type, int KaiseiFlg,int binCod, int gouNo, int mashi, int RsnCod , char *pConn)
{
    // 表示優先度
    // ① 基本データ
    // ② 付帯案内文章 期間あり(期間中) 路線コード
    // ③ 付帯案内文章 期間あり(期間中) 便コード
    // ④ 付帯案内文章 期間なし         路線コード
    // ⑤ 付帯案内文章 期間なし         便コード
    // ⑥ 一般案内文章

    char                Query[MAX_SQL_LEN];                 // SQL
    char                QuerySub[256];                      // SQLサブ
    SlctResult          strResult[MAX_RESULTNUM];           // 検索結果
    SlctResult          tmpResult;                          // 検索結果(一時格納用)
    int                 tmpRowCnt;
    int                 RowCnt;                             // 検索件数

    unsigned    long    DateSec;                            // スケジュール作成日付け取得に使用
    struct tm           Date;                               // スケジュール作成日付け取得に使用
    unsigned    long    DateYMD;
    int                 Seq;                                // シーケンス番号
    int                 i,k,x;                                  // ループカウンタ
    int                 ret;                                // ret
    int                 StIdx;                              // 開始INDEX
    int                 EdIdx;                              // 終了INDEX

    // 初期化
    memset( Query, 0x00, sizeof(Query));
    memset( strResult, 0x00, sizeof(strResult));
    RowCnt = 0;
    Seq = 1;

    // スケジュールタイプより日付けを取得
    time( &DateSec );
    if( sch_type == YOKUJITU_SCH )
    {
        DateSec = DateSec + ONE_DAY_SEC;        // 1日分の秒数を加算
    }
    else if( sch_type != TOUJITU_SCH )
    {
        printX(TSK_SOUND,LOG_YES|PRT_YES,"[%s]スケジュール種別が不正 sch_type =%d \n", __FUNCTION__ ,sch_type );
        return(-1);
    }
    localtime_r( &DateSec, &Date );
    DateYMD = ((Date.tm_year+1900) * 10000) + ((Date.tm_mon +1) * 100) +  Date.tm_mday;


    // ①基本データ 対象日・便コード・号車・増 現用/改正用からデータ取得
    memset( Query, 0x00, sizeof(Query));
    sprintf( Query,"SELECT FUTAI_NO FROM BUST12 WHERE COD = '%d' AND GEN_KAI = '%d' AND GOU_NO = '%d' AND MASHI = '%d' "\
                                                            "ORDER BY CAST(FUTAI_NO AS INTEGER);"
                                                        ,binCod,  KaiseiFlg ,  gouNo, mashi );
    RowCnt=GetData(pConn, (unsigned char*) Query, strResult );
    // 検索に失敗したら終了
    if( RowCnt < 0 )
    {
        printX(TSK_SOUND,LOG_YES|PRT_YES,"付帯文章 基本 検索失敗 RowCnt = %d SQL:%s\n ",RowCnt,Query);
        return (-1);
    }

    // 当日/翌日ダイヤに反映
    for( i=0; i<RowCnt; i++ )
    {
        memset( Query, 0x00, sizeof(Query));
        sprintf( Query, "INSERT INTO BUST13 ( COD, DAYBYDAY, GOU_NO, MASHI, FUTAI_NO, SEQ ) "\
                                    " VALUES ( '%d','%08d', '%d','%d','%d', '%d' ); "
                                                ,binCod
                                                ,DateYMD
                                                ,gouNo
                                                ,mashi
                                                ,atoi( strResult[i].Column[0])
                                                ,Seq);

        ret = Execute(pConn, (unsigned char*) Query );
        if( ret < 0 )
        {
            printX(TSK_SOUND, LOG_YES|PRT_YES,"[%s]付帯文章データ更新失敗(基本ダイヤ)ret = %d 文章番号 = %d SQL:%s\n",__FUNCTION__,ret, atoi( strResult[i].Column[0]) ,Query);
            continue;
        }
        Seq++;                      // シーケンス番号

        // 付帯文章が最大に達したら終了
        if( Seq == HUTAI_MAX )
        {
            return Seq;
        }
    }


    // ② 付帯案内文章 期間あり(期間中) 路線コード
    // SQL作成
    // カラム数が多いので2回に分けてとる
    memset(strResult2,0x00,sizeof(strResult2));
    RowCnt = 0;
    for( k=0; k<2; k++ )
    {

        // WHERE句の開始・終了INDEXを決める
        if( k==0 )
        {
            StIdx = 0;
            EdIdx = RSNCD_MAX / 2;
        }
        else
        {
            StIdx = RSNCD_MAX / 2;
            EdIdx = RSNCD_MAX;
        }

        memset( Query, 0x00, sizeof(Query));
        sprintf( Query,"SELECT MOJI_ID, SPAN_ST, SPAN_ED  FROM BUST14"\
                        " WHERE GEN_KAI='%d' AND CAST(SPAN_ST AS INTEGER)<='%08d' AND '%08d'<=CAST(SPAN_ED AS INTEGER)"\
                        " AND((SPAN_ST<>'00000000')AND(SPAN_ED<>'99999999')) "
                        ,KaiseiFlg, DateYMD, DateYMD );
        strcat( Query, "AND(");
        for( i=StIdx ; i < EdIdx; i++ )
        {
            memset(QuerySub,0x00,sizeof(QuerySub));

            if( i < EdIdx -1 )
            {
                sprintf( QuerySub,"RSN_CD%d='%d' OR ", i+1, RsnCod );
            }
            else
            {
                sprintf( QuerySub,"RSN_CD%d='%d') ORDER BY CAST(SPAN_ST AS INTEGER) ASC,CAST(SPAN_ED AS INTEGER) ASC,CAST(MOJI_ID AS INTEGER) ASC;", i+1, RsnCod );
            }
            strcat( Query, QuerySub );

        }
        // DB検索
        memset( strResult, 0x00, sizeof(strResult));
        tmpRowCnt=GetData(pConn, (unsigned char*) Query, strResult );
        // 検索に失敗したら終了
        if( tmpRowCnt < 0 )
        {
            printX(TSK_SOUND,LOG_YES|PRT_YES,"付帯文章 期間あり路線コード 検索失敗 RowCnt = %d SQL:%s\n ",RowCnt,Query);
            return (-2);
        }
        memcpy( &strResult2[RowCnt], strResult, sizeof(SlctResult) * tmpRowCnt );
        RowCnt = RowCnt + tmpRowCnt;
    }

    // ソートしなおし
    for( i=0; i<RowCnt-1 ;i++ )
    {
        for( x=i+1; k<RowCnt; x++ );
        {
            // 開始日が 遅ければ入れ替える
            if(atoi(strResult2[i].Column[1]) > atoi(strResult2[x].Column[1]))
            {
                memset( &tmpResult, 0x00, sizeof(tmpResult));                           // バッファの初期化

                memcpy( &tmpResult, &strResult2[i], sizeof(tmpResult));                 // メモリの退避
                memcpy( &strResult2[i], &strResult2[x], sizeof(SlctResult));                // メモリのコピー
                memcpy( &strResult2[x], &tmpResult, sizeof(tmpResult));                 // メモリのコピー

            }
            // 開始日が同じならば終了日で比較する
            else if(atoi(strResult2[i].Column[1]) == atoi(strResult2[x].Column[1]))
            {
                // 終了日が大きければ データ入れ替え
                if(atoi(strResult2[i].Column[2]) > atoi(strResult2[x].Column[2]))
                {
                    memset( &tmpResult, 0x00, sizeof(tmpResult));                           // バッファの初期化

                    memcpy( &tmpResult, &strResult2[i], sizeof(tmpResult));                 // メモリの退避
                    memcpy( &strResult2[i], &strResult2[x], sizeof(SlctResult));                // メモリのコピー
                    memcpy( &strResult2[x], &tmpResult, sizeof(tmpResult));                 // メモリのコピー
                }
                // 終了日が同じであれば、文章番号で比較する
                else if(atoi(strResult2[i].Column[2]) == atoi(strResult2[x].Column[2]))
                {
                    // 文章番号が大きければデータ入れ替え
                    if(atoi(strResult2[i].Column[0]) > atoi(strResult2[x].Column[0]))
                    {
                        memset( &tmpResult, 0x00, sizeof(tmpResult));                           // バッファの初期化

                        memcpy( &tmpResult, &strResult2[i], sizeof(tmpResult));                 // メモリの退避
                        memcpy( &strResult2[i], &strResult2[x], sizeof(SlctResult));                // メモリのコピー
                        memcpy( &strResult2[x], &tmpResult, sizeof(tmpResult));                 // メモリのコピー
                    }
                }
            }
        }
    }


    // 当日/翌日ダイヤに反映
    for( i=0; i<RowCnt; i++ )
    {
        memset( Query, 0x00, sizeof(Query));
        sprintf( Query, "INSERT INTO BUST13 ( COD, DAYBYDAY, GOU_NO, MASHI, FUTAI_NO, SEQ ) "\
                                    " VALUES ( '%d','%08d', '%d','%d','%d', '%d' ); "
                                                ,binCod
                                                ,DateYMD
                                                ,gouNo
                                                ,mashi
                                                ,atoi( strResult2[i].Column[0])
                                                ,Seq);

        ret = Execute(pConn, (unsigned char*) Query );
        if( ret < 0 )
        {
            printX(TSK_SOUND, LOG_YES|PRT_YES,"[%s]付帯文章データ更新失敗(期間あり 路線コード) ret = %d 文章番号 = %d SQL:%s\n",__FUNCTION__,ret, atoi( strResult2[i].Column[0]) ,Query);
            continue;
        }
        Seq++;                      // シーケンス番号

        // 付帯文章が最大に達したら終了
        if( Seq == HUTAI_MAX )
        {
            return Seq;
        }
    }

    // ③ 付帯案内文章 期間あり(期間中) 便コード
    // SQL作成
    // 2回にわけてとる
    memset(strResult2,0x00,sizeof(strResult2));
    RowCnt = 0;
    for( k=0; k<2; k++ )
    {

        // WHERE句の開始・終了INDEXを決める
        if( k==0 )
        {
            StIdx = 0;
            EdIdx = BINCD_MAX / 2;
        }
        else
        {
            StIdx = BINCD_MAX / 2;
            EdIdx = BINCD_MAX;
        }

        memset( Query, 0x00, sizeof(Query));
        sprintf( Query,"SELECT MOJI_ID , SPAN_ST, SPAN_ED FROM BUST14"\
                        " WHERE GEN_KAI='%d' AND CAST(SPAN_ST AS INTEGER)<='%08d' AND '%08d'<=CAST(SPAN_ED AS INTEGER)"\
                        " AND((SPAN_ST<>'00000000')AND(SPAN_ED<>'99999999')) "
                        ,KaiseiFlg, DateYMD, DateYMD );
        strcat( Query, "AND(");
        for( i=StIdx; i < EdIdx; i++ )
        {
            memset(QuerySub,0x00,sizeof(QuerySub));
            if( i < EdIdx -1 )
            {
                sprintf( QuerySub,"COD%d='%d' OR ", i+1, binCod );
            }
            else
            {
                sprintf( QuerySub,"COD%d='%d') ORDER BY CAST(SPAN_ST AS INTEGER) ASC,CAST(SPAN_ED AS INTEGER) ASC,CAST(MOJI_ID AS INTEGER) ASC;", i+1, binCod );
            }
            strcat( Query, QuerySub );

        }
        // DB検索
        memset( strResult, 0x00, sizeof(strResult));
        tmpRowCnt=GetData(pConn, (unsigned char*) Query, strResult );
        // 検索に失敗したら終了
        if( tmpRowCnt < 0 )
        {
            printX(TSK_SOUND,LOG_YES|PRT_YES,"付帯文章 期間あり 便コード  検索失敗 RowCnt = %d SQL:%s\n ",RowCnt,Query);
            return (-3);
        }
        memcpy( &strResult2[RowCnt], strResult, sizeof(SlctResult) * tmpRowCnt );
        RowCnt = RowCnt + tmpRowCnt;
    }
    // ソートしなおし
    for( i=0; i<RowCnt-1 ;i++ )
    {
        for( x=i+1; x<RowCnt; x++ );
        {
            // 開始日が 遅ければ入れ替える
            if(atoi(strResult2[i].Column[1]) > atoi(strResult2[x].Column[1]))
            {
                memset( &tmpResult, 0x00, sizeof(tmpResult));                           // バッファの初期化

                memcpy( &tmpResult, &strResult2[i], sizeof(tmpResult));                 // メモリの退避
                memcpy( &strResult2[i], &strResult2[x], sizeof(SlctResult));                // メモリのコピー
                memcpy( &strResult2[x], &tmpResult, sizeof(tmpResult));                 // メモリのコピー

            }
            // 開始日が同じならば終了日で比較する
            else if(atoi(strResult2[i].Column[1]) == atoi(strResult2[x].Column[1]))
            {
                // 終了日が大きければ データ入れ替え
                if(atoi(strResult2[i].Column[2]) > atoi(strResult2[x].Column[2]))
                {
                    memset( &tmpResult, 0x00, sizeof(tmpResult));                           // バッファの初期化

                    memcpy( &tmpResult, &strResult2[i], sizeof(tmpResult));                 // メモリの退避
                    memcpy( &strResult2[i], &strResult2[x], sizeof(SlctResult));                // メモリのコピー
                    memcpy( &strResult2[x], &tmpResult, sizeof(tmpResult));                 // メモリのコピー
                }
                // 終了日が同じであれば、文章番号で比較する
                else if(atoi(strResult2[i].Column[2]) == atoi(strResult2[x].Column[2]))
                {
                    // 文章番号が大きければデータ入れ替え
                    if(atoi(strResult2[i].Column[0]) > atoi(strResult2[x].Column[0]))
                    {
                        memset( &tmpResult, 0x00, sizeof(tmpResult));                           // バッファの初期化

                        memcpy( &tmpResult, &strResult2[i], sizeof(tmpResult));                 // メモリの退避
                        memcpy( &strResult2[i], &strResult2[x], sizeof(SlctResult));                // メモリのコピー
                        memcpy( &strResult2[x], &tmpResult, sizeof(tmpResult));                 // メモリのコピー
                    }
                }
            }
        }
    }

    // 当日/翌日ダイヤに反映
    for( i=0; i<RowCnt; i++ )
    {
        memset( Query, 0x00, sizeof(Query));
        sprintf( Query, "INSERT INTO BUST13 ( COD, DAYBYDAY, GOU_NO, MASHI, FUTAI_NO, SEQ ) "\
                                    " VALUES ( '%d','%08d', '%d','%d','%d', '%d' ); "
                                                ,binCod
                                                ,DateYMD
                                                ,gouNo
                                                ,mashi
                                                ,atoi( strResult2[i].Column[0])
                                                ,Seq);

        ret = Execute(pConn, (unsigned char*) Query );
        if( ret < 0 )
        {
            printX(TSK_SOUND, LOG_YES|PRT_YES,"[%s]付帯文章データ更新失敗(期間あり 便コード) ret = %d 文章番号 = %d SQL:%s\n",__FUNCTION__,ret, atoi( strResult2[i].Column[0]) ,Query);
            continue;
        }
        Seq++;                      // シーケンス番号

        // 付帯文章が最大に達したら終了
        if( Seq == HUTAI_MAX )
        {
            return Seq;
        }
    }

    // ④ 付帯案内文章 期間なし         路線コード
    // SQL作成
    // 2回に分けてとる
    memset(strResult2,0x00,sizeof(strResult2));
    RowCnt = 0;
    for( k=0; k<2; k++ )
    {

        // WHERE句の開始・終了INDEXを決める
        if( k==0 )
        {
            StIdx = 0;
            EdIdx = RSNCD_MAX / 2;
        }
        else
        {
            StIdx = RSNCD_MAX / 2;
            EdIdx = RSNCD_MAX;
        }

        memset( Query, 0x00, sizeof(Query));
        sprintf( Query,"SELECT MOJI_ID FROM BUST14"\
                        " WHERE GEN_KAI='%d' AND SPAN_ST='00000000' AND SPAN_ED='99999999'"
                        ,KaiseiFlg, DateYMD, DateYMD );
        strcat( Query, " AND(");
        for( i=StIdx; i < EdIdx; i++ )
        {
            memset(QuerySub,0x00,sizeof(QuerySub));
            if( i < EdIdx -1 )
            {
                sprintf( QuerySub,"RSN_CD%d='%d' OR ", i+1, RsnCod );
            }
            else
            {
                sprintf( QuerySub,"RSN_CD%d='%d') ORDER BY CAST(MOJI_ID AS INTEGER) ASC;", i+1, RsnCod );
            }
            strcat( Query, QuerySub );

        }
        // DB検索
        memset( strResult, 0x00, sizeof(strResult));
        tmpRowCnt=GetData(pConn, (unsigned char*) Query, strResult );
        // 検索に失敗したら終了
        if( RowCnt < 0 )
        {
            printX(TSK_SOUND,LOG_YES|PRT_YES,"付帯文章 期間あり路線コード 検索失敗 RowCnt = %d SQL:%s\n ",RowCnt,Query);
            return (-4);
        }
        memcpy( &strResult2[RowCnt], strResult, sizeof(SlctResult) * tmpRowCnt );
        RowCnt = RowCnt + tmpRowCnt;
    }

    // ソートしなおし
    for( i=0; i<RowCnt-1 ;i++ )
    {
        for( x=i+1; x<RowCnt; x++ );
        {
            // 開始日が 遅ければ入れ替える
            if(atoi(strResult2[i].Column[1]) > atoi(strResult2[x].Column[1]))
            {
                memset( &tmpResult, 0x00, sizeof(tmpResult));                           // バッファの初期化

                memcpy( &tmpResult, &strResult2[i], sizeof(tmpResult));                 // メモリの退避
                memcpy( &strResult2[i], &strResult2[x], sizeof(SlctResult));                // メモリのコピー
                memcpy( &strResult2[x], &tmpResult, sizeof(tmpResult));                 // メモリのコピー

            }
            // 開始日が同じならば終了日で比較する
            else if(atoi(strResult2[i].Column[1]) == atoi(strResult2[x].Column[1]))
            {
                // 終了日が大きければ データ入れ替え
                if(atoi(strResult2[i].Column[2]) > atoi(strResult2[x].Column[2]))
                {
                    memset( &tmpResult, 0x00, sizeof(tmpResult));                           // バッファの初期化

                    memcpy( &tmpResult, &strResult2[i], sizeof(tmpResult));                 // メモリの退避
                    memcpy( &strResult2[i], &strResult2[x], sizeof(SlctResult));                // メモリのコピー
                    memcpy( &strResult2[x], &tmpResult, sizeof(tmpResult));                 // メモリのコピー
                }
                // 終了日が同じであれば、文章番号で比較する
                else if(atoi(strResult2[i].Column[2]) == atoi(strResult2[x].Column[2]))
                {
                    // 文章番号が大きければデータ入れ替え
                    if(atoi(strResult2[i].Column[0]) > atoi(strResult2[x].Column[0]))
                    {
                        memset( &tmpResult, 0x00, sizeof(tmpResult));                           // バッファの初期化

                        memcpy( &tmpResult, &strResult2[i], sizeof(tmpResult));                 // メモリの退避
                        memcpy( &strResult2[i], &strResult2[x], sizeof(SlctResult));                // メモリのコピー
                        memcpy( &strResult2[x], &tmpResult, sizeof(tmpResult));                 // メモリのコピー
                    }
                }
            }
        }
    }

    // 当日/翌日ダイヤに反映
    for( i=0; i<RowCnt; i++ )
    {
        memset( Query, 0x00, sizeof(Query));
        sprintf( Query, "INSERT INTO BUST13 ( COD, DAYBYDAY, GOU_NO, MASHI, FUTAI_NO, SEQ ) "\
                                    " VALUES ( '%d','%08d', '%d','%d','%d', '%d' ); "
                                                ,binCod
                                                ,DateYMD
                                                ,gouNo
                                                ,mashi
                                                ,atoi(strResult2[i].Column[0])
                                                ,Seq);

        ret = Execute(pConn, (unsigned char*) Query );
        if( ret < 0 )
        {
            printX(TSK_SOUND, LOG_YES|PRT_YES,"[%s]付帯文章データ更新失敗(期間あり 路線コード) ret = %d 文章番号 = %d SQL:%s\n",__FUNCTION__,ret, atoi( strResult2[i].Column[0]) ,Query);
            continue;
        }
        Seq++;                      // シーケンス番号

        // 付帯文章が最大に達したら終了
        if( Seq == HUTAI_MAX )
        {
            return Seq;
        }
    }


    // ⑤ 付帯案内文章 期間なし         便コード
    memset( Query, 0x00, sizeof(Query));
    sprintf( Query,"SELECT MOJI_ID FROM BUST14 "\
                    " WHERE GEN_KAI='%d' AND SPAN_ST='00000000' AND SPAN_ED='99999999' "
                    ,KaiseiFlg );
    strcat( Query, "AND(");
    for( i=0; i < BINCD_MAX; i++ )
    {
        memset(QuerySub,0x00,sizeof(QuerySub));
        if( i < BINCD_MAX -1 )
        {
            sprintf( QuerySub,"COD%d='%d' OR ", i+1, RsnCod );
        }
        else
        {
            sprintf( QuerySub,"COD%d='%d') ORDER BY CAST(MOJI_ID AS INTEGER) ASC;", i+1, RsnCod );
        }
        strcat( Query, QuerySub );

    }
    // DB検索
    memset( strResult, 0x00, sizeof(strResult));
    RowCnt=GetData(pConn, (unsigned char*) Query, strResult );
    // 検索に失敗したら終了
    if( RowCnt < 0 )
    {
        printX(TSK_SOUND,LOG_YES|PRT_YES,"付帯文章 期間あり 便コード  検索失敗 RowCnt = %d SQL:%s\n ",RowCnt,Query);
        return (-5);
    }

    // 当日/翌日ダイヤに反映
    for( i=0; i<RowCnt; i++ )
    {
        memset( Query, 0x00, sizeof(Query));
        sprintf( Query, "INSERT INTO BUST13 ( COD, DAYBYDAY, GOU_NO, MASHI, FUTAI_NO, SEQ ) "\
                                    " VALUES ( '%d','%08d', '%d','%d','%d', '%d' ); "
                                                ,binCod
                                                ,DateYMD
                                                ,gouNo
                                                ,mashi
                                                ,atoi( strResult[i].Column[0])
                                                ,Seq);

        ret = Execute(pConn, (unsigned char*) Query );
        if( ret < 0 )
        {
            printX(TSK_SOUND, LOG_YES|PRT_YES,"[%s]付帯文章データ更新失敗(期間あり 便コード) ret = %d 文章番号 = %d SQL:%s\n",__FUNCTION__,ret, atoi( strResult[i].Column[0]) ,Query);
            continue;
        }
        Seq++;                      // シーケンス番号

        // 付帯文章が最大に達したら終了
        if( Seq == HUTAI_MAX )
        {
            return Seq;
        }
    }

    return Seq;
}
/*********************************************************************
 * NAME         : snd_d202
 * FUNCTION     : データ管理タスクへの送信
 *              :
 * INPUT        :
 * RETURN       :
 * PROGRAMMED   :
 * DATE(ORG)    :
 * CALL         : なし
 * FILE         : なし
 * REMARKS      :
 ********************************************************************/
int snd_d202()
{

    FmtTCDD202      *Snd_p;
    unsigned long   Tcd;
    int             size;
    int             rc;                                         //メッソドの返却値

    //送信処理
    Snd_p = (FmtTCDD202*)&tcp_snd_dat;                          //送信エリアポインタ設定
    memset(&tcp_snd_dat,0x00,sizeof(tcp_snd_dat));              //送信エリアクリア
    Tcd = TCDD202;                                              //TCD
    size = sizeof(FmtTCDD202) - sizeof(FmtIDCHead);

    //全乗り場を設定する
    Snd_p->toujitu_noriba = ALL_NORIBA;

    Tcp_Hed_datmk(Tcd,size,&tcp_snd_dat);                       //TCPヘッダ作成
    rc = QueMsgSet((QINDEX_*)&shm_sys.que_sys->index[0][SOCKET_SND],(unsigned char*)&tcp_snd_dat);
    if ( rc == QUE_FULL ){                                      //キュー書込失敗
        rc = 1;
        printX(TSK_SOUND,LOG_YES|ERR_YES|PRT_YES,"***TCD SND NG(QUE_FULL TCD=%08X)\n",Tcd);
    } else {
        rc = 0;
        printX(TSK_SOUND,LOG_YES|PRT_YES,"TCD SND D202 OK\n");
    }
    return (rc);

}

/*********************************************************************
 * NAME         : setExclusionFlag
 * FUNCTION     : スケジュールの排他フラグの設定
 *              :
 * INPUT        : 排他フラグ
 * RETURN       : 0:排他設定成功
 *                1:排他設定失敗
 * PROGRAMMED   : KOCHI_TYOU
 * DATE(ORG)    : 2011.04.05
 * CALL         : なし
 * FILE         : なし
 * REMARKS      :
 ********************************************************************/
int setExclusionFlag(int exclusionFlag, char *pConn)
{

    int retStop = 0;                                        //更新メッソドの返却値
    int rowcnt = 0;                                         //検索メソッドの返却値
    int rc = 0;                                             //メソッドの返却値
    char    Query[MAX_SQL_LEN];                             //更新用SQL文
    memset(&Query[0], 0x00, sizeof(Query));

    //排他フラグON/OFFの設定（渡された識別子次第）
    sprintf(Query, UPDATE_EXCLUSION_FLAG,
            exclusionFlag );                                //排他フラグ
    //更新メソッド
    rowcnt=Execute(pConn, (unsigned char*) Query );

    if( rowcnt < 0 ) {
        rc = 1;
        printX(TSK_SOUND,LOG_YES|PRT_YES,"setExclusionFlag : スケジュールの排他設定失敗 \n");
    }
    return rc;
}

/*********************************************************************
 * NAME         : check_kaise_date
 * FUNCTION     : 翌日は改正日かどうかのチェック
 *              :
 * INPUT        : 翌日の日付
 * RETURN       : 0:翌日は改正日ではない
 *                1:翌日は改正日である
 * PROGRAMMED   : KOCHI_TYOU
 * DATE(ORG)    : 2011.04.06
 * CALL         : なし
 * FILE         : なし
 * REMARKS      :
 ********************************************************************/
int check_kaise_date(int sch_type, char *pConn)
{

    int rc = 0;                                         //メソッドの返却値
    int rowcnt = 0;                                     //検索メソッドの返却値
    int n = 0;                                          //ループ用
    char    Query[MAX_SQL_LEN];                         //更新用SQL文
    memset(&Query[0], 0x00, sizeof(Query));
    SlctResult  strResultCheck[1];                  //データ格納構造体(MAX_RESULTNUM == 128)

    memset(&strResultCheck[0], 0x00, sizeof(strResultCheck));
    char kayse_date[9];
    memset(&kayse_date[0], 0x00, sizeof(kayse_date));

    char sys_date[9];                                           //当日/翌日の日付
    memset(&sys_date[0], 0x00, sizeof(sys_date));

    //システム時間を取得する
    time( &nowtime );
    timeinfo = localtime( &nowtime );

    //識別子は当日スケジュールの場合
    if(sch_type == TOUJITU_SCH)
    {
        //当日の日付を取得する
        sprintf(sys_date,"%04d%02d%02d",timeinfo->tm_year+1900, timeinfo->tm_mon+1, timeinfo->tm_mday);
    }
    //識別子は翌日スケジュールの場合
    else if(sch_type == YOKUJITU_SCH)
    {
        //翌日の日付を取得する
        sprintf(sys_date,"%04d%02d%02d",timeinfo->tm_year+1900, timeinfo->tm_mon+1, timeinfo->tm_mday+1);
    }

    //改正日の取得
    sprintf(Query, SELECT_KAYSE_DATE);

    //検索メソッド
    rowcnt=GetData(pConn, (unsigned char*) Query, strResultCheck );

    if( rowcnt < 0 ) {
        rc = 0;
        printX(TSK_SOUND,LOG_YES|PRT_YES,"check_kaise_date : 改正日情報が指定なし \n");
    }
    else
    {
        for(n = 0; n < rowcnt; n++)
        {
            memcpy(kayse_date,strResultCheck[n].Column[0],sizeof(kayse_date));

            //翌日は改正日の場合
            if(strcmp(sys_date,kayse_date) == 0)
            {
                rc = 1;
                break;
            }
        }
    }
    return rc;
}

/*********************************************************************
 * NAME         : check_data
 * FUNCTION     : 取得したデータが「NULL」かどうかのチェック
 *              :
 * INPUT        : データ格納構造体
 *              : インデックス
 * RETURN       : 0:DB中のデータが正常である
 *                1:DB中のデータが異常である
 * PROGRAMMED   : KOCHI_TYOU
 * DATE(ORG)    : 2011.04.06
 * CALL         : なし
 * FILE         : なし
 * REMARKS      :
 ********************************************************************/
int check_data(SlctResult   *strResult,int cnt)
{

    int m = 0;                                              //ループ用変数
    int rc = 0;                                             //メソッドの返却値

    //便コード、号車が「NULL」の場合
    if(strResult[cnt].Column[0] == NULL || strResult[cnt].Column[1] == NULL)
    {
        int rc = 1;
        return rc;
    }

    //増表示が「NULL」の場合
    if(strResult[cnt].Column[2] == NULL)
    {
        strcpy(strResult[cnt].Column[2],"0");
    }

    return rc;
}

/*********************************************************************
 * NAME         : check_bin_data
 * FUNCTION     : エラー検知チェック
 *              :
 * INPUT        : データ格納構造体
 *              : インデックス
 *              : 当日・翌日フラグ
 * RETURN       : 0:データが正常である
 *                1:データが異常である
 * PROGRAMMED   : KOCHI_TYOU
 * DATE(ORG)    : 2011.04.21
 * CALL         : なし
 * FILE         : なし
 * REMARKS      :
 ********************************************************************/
int check_bin_data(SlctResult   *strResult,int cnt, int type, char *pConn){

    int m = 0;                                              //ループ用変数
    int rc = 0;                                             //メソッドの返却値
    char    InsertQuery[MAX_SQL_LEN];                       //追加用SQL文
    int ret = 0;                                            //追加用メソッドの返却値

    //便コード、号車が「NULL」の場合
    if(strResult[cnt].Column[0] == NULL || strResult[cnt].Column[1] == NULL)
    {
        return 1;
    }

    //増表示が「NULL」の場合
    if(atoi(strResult[cnt].Column[2]) == 0)
    {
        strcpy(strResult[cnt].Column[2],"0");
    }

    //エラー種別:乗り場と乗車口が未設定である
    if(atoi(strResult[cnt].Column[8]) == 0 ||               //乗り場が未設定である
        atoi(strResult[cnt].Column[9]) == 0)                //乗車口が未設定である
    {
        if(type == YOKUJITU_SCH)
        {
            return 1;
        }
        memset(&InsertQuery[0], 0x00, sizeof(InsertQuery));
        sprintf(InsertQuery, INSERT_ERROR_DATA,
                strResult[cnt].Column[0],                           /* 便コード             :   COD             */
                strResult[cnt].Column[1],                           /* 号車                 :   GOU_NO          */
                atoi(strResult[cnt].Column[2]),                     /* 増表示               :   MASHI           */
                ERROR_TYPE_3                                        /* エラーコード         :   ERROR_COD       */
                );

        //当日/翌日の便データの追加
        ret = Execute(pConn, (unsigned char*)InsertQuery);

        if (ret < 0)
        {
            printX(TSK_SOUND,LOG_YES|PRT_YES,"[%s] エラーコード追加NG(sql=%s)\n", __FUNCTION__, InsertQuery);
        }
        return 1;
    }

    for(m = 0; m < cnt; m++)
    {

        //エラー種別:同一運行日かつ発車時刻に、同一乗り場が設定されている
        if(strcmp(strResult[m].Column[12],strResult[cnt].Column[12]) == 0 &&        //同一発車時刻
            strcmp(strResult[m].Column[8],strResult[cnt].Column[8]) == 0            //同一乗り場
            )
        {
            if(type == YOKUJITU_SCH)
            {
                return 1;
            }

            memset(&InsertQuery[0], 0x00, sizeof(InsertQuery));
            sprintf(InsertQuery, INSERT_ERROR_DATA,
                    strResult[cnt].Column[0],                           /* 便コード             :   COD             */
                    strResult[cnt].Column[1],                           /* 号車                 :   GOU_NO          */
                    atoi(strResult[cnt].Column[2]),                     /* 増表示               :   MASHI           */
                    ERROR_TYPE_1                                        /* エラーコード         :   ERROR_COD       */
                    );

            //当日/翌日の便データの追加
            ret = Execute(pConn, (unsigned char*)InsertQuery);

            if (ret < 0)
            {
                printX(TSK_SOUND,LOG_YES|PRT_YES,"[%s] エラーコード追加NG(sql=%s)\n", __FUNCTION__, InsertQuery);
            }
            rc = 1;
            break;
        }
    }

    return rc;
}

/*********************************************************************
 * NAME         : check_bin_data
 * FUNCTION     : エラー検知チェック
 *              :
 * INPUT        : データ格納構造体
 *              : インデックス
 *              : 当日・翌日フラグ
 * RETURN       : 0:データが正常である
 *                1:データが異常である
 * PROGRAMMED   : KOCHI_TYOU
 * DATE(ORG)    : 2011.04.21
 * CALL         : なし
 * FILE         : なし
 * REMARKS      :
 ********************************************************************/
int check_bin_mst_data(int type, char *pConn)
{

    int Rowcnt = 0;                                         //検索メソッドの返却値
    int Rowcnt_Check;                                       //検索メソッドの返却値
    int m = 0;                                              //ループ用変数
    int ret = 0;                                            //スケジュール追加用メソッドの返却値
    int rc = 0;                                             //メソッドの返却値
    memset(&strResult[0], 0x00, sizeof(strResult));
    SlctResult  strCheckResult[MAX_RESULTNUM];              //データ格納構造体(MAX_RESULTNUM == 128)
    char    SelectQuery[MAX_SQL_LEN];                       //検索用SQL文
    memset(&SelectQuery[0], 0x00, sizeof(SelectQuery));
    char    QueryCheck[MAX_SQL_LEN];                        //検索用SQL文
    char    InsertQuery[MAX_SQL_LEN];                       //追加用SQL文

    sprintf(SelectQuery, SELECT_BIN_MST_DATA);
    //検索メソッド
    Rowcnt=GetData(pConn, (unsigned char*) SelectQuery, strResult );
    if( Rowcnt <= 0 ) {
        printX(TSK_SOUND,LOG_YES|PRT_YES,"[%s] 基本便データがなし(sql=%s)\n", __FUNCTION__, SelectQuery);
        rc = 1;
    }
    else
    {
        //基本データから取得した結果を元に、当日/翌日のスケジュールを作成する
        for(m = 0; m < Rowcnt; m++)
        {
            rc = 0;
            memset(&QueryCheck[0], 0x00, sizeof(QueryCheck));
            memset(&strCheckResult[0], 0x00, sizeof(strCheckResult));
            Rowcnt_Check = 0;

            sprintf(QueryCheck, "select cod from bust01 where cod = '%s' and gen_kai = '%d' and gou_no = '%s' and mashi = '%d';",
                                strResult[m].Column[0],atoi(strResult[m].Column[5]),strResult[m].Column[1],atoi(strResult[m].Column[2]));

            Rowcnt_Check = GetData(pConn, (unsigned char*)QueryCheck, strCheckResult);                  //検索メソッド

            if (atoi(strCheckResult[0].Column[0]) == 0)
            {
                printX(TSK_SOUND,LOG_NO|PRT_YES,"check_bin_mst_data()：便マスタにデータがなし \n");
                rc = 1;
            }
            else
            {

                memset(&QueryCheck[0], 0x00, sizeof(QueryCheck));
                memset(&strCheckResult[0], 0x00, sizeof(strCheckResult));
                sprintf(QueryCheck, "select count(*) from bust09 where cod = '%s' and gen_kai = '%d' ;",
                                    strResult[m].Column[3],atoi(strResult[m].Column[5]));


                Rowcnt_Check = GetData(pConn, (unsigned char*)QueryCheck, strCheckResult);              //検索メソッド
                if (atoi(strCheckResult[0].Column[0]) == 0)
                {
                    printX(TSK_SOUND,LOG_NO|PRT_YES,"check_bin_mst_data()：方面マスタにデータがなし \n");
                    rc = 1;
                }
                else
                {
                    memset(&QueryCheck[0], 0x00, sizeof(QueryCheck));
                    memset(&strCheckResult[0], 0x00, sizeof(strCheckResult));
                    sprintf(QueryCheck, "select count(*) from bust10 where cod = '%s' and gen_kai = '%d' ;",
                                        strResult[m].Column[4],atoi(strResult[m].Column[5]));


                    Rowcnt_Check = GetData(pConn, (unsigned char*)QueryCheck, strCheckResult);          //検索メソッド
                    if (atoi(strCheckResult[0].Column[0]) == 0)
                    {
                        printX(TSK_SOUND,LOG_NO|PRT_YES,"check_bin_mst_data()：行き先マスタにデータがなし \n");
                        rc = 1;
                    }
                }
            }
            if(rc == 1)
            {
                if(type == YOKUJITU_SCH)
                {
                    return 1;
                }
                memset(&InsertQuery[0], 0x00, sizeof(InsertQuery));
                sprintf(InsertQuery, INSERT_ERROR_DATA,
                        strResult[m].Column[0],                             /* 便コード             :   COD             */
                        strResult[m].Column[1],                             /* 号車                 :   GOU_NO          */
                        atoi(strResult[m].Column[2]),                       /* 増表示               :   MASHI           */
                        ERROR_TYPE_2                                        /* エラーコード         :   ERROR_COD       */
                        );

                //当日/翌日の便データの追加
                ret = Execute(pConn, (unsigned char*)InsertQuery);

                if (ret < 0)
                {
                    printX(TSK_SOUND,LOG_YES|PRT_YES,"[%s] エラーコード追加NG(sql=%s)\n", __FUNCTION__, InsertQuery);
                }
            }
        }
    }

    return rc;
}

/*********************************************************************
 * NAME         : error_data_clear
 * FUNCTION     : エラーテーブル中のデータをクリアする
 *              :
 * INPUT        : 当日・翌日フラグ
 * RETURN       : なし
 * PROGRAMMED   : KOCHI_TYOU
 * DATE(ORG)    : 2011.04.22
 * CALL         : なし
 * FILE         : なし
 * REMARKS      :
 ********************************************************************/
void error_data_clear(int type, char *pConn)
{

    if(type == YOKUJITU_SCH)
    {
        return;
    }

    int binRowcnt = 0;
    char binQuery[MAX_SQL_LEN];
    memset(&binQuery[0], 0x00, sizeof(binQuery));


    //エラーテーブル中のデータを削除する
    sprintf(binQuery,"delete from BUST17; ");

    binRowcnt = Execute(pConn, (unsigned char*)binQuery);       //削除メソッド
    if (binRowcnt < 0)
    {
        printX(TSK_SOUND,LOG_YES|PRT_YES,"[%s] データ削除NG(sql=%s)\n", __FUNCTION__, binQuery);
    }

}

/*********************************************************************
 * NAME         : delete_genyo_data
 * FUNCTION     : 現用の基本データを削除する
 *              :
 * RETURN       : なし
 * PROGRAMMED   : KOCHI_TYOU
 * DATE(ORG)    : 2011.04.22
 * CALL         : なし
 * FILE         : なし
 * REMARKS      :
 ********************************************************************/
void delete_genyo_data(char *pConn)
{

    int binRowcnt = 0;
    char binQuery[MAX_SQL_LEN];
    memset(&binQuery[0], 0x00, sizeof(binQuery));

    //現用の基本データを削除する
    sprintf(binQuery,"delete from BUST03 where gen_kai = '0';");

    binRowcnt = Execute(pConn, (unsigned char*)binQuery);       //削除メソッド
    if (binRowcnt < 0)
    {
        printX(TSK_SOUND,LOG_YES|PRT_YES,"[%s] 現用の基本データ削除NG(sql=%s)\n", __FUNCTION__, binQuery);
    }
}

/*********************************************************************
 * NAME         : set_genyo_data
 * FUNCTION     : 改正用データから現用の基本データを作る
 *              :
 * RETURN       : なし
 * PROGRAMMED   : KOCHI_TYOU
 * DATE(ORG)    : 2011.04.22
 * CALL         : なし
 * FILE         : なし
 * REMARKS      :
 ********************************************************************/
void set_genyo_data(char *pConn)
{

    int binRowcnt = 0;
    char binQuery[MAX_SQL_LEN];
    memset(&binQuery[0], 0x00, sizeof(binQuery));

    //現用の基本データを削除する
    sprintf(binQuery,"update BUST03 set gen_kai = '0';");

    binRowcnt = Execute(pConn, (unsigned char*)binQuery);       //削除メソッド
    if (binRowcnt < 0)
    {
        printX(TSK_SOUND,LOG_YES|PRT_YES,"[%s] 改正用データから現用の基本データ作成NG(sql=%s)\n", __FUNCTION__, binQuery);
    }
}

