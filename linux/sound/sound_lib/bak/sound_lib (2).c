/******************************************************************************
 * SYSTEM       :   音声放送LIB
 * PROGRAM      :   放送を行うライブラリ
 * MODULE       :   sound_lib.c  
 * REMARKS      :
 * HISTORY      :
 * ID-----------DATE--------NAME----------NOTE --------------------------------
 * [V01.00.00]  2015.2.2   晏       初期作成
 *****************************************************************************/

#include <fcntl.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/soundcard.h>

#include "sound_lib.h"

#define HEADER_BUFSIZE		36												//ヘッダファイルのバッファサイズ
#define BUFSIZE				64000											//ファイル読み込む用バッファサイズ
#define ERR_MSG_SIZE		64
//#define FILENAME_SIZE		32												//ファイル名バッファサイズ
#define DSP_DEVICE			"/dev/dsp"										//音声再生用デバイス

//WAVファイルのヘッダ
typedef struct
{
	char	chunk_id[4];													//識別子"RIFF"
	int		chunk_size;														//ファイルのサイズ(識別子"RIFF"と自分を除く)
	char 	format[4];														//フォーマット部("WAVE")
	char 	sub_chunk_id[4];												//識別子"fmt "
	int  	sub_chunk_size;													//この後のファイルのサイズ
	short 	audio_format;													//音声のフォーマット(1:PCM 　1以外は他の圧縮ファイル)
	short 	num_channels;													//チャンネル
	int 	sample_rate;													//サンプリング周波数
	int 	byte_rate;														//1秒に使用するバイト数
	short 	block_align;													//ブロックのアラインメント(今使っていない)
	short 	bits_per_sample;												//量子化のビット数
}wave_file_header_info;

//WAVファイルのインフォーメーション
typedef struct
{
	unsigned int play_start_pos;											//データ開始位置
	unsigned int play_pos;													//停止場所(保留　今使っていない)
	FILE 	*fp;															//音声ファイル識別子
	char 	play_filename[FILENAME_SIZE];									//音声ファイル名
	int 	stop;															//中止状態(保留　今使っていない)
	int 	refresh_times;													//再生回数
	wave_file_header_info wave_header;										//WAVファイルヘッダ
	int 	data_size;	 													//データ部のサイズ
}play_wave_info;


static const char err_msg[][ERR_MSG_SIZE] = {
	[ERR_OK] 					= "エラー無し。",
	[ERR_FILE_OPEN] 			= "ファイルopen失敗しました。",
	[ERR_FILE_READ] 			= "ファイルの読み込みは失敗しました。",
	[ERR_FILE_WRITE] 			= "",
	[ERR_DEVICE_OPEN] 			= "デバイスopen失敗しました。",
	[ERR_DEVICE_READ] 			= "",
	[ERR_DEVICE_WRITE] 			= "DSPデバイスに書き込むことが失敗しました。",
	[ERR_DEVICE_SET] 			= "DSPデバイス設定失敗しました。",
	[ERR_BROADCAST] 			= "放送失敗しました。",
	[ERR_SIZE] 					= "ファイルサイズはヘッダサイズより小さい。",
	[ERR_NOT_RIFF] 				= "Specified file is not RIFF file.\n",
	[ERR_NOT_WAVE] 				= "Specified file is not WAVE file.\n",
	[ERR_NOT_FIND_FMT_CHUNK] 	= "Failed to find fmt chunk.\n",
	[ERR_NOT_FIND_DATA_CHUNK] 	= "Failed to find data chunk.\n",
	[ERR_NOT_FIND_DATA] 		= "Failed to find offset of PCM data.\n",
	[ERR_NOT_PCM] 				= "Specified file's sound data is not PCM format.\n",
	[ERR_BITS_PER_SAMPLE] 		= "Specified file's sound data is %d bits, not 8 nor 16 bits.\n",
	[ERR_SET_FMT] 				= "ioctl( SOUND_PCM_SETFMT )",
	[ERR_WRITE_RATE] 			= "ioctl( SOUND_PCM_WRITE_RATE )",
	[ERR_WRITE_CHANNEL] 		= "ioctl( SOUND_PCM_WRITE_CHANNELS )"
};

static void wave_init(const char *filename, play_wave_info *wave_info, int refresh_count);		//WAVファイルインフォーメーション初期化
static int wave_read_file_header(play_wave_info *wave_info);									//WAVファイルヘッダを読み込む
static int set_dsp(int *dsp, play_wave_info *wave_info);										//DSPデバイスを設定
static int dsp_play(int *dsp, play_wave_info *wave_info);										//放送



/*SOH*************************************************************************
 * NAME         : play_wave
 * FUNCTION     : WAVファイル放送インターフェス
 * VERSION      : 01.00.00
 * IN/OUT       : (i/ ) filename: 音声ファイル名
 *              : (i/ ) refresh_times : 再生回数
 * RETURN       : 0:OK <0:NG　
 * REMARKS      : 備考
 * HISTORY      :
 * ID-----------DATE--------NAME----------NOTE --------------------------------
 * [V01.00.00]  2015.02.02  晏         
 *************************************************************************EOH*/

int play_wave(const char *filename, int refresh_times)
{
	
	int dsp = 0;						//DSPデバイスの識別子
		
	play_wave_info wave_info;			//音声ファイルインフォーメーション
	
	wave_init(filename, &wave_info, refresh_times);					//音声ファイルインフォーメーションを初期化する
	
	
	//音声ファイルを開く
	if((wave_info.fp = fopen(filename, "r")) == NULL )
	{
		fprintf(stderr, "Failed to open %s\n", filename);
		return -1;
	}
	
	//音声ファイルのヘッダを読み込む、チェック処理を行う
	if (-1 == wave_read_file_header(&wave_info))
	{
		fclose(wave_info.fp);
		return -1;
	}
	
	//DSPデバイスを開く
	if ( -1 == ( dsp = open( DSP_DEVICE, O_WRONLY ) ) ) 
	{
		fprintf(stderr, "open()%s失敗しました。",  DSP_DEVICE);
		fclose(wave_info.fp);
		return -1;
	}
	
	//音声ファイルヘッダデータにより、DSPデバイスを設定する
	if ( -1 == set_dsp(&dsp, &wave_info))
	{
		perror("DSPデバイス設定失敗しました。");
		close(dsp);
		fclose(wave_info.fp);
		return -1;
	}
	
	//放送を行う
	if (dsp_play(&dsp, &wave_info) < 0)
	{
		perror("放送失敗しました。");
		close(dsp);
		fclose(wave_info.fp);
		return -1;
	}
	
	close(dsp);
	fclose(wave_info.fp);
	
	return 0;
}


/*SOH*************************************************************************
 * NAME         : wave_init
 * FUNCTION     : 音声ファイルインフォーメーションを初期化する
 * VERSION      : 01.00.00
 * IN/OUT       : (i/ ) filename: ファイル名
 *              : (i/o) wave_info : 音声ファイルインフォーメーション
 			    : (i/ )refresh_count:再生回数
 * RETURN       : 0:OK <0:NG　
 * REMARKS      : 備考
 * HISTORY      :
 * ID-----------DATE--------NAME----------NOTE --------------------------------
 * [V01.00.00]  2015.02.02  晏         
  *************************************************************************EOH*/

static void wave_init(const char *filename, play_wave_info *wave_info, int refresh_count)
{
	wave_info->play_start_pos = 0;
	wave_info->play_pos = 0;
	wave_info->stop = 0;
	wave_info->data_size = 0;
	wave_info->refresh_times = refresh_count;
	strncpy(wave_info->play_filename, filename, FILENAME_SIZE);
	memset(&wave_info->wave_header, 0x00, sizeof(wave_file_header_info));
}


/*SOH*************************************************************************
 * NAME         : wave_read_file_header
 * FUNCTION     : 音声ファイルヘッダデータを読み込んで、チェックします
 * VERSION      : 01.00.00
 * IN/OUT       : (i/o) wave_info: 音声ファイルインフォーメーション　　
 * RETURN       : 0:OK <0:NG　
 * REMARKS      : 備考
 * HISTORY      :
 * ID-----------DATE--------NAME----------NOTE --------------------------------
 * [V01.00.00]  2015.02.02  晏         
  *************************************************************************EOH*/

static int wave_read_file_header(play_wave_info *wave_info)
{
	int len = 0;							//臨時変数　サイズ
	char buf[HEADER_BUFSIZE];				//臨時バッファ
	
	memset(buf, 0x00, HEADER_BUFSIZE);		//臨時バッファを初期化する
	
	//ファイルヘッダをバッファに読み込む
	//fread( buf, 1, HEADER_BUFSIZE, wave_info->fp );
	
	//バッファからファイルヘッダを構造体にコピーする
	//memcpy(&wave_info->wave_header, buf, HEADER_BUFSIZE);
	
	//ファイルヘッダをバッファに読み込む
	len = fread( &wave_info->wave_header, 1, HEADER_BUFSIZE, wave_info->fp );
	
	if (len < HEADER_BUFSIZE)
	{
		if (feof(wave_info->fp))
		{
			perror("ファイルサイズはヘッダサイズより小さい。");
			return -1;
		}
		else
		{
			perror("読み込み失敗。"):
			return -1;
		}
	}
	/*チェック処理を行う*/
	//"RIFF"識別子をチェックする
	if ( strncmp( wave_info->wave_header.chunk_id, "RIFF", 4 ) != 0 ) 
	{
		fprintf( stderr, "Specified file is not RIFF file.\n" );
		return -1;
	}
	
	//フォーマット"WAVE"をチェックする
	if ( strncmp( wave_info->wave_header.format, "WAVE", 4 ) != 0 ) {
		fprintf( stderr, "Specified file is not WAVE file.\n" );
		return -1;
	}

	//"fmt"識別子をチェックする
	if ( strncmp(wave_info->wave_header.sub_chunk_id, "fmt ", 4) != 0)
	{
		fprintf( stderr, "Failed to find fmt chunk.\n" );
		return -1;
	}
	
	//無限ループでデータ部を探す	
	while ( 1 ) {
		len = fread( buf, 8, 1, wave_info->fp );								//識別子とサイズを読み込む
		if (len < 8)
		{
			if (feof(wave_info->fp))
			{
				perror("ファイルサイズはヘッダサイズより小さい。");
				return -1;
			}
			else
			{
				perror("読み込み失敗。"):
				return -1;
			}
		}
		
		len = *( int* )( &buf[4] );										//サイズをlenに保存する

		//"data"識別子をチェックする、"data"でなければ、lenバイト内容を飛ばして、次のエリアをチェックする
		if ( strncmp( buf, "data", 4 ) != 0 ) 
		{
			//位置lenバイト後を設定し、"data"を探し続く
			if ( fseek( wave_info->fp, len, SEEK_CUR ) == -1 ) 
			{
				fprintf( stderr, "Failed to find data chunk.\n" );
				return -1;
			}
		}
		else 
		{
			break;
		}
	}

	//データ部サイズを設定する
	wave_info->data_size = len;

	//データの開始位置を保存しながら、チェックします
	if ( ( wave_info->play_start_pos = ftell( wave_info->fp ) ) == -1 ) 
	{
		fprintf( stderr, "Failed to find offset of PCM data.\n" );
		return -1;
	}
	
	return 0;
}


/*SOH*************************************************************************
 * NAME         : set_dsp　　
 * FUNCTION     : DSPデバイスを設定する
 * VERSION      : 01.00.00
 * IN/OUT       : (i/ ) dsp: DSPデバイスの識別子
 *              : (i/ ) wave_info : 音声ファイルインフォーメーション
 * RETURN       : 0:OK <0:NG　
 * REMARKS      : 備考
 * HISTORY      :
 * ID-----------DATE--------NAME----------NOTE --------------------------------
 * [V01.00.00]  2015.02.02  晏         
  *************************************************************************EOH*/

static int set_dsp(int *dsp, play_wave_info *wave_info)
{
	int format = 0;					//フォーマットの初期化

	//メディアフォーマットは"1"でなければ、PCMフォーマットではないと判断します。(この状態は別の圧縮ファイルになってしまう)
	if ( 1 != wave_info->wave_header.audio_format)
	{
		fprintf( stderr, "Specified file's sound data is not PCM format.\n" );
		//fclose( wave_info->fp );
		return -1;
	}
	
	//量子化ビット数をチェックする
	//8bitの場合：フォーマットに"AFMT_U8"を設定する　　
	if ( wave_info->wave_header.bits_per_sample == 8 ) {
		format = AFMT_U8;
	}
	
	//16bitの場合：フォーマットに"AFMT_S16_LE"を設定する
	else if ( wave_info->wave_header.bits_per_sample == 16 ) {
		format = AFMT_S16_LE;
	}
	
	//その以外の場合、エラーとします
	else {
		fprintf( stderr, "Specified file's sound data is %d bits,"
			 "not 8 nor 16 bits.\n", wave_info->wave_header.bits_per_sample );
		//fclose( wave_info->fp );
		return -1;
	}

	int channel = ( int )wave_info->wave_header.num_channels;					//チャンネルを設定する

	//デバイスのフォーマットを設定する
	if ( ioctl( *dsp, SNDCTL_DSP_SETFMT, &format ) == -1 ) {
		perror( "ioctl( SOUND_PCM_SETFMT )" );
		return -1;
	}

	//デバイスのサンプリングの周波数を設定
	if ( ioctl( *dsp, SOUND_PCM_WRITE_RATE, &wave_info->wave_header.sample_rate ) == -1 ) {
		perror( "ioctl( SOUND_PCM_WRITE_RATE )" );
		return -1;
	}

	//デバイスのチャンネルを設定する
	if ( ioctl( *dsp, SOUND_PCM_WRITE_CHANNELS, &channel ) == -1 ) {
		perror( "ioctl( SOUND_PCM_WRITE_CHANNELS )" );
		return -1;
	}

	return 0;
}



/*SOH*************************************************************************
 * NAME         : dsp_play　　
 * FUNCTION     : 音声を放送する
 * VERSION      : 01.00.00
 * IN/OUT       : (i/ ) dsp: DSPデバイスの識別子
 *              : (i/ ) wave_info : 音声ファイルインフォーメーション
 * RETURN       : 0:OK <0:NG　
 * REMARKS      : 備考
 * HISTORY      :
 * ID-----------DATE--------NAME----------NOTE --------------------------------
 * [V01.00.00]  2015.02.02  晏         
  *************************************************************************EOH*/

static int dsp_play(int *dsp, play_wave_info *wave_info)
{
	int len;								//臨時変数サイズ
	char buf[BUFSIZE];						//臨時バッファ(ファイル読み込むとデバイスに書き込む用)
	int rebroadcast_cnt = 0;				//再生回数
	int ret = 0;							//リータン値
	
	printf( "Now playing specified wave file %s ...\n", wave_info->play_filename );
	fflush( stdout );

	//データの開始場所に設定する
	fseek( wave_info->fp, wave_info->play_start_pos, SEEK_SET );

	//ループで再生する
	while ( 1 ) {
		len = fread( buf, 1, BUFSIZE, wave_info->fp );

		//ファイルを再生完了場合や読み込み失敗の場合
		if ( len < BUFSIZE ) 
		{
			//ファイルを再生完了のチェック
			if ( feof( wave_info->fp ) ) 
			{
				//書込み失敗
				if ( -1 == write( *dsp, buf, len ) ) 
				{
					perror( "[EOF]DSPデバイスに書き込むことが失敗しました。" );
					ret = -1;
				}
				//正常の場合、再生回数により、終了するかもう一度再生するかを決める
				else 
				{
					if ( (++rebroadcast_cnt) < wave_info->refresh_times)
					{
						fseek( wave_info->fp, wave_info->play_start_pos, SEEK_SET );
						continue;
					}
				}
			}
			else 
			{
				perror( "放送中読み込み失敗。" );
				ret = -1;
			}

			break;
		}

		if ( -1 == write( *dsp, buf, len ) ) 
		{
			perror( "DSPデバイスに書き込むことが失敗しました。" );
			ret = -1;
			break;
		}

	}
	

	return ret;
}
