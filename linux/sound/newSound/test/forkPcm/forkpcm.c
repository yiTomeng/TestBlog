#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>

#define ERR_MSG_SIZE		64												//�G���[���b�Z�[�W
#define BUFSIZE				64000

enum ERROR_CODE
{
	ERR_ZERO,
	ERR_FILE_OPEN,
	ERR_FILE_READ,
	ERR_FILE_WRITE,
	ERR_FILE_HEADER_READ,
	ERR_DEVICE_OPEN,
	ERR_DEVICE_READ,
	ERR_DEVICE_WRITE,
	ERR_DEVICE_SET,
	ERR_DEVICE_DESCRIPTOR,
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


//�G���[���b�Z�[�W
static const char err_msg[][ERR_MSG_SIZE] = {
	[ERR_ZERO] 					= "�G���[�����B",
	[ERR_FILE_OPEN] 			= "�t�@�C��open���s���܂����B",
	[ERR_FILE_READ] 			= "�t�@�C���̓ǂݍ��݂͎��s���܂����B",
	[ERR_FILE_WRITE] 			= "",
	[ERR_FILE_HEADER_READ] 		= "WAV�t�@�C���w�b�_�̓ǂݍ��݂͎��s���܂����B",
	[ERR_DEVICE_OPEN] 			= "�f�o�C�Xopen���s���܂����B",
	[ERR_DEVICE_READ] 			= "",
	[ERR_DEVICE_WRITE] 			= "DSP�f�o�C�X�ɏ������ނ��Ƃ����s���܂����B",
	[ERR_DEVICE_SET] 			= "DSP�f�o�C�X�ݒ莸�s���܂����B",
	[ERR_DEVICE_DESCRIPTOR]		= "�t�@�C�����ʎq�ُ�",
	[ERR_BROADCAST] 			= "�������s���܂����B",
	[ERR_SIZE] 					= "�t�@�C���T�C�Y�̓w�b�_�T�C�Y��菬�����B",
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

//WAV�t�@�C���̃w�b�_
typedef struct
{
	char	chunk_id[4];													//���ʎq"RIFF"
	int		chunk_size;														//�t�@�C���̃T�C�Y(���ʎq"RIFF"�Ǝ���������)
	char 	format[4];														//�t�H�[�}�b�g��("WAVE")
	char 	sub_chunk_id[4];												//���ʎq"fmt "
	int  	sub_chunk_size;													//���̌�̃t�@�C���̃T�C�Y
	short 	audio_format;													//�����̃t�H�[�}�b�g(1:PCM �@1�ȊO�͑��̈��k�t�@�C��)
	short 	num_channels;													//�`�����l��
	int 	sample_rate;													//�T���v�����O���g��
	int 	byte_rate;														//1�b�Ɏg�p����o�C�g��
	short 	block_align;													//�u���b�N�̃A���C�������g(���g���Ă��Ȃ�)
	short 	bits_per_sample;												//�ʎq���̃r�b�g��
}wave_file_header_info;

//WAV�t�@�C���̃C���t�H�[���[�V����
typedef struct
{
	FILE 	*fp;															//�����t�@�C�����ʎq
	int 	stop;															//���~���(�ۗ��@���g���Ă��Ȃ�)
	int 	data_size;	 													//�f�[�^���̃T�C�Y
	int 	refresh_times;													//�Đ���
	unsigned int play_pos;													//��~�ꏊ(�ۗ��@���g���Ă��Ȃ�)
	unsigned int play_start_pos;											//�f�[�^�J�n�ʒu
	char 	play_filename[32];												//�����t�@�C����
	wave_file_header_info wave_header;										//WAV�t�@�C���w�b�_
}play_wave_info;
/*SOH*************************************************************************
 * NAME         : wave_read_file_header
 * FUNCTION     : �����t�@�C���w�b�_�f�[�^��ǂݍ���ŁA�`�F�b�N���܂�
 * VERSION      : 01.00.00
 * IN/OUT       : (i/o) wave_info: �����t�@�C���C���t�H�[���[�V�����@�@
 * RETURN       : 0:OK >0:NG�@
 * REMARKS      : ���l
 * HISTORY      :
 * ID-----------DATE--------NAME----------NOTE --------------------------------
 * [V01.00.00]  2015.02.02  KOCHI.��         ��JR BUS
  *************************************************************************EOH*/

static int wave_read_file_header(play_wave_info *wave_info)
{
	int len = 0;							//�Վ��ϐ��@�T�C�Y
	char buf[BUFSIZE];						//�Վ��o�b�t�@
	
	memset(buf, 0x00, BUFSIZE);				//�Վ��o�b�t�@������������
	
	//�t�@�C���w�b�_���o�b�t�@�ɓǂݍ���
	//fread( buf, 1, HEADER_BUFSIZE, wave_info->fp );
	
	//�o�b�t�@����t�@�C���w�b�_���\���̂ɃR�s�[����
	//memcpy(&wave_info->wave_header, buf, HEADER_BUFSIZE);
	
	//�t�@�C���w�b�_���o�b�t�@�ɓǂݍ���
	len = fread( &wave_info->wave_header, 1, sizeof(wave_file_header_info), wave_info->fp );
	
	if (len < sizeof(wave_file_header_info))
	{
		if (feof(wave_info->fp))
		{
			fprintf(stderr, "[%s:%d:%s()]%s�B",  __FILE__, __LINE__, __FUNCTION__, err_msg[ERR_SIZE]);
			return ERR_SIZE;
		}
		else
		{
			fprintf(stderr, "[%s:%d:%s()]%s�B",  __FILE__, __LINE__, __FUNCTION__, err_msg[ERR_FILE_READ]);
			return ERR_FILE_READ;
		}
	}
	/*�`�F�b�N�������s��*/
	//"RIFF"���ʎq���`�F�b�N����
	if ( strncmp( wave_info->wave_header.chunk_id, "RIFF", 4 ) != 0 ) 
	{
		fprintf(stderr, "[%s:%d:%s()]%s�B",  __FILE__, __LINE__, __FUNCTION__, err_msg[ERR_NOT_RIFF] );
		return ERR_NOT_RIFF;
	}
	
	//�t�H�[�}�b�g"WAVE"���`�F�b�N����
	if ( strncmp( wave_info->wave_header.format, "WAVE", 4 ) != 0 ) {
		fprintf(stderr, "[%s:%d:%s()]%s�B",  __FILE__, __LINE__, __FUNCTION__, err_msg[ERR_NOT_WAVE] );
		return ERR_NOT_WAVE;
	}

	//"fmt"���ʎq���`�F�b�N����
	if ( strncmp(wave_info->wave_header.sub_chunk_id, "fmt ", 4) != 0)
	{
		fprintf(stderr, "[%s:%d:%s()]%s�B",  __FILE__, __LINE__, __FUNCTION__, err_msg[ERR_NOT_FIND_FMT_CHUNK] );
		return ERR_NOT_FIND_FMT_CHUNK;
	}
	
	//�������[�v�Ńf�[�^����T��	
	while ( 1 ) {
		len = fread( buf, 8, 1, wave_info->fp );								//���ʎq�ƃT�C�Y��ǂݍ���
		if (len < 1)
		{
			if (feof(wave_info->fp))
			{
				fprintf(stderr, "[%s:%d:%s()]%s�B",  __FILE__, __LINE__, __FUNCTION__, err_msg[ERR_SIZE]);
				return ERR_SIZE;
			}
			else
			{
				fprintf(stderr, "[%s:%d:%s()]%s�B",  __FILE__, __LINE__, __FUNCTION__, err_msg[ERR_FILE_READ]);
				return ERR_FILE_READ;
			}
		}
		
		len = *( int* )( &buf[4] );										//�T�C�Y��len�ɕۑ�����

		//"data"���ʎq���`�F�b�N����A"data"�łȂ���΁Alen�o�C�g���e���΂��āA���̃G���A���`�F�b�N����
		if ( strncmp( buf, "data", 4 ) != 0 ) 
		{
			//�ʒulen�o�C�g���ݒ肵�A"data"��T������
			if ( fseek( wave_info->fp, len, SEEK_CUR ) == -1 ) 
			{
				fprintf(stderr, "[%s:%d:%s()]%s�B",  __FILE__, __LINE__, __FUNCTION__, err_msg[ERR_NOT_FIND_DATA_CHUNK] );
				return ERR_NOT_FIND_DATA_CHUNK;
			}
		}
		else 
		{
			break;
		}
	}

	//�f�[�^���T�C�Y��ݒ肷��
	wave_info->data_size = len;

	//�f�[�^�̊J�n�ʒu��ۑ����Ȃ���A�`�F�b�N���܂�
	if ( ( wave_info->play_start_pos = ftell( wave_info->fp ) ) == -1 ) 
	{
		fprintf(stderr, "[%s:%d:%s()]%s�B",  __FILE__, __LINE__, __FUNCTION__, err_msg[ERR_NOT_FIND_DATA] );
		return ERR_NOT_FIND_DATA;
	}
	
	return ERR_ZERO;
}

 
main (int argc, char *argv[])
{
	int i;
	int err;
	char buf[BUFSIZE];
	snd_pcm_t *playback_handle;
	snd_pcm_hw_params_t *hw_params;
	FILE *fp;
	play_wave_info wave_info;
	if (argc < 3)
	{
		fprintf (stderr, "parameters are not enough\n ");
		exit (1);
	}
	
	if ((wave_info.fp = fopen(argv[2], "r")) < 0)
	{
		fprintf (stderr, "cannot open file %s\n", 
			 argv[2]);
		exit (1);
	}
	
	wave_read_file_header(&wave_info);
do
{	
	if ((err = snd_pcm_open (&playback_handle, argv[1], SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
		fprintf (stderr, "cannot open audio device %s (%s)\n", 
			 argv[1],
			 snd_strerror (err));
		exit (1);
	}
	/*
	if ((err = snd_pcm_nonblock(playback_handle, 0)) < 0) 
	{
         printf("err:%s ", snd_strerror(err));
    } 
    else 
    {
		printf("alsa-init: device reopened in blocking mode\n");
    }
     */
//	printf("1\n");   
	if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
		fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
			 snd_strerror (err));
		exit (1);
	}
//	printf("2\n");		 
	if ((err = snd_pcm_hw_params_any (playback_handle, hw_params)) < 0) {
		fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
			 snd_strerror (err));
		exit (1);
	}
//	printf("3\n");
	if ((err = snd_pcm_hw_params_set_access (playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
		fprintf (stderr, "cannot set access type (%s)\n",
			 snd_strerror (err));
		exit (1);
	}
//	printf("4\n");
	if ((err = snd_pcm_hw_params_set_format (playback_handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
		fprintf (stderr, "cannot set sample format (%s)\n",
			 snd_strerror (err));
		exit (1);
	}
//	printf("5\n");
	//int rate = 44100;
	int rate = wave_info.wave_header.sample_rate;
	printf("rate:%d\n", rate);
	if ((err = snd_pcm_hw_params_set_rate_near (playback_handle, hw_params, &rate, 0)) < 0) {
		fprintf (stderr, "cannot set sample rate (%s)\n",
			 snd_strerror (err));
		exit (1);
	}
//	printf("6\n");
	if ((err = snd_pcm_hw_params_set_channels (playback_handle, hw_params, 2)) < 0) {
		fprintf (stderr, "cannot set channel count (%s)\n",
			 snd_strerror (err));
		exit (1);
	}
//	printf("7\n");
	if ((err = snd_pcm_hw_params (playback_handle, hw_params)) < 0) {
		fprintf (stderr, "cannot set parameters (%s)\n",
			 snd_strerror (err));
		exit (1);
	}
	
	int pause_able = snd_pcm_hw_params_can_pause(hw_params);
	printf("pause_able:%d\n", pause_able);
	
	snd_pcm_uframes_t size = 0;
	int dir = 0;
	err = snd_pcm_hw_params_get_period_size(hw_params, &size, &dir);
    if (err < 0) {
	    printf("Unable to get period size for playback: %s\n", snd_strerror(err));
	    return err;
    }
    
    
//	printf("8\n");
	snd_pcm_hw_params_free (hw_params);

	if ((err = snd_pcm_prepare (playback_handle)) < 0) {
		fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
			 snd_strerror (err));
		exit (1);
	}
//	printf("9\n");

	
    
    
#if 0
	
	
	snd_pcm_uframes_t buffer_size = 0;
	err = snd_pcm_hw_params_get_buffer_size (hw_params, &buffer_size);
	if (err < 0) {
	    printf("Unable to get period size for playback: %s\n", snd_strerror(err));
	    return err;
    }
    printf("buffer_size: %d\n", buffer_size);
    
    buffer_size = 64000;
	err = snd_pcm_hw_params_set_buffer_size (playback_handle, hw_params, &buffer_size);
    printf("buffer_size after: %d\n", buffer_size);
    
 	
    
    snd_pcm_uframes_t start_threshold = 0;
    snd_pcm_sw_params_t *sw_params;
    snd_pcm_sw_params_malloc(&sw_params);
    snd_pcm_sw_params_current (playback_handle, sw_params);
    snd_pcm_sw_params_get_start_threshold(sw_params, &start_threshold);
	printf("start_threshold: %d\n", start_threshold);
	start_threshold = 6580;
	
	snd_pcm_sw_params_set_start_threshold(playback_handle, sw_params, &start_threshold);
	printf("start_threshole After: %d\n", start_threshold);
	
	snd_pcm_uframes_t stop_threshold = 0;
    //snd_pcm_sw_params_t *sw_params;
    //snd_pcm_sw_params_malloc(&sw_params);
    //snd_pcm_sw_params_current (playback_handle, sw_params);
    snd_pcm_sw_params_get_stop_threshold(sw_params, &stop_threshold);
	printf("stop_threshold: %d\n", stop_threshold);
	stop_threshold = buffer_size;
	
	snd_pcm_sw_params_set_stop_threshold(playback_handle, sw_params, &stop_threshold);
	printf("stop_threshold After: %d\n", stop_threshold);
#endif	
	
	
    printf("size:%d\n", size); 
	//for (i = 0; i < 10; ++i) {
	//�f�[�^�̊J�n�ꏊ�ɐݒ肷��
	fseek( wave_info.fp, wave_info.play_start_pos, SEEK_SET );
	printf("pos: %d\n", wave_info.play_start_pos);
	int cnt = 0;
	int len = 0;
	//len = BUFSIZ / size * size * 4;		//OK
	//len = size*4;							//NG
	//len = BUFSIZ / size * size;			//OK
	len = size * 4 ;
	int num = 0;
//	int end = 0;
	//int num = len / wave_info.wave_header.bits_per_sample;
pid_t pid;
loop:	 pid = fork();
	
	if (pid == 0)
	{
		int a = 0;
		while(1)
		{
			int state = 0;
			
			while (!feof(wave_info.fp))
			{
				a++;
				if (a == 60)
				{
					snd_pcm_drain(playback_handle);
					//snd_pcm_close(playback_handle);
					exit(1);
				}
				state = snd_pcm_state(playback_handle);
				//printf("state:%d\n" ,state);
				if (SND_PCM_STATE_DRAINING == state)
				{
					printf("Snd draining\n");
					exit(1);
				}
				num = fread(buf, 1, len , wave_info.fp);
				//printf("cnt: %d\n", cnt++);
				//printf("num: %d\n", num);
				
				if ((err = snd_pcm_writei (playback_handle, buf, num/4)) != (num/4)) 
				{
					printf("err:%d\n", err);
					fprintf (stderr, "write to audio interface failed (%s)\n",
							 snd_strerror (err));
					//snd_pcm_recover(playback_handle, err, 0);
					//exit (1);
				}
				
				#if 0
				if ((err = snd_pcm_writei (playback_handle, buf, num / 4)) < 0) 
				{
					printf("err:%d\n", err);
					fprintf (stderr, "write to audio interface failed (%s)\n",
							 snd_strerror (err));
					
					exit (1);
				}
				#endif
				//sleep(1);  
		        if (err == -EPIPE)  
		        {  
		             /* EPIPE means underrun */  
		             fprintf(stderr, "underrun occurred\n");  
		             //�����d���Q��?�u�C�g??�y?�D  
		             snd_pcm_prepare(playback_handle);  
		                  //exit (1);
		        }   
		        
		        else if (err < 0)   
		        {  
		             fprintf(stderr,  
		                      "error from writei: %s\n",  
		                      snd_strerror(err));  
		        }    
			}
		fseek( wave_info.fp, wave_info.play_start_pos, SEEK_SET );
		printf("playback_handle:%d\n",  playback_handle);
		}
		printf("faefr: over2\n");
	}
	else if (pid > 0)
	{
		
		int cmd = 0;
		int resume = 0;
		while(1)
		{
			sleep(1);
			printf("Please input cmd:\n");
			printf("1: Stop\n");
			printf("2: Start\n");
			
			scanf("%d", &cmd);
			
			switch(cmd){
				case 1:
				//snd_pcm_pause(playback_handle, 1);  //this device is NG
				//snd_pcm_close(playback_handle);		//OK
				snd_pcm_drop(playback_handle);		//OK
				snd_pcm_close(playback_handle);
				playback_handle = NULL;
				//snd_pcm_drain(playback_handle);
				break;
				case 2:
				//snd_pcm_start(playback_handle);
				//resume = snd_pcm_resume(playback_handle);
				//printf("resume:%d playback_handle:%d\n", resume, playback_handle);
				printf("playback_handle: %d\n", playback_handle);
				if (playback_handle == NULL)
				{
					//snd_pcm_open (&playback_handle, argv[1], SND_PCM_STREAM_PLAYBACK, 0);
					resume = 1;
				}
				
				if (resume == -EAGAIN)
				{
					printf("EAGAIN\n");
				}
				else if(resume == -ENOSYS)
				{
					printf("ENOSYS\n");
				}
				break;
				case 3:
				//snd_pcm_start(playback_handle);
				//resume = snd_pcm_resume(playback_handle);
				//printf("resume:%d playback_handle:%d\n", resume, playback_handle);
				printf("playback_handle: %d\n", playback_handle);
				if (playback_handle == NULL)
				{
					//snd_pcm_open (&playback_handle, argv[1], SND_PCM_STREAM_PLAYBACK, 0);
					resume = 1;
				}
				else
				{
					goto loop;
				}
				if (resume == -EAGAIN)
				{
					printf("EAGAIN\n");
				}
				else if(resume == -ENOSYS)
				{
					printf("ENOSYS\n");
				}
				break;
				default:
				break;
			}
			if (resume == 1)
			{
				break;
			}
		}
	}
}while(1);
//	}

	fclose(wave_info.fp);
	snd_pcm_close (playback_handle);
	exit (0);
}
