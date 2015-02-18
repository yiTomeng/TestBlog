#include <alsa/asoundlib.h>

#define FILE_NAME 32					//ファイル名サイズ

#define	ERR_STR_ZERO 					 "エラー無し。"
#define	ERR_STR_FILE_OPEN 				 "ファイルopen失敗しました。"
#define	ERR_STR_FILE_READ 				 "ファイルの読み込みは失敗しました。"
#define	ERR_STR_FILE_WRITE	 			 ""
#define	ERR_STR_FILE_HEADER_READ 		 "WAVファイルヘッダの読み込みは失敗しました。"
#define	ERR_STR_DEVICE_OPEN 			 "デバイスopen失敗しました。"
#define	ERR_STR_DEVICE_CLOSE 			 "デバイスclose失敗しました。"
#define	ERR_STR_DEVICE_READ				 ""
#define	ERR_STR_DEVICE_WRITE 			 "DSPデバイスに書き込むことが失敗しました。"
#define	ERR_STR_DEVICE_SET 				 "DSPデバイス設定失敗しました。"
#define	ERR_STR_DEVICE_DESCRIPTOR		 "ファイル識別子異常"
#define	ERR_STR_DEVICE_DROP				 "デバイス停止エラー発生"
#define	ERR_STR_DEVICE_PREPARE		 	 "デバイス準備エラー発生"
#define	ERR_STR_BROADCAST 				 "放送失敗しました。"
#define	ERR_STR_SIZE					 "ファイルサイズはヘッダサイズより小さい。"
#define	ERR_STR_NOT_RIFF 				 "Specified file is not RIFF file.\n"
#define	ERR_STR_NOT_WAVE				 "Specified file is not WAVE file.\n"
#define	ERR_STR_NOT_FIND_FMT_CHUNK 		 "Failed to find fmt chunk.\n"
#define	ERR_STR_NOT_FIND_DATA_CHUNK 	 "Failed to find data chunk.\n"
#define	ERR_STR_NOT_FIND_DATA 			 "Failed to find offset of PCM data.\n"
#define	ERR_STR_NOT_PCM 				 "Specified file's sound data is not PCM format.\n"
#define	ERR_STR_BITS_PER_SAMPLE 		 "Specified file's sound data is %d bits, not 8 nor 16 bits.\n"
#define	ERR_STR_SET_FMT 				 "ioctl( SOUND_PCM_SETFMT )"
#define	ERR_STR_WRITE_RATE 				 "ioctl( SOUND_PCM_WRITE_RATE )"
#define	ERR_STR_WRITE_CHANNEL 			 "ioctl( SOUND_PCM_WRITE_CHANNELS )"

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

/*
#define	ERR_STR_ZERO 					 "?G???[?????B"
#define	ERR_STR_FILE_OPEN 				 "?t?@?C??open???s???܂????B"
#define	ERR_STR_FILE_READ 				 "?t?@?C???̓ǂݍ??݂͎??s???܂????B"
#define	ERR_STR_FILE_WRITE	 			 ""
#define	ERR_STR_FILE_HEADER_READ 		 "WAV?t?@?C???w?b?_?̓ǂݍ??݂͎??s???܂????B"
#define	ERR_STR_DEVICE_OPEN 			 "?f?o?C?Xopen???s???܂????B"
#define	ERR_STR_DEVICE_READ				 ""
#define	ERR_STR_DEVICE_WRITE 			 "DSP?f?o?C?X?ɏ??????ނ??Ƃ????s???܂????B"
#define	ERR_STR_DEVICE_SET 				 "DSP?f?o?C?X?ݒ莸?s???܂????B"
#define	ERR_STR_DEVICE_DESCRIPTOR		 "?t?@?C?????ʎq?ُ・
#define	ERR_STR_BROADCAST 				 "??s???܂????B"
#define	ERR_STR_SIZE					 "?t?@?C???T?C?Y?̓w?b?_?T?C?Y?謔・??????B"
#define	ERR_STR_NOT_RIFF 				 "Specified file is not RIFF file.\n"
#define	ERR_STR_NOT_WAVE				 "Specified file is not WAVE file.\n"
#define	ERR_STR_NOT_FIND_FMT_CHUNK 		 "Failed to find fmt chunk.\n"
#define	ERR_STR_NOT_FIND_DATA_CHUNK 	 "Failed to find data chunk.\n"
#define	ERR_STR_NOT_FIND_DATA 			 "Failed to find offset of PCM data.\n"
#define	ERR_STR_NOT_PCM 				 "Specified file's sound data is not PCM format.\n"
#define	ERR_STR_BITS_PER_SAMPLE 		 "Specified file's sound data is %d bits, not 8 nor 16 bits.\n"
#define	ERR_STR_SET_FMT 				 "ioctl( SOUND_PCM_SETFMT )"
#define	ERR_STR_WRITE_RATE 				 "ioctl( SOUND_PCM_WRITE_RATE )"
#define	ERR_STR_WRITE_CHANNEL 			 "ioctl( SOUND_PCM_WRITE_CHANNELS )"
*/

#define CONTROL_ERROR 	-1
#define CONTROL_OK 		0

//int set_volumn(char *mixer_name, long left_vol, long right_vol);

extern int open_device(snd_pcm_t **handle);
extern close_device(snd_pcm_t *handle);

extern int play_file(snd_pcm_t *handler, const char *file_name, int vol_left, int vol_right);
