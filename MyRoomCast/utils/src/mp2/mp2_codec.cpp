#include "mp2_codec.h"

Mp2_Codec::Mp2_Codec() :
		decode_context ( NULL ),
		encode_context ( NULL )
{
	decode_context = new MPADecodeContext;
	encode_context = new MpegAudioContext;

	memset ( decode_context, 0x0, sizeof ( MPADecodeContext ) );
	memset ( encode_context, 0x0, sizeof ( MpegAudioContext ) );

	pthread_mutex_init(&encode_mutex, NULL);
	pthread_mutex_init(&decode_mutex, NULL);

}

Mp2_Codec::~Mp2_Codec()
{
	if ( decode_context ) delete decode_context;
	if ( encode_context ) delete encode_context;
}

int Mp2_Codec::encode_init ( int X_SAMPLE_RATE, int X_BIT_RATE, int X_CHANNLES )
{
    int rt;
    pthread_mutex_lock(&encode_mutex);
	rt = MPA_encode_init ( X_SAMPLE_RATE, X_BIT_RATE, X_CHANNLES, encode_context );
	pthread_mutex_unlock(&encode_mutex);
	return rt;
}

int Mp2_Codec::encode_close()
{
	return 0;
}

int Mp2_Codec::decode_init()
{
    int rt;
    pthread_mutex_lock(&decode_mutex);
    rt = MPA_decode_init ( decode_context );
    pthread_mutex_unlock(&decode_mutex);
    return rt;
}

int Mp2_Codec::decode_close()
{
	return 0;
}

int Mp2_Codec::encode ( unsigned char *pcm, int pcm_len,
                        unsigned char *mp2, int *mp2_len )
{
    pthread_mutex_lock(&encode_mutex);
	*mp2_len = MPA_encode_frame ( mp2, 0, pcm, encode_context );
	pthread_mutex_unlock(&encode_mutex);
	return pcm_len;
}

int Mp2_Codec::decode ( unsigned char *mp2, int mp2_len,
                        unsigned char *pcm, int *pcm_len )
{
    pthread_mutex_lock(&decode_mutex);
	int spend_mp2_len = MPA_decode_frame ( pcm, pcm_len, mp2, mp2_len, decode_context );
	pthread_mutex_unlock(&decode_mutex);
	return spend_mp2_len;
}
