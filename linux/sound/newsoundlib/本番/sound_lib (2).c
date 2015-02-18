/******************************************************************************
 * SYSTEM       :   音声放送LIB
 * PROGRAM      :   放送を行うライブラリ
 * MODULE       :   sound_lib.c  
 * REMARKS      :
 * HISTORY      :
 * ID-----------DATE--------NAME----------NOTE --------------------------------
 * [V01.00.00]  2015.2.18   KOCHI.晏       初期作成
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "lib/sound_lib.h"
			

#define ERR_MSG_SIZE		128												//エラーメッセージサイズ
#define BUFSIZE				64000											//バッファサイズ
#define MIXER_MASTER_NAME	"Master"										//音声項目Master
#define MIXER_PCM_NAME		"PCM"											//音声項目PCM
#define DEVICE				"default"										//出力デバイス
#define FORMAT_U8			8												//8位量子化数
#define FORMAT_S16_LE		16												//16位量子化数
#define PLAY_FINAL_CHUNK	1												//最後かどうかのフラグ

#define DEF_ERR(_code, _str) [_code] = (_str)								//配列要素設定


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
	FILE 	*fp;															//音声ファイル識別子
	int 	stop;															//中止状態(保留　今使っていない)
	int 	data_size;	 													//データ部のサイズ
	int 	refresh_times;													//再生回数
	unsigned int play_pos;													//停止場所(保留　今使っていない)
	unsigned int play_start_pos;											//データ開始位置
	char 	play_filename[32];												//音声ファイル名
	wave_file_header_info wave_header;										//WAVファイルヘッダ
}play_wave_info;


static snd_pcm_uframes_t sl_buffersize = 0; 								//バッファサイズ
static snd_pcm_uframes_t sl_outburst = 0; 									//1 periodのサイズ
static size_t bytes_per_sample = 0;											//1 sampleのサイズ
static int s_task_no = 0;

//エラーメッセージ
static const char err_msg[][ERR_MSG_SIZE] = {
	/* エラー無し */
	DEF_ERR(ERR_ZERO, 										ERR_STR_ZERO),
	/* ファイル　エラー */
	DEF_ERR(ERR_FILE_OPEN, 									ERR_STR_FILE_OPEN),		
	DEF_ERR(ERR_FILE_READ, 									ERR_STR_FILE_READ),		
	DEF_ERR(ERR_FILE_WRITE, 								ERR_STR_FILE_WRITE),		
	DEF_ERR(ERR_FILE_HEADER_READ,							ERR_STR_FILE_HEADER_READ),
	/* デバイス　エラー */
	DEF_ERR(ERR_DEVICE_OPEN, 								ERR_STR_DEVICE_OPEN),	
	DEF_ERR(ERR_DEVICE_CLOSE, 								ERR_STR_DEVICE_CLOSE),			
	DEF_ERR(ERR_DEVICE_READ, 								ERR_STR_DEVICE_READ),		
	DEF_ERR(ERR_DEVICE_WRITE, 								ERR_STR_DEVICE_WRITE),		
	DEF_ERR(ERR_DEVICE_SET, 								ERR_STR_DEVICE_SET),		
	DEF_ERR(ERR_DEVICE_DESCRIPTOR,							ERR_STR_DEVICE_DESCRIPTOR),
	/* その他 */
	DEF_ERR(ERR_BROADCAST, 									ERR_STR_BROADCAST),		
	DEF_ERR(ERR_SIZE, 										ERR_STR_SIZE),	
	DEF_ERR(ERR_HANDLE_NULL, 								ERR_STR_HANDLE_NULL),
	DEF_ERR(ERR_HANDLE_NOT_NULL, 							ERR_STR_HANDLE_NOT_NULL),
	DEF_ERR(ERR_PLAYBACK_NO_FRAME, 							ERR_STR_PLAYBACK_NO_FRAME),
	
	/* WAVファイル　エラー */			
	DEF_ERR(ERR_NOT_RIFF, 									ERR_STR_NOT_RIFF),			
	DEF_ERR(ERR_NOT_WAVE, 									ERR_STR_NOT_WAVE),			
	DEF_ERR(ERR_NOT_FIND_FMT_CHUNK, 						ERR_STR_NOT_FIND_FMT_CHUNK),
	DEF_ERR(ERR_NOT_FIND_DATA_CHUNK, 						ERR_STR_NOT_FIND_DATA_CHUNK),
	DEF_ERR(ERR_NOT_FIND_DATA,								ERR_STR_NOT_FIND_DATA),
	DEF_ERR(ERR_NOT_PCM, 									ERR_STR_NOT_PCM),	
	/*  */		
	DEF_ERR(ERR_BITS_PER_SAMPLE, 							ERR_STR_BITS_PER_SAMPLE),	
	DEF_ERR(ERR_SET_FMT, 									ERR_STR_SET_FMT),			
	DEF_ERR(ERR_WRITE_RATE, 								ERR_STR_WRITE_RATE),		
	DEF_ERR(ERR_WRITE_CHANNEL,								ERR_STR_WRITE_CHANNEL), 
	
	/* 音声デバイス設定　エラー */
	DEF_ERR(ERR_ALSA_UnableToGetInitialParameters,			ERR_STR_ALSA_UnableToGetInitialParameters	),
	DEF_ERR(ERR_ALSA_UnableToSetAccessType,					ERR_STR_ALSA_UnableToSetAccessType			),
	DEF_ERR(ERR_ALSA_formatNotSupportedByHardware,			ERR_STR_ALSA_formatNotSupportedByHardware	),
	DEF_ERR(ERR_ALSA_UnableToSetFormat,						ERR_STR_ALSA_UnableToSetFormat				),
	DEF_ERR(ERR_ALSA_UnableToSetChannels,					ERR_STR_ALSA_UnableToSetChannels			),
	DEF_ERR(ERR_ALSA_UnableToDisableResampling,				ERR_STR_ALSA_UnableToDisableResampling		),
	DEF_ERR(ERR_ALSA_UnableToSetSamplerate2,				ERR_STR_ALSA_UnableToSetSamplerate2			),
	DEF_ERR(ERR_ALSA_UnableToSetBufferTimeNear,				ERR_STR_ALSA_UnableToSetBufferTimeNear		),
	DEF_ERR(ERR_ALSA_UnableToSetHwParameters,				ERR_STR_ALSA_UnableToSetHwParameters		),
	DEF_ERR(ERR_ALSA_UnableToGetBufferSize,					ERR_STR_ALSA_UnableToGetBufferSize			),
	DEF_ERR(ERR_ALSA_UnableToGetPeriodSize,					ERR_STR_ALSA_UnableToGetPeriodSize			),
	DEF_ERR(ERR_ALSA_UnableToGetBoundary,					ERR_STR_ALSA_UnableToGetBoundary			),
	DEF_ERR(ERR_ALSA_UnableToSetStartThreshold,				ERR_STR_ALSA_UnableToSetStartThreshold		),
	DEF_ERR(ERR_ALSA_UnableToSetStopThreshold,				ERR_STR_ALSA_UnableToSetStopThreshold		),
	DEF_ERR(ERR_ALSA_UnableToSetSilenceSize,				ERR_STR_ALSA_UnableToSetSilenceSize			),
	DEF_ERR(ERR_ALSA_UnableToGetSwParameters,				ERR_STR_ALSA_UnableToGetSwParameters		),
	DEF_ERR(ERR_ALSA_UnableToFindSimpleControl,				ERR_STR_ALSA_UnableToFindSimpleControl		),
	DEF_ERR(ERR_ALSA_UnableToSetPeriods,					ERR_STR_ALSA_UnableToSetPeriods				),
	DEF_ERR(ERR_ALSA_PcmInSuspendModeTryingResume,			ERR_STR_ALSA_PcmInSuspendModeTryingResume	),
	DEF_ERR(ERR_ALSA_TryingToResetSoundcard,				ERR_STR_ALSA_TryingToResetSoundcard			),
	DEF_ERR(ERR_ALSA_PcmPrepareError,						ERR_STR_ALSA_PcmPrepareError				),
	/* ミキサー設定　エラー */
	DEF_ERR(ERR_ALSA_MixerOpenError, 						ERR_STR_ALSA_MixerOpenError					),
	DEF_ERR(ERR_ALSA_MixerAttachError, 						ERR_STR_ALSA_MixerAttachError				),
	DEF_ERR(ERR_ALSA_MixerRegisterError,					ERR_STR_ALSA_MixerRegisterError				),
	DEF_ERR(ERR_ALSA_MixerLoadError,						ERR_STR_ALSA_MixerLoadError					),
	DEF_ERR(ERR_ALSA_MixerSetVolumeError,					ERR_STR_ALSA_MixerSetVolumeError			)
};                                                          

static int init(snd_pcm_t *handle, play_wave_info *wave_info);								//放送デバイスの初期化
static int wave_read_file_header(play_wave_info *wave_info);								//放送ファイルのヘッダを読み込む
static int set_volumn(const char *mixer_name, long left_vol, long right_vol);				//音声ボリュウムの設定
static int play(snd_pcm_t *handler, void* data, int len, int flags);						//音声ファイルを放送

/*SOH*************************************************************************
 * NAME         : open_voice_device
 * FUNCTION     : デバイスを開く
 * VERSION      : 01.00.00
 * IN/OUT       : (i/o) handle: 音声放送デバイスのハンドル
 				: (i/o) task_no: タスク番号
 * RETURN       : 0:OK >0:NG　
 * REMARKS      : 備考
 * HISTORY      :
 * ID-----------DATE--------NAME----------NOTE --------------------------------
 * [V01.00.00]  2015.02.18  KOCHI.晏      
 *************************************************************************EOH*/
int open_device(snd_pcm_t **handle, int task_no)
{
	int err = ERR_ZERO;
	
	s_task_no = task_no;						//デバイスを使っているタスク番号を記録
	
	//ハンドルはNULLではない場合、オープンしない
	if (NULL != *handle)
	{
		printX(s_task_no, ERR_YES|PRT_YES, "[%s_%s_%d]%s", __FILE__, __FUNCTION__, __LINE__, err_msg[ERR_HANDLE_NOT_NULL]);
		return ERR_HANDLE_NOT_NULL;
	}
	//デバイスを開く
	if ((err = snd_pcm_open(handle, DEVICE, SND_PCM_STREAM_PLAYBACK, 0)) < 0)
	{
		printX(s_task_no, ERR_YES|PRT_YES, "[%s_%s_%d]%s", __FILE__, __FUNCTION__, __LINE__, err_msg[ERR_DEVICE_OPEN]);
		return ERR_DEVICE_OPEN;
	}
	//デバイスを開いてからまだNULLの場合、エラーとします
	if (NULL == *handle)
	{
		printX(s_task_no, ERR_YES|PRT_YES, "[%s_%s_%d]%s", __FILE__, __FUNCTION__, __LINE__, err_msg[ERR_HANDLE_NULL]);
		return ERR_HANDLE_NULL;
	}
	
	return ERR_ZERO;
}


/*SOH*************************************************************************
 * NAME         : close_device
 * FUNCTION     : デバイスを閉じる
 * VERSION      : 01.00.00
 * IN/OUT       : (i/o) handle: 音声放送デバイスのハンドル
 * RETURN       : 0:OK >0:NG　
 * REMARKS      : 備考
 * HISTORY      :
 * ID-----------DATE--------NAME----------NOTE --------------------------------
 * [V01.00.00]  2015.02.18  KOCHI.晏      
 *************************************************************************EOH*/
int close_device(snd_pcm_t *handle)
{
	int err = ERR_ZERO;
	
	//ハンドルはNULLの場合、閉じられない
	if (NULL == handle)
	{
		printX(s_task_no, ERR_YES|PRT_YES, "[%s_%s_%d]%s", __FILE__, __FUNCTION__, __LINE__, err_msg[ERR_HANDLE_NULL]);
		return ERR_HANDLE_NULL;
	}
	//クロス失敗したら、エラーとします
	if ((err = snd_pcm_close(handle)) < 0)
	{
		printX(s_task_no, ERR_YES|PRT_YES, "[%s_%s_%d]%s", __FILE__, __FUNCTION__, __LINE__, err_msg[ERR_DEVICE_CLOSE]);
		return ERR_DEVICE_CLOSE;
	}
	
	handle = NULL;
	
	return ERR_ZERO;
}

/*
static void reset(snd_pcm_t *handle)
{
    int err;

    if ((err = snd_pcm_drop(handle)) < 0)
    {
		printX(s_task_no, ERR_YES|PRT_YES, "[%s_%s_%d]%s %s", __FILE__, __FUNCTION__, __LINE__, err_msg[ERR_DEVICE_DROP], snd_strerror(err));
		return;
    }
    if ((err = snd_pcm_prepare(handle)) < 0)
    {
		printX(s_task_no, ERR_YES|PRT_YES, "[%s_%s_%d]%s %s", __FILE__, __FUNCTION__, __LINE__, err_msg[ERR_DEVICE_PREPARE], snd_strerror(err));
		return;
    }
    return;
}
*/


/*SOH*************************************************************************
 * NAME         : init
 * FUNCTION     : デバイスを初期化
 * VERSION      : 01.00.00
 * IN/OUT       : (i/o) handle: 音声放送デバイスのハンドル
 				: (i/o) wave_info: 音声ファイル情報
 * RETURN       : 0:OK >0:NG　
 * REMARKS      : 備考
 * HISTORY      :
 * ID-----------DATE--------NAME----------NOTE --------------------------------
 * [V01.00.00]  2015.02.18  KOCHI.晏      
 *************************************************************************EOH*/
static int init(snd_pcm_t *handle, play_wave_info *wave_info)
{
    unsigned int alsa_buffer_time = 500000; 				//バッファタイマー、0.5sに設定 
    unsigned int alsa_fragcount = 16;						//periodを16に設定
    int err = ERR_ZERO;
    int format, rate, channel, bit_per_sample;
    
    
    snd_pcm_hw_params_t *alsa_hwparams;
	snd_pcm_sw_params_t *alsa_swparams;
	
    snd_pcm_uframes_t chunk_size;
    snd_pcm_uframes_t bufsize;
    snd_pcm_uframes_t boundary;
   
	assert(handle != NULL);
	assert(wave_info != NULL);
    
#if SND_LIB_VERSION >= 0x010005
    printX(s_task_no, ERR_YES|PRT_YES, "alsa-init: using ALSA %s\n", snd_asoundlib_version());
#else
    printX(s_task_no, ERR_YES|PRT_YES, "alsa-init: compiled for ALSA-%s\n", SND_LIB_VERSION_STR);
#endif

	bit_per_sample = wave_info->wave_header.bits_per_sample;
	rate = wave_info->wave_header.sample_rate;
	channel = wave_info->wave_header.num_channels;
	
	//量子化数により、フォーマットを設定
    switch (bit_per_sample)
    {
    case FORMAT_U8:
		format = SND_PCM_FORMAT_U8;
	break;
	
    case FORMAT_S16_LE:
		format = SND_PCM_FORMAT_S16_LE;
	break;
    
	default:
		format = SND_PCM_FORMAT_S16_LE; 
	break;
    }

    snd_pcm_hw_params_alloca(&alsa_hwparams);
    snd_pcm_sw_params_alloca(&alsa_swparams);

	assert(alsa_hwparams != NULL);
	assert(alsa_swparams != NULL);

    // ハードウェイパラメータを設定
    if ((err = snd_pcm_hw_params_any(handle, alsa_hwparams)) < 0)
	{
		printX(s_task_no, ERR_YES|PRT_YES, "[%s_%s_%d]%s:%s\n", __FILE__, __FUNCTION__, __LINE__, err_msg[ERR_ALSA_UnableToGetInitialParameters], snd_strerror(err));
	  	return ERR_ALSA_UnableToGetInitialParameters;
	}

	//アクセスモードを設定する
    err = snd_pcm_hw_params_set_access(handle, alsa_hwparams, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (err < 0) 
    {
		printX(s_task_no, ERR_YES|PRT_YES, "[%s_%s_%d]%s:%s\n", __FILE__, __FUNCTION__, __LINE__, err_msg[ERR_ALSA_UnableToSetAccessType],
		       snd_strerror(err));
		return ERR_ALSA_UnableToSetAccessType;
    }

      /* workaround for nonsupported formats
	 sets default format to S16_LE if the given formats aren't supported */
	if ((err = snd_pcm_hw_params_test_format(handle, alsa_hwparams, format)) < 0)
	{
		printX(s_task_no, ERR_YES|PRT_YES, "[%s_%s_%d]%s:%s\n", __FILE__, __FUNCTION__, __LINE__, err_msg[ERR_ALSA_formatNotSupportedByHardware], (format));
        format = SND_PCM_FORMAT_S16_LE;
	}

	//フォーマットを設定する
	if ((err = snd_pcm_hw_params_set_format(handle, alsa_hwparams, format)) < 0)
	{
		printX(s_task_no, ERR_YES|PRT_YES, "[%s_%s_%d]%s:%s\n", __FILE__, __FUNCTION__, __LINE__, err_msg[ERR_ALSA_UnableToSetFormat], snd_strerror(err));
		return ERR_ALSA_UnableToSetFormat;
	}

	//チャンネルを設定する
	if ((err = snd_pcm_hw_params_set_channels_near(handle, alsa_hwparams, &channel)) < 0)
	{
		printX(s_task_no, ERR_YES|PRT_YES, "[%s_%s_%d]%s:%s\n", __FILE__, __FUNCTION__, __LINE__, err_msg[ERR_ALSA_UnableToSetChannels], snd_strerror(err));
		return ERR_ALSA_UnableToSetChannels;
	}

      /* workaround for buggy rate plugin (should be fixed in ALSA 1.0.11)
         prefer our own resampler, since that allows users to choose the resampler,
         even per file if desired */
         
#if SND_LIB_VERSION >= 0x010009
#if 0
	printX(s_task_no, ERR_YES|PRT_YES, "rate before:%d\n", rate);
	
    if ((err = snd_pcm_hw_params_set_rate_resample(handle, alsa_hwparams, 0)) < 0)
	{
		printX(s_task_no, ERR_YES|PRT_YES, "[%s_%s_%d]%s:%s\n", __FILE__, __FUNCTION__, __LINE__, err_msg[ERR_ALSA_UnableToDisableResampling], snd_strerror(err));
	  return ERR_ALSA_UnableToDisableResampling;
	}
	
	printX(s_task_no, ERR_YES|PRT_YES, "rate after1:%d\n", rate);
	#endif
#endif

	//周波数を設定する
    if ((err = snd_pcm_hw_params_set_rate_near(handle, alsa_hwparams, &rate, NULL)) < 0)
    {
		 printX(s_task_no, ERR_YES|PRT_YES, "[%s_%s_%d]%s:%s\n", __FILE__, __FUNCTION__, __LINE__, err_msg[ERR_ALSA_UnableToSetSamplerate2], snd_strerror(err));
		 return ERR_ALSA_UnableToSetSamplerate2;
    }
    
    //1サンプルにバイト数を取得する
    bytes_per_sample =  bit_per_sample / 8;
    bytes_per_sample *= channel;

	//バッファタイマーの設定
	if ((err = snd_pcm_hw_params_set_buffer_time_near(handle, alsa_hwparams, &alsa_buffer_time, NULL)) < 0)
	{
	    printX(s_task_no, ERR_YES|PRT_YES, "[%s_%s_%d]%s:%s\n", __FILE__, __FUNCTION__, __LINE__, err_msg[ERR_ALSA_UnableToSetBufferTimeNear], snd_strerror(err));
	    return ERR_ALSA_UnableToSetBufferTimeNear;
	}

	//periodの設定
	if ((err = snd_pcm_hw_params_set_periods_near(handle, alsa_hwparams, &alsa_fragcount, NULL)) < 0) 
	{
	     printX(s_task_no, ERR_YES|PRT_YES, "[%s_%s_%d]%s:%s\n", __FILE__, __FUNCTION__, __LINE__, err_msg[ERR_ALSA_UnableToSetPeriods], snd_strerror(err));
	     return ERR_ALSA_UnableToSetPeriods;
	}

	// 最後に設定したハードウェイパラメータを適用する
    if ((err = snd_pcm_hw_params(handle, alsa_hwparams)) < 0)
	{
	     printX(s_task_no, ERR_YES|PRT_YES, "[%s_%s_%d]%s:%s\n", __FILE__, __FUNCTION__, __LINE__, err_msg[ERR_ALSA_UnableToSetHwParameters], snd_strerror(err));
	     return ERR_ALSA_UnableToSetHwParameters;
	}
    
    // コントロールのため、バッファサイズ取得
    if ((err = snd_pcm_hw_params_get_buffer_size(alsa_hwparams, &bufsize)) < 0)
	{
	     printX(s_task_no, ERR_YES|PRT_YES, "[%s_%s_%d]%s:%s\n", __FILE__, __FUNCTION__, __LINE__, err_msg[ERR_ALSA_UnableToGetBufferSize], snd_strerror(err));
	     return ERR_ALSA_UnableToGetBufferSize;
	}
    else 
    {
	    sl_buffersize = bufsize * bytes_per_sample;
	    printX(s_task_no, ERR_YES|PRT_YES, "alsa-init: got buffersize=%i\n", sl_buffersize);
    }

	//1periodのサイズを取得(フレーム数)
    if ((err = snd_pcm_hw_params_get_period_size(alsa_hwparams, &chunk_size, NULL)) < 0) 
    {
		printX(s_task_no, ERR_YES|PRT_YES, "[%s_%s_%d]%s:%s\n", __FILE__, __FUNCTION__, __LINE__, err_msg[ERR_ALSA_UnableToGetPeriodSize], snd_strerror(err));
		return ERR_ALSA_UnableToGetPeriodSize;
    } 
    else 
    {
		printX(s_task_no, ERR_YES|PRT_YES, "alsa-init: got period size %li\n", chunk_size);
    }
    
    //1　periodのバイト数を取得
    sl_outburst = chunk_size * bytes_per_sample;

    /* setting software parameters */
    if ((err = snd_pcm_sw_params_current(handle, alsa_swparams)) < 0) 
    {
		printX(s_task_no, ERR_YES|PRT_YES, "[%s_%s_%d]%s:%s\n", __FILE__, __FUNCTION__, __LINE__, err_msg[ERR_ALSA_UnableToGetSwParameters], snd_strerror(err));
		return ERR_ALSA_UnableToGetSwParameters;
    }
    //バッファのboundaryを取得
#if SND_LIB_VERSION >= 0x000901
    if ((err = snd_pcm_sw_params_get_boundary(alsa_swparams, &boundary)) < 0) 
    {
		printX(s_task_no, ERR_YES|PRT_YES, "[%s_%s_%d]%s:%s\n", __FILE__, __FUNCTION__, __LINE__, err_msg[ERR_ALSA_UnableToGetBoundary], snd_strerror(err));
		return ERR_ALSA_UnableToGetBoundary;
    }
#else
    boundary = 0x7fffffff;
#endif
      /* start playing when one period has been written */
    if ((err = snd_pcm_sw_params_set_start_threshold(handle, alsa_swparams, chunk_size)) < 0) 
    {
		printX(s_task_no, ERR_YES|PRT_YES, "[%s_%s_%d]%s:%s\n", __FILE__, __FUNCTION__, __LINE__, err_msg[ERR_ALSA_UnableToSetStartThreshold], snd_strerror(err));
		return ERR_ALSA_UnableToSetStartThreshold;
    }
      /* disable underrun reporting */
    if ((err = snd_pcm_sw_params_set_stop_threshold(handle, alsa_swparams, boundary)) < 0) 
    {
		printX(s_task_no, ERR_YES|PRT_YES, "[%s_%s_%d]%s:%s\n", __FILE__, __FUNCTION__, __LINE__, err_msg[ERR_ALSA_UnableToSetStopThreshold], snd_strerror(err));
		return ERR_ALSA_UnableToSetStopThreshold;
    }
#if SND_LIB_VERSION >= 0x000901
    /* play silence when there is an underrun */
    if ((err = snd_pcm_sw_params_set_silence_size(handle, alsa_swparams, boundary)) < 0) 
    {
		printX(s_task_no, ERR_YES|PRT_YES, "[%s_%s_%d]%s:%s\n", __FILE__, __FUNCTION__, __LINE__, err_msg[ERR_ALSA_UnableToSetSilenceSize], snd_strerror(err));
		return ERR_ALSA_UnableToSetSilenceSize;
    }
#endif
    if ((err = snd_pcm_sw_params(handle, alsa_swparams)) < 0) 
    {
		printX(s_task_no, ERR_YES|PRT_YES, "[%s_%s_%d]%s:%s\n", __FILE__, __FUNCTION__, __LINE__, err_msg[ERR_ALSA_UnableToGetSwParameters], snd_strerror(err));
		return ERR_ALSA_UnableToGetSwParameters;
    }

    printX(s_task_no, ERR_YES|PRT_YES, "alsa: %d Hz/%d channels/%d bpf/%d bytes buffer/%s\n",
	     rate, channel, (int)bytes_per_sample, sl_buffersize,
	     snd_pcm_format_description(format));

    // end switch handle (spdif)
    //alsa_can_pause = snd_pcm_hw_params_can_pause(alsa_hwparams);
    return ERR_ZERO;
} 

/*SOH*************************************************************************
 * NAME         : wave_read_file_header
 * FUNCTION     : 音声ファイルヘッダデータを読み込んで、チェックします
 * VERSION      : 01.00.00
 * IN/OUT       : (i/o) wave_info: 音声ファイルインフォーメーション　　
 * RETURN       : 0:OK >0:NG　
 * REMARKS      : 備考
 * HISTORY      :
 * ID-----------DATE--------NAME----------NOTE --------------------------------
 * [V01.00.00]  2015.02.02  KOCHI.晏         
  *************************************************************************EOH*/
static int wave_read_file_header(play_wave_info *wave_info)
{
	int len = 0;							//臨時変数　サイズ
	char buf[BUFSIZE];						//臨時バッファ
	
	memset(buf, 0x00, BUFSIZE);				//臨時バッファを初期化する
	
	assert(wave_info != NULL);
	//ファイルヘッダをバッファに読み込む
	len = fread( &wave_info->wave_header, 1, sizeof(wave_file_header_info), wave_info->fp );
	
	if (len < sizeof(wave_file_header_info))
	{
		if (feof(wave_info->fp))
		{
			printX(s_task_no, ERR_YES|PRT_YES, "[%s:%d:%s()]%s。",  __FILE__, __LINE__, __FUNCTION__, err_msg[ERR_SIZE]);
			return ERR_SIZE;
		}
		else
		{
			printX(s_task_no, ERR_YES|PRT_YES, "[%s:%d:%s()]%s。",  __FILE__, __LINE__, __FUNCTION__, err_msg[ERR_FILE_READ]);
			return ERR_FILE_READ;
		}
	}
	/*チェック処理を行う*/
	//"RIFF"識別子をチェックする
	if ( strncmp( wave_info->wave_header.chunk_id, "RIFF", 4 ) != 0 ) 
	{
		printX(s_task_no, ERR_YES|PRT_YES, "[%s:%d:%s()]%s。",  __FILE__, __LINE__, __FUNCTION__, err_msg[ERR_NOT_RIFF] );
		return ERR_NOT_RIFF;
	}
	
	//フォーマット"WAVE"をチェックする
	if ( strncmp( wave_info->wave_header.format, "WAVE", 4 ) != 0 ) {
		printX(s_task_no, ERR_YES|PRT_YES, "[%s:%d:%s()]%s。",  __FILE__, __LINE__, __FUNCTION__, err_msg[ERR_NOT_WAVE] );
		return ERR_NOT_WAVE;
	}

	//"fmt"識別子をチェックする
	if ( strncmp(wave_info->wave_header.sub_chunk_id, "fmt ", 4) != 0)
	{
		printX(s_task_no, ERR_YES|PRT_YES, "[%s:%d:%s()]%s。",  __FILE__, __LINE__, __FUNCTION__, err_msg[ERR_NOT_FIND_FMT_CHUNK] );
		return ERR_NOT_FIND_FMT_CHUNK;
	}
	
	//無限ループでデータ部を探す	
	while ( 1 ) {
		len = fread( buf, 8, 1, wave_info->fp );								//識別子とサイズを読み込む
		if (len < 1)
		{
			if (feof(wave_info->fp))
			{
				printX(s_task_no, ERR_YES|PRT_YES, "[%s:%d:%s()]%s。",  __FILE__, __LINE__, __FUNCTION__, err_msg[ERR_SIZE]);
				return ERR_SIZE;
			}
			else
			{
				printX(s_task_no, ERR_YES|PRT_YES, "[%s:%d:%s()]%s。",  __FILE__, __LINE__, __FUNCTION__, err_msg[ERR_FILE_READ]);
				return ERR_FILE_READ;
			}
		}
		
		len = *( int* )( &buf[4] );										//サイズをlenに保存する

		//"data"識別子をチェックする、"data"でなければ、lenバイト内容を飛ばして、次のエリアをチェックする
		if ( strncmp( buf, "data", 4 ) != 0 ) 
		{
			//位置lenバイト後を設定し、"data"を探し続く
			if ( fseek( wave_info->fp, len, SEEK_CUR ) == -1 ) 
			{
				printX(s_task_no, ERR_YES|PRT_YES, "[%s:%d:%s()]%s。",  __FILE__, __LINE__, __FUNCTION__, err_msg[ERR_NOT_FIND_DATA_CHUNK] );
				return ERR_NOT_FIND_DATA_CHUNK;
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
		printX(s_task_no, ERR_YES|PRT_YES, "[%s:%d:%s()]%s。",  __FILE__, __LINE__, __FUNCTION__, err_msg[ERR_NOT_FIND_DATA] );
		return ERR_NOT_FIND_DATA;
	}
	
	return ERR_ZERO;
}

/*SOH*************************************************************************
 * NAME         : set_volumn
 * FUNCTION     : 音声ボリュウムを設定
 * VERSION      : 01.00.00
 * IN/OUT       : (i/o) mixer_name: ミキサーの一つ項目名
 				: (i/o) left_vol: 左ボリュウム
 				: (i/o) right_vol: 右ボリュウム
 * RETURN       : 0:OK >0:NG　
 * REMARKS      : 備考
 * HISTORY      :
 * ID-----------DATE--------NAME----------NOTE --------------------------------
 * [V01.00.00]  2015.02.18  KOCHI.晏      
 *************************************************************************EOH*/
int set_volumn(const char *mixer_name, long left_vol, long right_vol)
{
	long pmin, pmax;
	float f_multi;
	int err;
    snd_mixer_t *handle;
    snd_mixer_elem_t *elem;
    snd_mixer_selem_id_t *sid;
    
	int mix_index = 0;
	const char *mix_name = mixer_name;
	long set_vol_left = left_vol;
	long set_vol_right = right_vol;
	
	//sidを初期化
	snd_mixer_selem_id_alloca(&sid);
	//sidのインデックスと名前を設定
	snd_mixer_selem_id_set_index(sid, mix_index);
    snd_mixer_selem_id_set_name(sid, mix_name);
  
#if 0  
	
	elem = snd_mixer_find_selem(handler, sid);
	
	snd_mixer_selem_get_playback_volume_range(elem, &min_vol, &max_vol);
	printX(s_task_no, ERR_YES|PRT_YES, "[Before]%s  minVol:%d, maxVol:%d\n", mix_name, min_vol, max_vol);
	
	set_vol_left = (float)set_vol_left / 100 * (max_vol - min_vol) + min_vol;
	set_vol_right = (float)set_vol_right / 100 * (max_vol - min_vol) + min_vol;
#endif
/*	
	snd_mixer_selem_set_playback_volume_range(elem, 0, 100);
	
	snd_mixer_selem_get_playback_volume_range(elem, &min_vol, &max_vol);
	printX(s_task_no, ERR_YES|PRT_YES, "[After]%s  minVol:%d, maxVol:%d\n", mix_name, min_vol, max_vol);
*/	
#if 0
	if (snd_mixer_selem_has_playback_switch(elem))
	{
		if (!snd_mixer_selem_has_playback_switch_joined(elem))
		{
			snd_mixer_selem_set_playback_switch(elem, SND_MIXER_SCHN_FRONT_RIGHT, 1);
		}
		snd_mixer_selem_set_playback_switch(elem, SND_MIXER_SCHN_FRONT_LEFT, 1);
	}
	
	
	snd_mixer_selem_set_playback_volume(elem, SND_MIXER_SCHN_FRONT_RIGHT, set_vol_right);
	snd_mixer_selem_set_playback_volume(elem, SND_MIXER_SCHN_FRONT_LEFT, set_vol_left);
	
	
	snd_mixer_selem_get_playback_volume_range(elem, &min_vol, &max_vol);
	printX(s_task_no, ERR_YES|PRT_YES, "[Before END]%s  minVol:%d, maxVol:%d\n", mix_name, min_vol, max_vol);
#endif

	//ミキサーを開く
	if ((err = snd_mixer_open(&handle, 0)) < 0) 
	{
		printX(s_task_no, ERR_YES|PRT_YES, "[%s_%s_%d]%s:%s\n", __FILE__, __FUNCTION__, __LINE__, err_msg[ERR_ALSA_MixerOpenError], snd_strerror(err));
		return ERR_ALSA_MixerOpenError;
    }

	//ハンドルとデバイスをアタッチする
	if ((err = snd_mixer_attach(handle, DEVICE)) < 0) {
		printX(s_task_no, ERR_YES|PRT_YES, "[%s_%s_%d]%s:%s\n", __FILE__, __FUNCTION__, __LINE__, err_msg[ERR_ALSA_MixerAttachError], snd_strerror(err));
		snd_mixer_close(handle);
		return ERR_ALSA_MixerAttachError;
	}
	
	//ミキサーを登録する
	if ((err = snd_mixer_selem_register(handle, NULL, NULL)) < 0)
	{
		printX(s_task_no, ERR_YES|PRT_YES, "[%s_%s_%d]%s:%s\n", __FILE__, __FUNCTION__, __LINE__, err_msg[ERR_ALSA_MixerRegisterError], snd_strerror(err));
		snd_mixer_close(handle);
		return ERR_ALSA_MixerRegisterError;
	}
	
	//ミキサーをロードする
	err = snd_mixer_load(handle);
	if (err < 0) 
	{
		printX(s_task_no, ERR_YES|PRT_YES, "[%s_%s_%d]%s:%s\n", __FILE__, __FUNCTION__, __LINE__, err_msg[ERR_ALSA_MixerLoadError], snd_strerror(err));
		snd_mixer_close(handle);
		return ERR_ALSA_MixerLoadError;
	}
	
	//sidにより、要素を取得
	elem = snd_mixer_find_selem(handle, sid);
	if (!elem) 
	{
		printX(s_task_no, ERR_YES|PRT_YES, "[%s_%s_%d]%s%s:[name]%s  [index]%d\n", __FILE__, __FUNCTION__, __LINE__, err_msg[ERR_ALSA_UnableToFindSimpleControl], 
				snd_mixer_selem_id_get_name(sid), snd_mixer_selem_id_get_index(sid));
				snd_mixer_close(handle);
		return ERR_ALSA_UnableToFindSimpleControl;
	}
	
	//現在のボリュウム範囲を取得
	snd_mixer_selem_get_playback_volume_range(elem,&pmin,&pmax);
	
	//現在の1単位は実際のボリュウムに変換
	f_multi = (100 / (float)(pmax - pmin));
	

	//左ボリュウムを取得
	set_vol_left = set_vol_left / f_multi + pmin + 0.5;	
	
	//setting channels
	if ((err = snd_mixer_selem_set_playback_volume(elem, SND_MIXER_SCHN_FRONT_LEFT, set_vol_left)) < 0) 
	{
		printX(s_task_no, ERR_YES|PRT_YES, "[%s_%s_%d]%s:%s\n", __FILE__, __FUNCTION__, __LINE__, err_msg[ERR_ALSA_MixerSetVolumeError], snd_strerror(err));
		snd_mixer_close(handle);
		return ERR_ALSA_MixerSetVolumeError;
	}
	
	//右ボリュウムを取得
	set_vol_right = set_vol_right / f_multi + pmin + 0.5;
	if ((err = snd_mixer_selem_set_playback_volume(elem, SND_MIXER_SCHN_FRONT_RIGHT, set_vol_right)) < 0) 
	{
		printX(s_task_no, ERR_YES|PRT_YES, "[%s_%s_%d]%s:%s\n", __FILE__, __FUNCTION__, __LINE__, err_msg[ERR_ALSA_MixerSetVolumeError], snd_strerror(err));
		snd_mixer_close(handle);
		return ERR_ALSA_MixerSetVolumeError;
	}
	
	//再生のスイッチがあるかどうかをチェックする
	if (snd_mixer_selem_has_playback_switch(elem)) 
	{
		int lmute = (left_vol == 0);
		int rmute = (right_vol == 0);
		//全部のチャンネルはひとつスイッチしかないかをチェックする
		if (snd_mixer_selem_has_playback_switch_joined(elem)) 
		{
			lmute = rmute = lmute && rmute;
		} 
		else 
		{
			snd_mixer_selem_set_playback_switch(elem, SND_MIXER_SCHN_FRONT_RIGHT, !rmute);
		}
		snd_mixer_selem_set_playback_switch(elem, SND_MIXER_SCHN_FRONT_LEFT, !lmute);
	}

	snd_mixer_close(handle);
	
	return ERR_ZERO;
}

/*
    plays 'len' bytes of 'data'
    returns: number of bytes played
    modified last at 29.06.02 by jp
    thanxs for marius <marius@rospot.com> for giving us the light ;)
*/
/*SOH*************************************************************************
 * NAME         : play
 * FUNCTION     : 音声ファイルデータを放送(内部)
 * VERSION      : 01.00.00
 * IN/OUT       : (i/o) handler: 音声放送デバイスのハンドル
 				: (i/ ) data: 放送データ
 				: (i/ ) len: データのサイズ
 				: (i/ ) flags: 最後がどうかのフラグ
 * RETURN       : 0:OK >0:NG　
 * REMARKS      : 備考
 * HISTORY      :
 * ID-----------DATE--------NAME----------NOTE --------------------------------
 * [V01.00.00]  2015.02.18  KOCHI.晏      
 *************************************************************************EOH*/
static int play(snd_pcm_t *handler, void* data, int len, int flags)
{
	int num_frames;
	snd_pcm_sframes_t res = 0;
	
	if (!(flags & PLAY_FINAL_CHUNK))
	{
	    len = len / sl_outburst * sl_outburst;
	}    
	num_frames = len / bytes_per_sample;
	
	assert(handler != NULL);
	
	//ハンドルはNULLかどうかをチェックする
	if (!handler) 
	{
		printX(s_task_no, ERR_YES|PRT_YES, "[%s_%s_%d]%s\n", __FILE__, __FUNCTION__, __LINE__,  err_msg[ERR_HANDLE_NULL]);
		return 0;
	}
	
	//再生必要なフレームがなければ、0を返す
	if (num_frames == 0)
	{
		printX(s_task_no, ERR_YES|PRT_YES, "[%s_%s_%d]%s\n", __FILE__, __FUNCTION__, __LINE__, err_msg[ERR_PLAYBACK_NO_FRAME]);
		return 0;
	}
	
	do 
	{
		//データの書き込む
		res = snd_pcm_writei(handler, data, num_frames);

		if (res == -EINTR) 
		{
			/* nothing to do */
			res = 0;
	    }
	    else if (res == -ESTRPIPE) 
	    {	/* suspend */
			printX(s_task_no, ERR_YES|PRT_YES, "[%s_%s_%d]%s\n", __FILE__, __FUNCTION__, __LINE__, err_msg[ERR_ALSA_PcmInSuspendModeTryingResume]);
			//停止中の場合、resume必要
			while ((res = snd_pcm_resume(handler)) == -EAGAIN)
			{
				sleep(1);
			}
	    }
	    if (res < 0) 
	    {
			printX(s_task_no, ERR_YES|PRT_YES, "[%s_%s_%d]%s\n", __FILE__, __FUNCTION__, __LINE__, err_msg[ERR_ALSA_TryingToResetSoundcard]);
			if ((res = snd_pcm_prepare(handler)) < 0) 
			{
			  	printX(s_task_no, ERR_YES|PRT_YES, "[%s_%s_%d]%s\n", __FILE__, __FUNCTION__, __LINE__, err_msg[ERR_ALSA_PcmPrepareError]);
				break;
			}
			res = 0;
	    }
	} while (res == 0);
	//書き込んだデータバイト数を返す、失敗した場合、0を返す
	return res < 0 ? 0 : res * bytes_per_sample;
}

/*SOH*************************************************************************
 * NAME         : play_file
 * FUNCTION     : 音声ファイル放送(外部呼び出す)
 * VERSION      : 01.00.00
 * IN/OUT       : (i/o) handler: 音声放送デバイスのハンドル
 				: (i/o) file_name: 音声ファイル名
 				: (i/o) vol: 音声ボリュウム
 				: (i/o) refresh_cnt: 再生カウンター
 * RETURN       : 0:OK >0:NG　
 * REMARKS      : 備考
 * HISTORY      :
 * ID-----------DATE--------NAME----------NOTE --------------------------------
 * [V01.00.00]  2015.02.18  KOCHI.晏      
 *************************************************************************EOH*/
int play_file(snd_pcm_t *handler, const char *file_name, st_vol vol, int refresh_cnt)
{
	
	play_wave_info wave_info;
	char buf[BUFSIZE];
	int read_len = 0, write_len = 0;
	int current_location = 0;
	int len = 0;
	memset(buf, 0x00, BUFSIZE);
	
	if (NULL == handler)
	{
		printX(s_task_no, ERR_YES|PRT_YES, "[%s_%s_%d]%s\n", __FILE__, __FUNCTION__, __LINE__, err_msg[ERR_DEVICE_CLOSE]);
		return ERR_DEVICE_CLOSE;
	}
	
	if ((wave_info.fp = fopen(file_name, "r")) < 0)
	{
		fprintf (stderr, "[%s_%s_%d]%s   ファイル名=%s\n", __FILE__, __FUNCTION__, __LINE__, err_msg[ERR_FILE_OPEN], file_name);
		return ERR_FILE_OPEN;
	}
	
	//ヘッダを分析する
	wave_read_file_header(&wave_info);
	
	//デバイスを初期化する
	init(handler, &wave_info);
	#if 0
	//ファイルポインタをファイルの最後に移動する
	fseek( wave_info.fp, 0, SEEK_END);
	//ファイル最後のアドレスを取得
	if ( ( len = ftell( wave_info.fp ) ) == -1 ) 
	{
		printX(s_task_no, ERR_YES|PRT_YES, "[%s:%d:%s]%s。",  __FILE__, __LINE__, __FUNCTION__, err_msg[ERR_NOT_FIND_DATA] );
		return ERR_NOT_FIND_DATA;
	}
	//ファイルのデータ部サイズを取得
	len = len - wave_info.play_start_pos;
	#endif
	len = wave_info.data_size;
	printf("len:%d\n", len);
	if (0 != (len % bytes_per_sample))
	{
		printX(s_task_no, ERR_YES|PRT_YES, "[%s:%d:%s]%s len=%d\n",  __FILE__, __LINE__, __FUNCTION__, err_msg[ERR_SIZE] , len);
		return 0;
	}
	
	//Master音声ボリュウムを設定
	set_volumn(MIXER_MASTER_NAME, vol.left_vol, vol.right_vol);
	//PCM音声ボリュウムを設定
	set_volumn(MIXER_PCM_NAME, vol.left_vol, vol.right_vol);
	//データ開始場所に移動
	fseek( wave_info.fp, wave_info.play_start_pos, SEEK_SET );
	//現在の場所を設定する
	current_location = wave_info.play_start_pos;
	
	//再生回数により設定
	while(refresh_cnt > 0)
	{
		//データ開始場所に移動
		fseek( wave_info.fp, wave_info.play_start_pos, SEEK_SET );
		//現在の場所を設定する
		current_location = wave_info.play_start_pos;
	
		printf("refresh_cnt:%d\n", refresh_cnt);
		while (!feof(wave_info.fp))
		{
			read_len= fread(buf, 1, sl_outburst, wave_info.fp);
			//ファイルの最後のデータかどうかをチェックする
			if (feof(wave_info.fp))
			{
				write_len = play(handler, buf, read_len, PLAY_FINAL_CHUNK);
				//snd_pcm_drain(handler);
			}
			else
			{
				write_len = play(handler, buf, read_len, 0);
			}
			
			current_location += write_len;
			//書き込んだデータ数と読み込んだデータ数が合わない場合、もう一度読み込んで放送する
			if (write_len != read_len)
			{
				fseek( wave_info.fp, current_location, SEEK_SET );
			}
		}
		//放送完了時、バッファにデータを全部放送し、バッファをクリアする
		snd_pcm_drain(handler);
		//次の放送の準備を行う
		if ((snd_pcm_prepare(handler)) < 0) 
		{
			printX(s_task_no, ERR_YES|PRT_YES, "[%s_%s_%d]%s\n", __FILE__, __FUNCTION__, __LINE__, err_msg[ERR_ALSA_PcmPrepareError]);
			return ERR_ALSA_PcmPrepareError;
		}
		
		//current_location = wave_info.play_start_pos;
		//fseek( wave_info.fp, wave_info.play_start_pos, SEEK_SET );
		refresh_cnt--;
	}
	return ERR_ZERO;	
}

