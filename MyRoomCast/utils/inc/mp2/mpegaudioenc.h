
#ifndef __MPEG_AUDIO_ENC_H__
#define __MPEG_AUDIO_ENC_H__

#include "bitstream.h"
#include "mpegaudio.h"

#define SAMPLES_BUF_SIZE 4096


typedef struct AFrame {
     char* data;
     int frame_size;
 } AFrame;
 

typedef struct MpegAudioContext {
	PutBitContext pb;
	int nb_channels;
	int freq, bit_rate;
	int lsf;           /* 1 if mpeg2 low bitrate selected */
	int bitrate_index; /* bit rate */
	int freq_index;
	int frame_size; /* frame size, in bits, without padding */
	int64_t nb_samples; /* total number of samples encoded */
	/* padding computation */
	int frame_frac, frame_frac_incr, do_padding;
	short samples_buf[MPA_MAX_CHANNELS][SAMPLES_BUF_SIZE]; /* buffer for filter */
	int samples_offset[MPA_MAX_CHANNELS];       /* offset in samples_buf */
	int sb_samples[MPA_MAX_CHANNELS][3][12][SBLIMIT];
	unsigned char scale_factors[MPA_MAX_CHANNELS][SBLIMIT][3]; /* scale factors */
	/* code to group 3 scale factors */
	unsigned char scale_code[MPA_MAX_CHANNELS][SBLIMIT];
	int sblimit; /* number of used subbands */
	const unsigned char *alloc_table;
 	//AFrame encodedframe;
} MpegAudioContext;


#endif
