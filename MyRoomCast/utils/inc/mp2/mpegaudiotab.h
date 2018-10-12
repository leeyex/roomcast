/*
 * mpeg audio layer 2 tables. Most of them come from the mpeg audio
 * specification.
 * 
 * Copyright (c) 2000, 2001 Fabrice Bellard.
 *
 * The licence of this code is contained in file LICENCE found in the
 * same archive 
 */

/**
 * @file mpegaudiotab.h
 * mpeg audio layer 2 tables. 
 * Most of them come from the mpeg audio specification.
 */
 
#ifndef	__MPEGAUDIO_TAB_H__
#define	__MPEGAUDIO_TAB_H__

#include "bitstream.h"
#include "mpegaudio.h"

#define SQRT2 1.41421356237309514547
#define FIX(a)   ((int)((a) * (1 << FRAC_BITS)))
#define FRAC_BITS 15

extern const int costab32[30] ;
extern const int bitinv32[32] ;
extern int16_t filter_bank[512];
extern int scale_factor_table[64];
extern float scale_factor_inv_table[64];
extern int8_t scale_factor_shift[64];
extern unsigned short scale_factor_mult[64];
extern unsigned char scale_diff_table[128];
extern unsigned short total_quant_bits[17];
extern unsigned short quant_snr[17];
extern const float fixed_smr[SBLIMIT];
extern const unsigned char nb_scale_factors[4];
extern const unsigned char alloc_table_0[];
extern const unsigned char alloc_table_1[];
extern const unsigned char alloc_table_2[];
extern const unsigned char alloc_table_3[];
extern const unsigned char alloc_table_4[];

#endif


