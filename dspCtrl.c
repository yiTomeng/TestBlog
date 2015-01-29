#include <fcntl.h>
#include <linux/soundcard.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>


static int dsp_parameters( int fd );
static int dsp_supported_format( int fd );
static int dsp_capabilities( int fd );


int 
main( void )
{
	int fd;

	if ( ( fd = open( "/dev/dsp", O_RDONLY ) ) == -1 ) {
		perror( "open()" );
		return 1;
	}

	/* /dev/dsp のデフォルトパラメータを取得 */
	if ( dsp_parameters( fd ) == -1 ) {
		perror( "dsp_parameters() failed\n" );
		close( fd );
		return 1;
	}

	/* /dev/dsp でサポートされているフォーマット情報を取得 */
	if ( dsp_supported_format( fd ) == -1 ) {
		perror( "supported_format() failed\n" );
		close( fd );
		return 1;
	}

	/* /dev/dsp でサポートされている DSP 機能情報を取得 */
	if ( dsp_capabilities( fd ) == -1 ) {
		perror( "dsp_capabilities() failed\n" );
		close( fd );
		return 1;
	}

	close( fd );
	return 0;
}


/*
 * /dev/dsp のデフォルトパラメータを取得
 */
static int 
dsp_parameters( int fd )
{
	int rate;
	int channels;
	int bits;
	int blksize;


	/* サンプリングレート値を取得 */
	if ( ioctl( fd, SOUND_PCM_READ_RATE, &rate ) == -1 ) {
		perror( "ioctl( SOUND_PCM_READ_RATE )" );
		return -1;
	}

	/* チャンネル数を取得 */
	if ( ioctl( fd, SOUND_PCM_READ_CHANNELS, &channels ) == -1 ) {
		perror( "ioctl( SOUND_PCM_READ_CHANNELS )" );
		return -1;
	}

	/* 量子化ビット数を取得 */
	if ( ioctl( fd, SOUND_PCM_READ_BITS, &bits ) == -1 ) {
		perror( "ioctl( SOUND_PCM_READ_BITS )" );
		return -1;
	}

	/* ドライバが内部に持っているバッファのブロックサイズを取得 */
	if ( ioctl( fd, SNDCTL_DSP_GETBLKSIZE, &blksize ) == -1 ) {
		perror( "ioctl( SNDCTL_DSP_GETBLKSIZE )" );
		return -1;
	}

	printf( "\n" );
	printf( "Sampling rate : %d Hz\n",    rate );
	printf( "Channels      : %d\n",       channels );
	printf( "Sample size   : %d bits\n",  bits );
	printf( "Block size    : %d bytes\n", blksize );
	printf( "\n" );

	return 0;
}


/*
 * /dev/dsp のデフォルトのサウンドフォーマット、および
 * サポートされているサウンドフォーマットを調べる。
 */
static int 
dsp_supported_format( int fd )
{
/* format_table に設定されているサウンドフォーマットの数 */
#define FORMAT_NUM 8

	int format_table[ FORMAT_NUM ] = {
		AFMT_MU_LAW,   /* mu-law */
		AFMT_A_LAW,    /* A-law  */
		AFMT_U8,       /* unsigned 8 bit */
		AFMT_S8,       /* signed 8 bit */
		AFMT_S16_LE,   /* signed 16 bit, little-endian */
		AFMT_S16_BE,   /* signed 16 bit, big-endian */
		AFMT_U16_LE,   /* unsigned 16 bit, little-endian */
		AFMT_U16_BE    /* unsigned 16 bit, big-endian */
	};

	char* format_msgtbl[ FORMAT_NUM ] = {
		"  mu-law",
		"  A-law",
		"  unsigned 8 bit",
		"  signed 8 bit",
		"  signed 16 bit, little-endian",
		"  signed 16 bit, big-endian",
		"  unsigned 16 bit, little-endian",
		"  unsigned 16 bit, big-endian"
	};

	int formats, default_format;
	int i;

	/* デフォルトフォーマット問い合わせ用の値を設定 */
	default_format = AFMT_QUERY;

	/* デフォルトのサウンドフォーマットを取得 */
	if ( ioctl( fd, SOUND_PCM_SETFMT, &default_format ) == -1 ) {
		perror( "ioctl( SOUND_PCM_SETFMT )" );
		return -1;
	}

	/* サポートされているフォーマットを取得 */
	if ( ioctl( fd, SOUND_PCM_GETFMTS, &formats ) == -1 ) {
		perror( "ioctl( SOUND_PCM_GETFMTS )" );
		return -1;
	}

	printf( "Supported formats\n" );

	/* 取得したフォーマット情報を解析して表示する */
	for ( i = 0; i < FORMAT_NUM; i ++ ) {
		if ( formats & format_table[i] ) {
			printf( "%s", format_msgtbl[i] );

			( default_format == format_table[i] ) ?
				printf( " ( default )\n" ) : printf( "\n" );
		}
	}

	return 0;
}


/*
 * /dev/dsp でサポートされている DSP 機能について調べる
 */
static int 
dsp_capabilities( int fd )
{
/* caps_table に設定されている機能の数 */
#define CAPS_NUM 7

	int caps_table[ CAPS_NUM ] = {
		DSP_CAP_REVISION,  /* バージョン */
		DSP_CAP_DUPLEX,    /* 録音再生を同時に行える */
		DSP_CAP_REALTIME,  /* SNDCTL_DSP_GET[IO]PTR を使って、再生中の
				      バッファ位置を正確に知ることが出来る */
		DSP_CAP_BATCH,     /* 録音再生にローカルストーレッジを使う */
		DSP_CAP_COPROC,    /* codec ではなく DSP チップが存在している */
		DSP_CAP_TRIGGER,   /* 録音再生の triggering が行える */
		DSP_CAP_MMAP       /* 録音再生にハードウェアのバッファを使える */
	};

	char* caps_msgtbl[ CAPS_NUM ] = {
		"  revision    : ",
		"  duplex      : ",
		"  realtime    : ",
		"  batch       : ",
		"  coprocessor : ",
		"  trigger     : ",
		"  mmap        : "
	};

	char* msgtbl[2] = {	"○", "×" };
	int   i, caps;

	/* DSP 機能情報を取得 */
	if ( ioctl( fd, SNDCTL_DSP_GETCAPS, &caps ) == -1 ) {
		perror( "ioctl( SNDCTL_DSP_GETCAPS )" );
		return -1;
	}

	printf( "\nDSP capabilities\n" );

	/* 取得した機能情報を解析して表示する */
	for ( i = 0; i < CAPS_NUM; i ++ ) {
		printf( "%s", caps_msgtbl[i] );

		/* リビジョンだけ他のと取り出し方が異なる */
		if ( i == 0 ) {
			printf( "%d\n", ( caps & DSP_CAP_REVISION ) );
		}
		else {
			( ( caps & caps_table[i] ) != 0 ) ?
				printf( "%s\n", msgtbl[0] ):
				printf( "%s\n", msgtbl[1] );
		}
	}

	printf( "\n" );
	return 0;
}


/* End of dsptest.c */