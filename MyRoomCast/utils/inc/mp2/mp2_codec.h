#ifndef MP2_CODEC_H
#define MP2_CODEC_H

//#include <sys/types.h>
#include <pthread.h>
#include "mpegaudiodec.h"
#include "mpegaudioenc.h"

/**
@return 0成功
*/
extern int MPA_encode_init(int X_SAMPLE_RATE, int X_BIT_RATE, int X_CHANNLES, MpegAudioContext *s);
/**
@return 编码的长度
*/
extern int MPA_encode_frame( unsigned char *frame, int buf_size, void *data , MpegAudioContext *s);


extern int MPA_decode_init(MPADecodeContext *s);

/**
@return 编码消耗的长度
@data   解码输出的空间
@data_size 解码输出的长度
*/
extern int MPA_decode_frame( void *data, int *data_size,  u_int8_t * buf, int buf_size, MPADecodeContext *s);




class Mp2_Codec
{
public:
    Mp2_Codec();
    ~Mp2_Codec();
    //22050, 32000, 1
    int encode_init(int X_SAMPLE_RATE = 22050,
                    int X_BIT_RATE = 32000,
                    int X_CHANNLES = 1);
    int encode_close();

    int decode_init();
    int decode_close();

    //return 消耗掉的pcm_len
    int encode(unsigned char *pcm, int pcm_len, unsigned char *mp2, int *mp2_len);
    //return 消耗掉的mp2_len
    int decode(unsigned char *mp2, int mp2_len, unsigned char *pcm, int *pcm_len);

private:
    MPADecodeContext *decode_context;
    MpegAudioContext *encode_context;

    pthread_mutex_t     decode_mutex;
    pthread_mutex_t     encode_mutex;
};

#endif
