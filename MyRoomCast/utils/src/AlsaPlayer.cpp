#include "AlsaPlayer.h"
#include <iostream>

using namespace std;

#define PERIOD_TIME 30000

AlsaPlayer::AlsaPlayer()
{
}

AlsaPlayer::~AlsaPlayer()
{
	snd_pcm_hw_params_free(hw_params);
	snd_pcm_close(playback_handle);
}

void AlsaPlayer::initAlsa()
{
	int i;
	int err;
	short buf[128];
	unsigned int rate = 44100;
	snd_pcm_uframes_t size;

	if ((err = snd_pcm_open(&playback_handle, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
		fprintf(stderr, "cannot open audio device default (%s)\n",
			snd_strerror(err));
		exit(1);
	}

	if ((err = snd_pcm_hw_params_malloc(&hw_params)) < 0) {
		fprintf(stderr, "cannot allocate hardware parameter structure (%s)\n",
			snd_strerror(err));
		exit(1);
	}

	if ((err = snd_pcm_hw_params_any(playback_handle, hw_params)) < 0) {
		fprintf(stderr, "cannot initialize hardware parameter structure (%s)\n",
			snd_strerror(err));
		exit(1);
	}

	if ((err = snd_pcm_hw_params_set_access(playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
		fprintf(stderr, "cannot set access type (%s)\n",
			snd_strerror(err));
		exit(1);
	}

	if ((err = snd_pcm_hw_params_set_format(playback_handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
		fprintf(stderr, "cannot set sample format (%s)\n",
			snd_strerror(err));
		exit(1);
	}

	if ((err = snd_pcm_hw_params_set_rate_near(playback_handle, hw_params, &rate, 0)) < 0) {
		fprintf(stderr, "cannot set sample rate (%s)\n",
			snd_strerror(err));
		exit(1);
	}

	if ((err = snd_pcm_hw_params_set_channels(playback_handle, hw_params, 2)) < 0) {
		fprintf(stderr, "cannot set channel count (%s)\n",
			snd_strerror(err));
		exit(1);
	}

/*	if ((err = snd_pcm_hw_params(playback_handle, hw_params)) < 0) {
		fprintf(stderr, "cannot set parameters (%s)\n",
			snd_strerror(err));
		exit(1);
	}	*/

	unsigned int period_time = 100000;
	/*	snd_pcm_hw_params_get_period_time_max(hw_params, &period_time, 0);


		if (period_time > PERIOD_TIME)
			period_time = PERIOD_TIME; */

	unsigned int buffer_time = 5 * period_time;


	err = snd_pcm_hw_params_set_buffer_time_near(playback_handle, hw_params, &buffer_time, 0);
	if (err < 0) {
		printf("Unable to set buffer time %i for playback: %s\n", buffer_time, snd_strerror(err));
		exit(1);
	}

	err = snd_pcm_hw_params_get_buffer_size(hw_params, &size);
	if (err < 0) {
		printf("Unable to get buffer size for playback: %s\n", snd_strerror(err));
		exit(1);
	}
	buffer_size = size;


	err = snd_pcm_hw_params_set_period_time_near(playback_handle, hw_params, &period_time, 0);
	if (err < 0) {
		printf("Unable to set period time %i for playback: %s\n", period_time, snd_strerror(err));
		exit(1);
	}
	err = snd_pcm_hw_params_get_period_size(hw_params, &size, 0);
	if (err < 0) {
		printf("Unable to get period size for playback: %s\n", snd_strerror(err));
		exit(1);
	}
	period_size = size;


	//	long unsigned int periodsize = stream_->format.msRate() * 50;//2*rate/50;
	//	if ((pcm = snd_pcm_hw_params_set_buffer_size_near(pcm_handle, params, &periodsize)) < 0)
	//		LOG(ERROR) << "Unable to set buffer size " << (long int)periodsize << ": " <<  snd_strerror(pcm) << "\n";

	/* Write parameters */
	if ((err = snd_pcm_hw_params(playback_handle, hw_params)) < 0)
		cout << "Can't set harware parameters: " << string(snd_strerror(err)); 

	/* Resume information */
	cout << "PCM name: " << snd_pcm_name(playback_handle) << "\n";
	cout << "PCM state: " << snd_pcm_state_name(snd_pcm_state(playback_handle)) << "\n";
	unsigned int tmp;
	snd_pcm_hw_params_get_channels(hw_params, &tmp);
	cout << "channels: " << tmp << "\n";

	snd_pcm_hw_params_get_rate(hw_params, &tmp, 0);
	cout << "rate: " << tmp << " bps\n";

	/* Allocate buffer to hold single period */
	snd_pcm_hw_params_get_period_size(hw_params, &frames_, 0);
	cout << "frames: " << frames_ << "\n";




	snd_pcm_hw_params_get_period_time(hw_params, &tmp, NULL);
	cout << "period time: " << tmp << "\n";

	snd_pcm_sw_params_t *swparams;
	snd_pcm_sw_params_alloca(&swparams);
	snd_pcm_sw_params_current(playback_handle, swparams);

	snd_pcm_sw_params_set_start_threshold(playback_handle, swparams, (buffer_size / period_size) * period_size);

	snd_pcm_sw_params_set_avail_min(playback_handle, swparams, period_size);

	//	snd_pcm_sw_params_set_stop_threshold(pcm_handle, swparams, frames_);
	snd_pcm_sw_params(playback_handle, swparams);


}

int AlsaPlayer::playPcm(unsigned char* buf, int buf_size)
{
	int err;
	snd_pcm_sframes_t frames_to_deliver;
	/* wait till the interface is ready for data, or 1 second
	has elapsed.
	*/
	int write_len = 0;
	buf_size /= 4;
	while (buf_size) {

		/*		if ((err = snd_pcm_wait(playback_handle, 1000)) < 0) {
					fprintf(stderr, "poll failed (%s)\n", strerror(errno));
					return -1;
				}*/

				/* find out how much space is available for playback data */

/*		if ((frames_to_deliver = snd_pcm_avail_update(playback_handle)) < 0) {
			if (frames_to_deliver == -EPIPE) {
				fprintf(stderr, "an xrun occured\n");
				xrun_recovery(playback_handle, frames_to_deliver);
				return -1;
			}
			else {
				fprintf(stderr, "unknown ALSA avail update return value (%d)\n",
					frames_to_deliver);
				return -1;
			}
		} */

		frames_to_deliver = frames_to_deliver > 4096 ? 4096 : frames_to_deliver;
		frames_to_deliver = frames_to_deliver > buf_size ? buf_size : frames_to_deliver;
		/* deliver the data */
		if ((err = snd_pcm_writei(playback_handle, buf + write_len * 4, frames_to_deliver)) != frames_to_deliver) {
			xrun_recovery(playback_handle, err);
			return -1;
		}
		buf_size -= frames_to_deliver;
		write_len += frames_to_deliver;
	}
	return write_len * 4;
}


 int AlsaPlayer::xrun_recovery(snd_pcm_t *handle, int err)
{
	
	if (err == -EPIPE) {    /* under-run */
		err = snd_pcm_prepare(handle);
		if (err < 0)
			printf("Can't recovery from underrun, prepare failed: %s\n", snd_strerror(err));
		return 0;
	}
	else if (err == -ESTRPIPE) {
		while ((err = snd_pcm_resume(handle)) == -EAGAIN)
			sleep(1);       /* wait until the suspend flag is released */
		if (err < 0) {
			err = snd_pcm_prepare(handle);
			if (err < 0)
				printf("Can't recovery from suspend, prepare failed: %s\n", snd_strerror(err));
		}
		return 0;
	}
	return err;
}