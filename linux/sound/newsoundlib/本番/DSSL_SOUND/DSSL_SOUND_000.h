/*
 *****************************************************************************
 * MODULE       : DSSL_SOUND.h
 * FUNCTION     : スケジュール管理ファイル処理共通ヘッダ
 * VERSION      : 1.00.00
 * REMARKS      :
 * HISTORY      :
 * ID-----------DATE--------NAME----------NOTE -------------------------------
 * [V01.00.00]	2011.3.11	KOCHI    初期作成
 *****************************************************************************
 */
#ifndef _DSSL_SCH_000_H__
#define _DSSL_SCH_000_H__


#include <time.h>
#include "defmil40.h"
#include "system.h"

#ifndef     LINTER_DBF_MODE
#define     LINTER_DBF_MODE			1
#endif


#define		CLIENT_MAX					1								//接続クライアントＭＡＸ

#define		DefLdSchNum					2								//表示器で表示するスケジュールデータの種類数
#define		DefSchMax					(DefLdSchNum*DefLdDspFileMax)	//スケジュールデータの数
#define     DefSchDatNum				(128)								//スケジュールデータの最大数
#define		TOUJITU_SCH				0					/* 当日スケジュール */
#define		YOKUJITU_SCH			1					/* 翌日スケジュール */
#define		ALL_NORIBA				0					/* 全乗り場 */
#define		EXCLUSIONFLAG_ON		1					/* 排他フラグON */
#define		EXCLUSIONFLAG_OFF		0					/* 排他フラグOFF */
#define		ONE_DAY_SEC				( 24 * 60 * 60 )	// 1日の秒数

// 付帯文章関係
#define		HUTAI_MAX				18					// 付帯文章の最大数
#define		RSNCD_MAX				75					// 路線コードの最大数
#define		BINCD_MAX				50					// 便コードの最大数

//----------------------------------------------------------
//共有メモリ

// LCD表示器に静止画スケジュールデータ
typedef	struct {
	FmtDayTim		Start;								/* 表示開始時刻							*/
	FmtDayTim		End;								/* 表示終了時刻							*/
	unsigned char   FileName[DefFileNameSize];			/* 静止画ファイル名						*/
} FmtLdDspSchDat;

typedef struct {
	UWORD           SchNum;								/* スケジュールデータの数					*/
	UWORD			CurDspNum;							/* 現在表示中の静止画数					*/
	UWORD			CurDspIndex[DefLdDspFileMax];		/* 現在表示中の静止画データのインデックス	*/
	FmtLdDspSchDat	SchDat[DefSchDatNum];				/* スケジュールデータ						*/
}FmtLdDspSchDatTable;




//日替わりデータ
typedef struct {
	unsigned char	ChangeFlg;			/* 内部日替わりフラグ 0:なし 1:あり */
    unsigned char   dummy[1];			/* */
    unsigned short  DayChgProcTime;		/* 日替わり処理実行時刻 */
	unsigned long	ChangeTime;			/* 日替わり時刻 */
	
} FmtDayChg;




//スケジュールデータ（スケジュール種別分)
typedef struct {
	FmtDayChg				DayChgDat;				//日替わりデータ
	FmtLdDspSchDatTable		SchDatTbl[DefLdSchNum];	//スケジュールテーブル index0:１・２番線　index1:３・４番線	
	FmtComEkiCd				PrmEki;					//駅データ
	long					SysTimeSabunLong;		// システム時刻差分
	int						FlgReLoadSchData;
	
	FmtDayTim				Now_Day;				//日替わりチェック用に現在のシステム時刻日付(3-26時表記)を保持する
													// sch_daychg_check で使用する
	
} FmtTshm;

extern	FmtTshm		*Tshm;							/* 共有メモリアドレス */

extern	void			sch_common_timeSYS(time_t * t);
extern	int				GetNowOver24H(FmtDayTim * Now);
extern	void			sch_picdisp_timer_check(void);								//静止画案内表示スケジュールチェック

#endif





