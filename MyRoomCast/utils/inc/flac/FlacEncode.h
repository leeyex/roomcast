#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compat.h"
#include "metadata.h"
#include "stream_encoder.h"

class FlacEncode
{
public:
	FlacEncode();
	~FlacEncode();
	int encoderInit(unsigned sample_rate_, unsigned bps_, unsigned channels_);
	void Encode(unsigned char *pcm_buf, unsigned pcm_len, unsigned char *flac_buf, unsigned *flac_len);
private:
	FLAC__StreamEncoder *encoder;
	FLAC__StreamMetadata *metadata[2];
	FLAC__int32 *pcmBuffer_;
	int pcmBufferSize_;
	int sample_size;
	int frame_size;

	unsigned sample_rate;
	unsigned channels;
	unsigned bps;
};