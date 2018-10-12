#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compat.h"
#include "metadata.h"
#include "stream_decoder.h"


class FlacDecode 
{
public:
	FlacDecode();
	~FlacDecode();
	int decodeReInit(unsigned sample_, unsigned  bps_, unsigned channels_);
	int decodeInit(unsigned sample_, unsigned  bps_, unsigned channels_ = 2);		//only support two channel
	void Decode(unsigned char *flac_buf, unsigned int flac_len, unsigned char *pcm_buf, unsigned int *pcm_len);

	unsigned sample_rate;
	unsigned channels ;
	unsigned bps ;
private:
	
	FLAC__StreamDecoder * decoder;
};
