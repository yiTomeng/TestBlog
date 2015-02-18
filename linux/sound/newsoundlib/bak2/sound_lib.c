#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "sound_lib.h"
			

#define ERR_MSG_SIZE		64												//エラーメッセージサイズ
#define BUFSIZE				64000											//バッファサイズ
#define MIXER_MASTER_NAME	"Master"										//音声項目Master
#define MIXER_PCM_NAME		"PCM"											//音声項目PCM
#define DEVICE				"default"										//出力デバイス
#define FORMAT_U8			8												//8位量子化数
#define FORMAT_S16_LE		16												//16位量子化数
#define PLAY_FINAL_CHUNK	1												//最後かどうかのフラグ

#define DEF_ERR(_code, _str) [_code] = (_str)


//エラーメッセージ
static const char err_msg[][ERR_MSG_SIZE] = {
	DEF_ERR(ERR_ZERO, 					ERR_STR_ZERO),
	DEF_ERR(ERR_FILE_OPEN, 				ERR_STR_FILE_OPEN),		
	DEF_ERR(ERR_FILE_READ, 				ERR_STR_FILE_READ),		
	DEF_ERR(ERR_FILE_WRITE, 			ERR_STR_FILE_WRITE),		
	DEF_ERR(ERR_FILE_HEADER_READ,		ERR_STR_FILE_HEADER_READ),
	DEF_ERR(ERR_DEVICE_OPEN, 			ERR_STR_DEVICE_OPEN),	
	DEF_ERR(ERR_DEVICE_CLOSE, 			ERR_STR_DEVICE_CLOSE),			
	DEF_ERR(ERR_DEVICE_READ, 			ERR_STR_DEVICE_READ),		
	DEF_ERR(ERR_DEVICE_WRITE, 			ERR_STR_DEVICE_WRITE),		
	DEF_ERR(ERR_DEVICE_SET, 			ERR_STR_DEVICE_SET),		
	DEF_ERR(ERR_DEVICE_DESCRIPTOR,		ERR_STR_DEVICE_DESCRIPTOR),
	DEF_ERR(ERR_BROADCAST, 				ERR_STR_BROADCAST),		
	DEF_ERR(ERR_SIZE, 					ERR_STR_SIZE),				
	DEF_ERR(ERR_NOT_RIFF, 				ERR_STR_NOT_RIFF),			
	DEF_ERR(ERR_NOT_WAVE, 				ERR_STR_NOT_WAVE),			
	DEF_ERR(ERR_NOT_FIND_FMT_CHUNK, 	ERR_STR_NOT_FIND_FMT_CHUNK),
	DEF_ERR(ERR_NOT_FIND_DATA_CHUNK, 	ERR_STR_NOT_FIND_DATA_CHUNK),
	DEF_ERR(ERR_NOT_FIND_DATA,			ERR_STR_NOT_FIND_DATA),
	DEF_ERR(ERR_NOT_PCM, 				ERR_STR_NOT_PCM),			
	DEF_ERR(ERR_BITS_PER_SAMPLE, 		ERR_STR_BITS_PER_SAMPLE),	
	DEF_ERR(ERR_SET_FMT, 				ERR_STR_SET_FMT),			
	DEF_ERR(ERR_WRITE_RATE, 			ERR_STR_WRITE_RATE),		
	DEF_ERR(ERR_WRITE_CHANNEL,			ERR_STR_WRITE_CHANNEL)
};


static enum alsa_err_def
{
	ALSA_UnableToGetInitialParameters,
	ALSA_UnableToSetAccessType,
	ALSA_formatNotSupportedByHardware,
	ALSA_UnableToSetFormat,
	ALSA_UnableToSetChannels,
	ALSA_UnableToDisableResampling,
	ALSA_UnableToSetSamplerate2,
	ALSA_UnableToSetBufferTimeNear,
	ALSA_UnableToSetHwParameters,
	ALSA_UnableToGetBufferSize,
	ALSA_UnableToGetPeriodSize,
	ALSA_UnableToGetBoundary,
	ALSA_UnableToSetStartThreshold,
	ALSA_UnableToSetStopThreshold,
	ALSA_UnableToSetSilenceSize,
	ALSA_UnableToGetSwParameters,
	ALSA_MixerAttachError,
	ALSA_MixerRegisterError,
	ALSA_MixerLoadError,
	ALSA_UnableToFindSimpleControl,
	ALSA_UnableToSetPeriods
}alsa_err;



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


static snd_pcm_uframes_t sl_buffersize = 0; 
static snd_pcm_uframes_t sl_outburst = 0; 
static size_t bytes_per_sample = 0;
static int set_volumn(const char *mixer_name, long left_vol, long right_vol);
static int init(snd_pcm_t *handle, play_wave_info *wave_info);
static int play(snd_pcm_t *handler, void* data, int len, int flags);
				    
int open_device(snd_pcm_t **handle)
{
	int err = ERR_ZERO;
	
	//assert(*handle == NULL);
	if (NULL != *handle)
	{
		printf("[%s_%s_%d]%s", __FILE__, __FUNCTION__, __LINE__, err_msg[ERR_DEVICE_OPEN]);
		return ERR_DEVICE_OPEN;
	}
	if ((err = snd_pcm_open(handle, DEVICE, SND_PCM_STREAM_PLAYBACK, 0)) < 0)
	{
		printf("[%s_%s_%d]%s", __FILE__, __FUNCTION__, __LINE__, err_msg[ERR_DEVICE_OPEN]);
		return ERR_DEVICE_OPEN;
	}
	
	if (NULL == *handle)
	{
		printf("[%s_%s_%d]%s", __FILE__, __FUNCTION__, __LINE__, err_msg[ERR_DEVICE_OPEN]);
		return ERR_DEVICE_OPEN;
	}
	
	return ERR_ZERO;
}

int close_device(snd_pcm_t *handle)
{
	int err = ERR_ZERO;
	
	//assert(handle != NULL);
	if (NULL == handle)
	{
		printf("[%s_%s_%d]%s", __FILE__, __FUNCTION__, __LINE__, err_msg[ERR_DEVICE_CLOSE]);
		return ERR_DEVICE_CLOSE;
	}
	if ((err = snd_pcm_close(handle)) < 0)
	{
		printf("[%s_%s_%d]%s", __FILE__, __FUNCTION__, __LINE__, err_msg[ERR_DEVICE_CLOSE]);
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
		printf("[%s_%s_%d]%s %s", __FILE__, __FUNCTION__, __LINE__, err_msg[ERR_DEVICE_DROP], snd_strerror(err));
		return;
    }
    if ((err = snd_pcm_prepare(handle)) < 0)
    {
		printf("[%s_%s_%d]%s %s", __FILE__, __FUNCTION__, __LINE__, err_msg[ERR_DEVICE_PREPARE], snd_strerror(err));
		return;
    }
    return;
}
*/

/*
    open & setup audio device
    return: 1=success 0=fail
*/
static int init(snd_pcm_t *handle, play_wave_info *wave_info)
{
    unsigned int alsa_buffer_time = 500000; /* 0.5 s */
    unsigned int alsa_fragcount = 16;
    int err;
    int block;
    int format, rate, channel, bit_per_sample;
    
    
    snd_pcm_hw_params_t *alsa_hwparams;
	snd_pcm_sw_params_t *alsa_swparams;
	
    snd_pcm_uframes_t chunk_size;
    snd_pcm_uframes_t bufsize;
    snd_pcm_uframes_t boundary;
   
	assert(handle != NULL);
	
    //handle = NULL;
#if SND_LIB_VERSION >= 0x010005
    printf("alsa-init: using ALSA %s\n", snd_asoundlib_version());
#else
    printf("alsa-init: compiled for ALSA-%s\n", SND_LIB_VERSION_STR);
#endif

	bit_per_sample = wave_info->wave_header.bits_per_sample;
	rate = wave_info->wave_header.sample_rate;
	channel = wave_info->wave_header.num_channels;
	
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

    // setting hw-parameters
    if ((err = snd_pcm_hw_params_any(handle, alsa_hwparams)) < 0)
	{
		printf("%d:%s\n", ALSA_UnableToGetInitialParameters, snd_strerror(err));
	  	return 0;
	}

	
    err = snd_pcm_hw_params_set_access(handle, alsa_hwparams, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (err < 0) 
    {
		printf("%d:%s\n", ALSA_UnableToSetAccessType,
		       snd_strerror(err));
		return 0;
    }

      /* workaround for nonsupported formats
	 sets default format to S16_LE if the given formats aren't supported */
	if ((err = snd_pcm_hw_params_test_format(handle, alsa_hwparams, format)) < 0)
	{
		printf("%d:%d\n", ALSA_formatNotSupportedByHardware, (format));
        format = SND_PCM_FORMAT_S16_LE;
	}

	if ((err = snd_pcm_hw_params_set_format(handle, alsa_hwparams, format)) < 0)
	{
		printf("%d:%s\n",ALSA_UnableToSetFormat,
		snd_strerror(err));
		return 0;
	}

	if ((err = snd_pcm_hw_params_set_channels_near(handle, alsa_hwparams, &channel)) < 0)
	{
		printf("%d:%s\n", ALSA_UnableToSetChannels,
		snd_strerror(err));
		return 0;
	}

      /* workaround for buggy rate plugin (should be fixed in ALSA 1.0.11)
         prefer our own resampler, since that allows users to choose the resampler,
         even per file if desired */
         
#if SND_LIB_VERSION >= 0x010009
#if 0
	printf("rate before:%d\n", rate);
	
    if ((err = snd_pcm_hw_params_set_rate_resample(handle, alsa_hwparams, 0)) < 0)
	{
		printf("%d:%s\n",ALSA_UnableToDisableResampling,
		snd_strerror(err));
	  return 0;
	}
	
	printf("rate after1:%d\n", rate);
	#endif
#endif
/**/
    if ((err = snd_pcm_hw_params_set_rate_near(handle, alsa_hwparams,
 							&rate, NULL)) < 0)
//    if ((err = snd_pcm_hw_params_set_rate(handle, alsa_hwparams,
//						 rate, 0)) < 0)
    {
		 printf("%d:%s\n", ALSA_UnableToSetSamplerate2,
		 snd_strerror(err));
		 return 0;
    }
    
    bytes_per_sample =  bit_per_sample / 8;
    bytes_per_sample *= channel;

	if ((err = snd_pcm_hw_params_set_buffer_time_near(handle, alsa_hwparams,
							  &alsa_buffer_time, NULL)) < 0)
	{
	    printf("%d:%s\n", ALSA_UnableToSetBufferTimeNear,
		snd_strerror(err));
	    return 0;
	}

	if ((err = snd_pcm_hw_params_set_periods_near(handle, alsa_hwparams,
						      &alsa_fragcount, NULL)) < 0) 
	{
	     printf("%d:%s\n", ALSA_UnableToSetPeriods,
		 snd_strerror(err));
	     return 0;
	}

      /* finally install hardware parameters */
    if ((err = snd_pcm_hw_params(handle, alsa_hwparams)) < 0)
	{
	     printf("%d:%s\n", ALSA_UnableToSetHwParameters,
		 snd_strerror(err));
	     return 0;
	}
      // end setting hw-params


      // gets buffersize for control
    if ((err = snd_pcm_hw_params_get_buffer_size(alsa_hwparams, &bufsize)) < 0)
	{
	     printf("%d:%s\n", ALSA_UnableToGetBufferSize, snd_strerror(err));
	     return 0;
	}
    else 
    {
	    sl_buffersize = bufsize * bytes_per_sample;
	    printf("alsa-init: got buffersize=%i\n", sl_buffersize);
    }

    if ((err = snd_pcm_hw_params_get_period_size(alsa_hwparams, &chunk_size, NULL)) < 0) 
    {
		printf("%d:%s\n",ALSA_UnableToGetPeriodSize, snd_strerror(err));
		return 0;
    } 
    else 
    {
		printf("alsa-init: got period size %li\n", chunk_size);
    }
    
    sl_outburst = chunk_size * bytes_per_sample;

      /* setting software parameters */
    if ((err = snd_pcm_sw_params_current(handle, alsa_swparams)) < 0) 
    {
		printf("%d:%s\n", ALSA_UnableToGetSwParameters,
		       snd_strerror(err));
		return 0;
    }
#if SND_LIB_VERSION >= 0x000901
    if ((err = snd_pcm_sw_params_get_boundary(alsa_swparams, &boundary)) < 0) 
    {
		printf("%d:%s\n", ALSA_UnableToGetBoundary,
		       snd_strerror(err));
		return 0;
    }
#else
    boundary = 0x7fffffff;
#endif
      /* start playing when one period has been written */
    if ((err = snd_pcm_sw_params_set_start_threshold(handle, alsa_swparams, chunk_size)) < 0) 
    {
		printf("%d:%s\n", ALSA_UnableToSetStartThreshold,
		       snd_strerror(err));
		return 0;
    }
      /* disable underrun reporting */
    if ((err = snd_pcm_sw_params_set_stop_threshold(handle, alsa_swparams, boundary)) < 0) 
    {
		printf("%d:%s\n", ALSA_UnableToSetStopThreshold,
		       snd_strerror(err));
		return 0;
    }
#if SND_LIB_VERSION >= 0x000901
      /* play silence when there is an underrun */
    if ((err = snd_pcm_sw_params_set_silence_size(handle, alsa_swparams, boundary)) < 0) 
    {
		printf("%d:%s\n", ALSA_UnableToSetSilenceSize,
		       snd_strerror(err));
		return 0;
    }
#endif
    if ((err = snd_pcm_sw_params(handle, alsa_swparams)) < 0) 
    {
		printf("%d:%s\n", ALSA_UnableToGetSwParameters,
		       snd_strerror(err));
		return 0;
    }
      /* end setting sw-params */

    printf("alsa: %d Hz/%d channels/%d bpf/%d bytes buffer/%s\n",
	     rate, channel, (int)bytes_per_sample, sl_buffersize,
	     snd_pcm_format_description(format));

    // end switch handle (spdif)
    //alsa_can_pause = snd_pcm_hw_params_can_pause(alsa_hwparams);
    return 1;
} // end init



/*SOH*************************************************************************
 * NAME         : wave_read_file_header
 * FUNCTION     : 音声ファイルヘッダデータを読み込んで、チェックします
 * VERSION      : 01.00.00
 * IN/OUT       : (i/o) wave_info: 音声ファイルインフォーメーション　　
 * RETURN       : 0:OK >0:NG　
 * REMARKS      : 備考
 * HISTORY      :
 * ID-----------DATE--------NAME----------NOTE --------------------------------
 * [V01.00.00]  2015.02.02  KOCHI.晏         西JR BUS
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
			fprintf(stderr, "[%s:%d:%s()]%s。",  __FILE__, __LINE__, __FUNCTION__, err_msg[ERR_SIZE]);
			return ERR_SIZE;
		}
		else
		{
			fprintf(stderr, "[%s:%d:%s()]%s。",  __FILE__, __LINE__, __FUNCTION__, err_msg[ERR_FILE_READ]);
			return ERR_FILE_READ;
		}
	}
	/*チェック処理を行う*/
	//"RIFF"識別子をチェックする
	if ( strncmp( wave_info->wave_header.chunk_id, "RIFF", 4 ) != 0 ) 
	{
		fprintf(stderr, "[%s:%d:%s()]%s。",  __FILE__, __LINE__, __FUNCTION__, err_msg[ERR_NOT_RIFF] );
		return ERR_NOT_RIFF;
	}
	
	//フォーマット"WAVE"をチェックする
	if ( strncmp( wave_info->wave_header.format, "WAVE", 4 ) != 0 ) {
		fprintf(stderr, "[%s:%d:%s()]%s。",  __FILE__, __LINE__, __FUNCTION__, err_msg[ERR_NOT_WAVE] );
		return ERR_NOT_WAVE;
	}

	//"fmt"識別子をチェックする
	if ( strncmp(wave_info->wave_header.sub_chunk_id, "fmt ", 4) != 0)
	{
		fprintf(stderr, "[%s:%d:%s()]%s。",  __FILE__, __LINE__, __FUNCTION__, err_msg[ERR_NOT_FIND_FMT_CHUNK] );
		return ERR_NOT_FIND_FMT_CHUNK;
	}
	
	//無限ループでデータ部を探す	
	while ( 1 ) {
		len = fread( buf, 8, 1, wave_info->fp );								//識別子とサイズを読み込む
		if (len < 1)
		{
			if (feof(wave_info->fp))
			{
				fprintf(stderr, "[%s:%d:%s()]%s。",  __FILE__, __LINE__, __FUNCTION__, err_msg[ERR_SIZE]);
				return ERR_SIZE;
			}
			else
			{
				fprintf(stderr, "[%s:%d:%s()]%s。",  __FILE__, __LINE__, __FUNCTION__, err_msg[ERR_FILE_READ]);
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
				fprintf(stderr, "[%s:%d:%s()]%s。",  __FILE__, __LINE__, __FUNCTION__, err_msg[ERR_NOT_FIND_DATA_CHUNK] );
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
		fprintf(stderr, "[%s:%d:%s()]%s。",  __FILE__, __LINE__, __FUNCTION__, err_msg[ERR_NOT_FIND_DATA] );
		return ERR_NOT_FIND_DATA;
	}
	
	return ERR_ZERO;
}


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
	
	snd_mixer_selem_id_alloca(&sid);
	
	snd_mixer_selem_id_set_index(sid, mix_index);
    snd_mixer_selem_id_set_name(sid, mix_name);
  
#if 0  
	
	elem = snd_mixer_find_selem(handler, sid);
	
	snd_mixer_selem_get_playback_volume_range(elem, &min_vol, &max_vol);
	printf("[Before]%s  minVol:%d, maxVol:%d\n", mix_name, min_vol, max_vol);
	
	set_vol_left = (float)set_vol_left / 100 * (max_vol - min_vol) + min_vol;
	set_vol_right = (float)set_vol_right / 100 * (max_vol - min_vol) + min_vol;
#endif
/*	
	snd_mixer_selem_set_playback_volume_range(elem, 0, 100);
	
	snd_mixer_selem_get_playback_volume_range(elem, &min_vol, &max_vol);
	printf("[After]%s  minVol:%d, maxVol:%d\n", mix_name, min_vol, max_vol);
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
	printf("[Before END]%s  minVol:%d, maxVol:%d\n", mix_name, min_vol, max_vol);
#endif

	if ((err = snd_mixer_open(&handle, 0)) < 0) 
	{
		return CONTROL_ERROR;
    }

	if ((err = snd_mixer_attach(handle, DEVICE)) < 0) {
		printf("MSGTR_AO_ALSA_MixerAttachError:%s\n",snd_strerror(err));
		snd_mixer_close(handle);
		return CONTROL_ERROR;
	}
	
	if ((err = snd_mixer_selem_register(handle, NULL, NULL)) < 0)
	{
		printf("MSGTR_AO_ALSA_MixerRegisterError:%s\n", snd_strerror(err));
		snd_mixer_close(handle);
		return CONTROL_ERROR;
	}
	err = snd_mixer_load(handle);
	if (err < 0) 
	{
		printf("ALSA_MixerLoadError:%s\n", snd_strerror(err));
		snd_mixer_close(handle);
		return CONTROL_ERROR;
	}
	
	elem = snd_mixer_find_selem(handle, sid);
	if (!elem) 
	{
		printf("ALSA_UnableToFindSimpleControl:[name]%s  [index]%d\n",
				snd_mixer_selem_id_get_name(sid), snd_mixer_selem_id_get_index(sid));
				snd_mixer_close(handle);
		return CONTROL_ERROR;
	}
	
	snd_mixer_selem_get_playback_volume_range(elem,&pmin,&pmax);
	f_multi = (100 / (float)(pmax - pmin));
	

	//set_vol = vol->left / f_multi + pmin + 0.5;
	set_vol_left = set_vol_left / f_multi + pmin + 0.5;
	
	//setting channels
	if ((err = snd_mixer_selem_set_playback_volume(elem, SND_MIXER_SCHN_FRONT_LEFT, set_vol_left)) < 0) 
	{
		snd_mixer_close(handle);
		return CONTROL_ERROR;
	}
	
	
	set_vol_right = set_vol_right / f_multi + pmin + 0.5;
	if ((err = snd_mixer_selem_set_playback_volume(elem, SND_MIXER_SCHN_FRONT_RIGHT, set_vol_right)) < 0) 
	{
		snd_mixer_close(handle);
		return CONTROL_ERROR;
	}
	
	if (snd_mixer_selem_has_playback_switch(elem)) 
	{
		int lmute = (left_vol == 0);
		int rmute = (right_vol == 0);
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
	
	return CONTROL_OK;
}

/*
    plays 'len' bytes of 'data'
    returns: number of bytes played
    modified last at 29.06.02 by jp
    thanxs for marius <marius@rospot.com> for giving us the light ;)
*/

static int play(snd_pcm_t *handler, void* data, int len, int flags)
{
	int num_frames;
	snd_pcm_sframes_t res = 0;
	if (!(flags & PLAY_FINAL_CHUNK))
	    len = len / sl_outburst * sl_outburst;
	    
	num_frames = len / bytes_per_sample;

	//mp_msg(MSGT_AO,MSGL_ERR,"alsa-play: frames=%i, len=%i\n",num_frames,len);

	assert(handler != NULL);
	
	if (!handler) 
	{
		//mp_msg(MSGT_AO,MSGL_ERR,MSGTR_AO_ALSA_DeviceConfigurationError);
		return 0;
	}

	if (num_frames == 0)
	{
	  return 0;
	}
	
	do 
	{
		res = snd_pcm_writei(handler, data, num_frames);

		if (res == -EINTR) 
		{
			/* nothing to do */
			res = 0;
	    }
	    else if (res == -ESTRPIPE) 
	    {	/* suspend */
			//mp_msg(MSGT_AO,MSGL_INFO,MSGTR_AO_ALSA_PcmInSuspendModeTryingResume);
			while ((res = snd_pcm_resume(handler)) == -EAGAIN)
				sleep(1);
	    }
	    if (res < 0) 
	    {
			//mp_msg(MSGT_AO,MSGL_ERR,MSGTR_AO_ALSA_WriteError, snd_strerror(res));
			//mp_msg(MSGT_AO,MSGL_INFO,MSGTR_AO_ALSA_TryingToResetSoundcard);
			if ((res = snd_pcm_prepare(handler)) < 0) 
			{
			  //mp_msg(MSGT_AO,MSGL_ERR,MSGTR_AO_ALSA_PcmPrepareError, snd_strerror(res));
			  break;
			}
			res = 0;
	    }
	} while (res == 0);

	return res < 0 ? 0 : res * bytes_per_sample;
}


int play_file(snd_pcm_t *handler, const char *file_name, int vol_left, int vol_right)
{
	
	play_wave_info wave_info;
	char buf[BUFSIZE];
	int read_len = 0, write_len = 0;
	int current_location = 0;
	int len = 0;
	memset(buf, 0x00, BUFSIZE);
	
	//assert(handler != NULL);
	//assert(file_name != NULL);
	if (NULL == handler)
	{
		printf("[%s_%s_%d]%s", __FILE__, __FUNCTION__, __LINE__, err_msg[ERR_DEVICE_CLOSE]);
		return ERR_DEVICE_CLOSE;
	}
	
	if ((wave_info.fp = fopen(file_name, "r")) < 0)
	{
		fprintf (stderr, "cannot open file %s\n", 
			 file_name);
		exit (1);
	}
	
	wave_read_file_header(&wave_info);
	
	init(handler, &wave_info);
	
	fseek( wave_info.fp, 0, SEEK_END);
	
	if ( ( len = ftell( wave_info.fp ) ) == -1 ) 
	{
		fprintf(stderr, "[%s:%d:%s()]%s。",  __FILE__, __LINE__, __FUNCTION__, err_msg[ERR_NOT_FIND_DATA] );
		return ERR_NOT_FIND_DATA;
	}
		char a = fgetc(wave_info.fp);
	printf("%d\n", a);
	//printf("len %d\n", len);
	len = len - wave_info.play_start_pos;
	if (0 != (len % bytes_per_sample))
	{
		printf("size error%d\n", len);
		return 0;
	}
	
	set_volumn(MIXER_MASTER_NAME, vol_left, vol_right);

	set_volumn(MIXER_PCM_NAME, vol_left, vol_right);
	
	fseek( wave_info.fp, wave_info.play_start_pos, SEEK_SET );
	

	
	current_location = wave_info.play_start_pos;
	
	while (!feof(wave_info.fp))
	{
		read_len= fread(buf, 1, sl_outburst, wave_info.fp);
		//printf ("cann2ot rqqrt file %d\n", read_len);
		if (feof(wave_info.fp))
		{
			write_len = play(handler, buf, read_len, PLAY_FINAL_CHUNK);
			snd_pcm_drain(handler);
			//fprintf (stderr, "cann2ot rqqrt file %s\n", file_name);
		}
		else
		{
			write_len = play(handler, buf, read_len, 0);
			//fprintf (stderr, "3cannot rqqrt file %s\n", file_name);
		}
		
		current_location += write_len;
		
		if (write_len != read_len)
		{
			fseek( wave_info.fp, current_location, SEEK_SET );
			//fprintf (stderr, "canno4t rqqrt file %s\n", file_name);
		}
		
	}
	//fprintf (stderr, "cannot rqqrt file %s\n", file_name);
	
	return ERR_ZERO;	
}

