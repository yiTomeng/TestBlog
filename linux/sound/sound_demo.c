#include <fcntl.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include<sys/types.h>
#include <linux/soundcard.h>
//#include <pthread.h>

#define MAX_PROC 10
#define STATUS_PLAY_ONE_ONLY 1
#define STATUS_PLAY_ONE_MORE 2
#define STATUS_PLAY_MASK    0x3
#define STATUS_PLAY_SUSPEND  4
#define STATUS_PLAY_REBROADCAST 8

//#define STATUS_PLAY_SUSPEND_MASK 8
#define LOOP_CAST			-1
#define HEADER_BUFSIZE		36
#define BUFSIZE				64000
#define FILENAME_SIZE		32

typedef struct 
{
	pid_t pid;
	int status;
	int repeat_cnt;
	char filename[FILENAME_SIZE];
}play_pid_info;

typedef struct
{
	char chunk_id[4];
	int chunk_size;
	char format[4];
	char sub_chunk_id[4];
	int sub_chunk_size;
	short audio_format;
	short num_channels;
	int sample_rate;
	int byte_rate;
	short block_align;
	short bits_per_sample;
	//char data_id[4];
	int data_size;	 	
}wave_file_header_info;

typedef struct
{
	unsigned int play_start_pos;
	unsigned int play_pos;
	FILE *fp;
	int stop;
	int refresh;
	wave_file_header_info wave_header;
}play_wave_info;

static int wave_read_file_header(const char* filename, play_wave_info *wave_info);
static int set_dsp(int *dsp, play_wave_info *wave_info);
static int dsp_play(play_wave_info *wave_info);

static play_pid_info child_proc[MAX_PROC];
static int count = 0;

int main(int argc, char *argv[])
{
	
	memset(child_proc, 0x00, sizeof(play_pid_info) * MAX_PROC);
	int cmd = 0;
	pid_t tmp_pid;
	
	play_wave_info wave_info;
	
	int status = 0;
	int cnt = 0;
	while(1)
	{
		scanf("%d", &cmd);
		switch (cmd)
		{
			case 1:
				if (count < MAX_PROC)
				{
					child_proc[count].pid = getpid();
					child_proc[count].status = STATUS_PLAY_ONE_MORE;
					child_proc[count].repeat_cnt = LOOP_CAST;
					strncpy(child_proc[count++].filename, argv[1], strlen(argv[1]));
					printf("wavファイルを再生\n");
					
					tmp_pid = fork();
					if (tmp_pid < 0)
					{
						printf("子プロセス作成失敗しました。\n");
						break;
					}
					else if (0 == tmp_pid)
					{
					
						
						printf("filename:%s\n", child_proc[0].filename);
						status = STATUS_PLAY_ONE_MORE;
						cnt = LOOP_CAST;
						printf("pid:%d\n", child_proc[0].pid);
						if (wave_read_file_header(child_proc[0].filename, &wave_info) == -1)
						{
							printf("WAVファイル読み込むには失敗しました。\n", getpid());
						}
						printf("count: %d\n", count);
						dsp_play(&wave_info);
					}
					else
					{
						break;
					}
				}
				else
				{
					printf("プロセス数は最大制限になりました。\n");
				}
				break;
			case 2:
				printf("pid:%d\n", child_proc[0].pid);
				
				if (0 == count)
				{
					printf("再生したプロセスありません。\n");
					break;
				}
				if ( (child_proc[0].status & STATUS_PLAY_SUSPEND) == 1 )
				{
					printf("再生したプロセスは中止されました。\n");
					break;
				}
				
				if ( (child_proc[0].status & STATUS_PLAY_MASK) == 1 )
				{
					//child_proc[0].status = 0;
					child_proc[0].status |= STATUS_PLAY_SUSPEND;
				}
				break;
			case 3:
				if (0 == count)
				{
					printf("再生したプロセスありません。\n");
					break;
				}
				if ( (child_proc[0].status & STATUS_PLAY_MASK) == 1 )
				{
					printf("プロセスは今再生中です。\n");
					break;
				}
				
				if ( (child_proc[0].status & STATUS_PLAY_SUSPEND) == 1 )
				{
					//child_proc[0].status = 0;
					child_proc[0].status &= ~STATUS_PLAY_SUSPEND;
					child_proc[0].status |= STATUS_PLAY_REBROADCAST;
				}
			default:
				printf("default.\n");
				break;
		}
				
	}
}



static int wave_read_file_header(const char* filename, play_wave_info *wave_info)
{
	char buf[HEADER_BUFSIZE];
	int len;
	
	if((wave_info->fp = fopen(filename, "r")) == NULL )
	{
		fprintf( stderr, "Failed to open %s\n", filename );
		return -1;
	}
	
	fread( buf, 1, HEADER_BUFSIZE, wave_info->fp );
	
	memcpy(&wave_info->wave_header, buf, HEADER_BUFSIZE);
	
	if ( strncmp( wave_info->wave_header.chunk_id, "RIFF", 4 ) != 0 ) {
	//if ( strncmp( &buf[2], "RIFF", 4 ) != 0 ) {
	
		fprintf( stderr, "Specified file is not RIFF file.\n" );
		fclose( wave_info->fp );
		return -1;
	}


	

	if ( strncmp( wave_info->wave_header.format, "WAVE", 4 ) != 0 ) {
		fprintf( stderr, "Specified file is not WAVE file.\n" );
		fclose( wave_info->fp );
		return -1;
	}

	if ( strncmp(wave_info->wave_header.sub_chunk_id, "fmt ", 4) != 0)
	{
		fprintf( stderr, "Failed to find fmt chunk.\n" );
		fclose( wave_info->fp );
		return -1;
	}
		
	while ( 1 ) {
		fread( buf, 8, 1, wave_info->fp );
		len = *( int* )( &buf[4] );

		if ( strncmp( buf, "data", 4 ) != 0 ) {
			if ( fseek( wave_info->fp, len, SEEK_CUR ) == -1 ) {
				fprintf( stderr, "Failed to find data chunk.\n" );
				fclose( wave_info->fp );
				return -1;
			}
		}
		else {
			break;
		}
	}

	wave_info->wave_header.data_size = len;

	if ( ( wave_info->play_start_pos = ftell( wave_info->fp ) ) == -1 ) {
		fprintf( stderr, "Failed to find offset of PCM data.\n" );
		fclose( wave_info->fp );
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


	if ( ( *dsp = open( "/dev/dsp", O_WRONLY ) ) == -1 ) {
		perror( "open()" );
		return -1;
	}

	if ( ioctl( *dsp, SNDCTL_DSP_SETFMT, &format ) == -1 ) {
		perror( "ioctl( SOUND_PCM_SETFMT )" );
		close( *dsp );
		return -1;
	}

	if ( ioctl( *dsp, SOUND_PCM_WRITE_RATE, &wave_info->wave_header.sample_rate ) == -1 ) {
		perror( "ioctl( SOUND_PCM_WRITE_RATE )" );
		close( *dsp );
		return -1;
	}

	if ( ioctl( *dsp, SOUND_PCM_WRITE_CHANNELS, &channel ) == -1 ) {
		printf("channels:%d\n", wave_info->wave_header.num_channels);
		perror( "ioctl( SOUND_PCM_WRITE_CHANNELS )" );
		close( *dsp );
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


static int dsp_play(play_wave_info *wave_info)
{
	int dsp;
	int len;
	char buf[BUFSIZE];
	
	set_dsp(&dsp, wave_info);
	
	printf("dsp : %d\n", dsp);
	printf( "Now playing specified wave file %s ...\n", child_proc[0].filename );
	fflush( stdout );

	fseek( wave_info->fp, wave_info->play_start_pos, SEEK_SET );
	printf("start_pos:%d\n", wave_info->play_start_pos);
	//wave_progress( &processed, 0, &wave );

	while ( 1 ) {
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
		len = fread( buf, 1, BUFSIZE, wave_info->fp );

		if ( len < BUFSIZE ) {
			if ( feof( wave_info->fp ) ) {
				if ( write( dsp, buf, len ) == -1 ) {
					perror( "write1()" );
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
			perror( "write2()" );
			break;
		}

		//wave_progress( &processed, len, &wave );
	}

	fclose( wave_info->fp );
}