/******************************************************************************
 * SYSTEM       :   音声放送LIB
 * PROGRAM      :   放送を行うライブラリ
 * MODULE       :   sound_lib.h 
 * VERSION      : 	1.00.00 
 * REMARKS      :
 * HISTORY      :
 * ID-----------DATE--------NAME----------NOTE --------------------------------
 * [V01.00.00]  2015.2.2   晏       初期作成
 *****************************************************************************/

#ifndef SOUND_LIB
#define SOUND_LIB

#define DEFAULT_PLAY_TIMES 1														//デフォルト再生回数：1回

//#define play_wave_default(filename) play_wave(filename, DEFAULT_PLAY_TIMES)		//デフォルト再生回数の関数
#define FILENAME_SIZE		32														//ファイル名バッファサイズ

//エラーコード
enum ERROR_CODE
{
	ERR_OK,
	ERR_FILE_OPEN,
	ERR_FILE_READ,
	ERR_FILE_WRITE,
	ERR_DEVICE_OPEN,
	ERR_DEVICE_READ,
	ERR_DEVICE_WRITE,
	ERR_DEVICE_SET,
	ERR_BROADCAST,
	ERR_SIZE,
	ERR_NOT_RIFF,
	ERR_NOT_WAVE,
	ERR_NOT_FIND_FMT_CHUNK,
	ERR_NOT_FIND_DATA_CHUNK,
	ERR_NOT_FIND_DATA,
	ERR_NOT_PCM,
	ERR_BITS_PER_SAMPLE,
	ERR_SET_FMT,
	ERR_WRITE_RATE,
	ERR_WRITE_CHANNEL	
};

extern int play_wave(const char *filename, int refresh_times);						//wavファイルを再生する関数


#endif
