#ifndef SOUND_LIB_ERR_H
#define SOUND_LIB_ERR_H

#define	ERR_STR_ZERO 									 "エラー無し。"

#define	ERR_STR_FILE_OPEN 								 "ファイルopen失敗しました"
#define	ERR_STR_FILE_READ 								 "ファイルの読み込みは失敗しました"
#define	ERR_STR_FILE_WRITE	 							 ""
#define	ERR_STR_FILE_HEADER_READ 						 "WAVファイルヘッダの読み込みは失敗しました"

#define	ERR_STR_DEVICE_OPEN 							 "デバイスopen失敗しました"
#define	ERR_STR_DEVICE_CLOSE 							 "デバイスclose失敗しました"
#define	ERR_STR_DEVICE_READ								 ""
#define	ERR_STR_DEVICE_WRITE 							 "DSPデバイスに書き込むことが失敗しました"
#define	ERR_STR_DEVICE_SET 								 "DSPデバイス設定失敗しました"
#define	ERR_STR_DEVICE_DESCRIPTOR						 "ファイル識別子異常"
#define	ERR_STR_DEVICE_DROP								 "デバイス停止エラー発生"
#define	ERR_STR_DEVICE_PREPARE						 	 "デバイス準備エラー発生"

#define	ERR_STR_BROADCAST 								 "放送失敗しました"
#define	ERR_STR_SIZE									 "ファイルサイズはヘッダサイズより小さい"
#define	ERR_STR_HANDLE_NULL		 						 "ハンドルはNULL"
#define	ERR_STR_HANDLE_NOT_NULL		 					 "ハンドルはNULLではありません"
#define	ERR_STR_PLAYBACK_NO_FRAME		 				 "再生できるフレームがありません"

#define	ERR_STR_NOT_RIFF 								 "Specified file is not RIFF file.\n"
#define	ERR_STR_NOT_WAVE								 "Specified file is not WAVE file.\n"
#define	ERR_STR_NOT_FIND_FMT_CHUNK 						 "Failed to find fmt chunk.\n"
#define	ERR_STR_NOT_FIND_DATA_CHUNK 					 "Failed to find data chunk.\n"
#define	ERR_STR_NOT_FIND_DATA 							 "Failed to find offset of PCM data.\n"
#define	ERR_STR_NOT_PCM 								 "Specified file's sound data is not PCM format.\n"

#define	ERR_STR_BITS_PER_SAMPLE 						 "Specified file's sound data is %d bits, not 8 nor 16 bits.\n"
#define	ERR_STR_SET_FMT 								 "ioctl( SOUND_PCM_SETFMT )"
#define	ERR_STR_WRITE_RATE 								 "ioctl( SOUND_PCM_WRITE_RATE )"
#define	ERR_STR_WRITE_CHANNEL 							 "ioctl( SOUND_PCM_WRITE_CHANNELS )"


#define	ERR_STR_ALSA_UnableToGetInitialParameters		 "パラメータ初期化失敗しました"
#define	ERR_STR_ALSA_UnableToSetAccessType				 "アクセスタイプの設定は失敗しました"
#define	ERR_STR_ALSA_formatNotSupportedByHardware		 "このフォーマットはデバイスがサポートされていません"
#define	ERR_STR_ALSA_UnableToSetFormat					 "このフォーマットは設定できません"
#define	ERR_STR_ALSA_UnableToSetChannels				 "このチャンネルは設定できません"
#define	ERR_STR_ALSA_UnableToDisableResampling			 "リサンプリング禁止の設定は失敗しました"
#define	ERR_STR_ALSA_UnableToSetSamplerate2				 "周波数の設定は失敗しました"
#define	ERR_STR_ALSA_UnableToSetBufferTimeNear			 "バッファタイマー設定失敗しました"
#define	ERR_STR_ALSA_UnableToSetHwParameters			 "ハードウェアの設定は失敗しました"
#define	ERR_STR_ALSA_UnableToGetBufferSize				 "バッファサイズの取得は失敗しました"
#define	ERR_STR_ALSA_UnableToGetPeriodSize				 "Periodサイズの取得は失敗しました"
#define	ERR_STR_ALSA_UnableToGetBoundary				 "Boundaryの取得は失敗しました"
#define	ERR_STR_ALSA_UnableToSetStartThreshold			 "StartThresholdの設定は失敗しました"
#define	ERR_STR_ALSA_UnableToSetStopThreshold			 "StopThresholdの設定は失敗しました"
#define	ERR_STR_ALSA_UnableToSetSilenceSize				 "SilenceSizeの設定は失敗しました"
#define	ERR_STR_ALSA_UnableToGetSwParameters			 "ソフトウェアパラメータの取得は失敗しました"
#define	ERR_STR_ALSA_UnableToFindSimpleControl			 "項目を見つけません"
#define	ERR_STR_ALSA_UnableToSetPeriods					 "Periodsは設定できません"	
#define	ERR_STR_ALSA_PcmInSuspendModeTryingResume		 "停止モードでResumeを試す"
#define	ERR_STR_ALSA_TryingToResetSoundcard				 "リセット音声カード"
#define	ERR_STR_ALSA_PcmPrepareError					 "PCMの準備は失敗しました"

#define	ERR_STR_ALSA_MixerOpenError						 "ミキサーオープンは失敗しました"
#define	ERR_STR_ALSA_MixerAttachError					 "ミキサー接続失敗しました"
#define	ERR_STR_ALSA_MixerRegisterError					 "ミキサー登録失敗しました"
#define	ERR_STR_ALSA_MixerLoadError						 "ミキサーロード失敗しました"
#define	ERR_STR_ALSA_MixerSetVolumeError				 "ミキサー音声ボリュウム設定は失敗しました"

//エラーメッセージコード
enum ERROR_CODE
{
	ERR_ZERO,
	
	ERR_FILE_OPEN,
	ERR_FILE_READ,
	ERR_FILE_WRITE,
	ERR_FILE_HEADER_READ,
	
	ERR_DEVICE_OPEN,
	ERR_DEVICE_CLOSE,
	ERR_DEVICE_READ,
	ERR_DEVICE_WRITE,
	ERR_DEVICE_SET,
	ERR_DEVICE_DESCRIPTOR,
	ERR_DEVICE_DROP,
	ERR_DEVICE_PREPARE,
	
	ERR_BROADCAST,
	ERR_SIZE,
	ERR_HANDLE_NULL,		
	ERR_HANDLE_NOT_NULL,
	ERR_PLAYBACK_NO_FRAME,
	
	ERR_NOT_RIFF,
	ERR_NOT_WAVE,
	ERR_NOT_FIND_FMT_CHUNK,
	ERR_NOT_FIND_DATA_CHUNK,
	ERR_NOT_FIND_DATA,
	ERR_NOT_PCM,
	
	ERR_BITS_PER_SAMPLE,
	ERR_SET_FMT,
	ERR_WRITE_RATE,
	ERR_WRITE_CHANNEL,
	
	ERR_ALSA_UnableToGetInitialParameters,
	ERR_ALSA_UnableToSetAccessType,
	ERR_ALSA_formatNotSupportedByHardware,
	ERR_ALSA_UnableToSetFormat,
	ERR_ALSA_UnableToSetChannels,
	ERR_ALSA_UnableToDisableResampling,
	ERR_ALSA_UnableToSetSamplerate2,
	ERR_ALSA_UnableToSetBufferTimeNear,
	ERR_ALSA_UnableToSetHwParameters,
	ERR_ALSA_UnableToGetBufferSize,
	ERR_ALSA_UnableToGetPeriodSize,
	ERR_ALSA_UnableToGetBoundary,
	ERR_ALSA_UnableToSetStartThreshold,
	ERR_ALSA_UnableToSetStopThreshold,
	ERR_ALSA_UnableToSetSilenceSize,
	ERR_ALSA_UnableToGetSwParameters,
	ERR_ALSA_UnableToFindSimpleControl,
	ERR_ALSA_UnableToSetPeriods,
	ERR_ALSA_PcmInSuspendModeTryingResume,
	ERR_ALSA_TryingToResetSoundcard,
	ERR_ALSA_PcmPrepareError,
	ERR_ALSA_MixerOpenError,
	ERR_ALSA_MixerAttachError,
	ERR_ALSA_MixerRegisterError,
	ERR_ALSA_MixerLoadError,
	ERR_ALSA_MixerSetVolumeError		
};  
#endif
