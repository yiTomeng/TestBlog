#include <fcntl.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/soundcard.h>


/* WAVE ファイルの情報を格納する構造体 */
typedef struct {
	FILE* fp;        /* ファイル構造体 */
	short is_pcm;    /* PCM フォーマットの場合は 1、それ以外は 0 */
	short channel;   /* モノラルの場合は 1、ステレオの場合は 2 */
	int   rate;      /* サンプリング周波数 */
	short bits;      /* 量子化ビット数 */
	long  offset;    /* ファイル先頭から PCM データまでのオフセット */
	int   len;       /* PCM データ部の長さ */
}
WAVE;


/* 関数プロトタイプ */
static int  wave_read_file( char* fname, WAVE* wave );
static int  wave_setup_dsp( int* fd, WAVE* wave );
static void wave_progress( int* processed, int current, WAVE* wave );


#define BUFSIZE 64000

int 
main( int argc, char** argv )
{
	WAVE wave;
	char buf[ BUFSIZE ];
	int  dsp;
	int  len;
	int  processed = 0;

	if ( argc != 2 ) {
		fprintf( stderr, "usage : wave filename\n" );
		return 1;
	}

	/* WAVE ファイルヘッダ読み込み */
	if ( wave_read_file( argv[1], &wave ) != 0 ) {
		fprintf( stderr, "Failed to read specified WAVE file"
			 ": %s\n", argv[1] );
		return 1;
	}

	printf( "\n" );
	printf( "WAVE file format :\n" );
	printf( " rate    = %d\n",  wave.rate );
	printf( " channel = %d\n",  wave.channel );
	printf( " bits    = %d\n",  wave.bits );
	printf( " offsets = %ld\n", wave.offset );
	printf( " length  = %d\n",  wave.len );
	printf( "\n" );

	/* /dev/dsp の設定 */
	if ( wave_setup_dsp( &dsp, &wave ) != 0 ) {
		fprintf( stderr, "Setup /dev/dsp failed.\n" );
		fclose( wave.fp );
		return 1;
	}

	printf( "Now playing specified wave file %s ...\n", argv[1] );
	fflush( stdout );

	fseek( wave.fp, wave.offset, SEEK_SET );
	wave_progress( &processed, 0, &wave );

	while ( 1 ) {
		len = fread( buf, 1, BUFSIZE, wave.fp );

		if ( len < BUFSIZE ) {
			if ( feof( wave.fp ) ) {
				if ( write( dsp, buf, len ) == -1 ) {
					perror( "write()" );
				}
				else {
					wave_progress( &processed, len, &wave );
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

		wave_progress( &processed, len, &wave );
	}

	fclose( wave.fp );
	close( dsp );

	printf( "\ndone.\n" );
	return 0;
}


/*
 * WAVE ファイル情報を読み込む
 */
static int 
wave_read_file( char* fname, WAVE* wave )
{
	char   buf[32];
	int    len;

	if ( ( wave->fp = fopen( fname, "r" ) ) == NULL ) {
		fprintf( stderr, "Failed to open %s\n", fname );
		return -1;
	}

	/* 
	 * 先頭 4 バイトが "RIFF" であることを確認
	 * 更に 4 バイトスキップしておく
	 */
	fread( buf, 8, 1, wave->fp );

	//fread( buf, 10, 1, wave->fp );

	if ( strncmp( buf, "RIFF", 4 ) != 0 ) {
	//if ( strncmp( &buf[2], "RIFF", 4 ) != 0 ) {
	
		fprintf( stderr, "Specified file is not RIFF file.\n" );
		fclose( wave->fp );
		return -1;
	}


	/* 次の 4 バイトが "WAVE" であることを確認 */
	fread( buf, 4, 1, wave->fp );

	if ( strncmp( buf, "WAVE", 4 ) != 0 ) {
		fprintf( stderr, "Specified file is not WAVE file.\n" );
		fclose( wave->fp );
		return -1;
	}

	/* fmt チャンクを探す */
	while ( 1 ) {
		fread( buf, 8, 1, wave->fp );
		len = *( int* )( &buf[4] );

		if ( strncmp( buf, "fmt ", 4 ) != 0 ) {
			if ( fseek( wave->fp, len, SEEK_CUR ) == -1 ) {
				fprintf( stderr, "Failed to find fmt chunk.\n" );
				fclose( wave->fp );
				return -1;
			}
		}
		else {
			break;
		}
	}


	/* WAVE フォーマットを読み込む */
	fread( buf, len, 1, wave->fp );

	wave->is_pcm  = *( ( short* )( &buf[0]  ) );
	wave->channel = *( ( short* )( &buf[2]  ) );
	wave->rate    = *( ( int*   )( &buf[4]  ) );
	wave->bits    = *( ( short* )( &buf[14] ) );

	if ( wave->is_pcm != 1 ) {
		wave->is_pcm = 0;
	}

	/* data チャンクを探す */
	while ( 1 ) {
		fread( buf, 8, 1, wave->fp );
		len = *( int* )( &buf[4] );

		if ( strncmp( buf, "data", 4 ) != 0 ) {
			if ( fseek( wave->fp, len, SEEK_CUR ) == -1 ) {
				fprintf( stderr, "Failed to find data chunk.\n" );
				fclose( wave->fp );
				return -1;
			}
		}
		else {
			break;
		}
	}

	wave->len = len;

	if ( ( wave->offset = ftell( wave->fp ) ) == -1 ) {
		fprintf( stderr, "Failed to find offset of PCM data.\n" );
		fclose( wave->fp );
		return -1;
	}

	return 0;
}


/*
 * /dev/dsp を設定する
 */
static int 
wave_setup_dsp( int* dsp, WAVE* wave )
{
	int format;
	int rate;
	int channel;

	/*
	 * PCM フォーマットであるかどうか確認する
	 */
	if ( ! wave->is_pcm ) {
		fprintf( stderr, "Specified file's sound data is not PCM format.\n" );
		fclose( wave->fp );
		return -1;
	}

	/*
	 * 8 bit の場合は AFMT_U8、16 bit の場合は AFMT_S16_LE をフォーマット
	 * として選択し、それ以外の数の場合はエラーとする。
	 */
	if ( wave->bits == 8 ) {
		format = AFMT_U8;
	}
	else if ( wave->bits == 16 ) {
		format = AFMT_S16_LE;
	}
	else {
		fprintf( stderr, "Specified file's sound data is %d bits,"
			 "not 8 nor 16 bits.\n", wave->bits );
		fclose( wave->fp );
		return -1;
	}

	rate    = ( int )wave->rate;
	channel = ( int )wave->channel;


	if ( ( *dsp = open( "/dev/dsp", O_WRONLY ) ) == -1 ) {
		perror( "open()" );
		return -1;
	}

	if ( ioctl( *dsp, SNDCTL_DSP_SETFMT, &format ) == -1 ) {
		perror( "ioctl( SOUND_PCM_SETFMT )" );
		close( *dsp );
		return -1;
	}

	if ( ioctl( *dsp, SOUND_PCM_WRITE_RATE, &rate ) == -1 ) {
		perror( "ioctl( SOUND_PCM_WRITE_RATE )" );
		close( *dsp );
		return -1;
	}

	if ( ioctl( *dsp, SOUND_PCM_WRITE_CHANNELS, &channel ) == -1 ) {
		perror( "ioctl( SOUND_PCM_WRITE_CHANNELS )" );
		close( *dsp );
		return -1;
	}

	return 0;
}


/*
 * 進捗率の表示
 */
static void 
wave_progress( int* processed, int current, WAVE* wave )
{
	int progress;

	*processed += current;
	progress = (int)( ( ( double )*processed / ( double )wave->len ) * 100 );
	printf( "\r%3d%% played.", progress );
	fflush( stdout );
}


/* End of wave.c */