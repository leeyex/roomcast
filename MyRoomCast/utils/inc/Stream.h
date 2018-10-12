#pragma  once
#include "DataList.h"
#include "DoubleBuffer.h"

class Stream
{
public:
	Stream();
	void init();
	void setRealSampleRate(double sampleRate);
	bool getPlayChunk(void *output_buf,  int64_t ouput_dac_time,  int64_t frames_per_buf, int *data_len, int *play_thd_running_ptr);
	void startPlay();
	int64_t  getSilentPlayerChunk(void *output_buf,  int64_t frames_per_buf);
	int64_t  getNextPlayerChunk (void* outputBuffer, int64_t  timeout, int64_t framesPerBuffer);
	int64_t  getNextPlayerChunk(void* outputBuffer, int64_t  timeout, int64_t framesPerBuffer, int64_t framesCorrection);
	
	
private:
	void updateBuffers(int64_t age);
	void resetBuffers();


	int64_t sleep_;
	int64_t playedFrames_;
	PcmChunk pcm_chunk;
	PcmChunk* pcm_chunk_ptr;
	int64_t correct_after_xframes_;

	int median_;
	int short_median_;
	time_t last_update_;
	DoubleBuffer<double> buffer_;
	DoubleBuffer<double> short_buffer_;
	DoubleBuffer<double> mini_buffer_;

};



