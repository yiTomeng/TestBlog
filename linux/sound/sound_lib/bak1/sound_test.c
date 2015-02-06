/******************************************************************************
 * SYSTEM       :   音声放送LIB テスト
 * PROGRAM      :   放送を行うライブラリ
 * MODULE       :   sound_test.c  ※ｿｰｽﾌｧｲﾙ名
 * REMARKS      :
 * HISTORY      :
 * ID-----------DATE--------NAME----------NOTE --------------------------------
 * [V01.00.00]  2015.2.2    晏       初期作成
 *****************************************************************************/
 
 
/**********INCLUDE AREA***********/
#include <stdio.h>
#include <unistd.h>
#include "sound_lib.h"


/*SOH*************************************************************************
 * NAME         : main
 * FUNCTION     : 単体テスト
 * VERSION      : 01.00.00
 * IN/OUT       : (i/ ) argc: パラメータ
 *              : (i/ ) argv : パラメータ配列
 * RETURN       : 0:OK <0:NG　※　リターン値の説明
 * REMARKS      : 備考
 * HISTORY      :
 * ID-----------DATE--------NAME----------NOTE --------------------------------
 * [V01.00.00]  2015.02.02  晏       
  *************************************************************************EOH*/

int main(int argc, char *argv[])
{
	//再生回数の格納先
	int refresh_times = 0;
	
	printf("再生回数を入力してください:\n");
	
	scanf("%d", &refresh_times);
	
	printf("再生回数は%d回\n", refresh_times);
	/*
	//デフォールト再生回数を使う関数のテスト
	if (DEFAULT_PLAY_TIMES == refresh_times)
	{
		if ( -1 == play_wave_default(argv[1]))
		{
			return -1;
		}
	}
	//普通の関数のテスト
	else
	{
		if ( -1 == play_wave(argv[1], refresh_times))
		{
			return -1;
		}
	}
	*/
	
	if ( ERR_OK != play_wave(argv[1], refresh_times))
	{
		return -1;
	}
	
	printf("音声ファイル再生：OVER!\n");
	
	return 0;
}
