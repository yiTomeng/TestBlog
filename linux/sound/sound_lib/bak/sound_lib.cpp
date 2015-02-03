#include <fcntl.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/soundcard.h>


/*
#define MAX_PROC 10
#define STATUS_PLAY_ONE_ONLY 1
#define STATUS_PLAY_ONE_MORE 2
#define STATUS_PLAY_MASK    0x3
#define STATUS_PLAY_SUSPEND  4
#define STATUS_PLAY_REBROADCAST 8


#define LOOP_CAST			-1
*/

#define HEADER_BUFSIZE		36
#define BUFSIZE				64000
#define FILENAME_SIZE		32
#define DSP_DEVICE			"/dev/dsp"
/*
typedef struct 
{
	pid_t pid;
	int status;
	int repeat_cnt;
	char filename[FILENAME_SIZE];
}play_pid_info;
*/

typedef struct
{
	char	chunk_id[4];
	int		chunk_size;
	char 	format[4];
	char 	sub_chunk_id[4];
	int  	sub_chunk_size;
	short 	audio_format;
	short 	num_channels;
	int 	sample_rate;
	int 	byte_rate;
	short 	block_align;
	short 	bits_per_sample;
	int 	data_size;	 	
}wave_file_header_info;

typedef struct
{
	unsigned int play_start_pos;
	unsigned int play_pos;
	FILE 	*fp;
	int 	stop;
	int 	refresh_times;
	wave_file_header_info wave_header;
}play_wave_info;

//int play_wave(const char *filename);
static void wave_init(play_wave_info *wave_info, int refresh_count);
static int wave_read_file_header(play_wave_info *wave_info);
static int set_dsp(int *dsp, play_wave_info *wave_info);
static int dsp_play(play_wave_info *wave_info);

//static play_pid_info child_proc[MAX_PROC];
//static int count = 0;



int play_wave(const char *filename, int refresh_times)
{
	int dsp = 0;
	
	play_wave_info wave_info;
	
	wave_init(&wave_info, refresh_times);
	
	if((wave_info->fp = fopen(filename, "r")) == NULL )
	{
		perror("Failed to open %s\n", filename);
		return -1;
	}
	
	if (-1 == wave_read_file_header(&wave_info))
	{
		fclose(wave_info->fp);
		return -1;
	}
	
	if ( -1 == ( dsp = open( DSP_DEVICE, O_WRONLY ) ) ) 
	{
		perror( "open()%s失敗しました。",  DSP_DEVICE);
		fclose(wave_info->fp);
		return -1;
	}
	
	if ( -1 == set_dsp(&dsp, wave_info))
	{
		perror("DSPデバイス設定失敗しました。");
		close(dsp);
		fclose(wave_info->fp);
		return -1;
	}
	
	if (dsp_play(&dsp, wave_info) < 0)
	{
		perror("放送失敗しました。");
		close(dsp);
		fclose(wave_info->fp);
		return -1;
	}
	
	return 0;
}

static void wave_init(play_wave_info *wave_info, int refresh_count)
{
	wave_info->play_start_pos = 0;
	wave_info->play_pos = 0;
	wave_info->stop = 0;
	wave_info->refresh_times = refresh_count;
	memset(wave_info->wave_header, 0x00, sizeof(wave_file_header_info));
}

static int wave_read_file_header(play_wave_info *wave_info)
{
	int len = 0;
	char buf[HEADER_BUFSIZE + 1];
	
	memset(buf, 0x00, HEADER_BUFSIZE + 1);
	
	
	fread( buf, 1, HEADER_BUFSIZE, wave_info->fp );
	
	memcpy(&wave_info->wave_header, buf, HEADER_BUFSIZE);
	
	if ( strncmp( wave_info->wave_header.chunk_id, "RIFF", 4 ) != 0 ) 
	{
		fprintf( stderr, "Specified file is not RIFF file.\n" );
		//fclose( wave_info->fp );
		return -1;
	}
	
	if ( strncmp( wave_info->wave_header.format, "WAVE", 4 ) != 0 ) {
		fprintf( stderr, "Specified file is not WAVE file.\n" );
		//fclose( wave_info->fp );
		return -1;
	}

	if ( strncmp(wave_info->wave_header.sub_chunk_id, "fmt ", 4) != 0)
	{
		fprintf( stderr, "Failed to find fmt chunk.\n" );
		//fclose( wave_info->fp );
		return -1;
	}
		
	while ( 1 ) {
		fread( buf, 8, 1, wave_info->fp );
		len = *( int* )( &buf[4] );

		if ( strncmp( buf, "data", 4 ) != 0 ) 
		{
			if ( fseek( wave_info->fp, len, SEEK_CUR ) == -1 ) 
			{
				fprintf( stderr, "Failed to find data chunk.\n" );
				//fclose( wave_info->fp );
				return -1;
			}
		}
		else 
		{
			break;
		}
	}

	wave_info->wave_header.data_size = len;

	if ( ( wave_info->play_start_pos = ftell( wave_info->fp ) ) == -1 ) 
	{
		fprintf( stderr, "Failed to find offset of PCM data.\n" );
		//fclose( wave_info->fp );
		return -1;
	}
	
	return 0;
}

static int set_dsp(int *dsp, play_wave_info *wave_info)
{
	int format = 0;
	//int dsp ;
	
	//int len;
	//char buf[BUFSIZE];
	if ( 1 != wave_info->wave_header.audio_format)
	{
		fprintf( stderr, "Specified file's sound data is not PCM format.\n" );
		fclose( wave_info->fp );
		return -1;
	}
	
	
	if ( wave_info->wave_header.bits_per_sample == 8 ) {
		format = AFMT_U8;
	}
	else if ( wave_info->wave_header.bits_per_sample == 16 ) {
		format = AFMT_S16_LE;
	}
	else {
		fprintf( stderr, "Specified file's sound data is %d bits,"
			 "not 8 nor 16 bits.\n", wave_info->wave_header.bits_per_sample );
		fclose( wave_info->fp );
		return -1;
	}

	//rate    = ( int )wave_info->wave_header.sample_rate;
	int channel = ( int )wave_info->wave_header.num_channels;

	if ( ioctl( *dsp, SNDCTL_DSP_SETFMT, &format ) == -1 ) {
		perror( "ioctl( SOUND_PCM_SETFMT )" );
		//close( *dsp );
		return -1;
	}

	if ( ioctl( *dsp, SOUND_PCM_WRITE_RATE, &wave_info->wave_header.sample_rate ) == -1 ) {
		perror( "ioctl( SOUND_PCM_WRITE_RATE )" );
		//close( *dsp );
		return -1;
	}

	if ( ioctl( *dsp, SOUND_PCM_WRITE_CHANNELS, &channel ) == -1 ) {
		printf("channels:%d\n", wave_info->wave_header.num_channels);
		perror( "ioctl( SOUND_PCM_WRITE_CHANNELS )" );
		//close( *dsp );
		return -1;
	}

#if 0
	//if ()
	printf( "Now playing specified wave file %s ...\n", child_proc[0].filename );
	fflush( stdout );

	fseek( wave_info->fp, wave_info->play_start_pos, SEEK_SET );
	printf("start_pos:%d\n", wave_info->play_start_pos);
	//wave_progress( &processed, 0, &wave );

	while ( 1 ) {
		if (STATUS_PLAY_SUSPEND == child_proc[0].status)
		{
			close();
			sleep(1);
			continue;
		}
		if (STATUS_PLAY_REBROADCAST & child_proc[0].status)
		len = fread( buf, 1, BUFSIZE, wave_info->fp );

		if ( len < BUFSIZE ) {
			if ( feof( wave_info->fp ) ) {
				if ( write( dsp, buf, len ) == -1 ) {
					perror( "write()" );
				}
				else {
					//wave_progress( &processed, len, &wave );
					fseek( wave_info->fp, wave_info->play_start_pos, SEEK_SET );
					continue;
				}
			}
			else {
				perror( "fread()" );
			}

			break;
		}

		if ( write( dsp, buf, len ) == -1 ) {
			perror( "write()" );
			break;
		}

		//wave_progress( &processed, len, &wave );
	}

	fclose( wave_info->fp );

	close( dsp );
#endif
	return 0;
}


static int dsp_play(int *dsp, play_wave_info *wave_info)
{
//	int dsp;
	int len;
	char buf[BUFSIZE];
	int rebroadcast_cnt = 0;
	int ret = 0;
//	set_dsp(&dsp, wave_info);
	
//	printf("dsp : %d\n", dsp);
	printf( "Now playing specified wave file %s ...\n", child_proc[0].filename );
	fflush( stdout );

	fseek( wave_info->fp, wave_info->play_start_pos, SEEK_SET );
//	printf("start_pos:%d\n", wave_info->play_start_pos);
//	wave_progress( &processed, 0, &wave );

	while ( 1 ) {
/*
		if ((STATUS_PLAY_SUSPEND & child_proc[0].status) == 1)
		{
			if ( ( wave_info->play_pos = ftell( wave_info->fp ) ) == -1 )
			 {
				fprintf( stderr, "Failed to find offset of current data.\n" );
				wave_info->play_pos = wave_info->play_start_pos;
			}
			fclose(wave_info->fp);
			close(dsp);
			
			sleep(1);
			continue;
		}
		if ( (STATUS_PLAY_REBROADCAST & child_proc[0].status) == 1)
		{
			if ( (wave_info->fp = fopen(child_proc[0].filename, "r")) == NULL )
			{
				fprintf ( stderr, "Failed to open %s\n", child_proc[0].filename );
				break;
			}
			if ( ( dsp = open( "/dev/dsp", O_WRONLY ) ) == -1 ) 
			{
				perror( "open(dsp) error" );
				break;
			}
			fseek( wave_info->fp, wave_info->play_pos, SEEK_SET );
			child_proc[0].status &= STATUS_PLAY_MASK;
		}
*/
		len = fread( buf, 1, BUFSIZE, wave_info->fp );

		if ( len < BUFSIZE ) 
		{
			if ( feof( wave_info->fp ) ) 
			{
				if ( -1 == write( dsp, buf, len ) ) 
				{
					perror( "[EOF]DSPデバイスに書き込むことが失敗しました。" );
					ret = -1;
					//break;
				}
				else 
				{
					//wave_progress( &processed, len, &wave );
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
				//break;
			}

			break;
		}

		if ( -1 == write( dsp, buf, len ) ) 
		{
			perror( "DSPデバイスに書き込むことが失敗しました。" );
			ret = -1;
			break;
		}

		//wave_progress( &processed, len, &wave );
	}

	return ret;
	//fclose( wave_info->fp );
}