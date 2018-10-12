#include <stdlib.h>
#include <stdio.h>
#include <cmath>
#include <time.h>
#include <unistd.h>

#include "Stream.h"
#include "DiffHandler.h"

using namespace std;

#define  MS 1000

#define FORMART_SIZE 4	

Stream::Stream() :playedFrames_(0), sleep_(0), median_(0), short_median_(0), last_update_(0), pcm_chunk_ptr(NULL)
{
	buffer_.setSize(500);
	short_buffer_.setSize(100);
	mini_buffer_.setSize(20);

}




void Stream::init()
{

}

void Stream::setRealSampleRate(double sampleRate)
{
	double rate = 44100;
	if (sampleRate == rate)
		correct_after_xframes_ = 0;
	else
		correct_after_xframes_ = round((rate / sampleRate) / (rate / sampleRate - 1.));
	cout << "correct_afer_xframes_ = " << correct_after_xframes_ << endl;
}

bool Stream::getPlayChunk(void *output_buf, int64_t output_dac_time, int64_t frames_per_buf, int *data_len, int *play_thd_running_ptr)
{
	static int tag = 0;
	if (tag == 0 && DiffHandler::instance()->serverNow() == 0)
		return false;
	tag = 1;
	int i_ = 0;
	while (DataList<PcmChunk>::instance()->read(&pcm_chunk) == -1)
	{
		if (i_ < output_dac_time / 1000) {
			usleep(1000);
			i_++;
			continue;
		}
		//LOG(INFO) << "no chunks available\n";
		cout << "no chunks available\n";
		return false;
	}

	//	unsigned int time_to_endec = 6400;				//us
	unsigned int time_to_endec = 0;				//us		 
	unsigned int my_buffer_ms = CACHE_MS;
	double my_sample = 44100;
	//	cout << "server time = " << DiffHandler::instance()->serverNow() << " current pcm_chunk = " << pcm_chunk.time_stamp.getUs() << "\n";
	int64_t my_age = 0;
	while (*play_thd_running_ptr) {
		my_age = DiffHandler::instance()->serverNow() * 1000 - pcm_chunk.time_stamp.getUs() - my_buffer_ms * 1000 - time_to_endec;
		//S	cout << "my age == " << my_age << " output_dac_time == " << output_dac_time << "\n";

		if (abs(my_age) <= output_dac_time) {							// in time , play it
			memcpy(output_buf, pcm_chunk.data_buf, pcm_chunk.data_size);
			//			cout << "play now -----------------\n";
			*data_len = pcm_chunk.data_size;
			return true;
		}
		else if (my_age > output_dac_time) {						// too old, discard

		//	memset(output_buf, 0, PCM_SIZE);
			cout << "discard ------------------\n";
			usleep(100);
			return false;
			//		memcpy(output_buf, pcm_chunk.data_buf, PCM_SIZE);
			//		return true;
		}
		else if (my_age < -output_dac_time) {						//is early, put it back to head

		//	memset(output_buf, 0, PCM_SIZE);
		//	PcmList::instance()->writeToHead(&pcm_chunk);
		//	cout << "put it to head --------------\n";
			usleep(100);
		}
	}
	return false;


	if (DiffHandler::instance()->serverNow() == 0)
		return false;

	unsigned int buffer_ms = CACHE_MS;
	double sample = 44100;
	/*		if (output_dac_time > buffer_ms*1000)
	{
	std::cout << "outputBufferDacTime > bufferMs: " << output_dac_time << " > " << CACHE_MS * 1000 << "\n";
	sleep_ = 0;
	return false;
	}	*/
	int i = 0;

	if (!pcm_chunk_ptr && DataList<PcmChunk>::instance()->read(&pcm_chunk) == -1)
	{
		//LOG(INFO) << "no chunks available\n";
		cout << "no chunks available\n";
		sleep_ = 0;
		return false;
	}
	if (!pcm_chunk_ptr)
		pcm_chunk_ptr = &pcm_chunk;

	playedFrames_ += frames_per_buf;

	/// we have a chunk
	/// age = chunk age (server now - rec time: some positive value) - buffer (e.g. 1000ms) + time to DAC
	/// age = 0 => play now
	/// age < 0 => play in -age
	/// age > 0 => too old
	cout << "server time = " << DiffHandler::instance()->serverNow() << " current pcm_chunk = " << pcm_chunk.time_stamp.getUs() << "\n";
	//		int64_t age = DiffHandler::instance()->serverNow()*1000 - pcm_chunk.time_stamp.getUs() - buffer_ms*1000 + output_dac_time;
	int64_t age = DiffHandler::instance()->serverNow() * 1000 - pcm_chunk.time_stamp.getUs() - buffer_ms * 1000;

	//	LOG(INFO) << "age: " << age.count() / 1000 << "\n";
	cout << "age: " << age / 1000 << "\n";
	if ((sleep_ == 0) && (abs(age) > 200 * MS))
	{
		std::cout << "age > 200: " << (age / 1000) << "\n";
		sleep_ = age;
	}

	try
	{

		//LOG(DEBUG) << "framesPerBuffer: " << framesPerBuffer << "\tms: " << framesPerBuffer*2 / PLAYER_CHUNK_MS_SIZE << "\t" << PLAYER_CHUNK_SIZE << "\n";
		int64_t buffer_Duration = frames_per_buf / (sample / 1000000.);
		//		LOG(DEBUG) << "buffer duration: " << buffer_Duration.count() << "\n";
		//			cout << "333333333333333333333333\n";
		int64_t correction = 0;
		if (sleep_ != 0)
		{
			resetBuffers();
			if (sleep_ < -buffer_Duration / 2)
			{
				std::cout << "sleep < -buffer_Duration/2: " << sleep_ << " < " << -(buffer_Duration) / 2 << "\n";
				// We're early: not enough chunks_. play silence. Reference chunk_ is the oldest (front) one
				cout << "server time = " << DiffHandler::instance()->serverNow() << "\n";
				//	sleep_ = DiffHandler::instance()->serverNow() * 1000 - getSilentPlayerChunk(output_buf, frames_per_buf) - buffer_ms * 1000 + output_dac_time;
				sleep_ = DiffHandler::instance()->serverNow() * 1000 - getSilentPlayerChunk(output_buf, frames_per_buf) - buffer_ms * 1000;

				//	sleep_ = 0;
				std::cout << "sleep: " << sleep_ << "\n";
				if (sleep_ < -buffer_Duration / 2)
					return true;
			}
			else if (sleep_ > buffer_Duration / 2)
			{
				std::cout << "sleep > buffer_Duration/2: " << sleep_ << " > " << (buffer_Duration) / 2 << "\n";
				// We're late: discard oldest chunks
				while (sleep_ > buffer_Duration * 1000)
				{
					//	LOG(INFO) << "sleep > chunkDuration: " << cs::duration<cs::msec>(sleep_) << " > " << chunk_->duration<cs::msec>().count() << ", chunks: " << chunks_.size() << ", out: " << cs::duration<cs::msec>(outputBufferDacTime) << ", needed: " << cs::duration<cs::msec>(buffer_Duration) << "\n";
					sleep_ = DiffHandler::instance()->serverNow() * 1000 - pcm_chunk.time_stamp.getUs() - buffer_ms * 1000;

					cout << " while sleep_ " << sleep_ << "\n";
					i = 0;
					if (DataList<PcmChunk>::instance()->read(&pcm_chunk) == -1)
					{
						//		LOG(INFO) << "no chunks available\n";
						pcm_chunk_ptr = NULL;
						sleep_ = 0;
						return false;
					}
					pcm_chunk_ptr = &pcm_chunk;
				}
			}

			cout << "sleep_ = " << sleep_ << "\n";
			// out of sync, can be corrected by playing faster/slower
			if (sleep_ < -100)
			{
				sleep_ += 100;
				correction = -100;
			}
			else if (sleep_ > 100)
			{
				sleep_ -= 100;
				correction = 100;
			}
			else
			{
				//					LOG(INFO) << "Sleep " << cs::duration<cs::msec>(sleep_) << "\n";
				correction = sleep_;
				sleep_ = 0;
			}
		}

		//			cout << "444444444444444444444444\n";
		// framesCorrection = number of frames to be read more or less to get in-sync
		int64_t framesCorrection = correction * sample / 1000000.;

		//			cout << "111111111111111111111111\n";
		// sample rate correction
		if ((correct_after_xframes_ != 0) && (playedFrames_ >= (int64_t)abs(correct_after_xframes_)))
		{
			framesCorrection += (correct_after_xframes_ > 0) ? 1 : -1;
			playedFrames_ -= abs(correct_after_xframes_);
			//				cout << "222222222222222222222\n";
		}
		//			cout << "3333333333333333333\n";
		//			age = DiffHandler::instance()->serverNow() * 1000 - getNextPlayerChunk(output_buf, output_dac_time, frames_per_buf, framesCorrection) - buffer_ms * 1000 + output_dac_time;
		age = DiffHandler::instance()->serverNow() * 1000 - getNextPlayerChunk(output_buf, output_dac_time, frames_per_buf, framesCorrection) - buffer_ms * 1000;
		//			cout << "5555555555555555555555555555555\n";
		setRealSampleRate(sample);
		if (sleep_ == 0)
		{
			if (buffer_.full())
			{
				if (abs(median_) > 1 * MS)
				{
					//					LOG(INFO) << "pBuffer->full() && (abs(median_) > 1): " << median_ << "\n";
					sleep_ = median_;
				}
				/*				else if (cs::usec(median_) > cs::usec(300))
				{
				setRealSampleRate(format_.rate - format_.rate / 1000);
				}
				else if (cs::usec(median_) < -cs::usec(300))
				{
				setRealSampleRate(format_.rate + format_.rate / 1000);
				}
				*/
			}
			else if (short_buffer_.full())
			{
				if ((abs(short_median_)) > 5 * MS)
				{
					//					LOG(INFO) << "pShortBuffer->full() && (abs(short_median_) > 5): " << short_median_ << "\n";
					sleep_ = short_median_;
				}
				/*				else
				{
				setRealSampleRate(format_.rate + -short_median_ / 100);
				}
				*/
			}
			else if (mini_buffer_.full() && (abs(mini_buffer_.median()) > 50 * MS))
			{
				//				LOG(INFO) << "pMiniBuffer->full() && (abs(pMiniBuffer->mean()) > 50): " << mini_buffer_.median() << "\n";
				sleep_ = mini_buffer_.mean();
			}
		}

		if (sleep_ != 0)
		{
			static int64_t lastAge = 0;
			int64_t msAge = age * MS;
			if (lastAge != msAge)
			{
				lastAge = msAge;
				//					LOG(INFO) << "Sleep " << cs::duration<cs::msec>(sleep_) << ", age: " << msAge << ", buffer_Duration: " << cs::duration<cs::msec>(buffer_Duration) << "\n";
			}
		}
		else if (short_buffer_.full())
		{
			if (short_median_ > 100)
				setRealSampleRate(sample * 0.9999);
			else if (short_median_ < -100)
				setRealSampleRate(sample * 1.0001);
		}

		updateBuffers(age);

		// print sync stats
		time_t now = time(NULL);
		if (now != last_update_)
		{
			last_update_ = now;
			median_ = buffer_.median();
			short_median_ = short_buffer_.median();
			//				LOG(INFO) << "Chunk: " << age.count() / 100 << "\t" << mini_buffer_.median() / 100 << "\t" << short_median_ / 100 << "\t" << median_ / 100 << "\t" << buffer_.size() << "\t" << cs::duration<cs::msec>(outputBufferDacTime) << "\n";
			//			LOG(INFO) << "Chunk: " << age.count()/1000 << "\t" << mini_buffer_.median()/1000 << "\t" << short_median_/1000 << "\t" << median_/1000 << "\t" << buffer_.size() << "\t" << cs::duration<cs::msec>(outputBufferDacTime) << "\n";
		}
		//			cout << "6666666666666666666\n";
		return ((abs(age) / 1000) < 500);
	}
	catch (int e)
	{
		cout << "catch excepetion =" << e << endl;
		sleep_ = 0;
		return false;
	}

}

void Stream::startPlay()
{

}

int64_t  Stream::getSilentPlayerChunk(void *output_buf, int64_t frames_per_buf)
{
	if (!pcm_chunk_ptr) {
		if (DataList<PcmChunk>::instance()->read(&pcm_chunk) != -1) {
			pcm_chunk_ptr = &pcm_chunk;
		}
	}
	memset(output_buf, 0, frames_per_buf * FORMART_SIZE);
	cout << " current pcm_chunk = " << pcm_chunk.time_stamp.getUs() << "\n";
	return pcm_chunk.time_stamp.getUs();
	//	return 0;
}


int64_t Stream::getNextPlayerChunk(void* outputBuffer, int64_t timeout, int64_t framesPerBuffer)
{
	int i = 0;
	if (!pcm_chunk_ptr &&  DataList<PcmChunk>::instance()->read(&pcm_chunk) == -1)
	{
		//LOG(INFO) << "no chunks available\n";
		throw 0;
	}

	int64_t tp = pcm_chunk.time_stamp.getUs();
	char* buffer = (char*)outputBuffer;
	int64_t read = 0;
	int num;
	while (read < framesPerBuffer)
	{
		//		cout << "i am here2222222222222222222\n";
		if (framesPerBuffer > PCM_SIZE)
		{
			memcpy(buffer + read * 4, pcm_chunk.data_buf, PCM_SIZE);
			read += PCM_SIZE;
			//			PcmList::instance()->read(&pcm_chunk);
			break;
		}
		else {
			memcpy(buffer + read * 4, pcm_chunk.data_buf, framesPerBuffer * 4);
			read += framesPerBuffer;
			break;
		}

	}
	return tp;
}

int64_t Stream::getNextPlayerChunk(void* outputBuffer, int64_t timeout, int64_t framesPerBuffer, int64_t framesCorrection)
{
	if (framesCorrection == 0)
		return getNextPlayerChunk(outputBuffer, timeout, framesPerBuffer);

	int frameSize = 4;
	int64_t toRead = framesPerBuffer + framesCorrection;
	char* buffer = (char*)malloc(toRead * frameSize);
	int64_t tp = getNextPlayerChunk(buffer, timeout, toRead);

	//	memcpy(outputBuffer, buffer, frameSize*framesPerBuffer);
	float factor = (float)toRead / framesPerBuffer;//(float)(framesPerBuffer*channels_);
												   //	if (abs(framesCorrection) > 1)
												   //		LOG(INFO) << "correction: " << framesCorrection << ", factor: " << factor << "\n";
	float idx = 0;
	for (size_t n = 0; n < framesPerBuffer; ++n)
	{
		//		cout << "i am here-----------\n";
		size_t index(floor(idx));// = (int)(ceil(n*factor));
		memcpy((char*)outputBuffer + n * frameSize, buffer + index * frameSize, frameSize);
		idx += factor;
	}
	free(buffer);

	return tp;
}

void Stream::updateBuffers(int64_t age)
{
	buffer_.add(age);
	mini_buffer_.add(age);
	short_buffer_.add(age);
}


void Stream::resetBuffers()
{
	buffer_.clear();
	mini_buffer_.clear();
	short_buffer_.clear();
}