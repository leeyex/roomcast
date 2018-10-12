#pragma once
#include <alsa/asoundlib.h>

class AlsaPlayer
{
public:
	AlsaPlayer();
	~AlsaPlayer();
	void initAlsa();
	int playPcm(unsigned char *buf, int buf_size);
	int xrun_recovery(snd_pcm_t * handle, int err);
private:
	snd_pcm_t * playback_handle;
	snd_pcm_hw_params_t *hw_params;
	snd_pcm_uframes_t frames_;

	snd_pcm_sframes_t buffer_size;
	snd_pcm_sframes_t period_size;
};
