/**
 * @file mpegaudio.h
 * mpeg audio declarations for both encoder and decoder.
 */

#ifndef __MPEGAUDIO_H__
#define __MPEGAUDIO_H__


#include <stdio.h>
#include <sys/types.h>
#include <math.h>
#ifndef M_PI
#define M_PI    3.14159265358979323846
#endif
#include <stddef.h>
#ifndef offsetof
# define offsetof(T,F) ((unsigned int)((char *)&((T *)0)->F))
#endif
#ifndef EMULATE_INTTYPES
#   include <inttypes.h>
#else
    typedef signed char  int8_t;
    typedef signed short int16_t;
    typedef signed int   int32_t;
    typedef unsigned char  uint8_t;
    typedef unsigned short uint16_t;
    typedef unsigned int   uint32_t;

#   ifdef CONFIG_WIN32
        typedef signed __int64   int64_t;
        typedef unsigned __int64 uint64_t;
#   else /* other OS */
        typedef signed int64_t int64_t   int64_t;
        typedef unsigned int64_t int64_t uint64_t;
#   endif /* other OS */
#endif /* HAVE_INTTYPES_H */
#ifndef restrict
#    define restrict
#endif

#ifndef always_inline
#if defined(__GNUC__) && (__GNUC__ > 3 || __GNUC__ == 3 && __GNUC_MINOR__ > 0)
#    define always_inline __attribute__((always_inline)) inline
#else
#    define always_inline inline
#endif
#endif

#ifndef attribute_used
#if defined(__GNUC__) && (__GNUC__ > 3 || __GNUC__ == 3 && __GNUC_MINOR__ > 0)
#    define attribute_used __attribute__((used))
#else
#    define attribute_used
#endif
#endif

#ifndef attribute_unused
#if defined(__GNUC__) && (__GNUC__ > 3 || __GNUC__ == 3 && __GNUC_MINOR__ > 0)
#    define attribute_unused __attribute__((unused))
#else
#    define attribute_unused
#endif
#endif

/* max frame size, in samples */
#define MPA_FRAME_SIZE 1152 

/* max compressed frame size */
#define MPA_MAX_CODED_FRAME_SIZE 1792

#define MPA_MAX_CHANNELS 2

#define SBLIMIT 32 /* number of subbands */

#define MPA_STEREO  0
#define MPA_JSTEREO 1
#define MPA_DUAL    2
#define MPA_MONO    3

/* header + layer + bitrate + freq + lsf/mpeg25 */
#define SAME_HEADER_MASK \
   (0xffe00000 | (3 << 17) | (0xf << 12) | (3 << 10) | (3 << 19))

int l2_select_table(int bitrate, int nb_channels, int freq, int lsf);
/*int mpa_decode_header(AVCodecContext *avctx, uint32_t head);*/


#include "mpegaudioenc.h"
#include "mpegaudiodec.h"

extern const uint16_t mpa_bitrate_tab[2][3][15];
extern const uint16_t mpa_freq_tab[3];
extern const unsigned char *alloc_tables[5];
extern const double enwindow[512];
extern const int sblimit_table[5];
extern const int quant_steps[17];
extern const int quant_bits[17];
extern const int32_t mpa_enwindow[257];

// 编码解码接口函数
int MPA_encode_init(int X_SAMPLE_RATE, int X_BIT_RATE, int X_CHANNLES,MpegAudioContext* context);
int MPA_encode_frame( unsigned char *frame, int buf_size, void *data,MpegAudioContext* context);
int MPA_encode_close();

//int MPA_decode_init(MPADecodeContext *s);
//int MPA_decode_frame(void *data, int *data_size,  uint8_t * buf, int buf_size,MPADecodeContext *s);
//int MPA_decode_close();

short	PCM_mix16(short x1, short x2);	// 用于2路16Bit混音

// PCM数组输入，MP2缓冲区输出的混音算法
int	PCM_mix16(char *mp2_buf, int *mp2_len, char *pcm_buf, short *in[], int loads, int packet_len);

#endif

