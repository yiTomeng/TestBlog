/******************************************************************************
 * SYSTEM       :   西JR BUS音声放送LIB
 * PROGRAM      :   放送を行うライブラリ
 * MODULE       :   sound_lib.h 
 * VERSION      : 	1.00.00 
 * REMARKS      :
 * HISTORY      :
 * ID-----------DATE--------NAME----------NOTE --------------------------------
 * [V01.00.00]  2015.2.18   KOCHI.晏       初期作成
 *****************************************************************************/
#ifndef SOUND_LIB_H
#define SOUND_LIB_H
#include <alsa/asoundlib.h>
#include "printff.h"
#include "sound_lib_err.h"

#define FILE_NAME 32					//ファイル名サイズ

//音声情報
typedef struct st_vol_def
{
	int left_vol;						//左音声チャンネル
	int right_vol;						//右音声チェンネル
}st_vol;


extern int open_device(snd_pcm_t **handle, int task_no);			//デバイスを開く
extern close_device(snd_pcm_t *handle);								//デバイスを閉じる
extern int play_file(snd_pcm_t *handler, const char *file_name, st_vol vol, int refresh_cnt);	//音量と再生回数を設定し、特定音声ファイルを再生

#endif
