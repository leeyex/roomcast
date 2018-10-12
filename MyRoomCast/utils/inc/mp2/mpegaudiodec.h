

#ifndef __MPEG_AUDIO_DEC_H__
#define __MPEG_AUDIO_DEC_H__

#include "bitstream.h"
#include "mpegaudio.h"

#define HEADER_SIZE 4
#define BACKSTEP_SIZE 512

typedef int16_t MPA_INT;
typedef int16_t OUT_INT;

typedef struct MPADecodeContext {
    uint8_t inbuf1[2][MPA_MAX_CODED_FRAME_SIZE + BACKSTEP_SIZE];        /* input buffer */
    int inbuf_index;
    uint8_t *inbuf_ptr, *inbuf;
    int frame_size;
    int free_format_frame_size; /* frame size in case of free format
                                   (zero if currently unknown) */
    /* next header (used in free format parsing) */
    uint32_t free_format_next_header;
    int error_protection;
    int layer;
    int sample_rate;
    int sample_rate_index; /* between 0 and 8 */
    int bit_rate;
    int old_frame_size;
    GetBitContext gb;
    int nb_channels;
    int mode;
    int mode_ext;
    int lsf;
    MPA_INT synth_buf[MPA_MAX_CHANNELS][512 * 2] __attribute__((aligned(16)));
    int synth_buf_offset[MPA_MAX_CHANNELS];
    int32_t sb_samples[MPA_MAX_CHANNELS][36][SBLIMIT] __attribute__((aligned(16)));
    int32_t mdct_buf[MPA_MAX_CHANNELS][SBLIMIT * 18]; /* previous samples, for layer 3 MDCT */

    unsigned int dither_state;
} MPADecodeContext;

#endif
