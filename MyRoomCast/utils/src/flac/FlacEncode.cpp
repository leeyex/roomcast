#include "FlacEncode.h"
#include "common.h"
#include <iostream>
using namespace std;

static unsigned char *write_buf = NULL;
static unsigned *write_len = NULL;
static size_t encodedSamples_ = 0;

static char flac_header[4096];
static int head_len = 0;

static FLAC__StreamEncoderWriteStatus my_write(const FLAC__StreamEncoder *encoder, const FLAC__byte buffer[], size_t bytes, unsigned samples, unsigned current_frame, void *client_data)
{
//    cout << "my write bytes="<<bytes<<" samples="<<samples<<" current_frame="<<current_frame<<endl;
//	printf("my write bytes=%d, samples=%d, current_frame=%d\n", bytes, samples, current_frame);
//	write_to_file("/tmp/flac.flac", (unsigned char *)buffer, bytes);
	if ((current_frame == 0) && (bytes > 0) && (samples == 0))
	{
		memcpy(flac_header + head_len, buffer, bytes);
		head_len += bytes;
	}
	else
	{
//		cout << "*write_len=" << *write_len << endl;
		if(write_buf)
			memcpy(write_buf, buffer, bytes);
		*write_len = bytes;
//		cout << "*write_len=" << *write_len << endl;
		
		encodedSamples_ += samples;
	}	
	return FLAC__STREAM_ENCODER_WRITE_STATUS_OK;
}

static FLAC__StreamEncoderSeekStatus my_seek(const FLAC__StreamEncoder *encoder, FLAC__uint64 absolute_byte_offset, void *client_data)
{
	printf("my seek\n");
	return FLAC__STREAM_ENCODER_SEEK_STATUS_OK;
}

static FLAC__StreamEncoderTellStatus my_tell(const FLAC__StreamEncoder *encoder, FLAC__uint64 *absolute_byte_offset, void *client_data)
{
//	printf("my tell\n");
	return FLAC__STREAM_ENCODER_TELL_STATUS_OK;
}

static void meta_data(const FLAC__StreamEncoder *encoder, const FLAC__StreamMetadata *metadata, void *client_data)
{
	printf("my metadata\n");
	//cout <<"call meta_data";
}


FlacEncode::FlacEncode():encoder(NULL),pcmBufferSize_(0),pcmBuffer_(NULL)
{
	
}

FlacEncode::~FlacEncode()
{
	if (encoder) {
		FLAC__stream_encoder_finish(encoder);
		FLAC__metadata_object_delete(metadata[0]);
		FLAC__metadata_object_delete(metadata[1]);
		FLAC__stream_encoder_delete(encoder);
	}
	if(pcmBuffer_)
		free(pcmBuffer_);
}


int FlacEncode::encoderInit(unsigned sample_rate_, unsigned bps_, unsigned channels_)
{
	FLAC__bool ok = true;
	FLAC__StreamEncoderInitStatus init_status;    
	FLAC__StreamMetadata_VorbisComment_Entry entry;
	channels = channels_;
	bps = bps_;
	sample_rate = sample_rate_;
	sample_size = bps_ / 8;
	frame_size = bps_ * channels / 8;
	if ((encoder = FLAC__stream_encoder_new()) == NULL) {
		cout<<"ERROR: allocating encoder\n";
		return -1;
	}

//	ok &= FLAC__stream_encoder_set_verify(encoder, true);
	// compression levels (0-8):
	// https://xiph.org/flac/api/group__flac__stream__encoder.html#gae49cf32f5256cb47eecd33779493ac85
	// latency:
	// 0-2: 1152 frames, ~26.1224ms
	// 3-8: 4096 frames, ~92.8798ms
	ok &= FLAC__stream_encoder_set_compression_level(encoder, 2);
	ok &= FLAC__stream_encoder_set_channels(encoder, channels);
	ok &= FLAC__stream_encoder_set_bits_per_sample(encoder, bps);
	ok &= FLAC__stream_encoder_set_sample_rate(encoder, sample_rate);
//	ok &= FLAC__stream_encoder_set_total_samples_estimate(encoder, total_samples);
	if (!ok){
		cout << "ERROR: init encoder\n";
		return -1;
	}
	if (
		(metadata[0] = FLAC__metadata_object_new(FLAC__METADATA_TYPE_VORBIS_COMMENT)) == NULL ||
		(metadata[1] = FLAC__metadata_object_new(FLAC__METADATA_TYPE_PADDING)) == NULL ||
		/* there are many tag (vorbiscomment) functions but these are convenient for this particular use: */
		!FLAC__metadata_object_vorbiscomment_entry_from_name_value_pair(&entry, "ARTIST", "Some Artist") ||
		!FLAC__metadata_object_vorbiscomment_append_comment(metadata[0], entry, /*copy=*/false) || /* copy=false: let metadata object take control of entry's allocated string */
		!FLAC__metadata_object_vorbiscomment_entry_from_name_value_pair(&entry, "YEAR", "1984") ||
		!FLAC__metadata_object_vorbiscomment_append_comment(metadata[0], entry, /*copy=*/false)
		) {
			cout<<"ERROR: out of memory or tag error\n";
			return -1;
	}

	metadata[1]->length = 1234; /* set the padding length */
	ok = FLAC__stream_encoder_set_metadata(encoder, metadata, 2);
	if (ok) {
		init_status = FLAC__stream_encoder_init_stream(encoder, my_write, my_seek, my_tell, meta_data, /*client_data=*/NULL);
		if (init_status != FLAC__STREAM_ENCODER_INIT_STATUS_OK) {
			cout << "ERROR: initializing encoder:" << FLAC__StreamEncoderInitStatusString[init_status] << endl;
			return -1;
		}
	}
	return 0;
}

void FlacEncode::Encode(unsigned char *pcm_buf, unsigned pcm_len, unsigned char *flac_buf, unsigned *flac_len)
{
	static int tag = 0;
	int frames = pcm_len / frame_size;
	int samples = pcm_len / sample_size;
	write_buf = flac_buf;
	write_len = flac_len;
	*write_len = 0;
	if (pcmBufferSize_ < samples)
	{
		pcmBufferSize_ = samples;
		pcmBuffer_ = (FLAC__int32*)realloc(pcmBuffer_, pcmBufferSize_ * sizeof(FLAC__int32));
	}

	if (sample_size == 1)
	{
		FLAC__int8* buffer = (FLAC__int8*)pcm_buf;
		for (int i = 0; i < samples; i++)
			pcmBuffer_[i] = (FLAC__int32)(buffer[i]);
	}
	else if (sample_size == 2)
	{
		FLAC__int16* buffer = (FLAC__int16*)pcm_buf;
		for (int i = 0; i < samples; i++)
			pcmBuffer_[i] = (FLAC__int32)(buffer[i]); 
/*		for (int i = 0; i < samples; i++)
			pcmBuffer_[i] = (FLAC__int32)(((FLAC__int16)(FLAC__int8)pcm_buf[2 * i + 1] << 8) | (FLAC__int16)pcm_buf[2 * i]);
		printf("sample_size=%d\n", sample_size);  */
	}
	else if (sample_size == 4)
	{
		FLAC__int32* buffer = (FLAC__int32*)pcm_buf;
		for (int i = 0; i < samples; i++)
			pcmBuffer_[i] = (FLAC__int32)(buffer[i]);
	}
//	cout << "start process pcm\n";
	if (head_len) {
		memcpy(write_buf, flac_header, head_len);
		*write_len = head_len;
		head_len = 0;
	}

	FLAC__stream_encoder_process_interleaved(encoder, pcmBuffer_, frames);
	

//	cout << "after process pcm\n";
	if (encodedSamples_ > 0)
	{
		encodedSamples_ = 0;
	//	*duaration = encodedSamples_ / ((double)sample_rate / 1000.);
	} 
}

