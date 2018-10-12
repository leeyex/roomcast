#pragma once

class OssPlayer 
{
public:
	OssPlayer(int format, int channels, int samplerate);
	int init();
	int playDma(unsigned char *samples, unsigned int bytes);
	int playPcm(unsigned char *samples, unsigned int bytes);

	int setDmaBuffer();				//if return -1£¬ does not support direct DMA
	~OssPlayer();
private:
	
	void deInit();

	int setFragment(int samplerate);

	
	int format; 
	int channels;
	int samplerate;

	int dsp_fd;
	int map_fd;
//	audio_buf_info zz;

	int frag_size;

	unsigned int dmabuffer_size ;
	int dmabuffer_flag ;
	unsigned char *dmabuffer;

};
