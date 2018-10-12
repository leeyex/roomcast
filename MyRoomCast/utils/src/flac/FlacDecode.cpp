#include "FlacDecode.h"
#include <iostream>

using namespace std;

static FLAC__StreamDecoderWriteStatus write_callback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *client_data);
static void metadata_callback(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data);
static void error_callback(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data);

static FLAC__uint64 total_samples = 0;


static unsigned char *read_buf = NULL;
static unsigned int read_len = 0;
static unsigned char *write_buf = NULL;
static unsigned int *write_len = 0;

FLAC__StreamDecoderReadStatus myRead(const FLAC__StreamDecoder *decoder, FLAC__byte buffer[], size_t *bytes, void *client_data)
{
/*	static FILE *file = NULL;
	if (file == NULL)
		file = fopen("/tmp/flac.flac", "rb"); 
	cout << "1111111111111111\n";
	if (*bytes > 0) {
		*bytes = fread(buffer, sizeof(FLAC__byte), *bytes, file);
		if (ferror(file))
			return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
		else if (*bytes == 0)
			return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
		else
			return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
	}
	else
		return FLAC__STREAM_DECODER_READ_STATUS_ABORT;	 */



	if (*bytes > 0) {
		*bytes = read_len;
		//		*bytes = (*bytes < read_len) ? *bytes : read_len;
		memcpy(buffer, read_buf, *bytes);
//		cout << "*bytes=" << *bytes << endl;
	}
	else {
			return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
	}
	
	return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
	
	
}

static FLAC__bool write_little_endian_uint16(unsigned char* buf, FLAC__uint16 x)
{
	*buf = (char)x;
	*(buf + 1) = (char)(x >> 8);
	*write_len += 2;
	return	true;
}

static FLAC__bool write_little_endian_int16(unsigned char* buf, FLAC__int16 x)
{
	return write_little_endian_uint16(buf, (FLAC__uint16)x);
}

static FLAC__bool write_little_endian_uint32(unsigned char* buf, FLAC__uint32 x)
{
	*buf = (char)x;
	*(buf+1) = (char)(x >> 8);
	*(buf+2)= (char)(x >> 16);
	*(buf+3)= (char)(x >> 24);
	*write_len += 4;
	return	true;
}



FlacDecode::FlacDecode():decoder(NULL)
{


}
 
FlacDecode::~FlacDecode()
{
	if (decoder)
		FLAC__stream_decoder_delete(decoder);
}


int FlacDecode::decodeReInit(unsigned sample_, unsigned  bps_, unsigned channels_)
{
	if (decoder) {
		FLAC__stream_decoder_delete(decoder);
	}
	decodeInit(sample_rate, bps, channels);

}


int FlacDecode::decodeInit(unsigned sample_, unsigned  bps_, unsigned channels_)
{
	sample_rate = sample_;
//	channels = channels_;
	channels = 2;
	bps = bps_;
	FLAC__StreamDecoderInitStatus init_status;

	if ((decoder = FLAC__stream_decoder_new()) == NULL) {
		cout << "ERROR: allocating decoder\n";
		return -1;
	}

	//	(void)FLAC__stream_decoder_set_md5_checking(decoder, true);
	init_status = FLAC__stream_decoder_init_stream(decoder, myRead, NULL, NULL, NULL, NULL, write_callback, metadata_callback, error_callback, this);
	if (init_status != FLAC__STREAM_DECODER_INIT_STATUS_OK)
		cout << "ERROR: initializing decoder: " << string(FLAC__StreamDecoderInitStatusString[init_status]) << endl;

//	FLAC__stream_decoder_process_until_end_of_metadata(decoder);

	return 0;

}

void FlacDecode::Decode(unsigned char *flac_buf, unsigned int flac_len, unsigned char *pcm_buf, unsigned int *pcm_len)
{
	read_buf = flac_buf;
	read_len = flac_len;
	write_buf = pcm_buf;
	write_len = pcm_len;
	*write_len = 0;
//	cout << "start process single read_len = "<<read_len<<endl;
	FLAC__stream_decoder_process_single(decoder);
}

FLAC__StreamDecoderWriteStatus write_callback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *client_data)
{
//	FILE *f = (FILE*)client_data;
//	const FLAC__uint32 total_size = (FLAC__uint32)(total_samples * channels * (bps / 8));
	unsigned int bps = 16;
	unsigned int channels = 2;
	size_t i;

//	cout << "call write \n";
	/*      if(total_samples == 0) {
	fprintf(stderr, "ERROR: this example only works for FLAC files that have a total_samples count in STREAMINFO\n");
	return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
	}*/
	if (channels != 2 || bps != 16) {
		fprintf(stderr, "ERROR: this example only supports 16bit stereo streams\n");
		return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
	}
	if (frame->header.channels != 2) {
		fprintf(stderr, "ERROR: This frame contains %d channels (should be 2)\n", frame->header.channels);
		return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
	}
	if (buffer[0] == NULL) {
		fprintf(stderr, "ERROR: buffer [0] is NULL\n");
		return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
	}
	if (buffer[1] == NULL) {
		fprintf(stderr, "ERROR: buffer [1] is NULL\n");
		return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
	}

	/* write WAVE header before we write the first frame */
/*	if (frame->header.number.sample_number == 0) {
		if (
			fwrite("RIFF", 1, 4, f) < 4 ||
			!write_little_endian_uint32(f, total_size + 36) ||
			fwrite("WAVEfmt ", 1, 8, f) < 8 ||
			!write_little_endian_uint32(f, 16) ||
			!write_little_endian_uint16(f, 1) ||
			!write_little_endian_uint16(f, (FLAC__uint16)channels) ||
			!write_little_endian_uint32(f, sample_rate) ||
			!write_little_endian_uint32(f, sample_rate * channels * (bps / 8)) ||
			!write_little_endian_uint16(f, (FLAC__uint16)(channels * (bps / 8))) || // block align 
			!write_little_endian_uint16(f, (FLAC__uint16)bps) ||
			fwrite("data", 1, 4, f) < 4 ||
			!write_little_endian_uint32(f, total_size)
			) {
			fprintf(stderr, "ERROR: write error\n");
			return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
		}
	}	*/

	/* write decoded PCM samples */
	for (i = 0; i < frame->header.blocksize; i++) {
		if (
			!write_little_endian_int16(write_buf+*write_len, (FLAC__int16)buffer[0][i]) ||  /* left channel */
			!write_little_endian_int16(write_buf+*write_len, (FLAC__int16)buffer[1][i])     /* right channel */
			) {
			fprintf(stderr, "ERROR: write error\n");
			return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
		}
	}
	*write_len = frame->header.blocksize * 4;
/*	static FILE *fp = NULL;
	if (fp == NULL) {
		fp = fopen("/tmp/flac.pcm", "wb");
	}
	fwrite(write_buf, 1, *write_len, fp);
	cout << "call write  write_len="<<*write_len<<endl;	   */
	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void metadata_callback(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data)
{
	(void)decoder, (void)client_data;
	
	/* print some stats */
	if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
		/* save for later */
		total_samples = metadata->data.stream_info.total_samples;
		unsigned sample_rate = metadata->data.stream_info.sample_rate;
		unsigned channels = metadata->data.stream_info.channels;
		unsigned bps = metadata->data.stream_info.bits_per_sample;

		fprintf(stderr, "sample rate    : %u Hz\n", sample_rate);
		fprintf(stderr, "channels       : %u\n", channels);
		fprintf(stderr, "bits per sample: %u\n", bps);
		//              fprintf(stderr, "total samples  : %" PRIu64 "\n", total_samples);
	}
}

void error_callback(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data)
{
	(void)decoder, (void)client_data;
	FlacDecode *t = (FlacDecode *)client_data;
	fprintf(stderr, "Got error callback: %s\n", FLAC__StreamDecoderErrorStatusString[status]);
	t->decodeReInit(t->sample_rate, t->bps, t->channels);
//	exit(-1);
}
