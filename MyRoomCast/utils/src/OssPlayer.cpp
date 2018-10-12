#include <sys/soundcard.h>
#include <sys/mman.h>

#include <iostream>

#include "BaseMessage.h"
#include "OssPlayer.h"
#include "common.h"

#define  DSP_DEVICE		"/dev/dsp"

extern int server_delay;

OssPlayer::OssPlayer(int format_, int channels_, int samplerate_)
{
	dsp_fd = 0;
	map_fd = 0;
	dmabuffer = NULL;

	format = format_;
	channels = channels_;
	samplerate = samplerate_;
	
//	setDmaBuffer();

}

OssPlayer::~OssPlayer()
{
	deInit();
}

int OssPlayer::init()
{
	dsp_fd = open(DSP_DEVICE, O_WRONLY, 0777);
//	map_fd = open(DSP_DEVICE, O_RDWR);

	if (dsp_fd < 0)
	{
		perror("open");
	}

	
	int rt = setFragment(samplerate);
	if (rt == -1) 
	{
		printf("Cannot set buffer size\n");
		return 1;
	}
	int arg;
	arg = format;
	if (ioctl(dsp_fd, SNDCTL_DSP_SETFMT, &arg) == -1)//设置量化位数
	{
		printf("Cannot set SOUND_PCM_WRITE_BITS\n");
		return 1;
	}

	arg = channels;
	if (ioctl(dsp_fd, SNDCTL_DSP_CHANNELS, &arg) == -1)//设置声道数
	{
		printf("Cannot set SOUND_PCM_WRITE_CHANNELS\n");
		return 1;
	}

	arg = samplerate;
	if (ioctl(dsp_fd, SNDCTL_DSP_SPEED, &arg) == -1)//设置采样率
	{
		printf("Cannot set SOUND_PCM_WRITE_WRITE\n");
		return 1;
	}

	ioctl(dsp_fd, SNDCTL_DSP_GETBLKSIZE, &frag_size);
	printf("fragment size is:%d \n", frag_size);

	return 0;
}

void OssPlayer::deInit()
{
	if (dsp_fd > 0)
		close(dsp_fd);
	if(map_fd > 0)
		close(map_fd);
	if(dmabuffer)
		munmap(dmabuffer, dmabuffer_size);
}


int  OssPlayer::setFragment(int samplerate)
{
	int marg = 0;

	if (samplerate == 44100)
	{
//		marg = 15 | (4 << 16);
		marg = 10 | (4 << 16);
		printf("marg=0x%x\n", marg);
	}

	
	return ioctl(dsp_fd, SNDCTL_DSP_SETFRAGMENT, &marg);
}

int OssPlayer::setDmaBuffer()
{
	/* Variables for ioctl's. */
	unsigned int requested, ioctl_caps, ioctl_enable;
	audio_buf_info ioctl_info;

	/* Buffer information. */
	unsigned int frag_count, frag_size;
	dmabuffer = NULL;
	dmabuffer_size = 0;
	dmabuffer_flag = 0;

	if (ioctl(map_fd, SNDCTL_DSP_GETCAPS, &ioctl_caps) != 0) {
		perror("DMA player: unable to read sound driver capabilities");
		return -1;
	}

	/* The MMAP and TRIGGER bits must be set for this to work.
	MMAP gives us the ability to access the DMA buffer directly,
	and TRIGGER gives us the ability to start the sound card's
	playback with a special ioctl. */
	if (!(ioctl_caps & DSP_CAP_MMAP) || !(ioctl_caps & DSP_CAP_TRIGGER)) {
		printf("DMA player: this sound driver is not capable of direct DMA.");
		return -1;
	}

	/* Query the sound driver for the actual fragment configuration
	so that we can calculate the total size of the DMA buffer.
	Note that we haven't selected a particular fragment size or
	count. Fragment boundaries are meaningless in a mapped buffer;
	we're really just interested in the total size. */
	if (ioctl(map_fd, SNDCTL_DSP_GETOSPACE, &ioctl_info) != 0) {
		perror("DMA player: unable to query buffer information");
		return -1;
	}

	frag_count = ioctl_info.fragstotal;
	frag_size = ioctl_info.fragsize;
	dmabuffer_size = frag_count * frag_size;

	/* We're good to go. Map a buffer onto the audio device. */
	dmabuffer = (unsigned char *)mmap(NULL,
		dmabuffer_size,         /* length of region to map */
		PROT_WRITE | PROT_READ,             /* select the output buffer
										   (PROT_READ alone selects input) */
										   /* NOTE: I had to add PROT_READ to
										   make this work with FreeBSD.
										   However, this causes the code to
										   fail under Linux. SNAFU. */
		MAP_FILE | MAP_SHARED,	 /* see the mmap() manual page */
		map_fd,			 /* opened file to map */
		0);			 /* start at offset zero */

					 /* This could fail for a number of reasons. */
	if (dmabuffer == (u_int8_t *)MAP_FAILED) {
		perror("DMA player: unable to mmap a DMA buffer");
		return -1;
	}

	/* Clear the buffer to avoid static at the beginning. */
	memset(dmabuffer, 0, dmabuffer_size);

	/* The DMA buffer is ready! Now we can start playback by toggling
	the device's PCM output bit. Yes, this is a very hacky interface.
	We're actually using the OSS "trigger" functionality here. */
	ioctl_enable = 0;
	if (ioctl(map_fd, SNDCTL_DSP_SETTRIGGER, &ioctl_enable) != 0) {
		perror("DMA player: unable to disable PCM output");
		return -1;
	}

	ioctl_enable = PCM_ENABLE_OUTPUT;
	if (ioctl(map_fd, SNDCTL_DSP_SETTRIGGER, &ioctl_enable) != 0) {
		perror("DMA player: unable to enable PCM output");
		return -1;
	}

}

int OssPlayer::playDma(unsigned char *samples,  unsigned int bytes)
{

	/* Playback status variables. */
	unsigned int position = 0, done = 0;
	while (done < 4) {
		struct count_info status;
		unsigned int i;

		/* Find the location of the DMA controller within the buffer.
		This will be exact at least to the level of a fragment. */
		if (ioctl(map_fd, SNDCTL_DSP_GETOPTR, &status) != 0) {
			perror("DMA player: unable to query playback status");
			return -1;
		}

		/* Our buffer is comprised of several fragments. However, in DMA
		mode, it is safe to treat the entire buffer as one big block.
		We will divide it into two logical chunks. While the first chunk
		is playing, we will fill the second with new samples, and
		vice versa. With a small buffer, we will still enjoy low latency.

		status.ptr contains the offset of the DMA controller within
		the buffer. */

		if (dmabuffer_flag == 0) {
			/* Do we need to refill the first chunk? */
			if ((unsigned)status.ptr < dmabuffer_size / 2) {
				unsigned int amount;

				/* Copy data into the DMA buffer. */
				if (bytes - position < dmabuffer_size / 2) {
					amount = bytes - position;
				}
				else amount = dmabuffer_size / 2;

				for (i = 0; i < amount; i++) {
					dmabuffer[i + dmabuffer_size / 2] = samples[position + i];
				}

				/* Zero the rest of this half. */
				for (; i < dmabuffer_size / 2; i++) {
					dmabuffer[i + dmabuffer_size / 2] = 0;
				}

				/* Update the buffer position. */
				position += amount;

				/* Next update will be the first chunk. */
				dmabuffer_flag = 1;

				/* Have we reached the end? */
				if (position >= bytes) done++;
			}
		}
		else if (dmabuffer_flag == 1) {
			/* Do we need to refill the first chunk? */
			if ((unsigned)status.ptr >= dmabuffer_size / 2) {
				unsigned int amount;

				/* Copy data into the DMA buffer. */
				if (bytes - position < dmabuffer_size / 2) {
					amount = bytes - position;
				}
				else amount = dmabuffer_size / 2;

				for (i = 0; i < amount; i++) {
					dmabuffer[i] = samples[position + i];
				}

				/* Zero the rest of this half. */
				for (; i < dmabuffer_size / 2; i++) {
					dmabuffer[i] = 0;
				}

				/* Update the buffer position. */
				position += amount;

				/* Next update will be the second chunk. */
				dmabuffer_flag = 0;

				/* Have we reached the end? */
				if (position >= bytes) done++;
			}
		}

//		WritePlaybackStatus(position, bytes, channels, bits, rate);
		printf(" (%i)\r", dmabuffer_flag);
		fflush(stdout);

		/* Wait a while. A game would normally do the rest of its
		processing here. */
		usleep(50);
	}

}

int OssPlayer::playPcm(unsigned char *samples, unsigned int bytes)
{
//	printf("dsp fd = %d\n", dsp_fd);
	static int tag = 0;
//	tv current;
//	static int64_t last = current.getUs();
//	char cmd[64];
	audio_buf_info info;

	if (ioctl(dsp_fd, SNDCTL_DSP_GETOSPACE, &info) != 0) {
			perror("Oss player: unable to query buffer information");
//			close(dsp_fd);
			return -1;
	}
/*	if (info.bytes == 0) {	//don't play, just drop it
		tv current;
//		std::string s(current.getUs() - last);
		snprintf(cmd, 63, "echo %lu >> /tmp/log.txt", current.getUs()-last);
		std::cout << current.getUs() - last << std::endl;
		system(cmd);
		last = current.getUs();
		printf("i drop it--bytes:%d  fragments:%d   fragsize:%d   fragstotal:%d\n", info.bytes, info.fragments, info.fragsize, info.fragstotal);
		return bytes;
	}		   */
	if (tag++ % 50 == 0) {
		printf("server_delay:%d bytes:%d  fragments:%d   fragsize:%d   fragstotal:%d\n", server_delay, info.bytes, info.fragments, info.fragsize, info.fragstotal);
	}
	if (info.bytes < 8192 && server_delay == 0) {
		server_delay = 100;
	}else if (info.bytes > 12888 && server_delay > 0) {
		server_delay -= 10;
	}
																		 
	int rt = write(dsp_fd, samples, bytes);
	if (rt < 0) {
		perror("write");
		return -1;
	}
	return rt;	 
}
